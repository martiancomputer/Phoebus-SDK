#ifndef _RTK_SWITCHDEV_GMAC_H_
#define _RTK_SWITCHDEV_GMAC_H_
#include "ca_ni.h"

#define RTK_SWITCHDEV_GMAC_ROOT_NETDEV	ni_info_data.dev[0]

struct rtksw_priv {
	ca_eth_private_t			cep;
	struct net_device			*ndev;
	struct device 				*dev;
	u32 						msg_enable;
	u8 							mac_addr[ETH_ALEN];
	bool 						rx_pause;
	bool 						tx_pause;
	u32 						emac_port;
	struct rtksw_common 		*rtksw;
	int 						offload_fwd_mark;
};

int ca_ni_dev_port_mapping(int port_num, char *name);
#if defined(CONFIG_CA_NIC_PHYDEV_SUPPORT)
void __ca_ni_adjust_link(struct net_device *dev);
#endif

int rtksw_gmac_sds_polarity_set(struct rtksw_priv *priv, int txPnInverse, int rxPnInverse);
void rtksw_gmac_adjust_link(struct net_device *dev);
int rtksw_gmac_dev_port_mapping(int port_num, char *name);
int rtksw_gmac_priv_init(struct rtksw_priv *priv, u32 port_id);
int rtksw_gmac_dev_init(struct net_device *dev);

#endif /* _RTK_SWITCHDEV_GMAC_H_ */