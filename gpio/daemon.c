/* SPDX-License-Identifier: GPL-2.0-only
 * Board LED policy, not a general GPIO interface. All hardware paths are fixed.
 */
#include "config.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/audit.h>
#include <linux/capability.h>
#include <linux/filter.h>
#include <linux/if_bridge.h>
#include <linux/if_link.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/seccomp.h>
#include <linux/if.h>
#include <netinet/ip.h>
#include <linux/icmp.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/file.h>
#include <sys/inotify.h>
#include <sys/prctl.h>
#include <sys/random.h>
#include <sys/resource.h>
#include <sys/signalfd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/timerfd.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#define CLIENTS 32
#define UID_SLOTS 32
#define LEDS 7
#define LEASES 128
#define NEVER UINT64_MAX
enum source { LISTEN_FD = 1, TIMER_FD, SIGNAL_FD, ROUTE_FD, WATCH_FD,
	PING_FD, HOST0_FD, HOST1_FD, CLIENT_FD = 32 };
static const char *const led_names[LEDS] = {
	"power", "lan", "internetgreen", "internetorange", "wlan2g", "wlan5g", "wps"
};
static const unsigned target_led[TARGETS] = {0, 1, 2, 4, 5, 6};
static const char *const lan_names[4] = {"eth0.2", "eth0.3", "eth0.4", "eth0.5"};
struct animation {
	enum effect effect;
	bool on;
	unsigned remaining;
	uint64_t next, identify_until;
};
struct client {
	int fd;
	uid_t uid;
	bool subscribed;
	uint64_t expires;
};
struct rate {
	uid_t uid;
	bool used;
	unsigned requests, identifies;
	uint64_t window;
};
struct radio {
	int fd;
	unsigned backoff;
	uint64_t retry, deadline;
	bool enabled;
};
struct lease { unsigned char mac[6]; uint64_t expires; };
struct daemon {
	int ep, listener, timer, signal, route, watch, ping, lock;
	int led[LEDS], last[LEDS];
	bool test, running, handoff, wan_carrier, bound, online, pending, lease_ack, resync;
	unsigned lan_mask, selected, failed_probes, failure_mask, wan_index, dump_seq, dump_type;
	unsigned state[TARGETS];
	uint16_t ping_id, ping_seq;
	uint64_t nonce, handoff_until, handoff_next, probe_next, probe_deadline, ack_at;
	char prefix[PATH_MAX], wan[IFNAMSIZ];
	struct gpio_config config;
	struct animation anim[TARGETS];
	struct client client[CLIENTS];
	struct rate rate[UID_SLOTS];
	struct radio radio[2];
	struct lease leases[LEASES];
	unsigned lease_count;
};
static uint64_t now_ms(void)
{
	struct timespec ts;
	if (clock_gettime(CLOCK_MONOTONIC, &ts)) abort();
	return (uint64_t)ts.tv_sec * 1000 + (unsigned)ts.tv_nsec/1000000;
}
static void path_of(const struct daemon *d, char *buf, size_t n, const char *fixed)
{
	if (snprintf(buf, n, "%s%s", d->prefix, fixed) >= (int)n) abort();
}
static int open_fixed(const struct daemon *d, const char *fixed, int flags)
{
	char path[PATH_MAX];
	path_of(d, path, sizeof(path), fixed);
	return open(path, flags | O_CLOEXEC | O_NOFOLLOW, 0600);
}
static void add_fd(struct daemon *d, int fd, unsigned source)
{
	struct epoll_event ev = { .events = EPOLLIN, .data.u32 = source };
	if (epoll_ctl(d->ep, EPOLL_CTL_ADD, fd, &ev)) { perror("gpio-daemon: epoll add"); exit(1); }
}
static void del_fd(struct daemon *d, int *fd)
{
	if (*fd >= 0) { epoll_ctl(d->ep, EPOLL_CTL_DEL, *fd, NULL); close(*fd); *fd = -1; }
}
static int read_fixed(const struct daemon *d, const char *path, char *buf, size_t n)
{
	int fd = open_fixed(d, path, O_RDONLY | O_NONBLOCK);
	if (fd < 0) return -1;
	ssize_t r = read(fd, buf, n-1);
	close(fd);
	if (r < 0) return -1;
	buf[r] = 0;
	return (int)r;
}
static bool owned_directory(const char *path, mode_t mode, bool test)
{
	if (mkdir(path, mode) && errno != EEXIST) return false;
	int fd = open(path, O_DIRECTORY | O_NOFOLLOW | O_RDONLY | O_CLOEXEC);
	struct stat st;
	bool ok = fd >= 0 && !fstat(fd, &st) && S_ISDIR(st.st_mode) &&
		st.st_uid == (test ? geteuid() : 0) && !(st.st_mode & 0022);
	if (fd >= 0) close(fd);
	return ok;
}
static int load_config(struct daemon *d)
{
	int dir = open_fixed(d, "/etc/phoebus", O_RDONLY | O_DIRECTORY);
	if (dir < 0) return -1;
	struct stat st;
	int fd = -1, ret = -1;
	if (fstat(dir, &st) || st.st_uid != (d->test ? geteuid() : 0) || (st.st_mode & 0022)) goto out;
	fd = openat(dir, "gpio.conf", O_RDONLY | O_NOFOLLOW | O_CLOEXEC | O_NONBLOCK);
	if (fd < 0 || fstat(fd, &st) || !S_ISREG(st.st_mode) || st.st_nlink != 1 ||
	    st.st_uid != (d->test ? geteuid() : 0) || (st.st_mode & 0022) || st.st_size > 8192) goto out;
	struct gpio_config next;
	struct stat after;
	if (!config_read(fd, &next) && !fstat(fd, &after) && after.st_size == st.st_size &&
	    after.st_mtim.tv_sec == st.st_mtim.tv_sec && after.st_mtim.tv_nsec == st.st_mtim.tv_nsec &&
	    after.st_ctim.tv_sec == st.st_ctim.tv_sec && after.st_ctim.tv_nsec == st.st_ctim.tv_nsec) {
		d->config = next; ret = 0;
	}
out:
	if (fd >= 0) close(fd);
	close(dir);
	return ret;
}
static void led_write(struct daemon *d, unsigned led, unsigned value)
{
	if (!d->handoff || d->led[led] < 0 || d->last[led] == (int)value) return;
	char buf[16];
	int n = snprintf(buf, sizeof(buf), "%u\n", value);
	if (lseek(d->led[led], 0, SEEK_SET) < 0 || write(d->led[led], buf, (size_t)n) != n) {
		fprintf(stderr, "gpio-daemon: LED %s unavailable; networking unaffected\n", led_names[led]);
		d->failure_mask |= 1u << led;
		close(d->led[led]); d->led[led] = -1;
		return;
	}
	d->last[led] = (int)value;
}
static void render(struct daemon *d, unsigned t, uint64_t now)
{
	struct animation *a = &d->anim[t];
	bool on = a->on;
	if (a->identify_until > now) on = true;
	unsigned brightness = on ? d->config.brightness : 0;
	if (t == INTERNET) {
		/* Carrier absence ALWAYS wins, including unprivileged identification. */
		if (!d->wan_carrier) brightness = 0;
		bool orange = a->effect == EFFECT_ORANGE || a->effect == EFFECT_BLINK_ORANGE;
		led_write(d, 2, orange ? 0 : brightness);
		led_write(d, 3, orange ? brightness : 0);
	} else led_write(d, target_led[t], brightness);
}
static void start_rule(struct daemon *d, unsigned t, unsigned rule, uint64_t now)
{
	struct animation *a = &d->anim[t];
	d->state[t] = rule;
	a->effect = d->config.target[t].action[rule];
	a->remaining = 0;
	a->next = NEVER;
	a->on = a->effect != EFFECT_OFF;
	if (a->effect == EFFECT_CONNECTED) a->on = d->lan_mask != 0;
	if (a->effect == EFFECT_BLINK || a->effect == EFFECT_BLINK_ORANGE || a->effect == EFFECT_FAST)
		a->next = now + (a->effect == EFFECT_FAST ? 100 : d->config.target[t].on_ms);
	if (a->effect == EFFECT_PULSE) {
		a->remaining = 2 * (d->config.pulse_count ? d->config.pulse_count : d->selected);
		if (a->remaining) a->next = now + d->config.target[t].on_ms;
		else a->on = d->lan_mask != 0;
	}
	if (a->effect == EFFECT_FLASH) { a->on = false; a->remaining = 1; a->next = now + 100; }
	render(d, t, now);
}
static void update_internet(struct daemon *d, uint64_t now)
{
	unsigned rule = !d->wan_carrier ? R_STOP : d->online ? R_ONLINE :
		d->failed_probes >= 3 ? R_LOST : d->bound ? R_READY : R_START;
	if (d->state[INTERNET] != rule) start_rule(d, INTERNET, rule, now);
}
static void close_client(struct daemon *d, unsigned i)
{
	del_fd(d, &d->client[i].fd);
	d->client[i].subscribed = false;
}
static struct gpio_msg snapshot(const struct daemon *d, unsigned op)
{
	struct gpio_msg m = gpio_message(op);
	m.data[0] = d->lan_mask; m.data[1] = d->selected;
	m.data[2] = d->wan_carrier | (d->bound << 1) | (d->online << 2);
	for (unsigned t = 0; t < TARGETS; ++t) m.data[3+t] = d->state[t];
	m.data[9] = d->failure_mask;
	return m;
}
static void publish(struct daemon *d)
{
	struct gpio_msg m = snapshot(d, GPIO_UPDATE);
	for (unsigned i = 0; i < CLIENTS; ++i)
		if (d->client[i].fd >= 0 && d->client[i].subscribed &&
		    send(d->client[i].fd, &m, sizeof(m), MSG_DONTWAIT | MSG_NOSIGNAL) != sizeof(m))
			close_client(d, i);
}
static void wan_clear(struct daemon *d)
{
	d->bound = d->online = d->pending = false;
	d->failed_probes = 0; d->probe_next = NEVER;
}
static bool allowed_wan(const char *name)
{
	return !strcmp(name, "nas0") || (strlen(name) == 6 && !strncmp(name, "eth0.", 5) && name[5] >= '2' && name[5] <= '9');
}
static bool select_wan(struct daemon *d)
{
	char name[IFNAMSIZ+2] = {0};
	if (read_fixed(d, "/run/wan_iface", name, sizeof(name)) > 0) name[strcspn(name, "\r\n")] = 0;
	if (!allowed_wan(name)) {
		/* The daemon may start before network-up creates wan_iface. The boot
		 * argument is the same protected selection used by that script. */
		char cmdline[4096], *save;
		if (read_fixed(d, "/proc/cmdline", cmdline, sizeof(cmdline)) > 0)
			for (char *p = strtok_r(cmdline, " \n", &save); p; p = strtok_r(NULL, " \n", &save))
				if (!strncmp(p, "wan=", 4) && allowed_wan(p+4)) strcpy(name, p+4);
	}
	char path[PATH_MAX]; path_of(d, path, sizeof(path), "/run/wan_iface");
	(void)inotify_add_watch(d->watch, path, IN_CLOSE_WRITE | IN_MOVE_SELF | IN_DELETE_SELF);
	if (!allowed_wan(name) || !strcmp(name, d->wan)) return false;
	strcpy(d->wan, name); d->wan_carrier = false; d->wan_index = 0; wan_clear(d);
	return true;
}
static void link_event(struct daemon *d, const char *name, unsigned index, bool up, uint64_t now)
{
	if (!strcmp(name, d->wan)) {
		bool changed = d->wan_carrier != up;
		d->wan_index = index; d->wan_carrier = up;
		if (!up) wan_clear(d);
		else if (changed && d->bound) d->probe_next = now;
		update_internet(d, now);
		if (changed) publish(d);
		return;
	}
	for (unsigned i = 0; i < 4; ++i) {
		if (strcmp(name, lan_names[i])) continue;
		unsigned bit = 1u << i;
		if (!!(d->lan_mask & bit) == up) return;
		if (up) {
			d->lan_mask |= bit; d->selected = i+1;
			d->lease_ack = false; d->ack_at = NEVER;
			start_rule(d, LAN, R_START, now);
		} else {
			d->lan_mask &= ~bit;
			if (d->selected == i+1 || !d->lan_mask) {
				d->selected = 0; d->lease_ack = false; d->ack_at = NEVER;
				start_rule(d, LAN, d->lan_mask ? R_STOP : R_FAIL, now);
			}
		}
		publish(d);
	}
}
static void route_dump(struct daemon *d, unsigned type)
{
	if (d->route < 0) return;
	/* One netlink socket supports one active dump. Coalesce hotplug bursts
	 * instead of issuing overlapping requests that return EBUSY. */
	if (d->dump_type) { d->resync = true; return; }
	struct { struct nlmsghdr h; struct rtgenmsg g; } q = {
		.h = { .nlmsg_len = NLMSG_LENGTH(sizeof(struct rtgenmsg)), .nlmsg_type = type,
			.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP, .nlmsg_seq = ++d->dump_seq },
		.g = { .rtgen_family = AF_UNSPEC }
	};
	struct sockaddr_nl peer = { .nl_family = AF_NETLINK };
	if (sendto(d->route, &q, q.h.nlmsg_len, MSG_DONTWAIT,
		(struct sockaddr *)&peer, sizeof(peer)) == (ssize_t)q.h.nlmsg_len) d->dump_type = type;
}
static void route_read(struct daemon *d, uint64_t now)
{
	for (unsigned budget = 0; budget < 16; ++budget) {
		if (d->route < 0) return;
		char buf[16384] __attribute__((aligned(__alignof__(struct nlmsghdr))));
		struct sockaddr_nl peer = {0};
		struct iovec io = { .iov_base = buf, .iov_len = sizeof(buf) };
		struct msghdr msg = { .msg_name = &peer, .msg_namelen = sizeof(peer), .msg_iov = &io, .msg_iovlen = 1 };
		ssize_t got = recvmsg(d->route, &msg, MSG_DONTWAIT);
		if (got < 0) {
			if (errno == ENOBUFS) { d->dump_type = 0; route_dump(d, RTM_GETLINK); }
			return;
		}
		if (msg.msg_namelen < sizeof(peer) || peer.nl_family != AF_NETLINK ||
		    peer.nl_pid || (msg.msg_flags & MSG_TRUNC)) continue;
		int len = (int)got;
		for (struct nlmsghdr *h = (void *)buf; NLMSG_OK(h, len); h = NLMSG_NEXT(h, len)) {
			if (h->nlmsg_type == NLMSG_DONE && h->nlmsg_seq == d->dump_seq) {
				unsigned type = d->dump_type; d->dump_type = 0;
				if (h->nlmsg_flags & NLM_F_DUMP_INTR) d->resync = true;
				if (type == RTM_GETLINK) route_dump(d, RTM_GETADDR);
				else if (d->resync) { d->resync = false; route_dump(d, RTM_GETLINK); }
				continue;
			}
			if (h->nlmsg_type == NLMSG_ERROR && h->nlmsg_seq == d->dump_seq) {
				d->dump_type = 0; continue;
			}
			if (h->nlmsg_type == RTM_NEWLINK || h->nlmsg_type == RTM_DELLINK) {
				if (h->nlmsg_len < NLMSG_LENGTH(sizeof(struct ifinfomsg))) continue;
				struct ifinfomsg *ifi = NLMSG_DATA(h);
				int alen = IFLA_PAYLOAD(h);
				char name[IFNAMSIZ] = {0};
				for (struct rtattr *a = IFLA_RTA(ifi); RTA_OK(a, alen); a = RTA_NEXT(a, alen))
					if (a->rta_type == IFLA_IFNAME && RTA_PAYLOAD(a) > 0 && RTA_PAYLOAD(a) <= IFNAMSIZ &&
					    memchr(RTA_DATA(a), 0, RTA_PAYLOAD(a))) memcpy(name, RTA_DATA(a), RTA_PAYLOAD(a));
				if (*name) link_event(d, name, (unsigned)ifi->ifi_index,
					h->nlmsg_type != RTM_DELLINK && (ifi->ifi_flags & IFF_LOWER_UP), now);
			} else if (h->nlmsg_type == RTM_NEWADDR || h->nlmsg_type == RTM_DELADDR) {
				if (h->nlmsg_len < NLMSG_LENGTH(sizeof(struct ifaddrmsg))) continue;
				struct ifaddrmsg *ifa = NLMSG_DATA(h);
				if (ifa->ifa_index != d->wan_index || ifa->ifa_family != AF_INET || ifa->ifa_scope != RT_SCOPE_UNIVERSE) continue;
				d->bound = h->nlmsg_type == RTM_NEWADDR;
				if (d->bound && d->wan_carrier) d->probe_next = now;
				else if (!d->bound) wan_clear(d);
				update_internet(d, now); publish(d);
			}
		}
	}
}
static void radio_close(struct daemon *d, unsigned r, uint64_t now)
{
	del_fd(d, &d->radio[r].fd);
	d->radio[r].enabled = false;
	d->radio[r].deadline = NEVER;
	if (d->state[WIFI_5G-r] != R_STOP) {
		start_rule(d, WIFI_5G-r, R_START, now);
		d->radio[r].retry = now + d->radio[r].backoff;
		if (d->radio[r].backoff < 30000) d->radio[r].backoff *= 2;
	}
}
static void radio_attach(struct daemon *d, unsigned r, uint64_t now)
{
	struct radio *radio = &d->radio[r];
	radio->retry = NEVER;
	if (radio->fd >= 0) return;
	int fd = socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
	if (fd < 0) return;
	struct sockaddr_un local = { .sun_family = AF_UNIX }, remote = { .sun_family = AF_UNIX };
	char fixed[80];
	snprintf(fixed, sizeof(fixed), "/run/phoebus/hostapd-%u", r);
	path_of(d, local.sun_path, sizeof(local.sun_path), fixed);
	snprintf(fixed, sizeof(fixed), "/run/hostapd/wlan%u", r);
	path_of(d, remote.sun_path, sizeof(remote.sun_path), fixed);
	unlink(local.sun_path);
	if (bind(fd, (struct sockaddr *)&local, sizeof(local)) ||
	    chmod(local.sun_path, 0600) || connect(fd, (struct sockaddr *)&remote, sizeof(remote)) ||
	    send(fd, "ATTACH", 6, MSG_DONTWAIT | MSG_NOSIGNAL) != 6) {
		close(fd); unlink(local.sun_path);
		radio_close(d, r, now);
		return;
	}
	radio->fd = fd; radio->deadline = now + 2000;
	add_fd(d, fd, HOST0_FD+r);
}
static void radio_read(struct daemon *d, unsigned r, uint64_t now)
{
	struct radio *radio = &d->radio[r];
	for (unsigned budget = 0; budget < 16; ++budget) {
		char buf[4096];
		ssize_t n = recv(radio->fd, buf, sizeof(buf)-1, MSG_DONTWAIT | MSG_TRUNC);
		if (n < 0 && (errno == EAGAIN || errno == EINTR)) return;
		if (n <= 0) { radio_close(d, r, now); return; }
		if (n >= (ssize_t)sizeof(buf)) continue;
		buf[n] = 0;
		if (!strcmp(buf, "OK\n")) {
			if (send(radio->fd, "STATUS", 6, MSG_DONTWAIT | MSG_NOSIGNAL) != 6) { radio_close(d, r, now); return; }
			radio->deadline = now + 2000;
			continue;
		}
		bool enabled = strstr(buf, "state=ENABLED\n") || strstr(buf, ">AP-ENABLED");
		bool disabled = strstr(buf, ">AP-DISABLED") || strstr(buf, ">INTERFACE-DISABLED");
		if (!strncmp(buf, "state=", 6) || strstr(buf, "\nstate=")) radio->deadline = NEVER;
		if (enabled || disabled) {
			radio->enabled = enabled; radio->deadline = NEVER; radio->backoff = 250;
			unsigned rule = enabled ? R_READY : R_FAIL;
			if (d->state[WIFI_5G-r] != rule) { start_rule(d, WIFI_5G-r, rule, now); publish(d); }
		}
		if (strstr(buf, ">CTRL-EVENT-TERMINATING")) { radio_close(d, r, now); return; }
	}
}
static void watch_refresh(struct daemon *d)
{
	char path[PATH_MAX];
	path_of(d, path, sizeof(path), "/run/hostapd");
	(void)inotify_add_watch(d->watch, path, IN_CREATE | IN_DELETE | IN_MOVED_TO | IN_DELETE_SELF);
}
static void watch_read(struct daemon *d, uint64_t now)
{
	char buf[4096] __attribute__((aligned(__alignof__(struct inotify_event))));
	ssize_t n;
	for (unsigned budget = 0; budget < 8 && (n = read(d->watch, buf, sizeof(buf))) > 0; ++budget) {
		for (size_t p = 0; p + sizeof(struct inotify_event) <= (size_t)n;) {
			struct inotify_event *e = (void *)(buf+p);
			if (e->len > (size_t)n-p-sizeof(*e)) break;
			if ((e->mask & IN_CLOSE_WRITE) || (e->len && memchr(e->name, 0, e->len) && !strcmp(e->name, "wan_iface"))) {
				if (select_wan(d)) { update_internet(d, now); route_dump(d, RTM_GETLINK); }
			}
			if (e->len && memchr(e->name, 0, e->len)) {
				if (!strcmp(e->name, "hostapd")) watch_refresh(d);
				for (unsigned r = 0; r < 2; ++r) {
					const char *name = r ? "wlan1" : "wlan0";
					if (strcmp(e->name, name)) continue;
					if (e->mask & IN_DELETE) radio_close(d, r, now);
					if (e->mask & (IN_CREATE | IN_MOVED_TO)) d->radio[r].retry = now;
				}
			}
			p += sizeof(*e)+e->len;
		}
	}
}
static uint64_t be_integer(const unsigned char *p, unsigned bytes)
{
	uint64_t n = 0;
	for (unsigned i = 0; i < bytes; ++i) n = (n << 8) | p[i];
	return n;
}
static unsigned lease_port(struct daemon *d, const unsigned char *mac)
{
	int fd = open_fixed(d, "/sys/class/net/br0/brforward", O_RDONLY);
	if (fd < 0) return 0;
	struct __fdb_entry fdb[256];
	ssize_t n = read(fd, fdb, sizeof(fdb));
	close(fd);
	unsigned port = 0;
	for (unsigned i = 0; n > 0 && i < (unsigned)n/sizeof(fdb[0]); ++i)
		if (!fdb[i].is_local && !memcmp(fdb[i].mac_addr, mac, 6)) {
			port = fdb[i].port_no | ((unsigned)fdb[i].port_hi << 8); break;
		}
	if (!port) return 0;
	for (unsigned i = 0; i < 4; ++i) {
		char path[96], buf[24];
		snprintf(path, sizeof(path), "/sys/class/net/br0/brif/%s/port_no", lan_names[i]);
		if (read_fixed(d, path, buf, sizeof(buf)) > 0) {
			char *end;
			unsigned long num = strtoul(buf, &end, 0);
			if (end != buf && num == port) return i+1;
		}
	}
	return 0;
}
static void leases_read(struct daemon *d, bool initial, uint64_t now)
{
	unsigned char buf[8+36*LEASES+1];
	int fd = open_fixed(d, "/var/lib/misc/udhcpd.leases", O_RDONLY | O_NONBLOCK);
	if (fd < 0) return;
	ssize_t n = read(fd, buf, sizeof(buf));
	close(fd);
	if (n < 8 || n == sizeof(buf) || (n-8)%36) return;
	struct lease next[LEASES];
	unsigned count = 0;
	uint64_t written = be_integer(buf, 8);
	for (size_t p = 8; p+36 <= (size_t)n; p += 36) {
		uint64_t duration = be_integer(buf+p, 4);
		if (!duration || written > UINT64_MAX-duration) continue;
		struct lease *l = &next[count++];
		memcpy(l->mac, buf+p+8, 6); l->expires = written+duration;
		bool changed = true;
		for (unsigned i = 0; i < d->lease_count; ++i)
			if (!memcmp(l->mac, d->leases[i].mac, 6) &&
			    l->expires <= d->leases[i].expires + (d->leases[i].expires <= UINT64_MAX-2 ? 2 : 0)) changed = false;
		if (!initial && changed && d->selected && lease_port(d, l->mac) == d->selected && !d->lease_ack) {
			d->lease_ack = true;
			d->ack_at = d->anim[LAN].remaining ? NEVER : now+1000;
		}
	}
	memcpy(d->leases, next, count*sizeof(next[0])); d->lease_count = count;
}
static uint16_t checksum(const unsigned char *p, size_t n)
{
	unsigned sum = 0;
	for (size_t i = 0; i+1 < n; i += 2) sum += ((unsigned)p[i] << 8) | p[i+1];
	if (n & 1) sum += (unsigned)p[n-1] << 8;
	while (sum >> 16) sum = (sum & 65535) + (sum >> 16);
	return (uint16_t)~sum;
}
static void probe_failed(struct daemon *d, uint64_t now)
{
	d->pending = false;
	if (d->failed_probes < 3) ++d->failed_probes;
	if (d->failed_probes >= 3) d->online = false;
	d->probe_next = now + 30000;
	update_internet(d, now); publish(d);
}
static void probe_send(struct daemon *d, uint64_t now)
{
	d->probe_next = NEVER;
	if (d->ping < 0 || !d->bound || !d->wan_carrier || !d->wan_index) return;
	unsigned char packet[16] = { ICMP_ECHO, 0, 0, 0 };
	++d->ping_seq;
	packet[4] = d->ping_id >> 8; packet[5] = d->ping_id & 255;
	packet[6] = d->ping_seq >> 8; packet[7] = d->ping_seq & 255;
	memcpy(packet+8, &d->nonce, 8);
	uint16_t sum = checksum(packet, sizeof(packet));
	packet[2] = sum >> 8; packet[3] = sum & 255;
	struct sockaddr_in dst = { .sin_family = AF_INET, .sin_addr.s_addr = htonl(0x01010101) };
	union { struct cmsghdr align; unsigned char data[CMSG_SPACE(sizeof(struct in_pktinfo))]; } control = {0};
	struct iovec io = { .iov_base = packet, .iov_len = sizeof(packet) };
	struct msghdr m = { .msg_name = &dst, .msg_namelen = sizeof(dst), .msg_iov = &io, .msg_iovlen = 1,
		.msg_control = control.data, .msg_controllen = sizeof(control.data) };
	struct cmsghdr *c = CMSG_FIRSTHDR(&m);
	c->cmsg_level = IPPROTO_IP; c->cmsg_type = IP_PKTINFO; c->cmsg_len = CMSG_LEN(sizeof(struct in_pktinfo));
	struct in_pktinfo *info = (void *)CMSG_DATA(c);
	memset(info, 0, sizeof(*info)); info->ipi_ifindex = (int)d->wan_index;
	if (sendmsg(d->ping, &m, MSG_DONTWAIT | MSG_NOSIGNAL) != sizeof(packet)) { probe_failed(d, now); return; }
	d->pending = true; d->probe_deadline = now+1500;
}
static void probe_read(struct daemon *d, uint64_t now)
{
	unsigned char buf[512];
	for (unsigned budget = 0; budget < 16; ++budget) {
		struct sockaddr_in peer = {0};
		socklen_t len = sizeof(peer);
		ssize_t n = recvfrom(d->ping, buf, sizeof(buf), MSG_DONTWAIT, (struct sockaddr *)&peer, &len);
		if (n < 0) return;
		if (!d->pending || n < 20 || len < sizeof(peer) || peer.sin_family != AF_INET ||
		    peer.sin_addr.s_addr != htonl(0x01010101) || buf[0] >> 4 != 4) continue;
		size_t off = (buf[0] & 15u)*4;
		if (off < 20 || (size_t)n < off+16) continue;
		unsigned char *p = buf+off;
		if (p[0] || p[1] || ((unsigned)p[4] << 8 | p[5]) != d->ping_id ||
		    ((unsigned)p[6] << 8 | p[7]) != d->ping_seq || memcmp(p+8, &d->nonce, 8)) continue;
		d->online = true; d->pending = false; d->failed_probes = 0; d->probe_next = now+30000;
		update_internet(d, now); publish(d);
	}
}
static void tick(struct daemon *d, uint64_t now)
{
	if (!d->handoff && now >= d->handoff_next) {
		char buf[24];
		if (read_fixed(d, "/sys/bus/platform/devices/boot-leds/phase", buf, sizeof(buf)) < 0 ||
		    buf[0] == '3' || now >= d->handoff_until) {
			d->handoff = true;
			for (unsigned i = 0; i < LEDS; ++i) {
				char path[96]; snprintf(path, sizeof(path), "/sys/class/leds/%s/trigger", led_names[i]);
				int fd = open_fixed(d, path, O_WRONLY);
				if (fd >= 0) { (void)write(fd, "none\n", 5); close(fd); }
			}
			for (unsigned t = 0; t < TARGETS; ++t) render(d, t, now);
		} else d->handoff_next = now+50; /* bounded, one-time kernel handoff only */
	}
	for (unsigned t = 0; t < TARGETS; ++t) {
		struct animation *a = &d->anim[t];
		if (a->identify_until && now >= a->identify_until) { a->identify_until = 0; render(d, t, now); }
		if (now < a->next) continue;
		a->on = !a->on;
		if (a->remaining && --a->remaining == 0) {
			a->on = d->lan_mask != 0; a->next = NEVER;
			if (d->lease_ack && d->state[LAN] == R_START) d->ack_at = now+1000;
			/* A completed one-shot becomes steady state. A config reload must
			 * not replay an old cable insertion or fabricate another ACK. */
			d->state[LAN] = R_READY; a->effect = EFFECT_SOLID;
			publish(d);
		} else a->next = now + (a->effect == EFFECT_FAST ? 100 :
			a->on ? d->config.target[t].on_ms : d->config.target[t].off_ms);
		render(d, t, now);
	}
	if (now >= d->ack_at) { d->ack_at = NEVER; start_rule(d, LAN, R_ACK, now); }
	for (unsigned r = 0; r < 2; ++r) {
		if (now >= d->radio[r].deadline) radio_close(d, r, now);
		if (now >= d->radio[r].retry) radio_attach(d, r, now);
	}
	if (d->pending && now >= d->probe_deadline) probe_failed(d, now);
	if (now >= d->probe_next) probe_send(d, now);
	for (unsigned i = 0; i < CLIENTS; ++i)
		if (d->client[i].fd >= 0 && !d->client[i].subscribed && now >= d->client[i].expires) close_client(d, i);
}
static void timer_arm(struct daemon *d, uint64_t now)
{
	uint64_t next = NEVER;
#define SOON(x) do { if ((x) < next) next = (x); } while (0)
	if (!d->handoff) SOON(d->handoff_next);
	for (unsigned t = 0; t < TARGETS; ++t) {
		SOON(d->anim[t].next);
		if (d->anim[t].identify_until) SOON(d->anim[t].identify_until);
	}
	SOON(d->ack_at); SOON(d->probe_next);
	if (d->pending) SOON(d->probe_deadline);
	for (unsigned r = 0; r < 2; ++r) { SOON(d->radio[r].retry); SOON(d->radio[r].deadline); }
	for (unsigned i = 0; i < CLIENTS; ++i)
		if (d->client[i].fd >= 0 && !d->client[i].subscribed) SOON(d->client[i].expires);
	struct itimerspec ts = {0};
	if (next != NEVER) {
		if (next <= now) next = now+1;
		ts.it_value.tv_sec = (time_t)(next/1000); ts.it_value.tv_nsec = (long)(next%1000)*1000000;
	}
	if (timerfd_settime(d->timer, TFD_TIMER_ABSTIME, &ts, NULL)) { perror("gpio-daemon: timer"); exit(1); }
#undef SOON
}
static bool rate_ok(struct daemon *d, uid_t uid, bool identify, uint64_t now)
{
	struct rate *slot = NULL;
	for (unsigned i = 0; i < UID_SLOTS; ++i) {
		if (d->rate[i].used && d->rate[i].uid == uid) { slot = &d->rate[i]; break; }
		if (!d->rate[i].used || now-d->rate[i].window >= 10000) slot = &d->rate[i];
	}
	if (!slot) return false;
	if (!slot->used || slot->uid != uid || now-slot->window >= 10000)
		*slot = (struct rate){ .uid = uid, .used = true, .window = now };
	if (++slot->requests > (uid ? 20u : 200u)) return false;
	return !identify || ++slot->identifies <= 2;
}
static bool valid_event(unsigned target, unsigned event)
{
	if (event >= EV_STARTING && event <= EV_FAILED)
		return target == POWER || target == WIFI_2G || target == WIFI_5G || target == VPN;
	if (event == EV_WAN_BOUND || event == EV_WAN_LOST) return target == INTERNET;
	if (event == EV_LEASES) return target == LAN;
	return event == EV_RESYNC && target == POWER;
}
static void trusted_event(struct daemon *d, unsigned t, unsigned event, uint64_t now)
{
	if (event == EV_LEASES) { leases_read(d, false, now); return; }
	if (event == EV_RESYNC) { (void)select_wan(d); update_internet(d, now); route_dump(d, RTM_GETLINK); watch_refresh(d); return; }
	if (event == EV_WAN_BOUND) {
		d->bound = true;
		if (d->wan_carrier && !d->pending) d->probe_next = now;
		update_internet(d, now); return;
	}
	if (event == EV_WAN_LOST) { wan_clear(d); update_internet(d, now); return; }
	unsigned rule = event == EV_STARTING ? R_START : event == EV_READY ? R_READY : event == EV_STOPPED ? R_STOP : R_FAIL;
	if (t == WIFI_2G || t == WIFI_5G) {
		unsigned r = WIFI_5G-t;
		/* A service starting is not proof of AP readiness: only hostapd confirms ready. */
		if (rule == R_READY) { d->radio[r].retry = now; return; }
		if (rule == R_STOP || rule == R_FAIL) {
			del_fd(d, &d->radio[r].fd); d->radio[r].retry = d->radio[r].deadline = NEVER;
		} else d->radio[r].retry = now;
	}
	if (d->state[t] != rule) start_rule(d, t, rule, now);
}
static int handle_request(struct daemon *d, struct client *c, const struct gpio_msg *m, uint64_t now)
{
	if (!rate_ok(d, c->uid, m->op == GPIO_IDENTIFY, now)) return EAGAIN;
	if (m->magic != GPIO_MAGIC || m->version != GPIO_VERSION || m->flags || m->status) return EINVAL;
	for (unsigned i = 0; i < 10; ++i) if (m->data[i]) return EINVAL;
	if (m->op < GPIO_STATUS || m->op > GPIO_EVENT) return EINVAL;
	if (m->op != GPIO_IDENTIFY && m->op != GPIO_EVENT && (m->target || m->arg)) return EINVAL;
	if ((m->op == GPIO_IDENTIFY || m->op == GPIO_EVENT) && m->target >= TARGETS) return EINVAL;
	switch (m->op) {
	case GPIO_STATUS: return 0;
	case GPIO_SUBSCRIBE: {
		unsigned subscribers = 0;
		for (unsigned i = 0; i < CLIENTS; ++i) subscribers += d->client[i].fd >= 0 && d->client[i].subscribed;
		if (subscribers >= 8 && !c->subscribed) return EBUSY;
		c->subscribed = true; return 0;
	}
	case GPIO_IDENTIFY:
		if (m->arg < 100 || m->arg > 3000) return EINVAL;
		if (d->anim[m->target].identify_until > now) return EBUSY;
		d->anim[m->target].identify_until = now+m->arg;
		render(d, m->target, now); return 0;
	case GPIO_RELOAD:
		if (c->uid) return EPERM;
		if (load_config(d)) return EINVAL;
		for (unsigned t = 0; t < TARGETS; ++t) {
			if (d->config.restore) start_rule(d, t, d->state[t], now);
			else render(d, t, now);
		}
		return 0;
	case GPIO_EVENT:
		if (c->uid) return EPERM;
		if (!valid_event(m->target, m->arg)) return EINVAL;
		trusted_event(d, m->target, m->arg, now); return 0;
	default: return EINVAL;
	}
}
static void client_read(struct daemon *d, unsigned i, uint64_t now)
{
	struct client *c = &d->client[i];
	for (unsigned budget = 0; budget < 4; ++budget) {
		struct gpio_msg request;
		struct iovec io = { .iov_base = &request, .iov_len = sizeof(request) };
		struct msghdr hdr = { .msg_iov = &io, .msg_iovlen = 1 };
		ssize_t n = recvmsg(c->fd, &hdr, MSG_DONTWAIT | MSG_CMSG_CLOEXEC);
		if (n < 0 && (errno == EAGAIN || errno == EINTR)) return;
		if (n <= 0) { close_client(d, i); return; }
		struct gpio_msg reply = snapshot(d, GPIO_STATUS);
		/* No SCM_RIGHTS or credentials are accepted from a client. With no
		 * control buffer Linux closes received rights and reports MSG_CTRUNC. */
		if (n != sizeof(request) || (hdr.msg_flags & (MSG_TRUNC | MSG_CTRUNC)))
			reply.status = rate_ok(d, c->uid, false, now) ? -EMSGSIZE : -EAGAIN;
		else reply.status = -handle_request(d, c, &request, now);
		struct gpio_msg state = snapshot(d, GPIO_STATUS);
		memcpy(reply.data, state.data, sizeof(reply.data));
		if (send(c->fd, &reply, sizeof(reply), MSG_DONTWAIT | MSG_NOSIGNAL) != sizeof(reply)) { close_client(d, i); return; }
		if (reply.status) { close_client(d, i); return; }
		c->expires = now+3000;
		if (request.op != GPIO_STATUS && request.op != GPIO_SUBSCRIBE) publish(d);
	}
}
static void clients_accept(struct daemon *d, uint64_t now)
{
	for (unsigned budget = 0; budget < 8; ++budget) {
		int fd = accept4(d->listener, NULL, NULL, SOCK_NONBLOCK | SOCK_CLOEXEC);
		if (fd < 0) return;
		struct ucred cred;
		socklen_t len = sizeof(cred);
		if (getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &cred, &len) || len != sizeof(cred)) { close(fd); continue; }
		unsigned same = 0, free_slot = CLIENTS;
		for (unsigned i = 0; i < CLIENTS; ++i) {
			if (d->client[i].fd < 0) free_slot = i;
			else if (d->client[i].uid == cred.uid) ++same;
		}
		if (same >= 4 || free_slot == CLIENTS) { close(fd); continue; }
		int size = 2048;
		setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));
		setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
		d->client[free_slot] = (struct client){ .fd = fd, .uid = cred.uid, .expires = now+3000 };
		add_fd(d, fd, CLIENT_FD+free_slot);
	}
}

/* Allow only the single-threaded event loop's syscall vocabulary. No exec,
 * fork, ioctl, ptrace, mount, register access, module loading, or reboot. */
static int sandbox(bool test)
{
#if defined(__mips__) && __SIZEOF_POINTER__ == 4 && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define GPIO_ARCH AUDIT_ARCH_MIPS
#elif defined(__x86_64__)
#define GPIO_ARCH AUDIT_ARCH_X86_64
#elif defined(__aarch64__)
#define GPIO_ARCH AUDIT_ARCH_AARCH64
#else
#error Add the exact seccomp audit architecture before porting
#endif
#define ALLOW(n) BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, SYS_##n, 0, 1), BPF_STMT(BPF_RET|BPF_K, SECCOMP_RET_ALLOW)
	struct sock_filter code[] = {
		BPF_STMT(BPF_LD|BPF_W|BPF_ABS, offsetof(struct seccomp_data, arch)),
		BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, GPIO_ARCH, 1, 0),
		BPF_STMT(BPF_RET|BPF_K, SECCOMP_RET_KILL_PROCESS),
		BPF_STMT(BPF_LD|BPF_W|BPF_ABS, offsetof(struct seccomp_data, nr)),
		ALLOW(read), ALLOW(write), ALLOW(close), ALLOW(openat), ALLOW(lseek),
		ALLOW(fcntl), ALLOW(socket), ALLOW(bind), ALLOW(connect), ALLOW(accept4),
		ALLOW(sendto), ALLOW(recvfrom), ALLOW(sendmsg), ALLOW(recvmsg),
		ALLOW(getsockopt), ALLOW(setsockopt), ALLOW(epoll_wait), ALLOW(epoll_ctl),
		ALLOW(timerfd_settime), ALLOW(clock_gettime), ALLOW(inotify_add_watch),
		ALLOW(unlink), ALLOW(chmod), ALLOW(getuid), ALLOW(geteuid), ALLOW(getpid),
		ALLOW(rt_sigreturn), ALLOW(rt_sigprocmask), ALLOW(exit), ALLOW(exit_group),
		ALLOW(brk), ALLOW(munmap), ALLOW(mprotect), ALLOW(futex), ALLOW(restart_syscall),
#ifdef SYS_fstat
		ALLOW(fstat),
#endif
#ifdef SYS_open
		ALLOW(open),
#endif
#ifdef SYS_send
		ALLOW(send),
#endif
#ifdef SYS_recv
		ALLOW(recv),
#endif
#ifdef SYS_unlinkat
		ALLOW(unlinkat),
#endif
#ifdef SYS_fchmodat
		ALLOW(fchmodat),
#endif
#ifdef SYS_fstat64
		ALLOW(fstat64),
#endif
#ifdef SYS_fcntl64
		ALLOW(fcntl64),
#endif
#ifdef SYS_newfstatat
		ALLOW(newfstatat),
#endif
#ifdef SYS_fstatat64
		ALLOW(fstatat64),
#endif
#ifdef SYS_statx
		ALLOW(statx),
#endif
#ifdef SYS_epoll_pwait
		ALLOW(epoll_pwait),
#endif
#ifdef SYS_epoll_pwait2
		ALLOW(epoll_pwait2),
#endif
#ifdef SYS_mmap
		ALLOW(mmap),
#endif
#ifdef SYS_mmap2
		ALLOW(mmap2),
#endif
#ifdef SYS_clock_gettime64
		ALLOW(clock_gettime64),
#endif
#ifdef SYS__llseek
		ALLOW(_llseek),
#endif
#ifdef SYS_timerfd_settime64
		ALLOW(timerfd_settime64),
#endif
		BPF_STMT(BPF_RET|BPF_K, SECCOMP_RET_ERRNO | EPERM),
	};
	struct sock_fprog program = { .len = sizeof(code)/sizeof(code[0]), .filter = code };
	if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)) return -1;
	if (!test && geteuid() == 0) {
		struct __user_cap_header_struct h = { .version = _LINUX_CAPABILITY_VERSION_3 };
		struct __user_cap_data_struct cap[2] = {{0}, {0}};
		/* Retain uid 0 only for owner-only hostapd/config paths. No Linux
		 * capabilities remain; the sole raw socket was opened beforehand. */
		if (syscall(SYS_capset, &h, cap)) return -1;
	}
	return prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &program);
#undef ALLOW
}
static int initialize(struct daemon *d)
{
	d->ep = epoll_create1(EPOLL_CLOEXEC);
	if (d->ep < 0) return -1;
	char path[PATH_MAX]; path_of(d, path, sizeof(path), "/run/phoebus");
	if (!owned_directory(path, 0755, d->test)) return -1;
	d->lock = open_fixed(d, "/run/phoebus/gpio.lock", O_RDWR | O_CREAT);
	if (d->lock < 0 || flock(d->lock, LOCK_EX | LOCK_NB)) return -1;
	if (load_config(d)) { fprintf(stderr, "gpio-daemon: invalid/missing protected configuration\n"); return -1; }
	struct sockaddr_un addr = { .sun_family = AF_UNIX };
	path_of(d, addr.sun_path, sizeof(addr.sun_path), GPIO_SOCKET);
	d->listener = socket(AF_UNIX, SOCK_SEQPACKET | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
	unlink(addr.sun_path);
	if (d->listener < 0 || bind(d->listener, (struct sockaddr *)&addr, sizeof(addr)) || chmod(addr.sun_path, 0666) || listen(d->listener, 16)) return -1;
	add_fd(d, d->listener, LISTEN_FD);
	d->timer = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
	if (d->timer < 0) return -1;
	add_fd(d, d->timer, TIMER_FD);
	sigset_t mask; sigemptyset(&mask); sigaddset(&mask, SIGTERM); sigaddset(&mask, SIGINT); sigaddset(&mask, SIGHUP);
	if (sigprocmask(SIG_BLOCK, &mask, NULL)) return -1;
	d->signal = signalfd(-1, &mask, SFD_CLOEXEC | SFD_NONBLOCK);
	if (d->signal < 0) return -1;
	add_fd(d, d->signal, SIGNAL_FD);
	d->watch = inotify_init1(IN_CLOEXEC | IN_NONBLOCK);
	if (d->watch < 0) return -1;
	add_fd(d, d->watch, WATCH_FD);
	path_of(d, path, sizeof(path), "/run");
	if (inotify_add_watch(d->watch, path, IN_CREATE | IN_MOVED_TO) < 0) return -1;
	watch_refresh(d);
	strcpy(d->wan, "eth0.8");
	(void)select_wan(d);
	if (!d->test) {
		d->route = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC | SOCK_NONBLOCK, NETLINK_ROUTE);
		struct sockaddr_nl nl = { .nl_family = AF_NETLINK, .nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR };
		if (d->route < 0 || bind(d->route, (struct sockaddr *)&nl, sizeof(nl))) return -1;
		add_fd(d, d->route, ROUTE_FD); route_dump(d, RTM_GETLINK);
		d->ping = socket(AF_INET, SOCK_RAW | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_ICMP);
		if (d->ping >= 0) {
			struct icmp_filter filter = { .data = ~(1u << ICMP_ECHOREPLY) };
			(void)setsockopt(d->ping, SOL_RAW, ICMP_FILTER, &filter, sizeof(filter));
			add_fd(d, d->ping, PING_FD);
		}
		else fprintf(stderr, "gpio-daemon: connectivity probe unavailable; will not claim WAN online\n");
	}
	if (getrandom(&d->nonce, sizeof(d->nonce), 0) != sizeof(d->nonce)) return -1;
	d->ping_id = (uint16_t)d->nonce;
	for (unsigned i = 0; i < LEDS; ++i) {
		char fixed[96]; snprintf(fixed, sizeof(fixed), "/sys/class/leds/%s/brightness", led_names[i]);
		d->led[i] = open_fixed(d, fixed, O_WRONLY);
		if (d->led[i] < 0) d->failure_mask |= 1u << i;
		d->last[i] = -1;
	}
	uint64_t now = now_ms();
	d->handoff_until = now+2000; d->handoff_next = now;
	int ready = open_fixed(d, "/sys/bus/platform/devices/boot-leds/userspace_ready", O_WRONLY);
	if (ready >= 0) { (void)write(ready, "1\n", 2); close(ready); }
	for (unsigned t = 0; t < TARGETS; ++t)
		start_rule(d, t, t == POWER ? R_READY : (t == WIFI_2G || t == WIFI_5G) ? R_START : t == LAN ? R_FAIL : R_STOP, now);
	leases_read(d, true, now);
	char vpn_state[3];
	if (read_fixed(d, "/run/phoebus/vpn.state", vpn_state, sizeof(vpn_state)) == 1 &&
	    vpn_state[0] >= EV_STARTING && vpn_state[0] <= EV_FAILED)
		trusted_event(d, VPN, (unsigned)vpn_state[0], now);
	for (unsigned r = 0; r < 2; ++r) radio_attach(d, r, now);
	struct rlimit core = {0, 0}; setrlimit(RLIMIT_CORE, &core);
	return sandbox(d->test);
}
int main(int argc, char **argv)
{
	static struct daemon d;
	d.ep = d.listener = d.timer = d.signal = d.route = d.watch = d.ping = d.lock = -1;
	d.running = true; d.probe_next = d.ack_at = NEVER;
	for (unsigned i = 0; i < CLIENTS; ++i) d.client[i].fd = -1;
	for (unsigned i = 0; i < 2; ++i) { d.radio[i].fd = -1; d.radio[i].backoff = 250; d.radio[i].retry = d.radio[i].deadline = NEVER; }
#ifdef GPIO_TEST
	if (argc == 3 && !strcmp(argv[1], "--test-root") && strlen(argv[2]) < sizeof(d.prefix)) {
		strcpy(d.prefix, argv[2]); d.test = true;
	} else
#endif
	if (argc != 1) { fprintf(stderr, "usage: gpio-daemon\n"); return 2; }
#ifndef GPIO_TEST
	(void)argv;
#endif
	if (initialize(&d)) { perror("gpio-daemon: initialization"); return 1; }
	fprintf(stderr, "gpio-daemon: event loop ready (no network service depends on it)\n");
	while (d.running) {
		uint64_t now = now_ms(); tick(&d, now); timer_arm(&d, now);
		struct epoll_event events[16];
		int n = epoll_wait(d.ep, events, 16, -1);
		if (n < 0 && errno == EINTR) continue;
		if (n < 0) break;
		for (int i = 0; i < n; ++i) {
			unsigned source = events[i].data.u32; now = now_ms();
			if (source == LISTEN_FD) clients_accept(&d, now);
			else if (source == TIMER_FD) { uint64_t count; (void)read(d.timer, &count, sizeof(count)); }
			else if (source == ROUTE_FD) route_read(&d, now);
			else if (source == WATCH_FD) watch_read(&d, now);
			else if (source == PING_FD) probe_read(&d, now);
			else if (source == HOST0_FD || source == HOST1_FD) {
				unsigned r = source-HOST0_FD;
				if (d.radio[r].fd >= 0) radio_read(&d, r, now);
			} else if (source == SIGNAL_FD) {
				struct signalfd_siginfo si;
				while (read(d.signal, &si, sizeof(si)) == sizeof(si)) {
					if (si.ssi_signo == SIGHUP) {
						struct client root = { .uid = 0 };
						struct gpio_msg m = gpio_message(GPIO_RELOAD);
						if (handle_request(&d, &root, &m, now)) fprintf(stderr, "gpio-daemon: reload rejected; previous configuration retained\n");
					} else d.running = false;
				}
			} else if (source >= CLIENT_FD && source < CLIENT_FD+CLIENTS) {
				unsigned slot = source-CLIENT_FD;
				if (d.client[slot].fd >= 0) client_read(&d, slot, now);
			}
		}
	}
	start_rule(&d, POWER, R_STOP, now_ms());
	char path[PATH_MAX]; path_of(&d, path, sizeof(path), GPIO_SOCKET); unlink(path);
	for (unsigned r = 0; r < 2; ++r) {
		char fixed[64]; snprintf(fixed, sizeof(fixed), "/run/phoebus/hostapd-%u", r);
		path_of(&d, path, sizeof(path), fixed); unlink(path);
	}
	return 0;
}
