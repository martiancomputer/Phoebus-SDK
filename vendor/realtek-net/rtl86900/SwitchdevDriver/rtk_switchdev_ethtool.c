#include <linux/ethtool.h>
#include <linux/etherdevice.h>
#include <linux/version.h>
#include "rtk_switchdev_ethtool.h"
#ifdef CONFIG_COMMON_RT_API
#include "rt_stat.h"
#include "rt_stat_ext.h"
#include "rt_gpon.h"
#include <common/error.h>
#endif
#include "rtk/port.h"

static void rtksw_ethtool_update_stats(struct net_device *dev)
{
#ifdef CONFIG_COMMON_RT_API
	rt_stat_port_cntr_t pPortCntrs;
#endif
	ca_eth_private_t *cep = netdev_priv(dev);
	unsigned long flags;

#ifdef CONFIG_COMMON_RT_API
	if (rt_stat_port_getAll(cep->port_cfg.tx_ldpid, &pPortCntrs) != RT_ERR_OK)
	{
		return;
	}
#endif

	spin_lock_irqsave(&cep->stats_lock, flags);

#ifdef CONFIG_COMMON_RT_API
	cep->ni_stats.rxbytecount_lo = pPortCntrs.ifInOctets;
	cep->ni_stats.rxinvalidfrmcnt = pPortCntrs.ifInDiscards;
	cep->ni_stats.rxucpktcnt = pPortCntrs.ifInUcastPkts;
	cep->ni_stats.rxmcfrmcnt = pPortCntrs.ifInMulticastPkts;
	cep->ni_stats.rxbcfrmcnt = pPortCntrs.ifInBroadcastPkts;
	cep->ni_stats.txbytecount_lo = pPortCntrs.ifOutOctets;
	cep->ni_stats.txucpktcnt = pPortCntrs.ifOutUcastPkts;
	cep->ni_stats.txmcfrmcnt = pPortCntrs.ifOutMulticastPkts;
	cep->ni_stats.txbcfrmcnt = pPortCntrs.ifOutBrocastPkts; 
	cep->ni_stats.txpausefrmcnt = pPortCntrs.dot3OutPauseFrames;
	cep->ni_stats.rxpausefrmcnt = pPortCntrs.dot3InPauseFrames;
	cep->ni_stats.rxcrcerrfrmcnt = pPortCntrs.dot3StatsFCSErrors;
	cep->ni_stats.rxstatsfrm64oct = pPortCntrs.etherStatsRxPkts64Octets;
	cep->ni_stats.rxstatsfrm65to127oct = pPortCntrs.etherStatsRxPkts65to127Octets;
	cep->ni_stats.rxstatsfrm128to255oct = pPortCntrs.etherStatsRxPkts128to255Octets;
	cep->ni_stats.rxstatsfrm256to511oct = pPortCntrs.etherStatsRxPkts256to511Octets;
	cep->ni_stats.rxstatsfrm512to1023oct = pPortCntrs.etherStatsRxPkts512to1023Octets;
	cep->ni_stats.rxstatsfrm1024to1518oct = pPortCntrs.etherStatsRxPkts1024to1518Octets;
	cep->ni_stats.rxstatsfrm1519to2100oct = pPortCntrs.etherStatsRxPkts1519toMaxOctets;
	cep->ni_stats.rxstatsfrm2101to9200oct = pPortCntrs.etherStatsRxPkts1519toMaxOctets;
	cep->ni_stats.rxstatsfrm9201tomaxoct = pPortCntrs.etherStatsRxPkts1519toMaxOctets;
	cep->ni_stats.rxovsizefrmcnt = pPortCntrs.etherStatsRxOversizePkts;
	cep->ni_stats.rxjabberfrmcnt = pPortCntrs.etherStatsJabbers;
	cep->ni_stats.txstatsfrm64oct = pPortCntrs.etherStatsTxPkts64Octets;
	cep->ni_stats.txstatsfrm65to127oct = pPortCntrs.etherStatsTxPkts65to127Octets;
	cep->ni_stats.txstatsfrm128to255oct = pPortCntrs.etherStatsTxPkts128to255Octets;
	cep->ni_stats.txstatsfrm256to511oct = pPortCntrs.etherStatsTxPkts256to511Octets;
	cep->ni_stats.txstatsfrm512to1023oct = pPortCntrs.etherStatsTxPkts512to1023Octets;
	cep->ni_stats.txstatsfrm1024to1518oct = pPortCntrs.etherStatsTxPkts1024to1518Octets;
	cep->ni_stats.txstatsfrm1519to2100oct = pPortCntrs.etherStatsTxPkts1519toMaxOctets;
	cep->ni_stats.txstatsfrm2101to9200oct = pPortCntrs.etherStatsTxPkts1519toMaxOctets;
	cep->ni_stats.txstatsfrm9201tomaxoct = pPortCntrs.etherStatsTxPkts1519toMaxOctets;
	cep->ni_stats.txovsizefrmcnt = pPortCntrs.etherStatsTxOversizePkts;
#endif

	spin_unlock_irqrestore(&cep->stats_lock, flags);
}

static void rtksw_ethtool_get_drvinfo(struct net_device *dev, struct ethtool_drvinfo *drvinfo)
{
	const struct ethtool_ops *ethtool_ops;
	ethtool_ops = RTK_SWITCHDEV_GMAC_ROOT_NETDEV->ethtool_ops;

	if (ethtool_ops->get_drvinfo)
		return ethtool_ops->get_drvinfo(dev, drvinfo);

	return;
}

static int rtksw_ethtool_get_regs_len(struct net_device *dev)
{
	const struct ethtool_ops *ethtool_ops;
	ethtool_ops = RTK_SWITCHDEV_GMAC_ROOT_NETDEV->ethtool_ops;

	if (ethtool_ops->get_regs_len)
		return ethtool_ops->get_regs_len(dev);

	return 0;
}

static void rtksw_ethtool_get_regs(struct net_device *dev, struct ethtool_regs *regs, void *_p)
{
	const struct ethtool_ops *ethtool_ops;
	ethtool_ops = RTK_SWITCHDEV_GMAC_ROOT_NETDEV->ethtool_ops;

	if (ethtool_ops->get_regs)
		return ethtool_ops->get_regs(dev, regs, _p);

	return;
}

static void rtksw_ethtool_get_wol(struct net_device *dev, struct ethtool_wolinfo *wol)
{
	const struct ethtool_ops *ethtool_ops;
	ethtool_ops = RTK_SWITCHDEV_GMAC_ROOT_NETDEV->ethtool_ops;

	if (ethtool_ops->get_wol)
		return ethtool_ops->get_wol(dev, wol);

	return;
}

static int rtksw_ethtool_set_wol(struct net_device *dev, struct ethtool_wolinfo *wol)
{
	const struct ethtool_ops *ethtool_ops;
	ethtool_ops = RTK_SWITCHDEV_GMAC_ROOT_NETDEV->ethtool_ops;

	if (ethtool_ops->set_wol)
		return ethtool_ops->set_wol(dev, wol);

	return -EOPNOTSUPP;
}

static u32 rtksw_ethtool_get_msglevel(struct net_device *dev)
{
	struct rtksw_priv *priv = netdev_priv(dev);

	return priv->msg_enable;
}

static void rtksw_ethtool_set_msglevel(struct net_device *dev, u32 data)
{
	struct rtksw_priv *priv = netdev_priv(dev);

	priv->msg_enable = data;
	return;
}

static void rtksw_ethtool_get_pauseparam(struct net_device *dev, struct ethtool_pauseparam *pause)
{
	const struct ethtool_ops *ethtool_ops;
	ethtool_ops = RTK_SWITCHDEV_GMAC_ROOT_NETDEV->ethtool_ops;

	if (ethtool_ops->get_pauseparam)
		return ethtool_ops->get_pauseparam(dev, pause);

	return;
}

static int rtksw_ethtool_set_pauseparam(struct net_device *dev, struct ethtool_pauseparam *pause)
{
	const struct ethtool_ops *ethtool_ops;
	ethtool_ops = RTK_SWITCHDEV_GMAC_ROOT_NETDEV->ethtool_ops;

	if (ethtool_ops->set_pauseparam)
		return ethtool_ops->set_pauseparam(dev, pause);

	return 0;
}

static void rtksw_ethtool_get_strings(struct net_device *dev, u32 stringset, u8 *data)
{
	switch(stringset) {
	case ETH_SS_STATS:
		memcpy(data, &rtksw_ethtool_stats_keys, sizeof(rtksw_ethtool_stats_keys));
		break;
	}
}

static void rtksw_ethtool_get_ethtool_stats(struct net_device *dev, struct ethtool_stats *stats, u64 *data)
{
	ca_eth_private_t *cep = netdev_priv(dev);

	rtksw_ethtool_update_stats(dev);

	memcpy(data, &cep->ni_stats, sizeof(cep->ni_stats));
}

static int rtksw_ethtool_get_sset_count(struct net_device *dev, int sset)
{
	switch (sset) {
	case ETH_SS_STATS:
		return RTKSW_NUM_STATS;
	}

	return -EOPNOTSUPP;
}

static const struct ethtool_ops rtksw_ethtool_ops = {
	.get_link_ksettings  = phy_ethtool_get_link_ksettings,
	.set_link_ksettings  = phy_ethtool_set_link_ksettings,
	.nway_reset		= phy_ethtool_nway_reset,

	.get_drvinfo		= rtksw_ethtool_get_drvinfo,
	.get_regs_len		= rtksw_ethtool_get_regs_len,
	.get_regs		= rtksw_ethtool_get_regs,
	.get_wol		= rtksw_ethtool_get_wol,
	.set_wol		= rtksw_ethtool_set_wol,
	.get_msglevel		= rtksw_ethtool_get_msglevel,
	.set_msglevel		= rtksw_ethtool_set_msglevel,
	.get_link		= ethtool_op_get_link,

	.get_pauseparam		= rtksw_ethtool_get_pauseparam,
	.set_pauseparam		= rtksw_ethtool_set_pauseparam,
	.get_strings		= rtksw_ethtool_get_strings,
	.get_ethtool_stats	= rtksw_ethtool_get_ethtool_stats,
	.get_sset_count		= rtksw_ethtool_get_sset_count,
};

void rtksw_set_ethtool_ops(struct net_device *dev)
{
	dev->ethtool_ops = &rtksw_ethtool_ops;
}

