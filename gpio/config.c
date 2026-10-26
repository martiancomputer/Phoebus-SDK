/* SPDX-License-Identifier: GPL-2.0-only */
#include "config.h"
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void config_defaults(struct gpio_config *c)
{
	memset(c, 0, sizeof(*c));
	c->brightness = 255;
	c->restore = true;
	for (unsigned t = 0; t < TARGETS; ++t) {
		c->target[t].on_ms = c->target[t].off_ms = 250;
		c->target[t].action[R_START] = EFFECT_BLINK;
		c->target[t].action[R_READY] = EFFECT_SOLID;
		c->target[t].action[R_FAIL] = EFFECT_FAST;
	}
	c->target[LAN].on_ms = c->target[LAN].off_ms = 125;
	c->target[LAN].action[R_START] = EFFECT_PULSE;
	c->target[LAN].action[R_STOP] = EFFECT_CONNECTED;
	c->target[LAN].action[R_ACK] = EFFECT_FLASH;
	c->target[LAN].action[R_FAIL] = EFFECT_OFF;
	c->target[INTERNET].on_ms = c->target[INTERNET].off_ms = 400;
	c->target[INTERNET].action[R_START] = EFFECT_BLINK_ORANGE;
	c->target[INTERNET].action[R_READY] = EFFECT_ORANGE;
	c->target[INTERNET].action[R_ONLINE] = EFFECT_GREEN;
	c->target[INTERNET].action[R_LOST] = EFFECT_BLINK_ORANGE;
}

struct directive { const char *section, *key; int target, rule; unsigned allowed; };
#define E(e) (1u << EFFECT_##e)
#define SIMPLE (E(OFF) | E(SOLID) | E(BLINK) | E(FAST))
static const struct directive directives[] = {
	{"power", "on_boot", POWER, R_START, SIMPLE},
	{"power", "on_system_ready", POWER, R_READY, SIMPLE},
	{"power", "on_shutdown", POWER, R_STOP, SIMPLE},
	{"lan", "on_all_disconnected", LAN, R_FAIL, E(OFF)},
	{"lan", "on_link_up", LAN, R_START, SIMPLE | E(PULSE)},
	{"lan", "on_link_down", LAN, R_STOP, SIMPLE | E(CONNECTED)},
	{"lan", "on_dhcp_ack", LAN, R_ACK, SIMPLE | E(FLASH)},
	{"internet", "on_no_carrier", INTERNET, R_STOP, E(OFF)},
	{"internet", "on_carrier_detected", INTERNET, R_START, SIMPLE | E(BLINK_ORANGE) | E(ORANGE)},
	{"internet", "on_dhcp_bound", INTERNET, R_READY, SIMPLE | E(BLINK_ORANGE) | E(ORANGE)},
	{"internet", "on_internet_online", INTERNET, R_ONLINE, SIMPLE | E(GREEN)},
	{"internet", "on_internet_lost", INTERNET, R_LOST, SIMPLE | E(BLINK_ORANGE) | E(ORANGE)},
	{"wifi_2g", "on_starting", WIFI_2G, R_START, SIMPLE},
	{"wifi_2g", "on_ready", WIFI_2G, R_READY, SIMPLE},
	{"wifi_2g", "on_stopped", WIFI_2G, R_STOP, SIMPLE},
	{"wifi_2g", "on_failed", WIFI_2G, R_FAIL, SIMPLE},
	{"wifi_5g", "on_starting", WIFI_5G, R_START, SIMPLE},
	{"wifi_5g", "on_ready", WIFI_5G, R_READY, SIMPLE},
	{"wifi_5g", "on_stopped", WIFI_5G, R_STOP, SIMPLE},
	{"wifi_5g", "on_failed", WIFI_5G, R_FAIL, SIMPLE},
	{"vpn", "on_connected", VPN, R_READY, SIMPLE},
	{"vpn", "on_disconnected", VPN, R_STOP, SIMPLE},
	{"vpn", "on_connecting", VPN, R_START, SIMPLE},
};
static const char *const sections[] = {"power", "lan", "internet", "wifi_2g", "wifi_5g", "vpn"};
static char *trim(char *p)
{
	while (isspace((unsigned char)*p)) ++p;
	char *end = p + strlen(p);
	while (end > p && isspace((unsigned char)end[-1])) --end;
	*end = 0;
	return p;
}
static int number(const char *s, unsigned lo, unsigned hi, unsigned *v)
{
	if (!*s) return -1;
	unsigned n = 0;
	for (; *s; ++s) {
		if (*s < '0' || *s > '9' || n > hi / 10) return -1;
		n = n * 10 + (unsigned)(*s - '0');
		if (n > hi) return -1;
	}
	if (n < lo) return -1;
	*v = n;
	return 0;
}
static int effect_value(const char *v)
{
	static const char *const names[] = {"off", "solid", "blink", "blink_fast", "pulse",
		"solid_if_connected", "flash", "solid_orange", "blink_orange", "solid_green"};
	for (unsigned i = 0; i < sizeof(names)/sizeof(names[0]); ++i)
		if (!strcmp(v, names[i])) return (int)i;
	return -1;
}
static int apply(struct gpio_config *c, int section, const char *key, const char *v)
{
	if (section == -1) {
		if (!strcmp(key, "brightness")) return number(v, 0, 255, &c->brightness);
		if (!strcmp(key, "restore_on_reload")) {
			if (strcmp(v, "true") && strcmp(v, "false")) return -1;
			c->restore = !strcmp(v, "true");
			return 0;
		}
		return -1;
	}
	struct pattern_config *p = &c->target[section];
	if (!strcmp(key, "blink_on_ms") || (section == LAN && !strcmp(key, "pulse_on_ms")))
		return number(v, 25, 10000, &p->on_ms);
	if (!strcmp(key, "blink_off_ms") || (section == LAN && !strcmp(key, "pulse_off_ms")))
		return number(v, 25, 10000, &p->off_ms);
	if (section == LAN && !strcmp(key, "pulse_count")) {
		if (!strcmp(v, "port_number")) { c->pulse_count = 0; return 0; }
		return number(v, 1, 4, &c->pulse_count);
	}
	for (unsigned i = 0; i < sizeof(directives)/sizeof(directives[0]); ++i) {
		const struct directive *d = &directives[i];
		if (section != d->target || strcmp(key, d->key)) continue;
		int e = effect_value(v);
		if (e < 0 || !(d->allowed & (1u << e))) return -1;
		p->action[d->rule] = (enum effect)e;
		return 0;
	}
	return -1;
}
int config_read(int fd, struct gpio_config *out)
{
	char buf[8193], seen[64][80];
	size_t n = 0;
	for (;;) {
		ssize_t r = read(fd, buf+n, sizeof(buf)-n);
		if (r < 0 && errno == EINTR) continue;
		if (r < 0 || n + (size_t)r == sizeof(buf)) return -1;
		if (!r) break;
		n += (size_t)r;
	}
	if (memchr(buf, 0, n)) return -1;
	buf[n] = 0;
	struct gpio_config c;
	config_defaults(&c);
	int section = -2;
	unsigned count = 0;
	char *save = NULL;
	for (char *line = strtok_r(buf, "\n", &save); line; line = strtok_r(NULL, "\n", &save)) {
		if (strlen(line) > 255) return -1;
		char *p = trim(line);
		if (!*p || *p == '#' || *p == ';') continue;
		if (*p == '[') {
			size_t len = strlen(p);
			if (len < 3 || p[len-1] != ']') return -1;
			p[len-1] = 0;
			section = -2;
			if (!strcmp(p+1, "general")) section = -1;
			for (unsigned i = 0; i < TARGETS; ++i)
				if (!strcmp(p+1, sections[i])) section = (int)i;
			if (section == -2) return -1;
			continue;
		}
		char *eq = strchr(p, '=');
		if (!eq || section == -2 || count == 64) return -1;
		*eq = 0;
		char *key = trim(p), *value = trim(eq+1);
		if (strlen(key) > 60) return -1;
		char id[80];
		/* section is a small bounded enum; no untrusted paths are constructed. */
		id[0] = (char)('A'+section+1); id[1] = ':';
		strcpy(id+2, key);
		for (unsigned i = 0; i < count; ++i) if (!strcmp(seen[i], id)) return -1;
		strcpy(seen[count++], id);
		if (apply(&c, section, key, value)) return -1;
	}
	if (!count) return -1;
	*out = c;
	return 0;
}
