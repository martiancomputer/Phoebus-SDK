#ifndef RTK_SWITCHDEV_H
#define RTK_SWITCHDEV_H
#include "rtk_switchdev_priv.h"
#include "rtk_switchdev_gmac.h"

typedef struct rtk_switchdev_event_s {
	char *event_name;
	int event_id;
} rtk_switchdev_event_t;

char *__get_event_name(rtk_switchdev_event_t *table, int table_num, int event_id);
struct net_device *rtksw_get_dev_by_port_id(unsigned int port_id);

#endif
