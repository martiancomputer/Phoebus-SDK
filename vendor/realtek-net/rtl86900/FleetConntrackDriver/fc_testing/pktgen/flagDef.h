#ifndef FLAGDEF_H
#define FLAGDEF_H

//Ethernet is always exist
#define L2_CVLAN			0x00000001
#define L2_SNAP				0x00000002
#define L2_PPPoE			0x00000004
#define L2_SVLAN			0x00000008
#if 0// WEN FIX ME
#define L2_OTHERVLAN		0x00000010
#endif
#define L2_PPPoE6			0x00004000
#if 0 // WEN FIX ME
#define L2_MASK				0x0000401f
#else
#define L2_MASK				0x0000400f
#endif

#define L2_CPUTAG_OTHER	0x00002000
//#define L2_CPUTAG_DSL		0x00004000
#define L2_CPUTAG_TX2		0x00008000
#define L2_CPUTAG_DSLTX		0x00010000
#define L2_CPUTAG_MASK		0x0001e000

#define L3_IPX			0x00000010
#define L3_ARP			0x00000020
#define L3_IP			0x00000040
#define L3_IPV6         0x00000080   //added by liujuan
#define L3_MASK			0x000000f0

#define L4_ICMP			0x00000100
#define L4_IGMP			0x00000200
#define L4_UDP			0x00000400
#define L4_TCP			0x00000800
#define	L4_PPTP			0x00001000		
#define L4_MASK			0x00001f00

#if 1 //cheney
#define L3_GRE			0x00010000
#define L3_GREPPP		0x00020000
#define L3_V6GRE		0x00040000

#define DUAL_OUTER_HDR	0x00080000
#endif

#define L2_CRC_ERR		0x80000000
#define L3_CKSUM_ERR	0x40000000
#define L4_CKSUM_ERR	0x20000000
#define L4_NO_CKSUM		0x10000000
#define L2_PPP_ERR		0x08000000
#define CTL_MASK		0xf8000000
//added by liujuan
#define hopbyhopHdr  	0x00100000
#define DesHdr          0x00200000
#define RouHdr          0x00400000
#define FraHdr          0x00800000
#define AutHdr          0x01000000
#define ESPHdr          0x02000000 
#define DesHdr2         0x04000000
#define NoNHdr          0x08000000
#define SRv6Hdr		0x00004000
#define ehMASK          0x0ff04000

#define PKT_PARSE_L3_CKSUM_OK	0x00000001
#define PKT_PARSE_L3_CKSUM_ERR	0x00000002

#define PKT_PARSE_L4_CKSUM_OK	0x00000001
#define PKT_PARSE_L4_CKSUM_ERR	0x00000002
#define PKT_PARSE_UDP_NO_CKSUM	0x00000004
#define PKT_PARSE_UDP_LEN_ERR	0x00000008

//#define L2_BCAST		0x00010000
#define L2_MCAST		0x00020000
#define L3_BCAST		0x00040000
#define L3_MCAST		0x00080000
#define RELAY_MASK		0x000f0000

#endif

