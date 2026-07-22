/*
* protoForm.h --
* 
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.                                                
* 
* $Header: /home/cvsroot/apollo/rtlTest/protoForm.h,v 1.8 2012/10/17 10:27:41 ysleu Exp $
* 
* Program : protoForm Headers
* Abstract: The header file describes the externally-visible
*           protocol forms. These protocols are ether, ipx,
*           arp, ip, icmp, igmp, tcp, udp. We can use them to 
*           form upper-layer packets.
* 
* $Author: ysleu $
*
* $Revision: 1.8 $
* $Id: protoForm.h,v 1.8 2012/10/17 10:27:41 ysleu Exp $
* $Log: protoForm.h,v $
* Revision 1.8  2012/10/17 10:27:41  ysleu
* *: Support ARP packet generation.
*
* Revision 1.7  2012/06/29 04:17:24  ysleu
* *: Modify packet generation.
*
* Revision 1.6  2012/05/28 07:34:36  luke
* *: add IPv6 hopbyhop function
*
* Revision 1.5  2012/05/17 08:39:26  luke
* *: add IPv6 related packet gen
*
* Revision 1.4  2012/05/16 10:46:38  luke
* *: add nextheader and check switch
*
* Revision 1.3  2012/04/23 07:25:17  ysleu
* *: Add SVLAN tpid packet generator configuration parameters.
*
* Revision 1.2  2012/03/14 11:47:39  ysleu
* *: Support SVLAN.
*
* Revision 1.1  2012/03/14 08:11:34  ysleu
* +: Add packet generator.
*
* Revision 1.2  2012/03/06 07:25:47  ysleu
* *: Discard RTL865xC register definition.
*
* Revision 1.1.1.1  2011/12/08 11:15:45  ysleu
* +: APOLLO project initial version.
*
* Revision 1.3  2011/09/23 02:33:43  ysleu
* *: Support DSL VC and Ext. port tx with cpu tag.
*
* Revision 1.2  2011/08/23 03:50:11  ysleu
* *: Modify test cases.
*
* Revision 1.1  2011/06/08 08:21:40  ysleu
* *: Add APOLLO project model and test codes.
*
* Revision 1.5  2007/01/02 01:38:10  caroline_liu
* add generate ipv6 pkt
*
* Revision 1.4  2004/07/19 12:54:43  chenyl
* *: bug fix: process udp packets withc zero-checksum
* *: bug fix: process L3/L4 packets whose ip-header length is not 20 byte
* +: add testing functions to compare/clear checksum field of mbuf/pkthdr
*
* Revision 1.3  2004/06/29 07:34:23  chenyl
* +: IGMPv3
* +: igmp proxy filter :
* 		protocol stack can let igmp-proxy
* 			1: trap specific multicast group
* 			2: ignore specific multicast group
*
* Revision 1.2  2004/04/30 08:58:08  chenyl
* +: ip multicast/igmp proxy
*
* Revision 1.1  2004/02/25 14:27:24  chhuang
* *** empty log message ***
*
* Revision 1.4  2004/02/25 06:34:22  chenyl
* * tcp mss option generate for "SYN" packet
*
* Revision 1.3  2004/02/18 13:12:21  chenyl
* *** empty log message ***
*
* Revision 1.1  2004/02/13 05:18:47  chenyl
* + Pkt generating functions
*
* Revision 1.15  2003/10/06 10:16:23  waynelee
* add pptp gre support
*
* Revision 1.14  2003/09/30 09:40:55  waynelee
* support llc_other
*
* Revision 1.13  2002/11/27 14:37:39  waynelee
* modify protoForm_ether -> protoForm_vsp_ether
*
*/

#ifndef PROTO_FORM_H
#define PROTO_FORM_H

#include "types.h"

/* 
 * general ethernet frames; we can indicate the ethertype of this frame
 */

void protoForm_vsp_ether(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
						uint8 protocol,uint8 reason,uint8 intpri,uint8 extspa,uint8 streamid,uint8 l3r,uint8 org,uint8 extdpmsk,uint8 spa, uint8 fbi, uint16 fbindex,
						uint16 txmsk,uint8 prisel, uint8 pri,uint8 directTx,uint8 keep,uint8 dislrn,uint8 psel,uint8 tx_extspa,uint8 pppoeact,uint8 pppoeidx,uint8 tx_streamid,
						uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority, 
						uint16 cvid, uint8 cvlanCfi, uint16 cpriority, 
						uint8 dsap, uint8 ssap,
						uint16 pppoeType,
						uint16 pppoeSession,
						uint16 etherType,
						int8 * content,
						uint32 * len);

/*
 * IPX packets; only support general IPX/SAP
 * Note: if using l2Flag, only vlan works.
 */

void protoForm_vsp_ipx(int8 * pktBuf, uint32 l2Flag,
					   int8 * da, int8 * sa, 
						uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority, 
						uint16 cvid, uint8 cvlanCfi, uint16 cpriority, 
	//					uint8 dsap, uint8 ssap,
						uint16 pppoeType,
						uint16 pppoeSession,
						uint8 type,
						uint32 dnet, uint8 dnode[6], uint16 dsock,
						uint32 snet, uint8 snode[6], uint16 ssock,
						int8 * content, uint32 * len);

/*
 * ARP packets; including ARP, RARP
 */

void protoForm_vs_arp(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
						uint8 protocol,uint8 reason,uint8 intpri,uint8 extspa, uint8 streamid,uint8 l3r,uint8 org,uint8 extdpmsk,uint8 spaidx, uint8 fbi, uint16 fbindex,
						uint16 txmsk,uint8 prisel, uint8 pri,uint8 directTx,uint8 keep,uint8 dislrn,uint8 psel,uint8 tx_extspa,uint8 pppoeact,uint8 pppoeidx,uint8 tx_streamid,
						uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority, 
						uint16 cvid, uint8 cvlanCfi, uint16 cpriority, 
						int8 * sha, uint32 sip,
						int8 * tha, uint32 tpa,
						uint16 op, uint32 * len);
/*
 * IP packets; len is the length of IP payload
 */

void protoForm_vsp_ip(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
						uint8 protocol,uint8 reason,uint8 intpri,uint8 extspa,uint8 streamid,uint8 l3r,uint8 org,uint8 extdpmsk,uint8 spa, uint8 fbi, uint16 fbindex,
						uint16 txmsk,uint8 prisel, uint8 pri,uint8 directTx,uint8 keep,uint8 dislrn,uint8 psel,uint8 tx_extspa,uint8 pppoeact,uint8 pppoeidx,uint8 tx_streamid,
						uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority, 
						uint16 cvid, uint8 cvlanCfi, uint16 cpriority, 
						uint16 pppoeType,
						uint16 pppoeSession,
						uint8 ipv, uint8 iphl,
						uint8 tos, uint16 ipid, uint8 df, uint8 mf, uint16 ipoff,
						uint8 ttl,uint32 sip, uint32 dip, int8 proto,
						int8 * content,
						uint32 errorFlag, uint32 * len);

void protoForm_vsp_ip_opt(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
						uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority, 
						uint16 cvid, uint8 cvlanCfi, uint16 cpriority, 
						uint16 pppoeType,
						uint16 pppoeSession,
						uint8 ipv, uint8 iphl,
						uint8 tos, uint16 ipid, uint8 df, uint8 mf, uint16 ipoff,
						uint8 ttl,uint32 sip, uint32 dip, int8 proto,
						int8 * content,
						uint32 errorFlag, uint32 * len);

void protoForm_vsp_pptp(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
						uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority, 
						uint16 cvid, uint8 cvlanCfi, uint16 cpriority, 
						uint16 pppoeType,
						uint16 pppoeSession, 
						uint8 ipv, uint8 iphl,
						uint8 tos, uint16 ipid, uint8 df, uint8 mf, uint16 ipoff,
						uint8 ttl, uint32 sip, uint32 dip,
						uint16 callid, uint32 seqno, uint32 ackno,
						int8 * content, 
						uint32 errorFlag, uint32 * len);
/*
 * ICMP packets; len is the length of ICMP payload
 */

void protoForm_vsp_icmp(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
						uint8 protocol,uint8 reason,uint8 intpri,uint8 extspa,uint8 streamid,uint8 l3r,uint8 org,uint8 extdpmsk,uint8 spa, uint8 fbi, uint16 fbindex,
						uint16 txmsk,uint8 prisel, uint8 pri,uint8 directTx,uint8 keep,uint8 dislrn,uint8 psel,uint8 tx_extspa,uint8 pppoeact,uint8 pppoeidx,uint8 tx_streamid,
						uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority, 
						uint16 cvid, uint8 cvlanCfi, uint16 cpriority, 
						uint16 pppoeType,
						uint16 pppoeSession, 
						uint8 ipv, uint8 iphl,
						uint8 tos, uint16 ipid, uint8 df, uint8 mf, uint16 ipoff,
						uint8 ttl, uint32 sip, uint32 dip,
						uint8 type, uint8 code, uint16 id, uint16 seq,
						int8 * content,
						uint32 errorFlag, uint32 * len);
/*
 * IGMP packets; len doesn't work now.
 */

void protoForm_vsp_igmp(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
					uint8 protocol,uint8 reason,uint8 intpri,uint8 extspa,uint8 streamid,uint8 l3r,uint8 org,uint8 extdpmsk,uint8 spa,uint8 fbi, uint16 fbindex,
					uint16 txmsk,uint8 prisel, uint8 pri,uint8 directTx,uint8 keep,uint8 dislrn,uint8 psel,uint8 tx_extspa,uint8 pppoeact,uint8 pppoeidx,uint8 tx_streamid,
					uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority, 
					uint16 cvid, uint8 cvlanCfi, uint16 cpriority, 
					uint16 pppoeType,
					uint16 pppoeSession, 
					uint8 ipv, uint8 iphl,
					uint8 tos, uint16 ipid, uint8 df, uint8 mf, uint16 ipoff,
					uint8 ttl, uint32 sip, uint32 dip,
					uint8 ver, uint8 type, uint8 respTime, uint32 gaddr, 
					uint8 qrsq, uint8 qqic, uint16 qnofs, uint32* qslist, uint16 rnofg,
					int8 * content,
					uint32 errorFlag, uint32 *len);
/*
 * TCP packets; len is the length of TCP payload
 */

void protoForm_vsp_tcp(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
					uint8 protocol,uint8 reason,uint8 intpri,uint8 extspa,uint8 streamid,uint8 l3r,uint8 org,uint8 extdpmsk,uint8 spa, uint8 fbi, uint16 fbindex,
					uint16 txmsk,uint8 prisel, uint8 pri,uint8 directTx,uint8 keep,uint8 dislrn,uint8 psel,uint8 tx_extspa,uint8 pppoeact,uint8 pppoeidx,uint8 tx_streamid,
					uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority, 
					uint16 ctpid,uint16 cvid, uint8 cvlanCfi, uint16 cpriority, 
					uint16 pppoeType,
					uint16 pppoeSession, 
					uint8 ipv, uint8 iphl,
					uint8 tos, uint16 ipid, uint8 df, uint8 mf, uint16 ipoff,
					uint8 ttl, uint32 sip, uint32 dip,
					uint16 sport, uint16 dport,
					uint32 seq, uint32 ack, uint8 tcpFlag, uint16 win, 
					int8 * content, uint32 errorFlag, uint32 * len);

void protoForm_vsp_tcp_option(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
					uint8 protocol,uint8 reason,uint8 intpri,uint8 extspa,uint8 streamid,uint8 l3r,uint8 org,uint8 extdpmsk,uint8 spa, uint8 fbi, uint16 fbindex,
					uint16 txmsk,uint8 prisel, uint8 pri,uint8 directTx,uint8 keep,uint8 dislrn,uint8 psel,uint8 tx_extspa,uint8 pppoeact,uint8 pppoeidx,uint8 tx_streamid,
					uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority, 
					uint16 cvid, uint8 cvlanCfi, uint16 cpriority, 
					uint16 pppoeType,
					uint16 pppoeSession, 
					uint8 ipv, uint8 iphl,
					uint8 tos, uint16 ipid, uint8 df, uint8 mf, uint16 ipoff,
					uint8 ttl, uint32 sip, uint32 dip,
					uint16 sport, uint16 dport,
					uint32 seq, uint32 ack, uint8 tcpFlag, uint16 win, 
					int8 * option, uint32 op_len, int8 * content, uint32 errorFlag, uint32 * len);

void protoForm_vsp_tcp_short_hdr(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa,
					uint8 protocol,uint8 reason,uint8 intpri,uint8 extspa,uint8 streamid,uint8 l3r,uint8 org,uint8 extdpmsk,uint8 spa, uint8 fbi, uint16 fbindex,
					uint16 txmsk,uint8 prisel, uint8 pri,uint8 directTx,uint8 keep,uint8 dislrn,uint8 psel,uint8 tx_extspa,uint8 pppoeact,uint8 pppoeidx,uint8 tx_streamid,
					uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority,
					uint16 cvid, uint8 cvlanCfi, uint16 cpriority,
					uint16 pppoeType,
					uint16 pppoeSession,
					uint8 ipv, uint8 iphl,
					uint8 tos, uint16 ipid, uint8 df, uint8 mf, uint16 ipoff,
					uint8 ttl, uint32 sip, uint32 dip,
					uint16 sport, uint16 dport,
					uint32 seq, uint32 ack, uint8 tcpFlag, uint16 win,
					int8 * content, uint32 errorFlag, uint32 * len);

/*
 * UDP packets; len is the length of UDP payload
 */

void protoForm_vsp_udp(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
					uint8 protocol,uint8 reason,uint8 intpri,uint8 extspa,uint8 streamid,uint8 l3r,uint8 org,uint8 extdpmsk,uint8 spa, uint8 fbi, uint16 fbindex,
					uint16 txmsk,uint8 prisel, uint8 pri,uint8 directTx,uint8 keep,uint8 dislrn,uint8 psel,uint8 tx_extspa,uint8 pppoeact,uint8 pppoeidx,uint8 tx_streamid,
					uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority, 
					uint16 ctpid,uint16 cvid, uint8 cvlanCfi, uint16 cpriority, 
					uint16 pppoeType,
					uint16 pppoeSession,
					uint8 ipv, uint8 iphl,
					uint8 tos, uint16 ipid, uint8 df, uint8 mf, uint16 ipoff,
					uint8 ttl, uint32 sip, uint32 dip,
					uint16 sport, uint16 dport,
					int8 * content, uint32 errorFlag, uint32 * len);

void protoForm_vsp_udp_bomb(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa,
					uint8 protocol,uint8 reason,uint8 intpri,uint8 extspa,uint8 streamid,uint8 l3r,uint8 org,uint8 extdpmsk,uint8 spa, uint8 fbi, uint16 fbindex,
					uint16 txmsk,uint8 prisel, uint8 pri,uint8 directTx,uint8 keep,uint8 dislrn,uint8 psel,uint8 tx_extspa,uint8 pppoeact,uint8 pppoeidx,uint8 tx_streamid,
					uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority,
					uint16 cvid, uint8 cvlanCfi, uint16 cpriority,
					uint16 pppoeType,
					uint16 pppoeSession,
					uint8 ipv, uint8 iphl,
					uint8 tos, uint16 ipid, uint8 df, uint8 mf, uint16 ipoff,
					uint8 ttl, uint32 sip, uint32 dip,
					uint16 sport, uint16 dport,
					int8 * content, uint32 errorFlag, uint32 * len);

void protoForm_vsp_udp_lite(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
					uint8 protocol,uint8 reason,uint8 intpri,uint8 extspa,uint8 streamid,uint8 l3r,uint8 org,uint8 extdpmsk,uint8 spa, uint8 fbi, uint16 fbindex,
					uint16 txmsk,uint8 prisel, uint8 pri,uint8 directTx,uint8 keep,uint8 dislrn,uint8 psel,uint8 tx_extspa,uint8 pppoeact,uint8 pppoeidx,uint8 tx_streamid,
					uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority, 
					uint16 ctpid,uint16 cvid, uint8 cvlanCfi, uint16 cpriority, 
					uint16 pppoeType,
					uint16 pppoeSession,
					uint8 ipv, uint8 iphl,
					uint8 tos, uint16 ipid, uint8 df, uint8 mf, uint16 ipoff,
					uint8 ttl, uint32 sip, uint32 dip,
					uint16 sport, uint16 dport,
					int8 * content, uint32 errorFlag, uint32 * len);

void protoForm_vsp_udp_opt(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
					uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority, 
					uint16 cvid, uint8 cvlanCfi, uint16 cpriority, 
					uint16 pppoeType,
					uint16 pppoeSession,
					uint8 ipv, uint8 iphl,
					uint8 tos, uint16 ipid, uint8 df, uint8 mf, uint16 ipoff,
					uint8 ttl, uint32 sip, uint32 dip,
					uint16 sport, uint16 dport,
					int8 * content, uint32 errorFlag, uint32 * len);
//added by liujuan
void protoForm_vsp_ipv6(int8 * pktBuf, uint32 l2Flag, uint32 extHdr, int8 * da, int8 * sa, 
					uint8 protocol,uint8 reason,uint8 intpri,uint8 extspa,uint8 streamid,uint8 l3r,uint8 org,uint8 extdpmsk,uint8 spa, uint8 fbi, uint16 fbindex,
					uint16 txmsk,uint8 prisel, uint8 pri,uint8 directTx,uint8 keep,uint8 dislrn,uint8 psel,uint8 tx_extspa,uint8 pppoeact,uint8 pppoeidx,uint8 tx_streamid,
					uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority, 
					uint16 cvid, uint8 cvlanCfi, uint16 cpriority, 
					uint16 pppoeType,
					uint16 pppoeSession, 
					uint8 ipv, uint8 tc, uint8 fl[3], uint8 nh, uint8 hl,
					uint8 sip[16], uint8 dip[16], 
					uint8 hdrlen, uint8 segleft, uint8 dstip[16],
					uint16 fragment, uint32 fragmentid,
					int8 * content,
					uint32 errorFlag, uint32 * len);

void protoForm_vsp_v6tcp(int8 * pktBuf, uint32 l2Flag, uint32 extHdr, int8 * da, int8 * sa, 
					uint8 protocol,uint8 reason,uint8 intpri,uint8 extspa,uint8 streamid,uint8 l3r,uint8 org,uint8 extdpmsk,uint8 spa, uint8 fbi, uint16 fbindex,
					uint16 txmsk,uint8 prisel, uint8 pri,uint8 directTx,uint8 keep,uint8 dislrn,uint8 psel,uint8 tx_extspa,uint8 pppoeact,uint8 pppoeidx,uint8 tx_streamid,
					uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority, 
					uint16 cvid, uint8 cvlanCfi, uint16 cpriority, 
					uint16 pppoeType,
					uint16 pppoeSession, 
					uint8 ipv, uint8 tc, uint8 fl[3],  uint8 hl,
					uint8 sip[16], uint8 dip[16], 
					uint8 hdrlen, uint8 segleft, uint8 dstip[16],
					uint16 fragment, uint32 fragmentid,
					uint16 sport, uint16 dport,
					uint32 seq, uint32 ack, uint8 tcpFlag, uint16 win, 
					int8 * content, uint32 errorFlag, uint32 * len);

void protoForm_vsp_v6tcp_option(int8 * pktBuf, uint32 l2Flag, uint32 extHdr, int8 * da, int8 * sa, 
					uint8 protocol,uint8 reason,uint8 intpri,uint8 extspa,uint8 streamid,uint8 l3r,uint8 org,uint8 extdpmsk,uint8 spa, uint8 fbi, uint16 fbindex,
					uint16 txmsk,uint8 prisel, uint8 pri,uint8 directTx,uint8 keep,uint8 dislrn,uint8 psel,uint8 tx_extspa,uint8 pppoeact,uint8 pppoeidx,uint8 tx_streamid,
					uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority, 
					uint16 cvid, uint8 cvlanCfi, uint16 cpriority, 
					uint16 pppoeType,
					uint16 pppoeSession, 
					uint8 ipv, uint8 tc, uint8 fl[3],  uint8 hl,
					uint8 sip[16], uint8 dip[16], 
					uint8 hdrlen, uint8 segleft, uint8 dstip[16],
					uint16 fragment, uint32 fragmentid,
					uint16 sport, uint16 dport,
					uint32 seq, uint32 ack, uint8 tcpFlag, uint16 win, 
					int8 * option, uint32 op_len, int8 * content, uint32 errorFlag, uint32 * len);

void protoForm_vsp_v6tcp_specify_offset(int8 * pktBuf, uint32 l2Flag, uint32 extHdr, int8 * da, int8 * sa, 
					uint8 protocol,uint8 reason,uint8 intpri,uint8 extspa,uint8 streamid,uint8 l3r,uint8 org,uint8 extdpmsk,uint8 spa, uint8 fbi, uint16 fbindex,
					uint16 txmsk,uint8 prisel, uint8 pri,uint8 directTx,uint8 keep,uint8 dislrn,uint8 psel,uint8 tx_extspa,uint8 pppoeact,uint8 pppoeidx,uint8 tx_streamid,
					uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority, 
					uint16 cvid, uint8 cvlanCfi, uint16 cpriority, 
					uint16 pppoeType,
					uint16 pppoeSession, 
					uint8 ipv, uint8 tc, uint8 fl[3],  uint8 hl,
					uint8 sip[16], uint8 dip[16], 
					uint8 hdrlen, uint8 segleft, uint8 dstip[16],
					uint16 fragment, uint32 fragmentid,
					uint16 sport, uint16 dport,
					uint32 seq, uint32 ack, uint8 tcpOffset, uint8 tcpFlag, uint16 win, 
					int8 * content, uint32 errorFlag, uint32 * len);

void protoForm_vsp_v6tcp_short_hdr(int8 * pktBuf, uint32 l2Flag, uint32 extHdr, int8 * da, int8 * sa,
					uint8 protocol,uint8 reason,uint8 intpri,uint8 extspa,uint8 streamid,uint8 l3r,uint8 org,uint8 extdpmsk,uint8 spa, uint8 fbi, uint16 fbindex,
					uint16 txmsk,uint8 prisel, uint8 pri,uint8 directTx,uint8 keep,uint8 dislrn,uint8 psel,uint8 tx_extspa,uint8 pppoeact,uint8 pppoeidx,uint8 tx_streamid,
					uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority,
					uint16 cvid, uint8 cvlanCfi, uint16 cpriority,
					uint16 pppoeType,
					uint16 pppoeSession,
					uint8 ipv, uint8 tc, uint8 fl[3],  uint8 hl,
					uint8 sip[16], uint8 dip[16],
					uint8 hdrlen, uint8 segleft, uint8 dstip[16],
					uint16 fragment, uint32 fragmentid,
					uint16 sport, uint16 dport,
					uint32 seq, uint32 ack, uint8 tcpFlag, uint16 win,
					int8 * content, uint32 errorFlag, uint32 * len);

void protoForm_vsp_v6udp(int8 * pktBuf, uint32 l2Flag, uint32 extHdr, int8 * da, int8 * sa, 
					uint8 protocol,uint8 reason,uint8 intpri,uint8 extspa,uint8 streamid,uint8 l3r,uint8 org,uint8 extdpmsk,uint8 spa, uint8 fbi, uint16 fbindex,
					uint16 txmsk,uint8 prisel, uint8 pri,uint8 directTx,uint8 keep,uint8 dislrn,uint8 psel,uint8 tx_extspa,uint8 pppoeact,uint8 pppoeidx,uint8 tx_streamid,
					uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority, 
					uint16 cvid, uint8 cvlanCfi, uint16 cpriority, 
					uint16 pppoeType,
					uint16 pppoeSession,
					uint8 ipv, uint8 tc, uint8 fl[3],  uint8 hl,
					uint8 sip[16], uint8 dip[16], 
					uint8 hdrlen, uint8 segleft, uint8 dstip[16],
					uint16 fragment, uint32 fragmentid,
					uint16 sport, uint16 dport,
					int8 * content, uint32 errorFlag, uint32 * len);

void protoForm_vsp_v6udp_bomb(int8 * pktBuf, uint32 l2Flag, uint32 extHdr, int8 * da, int8 * sa,
					uint8 protocol,uint8 reason,uint8 intpri,uint8 extspa,uint8 streamid,uint8 l3r,uint8 org,uint8 extdpmsk,uint8 spa, uint8 fbi, uint16 fbindex,
					uint16 txmsk,uint8 prisel, uint8 pri,uint8 directTx,uint8 keep,uint8 dislrn,uint8 psel,uint8 tx_extspa,uint8 pppoeact,uint8 pppoeidx,uint8 tx_streamid,
					uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority,
					uint16 cvid, uint8 cvlanCfi, uint16 cpriority,
					uint16 pppoeType,
					uint16 pppoeSession,
					uint8 ipv, uint8 tc, uint8 fl[3],  uint8 hl,
					uint8 sip[16], uint8 dip[16],
					uint8 hdrlen, uint8 segleft, uint8 dstip[16],
					uint16 fragment, uint32 fragmentid,
					uint16 sport, uint16 dport,
					int8 * content, uint32 errorFlag, uint32 * len);

void protoForm_vsp_v6udp_lite(int8 * pktBuf, uint32 l2Flag, uint32 extHdr, int8 * da, int8 * sa, 
					uint8 protocol,uint8 reason,uint8 intpri,uint8 extspa,uint8 streamid,uint8 l3r,uint8 org,uint8 extdpmsk,uint8 spa, uint8 fbi, uint16 fbindex,
					uint16 txmsk,uint8 prisel, uint8 pri,uint8 directTx,uint8 keep,uint8 dislrn,uint8 psel,uint8 tx_extspa,uint8 pppoeact,uint8 pppoeidx,uint8 tx_streamid,
					uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority, 
					uint16 cvid, uint8 cvlanCfi, uint16 cpriority, 
					uint16 pppoeType,
					uint16 pppoeSession,
					uint8 ipv, uint8 tc, uint8 fl[3],  uint8 hl,
					uint8 sip[16], uint8 dip[16], 
					uint8 hdrlen, uint8 segleft, uint8 dstip[16],
					uint16 fragment, uint32 fragmentid,
					uint16 sport, uint16 dport,
					int8 * content, uint32 errorFlag, uint32 * len);

void protoForm_vsp_v6icmp(int8 * pktBuf, uint32 l2Flag, uint32 extHdr, int8 * da, int8 * sa, 
					uint8 protocol,uint8 reason,uint8 intpri,uint8 extspa,uint8 streamid,uint8 l3r,uint8 org,uint8 extdpmsk,uint8 spa, uint8 fbi, uint16 fbindex,
					uint16 txmsk,uint8 prisel, uint8 pri,uint8 directTx,uint8 keep,uint8 dislrn,uint8 psel,uint8 tx_extspa,uint8 pppoeact,uint8 pppoeidx,uint8 tx_streamid,
					uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority, 
					uint16 cvid, uint8 cvlanCfi, uint16 cpriority, 
					uint16 pppoeType,
					uint16 pppoeSession, 
					uint8 ipv, uint8 tc, uint8 fl[3],  uint8 hl,
					uint8 sip[16], uint8 dip[16], 
					uint8 hdrlen, uint8 segleft, uint8 dstip[16],
					uint16 fragment, uint32 fragmentid,
					uint8 type, uint8 code, uint16 id, uint16 seq,
					int8 * content,
					uint32 errorFlag, uint32 * len);

void protoForm_vsp_v6hop(int8 * pktBuf, uint32 l2Flag, uint32 extHdr, int8 * da, int8 * sa, 
					uint8 protocol,uint8 reason,uint8 intpri,uint8 extspa,uint8 streamid,uint8 l3r,uint8 org,uint8 extdpmsk,uint8 spa, uint8 fbi, uint16 fbindex,
					uint16 txmsk,uint8 prisel, uint8 pri,uint8 directTx,uint8 keep,uint8 dislrn,uint8 psel,uint8 tx_extspa,uint8 pppoeact,uint8 pppoeidx,uint8 tx_streamid,
					uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority, 
					uint16 cvid, uint8 cvlanCfi, uint16 cpriority, 
					uint16 pppoeType,
					uint16 pppoeSession, 
					uint8 ipv, uint8 tc, uint8 fl[3],  uint8 hl,
					uint8 sip[16], uint8 dip[16], 
					uint8 hdrlen, uint8 segleft, uint8 dstip[16],
					uint16 fragment, uint32 fragmentid,
					uint16 sport, uint16 dport,
					uint32 seq, uint32 ack, uint8 tcpFlag, uint16 win, 
					int8 * content, uint32 errorFlag, uint32 * len);

void protoForm_vsp_v6pptp(int8 * pktBuf, uint32 l2Flag, uint32 extHdr, int8 * da, int8 * sa, 
					uint8 protocol,uint8 reason,uint8 intpri,uint8 extspa,uint8 streamid,uint8 l3r,uint8 org,uint8 extdpmsk,uint8 spa, uint8 fbi, uint16 fbindex,
					uint16 txmsk,uint8 prisel, uint8 pri,uint8 directTx,uint8 keep,uint8 dislrn,uint8 psel,uint8 tx_extspa,uint8 pppoeact,uint8 pppoeidx,uint8 tx_streamid,
					uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority, 
					uint16 cvid, uint8 cvlanCfi, uint16 cpriority, 
					uint16 pppoeType,
					uint16 pppoeSession, 
					uint8 ipv, uint8 tc, uint8 fl[3], uint8 nh, uint8 hl,
					uint8 sip[16], uint8 dip[16], 
					uint8 hdrlen, uint8 segleft, uint8 dstip[16],
					uint16 fragment, uint32 fragmentid,
					uint16 callid, uint32 seqno, uint32 ackno,
					int8 * content,
					uint32 errorFlag, uint32 * len);

void protoForm_vsp_v6tcp_option_specify_offset(int8 * pktBuf, uint32 l2Flag, uint32 extHdr, int8 * da, int8 * sa, 
					uint8 protocol,uint8 reason,uint8 intpri,uint8 extspa,uint8 streamid,uint8 l3r,uint8 org,uint8 extdpmsk,uint8 spa, uint8 fbi, uint16 fbindex,
					uint16 txmsk,uint8 prisel, uint8 pri,uint8 directTx,uint8 keep,uint8 dislrn,uint8 psel,uint8 tx_extspa,uint8 pppoeact,uint8 pppoeidx,uint8 tx_streamid,
					uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority, 
					uint16 cvid, uint8 cvlanCfi, uint16 cpriority, 
					uint16 pppoeType,
					uint16 pppoeSession, 
					uint8 ipv, uint8 tc, uint8 fl[3],  uint8 hl,
					uint8 sip[16], uint8 dip[16], 
					uint8 hdrlen, uint8 segleft, uint8 dstip[16],
					uint16 fragment, uint32 fragmentid,
					uint16 sport, uint16 dport,
					uint32 seq, uint32 ack, uint8 tcpOffset, uint8 tcpFlag, uint16 win, 
					int8 * option, uint32 op_len, int8 * content, uint32 errorFlag, uint32 * len);


#endif

