/* SPDX-License-Identifier: GPL-2.0-only */
#define GPIO_TEST 1
#define main gpio_daemon_main
#include "../daemon.c"
#undef main
#include <assert.h>
#include <sys/wait.h>

static struct daemon *fresh(void)
{
	static struct daemon d;
	memset(&d, 0, sizeof(d));
	d.test = true; d.route = d.ping = d.ep = -1;
	d.handoff = true; d.probe_next = d.ack_at = NEVER;
	for (unsigned i = 0; i < LEDS; ++i) d.led[i] = -1;
	for (unsigned i = 0; i < CLIENTS; ++i) d.client[i].fd = -1;
	for (unsigned r = 0; r < 2; ++r) {
		d.radio[r].fd = -1; d.radio[r].deadline = d.radio[r].retry = NEVER;
	}
	config_defaults(&d.config);
	for (unsigned t = 0; t < TARGETS; ++t) d.anim[t].next = NEVER;
	return &d;
}
static void config_test(const char *text, bool valid)
{
	FILE *fp = tmpfile(); assert(fp);
	assert(fputs(text, fp) >= 0); rewind(fp);
	struct gpio_config c;
	memset(&c, 0x55, sizeof(c));
	struct gpio_config before = c;
	assert((config_read(fileno(fp), &c) == 0) == valid);
	if (!valid) assert(!memcmp(&c, &before, sizeof(c)));
	fclose(fp);
}
static void save_file(const char *path, const void *data, size_t n)
{
	int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600); assert(fd >= 0);
	assert(write(fd, data, n) == (ssize_t)n); close(fd);
}
static void lease_test(void)
{
	char root[] = "/tmp/gpio-unit-XXXXXX";
	assert(mkdtemp(root));
	char path[PATH_MAX];
	const char *const dirs[] = {"/var", "/var/lib", "/var/lib/misc", "/sys", "/sys/class",
		"/sys/class/net", "/sys/class/net/br0", "/sys/class/net/br0/brif", "/sys/class/net/br0/brif/eth0.3"};
	for (unsigned i = 0; i < sizeof(dirs)/sizeof(dirs[0]); ++i) {
		snprintf(path, sizeof(path), "%s%s", root, dirs[i]); assert(!mkdir(path, 0700));
	}
	struct daemon *d = fresh(); strcpy(d->prefix, root);
	d->selected = 2; d->lan_mask = 2;
	struct __fdb_entry fdb = { .mac_addr = {2,0,0,0,0,1}, .port_no = 2 };
	snprintf(path, sizeof(path), "%s/sys/class/net/br0/brforward", root);
	save_file(path, &fdb, sizeof(fdb));
	snprintf(path, sizeof(path), "%s/sys/class/net/br0/brif/eth0.3/port_no", root);
	save_file(path, "0x2\n", 4);
	unsigned char lease[44] = {0};
	lease[7] = 100; lease[11] = 60; memcpy(lease+16, fdb.mac_addr, 6);
	snprintf(path, sizeof(path), "%s/var/lib/misc/udhcpd.leases", root);
	save_file(path, lease, sizeof(lease));
	leases_read(d, true, 1000); assert(d->lease_count == 1 && !d->lease_ack);
	lease[7] = 110; lease[11] = 50; save_file(path, lease, sizeof(lease));
	leases_read(d, false, 1100); assert(!d->lease_ack); /* rewrite, same expiry */
	lease[11] = 60; save_file(path, lease, sizeof(lease));
	leases_read(d, false, 1200); assert(d->lease_ack && d->ack_at == 2200);
	tick(d, 2200); assert(d->state[LAN] == R_ACK && !d->anim[LAN].on);
	tick(d, 2300); assert(d->anim[LAN].on && d->anim[LAN].next == NEVER);
	lease[11] = 0; save_file(path, lease, sizeof(lease));
	leases_read(d, false, 2400); assert(d->lease_count == 0);
	unlink(path);
	snprintf(path, sizeof(path), "%s/sys/class/net/br0/brforward", root); unlink(path);
	snprintf(path, sizeof(path), "%s/sys/class/net/br0/brif/eth0.3/port_no", root); unlink(path);
	for (unsigned i = sizeof(dirs)/sizeof(dirs[0]); i > 0; --i) {
		snprintf(path, sizeof(path), "%s%s", root, dirs[i-1]); assert(!rmdir(path));
	}
	assert(!rmdir(root));
}
int main(void)
{
	config_test("[general]\nbrightness=255\n[lan]\npulse_count=port_number\n", true);
	config_test("[general]\nbrightness=256\n", false);
	config_test("[general]\nbrightness=-1\n", false);
	config_test("[general]\nbrightness=999999999999999999999\n", false);
	config_test("[general]\nrestore_on_reload=yes\n", false);
	config_test("[gpio]\npin=2\n", false);
	config_test("[lan]\ncommand=reboot\n", false);
	config_test("[internet]\non_no_carrier=solid_green\n", false);
	config_test("[lan]\npulse_count=5\n", false);
	config_test("[power]\nblink_on_ms=0\n", false);
	config_test("[power]\nblink_off_ms=10001\n", false);
	config_test("[general]\nbrightness=1\nbrightness=2\n", false);
	config_test("[wifi_2g]\non_ready=solid_green\n", false);
	config_test("", false);
	config_test("# accidentally truncated policy\n", false);
	lease_test();

	struct daemon *d = fresh();
	strcpy(d->wan, "eth0.8");
	link_event(d, "eth0.3", 3, true, 1000);
	assert(d->lan_mask == 2 && d->selected == 2 && d->anim[LAN].remaining == 4);
	link_event(d, "eth0.3", 3, true, 1100);
	assert(d->anim[LAN].next == 1125); /* duplicate event doesn't restart */
	link_event(d, "eth0.5", 5, true, 1200);
	assert(d->lan_mask == 10 && d->selected == 4 && d->anim[LAN].remaining == 8);
	link_event(d, "eth0.3", 3, false, 1300);
	assert(d->lan_mask == 8 && d->selected == 4); /* older port doesn't preempt */
	for (unsigned i = 0; i < 8; ++i) tick(d, 1325+i*125);
	assert(d->anim[LAN].remaining == 0 && d->anim[LAN].on && d->anim[LAN].next == NEVER);
	link_event(d, "eth0.5", 5, false, 2400);
	assert(!d->lan_mask && !d->anim[LAN].on && d->anim[LAN].next == NEVER);
	link_event(d, "eth0.9", 9, true, 2500);
	assert(!d->lan_mask && !d->wan_carrier); /* internal SGMII isn't LAN4 */

	d = fresh();
	d->wan_carrier = d->bound = d->online = true;
	probe_failed(d, 1000); assert(d->online);
	probe_failed(d, 2000); assert(d->online);
	probe_failed(d, 3000); assert(!d->online && d->state[INTERNET] == R_LOST);
	link_event(d, "eth0.8", 8, false, 4000); /* no matching wan yet */
	strcpy(d->wan, "eth0.8"); link_event(d, "eth0.8", 8, false, 4000);
	assert(!d->bound && !d->pending && d->state[INTERNET] == R_STOP);

	d = fresh();
	struct client ordinary = {.uid = 1000}, root = {.uid = 0};
	struct gpio_msg m = gpio_message(GPIO_EVENT); m.target = VPN; m.arg = EV_READY;
	assert(handle_request(d, &ordinary, &m, 1000) == EPERM);
	assert(handle_request(d, &root, &m, 1000) == 0 && d->state[VPN] == R_READY);
	m.arg = UINT_MAX;
	assert(handle_request(d, &root, &m, 1001) == EINVAL);
	m = gpio_message(GPIO_STATUS); m.flags = 1;
	assert(handle_request(d, &ordinary, &m, 1002) == EINVAL);
	m = gpio_message(GPIO_STATUS); m.data[9] = 1;
	assert(handle_request(d, &ordinary, &m, 1002) == EINVAL);
	m = gpio_message(GPIO_IDENTIFY); m.target = LAN; m.arg = 500;
	assert(handle_request(d, &ordinary, &m, 1003) == 0);
	assert(handle_request(d, &ordinary, &m, 1004) == EBUSY);
	tick(d, 1504); assert(!d->anim[LAN].identify_until);
	m.target = TARGETS; assert(handle_request(d, &root, &m, 1005) == EINVAL);
	m = gpio_message(GPIO_RELOAD); assert(handle_request(d, &ordinary, &m, 1006) == EPERM);
	m = gpio_message(GPIO_STATUS);
	unsigned rejected = 0;
	for (unsigned i = 0; i < 30; ++i) rejected += handle_request(d, &ordinary, &m, 2000) == EAGAIN;
	assert(rejected > 0);

	unsigned char random[256];
	for (unsigned i = 0; i < sizeof(random); ++i) random[i] = (unsigned char)(i*73);
	for (unsigned n = 0; n <= sizeof(random); ++n) (void)checksum(random, n);
	assert(be_integer((const unsigned char *)"\x01\x02\x03\x04", 4) == 0x01020304);
#ifndef GPIO_SANITIZER_TEST
	pid_t pid = fork(); assert(pid >= 0);
	if (!pid) {
		assert(sandbox(true) == 0);
		errno = 0;
		assert(syscall(SYS_getppid) == -1 && errno == EPERM);
		_exit(0);
	}
	int status; assert(waitpid(pid, &status, 0) == pid && status == 0);
#endif
	puts("PASS: config, atomic rejection, LAN precedence/pulses, WAN hysteresis, auth, limits, sandbox");
	return 0;
}
