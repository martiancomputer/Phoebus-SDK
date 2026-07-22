#ifndef __RTK_FC_SKB_H__
#define __RTK_FC_SKB_H__
#include <linux/version.h>

typedef struct rtk_fc_ingress_vxlan_info_s
{

	unsigned char outer_tag_len:7;
	unsigned char outer_isV6:1; 					// For vxlan down stream
#if defined(CONFIG_FC_RTL9607C_SERIES)||defined(CONFIG_FC_CA8277B_SERIES)
	unsigned short outer_srcPort;
	unsigned char outer_v4tos;

#endif

}rtk_fc_ingress_vxlan_info_t;

typedef struct rtk_fc_ingress_nptv6_info_s
{
	unsigned int lan_prefix_len;
    unsigned int wan_prefix_len;
}rtk_fc_ingress_nptv6_info_t;

 
typedef struct rtk_fc_ingress_ipsec_info_s
{
	unsigned int ingress_spi;
    unsigned int ingress_seq_num;
	unsigned char ingressHitIPSecPolicy;
	unsigned int hookTbl_index;
	unsigned short outerUdp_srcPort;
	unsigned short outerUdp_dstPort;
	unsigned char outer_v4tos;
	
}rtk_fc_ingress_ipsec_info_t;

typedef struct rtk_fc_ingress_srv6_info_s
{
	unsigned char outer_tag_len;
	unsigned char outer_srh_off;
}rtk_fc_ingress_srv6_info_t;

typedef struct rtk_fc_dualHeader_mapt_info_s
{
	unsigned int src_style:2;
	unsigned int src_v6_pref_len:7;
	unsigned int src_v4_pref_len:5;
	unsigned int src_ea_len:6;
	unsigned int src_psid_offset:4;
	unsigned char src_v6_addr[8];

	unsigned int dst_style:2;
	unsigned int dst_v6_pref_len:7;
	unsigned int dst_v4_pref_len:5;
	unsigned int dst_ea_len:6;
	unsigned int dst_psid_offset:4;
}rtk_fc_dualHeader_mapt_info_t;

#define DEVIFIDX_INVALID_MIN (0)
#define DEVIFIDX_VALID_MIN 	(0x1)

#define DEV_STACK_MAX	 (4)
#define NETIF_DEV_IDX_SHIFT	 (8)
#define NETIF_DEV_IDX_SHIFT_MASK ((1<<NETIF_DEV_IDX_SHIFT)-1)
#define MAX_FC_VLAN_PARSING_CNT		7
#define MAX_FC_VLAN_CNT_BITS		3

typedef struct rtk_fc_ingress_data_s
{
	/* 
		index to fc_db.devGwMacTbl
		ingress stacking : cache stack in (br/ip) pre-routing
		egress stacking : cache stack in (br/ip) post-routing
	*/
	unsigned char igrDevStacking[DEV_STACK_MAX]; 
	unsigned char egrDevStacking[DEV_STACK_MAX]; 

	int flowHashIdx;					// -1 indicate invalid
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	unsigned int crc32;
#endif
	struct nf_conn *ct;
	unsigned int  ingressTagif;			// ref to rtk_fc_enum_pktHdrTagif_t
	unsigned char l4protol;
	unsigned int srcIp;				// ipv4 address or ipv6 hash value
	unsigned int dstIp;				// ipv4 address or ipv6 hash value
	union{
		unsigned short srcPort;
		unsigned short icmp_id;
	};
	unsigned short dstPort;
	
	unsigned short sessionId;			//ref to PPPOE_TAGIF
	unsigned short outer_sessionId;     // For vxlan down stream
	
	unsigned short ingressCallid;		// pptp gre
	unsigned char ingressPort;			// mac port number
	unsigned char wlan_dev_idx;		// wlan dev index indicate the using mac port and ext port
	
	unsigned char sa[6];			//store inner da/sa if dual-l2 
	unsigned char da[6];
	
	unsigned char v4tos;				//ref to IPV4_TAGIF
	unsigned char v6tos;				//ref to IPV6_TAGIF
	unsigned char dual_hdr_cnt;				//ref to IPV6_TAGIF
	unsigned int srcSwParseVlanCnt:MAX_FC_VLAN_CNT_BITS;
	unsigned int srcSVlanId:12;		//ref to SVLAN_TAGIF
	unsigned int srcCVlanPri:3;
	unsigned int srcCVlanCfi:1;
	
	unsigned int srcCVlanId:12;		//ref to CVLAN_TAGIF
	unsigned int srcSVlanPri:3;	
	unsigned int srcSVlanDei:1;
	unsigned int srcSVlanTpid_sel:2;		//ref to SVLAN_TAGIF
#if defined(CONFIG_RTK_FC_FORWARDING_AWARE_LAN_SWID)
	union{
		unsigned int ponStreamId:8;
		unsigned int ethSwId:8;
	};
#else
	unsigned int ponStreamId:8;		// ref: RTK_FC_RMK_STREAMID_LEN_DEF ASIC_FLOW_PON_STREAMID_BITS but hw support 8bits only in 9617C/9607F.
#endif
	unsigned int isDAGatewayMAC:1;
	unsigned int inner_isDAGatewayMAC:1;
	unsigned int isDualHeader:1;		// True: this content belongs to outer header (ingress parser result)
	unsigned int doLearning:1;
	unsigned int isNotLocalOut:1;
	unsigned int isDummyPkt:1;		// triger by house keeping
	unsigned int mcDummyPkt:1;
#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)
	unsigned int ipsec_skip_kernel:1;
	unsigned int ipsec_hwlookup:1;
#endif
	unsigned int srv6_hwlookup:1;
	unsigned int ipId:16;
	unsigned int SrcDevDevGwMacIdx:NETIF_DEV_IDX_SHIFT;		//point to fc_db.devGwMacTbl
	unsigned int outerHdrUnhit:1;
	unsigned int isHitSwFwdedAclTrapPri:1;
	unsigned int skbCloned:1;
	unsigned int ipFragFlag:3;		// for fragment shortcut only
#if defined(CONFIG_RTK_SOC_RTL8198D) || defined(CONFIG_FC_RTL8198F_SERIES) || defined(CONFIG_RTL8198X_SERIES)
	unsigned int 	skip_nf_conntrack_in:1;		// for fc shortcut tcp_in_window() check fail, skip kernel nf_conntrack_in()
	char			wlan_type;					// for traffic count
	unsigned char ingress_sa[6];
	unsigned char ingress_da[6];
#endif
#if defined(CONFIG_ARK_QOS)
	unsigned char src_phyport;
#endif
	unsigned int ingressDualFailed:1;
	unsigned int isHwCopy2CPU:1;		//is hit hardware and copy a packet to CPU
	unsigned int isDownStream:1;		//for net filter hook point using
	unsigned int traceFilterMatchShow:1;	//for linux protocol debug using

	union
	{
		/* please review all, we want to remove it */
		rtk_fc_ingress_vxlan_info_t vxlan_info;       		//VXLAN_TAGIF
		rtk_fc_ingress_nptv6_info_t nptv6_info;
		rtk_fc_ingress_ipsec_info_t ipsec_info;
		rtk_fc_ingress_srv6_info_t srv6_info;       		//VXLAN_TAGIF
	};
	struct	in6_addr ingress_v6ip_info;		//downstream->dip upstream->sip
#if defined(CONFIG_RTK_FC_PKT_QUEUE_OFFLOAD_BY_PE)	
	struct	in6_addr igr_ipv6_dip;
#endif
	unsigned int dualPattenHashKey;
}__attribute__((packed)) rtk_fc_ingress_data_t;

#endif
