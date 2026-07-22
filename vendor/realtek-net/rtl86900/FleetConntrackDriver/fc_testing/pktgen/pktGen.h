#ifndef PKT_GEN_
#define PKT_GEN_

#include "flagDef.h"
#include "l2pkt.h"
#include "l3pkt.h"
#include "l4pkt.h"
#include <rtl_protocol.h>

#define MAX_PKT_SIZE (64*1024 + 128)
#define MTU_PPPoE	1492
#define MTU_DEFAULT 1500
#define PPPOE_SID_WAN_ISP_1 0x55
#define PPPOE_SID_WAN_ISP_2 0x66
#define PPPOE_SID_WAN_DSLITE 0xaa
#define PPPOE_SID_LAN_DSLITE 0xbb
#define PPPOE_SID_LAN 0xcc

#define _TEST_IPV4_ 1
#define _TEST_UDP_ 0

#define TEST_IPID 0x2379

enum{

	TYPE_MULTICAST = 0,
	TYPE_RMA_CDP,
	TYPE_L2MULTICAST_IPV6,

	TYPE_L2MULTICAST_IPV6_G2W_WITH_PPPOE,
	TYPE_L34MULTICAST_G2W_WITH_PPPOE,
	TYPE_L34MULTICAST_IPV6_W2L_WITH_PPPOE,
	TYPE_L34MULTICAST_IPV6_G2L_WITH_PPPOE,

	TYPE_L34MULTICAST_W2L_WITH_PPPOE,
	TYPE_L34MULTICAST_G2L_WITH_PPPOE,

	TYPE_DUALHDR_PPTP_MULTICAST_ROUTING_G2W,
	TYPE_DUALHDR_PPTP_MULTICAST_IPV6_G2W,
	TYPE_DUALHDR_PPTP_MULTICAST_ROUTING_W2G,
	TYPE_DUALHDR_PPTP_MULTICAST_IPV6_W2G,

	TYPE_DUALHDR_L2TP_MULTICAST_ROUTING_G2W,
	TYPE_DUALHDR_L2TP_MULTICAST_IPV6_G2W,
	TYPE_DUALHDR_L2TP_MULTICAST_ROUTING_W2G,
	TYPE_DUALHDR_L2TP_MULTICAST_IPV6_W2G,

	TYPE_DUALHDR_DSLITE_MULTICAST_ROUTING_G2W,
	TYPE_DUALHDR_DSLITE_MULTICAST_IPV6_G2W,
	TYPE_DUALHDR_DSLITE_MULTICAST_ROUTING_W2G,
	TYPE_DUALHDR_DSLITE_MULTICAST_IPV6_W2G,


	TYPE_MULTICAST_IGMP,
	TYPE_MULTICAST_MLD,
	TYPE_MULTICAST_ICMP6,
	TYPE_MULTICAST_DHCP6,
	TYPE_RESERVED_MULTICAST_V4,
	TYPE_RESERVED_MULTICAST_V6,
	TYPE_IGMP_RESERVED_MULTICAST,
	TYPE_MLD_RESERVED_MULTICAST,
	TYPE_L34MULTICAST_L2W,
	TYPE_L34MULTICAST_G2W_L3,
	TYPE_L34MULTICAST_G2W_L4,
	TYPE_L34MULTICAST_L2W_WITH_PPPOE,
	TYPE_L34MULTICAST_W2L,
	TYPE_L34MULTICAST_G2L,
	
	TYPE_L2UNICAST = 100,				// tcp bridge
	TYPE_L2UNICAST1,
	TYPE_L2UNICAST_TO_EXTPORT,
	TYPE_L34UNICAST_L2W,
	TYPE_L34UNICAST_G2W,
	TYPE_L34UNICAST_W2G,
	TYPE_L34UNICAST_G2L,
	TYPE_L2UNICAST_Ethernet,
	TYPE_L2UNICAST_TCP,
	TYPE_L2UNICAST_UDP,

	TYPE_L3UNICAST_ROUTING_W2L,
	TYPE_L3UNICAST_ROUTING_G2L,
	TYPE_L3UNICAST_ROUTING_G2W,
	TYPE_L3UNICAST_IPV6_L2W,
	TYPE_L3UNICAST_IPV6_G2W,
	TYPE_L3UNICAST_IPV6_W2L,
	TYPE_L3UNICAST_IPV6_G2L,
	TYPE_L3UNICAST_IPV6_G2L_FRAG,

	
	TYPE_DUALHDR = 150,
	TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W,
	TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W,
	TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W,
	TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_UDP,
	TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_UDP,
	TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_UDP,
	TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_ICMP,
	TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_ICMP,
	TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_ICMP,
	
	TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L,
	TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G,
	TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L,
	TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_UDP,
	TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_UDP,
	TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_UDP,
	TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_ICMP,
	TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_ICMP,
	TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_ICMP,
	TYPE_DUALHDR_PPTPV6,

	TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W,
	TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W,
	TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W,
	TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_UDP,
	TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_UDP,
	TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_UDP,
	TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_ICMP,
	TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_ICMP,
	TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_ICMP,

	TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L,
	TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G,
	TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L,
	TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_UDP,
	TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_UDP,
	TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_UDP,
	TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_ICMP,
	TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_ICMP,
	TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_ICMP,

	//frag:mf
	TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_FRAG,
	TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_FRAG,
	TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_FRAG,
	TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_UDP_FRAG,
	TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_UDP_FRAG,
	TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_UDP_FRAG,
	TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_ICMP_FRAG,
	TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_ICMP_FRAG,
	TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_ICMP_FRAG,
	
	TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_FRAG,
	TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_FRAG,
	TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_FRAG,
	TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_UDP_FRAG,
	TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_UDP_FRAG,
	TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_UDP_FRAG,
	TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_ICMP_FRAG,
	TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_ICMP_FRAG,
	TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_ICMP_FRAG,
	TYPE_DUALHDR_PPTPV6_FRAG,

	TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_FRAG,
	TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_FRAG,
	TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_FRAG,
	TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_UDP_FRAG,
	TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_UDP_FRAG,
	TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_UDP_FRAG,
	TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_ICMP_FRAG,
	TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_ICMP_FRAG,
	TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_ICMP_FRAG,

	TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_FRAG,
	TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_FRAG,
	TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_FRAG,
	TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_UDP_FRAG,
	TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_UDP_FRAG,
	TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_UDP_FRAG,
	TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_ICMP_FRAG,
	TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_ICMP_FRAG,
	TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_ICMP_FRAG,

	//frag:frag_offset
	TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_FRAG2,
	TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_FRAG2,
	TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_FRAG2,
	TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_UDP_FRAG2,
	TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_UDP_FRAG2,
	TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_UDP_FRAG2,
	TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_ICMP_FRAG2,
	TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_ICMP_FRAG2,
	TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_ICMP_FRAG2,
	TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_ICMPPOD_FRAG2,
	TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_ICMPPOD_FRAG2,
	TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_ICMPPOD_FRAG2,
	
	TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_FRAG2,
	TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_FRAG2,
	TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_FRAG2,
	TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_UDP_FRAG2,
	TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_UDP_FRAG2,
	TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_UDP_FRAG2,
	TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_ICMP_FRAG2,
	TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_ICMP_FRAG2,
	TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_ICMP_FRAG2,
	TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_ICMPPOD_FRAG2,
	TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_ICMPPOD_FRAG2,
	TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_ICMPPOD_FRAG2,
	TYPE_DUALHDR_PPTPV6_FRAG2,

	TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_FRAG2,
	TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_FRAG2,
	TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_FRAG2,
	TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_UDP_FRAG2,
	TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_UDP_FRAG2,
	TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_UDP_FRAG2,
	TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_ICMP_FRAG2,
	TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_ICMP_FRAG2,
	TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_ICMP_FRAG2,
	TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_ICMPPOD_FRAG2,
	TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_ICMPPOD_FRAG2,
	TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_ICMPPOD_FRAG2,

	TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_FRAG2,
	TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_FRAG2,
	TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_FRAG2,
	TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_UDP_FRAG2,
	TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_UDP_FRAG2,
	TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_UDP_FRAG2,
	TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_ICMP_FRAG2,
	TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_ICMP_FRAG2,
	TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_ICMP_FRAG2,
	TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_ICMPPOD_FRAG2,
	TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_ICMPPOD_FRAG2,
	TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_ICMPPOD_FRAG2,

	//frag:mf+frag_offset
	TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_FRAG3,
	TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_FRAG3,
	TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_FRAG3,
	TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_UDP_FRAG3,
	TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_UDP_FRAG3,
	TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_UDP_FRAG3,
	TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_ICMP_FRAG3,
	TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_ICMP_FRAG3,
	TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_ICMP_FRAG3,
	TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_ICMPPOD_FRAG3,
	TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_ICMPPOD_FRAG3,
	TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_ICMPPOD_FRAG3,
	
	TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_FRAG3,
	TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_FRAG3,
	TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_FRAG3,
	TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_UDP_FRAG3,
	TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_UDP_FRAG3,
	TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_UDP_FRAG3,
	TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_ICMP_FRAG3,
	TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_ICMP_FRAG3,
	TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_ICMP_FRAG3,
	TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_ICMPPOD_FRAG3,
	TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_ICMPPOD_FRAG3,
	TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_ICMPPOD_FRAG3,
	TYPE_DUALHDR_PPTPV6_FRAG3,

	TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_FRAG3,
	TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_FRAG3,
	TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_FRAG3,
	TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_UDP_FRAG3,
	TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_UDP_FRAG3,
	TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_UDP_FRAG3,
	TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_ICMP_FRAG3,
	TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_ICMP_FRAG3,
	TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_ICMP_FRAG3,
	TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_ICMPPOD_FRAG3,
	TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_ICMPPOD_FRAG3,
	TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_ICMPPOD_FRAG3,

	TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_FRAG3,
	TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_FRAG3,
	TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_FRAG3,
	TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_UDP_FRAG3,
	TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_UDP_FRAG3,
	TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_UDP_FRAG3,
	TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_ICMP_FRAG3,
	TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_ICMP_FRAG3,
	TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_ICMP_FRAG3,
	TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_ICMPPOD_FRAG3,
	TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_ICMPPOD_FRAG3,
	TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_ICMPPOD_FRAG3,

	TYPE_DUALHDR_L2TP_UNICAST_ROUTING_G2W,
	TYPE_DUALHDR_L2TP_UNICAST_NAT_G2W,
	TYPE_DUALHDR_L2TP_UNICAST_IPV6_G2W,
	
	TYPE_DUALHDR_L2TP_UNICAST_ROUTING_W2L,
	TYPE_DUALHDR_L2TP_UNICAST_NAT_W2G,
	TYPE_DUALHDR_L2TP_UNICAST_IPV6_W2L,
	TYPE_DUALHDR_L2TPV6,

	TYPE_DUALHDR_DSLITE_UNICAST_IPV6_OUTER_G2W,
	TYPE_DUALHDR_DSLITE_UNICAST_IPV6_OUTER_W2G,
	TYPE_DUALHDR_DSLITE_MULTICAST_IPV6_OUTER_G2W,
	TYPE_DUALHDR_DSLITE_MULTICAST_IPV6_OUTER_W2G,

	TYPE_DUALHDR_DSLITE_UNICAST_ROUTING_G2W,
	TYPE_DUALHDR_DSLITE_UNICAST_NAT_G2W,
	
	TYPE_DUALHDR_DSLITE_UNICAST_ROUTING_W2L,
	TYPE_DUALHDR_DSLITE_UNICAST_NAT_W2G,

	TYPE_BROADCAST = 350,
	TYPE_BROADCAST_ARP,
	TYPE_BROADCAST_WAN,
	
	TYPE_FROM_CPU  = (1<<8),
	TYPE_FROM_EXT0 = (2<<8),
	TYPE_FROM_EXT1 = (3<<8),
	TYPE_FROM_EXT2 = (4<<8),
	TYPE_FROM_EXT3 = (5<<8),
	TYPE_FROM_EXT4 = (6<<8),
#ifdef CONFIG_RTL9602C_SERIES
	TYPE_FROM_EXT5 = (7<<8),
#endif
};

/* Interface gateway mac */
#define INTF0_MAC "00-00-00-11-11-00"
#define INTF1_MAC "00-00-00-11-11-01"
#define INTF2_MAC "00-00-00-11-11-02"
#define INTF3_MAC "00-00-00-11-11-03"
#define INTF4_MAC "00-00-00-11-11-04"
#define INTF5_MAC "00-00-00-11-11-05"
#define INTF6_MAC "00-00-00-11-11-06"
#define INTF7_MAC "00-00-00-11-11-07"
#define INTF8_MAC "00-00-00-11-11-08"
#define INTF9_MAC "00-00-00-11-11-09"
#define INTF10_MAC "00-00-00-11-11-0A"
#define INTF11_MAC "00-00-00-11-11-0B"
#define INTF12_MAC "00-00-00-11-11-0C"
#define INTF13_MAC "00-00-00-11-11-0D"
#define INTF14_MAC "00-00-00-11-11-0E"
#define INTF15_MAC "00-00-00-11-11-0F"

#define LAN_GWIPADDR_1 "192.168.1.1"
#define LAN_GWIPADDR_2 "192.168.2.1"

#define WAN_GWIPADDR_1 "114.34.1.1"
#define WAN_GWIPADDR_2 "114.34.2.1"

#define WAN_GWMAC_1 "00-E0-4C-55-66-71"
#define WAN_GWMAC_2 "00-E0-4C-55-66-72"

#define MACINDACCESSIDX_1 111
#define MACINDACCESSIDX_2 122
#define MACINDACCESSIDX_3 133

#define MACINDIDX_1_lan_host_mac 11			//0xb
#define MACINDIDX_2_lan_host_mac2 22			//0x16
#define MACINDIDX_2_lan_host_mac3 23			//0x16
#define MACINDIDX_3_wan_nexthop_mac 33		//0x21
#define MACINDIDX_4_mc_dmac 44				//0x2c
#define MACINDIDX_5_v6mc_dmac 55				//0x37
#define MACINDIDX_6_wan_gmac 66				//0x42
#define MACINDIDX_7_wan_gmac_2 77				//0x4d
#define MACINDIDX_8_lan_gmac 88				//0x58

#define LUTIDX_1 0x11								// lan_host_mac1
#define LUTIDX_2 0x22								// lan_host_mac2
#define LUTIDX_3 0x33								// wan_nexthop_mac
#define LUTIDX_4 0x44								// mc_dmac
#define LUTIDX_5 0x55								// v6mc_dmac
#define LUTIDX_6 0x66								// wan_gmac
#define LUTIDX_7 0x77								// wan_gmac_2
#define LUTIDX_8 0x88								// lan_gmac

#define lan_host_ip1 "192.168.1.100"
#define lan_host_mac1 "00-01-02-03-10-01" //lan host (192.168.1.100)

#define lan_host_ip11 "192.168.1.200"
//#define lan_host_mac11 "00-01-02-03-20-01" //lan host (192.168.1.200)
#define lan_host_mac11 "00-00-00-00-45-45" //lan host (192.168.1.200) for FPGA 4+8+1=>0d:0d,; Model/MP 4+64+1=>45:45

#define lan_host_ip2 "192.168.1.101"
#define lan_host_mac2 "00-01-02-03-10-02"//lan host (192.168.1.101)

#define lan_host_ip3 "192.168.1.103"
#define lan_host_mac3 "00-01-02-03-10-03"//lan host (192.168.1.101)

#define lan_host_ip4 "192.168.1.104"
#define lan_host_mac4 "00-01-02-03-10-04"//lan host (192.168.1.101)


#define lan_gip "192.168.1.254" 
#define lan_gmac "00-01-02-03-10-ff"  //lan netif gmac

#define wan_gip "172.10.0.1" 
#define wan_gmac "00-01-02-03-11-ff"  //wan netif gmac (172.10.0.1)

#define wan_remote_host "10.0.0.1"  //remote host
#define wan_nexthop_mac "00-11-22-33-44-55"   // nh for remote host 
#define wan_remote_host2 "10.0.0.2"  //remote host
#define wan_nexthop_mac2 "00-11-22-33-44-56"   // nh for remote host 

#define wan_gip_2 "172.10.0.2" 
#define wan_gmac_2 "00-01-02-03-11-fe" 

#define mc_dipv6dmac "33:33:e0:00:0a:01" 		// mapping from mc_dip2v6dip "ff05:dbdb::e000:0a01"
#define mc_dip2v6dip "ff05:dbdb::e000:0a01" 	// mapping from mc_dip "224.0.10.1"

#define mc_dip "224.0.10.1" 
#define mc_dmac "01-00-5e-00-0a-01" 
#define mc_dip_2 "224.0.10.2" 
#define mc_dmac_2 "01-00-5e-00-0a-03" 
#define mc_dip_3 "224.0.10.3" 
#define mc_dmac_3 "01-00-5e-00-0a-03" 

#define rsvd_mc_dip "224.0.0.1" 
#define rsvd_mc_dmac "01-00-5e-00-00-01" 
#define rsvd2_mc_dmac "01-80-c2-00-00-2a" 

#define cdp_dip "224.2.3.4" 
#define cdp_dmac "01:00:0C:CC:CC:CC" 
#define v6mc_dip "ff1e::1234:5678" 
#define v6mc_dmac "33:33:12:34:56:78" 
#define v6mld_dip "FF38::276" 
#define v6mld_sip "fe80::209:5bff:fe08:a674" 
#define v6mld_dmac "33:33:00:00:02:76" 
#define v6_rsvd_mc_dip "ff01::db8:0:1" 
#define v6_rsvd_mc_dmac "33-33-00-00-00-01" 

#define v6_lan_host_ip "2001:1010:2020:3030:1111:2222:3333:abcd" 
#define v6_lan_host_ip2 "2001:1010:2020:3030:1111:2222:4444:1234" 
#define v6_lan_host_ip3 "2001:1010:2020:3030:1111:2222:3333:1234" 
#define v6_wan_gip "2011:dbdb::1319:ffab:ffcd:0001" 
#define v6_wan_remote_host "2011:dbdb::3333:3333:3333:6666" 
#define v6_wan_remote_host3 "2011:dbdb:1111:2222:3333:3333:3333:6666" 
#define v6_common_src_ip "2001::01" 
#define v6_mapt_fmr_prefix "2345:9023:9052:9387::" 
#define v6_mapt_dmr_prefix "2011:dbdb:5423:2127:0039:8529::" 
#define v6_xlat_prefix "2011:dbdb:5423:2127:0039:8529::" 

#define ipsec_onu_mac "00-00-00-cc-15-55"
#define ipsec_remote_mac "00-00-00-cc-16-66"
#define ipsec_onu_ip "88.88.15.55"
#define ipsec_remote_ip "99.99.16.66"
#define ipsec_onu_ipv6 "2001:7777:7777:7777:7777:7777:7777:1555"
#define ipsec_remote_ipv6 "2001:7777:7777:7777:7777:7777:7777:1666"
#define special_onu_mac "00-00-00-00-00-00" 


#define l2tp_onu_mac "00-00-00-00-15-55" 
#define l2tp_remote_mac "00-00-00-00-16-66" 
#define l2tp_onu_ip "55.55.15.55" 
#define l2tp_remote_ip "66.66.16.66" 
#define l2tp_onu_ipv6 "2001:7777:7777:7777:7777:7777:7777:1555" 
#define l2tp_remote_ipv6 "2001:7777:7777:7777:7777:7777:7777:1666" 

#define l2tpv3_onu_mac "00-00-00-00-55-55" 
#define l2tpv3_remote_mac "00-00-00-00-66-66" 
#define l2tpv3_onu_ip "33.33.55.55" 
#define l2tpv3_remote_ip "44.44.66.66" 
#define l2tpv3_onu_ipv6 "2001:7777:7777:7777:7777:7777:7777:5555" 
#define l2tpv3_remote_ipv6 "2001:7777:7777:7777:7777:7777:7777:6666" 

#define ipgre_onu_ip "55.55.55.55" 
#define ipgre_remote_ip "66.66.66.66" 
#define ipgre_onu_ipv6 "2001:7777:7777:7777:7777:7777:7777:5555" 
#define ipgre_remote_ipv6 "2001:7777:7777:7777:7777:7777:7777:6666" 

#define vxlan_onu_mac "00-00-00-00-25-55" 
#define vxlan_remote_mac "00-00-00-00-26-66" 
#define vxlan_onu_ip "55.55.25.55" 
#define vxlan_remote_ip "66.66.26.66" 
#define vxlan_onu_ipv6 "2001:7777:7777:7777:7777:7777:7777:2555" 
#define vxlan_remote_ipv6 "2001:7777:7777:7777:7777:7777:7777:2666" 

#define l2gre_onu_mac "00-00-00-00-55-55" 
#define l2gre_remote_mac "00-00-00-00-66-66" 
#define l2gre_onu_ip "55.55.55.55" 
#define l2gre_remote_ip "66.66.66.66" 
#define l2gre_onu_ipv6 "2001:7777:7777:7777:7777:7777:7777:5555" 
#define l2gre_remote_ipv6 "2001:7777:7777:7777:7777:7777:7777:6666" 


#define PPTP_SERVER_IP "10.123.20.2"
#define PPTP_CLIENT_IP "10.123.20.5"
#define PPTPV6_SERVER_IP "2099:1654:2145:acfd::abbd"
#define PPTPV6_CLIENT_IP "2099:1654:2145:acfd::1334"
#define PPTP_CLIENT_MAC "00-01-02-03-11-fe" 	//wan_gmac_2
//#define PPTP_REMOTE_HOST_IP WAN0_HOST1_IP
//#define PPTP_REMOTE_HOST_MAC "00-08-08-08-08-01"
#define pptp_CALL_ID  0xAABB
#define pptp_CALL_ID2  0xCCDD
#define pptp_SEQ_NO  0xCCDDEEFF
#define pptp_ACK_NO  0x87654321
#define PPTP_session_port 0x0670

#define gre_key_no 0x87654321
#define gre_seq_no 0xCCDDEEFF
#define L2TP_udpsrc_port 1701	// reg.L2TP_UDP_SPORT = 1701
#define L2TP_session_port 0x0780

#define L2TP_SERVER_IP "10.123.33.2"
#define L2TP_CLIENT_IP "10.123.33.150"
#define L2TP_CLIENT_MAC "00-01-02-03-11-fe" 	//wan_gmac_2

#define bc_dip "192.168.1.255"
#define bc_dmac "ff-ff-ff-ff-ff-ff"

#define int_port 0x1234
#define ext_port 0x4321
#define REMOTE_PORT 0x80

typedef enum pptp_outerhdr_option_e
{
	PPTP_WITHSEQ_WITHACK = 0,
	PPTP_WITHSEQ_WOACK,
	PPTP_WOSEQ_WITHACK,
	PPTP_WOSEQ_WOACK,
}pptp_outerhdr_option_t;

typedef enum l2tp_outerhdr_option_e
{
	L2TP_WITHLEN_FIELD,
	L2TP_WOLEN_FIELD,
}l2tp_outerhdr_option_t;

typedef enum compressed_ppp_mode_e{
	COMPRESSED_PPP_FF0300XX = 0,
	COMPRESSED_PPP_FF03XX,
	COMPRESSED_PPP_00XX,
	COMPRESSED_PPP_XX,
	COMPRESSED_PPP_END,
	COMPRESSED_PPP_FF03C021,	//LCP
}compressed_ppp_mode_t;

typedef enum igmpType_e{
	IGMPv2_QUERY=0,
	IGMPv2_REPORT,
	IGMPv2_LEAVE,


}igmpType_t;


typedef struct rtl8651_IgmpGrConf_s {
	uint8	type;
	uint8	auxlen;
	uint16	nofs;
	uint32	gaddr;
	uint32 *	slist;
	uint8 *	auxdata;
} rtl8651_IgmpGrConf_t;

#define _PKT_STOP		0	// end of testcase, packet generator stop
#define _PKT_TYPE_ETHER		1
#define _PKT_TYPE_IPX			2
#define _PKT_TYPE_ARP			3
#define _PKT_TYPE_IP			4
#define _PKT_TYPE_PPTP			5
#define _PKT_TYPE_ICMP			6
#define _PKT_TYPE_IGMP			7
#define _PKT_TYPE_TCP			8
#define _PKT_TYPE_UDP			9
#define _PKT_TYPE_UDP_OPTION	10
#define _PKT_TYPE_IPV6                 11        //added by liujuan
#define _PKT_TYPE_V6ICMP            12       //added by liujuan
#define _PKT_TYPE_V6TCP              13       //added by liujuan
#define _PKT_TYPE_V6UDP              14       //added by liujuan
#define _PKT_TYPE_V6HOPBYHOP         15       //added by luke
#define _PKT_TYPE_V6GRE         16       //added by luke
#define _PKT_TYPE_DSLITE        17		 //added by anonymous
#define _PKT_TYPE_MAPE          18		 //added by anonymous
/* DOS packet type*/
#define _PKT_TYPE_TCP_SHORT_HDR			19
#define _PKT_TYPE_V6TCP_SHORT_HDR		20
#define _PKT_TYPE_UDP_BOMB				21
#define _PKT_TYPE_V6UDP_BOMB			22
/*================*/
#define _PKT_TYPE_UDP_LITE				23
#define _PKT_TYPE_V6UDP_LITE			24

#define _PKT_FLAG_INGRESS		1
#define _PKT_FLAG_EXPECTED	2

typedef struct rtl8651_PktConf_s {

	uint32		pktType;	/* Packet Type of packet */
	uint32		l2Flag;		/* l2Flags, defined in flagDef.h and masked using L2_MASK (L2_CVLAN/L2_PPPoE)*/
	uint32		ErrFlag;	/* Let L3 Checksum error with setting this flag, defined in flagDef.h and masked using CTL_MASK */
	uint32		relayType;	/* L2_BCAST L2_MCAST L3_BCAST, for default value setting of dmac,dip and pktHdr */
	uint32		paddingLen;
	uint32            extHdr;         //added by liujuan
	struct {
		uint8		ASIC_pppoeIdx;		/* pppoe index in asic */
		uint8		ASIC_extPortList;	/* dest extension port list. must be 0 for Tx */
		uint8		ASIC_srcExtPortNum;	/*Both in Rx & Tx. Source extension port number. */
		uint16		ASIC_vlanIdx;		/* vlan index in asic, note: it's used to indicate vlan id! */
		uint16		ASIC_portlist;		/* RX: input port, TX: output port */
		uint32		ASIC_reason;		/* reason bits */
		uint16		PKT_category;		/* categorey */
		uint16		PKT_otherFlags;		/* other flags to set :PKTHDR_PPPOE_AUTOADD, PKTHDR_VLAN_AUTOADD, CSUM_IP, CSUM_L4 */

	} pktHdrInfo; 
	// for pktHdr setting
	#define		conf_pppoeIdx			pktHdrInfo.ASIC_pppoeIdx
	/*note: conf_vlanIdx is used to indicate vlan id now!*/
	#define		conf_vlanIdx			pktHdrInfo.ASIC_vlanIdx
	#define		conf_extPortList		pktHdrInfo.ASIC_extPortList
	#define		conf_srcExtPortNum		pktHdrInfo.ASIC_srcExtPortNum
	#define		conf_portlist			pktHdrInfo.ASIC_portlist
	#define		conf_reason			pktHdrInfo.ASIC_reason
	#define		conf_category		pktHdrInfo.PKT_category
	#define		conf_Flags			pktHdrInfo.PKT_otherFlags

	#define		pkt_PKTHDR_PPPOE_AUTOADD	0x0008	// PKTHDR_PPPOE_AUTOADD in mbuf.h
	#define		pkt_PKTHDR_VLAN_AUTOADD		0x0004	// PKTHDR_VLAN_AUTOADD in mbuf.h       

	struct {
		int8*		content;	/* content field in packet */
		uint32		length;		/* content length */
	} payload;

	struct {
		uint8		da[6];		/* Destination mac */
		uint8		sa[6];		/* Destination mac */
		uint16		etherType;	/* ether type */
	} ether_header;
	#define	conf_dmac		ether_header.da
	#define conf_smac		ether_header.sa
	#define conf_ethtype		ether_header.etherType

	struct {
		uint8 protocol;
		uint8 reason;
		uint8 priority;
		uint8 extspa;
		uint8 streamid;
		uint8 l3r;
		uint8 org;
		uint8 extdpmsk;
		uint8 spa;
		uint8 fbi;
		uint32 fbindex;
	} ctag_header;

	struct {
		uint16 txmsk;
		uint8 prisel;
		uint8 pri;
		uint8 keep;
		uint8 dislrn;
		uint8 psel;
		uint8 directTx;
		uint8 extspa;
		uint8 pppoeact;
		uint8 pppoeidx;
		uint8 streamid;
	} ctag_tx2;

	#define conf_ctag_protocol ctag_header.protocol
	#define conf_ctag_reason ctag_header.reason
	#define conf_ctag_priority ctag_header.priority
	#define conf_ctag_extspa ctag_header.extspa
	#define conf_ctag_streamid ctag_header.streamid
	#define conf_ctag_l3r ctag_header.l3r
	#define conf_ctag_org ctag_header.org
	#define conf_ctag_extdpmsk ctag_header.extdpmsk
	#define conf_ctag_spa ctag_header.spa
	#define conf_ctag_fbi ctag_header.fbi
	#define conf_ctag_fb_index ctag_header.fbindex

	struct {
		uint16		tpid;
		uint16		svid;		/* svlan id */
		uint8		dei;		/* svlan DEI bit */
		uint16		sprio;		/* svlan priority field */
	} svlan;

	struct {
		int16		tpid;
		uint16		cvid;		/* cvlan id */
		uint8		cfi;		/* cvlan CFI bit */
		uint16		cprio;		/* cvlan priority field */
	} cvlan;

	struct {
		int16		tpid;
		uint16		cvid;		/* cvlan id */
		uint8		cfi;		/* cvlan CFI bit */
		uint16		cprio;		/* cvlan priority field */
	} othervlan;

	struct {
		uint8		dsap;
		uint8		ssap;
	} llc;

	struct {
		uint16		type;		/* pppoe Type */
		uint16		session;	/* pppoe Session */
	} pppoe;

	struct {
		uint8		type;
		uint32		dnet;
		uint8		dnode[6];
		uint16		dsock;
		uint32		snet;
		uint8		snode[6];
		uint16		ssock;
	} ipx;

	struct {
		int8		mac_src[6];
		uint32		ip_src;
		int8		mac_dst[6];
		uint32		ip_dst;
		uint16		op;		/* options : ARP/ARP_reply/RARP/RARP_reply */
	} arp;

	struct {
		uint8		version;
		uint8		headerLen;
		uint8		tos;
		uint16		id;
		uint8		diffserv;	/* DS and ECN */
		uint8		mf;
		uint16		offset;
		uint8		ttl;
		uint32		src_ip;
		uint32		dst_ip;
		int8		protocol;
	} ip;
	#define conf_sip		ip.src_ip
	#define conf_dip		ip.dst_ip

//added by liujuan
	struct{
		uint8       version;
		uint8       priority;
		uint8       flowlbl[3];
		uint16     pldlen;
		uint8       nxthdr;
		uint8       hoplmt;
		uint8       src_ip[16];
		uint8       dst_ip[16];
	}ipv6;
	#define conf_v6sip       ipv6.src_ip
	#define conf_v6dip       ipv6.dst_ip

	struct{
		uint8     nexthdr;
		uint8     hdrlen;
		uint8     segments_left;
		uint8     dip[16];
	}rhdr;

	struct{
		uint8     nexthdr;
		uint16    fragment;//offset:13 res:2 moreFrag:1
		uint32	  id;
	}frghdr;
	
	struct {
		uint16		callid;
		uint32		seqno;
		uint32		ackno;
	} pptp;

	struct {
		uint8		type;
		uint8		code;
		uint16		id;
		uint16		seq;
	} icmp;

	struct {
		uint8					ver;
		uint8					type;
		uint8					respTime;
		uint32					gaddr;
		// for query
		uint8					qrsq;
		uint8					qqic;
		uint16					qnofs;
		uint32 *					qslist;
		// for report
		uint16					rnofg;
		rtl8651_IgmpGrConf_t *	grCfg;
	} igmp;

	struct {
		uint16		src_port;
		uint16		dst_port;
		struct {
			uint32		seq;
			uint32		ack;
			uint8		offset;
			uint8		tcpFlag;
			uint16		win;
			uint16		mss;
		} tc;
	} l4hdr;
	#define conf_sport		l4hdr.src_port
	#define conf_dport		l4hdr.dst_port
	#define conf_tcp_seq		l4hdr.tc.seq
	#define	conf_tcp_ack		l4hdr.tc.ack
	#define conf_tcp_offset l4hdr.tc.offset
	#define	conf_tcp_flag		l4hdr.tc.tcpFlag
	#define conf_tcp_win		l4hdr.tc.win
	#define conf_tcp_mss		l4hdr.tc.mss
} rtl8651_PktConf_t;

typedef	struct cputag_s
{
	uint32 protocol;
	uint32 res;
	uint32 pri;
	uint32 extspa;
	uint32 streamid;
	uint32 l3r;
	uint32 org;
	uint32 spa;
	uint32 epmsk;
	uint32 fbi;
	uint32 fbindex;
} cputag_t;

typedef struct cputagTx_s{
	uint16 txmsk;
	uint8 prisel;
	uint8 pri;
	uint8 keep;
	uint8 dislrn;
	uint8 psel;
	uint8 directTx;
	uint8 extspa;
	uint8 pppoeact;
	uint8 pppoeidx;
	union {
		uint8 streamid;
		uint8 extpmask;
	};
} cputagTx_t;

void payloadGen(uint8 *pktData, int pktLen);
uint32 pktGen_withCRCAppend(rtl8651_PktConf_t *, int8 *);
uint32 pktGen_withCRCAppend_payloadGen(rtl8651_PktConf_t *Pktptr, int8 *buff);
uint32 pktGen_withCRCAppend_paddingCut(rtl8651_PktConf_t *Pktptr, int8 *buff, uint32 paddingCut);
uint32 pktGen(rtl8651_PktConf_t *, int8 *);
uint32 grGen(rtl8651_IgmpGrConf_t *, uint32, int8 *);
int pptpOuterHdrRemoveOptionalFields(uint8 *pktBuf, int offset, uint8 rmSeqBit, uint8 rmAckBit, int pktlen);
int pptpOuterHdrGen(uint8 *pktBuf, uint8 *sip, uint8 *dip, int pptp_callid, uint8 innerIsIPv4, uint32 outerHdrLen, uint32 outerHdrTos, uint32 outerHdrTTL, uint32 outerHdrFragment, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t cmpPPPMode);
int pptpOuterHdrGen_lcp(uint8 *pktBuf, uint8 *sip, uint8 *dip, int pptp_callid, uint8 innerIsIPv4, uint32 outerHdrLen, uint32 outerHdrTps, uint32 outerHdrTTL, uint32 outerHdrFragment, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t cmpPPPMode, uint8 ppp_lcp);
int pptpOuterV6HdrGen(uint8 *pktBuf, uint8 *sip6, uint8 *dip6, int pptp_callid, uint8 innerIsIPv4, uint32 outerHdrLen, uint32 outerHdrTos, uint32 outerHdrHoplimit, uint32 outerHdrFragment, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t cmpPPPMode);
int pptpOuterV6HdrGen_lcp(uint8 *pktBuf, uint8 *sip6, uint8 *dip6, int pptp_callid, uint8 innerIsIPv4, uint32 outerHdrLen, uint32 outerHdrTos, uint32 outerHdrHoplimit, uint32 outerHdrFragment, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t cmpPPPMode, uint8 ppp_lcp);
int l2tpOuterHdrGen(uint8 *pktBuf, uint8 *sip, uint8 *dip, uint16 length, uint16 tunnelID, uint16 sessionID, uint8 innerIsIPv4, uint32 outerHdrLen, uint8 reduceL2TPLen, compressed_ppp_mode_t cmpPPPMode);

int insertInnerVlanTags(uint8* buf,int *length,uint16 tpid,uint16 vlanid,uint8 vpri,uint8 dei);
int insertTopVlanTags(uint8* buf,int *length,uint16 tpid,uint16 vlanid,uint8 vpri,uint8 dei);


int normalPktCPUTxGen(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,//type: 0:l2_unicast/1:multicast/2:broadcast/3:l34_unicast
	int ctagif,int cvid,int cpri,
	int stagif,int svid,int spri, int dscp,
	cputag_t *cputag, cputagTx_t *cputagTx,
	int pppoetagif,int pppoe_session_id,int ttl);

int normalPktGen_ecn(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,//type: 0:l2_unicast/1:multicast/2:broadcast/3:l34_unicast
	int ctagif,int cvid,int cpri,
	int stagif,int svid,int spri, int dscp, int ecn,
	cputag_t *cputag, cputagTx_t *cputagTx,
	int pppoetagif,int pppoe_session_id,int ttl);

int normalPktGen(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,//type: 0:l2_unicast/1:multicast/2:broadcast/3:l34_unicast
	int ctagif,int cvid,int cpri,
	int stagif,int svid,int spri, int dscp,
	cputag_t *cputag, cputagTx_t *cputagTx,
	int pppoetagif,int pppoe_session_id,int ttl);

int pptpPktGen_gerneral_errFlag(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,uint32 errFlag,uint32 ipver, uint32 iphdl, uint32 ip6ver,
	int ctagif,int cvid,int cpri,
	int stagif,int svid,int spri, int tos, int pptp_tos,
	cputag_t *cputag, uint8 TCPflag, int innerPaddingCut, int origPadding,
	int pppoetagif,int pppoe_session_id,int ttl,int pptp_ttl,int pptp_fragment, int pptp_lcp,
	int pptp_callid,int pptp_seqno,int pptp_ackno,int ipv4_id, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t pptp_cmpPPPMode);

int pptpPktGen_general(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,
	int ctagif,int cvid,int cpri,
	int stagif,int svid,int spri, int tos, int pptp_tos,
	cputag_t *cputag, uint8 TCPflag, int innerPaddingCut, int origPadding,
	int pppoetagif,int pppoe_session_id,int ttl,int pptp_ttl,int pptp_fragment, int pptp_lcp,
	int pptp_callid,int pptp_seqno,int pptp_ackno,int ipv4_id, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t pptp_cmpPPPMode);

int pptpPktGenCutPadding(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,
	int ctagif,int cvid,int cpri,
	int stagif,int svid,int spri, int tos, int pptp_tos,
	cputag_t *cputag, uint8 TCPflag, int innerPaddingCut, int origPadding,
	int pppoetagif,int pppoe_session_id,int ttl,int pptp_ttl,int pptp_fragment,
	int pptp_callid,int pptp_seqno,int pptp_ackno,int ipv4_id, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t pptp_cmpPPPMode);

int pptpPktGen(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,
	int ctagif,int cvid,int cpri,
	int stagif,int svid,int spri, int tos, int pptp_tos,
	cputag_t *cputag, uint8 TCPflag,
	int pppoetagif,int pppoe_session_id,int ttl,int pptp_ttl,int pptp_fragment,
	int pptp_callid,int pptp_seqno,int pptp_ackno,int ipv4_id, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t cmpPPPMode);

int pptpPktGen_lcp(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,
		int ctagif,int cvid,int cpri,
		int stagif,int svid,int spri, int tos, int pptp_tos,
		cputag_t *cputag, uint8 TCPflag,
		int pppoetagif,int pppoe_session_id,int ttl,int pptp_ttl,int pptp_fragment,
		int pptp_callid,int pptp_seqno,int pptp_ackno,int ipv4_id, pptp_outerhdr_option_t reducePPTPOpt);

int pptpPktGen_errFlag(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,uint32 errFlag,
		int ctagif,int cvid,int cpri,
		int stagif,int svid,int spri, int tos, int pptp_tos,
		cputag_t *cputag, uint8 TCPflag,
		int pppoetagif,int pppoe_session_id,int ttl,int pptp_ttl,int pptp_fragment,
		int pptp_callid,int pptp_seqno,int pptp_ackno,int ipv4_id, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t cmpPPPMode);

int pptpPktGen_errFlag_ipver(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,uint32 errFlag,
		int ctagif,int cvid,int cpri,
		int stagif,int svid,int spri, int tos, int pptp_tos,
		cputag_t *cputag, uint8 TCPflag,
		int pppoetagif,int pppoe_session_id,int ttl,int pptp_ttl,int pptp_fragment,
		int pptp_callid,int pptp_seqno,int pptp_ackno,int ipv4_id, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t cmpPPPMode);

int pptpPktGen_errFlag_iphdl(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,uint32 errFlag,
		int ctagif,int cvid,int cpri,
		int stagif,int svid,int spri, int tos, int pptp_tos,
		cputag_t *cputag, uint8 TCPflag,
		int pppoetagif,int pppoe_session_id,int ttl,int pptp_ttl,int pptp_fragment,
		int pptp_callid,int pptp_seqno,int pptp_ackno,int ipv4_id, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t cmpPPPMode);

int pptpv6PktGen_gerneral_errFlag(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,uint32 errFlag, uint32 ipver, uint32 iphdl, uint32 ip6ver,
	int ctagif,int cvid,int cpri,
	int stagif,int svid,int spri, int tos, int pptp_tos,
	cputag_t *cputag, uint8 TCPflag, int innerPaddingCut, int origPadding,
	int pppoetagif,int pppoe_session_id,int ttl,int pptp_ttl,int pptp_fragment, int pptp_lcp,
	int pptp_callid,int pptp_seqno,int pptp_ackno, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t pptp_cmpPPPMode);

int pptpv6PktGen_gerneral(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,
	int ctagif,int cvid,int cpri,
	int stagif,int svid,int spri, int tos, int pptp_tos,
	cputag_t *cputag, uint8 TCPflag, int innerPaddingCut, int origPadding,
	int pppoetagif,int pppoe_session_id,int ttl,int pptp_ttl,int pptp_fragment, int pptp_lcp,
	int pptp_callid,int pptp_seqno,int pptp_ackno, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t pptp_cmpPPPMode);

int pptpv6PktGenCutPadding(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,
	int ctagif,int cvid,int cpri,
	int stagif,int svid,int spri, int tos, int pptp_tos,
	cputag_t *cputag, uint8 TCPflag, int innerPaddingCut, int origPadding,
	int pppoetagif,int pppoe_session_id,int ttl,int pptp_ttl,int pptp_fragment,
	int pptp_callid,int pptp_seqno,int pptp_ackno, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t pptp_cmpPPPMode);

int pptpv6PktGen(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,
	int ctagif,int cvid,int cpri,
	int stagif,int svid,int spri, int tos, int pptp_tos,
	cputag_t *cputag, uint8 TCPflag,
	int pppoetagif,int pppoe_session_id,int ttl,int pptp_ttl,int pptp_fragment,
	int pptp_callid,int pptp_seqno,int pptp_ackno, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t cmpPPPMode);

int pptpv6PktGen_lcp(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,
		int ctagif,int cvid,int cpri,
		int stagif,int svid,int spri, int tos, int pptp_tos,
		cputag_t *cputag, uint8 TCPflag,
		int pppoetagif,int pppoe_session_id,int ttl,int pptp_ttl,int pptp_fragment,
		int pptp_callid,int pptp_seqno,int pptp_ackno, pptp_outerhdr_option_t reducePPTPOpt);

int pptpv6PktGen_errFlag(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,uint32 errFlag,
	int ctagif,int cvid,int cpri,
	int stagif,int svid,int spri, int tos, int pptp_tos,
	cputag_t *cputag, uint8 TCPflag,
	int pppoetagif,int pppoe_session_id,int ttl,int pptp_ttl,int pptp_fragment,
	int pptp_callid,int pptp_seqno,int pptp_ackno, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t cmpPPPMode);

int pptpv6PktGen_errFlag_ipver(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type, uint32 errFlag,
	int ctagif,int cvid,int cpri,
	int stagif,int svid,int spri, int tos, int pptp_tos,
	cputag_t *cputag, uint8 TCPflag,
	int pppoetagif,int pppoe_session_id,int ttl,int pptp_ttl,int pptp_fragment,
	int pptp_callid,int pptp_seqno,int pptp_ackno, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t pptp_cmpPPPMode);

int pptpv6PktGen_errFlag_iphdl(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type, uint32 errFlag,
	int ctagif,int cvid,int cpri,
	int stagif,int svid,int spri, int tos, int pptp_tos,
	cputag_t *cputag, uint8 TCPflag,
	int pppoetagif,int pppoe_session_id,int ttl,int pptp_ttl,int pptp_fragment,
	int pptp_callid,int pptp_seqno,int pptp_ackno, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t pptp_cmpPPPMode);

int l2tpPktGen(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,
	int ctagif,int cvid,int cpri,
	int stagif,int svid,int spri, int dscp,
	cputag_t *cputag,
	int pppoetagif,int pppoe_session_id,int ttl,
	int l2tp_tunnelID,int l2tp_sesstionID,int ipv4_id, uint8 reduceL2TPLen, compressed_ppp_mode_t cmpPPPMode);

int dslitePktGen(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,
	int ctagif,int cvid,int cpri,
	int stagif,int svid,int spri, int dscp,
	cputag_t *cputag,
	int pppoetagif,int pppoe_session_id,int ttl,
	int l2tp_tunnelID,int l2tp_sesstionID,int ipv4_id);

int pktGen_Insert_rxCpuTag(unsigned char *packet, _cpu_rx_Hdr_t *cpu_tag, uint32 *len);

int v4TcpUdpPktGen(uint8 *pktBuf,uint8 *pktData,int *pktLen,
	unsigned char *smac, unsigned char *dmac,
	unsigned char *sip, unsigned short sport,
	unsigned char *dip, unsigned short dport,
	int protocol,int ttl,
	int ctagif,int cvid,int cpri,
	int stagif,int tpid, int svid, int spri, int dscp,
	cputag_t *cputag, cputagTx_t *cputagTx,
	int pppoetagif,int pppoe_session_id);


int v4TcpUdpPktGen2(
		uint8 *pktBuf,uint8 *pktData,int *pktLen,int pktType,
		unsigned char *smac, unsigned char *dmac,
		uint32 sip, uint32 dip,
		unsigned short sport,unsigned short dport,
		int ttl, int tos,
		int ctagif,int cvid,int cpri,
		int stagif,int tpid, int svid,int spri,
		int pppoetagif,int pppoe_session_id);

int v4TcpUdpPktGen3(
	uint8 *pktBuf,uint8 *pktData,int *pktLen,int pktType,
	unsigned char *smac, unsigned char *dmac,
	uint32 sip, uint32 dip,
	unsigned short sport,unsigned short dport,
	int ttl, int dscp,
	int ctagif,int ctpid, int cvid,int cpri,int cdei,
	int stagif,int stpid, int svid,int spri,int sdei,
	int pppoetagif,int pppoe_session_id);

int v4TcpUdpPktGen4(
	uint8 *pktBuf,uint8 *pktData,int *pktLen,int pktType,uint8 tcpFlag,
	unsigned char *smac, unsigned char *dmac,
	uint32 sip, uint32 dip,
	unsigned short sport,unsigned short dport,
	int ttl, int dscp,
	int ctagif,int ctpid, int cvid,int cpri,int cdei,
	int stagif,int stpid, int svid,int spri,int sdei,
	int pppoetagif,int pppoe_session_id,
	uint8 ipmf ,uint8 fragOffset,uint16 tcsepq ,uint8 tcpDataOffsetSmallErr,uint8 v4HdrLen);

int v4TcpUdpPktGen5(
	uint8 *pktBuf,uint8 *pktData,int *pktLen,int pktType,
	unsigned char *smac, unsigned char *dmac,
	uint32 sip, uint32 dip,
	unsigned short sport,unsigned short dport,
	int ttl, int tos,
	int ctagif,int cvid,int cpri,
	int stagif,int tpid, int svid,int spri,
	int pppoetagif,int pppoe_session_id,uint16 ipid);


int v4TcpUdpPktGen2ContentZero(
	uint8 *pktBuf,uint8 *pktData,int *pktLen,int pktType,
	unsigned char *smac, unsigned char *dmac,
	uint32 sip, uint32 dip,
	unsigned short sport,unsigned short dport,
	int ttl, int tos,
	int ctagif,int cvid,int cpri,
	int stagif,int tpid, int svid,int spri,
	int pppoetagif,int pppoe_session_id);


int v6TcpUdpPktGen(
		uint8 *pktBuf,uint8 *pktData,int *pktLen,int pktType,
		unsigned char *smac, unsigned char *dmac,
		unsigned char *sip, unsigned char *dip,
		unsigned short sport,unsigned short dport,
		int ttl, int dscp,
		int ctagif,int cvid,int cpri,
		int stagif,int tpid, int svid,int spri,
		int pppoetagif,int pppoe_session_id);

int v6TcpUdpPktGen2(
	uint8 *pktBuf,uint8 *pktData,int *pktLen,int pktType,
	unsigned char *smac, unsigned char *dmac,
	unsigned char *sip, unsigned char *dip,
	unsigned short sport,unsigned short dport,
	int hoplimit, int tc,
	int ctagif,int cvid,int cpri,
	int stagif,int tpid, int svid,int spri,
	int pppoetagif,int pppoe_session_id);

int v6TcpUdpPktGen3(
	uint8 *pktBuf,uint8 *pktData,int *pktLen,int pktType,
	unsigned char *smac, unsigned char *dmac,
	unsigned char *sip, unsigned char *dip,
	unsigned short sport,unsigned short dport,
	int hoplimit, int tc,
	int ctagif,int cvid,int cpri,int cdei,
	int stagif,int tpid, int svid,int spri,int sdei,
	int pppoetagif,int pppoe_session_id);


int v6TcpUdpPktGen4(
	uint8 *pktBuf,uint8 *pktData,int *pktLen,int pktType,
	unsigned char *smac, unsigned char *dmac,
	unsigned char *sip, unsigned char *dip,
	unsigned short sport,unsigned short dport,
	int hoplimit, int tc,
	int ctagif,int ctpid, int cvid,int cpri,int cdei,
	int stagif,int stpid, int svid,int spri,int sdei,
	int pppoetagif,int pppoe_session_id);


int v6rdPktGenByPatten(uint8 *pktBuf, uint8 *pktData, int *pktLen, uint32 pktType, uint8 tcpFlag,
		uint8* pktsmac, uint8* pktdmac,
		uint8* pktV6sip, uint8 *pktV6dip,
		uint32 pktsip, uint32 pktdip,
		uint16 sport, uint16 dport,
		int ctagif,int cvid,int cpri,
		int stagif,int svid,int spri, 
		uint8 pppoeTagif,uint16 pppoeSID, 
		uint16 ipid, uint8 ttl, uint8 hoplimit,	uint8 mf_frag, uint8 tos, uint8 tc);

int v6rdTCPPktGenByPatten(uint8 *pktBuf, uint8 *pktData, int *pktLen, uint32 pktType, 
		uint8 tcpFlag, int data_offset, int seq, int ack,
		uint8* pktsmac, uint8* pktdmac,
		uint8* pktV6sip, uint8 *pktV6dip,
		uint32 pktsip, uint32 pktdip,
		uint16 sport, uint16 dport,
		int ctagif,int cvid,int cpri,
		int stagif,int svid,int spri, 
		uint8 pppoeTagif,uint16 pppoeSID, 
		uint16 ipid, uint8 ttl, uint8 hoplimit, uint8 mf_frag, uint8 tos, uint8 tc);

int v6rdFragmentPktGenByPatten(uint8 *pktBuf, uint8 *pktData, int *pktLen, uint32 pktType, uint8 tcpFlag,
		uint8* pktsmac, uint8* pktdmac,
		uint8* pktV6sip, uint8 *pktV6dip,
		uint32 pktsip, uint32 pktdip,
		uint16 sport, uint16 dport,
		int ctagif,int cvid,int cpri,
		int stagif,int svid,int spri, 
		uint8 pppoeTagif,uint16 pppoeSID, 
		uint16 ipid, uint8 ttl, uint8 hoplimit, uint8 mf_frag, uint16 v6_mf_frag, uint16 v6_frag_id, uint8 tos, uint8 tc);

int dslitePktGenByPatten(uint8 *pktBuf, uint8 *pktData, int *pktLen, uint32 pktType, uint8 tcpFlag,
		uint8* pktsmac, uint8* pktdmac,
		uint8* pktV6sip, uint8 *pktV6dip,
		uint32 pktsip, uint32 pktdip,
		uint16 sport, uint16 dport,
		int ctagif,int cvid,int cpri,
		int stagif,int svid,int spri, 
		uint8 pppoeTagif,uint8 pppoeSID, 
		uint8 ttl, uint8 hoplimit,	uint8 mf, uint8 tos,uint8 tc);

int MAPE_PktGenByPatten(uint8 *pktBuf, uint8 *pktData, int *pktLen, uint32 pktType, uint8 tcpFlag,
		uint8* pktsmac, uint8* pktdmac,
		uint8* pktV6sip, uint8 *pktV6dip,
		uint32 pktsip, uint32 pktdip,
		uint16 sport, uint16 dport,
		int ctagif,int cvid,int cpri,
		int stagif,int svid,int spri, 
		uint8 pppoeTagif,uint8 pppoeSID, 
		uint8 ttl, uint8 hoplimit,	uint8 mf, uint8 tos,uint8 tc);

int MAPE_PktGenByPattenContentZero(uint8 *pktBuf, uint8 *pktData, int *pktLen, uint32 pktType, uint8 tcpFlag,
		uint8* pktsmac, uint8* pktdmac,
		uint8* pktV6sip, uint8 *pktV6dip,
		uint32 pktsip, uint32 pktdip,
		uint16 sport, uint16 dport,
		int ctagif,int cvid,int cpri,
		int stagif,int svid,int spri, 
		uint8 pppoeTagif,uint8 pppoeSID, 
		uint8 ttl, uint8 hoplimit,	uint8 mf, uint8 tos,uint8 tc);


int MAPE_PktGenByPatten2(uint8 *pktBuf, uint8 *pktData, int *pktLen, uint32 pktType, uint8 tcpFlag,
		uint8* pktsmac, uint8* pktdmac,
		uint8* pktV6sip, uint8 *pktV6dip,
		uint32 pktsip, uint32 pktdip,
		uint16 sport, uint16 dport,
		int ctagif,int cvid,int cpri,
		int stagif,int svid,int spri, 
		uint8 pppoeTagif,uint8 pppoeSID, 
		uint8 ttl, uint8 hoplimit,	uint8 inner_mf ,uint8 outer_mf, uint8 tos,uint8 tc,
		uint16 outer_ipfragOffset, uint16 inner_ipfragOffset, uint16 tcpseq, uint8 smallTcpOffset);

int MAPE_PktGenByPatten3(uint8 *pktBuf, uint8 *pktData, int *pktLen, uint32 pktType, uint8 tcpFlag,
		uint8* pktsmac, uint8* pktdmac,
		uint8* pktV6sip, uint8 *pktV6dip,
		uint32 pktsip, uint32 pktdip,
		uint16 sport, uint16 dport,
		int ctagif,int cvid,int cpri,
		int stagif,int svid,int spri, 
		uint8 pppoeTagif,uint8 pppoeSID, 
		uint8 ttl, uint8 hoplimit,	uint8 inner_mf ,uint8 outer_mf, uint8 tos,uint8 tc,
		uint16 outer_ipfragOffset, uint16 inner_ipfragOffset, uint16 tcpseq, uint8 smallTcpOffset);

int l2PktGen(uint8 *pktBuf,uint8 *pktData,int *pktLen,
	unsigned char *smac, unsigned char *dmac, int etype,
	int ctagif,int cvid,int cpri,
	int stagif,int tpid, int svid,int spri,
	cputag_t *cputag, cputagTx_t *cputagTx,
	int pppoetagif,int pppoe_session_id);

int SRv6L2TunnelwithPureL2PktGen(uint8 *pktBuf,uint8 *pktData,int *pktLen,
	unsigned char *l2tun_smac, unsigned char *l2tun_dmac,
	unsigned char *l2tun_sip,unsigned char *l2tun_dip,
	int l2tun_protocol,int l2tun_hoplimit, int l2tun_tc, int flow_label,
	int l2tun_ctagif,int l2tun_cvid,int l2tun_cpri,
	int l2tun_stagif,int l2tun_tpid, int l2tun_svid,int l2tun_spri,
	int l2tun_pppoetagif,int l2tun_pppoe_session_id,
	int srhHdrLen, int srhSegmentLeft,
	unsigned char *smac, unsigned char *dmac, int etype,
	int ctagif,int cvid,int cpri,
	int stagif,int tpid, int svid,int spri,
	cputag_t *cputag, cputagTx_t *cputagTx,
	int pppoetagif,int pppoe_session_id);

int SRv6L2TunnelwithPaddingPureL2PktGen(uint8 *pktBuf,uint8 *pktData,int *pktLen,int paddingLen,
	unsigned char *l2tun_smac, unsigned char *l2tun_dmac,
	unsigned char *l2tun_sip,unsigned char *l2tun_dip,
	int l2tun_protocol,int l2tun_hoplimit, int l2tun_tc, int flow_label,
	int l2tun_ctagif,int l2tun_cvid,int l2tun_cpri,
	int l2tun_stagif,int l2tun_tpid, int l2tun_svid,int l2tun_spri,
	int l2tun_pppoetagif,int l2tun_pppoe_session_id,
	int srhHdrLen, int srhSegmentLeft,
	unsigned char *smac, unsigned char *dmac, int etype,
	int ctagif,int cvid,int cpri,
	int stagif,int tpid, int svid,int spri,
	cputag_t *cputag, cputagTx_t *cputagTx,
	int pppoetagif,int pppoe_session_id);


int SRv6L2TunnelwithV4TcpUdpPktGen(uint8 *pktBuf,uint8 *pktData,int *pktLen,
	unsigned char *l2tun_smac, unsigned char *l2tun_dmac,
	unsigned char *l2tun_sip,unsigned char *l2tun_dip,
	int l2tun_protocol,int l2tun_hoplimit, int l2tun_tc, int flow_label,
	int l2tun_ctagif,int l2tun_cvid,int l2tun_cpri,
	int l2tun_stagif,int l2tun_tpid, int l2tun_svid,int l2tun_spri,
	int l2tun_pppoetagif,int l2tun_pppoe_session_id,
	int srhHdrLen, int srhSegmentLeft,
	unsigned char *smac, unsigned char *dmac,
	unsigned char *sip, unsigned short sport,
	unsigned char *dip, unsigned short dport,
	int protocol,int ttl, int iptos,
	int ctagif,int cvid,int cpri,
	int stagif,int tpid, int svid,int spri,
	cputag_t *cputag, cputagTx_t *cputagTx,
	int pppoetagif,int pppoe_session_id);

int SRv6L2TunnelwithV4TcpUdpPktGen2(uint8 *pktBuf,uint8 *pktData,int *pktLen,
	unsigned char *l2tun_smac, unsigned char *l2tun_dmac,
	unsigned char *l2tun_sip,unsigned char *l2tun_dip,
	int l2tun_protocol,int l2tun_hoplimit, int l2tun_tc, int flow_label,
	int l2tun_ctagif,int l2tun_cvid,int l2tun_cpri,
	int l2tun_stagif,int l2tun_tpid, int l2tun_svid,int l2tun_spri,
	int l2tun_pppoetagif,int l2tun_pppoe_session_id,
	int srhHdrLen, int srhSegmentLeft,
	unsigned char *smac, unsigned char *dmac,
	unsigned char *sip, unsigned short sport,
	unsigned char *dip, unsigned short dport,
	int protocol,int ttl, int iptos,int ipmf,int ipfost,int tcpFlag,
	int ctagif,int cvid,int cpri,
	int stagif,int tpid, int svid,int spri,
	cputag_t *cputag, cputagTx_t *cputagTx,
	int pppoetagif,int pppoe_session_id);

int SRv6L2TunnelwithPaddingV4TcpUdpPktGen(uint8 *pktBuf,uint8 *pktData,int *pktLen,int paddingLen,
	unsigned char *l2tun_smac, unsigned char *l2tun_dmac,
	unsigned char *l2tun_sip,unsigned char *l2tun_dip,
	int l2tun_protocol,int l2tun_hoplimit, int l2tun_tc, int flow_label,
	int l2tun_ctagif,int l2tun_cvid,int l2tun_cpri,
	int l2tun_stagif,int l2tun_tpid, int l2tun_svid,int l2tun_spri,
	int l2tun_pppoetagif,int l2tun_pppoe_session_id,
	int srhHdrLen, int srhSegmentLeft,
	unsigned char *smac, unsigned char *dmac,
	unsigned char *sip, unsigned short sport,
	unsigned char *dip, unsigned short dport,
	int protocol,int ttl, int iptos,
	int ctagif,int cvid,int cpri,
	int stagif,int tpid, int svid,int spri,
	cputag_t *cputag, cputagTx_t *cputagTx,
	int pppoetagif,int pppoe_session_id);

int SRv6L2TunnelwithV6TcpUdpPktGen(uint8 *pktBuf,uint8 *pktData,int *pktLen,
	unsigned char *l2tun_smac, unsigned char *l2tun_dmac,
	unsigned char *l2tun_sip,unsigned char *l2tun_dip,
	int l2tun_protocol,int l2tun_hoplimit, int l2tun_tc, int l2tun_flowlabel,
	int l2tun_ctagif,int l2tun_cvid,int l2tun_cpri,
	int l2tun_stagif,int l2tun_tpid, int l2tun_svid,int l2tun_spri,
	int l2tun_pppoetagif,int l2tun_pppoe_session_id,
	int srhHdrLen, int srhSegmentLeft,
	unsigned char *smac, unsigned char *dmac,
	unsigned char *sip, unsigned short sport,
	unsigned char *dip, unsigned short dport,
	int nextheader, int flowlabel, int hoplimit, int tc,int ipmf,int ipfost,int tcpFlag,
	int ctagif,int cvid,int cpri,
	int stagif,int tpid, int svid,int spri,
	cputag_t *cputag, cputagTx_t *cputagTx,
	int pppoetagif,int pppoe_session_id);

int SRv6L2TunnelwithV6TcpUdpPktGen2(uint8 *pktBuf,uint8 *pktData,int *pktLen,
	unsigned char *l2tun_smac, unsigned char *l2tun_dmac,
	unsigned char *l2tun_sip,unsigned char *l2tun_dip,
	int l2tun_protocol,int l2tun_hoplimit, int l2tun_tc, int l2tun_flowlabel,
	int l2tun_ctagif,int l2tun_cvid,int l2tun_cpri,
	int l2tun_stagif,int l2tun_tpid, int l2tun_svid,int l2tun_spri,
	int l2tun_pppoetagif,int l2tun_pppoe_session_id,
	int srhHdrLen, int srhSegmentLeft,
	unsigned char *smac, unsigned char *dmac,
	unsigned char *sip, unsigned short sport,
	unsigned char *dip, unsigned short dport,
	int nextheader, int flowlabel, int hoplimit, int tc, int mf_frag,
	int ctagif,int cvid,int cpri,
	int stagif,int tpid, int svid,int spri,
	cputag_t *cputag, cputagTx_t *cputagTx,
	int pppoetagif,int pppoe_session_id);

int SRv6L2TunnelPktGen(uint8 *pktBuf,uint8 *pktData,int *pktLen,
	unsigned char *l2tun_smac, unsigned char *l2tun_dmac,
	unsigned char *l2tun_sip,unsigned char *l2tun_dip,
	int l2tun_protocol,int l2tun_hoplimit, int l2tun_tc, int flow_label,
	int l2tun_ctagif,int l2tun_cvid,int l2tun_cpri,
	int l2tun_stagif,int l2tun_tpid, int l2tun_svid,int l2tun_spri,
	int l2tun_pppoetagif,int l2tun_pppoe_session_id,
	int srhHdrLen, int srhSegmentLeft, int srhLastEntry,
	unsigned char *sid[],
	unsigned char *smac, unsigned char *dmac, int etype,
	int ctagif,int cvid,int cpri,
	int stagif,int tpid, int svid,int spri,
	cputag_t *cputag, cputagTx_t *cputagTx,
	int pppoetagif,int pppoe_session_id);

int SRv6SRHL2TunnelwithV4TcpUdpPktGen(uint8 *pktBuf,uint8 *pktData,int *pktLen,
	unsigned char *l2tun_smac, unsigned char *l2tun_dmac,
	unsigned char *l2tun_sip,unsigned char *l2tun_dip,
	int l2tun_protocol,int l2tun_hoplimit, int l2tun_tc, int flow_label,
	int l2tun_ctagif,int l2tun_cvid,int l2tun_cpri,
	int l2tun_stagif,int l2tun_tpid, int l2tun_svid,int l2tun_spri,
	int l2tun_pppoetagif,int l2tun_pppoe_session_id,
	int srhHdrLen, int srhSegmentLeft, int srhLastEntry,
	unsigned char *sid[],
	unsigned char *smac, unsigned char *dmac,
	unsigned char *sip, unsigned short sport,
	unsigned char *dip, unsigned short dport,
	int protocol,int ttl, int iptos,int ipmf,int ipfost,int tcpFlag,
	int ctagif,int cvid,int cpri,
	int stagif,int tpid, int svid,int spri,
	cputag_t *cputag, cputagTx_t *cputagTx,
	int pppoetagif,int pppoe_session_id);

int SRv6SRHL2TunnelwithV4TcpUdpPktGen2(uint8 *pktBuf,uint8 *pktData,int *pktLen,
	unsigned char *l2tun_smac, unsigned char *l2tun_dmac,
	unsigned char *l2tun_sip,unsigned char *l2tun_dip,
	int l2tun_protocol,int l2tun_hoplimit, int l2tun_tc, int flow_label,
	int l2tun_ctagif,int l2tun_cvid,int l2tun_cpri,
	int l2tun_stagif,int l2tun_tpid, int l2tun_svid,int l2tun_spri,
	int l2tun_pppoetagif,int l2tun_pppoe_session_id,
	int srhHdrLen, int srhSegmentLeft,
	unsigned char *sid[],
	unsigned char *smac, unsigned char *dmac,
	unsigned char *sip, unsigned short sport,
	unsigned char *dip, unsigned short dport,
	int protocol,int ttl, int iptos, int mf_frag, int tcp_flags,
	int ctagif,int cvid,int cpri,
	int stagif,int tpid, int svid,int spri,
	cputag_t *cputag, cputagTx_t *cputagTx,
	int pppoetagif,int pppoe_session_id);

int SRv6SRHL2TunnelwithV6TcpUdpPktGen(uint8 *pktBuf,uint8 *pktData,int *pktLen,
	unsigned char *l2tun_smac, unsigned char *l2tun_dmac,
	unsigned char *l2tun_sip,unsigned char *l2tun_dip,
	int l2tun_protocol,int l2tun_hoplimit, int l2tun_tc, int l2tun_flowlabel,
	int l2tun_ctagif,int l2tun_cvid,int l2tun_cpri,
	int l2tun_stagif,int l2tun_tpid, int l2tun_svid,int l2tun_spri,
	int l2tun_pppoetagif,int l2tun_pppoe_session_id,
	int srhHdrLen, int srhSegmentLeft, int srhLastEntry,
	unsigned char *sid[],
	unsigned char *smac, unsigned char *dmac,
	unsigned char *sip, unsigned short sport,
	unsigned char *dip, unsigned short dport,
	int nextheader, int flowlabel, int hoplimit, int tc,int ipmf,int ipfost,int tcpFlag,
	int ctagif,int cvid,int cpri,
	int stagif,int tpid, int svid,int spri,
	cputag_t *cputag, cputagTx_t *cputagTx,
	int pppoetagif,int pppoe_session_id);

int SRv6TcpUdpPktGen(uint8 *pktBuf,uint8 *pktData,int *pktLen,
	unsigned char *smac, unsigned char *dmac,
	int srhHdrLen, int srhSegmentLeft,int srhLastEntry,
	unsigned char *sid[],unsigned char *srh_dip,
	unsigned char *sip, unsigned short sport,
	unsigned char *dip, unsigned short dport,
	int nextheader, int flowlabel, int hoplimit, int tc, int ipmf,int ipfost,int tcpFlag,
	int ctagif,int cvid,int cpri,
	int stagif,int tpid, int svid,int spri,
	int pppoetagif,int pppoe_session_id);

int IGMPv2_pktGen(uint8 *pktBuf,int *pktLen,igmpType_t igmpType,uint8 *smac,uint32 sourceip,uint32 groupip);


#endif
