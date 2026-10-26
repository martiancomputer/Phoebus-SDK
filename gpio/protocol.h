/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef GPIO_PROTOCOL_H
#define GPIO_PROTOCOL_H
#include <stdint.h>
#define GPIO_MAGIC UINT32_C(0x47504931)
#define GPIO_VERSION 1
#define GPIO_SOCKET "/run/phoebus/gpio.sock"
enum gpio_op { GPIO_STATUS = 1, GPIO_IDENTIFY, GPIO_RELOAD, GPIO_SUBSCRIBE,
	GPIO_EVENT, GPIO_UPDATE };
enum gpio_target { POWER, LAN, INTERNET, WIFI_2G, WIFI_5G, VPN, TARGETS };
enum gpio_event { EV_STARTING = 1, EV_READY, EV_STOPPED, EV_FAILED,
	EV_WAN_BOUND, EV_WAN_LOST, EV_LEASES, EV_RESYNC };
/* Local-only, native-endian protocol. No pointers, paths, strings or uid fields.
 * Requests MUST zero all fields not defined for their operation. */
struct gpio_msg {
	uint32_t magic;
	uint16_t version, op;
	uint32_t target, arg, flags;
	int32_t status;
	uint32_t data[10];
};
_Static_assert(sizeof(struct gpio_msg) == 64, "fixed IPC ABI");
static inline struct gpio_msg gpio_message(unsigned op)
{
	struct gpio_msg m = { .magic = GPIO_MAGIC, .version = GPIO_VERSION, .op = op };
	return m;
}
#endif
