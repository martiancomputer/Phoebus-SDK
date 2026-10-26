/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef GPIO_CONFIG_H
#define GPIO_CONFIG_H
#include <stdbool.h>
#include "protocol.h"
enum effect { EFFECT_OFF, EFFECT_SOLID, EFFECT_BLINK, EFFECT_FAST,
	EFFECT_PULSE, EFFECT_CONNECTED, EFFECT_FLASH, EFFECT_ORANGE,
	EFFECT_BLINK_ORANGE, EFFECT_GREEN };
enum rule { R_START, R_READY, R_STOP, R_FAIL, R_ACK, R_ONLINE, R_LOST, RULES };
struct pattern_config {
	unsigned on_ms, off_ms;
	enum effect action[RULES];
};
struct gpio_config {
	unsigned brightness, pulse_count;
	bool restore;
	struct pattern_config target[TARGETS];
};
void config_defaults(struct gpio_config *c);
int config_read(int fd, struct gpio_config *out);
#endif
