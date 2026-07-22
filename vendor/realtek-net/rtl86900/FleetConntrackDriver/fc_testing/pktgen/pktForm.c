/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.                                                
* 
* Program : Form the pkt
*
* Original Creator: Edward Jin-Ru Chen 2002/4/29
* $Author $
*
* $Revision: 1.17 $
* $Id: pktForm.c,v 1.17 2012/10/17 11:12:07 ysleu Exp $
* $Log: pktForm.c,v $
* Revision 1.17  2012/10/17 11:12:07  ysleu
* *: Fix errors.
*
* Revision 1.16  2012/10/17 10:27:41  ysleu
* *: Support ARP packet generation.
*
* Revision 1.15  2012/09/24 02:58:16  ysleu
* *: Add virtual server & UPNP table test cases.
*
* Revision 1.14  2012/09/06 08:16:04  ysleu
* *: Modify to support linux-2.6.30.
*
* Revision 1.13  2012/08/23 02:51:16  ysleu
* *: Add CRC verification.
*
* Revision 1.12  2012/07/11 03:49:17  chuck.lin
* *fix for CPU tag protocal = 0x4
*
* Revision 1.11  2012/07/04 02:35:46  ysleu
* *: Add binding test cases.
*
* Revision 1.10  2012/06/29 04:17:24  ysleu
* *: Modify packet generation.
*
* Revision 1.9  2012/05/28 07:33:15  luke
* *: add IPv6 hopbyhop gen function and display
*
* Revision 1.8  2012/04/23 09:20:30  ysleu
* *: Modify ALE34 ale output parameters.
*
* Revision 1.7  2012/04/23 07:25:17  ysleu
* *: Add SVLAN tpid packet generator configuration parameters.
*
* Revision 1.6  2012/04/02 08:55:06  ysleu
* *: Fix PPPoE bugs.
*
* Revision 1.5  2012/03/30 08:50:49  ysleu
* *: Fix bug of SVLAN.
*
* Revision 1.4  2012/03/30 08:22:44  ysleu
* *: Fix bug of CVLAN.
*
* Revision 1.3  2012/03/30 08:12:57  ysleu
* *: Fix bug of SVLAN Tag.
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
* Revision 1.4  2011/09/23 02:33:43  ysleu
* *: Support DSL VC and Ext. port tx with cpu tag.
*
* Revision 1.3  2011/09/07 10:47:27  ysleu
* *: Modify test cases. Modify model to support VC/Ext. forwarding.
*
* Revision 1.2  2011/08/23 03:50:11  ysleu
* *: Modify test cases.
*
* Revision 1.1  2011/06/08 08:21:40  ysleu
* *: Add APOLLO project model and test codes.
*
* Revision 1.12  2007/01/02 01:37:17  caroline_liu
* add generate ipv6 pkt
*
* Revision 1.11  2006/05/05 08:28:00  yjlou
* *: fixed the bug of l2FormCrc(): backward compatible for drvTest
*
* Revision 1.10  2006/05/04 13:26:09  yjlou
* *: fixed the bug of modelPktTranslator(): L2 CRC should be re-calculated for every packet.
* *: fixed the bug of modelPktTranslator(): pad 0x20 if packet is shorter than 60 bytes when untag VLAN and PPPoE header.
* *: uniform the length field definitions:
*    in virtualMacInput/Output(): length is included L2 CRC.
*    in modelPktParser() and modelPktTranslator(): length is not included L2 CRC.
*
* Revision 1.9  2006/05/03 06:21:06  yjlou
* *: fixed the bug of l4FormUdpBasic() and l4FormTcpBasic().
*
* Revision 1.8  2006/04/17 05:21:39  yjlou
* *: Since pktForm_crc32() generates reversed endian of CRC (x0 is in crc[24]), we place CRC in little endian manner.
*
* Revision 1.7  2006/04/13 14:00:36  yjlou
* *: fixed the problem of L2 CRC: should be append at the tailing of packet.
* *: fixed output packet length
* *: fixed preamble from aaad to 55d5
* *: fixed the data valid bit when transmitting tailing bytes.
*
* Revision 1.6  2006/02/13 03:43:44  chenyl
* *: bug fix: QQIC calculation when QQI > 128
*
* Revision 1.5  2005/09/16 07:45:06  rupert
* +: fix crc32 conflict with wirless device driver(8185)
*
* Revision 1.4  2005/07/20 15:29:38  yjlou
* +: porting Model Code to Linux Kernel: check RTL865X_MODEL_KERNEL.
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
* Revision 1.2  2004/02/17 13:01:47  chenyl
* *** empty log message ***
*
* Revision 1.1  2004/02/13 05:18:47  chenyl
* + Pkt generating functions
*
* Revision 1.12  2003/10/06 10:16:23  waynelee
* add pptp gre support
*
* Revision 1.11  2002/11/27 14:37:39  waynelee
* modify protoForm_ether -> protoForm_vsp_ether
*
* Revision 1.10  2002/10/25 11:22:42  waynelee
* change the rule of filling the content of payload
*
* Revision 1.9  2002/10/20 07:32:20  waynelee
* users can define the content of payload (but only indicate one byte)
*
* Revision 1.8  2002/10/07 12:29:12  waynelee
* user-defined vcfi (vlan CFI) and L4 No checksum
*
* Revision 1.7  2002/10/02 15:12:32  waynelee
* ipx support debug
*
* Revision 1.6  2002/09/16 07:31:16  waynelee
* support IPX (experimental)
*
*/

#include "types.h"
#include "l2pkt.h"
#include "l3pkt.h"
#include "l4pkt.h"
#include "pktForm.h"
#include "pktGen.h"
#include "utility.h"
#include "crc32.h"
#ifdef __KERNEL__
#include <linux/string.h>
#else
#include <stdio.h>
#include <string.h>
#endif

#include <rtl_glue.h>
#include <rtl_protocol.h>

extern void memDump (void *start, uint32 size, int8 * strHeader);

//#if defined(CONFIG_APOLLO)
uint32 lenTAG=0;
uint32 skip_minPktLen_check=0;	//to verify local out padding function
uint32 llc_len_including_padding = 0;
uint32 llc_len_excluding_padding = 0;
uint32 pppoe_length_exclude_padding = 0;
uint32 pppoe_length_include_padding = 0;
uint32 ipv4_length_exclude_padding = 0;
uint32 ipv6_length_exclude_padding = 0;
uint32 ipv6_length_include_padding = 0;
//#endif

void l2FormEtherAddress(int8 * pktBuf, int8 * da, int8 * sa) {
	_etherHdr_t * etherPtr = (_etherHdr_t *)pktBuf;
	lenTAG = 0;
	memcpy(etherPtr->da, da, 6);
	memcpy(etherPtr->sa, sa, 6);
}

void l2FormCrc(uint32 flag, int8 * pktBuf, uint32 payloadSize) {
	uint32 crc, i;

	/* Ethernet assume a packet will longer than 60 bytes, therefore, 
	 * we must pad packet content first if the length is smaller than 60 bytes.
	 * Then, we append L2 CRC in the last 4 bytes. */
	 int minPktLen = 60;
	/*if((flag & L2_CPUTAG_OTHER) | (flag & L2_CPUTAG_TX2))
		minPktLen +=8;
	if(flag & L2_CVLAN)
		minPktLen +=4;
	if(flag & L2_SVLAN)
		minPktLen +=4;*/

	if(skip_minPktLen_check)
	{	//skip minPktLen check
	}
	else if ( payloadSize < minPktLen )
	{//rtlglue_printf("%s %d  payloadSize=%d minPktLen=%d\n",__FUNCTION__,__LINE__,payloadSize,minPktLen);
#if defined(CONFIG_RG_RTL9603D_SERIES)
		memset(&pktBuf[payloadSize], 0x00/*switch default befavior*/, (minPktLen - payloadSize));
#else
#if defined(RTL865X_MODEL_USER)||defined(RTL865X_MODEL_KERNEL)
		memset(&pktBuf[payloadSize], 0x20/*Ethernet standard*/, (minPktLen - payloadSize));
//		memset(&pktBuf[payloadSize], 0x0/*Ethernet standard*/, (60 - payloadSize));
#else
		memset(&pktBuf[payloadSize], 0x00/* backward compatible for drvTest*/, (minPktLen - payloadSize));
#endif
#endif
		payloadSize = minPktLen;//rtlglue_printf("%s %d  payloadSize=%d minPktLen=%d\n",__FUNCTION__,__LINE__,payloadSize,minPktLen);
	}
	pktForm_crc32(pktBuf,payloadSize,&crc);

	if(flag & L2_CRC_ERR)
		crc = ~crc;

#if 1 /* Since pktForm_crc32() generates reversed endian of CRC (x0 is in crc[24]), we place CRC in little endian manner. */
	for(i=0;i<4;i++){
//		rtlglue_printf("$$$$$$$$$$$$$$$$$hello\n");
		pktBuf[payloadSize+i] = (crc>>(i*8)) & 0xff; /* New version, reversed again */
		}
#else
	for(i=0;i<4;i++)
		pktBuf[payloadSize+i] = (crc>>((3-i)*8)) & 0xff; /* Original version */
#endif
}

//#if defined(CONFIG_APOLLO)
void l2FormCTagOther(uint32 flag, int8 *pktBuf, uint8 protocol, uint8 reason, uint32 priority, uint32 streamid, uint32 l3r, uint8 org, uint8 extdpmsk, uint8 spa, uint8 extspa, uint8 fbi, uint32 fbindex)
{
	int8 * tmpPtr;
	_cpu_rx_Hdr_general_for_other_port_t *ctag0;
	_cpu_rx_Hdr_union1_for_other_port_t *ctag1;

	if((flag & L2_CPUTAG_OTHER) == 0)
		return;

	tmpPtr = pktBuf + 12;//Ethernet is the MUST

	ctag0 = (_cpu_rx_Hdr_general_for_other_port_t *)tmpPtr;
	memset(ctag0,0,sizeof(_cpu_rx_Hdr_general_for_other_port_t));
	ctag0->ether_type = htons(0x8899);
	ctag0->protocol = protocol;
	ctag0->reason = reason;
	lenTAG = sizeof(_cpu_rx_Hdr_general_for_other_port_t);
	
	ctag1 = (_cpu_rx_Hdr_union1_for_other_port_t *)(tmpPtr+lenTAG);
	memset(ctag1,0,sizeof(_cpu_rx_Hdr_union1_for_other_port_t));
	ctag1->priority = priority;
	ctag1->l3r = l3r;
	ctag1->spa = spa;
	ctag1->org = org;
#if defined(CONFIG_RG_RTL9607C_SERIES)
	if(spa==7 || spa==9 || spa==10)
	{
		ctag1->streamid_extspa_h = (extspa<<2)&0x1f;
		ctag1->streamid_extspa_l = 0;
	}
	else if(spa==5)
	{
		ctag1->streamid_extspa_h = (streamid>>2)&0x1f;
		ctag1->streamid_extspa_l = streamid&0x3;
	}
	else
#elif defined(CONFIG_RG_RTL9603D_SERIES)
	if(spa==5)
	{
		ctag1->streamid_extspa_h = (extspa<<2)&0x1f;
		ctag1->streamid_extspa_l = 0;
	}
	else if(spa==4)
	{
		ctag1->streamid_extspa_h = (streamid>>2)&0x1f;
		ctag1->streamid_extspa_l = streamid&0x3;
	}
	else
#endif
	{
		ctag1->streamid_extspa_h= 0;
		ctag1->streamid_extspa_l=0;
	}
	lenTAG += sizeof(_cpu_rx_Hdr_union1_for_other_port_t);

	if(fbi)
	{
		_cpu_rx_Hdr_union2_for_flow_based_t *ctag2;
		ctag2 = (_cpu_rx_Hdr_union2_for_flow_based_t *)(tmpPtr+lenTAG);
		memset(ctag2,0,sizeof(_cpu_rx_Hdr_union2_for_flow_based_t));
		ctag2->fbi = fbi;
		ctag2->flow_based_hash_index_h=(fbindex>>8)&0x7f;
		ctag2->flow_based_hash_index_l=(fbindex)&0xff;
		lenTAG += sizeof(_cpu_rx_Hdr_union2_for_flow_based_t);
	}
	else
	{
		_cpu_rx_Hdr_union2_for_switch_t *ctag2;
		ctag2 = (_cpu_rx_Hdr_union2_for_switch_t *)(tmpPtr+lenTAG);
		memset(ctag2,0,sizeof(_cpu_rx_Hdr_union2_for_switch_t));
		ctag2->ext_port_mask = extdpmsk;
		lenTAG += sizeof(_cpu_rx_Hdr_union2_for_switch_t);
	}

}

#if 0
void l2FormCTagDSLTX(uint32 flag, int8 *pktBuf, uint8 protocol,uint8 trap, uint8 dslspa)
{
	int8 * tmpPtr;
	_ctag_cpu_dsltx_header_t* ctag;

	if((flag & L2_CPUTAG_DSLTX) == 0)
		return;

	{
		tmpPtr = pktBuf + 12;//Ethernet is the MUST
		ctag = (_ctag_cpu_dsltx_header_t *)tmpPtr;
		memset(ctag,0,sizeof(_ctag_cpu_dsltx_header_t));
		ctag->ether_type = htons(0x8899);
		ctag->protocol = protocol;
		ctag->trap = trap;
		ctag->dslspa = dslspa;
		lenTAG = sizeof(_ctag_cpu_dsltx_header_t);
	}
}
#endif

void l2FormCTagTX2(uint32 flag, int8 *pktBuf, uint8 protocol,
								uint16 txmsk, uint8 prisel, uint8 priority, uint8 keep,
								uint8 directTx, uint8 dislrn, uint8 psel, uint8 extspa, uint8 pppoeact, uint8 pppoeidx,
								uint8 streamid)
{
	int8 * tmpPtr;
	_ctag_cpu_tx2_header_t* ctag;

	if((flag & L2_CPUTAG_TX2) == 0)
		return;

	{
		tmpPtr = pktBuf + 12;//Ethernet is the MUST
		ctag = (_ctag_cpu_tx2_header_t *)tmpPtr;
		memset(ctag,0,sizeof(_ctag_cpu_tx2_header_t));
		ctag->ether_type = htons(0x8899);
		//ctag->protocol = protocol;
		ctag->protocol = 0x4;
		ctag->txmsk_10_8 = (txmsk>>8)&0x7;
		ctag->txmsk_7_0 = (txmsk&0xff);
		ctag->prisel = prisel; 
		ctag->pri = priority;
		ctag->keep = keep;
		ctag->dislrn = dislrn;
		ctag->psel = psel;
		ctag->extspa = extspa;
		ctag->pppoeact = pppoeact; 
		ctag->pppoeidx_h= (pppoeidx>>1)&0x7;
		ctag->pppoeidx_l= pppoeidx&0x1;
		ctag->directTx = directTx;
		ctag->streamid_extmsk = streamid; 
		lenTAG = sizeof(_ctag_cpu_tx2_header_t);
	}
}

#if 0
void l2FormCTagDSL(uint32 flag, int8 *pktBuf, uint8 protocol, uint32 priority, uint16 dslmsk)
{
	int8 * tmpPtr;
	_ctag_cpu_dsl_header_t* ctag;

	if((flag & L2_CPUTAG_DSL) == 0)
		return;

	{
		tmpPtr = pktBuf + 12;//Ethernet is the MUST
		ctag = (_ctag_cpu_dsl_header_t *)tmpPtr;
		memset(ctag,0,sizeof(_ctag_cpu_dsl_header_t));
		ctag->ether_type = htons(0x8899);
		ctag->protocol = protocol;
		ctag->priority = priority;
		ctag->resv1 = 0;
		ctag->resv2 = 0;
		ctag->dslmsk = htons(dslmsk);
		lenTAG = sizeof(_ctag_cpu_dsl_header_t);
	}
}
#endif

//#endif

void l2FormCVlanTag(uint32 flag, int8 * pktBuf, uint16 tpid, uint16 vlanTag, uint8 vlanCfi, uint16 vlanPriority) {
	_vlanHdr_t * vlanPtr;
	_etherHdr_t * etherPtr;
	uint16 tag;

	if((flag & L2_CVLAN) == 0)
		return;
#if 0 // WEN: FIX ME
	if((flag & L2_SVLAN) == 0 && (flag & L2_OTHERVLAN) == 0)
	{
		etherPtr = (_etherHdr_t *)(pktBuf + lenTAG);
 		vlanPtr = (_vlanHdr_t *)(pktBuf + sizeof(_etherHdr_t) + lenTAG);
	}
	else if ( (flag & L2_SVLAN) == 1 && (flag & L2_OTHERVLAN) == 0 )
	{
		etherPtr = (_etherHdr_t *)(pktBuf + lenTAG + sizeof(_vlanHdr_t));
 		vlanPtr = (_vlanHdr_t *)(pktBuf + sizeof(_etherHdr_t) + lenTAG + sizeof(_vlanHdr_t));
	}
	else if ( (flag & L2_SVLAN) == 1 && (flag & L2_OTHERVLAN) == 1 )
	{
		etherPtr = (_etherHdr_t *)(pktBuf + lenTAG + sizeof(_vlanHdr_t)+ sizeof(_vlanHdr_t));
 		vlanPtr = (_vlanHdr_t *)(pktBuf + sizeof(_etherHdr_t) + lenTAG + sizeof(_vlanHdr_t)+ sizeof(_vlanHdr_t));
	}
#else
	if((flag & L2_CVLAN) == 0)
		return;

	if((flag & L2_SVLAN) == 0)
	{
		etherPtr = (_etherHdr_t *)(pktBuf + lenTAG);
 		vlanPtr = (_vlanHdr_t *)(pktBuf + sizeof(_etherHdr_t) + lenTAG);
	}
	else
	{
		etherPtr = (_etherHdr_t *)(pktBuf + lenTAG + sizeof(_vlanHdr_t));
 		vlanPtr = (_vlanHdr_t *)(pktBuf + sizeof(_etherHdr_t) + lenTAG + sizeof(_vlanHdr_t));
	}
#endif
	
	if(tpid!=0)
		etherPtr->etherType = htons(tpid);
	else
 		etherPtr->etherType = htons(0x8100);
 
	if(vlanCfi !=0)
		vlanCfi =0x1;
 
	tag = (vlanTag & 0x0fff) | (vlanPriority & 0x7)<<13 | (vlanCfi <<12) ;
 	vlanPtr->tag = htons(tag);
 }

//#if defined(CONFIG_APOLLO)
void l2FormSVlanTag(uint32 flag, int8 * pktBuf, uint16 tpid, uint16 vlanTag, uint8 vlanDei, uint16 vlanPriority) {
	_vlanHdr_t * vlanPtr;
	_etherHdr_t * etherPtr;
	uint16 tag;
	if((flag & L2_SVLAN) == 0)
		return;
#if 0 //WEN FIX ME
	if((flag & L2_OTHERVLAN) == 0)
	{
		etherPtr = (_etherHdr_t *)(pktBuf + lenTAG);
 		vlanPtr = (_vlanHdr_t *)(pktBuf + sizeof(_etherHdr_t) + lenTAG);
	}
	else
	{
		etherPtr = (_etherHdr_t *)(pktBuf + lenTAG + sizeof(_vlanHdr_t));
 		vlanPtr = (_vlanHdr_t *)(pktBuf + sizeof(_etherHdr_t) + lenTAG + sizeof(_vlanHdr_t));
	}
	etherPtr->etherType = htons(tpid);

#endif
	etherPtr = (_etherHdr_t *)(pktBuf + lenTAG);
 	etherPtr->etherType = htons(tpid);
	vlanPtr = (_vlanHdr_t *)(pktBuf + sizeof(_etherHdr_t) + lenTAG);
 
	if(vlanDei !=0)
		vlanDei =1;
 
	tag = (vlanTag & 0x0fff) | (vlanPriority & 0x7)<<13 | (vlanDei <<12) ;
 	vlanPtr->tag = htons(tag);
 }
//#endif
#if 0 // WEN FIX ME
void l2FormOtherVlanTag(uint32 flag, int8 * pktBuf, uint16 tpid, uint16 vlanTag, uint8 vlanDei, uint16 vlanPriority) {
	_vlanHdr_t * vlanPtr;
	_etherHdr_t * etherPtr;
	uint16 tag;
	if((flag & L2_OTHERVLAN) == 0)
		return;

	etherPtr = (_etherHdr_t *)(pktBuf + lenTAG);
 	etherPtr->etherType = htons(tpid);
	vlanPtr = (_vlanHdr_t *)(pktBuf + sizeof(_etherHdr_t) + lenTAG);
 
	if(vlanDei !=0)
		vlanDei =1;
 
	tag = (vlanTag & 0x0fff) | (vlanPriority & 0x7)<<13 | (vlanDei <<12) ;
 	vlanPtr->tag = htons(tag);
 }
#endif
void l2FormLlc(uint32 flag, uint8 dsap, uint8 ssap, int8 * pktBuf) {
	int8 * tmpPtr;
	_llcHdr_t * llcPtr;
	
	if((flag & L2_SNAP) == 0)
		return;

	tmpPtr = pktBuf + sizeof(_etherHdr_t) + lenTAG;

	if((flag & L2_SVLAN) == L2_SVLAN)
		tmpPtr += sizeof(_vlanHdr_t);
	if((flag & L2_CVLAN) == L2_CVLAN)
		tmpPtr += sizeof(_vlanHdr_t);
	
	llcPtr = (_llcHdr_t *)tmpPtr;
	llcPtr->llc_dsap = dsap;
	llcPtr->llc_ssap = ssap;
	llcPtr->ctrl = 0x03;
	memset(llcPtr->org_code, 0, 3);
}

void l2FormPppoeSession(uint32 flag, int8 * pktBuf, uint16 etherType, uint16 sessionId) {
	int8 * tmpPtr = pktBuf + sizeof(_etherHdr_t) + lenTAG;//Ethernet is the MUST
	_pppoeHdr_t * pppoePtr;
	_llcHdr_t * llcPtr;
	_vlanHdr_t * vlanPtr;
	_etherHdr_t * etherPtr;

	if((flag & L2_PPPoE) == 0)
		return;

	if(((flag & L2_SVLAN) == 0) && ((flag & L2_CVLAN) == 0) && ((flag & L2_SNAP) == 0))
	{
		etherPtr = (_etherHdr_t *)(pktBuf + lenTAG);
		etherPtr->etherType = htons(etherType);
	}

	if((flag & L2_SVLAN) == L2_SVLAN) 
	{
		vlanPtr = (_vlanHdr_t *)tmpPtr;
		tmpPtr += sizeof(_vlanHdr_t);
		if(((flag & L2_SNAP) == 0) && ((flag & L2_CVLAN) == 0))
			vlanPtr->etherType = htons(etherType);
	}
	if((flag & L2_CVLAN) == L2_CVLAN) 
	{
		vlanPtr = (_vlanHdr_t *)tmpPtr;
		tmpPtr += sizeof(_vlanHdr_t);
		if((flag & L2_SNAP) == 0)
			vlanPtr->etherType = htons(etherType);
	}
	
	if((flag & L2_SNAP) == L2_SNAP) {
		llcPtr = (_llcHdr_t *)tmpPtr;
		tmpPtr += sizeof(_llcHdr_t);
		llcPtr->etherType = htons(etherType);
	}

	pppoePtr = (_pppoeHdr_t*)tmpPtr;
	pppoePtr->ver = 0x1;
	pppoePtr->type = 0x1;
	pppoePtr->code = 0x0;
	pppoePtr->sessionId = htons(sessionId);
}

//
void l2FormLayer2Len(uint32 flag, int8 * pktBuf, uint16 payloadLen) {
	int8 * tmpPtr = pktBuf + sizeof(_etherHdr_t);//Ethernet is the MUST
	uint16 llcLen;
	_pppoeHdr_t * pppoePtr;
//	_llcHdr_t * llcPtr;
	_vlanHdr_t * vlanPtr;
	_etherHdr_t * etherPtr;

//#if defined(CONFIG_APOLLO)
	if(lenTAG != 0)
		tmpPtr = pktBuf + sizeof(_etherHdr_t) + lenTAG;
//#endif

	if((flag & L2_SNAP) == L2_SNAP) {
		if((flag & L2_PPPoE) == L2_PPPoE)
			llcLen = payloadLen + sizeof(_llcHdr_t) + sizeof(_pppoeHdr_t) + llc_len_including_padding - llc_len_excluding_padding;
		else
			llcLen = payloadLen + sizeof(_llcHdr_t) + llc_len_including_padding - llc_len_excluding_padding;

		if ((flag & (L2_SVLAN | L2_CVLAN)) == (L2_SVLAN | L2_CVLAN)) {
			vlanPtr = (_vlanHdr_t*)(tmpPtr+4);
			vlanPtr->etherType = htons(llcLen);
		}
		else if ((flag & L2_SVLAN) == L2_SVLAN) {
			vlanPtr = (_vlanHdr_t*)tmpPtr;
			vlanPtr->etherType = htons(llcLen);
		}
		else if ((flag & L2_CVLAN) == L2_CVLAN) {
			vlanPtr = (_vlanHdr_t*)tmpPtr;
			vlanPtr->etherType = htons(llcLen);
		}
		else {
			etherPtr = (_etherHdr_t*)pktBuf;
			etherPtr->etherType = htons(llcLen);
		}
	}
	if((flag & L2_PPPoE) == L2_PPPoE) {
		#if 1 // patch by cheney, 20160108
		if ((flag & L2_SVLAN) == L2_SVLAN) 
			tmpPtr += sizeof(_vlanHdr_t);
		#endif
		if ((flag & L2_CVLAN) == L2_CVLAN)
			tmpPtr += sizeof(_vlanHdr_t);
		if((flag & L2_SNAP) == L2_SNAP)
			tmpPtr += sizeof(_llcHdr_t);
		pppoePtr = (_pppoeHdr_t *)tmpPtr;
		pppoePtr->length = htons(payloadLen + 2 + pppoe_length_include_padding -pppoe_length_exclude_padding);   // 2 is the PPP header
	}
}

void l2FormLayer3Type(uint32 flag, int8 * pktBuf, uint16 type) {
	int8 * tmpPtr = pktBuf + sizeof(_etherHdr_t) + lenTAG;//Ethernet is the MUST
	_pppoeHdr_t * pppoePtr=NULL;
	_llcHdr_t * llcPtr=NULL;
	_vlanHdr_t * vlanPtr=NULL;
	_etherHdr_t * etherPtr=NULL;

	if((flag&(L2_SVLAN | L2_CVLAN | L2_SNAP | L2_PPPoE)) == 0) 
	{//Ethernet only
		etherPtr = (_etherHdr_t*)(pktBuf + lenTAG);
		etherPtr->etherType = htons(type);
		return;
	}
	if ((flag & L2_SVLAN) == L2_SVLAN) {
		vlanPtr = (_vlanHdr_t *)tmpPtr;
		tmpPtr += sizeof(_vlanHdr_t);
	}
	if ((flag & L2_CVLAN) == L2_CVLAN) {
		vlanPtr = (_vlanHdr_t *)tmpPtr;
		tmpPtr += sizeof(_vlanHdr_t);
	}
	if((flag & L2_SNAP) == 0 && (flag & L2_PPPoE) == 0) {
		vlanPtr->etherType = htons(type);
		return;
	}

	if((flag & L2_SNAP) == L2_SNAP) {
		llcPtr = (_llcHdr_t*)tmpPtr;
		tmpPtr += sizeof(_llcHdr_t);
		if((flag & L2_PPPoE) == 0) {
			llcPtr->etherType = htons(type);
			return;
		}
		else {
			pppoePtr = (_pppoeHdr_t*)tmpPtr;
			pppoePtr->proto = htons(type);
			return;
		}
	}

	if((flag & L2_PPPoE) == L2_PPPoE) {
		pppoePtr = (_pppoeHdr_t*)tmpPtr;
		pppoePtr->proto = htons(type);
		return;
	}
}

static int32 l3Offset(uint32 flag, int8 * pktBuf) {
	int32 offset;
	
	offset = sizeof(_etherHdr_t) + lenTAG;

	if ((flag & L2_SVLAN) == L2_SVLAN) 
		offset += sizeof(_vlanHdr_t);

	if ((flag & L2_CVLAN) == L2_CVLAN) 
		offset += sizeof(_vlanHdr_t);

	if((flag & L2_SNAP) == L2_SNAP)
		offset += sizeof(_llcHdr_t);

	if((flag & L2_PPPoE) == L2_PPPoE)
		offset += sizeof(_pppoeHdr_t);
#if 1 //cheney
	if((flag & L3_GRE) == L3_GRE){
		int8 *tmpPtr = pktBuf + offset;
		ipHdr_t *ipPtr = (ipHdr_t *)tmpPtr;	// Outer IP
		offset += (ipPtr->ip_hl*4);			// Outer IP header
		offset += 16;						// GRE
	}
	if((flag & L3_GREPPP) == L3_GREPPP)
		offset += sizeof(_grepppHdr_t);		// GREPPP
#endif
	return offset;
}

static int32 bitCount(uint32 flags) {
	uint32 i, cntr = 0;

	for(i=0;i<32;i++)
		if(flags & (0x1<<i))
			cntr++;
	return cntr;
}

// user defined etherType
void l2FormOtherEtherType(uint32 flag, int8 * pktBuf, uint16 etherType, int8 * content, uint32 payloadLen) {
	
	int8 * tmpPtr = pktBuf + l3Offset(flag, pktBuf);

	memcpy(tmpPtr,content,payloadLen);
	l2FormLayer2Len(flag, pktBuf, payloadLen);
	l2FormLayer3Type(flag, pktBuf, etherType);

}

void l3FormIpxBasic(uint32 flag, uint8 type, int8 * pktBuf) {
	int8 * tmpPtr = pktBuf + l3Offset(flag, pktBuf);
	ipxHdr_t * ipxPtr;

	ipxPtr = (ipxHdr_t*)tmpPtr;

	// memset(tmpPtr, 1, sizeof(ipxHdr_t));
	ipxPtr->ipx_sum = htons(65535);
	ipxPtr->ipx_tctrl = 0; /* used by NetWare Router */
	ipxPtr->ipx_type = type;

	l2FormLayer3Type(flag,pktBuf,0x8137);
}

void l3FormIpxDnet(uint32 flag, uint32 dnet, uint8 * dnode, uint16 dsock, int8 * pktBuf) {
	int8 * tmpPtr = pktBuf + l3Offset(flag, pktBuf);
	ipxHdr_t * ipxPtr;

	ipxPtr = (ipxHdr_t*)tmpPtr;
	ipxPtr->ipx_dnet = htonl(dnet);
	memcpy(ipxPtr->ipx_dnode,dnode,6);
	ipxPtr->ipx_dsock = htons(dsock);
}

void l3FormIpxSnet(uint32 flag, uint32 snet, uint8 * snode, uint16 ssock, int8 * pktBuf) {
	int8 * tmpPtr = pktBuf + l3Offset(flag, pktBuf);
	ipxHdr_t * ipxPtr;

	ipxPtr = (ipxHdr_t*)tmpPtr;
	ipxPtr->ipx_snet = htonl(snet);
	memcpy(ipxPtr->ipx_snode,snode,6);
	ipxPtr->ipx_ssock = htons(ssock);
}

void l3FormIpxContent(uint32 flag, int8 * pktBuf, int8 * content, uint32 size) {
//	ipxHdr_t * ipxPtr = (ipxHdr_t*)(pktBuf + l3Offset(flag));
	int8 * tmpPtr = pktBuf + l3Offset(flag, pktBuf) + sizeof(ipxHdr_t);
//	int8 * tmpPtr = pktBuf + l3Offset(flag);
//	ipxHdr_t * ipxPtr;
	ipxHdr_t * ipxPtr = (ipxHdr_t*)(pktBuf + l3Offset(flag, pktBuf));

	memcpy(tmpPtr, content, size);
	ipxPtr->ipx_len = htons(46);

	l2FormLayer2Len(flag,pktBuf,sizeof(ipxHdr_t)+size);

//	l3FormIpTotalLen(flag, pktBuf, size);
//	l3FormIpChecksum(flag, pktBuf);
}

static int32 l3FlagValidArp(uint32 flag) {
	if(bitCount(flag & L3_MASK) > 1 || (flag & L3_ARP) != L3_ARP)
		return 0;
	return 1;
}

void l3FormArpBasic(uint32 flag, uint16 op, int8 * pktBuf) {
	int8 * tmpPtr = pktBuf + l3Offset(flag, pktBuf);
	arpHdr_t * arpPtr;

	if(!l3FlagValidArp(flag) || (flag & L2_PPPoE) == L2_PPPoE) // PPPoE do not have Ethernet ARP
		return;

	memset(tmpPtr, 0, sizeof(arpHdr_t));
	arpPtr = (arpHdr_t*)tmpPtr;
	arpPtr->ar_hrd = htons(ARPHRD_ETHER);
	arpPtr->ar_pro = htons(0x0800);
	arpPtr->ar_hln = 0x6;
	arpPtr->ar_pln = 0x4;
//	arpPtr->ar_hln = 0x6;
	// 0x0806 is ARP ; 8035 is RARP
	if(op == ARPOP_REQUEST || op == ARPOP_REPLY || op == ARPOP_INARPREQUEST || op == ARPOP_INARPREPLY)
		l2FormLayer3Type(flag, pktBuf, 0x0806);
	else
		l2FormLayer3Type(flag, pktBuf, 0x8035);

	l2FormLayer2Len(flag, pktBuf, sizeof(arpHdr_t));
}

void l3FormArpSelfInfo(uint32 flag, int8 * pktBuf, uint32 ipAddr, int8 * macAddr) {
	int8 * tmpPtr = pktBuf + l3Offset(flag, pktBuf);
	uint32 i;
	arpHdr_t * arpPtr;
	uint32 ip;

	if(!l3FlagValidArp(flag))
		return;

	arpPtr = (arpHdr_t*)tmpPtr;
	memcpy(arpPtr->ar_sha, macAddr, 6);
	#ifdef _ENDINEFREE_IP_PORT
	ip = htonl(ipAddr);
	#else
	ip = ipAddr;
	#endif
	for(i=0;i<4;i++)
		arpPtr->ar_spa[i] = (ip >> (8*(3-i)))&0xff;
}

void l3FormArpPeerInfo(uint32 flag, int8 * pktBuf, uint16 op, uint32 ipAddr, int8 * macAddr) {
	int8 * tmpPtr = pktBuf + l3Offset(flag, pktBuf);
	uint32 i;
	arpHdr_t * arpPtr;
	uint32 ip;

	if(!l3FlagValidArp(flag))
		return;

	arpPtr = (arpHdr_t*)tmpPtr;
	arpPtr->ar_op = htons(op);
	if(op == ARPOP_REPLY)
		memcpy(arpPtr->ar_tha, macAddr, 6);
	else if(op == ARPOP_REQUEST)
		memset(arpPtr->ar_tha, 0, 6);
	else
		memcpy(arpPtr->ar_tha, macAddr, 6);
	#ifdef _ENDINEFREE_IP_PORT
	ip = htonl(ipAddr);
	#else
	ip = ipAddr;
	#endif
	for(i=0;i<4;i++)
		arpPtr->ar_tpa[i] = (ip >> (8*(3-i)))&0xff;
}

static int32 l3FlagValidIp(uint32 flag) {
	if(bitCount(flag & L3_MASK) > 1 || (flag & L3_IP) != L3_IP)
		return 0;
	return 1;
}

static int32 l3FlagValidIpV6(uint32 flag) {
	if(bitCount(flag & L3_MASK) > 1 || (flag & L3_IPV6) != L3_IPV6)
		return 0;
	return 1;
}

void l3FormIpBasic(uint32 flag, int8 * pktBuf, uint8 ver, uint8 len) {
	int8 * tmpPtr = pktBuf + l3Offset(flag, pktBuf);
	ipHdr_t * ipPtr;
	uint32 ipHeaderLen;

	if(!l3FlagValidIp(flag))
		return;

	if(len%4)
		ipHeaderLen = len/4 + 1;
	else
		ipHeaderLen = len/4;

	memset(tmpPtr, 0, ipHeaderLen*4);
	ipPtr = (ipHdr_t*) tmpPtr;
	ipPtr->ip_hl = ipHeaderLen;
	ipPtr->ip_v = ver;

	if((flag & L2_PPP_ERR) == L2_PPP_ERR)
		l2FormLayer3Type(flag, pktBuf, 0x0033);
	else if((flag & L2_PPPoE) == L2_PPPoE)
			l2FormLayer3Type(flag, pktBuf, 0x0021); // IP is encapsulated in PPPoE
	else if((flag & L2_PPPoE) == L2_PPPoE)				// cheney
			l2FormLayer3Type(flag, pktBuf, 0x0021); // IP is encapsulated in PPTP(GRE)
	else if((flag & L2_PPPoE6) == L2_PPPoE6)	
			l2FormLayer3Type(flag, pktBuf, 0x0057); // IPv6 PPPoE
	else
			l2FormLayer3Type(flag, pktBuf, 0x0800); // IP
	l2FormLayer2Len(flag, pktBuf, ipHeaderLen*4);
	l3FormIpChecksum(flag, pktBuf);
}

void l3FormIpFlow(uint32 flag, int8 *pktBuf, uint8 tos, uint8 ttl) {
	ipHdr_t * ipPtr = (ipHdr_t *)(pktBuf + l3Offset(flag, pktBuf));

	if(!l3FlagValidIp(flag))
		return;

	ipPtr->ip_tos = tos;
	ipPtr->ip_ttl = ttl;
	l3FormIpChecksum(flag, pktBuf);
}

void l3FormIpTotalLen(uint32 flag, int8 * pktBuf, uint16 len) {
	ipHdr_t * ipPtr = (ipHdr_t *)(pktBuf + l3Offset(flag, pktBuf));

	if(!l3FlagValidIp(flag))
		return;

	ipPtr->ip_len = htons(len + ipPtr->ip_hl*4 - ipv4_length_exclude_padding);
	l3FormIpChecksum(flag, pktBuf);
	l2FormLayer2Len(flag, pktBuf, len + ipPtr->ip_hl*4);
}

void l3FormIpTotalLenZero(uint32 flag, int8 * pktBuf) {
	ipHdr_t * ipPtr = (ipHdr_t *)(pktBuf + l3Offset(flag, pktBuf));

	if(!l3FlagValidIp(flag))
		return;

	ipPtr->ip_len = 0;
	l3FormIpChecksum(flag, pktBuf);
}

//added by liujuan
void l3FormIpv6TotalLen(uint32 flag, int8 * pktBuf, uint8 hdrlen, uint16 len) {
	int32 exthdrlen;
//	rtlglue_printf("in l3 Form ipv6 total lenth \n\n");	
	ip6Hdr_t * ipPtr = (ip6Hdr_t *)(pktBuf + l3Offset(flag, pktBuf));
//	rtlglue_printf("the len is [%d]\n\n", len);
	exthdrlen=v6ExtHdrLen(flag, hdrlen);
//	rtlglue_printf("the extension header length is [%d]\n\n", exthdrlen);
	ipPtr->ip_len =htons( len + exthdrlen + ipv6_length_include_padding - ipv6_length_exclude_padding);
//	ipPtr->ip_len=exthdrlen+len;
//	rtlglue_printf("the len + exthdrlen is [%d]\n\n", len+exthdrlen);
//	rtlglue_printf("the ipv6 ip_len is [%d]\n\n", ipPtr->ip_len);
	l2FormLayer2Len(flag, pktBuf, len+exthdrlen+40);
}

void l3FormIpv6TotalLenZero(uint32 flag, int8 * pktBuf) {
	//int32 exthdrlen;	
	ip6Hdr_t * ipPtr = (ip6Hdr_t *)(pktBuf + l3Offset(flag, pktBuf));
	ipPtr->ip_len = 0;
	l2FormLayer2Len(flag, pktBuf, 40);
}

void l3FormIpIdFrag(uint32 flag, int8 * pktBuf, uint16 id, uint8 df, uint8 mf, uint16 offset) {
	uint16	ip_off;
	ipHdr_t * ipPtr = (ipHdr_t *)(pktBuf + l3Offset(flag, pktBuf));

	if(!l3FlagValidIp(flag))
		return;

	ip_off = 0x0;
	if(df)
		ip_off |= IP_DF;
	else
		ip_off &= ~IP_DF;

	if(mf)
		ip_off |= IP_MF;
	else
		ip_off &= ~IP_MF;

	ip_off |= offset & IP_OFFMASK;

	ipPtr->ip_id = htons(id);
	ipPtr->ip_off = htons(ip_off);
	l3FormIpChecksum(flag, pktBuf);
}

void l3FormIpProtocol(uint32 flag, int8 * pktBuf, int8 protocol){
	int8 * tmpPtr = pktBuf + l3Offset(flag, pktBuf);
	ipHdr_t * ipPtr;


	if(!l3FlagValidIp(flag))
		return;

	ipPtr = (ipHdr_t*) tmpPtr;
	ipPtr->ip_p = protocol;
	l3FormIpChecksum(flag, pktBuf);
}

void l3FormIpAddr(uint32 flag, int8 * pktBuf, uint32 sip, uint32 dip){
	ipHdr_t * ipPtr = (ipHdr_t*)(pktBuf + l3Offset(flag, pktBuf));

	if(!l3FlagValidIp(flag))
		return;

	#ifdef _ENDINEFREE_IP_PORT
	ipPtr->ip_src = sip;
	ipPtr->ip_dst = dip;
	#else
	ipPtr->ip_src = htonl(sip);
	ipPtr->ip_dst = htonl(dip);
	#endif
	l3FormIpChecksum(flag, pktBuf);
}

void l3FormIpContent(uint32 flag, int8 * pktBuf, uint8 * content, uint32 size) {
	ipHdr_t * ipPtr = (ipHdr_t*)(pktBuf + l3Offset(flag, pktBuf));
	int8 * tmpPtr = pktBuf + l3Offset(flag, pktBuf) + ipPtr->ip_hl * 4;

	memcpy(tmpPtr, content, size);
	l3FormIpTotalLen(flag, pktBuf, size);
	l3FormIpChecksum(flag, pktBuf);
}

void l3FormIpOption(uint32 flag, int8 * pktBuf, uint8 * content, uint32 size) {
	ipHdr_t * ipPtr = (ipHdr_t*)(pktBuf + l3Offset(flag, pktBuf));
	int8 * tmpPtr = pktBuf + l3Offset(flag, pktBuf) + ipPtr->ip_hl * 4;

	ipPtr->ip_hl += (size/4);

	memcpy(tmpPtr, content, size);
	l3FormIpTotalLen(flag, pktBuf, size);
	l3FormIpChecksum(flag, pktBuf);
}

void l3FormIpChecksum(uint32 flag, int8 *pktBuf) {
	int8 * tmpPtr = pktBuf + l3Offset(flag, pktBuf);
	ipHdr_t * ipPtr;
	uint16 cksum;

	if(!l3FlagValidIp(flag))
		return;

	ipPtr = (ipHdr_t*) tmpPtr;
	ipPtr->ip_sum = 0x0;
	cksum = ipcsum((uint16*)ipPtr, ipPtr->ip_hl * 4, 0);

	if(flag & L3_CKSUM_ERR){
		if(cksum==0xffff)cksum=1;
		ipPtr->ip_sum = htons(cksum);
	}else
		ipPtr->ip_sum = htons(~cksum);
}

//added by liujuan
void l3FormIpv6Basic(uint32 flag, int8* pktBuf, uint8 ver){
       int8 * tmpPtr = pktBuf + l3Offset(flag, pktBuf);
	ip6Hdr_t * ipPtr;

	ipPtr = (ip6Hdr_t*) tmpPtr;
	
	ipPtr->ip_v = ver;

	if((flag & L2_PPPoE) == L2_PPPoE)
			l2FormLayer3Type(flag, pktBuf, 0x0057);
	else
			l2FormLayer3Type(flag, pktBuf, 0x86DD);
	//l2FormLayer2Len(flag, pktBuf, ipHeaderLen*4);
	//l3FormIpChecksum(flag, pktBuf);

}

//added by liujuan
void l3FormIpv6Flow(uint32 flag, int8* pktBuf, uint8 pri, uint8 *fl, uint8 hl ){
	int8 * tmpPtr = pktBuf + l3Offset(flag, pktBuf);
#if 0 // improve by cheney
	ip6Hdr_t * ipPtr;

	ipPtr = (ip6Hdr_t*) tmpPtr;
	
	ipPtr->ip_pri = pri;
	memcpy(ipPtr->ip_flow,fl,3);                                    //???????????????????????????????????????
	ipPtr->ip_hop = hl;
#else
	ipv6hdr_t *ipPtr = (ipv6hdr_t*) tmpPtr;
	ipPtr->tclassH = pri >> 4;
	ipPtr->tclassL = pri & 0xf;
	// default flowlabel (0x123456) will become 0x2_3456	(5*4 bits)
	ipPtr->flow_lblH = *fl & 0xf;
	ipPtr->flow_lblL = htons((*(fl+1) << 8) | *(fl+2));
	ipPtr->hop_limit = hl;
#endif
	
}

//added by liujuan
void l3FormIpv6Addr(uint32 flag, int8* pktBuf, uint8 * sip, uint8 *  dip){
	int8 * tmpPtr = pktBuf + l3Offset(flag, pktBuf);
	ip6Hdr_t * ipPtr = (ip6Hdr_t *)tmpPtr;

	memcpy(ipPtr->ip_src, sip, 16);
	memcpy(ipPtr->ip_dst, dip, 16);

}

//added by liujuan
void l3FormIpv6NextHeader(uint32 flag, int8 * pktBuf, uint8 nh) {
	int8 * tmpPtr = pktBuf + l3Offset(flag, pktBuf);
	ip6Hdr_t * ipPtr = (ip6Hdr_t *)tmpPtr;
//rtlglue_printf("%s %d nh =%d\n",__FUNCTION__,__LINE__,nh);
	ipPtr->ip_next=nh;

}


//added by liujuan
int32 v6ExtHdrLen(uint32 flag, uint8 rhdrlen) {
	int32 exthdrlen;
	exthdrlen=0;
	//rtlglue_printf("the flag is [0x%x]\n", flag);	
	if((flag & L3_MASK)==0)
		return 0;
	if (flag & L3_IPV6) {                      //added by liujuan , not finish
		if (flag & hopbyhopHdr)
			//exthdrlen+=16;
			exthdrlen+=8;
		if (flag &DesHdr)
			exthdrlen+=16;
		if (flag & RouHdr)
			exthdrlen+=(rhdrlen*8 + 8);
		if (flag & FraHdr)
			exthdrlen+=8;
		if (flag & AutHdr)
			exthdrlen+=20;
		if (flag & ESPHdr)
			exthdrlen+= 32;
		if (flag & DesHdr2)
			exthdrlen+=16;
		}
	//rtlglue_printf("the exthdr lenght is [%d]\n", exthdrlen);
	return exthdrlen; 
//	rtlglue_printf("in v6ExtHdrLen\n");
//	return 20;

}

//added by liujuan
void l3FormIpv6Content(uint32 flag, int8 * pktBuf, uint8 hdrlen, int8 * content, uint32 size) {
	//ip6Hdr_t * ipPtr = (ip6Hdr_t)(pktBuf + l3Offset(flag));
	int8 * tmpPtr = pktBuf + l3Offset(flag, pktBuf) + 40 + v6ExtHdrLen(flag, hdrlen);

	memcpy(tmpPtr, content, size);
	l3FormIpv6TotalLen(flag, pktBuf, hdrlen,size);
}
//added by liujuan.  ?????????????????????????????????????????????????????????
uint32 l3FormIpv6ExtHdr(uint32 flag, int8 * pktBuf, uint8 hdrlen, uint8 segleft, uint8 * dstip, uint16 fragment, uint32 fragmentid){
	int8 * tmpPtr= pktBuf + l3Offset(flag, pktBuf) +40;
	uint32 lasthdr;
	uint8 nh;
	hopbyhopHdr_t * hopbyhopptr=NULL;
	DesHdr_t * desptr=NULL; 
	DesHdr_t * desptr2=NULL;
	RouHdr_t * rouptr=NULL;
	FraHdr_t * fraptr=NULL;
	AutHdr_t * autptr=NULL;
//	uint8 * p;
	
	//rtlglue_printf("the flag in form v6 entension header is [0x%x]\n", flag);	

	lasthdr=L3_IPV6;

	if (flag&hopbyhopHdr) {
		hopbyhopptr = (hopbyhopHdr_t *)tmpPtr;
		//tmpPtr +=16;
		memset(tmpPtr, 0, 8);
		tmpPtr +=8;
		nh=0;
		l3FormIpv6NextHeader(flag, pktBuf, nh);
		//hopbyhopptr->hdrlen=1;
		hopbyhopptr->hdrlen=0;	//only one 8-octets
		lasthdr=hopbyhopHdr;
	}
	
	if (flag&DesHdr) {
		desptr = (DesHdr_t *)tmpPtr;
		tmpPtr += 16;
		nh=60;
		desptr->hdrlen=1;
		if (lasthdr==L3_IPV6)
			l3FormIpv6NextHeader(flag, pktBuf, nh);
		if (lasthdr==hopbyhopHdr)
			hopbyhopptr->nexthdr=nh;
		lasthdr=DesHdr;
	}

	if ((flag& SRv6Hdr) != 0 && (flag&L4_PPTP) == 0) {
		int i;
		uint8 lastEntry;
		//rtlglue_printf("@@@@@@@@@@@@@the flag is [%d]\n", flag&SRv6Hdr);
		//rtlglue_printf("@@@@@@@@the hdrlen is [%d]\n", hdrlen);
		rouptr = (RouHdr_t *)tmpPtr;
		memset(tmpPtr, 0,8+(hdrlen*8));
		nh=43;
		//rouptr->nexthdr = nh;
		rouptr->hdrlen= hdrlen;
		rouptr->routingtype=4;
		rouptr->segmentleft=segleft;
		lastEntry = ((hdrlen>>1)-1);
		tmpPtr[4] = lastEntry;
		for(i=lastEntry;i>=0;i--)
		{
			memcpy(tmpPtr+8+(i*16), dstip, 16);
			if(i!=lastEntry)
				tmpPtr[8+(i*16)]+=1;
		}

		if (lasthdr==L3_IPV6)
			l3FormIpv6NextHeader(flag, pktBuf, nh);
		if (lasthdr==hopbyhopHdr)
			hopbyhopptr->nexthdr=nh;
		if (lasthdr==DesHdr)
			desptr->nexthdr=nh;

		lasthdr=RouHdr;
		tmpPtr+=(hdrlen*8+8);  
	}
	else if (flag& RouHdr) {
		rouptr = (RouHdr_t *)tmpPtr;
//		tmpPtr += (hdrlen*8+8);
		nh=43;
		rouptr->hdrlen= hdrlen;
		rouptr->routingtype=0;
		rouptr->segmentleft=segleft;
	//	memcpy(rouptr->dstip, dstip, 16);

//		p=melloc(sizeof(uint8) * (hdrlen*8-12));
//		*p=0;
//		rouptr->data=p;
//		memcpy(rouptr->dstip, dstip, 16);
		memset(tmpPtr+4, '0', hdrlen*8-12);
//		rtlglue_printf("@@@@@@@@@@@@@the flag is [%d]\n", flag&RouHdr);
//		rtlglue_printf("@@@@@@@@the hdrlen is [%d]\n", hdrlen);
		memcpy(tmpPtr+4+(hdrlen*8-12), dstip, 16);

		if (lasthdr==L3_IPV6)
			l3FormIpv6NextHeader(flag, pktBuf, nh);
		if (lasthdr==hopbyhopHdr)
			hopbyhopptr->nexthdr=nh;
		if (lasthdr==DesHdr)
			desptr->nexthdr=nh;

		lasthdr=RouHdr;
//		free(p);
		tmpPtr+=(hdrlen*8+8);  
	}

	if (flag & FraHdr) {
		fraptr = (FraHdr_t *)tmpPtr;
		memset(fraptr,0,8);
		fraptr->fragment=htons(fragment);
		fraptr->id=htonl(fragmentid);
		tmpPtr += 8;
		nh=44;
		
		if (lasthdr==L3_IPV6)
			l3FormIpv6NextHeader(flag, pktBuf, nh);
		if (lasthdr==hopbyhopHdr)
			hopbyhopptr->nexthdr=nh;
		if (lasthdr==DesHdr)
			desptr->nexthdr=nh;
		if (lasthdr==RouHdr)
			rouptr->nexthdr=nh;

		lasthdr=FraHdr;
	}

	if (flag & AutHdr) {
		autptr = (AutHdr_t *)tmpPtr;
		tmpPtr+=20;
		nh=51;

		autptr->hdrlen=3;

		if (lasthdr==L3_IPV6)
			l3FormIpv6NextHeader(flag, pktBuf, nh);
		if (lasthdr==hopbyhopHdr)
			hopbyhopptr->nexthdr=nh;
		if (lasthdr==DesHdr)
			desptr->nexthdr=nh;
		if (lasthdr==RouHdr)
			rouptr->nexthdr=nh;
		if (lasthdr==FraHdr)
			fraptr->nexthdr=nh;

		lasthdr=AutHdr;
	}

	if (flag&DesHdr2) {
		desptr2 = (DesHdr_t *)tmpPtr;
		tmpPtr += 16;
		nh=60;
		desptr2->hdrlen=1;

		if (lasthdr==L3_IPV6)
			l3FormIpv6NextHeader(flag, pktBuf, nh);
		if (lasthdr==hopbyhopHdr)
			hopbyhopptr->nexthdr=nh;
		if (lasthdr==DesHdr)
			desptr->nexthdr=nh;
		if (lasthdr==RouHdr)
			rouptr->nexthdr=nh;
		if (lasthdr==FraHdr)
			fraptr->nexthdr=nh;
		if (lasthdr==AutHdr)
			autptr->nexthdr=nh;

		lasthdr=DesHdr2;
		
	}

	if (flag & NoNHdr) {
		nh=59;

		if (lasthdr==L3_IPV6)
			l3FormIpv6NextHeader(flag, pktBuf, nh);
		if (lasthdr==hopbyhopHdr)
			hopbyhopptr->nexthdr=nh;
		if (lasthdr==DesHdr)
			desptr->nexthdr=nh;
		if (lasthdr==RouHdr)
			rouptr->nexthdr=nh;
		if (lasthdr==FraHdr)
			fraptr->nexthdr=nh;
		if (lasthdr==AutHdr)
			autptr->nexthdr=nh;
		if (lasthdr==DesHdr2)
			desptr2->nexthdr=nh;

		lasthdr=NoNHdr;
	} 

	if(flag & DUAL_OUTER_HDR){
		lasthdr=L3_IPV6;
	}

	return lasthdr;
}

//added by liujuan
void l3FormLastHeader(uint32 flag, int8 * pktBuf, uint8 hdrlen, uint32 lasthdr, uint8 nh) {
	int8 * tmpPtr=pktBuf+l3Offset(flag, pktBuf)+40+v6ExtHdrLen(flag, hdrlen);
	hopbyhopHdr_t * hopbyhopptr;
	DesHdr_t * desptr;
	DesHdr_t * desptr2;
	RouHdr_t * rouptr;
	FraHdr_t * fraptr;
	AutHdr_t * autptr;
//rtlglue_printf("%s %d flag=0x%x lasthdr=0x%x\n",__FUNCTION__,__LINE__,flag,lasthdr);
	if(lasthdr==hopbyhopHdr) {
		//tmpPtr=tmpPtr-16;
		tmpPtr=tmpPtr-8;
		hopbyhopptr=(hopbyhopHdr_t *)tmpPtr;
		hopbyhopptr->nexthdr=nh;
	}
	else if(lasthdr==DesHdr) {
		tmpPtr=tmpPtr-16;
		desptr=(DesHdr_t *)tmpPtr;
		desptr->nexthdr=nh;
	}
	else if(lasthdr==RouHdr) {
		tmpPtr=tmpPtr-(hdrlen*8 + 8);
		rouptr=(RouHdr_t *)tmpPtr;
		rouptr->nexthdr=nh;
	}
	else if(lasthdr==FraHdr) {
		tmpPtr=tmpPtr-8;
		fraptr=(FraHdr_t *)tmpPtr;
		fraptr->nexthdr=nh;
	}
	else if(lasthdr==AutHdr) {
		tmpPtr=tmpPtr-20;
		autptr=(AutHdr_t *)tmpPtr;
		autptr->nexthdr=nh;
	}
	else if(lasthdr==DesHdr2) {
		tmpPtr=tmpPtr-16;
		desptr2=(DesHdr_t *)tmpPtr;
		desptr2->nexthdr=nh;
	}
}

static int32 l4Offset(uint32 flag, int8 * pktBuf) {
	int8 * tmpPtr = pktBuf + l3Offset(flag, pktBuf);
	ipHdr_t * ipPtr = (ipHdr_t *)tmpPtr;
	
	if((flag & L3_MASK) == 0)
		return l3Offset(flag, pktBuf);
	else if (flag & L3_ARP)
		return l3Offset(flag, pktBuf) + sizeof(arpHdr_t);
	else if (flag & L3_IPX)
		return l3Offset(flag, pktBuf) + sizeof(ipxHdr_t);
	


	return l3Offset(flag, pktBuf) + ipPtr->ip_hl * 4;
}

//added by liujuan
static int32 l4OffsetV6(uint32 flag, int8 * pktBuf, uint8 hdrlen ) {
//	int8 * tmpPtr = pktBuf + l3Offset(flag);
//	ip6Hdr_t * ipPtr = (ip6Hdr_t *)tmpPtr;
	int32 exthdrlen=0;                //added by liujuan

	if((flag & L3_MASK)==0)
		return l3Offset(flag, pktBuf);
	else if (flag & L3_IPV6) 
		exthdrlen=v6ExtHdrLen(flag, hdrlen);
	return l3Offset(flag, pktBuf) +40 +exthdrlen;
}
static uint16 l4PayloadLen(uint32 flag, int8 * pktBuf) {
	ipHdr_t * ipPtr = (ipHdr_t *)(pktBuf + l3Offset(flag, pktBuf));

	if((flag & L4_MASK) == 0)
		return 0;

	return ntohs(ipPtr->ip_len) - ipPtr->ip_hl * 4;
}

//added by liujuan
static uint16 l4PayloadLenV6(uint32 flag, int8 * pktBuf, uint8 hdrlen) {
	int8 * tmpPtr = pktBuf + l3Offset(flag, pktBuf);
	ip6Hdr_t * ipPtr = (ip6Hdr_t *)tmpPtr;
	int32 exthdrlen=0;                //added by liujuan

	//if((flag & L4_MASK)==0)
		//return 0;
	if (flag & L3_IPV6) 
		exthdrlen=v6ExtHdrLen(flag, hdrlen);
//	rtlglue_printf("the extension header leghth is [%d]\n", exthdrlen);
//	rtlglue_printf("the ipv6 payload is [%d]\n", ntohs(ipPtr->ip_len));
	return ntohs(ipPtr->ip_len) - exthdrlen;
}

static int32 l4FlagValidPptp(uint32 flag) {
	if(!l3FlagValidIp(flag) ||
		bitCount(flag & L4_MASK) > 1 || (flag & L4_PPTP) != L4_PPTP)
		return 0;
	return 1;
}

static int32 l4FlagValidPptpV6(uint32 flag) {
	if(!l3FlagValidIpV6(flag) ||
		bitCount(flag & L4_MASK) > 1 || (flag & L4_PPTP) != L4_PPTP)
		return 0;
	return 1;
}

void l4FormPptpModify(uint32 flag, int8 * pktBuf, 
					 uint8 Cbit, uint8 Rbit, uint8 Kbit, uint8 Sbit, uint8 sbit,
					 uint8 recur, uint8	Abit, uint8	otherflags, uint8 ver, uint16 protocol) {
	uint8	pptpflagsx1,pptpflagsx2;
	int8 * tmpPtr = (int8 *)(pktBuf + l4Offset(flag, pktBuf));
	greHdr_t * pptpPtr = (greHdr_t *)tmpPtr;

	if(!l4FlagValidPptp(flag))
		return;

	/*memset(tmpPtr, 0, sizeof(greHdr_t));*/

	pptpflagsx1 = 0x00;

	if(Cbit)
		pptpflagsx1 |= GRE_C_BIT;
	if(Rbit)
		pptpflagsx1 |= GRE_R_BIT;
	if(Kbit)
		pptpflagsx1 |= GRE_K_BIT;
	if(Sbit)
		pptpflagsx1 |= GRE_S_BIT;
	if(sbit)
		pptpflagsx1 |= GRE_s_BIT;
	
	pptpflagsx1 |= (recur & ~GRE_FLAGS_x1);

	pptpPtr->gre_flags_x1 = pptpflagsx1;

	pptpflagsx2 = 0x00;

	if(Abit)
		pptpflagsx2 |= 0x80;

	pptpflagsx2 |= ((otherflags & 0x0f)<<3);
	pptpflagsx2 |= (ver & 0x07);
	pptpPtr->gre_flags_x2 = pptpflagsx2;
	pptpPtr->gre_protocol = htons(protocol);
}

void l4FormPptpBasic(uint32 flag, int8 * pktBuf, 
					 uint8 Cbit, uint8 Rbit, uint8 Kbit, uint8 Sbit, uint8 sbit,
					 uint8 recur, uint8	Abit, uint8	otherflags, uint8 ver, uint16 protocol) {
	uint8	pptpflagsx1,pptpflagsx2;
	int8 * tmpPtr = (int8 *)(pktBuf + l4Offset(flag, pktBuf));
	greHdr_t * pptpPtr = (greHdr_t *)tmpPtr;

	if(!l4FlagValidPptp(flag))
		return;

	memset(tmpPtr, 0, sizeof(greHdr_t));

	pptpflagsx1 = 0x00;

	if(Cbit)
		pptpflagsx1 |= GRE_C_BIT;
	if(Rbit)
		pptpflagsx1 |= GRE_R_BIT;
	if(Kbit)
		pptpflagsx1 |= GRE_K_BIT;
	if(Sbit)
		pptpflagsx1 |= GRE_S_BIT;
	if(sbit)
		pptpflagsx1 |= GRE_s_BIT;
	
	pptpflagsx1 |= (recur & ~GRE_FLAGS_x1);

	pptpPtr->gre_flags_x1 = pptpflagsx1;

	pptpflagsx2 = 0x00;

	if(Abit)
		pptpflagsx2 |= 0x80;

	pptpflagsx2 |= ((otherflags & 0x0f)<<3);
	pptpflagsx2 |= (ver & 0x07);
	pptpPtr->gre_flags_x2 = pptpflagsx2;
	pptpPtr->gre_protocol = htons(protocol);

}

void l4FormPptpKey (uint32 flag, int8 * pktBuf, 
					uint16 callid, uint32 seqno, uint32 ackno) {
	int8 * tmpPtr = (int8 *)(pktBuf + l4Offset(flag, pktBuf));
	greHdr_t * pptpPtr = (greHdr_t *)tmpPtr;

	if(!l4FlagValidPptp(flag))
		return;

	pptpPtr->gre_callid = htons(callid);
	pptpPtr->gre_seqno = htonl(seqno);
	pptpPtr->gre_ackno = htonl(ackno);
}

void l4FormPptpOther (uint32 flag, int8 * pktBuf, int8 * content, uint32 size) {
	int8 * cntPtr = (int8 *)(pktBuf + l4Offset(flag, pktBuf) + sizeof(greHdr_t));
	int8 * tmpPtr = (int8 *)(pktBuf + l4Offset(flag, pktBuf));
	greHdr_t * pptpPtr = (greHdr_t *)tmpPtr;

	if(!l4FlagValidPptp(flag))
		return;

	pptpPtr->gre_length = htons(size);

	memcpy(cntPtr, content, size);
	l3FormIpProtocol(flag, pktBuf, IP_PROTO_PPTP);
	l3FormIpTotalLen(flag, pktBuf, size + sizeof(greHdr_t));
}

void l4FormPptpV6Modify(uint32 flag, int8 * pktBuf, uint8 hdrlen,
					 uint8 Cbit, uint8 Rbit, uint8 Kbit, uint8 Sbit, uint8 sbit,
					 uint8 recur, uint8	Abit, uint8	otherflags, uint8 ver, uint16 protocol) {
	uint8	pptpflagsx1,pptpflagsx2;
	int8 * tmpPtr = (int8 *)(pktBuf + l4OffsetV6(flag, pktBuf, hdrlen));
	greHdr_t * pptpPtr = (greHdr_t *)tmpPtr;

	if(!l4FlagValidPptpV6(flag))
		return;

	//memset(tmpPtr, 0, sizeof(greHdr_t));

	pptpflagsx1 = 0x00;

	if(Cbit)
		pptpflagsx1 |= GRE_C_BIT;
	if(Rbit)
		pptpflagsx1 |= GRE_R_BIT;
	if(Kbit)
		pptpflagsx1 |= GRE_K_BIT;
	if(Sbit)
		pptpflagsx1 |= GRE_S_BIT;
	if(sbit)
		pptpflagsx1 |= GRE_s_BIT;

	pptpflagsx1 |= (recur & ~GRE_FLAGS_x1);

	pptpPtr->gre_flags_x1 = pptpflagsx1;

	pptpflagsx2 = 0x00;

	if(Abit)
		pptpflagsx2 |= 0x80;

	pptpflagsx2 |= ((otherflags & 0x0f)<<3);
	pptpflagsx2 |= (ver & 0x07);
	pptpPtr->gre_flags_x2 = pptpflagsx2;
	pptpPtr->gre_protocol = htons(protocol);

}

void l4FormPptpV6Basic(uint32 flag, int8 * pktBuf, uint8 hdrlen,
					 uint8 Cbit, uint8 Rbit, uint8 Kbit, uint8 Sbit, uint8 sbit,
					 uint8 recur, uint8	Abit, uint8	otherflags, uint8 ver, uint16 protocol) {
	uint8	pptpflagsx1,pptpflagsx2;
	int8 * tmpPtr = (int8 *)(pktBuf + l4OffsetV6(flag, pktBuf, hdrlen));
	greHdr_t * pptpPtr = (greHdr_t *)tmpPtr;

	if(!l4FlagValidPptpV6(flag))
		return;

	memset(tmpPtr, 0, sizeof(greHdr_t));

	pptpflagsx1 = 0x00;

	if(Cbit)
		pptpflagsx1 |= GRE_C_BIT;
	if(Rbit)
		pptpflagsx1 |= GRE_R_BIT;
	if(Kbit)
		pptpflagsx1 |= GRE_K_BIT;
	if(Sbit)
		pptpflagsx1 |= GRE_S_BIT;
	if(sbit)
		pptpflagsx1 |= GRE_s_BIT;

	pptpflagsx1 |= (recur & ~GRE_FLAGS_x1);

	pptpPtr->gre_flags_x1 = pptpflagsx1;

	pptpflagsx2 = 0x00;

	if(Abit)
		pptpflagsx2 |= 0x80;

	pptpflagsx2 |= ((otherflags & 0x0f)<<3);
	pptpflagsx2 |= (ver & 0x07);
	pptpPtr->gre_flags_x2 = pptpflagsx2;
	pptpPtr->gre_protocol = htons(protocol);

}

void l4FormPptpV6Key (uint32 flag, int8 * pktBuf, uint8 hdrlen,
					uint16 callid, uint32 seqno, uint32 ackno) {
	int8 * tmpPtr = (int8 *)(pktBuf + l4OffsetV6(flag, pktBuf, hdrlen));
	greHdr_t * pptpPtr = (greHdr_t *)tmpPtr;

	if(!l4FlagValidPptpV6(flag))
		return;

	pptpPtr->gre_callid = htons(callid);
	pptpPtr->gre_seqno = htonl(seqno);
	pptpPtr->gre_ackno = htonl(ackno);
}

void l4FormPptpV6Other (uint32 flag, int8 * pktBuf, uint8 hdrlen, int8 * content, uint32 size) {
	int8 * cntPtr = (int8 *)(pktBuf + l4OffsetV6(flag, pktBuf, hdrlen) + sizeof(greHdr_t));
	int8 * tmpPtr = (int8 *)(pktBuf + l4OffsetV6(flag, pktBuf, hdrlen));
	greHdr_t * pptpPtr = (greHdr_t *)tmpPtr;

	if(!l4FlagValidPptpV6(flag))
		return;

	pptpPtr->gre_length = htons(size);

	memcpy(cntPtr, content, size);
	//l3FormIpProtocol(flag, pktBuf, IP_PROTO_PPTP);
	if((flag&ehMASK)==0)l3FormIpv6NextHeader(flag, pktBuf, IP_PROTO_PPTP);
	l3FormIpv6TotalLen(flag, pktBuf, hdrlen, size + sizeof(greHdr_t));
}

static int32 l4FlagValidIcmp(uint32 flag) {
	if(bitCount(flag & L4_MASK) > 1 || (flag & L4_ICMP) != L4_ICMP)
		return 0;
	return 1;
}

void l4FormIcmpBasic(uint32 flag, int8 * pktBuf, uint8 type, uint8 code) {
	int8 * tmpPtr = pktBuf + l4Offset(flag, pktBuf);
	icmpHdr_t * icmpPtr = (icmpHdr_t *)tmpPtr;

	if(!l4FlagValidIcmp(flag))
		return;
//	rtlglue_printf("*****************the type is [%d]\n", type);
//	rtlglue_printf("****************the code is [%d]\n", code);
	memset(tmpPtr, 0, sizeof(icmpHdr_t));
	icmpPtr->icmp_type = type;
	icmpPtr->icmp_code = code;
	l3FormIpProtocol(flag, pktBuf, IP_PROTO_ICMP);
	l3FormIpTotalLen(flag, pktBuf, sizeof(icmpHdr_t));
	l4FormIcmpChecksum(flag, pktBuf);
}
//added by liujuan
void l4FormIcmpBasicV6(uint32 flag, int8 * pktBuf, uint8 hdrlen, uint8 type, uint8 code, uint8 * dip, uint8 segleft) {
	int8 *tmpPtr = pktBuf+l4OffsetV6(flag, pktBuf, hdrlen);
	icmpHdr_t * icmpPtr = (icmpHdr_t *)tmpPtr;

	if(!l4FlagValidIcmp(flag))
		return;
//	rtlglue_printf("in form v6 icmp basic\n");
//	rtlglue_printf("*****************the type is [%d]\n", type);
 //      rtlglue_printf("****************the code is [%d]\n", code);
//	rtlglue_printf("the dip[0] is [%d]\n", dip[0]);
  //      rtlglue_printf("the dip[1] is [%d]\n", dip[1]);
	memset(tmpPtr, 0, sizeof(icmpHdr_t));
	icmpPtr->icmp_type = type;
	icmpPtr->icmp_code = code;

	l3FormIpv6TotalLen(flag, pktBuf, hdrlen, sizeof(icmpHdr_t));
	l4FormIcmpChecksumV6(flag, pktBuf, hdrlen, dip, segleft);
}	
void l4FormIcmpEcho(uint32 flag, int8 * pktBuf, uint16 id, uint16 seq) {
	int8 * tmpPtr = pktBuf + l4Offset(flag, pktBuf);
	icmpHdr_t * icmpPtr = (icmpHdr_t *)tmpPtr;

	if(!l4FlagValidIcmp(flag))
		return;

	icmpPtr->content.echo_s.icd_id = htons(id);
	icmpPtr->content.echo_s.icd_seq = htons(seq);
	l3FormIpTotalLen(flag, pktBuf, sizeof(icmpHdr_t));
	l4FormIcmpChecksum(flag, pktBuf);
}

//added by liujuan
void l4FormIcmpEchoV6(uint32 flag, int8 * pktBuf, uint8 hdrlen, uint16 id, uint16 seq,uint8 * dip, uint8 segleft) {
	int8 * tmpPtr=pktBuf + l4OffsetV6(flag, pktBuf, hdrlen);
	icmpHdr_t * icmpPtr=(icmpHdr_t *)tmpPtr;

	if(!l4FlagValidIcmp(flag))
		return;
//	rtlglue_printf("in form icmp echo\n");
//	rtlglue_printf("*************the id is [%d]\n", id);
//	rtlglue_printf("****************the seq is [%d]\n", seq);
//	rtlglue_printf("the dip[0] is [%d]\n", dip[0]);
  //      rtlglue_printf("the dip[1] is [%d]\n", dip[1]);
	icmpPtr->content.echo_s.icd_id = htons(id);
	icmpPtr->content.echo_s.icd_seq = htons(seq);
//	rtlglue_printf("*************the id is [%d]\n", icmpPtr->content.echo_s.icd_id);
  //      rtlglue_printf("****************the seq is [%d]\n", icmpPtr->content.echo_s.icd_seq);
	l3FormIpv6TotalLen(flag, pktBuf, hdrlen, sizeof(icmpHdr_t));
	l4FormIcmpChecksumV6(flag, pktBuf, hdrlen, dip, segleft);
}
void l4FormIcmpData(uint32 flag, int8 * pktBuf, int8 * data, uint32 size) {
	int8 * tmpPtr = (int8 *)(pktBuf + l4Offset(flag, pktBuf) + sizeof(icmpHdr_t));
//	int8 * tmpPtr = (int8 *)(pktBuf + l4Offset(flag, pktBuf));
	//icmpHdr_t * icmpPtr = (icmpHdr_t *)(pktBuf + l4Offset(flag, pktBuf));

	if(!l4FlagValidIcmp(flag))
		return;

	memcpy(tmpPtr, data, size);

	l3FormIpTotalLen(flag, pktBuf, size+sizeof(icmpHdr_t));
	l4FormIcmpChecksum(flag, pktBuf);
}

//added by liujuan
void l4FormIcmpDataV6(uint32 flag, int8 * pktBuf, uint8 hdrlen, int8 * data, uint32 size, uint8 * dip, uint8 segleft) {
	int8 * tmpPtr= (int8 *)(pktBuf + l4OffsetV6(flag, pktBuf, hdrlen)+ sizeof(icmpHdr_t));

	if(!l4FlagValidIcmp(flag))
		return;
//	rtlglue_printf("in form icmp data \n");
//	rtlglue_printf("the data length is [%d]\n", size);
	memcpy(tmpPtr, data, size);
//	rtlglue_printf("the dip[0] is [%d]\n", dip[0]);
  //      rtlglue_printf("the dip[1] is [%d]\n", dip[1]);
	l3FormIpv6TotalLen(flag, pktBuf,hdrlen, size+sizeof(icmpHdr_t));
	l4FormIcmpChecksumV6(flag, pktBuf, hdrlen, dip, segleft);
}
void l4FormIcmpChecksum(uint32 flag, int8 * pktBuf) {
	ipHdr_t * ipPtr = (ipHdr_t *)(pktBuf + l3Offset(flag, pktBuf));
	icmpHdr_t * icmpPtr = (icmpHdr_t*)(pktBuf + l4Offset(flag, pktBuf));
	uint16 cksum;

	if(!l4FlagValidIcmp(flag))
		return;

	icmpPtr->icmp_cksum = 0;
	cksum = ipcsum((uint16*)icmpPtr,  ntohs(ipPtr->ip_len) - ipPtr->ip_hl*4, 0);
	
	if(flag & L4_CKSUM_ERR)
		icmpPtr->icmp_cksum = htons(cksum);
	else
		icmpPtr->icmp_cksum = htons(~cksum);
}

//added by liujuan
void l4FormIcmpChecksumV6(uint32 flag, int8 * pktBuf, uint8 hdrlen, uint8 * dip, uint8 segleft) {
	ip6Hdr_t * ipPtr=(ip6Hdr_t *)(pktBuf + l3Offset(flag, pktBuf));
	icmpHdr_t * icmpPtr=(icmpHdr_t *)(pktBuf + l4OffsetV6(flag, pktBuf, hdrlen));
	uint16 tmp,csum;
	uint8 nh;
	uint32 len, lenb;
	
	nh=58;
	if(!l4FlagValidIcmp(flag))
		return;
//	rtlglue_printf("in form icmp checkusm\n");
	if ((flag & RouHdr)&& (segleft!=0)) {
//		rtlglue_printf("\n********************************\n");
		csum=ipcsum((uint16 *)ipPtr->ip_src, 16, 0);
		csum=ipcsum((uint16 *)dip, 16, csum);
//		rtlglue_printf("the dip[0] is [%d]\n", dip[0]);
//		rtlglue_printf("the dip[1] is [%d]\n", dip[1]);
	}
	else
		csum=ipcsum((uint16 *)ipPtr->ip_src, 32, 0);
	len=l4PayloadLenV6(flag, pktBuf, hdrlen);
	lenb=htonl(len);
	csum=ipcsum((uint16 *)&lenb, 4, csum);
	tmp=nh;
	tmp=htons(tmp);
	csum=ipcsum(&tmp, 2, csum);
	icmpPtr->icmp_cksum=0;
	csum=ipcsum((uint16 *)icmpPtr, len, csum);
//	rtlglue_printf("the icmp checksum is [%d]", htons(~csum));
	if(flag & L4_CKSUM_ERR)
		icmpPtr->icmp_cksum=htons(csum);
	else
		icmpPtr->icmp_cksum=htons(~csum);
//	rtlglue_printf("the checksum in icmp pkt is [%d]\n", icmpPtr->icmp_cksum);
	
	
}
static int32 l4FlagValidIgmp(uint32 flag) {
	if(!l3FlagValidIp(flag) || 
		bitCount(flag & L4_MASK) > 1 || (flag & L4_IGMP) != L4_IGMP)
		return 0;
	return 1;
}

void l4FormIgmpBasic(uint32 flag, int8 * pktBuf, uint8 ver, uint8 type, uint8 respTime, uint32 addr,
	uint8 qrsq, uint8 qqic, uint16 qnofs, uint32 * qslist, uint16 rnofg) {
	//ipHdr_t * ipPtr = (ipHdr_t *)(pktBuf + l3Offset(flag));
	igmpHdr_t * igmpPtr = (igmpHdr_t*)(pktBuf + l4Offset(flag, pktBuf));

	if(!l4FlagValidIgmp(flag))
		return;

	l3FormIpProtocol(flag, pktBuf, IP_PROTO_IGMP);

	switch (ver)
	{
		case 1:	// V1
		case 2:	// V2
			igmpPtr->igmp_type = type;
			igmpPtr->igmp_respTime = respTime;
			igmpPtr->igmp_cksum = 0;
			igmpPtr->igmp_group = htonl(addr);
			l3FormIpTotalLen(flag, pktBuf, 8);
			break;
		case 3:	// V3
			if (type == IGMP_MEMBERSHIP_QUERY)
			{
				int i;
				uint32 *p, *s;
				igmpPtr->igmp_type = type;
				igmpPtr->igmp_respTime = respTime;
				igmpPtr->igmp_cksum = 0;
				igmpPtr->igmp_group = htonl(addr);
				igmpPtr->igmp_QRSQ = qrsq;
				igmpPtr->igmp_QQIC = qqic;
				igmpPtr->igmp_QNOFS = htons(qnofs);
				p = &(igmpPtr->igmp_QSLIST);
				s = qslist;
				for (i = 0 ; i < qnofs ; i ++, p++, s ++)
					*p = htonl(*s);

				l3FormIpTotalLen(flag, pktBuf, (12 + (4 * qnofs)));
			}else if (type == IGMP_V3_MEMBERSHIP_REPORT)
			{
				igmpPtr->igmp_type = type;
				igmpPtr->igmp_respTime = 0;
				igmpPtr->igmp_cksum = 0;
				igmpPtr->igmp_RRESV = 0;
				igmpPtr->igmp_RNOFG = htons(rnofg);
				l3FormIpTotalLen(flag, pktBuf, 8);
			}
			break;
	}
	
	l4FormIgmpChecksum(flag, pktBuf);
}

void l4FormIgmpContent(uint32 flag, int8 * pktBuf, int8 * content, uint32 size, uint8 ver, uint8 type, uint16 qnofs) {
	int8 * cntPtr;
	int32 igmpHdrLen = 0;
	
	if(!l4FlagValidIgmp(flag))
		return;

	switch (ver)
	{
		case 1:
		case 2:
			igmpHdrLen = 8;
			break;
		case 3:
			if (type == IGMP_MEMBERSHIP_QUERY)
				igmpHdrLen = (12 + (4 * qnofs));
			else
				igmpHdrLen = 8;
			break;
	}

	cntPtr = (int8 *) (pktBuf + l4Offset(flag, pktBuf) + igmpHdrLen);

	memcpy(cntPtr, content, size);
	l3FormIpTotalLen(flag, pktBuf, (size + igmpHdrLen));
	switch (ver)
	{
		case 1:
		case 2:
			l3FormIpTotalLen(flag, pktBuf, (size + 8));
			break;
		case 3:
			if (type == IGMP_MEMBERSHIP_QUERY)
				l3FormIpTotalLen(flag, pktBuf, (size + (12 + (4 * qnofs))));
			else
				l3FormIpTotalLen(flag, pktBuf, (size + 8));
			break;
	}
	l4FormIgmpChecksum(flag, pktBuf);
}

void l4FormIgmpChecksum(uint32 flag, int8 * pktBuf) {
	ipHdr_t * ipPtr = (ipHdr_t *)(pktBuf + l3Offset(flag, pktBuf));
	igmpHdr_t * igmpPtr = (igmpHdr_t*)(pktBuf + l4Offset(flag, pktBuf));
	uint16 cksum;

	if(!l4FlagValidIgmp(flag))
		return;

	igmpPtr->igmp_cksum = 0;

	cksum = ipcsum((uint16*)igmpPtr, ntohs(ipPtr->ip_len) - ipPtr->ip_hl*4, 0);

	if(flag & L4_CKSUM_ERR)
		igmpPtr->igmp_cksum = htons(cksum);
	else
		igmpPtr->igmp_cksum = htons(~cksum);
}

static int32 l4FlagValidUdp(uint32 flag) {
	if(bitCount(flag & L4_MASK) > 1 || (flag & L4_UDP) != L4_UDP)
		return 0;
	return 1;
}

void l4FormUdpBasic(uint32 flag, int8 * pktBuf, uint16 sPort, uint16 dPort) {
	udpHdr_t * udpPtr = (udpHdr_t*)(pktBuf + l4Offset(flag, pktBuf));

	if(!l4FlagValidUdp(flag))
		return;

	/* clean header */
	memset( udpPtr, 0, sizeof(udpHdr_t) );
	
	#ifdef _ENDINEFREE_IP_PORT
	udpPtr->uh_sport = sPort;
	udpPtr->uh_dport = dPort;
	#else
	udpPtr->uh_sport = htons(sPort);
	udpPtr->uh_dport = htons(dPort);
	#endif
	l3FormIpProtocol(flag, pktBuf, IP_PROTO_UDP);
	l3FormIpTotalLen(flag, pktBuf, sizeof(udpHdr_t));
	l4FormUdpChecksum(flag, pktBuf);
}

void l4FormUdpLiteBasic(uint32 flag, int8 * pktBuf, uint16 sPort, uint16 dPort) {
	udpHdr_t * udpPtr = (udpHdr_t*)(pktBuf + l4Offset(flag, pktBuf));

	if(!l4FlagValidUdp(flag))
		return;

	/* clean header */
	memset( udpPtr, 0, sizeof(udpHdr_t) );
	
	#ifdef _ENDINEFREE_IP_PORT
	udpPtr->uh_sport = sPort;
	udpPtr->uh_dport = dPort;
	#else
	udpPtr->uh_sport = htons(sPort);
	udpPtr->uh_dport = htons(dPort);
	#endif
	l3FormIpProtocol(flag, pktBuf, 0x88);
	l3FormIpTotalLen(flag, pktBuf, sizeof(udpHdr_t));
	l4FormUdpChecksum(flag, pktBuf);
}

//added by liujuan
void l4FormUdpBasicV6(uint32 flag, int8 * pktBuf, uint8 hdrlen, uint16 sPort, uint16 dPort, uint8 * dip, uint8 segleft) {
	udpHdr_t * udpPtr=(udpHdr_t *)(pktBuf+l4OffsetV6(flag, pktBuf, hdrlen));

	if (!l4FlagValidUdp(flag))
		return;

	memset(udpPtr, 0 , sizeof(udpHdr_t));

	#ifdef _ENDINEFREE_IP_PORT
	udpPtr->uh_sport = sPort;
	udpPtr->uh_dport = dPort;
	#else
	udpPtr->uh_sport = htons(sPort);
	udpPtr->uh_dport = htons(dPort);
	#endif

	l3FormIpv6TotalLen(flag, pktBuf, hdrlen, sizeof(udpHdr_t));
	l4FormUdpChecksumV6(flag, pktBuf, hdrlen, dip, segleft);
}
void l4FormUdpContent(uint32 flag, int8 * pktBuf, int8 * content, uint32 size) {
	int8 * cntPtr = pktBuf + l4Offset(flag, pktBuf) + sizeof(udpHdr_t);
	udpHdr_t * udpPtr = (udpHdr_t*)(pktBuf + l4Offset(flag, pktBuf));

	if(!l4FlagValidUdp(flag))
		return;
	
	memcpy(cntPtr, content, size);
	udpPtr->uh_ulen = htons(size+sizeof(udpHdr_t));
	l3FormIpTotalLen(flag, pktBuf, size+sizeof(udpHdr_t));
	l4FormUdpChecksum(flag, pktBuf);
}

void l4FormUdpContent_for_udp_bomb(uint32 flag, int8 * pktBuf, int8 * content, uint32 size) {
	int8 * cntPtr = pktBuf + l4Offset(flag, pktBuf) + sizeof(udpHdr_t);
	udpHdr_t * udpPtr = (udpHdr_t*)(pktBuf + l4Offset(flag, pktBuf));

	int udp_bomb_padding_len = 3;

	if(!l4FlagValidUdp(flag))
		return;

	memcpy(cntPtr, content, size);
	udpPtr->uh_ulen = htons(size+sizeof(udpHdr_t)) + udp_bomb_padding_len;
	l3FormIpTotalLen(flag, pktBuf, size+sizeof(udpHdr_t));
	l4FormUdpChecksum(flag, pktBuf);
}

void l4FormUdpLenZero(uint32 flag, int8 * pktBuf) {
	//int8 * cntPtr = pktBuf + l4Offset(flag, pktBuf) + sizeof(udpHdr_t);
	//udpHdr_t * udpPtr = (udpHdr_t*)(pktBuf + l4Offset(flag, pktBuf));

	if(!l4FlagValidUdp(flag))
		return;
	
	l3FormIpTotalLenZero(flag, pktBuf);
	l4FormUdpChecksum(flag, pktBuf);
}

//added by liujuan
void l4FormUdpContentV6(uint32 flag, int8 * pktBuf, uint8 hdrlen, int8 * content, uint32 size, uint8 * dip, uint8 segleft) {
	int8 * cntPtr=pktBuf+l4OffsetV6(flag, pktBuf, hdrlen)+sizeof(udpHdr_t);
	udpHdr_t * udpPtr=(udpHdr_t *)(pktBuf+l4OffsetV6(flag, pktBuf, hdrlen));

	if (!l4FlagValidUdp(flag))
		return;

	memcpy(cntPtr, content, size);
	udpPtr->uh_ulen=htons(size+sizeof(udpHdr_t));
	l3FormIpv6TotalLen(flag, pktBuf, hdrlen, size+sizeof(udpHdr_t));
	l4FormUdpChecksumV6(flag, pktBuf, hdrlen, dip, segleft);
}

void l4FormUdpContentV6_for_udp_bomb(uint32 flag, int8 * pktBuf, uint8 hdrlen, int8 * content, uint32 size, uint8 * dip, uint8 segleft) {
	int8 * cntPtr=pktBuf+l4OffsetV6(flag, pktBuf, hdrlen)+sizeof(udpHdr_t);
	udpHdr_t * udpPtr=(udpHdr_t *)(pktBuf+l4OffsetV6(flag, pktBuf, hdrlen));

	int udp_bomb_padding_len = 3;

	if (!l4FlagValidUdp(flag))
		return;

	memcpy(cntPtr, content, size);
	udpPtr->uh_ulen=htons(size+sizeof(udpHdr_t)) + udp_bomb_padding_len;
	l3FormIpv6TotalLen(flag, pktBuf, hdrlen, size+sizeof(udpHdr_t));
	l4FormUdpChecksumV6(flag, pktBuf, hdrlen, dip, segleft);
}

void l4FormUdpChecksum(uint32 flag, int8 * pktBuf) {
	ipHdr_t * ipPtr = (ipHdr_t *)(pktBuf + l3Offset(flag, pktBuf));
	udpHdr_t * udpPtr = (udpHdr_t*)(pktBuf + l4Offset(flag, pktBuf));
	uint16 tmp, csum;

	if(!l4FlagValidUdp(flag))
		return;

	csum = ipcsum((uint16 *)&ipPtr->ip_src, 8, 0);
	tmp = htons(IP_PROTO_UDP);
	csum = ipcsum(&tmp, 2, csum);
	if((flag & DUAL_OUTER_HDR)==0)	// cheney patch for dual header pkt gen
		csum = ipcsum((uint16 *)&udpPtr->uh_ulen, 2, csum);
	udpPtr->uh_sum = 0;
	if((flag & DUAL_OUTER_HDR)==0)	// cheney patch for dual header pkt gen
		csum = ipcsum((uint16 *)udpPtr, ntohs(udpPtr->uh_ulen), csum);
	else
	{
		// for L2TP outer udp header
		uint16 udplen = ntohs(udpPtr->uh_ulen);
		//printf("@@ udp len = 0x%x\r\n", udplen);
		udpPtr->uh_ulen = 0;
		csum = ipcsum((uint16 *)udpPtr, udplen, csum);
	
	}
	if((flag & L4_CKSUM_ERR) || (csum==0xffff))
		udpPtr->uh_sum = htons(csum);
	else
		udpPtr->uh_sum = htons(~csum);
	
	if(flag & L4_NO_CKSUM)
		udpPtr->uh_sum = 0;
}

//added by liujuan
void l4FormUdpChecksumV6(uint32 flag, int8 * pktBuf, uint8 hdrlen, uint8 * dip, uint8 segleft) {
	ip6Hdr_t * ipPtr=(ip6Hdr_t *)(pktBuf + l3Offset(flag, pktBuf));
	udpHdr_t * udpPtr=(udpHdr_t *)(pktBuf + l4OffsetV6(flag, pktBuf, hdrlen));
	uint16 tmp, csum;
	uint8 nh;

	nh=17;
	
	if(!l4FlagValidUdp(flag))
		return;
	if ((flag & RouHdr) && (segleft!=0)){
		csum=ipcsum((uint16 *) &ipPtr->ip_src, 16, 0);
		//tmp=(uint16) *dip;              //?????????????????????????????????????????????????????????????????????????????????
		csum=ipcsum((uint16 *)dip, 16, csum);
	}
	else
		csum=ipcsum((uint16 *)&ipPtr->ip_src, 32, 0);
//	rtlglue_printf("the udp uh_ulen is [%d]\n", udpPtr->uh_ulen);
//	rtlglue_printf("the v6 payload lenth is [%d]\n", ipPtr->ip_len);
	csum=ipcsum((uint16 *)&udpPtr->uh_ulen, 2, csum);
	tmp=nh;
	tmp=htons(tmp);
	csum=ipcsum(&tmp, 2, csum);
	udpPtr->uh_sum=0;
	csum = ipcsum((uint16 *)udpPtr, ntohs(udpPtr->uh_ulen),csum);
	if(flag & L4_NO_CKSUM)
		udpPtr->uh_sum=0;
	else if ((flag & L4_CKSUM_ERR) || (csum==0xffff))
		udpPtr->uh_sum=htons(csum);
	else
		udpPtr->uh_sum=htons(~csum);
	
}
static int32 l4FlagValidTcp(uint32 flag) {
//	rtlglue_printf("the flag in l4flagvalidtcp is [%d]\n", flag);
	if(bitCount(flag & L4_MASK) > 1 || (flag & L4_TCP) != L4_TCP)
		return 0;
	return 1;
}

void l4FormTcpBasic(uint32 flag, int8 * pktBuf, uint16 sPort, uint16 dPort, uint8 hlen) {
	tcpHdr_t * tcpPtr = (tcpHdr_t*)(pktBuf + l4Offset(flag, pktBuf));
	uint32 tcpHlen;

	if(!l4FlagValidTcp(flag))
		return;
	
	if(hlen%4)
		tcpHlen = hlen/4 + 1;
	else
		tcpHlen = hlen/4;

	memset(tcpPtr, 0,tcpHlen*4);
	
	#ifdef _ENDINEFREE_IP_PORT
	tcpPtr->th_sport = sPort;
	tcpPtr->th_dport = dPort;
	#else
	tcpPtr->th_sport = htons(sPort);
	tcpPtr->th_dport = htons(dPort);
	#endif

	tcpPtr->th_off = tcpHlen;

	l3FormIpProtocol(flag, pktBuf, IP_PROTO_TCP);
	l3FormIpTotalLen(flag, pktBuf, sizeof(tcpHdr_t));
	l4FormTcpChecksum(flag, pktBuf);
}

//added by liujuan
void l4FormTcpBasicV6(uint32 flag, int8 * pktBuf, uint8 hdrlen, uint16 sPort, uint16 dPort, uint8 hlen, uint8* dip, uint8 segleft){
	tcpHdr_t * tcpPtr=(tcpHdr_t *)(pktBuf + l4OffsetV6(flag, pktBuf, hdrlen));
	uint32 tcpHlen;
//	rtlglue_printf("in form tcp basic \n\n");
//	rtlglue_printf("the flag is [%d]\n\n", flag);
	if(!l4FlagValidTcp( flag)){
//		rtlglue_printf("the tcp flag is error \n\n");
		return;
	}

	if(hlen%4)
		tcpHlen = hlen/4 +1;
	else
		tcpHlen=hlen/4;
	memset(tcpPtr, 0,tcpHlen*4);

	#ifdef _ENDINEFREE_IP_PORT
	tcpPtr->th_sport = sPort;
	tcpPtr->th_dport = dPort;
	#else
	tcpPtr->th_sport = htons(sPort);
	tcpPtr->th_dport = htons(dPort);
	#endif
//	rtlglue_printf("befor form totoal len");
	tcpPtr->th_off = tcpHlen;
//	rtlglue_printf("the ipv6 packet lenth in l4formtcpbasic is [%d]\n", sizeof(tcpHdr_t));
	l3FormIpv6TotalLen(flag, pktBuf, hdrlen, sizeof(tcpHdr_t));
	l4FormTcpChecksumV6(flag, pktBuf, hdrlen, dip, segleft);

}

void l4FormTcpBasicV6_specify_offset(uint32 flag, int8 * pktBuf, uint8 hdrlen, uint16 sPort, uint16 dPort, uint8 hlen, uint8* dip, uint8 segleft, uint8 offset){
	tcpHdr_t * tcpPtr=(tcpHdr_t *)(pktBuf + l4OffsetV6(flag, pktBuf, hdrlen));
	uint32 tcpHlen;
//	rtlglue_printf("in form tcp basic \n\n");
//	rtlglue_printf("the flag is [%d]\n\n", flag);
	if(!l4FlagValidTcp( flag)){
//		rtlglue_printf("the tcp flag is error \n\n");
		return;
	}

	if(hlen%4)
		tcpHlen = hlen/4 +1;
	else
		tcpHlen=hlen/4;
	memset(tcpPtr, 0,tcpHlen*4);

	#ifdef _ENDINEFREE_IP_PORT
	tcpPtr->th_sport = sPort;
	tcpPtr->th_dport = dPort;
	#else
	tcpPtr->th_sport = htons(sPort);
	tcpPtr->th_dport = htons(dPort);
	#endif
//	rtlglue_printf("befor form totoal len");
	tcpPtr->th_off = offset;
//	rtlglue_printf("the ipv6 packet lenth in l4formtcpbasic is [%d]\n", sizeof(tcpHdr_t));
	l3FormIpv6TotalLen(flag, pktBuf, hdrlen, sizeof(tcpHdr_t));
	l4FormTcpChecksumV6(flag, pktBuf, hdrlen, dip, segleft);

}

void l4FormTcpFlow(uint32 flag, int8 * pktBuf, uint32 seq, uint32 ack, uint16 winSize, uint8 tcpFlag, uint16 urgPtr) {
	tcpHdr_t * tcpPtr = (tcpHdr_t*)(pktBuf + l4Offset(flag, pktBuf));

	if(!l4FlagValidTcp(flag))
		return;

	tcpPtr->th_seq = htonl(seq);
	tcpPtr->th_ack = htonl(ack);
	tcpPtr->th_win = htons(winSize);
	tcpPtr->th_flags = tcpFlag;
	tcpPtr->th_urp = htons(urgPtr);
//	rtlglue_printf("the ipv6 pkt length in l4formtcpflow is [%d]\n", sizeof(tcpHdr_t));
	l3FormIpTotalLen(flag, pktBuf, sizeof(tcpHdr_t));
	l4FormTcpChecksum(flag, pktBuf);
}

//added by liujuan
void l4FormTcpFlowV6(uint32 flag, int8 * pktBuf, uint8 hdrlen, uint32 seq, uint32 ack, uint16 winSize, uint8 tcpFlag, uint16 urgPtr, uint8* dip, uint8 segleft) {
	tcpHdr_t * tcpPtr=(tcpHdr_t*)(pktBuf + l4OffsetV6(flag, pktBuf, hdrlen));

	if(!l4FlagValidTcp(flag))
		return;

	tcpPtr->th_seq = htonl(seq);
	tcpPtr->th_ack = htonl(ack);
	tcpPtr->th_win = htons(winSize);
	tcpPtr->th_flags = tcpFlag;
	tcpPtr->th_urp = htons(urgPtr);
//	rtlglue_printf("the ipv6 pkt length in l4formtcpflow is [%d]\n", sizeof(tcpHdr_t));
	l3FormIpv6TotalLen(flag, pktBuf, hdrlen, sizeof(tcpHdr_t));
	l4FormTcpChecksumV6(flag, pktBuf, hdrlen, dip, segleft);
}

void l4FormTcpOption(uint32 flag, int8 * pktBuf, int8 * option, uint8 size) {
	tcpHdr_t * tcpPtr = (tcpHdr_t*)(pktBuf + l4Offset(flag, pktBuf));
	int8 * optPtr = pktBuf + l4Offset(flag, pktBuf) + sizeof(tcpHdr_t);

	if(!l4FlagValidTcp(flag))
		return;
	
	if(size + sizeof(tcpHdr_t) > (uint32)(tcpPtr->th_off*4))
		return;
	
	memcpy(optPtr, option, size);
	
	l3FormIpTotalLen(flag, pktBuf, (sizeof(tcpHdr_t)+size));
	l4FormTcpChecksum(flag, pktBuf);
}

//added by liujuan
void l4FormTcpOptionV6(uint32 flag, int8 * pktBuf, uint8 hdrlen, uint8 * option, uint8 size, uint8* dip, uint8 segleft) {
	tcpHdr_t * tcpPtr=(tcpHdr_t *)(pktBuf + l4OffsetV6(flag, pktBuf, hdrlen));
	int8 * optPtr= pktBuf + l4OffsetV6(flag, pktBuf, hdrlen) + sizeof(tcpHdr_t);

	if(!l4FlagValidTcp(flag))
		return;

	if(size + sizeof(tcpHdr_t) > (uint32)(tcpPtr->th_off*4))
		return;

	memcpy(optPtr, option, size);
//	rtlglue_printf("the ipv6 pkt length in l4formtcpopton is [%d]\n", sizeof(tcpHdr_t)+size);
	l3FormIpv6TotalLen(flag, pktBuf, hdrlen, (sizeof(tcpHdr_t)+size));
	l4FormTcpChecksumV6(flag, pktBuf, hdrlen, dip, segleft);
//	rtlglue_printf("the tcp checksum in formtcpoption is [%d\n]",tcpPtr->th_sum);
}

void l4FormTcpContent(uint32 flag, int8 * pktBuf, int8 * content, uint32 size) {
	tcpHdr_t * tcpPtr = (tcpHdr_t*)(pktBuf + l4Offset(flag, pktBuf));
	int8 * cntPtr = pktBuf + l4Offset(flag, pktBuf) + tcpPtr->th_off*4;

	if(!l4FlagValidTcp(flag))
		return;

	memcpy(cntPtr, content, size);
	l3FormIpTotalLen(flag, pktBuf, size+tcpPtr->th_off*4);
	l4FormTcpChecksum(flag, pktBuf);
//	rtlglue_printf("the tcp checksum in formtcpcontent is [%d\n]",tcpPtr->th_sum);
}

//added by liujuan
void l4FormTcpContentV6(uint32 flag, int8 * pktBuf, uint8 hdrlen, int8 * content, uint32 size, uint8* dip, uint8 segleft) {
	tcpHdr_t * tcpPtr =(tcpHdr_t *)(pktBuf + l4OffsetV6(flag, pktBuf, hdrlen));
	int8 *cntPtr=pktBuf + l4OffsetV6(flag, pktBuf, hdrlen)+tcpPtr->th_off*4;

	if(!l4FlagValidTcp(flag))
		return;

	memcpy(cntPtr, content, size);
//	rtlglue_printf("the ipv6 pkt length in l4formtcpcontent is [%d]\n", size+tcpPtr->th_off*4);
	l3FormIpv6TotalLen(flag, pktBuf, hdrlen, size+tcpPtr->th_off*4);
	l4FormTcpChecksumV6(flag, pktBuf, hdrlen, dip, segleft);
//	rtlglue_printf("the tcp checksum in formtcpcontent is [%d\n]",tcpPtr->th_sum);
}

void l4FormTcpChecksum(uint32 flag, int8 * pktBuf) {
	ipHdr_t * ipPtr = (ipHdr_t *)(pktBuf + l3Offset(flag, pktBuf));
	tcpHdr_t * tcpPtr = (tcpHdr_t*)(pktBuf + l4Offset(flag, pktBuf));
	uint16 tmp, csum;

	if(!l4FlagValidTcp(flag))
		return;

	csum = ipcsum((uint16 *)&ipPtr->ip_src, 8, 0);
	tmp = htons(IP_PROTO_TCP);
	csum = ipcsum(&tmp, 2, csum);
	tmp = htons(l4PayloadLen(flag, pktBuf));
	//rtlglue_printf("\n\nthe tmp is [%d]\n\n", tmp);
	csum = ipcsum(&tmp, 2, csum);
	tcpPtr->th_sum = 0;
	csum = ipcsum((uint16 *)tcpPtr, l4PayloadLen(flag, pktBuf), csum);
	if(flag & L4_CKSUM_ERR) 
		tcpPtr->th_sum = htons(csum);
	else
		tcpPtr->th_sum = htons(~csum);
	//rtlglue_printf("the tcp checksum is [%d]\n\n", csum);
}

//added by liujuan
void l4FormTcpChecksumV6(uint32 flag, int8 * pktBuf, uint8 hdrlen, uint8* dip, uint8 segleft) {
	ip6Hdr_t * ipPtr=(ip6Hdr_t *)(pktBuf + l3Offset(flag, pktBuf));
	tcpHdr_t * tcpPtr=(tcpHdr_t *)(pktBuf + l4OffsetV6(flag, pktBuf, hdrlen));
	uint16 tmp,csum;
	uint32 len;
	uint32 lenb;
//	uint8 nh;

	if(!l4FlagValidTcp(flag))
		return;
	//nh=6;
//	rtlglue_printf("hello!! in the form tcp checksum!!!\n\n");
	if ((flag & RouHdr)&& (segleft!=0)) {
		csum=ipcsum((uint16 *)&ipPtr->ip_src, 16, 0);
		csum=ipcsum((uint16 *)dip, 16, csum);
	//	rtlglue_printf("                 here\n");
	}
	else
		csum=ipcsum((uint16 *)&ipPtr->ip_src, 32, 0);
//	rtlglue_printf("the csum1 is [%d]\n", csum);
//	rtlglue_printf("the ip address is [%d]\n", ipPtr->ip_src[0]);
//	rtlglue_printf("the ip address is [%d]\n", ipPtr->ip_src[1]);
//	rtlglue_printf("the ip address is [%d]\n", ipPtr->ip_src[2]);
//	rtlglue_printf("the ip address is [%d]\n", ipPtr->ip_src[3]);
//	rtlglue_printf("the ip address is [%d]\n", ipPtr->ip_src[4]);
//	tmp=htons(l4PayloadLenV6(flag, pktBuf, hdrlen));
	len=0;
	len=l4PayloadLenV6(flag, pktBuf, hdrlen);
	lenb=htonl(len);
//	len=htons(len);
//	tmp=htons(len);
//	rtlglue_printf("the v6 payload lenth is [%d]\n", len);
//	rtlglue_printf("\n\nthe tmp is [%d]\n\n", tmp);
	//tmp=30;
	//csum=ipcsum(&tmp, 4, csum);
//	tmp=htons(len);
	csum= ipcsum((uint16 *)&lenb, 4, csum);
//	rtlglue_printf("the csum2 is [%d]\n", csum);
	tmp=6;
	tmp=htons(tmp);
	csum=ipcsum(&tmp, 2, csum);
//	rtlglue_printf("the csum3 is [%d]\n", csum);
	tcpPtr->th_sum=0;
//	rtlglue_printf("              hi             ");
	csum=ipcsum((uint16 *)tcpPtr, len, csum);
//	rtlglue_printf("the cusm4 is [%d]\n", csum);
//	rtlglue_printf("the tcp checksum is [%d] \n\n", csum);
	if(flag & L4_CKSUM_ERR){
		tcpPtr->th_sum=htons(csum);
//		rtlglue_printf("the error tcp checksum is [%d]\n", tcpPtr->th_sum);
		}
	else {
		tcpPtr->th_sum=htons(~csum);
//		rtlglue_printf("the right tcp checksum is [%d]\n", htons(tcpPtr->th_sum));
//		rtlglue_printf("the right tcp checkusm without htons is [%d]\n", tcpPtr->th_sum);
		}
	//rtlglue_printf("the tcp checksum is [ ]\n\n");
}

uint32 formedPktLen(uint32 flag, int8 * pktBuf) {
	if(flag & L4_MASK)
		return l4PayloadLen(flag, pktBuf) + l4Offset(flag, pktBuf);

	return l4Offset(flag, pktBuf);
}
//added by liujuan
uint32 formedpktLenV6(uint32 flag, int8 * pktBuf, uint8 hdrlen) {
//	rtlglue_printf("the pkttype is [%d]\n", flag);
	if(flag & L4_MASK){
//		rtlglue_printf("the l4offset is [%d]\n", l4OffsetV6(flag, pktBuf, hdrlen));
//		rtlglue_printf("the l4payload is [%d]\n", l4PayloadLenV6(flag, pktBuf, hdrlen));
		return l4PayloadLenV6(flag, pktBuf, hdrlen)+ l4OffsetV6(flag,  pktBuf, hdrlen);
	}

	return l4PayloadLenV6(flag, pktBuf, hdrlen) + l4OffsetV6(flag, pktBuf, hdrlen);
}
uint32 formedFrameLen(uint32 flag, int8 * pktBuf, uint32 payloadLen) {

	return l3Offset(flag, pktBuf) + payloadLen;

}

#if 1 //cheney
void l2FormL2TPPPProtocol(uint32 flag, int8 * pktBuf, int length, uint16 tunnelID, uint16 sessionID, uint16 protocol, compressed_ppp_mode_t cmpPPPMode) {
	int offset = 0;
	int8 * tmpPtr = pktBuf;// + l4Offset(flag, pktBuf) + sizeof(udpHdr_t);
	uint16 *l2tpFlagPtr = (uint16 *)tmpPtr;

//printf("### L2TP - pktbuf = 0x%x, l4offset = %d\e\n", pktBuf, l4Offset(flag, pktBuf));
 #define L2TP_HDRFLAG_T     0x8000		// 1: control packet; 0: data packet
 #define L2TP_HDRFLAG_L     0x4000		// Length bit, must be 1 if control packet
 #define L2TP_HDRFLAG_S     0x0800		// 
 #define L2TP_HDRFLAG_O     0x0200
 #define L2TP_HDRFLAG_P     0x0100
 
 #define L2TP_HDR_VER_MASK  0x000F
 #define L2TP_HDR_VER_2     0x0002
 #define L2TP_HDR_VER_3     0x0003

	 memset(tmpPtr, 0, 8);

  	if(length>=0)
 	{
 		// flags
	 	*l2tpFlagPtr = htons(L2TP_HDRFLAG_L | L2TP_HDR_VER_2);
	 	offset+=1;
	 	
	 	// Length
	 	*(l2tpFlagPtr+offset)  = htons(length);
		offset+=1;
  	}else{
  		// flags
	 	*l2tpFlagPtr = htons(L2TP_HDR_VER_2);
	 	offset+=1;

		// without length field
  	}
	
 	// Tunnel ID
 	*(l2tpFlagPtr+offset)  = htons(tunnelID);
	offset+=1;
	
 	// Session ID
 	*(l2tpFlagPtr+offset)  = htons(sessionID);
	offset+=1;

	tmpPtr += (offset*2);

	switch(cmpPPPMode)
	{
		case COMPRESSED_PPP_FF0300XX:
			tmpPtr[0] = 0xff;
			tmpPtr[1] = 0x03;
			tmpPtr[2] = (protocol>>8) & 0xff;
			tmpPtr[3] = (protocol) & 0xff;
			break;
		case COMPRESSED_PPP_FF03XX:
			tmpPtr[0] = 0xff;
			tmpPtr[1] = 0x03;
			tmpPtr[2] = (protocol) & 0xff;
			break;
		case COMPRESSED_PPP_00XX:
			tmpPtr[0] = (protocol>>8) & 0xff;
			tmpPtr[1] = (protocol) & 0xff;
			break;
		case COMPRESSED_PPP_XX:
			tmpPtr[0] = (protocol) & 0xff;
			break;
		default:
			//not support
			break;
	}
	/*	
	_grepppHdr_t* grepppPtr = (_grepppHdr_t *) tmpPtr;
	memset(tmpPtr, 0, sizeof(_grepppHdr_t));
	grepppPtr->adress = 0xff;		// cheney ?? FIXME
	grepppPtr->control = 0x03;		// cheney ?? FIXME
	grepppPtr->proto = htons(protocol);
	*/
}

void l3FormGREPPPProtocol(uint32 flag, int8 * pktBuf, uint16 protocol, compressed_ppp_mode_t cmpPPPMode) {
	int8 * tmpPtr = pktBuf + l3Offset(flag, pktBuf);
	ipHdr_t * ipPtr = (ipHdr_t *)tmpPtr;
	tmpPtr += ipPtr->ip_hl * 4;

	switch(cmpPPPMode)
	{
		case COMPRESSED_PPP_FF0300XX:
			tmpPtr[0] = 0xff;
			tmpPtr[1] = 0x03;
			tmpPtr[2] = (protocol>>8) & 0xff;
			tmpPtr[3] = (protocol) & 0xff;
			break;
		case COMPRESSED_PPP_FF03XX:
			tmpPtr[0] = 0xff;
			tmpPtr[1] = 0x03;
			tmpPtr[2] = (protocol) & 0xff;
			break;
		case COMPRESSED_PPP_00XX:
			tmpPtr[0] = (protocol>>8) & 0xff;
			tmpPtr[1] = (protocol) & 0xff;
			break;
		case COMPRESSED_PPP_XX:
			tmpPtr[0] = (protocol) & 0xff;
			break;
		default:
			//not support
			break;
	}
	/*
	_grepppHdr_t* grepppPtr = (_grepppHdr_t *) tmpPtr;


	memset(tmpPtr, 0, sizeof(_grepppHdr_t));
	grepppPtr->adress = 0xff;		// cheney ?? FIXME
	grepppPtr->control = 0x03;		// cheney ?? FIXME
	grepppPtr->proto = htons(protocol);
	*/
}

void l3FormV6GREPPPProtocol(uint32 flag, int8 * pktBuf, uint8 extHdrLen, uint16 protocol, compressed_ppp_mode_t cmpPPPMode) {
	int8 * tmpPtr = pktBuf + l3Offset(flag, pktBuf);
	tmpPtr += 40;
	tmpPtr += extHdrLen;
	tmpPtr += 16;

	switch(cmpPPPMode)
	{
		case COMPRESSED_PPP_FF0300XX:
			tmpPtr[0] = 0xff;
			tmpPtr[1] = 0x03;
			tmpPtr[2] = (protocol>>8) & 0xff;
			tmpPtr[3] = (protocol) & 0xff;
			break;
		case COMPRESSED_PPP_FF03XX:
			tmpPtr[0] = 0xff;
			tmpPtr[1] = 0x03;
			tmpPtr[2] = (protocol) & 0xff;
			break;
		case COMPRESSED_PPP_00XX:
			tmpPtr[0] = (protocol>>8) & 0xff;
			tmpPtr[1] = (protocol) & 0xff;
			break;
		case COMPRESSED_PPP_XX:
			tmpPtr[0] = (protocol) & 0xff;
			break;
		default:
			//not support
			break;
	}
	/*
	_grepppHdr_t* grepppPtr = (_grepppHdr_t *) tmpPtr;


	memset(tmpPtr, 0, sizeof(_grepppHdr_t));
	grepppPtr->adress = 0xff;		// cheney ?? FIXME
	grepppPtr->control = 0x03;		// cheney ?? FIXME
	grepppPtr->proto = htons(protocol);
	*/
}

void l3FormGREPPPProtocolModify(uint32 flag, int8 * pktBuf, uint16 protocol, compressed_ppp_mode_t cmpPPPMode, pptp_outerhdr_option_t fmtRun, uint8 err) {
	int8 * tmpPtr = pktBuf + l3Offset(flag, pktBuf);
	ipHdr_t * ipPtr = (ipHdr_t *)tmpPtr;
	tmpPtr += ipPtr->ip_hl * 4;
	tmpPtr += 8;
	switch(fmtRun){
		case PPTP_WITHSEQ_WITHACK:
			tmpPtr += 8;
			break;
		case PPTP_WITHSEQ_WOACK:
			tmpPtr += 4;
			break;
		case PPTP_WOSEQ_WITHACK:
			tmpPtr += 4;
			break;
		default:
			break;
	}

	switch(cmpPPPMode)
	{
		case COMPRESSED_PPP_FF0300XX:
			tmpPtr[0] = 0xff;
			tmpPtr[1] = 0x03;
			tmpPtr[2] = (protocol>>8) & 0xff;
			tmpPtr[3] = (protocol) & 0xff;
			tmpPtr[err] = 0xcc;
			break;
		case COMPRESSED_PPP_FF03XX:
			tmpPtr[0] = 0xff;
			tmpPtr[1] = 0x03;
			tmpPtr[2] = (protocol) & 0xff;
			tmpPtr[err] = 0xcc;
			break;
		case COMPRESSED_PPP_00XX:
			tmpPtr[0] = (protocol>>8) & 0xff;
			tmpPtr[1] = (protocol) & 0xff;
			tmpPtr[err] = 0xcc;
			break;
		case COMPRESSED_PPP_XX:
			tmpPtr[0] = 0xcc;
			break;
		default:
			//not support
			break;
	}
	/*
	_grepppHdr_t* grepppPtr = (_grepppHdr_t *) tmpPtr;


	memset(tmpPtr, 0, sizeof(_grepppHdr_t));
	grepppPtr->adress = 0xff;		// cheney ?? FIXME
	grepppPtr->control = 0x03;		// cheney ?? FIXME
	grepppPtr->proto = htons(protocol);
	*/
}

void l3FormV6GREPPPProtocolModify(uint32 flag, int8 * pktBuf, uint8 hdrlen, uint16 protocol, compressed_ppp_mode_t cmpPPPMode, pptp_outerhdr_option_t fmtRun,uint8 err) {
	int8 * tmpPtr = pktBuf + l3Offset(flag, pktBuf);
	tmpPtr += 40;
	tmpPtr += hdrlen;
	tmpPtr += 8;
	switch(fmtRun){
		case PPTP_WITHSEQ_WITHACK:
			tmpPtr += 8;
			break;
		case PPTP_WITHSEQ_WOACK:
			tmpPtr += 4;
			break;
		case PPTP_WOSEQ_WITHACK:
			tmpPtr += 4;
			break;
		default:
			break;
	}

	switch(cmpPPPMode)
	{
		case COMPRESSED_PPP_FF0300XX:
			tmpPtr[0] = 0xff;
			tmpPtr[1] = 0x03;
			tmpPtr[2] = (protocol>>8) & 0xff;
			tmpPtr[3] = (protocol) & 0xff;
			tmpPtr[err] = 0xcc;
			break;
		case COMPRESSED_PPP_FF03XX:
			tmpPtr[0] = 0xff;
			tmpPtr[1] = 0x03;
			tmpPtr[2] = (protocol) & 0xff;
			tmpPtr[err] = 0xcc;
			break;
		case COMPRESSED_PPP_00XX:
			tmpPtr[0] = (protocol>>8) & 0xff;
			tmpPtr[1] = (protocol) & 0xff;
			tmpPtr[err] = 0xcc;
			break;
		case COMPRESSED_PPP_XX:
			tmpPtr[0] = 0xcc;
			break;
		default:
			//not support
			break;
	}
	/*
	_grepppHdr_t* grepppPtr = (_grepppHdr_t *) tmpPtr;


	memset(tmpPtr, 0, sizeof(_grepppHdr_t));
	grepppPtr->adress = 0xff;		// cheney ?? FIXME
	grepppPtr->control = 0x03;		// cheney ?? FIXME
	grepppPtr->proto = htons(protocol);
	*/
}

#endif

