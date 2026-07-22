#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <linux/notifier.h>
#include "rtk_switchdev_main.h"
#include "rtk_switchdev_hw.h"

extern rtk_switchdev_event_t rtk_switchdev_event_tbl[];
extern struct rtksw_common *g_rtksw;

rtk_switchdev_event_t rtk_switchdev_hw_event_tbl[RTK_SWITCHDEV_FDB_DEL_TO_PORT] = {
	{"RTK_FDB_ADD_TO_PORT", RTK_SWITCHDEV_FDB_ADD_TO_PORT},
	{"RTK_FDB_DEL_TO_PORT", RTK_SWITCHDEV_FDB_DEL_TO_PORT},
};

static ATOMIC_NOTIFIER_HEAD(rtk_switchdev_notif_chain);

/**
 *	register_rtk_switchdev_notifier - Register notifier
 *	@nb: notifier_block
 *
 *	Register switch device notifier.
 */
int register_rtk_switchdev_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_register(&rtk_switchdev_notif_chain, nb);
}
EXPORT_SYMBOL_GPL(register_rtk_switchdev_notifier);

/**
 *	unregister_switchdev_notifier - Unregister notifier
 *	@nb: notifier_block
 *
 *	Unregister switch device notifier.
 */
int unregister_rtk_switchdev_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_unregister(&rtk_switchdev_notif_chain, nb);
}
EXPORT_SYMBOL_GPL(unregister_rtk_switchdev_notifier);

int call_rtk_switchdev_notifiers(unsigned long val, unsigned int port_id,
			     struct rtk_switchdev_notifier_info *info,
			     struct netlink_ext_ack *extack)
{
	info->port_id = port_id;
	info->extack = extack;
	return atomic_notifier_call_chain(&rtk_switchdev_notif_chain, val, info);
}
EXPORT_SYMBOL_GPL(call_rtk_switchdev_notifiers);

static int rtksw_switchdev_hw_get_vlan_vid(struct net_device *dev, int *vid) {
#ifdef CONFIG_VLAN_8021Q
	struct vlan_dev_priv *vlan;
#endif

#ifdef CONFIG_VLAN_8021Q
    if (is_vlan_dev(dev)) {
        vlan = vlan_dev_priv(dev);
        *vid = vlan->vlan_id;
        return 0;
    }
#endif

    return -1;
}

struct rtksw_switchdev_hw_notifier_data {
	unsigned long event;
	struct switchdev_notifier_info *info;
};

static inline int __rtksw_switchdev_hw_call_switchdev_notifiers(struct net_device *dev,
			       __always_unused struct netdev_nested_priv *priv)
{
	struct rtksw_switchdev_hw_notifier_data *data = priv->data;
	struct switchdev_notifier_fdb_info *fdb_info;
	int vid;

	if (netif_is_bridge_port(dev))
	{
		fdb_info = container_of(data->info, struct switchdev_notifier_fdb_info, info);
		if (rtksw_switchdev_hw_get_vlan_vid(dev, &vid) == 0) {
			if (fdb_info->vid != vid)
				return 0;
		}
		dev_info(g_rtksw->dev, "dev(%s) send event [%ld:%s]\n"
			, dev->name, data->event, __get_event_name(rtk_switchdev_event_tbl, SWITCHDEV_VXLAN_FDB_OFFLOADED, data->event));
		call_switchdev_notifiers(data->event, dev, data->info, NULL);
	}

	return 0;
}

static inline int rtksw_switchdev_hw_call_switchdev_notifiers(unsigned long val,
					   struct net_device *dev,
					   struct switchdev_notifier_info *info,
					   struct netlink_ext_ack *extack)
{
	struct switchdev_notifier_fdb_info *fdb_info = 
		container_of(info, struct switchdev_notifier_fdb_info, info);
	struct rtksw_priv *priv;
	int vid;

	if (netif_is_bridge_port(dev))
	{
		priv = netdev_priv(dev);

		if (rtksw_switchdev_hw_get_vlan_vid(dev, &vid) == 0) {
			if (fdb_info->vid != vid)
				return NOTIFY_DONE;
		}

		dev_info(priv->dev, "dev(%s) send event [%ld:%s]\n"
			, dev->name, val, __get_event_name(rtk_switchdev_event_tbl, SWITCHDEV_VXLAN_FDB_OFFLOADED, val));
		call_switchdev_notifiers(val, dev, info, extack);
	}
	else
	{
		struct rtksw_switchdev_hw_notifier_data data = {
			.event = val,
			.info = info
		};
		struct netdev_nested_priv nested_priv = {
			.data = (void *)&data,
		};
	    rcu_read_lock();
		netdev_walk_all_upper_dev_rcu(dev, __rtksw_switchdev_hw_call_switchdev_notifiers,
						      &nested_priv);
		rcu_read_unlock();
	}

	return NOTIFY_DONE;
}

static void rtksw_switchdev_hw_event_work(struct work_struct *work)
{
	struct switchdev_notifier_fdb_info info;
	struct rtksw_switchdev_hw_event_work *switchdev_hw_work =
		container_of(work, struct rtksw_switchdev_hw_event_work, work);
	struct rtk_switchdev_hw_notifier_fdb_info *fdb;
	struct net_device *ndev = NULL;
	struct rtksw_priv *priv;
	int port;

	rtnl_lock();
	switch (switchdev_hw_work->event) {
		case RTK_SWITCHDEV_FDB_ADD_TO_PORT:
			fdb = &switchdev_hw_work->fdb_info;
			port = fdb->info.port_id;
			if ((ndev = rtksw_get_dev_by_port_id(port)) != NULL) {
				priv = netdev_priv(ndev);
				dev_dbg(priv->dev, "rtksw_hw_fdb_add: MACID = %pM vid = %u -- port %d, ndev = %s\n",
					fdb->addr, fdb->vid, port, ndev->name);
				info.addr = fdb->addr;
				info.vid = fdb->vid;
				info.offloaded = true;
				rtksw_switchdev_hw_call_switchdev_notifiers(SWITCHDEV_FDB_ADD_TO_BRIDGE, ndev, &info.info, NULL);
			}
			break;
		case RTK_SWITCHDEV_FDB_DEL_TO_PORT:
			fdb = &switchdev_hw_work->fdb_info;
			port = fdb->info.port_id;
			if ((ndev = rtksw_get_dev_by_port_id(port)) != NULL) {
				priv = netdev_priv(ndev);
				dev_dbg(priv->dev, "rtksw_hw_fdb_del: MACID = %pM vid = %u -- port %d, ndev = %s\n",
					fdb->addr, fdb->vid, port, ndev->name);
				info.addr = fdb->addr;
				info.vid = fdb->vid;
				info.offloaded = false;
				rtksw_switchdev_hw_call_switchdev_notifiers(SWITCHDEV_FDB_DEL_TO_BRIDGE, ndev, &info.info, NULL);
			}
			break;
		default:
			break;
	}
	rtnl_unlock();

	kfree(switchdev_hw_work->fdb_info.addr);
	kfree(switchdev_hw_work);
}

/* called with RTNL or RCU */
static int rtk_switchdev_hw_event(struct notifier_block *unused,
			      unsigned long event, void *ptr)
{
	struct rtk_switchdev_hw_notifier_fdb_info *fdb_info = ptr;
	struct rtksw_switchdev_hw_event_work *switchdev_hw_work;
	struct net_device *ndev = NULL;
	struct rtksw_priv *priv;

	switchdev_hw_work = kzalloc(sizeof(*switchdev_hw_work), GFP_ATOMIC);
	if (WARN_ON(!switchdev_hw_work))
		return NOTIFY_BAD;

	INIT_WORK(&switchdev_hw_work->work, rtksw_switchdev_hw_event_work);
	switchdev_hw_work->event = event;

	switch (event) {
		case RTK_SWITCHDEV_FDB_ADD_TO_PORT:
		case RTK_SWITCHDEV_FDB_DEL_TO_PORT:
			memcpy(&switchdev_hw_work->fdb_info, ptr,
			       sizeof(switchdev_hw_work->fdb_info));
			switchdev_hw_work->fdb_info.addr = kzalloc(ETH_ALEN, GFP_ATOMIC);
			if (!switchdev_hw_work->fdb_info.addr)
				goto err_addr_alloc;
			ether_addr_copy((u8 *)switchdev_hw_work->fdb_info.addr,
					fdb_info->addr);

			if ((ndev = rtksw_get_dev_by_port_id(fdb_info->info.port_id)) != NULL) {
				priv = netdev_priv(ndev);

				dev_info(priv->dev, "dev(%s) rcv event [%ld:%s]\n",
					ndev->name, event, __get_event_name(rtk_switchdev_hw_event_tbl, RTK_SWITCHDEV_FDB_DEL_TO_PORT, event));
			}

			break;
		default:
			kfree(switchdev_hw_work);
			return NOTIFY_DONE;
	}

	queue_work(system_long_wq, &switchdev_hw_work->work);

	return NOTIFY_DONE;

err_addr_alloc:
	kfree(switchdev_hw_work);
	return NOTIFY_BAD;
}

static struct notifier_block rtksw_switchdev_hw_notifier = {
	.notifier_call = rtk_switchdev_hw_event,
};

int rtksw_switchdev_hw_register_notifiers(struct rtksw_common *rtksw)
{
	int ret = 0;

	ret = register_rtk_switchdev_notifier(&rtksw_switchdev_hw_notifier);
	if (ret) {
		dev_err(rtksw->dev, "register switchdev notifier fail ret:%d\n",
			ret);
		return ret;
	}

	return ret;
}

void rtksw_switchdev_hw_unregister_notifiers(struct rtksw_common *rtksw)
{
	unregister_rtk_switchdev_notifier(&rtksw_switchdev_hw_notifier);
}

