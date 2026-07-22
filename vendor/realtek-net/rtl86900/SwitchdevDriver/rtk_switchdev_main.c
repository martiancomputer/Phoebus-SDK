#include <generated/autoconf.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/compiler.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/in.h>
#include <linux/if_vlan.h>
#include <linux/crc32.h>
#include <linux/inet.h>
#include <linux/if_pppox.h>
#include <linux/ppp_defs.h>
#include <linux/icmp.h>
#include <linux/igmp.h>
#include <linux/of_net.h>
#include <linux/of_device.h>
#include <linux/of_mdio.h>

#include <net/ipv6.h>
#include <net/tcp.h>
#include <net/udp.h>
#include <net/ip6_checksum.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <net/xfrm.h>
#include <linux/proc_fs.h>
#include <linux/if_bridge.h>
#include <net/switchdev.h>
///--#include <bspchip.h>
#include "rtk_switchdev_main.h"
#include "rtk_switchdev_hw.h"
#include "rtk_switchdev_ethtool.h"
#include "error.h"
#include "rt_l2.h"
#include "rt_stat.h"

#define DRIVER_NAME "rtksw-switchdev"

static int debug_level = 0;
module_param(debug_level, int, 0);
MODULE_PARM_DESC(debug_level, "rtk_switchdev debug level (NETIF_MSG bits)");

struct rtksw_common *g_rtksw = NULL;

struct rtksw_switchdev_event_work {
	struct work_struct work;
	struct switchdev_notifier_fdb_info fdb_info;
	struct rtksw_priv *priv;
	unsigned long event;
};

rtk_switchdev_event_t rtk_port_attr_tbl[SWITCHDEV_ATTR_ID_BRIDGE_MROUTER+1] = {
	{"UNDEFINED", SWITCHDEV_ATTR_ID_UNDEFINED},
	{"PORT_STP_STATE", SWITCHDEV_ATTR_ID_PORT_STP_STATE},
	{"PORT_BRIDGE_FLAGS", SWITCHDEV_ATTR_ID_PORT_BRIDGE_FLAGS},
	{"PORT_MROUTE", SWITCHDEV_ATTR_ID_PORT_MROUTER},
	{"BRIDGE_AGEING_TIME", SWITCHDEV_ATTR_ID_BRIDGE_AGEING_TIME},
	{"BRIDGE_VLAN_FILTERING", SWITCHDEV_ATTR_ID_BRIDGE_VLAN_FILTERING},
	{"BRIDGE_MC_DISABLED", SWITCHDEV_ATTR_ID_BRIDGE_MC_DISABLED},
	{"BRIDGE_MROUTER", SWITCHDEV_ATTR_ID_BRIDGE_MROUTER},
};

#if !IS_ENABLED(CONFIG_BRIDGE_MRP)
rtk_switchdev_event_t rtk_port_obj_tbl[SWITCHDEV_OBJ_ID_HOST_MDB+1] = {
#else
rtk_switchdev_event_t rtk_port_obj_tbl[SWITCHDEV_OBJ_ID_IN_STATE_MRP+1] = {
#endif
	{"UNDEFINED", SWITCHDEV_OBJ_ID_UNDEFINED},
	{"PORT_VLAN", SWITCHDEV_OBJ_ID_PORT_VLAN},
	{"PORT_MDB", SWITCHDEV_OBJ_ID_PORT_MDB},
	{"HOST_MDB", SWITCHDEV_OBJ_ID_HOST_MDB},
#if IS_ENABLED(CONFIG_BRIDGE_MRP)
	{"MRP", SWITCHDEV_OBJ_ID_MRP},
	{"RING_TEST_MRP", SWITCHDEV_OBJ_ID_RING_TEST_MRP},
	{"RING_ROLE_MRP", SWITCHDEV_OBJ_ID_RING_ROLE_MRP},
	{"RING_STATE_MRP", SWITCHDEV_OBJ_ID_RING_STATE_MRP},
	{"IN_TEST_MRP", SWITCHDEV_OBJ_ID_IN_TEST_MRP},
	{"IN_ROLE_MRP", SWITCHDEV_OBJ_ID_IN_ROLE_MRP},
	{"IN_STATE_MRP", SWITCHDEV_OBJ_ID_IN_STATE_MRP},
#endif
};

/* SWITCHDEV_FDB_ADD_TO_BRIDGE start from 1 */
rtk_switchdev_event_t rtk_switchdev_event_tbl[SWITCHDEV_VXLAN_FDB_OFFLOADED] = {
	{"FDB_ADD_TO_BRIDGE", SWITCHDEV_FDB_ADD_TO_BRIDGE},
	{"FDB_DEL_TO_BRIDGE", SWITCHDEV_FDB_DEL_TO_BRIDGE},
	{"FDB_ADD_TO_DEVICE", SWITCHDEV_FDB_ADD_TO_DEVICE},
	{"FDB_DEL_TO_DEVICE", SWITCHDEV_FDB_DEL_TO_DEVICE},
	{"FDB_OFFLOADED", SWITCHDEV_FDB_OFFLOADED},
	{"FDB_FLUSH_TO_BRIDGE", SWITCHDEV_FDB_FLUSH_TO_BRIDGE},
	{"PORT_OBJ_ADD", SWITCHDEV_PORT_OBJ_ADD},
	{"PORT_OBJ_DEL", SWITCHDEV_PORT_OBJ_DEL},
	{"PORT_ATTR_SET", SWITCHDEV_PORT_ATTR_SET},
	{"VXLAN_FDB_ADD_TO_BRIDGE", SWITCHDEV_VXLAN_FDB_ADD_TO_BRIDGE},
	{"VXLAN_FDB_DEL_TO_BRIDGE", SWITCHDEV_VXLAN_FDB_DEL_TO_BRIDGE},
	{"VXLAN_FDB_ADD_TO_DEVICE", SWITCHDEV_VXLAN_FDB_ADD_TO_DEVICE},
	{"VXLAN_FDB_DEL_TO_DEVICE", SWITCHDEV_VXLAN_FDB_DEL_TO_DEVICE},
	{"VXLAN_FDB_OFFLOADED", SWITCHDEV_VXLAN_FDB_OFFLOADED},
};

rtk_switchdev_event_t rtk_fib_event_tbl[FIB_EVENT_VIF_DEL+1] = {
	{"ENTRY_REPLACE", FIB_EVENT_ENTRY_REPLACE},
	{"ENTRY_APPEND", FIB_EVENT_ENTRY_APPEND},
	{"ENTRY_ADD", FIB_EVENT_ENTRY_ADD},
	{"ENTRY_DEL", FIB_EVENT_ENTRY_DEL},
	{"RULE_ADD", FIB_EVENT_RULE_ADD},
	{"RULE_DEL", FIB_EVENT_RULE_DEL},
	{"NH_ADD", FIB_EVENT_NH_ADD},
	{"NH_DEL", FIB_EVENT_NH_DEL},
	{"VIF_ADD", FIB_EVENT_VIF_ADD},
	{"VIF_DEL", FIB_EVENT_VIF_DEL},
};

char *__get_event_name(rtk_switchdev_event_t *table, int table_num, int event_id)
{
	int i;

	for (i = 0; i < table_num; i++) {
		if (event_id == table[i].event_id)
			return table[i].event_name;
	}

	return "ERROR";
}

static int rtksw_open(struct net_device *dev)
{
	printk(KERN_INFO "%s opened\n", dev->name);

	if (dev->phydev)
		phy_start(dev->phydev);

	return 0;
}

static int rtksw_stop(struct net_device *dev)
{
	printk(KERN_INFO "%s closed\n", dev->name);

	if (dev->phydev)
		phy_stop(dev->phydev);

	return 0;
}

static netdev_tx_t rtksw_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	const struct net_device_ops *netdev_ops;
	netdev_ops = RTK_SWITCHDEV_GMAC_ROOT_NETDEV->netdev_ops;

	if (netdev_ops->ndo_start_xmit)
		return netdev_ops->ndo_start_xmit(skb, dev);

	return NETDEV_TX_BUSY;
}

static int rtksw_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	const struct net_device_ops *netdev_ops;
	netdev_ops = RTK_SWITCHDEV_GMAC_ROOT_NETDEV->netdev_ops;

	if (netdev_ops->ndo_do_ioctl)
		return netdev_ops->ndo_do_ioctl(dev, ifr, cmd);

	return -EOPNOTSUPP;
}

static void rtksw_tx_timeout(struct net_device *dev, unsigned int txqueue)
{
	const struct net_device_ops *netdev_ops;
	netdev_ops = RTK_SWITCHDEV_GMAC_ROOT_NETDEV->netdev_ops;

	if (netdev_ops->ndo_tx_timeout)
		return netdev_ops->ndo_tx_timeout(dev, txqueue);

	return;
}

static int rtksw_set_mac_address(struct net_device *dev, void *p)
{
	const struct net_device_ops *netdev_ops;
	netdev_ops = RTK_SWITCHDEV_GMAC_ROOT_NETDEV->netdev_ops;

	if (netdev_ops->ndo_set_mac_address)
		return netdev_ops->ndo_set_mac_address(dev, p);

	return 0;
}

static struct net_device_stats *rtksw_get_stats(struct net_device *dev)
{
	const struct net_device_ops *netdev_ops;
	netdev_ops = RTK_SWITCHDEV_GMAC_ROOT_NETDEV->netdev_ops;

	if (netdev_ops->ndo_get_stats)
		return netdev_ops->ndo_get_stats(dev);

	return &dev->stats;
}

static void rtksw_get_stats64(struct net_device *dev, struct rtnl_link_stats64 *storage)
{
	struct rtksw_priv *priv = netdev_priv(dev);
#ifdef CONFIG_COMMON_RT_API
	rt_stat_port_cntr_t pPortCntrs;
#endif

	if (unlikely(storage == NULL))
		return;

#ifdef CONFIG_COMMON_RT_API
	if (rt_stat_port_getAll(priv->emac_port, &pPortCntrs) == RT_ERR_OK)
	{
		//memset(stats, 0, sizeof(*stats));
		storage->rx_bytes = (unsigned long long) pPortCntrs.ifInOctets;
		storage->rx_packets = (unsigned long long) (pPortCntrs.ifInUcastPkts+pPortCntrs.ifInMulticastPkts+pPortCntrs.ifInBroadcastPkts);
		storage->rx_errors += (unsigned long long) (pPortCntrs.dot3StatsAligmentErrors+pPortCntrs.dot3StatsFCSErrors+pPortCntrs.dot3StatsSymbolErrors+pPortCntrs.etherStatsCRCAlignErrors);
		storage->rx_dropped += (unsigned long long) (pPortCntrs.ifInDiscards);
		storage->rx_length_errors += (unsigned long long) (pPortCntrs.etherStatsRxUndersizePkts+pPortCntrs.etherStatsRxOversizePkts+pPortCntrs.etherStatsCRCAlignErrors);
		storage->multicast = (unsigned long long) pPortCntrs.ifInMulticastPkts;
		storage->tx_bytes = (unsigned long long) pPortCntrs.ifOutOctets;
		storage->tx_packets = (unsigned long long) (pPortCntrs.ifOutUcastPkts+pPortCntrs.ifOutMulticastPkts+pPortCntrs.ifOutBrocastPkts);
		storage->tx_errors += (unsigned long long) pPortCntrs.etherStatsTxCRCAlignErrors;
		storage->tx_dropped += (unsigned long long) pPortCntrs.ifOutDiscards;
		storage->collisions += (unsigned long long) (pPortCntrs.dot3StatsSingleCollisionFrames+pPortCntrs.dot3StatsMultipleCollisionFrames);
#if defined(CONFIG_RTK_NETIF_EXTRA_STATS)
		storage->rx_mc_bytes	 = (unsigned long long) 0;
		storage->tx_mc_bytes	 = (unsigned long long) 0;
		storage->tx_mc_packets = (unsigned long long) pPortCntrs.ifOutMulticastPkts;
						
		storage->rx_bc_bytes	 = (unsigned long long) 0;
		storage->rx_bc_packets = (unsigned long long) pPortCntrs.ifInBroadcastPkts;
		storage->tx_bc_bytes	 = (unsigned long long) 0;
		storage->tx_bc_packets = (unsigned long long) pPortCntrs.ifOutBrocastPkts;
						
		storage->rx_uc_bytes	 = (unsigned long long) 0;
		storage->rx_uc_packets = (unsigned long long) pPortCntrs.ifInUcastPkts;
		storage->tx_uc_bytes	 = (unsigned long long) 0;
		storage->tx_uc_packets = (unsigned long long) pPortCntrs.ifOutUcastPkts;
#endif
	}
#endif
	return;
}

static int rtksw_ndo_get_phys_port_name(struct net_device *ndev, char *name,
				       size_t len)
{
	struct rtksw_priv *priv = netdev_priv(ndev);
	int err;

	err = snprintf(name, len, "p%d", priv->emac_port);

	if (err >= len)
		return -EINVAL;

	return 0;
}

static int rtksw_get_port_parent_id(struct net_device *ndev,
				   struct netdev_phys_item_id *ppid)
{
	struct rtksw_common *rtksw = ndev_to_rtksw(ndev);

	ppid->id_len = sizeof(rtksw->base_mac);
	memcpy(&ppid->id, &rtksw->base_mac, ppid->id_len);

	return 0;
}

static const struct net_device_ops rtksw_netdev_ops = {
	.ndo_open		= rtksw_open,
	.ndo_stop		= rtksw_stop,
	.ndo_start_xmit 	= rtksw_start_xmit,
	.ndo_do_ioctl		= rtksw_ioctl,
	.ndo_tx_timeout		= rtksw_tx_timeout,
	.ndo_set_mac_address = rtksw_set_mac_address,
	.ndo_get_stats		= rtksw_get_stats,
	.ndo_get_stats64	= rtksw_get_stats64,
	.ndo_get_phys_port_name = rtksw_ndo_get_phys_port_name,
	.ndo_get_port_parent_id	= rtksw_get_port_parent_id,
};

static int rtksw_probe_dt(struct rtksw_common *rtksw)
{
	struct device_node *node = rtksw->dev->of_node, *tmp_node, *port_np;
	struct rtksw_platform_data *data = &rtksw->data;
	struct device *dev = rtksw->dev;
	const char *netdev_name;
	u32 slave_data_index;
	int ret, i;

	if (!node)
		return -EINVAL;

	tmp_node = of_get_child_by_name(node, "ethernet-ports");
	if (!tmp_node)
		return -ENOENT;
	data->slaves = of_get_child_count(tmp_node);
	if (data->slaves > RTKSW_MAX_SLAVE_PORTS_NUM) {
		of_node_put(tmp_node);
		return -ENOENT;
	}

	data->active_slave = 0;
	data->mac_control = 0;

	data->slave_data = devm_kcalloc(dev, data->slaves,
					sizeof(struct rtksw_slave_data),
					GFP_KERNEL);
	if (!data->slave_data)
		return -ENOMEM;

	/* Populate all the child nodes here...
	 */
	ret = devm_of_platform_populate(dev);
	/* We do not want to force this, as in some cases may not have child */
	if (ret)
		dev_warn(dev, "Doesn't have any child node\n");

	slave_data_index = 0;
	for_each_child_of_node(tmp_node, port_np) {
		struct rtksw_slave_data *slave_data;
		const void *mac_addr;
		u32 port_id;

		if (slave_data_index >= data->slaves) {
			dev_err(dev, "%pOF has invalid slave_data_index %u\n",
				port_np, slave_data_index);
			ret = -EINVAL;
			goto err_node_put;
		}

		slave_data = &data->slave_data[slave_data_index];
		slave_data->slave_node = port_np;

		ret = of_property_read_u32(port_np, "id", &port_id);
		if (ret < 0) {
			dev_err(dev, "%pOF error reading port_id %d\n",
				port_np, ret);
			goto err_node_put;
		}
		slave_data->port_id = port_id;

		ret = of_property_read_string(port_np, "netdev-name", &netdev_name);
        if (ret == 0) {
            memcpy(slave_data->netdev_name, netdev_name, sizeof(slave_data->netdev_name));
        }

		mac_addr = of_get_mac_address(port_np);
		if (!IS_ERR(mac_addr)) {
			ether_addr_copy(slave_data->mac_addr, mac_addr);
		} else {
			for (i = 0; i < 3; i++)
				((u16 *) (slave_data->mac_addr))[i] = i;
		}

		rtksw->slaves[slave_data_index].node = port_np;

		slave_data_index++;
	}

	of_node_put(tmp_node);
	return 0;

err_node_put:
	of_node_put(port_np);
	return ret;
}

static void rtksw_remove_dt(struct rtksw_common *rtksw)
{
	return;
}

static void rtksw_unregister_ports(struct rtksw_common *rtksw)
{
	int i = 0;

	for (i = 0; i < rtksw->data.slaves; i++) {
		if (!rtksw->slaves[i].ndev)
			continue;

		unregister_netdev(rtksw->slaves[i].ndev);
	}
}

static int rtksw_register_ports(struct rtksw_common *rtksw)
{
	struct rtksw_priv *priv;
	int ret = 0, i = 0;
	int rx_polarity_reverse = 0;
	int tx_polarity_reverse = 0;

	for (i = 0; i < rtksw->data.slaves; i++) {
		if (!rtksw->slaves[i].ndev)
			continue;

		/* register the network device */
		ret = register_netdev(rtksw->slaves[i].ndev);
		if (ret) {
			dev_err(rtksw->dev,
				"err registering net device%d\n", i);
			rtksw->slaves[i].ndev = NULL;
			break;
		}

		priv = netdev_priv(rtksw->slaves[i].ndev);
		if (priv)
			rtksw_gmac_dev_port_mapping(priv->emac_port, rtksw->slaves[i].ndev->name);

		if (of_phy_is_fixed_link(rtksw->slaves[i].node)) {
			if (rtksw->slaves[i].ndev->phydev) {
				dev_info(rtksw->dev, "fixed-link: %s\n", phy_modes(rtksw->slaves[i].ndev->phydev->interface));
				phy_start(rtksw->slaves[i].ndev->phydev);
			}
		}

			if (of_find_property(rtksw->slaves[i].node, "rx-invert-polarity", NULL) 
				|| of_find_property(rtksw->slaves[i].node, "tx-invert-polarity", NULL)) {
 				rx_polarity_reverse = of_property_read_bool(rtksw->slaves[i].node, "rx-invert-polarity");
				tx_polarity_reverse = of_property_read_bool(rtksw->slaves[i].node, "tx-invert-polarity");
				dev_info(rtksw->dev, "polarity: txPn=%d, rxPn=%d\n", tx_polarity_reverse, rx_polarity_reverse);
				rtksw_gmac_sds_polarity_set(priv, tx_polarity_reverse, rx_polarity_reverse);
			}
	}

	if (ret)
		rtksw_unregister_ports(rtksw);

	return ret;
}

bool rtksw_port_dev_real_check(const struct net_device *ndev)
{
	if (ndev->netdev_ops == &rtksw_netdev_ops) {
		struct rtksw_common *rtksw = ndev_to_rtksw(ndev);
		struct rtksw_slave_data *slave_data = rtksw->data.slave_data;

		return !slave_data->disabled;
	}

	return false;
}

bool rtksw_port_dev_virt_check(struct net_device *ndev, struct net_device **real_ndev)
{
	struct net_device *lower_ndev;
	struct list_head *iter;
#ifdef CONFIG_VLAN_8021Q
	struct net_device *temp_ndev;
#endif

#ifdef CONFIG_RTL_SMUX_DEV
	unsigned int flags = rtk_netdev_get_flags(ndev);

	while (flags & RTK_IFF_VSMUX)
	{
		rcu_read_lock();  // Acquire the RCU read lock for safe traversal

	    netdev_for_each_lower_dev(ndev, lower_ndev, iter) {
	        ///--printk(KERN_INFO "Lower netdev: %s\n", lower_ndev->name);
	        if (rtksw_port_dev_real_check(lower_ndev)) {
				*real_ndev = lower_ndev;
				return true;
			}
	        flags = rtk_netdev_get_flags(lower_ndev);
	    }

	    rcu_read_unlock();  // Release the RCU read lock
	}
#endif

#ifdef CONFIG_VLAN_8021Q
	temp_ndev = ndev;
	while (is_vlan_dev(temp_ndev))
	{
		rcu_read_lock();  // Acquire the RCU read lock for safe traversal

	    netdev_for_each_lower_dev(temp_ndev, lower_ndev, iter) {
	        ///--printk(KERN_INFO "Lower netdev: %s\n", lower_ndev->name);
	        if (rtksw_port_dev_real_check(lower_ndev)) {
				*real_ndev = lower_ndev;
				return true;
			}
	        temp_ndev = lower_ndev;
	    }

	    rcu_read_unlock();  // Release the RCU read lock
	}
#endif

	return false;
}

bool rtksw_port_dev_check(const struct net_device *ndev)
{
	struct net_device *real_ndev = NULL;

	if (rtksw_port_dev_virt_check((struct net_device *)ndev, &real_ndev))
		return true;

	if (rtksw_port_dev_real_check(ndev))
		return true;

	return false;
}

struct net_device *rtksw_get_dev_by_port_id(unsigned int port_id)
{
	struct rtksw_common *rtksw = g_rtksw;
	struct rtksw_platform_data *data;
	struct rtksw_slave *slaves;
	int i = 0;

	if (rtksw == NULL)
		return NULL;

	data = &rtksw->data;
	for (i = 0; i < rtksw->data.slaves; i++) {
		struct rtksw_slave_data *slave_data = &data->slave_data[i];

		if (slave_data->disabled)
			continue;

		if (slave_data->port_id != port_id)
			continue;

		if (rtksw->slaves == NULL)
			continue;

		slaves = &rtksw->slaves[i];
		return slaves->ndev;
	}

	return NULL;
}

static int rtksw_create_ports(struct rtksw_common *rtksw)
{
	struct rtksw_platform_data *data = &rtksw->data;
	struct net_device *ndev;
	struct device *dev = rtksw->dev;
	struct rtksw_priv *priv;
	int ret = 0, i = 0;
	struct phy_device *phydev;

	for (i = 0; i < rtksw->data.slaves; i++) {
		struct rtksw_slave_data *slave_data = &data->slave_data[i];

		if (slave_data->disabled)
			continue;

		ndev = devm_alloc_etherdev_mqs(dev, sizeof(struct rtksw_priv),
					       RTKSW_MAX_QUEUES,
					       RTKSW_MAX_QUEUES);
		if (!ndev) {
			dev_err(dev, "error allocating net_device\n");
			return -ENOMEM;
		}

		priv = netdev_priv(ndev);
		priv->rtksw = rtksw;
		priv->ndev = ndev;
		priv->dev = dev;
		priv->msg_enable = netif_msg_init(debug_level, RTKSW_DEBUG);
		priv->emac_port = slave_data->port_id;

		rtksw_gmac_priv_init(priv, slave_data->port_id);
		rtksw_gmac_dev_init(ndev);

		if (is_valid_ether_addr(slave_data->mac_addr)) {
			ether_addr_copy(priv->mac_addr, slave_data->mac_addr);
			dev_info(rtksw->dev, "Detected Netdev = %s MACID = %pM at Port %d\n",
				 slave_data->netdev_name, priv->mac_addr, priv->emac_port);
		} else {
			eth_random_addr(slave_data->mac_addr);
			dev_info(rtksw->dev, "Random MACID = %pM\n",
				 priv->mac_addr);
		}
		ether_addr_copy(ndev->dev_addr, slave_data->mac_addr);
		ether_addr_copy(priv->mac_addr, slave_data->mac_addr);
		rtksw->slaves[i].ndev = ndev;
		ndev->features |= NETIF_F_NETNS_LOCAL;

		ndev->netdev_ops = &rtksw_netdev_ops;
		rtksw_set_ethtool_ops(ndev);

		memcpy(ndev->name, slave_data->netdev_name, sizeof(ndev->name));
		SET_NETDEV_DEV(ndev, dev);

		if (rtksw->slaves[i].node) {
			phydev = of_phy_get_and_connect(ndev, rtksw->slaves[i].node,
				rtksw_gmac_adjust_link);
		}
	}

	return ret;
}

static int rtksw_port_attr_br_flags_pre_set(struct net_device *netdev,
					   struct switchdev_trans *trans,
					   unsigned long flags)
{
	if (flags & ~(BR_LEARNING | BR_MCAST_FLOOD))
		return -EINVAL;

	return 0;
}

static int rtksw_port_attr_set(struct net_device *ndev,
			      const struct switchdev_attr *attr,
			      struct switchdev_trans *trans)
{
	struct net_device *real_ndev = NULL, *target_ndev = ndev;
	struct rtksw_priv *priv = NULL;
#ifdef CONFIG_COMMON_RT_API
	unsigned int data;
#endif
	struct timespec64 value;
	int ret;

	if (rtksw_port_dev_virt_check(ndev, &real_ndev))
	{
		target_ndev = real_ndev;
	}
	else if (!rtksw_port_dev_real_check(ndev))
	{
		ret = -EOPNOTSUPP;
		return ret;
	}

	priv = netdev_priv(target_ndev);

	dev_info(priv->dev, "attr: id [%u:%s] port: %u\n", attr->id
		, __get_event_name(rtk_port_attr_tbl, (SWITCHDEV_ATTR_ID_BRIDGE_MROUTER+1), attr->id)
		, priv->emac_port);

	switch (attr->id) {
		case SWITCHDEV_ATTR_ID_PORT_PRE_BRIDGE_FLAGS:
			ret = rtksw_port_attr_br_flags_pre_set(ndev, trans,
							      attr->u.brport_flags);
			break;
		case SWITCHDEV_ATTR_ID_PORT_STP_STATE:
			dev_dbg(priv->dev, "Do nothing now ...\n");
			break;
#if 0
			ret = ca_ni_get_port_id_by_dev(attr->orig_dev, &port_id);
			if (ret != CA_E_OK) {
				printk("%s: ca_ni_get_port_id_by_dev failed, dev=%s\n", __func__, attr->orig_dev->name);
				return -EPERM;
			}

			ret = ca_port_stp_state_get(0, port_id, &stp_state);
			if (ret != CA_E_OK)
				printk("%s: ca_port_stp_state_get() failed, ret=%d\n", __func__, ret);

			CA_SWDEV_LOG(SWDEV_DBG_STP, "SWITCHDEV_ATTR_ID_PORT_STP_STATE, attr->u.stp_state=%d, attr->orig_dev->name=%s, port_id=0x%x\n",
					 attr->u.stp_state, attr->orig_dev->name, port_id);
			CA_SWDEV_LOG(SWDEV_DBG_STP, "current STP state is %d\n", stp_state);
			switch (attr->u.stp_state) {
				case BR_STATE_DISABLED:
					stp_state = CA_PORT_STP_DISABLED;
					break;
				case BR_STATE_LISTENING:
					stp_state = CA_PORT_STP_LEARNING;
					break;
				case BR_STATE_LEARNING:
					stp_state = CA_PORT_STP_LEARNING;
					break;
				case BR_STATE_FORWARDING:
					stp_state = CA_PORT_STP_FORWARDING;
					break;
				case BR_STATE_BLOCKING:
					stp_state = CA_PORT_STP_BLOCKING;
					break;
			}
			ret = ca_port_stp_state_set(0, port_id, stp_state);
			if (ret != CA_E_OK)
				printk("%s: ca_port_stp_state_set() failed, ret=%d\n", __func__, ret);

			CA_SWDEV_LOG(SWDEV_DBG_STP, "STP state set to %d\n", stp_state);
			break;
#endif
		case SWITCHDEV_ATTR_ID_PORT_BRIDGE_FLAGS:
			dev_dbg(priv->dev, "SWITCHDEV_ATTR_ID_PORT_BRIDGE_FLAGS, attr->u.brport_flags=%ld\n", attr->u.brport_flags);
			break;
		case SWITCHDEV_ATTR_ID_BRIDGE_AGEING_TIME:
			jiffies_to_timespec64(clock_t_to_jiffies(attr->u.ageing_time), &value);
			dev_dbg(priv->dev, "SWITCHDEV_ATTR_ID_BRIDGE_AGEING_TIME, attr->u.ageing_time=%ld\n", attr->u.ageing_time);
			dev_dbg(priv->dev, "Svalue.tv_sec = %lld, value->tv_nsec=%ld\n", value.tv_sec, value.tv_nsec);
#ifdef CONFIG_COMMON_RT_API
			data = (unsigned int) value.tv_sec;
			if ((ret = rt_l2_ageTime_set(data)) == RT_ERR_OK) {
				data = 0;
				if ((ret = rt_l2_ageTime_get(&data)) == RT_ERR_OK) {
					dev_dbg(priv->dev, "ageing time is updated to %d(s)\n", data);
				}
			}
#endif
			break;
		case SWITCHDEV_ATTR_ID_BRIDGE_VLAN_FILTERING:
			dev_dbg(priv->dev, "setup vlan filtering to %d\n", attr->u.vlan_filtering);
			break;
		default:
			ret = -EOPNOTSUPP;
			break;
	}

	return ret;
}

static int rtksw_port_obj_add(struct net_device *ndev,
			     const struct switchdev_obj *obj,
			     struct switchdev_trans *trans,
			     struct netlink_ext_ack *extack)
{
	///--struct switchdev_obj_port_vlan *vlan = SWITCHDEV_OBJ_PORT_VLAN(obj);
	///--struct switchdev_obj_port_mdb *mdb = SWITCHDEV_OBJ_PORT_MDB(obj);
	struct net_device *real_ndev = NULL, *target_ndev = ndev;
	struct rtksw_priv *priv = NULL;
	int err = 0;

	if (rtksw_port_dev_virt_check(ndev, &real_ndev))
	{
		target_ndev = real_ndev;
	}
	else if (!rtksw_port_dev_real_check(ndev))
	{
		err = -EOPNOTSUPP;
		return err;
	}

	priv = netdev_priv(target_ndev);

#if IS_ENABLED(CONFIG_BRIDGE_MRP)
	dev_info(priv->dev, "obj_add: id [%u:%s] port: %u\n",
		obj->id, __get_event_name(rtk_port_obj_tbl, (SWITCHDEV_OBJ_ID_IN_STATE_MRP+1), obj->id), priv->emac_port);
#else
	dev_info(priv->dev, "obj_add: id [%u:%s] port: %u\n",
		obj->id, __get_event_name(rtk_port_obj_tbl, (SWITCHDEV_OBJ_ID_HOST_MDB+1), obj->id), priv->emac_port);
#endif

	switch (obj->id) {
		case SWITCHDEV_OBJ_ID_PORT_VLAN:
			///--err = cpsw_port_vlans_add(priv, vlan, trans);
			break;
		case SWITCHDEV_OBJ_ID_PORT_MDB:
		case SWITCHDEV_OBJ_ID_HOST_MDB:
			///--err = cpsw_port_mdb_add(priv, mdb, trans);
			break;
		default:
			err = -EOPNOTSUPP;
			break;
	}

	return err;
}

static int rtksw_port_obj_del(struct net_device *ndev,
			     const struct switchdev_obj *obj)
{
	///--struct switchdev_obj_port_vlan *vlan = SWITCHDEV_OBJ_PORT_VLAN(obj);
	///--struct switchdev_obj_port_mdb *mdb = SWITCHDEV_OBJ_PORT_MDB(obj);
	struct net_device *real_ndev = NULL, *target_ndev = ndev;
	struct rtksw_priv *priv = NULL;
	int err = 0;

	if (rtksw_port_dev_virt_check(ndev, &real_ndev))
	{
		target_ndev = real_ndev;
	}
	else if (!rtksw_port_dev_real_check(ndev))
	{
		err = -EOPNOTSUPP;
		return err;
	}

	priv = netdev_priv(target_ndev);

#if IS_ENABLED(CONFIG_BRIDGE_MRP)
	dev_info(priv->dev, "obj_del: id [%u:%s] port: %u\n",
		obj->id, __get_event_name(rtk_port_obj_tbl, (SWITCHDEV_OBJ_ID_IN_STATE_MRP+1), obj->id), priv->emac_port);
#else
	dev_info(priv->dev, "obj_del: id [%u:%s] port: %u\n",
		obj->id, __get_event_name(rtk_port_obj_tbl, (SWITCHDEV_OBJ_ID_HOST_MDB+1), obj->id), priv->emac_port);
#endif

	switch (obj->id) {
	case SWITCHDEV_OBJ_ID_PORT_VLAN:
		//--err = cpsw_port_vlans_del(priv, vlan);
		break;
	case SWITCHDEV_OBJ_ID_PORT_MDB:
	case SWITCHDEV_OBJ_ID_HOST_MDB:
		///--err = cpsw_port_mdb_del(priv, mdb);
		break;
	default:
		err = -EOPNOTSUPP;
		break;
	}

	return err;
}

#if 0
static void rtksw_fdb_offload_notify(struct net_device *ndev,
				    struct switchdev_notifier_fdb_info *rcv)
{
	struct switchdev_notifier_fdb_info info;

	info.addr = rcv->addr;
	info.vid = rcv->vid;
	info.offloaded = true;
	call_switchdev_notifiers(SWITCHDEV_FDB_OFFLOADED,
				 ndev, &info.info, NULL);
}
#endif

static void rtksw_switchdev_event_work(struct work_struct *work)
{
	struct rtksw_switchdev_event_work *switchdev_work =
		container_of(work, struct rtksw_switchdev_event_work, work);
	struct rtksw_priv *priv = switchdev_work->priv;
	struct switchdev_notifier_fdb_info *fdb;
	struct rtksw_common *rtksw = priv->rtksw;
	int port = priv->emac_port;
#if 0//def CONFIG_COMMON_RT_API
	rt_l2_ucastAddr_t l2Addr;
	int ret;
#endif

	rtnl_lock();
	switch (switchdev_work->event) {
		case SWITCHDEV_FDB_ADD_TO_DEVICE:
			fdb = &switchdev_work->fdb_info;

			dev_dbg(rtksw->dev, "rtksw_fdb_add: MACID = %pM vid = %u flags = %u %u -- port %d\n",
				fdb->addr, fdb->vid, fdb->added_by_user,
				fdb->offloaded, port);

			if (!fdb->added_by_user)
				break;
			if (memcmp(priv->mac_addr, (u8 *)fdb->addr, ETH_ALEN) == 0)
				port = 0;

#if 0//def CONFIG_COMMON_RT_API
			l2Addr.port = port;
			memcpy(l2Addr.mac.octet, fdb->addr, sizeof(l2Addr.mac.octet));
			l2Addr.vid = fdb->vid;
			if ((ret = rt_l2_addr_add(&l2Addr)) == RT_ERR_OK)
				rtksw_fdb_offload_notify(priv->ndev, fdb);
#else
			dev_dbg(rtksw->dev, "do nothing now. \n");
#endif
			break;
		case SWITCHDEV_FDB_DEL_TO_DEVICE:
			fdb = &switchdev_work->fdb_info;

			dev_dbg(rtksw->dev, "rtksw_fdb_del: MACID = %pM vid = %u flags = %u %u -- port %d\n",
				fdb->addr, fdb->vid, fdb->added_by_user,
				fdb->offloaded, port);

			if (!fdb->added_by_user)
				break;
			if (memcmp(priv->mac_addr, (u8 *)fdb->addr, ETH_ALEN) == 0)
				port = 0;

#if 0//def CONFIG_COMMON_RT_API
			l2Addr.port = port;
			memcpy(l2Addr.mac.octet, fdb->addr, sizeof(l2Addr.mac.octet));
			l2Addr.vid = fdb->vid;
			ret = rt_l2_addr_del(&l2Addr);
#else
			dev_dbg(rtksw->dev, "do nothing now. \n");
#endif

			break;
		default:
			break;
	}
	rtnl_unlock();

	kfree(switchdev_work->fdb_info.addr);
	kfree(switchdev_work);
	dev_put(priv->ndev);
}


/* called under rcu_read_lock() */
static int rtksw_switchdev_event(struct notifier_block *unused,
				unsigned long event, void *ptr)
{
	struct net_device *ndev = switchdev_notifier_info_to_dev(ptr), *real_ndev = NULL;
	struct switchdev_notifier_fdb_info *fdb_info = ptr;
	struct rtksw_switchdev_event_work *switchdev_work;
	struct rtksw_priv *priv;
	int err;

	if (rtksw_port_dev_virt_check(ndev, &real_ndev))
	{
		ndev = real_ndev;
	}
	else if (!rtksw_port_dev_real_check(ndev))
	{
		return NOTIFY_DONE;
	}

	priv = netdev_priv(ndev);

	dev_info(priv->dev, "dev(%s) rcv event [%ld:%s]\n",
		ndev->name, event, __get_event_name(rtk_switchdev_event_tbl, SWITCHDEV_VXLAN_FDB_OFFLOADED, event));

	if (event == SWITCHDEV_PORT_ATTR_SET) {
		err = switchdev_handle_port_attr_set(ndev, ptr,
						     rtksw_port_dev_check,
						     rtksw_port_attr_set);
		return notifier_from_errno(err);
	}

	switchdev_work = kzalloc(sizeof(*switchdev_work), GFP_ATOMIC);
	if (WARN_ON(!switchdev_work))
		return NOTIFY_BAD;

	INIT_WORK(&switchdev_work->work, rtksw_switchdev_event_work);
	switchdev_work->priv = priv;
	switchdev_work->event = event;

	switch (event) {
		case SWITCHDEV_FDB_ADD_TO_DEVICE:
		case SWITCHDEV_FDB_DEL_TO_DEVICE:
			memcpy(&switchdev_work->fdb_info, ptr,
			       sizeof(switchdev_work->fdb_info));
			switchdev_work->fdb_info.addr = kzalloc(ETH_ALEN, GFP_ATOMIC);
			if (!switchdev_work->fdb_info.addr)
				goto err_addr_alloc;
			ether_addr_copy((u8 *)switchdev_work->fdb_info.addr,
					fdb_info->addr);
			dev_hold(ndev);
			break;
		default:
			kfree(switchdev_work);
			return NOTIFY_DONE;
	}

	queue_work(system_long_wq, &switchdev_work->work);

	return NOTIFY_DONE;

err_addr_alloc:
	kfree(switchdev_work);
	return NOTIFY_BAD;
}

static struct notifier_block rtksw_switchdev_notifier = {
	.notifier_call = rtksw_switchdev_event,
};

static int rtksw_switchdev_blocking_event(struct notifier_block *unused,
					 unsigned long event, void *ptr)
{
	struct net_device *dev = switchdev_notifier_info_to_dev(ptr);
	int err;

	switch (event) {
		case SWITCHDEV_PORT_OBJ_ADD:
			err = switchdev_handle_port_obj_add(dev, ptr,
							    rtksw_port_dev_check,
							    rtksw_port_obj_add);
			return notifier_from_errno(err);
		case SWITCHDEV_PORT_OBJ_DEL:
			err = switchdev_handle_port_obj_del(dev, ptr,
							    rtksw_port_dev_check,
							    rtksw_port_obj_del);
			return notifier_from_errno(err);
		case SWITCHDEV_PORT_ATTR_SET:
			err = switchdev_handle_port_attr_set(dev, ptr,
							     rtksw_port_dev_check,
							     rtksw_port_attr_set);
			return notifier_from_errno(err);
		default:
			break;
	}

	return NOTIFY_DONE;
}

static struct notifier_block rtksw_switchdev_bl_notifier = {
	.notifier_call = rtksw_switchdev_blocking_event,
};

int rtksw_switchdev_register_notifiers(struct rtksw_common *rtksw)
{
	int ret = 0;

	ret = register_switchdev_notifier(&rtksw_switchdev_notifier);
	if (ret) {
		dev_err(rtksw->dev, "register switchdev notifier fail ret:%d\n",
			ret);
		return ret;
	}

	ret = register_switchdev_blocking_notifier(&rtksw_switchdev_bl_notifier);
	if (ret) {
		dev_err(rtksw->dev, "register switchdev blocking notifier ret:%d\n",
			ret);
		unregister_switchdev_notifier(&rtksw_switchdev_notifier);
	}

	return ret;
}

void rtksw_switchdev_unregister_notifiers(struct rtksw_common *rtksw)
{
	unregister_switchdev_blocking_notifier(&rtksw_switchdev_bl_notifier);
	unregister_switchdev_notifier(&rtksw_switchdev_notifier);
}

static int rtksw_register_notifiers(struct rtksw_common *rtksw)
{
	int ret = 0;

	ret = rtksw_switchdev_register_notifiers(rtksw);
	if (ret) {
		dev_err(rtksw->dev, "can't register switchdev notifier\n");
		return ret;
	}

	ret = rtksw_switchdev_hw_register_notifiers(rtksw);
	if (ret) {
		dev_err(rtksw->dev, "can't register rtk_switchdev HW notifier\n");
		return ret;
	}

	return ret;
}

static void rtksw_unregister_notifiers(struct rtksw_common *rtksw)
{
	rtksw_switchdev_unregister_notifiers(rtksw);
	rtksw_switchdev_hw_unregister_notifiers(rtksw);
}

static int rtksw_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct rtksw_common *rtksw;
	int ret = 0;

	printk(KERN_INFO "%s probe.\n", DRIVER_NAME);

	rtksw = devm_kzalloc(dev, sizeof(struct rtksw_common), GFP_KERNEL);
	if (!rtksw)
		return -ENOMEM;

	rtksw->dev = dev;

	rtksw->slaves = devm_kcalloc(dev,
				    RTKSW_MAX_SLAVE_PORTS_NUM,
				    sizeof(struct rtksw_slave),
				    GFP_KERNEL);
	if (!rtksw->slaves)
		return -ENOMEM;

	ret = rtksw_probe_dt(rtksw);
	if (ret)
		goto clean_dt_ret;

	/* setup netdevs */
	ret = rtksw_create_ports(rtksw);
	if (ret)
		goto clean_unregister_netdev;

	ret = rtksw_register_notifiers(rtksw);
	if (ret)
		goto clean_unregister_netdev;

	ret = rtksw_register_ports(rtksw);
	if (ret)
		goto clean_unregister_notifiers;

	g_rtksw = rtksw;

	return 0;
clean_unregister_notifiers:
	rtksw_unregister_notifiers(rtksw);
clean_unregister_netdev:
	rtksw_unregister_ports(rtksw);
clean_dt_ret:
	rtksw_remove_dt(rtksw);
	return ret;
}

static int rtksw_remove(struct platform_device *pdev)
{
	struct rtksw_common *rtksw = platform_get_drvdata(pdev);

	printk(KERN_INFO "%s remove.\n", DRIVER_NAME);

	rtksw_unregister_ports(rtksw);
	rtksw_remove_dt(rtksw);

	return 0;
}

#ifdef CONFIG_PM
static int rtksw_resume(struct platform_device *pdev)
{
	return 0;
}

static int rtksw_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}
#else /* CONFIG_PM */
#define rtksw_resume	NULL
#define rtksw_suspend	NULL
#endif /* CONFIG_PM */

static const struct of_device_id rtksw_of_mtable[] = {
	{ .compatible = "realtek,switchdev" },
	{ /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, rtksw_of_mtable);

static struct platform_driver rtksw_driver = {
	.probe = rtksw_probe,
	.remove = rtksw_remove,
	.resume = rtksw_resume,
	.suspend = rtksw_suspend,
	.driver = {
		.name	 = DRIVER_NAME,
		.of_match_table = rtksw_of_mtable,
	},
};

module_platform_driver(rtksw_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Realtek switch driver");

