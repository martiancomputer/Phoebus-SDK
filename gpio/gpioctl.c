/* SPDX-License-Identifier: GPL-2.0-only */
#include "protocol.h"
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

static const char *const targets[] = {"power", "lan", "internet", "wifi_2g", "wifi_5g", "vpn"};
static void print_state(const struct gpio_msg *m)
{
	printf("LAN mask=0x%x selected=%u WAN carrier=%u lease=%u online=%u LED-failures=0x%x\n",
		m->data[0], m->data[1], m->data[2]&1, (m->data[2]>>1)&1, (m->data[2]>>2)&1, m->data[9]);
	static const char *const states[] = {"starting", "ready", "stopped", "failed", "lease-ack", "online", "lost"};
	for (unsigned i = 0; i < TARGETS; ++i)
		printf("%s=%s%s", targets[i], m->data[3+i] < 7 ? states[m->data[3+i]] : "invalid", i+1 == TARGETS ? "\n" : " ");
}
static unsigned target(const char *s)
{
	for (unsigned i = 0; i < TARGETS; ++i) if (!strcmp(targets[i], s)) return i;
	return TARGETS;
}
static void remember_vpn(const struct gpio_msg *m)
{
	/* A root service can report while the daemon is stopped. Preserve that
	 * notification in tmpfs so restart synchronization doesn't use stale state.
	 * An ordinary caller never gets a filesystem write path here. */
	if (geteuid() || m->op != GPIO_EVENT || m->target != VPN ||
	    m->arg < EV_STARTING || m->arg > EV_FAILED) return;
	char path[108] = "/run/phoebus";
#ifdef GPIO_TEST
	const char *prefix = getenv("GPIO_TEST_ROOT");
	if (prefix && snprintf(path, sizeof(path), "%s/run/phoebus", prefix) >= (int)sizeof(path)) return;
#endif
	if (mkdir(path, 0755) && errno != EEXIST) return;
	int dir = open(path, O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
	struct stat st;
	if (dir < 0) return;
	if (fstat(dir, &st) || st.st_uid || (st.st_mode & 0022)) { close(dir); return; }
	int fd = openat(dir, "vpn.state", O_WRONLY | O_CREAT | O_NONBLOCK | O_CLOEXEC | O_NOFOLLOW, 0600);
	if (fd >= 0) {
		if (!fstat(fd, &st) && S_ISREG(st.st_mode) && !st.st_uid && st.st_nlink == 1 && !(st.st_mode & 0022)) {
			unsigned char value = (unsigned char)m->arg;
			if (write(fd, &value, 1) == 1) (void)ftruncate(fd, 1);
		}
		close(fd);
	}
	close(dir);
}
int main(int argc, char **argv)
{
	struct gpio_msg m = gpio_message(GPIO_STATUS);
	const char *base = strrchr(argv[0], '/'); base = base ? base+1 : argv[0];
	/* udhcpd supplies its filename to notify_file. It is deliberately ignored:
	 * only the protected daemon's fixed lease file can ever be opened. */
	bool notify = !strcmp(base, "gpio-lease-notify");
	if (notify) { m.op = GPIO_EVENT; m.target = LAN; m.arg = EV_LEASES; }
	else if (argc == 2 && !strcmp(argv[1], "status")) m.op = GPIO_STATUS;
	else if (argc == 2 && !strcmp(argv[1], "events")) m.op = GPIO_SUBSCRIBE;
	else if (argc == 2 && !strcmp(argv[1], "reload")) m.op = GPIO_RELOAD;
	else if (argc == 4 && !strcmp(argv[1], "identify")) {
		m.op = GPIO_IDENTIFY; m.target = target(argv[2]);
		char *end; errno = 0;
		unsigned long n = strtoul(argv[3], &end, 10);
		if (errno || *end || end == argv[3] || n < 100 || n > 3000 || m.target == TARGETS) return 2;
		m.arg = (unsigned)n;
	} else if (argc >= 3 && !strcmp(argv[1], "event")) {
		m.op = GPIO_EVENT;
		if (argc == 3 && !strcmp(argv[2], "leases")) { m.target = LAN; m.arg = EV_LEASES; }
		else if (argc == 3 && !strcmp(argv[2], "resync")) { m.target = POWER; m.arg = EV_RESYNC; }
		else if (argc == 4) {
			m.target = target(argv[2]);
			static const char *const names[] = {"starting", "ready", "stopped", "failed", "bound", "lost"};
			for (unsigned i = 0; i < 6; ++i) if (!strcmp(names[i], argv[3])) m.arg = i+1;
			if (m.target == TARGETS || !m.arg) return 2;
		} else return 2;
	} else {
		fprintf(stderr, "usage: gpioctl status|events|reload|identify <LED> <100..3000 ms>\n"
			"       gpioctl event <target> starting|ready|stopped|failed|bound|lost\n"
			"       gpioctl event leases|resync\n");
		return 2;
	}
	remember_vpn(&m);
	struct sockaddr_un addr = { .sun_family = AF_UNIX };
#ifdef GPIO_TEST
	const char *prefix = getenv("GPIO_TEST_ROOT");
	if (prefix) {
		if (snprintf(addr.sun_path, sizeof(addr.sun_path), "%s%s", prefix, GPIO_SOCKET) >= (int)sizeof(addr.sun_path)) return 2;
	} else
#endif
	strcpy(addr.sun_path, GPIO_SOCKET);
	int fd = socket(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
	if (fd < 0 || connect(fd, (struct sockaddr *)&addr, sizeof(addr)) ||
	    send(fd, &m, sizeof(m), MSG_DONTWAIT | MSG_NOSIGNAL) != sizeof(m)) {
		/* Hooks are advisory: an absent or overloaded LED daemon NEVER fails DHCP. */
		if (!notify && m.op != GPIO_EVENT) perror("gpioctl: unavailable");
		return (notify || m.op == GPIO_EVENT) ? 0 : 1;
	}
	struct pollfd p = { .fd = fd, .events = POLLIN };
	bool first = true;
	do {
		int ready = poll(&p, 1, first ? 200 : -1);
		if (ready < 0 && errno == EINTR) continue;
		if (ready <= 0) return (notify || m.op == GPIO_EVENT) ? 0 : 1;
		struct gpio_msg reply;
		ssize_t n = recv(fd, &reply, sizeof(reply), MSG_TRUNC);
		if (n != sizeof(reply) || reply.magic != GPIO_MAGIC || reply.version != GPIO_VERSION ||
		    (reply.op != GPIO_STATUS && reply.op != GPIO_UPDATE)) return 1;
		if (reply.status > 0 || reply.status < -4095) return 1;
		if (reply.status) {
			if (!notify && m.op != GPIO_EVENT) fprintf(stderr, "gpioctl: %s\n", strerror(-reply.status));
			return (notify || m.op == GPIO_EVENT) ? 0 : 1;
		}
		if (m.op == GPIO_STATUS || m.op == GPIO_SUBSCRIBE) { print_state(&reply); fflush(stdout); }
		first = false;
	} while (m.op == GPIO_SUBSCRIBE);
	close(fd);
	return 0;
}
