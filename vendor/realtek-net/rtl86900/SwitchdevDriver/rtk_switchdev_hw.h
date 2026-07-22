#ifndef RTK_SWITCHDEV_HW_H
#define RTK_SWITCHDEV_HW_H
#include <net/switchdev.h>
#include "rtk_switchdev_priv.h"

enum rtk_switchdev_hw_notifier_type {
	RTK_SWITCHDEV_FDB_ADD_TO_PORT = 1,
	RTK_SWITCHDEV_FDB_DEL_TO_PORT,
};

struct rtk_switchdev_notifier_info {
	unsigned int port_id;
	struct netlink_ext_ack *extack;
};

struct rtk_switchdev_hw_notifier_fdb_info {
	struct rtk_switchdev_notifier_info info; /* must be first */
	const unsigned char *addr;
	u16 vid;
};

struct rtksw_switchdev_hw_event_work {
	struct work_struct work;
	struct rtk_switchdev_hw_notifier_fdb_info fdb_info;
	unsigned long event;
};

int call_rtk_switchdev_notifiers(unsigned long val, unsigned int port_id,
			     struct rtk_switchdev_notifier_info *info,
			     struct netlink_ext_ack *extack);
int rtksw_switchdev_hw_register_notifiers(struct rtksw_common *rtksw);
void rtksw_switchdev_hw_unregister_notifiers(struct rtksw_common *rtksw);

#endif
