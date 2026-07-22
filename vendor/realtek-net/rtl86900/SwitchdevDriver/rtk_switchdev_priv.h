#ifndef _RTK_SWITCHDEV_PRIV_H_
#define _RTK_SWITCHDEV_PRIV_H_
#include "rtk_switchdev_gmac.h"

#define RTKSW_DEBUG	(NETIF_MSG_HW		| NETIF_MSG_WOL		| \
			 NETIF_MSG_DRV		| NETIF_MSG_LINK	| \
			 NETIF_MSG_IFUP		| NETIF_MSG_INTR	| \
			 NETIF_MSG_PROBE	| NETIF_MSG_TIMER	| \
			 NETIF_MSG_IFDOWN	| NETIF_MSG_RX_ERR	| \
			 NETIF_MSG_TX_ERR	| NETIF_MSG_TX_DONE	| \
			 NETIF_MSG_PKTDATA	| NETIF_MSG_TX_QUEUED	| \
			 NETIF_MSG_RX_STATUS)

#define RTKSW_MAX_SLAVE_PORTS_NUM	16

#define RTKSW_MAX_QUEUES		8

#define RTKSW_MIN_PACKET_SIZE	(ETH_ZLEN)

struct rtksw_slave_data {
	u32 						port_id;
	char 						netdev_name[IFNAMSIZ];
	struct device_node 			*slave_node;
	u8							mac_addr[ETH_ALEN];
	bool						disabled;
};

struct rtksw_platform_data {
	struct rtksw_slave_data		*slave_data;
	u32							slaves;		/* number of slave rtkgmac ports */
	u32							active_slave;/* time stamping, ethtool and SIOCGMIIPHY slave */
	u32							bd_ram_size;	/*buffer descriptor ram size */
	u32							mac_control;	/* Mac control register */
	u16							default_vlan;	/* Def VLAN for ALE lookup in VLAN aware mode*/
	bool						dual_emac;	/* Enable Dual EMAC mode */
};

struct rtksw_slave {
	void __iomem				*regs;
	int							slave_num;
	u32							mac_control;
	struct rtksw_slave_data		*data;
	struct device_node			*node;
	struct net_device			*ndev;
	u32							port_vlan;
};

struct rtksw_common {
	struct device				*dev;
	struct rtksw_platform_data	data;
	struct rtksw_slave			*slaves;
	struct devlink 				*devlink;
	struct net_device 			*hw_bridge_dev;
	u8 							base_mac[ETH_ALEN];
};

#define ndev_to_rtksw(ndev) (((struct rtksw_priv *)netdev_priv(ndev))->rtksw)

#endif /* _RTK_SWITCHDEV_PRIV_H_ */