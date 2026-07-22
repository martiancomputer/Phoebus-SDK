/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.                                                
* 
* Program : protoForm
*
* Original Creator: Edward Jin-Ru Chen 2002/4/29
* $Author: ysleu $
*
* $Revision: 1.11 $
* $Id: protoForm.c,v 1.11 2012/10/17 10:27:41 ysleu Exp $
* $Log: protoForm.c,v $
* Revision 1.11  2012/10/17 10:27:41  ysleu
* *: Support ARP packet generation.
*
* Revision 1.10  2012/06/29 04:17:24  ysleu
* *: Modify packet generation.
*
* Revision 1.9  2012/06/19 07:13:08  ysleu
* *: Sync model & ASIC.
*
* Revision 1.8  2012/05/28 07:34:36  luke
* *: add IPv6 hopbyhop function
*
* Revision 1.7  2012/05/17 08:39:26  luke
* *: add IPv6 related packet gen
*
* Revision 1.6  2012/05/16 10:46:38  luke
* *: add nextheader and check switch
*
* Revision 1.5  2012/04/23 07:25:17  ysleu
* *: Add SVLAN tpid packet generator configuration parameters.
*
* Revision 1.4  2012/04/16 11:19:54  ysleu
* *: Modify HSA SVLAN fields DSVID/DSPRI to array.
*
* Revision 1.3  2012/04/02 09:53:58  ysleu
* *: Modify translator for SVLAN support.
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
* Revision 1.7  2007/01/02 01:38:07  caroline_liu
* add generate ipv6 pkt
*
* Revision 1.6  2006/04/28 11:05:43  yjlou
* *: By default, we do not clear packet buffer to speed up
*
* Revision 1.5  2005/07/20 15:29:38  yjlou
* +: porting Model Code to Linux Kernel: check RTL865X_MODEL_KERNEL.
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
* Revision 1.17  2003/10/06 10:16:23  waynelee
* add pptp gre support
*
* Revision 1.16  2003/09/30 09:40:55  waynelee
* support llc_other
*
* Revision 1.15  2002/11/27 14:37:39  waynelee
* modify protoForm_ether -> protoForm_vsp_ether
*
* Revision 1.14  2002/10/20 07:32:20  waynelee
* users can define the content of payload (but only indicate one byte)
*
* Revision 1.13  2002/10/16 02:44:15  waynelee
* tcp header length 24 bytes -> 20 bytes
*
* Revision 1.12  2002/10/07 12:29:12  waynelee
* user-defined vcfi (vlan CFI) and L4 No checksum
*
* Revision 1.11  2002/10/04 14:30:01  waynelee
* support user-defined ipv and iphl
*
* Revision 1.10  2002/10/02 15:12:31  waynelee
* ipx support debug
*
* Revision 1.9  2002/10/01 07:47:33  waynelee
* support user-defined ipid, df, mf, ipoff
*
* Revision 1.8  2002/09/16 07:31:16  waynelee
* support IPX (experimental)
*
*/

#include "rtl_glue.h"
#include "protoForm.h"
#include "pktForm.h"
#include "l4pkt.h"
#include "l3pkt.h"
#ifdef __KERNEL__
#include <linux/string.h>
#else
#include <stdio.h>
#include <string.h>
#endif

extern void dump_packet(uint8 *pkt,uint32 size,char *memo);

/*  CLEAR_PKTBUF
 *  If defined, always clean pktBuf before build packet
 *  Not defined, ignore cleaning packet buffer to speed up
 */
#undef CLEAN_PKTBUF

void protoForm_vsp_ether(int8 * pktBuf, uint32 l2Flag,	int8 * da, int8 * sa, 
						uint8 protocol,uint8 reason,uint8 intpri,uint8 extspa,uint8 streamid,uint8 l3r,uint8 org,uint8 extdpmsk,uint8 spa, uint8 fbi, uint16 fbindex,
						uint16 txmsk,uint8 prisel, uint8 pri,uint8 directTx,uint8 keep,uint8 dislrn,uint8 psel,uint8 tx_extspa,uint8 pppoeact,uint8 pppoeidx,uint8 tx_streamid,
						uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority, 
						uint16 cvid, uint8 cvlanCfi, uint16 cpriority, 
						uint8 dsap, uint8 ssap,
						uint16 pppoeType,
						uint16 pppoeSession,
						uint16 etherType,
						int8 * content,
						uint32 * len) {
	uint32 pktFmt;

	pktFmt = (l2Flag & (L2_MASK | L2_CPUTAG_MASK)) ;
#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormCTagOther(pktFmt,pktBuf,protocol,reason,intpri,streamid,l3r,org,extdpmsk,spa,extspa,fbi,fbindex);
	l2FormCTagTX2(pktFmt,pktBuf,protocol,txmsk,prisel,pri,keep,directTx,dislrn,psel,tx_extspa,pppoeact,pppoeidx,tx_streamid);
	l2FormSVlanTag(pktFmt, pktBuf, tpid, svid, svlanDei, spriority);
	l2FormCVlanTag(pktFmt, pktBuf,0x8100, cvid, cvlanCfi, cpriority);
	l2FormLlc(pktFmt, dsap, ssap, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l2FormOtherEtherType(pktFmt, pktBuf, etherType, content, *len);

	*len = formedFrameLen(pktFmt, pktBuf, *len);
	l2FormCrc(pktFmt, pktBuf, *len);
}


void protoForm_vsp_ipx(int8 * pktBuf, uint32 l2Flag,
					   int8 * da, int8 * sa, 
						uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority, 
						uint16 cvid, uint8 cvlanCfi, uint16 cpriority, 
						uint16 pppoeType,
						uint16 pppoeSession,
						uint8 type,
						uint32 dnet, uint8 dnode[6], uint16 dsock,
						uint32 snet, uint8 snode[6], uint16 ssock,
						int8 * content, uint32 * len) {
	uint32 pktFmt;

	pktFmt = (l2Flag & L2_MASK) | L3_IPX ;
#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0, 2048);
#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormSVlanTag(pktFmt, pktBuf, tpid, svid, svlanDei, spriority);
	l2FormCVlanTag(pktFmt, pktBuf,0x8100, cvid, cvlanCfi, cpriority);
	l2FormLlc(pktFmt, 0xe0, 0xe0, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	// printf("before ipxb address: %02x%02x\n",pktBuf[20],pktBuf[21]);
	l3FormIpxBasic(pktFmt, type, pktBuf);
	// printf("before ipxd address: %02x%02x\n",pktBuf[20],pktBuf[21]);
	l3FormIpxDnet(pktFmt, dnet, dnode, dsock, pktBuf);
	// printf("before ipxs address: %02x%02x\n",pktBuf[20],pktBuf[21]);
	l3FormIpxSnet(pktFmt, snet, snode, ssock, pktBuf);
	// printf("after ipx address: %02x%02x\n",pktBuf[20],pktBuf[21]);

	l3FormIpxContent(pktFmt,pktBuf,content,*len);
	// printf("after ipxc address: %02x%02x\n",pktBuf[20],pktBuf[21]);

	*len = formedPktLen(pktFmt, pktBuf);
	l2FormCrc(pktFmt, pktBuf, *len);
}


void protoForm_vs_arp(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
						uint8 protocol,uint8 reason,uint8 intpri,uint8 extspa, uint8 streamid,uint8 l3r,uint8 org,uint8 extdpmsk,uint8 spaidx, uint8 fbi, uint16 fbindex,
						uint16 txmsk,uint8 prisel, uint8 pri,uint8 directTx,uint8 keep,
						uint8 dislrn,uint8 psel,uint8 tx_extspa,uint8 pppoeact,uint8 pppoeidx,uint8 tx_streamid,
						uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority, 
						uint16 cvid, uint8 cvlanCfi, uint16 cpriority, 
						int8 * sha, uint32 sip,
						int8 * tha, uint32 tpa,
						uint16 op, uint32 * len) {
	uint32 pktFmt;
	int8	zero[6]={0};

		pktFmt = (l2Flag & (L2_MASK | L2_CPUTAG_MASK)) | L3_ARP; //pppoe is ignored automatically
	
#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormCTagOther(pktFmt,pktBuf,protocol,reason,intpri,streamid,l3r,org,extdpmsk,spaidx,extspa,fbi,fbindex);
	l2FormCTagTX2(pktFmt,pktBuf,protocol,txmsk,prisel,pri,keep,directTx,dislrn,psel,tx_extspa,pppoeact,pppoeidx,tx_streamid);
	l2FormSVlanTag(pktFmt, pktBuf, tpid, svid, svlanDei, spriority);
	l2FormCVlanTag(pktFmt, pktBuf,0x8100, cvid, cvlanCfi, cpriority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l3FormArpBasic(pktFmt, op, pktBuf);

	// if users set "sha", use "sha"; if not, use "sa" as "sha"
	if(memcmp(sha,zero,6)==0)
		l3FormArpSelfInfo(pktFmt, pktBuf, sip, sa);
	else
		l3FormArpSelfInfo(pktFmt, pktBuf, sip, sha);

	// ARP: if users set "tha", use "tha" ; if not, use "da" as "tha"
	// RARP: if users set "tha", use "tha" ; if not, use "sa" as "tha" 
	if(memcmp(tha,zero,6)==0)
		if(op == ARPOP_REVREQUEST)
			l3FormArpPeerInfo(pktFmt, pktBuf, op, tpa, sa);
		else
			l3FormArpPeerInfo(pktFmt, pktBuf, op, tpa, da);
	else
		l3FormArpPeerInfo(pktFmt, pktBuf, op, tpa, tha);


	*len = formedPktLen(pktFmt, pktBuf);
	l2FormCrc(pktFmt, pktBuf, *len);
}

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
						uint32 errorFlag, uint32 * len) {
	uint32 pktFmt;

		pktFmt = (l2Flag & (L2_MASK | L2_CPUTAG_MASK)) | L3_IP | L4_ICMP | (errorFlag&CTL_MASK);

#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormCTagOther(pktFmt,pktBuf,protocol,reason,intpri,streamid,l3r,org,extdpmsk,spa,extspa,fbi,fbindex);
	l2FormCTagTX2(pktFmt,pktBuf,protocol,txmsk,prisel,pri,keep,directTx,dislrn,psel,tx_extspa,pppoeact,pppoeidx,tx_streamid);
	l2FormSVlanTag(pktFmt, pktBuf, tpid, svid, svlanDei, spriority);
	l2FormCVlanTag(pktFmt, pktBuf,0x8100, cvid, cvlanCfi, cpriority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpBasic(pktFmt, pktBuf, ipv, iphl); //Version 4, header length 20
	l3FormIpIdFrag(pktFmt, pktBuf, ipid, df, mf, ipoff/8);
	// l3FormIpIdFlag(uint32 flag, int8 * pktBuf, uint16 id, uint8 df, uint8 mf, uint16 offset)
	l3FormIpFlow(pktFmt, pktBuf, tos, ttl); //TOS 0, TTL 16
	l3FormIpAddr(pktFmt, pktBuf, sip, dip);
	l3FormIpProtocol(pktFmt, pktBuf, proto);
	l3FormIpContent(pktFmt, pktBuf, content, *len);

	*len = formedPktLen(pktFmt, pktBuf);
	l2FormCrc(pktFmt, pktBuf, *len);
}

void protoForm_vsp_ip_opt(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
						uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority, 
						uint16 cvid, uint8 cvlanCfi, uint16 cpriority, 
						uint16 pppoeType,
						uint16 pppoeSession,
						uint8 ipv, uint8 iphl,
						uint8 tos, uint16 ipid, uint8 df, uint8 mf, uint16 ipoff,
						uint8 ttl,uint32 sip, uint32 dip, int8 proto,
						int8 * content,
						uint32 errorFlag, uint32 * len) {
	uint32 pktFmt;
	uint8	opt[4];

	// Add: router alert Option
	opt[0] = 0x94;
	opt[1] = 0x04;
	opt[2] = 0;
	opt[3] = 0;

	pktFmt = (l2Flag & L2_MASK) | L3_IP | L4_ICMP | (errorFlag&CTL_MASK);
#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormSVlanTag(pktFmt, pktBuf, tpid, svid, svlanDei, spriority);
	l2FormCVlanTag(pktFmt, pktBuf,0x8100, cvid, cvlanCfi, cpriority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpBasic(pktFmt, pktBuf, ipv, iphl); //Version 4, header length 20
	l3FormIpIdFrag(pktFmt, pktBuf, ipid, df, mf, ipoff/8);
	// l3FormIpIdFlag(uint32 flag, int8 * pktBuf, uint16 id, uint8 df, uint8 mf, uint16 offset)
	l3FormIpFlow(pktFmt, pktBuf, tos, ttl); //TOS 0, TTL 16
	l3FormIpAddr(pktFmt, pktBuf, sip, dip);
	l3FormIpProtocol(pktFmt, pktBuf, proto);
	l3FormIpContent(pktFmt, pktBuf, content, *len);

	// option
	l3FormIpOption(pktFmt, pktBuf, opt, 4);

	*len = formedPktLen(pktFmt, pktBuf);
	l2FormCrc(pktFmt, pktBuf, *len);
}


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
						uint32 errorFlag, uint32 * len) {

	uint32 pktFmt;

	pktFmt = (l2Flag & L2_MASK) | L3_IP | L4_PPTP | (errorFlag&CTL_MASK);
#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormSVlanTag(pktFmt, pktBuf, tpid, svid, svlanDei, spriority);
	l2FormCVlanTag(pktFmt, pktBuf,0x8100, cvid, cvlanCfi, cpriority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpBasic(pktFmt, pktBuf, ipv, iphl); //Version 4, header length 20
	l3FormIpIdFrag(pktFmt, pktBuf, ipid, df, mf, ipoff/8);
	l3FormIpFlow(pktFmt, pktBuf, tos, ttl); //TOS 0, TTL 16
	l3FormIpAddr(pktFmt, pktBuf, sip, dip);

	l4FormPptpBasic(pktFmt, pktBuf, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0x880B);	//cheney: change recur bit to 0
	l4FormPptpKey(pktFmt, pktBuf, callid, seqno, ackno);
	l4FormPptpOther(pktFmt, pktBuf, content, *len);
	*len = formedPktLen(pktFmt, pktBuf);

	if(l2Flag & DUAL_OUTER_HDR)
	{
		l3FormIpTotalLenZero(pktFmt, pktBuf);
	}
	l2FormCrc(pktFmt, pktBuf, *len);
}

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
						uint32 errorFlag, uint32 * len) {
	uint32 pktFmt;

		pktFmt = (l2Flag & (L2_MASK | L2_CPUTAG_MASK)) | L3_IP | L4_ICMP | (errorFlag&CTL_MASK);

#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormCTagOther(pktFmt,pktBuf,protocol,reason,intpri,streamid,l3r,org,extdpmsk,spa,extspa,fbi,fbindex);
	l2FormSVlanTag(pktFmt, pktBuf, tpid, svid, svlanDei, spriority);
	l2FormCVlanTag(pktFmt, pktBuf,0x8100, cvid, cvlanCfi, cpriority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpBasic(pktFmt, pktBuf, ipv, iphl); //Version 4, header length 20
	l3FormIpIdFrag(pktFmt, pktBuf, ipid, df, mf, ipoff/8);
	l3FormIpFlow(pktFmt, pktBuf, tos, ttl); //TOS 0, TTL 16
	l3FormIpAddr(pktFmt, pktBuf, sip, dip);

	l4FormIcmpBasic(pktFmt, pktBuf, type, code);
	l4FormIcmpEcho(pktFmt, pktBuf, id, seq);

	l4FormIcmpData(pktFmt, pktBuf, content, *len);

	*len = formedPktLen(pktFmt, pktBuf);
	l2FormCrc(pktFmt, pktBuf, *len);
}

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
					uint32 errorFlag, uint32 *len) {
	uint32 pktFmt;
	uint8	opt[4];

	// router alert Option
	opt[0] = 0x94;
	opt[1] = 0x04;
	opt[2] = 0;
	opt[3] = 0;

	pktFmt = (l2Flag & (L2_MASK | L2_CPUTAG_MASK)) | L3_IP | L4_IGMP | (errorFlag&CTL_MASK);
#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormCTagOther(pktFmt,pktBuf,protocol,reason,intpri,streamid,l3r,org,extdpmsk,spa,extspa,fbi,fbindex);
	l2FormCTagTX2(pktFmt,pktBuf,protocol,txmsk,prisel,pri,keep,directTx,dislrn,psel,tx_extspa,pppoeact,pppoeidx,tx_streamid);
	l2FormSVlanTag(pktFmt, pktBuf, tpid, svid, svlanDei, spriority);
	l2FormCVlanTag(pktFmt, pktBuf,0x8100, cvid, cvlanCfi, cpriority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpBasic(pktFmt, pktBuf, ipv, iphl); //Version 4, header length 20
	l3FormIpIdFrag(pktFmt, pktBuf, ipid, df, mf, ipoff/8);
	l3FormIpFlow(pktFmt, pktBuf, tos, ttl); //TOS 0
	l3FormIpAddr(pktFmt, pktBuf, sip, dip);

	if (ver >= 2)	// igmpv1 don't set Router Alert Option in packet
		l3FormIpOption(pktFmt, pktBuf, opt, 4);

	l4FormIgmpBasic(pktFmt, pktBuf, ver, type, respTime, gaddr, qrsq, qqic, qnofs, qslist, rnofg);
	l4FormIgmpContent(pktFmt, pktBuf, content, *len, ver, type, qnofs);

	*len = formedPktLen(pktFmt, pktBuf);
	l2FormCrc(pktFmt, pktBuf, *len);
}

void protoForm_vsp_tcp(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
					uint8 protocol,uint8 reason,uint8 intpri,uint8 extspa,uint8 streamid,uint8 l3r,uint8 org,uint8 extdpmsk,uint8 spa,uint8 fbi, uint16 fbindex,
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
					int8 * content, uint32 errorFlag, uint32 * len) {
	uint32 pktFmt;
	pktFmt = (l2Flag & (L2_MASK | L2_CPUTAG_MASK)) | L3_IP | L4_TCP | (errorFlag&CTL_MASK);

#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormCTagOther(pktFmt,pktBuf,protocol,reason,intpri,streamid,l3r,org,extdpmsk,spa,extspa,fbi,fbindex);
	l2FormCTagTX2(pktFmt,pktBuf,protocol,txmsk,prisel,pri,keep,directTx,dislrn,psel,tx_extspa,pppoeact,pppoeidx,tx_streamid);
	//l2FormOtherVlanTag(pktFmt, pktBuf,ctpid, cvid, cvlanCfi, cpriority); //WEN FIX ME
	l2FormSVlanTag(pktFmt, pktBuf, tpid, svid, svlanDei, spriority);
	l2FormCVlanTag(pktFmt, pktBuf,ctpid, cvid, cvlanCfi, cpriority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpBasic(pktFmt, pktBuf, ipv, iphl); //Version 4, header length 20
	l3FormIpIdFrag(pktFmt, pktBuf, ipid, df, mf, ipoff/8);
	l3FormIpFlow(pktFmt, pktBuf, tos, ttl); //TOS 0, TTL 16
	l3FormIpAddr(pktFmt, pktBuf, sip, dip);
	l4FormTcpBasic(pktFmt, pktBuf, sport, dport, 20);
	l4FormTcpFlow(pktFmt, pktBuf, seq, ack, win, tcpFlag, 0);
//	l4FormTcpFlow(pktFmt, pktBuf, seq, ack, win, ack?TH_ACK:0, 0);
	l4FormTcpContent(pktFmt, pktBuf, content, *len);

	*len = formedPktLen(pktFmt, pktBuf);
	l2FormCrc(pktFmt, pktBuf, *len);
}

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
					int8 * option, uint32 op_len, int8 * content, uint32 errorFlag, uint32 * len) {
	uint32 pktFmt;

	pktFmt = (l2Flag & (L2_MASK | L2_CPUTAG_MASK)) | L3_IP | L4_TCP | (errorFlag&CTL_MASK);
#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormCTagOther(pktFmt,pktBuf,protocol,reason,intpri,streamid,l3r,org,extdpmsk,spa,extspa,fbi,fbindex);
	l2FormCTagTX2(pktFmt,pktBuf,protocol,txmsk,prisel,pri,keep,directTx,dislrn,psel,tx_extspa,pppoeact,pppoeidx,tx_streamid);
	l2FormSVlanTag(pktFmt, pktBuf, tpid, svid, svlanDei, spriority);
	l2FormCVlanTag(pktFmt, pktBuf,0x8100 ,cvid, cvlanCfi, cpriority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpBasic(pktFmt, pktBuf, ipv, iphl); //Version 4, header length 20
	l3FormIpIdFrag(pktFmt, pktBuf, ipid, df, mf, ipoff/8);
	l3FormIpFlow(pktFmt, pktBuf, tos, ttl); //TOS 0, TTL 16
	l3FormIpAddr(pktFmt, pktBuf, sip, dip);

	l4FormTcpBasic(pktFmt, pktBuf, sport, dport, (20+op_len));
	l4FormTcpFlow(pktFmt, pktBuf, seq, ack, win, tcpFlag, 0);
//	l4FormTcpFlow(pktFmt, pktBuf, seq, ack, win, ack?TH_ACK:0, 0);
	l4FormTcpOption(pktFmt, pktBuf, option, op_len);
	l4FormTcpContent(pktFmt, pktBuf, content, *len);

	*len = formedPktLen(pktFmt, pktBuf);
	l2FormCrc(pktFmt, pktBuf, *len);
}

void protoForm_vsp_tcp_short_hdr(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa,
					uint8 protocol,uint8 reason,uint8 intpri,uint8 extspa,uint8 streamid,uint8 l3r,uint8 org,uint8 extdpmsk,uint8 spa,uint8 fbi, uint16 fbindex,
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
					int8 * content, uint32 errorFlag, uint32 * len) {
	uint32 pktFmt;
	pktFmt = (l2Flag & (L2_MASK | L2_CPUTAG_MASK)) | L3_IP | L4_TCP | (errorFlag&CTL_MASK);

#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormCTagOther(pktFmt,pktBuf,protocol,reason,intpri,streamid,l3r,org,extdpmsk,spa,extspa,fbi,fbindex);
	l2FormCTagTX2(pktFmt,pktBuf,protocol,txmsk,prisel,pri,keep,directTx,dislrn,psel,tx_extspa,pppoeact,pppoeidx,tx_streamid);
	l2FormSVlanTag(pktFmt, pktBuf, tpid, svid, svlanDei, spriority);
	l2FormCVlanTag(pktFmt, pktBuf, 0x8100, cvid, cvlanCfi, cpriority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpBasic(pktFmt, pktBuf, ipv, iphl); //Version 4, header length 20
	l3FormIpIdFrag(pktFmt, pktBuf, ipid, df, mf, ipoff/8);
	l3FormIpFlow(pktFmt, pktBuf, tos, ttl); //TOS 0, TTL 16
	l3FormIpAddr(pktFmt, pktBuf, sip, dip);
	l4FormTcpBasic(pktFmt, pktBuf, sport, dport, 16);
	l4FormTcpFlow(pktFmt, pktBuf, seq, ack, win, tcpFlag, 0);
//	l4FormTcpFlow(pktFmt, pktBuf, seq, ack, win, ack?TH_ACK:0, 0);
	l4FormTcpContent(pktFmt, pktBuf, content, *len);

	*len = formedPktLen(pktFmt, pktBuf);
	l2FormCrc(pktFmt, pktBuf, *len);
}


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
					int8 * content, uint32 errorFlag, uint32 * len) {
	uint32 pktFmt;

	pktFmt = (l2Flag & (L2_MASK | L2_CPUTAG_MASK)) | L3_IP | L4_UDP | (errorFlag&CTL_MASK);
#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormCTagOther(pktFmt,pktBuf,protocol,reason,intpri,streamid,l3r,org,extdpmsk,spa,extspa,fbi,fbindex);
	l2FormCTagTX2(pktFmt,pktBuf,protocol,txmsk,prisel,pri,keep,directTx,dislrn,psel,tx_extspa,pppoeact,pppoeidx,tx_streamid);
	l2FormSVlanTag(pktFmt, pktBuf, tpid, svid, svlanDei, spriority);
	l2FormCVlanTag(pktFmt, pktBuf,ctpid ,cvid, cvlanCfi, cpriority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpBasic(pktFmt, pktBuf, ipv, iphl); //Version 4, header length 20
	l3FormIpIdFrag(pktFmt, pktBuf, ipid, df, mf, ipoff/8);
	l3FormIpFlow(pktFmt, pktBuf, tos, ttl); //TOS 0, TTL 16
	l3FormIpAddr(pktFmt, pktBuf, sip, dip);

	l4FormUdpBasic(pktFmt, pktBuf, sport, dport);
	l4FormUdpContent(pktFmt, pktBuf, content, *len);

	*len = formedPktLen(pktFmt, pktBuf);

	if(l2Flag & DUAL_OUTER_HDR)
	{
		pktFmt |= DUAL_OUTER_HDR;
		l4FormUdpLenZero(pktFmt, pktBuf);
	}
	
	l2FormCrc(pktFmt, pktBuf, *len);
}

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
					int8 * content, uint32 errorFlag, uint32 * len) {
	uint32 pktFmt;

	pktFmt = (l2Flag & (L2_MASK | L2_CPUTAG_MASK)) | L3_IP | L4_UDP | (errorFlag&CTL_MASK);
#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormCTagOther(pktFmt,pktBuf,protocol,reason,intpri,streamid,l3r,org,extdpmsk,spa,extspa,fbi,fbindex);
	l2FormCTagTX2(pktFmt,pktBuf,protocol,txmsk,prisel,pri,keep,directTx,dislrn,psel,tx_extspa,pppoeact,pppoeidx,tx_streamid);
	l2FormSVlanTag(pktFmt, pktBuf, tpid, svid, svlanDei, spriority);
	l2FormCVlanTag(pktFmt, pktBuf,0x8100, cvid, cvlanCfi, cpriority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpBasic(pktFmt, pktBuf, ipv, iphl); //Version 4, header length 20
	l3FormIpIdFrag(pktFmt, pktBuf, ipid, df, mf, ipoff/8);
	l3FormIpFlow(pktFmt, pktBuf, tos, ttl); //TOS 0, TTL 16
	l3FormIpAddr(pktFmt, pktBuf, sip, dip);

	l4FormUdpBasic(pktFmt, pktBuf, sport, dport);
	l4FormUdpContent_for_udp_bomb(pktFmt, pktBuf, content, *len);

	*len = formedPktLen(pktFmt, pktBuf);

	if(l2Flag & DUAL_OUTER_HDR)
	{
		pktFmt |= DUAL_OUTER_HDR;
		l4FormUdpLenZero(pktFmt, pktBuf);
	}

	l2FormCrc(pktFmt, pktBuf, *len);
}

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
					int8 * content, uint32 errorFlag, uint32 * len) {
	uint32 pktFmt;

	pktFmt = (l2Flag & (L2_MASK | L2_CPUTAG_MASK)) | L3_IP | L4_UDP | (errorFlag&CTL_MASK);
#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormCTagOther(pktFmt,pktBuf,protocol,reason,intpri,streamid,l3r,org,extdpmsk,spa,extspa,fbi,fbindex);
	l2FormCTagTX2(pktFmt,pktBuf,protocol,txmsk,prisel,pri,keep,directTx,dislrn,psel,tx_extspa,pppoeact,pppoeidx,tx_streamid);
	l2FormSVlanTag(pktFmt, pktBuf, tpid, svid, svlanDei, spriority);
	l2FormCVlanTag(pktFmt, pktBuf,ctpid ,cvid, cvlanCfi, cpriority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpBasic(pktFmt, pktBuf, ipv, iphl); //Version 4, header length 20
	l3FormIpIdFrag(pktFmt, pktBuf, ipid, df, mf, ipoff/8);
	l3FormIpFlow(pktFmt, pktBuf, tos, ttl); //TOS 0, TTL 16
	l3FormIpAddr(pktFmt, pktBuf, sip, dip);

	l4FormUdpLiteBasic(pktFmt, pktBuf, sport, dport);
	l4FormUdpContent(pktFmt, pktBuf, content, *len);

	*len = formedPktLen(pktFmt, pktBuf);

	if(l2Flag & DUAL_OUTER_HDR)
	{
		pktFmt |= DUAL_OUTER_HDR;
		l4FormUdpLenZero(pktFmt, pktBuf);
	}
	
	l2FormCrc(pktFmt, pktBuf, *len);
}


void protoForm_vsp_udp_opt(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
					uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority, 
					uint16 cvid, uint8 cvlanCfi, uint16 cpriority, 
					uint16 pppoeType,
					uint16 pppoeSession,
					uint8 ipv, uint8 iphl,
					uint8 tos, uint16 ipid, uint8 df, uint8 mf, uint16 ipoff,
					uint8 ttl, uint32 sip, uint32 dip,
					uint16 sport, uint16 dport,
					int8 * content, uint32 errorFlag, uint32 * len) {
	uint32 pktFmt;
	uint8	opt[4];

	// Add: router alert Option
	opt[0] = 0x94;
	opt[1] = 0x04;
	opt[2] = 0;
	opt[3] = 0;

	pktFmt = (l2Flag & L2_MASK) | L3_IP | L4_UDP | (errorFlag&CTL_MASK);
#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormSVlanTag(pktFmt, pktBuf, tpid, svid, svlanDei, spriority);
	l2FormCVlanTag(pktFmt, pktBuf,0x8100, cvid, cvlanCfi, cpriority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpBasic(pktFmt, pktBuf, ipv, iphl); //Version 4, header length 20
	l3FormIpIdFrag(pktFmt, pktBuf, ipid, df, mf, ipoff/8);
	l3FormIpFlow(pktFmt, pktBuf, tos, ttl); //TOS 0, TTL 16
	l3FormIpAddr(pktFmt, pktBuf, sip, dip);

	// option
	l3FormIpOption(pktFmt, pktBuf, opt, 4);

	l4FormUdpBasic(pktFmt, pktBuf, sport, dport);
	l4FormUdpContent(pktFmt, pktBuf, content, *len);

	*len = formedPktLen(pktFmt, pktBuf);
	l2FormCrc(pktFmt, pktBuf, *len);
}

//added by liujuan
void protoForm_vsp_ipv6(int8 * pktBuf, uint32 l2Flag, uint32 extHdr,int8 * da, int8 * sa, 
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
					uint32 errorFlag, uint32 * len){

	uint32 pktFmt;
	uint32 lasthdr;
	//uint8  nh;
	pktFmt = (l2Flag & (L2_MASK | L2_CPUTAG_MASK | ehMASK)) |L3_IPV6|(extHdr & ehMASK)| (errorFlag&CTL_MASK);
	#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
	#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormCTagOther(pktFmt,pktBuf,protocol,reason,intpri,streamid,l3r,org,extdpmsk,spa,extspa,fbi,fbindex);
	l2FormCTagTX2(pktFmt,pktBuf,protocol,txmsk,prisel,pri,keep,directTx,dislrn,psel,tx_extspa,pppoeact,pppoeidx,tx_streamid);
	l2FormSVlanTag(pktFmt, pktBuf, tpid, svid, svlanDei, spriority);
	l2FormCVlanTag(pktFmt, pktBuf,0x8100, cvid, cvlanCfi, cpriority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpv6Basic(pktFmt, pktBuf, ipv);
	l3FormIpv6Flow(pktFmt,  pktBuf, tc, fl, hl);
	//l3FormIpv6NextHeader(pktFmt, pktBuf, nh);
	l3FormIpv6Addr(pktFmt, pktBuf, sip, dip);

	//luke
	switch(nh)
	{
		case 1:
			pktFmt |= L4_ICMP;
			break;
		case 2:
			pktFmt |= L4_IGMP;
			break;
		case 17:
			pktFmt |= L4_UDP;
			break;
		case 6:
			pktFmt |= L4_TCP;
			break;
		case 47:
			pktFmt |= L3_V6GRE;
			break;
	}
	
	lasthdr=l3FormIpv6ExtHdr(pktFmt, pktBuf,hdrlen, segleft, dstip, fragment, fragmentid); 
	//nh=2;	//IGMPv4
	if (lasthdr==L3_IPV6)
		l3FormIpv6NextHeader(pktFmt, pktBuf, nh);
	else
		l3FormLastHeader(pktFmt, pktBuf, hdrlen, lasthdr, nh);
	if ((lasthdr&NoNHdr)==0)
		l3FormIpv6Content(pktFmt, pktBuf, hdrlen, content, *len);
//	rtlglue_printf("the last header is [%d]\n", lasthdr);
//	rtlglue_printf("the pktlen before formedpktLenV6 is [%d]\n", *len);
	*len = formedpktLenV6(pktFmt, pktBuf, hdrlen); 
//	rtlglue_printf("the pktlen after formedpktLenV6 is [%d]\n", *len);
	if (*len < 60)
		*len = 60;
	l2FormCrc(pktFmt, pktBuf, *len);
//	*len+=4;

}		

//added by liujuan
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
					int8 * content, uint32 errorFlag, uint32 * len){
	uint32 pktFmt;
	uint32 lasthdr;
	uint8  nh;
	pktFmt = (l2Flag & (L2_MASK | L2_CPUTAG_MASK | ehMASK)) |L3_IPV6|L4_TCP|(extHdr & ehMASK)| (errorFlag&CTL_MASK);
	#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
	#endif
	l2FormEtherAddress(pktBuf, da, sa);	
	l2FormCTagOther(pktFmt,pktBuf,protocol,reason,intpri,streamid,l3r,org,extdpmsk,spa,extspa,fbi,fbindex);
	l2FormCTagTX2(pktFmt,pktBuf,protocol,txmsk,prisel,pri,keep,directTx,dislrn,psel,tx_extspa,pppoeact,pppoeidx,tx_streamid);
	l2FormSVlanTag(pktFmt, pktBuf, tpid, svid, svlanDei, spriority);
	l2FormCVlanTag(pktFmt, pktBuf,0x8100, cvid, cvlanCfi, cpriority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpv6Basic(pktFmt, pktBuf, ipv);
	l3FormIpv6Flow(pktFmt,  pktBuf, tc, fl, hl);
	//l3FormIpv6NextHeader(pktFmt, pktBuf, nh);
	l3FormIpv6Addr(pktFmt, pktBuf, sip, dip);
	lasthdr=l3FormIpv6ExtHdr(pktFmt, pktBuf,hdrlen, segleft, dstip, fragment, fragmentid); 
	nh=6;
//	rtlglue_printf("the last header is [%d]\n", lasthdr);
	if (lasthdr==L3_IPV6)       // ??????????????????????????????
		l3FormIpv6NextHeader(pktFmt, pktBuf, nh);
	
	else 
		l3FormLastHeader(pktFmt, pktBuf, hdrlen, lasthdr, nh);
		
//	if(lasthdr!=NoNHdr) {
//		rtlglue_printf("begin form tcp!\n");
		l4FormTcpBasicV6(pktFmt, pktBuf, hdrlen, sport, dport, 20, dstip, segleft);
//		rtlglue_printf("after form tcp basic\n");
		l4FormTcpFlowV6(pktFmt, pktBuf, hdrlen, seq, ack, win, tcpFlag, 0, dstip, segleft);
		l4FormTcpContentV6(pktFmt, pktBuf, hdrlen, content, *len, dstip, segleft);

//	}
//	rtlglue_printf("the pktlen before formedpktLenV6 is [%d]\n", *len);
	*len = formedpktLenV6(pktFmt, pktBuf,hdrlen);
//	rtlglue_printf("the pktlen after formedpktLenV6 is [%d]\n", *len);
	if(l2Flag & DUAL_OUTER_HDR)
	{
		l3FormIpv6TotalLenZero(pktFmt, pktBuf);
	}
	l2FormCrc(pktFmt, pktBuf, *len);
//	rtlglue_printf("the length after from crc is [%d]\n", *len);
}

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
					int8 * option, uint32 op_len, int8 * content, uint32 errorFlag, uint32 * len){
	uint32 pktFmt;
	uint32 lasthdr;
	uint8  nh;
	pktFmt = (l2Flag & (L2_MASK | L2_CPUTAG_MASK | ehMASK)) |L3_IPV6|L4_TCP|(extHdr & ehMASK)| (errorFlag&CTL_MASK);
#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
#endif
	l2FormEtherAddress(pktBuf, da, sa); 
	l2FormCTagOther(pktFmt,pktBuf,protocol,reason,intpri,streamid,l3r,org,extdpmsk,spa,extspa,fbi,fbindex);
	l2FormCTagTX2(pktFmt,pktBuf,protocol,txmsk,prisel,pri,keep,directTx,dislrn,psel,tx_extspa,pppoeact,pppoeidx,tx_streamid);
	l2FormSVlanTag(pktFmt, pktBuf, tpid, svid, svlanDei, spriority);
	l2FormCVlanTag(pktFmt, pktBuf,0x8100, cvid, cvlanCfi, cpriority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpv6Basic(pktFmt, pktBuf, ipv);
	l3FormIpv6Flow(pktFmt,	pktBuf, tc, fl, hl);
	//l3FormIpv6NextHeader(pktFmt, pktBuf, nh);
	l3FormIpv6Addr(pktFmt, pktBuf, sip, dip);
	lasthdr=l3FormIpv6ExtHdr(pktFmt, pktBuf,hdrlen, segleft, dstip, fragment, fragmentid); 
	nh=6;
//	rtlglue_printf("the last header is [%d]\n", lasthdr);
	if (lasthdr==L3_IPV6)		// ??????????????????????????????
		l3FormIpv6NextHeader(pktFmt, pktBuf, nh);
	
	else 
		l3FormLastHeader(pktFmt, pktBuf, hdrlen, lasthdr, nh);
		
//	if(lasthdr!=NoNHdr) {
//		rtlglue_printf("begin form tcp!\n");
		l4FormTcpBasicV6(pktFmt, pktBuf, hdrlen, sport, dport, (20+op_len), dstip, segleft);
//		rtlglue_printf("after form tcp basic\n");
		l4FormTcpFlowV6(pktFmt, pktBuf, hdrlen, seq, ack, win, tcpFlag, 0, dstip, segleft);
		l4FormTcpOptionV6(pktFmt, pktBuf, hdrlen, option, op_len, dstip, segleft);
		l4FormTcpContentV6(pktFmt, pktBuf, hdrlen, content, *len, dstip, segleft);

//	}
//	rtlglue_printf("the pktlen before formedpktLenV6 is [%d]\n", *len);
	*len = formedpktLenV6(pktFmt, pktBuf,hdrlen);
//	rtlglue_printf("the pktlen after formedpktLenV6 is [%d]\n", *len);
	if(l2Flag & DUAL_OUTER_HDR)
	{
		l3FormIpv6TotalLenZero(pktFmt, pktBuf);
	}
	l2FormCrc(pktFmt, pktBuf, *len);
//	rtlglue_printf("the length after from crc is [%d]\n", *len);
}

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
					int8 * option, uint32 op_len, int8 * content, uint32 errorFlag, uint32 * len){
	uint32 pktFmt;
	uint32 lasthdr;
	uint8  nh;
	pktFmt = (l2Flag & (L2_MASK | L2_CPUTAG_MASK | ehMASK)) |L3_IPV6|L4_TCP|(extHdr & ehMASK)| (errorFlag&CTL_MASK);
#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
#endif
	l2FormEtherAddress(pktBuf, da, sa); 
	l2FormCTagOther(pktFmt,pktBuf,protocol,reason,intpri,streamid,l3r,org,extdpmsk,spa,extspa,fbi,fbindex);
	l2FormCTagTX2(pktFmt,pktBuf,protocol,txmsk,prisel,pri,keep,directTx,dislrn,psel,tx_extspa,pppoeact,pppoeidx,tx_streamid);
	l2FormSVlanTag(pktFmt, pktBuf, tpid, svid, svlanDei, spriority);
	l2FormCVlanTag(pktFmt, pktBuf,0x8100, cvid, cvlanCfi, cpriority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpv6Basic(pktFmt, pktBuf, ipv);
	l3FormIpv6Flow(pktFmt,	pktBuf, tc, fl, hl);
	//l3FormIpv6NextHeader(pktFmt, pktBuf, nh);
	l3FormIpv6Addr(pktFmt, pktBuf, sip, dip);
	lasthdr=l3FormIpv6ExtHdr(pktFmt, pktBuf,hdrlen, segleft, dstip, fragment, fragmentid); 
	nh=6;
//	rtlglue_printf("the last header is [%d]\n", lasthdr);
	if (lasthdr==L3_IPV6)		// ??????????????????????????????
		l3FormIpv6NextHeader(pktFmt, pktBuf, nh);
	
	else 
		l3FormLastHeader(pktFmt, pktBuf, hdrlen, lasthdr, nh);
		
//	if(lasthdr!=NoNHdr) {
//		rtlglue_printf("begin form tcp!\n");
		l4FormTcpBasicV6_specify_offset(pktFmt, pktBuf, hdrlen, sport, dport, 20, dstip, segleft, tcpOffset);
//		rtlglue_printf("after form tcp basic\n");
		l4FormTcpFlowV6(pktFmt, pktBuf, hdrlen, seq, ack, win, tcpFlag, 0, dstip, segleft);
		l4FormTcpOptionV6(pktFmt, pktBuf, hdrlen, option, op_len, dstip, segleft);
		l4FormTcpContentV6(pktFmt, pktBuf, hdrlen, content, *len, dstip, segleft);

//	}
//	rtlglue_printf("the pktlen before formedpktLenV6 is [%d]\n", *len);
	*len = formedpktLenV6(pktFmt, pktBuf,hdrlen);
//	rtlglue_printf("the pktlen after formedpktLenV6 is [%d]\n", *len);
	if(l2Flag & DUAL_OUTER_HDR)
	{
		l3FormIpv6TotalLenZero(pktFmt, pktBuf);
	}
	l2FormCrc(pktFmt, pktBuf, *len);
//	rtlglue_printf("the length after from crc is [%d]\n", *len);
}


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
					int8 * content, uint32 errorFlag, uint32 * len){
					return protoForm_vsp_v6tcp_option_specify_offset(pktBuf, l2Flag, extHdr, da, sa, 
										protocol, reason, intpri, extspa, streamid, l3r, org, extdpmsk, spa, fbi, fbindex,
										txmsk, prisel, pri, directTx, keep, dislrn, psel, tx_extspa, pppoeact, pppoeidx, tx_streamid,
										tpid,  svid,  svlanDei,  spriority, 
										cvid,  cvlanCfi,  cpriority, 
										pppoeType,
										pppoeSession, 
										ipv, tc, fl, hl,
										sip, dip, 
										hdrlen, segleft, dstip,
										fragment, fragmentid,
										sport, dport,
										seq, ack, tcpOffset, tcpFlag, win, 
										NULL, 0, content, errorFlag, len);						
}

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
					int8 * content, uint32 errorFlag, uint32 * len){
	uint32 pktFmt;
	uint32 lasthdr;
	uint8  nh;
	pktFmt = (l2Flag & (L2_MASK | L2_CPUTAG_MASK | ehMASK)) |L3_IPV6|L4_TCP|(extHdr & ehMASK)| (errorFlag&CTL_MASK);
#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormCTagOther(pktFmt,pktBuf,protocol,reason,intpri,streamid,l3r,org,extdpmsk,spa,extspa,fbi,fbindex);
	l2FormCTagTX2(pktFmt,pktBuf,protocol,txmsk,prisel,pri,keep,directTx,dislrn,psel,tx_extspa,pppoeact,pppoeidx,tx_streamid);
	l2FormSVlanTag(pktFmt, pktBuf, tpid, svid, svlanDei, spriority);
	l2FormCVlanTag(pktFmt, pktBuf,0x8100, cvid, cvlanCfi, cpriority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpv6Basic(pktFmt, pktBuf, ipv);
	l3FormIpv6Flow(pktFmt,	pktBuf, tc, fl, hl);
	//l3FormIpv6NextHeader(pktFmt, pktBuf, nh);
	l3FormIpv6Addr(pktFmt, pktBuf, sip, dip);
	lasthdr=l3FormIpv6ExtHdr(pktFmt, pktBuf,hdrlen, segleft, dstip, fragment, fragmentid);
	nh=6;
//	rtlglue_printf("the last header is [%d]\n", lasthdr);
	if (lasthdr==L3_IPV6)		// ??????????????????????????????
		l3FormIpv6NextHeader(pktFmt, pktBuf, nh);

	else
		l3FormLastHeader(pktFmt, pktBuf, hdrlen, lasthdr, nh);

//	if(lasthdr!=NoNHdr) {
//		rtlglue_printf("begin form tcp!\n");
		l4FormTcpBasicV6(pktFmt, pktBuf, hdrlen, sport, dport, 16, dstip, segleft);
//		rtlglue_printf("after form tcp basic\n");
		l4FormTcpFlowV6(pktFmt, pktBuf, hdrlen, seq, ack, win, tcpFlag, 0, dstip, segleft);
		l4FormTcpContentV6(pktFmt, pktBuf, hdrlen, content, *len, dstip, segleft);

//	}
//	rtlglue_printf("the pktlen before formedpktLenV6 is [%d]\n", *len);
	*len = formedpktLenV6(pktFmt, pktBuf,hdrlen);
//	rtlglue_printf("the pktlen after formedpktLenV6 is [%d]\n", *len);
	if(l2Flag & DUAL_OUTER_HDR)
	{
		l3FormIpv6TotalLenZero(pktFmt, pktBuf);
	}
	l2FormCrc(pktFmt, pktBuf, *len);
//	rtlglue_printf("the length after from crc is [%d]\n", *len);
}

//added by liujuan
void protoForm_vsp_v6udp(int8 * pktBuf, uint32 l2Flag, uint32 extHdr, int8 * da, int8 * sa,
					uint8 protocol,uint8 reason,uint8 intpri,uint8 extspa,uint8 streamid,uint8 l3r,uint8 org,uint8 extdpmsk,uint8 spa, uint8 fbi, uint16 fbindex,
					uint16 txmsk,uint8 prisel, uint8 pri,uint8 directTx,uint8 keep,uint8 dislrn,uint8 psel,uint8 tx_extspa,uint8 pppoeact,uint8 pppoeidx,uint8 tx_streamid,
					uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority,
					uint16 cvid, uint8 cvlanCfi, uint16 cpriority,
					uint16 pppoeType,
					uint16 pppoeSession,
					uint8 ipv, uint8 tc, uint8 fl[3], uint8 hl,
					uint8 sip[16], uint8 dip[16],
					uint8 hdrlen, uint8 segleft, uint8 dstip[16],
					uint16 fragment, uint32 fragmentid,
					uint16 sport, uint16 dport,
					int8 * content, uint32 errorFlag, uint32 * len){
	uint32 pktFmt;
	uint32 lasthdr;
	uint8  nh;
	pktFmt = (l2Flag & (L2_MASK | L2_CPUTAG_MASK | ehMASK)) |L3_IPV6|L4_UDP|(extHdr & ehMASK)| (errorFlag&CTL_MASK);
	#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
	#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormCTagOther(pktFmt,pktBuf,protocol,reason,intpri,streamid,l3r,org,extdpmsk,spa,extspa,fbi,fbindex);
	l2FormCTagTX2(pktFmt,pktBuf,protocol,txmsk,prisel,pri,keep,directTx,dislrn,psel,tx_extspa,pppoeact,pppoeidx,tx_streamid);
	l2FormSVlanTag(pktFmt, pktBuf, tpid, svid, svlanDei, spriority);
	l2FormCVlanTag(pktFmt, pktBuf,0x8100, cvid, cvlanCfi, cpriority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpv6Basic(pktFmt, pktBuf, ipv);
	l3FormIpv6Flow(pktFmt,  pktBuf, tc, fl, hl);
	//l3FormIpv6NextHeader(pktFmt, pktBuf, nh);
	l3FormIpv6Addr(pktFmt, pktBuf, sip, dip);
	lasthdr=l3FormIpv6ExtHdr(pktFmt, pktBuf,hdrlen, segleft, dstip, fragment, fragmentid); 
	nh=17;
	if (lasthdr==L3_IPV6)      
		l3FormIpv6NextHeader(pktFmt, pktBuf, nh);
	else 
		l3FormLastHeader(pktFmt, pktBuf, hdrlen, lasthdr, nh);

	if (lasthdr!=NoNHdr) {
		l4FormUdpBasicV6(pktFmt, pktBuf, hdrlen, sport, dport, dstip, segleft);
		l4FormUdpContentV6(pktFmt, pktBuf, hdrlen, content, *len,dstip, segleft);
	}

	*len=formedpktLenV6(pktFmt, pktBuf, hdrlen);
	l2FormCrc(pktFmt, pktBuf, *len);

}

void protoForm_vsp_v6udp_bomb(int8 * pktBuf, uint32 l2Flag, uint32 extHdr, int8 * da, int8 * sa,
					uint8 protocol,uint8 reason,uint8 intpri,uint8 extspa,uint8 streamid,uint8 l3r,uint8 org,uint8 extdpmsk,uint8 spa, uint8 fbi, uint16 fbindex,
					uint16 txmsk,uint8 prisel, uint8 pri,uint8 directTx,uint8 keep,uint8 dislrn,uint8 psel,uint8 tx_extspa,uint8 pppoeact,uint8 pppoeidx,uint8 tx_streamid,
					uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority,
					uint16 cvid, uint8 cvlanCfi, uint16 cpriority,
					uint16 pppoeType,
					uint16 pppoeSession,
					uint8 ipv, uint8 tc, uint8 fl[3], uint8 hl,
					uint8 sip[16], uint8 dip[16],
					uint8 hdrlen, uint8 segleft, uint8 dstip[16],
					uint16 fragment, uint32 fragmentid,
					uint16 sport, uint16 dport,
					int8 * content, uint32 errorFlag, uint32 * len){
	uint32 pktFmt;
	uint32 lasthdr;
	uint8  nh;
	pktFmt = (l2Flag & (L2_MASK | L2_CPUTAG_MASK | ehMASK)) |L3_IPV6|L4_UDP|(extHdr & ehMASK)| (errorFlag&CTL_MASK);
#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormCTagOther(pktFmt,pktBuf,protocol,reason,intpri,streamid,l3r,org,extdpmsk,spa,extspa,fbi,fbindex);
	l2FormCTagTX2(pktFmt,pktBuf,protocol,txmsk,prisel,pri,keep,directTx,dislrn,psel,tx_extspa,pppoeact,pppoeidx,tx_streamid);
	l2FormSVlanTag(pktFmt, pktBuf, tpid, svid, svlanDei, spriority);
	l2FormCVlanTag(pktFmt, pktBuf,0x8100, cvid, cvlanCfi, cpriority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpv6Basic(pktFmt, pktBuf, ipv);
	l3FormIpv6Flow(pktFmt,	pktBuf, tc, fl, hl);
	//l3FormIpv6NextHeader(pktFmt, pktBuf, nh);
	l3FormIpv6Addr(pktFmt, pktBuf, sip, dip);
	lasthdr=l3FormIpv6ExtHdr(pktFmt, pktBuf,hdrlen, segleft, dstip, fragment, fragmentid);
	nh=17;
	if (lasthdr==L3_IPV6)
		l3FormIpv6NextHeader(pktFmt, pktBuf, nh);

	else
		l3FormLastHeader(pktFmt, pktBuf, hdrlen, lasthdr, nh);

	if (lasthdr!=NoNHdr) {
		l4FormUdpBasicV6(pktFmt, pktBuf, hdrlen, sport, dport, dstip, segleft);
		l4FormUdpContentV6_for_udp_bomb(pktFmt, pktBuf, hdrlen, content, *len,dstip, segleft);
	}

	*len=formedpktLenV6(pktFmt, pktBuf, hdrlen);
	l2FormCrc(pktFmt, pktBuf, *len);

}

void protoForm_vsp_v6udp_lite(int8 * pktBuf, uint32 l2Flag, uint32 extHdr, int8 * da, int8 * sa,
					uint8 protocol,uint8 reason,uint8 intpri,uint8 extspa,uint8 streamid,uint8 l3r,uint8 org,uint8 extdpmsk,uint8 spa, uint8 fbi, uint16 fbindex,
					uint16 txmsk,uint8 prisel, uint8 pri,uint8 directTx,uint8 keep,uint8 dislrn,uint8 psel,uint8 tx_extspa,uint8 pppoeact,uint8 pppoeidx,uint8 tx_streamid,
					uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority,
					uint16 cvid, uint8 cvlanCfi, uint16 cpriority,
					uint16 pppoeType,
					uint16 pppoeSession,
					uint8 ipv, uint8 tc, uint8 fl[3], uint8 hl,
					uint8 sip[16], uint8 dip[16],
					uint8 hdrlen, uint8 segleft, uint8 dstip[16],
					uint16 fragment, uint32 fragmentid,
					uint16 sport, uint16 dport,
					int8 * content, uint32 errorFlag, uint32 * len){
	uint32 pktFmt;
	uint32 lasthdr;
	uint8  nh;
	pktFmt = (l2Flag & (L2_MASK | L2_CPUTAG_MASK | ehMASK)) |L3_IPV6|L4_UDP|(extHdr & ehMASK)| (errorFlag&CTL_MASK);
	#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
	#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormCTagOther(pktFmt,pktBuf,protocol,reason,intpri,streamid,l3r,org,extdpmsk,spa,extspa,fbi,fbindex);
	l2FormCTagTX2(pktFmt,pktBuf,protocol,txmsk,prisel,pri,keep,directTx,dislrn,psel,tx_extspa,pppoeact,pppoeidx,tx_streamid);
	l2FormSVlanTag(pktFmt, pktBuf, tpid, svid, svlanDei, spriority);
	l2FormCVlanTag(pktFmt, pktBuf,0x8100, cvid, cvlanCfi, cpriority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpv6Basic(pktFmt, pktBuf, ipv);
	l3FormIpv6Flow(pktFmt,  pktBuf, tc, fl, hl);
	//l3FormIpv6NextHeader(pktFmt, pktBuf, nh);
	l3FormIpv6Addr(pktFmt, pktBuf, sip, dip);
	lasthdr=l3FormIpv6ExtHdr(pktFmt, pktBuf,hdrlen, segleft, dstip, fragment, fragmentid); 
	nh=0x88;
	if (lasthdr==L3_IPV6)      
		l3FormIpv6NextHeader(pktFmt, pktBuf, nh);
	else 
		l3FormLastHeader(pktFmt, pktBuf, hdrlen, lasthdr, nh);

	if (lasthdr!=NoNHdr) {
		l4FormUdpBasicV6(pktFmt, pktBuf, hdrlen, sport, dport, dstip, segleft);
		l4FormUdpContentV6(pktFmt, pktBuf, hdrlen, content, *len,dstip, segleft);
	}

	*len=formedpktLenV6(pktFmt, pktBuf, hdrlen);
	l2FormCrc(pktFmt, pktBuf, *len);

}

extern void memDump (void *start, uint32 size, int8 * strHeader);
//added by liujuan
void protoForm_vsp_v6icmp(int8 * pktBuf, uint32 l2Flag, uint32 extHdr, int8 * da, int8 * sa, 
					uint8 protocol,uint8 reason,uint8 intpri,uint8 extspa,uint8 streamid,uint8 l3r,uint8 org,uint8 extdpmsk,uint8 spa, uint8 fbi, uint16 fbindex,
					uint16 txmsk,uint8 prisel, uint8 pri,uint8 directTx,uint8 keep,uint8 dislrn,uint8 psel,uint8 tx_extspa,uint8 pppoeact,uint8 pppoeidx,uint8 tx_streamid,
					uint16 tpid, uint16 svid, uint8 svlanDei, uint16 spriority, 
					uint16 cvid, uint8 cvlanCfi, uint16 cpriority, 
					uint16 pppoeType,
					uint16 pppoeSession, 
					uint8 ipv, uint8 tc, uint8 fl[3], uint8 hl,
					uint8 sip[16], uint8 dip[16], 
					uint8 hdrlen, uint8 segleft, uint8 dstip[16],
					uint16 fragment, uint32 fragmentid,
					uint8 type, uint8 code, uint16 id, uint16 seq,
					int8 * content,
					uint32 errorFlag, uint32 * len){
	uint32 pktFmt;
	uint32 lasthdr;
	uint8 nh;
	pktFmt = (l2Flag & (L2_MASK | L2_CPUTAG_MASK | ehMASK)) |L3_IPV6|L4_ICMP|(extHdr & ehMASK)| (errorFlag&CTL_MASK);
	#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
	#endif
	l2FormEtherAddress(pktBuf, da, sa);	
	l2FormCTagOther(pktFmt,pktBuf,protocol,reason,intpri,streamid,l3r,org,extdpmsk,spa,extspa,fbi,fbindex);
	l2FormCTagTX2(pktFmt,pktBuf,protocol,txmsk,prisel,pri,keep,directTx,dislrn,psel,tx_extspa,pppoeact,pppoeidx,tx_streamid);
	l2FormSVlanTag(pktFmt, pktBuf, tpid, svid, svlanDei, spriority);
	l2FormCVlanTag(pktFmt, pktBuf,0x8100, cvid, cvlanCfi, cpriority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpv6Basic(pktFmt, pktBuf, ipv);
	l3FormIpv6Flow(pktFmt, pktBuf,tc, fl, hl);
	l3FormIpv6Addr(pktFmt, pktBuf, sip, dip);
//	rtlglue_printf("the flag in v6icmp is [%d]\n", pktFmt);
	lasthdr=l3FormIpv6ExtHdr(pktFmt, pktBuf, hdrlen, segleft, dstip, fragment, fragmentid);	
//	rtlglue_printf("the len in proto is [%d]\n", *len);
	nh=58;
//	lasthdr=L3_IPV6;
	if(lasthdr==L3_IPV6)
		l3FormIpv6NextHeader(pktFmt, pktBuf, nh);
	else
		l3FormLastHeader(pktFmt, pktBuf, hdrlen, lasthdr, nh);
	if (lasthdr!=NoNHdr) {
//		rtlglue_printf("********in form l4 v6 icmp***********\n");
		l4FormIcmpBasicV6(pktFmt, pktBuf, hdrlen, type, code, dstip, segleft);
//		memDump(pktBuf, 62, NULL);
		l4FormIcmpEchoV6(pktFmt, pktBuf, hdrlen, id, seq, dstip, segleft);
//		memDump(pktBuf, 62, NULL);
		l4FormIcmpDataV6(pktFmt, pktBuf, hdrlen, content, *len, dstip, segleft);
//		memDump(pktBuf, 95, NULL);
	} 

	*len=formedpktLenV6(pktFmt, pktBuf, hdrlen);
//	rtlglue_printf("the length after formpktlenv6 is [%d]\n", *len);
	l2FormCrc(pktFmt, pktBuf, *len);
//	rtlglue_printf("the length after form crc is [%d]\n", *len);
//	memDump(pktBuf, *len, NULL);
	
}

//added by luke
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
					int8 * content, uint32 errorFlag, uint32 * len){
	uint32 pktFmt;
	uint32 lasthdr;
	uint8  nh;
	pktFmt = (l2Flag & (L2_MASK | L2_CPUTAG_MASK | ehMASK)) |L3_IPV6|hopbyhopHdr|L4_TCP|(extHdr & ehMASK)| (errorFlag&CTL_MASK);
	#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
	#endif
	l2FormEtherAddress(pktBuf, da, sa);	
	l2FormCTagOther(pktFmt,pktBuf,protocol,reason,intpri,streamid,l3r,org,extdpmsk,spa,extspa,fbi,fbindex);
	l2FormCTagTX2(pktFmt,pktBuf,protocol,txmsk,prisel,pri,keep,directTx,dislrn,psel,tx_extspa,pppoeact,pppoeidx,tx_streamid);
	l2FormSVlanTag(pktFmt, pktBuf, tpid, svid, svlanDei, spriority);
	l2FormCVlanTag(pktFmt, pktBuf,0x8100, cvid, cvlanCfi, cpriority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpv6Basic(pktFmt, pktBuf, ipv);
	l3FormIpv6Flow(pktFmt,  pktBuf, tc, fl, hl);
	//l3FormIpv6NextHeader(pktFmt, pktBuf, nh);
	l3FormIpv6Addr(pktFmt, pktBuf, sip, dip);
	lasthdr=l3FormIpv6ExtHdr(pktFmt, pktBuf,hdrlen, segleft, dstip, fragment, fragmentid); 
	nh=6;
//	rtlglue_printf("the last header is [%d]\n", lasthdr);
	if (lasthdr==L3_IPV6)       // ??????????????????????????????
		l3FormIpv6NextHeader(pktFmt, pktBuf, nh);
	
	else 
		l3FormLastHeader(pktFmt, pktBuf, hdrlen, lasthdr, nh);
		
//	if(lasthdr!=NoNHdr) {
//		rtlglue_printf("begin form tcp!\n");
		l4FormTcpBasicV6(pktFmt, pktBuf, hdrlen, sport, dport, 20, dstip, segleft);
//		rtlglue_printf("after form tcp basic\n");
		l4FormTcpFlowV6(pktFmt, pktBuf, hdrlen, seq, ack, win, tcpFlag, 0, dstip, segleft);
		l4FormTcpContentV6(pktFmt, pktBuf, hdrlen, content, *len, dstip, segleft);

//	}
//	rtlglue_printf("the pktlen before formedpktLenV6 is [%d]\n", *len);
	*len = formedpktLenV6(pktFmt, pktBuf,hdrlen);
//	rtlglue_printf("the pktlen after formedpktLenV6 is [%d]\n", *len);
	l2FormCrc(pktFmt, pktBuf, *len);
//	rtlglue_printf("the length after from crc is [%d]\n", *len);
}

void protoForm_vsp_v6pptp(int8 * pktBuf, uint32 l2Flag, uint32 extHdr,int8 * da, int8 * sa, 
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
					uint32 errorFlag, uint32 * len){

	uint32 pktFmt;
	uint32 lasthdr;
	//uint8  nh;
	//rtlglue_printf("extHdr = 0x%x\n",extHdr);
	pktFmt = (l2Flag & (L2_MASK | L2_CPUTAG_MASK | ehMASK)) |L3_IPV6|(extHdr & ehMASK)| (errorFlag&CTL_MASK);
#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormCTagOther(pktFmt,pktBuf,protocol,reason,intpri,streamid,l3r,org,extdpmsk,spa,extspa,fbi,fbindex);
	l2FormCTagTX2(pktFmt,pktBuf,protocol,txmsk,prisel,pri,keep,directTx,dislrn,psel,tx_extspa,pppoeact,pppoeidx,tx_streamid);
	l2FormSVlanTag(pktFmt, pktBuf, tpid, svid, svlanDei, spriority);
	l2FormCVlanTag(pktFmt, pktBuf,0x8100, cvid, cvlanCfi, cpriority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpv6Basic(pktFmt, pktBuf, ipv);
	l3FormIpv6Flow(pktFmt,	pktBuf, tc, fl, hl);
	//l3FormIpv6NextHeader(pktFmt, pktBuf, nh);
	l3FormIpv6Addr(pktFmt, pktBuf, sip, dip);
//rtlglue_printf("%s %d #####################\n",__FUNCTION__,__LINE__);
	//luke
	switch(nh)
	{
		case 1:
			pktFmt |= L4_ICMP;
			break;
		case 2:
			pktFmt |= L4_IGMP;
			break;
		case 17:
			pktFmt |= L4_UDP;
			break;
		case 6:
			pktFmt |= L4_TCP;
			break;
		case 47:
			pktFmt |= L3_V6GRE | L4_PPTP;
			break;
	}
	
	lasthdr=l3FormIpv6ExtHdr(pktFmt, pktBuf,hdrlen, segleft, dstip, fragment, fragmentid);							 //?????????????????????????//
	//nh=2; //IGMPv4
	//rtlglue_printf("the last header is [0x%x]\n", lasthdr);
	if (lasthdr==L3_IPV6)
		l3FormIpv6NextHeader(pktFmt, pktBuf, nh);
	else
		l3FormLastHeader(pktFmt, pktBuf, hdrlen, lasthdr, nh);
	//if ((lasthdr&NoNHdr)==0)
		l3FormIpv6Content(pktFmt, pktBuf, hdrlen, content, *len);
	//rtlglue_printf("%s %d the last header is [0x%x]\n", __FUNCTION__,__LINE__,lasthdr);
	//rtlglue_printf("the pktlen before formedpktLenV6 is [%d]\n", *len);
	l4FormPptpV6Basic(pktFmt, pktBuf, hdrlen, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0x880b); //cheney: change recur bit to 0
	l4FormPptpV6Key(pktFmt, pktBuf, hdrlen, callid, seqno, ackno);
	l4FormPptpV6Other(pktFmt, pktBuf, hdrlen, content, *len);
	*len = formedpktLenV6(pktFmt, pktBuf, hdrlen); 
//	rtlglue_printf("the pktlen after formedpktLenV6 is [%d]\n", *len);
	if (*len < 60)
		*len = 60;
	l2FormCrc(pktFmt, pktBuf, *len);
//	*len+=4;

}		


