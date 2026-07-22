#include "rtk_switchdev_gmac.h"

extern ni_info_t ni_info_data;

int rtksw_gmac_sds_polarity_set(struct rtksw_priv *priv, int txPnInverse, int rxPnInverse)
{
	return 0;
}

void rtksw_gmac_adjust_link(struct net_device *dev)
{
	return;
}

int rtksw_gmac_dev_port_mapping(int port_num, char *name)
{
	return ca_ni_dev_port_mapping(port_num, name);
}

int rtksw_gmac_priv_init(struct rtksw_priv *priv, u32 port_id)
{
	priv->cep.dev = RTK_SWITCHDEV_GMAC_ROOT_NETDEV;
	priv->cep.port_cfg.tx_ldpid = port_id;
	return 0;
}

int rtksw_gmac_dev_init(struct net_device *dev)
{
	struct net_device *rootDev = RTK_SWITCHDEV_GMAC_ROOT_NETDEV;

	dev->features = rootDev->features;
	dev->hw_features = rootDev->hw_features;
	dev->vlan_features = rootDev->vlan_features;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0)
	dev->min_mtu = rootDev->min_mtu;
	dev->max_mtu = rootDev->max_mtu;
#endif
	dev->watchdog_timeo = rootDev->watchdog_timeo;
	return 0;
}

