#ifndef _RTL_PROTOCOL_
#define _RTL_PROTOCOL_

#include "rtl_types.h"
#include "l4pkt.h"

#define _LITTLE_ENDIAN 1

typedef struct vlanHdr_s {
#ifdef _LITTLE_ENDIAN
	uint16	vidh:4;
	uint16	cfi:1;
	uint16	priority:3;
	uint16	vidl:8;
#else
	uint16	priority:3;
	uint16	cfi:1;
	uint16	vidh:4;
	uint16	vidl:8;
#endif
	uint16	ether_type;
} vlanHdr_t;

typedef struct svlanHdr_s {
#ifdef _LITTLE_ENDIAN
	uint16	vidh:4;
	uint16	dei:1;
	uint16	priority:3;
	uint16	vidl:8;
#else
	uint16	priority:3;
	uint16	dei:1;
	uint16	vidh:4;
	uint16	vidl:8;
#endif
	uint16	ether_type;
} svlanHdr_t;

typedef struct ipv6hdr_s {
#ifndef _LITTLE_ENDIAN
	uint32 	version:4;	
	uint32 	tclassH:4;
	uint32	tclassL:4;	
	uint32	flow_lblH:4;
	uint32	flow_lblL:16;
#else
	uint32	tclassH:4;
	uint32	version:4;
	uint32	flow_lblH:4;
	uint32	tclassL:4;
	uint32	flow_lblL:16;
#endif

	uint32 	payload_len:16;
	uint32 	nexthdr:8;
	uint32	hop_limit:8;
	uint8  	saddr[16];
	uint8  	daddr[16];

} ipv6hdr_t;

typedef struct ip {
	/* replace bit field */
	uint8 ip_vhl;

	uint8	ip_tos;			/* type of service */
	uint16	ip_len;			/* total length */
	uint16	ip_id;			/* identification */
	uint16	ip_off;			/* fragment offset field */
	uint8	ip_ttl;			/* time to live */
	uint8	ip_p;			/* protocol */
	uint16	ip_sum;			/* checksum */
	uint32 		ip_src,ip_dst;	/* source and dest address */
} ip_t;

#ifndef RTK_RG_STRUCT_H

/* PPTP */
//PPTP message type
typedef enum rtk_rg_pptpCtrlMsgType_e
{
	PPTP_StartCtrlConnRequest 	= 1,
	PPTP_StartCtrlConnReply 	= 2,
	PPTP_StopCtrlConnRequest 	= 3,
	PPTP_StopCtrlConnReply 		= 4,
	PPTP_EchoRequest 			= 5,
	PPTP_EchoReply 				= 6,
	PPTP_OutCallRequest 		= 7,
	PPTP_OutCallReply 			= 8,
	PPTP_InCallRequest 			= 9,
	PPTP_InCallReply 			= 10,
	PPTP_InCallConn 			= 11,
	PPTP_CallClearRequest 		= 12,
	PPTP_CallDiscNotify 		= 13,
	PPTP_WanErrorNotify 		= 14,
	PPTP_SetLinkInfo 			= 15
}rtk_rg_pptpCtrlMsgType_t;

typedef struct rtk_rg_pptpMsgHead_s
{
	unsigned short int    length;			/* total length */
	unsigned short int    msgType;			/* PPTP message type */
	unsigned int      	  magic;			/* magic cookie */
	unsigned short int    type;				/* control message type */
	unsigned short int    resv0;			/* reserved */
}rtk_rg_pptpMsgHead_t;

typedef struct rtk_rg_pptpCallIds_s
{
	unsigned short int    cid1;				/* Call ID field #1 */
	unsigned short int    cid2;				/* Call ID field #2 */
}rtk_rg_pptpCallIds_t;

typedef struct rtk_rg_pptpCodes_s
{
	unsigned char     resCode;				/* Result Code */
	unsigned char     errCode;				/* Error Code */
}rtk_rg_pptpCodes_t;

#endif //RTK_RG_STRUCT_H

#define PPTP_CTRL_MSG_TYPE		1		// 1 for Control Message
#define PPTP_MAGIC				0x1a2b3c4d

#ifndef _UAPI_LINUX_IN_H
/* Standard well-defined IP protocols.  */
enum {
  IPPROTO_IP = 0,		/* Dummy protocol for TCP		*/
#define IPPROTO_IP		IPPROTO_IP
  IPPROTO_ICMP = 1,		/* Internet Control Message Protocol	*/
#define IPPROTO_ICMP		IPPROTO_ICMP
  IPPROTO_IGMP = 2,		/* Internet Group Management Protocol	*/
#define IPPROTO_IGMP		IPPROTO_IGMP
  IPPROTO_IPIP = 4,		/* IPIP tunnels (older KA9Q tunnels use 94) */
#define IPPROTO_IPIP		IPPROTO_IPIP
  IPPROTO_TCP = 6,		/* Transmission Control Protocol	*/
#define IPPROTO_TCP		IPPROTO_TCP
  IPPROTO_EGP = 8,		/* Exterior Gateway Protocol		*/
#define IPPROTO_EGP		IPPROTO_EGP
  IPPROTO_PUP = 12,		/* PUP protocol				*/
#define IPPROTO_PUP		IPPROTO_PUP
  IPPROTO_UDP = 17,		/* User Datagram Protocol		*/
#define IPPROTO_UDP		IPPROTO_UDP
  IPPROTO_IDP = 22,		/* XNS IDP protocol			*/
#define IPPROTO_IDP		IPPROTO_IDP
  IPPROTO_TP = 29,		/* SO Transport Protocol Class 4	*/
#define IPPROTO_TP		IPPROTO_TP
  IPPROTO_DCCP = 33,		/* Datagram Congestion Control Protocol */
#define IPPROTO_DCCP		IPPROTO_DCCP
  IPPROTO_IPV6 = 41,		/* IPv6-in-IPv4 tunnelling		*/
#define IPPROTO_IPV6		IPPROTO_IPV6
  IPPROTO_RSVP = 46,		/* RSVP Protocol			*/
#define IPPROTO_RSVP		IPPROTO_RSVP
  IPPROTO_GRE = 47,		/* Cisco GRE tunnels (rfc 1701,1702)	*/
#define IPPROTO_GRE		IPPROTO_GRE
  IPPROTO_ESP = 50,		/* Encapsulation Security Payload protocol */
#define IPPROTO_ESP		IPPROTO_ESP
  IPPROTO_AH = 51,		/* Authentication Header protocol	*/
#define IPPROTO_AH		IPPROTO_AH
  IPPROTO_MTP = 92,		/* Multicast Transport Protocol		*/
#define IPPROTO_MTP		IPPROTO_MTP
  IPPROTO_BEETPH = 94,		/* IP option pseudo header for BEET	*/
#define IPPROTO_BEETPH		IPPROTO_BEETPH
  IPPROTO_ENCAP = 98,		/* Encapsulation Header			*/
#define IPPROTO_ENCAP		IPPROTO_ENCAP
  IPPROTO_PIM = 103,		/* Protocol Independent Multicast	*/
#define IPPROTO_PIM		IPPROTO_PIM
  IPPROTO_COMP = 108,		/* Compression Header Protocol		*/
#define IPPROTO_COMP		IPPROTO_COMP
  IPPROTO_SCTP = 132,		/* Stream Control Transport Protocol	*/
#define IPPROTO_SCTP		IPPROTO_SCTP
  IPPROTO_UDPLITE = 136,	/* UDP-Lite (RFC 3828)			*/
#define IPPROTO_UDPLITE		IPPROTO_UDPLITE
  IPPROTO_RAW = 255,		/* Raw IP packets			*/
#define IPPROTO_RAW		IPPROTO_RAW
  IPPROTO_MAX
};

#define IPPROTO_HOPOPTS		0	/* IPv6 hop-by-hop options	*/
#define IPPROTO_ROUTING		43	/* IPv6 routing header		*/
#define IPPROTO_FRAGMENT	44	/* IPv6 fragmentation header	*/
#define IPPROTO_ICMPV6		58	/* ICMPv6			*/
#define IPPROTO_NONE		59	/* IPv6 no next header		*/
#define IPPROTO_DSTOPTS		60	/* IPv6 destination options	*/
#define IPPROTO_MH		135	/* IPv6 mobility header		*/

#endif //_UAPI_LINUX_IN_H

uint16 model_ipChecksum(struct ip *pip);
uint16 model_tcpChecksum(struct ip *pip, tcpHdr_t *ptcp);
uint16 model_udpChecksum(struct ip *pip, udpHdr_t *pudp);
uint16 model_icmpChecksum(struct ip * pip);
uint16 model_tcpChecksumV6(struct ipv6hdr_s *pipv6, tcpHdr_t *ptcp, uint32 v6ExtLen);
uint16 model_udpChecksumV6(struct ipv6hdr_s *pipv6, udpHdr_t *pudp);

#endif //_RTL_PROTOCOL_