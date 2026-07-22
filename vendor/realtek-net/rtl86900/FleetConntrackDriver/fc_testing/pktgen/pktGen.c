/*
* Copyright c                  Realtek Semiconductor Corporation, 2002
* All rights reserved.
*
* Program : Code for packet generation
* Abstract :
* Author : 
* $Id: pktGen.c,v 1.14 2012/10/17 11:12:07 ysleu Exp $
*/

#include "rtl_glue.h"
#if !defined(__KERNEL__)
#include <stdio.h>
#include <string.h>
#else

#include <common/type.h>
#include <uapi/linux/types.h>
//#include <linux/byteorder/generic.h>
//#include <linux/byteorder/little_endian.h>
//#include <uapi/linux/byteorder/little_endian.h>
#include <linux/slab.h>
#endif

#include "protoForm.h"
#include "pktForm.h"
#include "utility.h"
#include "pktGen.h"
#include <dump.h>

#define _MSG


extern uint32 skip_minPktLen_check;
extern uint32 llc_len_including_padding;
extern uint32 llc_len_excluding_padding;
extern uint32 pppoe_length_exclude_padding;
extern uint32 pppoe_length_include_padding;
extern uint32 ipv4_length_exclude_padding;
extern uint32 ipv6_length_exclude_padding;
extern uint32 ipv6_length_include_padding;
uint32 inner_skip_minPktLen_check = 0;
uint32 inner_tail_padding_len = 0;
uint32 inner_llc_len_include_ingress_padding = 0;
uint32 outer_llc_len_include_ingress_padding = 0;
uint32 outer_llc_len_exclude_ingress_padding = 0;
uint32 outer_ppp_length_exclude_padding = 0;
uint32 outer_ppp_length_include_padding = 0;
uint32 inner_ppp_length_exclude_padding = 0;
uint32 inner_ppp_length_include_padding = 0;
uint32 inner_ipv4_len_exclude_padding = 0;
uint32 outer_ipv6_len_exclude_padding = 0;
uint32 outer_ipv6_len_include_padding = 0;
uint32 inner_ipv6_len_exclude_padding = 0;
uint32 inner_insert_len_snap_before_vlan = 0;
uint32 inner_insert_len_snap_after_vlan = 0;
uint32 outer_insert_len_snap = 0;
uint32 outer_insert_len_snap_before_vlan = 0;
unsigned char innerPktContent[2048];

#define VLAN_TAG_LEN 4
//push vlan on top vlan
int insertTopVlanTags(uint8* buf,int *length,uint16 tpid,uint16 vlanid,uint8 vpri,uint8 dei)
{

	uint8 otherVlan[VLAN_TAG_LEN]={0};
	int32 offset=12;
	//int32 vlanCnt=0;
	
	otherVlan[0] = (tpid>>8)&0xff;
	otherVlan[1] = (tpid>>0)&0xff;
	otherVlan[2] = ((vpri&0x7)<<5) | ((dei&1)<<4) | ((vlanid>>8) &0xf);
	otherVlan[3] = (vlanid&0xff);

	memmove(&buf[offset+VLAN_TAG_LEN],&buf[offset],*length-offset);
	memmove(&buf[offset],otherVlan,VLAN_TAG_LEN);
	/* VLAN_TAG_LEN=4*/
	*length+=VLAN_TAG_LEN;

	l2FormCrc(0 , buf, *length-4/*for CRC*/);

	//dump_packet(buf,*length,"insertTopVlanTags");
	//printk("insert tpid:0x%x vlanid:%d vpir:%d dei:%d  [%x][%x][%x][%x]\n",tpid,vlanid,vpri,dei,otherVlan[0],otherVlan[1],otherVlan[2],otherVlan[3]);
	
	return SUCCESS;
	
}


int insertInnerVlanTags(uint8* buf,int *length,uint16 tpid,uint16 vlanid,uint8 vpri,uint8 dei)
{

	uint8 otherVlan[VLAN_TAG_LEN]={0};
	int32 offset=12;
	int32 vlanCnt=0;
	
	otherVlan[0] = (tpid>>8)&0xff;
	otherVlan[1] = (tpid>>0)&0xff;
	otherVlan[2] = ((vpri&0x7)<<5) | ((dei&1)<<4) | ((vlanid>>8) &0xf);
	otherVlan[3] = (vlanid&0xff);

	for(vlanCnt=0;vlanCnt<4;vlanCnt++)
	{
		if(buf[offset]==0x88 && buf[offset+1]==0xa8)
			offset+=4;
		if(buf[offset]==0x81 && buf[offset+1]==0x00)
			offset+=4;
		if(buf[offset]==0x91 && buf[offset+1]==0x00)
			offset+=4;
		if(buf[offset]==0x92 && buf[offset+1]==0x00)
			offset+=4;
		if(buf[offset]==0x93 && buf[offset+1]==0x00)
			offset+=4;
		if(buf[offset]==0x94 && buf[offset+1]==0x00)
			offset+=4;
		if(buf[offset]==0x55 && buf[offset+1]==0x66)
			offset+=4;
		if(buf[offset]==0x77 && buf[offset+1]==0x88)
			offset+=4;		
	}


	memmove(&buf[offset+VLAN_TAG_LEN],&buf[offset],*length-offset);
	memmove(&buf[offset],otherVlan,VLAN_TAG_LEN);
	/* VLAN_TAG_LEN=4*/
	*length+=VLAN_TAG_LEN;

	l2FormCrc(0 , buf, *length-4/*for CRC*/);

	//dump_packet(buf,*length,"insertInnerVlanTags");
	//printk("insert tpid:0x%x vlanid:%d vpir:%d dei:%d  [%x][%x][%x][%x]\n",tpid,vlanid,vpri,dei,otherVlan[0],otherVlan[1],otherVlan[2],otherVlan[3]);
	
	return SUCCESS;
	
}



void payloadGen(uint8 *pktData, int pktLen)
{
	int i;
	for (i=0;i<pktLen;i++)
		pktData[i] = i&0xff;
	//cheney
	if(pktLen > 8){
		pktData[0] = 0x70;	//p
		pktData[1] = 0x61;	//a
		pktData[2] = 0x79;	//y
		pktData[3] = 0x6c;	//l
		pktData[4] = 0x6f;	//o
		pktData[5] = 0x61;	//a
		pktData[6] = 0x64;	//d
	}
}

uint32 pktGen_withCRCAppend(rtl8651_PktConf_t *Pktptr, int8 *buff){
	return (pktGen(Pktptr, buff) + 4);	// 4bytes CRC
}

uint32 pktGen_withCRCAppend_payloadGen(rtl8651_PktConf_t *Pktptr, int8 *buff){
	payloadGen(Pktptr->payload.content, Pktptr->payload.length);
	return (pktGen(Pktptr, buff) + 4);	// 4bytes CRC
}

uint32 pktGen_withCRCAppend_paddingCut(rtl8651_PktConf_t *Pktptr, int8 *buff, uint32 paddingCut){
	uint32 ret,orig_check=skip_minPktLen_check;

	skip_minPktLen_check=paddingCut;
	ret=pktGen(Pktptr, buff);
	skip_minPktLen_check=orig_check;
	return (ret + 4);	// 4bytes CRC
}

/*
 *  pktGen() - Generate a packet as you want
 *
 *  The returned value is the packet length.
 *  The length does not included L2 CRC, however, the L2 CRC is followed after the tail of packet.
 */
uint32 pktGen(rtl8651_PktConf_t *Pktptr, int8 *buff){
	rtl8651_PktConf_t *ptr = 	kmalloc(sizeof(rtl8651_PktConf_t), GFP_ATOMIC | __GFP_ZERO);
	uint32 pktLen;
	uint32 l2Flag;
	int8 dummy[6]={0};
	int8* PayloadPtr = dummy;

	// copy pkt conf to fake pkt
	memcpy(ptr, Pktptr, sizeof(rtl8651_PktConf_t));

	// End of test case
	if (ptr->pktType == _PKT_STOP){
 		return 0;
	}

	/*
			IP
	*/
	if (ptr->conf_sip == 0){
		// default sip
		ptr->conf_sip = 0xc0a80101;	// 192.168.1.1
	}
	if (ptr->conf_dip == 0){
		// default dip (unicast)
		ptr->conf_dip = 0x8c710101;	// 140.113.1.1
	}

#if !defined(CONFIG_APOLLO)
	if (ptr->ip.version == 0){
		ptr->ip.version = 4;
	}
	if (ptr->ip.headerLen == 0){
		ptr->ip.headerLen = 20;
	}
	#if 0
	if (ptr->ip.ttl == 0){
		ptr->ip.ttl = 255;
	}
	#endif
#endif

	/*
		Payload
	*/
	if ((ptr->payload.length == 0) || (ptr->payload.content == NULL)){
		pktLen = 0;
	}else{
		pktLen = ptr->payload.length;
		PayloadPtr = ptr->payload.content;
	}
	#ifdef _MSG
	//TRACE("\t* -------------- Packet Payload Length [%d] --------------\n", pktLen);
	#endif

	/*
		l2Flag
	*/
	l2Flag = ptr->l2Flag;
//	TEST_DEBUG("the l2flag is  :[%d]\n", l2Flag);
//	TEST_DEBUG("the pktType is: [%d]\n", ptr->pktType );
	// for 50B,(50 is also ok), when AUTOADD, we don't change ethertype
//	TEST_DEBUG("before switch prt->pktType\n");
	switch (ptr->pktType){
	//	TEST_DEBUG("just in switch pktType\n");
		case _PKT_TYPE_ETHER:
			#ifdef _MSG
			TEST_DEBUG("\t* Generate ETHER packet...");
			#endif
			protoForm_vsp_ether(buff, l2Flag, ptr->conf_dmac, ptr->conf_smac,
						ptr->conf_ctag_protocol,ptr->conf_ctag_reason,ptr->conf_ctag_priority,ptr->conf_ctag_extspa,ptr->conf_ctag_streamid,ptr->conf_ctag_l3r,ptr->conf_ctag_org,ptr->conf_ctag_extdpmsk,ptr->conf_ctag_spa,ptr->conf_ctag_fbi,ptr->conf_ctag_fb_index,
						ptr->ctag_tx2.txmsk,ptr->ctag_tx2.prisel,ptr->ctag_tx2.pri,ptr->ctag_tx2.directTx,ptr->ctag_tx2.keep,
						ptr->ctag_tx2.dislrn,ptr->ctag_tx2.psel,ptr->ctag_tx2.extspa,ptr->ctag_tx2.pppoeact,ptr->ctag_tx2.pppoeidx,ptr->ctag_tx2.streamid,
						ptr->svlan.tpid,ptr->svlan.svid, ptr->svlan.dei, ptr->svlan.sprio,
						ptr->cvlan.cvid, ptr->cvlan.cfi, ptr->cvlan.cprio,
						ptr->llc.dsap, ptr->llc.ssap,
						ptr->pppoe.type, ptr->pppoe.session,
						ptr->conf_ethtype,
						PayloadPtr, &pktLen);
			break;
		case _PKT_TYPE_ARP:
			#ifdef _MSG
			TEST_DEBUG("\t* Generate ARP packet...");
			#endif
			protoForm_vs_arp(buff, l2Flag, ptr->conf_dmac, ptr->conf_smac,
								ptr->conf_ctag_protocol,ptr->conf_ctag_reason,ptr->conf_ctag_priority,ptr->conf_ctag_extspa,ptr->conf_ctag_streamid,ptr->conf_ctag_l3r,ptr->conf_ctag_org,ptr->conf_ctag_extdpmsk,ptr->conf_ctag_spa,ptr->conf_ctag_fbi,ptr->conf_ctag_fb_index,
								ptr->ctag_tx2.txmsk,ptr->ctag_tx2.prisel,ptr->ctag_tx2.pri,ptr->ctag_tx2.directTx,ptr->ctag_tx2.keep,
								ptr->ctag_tx2.dislrn,ptr->ctag_tx2.psel,ptr->ctag_tx2.extspa,ptr->ctag_tx2.pppoeact,ptr->ctag_tx2.pppoeidx,ptr->ctag_tx2.streamid,
								ptr->svlan.tpid,ptr->svlan.svid, ptr->svlan.dei, ptr->svlan.sprio,
								ptr->cvlan.cvid, ptr->cvlan.cfi, ptr->cvlan.cprio,
                                ptr->arp.mac_src, ptr->arp.ip_src,
                                ptr->arp.mac_dst, ptr->arp.ip_dst,
		                        ptr->arp.op, &pktLen);
			break;
		case _PKT_TYPE_IP:
			#ifdef _MSG
			TEST_DEBUG("\t* Generate IP packet...");
			#endif
			protoForm_vsp_ip(buff, l2Flag, ptr->conf_dmac, ptr->conf_smac,
						ptr->conf_ctag_protocol,ptr->conf_ctag_reason,ptr->conf_ctag_priority,ptr->conf_ctag_extspa,ptr->conf_ctag_streamid,ptr->conf_ctag_l3r,ptr->conf_ctag_org,ptr->conf_ctag_extdpmsk,ptr->conf_ctag_spa,ptr->conf_ctag_fbi,ptr->conf_ctag_fb_index,
						ptr->ctag_tx2.txmsk,ptr->ctag_tx2.prisel,ptr->ctag_tx2.pri,ptr->ctag_tx2.directTx,ptr->ctag_tx2.keep,
						ptr->ctag_tx2.dislrn,ptr->ctag_tx2.psel,ptr->ctag_tx2.extspa,ptr->ctag_tx2.pppoeact,ptr->ctag_tx2.pppoeidx,ptr->ctag_tx2.streamid,
						ptr->svlan.tpid,ptr->svlan.svid, ptr->svlan.dei, ptr->svlan.sprio,
						ptr->cvlan.cvid, ptr->cvlan.cfi, ptr->cvlan.cprio,
						ptr->pppoe.type, ptr->pppoe.session,
						ptr->ip.version, ptr->ip.headerLen,
						ptr->ip.tos, ptr->ip.id, ptr->ip.diffserv, ptr->ip.mf, ptr->ip.offset,
						ptr->ip.ttl, ptr->conf_sip, ptr->conf_dip, ptr->ip.protocol,
						PayloadPtr, ptr->ErrFlag, &pktLen);
			break;
		case _PKT_TYPE_PPTP:
			#ifdef _MSG
			TEST_DEBUG("\t* Generate PPTP packet...");
			#endif
			protoForm_vsp_pptp(buff, l2Flag, ptr->conf_dmac, ptr->conf_smac,
						ptr->svlan.tpid,ptr->svlan.svid, ptr->svlan.dei, ptr->svlan.sprio,
						ptr->cvlan.cvid, ptr->cvlan.cfi, ptr->cvlan.cprio,
						ptr->pppoe.type, ptr->pppoe.session,
						ptr->ip.version, ptr->ip.headerLen,
						ptr->ip.tos, ptr->ip.id, ptr->ip.diffserv, ptr->ip.mf, ptr->ip.offset,
						ptr->ip.ttl, ptr->conf_sip, ptr->conf_dip,
						ptr->pptp.callid, ptr->pptp.seqno, ptr->pptp.ackno,
						PayloadPtr, ptr->ErrFlag, &pktLen);
			break;
		case _PKT_TYPE_ICMP:
			#ifdef _MSG
			TEST_DEBUG("\t* Generate ICMP packet...");
			#endif
			if (ptr->ip.offset){
				protoForm_vsp_ip(buff, l2Flag, ptr->conf_dmac, ptr->conf_smac,
							ptr->conf_ctag_protocol,ptr->conf_ctag_reason,ptr->conf_ctag_priority,ptr->conf_ctag_extspa,ptr->conf_ctag_streamid,ptr->conf_ctag_l3r,ptr->conf_ctag_org,ptr->conf_ctag_extdpmsk,ptr->conf_ctag_spa,ptr->conf_ctag_fbi,ptr->conf_ctag_fb_index,
							ptr->ctag_tx2.txmsk,ptr->ctag_tx2.prisel,ptr->ctag_tx2.pri,ptr->ctag_tx2.directTx,ptr->ctag_tx2.keep,
							ptr->ctag_tx2.dislrn,ptr->ctag_tx2.psel,ptr->ctag_tx2.extspa,ptr->ctag_tx2.pppoeact,ptr->ctag_tx2.pppoeidx,ptr->ctag_tx2.streamid,
							ptr->svlan.tpid,ptr->svlan.svid, ptr->svlan.dei, ptr->svlan.sprio,
							ptr->cvlan.cvid, ptr->cvlan.cfi, ptr->cvlan.cprio,
							ptr->pppoe.type, ptr->pppoe.session,
							ptr->ip.version, ptr->ip.headerLen,
							ptr->ip.tos, ptr->ip.id, ptr->ip.diffserv, ptr->ip.mf, ptr->ip.offset,
							ptr->ip.ttl, ptr->conf_sip, ptr->conf_dip, 1,
							PayloadPtr, ptr->ErrFlag, &pktLen);
			}else{
			protoForm_vsp_icmp(buff, l2Flag, ptr->conf_dmac, ptr->conf_smac,
						ptr->conf_ctag_protocol,ptr->conf_ctag_reason,ptr->conf_ctag_priority,ptr->conf_ctag_extspa,ptr->conf_ctag_streamid,ptr->conf_ctag_l3r,ptr->conf_ctag_org,ptr->conf_ctag_extdpmsk,ptr->conf_ctag_spa,ptr->conf_ctag_fbi,ptr->conf_ctag_fb_index,
						ptr->ctag_tx2.txmsk,ptr->ctag_tx2.prisel,ptr->ctag_tx2.pri,ptr->ctag_tx2.directTx,ptr->ctag_tx2.keep,
						ptr->ctag_tx2.dislrn,ptr->ctag_tx2.psel,ptr->ctag_tx2.extspa,ptr->ctag_tx2.pppoeact,ptr->ctag_tx2.pppoeidx,ptr->ctag_tx2.streamid,
						ptr->svlan.tpid,ptr->svlan.svid, ptr->svlan.dei, ptr->svlan.sprio,
						ptr->cvlan.cvid, ptr->cvlan.cfi, ptr->cvlan.cprio,
						ptr->pppoe.type, ptr->pppoe.session,
						ptr->ip.version, ptr->ip.headerLen,
						ptr->ip.tos, ptr->ip.id, ptr->ip.diffserv, ptr->ip.mf, ptr->ip.offset,
						ptr->ip.ttl, ptr->conf_sip, ptr->conf_dip,
						ptr->icmp.type, ptr->icmp.code, ptr->icmp.id, ptr->icmp.seq,
						PayloadPtr, ptr->ErrFlag, &pktLen);
			}
			break;
		case _PKT_TYPE_IGMP:
			#ifdef _MSG
			TEST_DEBUG("\t* Generate IGMP packet...");
			#endif
			{
				int8 *private_buff=kmalloc(2048, GFP_ATOMIC | __GFP_ZERO);
				int8 *buffptr = private_buff;
				uint32 grlen = 0;

				if (ptr->igmp.ver == 3)
				{
					if ((ptr->igmp.type != IGMP_V3_MEMBERSHIP_REPORT) && (ptr->igmp.type != IGMP_MEMBERSHIP_QUERY))
						return 0;
				}

				if ((ptr->igmp.ver == 3) && (ptr->igmp.type == IGMP_V3_MEMBERSHIP_REPORT))
				{	// generate group record
					grlen = grGen(ptr->igmp.grCfg, ptr->igmp.rnofg, private_buff);
					if (grlen == 0)
						return 0;	// group record generate error
					buffptr = (int8 *)(private_buff + grlen);
				}

				if (pktLen && PayloadPtr)
					memcpy(buffptr, PayloadPtr, pktLen);
				pktLen += grlen;

				protoForm_vsp_igmp(buff, l2Flag, ptr->conf_dmac, ptr->conf_smac,
					ptr->conf_ctag_protocol,ptr->conf_ctag_reason,ptr->conf_ctag_priority,ptr->conf_ctag_extspa,ptr->conf_ctag_streamid,ptr->conf_ctag_l3r,ptr->conf_ctag_org,ptr->conf_ctag_extdpmsk,ptr->conf_ctag_spa,ptr->conf_ctag_fbi,ptr->conf_ctag_fb_index,
					ptr->ctag_tx2.txmsk,ptr->ctag_tx2.prisel,ptr->ctag_tx2.pri,ptr->ctag_tx2.directTx,ptr->ctag_tx2.keep,
					ptr->ctag_tx2.dislrn,ptr->ctag_tx2.psel,ptr->ctag_tx2.extspa,ptr->ctag_tx2.pppoeact,ptr->ctag_tx2.pppoeidx,ptr->ctag_tx2.streamid,
					ptr->svlan.tpid,ptr->svlan.svid, ptr->svlan.dei, ptr->svlan.sprio,
					ptr->cvlan.cvid, ptr->cvlan.cfi, ptr->cvlan.cprio,
					ptr->pppoe.type, ptr->pppoe.session,
					ptr->ip.version, ptr->ip.headerLen,
					ptr->ip.tos, ptr->ip.id, ptr->ip.diffserv, ptr->ip.mf, ptr->ip.offset,
					ptr->ip.ttl, ptr->conf_sip, ptr->conf_dip,
					ptr->igmp.ver, ptr->igmp.type, ptr->igmp.respTime, ptr->igmp.gaddr,
					ptr->igmp.qrsq, ptr->igmp.qqic, ptr->igmp.qnofs, ptr->igmp.qslist, ptr->igmp.rnofg,
                   	private_buff, ptr->ErrFlag, &pktLen);
				kfree(private_buff);
			}
			break;
		case _PKT_TYPE_TCP:
			if (ptr->conf_tcp_flag & TH_SYN)
			{
				int8 tcp_option[4];
				int8 *option_ptr = tcp_option;

				if (ptr->conf_tcp_mss == 0)
				{
					ptr->conf_tcp_mss = 1460;			// default mss
				}
				*((uint8*)option_ptr) = 2;			// kind
				*((uint8*)(option_ptr+1)) = 4;		// length
				*((uint16*)(option_ptr+2)) = htons(ptr->conf_tcp_mss);	//mss
				#ifdef _MSG
				TEST_DEBUG("\t* Generate TCP packet with option");
				#endif
				protoForm_vsp_tcp_option(buff, l2Flag, ptr->conf_dmac, ptr->conf_smac,
						ptr->conf_ctag_protocol,ptr->conf_ctag_reason,ptr->conf_ctag_priority,ptr->conf_ctag_extspa,ptr->conf_ctag_streamid,ptr->conf_ctag_l3r,ptr->conf_ctag_org,ptr->conf_ctag_extdpmsk,ptr->conf_ctag_spa,ptr->conf_ctag_fbi,ptr->conf_ctag_fb_index,
						ptr->ctag_tx2.txmsk,ptr->ctag_tx2.prisel,ptr->ctag_tx2.pri,ptr->ctag_tx2.directTx,ptr->ctag_tx2.keep,
						ptr->ctag_tx2.dislrn,ptr->ctag_tx2.psel,ptr->ctag_tx2.extspa,ptr->ctag_tx2.pppoeact,ptr->ctag_tx2.pppoeidx,ptr->ctag_tx2.streamid,
						ptr->svlan.tpid,ptr->svlan.svid, ptr->svlan.dei, ptr->svlan.sprio,
						ptr->cvlan.cvid, ptr->cvlan.cfi, ptr->cvlan.cprio,
						ptr->pppoe.type, ptr->pppoe.session,
						ptr->ip.version, ptr->ip.headerLen,
						ptr->ip.tos, ptr->ip.id, ptr->ip.diffserv, ptr->ip.mf, ptr->ip.offset,
						ptr->ip.ttl, ptr->conf_sip, ptr->conf_dip,
						ptr->conf_sport, ptr->conf_dport,
						ptr->conf_tcp_seq, ptr->conf_tcp_ack, ptr->conf_tcp_flag, ptr->conf_tcp_win,
						tcp_option, 4, PayloadPtr, ptr->ErrFlag, &pktLen);
			}
			else
			{
				#ifdef _MSG
				TEST_DEBUG("\t* Generate TCP packet...");
				#endif
				protoForm_vsp_tcp(buff, l2Flag, ptr->conf_dmac, ptr->conf_smac,
						ptr->conf_ctag_protocol,ptr->conf_ctag_reason,ptr->conf_ctag_priority,ptr->conf_ctag_extspa,ptr->conf_ctag_streamid,ptr->conf_ctag_l3r,ptr->conf_ctag_org,ptr->conf_ctag_extdpmsk,ptr->conf_ctag_spa,ptr->conf_ctag_fbi,ptr->conf_ctag_fb_index,
						ptr->ctag_tx2.txmsk,ptr->ctag_tx2.prisel,ptr->ctag_tx2.pri,ptr->ctag_tx2.directTx,ptr->ctag_tx2.keep,
						ptr->ctag_tx2.dislrn,ptr->ctag_tx2.psel,ptr->ctag_tx2.extspa,ptr->ctag_tx2.pppoeact,ptr->ctag_tx2.pppoeidx,ptr->ctag_tx2.streamid,
						ptr->svlan.tpid,ptr->svlan.svid, ptr->svlan.dei, ptr->svlan.sprio,
						ptr->cvlan.tpid,ptr->cvlan.cvid, ptr->cvlan.cfi, ptr->cvlan.cprio,
						ptr->pppoe.type, ptr->pppoe.session,
						ptr->ip.version, ptr->ip.headerLen,
						ptr->ip.tos, ptr->ip.id, ptr->ip.diffserv, ptr->ip.mf, ptr->ip.offset,
						ptr->ip.ttl, ptr->conf_sip, ptr->conf_dip,
						ptr->conf_sport, ptr->conf_dport,
						ptr->conf_tcp_seq, ptr->conf_tcp_ack, ptr->conf_tcp_flag, ptr->conf_tcp_win,
						PayloadPtr, ptr->ErrFlag, &pktLen);
			}
			break;
		case _PKT_TYPE_UDP:
			#ifdef _MSG
			TEST_DEBUG("\t* Generate UDP packet...");
			#endif
			if (ptr->ip.offset){
				protoForm_vsp_ip(buff, l2Flag, ptr->conf_dmac, ptr->conf_smac,
							ptr->conf_ctag_protocol,ptr->conf_ctag_reason,ptr->conf_ctag_priority,ptr->conf_ctag_extspa,ptr->conf_ctag_streamid,ptr->conf_ctag_l3r,ptr->conf_ctag_org,ptr->conf_ctag_extdpmsk,ptr->conf_ctag_spa,ptr->conf_ctag_fbi,ptr->conf_ctag_fb_index,
							ptr->ctag_tx2.txmsk,ptr->ctag_tx2.prisel,ptr->ctag_tx2.pri,ptr->ctag_tx2.directTx,ptr->ctag_tx2.keep,
							ptr->ctag_tx2.dislrn,ptr->ctag_tx2.psel,ptr->ctag_tx2.extspa,ptr->ctag_tx2.pppoeact,ptr->ctag_tx2.pppoeidx,ptr->ctag_tx2.streamid,
							ptr->svlan.tpid,ptr->svlan.svid, ptr->svlan.dei, ptr->svlan.sprio,
							ptr->cvlan.cvid, ptr->cvlan.cfi, ptr->cvlan.cprio,
							ptr->pppoe.type, ptr->pppoe.session,
							ptr->ip.version, ptr->ip.headerLen,
							ptr->ip.tos, ptr->ip.id, ptr->ip.diffserv, ptr->ip.mf, ptr->ip.offset,
							ptr->ip.ttl, ptr->conf_sip, ptr->conf_dip, 17,
							PayloadPtr, ptr->ErrFlag, &pktLen);
			}else{
			protoForm_vsp_udp(buff, l2Flag, ptr->conf_dmac, ptr->conf_smac,
						ptr->conf_ctag_protocol,ptr->conf_ctag_reason,ptr->conf_ctag_priority,ptr->conf_ctag_extspa,ptr->conf_ctag_streamid,ptr->conf_ctag_l3r,ptr->conf_ctag_org,ptr->conf_ctag_extdpmsk,ptr->conf_ctag_spa,ptr->conf_ctag_fbi,ptr->conf_ctag_fb_index,
						ptr->ctag_tx2.txmsk,ptr->ctag_tx2.prisel,ptr->ctag_tx2.pri,ptr->ctag_tx2.directTx,ptr->ctag_tx2.keep,
						ptr->ctag_tx2.dislrn,ptr->ctag_tx2.psel,ptr->ctag_tx2.extspa,ptr->ctag_tx2.pppoeact,ptr->ctag_tx2.pppoeidx,ptr->ctag_tx2.streamid,
						ptr->svlan.tpid,ptr->svlan.svid, ptr->svlan.dei, ptr->svlan.sprio,
						ptr->cvlan.tpid,ptr->cvlan.cvid, ptr->cvlan.cfi, ptr->cvlan.cprio,
						ptr->pppoe.type, ptr->pppoe.session,
						ptr->ip.version, ptr->ip.headerLen,
						ptr->ip.tos, ptr->ip.id, ptr->ip.diffserv, ptr->ip.mf, ptr->ip.offset,
						ptr->ip.ttl, ptr->conf_sip, ptr->conf_dip,
						ptr->conf_sport, ptr->conf_dport,
						PayloadPtr, ptr->ErrFlag, &pktLen);
			}
			break;
		case _PKT_TYPE_UDP_OPTION:
			#ifdef _MSG
			TEST_DEBUG("\t* Generate UDP packet with option...");
			#endif
			if (ptr->ip.offset)
			{
				protoForm_vsp_ip_opt(buff, l2Flag, ptr->conf_dmac, ptr->conf_smac,
							ptr->svlan.tpid,ptr->svlan.svid, ptr->svlan.dei, ptr->svlan.sprio,
							ptr->cvlan.cvid, ptr->cvlan.cfi, ptr->cvlan.cprio,
							ptr->pppoe.type, ptr->pppoe.session,
							ptr->ip.version, ptr->ip.headerLen,
							ptr->ip.tos, ptr->ip.id, ptr->ip.diffserv, ptr->ip.mf, ptr->ip.offset,
							ptr->ip.ttl, ptr->conf_sip, ptr->conf_dip, 17,
							PayloadPtr, ptr->ErrFlag, &pktLen);
			}else{
			protoForm_vsp_udp_opt(buff, l2Flag, ptr->conf_dmac, ptr->conf_smac,
						ptr->svlan.tpid,ptr->svlan.svid, ptr->svlan.dei, ptr->svlan.sprio,
						ptr->cvlan.cvid, ptr->cvlan.cfi, ptr->cvlan.cprio,
						ptr->pppoe.type, ptr->pppoe.session,
						ptr->ip.version, ptr->ip.headerLen,
						ptr->ip.tos, ptr->ip.id, ptr->ip.diffserv, ptr->ip.mf, ptr->ip.offset,
						ptr->ip.ttl, ptr->conf_sip, ptr->conf_dip,
						ptr->conf_sport, ptr->conf_dport,
						PayloadPtr, ptr->ErrFlag, &pktLen);
			}
			break;
		case _PKT_TYPE_IPV6:
		//	TEST_DEBUG("in IPV6");
			#ifdef _MSG
			TEST_DEBUG("\t* Generate IPV6 packet!");
			#endif
			protoForm_vsp_ipv6(buff, l2Flag, ptr->extHdr, ptr->conf_dmac, ptr->conf_smac, 
							ptr->conf_ctag_protocol,ptr->conf_ctag_reason,ptr->conf_ctag_priority,ptr->conf_ctag_extspa,ptr->conf_ctag_streamid,ptr->conf_ctag_l3r,ptr->conf_ctag_org,ptr->conf_ctag_extdpmsk,ptr->conf_ctag_spa,ptr->conf_ctag_fbi,ptr->conf_ctag_fb_index,
							ptr->ctag_tx2.txmsk,ptr->ctag_tx2.prisel,ptr->ctag_tx2.pri,ptr->ctag_tx2.directTx,ptr->ctag_tx2.keep,
							ptr->ctag_tx2.dislrn,ptr->ctag_tx2.psel,ptr->ctag_tx2.extspa,ptr->ctag_tx2.pppoeact,ptr->ctag_tx2.pppoeidx,ptr->ctag_tx2.streamid,
							ptr->svlan.tpid,ptr->svlan.svid, ptr->svlan.dei, ptr->svlan.sprio,
							ptr->cvlan.cvid, ptr->cvlan.cfi, ptr->cvlan.cprio,
							ptr->pppoe.type, ptr->pppoe.session, 
							ptr->ipv6.version, ptr->ipv6.priority, ptr->ipv6.flowlbl, ptr->ipv6.nxthdr, ptr->ipv6.hoplmt,
							ptr->conf_v6sip, ptr->conf_v6dip,
							ptr->rhdr.hdrlen, ptr->rhdr.segments_left, ptr->rhdr.dip,
							ptr->frghdr.fragment, ptr->frghdr.id,
							PayloadPtr, ptr->ErrFlag, &pktLen);   
			break;
		case _PKT_TYPE_V6ICMP:
			#ifdef _MSG
			TEST_DEBUG("\t* Generate V6ICMP packet!");
			#endif
			protoForm_vsp_v6icmp(buff, l2Flag, ptr->extHdr, ptr->conf_dmac, ptr->conf_smac, 
							ptr->conf_ctag_protocol,ptr->conf_ctag_reason,ptr->conf_ctag_priority,ptr->conf_ctag_extspa,ptr->conf_ctag_streamid,ptr->conf_ctag_l3r,ptr->conf_ctag_org,ptr->conf_ctag_extdpmsk,ptr->conf_ctag_spa,ptr->conf_ctag_fbi,ptr->conf_ctag_fb_index,
							ptr->ctag_tx2.txmsk,ptr->ctag_tx2.prisel,ptr->ctag_tx2.pri,ptr->ctag_tx2.directTx,ptr->ctag_tx2.keep,
							ptr->ctag_tx2.dislrn,ptr->ctag_tx2.psel,ptr->ctag_tx2.extspa,ptr->ctag_tx2.pppoeact,ptr->ctag_tx2.pppoeidx,ptr->ctag_tx2.streamid,
							ptr->svlan.tpid,ptr->svlan.svid, ptr->svlan.dei, ptr->svlan.sprio,
							ptr->cvlan.cvid, ptr->cvlan.cfi, ptr->cvlan.cprio,
							ptr->pppoe.type, ptr->pppoe.session, 
							ptr->ipv6.version, ptr->ipv6.priority, ptr->ipv6.flowlbl, ptr->ipv6.hoplmt, 
							ptr->conf_v6sip, ptr->conf_v6dip,
							ptr->rhdr.hdrlen, ptr->rhdr.segments_left, ptr->rhdr.dip,
							ptr->frghdr.fragment, ptr->frghdr.id,
							ptr->icmp.type, ptr->icmp.code, ptr->icmp.id, ptr->icmp.seq,
							PayloadPtr, ptr->ErrFlag, &pktLen);
			break; 
		case _PKT_TYPE_V6TCP:
			if(ptr->conf_tcp_flag & TH_SYN)
			{
				int8 tcp_option[4];
				int8 *option_ptr = tcp_option;

				if (ptr->conf_tcp_mss == 0)
				{
					ptr->conf_tcp_mss = 1440;			// default mss
				}
				*((uint8*)option_ptr) = 2;			// kind
				*((uint8*)(option_ptr+1)) = 4;		// length
				*((uint16*)(option_ptr+2)) = htons(ptr->conf_tcp_mss);	//mss
				if(ptr->conf_tcp_offset > 0){
					#ifdef _MSG
					TEST_DEBUG("\t* Generate V6TCP packet with option and offset");
					#endif
					protoForm_vsp_v6tcp_option_specify_offset(buff, l2Flag, ptr->extHdr, ptr->conf_dmac, ptr->conf_smac, 
								ptr->conf_ctag_protocol,ptr->conf_ctag_reason,ptr->conf_ctag_priority,ptr->conf_ctag_extspa,ptr->conf_ctag_streamid,ptr->conf_ctag_l3r,ptr->conf_ctag_org,ptr->conf_ctag_extdpmsk,ptr->conf_ctag_spa,ptr->conf_ctag_fbi,ptr->conf_ctag_fb_index,
								ptr->ctag_tx2.txmsk,ptr->ctag_tx2.prisel,ptr->ctag_tx2.pri,ptr->ctag_tx2.directTx,ptr->ctag_tx2.keep,
								ptr->ctag_tx2.dislrn,ptr->ctag_tx2.psel,ptr->ctag_tx2.extspa,ptr->ctag_tx2.pppoeact,ptr->ctag_tx2.pppoeidx,ptr->ctag_tx2.streamid,
								ptr->svlan.tpid,ptr->svlan.svid, ptr->svlan.dei, ptr->svlan.sprio,
								ptr->cvlan.cvid, ptr->cvlan.cfi, ptr->cvlan.cprio,
								ptr->pppoe.type, ptr->pppoe.session, 
								ptr->ipv6.version, ptr->ipv6.priority, ptr->ipv6.flowlbl, ptr->ipv6.hoplmt, 
								ptr->conf_v6sip, ptr->conf_v6dip,
								ptr->rhdr.hdrlen, ptr->rhdr.segments_left, ptr->rhdr.dip,
								ptr->frghdr.fragment, ptr->frghdr.id,
								ptr->conf_sport, ptr->conf_dport,
								ptr->conf_tcp_seq, ptr->conf_tcp_ack, ptr->conf_tcp_offset, ptr->conf_tcp_flag, ptr->conf_tcp_win,
								tcp_option, 4, PayloadPtr, ptr->ErrFlag, &pktLen);
				}else{
					#ifdef _MSG
					TEST_DEBUG("\t* Generate V6TCP packet with option");
					#endif
					protoForm_vsp_v6tcp_option(buff, l2Flag, ptr->extHdr, ptr->conf_dmac, ptr->conf_smac, 
							ptr->conf_ctag_protocol,ptr->conf_ctag_reason,ptr->conf_ctag_priority,ptr->conf_ctag_extspa,ptr->conf_ctag_streamid,ptr->conf_ctag_l3r,ptr->conf_ctag_org,ptr->conf_ctag_extdpmsk,ptr->conf_ctag_spa,ptr->conf_ctag_fbi,ptr->conf_ctag_fb_index,
							ptr->ctag_tx2.txmsk,ptr->ctag_tx2.prisel,ptr->ctag_tx2.pri,ptr->ctag_tx2.directTx,ptr->ctag_tx2.keep,
							ptr->ctag_tx2.dislrn,ptr->ctag_tx2.psel,ptr->ctag_tx2.extspa,ptr->ctag_tx2.pppoeact,ptr->ctag_tx2.pppoeidx,ptr->ctag_tx2.streamid,
							ptr->svlan.tpid,ptr->svlan.svid, ptr->svlan.dei, ptr->svlan.sprio,
							ptr->cvlan.cvid, ptr->cvlan.cfi, ptr->cvlan.cprio,
							ptr->pppoe.type, ptr->pppoe.session, 
							ptr->ipv6.version, ptr->ipv6.priority, ptr->ipv6.flowlbl, ptr->ipv6.hoplmt, 
							ptr->conf_v6sip, ptr->conf_v6dip,
							ptr->rhdr.hdrlen, ptr->rhdr.segments_left, ptr->rhdr.dip,
							ptr->frghdr.fragment, ptr->frghdr.id,
							ptr->conf_sport, ptr->conf_dport,
							ptr->conf_tcp_seq, ptr->conf_tcp_ack, ptr->conf_tcp_flag, ptr->conf_tcp_win,
							tcp_option, 4, PayloadPtr, ptr->ErrFlag, &pktLen);
				}
			}
			else if(ptr->conf_tcp_offset > 0)
			{
				#ifdef _MSG
				TEST_DEBUG("\t* Generate V6TCP packet with offset!");
				#endif
				protoForm_vsp_v6tcp_specify_offset(buff, l2Flag, ptr->extHdr, ptr->conf_dmac, ptr->conf_smac, 
								ptr->conf_ctag_protocol,ptr->conf_ctag_reason,ptr->conf_ctag_priority,ptr->conf_ctag_extspa,ptr->conf_ctag_streamid,ptr->conf_ctag_l3r,ptr->conf_ctag_org,ptr->conf_ctag_extdpmsk,ptr->conf_ctag_spa,ptr->conf_ctag_fbi,ptr->conf_ctag_fb_index,
								ptr->ctag_tx2.txmsk,ptr->ctag_tx2.prisel,ptr->ctag_tx2.pri,ptr->ctag_tx2.directTx,ptr->ctag_tx2.keep,
								ptr->ctag_tx2.dislrn,ptr->ctag_tx2.psel,ptr->ctag_tx2.extspa,ptr->ctag_tx2.pppoeact,ptr->ctag_tx2.pppoeidx,ptr->ctag_tx2.streamid,
								ptr->svlan.tpid,ptr->svlan.svid, ptr->svlan.dei, ptr->svlan.sprio,
								ptr->cvlan.cvid, ptr->cvlan.cfi, ptr->cvlan.cprio,
								ptr->pppoe.type, ptr->pppoe.session, 
								ptr->ipv6.version, ptr->ipv6.priority, ptr->ipv6.flowlbl, ptr->ipv6.hoplmt, 
								ptr->conf_v6sip, ptr->conf_v6dip,
								ptr->rhdr.hdrlen, ptr->rhdr.segments_left, ptr->rhdr.dip,
								ptr->frghdr.fragment, ptr->frghdr.id,
								ptr->conf_sport, ptr->conf_dport,
								ptr->conf_tcp_seq, ptr->conf_tcp_ack, ptr->conf_tcp_offset, ptr->conf_tcp_flag, ptr->conf_tcp_win,
								PayloadPtr, ptr->ErrFlag, &pktLen);
			}
			else
			{
				#ifdef _MSG
				TEST_DEBUG("\t* Generate V6TCP packet!");
				#endif
				protoForm_vsp_v6tcp(buff, l2Flag, ptr->extHdr, ptr->conf_dmac, ptr->conf_smac, 
							ptr->conf_ctag_protocol,ptr->conf_ctag_reason,ptr->conf_ctag_priority,ptr->conf_ctag_extspa,ptr->conf_ctag_streamid,ptr->conf_ctag_l3r,ptr->conf_ctag_org,ptr->conf_ctag_extdpmsk,ptr->conf_ctag_spa,ptr->conf_ctag_fbi,ptr->conf_ctag_fb_index,
							ptr->ctag_tx2.txmsk,ptr->ctag_tx2.prisel,ptr->ctag_tx2.pri,ptr->ctag_tx2.directTx,ptr->ctag_tx2.keep,
							ptr->ctag_tx2.dislrn,ptr->ctag_tx2.psel,ptr->ctag_tx2.extspa,ptr->ctag_tx2.pppoeact,ptr->ctag_tx2.pppoeidx,ptr->ctag_tx2.streamid,
							ptr->svlan.tpid,ptr->svlan.svid, ptr->svlan.dei, ptr->svlan.sprio,
							ptr->cvlan.cvid, ptr->cvlan.cfi, ptr->cvlan.cprio,
							ptr->pppoe.type, ptr->pppoe.session, 
							ptr->ipv6.version, ptr->ipv6.priority, ptr->ipv6.flowlbl, ptr->ipv6.hoplmt, 
							ptr->conf_v6sip, ptr->conf_v6dip,
							ptr->rhdr.hdrlen, ptr->rhdr.segments_left, ptr->rhdr.dip,
							ptr->frghdr.fragment, ptr->frghdr.id,
							ptr->conf_sport, ptr->conf_dport,
							ptr->conf_tcp_seq, ptr->conf_tcp_ack, ptr->conf_tcp_flag, ptr->conf_tcp_win,
							PayloadPtr, ptr->ErrFlag, &pktLen);
			}
			break;
		case _PKT_TYPE_V6UDP:
			#ifdef _MSG
			TEST_DEBUG("\t* Generate V6UDP packet!");
			#endif
			protoForm_vsp_v6udp(buff, l2Flag, ptr->extHdr, ptr->conf_dmac, ptr->conf_smac, 
							ptr->conf_ctag_protocol,ptr->conf_ctag_reason,ptr->conf_ctag_priority,ptr->conf_ctag_extspa,ptr->conf_ctag_streamid,ptr->conf_ctag_l3r,ptr->conf_ctag_org,ptr->conf_ctag_extdpmsk,ptr->conf_ctag_spa,ptr->conf_ctag_fbi,ptr->conf_ctag_fb_index,
							ptr->ctag_tx2.txmsk,ptr->ctag_tx2.prisel,ptr->ctag_tx2.pri,ptr->ctag_tx2.directTx,ptr->ctag_tx2.keep,
							ptr->ctag_tx2.dislrn,ptr->ctag_tx2.psel,ptr->ctag_tx2.extspa,ptr->ctag_tx2.pppoeact,ptr->ctag_tx2.pppoeidx,ptr->ctag_tx2.streamid,
							ptr->svlan.tpid,ptr->svlan.svid, ptr->svlan.dei, ptr->svlan.sprio,
							ptr->cvlan.cvid, ptr->cvlan.cfi, ptr->cvlan.cprio,
							ptr->pppoe.type, ptr->pppoe.session, 
							ptr->ipv6.version, ptr->ipv6.priority, ptr->ipv6.flowlbl, ptr->ipv6.hoplmt, 
							ptr->conf_v6sip, ptr->conf_v6dip,
							ptr->rhdr.hdrlen, ptr->rhdr.segments_left, ptr->rhdr.dip,
							ptr->frghdr.fragment, ptr->frghdr.id,
							ptr->conf_sport, ptr->conf_dport,
							PayloadPtr, ptr->ErrFlag, &pktLen);
			break;
		case _PKT_TYPE_V6HOPBYHOP:
			#ifdef _MSG
			TEST_DEBUG("\t* Generate V6 hop by hop packet!");
			#endif
			protoForm_vsp_v6hop(buff, l2Flag, ptr->extHdr, ptr->conf_dmac, ptr->conf_smac, 
							ptr->conf_ctag_protocol,ptr->conf_ctag_reason,ptr->conf_ctag_priority,ptr->conf_ctag_extspa,ptr->conf_ctag_streamid,ptr->conf_ctag_l3r,ptr->conf_ctag_org,ptr->conf_ctag_extdpmsk,ptr->conf_ctag_spa,ptr->conf_ctag_fbi,ptr->conf_ctag_fb_index,
							ptr->ctag_tx2.txmsk,ptr->ctag_tx2.prisel,ptr->ctag_tx2.pri,ptr->ctag_tx2.directTx,ptr->ctag_tx2.keep,
							ptr->ctag_tx2.dislrn,ptr->ctag_tx2.psel,ptr->ctag_tx2.extspa,ptr->ctag_tx2.pppoeact,ptr->ctag_tx2.pppoeidx,ptr->ctag_tx2.streamid,
							ptr->svlan.tpid,ptr->svlan.svid, ptr->svlan.dei, ptr->svlan.sprio,
							ptr->cvlan.cvid, ptr->cvlan.cfi, ptr->cvlan.cprio,
							ptr->pppoe.type, ptr->pppoe.session, 
							ptr->ipv6.version, ptr->ipv6.priority, ptr->ipv6.flowlbl, ptr->ipv6.hoplmt, 
							ptr->conf_v6sip, ptr->conf_v6dip,
							ptr->rhdr.hdrlen, ptr->rhdr.segments_left, ptr->rhdr.dip,
							ptr->frghdr.fragment, ptr->frghdr.id,
							ptr->conf_sport, ptr->conf_dport,
							ptr->conf_tcp_seq, ptr->conf_tcp_ack, ptr->conf_tcp_flag, ptr->conf_tcp_win,
							PayloadPtr, ptr->ErrFlag, &pktLen);
			break;
		case _PKT_TYPE_V6GRE:
		//	TEST_DEBUG("in IPV6");
			#ifdef _MSG
			TEST_DEBUG("\t* Generate V6 PPTP(GRE) packet!");
			#endif
			protoForm_vsp_v6pptp(buff, l2Flag, ptr->extHdr, ptr->conf_dmac, ptr->conf_smac, 
							ptr->conf_ctag_protocol,ptr->conf_ctag_reason,ptr->conf_ctag_priority,ptr->conf_ctag_extspa,ptr->conf_ctag_streamid,ptr->conf_ctag_l3r,ptr->conf_ctag_org,ptr->conf_ctag_extdpmsk,ptr->conf_ctag_spa,ptr->conf_ctag_fbi,ptr->conf_ctag_fb_index,
							ptr->ctag_tx2.txmsk,ptr->ctag_tx2.prisel,ptr->ctag_tx2.pri,ptr->ctag_tx2.directTx,ptr->ctag_tx2.keep,
							ptr->ctag_tx2.dislrn,ptr->ctag_tx2.psel,ptr->ctag_tx2.extspa,ptr->ctag_tx2.pppoeact,ptr->ctag_tx2.pppoeidx,ptr->ctag_tx2.streamid,
							ptr->svlan.tpid,ptr->svlan.svid, ptr->svlan.dei, ptr->svlan.sprio,
							ptr->cvlan.cvid, ptr->cvlan.cfi, ptr->cvlan.cprio,
							ptr->pppoe.type, ptr->pppoe.session, 
							ptr->ipv6.version, ptr->ipv6.priority, ptr->ipv6.flowlbl, ptr->ipv6.nxthdr, ptr->ipv6.hoplmt,
							ptr->conf_v6sip, ptr->conf_v6dip,
							ptr->rhdr.hdrlen, ptr->rhdr.segments_left, ptr->rhdr.dip,
							ptr->frghdr.fragment, ptr->frghdr.id,
							ptr->pptp.callid, ptr->pptp.seqno, ptr->pptp.ackno,
							PayloadPtr, ptr->ErrFlag, &pktLen);
			break;
		/*DOS*/
		case _PKT_TYPE_TCP_SHORT_HDR:
			#ifdef _MSG
			TEST_DEBUG("\t* Generate TCP short header packet...");
			#endif
			protoForm_vsp_tcp_short_hdr(buff, l2Flag, ptr->conf_dmac, ptr->conf_smac,
					ptr->conf_ctag_protocol,ptr->conf_ctag_reason,ptr->conf_ctag_priority,ptr->conf_ctag_extspa,ptr->conf_ctag_streamid,ptr->conf_ctag_l3r,ptr->conf_ctag_org,ptr->conf_ctag_extdpmsk,ptr->conf_ctag_spa,ptr->conf_ctag_fbi,ptr->conf_ctag_fb_index,
					ptr->ctag_tx2.txmsk,ptr->ctag_tx2.prisel,ptr->ctag_tx2.pri,ptr->ctag_tx2.directTx,ptr->ctag_tx2.keep,
					ptr->ctag_tx2.dislrn,ptr->ctag_tx2.psel,ptr->ctag_tx2.extspa,ptr->ctag_tx2.pppoeact,ptr->ctag_tx2.pppoeidx,ptr->ctag_tx2.streamid,
					ptr->svlan.tpid,ptr->svlan.svid, ptr->svlan.dei, ptr->svlan.sprio,
					ptr->cvlan.cvid, ptr->cvlan.cfi, ptr->cvlan.cprio,
					ptr->pppoe.type, ptr->pppoe.session,
					ptr->ip.version, ptr->ip.headerLen,
					ptr->ip.tos, ptr->ip.id, ptr->ip.diffserv, ptr->ip.mf, ptr->ip.offset,
					ptr->ip.ttl, ptr->conf_sip, ptr->conf_dip,
					ptr->conf_sport, ptr->conf_dport,
					ptr->conf_tcp_seq, ptr->conf_tcp_ack, ptr->conf_tcp_flag, ptr->conf_tcp_win,
					PayloadPtr, ptr->ErrFlag, &pktLen);
			break;
		case _PKT_TYPE_V6TCP_SHORT_HDR:
			#ifdef _MSG
			TEST_DEBUG("\t* Generate V6TCP short header packet!");
			#endif
			protoForm_vsp_v6tcp_short_hdr(buff, l2Flag, ptr->extHdr, ptr->conf_dmac, ptr->conf_smac,
							ptr->conf_ctag_protocol,ptr->conf_ctag_reason,ptr->conf_ctag_priority,ptr->conf_ctag_extspa,ptr->conf_ctag_streamid,ptr->conf_ctag_l3r,ptr->conf_ctag_org,ptr->conf_ctag_extdpmsk,ptr->conf_ctag_spa,ptr->conf_ctag_fbi,ptr->conf_ctag_fb_index,
							ptr->ctag_tx2.txmsk,ptr->ctag_tx2.prisel,ptr->ctag_tx2.pri,ptr->ctag_tx2.directTx,ptr->ctag_tx2.keep,
							ptr->ctag_tx2.dislrn,ptr->ctag_tx2.psel,ptr->ctag_tx2.extspa,ptr->ctag_tx2.pppoeact,ptr->ctag_tx2.pppoeidx,ptr->ctag_tx2.streamid,
							ptr->svlan.tpid,ptr->svlan.svid, ptr->svlan.dei, ptr->svlan.sprio,
							ptr->cvlan.cvid, ptr->cvlan.cfi, ptr->cvlan.cprio,
							ptr->pppoe.type, ptr->pppoe.session,
							ptr->ipv6.version, ptr->ipv6.priority, ptr->ipv6.flowlbl, ptr->ipv6.hoplmt,
							ptr->conf_v6sip, ptr->conf_v6dip,
							ptr->rhdr.hdrlen, ptr->rhdr.segments_left, ptr->rhdr.dip,
							ptr->frghdr.fragment, ptr->frghdr.id,
							ptr->conf_sport, ptr->conf_dport,
							ptr->conf_tcp_seq, ptr->conf_tcp_ack, ptr->conf_tcp_flag, ptr->conf_tcp_win,
							PayloadPtr, ptr->ErrFlag, &pktLen);
			break;
		case _PKT_TYPE_UDP_BOMB:
			#ifdef _MSG
			TEST_DEBUG("\t* Generate UDP bomb packet...");
			#endif
			protoForm_vsp_udp_bomb(buff, l2Flag, ptr->conf_dmac, ptr->conf_smac,
						ptr->conf_ctag_protocol,ptr->conf_ctag_reason,ptr->conf_ctag_priority,ptr->conf_ctag_extspa,ptr->conf_ctag_streamid,ptr->conf_ctag_l3r,ptr->conf_ctag_org,ptr->conf_ctag_extdpmsk,ptr->conf_ctag_spa,ptr->conf_ctag_fbi,ptr->conf_ctag_fb_index,
						ptr->ctag_tx2.txmsk,ptr->ctag_tx2.prisel,ptr->ctag_tx2.pri,ptr->ctag_tx2.directTx,ptr->ctag_tx2.keep,
						ptr->ctag_tx2.dislrn,ptr->ctag_tx2.psel,ptr->ctag_tx2.extspa,ptr->ctag_tx2.pppoeact,ptr->ctag_tx2.pppoeidx,ptr->ctag_tx2.streamid,
						ptr->svlan.tpid,ptr->svlan.svid, ptr->svlan.dei, ptr->svlan.sprio,
						ptr->cvlan.cvid, ptr->cvlan.cfi, ptr->cvlan.cprio,
						ptr->pppoe.type, ptr->pppoe.session,
						ptr->ip.version, ptr->ip.headerLen,
						ptr->ip.tos, ptr->ip.id, ptr->ip.diffserv, ptr->ip.mf, ptr->ip.offset,
						ptr->ip.ttl, ptr->conf_sip, ptr->conf_dip,
						ptr->conf_sport, ptr->conf_dport,
						PayloadPtr, ptr->ErrFlag, &pktLen);
			break;
		case _PKT_TYPE_V6UDP_BOMB:
			#ifdef _MSG
			TEST_DEBUG("\t* Generate V6UDP bomb packet!");
			#endif
			protoForm_vsp_v6udp_bomb(buff, l2Flag, ptr->extHdr, ptr->conf_dmac, ptr->conf_smac,
							ptr->conf_ctag_protocol,ptr->conf_ctag_reason,ptr->conf_ctag_priority,ptr->conf_ctag_extspa,ptr->conf_ctag_streamid,ptr->conf_ctag_l3r,ptr->conf_ctag_org,ptr->conf_ctag_extdpmsk,ptr->conf_ctag_spa,ptr->conf_ctag_fbi,ptr->conf_ctag_fb_index,
							ptr->ctag_tx2.txmsk,ptr->ctag_tx2.prisel,ptr->ctag_tx2.pri,ptr->ctag_tx2.directTx,ptr->ctag_tx2.keep,
							ptr->ctag_tx2.dislrn,ptr->ctag_tx2.psel,ptr->ctag_tx2.extspa,ptr->ctag_tx2.pppoeact,ptr->ctag_tx2.pppoeidx,ptr->ctag_tx2.streamid,
							ptr->svlan.tpid,ptr->svlan.svid, ptr->svlan.dei, ptr->svlan.sprio,
							ptr->cvlan.cvid, ptr->cvlan.cfi, ptr->cvlan.cprio,
							ptr->pppoe.type, ptr->pppoe.session,
							ptr->ipv6.version, ptr->ipv6.priority, ptr->ipv6.flowlbl, ptr->ipv6.hoplmt,
							ptr->conf_v6sip, ptr->conf_v6dip,
							ptr->rhdr.hdrlen, ptr->rhdr.segments_left, ptr->rhdr.dip,
							ptr->frghdr.fragment, ptr->frghdr.id,
							ptr->conf_sport, ptr->conf_dport,
							PayloadPtr, ptr->ErrFlag, &pktLen);
			break;
		/*==========*/
		case _PKT_TYPE_UDP_LITE:
			#ifdef _MSG
			TEST_DEBUG("\t* Generate UDP-Lite packet...");
			#endif
			if (ptr->ip.offset){
				protoForm_vsp_ip(buff, l2Flag, ptr->conf_dmac, ptr->conf_smac,
							ptr->conf_ctag_protocol,ptr->conf_ctag_reason,ptr->conf_ctag_priority,ptr->conf_ctag_extspa,ptr->conf_ctag_streamid,ptr->conf_ctag_l3r,ptr->conf_ctag_org,ptr->conf_ctag_extdpmsk,ptr->conf_ctag_spa,ptr->conf_ctag_fbi,ptr->conf_ctag_fb_index,
							ptr->ctag_tx2.txmsk,ptr->ctag_tx2.prisel,ptr->ctag_tx2.pri,ptr->ctag_tx2.directTx,ptr->ctag_tx2.keep,
							ptr->ctag_tx2.dislrn,ptr->ctag_tx2.psel,ptr->ctag_tx2.extspa,ptr->ctag_tx2.pppoeact,ptr->ctag_tx2.pppoeidx,ptr->ctag_tx2.streamid,
							ptr->svlan.tpid,ptr->svlan.svid, ptr->svlan.dei, ptr->svlan.sprio,
							ptr->cvlan.cvid, ptr->cvlan.cfi, ptr->cvlan.cprio,
							ptr->pppoe.type, ptr->pppoe.session,
							ptr->ip.version, ptr->ip.headerLen,
							ptr->ip.tos, ptr->ip.id, ptr->ip.diffserv, ptr->ip.mf, ptr->ip.offset,
							ptr->ip.ttl, ptr->conf_sip, ptr->conf_dip, 0x88,
							PayloadPtr, ptr->ErrFlag, &pktLen);
			}else{
			protoForm_vsp_udp_lite(buff, l2Flag, ptr->conf_dmac, ptr->conf_smac,
						ptr->conf_ctag_protocol,ptr->conf_ctag_reason,ptr->conf_ctag_priority,ptr->conf_ctag_extspa,ptr->conf_ctag_streamid,ptr->conf_ctag_l3r,ptr->conf_ctag_org,ptr->conf_ctag_extdpmsk,ptr->conf_ctag_spa,ptr->conf_ctag_fbi,ptr->conf_ctag_fb_index,
						ptr->ctag_tx2.txmsk,ptr->ctag_tx2.prisel,ptr->ctag_tx2.pri,ptr->ctag_tx2.directTx,ptr->ctag_tx2.keep,
						ptr->ctag_tx2.dislrn,ptr->ctag_tx2.psel,ptr->ctag_tx2.extspa,ptr->ctag_tx2.pppoeact,ptr->ctag_tx2.pppoeidx,ptr->ctag_tx2.streamid,
						ptr->svlan.tpid,ptr->svlan.svid, ptr->svlan.dei, ptr->svlan.sprio,
						ptr->cvlan.tpid,ptr->cvlan.cvid, ptr->cvlan.cfi, ptr->cvlan.cprio,
						ptr->pppoe.type, ptr->pppoe.session,
						ptr->ip.version, ptr->ip.headerLen,
						ptr->ip.tos, ptr->ip.id, ptr->ip.diffserv, ptr->ip.mf, ptr->ip.offset,
						ptr->ip.ttl, ptr->conf_sip, ptr->conf_dip,
						ptr->conf_sport, ptr->conf_dport,
						PayloadPtr, ptr->ErrFlag, &pktLen);
			}
			break;
		case _PKT_TYPE_V6UDP_LITE:
			#ifdef _MSG
			TEST_DEBUG("\t* Generate V6UDP-lite packet!");
			#endif
			protoForm_vsp_v6udp_lite(buff, l2Flag, ptr->extHdr, ptr->conf_dmac, ptr->conf_smac, 
							ptr->conf_ctag_protocol,ptr->conf_ctag_reason,ptr->conf_ctag_priority,ptr->conf_ctag_extspa,ptr->conf_ctag_streamid,ptr->conf_ctag_l3r,ptr->conf_ctag_org,ptr->conf_ctag_extdpmsk,ptr->conf_ctag_spa,ptr->conf_ctag_fbi,ptr->conf_ctag_fb_index,
							ptr->ctag_tx2.txmsk,ptr->ctag_tx2.prisel,ptr->ctag_tx2.pri,ptr->ctag_tx2.directTx,ptr->ctag_tx2.keep,
							ptr->ctag_tx2.dislrn,ptr->ctag_tx2.psel,ptr->ctag_tx2.extspa,ptr->ctag_tx2.pppoeact,ptr->ctag_tx2.pppoeidx,ptr->ctag_tx2.streamid,
							ptr->svlan.tpid,ptr->svlan.svid, ptr->svlan.dei, ptr->svlan.sprio,
							ptr->cvlan.cvid, ptr->cvlan.cfi, ptr->cvlan.cprio,
							ptr->pppoe.type, ptr->pppoe.session, 
							ptr->ipv6.version, ptr->ipv6.priority, ptr->ipv6.flowlbl, ptr->ipv6.hoplmt, 
							ptr->conf_v6sip, ptr->conf_v6dip,
							ptr->rhdr.hdrlen, ptr->rhdr.segments_left, ptr->rhdr.dip,
							ptr->frghdr.fragment, ptr->frghdr.id,
							ptr->conf_sport, ptr->conf_dport,
							PayloadPtr, ptr->ErrFlag, &pktLen);
			break;
		default:
			#ifdef _MSG
			TEST_DEBUG("\t* Unknown pkt type[%d], Abort!\n", ptr->pktType);
			#endif
			break;
	}

	/* for 50B,(50 is also ok), when AUTOADD, we don't change ethertype */
	#if 0
	if (ptr->conf_Flags & pkt_PKTHDR_PPPOE_AUTOADD){
		l2FormLayer3Type(l2Flag, buff, 0x8864);
	}
	#endif

	/* If packet is smaller than 60 bytes, add tailer padding. */
	if((l2Flag & L2_CPUTAG_OTHER) | (l2Flag & L2_CPUTAG_TX2))
	{
		if (pktLen < 68)
			pktLen = 68;
	}else if(!skip_minPktLen_check)
	{
		if (pktLen < 60)
		{
			//TEST_WARNING("pktLen=%d",pktLen);
			//dump_packet(buff, 84, "\033[1;33;40m[gg]\033[0m");	
#if 0 /* Padding has been done in l2FormCrc(). */
			memset(&buff[pktLen], 0x20/*Ethernet standard*/, (60 - pktLen));
#endif
			pktLen = 60;
/*
			if(l2Flag & L2_CVLAN)
				pktLen += 4;
			if(l2Flag & L2_SVLAN)
				pktLen += 4;
*/
			//TEST_WARNING("after pktLen=%d",pktLen);
		}
	}

	#ifdef _MSG
	//TEST_DEBUG("\tSUCCESS!!\n");
	TEST_DEBUG ("\t* Packet length\t\t\t[%d]\n", pktLen);
	#endif

	kfree(ptr);
	
	return pktLen;
}


uint32 grGen(rtl8651_IgmpGrConf_t *grCfg, uint32 grCnt, int8 *buff)
{
	rtl8651_IgmpGrConf_t *cfgptr;
	igmpGr_t* ptr;
	uint32 grLen = 0;
	uint32 cnt;

	if (!buff || !grCfg)
		goto out;

	ptr = (igmpGr_t*)buff;
	cfgptr = grCfg;

	for (cnt = 0 ; cnt < grCnt ; cnt ++, cfgptr ++)
	{	// generate
		uint32 i;
		uint32 *ss, *sd;
	
		ptr->igmp_gr_rt		= cfgptr->type;
		ptr->igmp_gr_auxlen	= cfgptr->auxlen;
		ptr->igmp_gr_nofs		= htons(cfgptr->nofs);
		ptr->igmp_gr_group	= htonl(cfgptr->gaddr);
		sd = &(ptr->src_list);
		ss = cfgptr->slist;
		for ( i = 0 ; i < cfgptr->nofs ; i ++)
		{
			*sd = htonl(*ss);
			sd ++;
			ss ++;
		}
		if (cfgptr->auxlen && cfgptr->auxdata)
			memcpy((void*)sd, cfgptr->auxdata, cfgptr->auxlen);

		grLen += (8 + (4 * cfgptr->nofs) + cfgptr->auxlen);
		ptr = (igmpGr_t *)(ptr + (uint32)(8 + (4 * cfgptr->nofs) + cfgptr->auxlen));
	}
out:
	return grLen;
}

int pptpOuterHdrRemoveOptionalFields(uint8 *pktBuf, int offset, uint8 rmSeqBit, uint8 rmAckBit, int pktlen)
{
	// remove seq or ack field by reset the filed to 0; caller function should take care the outer header len by itself.
	greHdr_t *pptpPtr = (greHdr_t *)(pktBuf+offset);

	//dump_packet(pktBuf, pktlen, "PPTP Outer heder(Before)");
	if(rmAckBit)
	{
		// ack bit should be 0 and remvoe ack field in gre header.		
		pptpPtr->gre_flags_x2 &= ~0x80;	// cancel A BIT: 0x80
		memmove(pktBuf+offset+12, pktBuf+offset+16, pktlen-offset-12-4);// remove ack field
#ifdef _MSG		
		TEST_DEBUG("remove GRE ACK field");
#endif
	}
	
	if(rmSeqBit)
	{
		// seq bit should be 0 and remvoe seq field in gre header.
		pptpPtr->gre_flags_x1 &= ~GRE_S_BIT;		// cancel GRE_S_BIT: 0x10		
		memmove(pktBuf+offset+8, pktBuf+offset+12, pktlen-offset-8-4);// remove seq field
#ifdef _MSG		
		TEST_DEBUG("remove GRE SEQ field");
#endif
	}
	//dump_packet(pktBuf, pktlen, "PPTP Outer heder(After)");
	
	return SUCCESS;
}

/* Gen IPv4+GRE+PPP header only */
int pptpOuterHdrGen_lcp(uint8 *pktBuf, uint8 *sip, uint8 *dip, int pptp_callid, uint8 innerIsIPv4, uint32 outerHdrLen, uint32 outerHdrTos, uint32 outerHdrTTL, uint32 outerHdrFragment, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t cmpPPPMode, uint8 ppp_lcp)
{
	rtl8651_PktConf_t conf;
	uint32 pktLen, pktFmt;
	uint8 *tmpPktBuf = NULL;	
	uint8 rmSeqBit = (reducePPTPOpt&0x2)?1:0;
	uint8 rmAckBit = (reducePPTPOpt&0x1)?1:0;
	if((tmpPktBuf = rtlglue_malloc(64)) == NULL)
		return FAILED;
	bzero(tmpPktBuf, 64);
	bzero(&conf, sizeof(conf));

#ifdef _MSG
	if((reducePPTPOpt==0 && outerHdrLen<37))
		TEST_DEBUG("!!! PLEASE confirm if %s function support len %d !!!\r\n", __FUNCTION__, outerHdrLen);
#endif
	
	conf.pktType			= _PKT_TYPE_PPTP; 				   
	conf.ip.id 				= 0;		// keep zero and replace it by extra tag action
	conf.ip.tos				= outerHdrTos;	//dscp+ecn
	conf.ip.ttl				= outerHdrTTL;
	conf.ip.mf 				= (outerHdrFragment&0x2000)?1:0;
	conf.ip.offset 			= (outerHdrFragment&0x1fff)<<3;
	conf.payload.length		= 0;		// keep zero and replace it by extra tag action
	conf.ip.headerLen		= 20;
	conf.ip.protocol		= 0x2f;		//GRE
	conf.ip.version			=0x4;
	conf.payload.content	= NULL;
	conf.l2Flag				= DUAL_OUTER_HDR;
	conf.conf_sip	 		= ntohl(inet_addr(sip));
	conf.conf_sport 		= int_port;
	conf.conf_dip	 		= ntohl(inet_addr(dip));
	conf.conf_dport 		= REMOTE_PORT;
	strtomac((void *)&conf.conf_smac[0], lan_host_mac1);	// any one is fine because it will be removed
	strtomac((void *)&conf.conf_dmac[0], lan_host_mac1);	// any one is fine because it will be removed

	conf.pptp.callid = pptp_callid;
	conf.pptp.seqno = 0;			// keep zero and replace it by extra tag action
	conf.pptp.ackno = 0;			// keep zero and replace it by extra tag action

	pktLen = pktGen(&conf, (int8*)tmpPktBuf); 		   /* generate pkt in buff */
	/*PPP*/
	pktFmt = (conf.l2Flag & L2_MASK) | L3_IP | L4_PPTP | L3_GRE;
	if(ppp_lcp)
		l3FormGREPPPProtocol(pktFmt, tmpPktBuf, 	0xc021, cmpPPPMode);
	else	
		l3FormGREPPPProtocol(pktFmt, tmpPktBuf, 	innerIsIPv4?(0x0021):(0x0057), cmpPPPMode);

	//dump_packet(tmpPktBuf, 64, "\033[1;33;40m[PPTP Outer header]\033[0m");

	memcpy(pktBuf, (tmpPktBuf+sizeof(_etherHdr_t)), 40-cmpPPPMode);	// copy complete 37~40 bytes: IPv4(20)+PPTP(16)+PPP(1~4) header
	
	// adjust outer header format
	pptpOuterHdrRemoveOptionalFields(pktBuf, sizeof(ipHdr_t), rmSeqBit,  rmAckBit, 40-cmpPPPMode);

	rtlglue_free(tmpPktBuf);
	return 0;
}

int pptpOuterHdrGen(uint8 *pktBuf, uint8 *sip, uint8 *dip, int pptp_callid, uint8 innerIsIPv4, uint32 outerHdrLen, uint32 outerHdrTos, uint32 outerHdrTTL, uint32 outerHdrFragment, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t cmpPPPMode)
{
	return pptpOuterHdrGen_lcp(pktBuf, sip, dip, pptp_callid, innerIsIPv4, outerHdrLen, outerHdrTos, outerHdrTTL, outerHdrFragment, reducePPTPOpt, cmpPPPMode, 0);
}

/* Gen IPv6+GRE+PPP header only */
int pptpOuterV6HdrGen_lcp(uint8 *pktBuf, uint8 *sip6, uint8 *dip6, int pptp_callid, uint8 innerIsIPv4, uint32 outerHdrLen, uint32 outerHdrTos, uint32 outerHdrHoplimit, uint32 outerHdrFragment, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t cmpPPPMode, uint8 ppp_lcp)
{
	rtl8651_PktConf_t conf;
	uint32 pktLen, pktFmt;
	uint8 *tmpPktBuf = NULL;	
	uint8 rmSeqBit = (reducePPTPOpt&0x2)?1:0;
	uint8 rmAckBit = (reducePPTPOpt&0x1)?1:0;
	uint8 extHdrLen=0;
	if((tmpPktBuf = rtlglue_malloc(84)) == NULL)
		return FAILED;
	bzero(tmpPktBuf, 84);
	bzero(&conf, sizeof(conf));

#ifdef _MSG
	if((reducePPTPOpt==0 && outerHdrLen<57))
		TEST_DEBUG("!!! PLEASE confirm if %s function support len %d !!!\r\n", __FUNCTION__, outerHdrLen);
#endif
	
	conf.pktType			= _PKT_TYPE_V6GRE; 		
	conf.ipv6.pldlen		= 0;	// keep zero and replace it by dual hdr control register
	conf.ipv6.version		= 6;
	conf.rhdr.hdrlen		= 0;
	conf.ipv6.priority		= outerHdrTos;	//dscp+ecn
	conf.ipv6.flowlbl[0] 	= 0x0;
	conf.ipv6.flowlbl[1] 	= 0x0;
	conf.ipv6.flowlbl[2] 	= 0x0;
	conf.ipv6.hoplmt		= outerHdrHoplimit;
	if(outerHdrFragment==0x1){
		conf.extHdr = FraHdr;
		conf.frghdr.fragment = 0x1;
		conf.frghdr.id = 0x2379;
		extHdrLen=8;
	}else if(outerHdrFragment==0x8){
		conf.extHdr = FraHdr;
		conf.frghdr.fragment = 0x368;
	   	conf.frghdr.id = 0x2379;
		extHdrLen=8;
	}else if(outerHdrFragment==0x10){		//for ping of death
		conf.extHdr = FraHdr;
		conf.frghdr.fragment = (0x1ffff<<3)&0xff;
	   	conf.frghdr.id = 0x2379;
		extHdrLen=8;
	}else if(outerHdrFragment>0x10){		//for ping of death
 		conf.extHdr = FraHdr;
		conf.frghdr.fragment = ((0x1ffff<<3) | 1)&0xff;
	   	conf.frghdr.id = 0x2379;
		extHdrLen=8;
	}else if(outerHdrFragment>0){
 		conf.extHdr = FraHdr;
		conf.frghdr.fragment = 0x369;
	   	conf.frghdr.id = 0x2379;
		extHdrLen=8;
	}
	//TEST_DEBUG("conf.extHdr=%x",conf.extHdr);
	conf.ipv6.nxthdr		= 0x2f;	//47
	memcpy(conf.conf_v6sip,sip6,16);
	memcpy(conf.conf_v6dip,dip6,16);
	//in6_pton(v6_wan_gip,-1,(unsigned char *)&conf.conf_v6sip,-1,NULL);
	//in6_pton(v6_wan_remote_host,-1,(unsigned char *)&conf.conf_v6dip,-1,NULL);
	conf.conf_sport = int_port; 	
	conf.conf_dport = REMOTE_PORT;		
	strtomac((void *)&conf.conf_smac[0], lan_host_mac1);	// any one is fine because it will be removed
	strtomac((void *)&conf.conf_dmac[0], lan_host_mac1);	// any one is fine because it will be removed

	conf.pptp.callid = pptp_callid;
	conf.pptp.seqno = 0;			// keep zero and replace it by dual hdr control register
	conf.pptp.ackno = 0;			// keep zero and replace it by dual hdr control register
	
	pktLen = pktGen(&conf, (int8*)tmpPktBuf); 		   /* generate pkt in buff */
	/*PPP*/
	pktFmt = (conf.l2Flag & L2_MASK) | L3_IPV6 | L4_PPTP;

	if(ppp_lcp)
		l3FormV6GREPPPProtocol(pktFmt, tmpPktBuf, extHdrLen, 0xc021, cmpPPPMode);
	else
		l3FormV6GREPPPProtocol(pktFmt, tmpPktBuf, extHdrLen, innerIsIPv4?(0x0021):(0x0057), cmpPPPMode);

	//dump_packet(tmpPktBuf, 84, "\033[1;33;40m[PPTP Outerv6 header]\033[0m");	

	memcpy(pktBuf, (tmpPktBuf+sizeof(_etherHdr_t)), 60-cmpPPPMode+extHdrLen);	// copy complete 57~60 bytes: IPv6(40)+PPTP(16)+PPP(1~4) header
	
	// adjust outer header format
	pptpOuterHdrRemoveOptionalFields(pktBuf, sizeof(ip6Hdr_t)+extHdrLen, rmSeqBit,  rmAckBit, 60-cmpPPPMode+extHdrLen);

	rtlglue_free(tmpPktBuf);
	return 0;
}

int pptpOuterV6HdrGen(uint8 *pktBuf, uint8 *sip6, uint8 *dip6, int pptp_callid, uint8 innerIsIPv4, uint32 outerHdrLen, uint32 outerHdrTos, uint32 outerHdrHoplimit, uint32 outerHdrFragment, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t cmpPPPMode)
{
	return pptpOuterV6HdrGen_lcp(pktBuf, sip6, dip6, pptp_callid, innerIsIPv4, outerHdrLen, outerHdrTos, outerHdrHoplimit, outerHdrFragment, reducePPTPOpt, cmpPPPMode, 0);
}

/* Gen IPv4+UDP+L2TP+PPP header only */
int l2tpOuterHdrGen(uint8 *pktBuf, uint8 *sip, uint8 *dip, uint16 length, uint16 tunnelID, uint16 sessionID, uint8 innerIsIPv4, uint32 outerHdrLen, uint8 reduceL2TPLen, compressed_ppp_mode_t cmpPPPMode)
{
	rtl8651_PktConf_t conf;
	uint32 pktLen, pktFmt;
	int8 udppayload[12];
	uint8 *tmpPktBuf = NULL;
	if((tmpPktBuf = rtlglue_malloc(128)) == NULL)
		return FAILED;
	bzero(tmpPktBuf, 128);
	bzero(&conf, sizeof(conf));

#ifdef _MSG	
	if((reduceL2TPLen==0 && outerHdrLen<37) || (reduceL2TPLen==1 && outerHdrLen< 35))	// supported l2tp format
		TEST_WARNING("!!! PLEASE confirm if %s function support len %d !!!", __FUNCTION__, outerHdrLen);
#endif	

	{
		// MUST prepare UDP payload first for udp checksum calculation.
		/*L2TP + PPP*/
		//pktLen += (8+4);		// L2TP + PPP
		pktFmt = (conf.l2Flag & L2_MASK) | L3_IP | L4_UDP;
		if(reduceL2TPLen)	// for the format without len filed in L2TP header.
			l2FormL2TPPPProtocol(pktFmt, &udppayload[0], -1, tunnelID, sessionID, innerIsIPv4?(0x0021):(0x0057), cmpPPPMode);
		else if(cmpPPPMode == COMPRESSED_PPP_FF03C021){	// L2tp LCP
			cmpPPPMode = COMPRESSED_PPP_FF0300XX;
			l2FormL2TPPPProtocol(pktFmt, &udppayload[0], -1, tunnelID, sessionID, (0xc021), cmpPPPMode);
		}else
			l2FormL2TPPPProtocol(pktFmt, &udppayload[0], length, tunnelID, sessionID, innerIsIPv4?(0x0021):(0x0057), cmpPPPMode);
	}
	
	conf.pktType			= _PKT_TYPE_UDP; 				   
	conf.ip.id 			= 0;			// keep zero and replace it by extra tag action
	conf.ip.ttl				= 254;
	conf.payload.length	= 0;
	conf.ip.headerLen		= 20;
	conf.ip.protocol		= 17;
	conf.ip.version		=0x4;
	conf.payload.content	= NULL;
	conf.l2Flag			= DUAL_OUTER_HDR;
	conf.conf_sip	 = ntohl(inet_addr(sip));
	conf.conf_sport = L2TP_udpsrc_port;
	conf.conf_dip	 = ntohl(inet_addr(dip));
	conf.conf_dport = L2TP_session_port;
	strtomac((void *)&conf.conf_smac[0], lan_host_mac1);	// any one is fine because it will be removed
	strtomac((void *)&conf.conf_dmac[0], lan_host_mac1);	// any one is fine because it will be removed

	conf.payload.length 			= 8+(4-cmpPPPMode);	// L2TP + PPP
	conf.payload.content			= &udppayload[0];

	
	
	pktLen = pktGen(&conf, (int8*)tmpPktBuf); 		   /* generate pkt in buff */



	//dump_packet(tmpPktBuf, 64, "\033[1;33;40m[L2TP Outer header]\033[0m");
	
	memcpy(pktBuf, (tmpPktBuf+sizeof(_etherHdr_t)), outerHdrLen);

	rtlglue_free(tmpPktBuf);
	return 0;
}

int normalPktGen_ecn_syn_errFlag(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,//type: 0:l2_unicast/1:multicast/2:broadcast/3:l34_unicast
	int ctagif,int cvid,int cpri,
	int stagif,int svid,int spri, int dscp, int ecn, int syn_option,
	cputag_t *cputag, cputagTx_t *cputagTx,
	int pppoetagif,int pppoe_session_id,int ttl,uint32 errFlag, uint32 ipver, uint32 ip6ver) 
{
	//int i;
	rtl8651_PktConf_t conf;
	uint32 proto_type = type&0x1ff;
	
	bzero(&conf, sizeof(conf));

	if(type!=TYPE_DUALHDR_PPTPV6&&type!=TYPE_DUALHDR_L2TPV6){
		payloadGen(pktData, *pktLen);
	}
	if (type<TYPE_L2UNICAST) //MC
		conf.pktType	= _PKT_TYPE_UDP;
	else {
		conf.pktType	= _PKT_TYPE_TCP;
		if(syn_option)
			conf.l4hdr.tc.tcpFlag = TH_SYN;
	}
	conf.ip.id		= 0x123;
	conf.ip.ttl 		= ttl;
	conf.ip.tos 		= dscp<<2|ecn;
	conf.payload.length 	= *pktLen;
	conf.ip.version		= 0x4;
	conf.ip.headerLen	= 20;
	conf.payload.content	= pktData;
	conf.pppoe.type 	= 0x0800;
	conf.pppoe.session	= 0;
	conf.cvlan.cprio	= 1;
	conf.l2Flag 		= 0;
	conf.ErrFlag		= errFlag;

	//conf.conf_tcp_flag = TH_RST;
	if (ctagif) {
		conf.l2Flag 		= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 	= cvid;
		conf.cvlan.cprio	= cpri;
	}
	
	if (stagif) {
		conf.l2Flag 		= conf.l2Flag | L2_SVLAN;
		conf.svlan.svid 	= svid;
		conf.svlan.sprio	= spri;
#if defined(CONFIG_FPGA_PON_TEST)
		conf.svlan.tpid 	= 0x8100;
#else
		conf.svlan.tpid		= 0x88a8;
#endif
	}
	
	if (cputag!=NULL) {
		conf.l2Flag 		= conf.l2Flag | L2_CPUTAG_OTHER;
		conf.conf_ctag_protocol	= 0x4;
		conf.conf_ctag_reason	= cputag->res;
		conf.conf_ctag_priority	= cputag->pri;
		conf.conf_ctag_streamid = cputag->streamid;
//		conf.conf_ctag_ttlde	= cputag->ttl_1;
		conf.conf_ctag_l3r	= cputag->l3r;
		conf.conf_ctag_org	= cputag->org;
		conf.conf_ctag_spa	= cputag->spa;
		conf.conf_ctag_extspa	= cputag->extspa;
		conf.conf_ctag_extdpmsk	= cputag->epmsk;
		conf.conf_ctag_fbi = cputag->fbi;
		conf.conf_ctag_fb_index = cputag->fbindex;
	}else if (cputagTx != NULL) {
		conf.l2Flag 		= conf.l2Flag | L2_CPUTAG_TX2;
		conf.conf_ctag_protocol	= 0x4;
		conf.ctag_tx2.txmsk = cputagTx->txmsk;
		conf.ctag_tx2.prisel = cputagTx->prisel;
		conf.ctag_tx2.pri = cputagTx->pri;
		conf.ctag_tx2.keep = cputagTx->keep;
		conf.ctag_tx2.dislrn = cputagTx->dislrn;
		conf.ctag_tx2.psel = cputagTx->psel;
		conf.ctag_tx2.directTx = cputagTx->directTx;
		conf.ctag_tx2.extspa = cputagTx->extspa;
		conf.ctag_tx2.pppoeact = cputagTx->pppoeact;
		conf.ctag_tx2.pppoeidx = cputagTx->pppoeidx;
		conf.ctag_tx2.streamid = cputagTx->streamid;
	}

	if (pppoetagif==1) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8864;
		conf.pppoe.session 	= pppoe_session_id;		
	}else if(pppoetagif==2) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8863;
		conf.pppoe.session 	= pppoe_session_id;		
	}
	
	conf.conf_sip	= ntohl(inet_addr(lan_host_ip1));
	conf.conf_sport = int_port;
	conf.conf_dport = REMOTE_PORT;	
	strtomac((void *)&conf.conf_smac[0], lan_host_mac1);
	switch (proto_type)
	{
	case TYPE_L2UNICAST:
		conf.conf_dip	 = ntohl(inet_addr(lan_host_ip2));
		strtomac((void *)&conf.conf_dmac[0], lan_host_mac2);
		break;
	case TYPE_L2UNICAST1:
		conf.conf_dip	 = ntohl(inet_addr(lan_host_ip3));
		strtomac((void *)&conf.conf_dmac[0], lan_host_mac3);
		break;
	case TYPE_RMA_CDP:
		conf.conf_dip	 = ntohl(inet_addr(cdp_dip));
		strtomac((void *)&conf.conf_dmac[0], cdp_dmac);
		break;
	case TYPE_L2UNICAST_TO_EXTPORT:
		conf.conf_sip	= ntohl(inet_addr(lan_host_ip3));
		conf.conf_sport = int_port;
		conf.conf_dport = REMOTE_PORT;	
		strtomac((void *)&conf.conf_smac[0], lan_host_mac3);
		conf.conf_dip	 = ntohl(inet_addr(lan_host_ip1));
		strtomac((void *)&conf.conf_dmac[0], lan_host_mac1);
		break;
	case TYPE_MULTICAST:
		strtomac((void *)&conf.conf_dmac[0], mc_dmac);
		conf.conf_dip	 = ntohl(inet_addr(mc_dip));	
		break;
	case TYPE_MULTICAST_IGMP:
		strtomac((void *)&conf.conf_dmac[0], mc_dmac);
		conf.conf_dip	 = ntohl(inet_addr(mc_dip));	
		conf.pktType = _PKT_TYPE_IGMP;
		conf.igmp.ver					= 2;
		conf.igmp.type					= IGMP_V2_MEMBERSHIP_REPORT;
		break;
	case TYPE_IGMP_RESERVED_MULTICAST:
		strtomac((void *)&conf.conf_dmac[0], rsvd_mc_dmac);
		conf.conf_dip	 = ntohl(inet_addr(rsvd_mc_dip));	
		conf.pktType = _PKT_TYPE_IGMP;
		conf.igmp.ver					= 2;
		conf.igmp.type					= IGMP_V2_MEMBERSHIP_REPORT;
		break;
	case TYPE_RESERVED_MULTICAST_V4:
		strtomac((void *)&conf.conf_dmac[0], rsvd_mc_dmac);
		conf.conf_dip	 = ntohl(inet_addr(rsvd_mc_dip));	
		break;
	case TYPE_MULTICAST_ICMP6:
		conf.ipv6.pldlen 		= *pktLen + 20;
		conf.pktType	 		= _PKT_TYPE_V6ICMP;
		conf.icmp.type			= 135;
		conf.ipv6.version		= 6;
		conf.rhdr.hdrlen 		= 0;
		conf.ipv6.priority 		= dscp<<2|ecn;
		conf.ipv6.flowlbl[0] = 0x12;
		conf.ipv6.flowlbl[1] = 0x34;
		conf.ipv6.flowlbl[2] = 0x56;
		conf.ipv6.hoplmt 		= ttl;
		in6_pton(v6mld_sip,-1,(unsigned char *)&conf.conf_v6sip,-1,NULL);
		in6_pton(v6mld_dip,-1,(unsigned char *)&conf.conf_v6dip,-1,NULL);
		strtomac((void *)&conf.conf_dmac[0],v6mld_dmac);
		break;
	case TYPE_MULTICAST_DHCP6:
		conf.ipv6.pldlen 		= *pktLen + 20;
		conf.pktType	 		= _PKT_TYPE_V6UDP;
		conf.conf_sport 		= 546;
		conf.conf_dport 		= 547;	
		conf.ipv6.version		= 6;
		conf.rhdr.hdrlen 		= 0;
		conf.ipv6.priority 		= dscp<<2|ecn;
		conf.ipv6.flowlbl[0] = 0x12;
		conf.ipv6.flowlbl[1] = 0x34;
		conf.ipv6.flowlbl[2] = 0x56;
		conf.ipv6.hoplmt 		= ttl;
		in6_pton(v6mld_sip,-1,(unsigned char *)&conf.conf_v6sip,-1,NULL);
		in6_pton(v6mld_dip,-1,(unsigned char *)&conf.conf_v6dip,-1,NULL);
		strtomac((void *)&conf.conf_dmac[0],v6mld_dmac);
		break;
	case TYPE_MULTICAST_MLD:
		conf.ipv6.pldlen 		= *pktLen + 20;
		conf.pktType	 		= _PKT_TYPE_V6ICMP;
		conf.icmp.type			= 131;
		conf.ipv6.version		= 6;
		conf.rhdr.hdrlen 		= 0;
		conf.ipv6.priority 		= dscp<<2|ecn;
		conf.ipv6.flowlbl[0] = 0x12;
		conf.ipv6.flowlbl[1] = 0x34;
		conf.ipv6.flowlbl[2] = 0x56;
		conf.ipv6.hoplmt 		= ttl;
		in6_pton(v6mld_sip,-1,(unsigned char *)&conf.conf_v6sip,-1,NULL);
		in6_pton(v6mld_dip,-1,(unsigned char *)&conf.conf_v6dip,-1,NULL);
		strtomac((void *)&conf.conf_dmac[0],v6mld_dmac);
		break;
	case TYPE_RESERVED_MULTICAST_V6:
		conf.ipv6.pldlen 		= *pktLen + 20;
		conf.pktType	 		= _PKT_TYPE_V6UDP;
		conf.ipv6.version		= 6;
		conf.rhdr.hdrlen 		= 0;
		conf.ipv6.priority 		= dscp<<2|ecn;
		conf.ipv6.flowlbl[0] = 0x12;
		conf.ipv6.flowlbl[1] = 0x34;
		conf.ipv6.flowlbl[2] = 0x56;
		conf.ipv6.hoplmt 		= ttl;
		in6_pton(v6_common_src_ip,-1,(unsigned char *)&conf.conf_v6sip,-1,NULL);
		in6_pton(v6_rsvd_mc_dip,-1,(unsigned char *)&conf.conf_v6dip,-1,NULL);
		strtomac((void *)&conf.conf_dmac[0], v6_rsvd_mc_dmac);
		break;
	case TYPE_MLD_RESERVED_MULTICAST:
		conf.ipv6.pldlen 		= *pktLen + 20;
		conf.pktType	 		= _PKT_TYPE_V6ICMP;
		conf.icmp.type			= 131;
		conf.ipv6.version		= 6;
		conf.rhdr.hdrlen 		= 0;
		conf.ipv6.priority 		= dscp<<2|ecn;
		conf.ipv6.flowlbl[0] = 0x12;
		conf.ipv6.flowlbl[1] = 0x34;
		conf.ipv6.flowlbl[2] = 0x56;
		conf.ipv6.hoplmt 		= ttl;
		in6_pton(v6mld_sip,-1,(unsigned char *)&conf.conf_v6sip,-1,NULL);
		in6_pton(v6_rsvd_mc_dip,-1,(unsigned char *)&conf.conf_v6dip,-1,NULL);
		strtomac((void *)&conf.conf_dmac[0], v6_rsvd_mc_dmac);
		break;
	case TYPE_L2MULTICAST_IPV6:
		conf.ipv6.pldlen 		= *pktLen + 20;
		conf.pktType	 = _PKT_TYPE_V6UDP;
		conf.ipv6.version		= 6;
		conf.rhdr.hdrlen 		= 0;
		conf.ipv6.priority 		= dscp<<2|ecn;
		conf.ipv6.flowlbl[0] = 0x12;
		conf.ipv6.flowlbl[1] = 0x34;
		conf.ipv6.flowlbl[2] = 0x56;
		conf.ipv6.hoplmt 		= ttl;
		in6_pton(v6_common_src_ip,-1,(unsigned char *)&conf.conf_v6sip,-1,NULL);
		in6_pton(v6mc_dip,-1,(unsigned char *)&conf.conf_v6dip,-1,NULL);
		strtomac((void *)&conf.conf_dmac[0],v6mc_dmac);
		break;
	case TYPE_L2MULTICAST_IPV6_G2W_WITH_PPPOE:
		conf.ipv6.pldlen 		= *pktLen + 20;
		conf.pktType	 = _PKT_TYPE_V6UDP;
		conf.ipv6.version		= 6;
		conf.rhdr.hdrlen 		= 0;
		conf.ipv6.priority 		= dscp<<2|ecn;
		conf.ipv6.flowlbl[0] = 0x12;
		conf.ipv6.flowlbl[1] = 0x34;
		conf.ipv6.flowlbl[2] = 0x56;
		conf.ipv6.hoplmt 		= ttl;
		in6_pton(v6_common_src_ip,-1,(unsigned char *)&conf.conf_v6sip,-1,NULL);
		in6_pton(v6mc_dip,-1,(unsigned char *)&conf.conf_v6dip,-1,NULL);
		strtomac((void *)&conf.conf_smac[0], wan_gmac);
		if (pppoetagif==1)
			strtomac((void *)&conf.conf_dmac[0], wan_nexthop_mac);
		else
			strtomac((void *)&conf.conf_dmac[0], v6mc_dmac);
		break;
		
	case TYPE_BROADCAST_WAN:
		strtomac((void *)&conf.conf_smac[0], wan_gmac);
		conf.conf_dip	 = ntohl(inet_addr(bc_dip));
		strtomac((void *)&conf.conf_dmac[0], bc_dmac);
		break;		
	case TYPE_BROADCAST:
		conf.conf_dip	 = ntohl(inet_addr(bc_dip));
		strtomac((void *)&conf.conf_dmac[0], bc_dmac);
		break;
	case TYPE_BROADCAST_ARP:
		strtomac((void *)&conf.conf_dmac[0], bc_dmac);
		conf.pktType		= _PKT_TYPE_ARP;
		strtomac((void *)&conf.arp.mac_src[0], lan_host_mac1);
		strtomac((void *)&conf.arp.mac_dst[0], "00-00-00-00-00-00");
		conf.arp.ip_src	 = ntohl(inet_addr(lan_host_ip1));
		conf.arp.ip_dst	 = ntohl(inet_addr(lan_host_ip2));
		conf.arp.op = 1;	// arp request
		break;

	case TYPE_L3UNICAST_ROUTING_W2L:
	case TYPE_DUALHDR_DSLITE_UNICAST_ROUTING_W2L:
		strtomac((void *)&conf.conf_smac[0], wan_nexthop_mac);
		strtomac((void *)&conf.conf_dmac[0], wan_gmac);
		conf.conf_sip	 = ntohl(inet_addr(wan_remote_host));
		conf.conf_dip	 = ntohl(inet_addr(lan_host_ip1));		
		conf.conf_dport = int_port; 		
		conf.conf_sip	 = ntohl(inet_addr(wan_remote_host));
		conf.conf_sport = REMOTE_PORT;
	break;
	
	case TYPE_L3UNICAST_ROUTING_G2L:
		strtomac((void *)&conf.conf_smac[0], lan_gmac);
		strtomac((void *)&conf.conf_dmac[0], lan_host_mac1);
		conf.conf_sip	 = ntohl(inet_addr(wan_remote_host));
		conf.conf_dip	 = ntohl(inet_addr(lan_host_ip1));		
		conf.conf_dport = int_port; 		
		conf.conf_sip	 = ntohl(inet_addr(wan_remote_host));
		conf.conf_sport = REMOTE_PORT;
	break;
	
	case TYPE_L3UNICAST_ROUTING_G2W:
	case TYPE_DUALHDR_DSLITE_UNICAST_ROUTING_G2W: 	// cheney test 20151221
		strtomac((void *)&conf.conf_smac[0], wan_gmac);
		strtomac((void *)&conf.conf_dmac[0], wan_nexthop_mac);
		conf.conf_dip	 = ntohl(inet_addr(wan_remote_host));
	break;
	
	case TYPE_L3UNICAST_IPV6_L2W:
		conf.ipv6.pldlen 		= *pktLen + 20;
		conf.pktType	 = _PKT_TYPE_V6TCP;
		conf.ipv6.version		= 6;
		conf.rhdr.hdrlen 		= 0;
		conf.ipv6.priority 		= dscp<<2|ecn;
		conf.ipv6.flowlbl[0] = 0x12;
		conf.ipv6.flowlbl[1] = 0x34;
		conf.ipv6.flowlbl[2] = 0x56;
		conf.ipv6.hoplmt 		= ttl;
		in6_pton(v6_lan_host_ip,-1,(unsigned char *)&conf.conf_v6sip,-1,NULL);
		in6_pton(v6_wan_remote_host,-1,(unsigned char *)&conf.conf_v6dip,-1,NULL);
		strtomac((void *)&conf.conf_dmac[0], lan_gmac);
		conf.conf_sport = int_port; 	
		conf.conf_dport = REMOTE_PORT; 	
		break;
	case TYPE_L3UNICAST_IPV6_G2W:
		conf.ipv6.pldlen 		= *pktLen + 20;
		conf.pktType	 = _PKT_TYPE_V6TCP;
		conf.ipv6.version		= 6;
		conf.rhdr.hdrlen 		= 0;
		conf.ipv6.priority 		= dscp<<2|ecn;
		conf.ipv6.flowlbl[0] = 0x12;
		conf.ipv6.flowlbl[1] = 0x34;
		conf.ipv6.flowlbl[2] = 0x56;
		conf.ipv6.hoplmt 		= ttl;
		in6_pton(v6_lan_host_ip,-1,(unsigned char *)&conf.conf_v6sip,-1,NULL);
		in6_pton(v6_wan_remote_host,-1,(unsigned char *)&conf.conf_v6dip,-1,NULL);
		strtomac((void *)&conf.conf_smac[0], wan_gmac);
		strtomac((void *)&conf.conf_dmac[0], wan_nexthop_mac);
		conf.conf_sport = int_port; 	
		conf.conf_dport = REMOTE_PORT; 	
		break;
		
	case TYPE_L3UNICAST_IPV6_W2L:
		conf.ipv6.pldlen 		= *pktLen + 20;
		conf.pktType	 = _PKT_TYPE_V6TCP;
		conf.ipv6.version		= 6;
		conf.rhdr.hdrlen 		= 0;
		conf.ipv6.priority 		= dscp<<2|ecn;
		conf.ipv6.flowlbl[0] = 0x12;
		conf.ipv6.flowlbl[1] = 0x34;
		conf.ipv6.flowlbl[2] = 0x56;
		conf.ipv6.hoplmt 		= ttl;
		in6_pton(v6_wan_remote_host,-1,(unsigned char *)&conf.conf_v6sip,-1,NULL);
		in6_pton(v6_lan_host_ip,-1,(unsigned char *)&conf.conf_v6dip,-1,NULL);
		strtomac((void *)&conf.conf_smac[0], wan_nexthop_mac);
		strtomac((void *)&conf.conf_dmac[0], wan_gmac);
		conf.conf_sport = REMOTE_PORT; 	
		conf.conf_dport = int_port; 
		break;
		
	case TYPE_L3UNICAST_IPV6_G2L:
		conf.ipv6.pldlen 		= *pktLen + 20;
		conf.pktType	 = _PKT_TYPE_V6TCP;
		conf.ipv6.version		= 6;
		conf.rhdr.hdrlen 		= 0;
		conf.ipv6.priority 		= dscp<<2|ecn;
		conf.ipv6.flowlbl[0] = 0x12;
		conf.ipv6.flowlbl[1] = 0x34;
		conf.ipv6.flowlbl[2] = 0x56;
		conf.ipv6.hoplmt 		= ttl;
		in6_pton(v6_wan_remote_host,-1,(unsigned char *)&conf.conf_v6sip,-1,NULL);
		in6_pton(v6_lan_host_ip,-1,(unsigned char *)&conf.conf_v6dip,-1,NULL);
		strtomac((void *)&conf.conf_smac[0], lan_gmac);
		strtomac((void *)&conf.conf_dmac[0], lan_host_mac1);
		conf.conf_sport = REMOTE_PORT; 	
		conf.conf_dport = int_port; 
		break;

	case TYPE_L3UNICAST_IPV6_G2L_FRAG:
		conf.ipv6.pldlen		= *pktLen + 20;
		conf.pktType	 = _PKT_TYPE_V6TCP;
		conf.ipv6.version		= 6;
		conf.rhdr.hdrlen		= 0;
		conf.ipv6.priority		= dscp<<2|ecn;
		conf.ipv6.flowlbl[0] = 0x12;
		conf.ipv6.flowlbl[1] = 0x34;
		conf.ipv6.flowlbl[2] = 0x56;
		conf.ipv6.hoplmt		= ttl;
		conf.extHdr = FraHdr;
		conf.frghdr.fragment = 0x1;
		conf.frghdr.id = 0x2379;
		in6_pton(v6_wan_remote_host,-1,(unsigned char *)&conf.conf_v6sip,-1,NULL);
		in6_pton(v6_lan_host_ip,-1,(unsigned char *)&conf.conf_v6dip,-1,NULL);
		strtomac((void *)&conf.conf_smac[0], lan_gmac);
		strtomac((void *)&conf.conf_dmac[0], lan_host_mac1);
		conf.conf_sport = REMOTE_PORT;
		conf.conf_dport = int_port;
		break;
		
	case TYPE_L34UNICAST_L2W:
		strtomac((void *)&conf.conf_dmac[0], lan_gmac);
		conf.conf_dip	 = ntohl(inet_addr(wan_remote_host));
		break;
		
	case TYPE_L34UNICAST_G2W:
	case TYPE_DUALHDR_DSLITE_UNICAST_NAT_G2W:
		strtomac((void *)&conf.conf_smac[0], wan_gmac);
		strtomac((void *)&conf.conf_dmac[0], wan_nexthop_mac);
		conf.conf_sip	 = ntohl(inet_addr(wan_gip));
		conf.conf_dip	 = ntohl(inet_addr(wan_remote_host));
		conf.conf_sport  = ext_port;
		//conf.ip.ttl--;
		break;
		
	case TYPE_L34MULTICAST_L2W:			
		conf.conf_dip	 = ntohl(inet_addr(mc_dip));
		strtomac((void *)&conf.conf_dmac[0], mc_dmac);
		break;
		
	case TYPE_L34MULTICAST_L2W_WITH_PPPOE:
		conf.conf_dip	 = ntohl(inet_addr(mc_dip));
		if (pppoetagif==1)
			strtomac((void *)&conf.conf_dmac[0], lan_gmac);
		else
			strtomac((void *)&conf.conf_dmac[0], mc_dmac);
		break;	

	case TYPE_L34MULTICAST_G2W_WITH_PPPOE:
		strtomac((void *)&conf.conf_smac[0], wan_gmac);
		conf.conf_dip	 = ntohl(inet_addr(mc_dip));
		if (pppoetagif==1)
			strtomac((void *)&conf.conf_dmac[0], wan_nexthop_mac);
		else
			strtomac((void *)&conf.conf_dmac[0], mc_dmac);
		break;
		
	case TYPE_L34MULTICAST_W2L_WITH_PPPOE:
		strtomac((void *)&conf.conf_smac[0], wan_nexthop_mac);
		conf.conf_sip	 = ntohl(inet_addr(wan_remote_host));
		conf.conf_dip = ntohl(inet_addr(mc_dip));
		conf.conf_sport  = REMOTE_PORT;
		conf.conf_dport  = int_port;
		if (pppoetagif==1)
			strtomac((void *)&conf.conf_dmac[0], wan_gmac);
		else
			strtomac((void *)&conf.conf_dmac[0], mc_dmac);
		break;
	case TYPE_L34MULTICAST_G2L_WITH_PPPOE:
		strtomac((void *)&conf.conf_smac[0], lan_gmac);
		conf.conf_sip	 = ntohl(inet_addr(wan_remote_host));
		conf.conf_dip = ntohl(inet_addr(mc_dip));
		conf.conf_sport  = REMOTE_PORT;
		conf.conf_dport  = int_port;
		if (pppoetagif==1)
			strtomac((void *)&conf.conf_dmac[0], lan_host_mac1);
		else
			strtomac((void *)&conf.conf_dmac[0], mc_dmac);
		break;
		
	case TYPE_L34MULTICAST_IPV6_W2L_WITH_PPPOE:
		conf.ipv6.pldlen 		= *pktLen + 20;
		conf.pktType	 = _PKT_TYPE_V6UDP;
		conf.ipv6.version		= 6;
		conf.rhdr.hdrlen 		= 0;
		conf.ipv6.priority 		= dscp<<2|ecn;
		conf.ipv6.flowlbl[0] = 0x12;
		conf.ipv6.flowlbl[1] = 0x34;
		conf.ipv6.flowlbl[2] = 0x56;
		conf.ipv6.hoplmt 		= ttl;
		strtomac((void *)&conf.conf_smac[0], wan_nexthop_mac);
		in6_pton(v6_wan_remote_host,-1,(unsigned char *)&conf.conf_v6sip,-1,NULL);
		in6_pton(v6mc_dip,-1,(unsigned char *)&conf.conf_v6dip,-1,NULL);
		conf.conf_sport  = REMOTE_PORT;
		conf.conf_dport  = int_port;
		if (pppoetagif==1)
			strtomac((void *)&conf.conf_dmac[0], wan_gmac);
		else
			strtomac((void *)&conf.conf_dmac[0], v6mc_dmac);
		break;	

	case TYPE_L34MULTICAST_IPV6_G2L_WITH_PPPOE:
		conf.ipv6.pldlen 		= *pktLen + 20;
		conf.pktType	 = _PKT_TYPE_V6UDP;
		conf.ipv6.version		= 6;
		conf.rhdr.hdrlen 		= 0;
		conf.ipv6.priority 		= dscp<<2|ecn;
		conf.ipv6.flowlbl[0] = 0x12;
		conf.ipv6.flowlbl[1] = 0x34;
		conf.ipv6.flowlbl[2] = 0x56;
		conf.ipv6.hoplmt 		= ttl;
		strtomac((void *)&conf.conf_smac[0], lan_gmac);
		in6_pton(v6_wan_remote_host,-1,(unsigned char *)&conf.conf_v6sip,-1,NULL);
		in6_pton(v6mc_dip,-1,(unsigned char *)&conf.conf_v6dip,-1,NULL);
		conf.conf_sport  = REMOTE_PORT;
		conf.conf_dport  = int_port;
		if (pppoetagif==1)
			strtomac((void *)&conf.conf_dmac[0], lan_host_mac1);
		else
			strtomac((void *)&conf.conf_dmac[0], v6mc_dmac);
		break;	
		
	case TYPE_L34MULTICAST_G2W_L3:
		strtomac((void *)&conf.conf_smac[0], wan_gmac);
		conf.conf_dip	 = ntohl(inet_addr(mc_dip));
		strtomac((void *)&conf.conf_dmac[0], mc_dmac);
		break;
		
	case TYPE_L34MULTICAST_G2W_L4:  //L4
		strtomac((void *)&conf.conf_smac[0], wan_gmac);
		conf.conf_dip	 = ntohl(inet_addr(mc_dip));
		conf.conf_sip	 = ntohl(inet_addr(wan_gip));
		strtomac((void *)&conf.conf_dmac[0], mc_dmac);
		break;
		
	case TYPE_L34UNICAST_W2G:
	case TYPE_DUALHDR_DSLITE_UNICAST_NAT_W2G:
		strtomac((void *)&conf.conf_dmac[0], wan_gmac);
		strtomac((void *)&conf.conf_smac[0], wan_nexthop_mac);
		conf.conf_dip	 = ntohl(inet_addr(wan_gip));
		conf.conf_dport = ext_port;
		conf.conf_sip	 = ntohl(inet_addr(wan_remote_host));
		conf.conf_sport = REMOTE_PORT;
		break;
		
	case TYPE_L34UNICAST_G2L:
		strtomac((void *)&conf.conf_dmac[0], lan_host_mac1);
		strtomac((void *)&conf.conf_smac[0], lan_gmac);
		conf.conf_dip	 = ntohl(inet_addr(lan_host_ip1));		
		conf.conf_dport = int_port; 		
		conf.conf_sip	 = ntohl(inet_addr(wan_remote_host));
		conf.conf_sport = REMOTE_PORT;
		//conf.ip.ttl--;
		break;
		
	case TYPE_L34MULTICAST_W2L:
		strtomac((void *)&conf.conf_smac[0], wan_nexthop_mac);
		conf.conf_dip	 = ntohl(inet_addr(mc_dip));
		conf.conf_sip	 =  ntohl(inet_addr(wan_remote_host));
		strtomac((void *)&conf.conf_dmac[0], mc_dmac);
		conf.conf_dport = int_port;
		conf.conf_sport = REMOTE_PORT;				
		break;
		
	case TYPE_L34MULTICAST_G2L:
		strtomac((void *)&conf.conf_smac[0], lan_gmac);
		conf.conf_dip	 = ntohl(inet_addr(mc_dip));
		conf.conf_sip	 =  ntohl(inet_addr(wan_remote_host));
		strtomac((void *)&conf.conf_dmac[0], mc_dmac);
		conf.conf_dport = int_port;
		conf.conf_sport = REMOTE_PORT;				
		break;
	case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W:	
	case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W:
	case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_FRAG:
	case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_FRAG:
	case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_FRAG2:	
	case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_FRAG2:
	case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_FRAG3:	
	case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_FRAG3:
		strtomac((void *)&conf.conf_smac[0], PPTP_CLIENT_MAC);
		strtomac((void *)&conf.conf_dmac[0], wan_nexthop_mac);
		conf.conf_sip	 = ntohl(inet_addr(lan_host_ip1));
		conf.conf_dip	 = ntohl(inet_addr(wan_remote_host));
		if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_FRAG||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_FRAG||
		   proto_type==TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_FRAG3)
		   conf.ip.mf = 1;
		if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_FRAG2||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_FRAG2||
		   proto_type==TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_FRAG3)
		   conf.ip.offset = 8;
		break;
	case TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W:
	case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W:
	case TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_FRAG:
	case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_FRAG:
	case TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_FRAG2:
	case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_FRAG2:
	case TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_FRAG3:
	case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_FRAG3:
		strtomac((void *)&conf.conf_smac[0], PPTP_CLIENT_MAC);
		strtomac((void *)&conf.conf_dmac[0], wan_nexthop_mac);
		conf.conf_sip	 = ntohl(inet_addr(wan_gip_2));
		conf.conf_dip	 = ntohl(inet_addr(wan_remote_host));
		conf.conf_sport = ext_port;
		if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_FRAG||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_FRAG||
		   proto_type==TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_FRAG3)
		   conf.ip.mf = 1;
		if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_FRAG2||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_FRAG2||
		   proto_type==TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_FRAG3)
		   conf.ip.offset = 8;
		break;
	case TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W:
	case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W:
	case TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_FRAG:
	case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_FRAG:
	case TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_FRAG2:
	case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_FRAG2:
	case TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_FRAG3:
	case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_FRAG3:
		conf.ipv6.pldlen 		= *pktLen + 20;
		conf.pktType	 		= _PKT_TYPE_V6TCP;
		conf.ipv6.version		= 6;
		conf.rhdr.hdrlen 		= 0;
		conf.ipv6.priority 		= dscp<<2|ecn;
		conf.ipv6.flowlbl[0] = 0x12;
		conf.ipv6.flowlbl[1] = 0x34;
		conf.ipv6.flowlbl[2] = 0x56;
		conf.ipv6.hoplmt 		= ttl;
		if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_FRAG||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_FRAG){
		   conf.extHdr = FraHdr;
		   conf.frghdr.fragment = 0x1;
		   conf.frghdr.id = 0x2379;
		}else if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_FRAG2||
				proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_FRAG2){
				conf.extHdr = FraHdr;
			   	conf.frghdr.fragment = 0x8;
			   	conf.frghdr.id = 0x2379;
		}else if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_FRAG3||
		 		proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_FRAG3){
		 		conf.extHdr = FraHdr;
				conf.frghdr.fragment = 0x9;
			   	conf.frghdr.id = 0x2379;
		 }
		in6_pton(v6_lan_host_ip,-1,(unsigned char *)&conf.conf_v6sip,-1,NULL);
		in6_pton(v6_wan_remote_host,-1,(unsigned char *)&conf.conf_v6dip,-1,NULL);
		strtomac((void *)&conf.conf_smac[0], PPTP_CLIENT_MAC);
		strtomac((void *)&conf.conf_dmac[0], wan_nexthop_mac);
		break;
	case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_UDP:
	case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_UDP:
	case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_UDP_FRAG:
	case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_UDP_FRAG:
	case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_UDP_FRAG2:
	case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_UDP_FRAG2:
	case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_UDP_FRAG3:
	case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_UDP_FRAG3:
		conf.pktType	 = _PKT_TYPE_UDP;
		strtomac((void *)&conf.conf_smac[0], PPTP_CLIENT_MAC);
		strtomac((void *)&conf.conf_dmac[0], wan_nexthop_mac);
		conf.conf_sip	 = ntohl(inet_addr(lan_host_ip1));
		conf.conf_dip	 = ntohl(inet_addr(wan_remote_host));
		if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_UDP_FRAG||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_UDP_FRAG||
		   proto_type==TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_UDP_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_UDP_FRAG3)
		   conf.ip.mf = 1;
		if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_UDP_FRAG2||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_UDP_FRAG2||
		   proto_type==TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_UDP_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_UDP_FRAG3)
		   conf.ip.offset = 8;
		break;
	case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_ICMP:
	case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_ICMP:
	case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_ICMP_FRAG:
	case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_ICMP_FRAG:
	case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_ICMP_FRAG2:
	case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_ICMP_FRAG2:
	case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_ICMP_FRAG3:
	case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_ICMP_FRAG3:
	case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_ICMPPOD_FRAG2:
	case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_ICMPPOD_FRAG2:
	case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_ICMPPOD_FRAG3:
	case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_ICMPPOD_FRAG3:
		conf.pktType	 = _PKT_TYPE_ICMP;
		strtomac((void *)&conf.conf_smac[0], PPTP_CLIENT_MAC);
		strtomac((void *)&conf.conf_dmac[0], wan_nexthop_mac);
		conf.conf_sip	 = ntohl(inet_addr(lan_host_ip1));
		conf.conf_dip	 = ntohl(inet_addr(wan_remote_host));
		if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_ICMP_FRAG||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_ICMP_FRAG||
		   proto_type==TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_ICMP_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_ICMP_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_ICMPPOD_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_ICMPPOD_FRAG3)
		   conf.ip.mf = 1;
		if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_ICMP_FRAG2||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_ICMP_FRAG2||
		   proto_type==TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_ICMP_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_ICMP_FRAG3)
		   conf.ip.offset = 8;
	  	if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_ICMPPOD_FRAG2||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_ICMPPOD_FRAG2||
		   proto_type==TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_ICMPPOD_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_ICMPPOD_FRAG3)
		   conf.ip.offset = 0xfff8;		//for ping of death
		break;
	case TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_UDP:
	case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_UDP:
	case TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_UDP_FRAG:
	case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_UDP_FRAG:
	case TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_UDP_FRAG2:
	case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_UDP_FRAG2:
	case TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_UDP_FRAG3:
	case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_UDP_FRAG3:
		conf.pktType	 = _PKT_TYPE_UDP;
		strtomac((void *)&conf.conf_smac[0], PPTP_CLIENT_MAC);
		strtomac((void *)&conf.conf_dmac[0], wan_nexthop_mac);
		conf.conf_sip	 = ntohl(inet_addr(wan_gip_2));
		conf.conf_dip	 = ntohl(inet_addr(wan_remote_host));
		conf.conf_sport = ext_port;
		if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_UDP_FRAG||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_UDP_FRAG||
		   proto_type==TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_UDP_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_UDP_FRAG3)
		   conf.ip.mf = 1;
		if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_UDP_FRAG2||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_UDP_FRAG2||
		   proto_type==TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_UDP_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_UDP_FRAG3)
		   conf.ip.offset = 8;
		break;
	case TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_ICMP:
	case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_ICMP:
	case TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_ICMP_FRAG:
	case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_ICMP_FRAG:
	case TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_ICMP_FRAG2:
	case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_ICMP_FRAG2:
	case TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_ICMP_FRAG3:
	case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_ICMP_FRAG3:
	case TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_ICMPPOD_FRAG2:
	case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_ICMPPOD_FRAG2:
	case TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_ICMPPOD_FRAG3:
	case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_ICMPPOD_FRAG3:
		conf.pktType	 = _PKT_TYPE_ICMP;
		strtomac((void *)&conf.conf_smac[0], PPTP_CLIENT_MAC);
		strtomac((void *)&conf.conf_dmac[0], wan_nexthop_mac);
		conf.conf_sip	 = ntohl(inet_addr(wan_gip_2));
		conf.conf_dip	 = ntohl(inet_addr(wan_remote_host));
		conf.conf_sport = ext_port;
		if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_ICMP_FRAG||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_ICMP_FRAG||
		   proto_type==TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_ICMP_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_ICMP_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_ICMPPOD_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_ICMPPOD_FRAG3)
		   conf.ip.mf = 1;
		if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_ICMP_FRAG2||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_ICMP_FRAG2||
		   proto_type==TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_ICMP_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_ICMP_FRAG3)
		   conf.ip.offset = 8;
	   if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_ICMPPOD_FRAG2||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_ICMPPOD_FRAG2||
		   proto_type==TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_ICMPPOD_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_ICMPPOD_FRAG3)
		   conf.ip.offset = 0xfff8;		//for ping of death
		break;
	case TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_UDP:
	case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_UDP:
	case TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_UDP_FRAG:
	case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_UDP_FRAG:
	case TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_UDP_FRAG2:
	case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_UDP_FRAG2:
	case TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_UDP_FRAG3:
	case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_UDP_FRAG3:
		conf.ipv6.pldlen 		= *pktLen + 20;
		conf.pktType	 		= _PKT_TYPE_V6UDP;
		conf.ipv6.version		= 6;
		conf.rhdr.hdrlen 		= 0;
		conf.ipv6.priority 		= dscp<<2|ecn;
		conf.ipv6.flowlbl[0] = 0x12;
		conf.ipv6.flowlbl[1] = 0x34;
		conf.ipv6.flowlbl[2] = 0x56;
		conf.ipv6.hoplmt 		= ttl;
		if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_UDP_FRAG||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_UDP_FRAG){
		   conf.extHdr = FraHdr;
		   conf.frghdr.fragment = 0x1;
		   conf.frghdr.id = 0x2379;
		}else if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_UDP_FRAG2||
				proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_UDP_FRAG2){
				conf.extHdr = FraHdr;
			   	conf.frghdr.fragment = 0x8;
			   	conf.frghdr.id = 0x2379;
		}else if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_UDP_FRAG3||
		 		proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_UDP_FRAG3){
		 		conf.extHdr = FraHdr;
				conf.frghdr.fragment = 0x9;
			   	conf.frghdr.id = 0x2379;
		 }
		in6_pton(v6_lan_host_ip,-1,(unsigned char *)&conf.conf_v6sip,-1,NULL);
		in6_pton(v6_wan_remote_host,-1,(unsigned char *)&conf.conf_v6dip,-1,NULL);
		strtomac((void *)&conf.conf_smac[0], PPTP_CLIENT_MAC);
		strtomac((void *)&conf.conf_dmac[0], wan_nexthop_mac);
		break;
	case TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_ICMP:
	case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_ICMP:
	case TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_ICMP_FRAG:
	case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_ICMP_FRAG:
	case TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_ICMP_FRAG2:
	case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_ICMP_FRAG2:
	case TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_ICMP_FRAG3:
	case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_ICMP_FRAG3:
	case TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_ICMPPOD_FRAG2:
	case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_ICMPPOD_FRAG2:
	case TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_ICMPPOD_FRAG3:
	case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_ICMPPOD_FRAG3:
		conf.ipv6.pldlen 		= *pktLen + 20;
		conf.pktType	 		= _PKT_TYPE_V6ICMP;
		conf.ipv6.version		= 6;
		conf.rhdr.hdrlen 		= 0;
		conf.ipv6.priority 		= dscp<<2|ecn;
		conf.ipv6.flowlbl[0] = 0x12;
		conf.ipv6.flowlbl[1] = 0x34;
		conf.ipv6.flowlbl[2] = 0x56;
		conf.ipv6.hoplmt 		= ttl;
		if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_ICMP_FRAG||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_ICMP_FRAG){
		   conf.extHdr = FraHdr;
		   conf.frghdr.fragment = 0x1;
		   conf.frghdr.id = 0x2379;
		}else if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_ICMP_FRAG2||
				proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_ICMP_FRAG2){
				conf.extHdr = FraHdr;
			   	conf.frghdr.fragment = 0x8;			//for ping of death
			   	conf.frghdr.id = 0x2379;
		}else if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_ICMPPOD_FRAG2||
				proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_ICMPPOD_FRAG2){
				conf.extHdr = FraHdr;
			   	conf.frghdr.fragment = (0x1ffff<<3)&0xff;			//for ping of death
			   	conf.frghdr.id = 0x2379;
		}else if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_ICMP_FRAG3||
		 		proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_ICMP_FRAG3){
		 		conf.extHdr = FraHdr;
				conf.frghdr.fragment = 0x9;
			   	conf.frghdr.id = 0x2379;
		}else if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_ICMPPOD_FRAG3||
		 		proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_ICMPPOD_FRAG3){
		 		conf.extHdr = FraHdr;
				conf.frghdr.fragment = ((0x1ffff<<3) | 1) &0xff;		//for ping of death
			   	conf.frghdr.id = 0x2379;
		 }
		in6_pton(v6_lan_host_ip,-1,(unsigned char *)&conf.conf_v6sip,-1,NULL);
		in6_pton(v6_wan_remote_host,-1,(unsigned char *)&conf.conf_v6dip,-1,NULL);
		strtomac((void *)&conf.conf_smac[0], PPTP_CLIENT_MAC);
		strtomac((void *)&conf.conf_dmac[0], wan_nexthop_mac);
		break;
	case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L:
	case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L:
	case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_FRAG:
	case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_FRAG:
	case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_FRAG2:
	case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_FRAG2:
	case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_FRAG3:
	case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_FRAG3:
		strtomac((void *)&conf.conf_smac[0], wan_nexthop_mac);
		strtomac((void *)&conf.conf_dmac[0], PPTP_CLIENT_MAC);
		conf.conf_sip	 = ntohl(inet_addr(wan_remote_host));
		conf.conf_dip	 = ntohl(inet_addr(lan_host_ip1));
		conf.conf_sport = REMOTE_PORT;
		conf.conf_dport = int_port;
		if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_FRAG||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_FRAG||
		   proto_type==TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_FRAG3)
		   conf.ip.mf = 1;
		if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_FRAG2||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_FRAG2||
		   proto_type==TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_FRAG3)
		   conf.ip.offset = 8;
		break;
	case TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G:
	case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G:
	case TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_FRAG:
	case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_FRAG:
	case TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_FRAG2:
	case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_FRAG2:
	case TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_FRAG3:
	case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_FRAG3:
		strtomac((void *)&conf.conf_smac[0], wan_nexthop_mac);
		strtomac((void *)&conf.conf_dmac[0], PPTP_CLIENT_MAC);
		conf.conf_sip	 = ntohl(inet_addr(wan_remote_host));
		conf.conf_dip	 = ntohl(inet_addr(wan_gip_2));
		conf.conf_sport = REMOTE_PORT;
		conf.conf_dport = ext_port;
		if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_FRAG||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_FRAG||
		   proto_type==TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_FRAG3)
		   conf.ip.mf = 1;
		if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_FRAG2||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_FRAG2||
		   proto_type==TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_FRAG3)
		   conf.ip.offset = 8;
		break;
	case TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L:
	case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L:
	case TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_FRAG:
	case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_FRAG:
	case TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_FRAG2:
	case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_FRAG2:
	case TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_FRAG3:
	case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_FRAG3:
		conf.ipv6.pldlen 		= *pktLen + 20;
		conf.pktType	 		= _PKT_TYPE_V6TCP;
		conf.ipv6.version		= 6;
		conf.rhdr.hdrlen 		= 0;
		conf.ipv6.priority 		= dscp<<2|ecn;
		conf.ipv6.flowlbl[0] = 0x12;
		conf.ipv6.flowlbl[1] = 0x34;
		conf.ipv6.flowlbl[2] = 0x56;
		conf.ipv6.hoplmt 		= ttl;
		
		if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_FRAG||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_FRAG){
		   conf.extHdr = FraHdr;
		   conf.frghdr.fragment = 0x1;
		   conf.frghdr.id = 0x2379;
		}else if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_FRAG2||
				proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_FRAG2){
				conf.extHdr = FraHdr;
			   	conf.frghdr.fragment = 0x8;
			   	conf.frghdr.id = 0x2379;
		}else if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_FRAG3||
		 		proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_FRAG3){
		 		conf.extHdr = FraHdr;
				conf.frghdr.fragment = 0x9;
			   	conf.frghdr.id = 0x2379;
		}
		in6_pton(v6_wan_remote_host,-1,(unsigned char *)&conf.conf_v6sip,-1,NULL);
		in6_pton(v6_lan_host_ip,-1,(unsigned char *)&conf.conf_v6dip,-1,NULL);
		strtomac((void *)&conf.conf_smac[0], wan_nexthop_mac);
		strtomac((void *)&conf.conf_dmac[0], PPTP_CLIENT_MAC);
		conf.conf_sport  = REMOTE_PORT;
		conf.conf_dport  = int_port;
		break;
	case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_UDP:
	case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_UDP:
	case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_UDP_FRAG:
	case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_UDP_FRAG:
	case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_UDP_FRAG2:
	case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_UDP_FRAG2:
	case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_UDP_FRAG3:
	case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_UDP_FRAG3:
		conf.pktType	 = _PKT_TYPE_UDP;
		strtomac((void *)&conf.conf_smac[0], wan_nexthop_mac);
		strtomac((void *)&conf.conf_dmac[0], PPTP_CLIENT_MAC);
		conf.conf_sip	 = ntohl(inet_addr(wan_remote_host));
		conf.conf_dip	 = ntohl(inet_addr(lan_host_ip1));
		conf.conf_sport = REMOTE_PORT;
		conf.conf_dport = int_port;
		if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_UDP_FRAG||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_UDP_FRAG||
		   proto_type==TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_UDP_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_UDP_FRAG3)
		   conf.ip.mf = 1;
		if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_UDP_FRAG2||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_UDP_FRAG2||
		   proto_type==TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_UDP_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_UDP_FRAG3)
		   conf.ip.offset = 8;
		break;
	case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_ICMP:
	case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_ICMP:
	case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_ICMP_FRAG:
	case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_ICMP_FRAG:
	case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_ICMP_FRAG2:
	case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_ICMP_FRAG2:
	case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_ICMP_FRAG3:
	case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_ICMP_FRAG3:
	case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_ICMPPOD_FRAG2:
	case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_ICMPPOD_FRAG2:
	case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_ICMPPOD_FRAG3:
	case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_ICMPPOD_FRAG3:
		conf.pktType	 = _PKT_TYPE_ICMP;
		strtomac((void *)&conf.conf_smac[0], wan_nexthop_mac);
		strtomac((void *)&conf.conf_dmac[0], PPTP_CLIENT_MAC);
		conf.conf_sip	 = ntohl(inet_addr(wan_remote_host));
		conf.conf_dip	 = ntohl(inet_addr(lan_host_ip1));
		conf.conf_sport = REMOTE_PORT;
		conf.conf_dport = int_port;
		if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_ICMP_FRAG||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_ICMP_FRAG||
		   proto_type==TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_ICMP_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_ICMP_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_ICMPPOD_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_ICMPPOD_FRAG3)
		   conf.ip.mf = 1;
		if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_ICMP_FRAG2||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_ICMP_FRAG2||
		   proto_type==TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_ICMP_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_ICMP_FRAG3)
		   conf.ip.offset = 8;
		if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_ICMPPOD_FRAG2||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_ICMPPOD_FRAG2||
		   proto_type==TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_ICMPPOD_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_ICMPPOD_FRAG3)
		   conf.ip.offset = 0xfff8;		//for ping of death
		break;
	case TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_UDP:
	case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_UDP:
	case TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_UDP_FRAG:
	case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_UDP_FRAG:
	case TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_UDP_FRAG2:
	case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_UDP_FRAG2:
	case TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_UDP_FRAG3:
	case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_UDP_FRAG3:
		conf.pktType	 = _PKT_TYPE_UDP;
		strtomac((void *)&conf.conf_smac[0], wan_nexthop_mac);
		strtomac((void *)&conf.conf_dmac[0], PPTP_CLIENT_MAC);
		conf.conf_sip	 = ntohl(inet_addr(wan_remote_host));
		conf.conf_dip	 = ntohl(inet_addr(wan_gip_2));
		conf.conf_sport = REMOTE_PORT;
		conf.conf_dport = ext_port;
		if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_UDP_FRAG||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_UDP_FRAG||
		   proto_type==TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_UDP_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_UDP_FRAG3)
		   conf.ip.mf = 1;
		if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_UDP_FRAG2||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_UDP_FRAG2||
		   proto_type==TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_UDP_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_UDP_FRAG3)
		   conf.ip.offset = 8;
		break;
	case TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_ICMP:
	case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_ICMP:
	case TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_ICMP_FRAG:
	case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_ICMP_FRAG:
	case TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_ICMP_FRAG2:
	case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_ICMP_FRAG2:
	case TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_ICMP_FRAG3:
	case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_ICMP_FRAG3:
	case TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_ICMPPOD_FRAG2:
	case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_ICMPPOD_FRAG2:
	case TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_ICMPPOD_FRAG3:
	case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_ICMPPOD_FRAG3:
		conf.pktType	 = _PKT_TYPE_ICMP;
		strtomac((void *)&conf.conf_smac[0], wan_nexthop_mac);
		strtomac((void *)&conf.conf_dmac[0], PPTP_CLIENT_MAC);
		conf.conf_sip	 = ntohl(inet_addr(wan_remote_host));
		conf.conf_dip	 = ntohl(inet_addr(wan_gip_2));
		conf.conf_sport = REMOTE_PORT;
		conf.conf_dport = ext_port;
		if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_ICMP_FRAG||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_ICMP_FRAG||
		   proto_type==TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_ICMP_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_ICMP_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_ICMPPOD_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_ICMPPOD_FRAG3)
		   conf.ip.mf = 1;
		if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_ICMP_FRAG2||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_ICMP_FRAG2||
		   proto_type==TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_ICMP_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_ICMP_FRAG3)
		   conf.ip.offset = 8;
		if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_ICMPPOD_FRAG2||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_ICMPPOD_FRAG2||
		   proto_type==TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_ICMPPOD_FRAG3||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_ICMPPOD_FRAG3)
		   conf.ip.offset = 0xfff8;		//for ping of death
		break;
	case TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_UDP:
	case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_UDP:
	case TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_UDP_FRAG:
	case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_UDP_FRAG:
	case TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_UDP_FRAG2:
	case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_UDP_FRAG2:
	case TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_UDP_FRAG3:
	case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_UDP_FRAG3:
		conf.ipv6.pldlen 		= *pktLen + 20;
		conf.pktType	 		= _PKT_TYPE_V6UDP;
		conf.ipv6.version		= 6;
		conf.rhdr.hdrlen 		= 0;
		conf.ipv6.priority 		= dscp<<2|ecn;
		conf.ipv6.flowlbl[0] = 0x12;
		conf.ipv6.flowlbl[1] = 0x34;
		conf.ipv6.flowlbl[2] = 0x56;
		conf.ipv6.hoplmt 		= ttl;
		if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_UDP_FRAG||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_UDP_FRAG){
		   conf.extHdr = FraHdr;
		   conf.frghdr.fragment = 0x1;
		   conf.frghdr.id = 0x2379;
		}else if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_UDP_FRAG2||
				proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_UDP_FRAG2){
				conf.extHdr = FraHdr;
			   	conf.frghdr.fragment = 0x8;
			   	conf.frghdr.id = 0x2379;
		}else if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_UDP_FRAG3||
		 		proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_UDP_FRAG3){
		 		conf.extHdr = FraHdr;
				conf.frghdr.fragment = 0x9;
			   	conf.frghdr.id = 0x2379;
		}
		in6_pton(v6_wan_remote_host,-1,(unsigned char *)&conf.conf_v6sip,-1,NULL);
		in6_pton(v6_lan_host_ip,-1,(unsigned char *)&conf.conf_v6dip,-1,NULL);
		strtomac((void *)&conf.conf_smac[0], wan_nexthop_mac);
		strtomac((void *)&conf.conf_dmac[0], PPTP_CLIENT_MAC);
		conf.conf_sport  = REMOTE_PORT;
		conf.conf_dport  = int_port;
		break;
	case TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_ICMP:
	case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_ICMP:
	case TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_ICMP_FRAG:
	case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_ICMP_FRAG:
	case TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_ICMP_FRAG2:
	case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_ICMP_FRAG2:
	case TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_ICMP_FRAG3:
	case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_ICMP_FRAG3:
	case TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_ICMPPOD_FRAG2:
	case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_ICMPPOD_FRAG2:
	case TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_ICMPPOD_FRAG3:
	case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_ICMPPOD_FRAG3:
		conf.ipv6.pldlen 		= *pktLen + 20;
		conf.pktType	 		= _PKT_TYPE_V6ICMP;
		conf.ipv6.version		= 6;
		conf.rhdr.hdrlen 		= 0;
		conf.ipv6.priority 		= dscp<<2|ecn;
		conf.ipv6.flowlbl[0] = 0x12;
		conf.ipv6.flowlbl[1] = 0x34;
		conf.ipv6.flowlbl[2] = 0x56;
		conf.ipv6.hoplmt 		= ttl;
		if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_ICMP_FRAG||
		   proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_ICMP_FRAG){
		   conf.extHdr = FraHdr;
		   conf.frghdr.fragment = 0x1;
		   conf.frghdr.id = 0x2379;
		}else if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_ICMP_FRAG2||
				proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_ICMP_FRAG2){
				conf.extHdr = FraHdr;
			   	conf.frghdr.fragment = 0x8;
			   	conf.frghdr.id = 0x2379;
		}else if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_ICMPPOD_FRAG2||
				proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_ICMPPOD_FRAG2){
				conf.extHdr = FraHdr;
			   	conf.frghdr.fragment = (0x1ffff<<3)&0xff;			//for ping of death
			   	conf.frghdr.id = 0x2379;
		}else if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_ICMP_FRAG3||
		 		proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_ICMP_FRAG3){
		 		conf.extHdr = FraHdr;
				conf.frghdr.fragment = 0x9;
			   	conf.frghdr.id = 0x2379;
		}else if(proto_type==TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_ICMPPOD_FRAG3||
		 		proto_type==TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_ICMPPOD_FRAG3){
		 		conf.extHdr = FraHdr;
				conf.frghdr.fragment = ((0x1ffff<<3) | 1)&0xff;			//for ping of death
			   	conf.frghdr.id = 0x2379;
		}
		in6_pton(v6_wan_remote_host,-1,(unsigned char *)&conf.conf_v6sip,-1,NULL);
		in6_pton(v6_lan_host_ip,-1,(unsigned char *)&conf.conf_v6dip,-1,NULL);
		strtomac((void *)&conf.conf_smac[0], wan_nexthop_mac);
		strtomac((void *)&conf.conf_dmac[0], PPTP_CLIENT_MAC);
		conf.conf_sport  = REMOTE_PORT;
		conf.conf_dport  = int_port;
		break;
	case TYPE_DUALHDR_PPTP_MULTICAST_ROUTING_G2W:
		strtomac((void *)&conf.conf_smac[0], PPTP_CLIENT_MAC);
		strtomac((void *)&conf.conf_dmac[0], wan_nexthop_mac);
		conf.conf_sip	 = ntohl(inet_addr(lan_host_ip1));
		conf.conf_dip	 = ntohl(inet_addr(mc_dip));
		break;
	case TYPE_DUALHDR_PPTP_MULTICAST_IPV6_G2W:
		conf.ipv6.pldlen 		= *pktLen + 20;
		conf.pktType	 = _PKT_TYPE_V6UDP;
		conf.ipv6.version		= 6;
		conf.rhdr.hdrlen 		= 0;
		conf.ipv6.priority 		= dscp<<2|ecn;
		conf.ipv6.flowlbl[0] = 0x12;
		conf.ipv6.flowlbl[1] = 0x34;
		conf.ipv6.flowlbl[2] = 0x56;
		conf.ipv6.hoplmt 		= ttl;
		in6_pton(v6_common_src_ip,-1,(unsigned char *)&conf.conf_v6sip,-1,NULL);
		in6_pton(v6mc_dip,-1,(unsigned char *)&conf.conf_v6dip,-1,NULL);
		strtomac((void *)&conf.conf_smac[0], PPTP_CLIENT_MAC);
		strtomac((void *)&conf.conf_dmac[0], wan_nexthop_mac);
		break;
	case TYPE_DUALHDR_PPTP_MULTICAST_ROUTING_W2G:
		strtomac((void *)&conf.conf_smac[0], wan_nexthop_mac);
		strtomac((void *)&conf.conf_dmac[0], PPTP_CLIENT_MAC);
		conf.conf_sip	 = ntohl(inet_addr(wan_remote_host));
		conf.conf_dip	 = ntohl(inet_addr(mc_dip));
		conf.conf_sport = REMOTE_PORT;
		conf.conf_dport = int_port;
		break;
	case TYPE_DUALHDR_PPTP_MULTICAST_IPV6_W2G:
		conf.ipv6.pldlen 		= *pktLen + 20;
		conf.pktType	 = _PKT_TYPE_V6UDP;
		conf.ipv6.version		= 6;
		conf.rhdr.hdrlen 		= 0;
		conf.ipv6.priority 		= dscp<<2|ecn;
		conf.ipv6.flowlbl[0] = 0x12;
		conf.ipv6.flowlbl[1] = 0x34;
		conf.ipv6.flowlbl[2] = 0x56;
		conf.ipv6.hoplmt 		= ttl;
		in6_pton(v6_wan_remote_host,-1,(unsigned char *)&conf.conf_v6sip,-1,NULL);
		in6_pton(v6mc_dip,-1,(unsigned char *)&conf.conf_v6dip,-1,NULL);
		strtomac((void *)&conf.conf_smac[0], wan_nexthop_mac);
		strtomac((void *)&conf.conf_dmac[0], PPTP_CLIENT_MAC);
		conf.conf_sport = REMOTE_PORT;
		conf.conf_dport = int_port;
		break;	
	case TYPE_DUALHDR_PPTPV6:
		conf.ipv6.pldlen 		= *pktLen + 20;
		conf.pktType	 		= _PKT_TYPE_IPV6;
		conf.ipv6.version		= 6;
		conf.rhdr.hdrlen 		= 0;
		conf.ipv6.priority 		= dscp<<2|ecn;
		conf.ipv6.flowlbl[0] 	= 0x12;
		conf.ipv6.flowlbl[1] 	= 0x34;
		conf.ipv6.flowlbl[2] 	= 0x56;
		conf.ipv6.hoplmt 		= ttl;
		conf.ipv6.nxthdr		= 0x2f;	//47
		in6_pton(v6_wan_remote_host,-1,(unsigned char *)&conf.conf_v6sip,-1,NULL);
		in6_pton(v6_wan_gip,-1,(unsigned char *)&conf.conf_v6dip,-1,NULL);
		strtomac((void *)&conf.conf_smac[0], wan_nexthop_mac);
		strtomac((void *)&conf.conf_dmac[0], PPTP_CLIENT_MAC);
		conf.conf_sport = REMOTE_PORT; 	
		conf.conf_dport = int_port; 
		break;
	case TYPE_DUALHDR_L2TP_UNICAST_ROUTING_G2W:
		strtomac((void *)&conf.conf_smac[0], L2TP_CLIENT_MAC);
		strtomac((void *)&conf.conf_dmac[0], wan_nexthop_mac);
		conf.conf_sip	 = ntohl(inet_addr(lan_host_ip1));
		conf.conf_dip	 = ntohl(inet_addr(wan_remote_host));
		break;
	case TYPE_DUALHDR_L2TP_UNICAST_NAT_G2W:
		strtomac((void *)&conf.conf_smac[0], L2TP_CLIENT_MAC);
		strtomac((void *)&conf.conf_dmac[0], wan_nexthop_mac);
		conf.conf_sip	 = ntohl(inet_addr(wan_gip_2));
		conf.conf_dip	 = ntohl(inet_addr(wan_remote_host));
		conf.conf_sport = ext_port;
		break;
	case TYPE_DUALHDR_L2TP_UNICAST_IPV6_G2W:
		conf.ipv6.pldlen 		= *pktLen + 20;
		conf.pktType	 		= _PKT_TYPE_V6TCP;
		conf.ipv6.version		= 6;
		conf.rhdr.hdrlen 		= 0;
		conf.ipv6.priority 		= dscp<<2|ecn;
		conf.ipv6.flowlbl[0] = 0x12;
		conf.ipv6.flowlbl[1] = 0x34;
		conf.ipv6.flowlbl[2] = 0x56;
		conf.ipv6.hoplmt 		= ttl;
		in6_pton(v6_lan_host_ip,-1,(unsigned char *)&conf.conf_v6sip,-1,NULL);
		in6_pton(v6_wan_remote_host,-1,(unsigned char *)&conf.conf_v6dip,-1,NULL);
		strtomac((void *)&conf.conf_smac[0], L2TP_CLIENT_MAC);
		strtomac((void *)&conf.conf_dmac[0], wan_nexthop_mac);
		break;
	case TYPE_DUALHDR_L2TP_UNICAST_ROUTING_W2L:
		strtomac((void *)&conf.conf_smac[0], wan_nexthop_mac);
		strtomac((void *)&conf.conf_dmac[0], L2TP_CLIENT_MAC);
		conf.conf_sip	 = ntohl(inet_addr(wan_remote_host));
		conf.conf_dip	 = ntohl(inet_addr(lan_host_ip1));
		conf.conf_sport = REMOTE_PORT;
		conf.conf_dport = int_port;
		break;
	case TYPE_DUALHDR_L2TP_UNICAST_NAT_W2G:
		strtomac((void *)&conf.conf_smac[0], wan_nexthop_mac);
		strtomac((void *)&conf.conf_dmac[0], L2TP_CLIENT_MAC);
		conf.conf_sip	 = ntohl(inet_addr(wan_remote_host));
		conf.conf_dip	 = ntohl(inet_addr(wan_gip_2));
		conf.conf_sport = REMOTE_PORT;
		conf.conf_dport = ext_port;
		break;
	case TYPE_DUALHDR_L2TP_UNICAST_IPV6_W2L:
		conf.ipv6.pldlen 		= *pktLen + 20;
		conf.pktType	 		= _PKT_TYPE_V6TCP;
		conf.ipv6.version		= 6;
		conf.rhdr.hdrlen 		= 0;
		conf.ipv6.priority 		= dscp<<2|ecn;
		conf.ipv6.flowlbl[0] = 0x12;
		conf.ipv6.flowlbl[1] = 0x34;
		conf.ipv6.flowlbl[2] = 0x56;
		conf.ipv6.hoplmt 		= ttl;
		in6_pton(v6_wan_remote_host,-1,(unsigned char *)&conf.conf_v6sip,-1,NULL);
		in6_pton(v6_lan_host_ip,-1,(unsigned char *)&conf.conf_v6dip,-1,NULL);
		strtomac((void *)&conf.conf_smac[0], wan_nexthop_mac);
		strtomac((void *)&conf.conf_dmac[0], L2TP_CLIENT_MAC);
		conf.conf_sport  = REMOTE_PORT;
		conf.conf_dport  = int_port;
		break;
	case TYPE_DUALHDR_L2TP_MULTICAST_ROUTING_G2W:
		strtomac((void *)&conf.conf_smac[0], L2TP_CLIENT_MAC);
		strtomac((void *)&conf.conf_dmac[0], wan_nexthop_mac);
		conf.conf_sip	 = ntohl(inet_addr(lan_host_ip1));
		conf.conf_dip	 = ntohl(inet_addr(mc_dip));
		break;			
	case TYPE_DUALHDR_L2TP_MULTICAST_IPV6_G2W:
		conf.ipv6.pldlen 		= *pktLen + 20;
		conf.pktType	 = _PKT_TYPE_V6UDP;
		conf.ipv6.version		= 6;
		conf.rhdr.hdrlen 		= 0;
		conf.ipv6.priority 		= dscp<<2|ecn;
		conf.ipv6.flowlbl[0] = 0x12;
		conf.ipv6.flowlbl[1] = 0x34;
		conf.ipv6.flowlbl[2] = 0x56;
		conf.ipv6.hoplmt 		= ttl;
		in6_pton(v6_common_src_ip,-1,(unsigned char *)&conf.conf_v6sip,-1,NULL);
		in6_pton(v6mc_dip,-1,(unsigned char *)&conf.conf_v6dip,-1,NULL);
		strtomac((void *)&conf.conf_smac[0], L2TP_CLIENT_MAC);
		strtomac((void *)&conf.conf_dmac[0], wan_nexthop_mac);
		break;		
	case TYPE_DUALHDR_L2TP_MULTICAST_ROUTING_W2G:
		strtomac((void *)&conf.conf_smac[0], wan_nexthop_mac);
		strtomac((void *)&conf.conf_dmac[0], L2TP_CLIENT_MAC);
		conf.conf_sip	 = ntohl(inet_addr(wan_remote_host));
		conf.conf_dip	 = ntohl(inet_addr(mc_dip));
		conf.conf_sport = REMOTE_PORT;
		conf.conf_dport = int_port;
		break;
	case TYPE_DUALHDR_L2TP_MULTICAST_IPV6_W2G:
		conf.ipv6.pldlen 		= *pktLen + 20;
		conf.pktType	 = _PKT_TYPE_V6UDP;
		conf.ipv6.version		= 6;
		conf.rhdr.hdrlen 		= 0;
		conf.ipv6.priority 		= dscp<<2|ecn;
		conf.ipv6.flowlbl[0] = 0x12;
		conf.ipv6.flowlbl[1] = 0x34;
		conf.ipv6.flowlbl[2] = 0x56;
		conf.ipv6.hoplmt 		= ttl;
		in6_pton(v6_wan_remote_host,-1,(unsigned char *)&conf.conf_v6sip,-1,NULL);
		in6_pton(v6mc_dip,-1,(unsigned char *)&conf.conf_v6dip,-1,NULL);
		strtomac((void *)&conf.conf_smac[0], wan_nexthop_mac);
		strtomac((void *)&conf.conf_dmac[0], L2TP_CLIENT_MAC);
		conf.conf_sport = REMOTE_PORT;
		conf.conf_dport = int_port;
		break;
	case TYPE_DUALHDR_L2TPV6:
		conf.ipv6.pldlen 		= *pktLen + 20;
		conf.pktType	 		= _PKT_TYPE_IPV6;
		conf.ipv6.version		= 6;
		conf.rhdr.hdrlen 		= 0;
		conf.ipv6.priority 		= dscp<<2|ecn;
		conf.ipv6.flowlbl[0] 	= 0x12;
		conf.ipv6.flowlbl[1] 	= 0x34;
		conf.ipv6.flowlbl[2] 	= 0x56;
		conf.ipv6.hoplmt 		= ttl;
		conf.ipv6.nxthdr		= 0x11;	//17
		in6_pton(v6_wan_remote_host,-1,(unsigned char *)&conf.conf_v6sip,-1,NULL);
		in6_pton(v6_wan_gip,-1,(unsigned char *)&conf.conf_v6dip,-1,NULL);
		strtomac((void *)&conf.conf_smac[0], wan_nexthop_mac);
		strtomac((void *)&conf.conf_dmac[0], L2TP_CLIENT_MAC);
		conf.conf_sport = REMOTE_PORT; 	
		conf.conf_dport = int_port; 
		break;
	case TYPE_DUALHDR_DSLITE_UNICAST_IPV6_OUTER_G2W:
		
		conf.l2Flag 		= conf.l2Flag | DUAL_OUTER_HDR;
		
		conf.ipv6.pldlen 		= *pktLen + 20;
		conf.pktType	 = _PKT_TYPE_IPV6;
		conf.ipv6.version		= 6;
		conf.rhdr.hdrlen 		= 0;
		conf.ipv6.priority 		= dscp<<2|ecn;
		conf.ipv6.flowlbl[0] = 0x0;
		conf.ipv6.flowlbl[1] = 0x0;
		conf.ipv6.flowlbl[2] = 0x0;
		conf.ipv6.hoplmt 		= ttl;
		conf.ipv6.nxthdr		= 4; //IPIP(4)
		in6_pton(v6_wan_gip,-1,(unsigned char *)&conf.conf_v6sip,-1,NULL);
		in6_pton(v6_wan_remote_host,-1,(unsigned char *)&conf.conf_v6dip,-1,NULL);
		strtomac((void *)&conf.conf_smac[0], wan_gmac);
		strtomac((void *)&conf.conf_dmac[0], wan_nexthop_mac);
		conf.conf_sport = int_port; 	
		conf.conf_dport = REMOTE_PORT; 	
		break;
	case TYPE_DUALHDR_DSLITE_UNICAST_IPV6_OUTER_W2G:
		
		conf.l2Flag 		= conf.l2Flag | DUAL_OUTER_HDR;
		
		conf.ipv6.pldlen 		= *pktLen + 20;
		conf.pktType	 = _PKT_TYPE_IPV6;
		conf.ipv6.version		= 6;
		conf.rhdr.hdrlen 		= 0;
		conf.ipv6.priority 		= dscp<<2|ecn;
		conf.ipv6.flowlbl[0] = 0x0;
		conf.ipv6.flowlbl[1] = 0x0;
		conf.ipv6.flowlbl[2] = 0x0;
		conf.ipv6.hoplmt 		= ttl;
		conf.ipv6.nxthdr		= 4; //IPIP(4)
		in6_pton(v6_wan_remote_host,-1,(unsigned char *)&conf.conf_v6sip,-1,NULL);
		in6_pton(v6_wan_gip,-1,(unsigned char *)&conf.conf_v6dip,-1,NULL);
		strtomac((void *)&conf.conf_smac[0], wan_nexthop_mac);
		strtomac((void *)&conf.conf_dmac[0], wan_gmac);
		conf.conf_sport = int_port; 	
		conf.conf_dport = REMOTE_PORT; 	
		break;
	case TYPE_DUALHDR_DSLITE_MULTICAST_IPV6_OUTER_G2W:
		
		conf.l2Flag 		= conf.l2Flag | DUAL_OUTER_HDR;
		
		conf.ipv6.pldlen 		= *pktLen + 20;
		conf.pktType	 = _PKT_TYPE_IPV6;
		conf.ipv6.version		= 6;
		conf.rhdr.hdrlen 		= 0;
		conf.ipv6.priority 		= dscp<<2|ecn;
		conf.ipv6.flowlbl[0] = 0x0;
		conf.ipv6.flowlbl[1] = 0x0;
		conf.ipv6.flowlbl[2] = 0x0;
		conf.ipv6.hoplmt 		= ttl;
		conf.ipv6.nxthdr		= 4; //IPIP(4)
		in6_pton(v6_wan_gip,-1,(unsigned char *)&conf.conf_v6sip,-1,NULL);
		in6_pton(v6mc_dip,-1,(unsigned char *)&conf.conf_v6dip,-1,NULL);
		strtomac((void *)&conf.conf_smac[0], wan_gmac);
		strtomac((void *)&conf.conf_dmac[0], v6mc_dmac);
		conf.conf_sport = int_port; 	
		conf.conf_dport = REMOTE_PORT; 	
		break;
	case TYPE_DUALHDR_DSLITE_MULTICAST_IPV6_OUTER_W2G:
		
		conf.l2Flag 		= conf.l2Flag | DUAL_OUTER_HDR;
		
		conf.ipv6.pldlen 		= *pktLen + 20;
		conf.pktType	 = _PKT_TYPE_IPV6;
		conf.ipv6.version		= 6;
		conf.rhdr.hdrlen 		= 0;
		conf.ipv6.priority 		= dscp<<2|ecn;
		conf.ipv6.flowlbl[0] = 0x0;
		conf.ipv6.flowlbl[1] = 0x0;
		conf.ipv6.flowlbl[2] = 0x0;
		conf.ipv6.hoplmt 		= ttl;
		conf.ipv6.nxthdr		= 4; //IPIP(4)
		in6_pton(v6_wan_remote_host,-1,(unsigned char *)&conf.conf_v6sip,-1,NULL);
		in6_pton(mc_dip2v6dip,-1,(unsigned char *)&conf.conf_v6dip,-1,NULL);
		strtomac((void *)&conf.conf_smac[0], wan_nexthop_mac);
		strtomac((void *)&conf.conf_dmac[0], mc_dipv6dmac);
		conf.conf_sport = REMOTE_PORT; 	
		conf.conf_dport = int_port; 	
		break;
	case TYPE_DUALHDR_DSLITE_MULTICAST_ROUTING_G2W:
		strtomac((void *)&conf.conf_smac[0], wan_gmac);
		conf.conf_dip	 = ntohl(inet_addr(mc_dip));
		if (pppoetagif==1)
			strtomac((void *)&conf.conf_dmac[0], wan_nexthop_mac);
		else
			strtomac((void *)&conf.conf_dmac[0], v6mc_dmac);
		break;
	case TYPE_DUALHDR_DSLITE_MULTICAST_ROUTING_W2G:
		strtomac((void *)&conf.conf_smac[0], wan_nexthop_mac);
		conf.conf_sip	 = ntohl(inet_addr(wan_remote_host));
		conf.conf_dip = ntohl(inet_addr(mc_dip));
		conf.conf_sport  = REMOTE_PORT;
		conf.conf_dport  = int_port;
		if (pppoetagif==1)
			strtomac((void *)&conf.conf_dmac[0], wan_gmac);
		else
			strtomac((void *)&conf.conf_dmac[0], mc_dipv6dmac);
		break;
	}

	conf.ip.version   = ipver;
	conf.ipv6.version = ip6ver;
	
	*pktLen = pktGen(&conf, (int8*)pktBuf);			 /* generate pkt in buff */ 
	
	*pktLen += 4;	// packet CRC 
	
	return 0;
}

int normalPktGen_ecn_syn(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,//type: 0:l2_unicast/1:multicast/2:broadcast/3:l34_unicast
	int ctagif,int cvid,int cpri,
	int stagif,int svid,int spri, int dscp, int ecn, int syn_option,
	cputag_t *cputag, cputagTx_t *cputagTx,
	int pppoetagif,int pppoe_session_id,int ttl) 
{
	return normalPktGen_ecn_syn_errFlag(pktBuf, pktData, pktLen, type, ctagif, cvid, cpri, stagif, svid, spri, dscp, ecn, syn_option, cputag, cputagTx, pppoetagif, pppoe_session_id, ttl, 0, 4, 6);
}

int normalPktGen_ecn(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,//type: 0:l2_unicast/1:multicast/2:broadcast/3:l34_unicast
	int ctagif,int cvid,int cpri,
	int stagif,int svid,int spri, int dscp, int ecn,
	cputag_t *cputag, cputagTx_t *cputagTx,
	int pppoetagif,int pppoe_session_id,int ttl) 
{
	return normalPktGen_ecn_syn(pktBuf, pktData, pktLen, type, ctagif, cvid, cpri, stagif, svid, spri, dscp, ecn, 0, cputag, cputagTx, pppoetagif, pppoe_session_id, ttl);
}

int normalPktGen(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,//type: 0:l2_unicast/1:multicast/2:broadcast/3:l34_unicast
		int ctagif,int cvid,int cpri,
		int stagif,int svid,int spri, int dscp,
		cputag_t *cputag, cputagTx_t *cputagTx,
		int pppoetagif,int pppoe_session_id,int ttl) 
{
	return normalPktGen_ecn_syn(pktBuf, pktData, pktLen, type, ctagif, cvid, cpri, stagif, svid, spri, dscp, 0, 0, cputag, cputagTx, pppoetagif, pppoe_session_id, ttl);
}


int IGMPv2_pktGen(uint8 *pktBuf,int *pktLen,igmpType_t igmpType,uint8 *smac,uint32 sourceip,uint32 groupip)
{

	rtl8651_PktConf_t conf;
	memset(&conf,0,sizeof(conf));

	conf.conf_dmac[0]=0x01;
	conf.conf_dmac[1]=0x00;
	conf.conf_dmac[2]=0x5e;
	conf.conf_dmac[3]=(groupip>>16)&0x7f;
	conf.conf_dmac[4]=(groupip>>8)&0xff;
	conf.conf_dmac[5]=(groupip>>0)&0xff;

	memcpy(conf.conf_smac,smac,6);
	conf.pktType = _PKT_TYPE_IGMP;
	conf.conf_dip	 = groupip;
	conf.conf_sip	 = sourceip;
	conf.ip.ttl = 10;
	
	conf.igmp.ver = 2;
	conf.igmp.gaddr = groupip;
	if(igmpType==IGMPv2_REPORT)
		conf.igmp.type					= IGMP_V2_MEMBERSHIP_REPORT;
	else if(igmpType==IGMPv2_QUERY)
		conf.igmp.type					= IGMP_MEMBERSHIP_QUERY;		
	else if(igmpType==IGMPv2_LEAVE)
		conf.igmp.type					= IGMP_V2_LEAVE_GROUP;


	*pktLen = pktGen(&conf, (int8*)pktBuf);			 /* generate pkt in buff */

	*pktLen += 4;	// packet CRC		

	return SUCCESS;

}

/* reducePPTPOpt: 0 to gen normal pptp gre header */
int pptpPktGen_gerneral_errFlag(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,uint32 errFlag,uint32 ipver, uint32 iphdl, uint32 ip6ver,
	int ctagif,int cvid,int cpri,
	int stagif,int svid,int spri, int tos, int pptp_tos,
	cputag_t *cputag, uint8 TCPflag, int innerPaddingCut, int origPadding,
	int pppoetagif,int pppoe_session_id,int ttl,int pptp_ttl,int pptp_fragment, int pptp_lcp,
	int pptp_callid,int pptp_seqno,int pptp_ackno,int ipv4_id, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t pptp_cmpPPPMode) 
{
	int32 ori_skip_minPktLen_check;
	uint8 innerIsIPv4 = FALSE;
	uint8 *outerSrcIP = NULL, *outerDstIP = NULL;
	uint16 contBufSize = 64;
	uint16 l3sum = 0;
	uint16 shiftOffset = 0;
	uint16 extraHdrLen = 40;
	uint16 ipv6ExtHdrlen=0;
	compressed_ppp_mode_t cmpPPPMode=COMPRESSED_PPP_FF0300XX;
	uint8 rmSeqBit = (reducePPTPOpt&0x2)?1:0;
	uint8 rmAckBit = (reducePPTPOpt&0x1)?1:0;
	int seqackLen = 0;
	_etherHdr_t *ether = NULL;
	ipHdr_t *ipv4h = NULL;
	//ipv6hdr_t *ipv6h = NULL;
	tcpHdr_t *tcph = NULL;
	greHdr_t *pptpPtr = NULL;
	uint8 *outerHdr = rtlglue_malloc(contBufSize);
	bzero(outerHdr, contBufSize);

	//check lcp, should not compressed!! reference by RFC 1661
	if(pptp_lcp==0)cmpPPPMode=pptp_cmpPPPMode;
	extraHdrLen-=cmpPPPMode;
	
	switch(type)
	{
		case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W:
		case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_UDP:
		case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_ICMP:
		case TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W:
		case TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_UDP:
		case TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_ICMP:
		case TYPE_DUALHDR_PPTP_MULTICAST_ROUTING_G2W:
		//frag:mf
		case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_FRAG:
		case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_UDP_FRAG:
		case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_ICMP_FRAG:
		case TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_FRAG:
		case TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_UDP_FRAG:
		case TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_ICMP_FRAG:
		//frag:frag_offset
		case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_FRAG2:
		case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_UDP_FRAG2:
		case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_ICMP_FRAG2:
		case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_ICMPPOD_FRAG2:
		case TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_FRAG2:
		case TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_UDP_FRAG2:
		case TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_ICMP_FRAG2:
		case TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_ICMPPOD_FRAG2:
		//frag:mf+frag_offset
		case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_FRAG3:
		case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_UDP_FRAG3:
		case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_ICMP_FRAG3:
		case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_G2W_ICMPPOD_FRAG3:
		case TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_FRAG3:
		case TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_UDP_FRAG3:
		case TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_ICMP_FRAG3:
		case TYPE_DUALHDR_PPTP_UNICAST_NAT_G2W_ICMPPOD_FRAG3:
			innerIsIPv4 = TRUE;
			outerSrcIP = PPTP_CLIENT_IP;
			outerDstIP = PPTP_SERVER_IP;
			break;
		case TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W:
		case TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_UDP:
		case TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_ICMP:
		case TYPE_DUALHDR_PPTP_MULTICAST_IPV6_G2W:
			innerIsIPv4 = FALSE;
			outerSrcIP = PPTP_CLIENT_IP;
			outerDstIP = PPTP_SERVER_IP;
			break;
		//frag:mf
		case TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_FRAG:
		case TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_UDP_FRAG:
		case TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_ICMP_FRAG:
		//frag:frag_offset
		case TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_FRAG2:
		case TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_UDP_FRAG2:
		case TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_ICMP_FRAG2:
		case TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_ICMPPOD_FRAG2:
		//frag:mf+frag_offset
		case TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_FRAG3:
		case TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_UDP_FRAG3:
		case TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_ICMP_FRAG3:
		case TYPE_DUALHDR_PPTP_UNICAST_IPV6_G2W_ICMPPOD_FRAG3:
			innerIsIPv4 = FALSE;
			outerSrcIP = PPTP_CLIENT_IP;
			outerDstIP = PPTP_SERVER_IP;
			ipv6ExtHdrlen = 8;
			break;
		case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L:
		case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_UDP:
		case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_ICMP:
		case TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G:
		case TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_UDP:
		case TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_ICMP:
		case TYPE_DUALHDR_PPTP_MULTICAST_ROUTING_W2G:
		//frag:mf
		case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_FRAG:
		case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_UDP_FRAG:
		case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_ICMP_FRAG:
		case TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_FRAG:
		case TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_UDP_FRAG:
		case TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_ICMP_FRAG:
		//frag:frag_offset
		case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_FRAG2:
		case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_UDP_FRAG2:
		case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_ICMP_FRAG2:
		case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_ICMPPOD_FRAG2:
		case TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_FRAG2:
		case TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_UDP_FRAG2:
		case TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_ICMP_FRAG2:
		case TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_ICMPPOD_FRAG2:
		//frag:mf+frag_offset
		case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_FRAG3:
		case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_UDP_FRAG3:
		case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_ICMP_FRAG3:
		case TYPE_DUALHDR_PPTP_UNICAST_ROUTING_W2L_ICMPPOD_FRAG3:
		case TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_FRAG3:
		case TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_UDP_FRAG3:
		case TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_ICMP_FRAG3:
		case TYPE_DUALHDR_PPTP_UNICAST_NAT_W2G_ICMPPOD_FRAG3:
			innerIsIPv4 = TRUE;
			outerSrcIP = PPTP_SERVER_IP;
			outerDstIP = PPTP_CLIENT_IP;
			break;
		case TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L:
		case TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_UDP:
		case TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_ICMP:
		case TYPE_DUALHDR_PPTP_MULTICAST_IPV6_W2G:
			innerIsIPv4 = FALSE;
			outerSrcIP = PPTP_SERVER_IP;
			outerDstIP = PPTP_CLIENT_IP;
			break;
		//frag:mf
		case TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_FRAG:
		case TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_UDP_FRAG:
		case TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_ICMP_FRAG:
		//frag:frag_offset		
		case TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_FRAG2:
		case TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_UDP_FRAG2:
		case TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_ICMP_FRAG2:
		case TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_ICMPPOD_FRAG2:
		//frag:mf+frag_offset
		case TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_FRAG3:
		case TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_UDP_FRAG3:
		case TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_ICMP_FRAG3:
		case TYPE_DUALHDR_PPTP_UNICAST_IPV6_W2L_ICMPPOD_FRAG3:
			innerIsIPv4 = FALSE;
			outerSrcIP = PPTP_SERVER_IP;
			outerDstIP = PPTP_CLIENT_IP;
			ipv6ExtHdrlen = 8;
			break;
		default:
#ifdef _MSG			
		TEST_WARNING("Not support this packet type");
#endif
			break;
	}

	/*	step 1. gen inner include ehter header
    	step 2. gen outer: [IP-PPTP_client to PPTP_server] or [IP-PPTP_server to PPTP_client]
    	step 3. update outer IP/GRE info; then merge them	*/

	if(pppoetagif)
		shiftOffset = sizeof(_etherHdr_t) + sizeof(_pppoeHdr_t);
	else
		shiftOffset = sizeof(_etherHdr_t);

	if(stagif)
		shiftOffset += sizeof(_vlanHdr_t);
	if(ctagif)
		shiftOffset += sizeof(_vlanHdr_t);
	if(cputag)
		shiftOffset += 8;

	ori_skip_minPktLen_check=skip_minPktLen_check;
	skip_minPktLen_check=innerPaddingCut;

	ASSERT_EQ(normalPktGen_ecn_syn_errFlag(pktBuf,pktData, pktLen, type,
		ctagif, cvid, cpri,
		stagif, svid, spri, (tos&0xfc)>>2, tos&0x3, (TCPflag&TH_SYN)>0,
		cputag, NULL,
		pppoetagif, pppoe_session_id, ttl, errFlag, ipver, ip6ver), 0);
	*pktLen -= 4;	// packet CRC 

	if((innerPaddingCut==0)&&(origPadding>0)){
		int compensate_padding=0;
		if(ctagif)
			compensate_padding+=4;
		if(stagif)
			compensate_padding+=4;
		if(cputag)
			compensate_padding+=8;
		if(pppoetagif)
			compensate_padding+=8;
		if(compensate_padding>origPadding)compensate_padding=origPadding;
		if(compensate_padding>0){
			memset(pktBuf+(*pktLen), 0, compensate_padding);	//clear old CRC
			*pktLen+=compensate_padding;
		}
	}
	skip_minPktLen_check=ori_skip_minPktLen_check;

	//setup inner TCP flags
	if(TCPflag){
		uint16 tmp, csum, l4payloadlen;
		if(innerIsIPv4){
			ipv4h = (ipHdr_t *)(pktBuf+shiftOffset);
			tcph = (tcpHdr_t *)(pktBuf+shiftOffset+sizeof(ipHdr_t));
			tcph->th_flags = TCPflag;

			l4payloadlen = ntohs(ipv4h->ip_len) - (ipv4h->ip_hl * 4);

			csum = ipcsum((uint16 *)&ipv4h->ip_src, 8, 0);
			tmp = htons(IP_PROTO_TCP);
			csum = ipcsum(&tmp, 2, csum);
			tmp = htons(l4payloadlen);
			//rtlglue_printf("\n\nthe tmp is [%d]\n\n", tmp);
			csum = ipcsum(&tmp, 2, csum);
			tcph->th_sum = 0;
			csum = ipcsum((uint16 *)tcph, l4payloadlen, csum);
			tcph->th_sum = htons(~csum);
		}else{
			ip6Hdr_t * ipPtr;
			uint32 len;
			uint32 lenb;			
			
			ipPtr = (ip6Hdr_t *)(pktBuf+shiftOffset);
			tcph = (tcpHdr_t *)(pktBuf+shiftOffset+sizeof(ipv6hdr_t)+ipv6ExtHdrlen);
			tcph->th_flags = TCPflag;
			
			csum=ipcsum((uint16 *)&ipPtr->ip_src, 32, 0);
			len=0;
			len=ntohs(ipPtr->ip_len) - ipv6ExtHdrlen;
			lenb=htonl(len);
			csum= ipcsum((uint16 *)&lenb, 4, csum);
			tmp=6;
			tmp=htons(tmp);
			csum=ipcsum(&tmp, 2, csum);
			tcph->th_sum=0;
			csum=ipcsum((uint16 *)tcph, len, csum);
			tcph->th_sum=htons(~csum);			
		}
	}

	//change iphdl
	if(innerIsIPv4){
		ipv4h = (ipHdr_t *)(pktBuf+shiftOffset);
		if(iphdl%4)
			ipv4h->ip_hl = iphdl/4 + 1;
		else
			ipv4h->ip_hl = iphdl/4;
	}
	
	ASSERT_EQ(pptpOuterHdrGen_lcp(outerHdr, outerSrcIP, outerDstIP, pptp_callid, innerIsIPv4, 40-cmpPPPMode, pptp_tos, pptp_ttl, pptp_fragment, reducePPTPOpt, cmpPPPMode, pptp_lcp), 0);

	ipv4h = (ipHdr_t *)outerHdr;
	pptpPtr = (greHdr_t *)(outerHdr+sizeof(ipHdr_t));

	if(rmSeqBit!=1){
		seqackLen += 4;
		pptpPtr->gre_seqno = htonl(pptp_seqno);
	}
	if(rmAckBit!=1){
		seqackLen += 4;
		if(rmSeqBit==1)
			pptpPtr->gre_seqno = htonl(pptp_ackno);		// bacuse no seq filed in this packet, the pointer to gre_seqno should be ack value;
		else
			pptpPtr->gre_ackno = htonl(pptp_ackno);
	}
	pptpPtr->gre_length = htons(*pktLen-shiftOffset+(4-cmpPPPMode));		// 1~4: PPP header
	extraHdrLen -= (8-seqackLen);		 // complete len(40) - reducedLen(0/4/8)
	
	if(pppoetagif){	 	
		_pppoeHdr_t *pppoePtr = (_pppoeHdr_t *)(pktBuf+shiftOffset-sizeof(_pppoeHdr_t));
		pppoePtr->length = htons(*pktLen-shiftOffset+extraHdrLen+2);//htons(ntohs(pppoePtr->length) + extraHdrLen);
		if(!innerIsIPv4)	// original protocol is IPv6(0x0057), now change to IPv4
			pppoePtr->proto = htons(0x0021);
	}else{
		ether = (_etherHdr_t *)pktBuf;
		if(ctagif || stagif){
			_vlanHdr_t *vlanPtr = (_vlanHdr_t *)(pktBuf + shiftOffset - sizeof(_vlanHdr_t));
			if(!innerIsIPv4)	// original ethertype is IPv6 for inner header, now change to ipv4 for outer header
				vlanPtr->etherType= htons(0x0800);
		}else{
			if(!innerIsIPv4){	// original ethertype is IPv6 for inner header, now change to ipv4 for outer header
				if(cputag)*((&ether->etherType)+4) = htons(0x0800);	
				else ether->etherType = htons(0x0800);	
			}
		}	
	}
	

	ipv4h->ip_id = htons(ipv4_id);
	ipv4h->ip_len = htons(*pktLen-shiftOffset+extraHdrLen);
	l3sum = model_ipChecksum((struct ip*)ipv4h);
	ipv4h->ip_sum = l3sum;


	/* Shift original ip header to inner header location */
	memmove(pktBuf+shiftOffset+extraHdrLen, pktBuf+shiftOffset, *pktLen-shiftOffset);	
	/* Insert outer ip header after insert_location */
	memcpy(pktBuf+shiftOffset, outerHdr, extraHdrLen);
	*pktLen += extraHdrLen;

	l2FormCrc(0, pktBuf, *pktLen);
	*pktLen += 4;	// packet CRC 

	rtlglue_free(outerHdr);
	return SUCCESS;
}
	
int pptpPktGen_gerneral(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,
	int ctagif,int cvid,int cpri,
	int stagif,int svid,int spri, int tos, int pptp_tos,
	cputag_t *cputag, uint8 TCPflag, int innerPaddingCut, int origPadding,
	int pppoetagif,int pppoe_session_id,int ttl,int pptp_ttl,int pptp_fragment, int pptp_lcp,
	int pptp_callid,int pptp_seqno,int pptp_ackno,int ipv4_id, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t pptp_cmpPPPMode) 
{
	return pptpPktGen_gerneral_errFlag(pktBuf, pktData, pktLen, type, 0, 4, 20, 6,
							ctagif, cvid, cpri, 
							stagif, svid, spri, tos, pptp_tos, 
							cputag, TCPflag, innerPaddingCut, origPadding, pppoetagif, pppoe_session_id, ttl, pptp_ttl, pptp_fragment, pptp_lcp,
							pptp_callid, pptp_seqno, pptp_ackno, ipv4_id, reducePPTPOpt, pptp_cmpPPPMode);
}


int pptpPktGenCutPadding(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,
	int ctagif,int cvid,int cpri,
	int stagif,int svid,int spri, int tos, int pptp_tos,
	cputag_t *cputag, uint8 TCPflag, int innerPaddingCut, int origPadding,
	int pppoetagif,int pppoe_session_id,int ttl,int pptp_ttl,int pptp_fragment,
	int pptp_callid,int pptp_seqno,int pptp_ackno,int ipv4_id, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t pptp_cmpPPPMode) 
{
	return pptpPktGen_gerneral(pktBuf, pktData, pktLen, type,
							ctagif, cvid, cpri, 
							stagif, svid, spri, tos, pptp_tos, 
							cputag, TCPflag, innerPaddingCut, origPadding, pppoetagif, pppoe_session_id, ttl, pptp_ttl, pptp_fragment, 0,
							pptp_callid, pptp_seqno, pptp_ackno, ipv4_id, reducePPTPOpt, pptp_cmpPPPMode);
}

int pptpPktGen(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,
		int ctagif,int cvid,int cpri,
		int stagif,int svid,int spri, int tos, int pptp_tos,
		cputag_t *cputag, uint8 TCPflag,
		int pppoetagif,int pppoe_session_id,int ttl,int pptp_ttl,int pptp_fragment, 
		int pptp_callid,int pptp_seqno,int pptp_ackno,int ipv4_id, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t cmpPPPMode) 
{
	return pptpPktGen_gerneral(pktBuf, pktData, pktLen, type,
							ctagif, cvid, cpri, 
							stagif, svid, spri, tos, pptp_tos, 
							cputag, TCPflag, 0, 0, pppoetagif, pppoe_session_id, ttl, pptp_ttl, pptp_fragment, 0,
							pptp_callid, pptp_seqno, pptp_ackno, ipv4_id, reducePPTPOpt, cmpPPPMode);
}

int pptpPktGen_lcp(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,
		int ctagif,int cvid,int cpri,
		int stagif,int svid,int spri, int tos, int pptp_tos,
		cputag_t *cputag, uint8 TCPflag,
		int pppoetagif,int pppoe_session_id,int ttl,int pptp_ttl,int pptp_fragment, 
		int pptp_callid,int pptp_seqno,int pptp_ackno,int ipv4_id, pptp_outerhdr_option_t reducePPTPOpt) 
{
	return pptpPktGen_gerneral(pktBuf, pktData, pktLen, type,
							ctagif, cvid, cpri, 
							stagif, svid, spri, tos, pptp_tos, 
							cputag, TCPflag, 0, 0, pppoetagif, pppoe_session_id, ttl, pptp_ttl, pptp_fragment, 1,
							pptp_callid, pptp_seqno, pptp_ackno, ipv4_id, reducePPTPOpt, COMPRESSED_PPP_FF0300XX);
}

int pptpPktGen_errFlag(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,uint32 errFlag,
		int ctagif,int cvid,int cpri,
		int stagif,int svid,int spri, int tos, int pptp_tos,
		cputag_t *cputag, uint8 TCPflag,
		int pppoetagif,int pppoe_session_id,int ttl,int pptp_ttl,int pptp_fragment, 
		int pptp_callid,int pptp_seqno,int pptp_ackno,int ipv4_id, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t cmpPPPMode) 
{
	return pptpPktGen_gerneral_errFlag(pktBuf, pktData, pktLen, type, errFlag,4,20,6,
							ctagif, cvid, cpri, 
							stagif, svid, spri, tos, pptp_tos, 
							cputag, TCPflag, 0, 0, pppoetagif, pppoe_session_id, ttl, pptp_ttl, pptp_fragment, 0,
							pptp_callid, pptp_seqno, pptp_ackno, ipv4_id, reducePPTPOpt, cmpPPPMode);
}

int pptpPktGen_errFlag_ipver(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,uint32 errFlag,
		int ctagif,int cvid,int cpri,
		int stagif,int svid,int spri, int tos, int pptp_tos,
		cputag_t *cputag, uint8 TCPflag,
		int pppoetagif,int pppoe_session_id,int ttl,int pptp_ttl,int pptp_fragment, 
		int pptp_callid,int pptp_seqno,int pptp_ackno,int ipv4_id, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t cmpPPPMode) 
{
	return pptpPktGen_gerneral_errFlag(pktBuf, pktData, pktLen, type, errFlag, 3, 20, 5,
							ctagif, cvid, cpri, 
							stagif, svid, spri, tos, pptp_tos, 
							cputag, TCPflag, 0, 0, pppoetagif, pppoe_session_id, ttl, pptp_ttl, pptp_fragment, 0,
							pptp_callid, pptp_seqno, pptp_ackno, ipv4_id, reducePPTPOpt, cmpPPPMode);
}

int pptpPktGen_errFlag_iphdl(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,uint32 errFlag,
		int ctagif,int cvid,int cpri,
		int stagif,int svid,int spri, int tos, int pptp_tos,
		cputag_t *cputag, uint8 TCPflag,
		int pppoetagif,int pppoe_session_id,int ttl,int pptp_ttl,int pptp_fragment, 
		int pptp_callid,int pptp_seqno,int pptp_ackno,int ipv4_id, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t cmpPPPMode) 
{
	return pptpPktGen_gerneral_errFlag(pktBuf, pktData, pktLen, type, errFlag, 4, 16, 6,
							ctagif, cvid, cpri, 
							stagif, svid, spri, tos, pptp_tos, 
							cputag, TCPflag, 0, 0, pppoetagif, pppoe_session_id, ttl, pptp_ttl, pptp_fragment, 0,
							pptp_callid, pptp_seqno, pptp_ackno, ipv4_id, reducePPTPOpt, cmpPPPMode);
}

int pptpv6PktGen_gerneral_errFlag(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,uint32 errFlag, uint32 ipver, uint32 iphdl, uint32 ip6ver,
	int ctagif,int cvid,int cpri,
	int stagif,int svid,int spri, int tos, int pptp_tos,
	cputag_t *cputag, uint8 TCPflag, int innerPaddingCut, int origPadding,
	int pppoetagif,int pppoe_session_id,int ttl,int pptp_ttl,int pptp_fragment, int pptp_lcp,
	int pptp_callid,int pptp_seqno,int pptp_ackno, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t pptp_cmpPPPMode) 
{
	int32 ori_skip_minPktLen_check;
	uint8 innerIsIPv4 = FALSE;
	uint8 outerSrcIP[16] = {0}, outerDstIP[16] = {0};
	uint16 contBufSize = 64;
	//uint16 l3sum = 0;
	uint16 shiftOffset = 0;
	uint16 extraHdrLen = 60;
	uint16 ipv6ExtHdrlen=0;
	compressed_ppp_mode_t cmpPPPMode=COMPRESSED_PPP_FF0300XX;
	uint8 rmSeqBit = (reducePPTPOpt&0x2)?1:0;
	uint8 rmAckBit = (reducePPTPOpt&0x1)?1:0;
	int seqackLen = 0;
	_etherHdr_t *ether = NULL;
	ipHdr_t *ipv4h = NULL;
	ipv6hdr_t *ipv6h = NULL;
	tcpHdr_t *tcph = NULL;
	greHdr_t *pptpPtr = NULL;
	uint8 *outerHdr = rtlglue_malloc(contBufSize);
	bzero(outerHdr, contBufSize);

	//check lcp, should not compressed!! reference by RFC 1661
	if(pptp_lcp==0)cmpPPPMode=pptp_cmpPPPMode;
	extraHdrLen-=cmpPPPMode;
		
	switch(type)
	{
		case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W:
		case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_UDP:
		case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_ICMP:
		case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W:
		case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_UDP:
		case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_ICMP:
		//frag:mf
		case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_FRAG:
		case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_UDP_FRAG:
		case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_ICMP_FRAG:
		case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_FRAG:
		case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_UDP_FRAG:
		case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_ICMP_FRAG:
		//frag:frag_offset
		case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_FRAG2:
		case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_UDP_FRAG2:
		case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_ICMP_FRAG2:
		case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_ICMPPOD_FRAG2:
		case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_FRAG2:
		case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_UDP_FRAG2:
		case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_ICMP_FRAG2:
		case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_ICMPPOD_FRAG2:
		//frag:mf+frag_offset
		case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_FRAG3:
		case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_UDP_FRAG3:
		case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_ICMP_FRAG3:
		case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_G2W_ICMPPOD_FRAG3:
		case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_FRAG3:
		case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_UDP_FRAG3:
		case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_ICMP_FRAG3:
		case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_G2W_ICMPPOD_FRAG3:
			innerIsIPv4 = TRUE;
			in6_pton(PPTPV6_CLIENT_IP,-1,outerSrcIP,-1,NULL);
			in6_pton(PPTPV6_SERVER_IP,-1,outerDstIP,-1,NULL);
			break;
		case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W:
		case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_UDP:
			innerIsIPv4 = FALSE;
			in6_pton(PPTPV6_CLIENT_IP,-1,outerSrcIP,-1,NULL);
			in6_pton(PPTPV6_SERVER_IP,-1,outerDstIP,-1,NULL);
			break;
		//frag:mf
		case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_FRAG:
		case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_UDP_FRAG:
		case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_ICMP_FRAG:
		//frag:frag_offset
		case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_FRAG2:
		case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_UDP_FRAG2:
		case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_ICMP_FRAG2:
		case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_ICMPPOD_FRAG2:
		//frag:mf+frag_offset
		case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_FRAG3:
		case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_UDP_FRAG3:
		case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_ICMP_FRAG3:
		case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_G2W_ICMPPOD_FRAG3:
			innerIsIPv4 = FALSE;
			in6_pton(PPTPV6_CLIENT_IP,-1,outerSrcIP,-1,NULL);
			in6_pton(PPTPV6_SERVER_IP,-1,outerDstIP,-1,NULL);
			ipv6ExtHdrlen=8;
			break;
		case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L:
		case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_UDP:
		case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_ICMP:
		case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G:
		case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_UDP:
		case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_ICMP:
		//frag:mf
		case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_FRAG:
		case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_UDP_FRAG:
		case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_ICMP_FRAG:
		case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_FRAG:
		case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_UDP_FRAG:
		case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_ICMP_FRAG:
		//frag:frag_offset
		case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_FRAG2:
		case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_UDP_FRAG2:
		case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_ICMP_FRAG2:
		case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_ICMPPOD_FRAG2:
		case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_FRAG2:
		case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_UDP_FRAG2:
		case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_ICMP_FRAG2:
		case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_ICMPPOD_FRAG2:
		//frag:mf+frag_offset
		case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_FRAG3:
		case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_UDP_FRAG3:
		case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_ICMP_FRAG3:
		case TYPE_DUALHDR_PPTPV6_UNICAST_ROUTING_W2L_ICMPPOD_FRAG3:
		case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_FRAG3:
		case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_UDP_FRAG3:
		case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_ICMP_FRAG3:
		case TYPE_DUALHDR_PPTPV6_UNICAST_NAT_W2G_ICMPPOD_FRAG3:
			innerIsIPv4 = TRUE;
			in6_pton(PPTPV6_SERVER_IP,-1,outerSrcIP,-1,NULL);
			in6_pton(PPTPV6_CLIENT_IP,-1,outerDstIP,-1,NULL);
			break;
		case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L:
		case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_UDP:
		case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_ICMP:
			innerIsIPv4 = FALSE;
			in6_pton(PPTPV6_SERVER_IP,-1,outerSrcIP,-1,NULL);
			in6_pton(PPTPV6_CLIENT_IP,-1,outerDstIP,-1,NULL);
			break;
		//frag:mf
		case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_FRAG:
		case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_UDP_FRAG:
		case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_ICMP_FRAG:
		//frag:frag_offset
		case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_FRAG2:
		case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_UDP_FRAG2:
		case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_ICMP_FRAG2:
		case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_ICMPPOD_FRAG2:
		//frag:mf+frag_offset
		case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_FRAG3:
		case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_UDP_FRAG3:
		case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_ICMP_FRAG3:
		case TYPE_DUALHDR_PPTPV6_UNICAST_IPV6_W2L_ICMPPOD_FRAG3:
			innerIsIPv4 = FALSE;
			in6_pton(PPTPV6_SERVER_IP,-1,outerSrcIP,-1,NULL);
			in6_pton(PPTPV6_CLIENT_IP,-1,outerDstIP,-1,NULL);
			ipv6ExtHdrlen=8;
			break;
		default:
#ifdef _MSG			
			TEST_WARNING("Not support this packet type");
#endif
			break;
	}

	/*	step 1. gen inner include ehter header
		step 2. gen outer: [IP-PPTP_client to PPTP_server] or [IP-PPTP_server to PPTP_client]
		step 3. update outer IP/GRE info; then merge them	*/

	if(pppoetagif)
		shiftOffset = sizeof(_etherHdr_t) + sizeof(_pppoeHdr_t);
	else
		shiftOffset = sizeof(_etherHdr_t);

	if(stagif)
		shiftOffset += sizeof(_vlanHdr_t);
	if(ctagif)
		shiftOffset += sizeof(_vlanHdr_t);
	if(cputag)
		shiftOffset += 8;

	ori_skip_minPktLen_check=skip_minPktLen_check;
	skip_minPktLen_check=innerPaddingCut;

	ASSERT_EQ(normalPktGen_ecn_syn_errFlag(pktBuf,pktData, pktLen, type,
		ctagif, cvid, cpri,
		stagif, svid, spri, (tos&0xfc)>>2, tos&0x3, (TCPflag&TH_SYN)>0,
		cputag, NULL,
		pppoetagif, pppoe_session_id, ttl, errFlag, ipver, ip6ver), 0);
	*pktLen -= 4;	// packet CRC 

	if((innerPaddingCut==0)&&(origPadding>0)){
		int compensate_padding=0;
		if(ctagif)
			compensate_padding+=4;
		if(stagif)
			compensate_padding+=4;
		if(cputag)
			compensate_padding+=8;
		if(pppoetagif)
			compensate_padding+=8;
		if(compensate_padding>origPadding)compensate_padding=origPadding;
		if(compensate_padding>0){
			memset(pktBuf+(*pktLen), 0, compensate_padding);	//clear old CRC
			*pktLen+=compensate_padding;
		}
	}
	skip_minPktLen_check=ori_skip_minPktLen_check;

	//setup inner TCP flags
	if(TCPflag){
		uint16 tmp, csum, l4payloadlen;
		if(innerIsIPv4){
			ipv4h = (ipHdr_t *)(pktBuf+shiftOffset);
			tcph = (tcpHdr_t *)(pktBuf+shiftOffset+sizeof(ipHdr_t));
			tcph->th_flags = TCPflag;

			l4payloadlen = ntohs(ipv4h->ip_len) - (ipv4h->ip_hl * 4);

			csum = ipcsum((uint16 *)&ipv4h->ip_src, 8, 0);
			tmp = htons(IP_PROTO_TCP);
			csum = ipcsum(&tmp, 2, csum);
			tmp = htons(l4payloadlen);
			//rtlglue_printf("\n\nthe tmp is [%d]\n\n", tmp);
			csum = ipcsum(&tmp, 2, csum);
			tcph->th_sum = 0;
			csum = ipcsum((uint16 *)tcph, l4payloadlen, csum);
			tcph->th_sum = htons(~csum);
		}else{
			ip6Hdr_t * ipPtr;
			uint32 len;
			uint32 lenb;			
			
			ipPtr = (ip6Hdr_t *)(pktBuf+shiftOffset);
			tcph = (tcpHdr_t *)(pktBuf+shiftOffset+sizeof(ipv6hdr_t)+ipv6ExtHdrlen);
			tcph->th_flags = TCPflag;
			
			csum=ipcsum((uint16 *)&ipPtr->ip_src, 32, 0);
			len=0;
			len=ntohs(ipPtr->ip_len) - ipv6ExtHdrlen;
			lenb=htonl(len);
			csum= ipcsum((uint16 *)&lenb, 4, csum);
			tmp=6;
			tmp=htons(tmp);
			csum=ipcsum(&tmp, 2, csum);
			tcph->th_sum=0;
			csum=ipcsum((uint16 *)tcph, len, csum);
			tcph->th_sum=htons(~csum);			
		}
	}

	//change iphdl
	if(innerIsIPv4){
		ipv4h = (ipHdr_t *)(pktBuf+shiftOffset);
		if(iphdl%4)
			ipv4h->ip_hl = iphdl/4 + 1;
		else
			ipv4h->ip_hl = iphdl/4;
	}

	ASSERT_EQ(pptpOuterV6HdrGen_lcp(outerHdr, outerSrcIP, outerDstIP, pptp_callid, innerIsIPv4, 60-cmpPPPMode, pptp_tos, pptp_ttl, pptp_fragment, reducePPTPOpt, cmpPPPMode, pptp_lcp), 0);

	ipv6h = (ipv6hdr_t *)outerHdr;
	pptpPtr = (greHdr_t *)(outerHdr+sizeof(ipv6hdr_t)+(pptp_fragment>0?8:0));
	extraHdrLen += (pptp_fragment>0?8:0);		//ext frag hdr

	if(rmSeqBit!=1){
		seqackLen += 4;
		pptpPtr->gre_seqno = htonl(pptp_seqno);
	}
	if(rmAckBit!=1){
		seqackLen += 4;
		if(rmSeqBit==1)
			pptpPtr->gre_seqno = htonl(pptp_ackno); 	// bacuse no seq filed in this packet, the pointer to gre_seqno should be ack value;
		else
			pptpPtr->gre_ackno = htonl(pptp_ackno);
	}
	pptpPtr->gre_length = htons(*pktLen-shiftOffset+(4-cmpPPPMode));		// 1~4: PPP header
	extraHdrLen -= (8-seqackLen);		 // complete len(60) - reducedLen(0/4/8)
	
	if(pppoetagif){ 	
		_pppoeHdr_t *pppoePtr = (_pppoeHdr_t *)(pktBuf+shiftOffset-sizeof(_pppoeHdr_t));
		pppoePtr->length = htons(*pktLen-shiftOffset+extraHdrLen+2);//htons(ntohs(pppoePtr->length) + extraHdrLen);
		if(innerIsIPv4)	// original protocol is IPv4(0x0021), now change to IPv4
			pppoePtr->proto = htons(0x0057);
	}else{
		ether = (_etherHdr_t *)pktBuf;
		if(ctagif || stagif){
			_vlanHdr_t *vlanPtr = (_vlanHdr_t *)(pktBuf + shiftOffset - sizeof(_vlanHdr_t));
			if(innerIsIPv4)	// original ethertype is IPv4 for inner header, now change to ipv6 for outer header
				vlanPtr->etherType= htons(0x86dd);
		}else{
			if(innerIsIPv4){	// original ethertype is IPv4 for inner header, now change to ipv6 for outer header
				if(cputag)*((&ether->etherType)+4) = htons(0x86dd); 
				else ether->etherType = htons(0x86dd);	
			}
		}	
	}
	

	//ipv6h->ip_id = htons(ipv4_id);
	ipv6h->payload_len = htons(*pktLen-shiftOffset+extraHdrLen-40);
	//l3sum = model_ipChecksum((struct ip*)ipv4h);
	//ipv4h->ip_sum = l3sum;


	/* Shift original ip header to inner header location */
	memmove(pktBuf+shiftOffset+extraHdrLen, pktBuf+shiftOffset, *pktLen-shiftOffset);	
	/* Insert outer ip header after insert_location */
	memcpy(pktBuf+shiftOffset, outerHdr, extraHdrLen);
	*pktLen += extraHdrLen;

	l2FormCrc(0, pktBuf, *pktLen);
	*pktLen += 4;	// packet CRC 

	rtlglue_free(outerHdr);
	return SUCCESS;
}

int pptpv6PktGen_gerneral(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,
	int ctagif,int cvid,int cpri,
	int stagif,int svid,int spri, int tos, int pptp_tos,
	cputag_t *cputag, uint8 TCPflag, int innerPaddingCut, int origPadding,
	int pppoetagif,int pppoe_session_id,int ttl,int pptp_ttl,int pptp_fragment, int pptp_lcp,
	int pptp_callid,int pptp_seqno,int pptp_ackno, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t pptp_cmpPPPMode) 
{
	return pptpv6PktGen_gerneral_errFlag(pktBuf, pktData, pktLen, type,0,4,20,6,
							ctagif, cvid, cpri, 
							stagif, svid, spri, tos, pptp_tos, 
							cputag, TCPflag, innerPaddingCut, origPadding,
							pppoetagif, pppoe_session_id, ttl, pptp_ttl, pptp_fragment, pptp_lcp,
							pptp_callid, pptp_seqno, pptp_ackno, reducePPTPOpt, pptp_cmpPPPMode);
}

int pptpv6PktGenCutPadding(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,
		int ctagif,int cvid,int cpri,
		int stagif,int svid,int spri, int tos, int pptp_tos,
		cputag_t *cputag, uint8 TCPflag, int innerPaddingCut, int origPadding,
		int pppoetagif,int pppoe_session_id,int ttl,int pptp_ttl,int pptp_fragment,
		int pptp_callid,int pptp_seqno,int pptp_ackno, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t pptp_cmpPPPMode) 
{
	return pptpv6PktGen_gerneral(pktBuf, pktData, pktLen, type,
							ctagif, cvid, cpri, 
							stagif, svid, spri, tos, pptp_tos, 
							cputag, TCPflag, innerPaddingCut, origPadding,
							pppoetagif, pppoe_session_id, ttl, pptp_ttl, pptp_fragment, 0,
							pptp_callid, pptp_seqno, pptp_ackno, reducePPTPOpt, pptp_cmpPPPMode);
}


int pptpv6PktGen(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,
	int ctagif,int cvid,int cpri,
	int stagif,int svid,int spri, int tos, int pptp_tos,
	cputag_t *cputag, uint8 TCPflag,
	int pppoetagif,int pppoe_session_id,int ttl,int pptp_ttl,int pptp_fragment,
	int pptp_callid,int pptp_seqno,int pptp_ackno, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t cmpPPPMode) 
{
	return pptpv6PktGen_gerneral(pktBuf, pktData, pktLen, type,
							ctagif, cvid, cpri, 
							stagif, svid, spri, tos, pptp_tos, 
							cputag, TCPflag, 0, 0,
							pppoetagif, pppoe_session_id, ttl, pptp_ttl, pptp_fragment, 0,
							pptp_callid, pptp_seqno, pptp_ackno, reducePPTPOpt, cmpPPPMode);
}

int pptpv6PktGen_lcp(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,
	int ctagif,int cvid,int cpri,
	int stagif,int svid,int spri, int tos, int pptp_tos,
	cputag_t *cputag, uint8 TCPflag,
	int pppoetagif,int pppoe_session_id,int ttl,int pptp_ttl,int pptp_fragment,
	int pptp_callid,int pptp_seqno,int pptp_ackno, pptp_outerhdr_option_t reducePPTPOpt) 
{
	return pptpv6PktGen_gerneral(pktBuf, pktData, pktLen, type,
							ctagif, cvid, cpri, 
							stagif, svid, spri, tos, pptp_tos, 
							cputag, TCPflag, 0, 0,
							pppoetagif, pppoe_session_id, ttl, pptp_ttl, pptp_fragment, 1,
							pptp_callid, pptp_seqno, pptp_ackno, reducePPTPOpt, COMPRESSED_PPP_FF0300XX);
}

int pptpv6PktGen_errFlag(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,uint32 errFlag,
	int ctagif,int cvid,int cpri,
	int stagif,int svid,int spri, int tos, int pptp_tos,
	cputag_t *cputag, uint8 TCPflag,
	int pppoetagif,int pppoe_session_id,int ttl,int pptp_ttl,int pptp_fragment,
	int pptp_callid,int pptp_seqno,int pptp_ackno, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t cmpPPPMode) 
{
	return pptpv6PktGen_gerneral_errFlag(pktBuf, pktData, pktLen, type, errFlag, 4, 20, 6,
							ctagif, cvid, cpri, 
							stagif, svid, spri, tos, pptp_tos, 
							cputag, TCPflag, 0, 0,
							pppoetagif, pppoe_session_id, ttl, pptp_ttl, pptp_fragment, 0,
							pptp_callid, pptp_seqno, pptp_ackno, reducePPTPOpt, cmpPPPMode);
}
	
int pptpv6PktGen_errFlag_ipver(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type, uint32 errFlag,
	int ctagif,int cvid,int cpri,
	int stagif,int svid,int spri, int tos, int pptp_tos,
	cputag_t *cputag, uint8 TCPflag,
	int pppoetagif,int pppoe_session_id,int ttl,int pptp_ttl,int pptp_fragment,
	int pptp_callid,int pptp_seqno,int pptp_ackno, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t pptp_cmpPPPMode) 
{
	return pptpv6PktGen_gerneral_errFlag(pktBuf, pktData, pktLen, type, errFlag, 3, 20, 5,
							ctagif, cvid, cpri, 
							stagif, svid, spri, tos, pptp_tos, 
							cputag, TCPflag, 0, 0,
							pppoetagif, pppoe_session_id, ttl, pptp_ttl, pptp_fragment, 0,
							pptp_callid, pptp_seqno, pptp_ackno, reducePPTPOpt, pptp_cmpPPPMode);
}

int pptpv6PktGen_errFlag_iphdl(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type, uint32 errFlag,
	int ctagif,int cvid,int cpri,
	int stagif,int svid,int spri, int tos, int pptp_tos,
	cputag_t *cputag, uint8 TCPflag,
	int pppoetagif,int pppoe_session_id,int ttl,int pptp_ttl,int pptp_fragment,
	int pptp_callid,int pptp_seqno,int pptp_ackno, pptp_outerhdr_option_t reducePPTPOpt, compressed_ppp_mode_t pptp_cmpPPPMode) 
{
	return pptpv6PktGen_gerneral_errFlag(pktBuf, pktData, pktLen, type, errFlag, 4, 16, 6,
							ctagif, cvid, cpri, 
							stagif, svid, spri, tos, pptp_tos, 
							cputag, TCPflag, 0, 0,
							pppoetagif, pppoe_session_id, ttl, pptp_ttl, pptp_fragment, 0,
							pptp_callid, pptp_seqno, pptp_ackno, reducePPTPOpt, pptp_cmpPPPMode);
}

/* reduceL2TPLen: 0 to gen normal l2tp header */
int l2tpPktGen(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,
	int ctagif,int cvid,int cpri,
	int stagif,int svid,int spri, int dscp,
	cputag_t *cputag,
	int pppoetagif,int pppoe_session_id,int ttl,
	int l2tp_tunnelID,int l2tp_sesstionID,int ipv4_id, uint8 reduceL2TPLen, compressed_ppp_mode_t cmpPPPMode) 
{
	uint8 innerIsIPv4 = FALSE;
	uint8 *outerSrcIP = NULL, *outerDstIP = NULL;
	uint16 contBufSize = 128;
	uint16 shiftOffset = 0;
	uint16 extraHdrLen = 20+8+8+4;	// IPv4+UDP+L2TP+PPP
	_etherHdr_t *ether = NULL;
	ipHdr_t *ipv4h = NULL;
	udpHdr_t *udph = NULL;
	uint8 *outerHdr = rtlglue_malloc(contBufSize);
	bzero(outerHdr, contBufSize);

	if(reduceL2TPLen) extraHdrLen-=2;
	if(cmpPPPMode && (cmpPPPMode < COMPRESSED_PPP_END)) extraHdrLen-=cmpPPPMode;
		
	switch(type)
	{
		case TYPE_DUALHDR_L2TP_UNICAST_ROUTING_G2W:
		case TYPE_DUALHDR_L2TP_UNICAST_NAT_G2W:
		case TYPE_DUALHDR_L2TP_MULTICAST_ROUTING_G2W:
			innerIsIPv4 = TRUE;
			outerSrcIP = L2TP_CLIENT_IP;
			outerDstIP = L2TP_SERVER_IP;
			break;
		case TYPE_DUALHDR_L2TP_UNICAST_IPV6_G2W:
		case TYPE_DUALHDR_L2TP_MULTICAST_IPV6_G2W:
			innerIsIPv4 = FALSE;
			outerSrcIP = L2TP_CLIENT_IP;
			outerDstIP = L2TP_SERVER_IP;
			break;
		case TYPE_DUALHDR_L2TP_UNICAST_ROUTING_W2L:
		case TYPE_DUALHDR_L2TP_UNICAST_NAT_W2G:
		case TYPE_DUALHDR_L2TP_MULTICAST_ROUTING_W2G:
			innerIsIPv4 = TRUE;
			outerSrcIP = L2TP_SERVER_IP;
			outerDstIP = L2TP_CLIENT_IP;
			break;
		case TYPE_DUALHDR_L2TP_UNICAST_IPV6_W2L:
		case TYPE_DUALHDR_L2TP_MULTICAST_IPV6_W2G:
			innerIsIPv4 = FALSE;
			outerSrcIP = L2TP_SERVER_IP;
			outerDstIP = L2TP_CLIENT_IP;
			break;
		default:
#ifdef _MSG			
			TEST_WARNING("Not support this packet type");
#endif
			break;
	}

	if(pppoetagif)
		shiftOffset = sizeof(_etherHdr_t) + sizeof(_pppoeHdr_t);
	else
		shiftOffset = sizeof(_etherHdr_t);

	if(stagif)
		shiftOffset += sizeof(_vlanHdr_t);
	if(ctagif)
		shiftOffset += sizeof(_vlanHdr_t);
	if(cputag)
			shiftOffset += 8;

	ASSERT_EQ(normalPktGen(pktBuf,pktData, pktLen, type,
		ctagif, cvid, cpri,
		stagif, svid, spri, dscp,
		cputag, NULL,
		pppoetagif, pppoe_session_id, ttl), 0);
	*pktLen -= 4;	// packet CRC 
	l2tpOuterHdrGen(outerHdr, outerSrcIP, outerDstIP, *pktLen-shiftOffset+8+(4-cmpPPPMode), l2tp_tunnelID, l2tp_sesstionID, innerIsIPv4, extraHdrLen, reduceL2TPLen, cmpPPPMode);

	ipv4h = (ipHdr_t *)outerHdr;
	
	if(pppoetagif){	 	
		_pppoeHdr_t *pppoePtr = (_pppoeHdr_t *)(pktBuf+shiftOffset-sizeof(_pppoeHdr_t));
		pppoePtr->length = htons(ntohs(pppoePtr->length) + extraHdrLen);
		if(!innerIsIPv4)	// original protocol is IPv6(0x0057), now change to IPv4
			pppoePtr->proto = htons(0x0021);
	}else{
		ether = (_etherHdr_t *)pktBuf;
		if(ctagif || stagif){
			_vlanHdr_t *vlanPtr = (_vlanHdr_t *)(pktBuf + shiftOffset - sizeof(_vlanHdr_t));
			if(!innerIsIPv4)	// original ethertype is IPv6 for inner header, now change to ipv4 for outer header
				vlanPtr->etherType= htons(0x0800);
		}else{
			if(!innerIsIPv4){	// original ethertype is IPv6 for inner header, now change to ipv4 for outer header
				if(cputag)*((&ether->etherType)+4) = htons(0x0800);	
				else
					ether->etherType = htons(0x0800);
			}
		}	
	}

	if(cmpPPPMode == COMPRESSED_PPP_FF03C021){	//care lcp length 8
		ipv4h->ip_id = htons(ipv4_id);
		ipv4h->ip_len = htons(*pktLen-shiftOffset+8);	//ignore padding or payload for l2tp lcp (hdr+lcp 8)
		ipv4h->ip_sum = model_ipChecksum((struct ip*)ipv4h);

		memset(pktBuf+shiftOffset, 0x0, *pktLen-shiftOffset+8);	//skip inner header copy to prevent different l4 checksum
		memcpy(pktBuf+shiftOffset, outerHdr, 20+8+10);	//IP+UDP+L2tp6+ppp4
		*pktLen += 8;
	}else{
		ipv4h->ip_id = htons(ipv4_id);
		ipv4h->ip_len = htons(*pktLen-shiftOffset+extraHdrLen);
		ipv4h->ip_sum = model_ipChecksum((struct ip*)ipv4h);
	
		/* Shift original ip header to inner header location */
		memmove(pktBuf+shiftOffset+extraHdrLen, pktBuf+shiftOffset, *pktLen-shiftOffset);	
		/* Insert outer ip header after insert_location */
		memcpy(pktBuf+shiftOffset, outerHdr, extraHdrLen);
		*pktLen += extraHdrLen;
	}

	udph = (udpHdr_t *)(pktBuf + shiftOffset + (ipv4h->ip_hl)*4);
	udph->uh_ulen = htons(ntohs(ipv4h->ip_len) - (ipv4h->ip_hl * 4));
	udph->uh_sum = model_udpChecksum((struct ip *)ipv4h, udph); 

	l2FormCrc(0, pktBuf, *pktLen);
	*pktLen += 4;	// packet CRC 

	rtlglue_free(outerHdr);
	return SUCCESS;
}

int dslitePktGen(uint8 *pktBuf,uint8 *pktData,int *pktLen,int type,
	int ctagif,int cvid,int cpri,
	int stagif,int svid,int spri, int dscp,
	cputag_t *cputag,
	int pppoetagif,int pppoe_session_id,int ttl,
	int l2tp_tunnelID,int l2tp_sesstionID,int ipv4_id)
{
	//uint8 *outerSrcIP = NULL, *outerDstIP = NULL;
	uint16 contBufSize = 128;
	uint16 shiftOffset = 0;
	uint32 extraHdrLen = 0;
	//_etherHdr_t *ether = NULL;
	ip6Hdr_t *ipv6h = NULL;
	//udpHdr_t *udph = NULL;
	uint8 *outerHdr = rtlglue_malloc(contBufSize);
	bzero(outerHdr, contBufSize);
		
	switch(type)
	{
		case TYPE_DUALHDR_DSLITE_UNICAST_ROUTING_G2W:
		case TYPE_DUALHDR_DSLITE_UNICAST_NAT_G2W:
			/* Outer header (IPv6) */
			ASSERT_EQ(normalPktGen(outerHdr,NULL,&extraHdrLen, TYPE_DUALHDR_DSLITE_UNICAST_IPV6_OUTER_G2W,
				0,0,0,
				0,0,0,0,
				NULL, NULL,
				0, PPPOE_SID_WAN_ISP_1, 254), 0);
			break;
		case TYPE_DUALHDR_DSLITE_MULTICAST_ROUTING_G2W:
			ASSERT_EQ(normalPktGen(outerHdr,NULL,&extraHdrLen, TYPE_DUALHDR_DSLITE_MULTICAST_IPV6_OUTER_G2W,
				0,0,0,
				0,0,0,0,
				NULL, NULL,
				0, PPPOE_SID_WAN_ISP_1, 254), 0);
			break;
		case TYPE_DUALHDR_DSLITE_UNICAST_ROUTING_W2L:
		case TYPE_DUALHDR_DSLITE_UNICAST_NAT_W2G:
			ASSERT_EQ(normalPktGen(outerHdr,NULL,&extraHdrLen, TYPE_DUALHDR_DSLITE_UNICAST_IPV6_OUTER_W2G,
				0,0,0,
				0,0,0,0,
				NULL, NULL,
				0, PPPOE_SID_WAN_ISP_1, 254), 0);
			break;
		case TYPE_DUALHDR_DSLITE_MULTICAST_ROUTING_W2G:
			ASSERT_EQ(normalPktGen(outerHdr,NULL,&extraHdrLen, TYPE_DUALHDR_DSLITE_MULTICAST_IPV6_OUTER_W2G,
				0,0,0,
				0,0,0,0,
				NULL, NULL,
				0, PPPOE_SID_WAN_ISP_1, 254), 0);
			break;
		default:
#ifdef _MSG			
			TEST_WARNING("Not support this packet type");
#endif
			break;
	}

	if(pppoetagif)
		shiftOffset = sizeof(_etherHdr_t) + sizeof(_pppoeHdr_t);
	else
		shiftOffset = sizeof(_etherHdr_t);

	if(stagif)
		shiftOffset += sizeof(_vlanHdr_t);
	if(ctagif)
		shiftOffset += sizeof(_vlanHdr_t);
	if(cputag)
		shiftOffset += 8;

	/* Inner packet */
	ASSERT_EQ(normalPktGen(pktBuf,pktData, pktLen, type,
		ctagif, cvid, cpri,
		stagif, svid, spri, dscp,
		cputag, NULL,
		pppoetagif, pppoe_session_id, ttl), 0);
	*pktLen -= 4;	// packet CRC 
	ipv6h = (ip6Hdr_t *)(outerHdr+sizeof(_etherHdr_t));
	
	if(pppoetagif){	 	
		_pppoeHdr_t *pppoePtr = (_pppoeHdr_t *)(pktBuf+shiftOffset-sizeof(_pppoeHdr_t));
		pppoePtr->length = htons(ntohs(pppoePtr->length) + 40);
		pppoePtr->proto = htons(0x0057);
	}else{
		//ether = (_etherHdr_t *)pktBuf;
		//ether->etherType = htons(0x86dd);		
		*(uint16 *)(pktBuf+shiftOffset-2) = htons(0x86dd);
	}
	ipv6h->ip_next = 4; // IPIP(4)
	ipv6h->ip_len = htons(*pktLen-shiftOffset);
	
	/* Shift original ip header to inner header location */
	memmove(pktBuf+shiftOffset+40, pktBuf+shiftOffset, *pktLen-shiftOffset);	
	/* Insert outer ip header after insert_location */
	memcpy(pktBuf+shiftOffset, ipv6h, 40);	// IPv6 header length
	*pktLen += 40;
	
	l2FormCrc(0, pktBuf, *pktLen);
	*pktLen += 4;	// packet CRC 
	return SUCCESS;
}

int pktGen_Insert_rxCpuTag(unsigned char *packet, _cpu_rx_Hdr_t *cpu_tag, uint32 *len){

	unsigned char *pktBuf=rtlglue_malloc(MAX_PKT_SIZE);
	if(pktBuf==NULL) return FAILED;
	bzero(pktBuf,MAX_PKT_SIZE);

	// copy pacekt to pktBuf to build packet with cpuTag
	memcpy(pktBuf, packet, 12); //copy DA, SA.
	memcpy(pktBuf+12, cpu_tag, sizeof(_cpu_rx_Hdr_t)); //insert cpu tag
	memcpy(pktBuf+12+sizeof(_cpu_rx_Hdr_t), packet+12, *len-12); //copy rest content.	
	*len +=sizeof(_cpu_rx_Hdr_t); //add cpuTag_len to len

	memcpy(packet, pktBuf, *len); //copy back original packet

#ifdef _MSG
	TEST_DEBUG("\t* insert CPU tag... len=%d \n",*len);
#endif

	if(pktBuf) rtlglue_free(pktBuf);	
	return SUCCESS;
}

int v4TcpUdpPktGen(uint8 *pktBuf,uint8 *pktData,int *pktLen,
	unsigned char *smac, unsigned char *dmac,
	unsigned char *sip, unsigned short sport,
	unsigned char *dip, unsigned short dport,
	int protocol,int ttl, int dscp,
	int ctagif,int cvid,int cpri,
	int stagif,int tpid, int svid,int spri,
	cputag_t *cputag, cputagTx_t *cputagTx,
	int pppoetagif,int pppoe_session_id)
{
	int i;
	rtl8651_PktConf_t conf;

	bzero(&conf, sizeof(conf));

	for (i=0;i<*pktLen;i++)
		pktData[i] = i&0xff;

	if(*pktLen > 8)
	{
		pktData[0] = 0x70;	//p
		pktData[1] = 0x61;	//a
		pktData[2] = 0x79;	//y
		pktData[3] = 0x6c;	//l
		pktData[4] = 0x6f;	//o
		pktData[5] = 0x61;	//a
		pktData[6] = 0x64;	//d
	}

	if (protocol == IPPROTO_UDP)
		conf.pktType	= _PKT_TYPE_UDP;
	else
		conf.pktType	= _PKT_TYPE_TCP;

	conf.ip.id		= 0x123;
	conf.ip.ttl 		= ttl;
	conf.ip.tos 		= dscp<<2;
	conf.payload.length 	= *pktLen;
	conf.ip.version		= 0x4;
	conf.ip.headerLen	= 20;
	conf.payload.content	= pktData;
	conf.pppoe.type 	= 0x0800;
	conf.pppoe.session	= 0;
	conf.cvlan.cprio	= 1;
	conf.l2Flag 		= 0;

	//conf.conf_tcp_flag = TH_RST;
	if (ctagif) {
		conf.l2Flag 		= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 	= cvid;
		conf.cvlan.cprio	= cpri;
	}

	if (stagif) {
		conf.l2Flag 		= conf.l2Flag | L2_SVLAN;
		conf.svlan.svid 	= svid;
		conf.svlan.sprio	= spri;
		if(tpid)
			conf.svlan.tpid		 = tpid;
		else
			conf.svlan.tpid = 0x88a8;
	}

	if (cputag!=NULL) {
		conf.l2Flag 		= conf.l2Flag | L2_CPUTAG_OTHER;
		conf.conf_ctag_protocol	= 0x4;
		conf.conf_ctag_reason	= cputag->res;
		conf.conf_ctag_priority	= cputag->pri;
		conf.conf_ctag_streamid = cputag->streamid;
//		conf.conf_ctag_ttlde	= cputag->ttl_1;
		conf.conf_ctag_l3r	= cputag->l3r;
		conf.conf_ctag_org	= cputag->org;
		conf.conf_ctag_spa	= cputag->spa;
		conf.conf_ctag_extspa	= cputag->extspa;
		conf.conf_ctag_extdpmsk	= cputag->epmsk;
		conf.conf_ctag_fbi = cputag->fbi;
		conf.conf_ctag_fb_index = cputag->fbindex;
	}else if (cputagTx != NULL) {
		conf.l2Flag 		= conf.l2Flag | L2_CPUTAG_TX2;
		conf.conf_ctag_protocol	= 0x4;
		conf.ctag_tx2.txmsk = cputagTx->txmsk;
		conf.ctag_tx2.prisel = cputagTx->prisel;
		conf.ctag_tx2.pri = cputagTx->pri;
		conf.ctag_tx2.keep = cputagTx->keep;
		conf.ctag_tx2.dislrn = cputagTx->dislrn;
		conf.ctag_tx2.psel = cputagTx->psel;
		conf.ctag_tx2.directTx = cputagTx->directTx;
		conf.ctag_tx2.extspa = cputagTx->extspa;
		conf.ctag_tx2.pppoeact = cputagTx->pppoeact;
		conf.ctag_tx2.pppoeidx = cputagTx->pppoeidx;
		conf.ctag_tx2.streamid = cputagTx->streamid;

	}

	if (pppoetagif==1) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8864;
		conf.pppoe.session 	= pppoe_session_id;
	}else if(pppoetagif==2) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8863;
		conf.pppoe.session 	= pppoe_session_id;
	}

	//strtomac((void *)&conf.conf_smac[0], smac);
	//strtomac((void *)&conf.conf_dmac[0], dmac);
	memcpy((void *)&conf.conf_smac[0],&smac[0],6);
	memcpy((void *)&conf.conf_dmac[0],&dmac[0],6);
	conf.conf_sip	= ntohl(inet_addr(sip));
	conf.conf_dip	= ntohl(inet_addr(dip));
	conf.conf_sport = sport;
	conf.conf_dport = dport;

	*pktLen = pktGen(&conf, (int8*)pktBuf);			 /* generate pkt in buff */

	*pktLen += 4;	// packet CRC

	return 0;
}


struct ipv6hdr2 {
#ifndef _LITTLE_ENDIAN	//big endian
	
		uint32 version:4;	
		uint32 tclassH:4;
	
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
	
		uint32	payload_len:16;
		uint32	nexthdr:8;
		uint32	hop_limit:8;
	
		uint8  saddr[16];
		uint8  daddr[16];
};




int v4TcpUdpPktGen2(
	uint8 *pktBuf,uint8 *pktData,int *pktLen,int pktType,
	unsigned char *smac, unsigned char *dmac,
	uint32 sip, uint32 dip,
	unsigned short sport,unsigned short dport,
	int ttl, int tos,
	int ctagif,int cvid,int cpri,
	int stagif,int tpid, int svid,int spri,
	int pppoetagif,int pppoe_session_id)
{
	int i;
	rtl8651_PktConf_t conf;

	bzero(&conf, sizeof(conf));

	for(i=0;i<*pktLen;i++)
		pktData[i] = i&0xff;


	if(*pktLen > 8)
	{
		pktData[0] = 0x70;	//p
		pktData[1] = 0x61;	//a
		pktData[2] = 0x79;	//y
		pktData[3] = 0x6c;	//l
		pktData[4] = 0x6f;	//o
		pktData[5] = 0x61;	//a
		pktData[6] = 0x64;	//d
	}

	conf.pktType	= pktType;
	conf.ip.id		= 0x123;
	conf.ip.ttl 		= ttl;
	conf.ip.tos 		= tos;
	conf.payload.length 	= *pktLen;
	conf.ip.version		= 0x4;
	conf.ip.headerLen	= 20;
	conf.payload.content	= pktData;
	conf.pppoe.type 	= 0x0800;
	conf.pppoe.session	= 0;
	conf.cvlan.cprio	= 1;
	conf.l2Flag 		= 0;

	//conf.conf_tcp_flag = TH_RST;
	if (ctagif) {
		conf.l2Flag 		= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 	= cvid;
		conf.cvlan.cprio	= cpri;
	}

	if (stagif) {
		conf.l2Flag 		= conf.l2Flag | L2_SVLAN;
		conf.svlan.svid 	= svid;
		conf.svlan.sprio	= spri;
		if(tpid)
			conf.svlan.tpid		 = tpid;
		else
			conf.svlan.tpid = 0x88a8;
	}

	if(inner_insert_len_snap_after_vlan || inner_insert_len_snap_before_vlan)
		conf.l2Flag |= L2_SNAP;

	if (pppoetagif==1) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8864;
		conf.pppoe.session 	= pppoe_session_id;
	}else if(pppoetagif==2) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8863;
		conf.pppoe.session 	= pppoe_session_id;
	}

	memcpy((void *)&conf.conf_smac[0],&smac[0],6);
	memcpy((void *)&conf.conf_dmac[0],&dmac[0],6);
	conf.conf_sip	= sip;
	conf.conf_dip	= dip;
	conf.conf_sport = sport;
	conf.conf_dport = dport;

	pppoe_length_exclude_padding = inner_ppp_length_exclude_padding;
	pppoe_length_include_padding = inner_ppp_length_include_padding;
	//printk("pppoe_length_exclude_padding:%d @ %s %d\n",pppoe_length_exclude_padding,__func__,__LINE__);
	ipv4_length_exclude_padding = inner_ipv4_len_exclude_padding;
	//printk("ipv4_length_exclude_padding:%d @ %s %d\n",ipv4_length_exclude_padding,__func__,__LINE__);
	skip_minPktLen_check = inner_skip_minPktLen_check;
//	printk("skip_minPktLen_check:%d @ %s %d\n",skip_minPktLen_check,__func__,__LINE__);
	llc_len_including_padding = inner_llc_len_include_ingress_padding;
	*pktLen = pktGen(&conf, (int8*)pktBuf);			 /* generate pkt in buff */
	/* generate pkt in buff */
	if(inner_tail_padding_len > 0)
	{
//		printk("inner_tail_padding_len:%d @ %s %d\n",inner_tail_padding_len,__func__,__LINE__);
		memset(pktBuf+*pktLen,0,inner_tail_padding_len);
		*pktLen += inner_tail_padding_len;
	}
	llc_len_excluding_padding = 0;
	pppoe_length_exclude_padding = 0;
	pppoe_length_include_padding = 0;
	ipv4_length_exclude_padding = 0;
	skip_minPktLen_check = 0;

	if(inner_insert_len_snap_before_vlan && (ctagif || stagif))
	{
		unsigned char lenSnap[8];
		unsigned char tag[8];
		unsigned short l2Len;
		int lenTag = (ctagif?4:0) + (stagif?4:0);
		
		memcpy(&tag[0], pktBuf+12, lenTag);
		memcpy(&lenSnap[0], pktBuf+12+lenTag, 8);
		l2Len = (lenSnap[0]<<8|lenSnap[1]);
		l2Len += lenTag;
		lenSnap[0] = (l2Len>>8)&0xff;
		lenSnap[1] = l2Len&0xff;
		memcpy(pktBuf + 12, &lenSnap[0], 8);
		memcpy(pktBuf + 20, &tag[0], lenTag);
	}

	*pktLen += 4;	// packet CRC

	//test_dump_packet(pktBuf,*pktLen,"Packet");


	return 0;
}

int v4TcpUdpPktGen5(
	uint8 *pktBuf,uint8 *pktData,int *pktLen,int pktType,
	unsigned char *smac, unsigned char *dmac,
	uint32 sip, uint32 dip,
	unsigned short sport,unsigned short dport,
	int ttl, int tos,
	int ctagif,int cvid,int cpri,
	int stagif,int tpid, int svid,int spri,
	int pppoetagif,int pppoe_session_id,uint16 ipid)
{
	int i;
	rtl8651_PktConf_t conf;

	bzero(&conf, sizeof(conf));

	for(i=0;i<*pktLen;i++)
		pktData[i] = i&0xff;


	if(*pktLen > 8)
	{
		pktData[0] = 0x70;	//p
		pktData[1] = 0x61;	//a
		pktData[2] = 0x79;	//y
		pktData[3] = 0x6c;	//l
		pktData[4] = 0x6f;	//o
		pktData[5] = 0x61;	//a
		pktData[6] = 0x64;	//d
	}

	conf.pktType	= pktType;
	conf.ip.id		= ipid;
	conf.ip.ttl 		= ttl;
	conf.ip.tos 		= tos;
	conf.payload.length 	= *pktLen;
	conf.ip.version 	= 0x4;
	conf.ip.headerLen	= 20;
	conf.payload.content	= pktData;
	conf.pppoe.type 	= 0x0800;
	conf.pppoe.session	= 0;
	conf.cvlan.cprio	= 1;
	conf.l2Flag 		= 0;

	//conf.conf_tcp_flag = TH_RST;
	if (ctagif) {
		conf.l2Flag 		= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 	= cvid;
		conf.cvlan.cprio	= cpri;
	}

	if (stagif) {
		conf.l2Flag 		= conf.l2Flag | L2_SVLAN;
		conf.svlan.svid 	= svid;
		conf.svlan.sprio	= spri;
		if(tpid)
			conf.svlan.tpid 	 = tpid;
		else
			conf.svlan.tpid = 0x88a8;
	}


	if (pppoetagif==1) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8864;
		conf.pppoe.session	= pppoe_session_id;
	}else if(pppoetagif==2) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8863;
		conf.pppoe.session	= pppoe_session_id;
	}

	memcpy((void *)&conf.conf_smac[0],&smac[0],6);
	memcpy((void *)&conf.conf_dmac[0],&dmac[0],6);
	conf.conf_sip	= sip;
	conf.conf_dip	= dip;
	conf.conf_sport = sport;
	conf.conf_dport = dport;

	*pktLen = pktGen(&conf, (int8*)pktBuf); 		 /* generate pkt in buff */

	*pktLen += 4;	// packet CRC

	return 0;
}


int v4TcpUdpPktGen2ContentZero(
	uint8 *pktBuf,uint8 *pktData,int *pktLen,int pktType,
	unsigned char *smac, unsigned char *dmac,
	uint32 sip, uint32 dip,
	unsigned short sport,unsigned short dport,
	int ttl, int tos,
	int ctagif,int cvid,int cpri,
	int stagif,int tpid, int svid,int spri,
	int pppoetagif,int pppoe_session_id)
{
	int i;
	rtl8651_PktConf_t conf;

	bzero(&conf, sizeof(conf));

	for(i=0;i<*pktLen;i++)
		pktData[i] = 0x0;


	conf.pktType	= pktType;
	conf.ip.id		= 0x123;
	conf.ip.ttl 		= ttl;
	conf.ip.tos 		= tos;
	conf.payload.length 	= *pktLen;
	conf.ip.version 	= 0x4;
	conf.ip.headerLen	= 20;
	conf.payload.content	= pktData;
	conf.pppoe.type 	= 0x0800;
	conf.pppoe.session	= 0;
	conf.cvlan.cprio	= 1;
	conf.l2Flag 		= 0;

	//conf.conf_tcp_flag = TH_RST;
	if (ctagif) {
		conf.l2Flag 		= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 	= cvid;
		conf.cvlan.cprio	= cpri;
	}

	if (stagif) {
		conf.l2Flag 		= conf.l2Flag | L2_SVLAN;
		conf.svlan.svid 	= svid;
		conf.svlan.sprio	= spri;
		if(tpid)
			conf.svlan.tpid 	 = tpid;
		else
			conf.svlan.tpid = 0x88a8;
	}


	if (pppoetagif==1) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8864;
		conf.pppoe.session	= pppoe_session_id;
	}else if(pppoetagif==2) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8863;
		conf.pppoe.session	= pppoe_session_id;
	}

	memcpy((void *)&conf.conf_smac[0],&smac[0],6);
	memcpy((void *)&conf.conf_dmac[0],&dmac[0],6);
	conf.conf_sip	= sip;
	conf.conf_dip	= dip;
	conf.conf_sport = sport;
	conf.conf_dport = dport;

	*pktLen = pktGen(&conf, (int8*)pktBuf); 		 /* generate pkt in buff */

	*pktLen += 4;	// packet CRC

	return 0;
}


int v4TcpUdpPktGen3(
	uint8 *pktBuf,uint8 *pktData,int *pktLen,int pktType,
	unsigned char *smac, unsigned char *dmac,
	uint32 sip, uint32 dip,
	unsigned short sport,unsigned short dport,
	int ttl, int dscp,
	int ctagif,int ctpid, int cvid,int cpri,int cdei,
	int stagif,int stpid, int svid,int spri,int sdei,
	int pppoetagif,int pppoe_session_id)

{
	int i;
	rtl8651_PktConf_t conf;

	bzero(&conf, sizeof(conf));

	for(i=0;i<*pktLen;i++)
		pktData[i] = i&0xff;


	if(*pktLen > 8)
	{
		pktData[0] = 0x70;	//p
		pktData[1] = 0x61;	//a
		pktData[2] = 0x79;	//y
		pktData[3] = 0x6c;	//l
		pktData[4] = 0x6f;	//o
		pktData[5] = 0x61;	//a
		pktData[6] = 0x64;	//d
	}


	conf.pktType	= pktType;
	conf.ip.id		= 0x123;
	conf.ip.ttl 		= ttl;
	conf.ip.tos 		= dscp<<2;
	conf.payload.length 	= *pktLen;
	conf.ip.version 	= 0x4;
	conf.ip.headerLen	= 20;
	conf.payload.content	= pktData;
	conf.pppoe.type 	= 0x0800;
	conf.pppoe.session	= 0;
	conf.cvlan.cprio	= 1;
	conf.l2Flag 		= 0;

	//conf.conf_tcp_flag = TH_RST;
	/*
	if (ctagif) {
		conf.l2Flag 		= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 	= cvid;
		conf.cvlan.cprio	= cpri;
		conf.cvlan.cfi		= cdei;
		if(ctpid)
			conf.cvlan.tpid = ctpid;
		else
			conf.cvlan.tpid = 0x8100;
	}

	if (stagif) {
		conf.l2Flag 		= conf.l2Flag | L2_SVLAN;
		conf.svlan.svid 	= svid;
		conf.svlan.sprio	= spri;
		conf.svlan.dei		= sdei;
		if(stpid)
			conf.svlan.tpid = stpid;
		else
			conf.svlan.tpid = 0x88a8;
	}
*/

	if (pppoetagif==1) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8864;
		conf.pppoe.session	= pppoe_session_id;
	}else if(pppoetagif==2) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8863;
		conf.pppoe.session	= pppoe_session_id;
	}

	memcpy((void *)&conf.conf_smac[0],&smac[0],6);
	memcpy((void *)&conf.conf_dmac[0],&dmac[0],6);
	conf.conf_sip	= sip;
	conf.conf_dip	= dip;
	conf.conf_sport = sport;
	conf.conf_dport = dport;

	*pktLen = pktGen(&conf, (int8*)pktBuf); 		 /* generate pkt in buff */

	*pktLen += 4;	// packet CRC

	//inner vlan
	if(ctagif)
		insertTopVlanTags(pktBuf,pktLen,ctpid?ctpid:0x8100,cvid,cpri,cdei);
	//top vlan
	if(stagif)
		insertTopVlanTags(pktBuf,pktLen,stpid?stpid:0x88a8,svid,spri,sdei);




	return 0;
}


int v4TcpUdpPktGen4(
	uint8 *pktBuf,uint8 *pktData,int *pktLen,int pktType,uint8 tcpFlag,
	unsigned char *smac, unsigned char *dmac,
	uint32 sip, uint32 dip,
	unsigned short sport,unsigned short dport,
	int ttl, int dscp,
	int ctagif,int ctpid, int cvid,int cpri,int cdei,
	int stagif,int stpid, int svid,int spri,int sdei,
	int pppoetagif,int pppoe_session_id,
	uint8 ipmf ,uint8 fragOffset,uint16 tcsepq ,uint8 tcpDataOffsetSmall,uint8 v4Hdrlen)

{
	int i;
	rtl8651_PktConf_t conf;
	int _v4HdrLen=v4Hdrlen;

	bzero(&conf, sizeof(conf));

	for(i=0;i<*pktLen;i++)
		pktData[i] = i&0xff;


	if(*pktLen > 8)
	{
		pktData[0] = 0x70;	//p
		pktData[1] = 0x61;	//a
		pktData[2] = 0x79;	//y
		pktData[3] = 0x6c;	//l
		pktData[4] = 0x6f;	//o
		pktData[5] = 0x61;	//a
		pktData[6] = 0x64;	//d
	}


	if(_v4HdrLen<16)
		v4Hdrlen=20;

	conf.conf_tcp_flag		= (pktType==_PKT_TYPE_TCP)?tcpFlag:0;

	if(tcpDataOffsetSmall)
		conf.pktType			= _PKT_TYPE_TCP_SHORT_HDR;
	else
		conf.pktType			= pktType;

	conf.ip.id		= 0x123;
	conf.ip.ttl 		= ttl;
	conf.ip.tos 		= dscp;
	conf.payload.length 	= *pktLen;
	conf.ip.version 	= 0x4;
	conf.ip.headerLen	= v4Hdrlen;
	conf.payload.content	= pktData;
	conf.pppoe.type 	= 0x0800;
	conf.pppoe.session	= 0;
	conf.cvlan.cprio	= 1;
	conf.l2Flag 		= 0;
	conf.ip.mf				= ipmf;
	conf.ip.offset			= fragOffset;
	conf.l4hdr.tc.seq		= tcsepq;

	//conf.conf_tcp_flag = TH_RST;
	if (ctagif) {
		conf.l2Flag 		= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 	= cvid;
		conf.cvlan.cprio	= cpri;
		conf.cvlan.cfi		= cdei;
		if(ctpid)
			conf.cvlan.tpid = ctpid;
		else
			conf.cvlan.tpid = 0x8100;
	}

	if (stagif) {
		conf.l2Flag 		= conf.l2Flag | L2_SVLAN;
		conf.svlan.svid 	= svid;
		conf.svlan.sprio	= spri;
		conf.svlan.dei		= sdei;
		if(stpid)
			conf.svlan.tpid = stpid;
		else
			conf.svlan.tpid = 0x88a8;
	}


	if (pppoetagif==1) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8864;
		conf.pppoe.session	= pppoe_session_id;
	}else if(pppoetagif==2) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8863;
		conf.pppoe.session	= pppoe_session_id;
	}

	memcpy((void *)&conf.conf_smac[0],&smac[0],6);
	memcpy((void *)&conf.conf_dmac[0],&dmac[0],6);
	conf.conf_sip	= sip;
	conf.conf_dip	= dip;
	conf.conf_sport = sport;
	conf.conf_dport = dport;

	*pktLen = pktGen(&conf, (int8*)pktBuf); 		 /* generate pkt in buff */

	*pktLen += 4;	// packet CRC

	return 0;
}



int v6TcpUdpPktGen(
	uint8 *pktBuf,uint8 *pktData,int *pktLen,int pktType,
	unsigned char *smac, unsigned char *dmac,
	unsigned char *sip, unsigned char *dip,
	unsigned short sport,unsigned short dport,
	int hoplimit, int tc,
	int ctagif,int cvid,int cpri,
	int stagif,int tpid, int svid,int spri,
	int pppoetagif,int pppoe_session_id)
{
	int i;
	rtl8651_PktConf_t conf;

	bzero(&conf, sizeof(conf));

	for (i=0;i<*pktLen;i++)
		pktData[i] = i&0xff;

	if(*pktLen > 8)
	{
		pktData[0] = 0x70;	//p
		pktData[1] = 0x61;	//a
		pktData[2] = 0x79;	//y
		pktData[3] = 0x6c;	//l
		pktData[4] = 0x6f;	//o
		pktData[5] = 0x61;	//a
		pktData[6] = 0x64;	//d
	}

	conf.pktType	= pktType;
	conf.ipv6.   version = 6;
	conf.ipv6.priority		= tc;
	conf.ipv6.flowlbl[0]	= conf.ipv6.flowlbl[1]	= conf.ipv6.flowlbl[2] = 0;
	conf.payload.content	= pktData;	  
	conf.payload.length 	= *pktLen;
	//conf.ipv6.pldlen		= *pktLen;
	conf.ipv6.nxthdr			= 17;
	conf.ipv6.hoplmt		= hoplimit;

	//conf.conf_tcp_flag = TH_RST;
	if (ctagif) {
		conf.l2Flag 		= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 	= cvid;
		conf.cvlan.cprio	= cpri;
	}

	if (stagif) {
		conf.l2Flag 		= conf.l2Flag | L2_SVLAN;
		conf.svlan.svid 	= svid;
		conf.svlan.sprio	= spri;
		if(tpid)
			conf.svlan.tpid 	 = tpid;
		else
			conf.svlan.tpid = 0x88a8;
	}

	if(inner_insert_len_snap_after_vlan || inner_insert_len_snap_before_vlan)
		conf.l2Flag |= L2_SNAP;
	
	if (pppoetagif==1) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8864;
		conf.pppoe.session	= pppoe_session_id;
	}else if(pppoetagif==2) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8863;
		conf.pppoe.session	= pppoe_session_id;
	}

	memcpy((void *)&conf.conf_smac[0],&smac[0],6);
	memcpy((void *)&conf.conf_dmac[0],&dmac[0],6);
	in6_pton(sip,-1,(unsigned char *)&conf.ipv6.src_ip[0],-1,NULL);
	in6_pton(dip,-1,(unsigned char *)&conf.ipv6.dst_ip[0],-1,NULL);
	conf.conf_sport = sport;
	conf.conf_dport = dport;

	*pktLen = pktGen(&conf, (int8*)pktBuf); 		 /* generate pkt in buff */

	if(inner_insert_len_snap_before_vlan && (ctagif || stagif))
	{
		unsigned char lenSnap[8];
		unsigned char tag[8];
		unsigned short l2Len;
		int lenTag = (ctagif?4:0) + (stagif?4:0);
		
		memcpy(&tag[0], pktBuf+12, lenTag);
		memcpy(&lenSnap[0], pktBuf+12+lenTag, 8);
		l2Len = (lenSnap[0]<<8|lenSnap[1]);
		l2Len += lenTag;
		lenSnap[0] = (l2Len>>8)&0xff;
		lenSnap[1] = l2Len&0xff;
		memcpy(pktBuf + 12, &lenSnap[0], 8);
		memcpy(pktBuf + 20, &tag[0], lenTag);
		//dump_packet(pktBuf,*pktLen,"Packet with snap");
	}

	*pktLen += 4;	// packet CRC

	return 0;
}


/* v6sip v6dip by buffe[16]*/
int v6TcpUdpPktGen2(
	uint8 *pktBuf,uint8 *pktData,int *pktLen,int pktType,
	unsigned char *smac, unsigned char *dmac,
	unsigned char *sip, unsigned char *dip,
	unsigned short sport,unsigned short dport,
	int hoplimit, int tc,
	int ctagif,int cvid,int cpri,
	int stagif,int tpid, int svid,int spri,
	int pppoetagif,int pppoe_session_id)
{
	int i;
	rtl8651_PktConf_t conf;

	bzero(&conf, sizeof(conf));

	for (i=0;i<*pktLen;i++)
		pktData[i] = i&0xff;

	if(*pktLen > 8)
	{
		pktData[0] = 0x70;	//p
		pktData[1] = 0x61;	//a
		pktData[2] = 0x79;	//y
		pktData[3] = 0x6c;	//l
		pktData[4] = 0x6f;	//o
		pktData[5] = 0x61;	//a
		pktData[6] = 0x64;	//d
	}

	conf.pktType	= pktType;
	conf.ipv6.	 version = 6;
	conf.ipv6.priority		= tc;
	conf.ipv6.flowlbl[0]	= conf.ipv6.flowlbl[1]	= conf.ipv6.flowlbl[2] = 0;
	conf.payload.content	= pktData;	  
	conf.payload.length 	= *pktLen;
	//conf.ipv6.pldlen		= *pktLen;
	conf.ipv6.nxthdr			= 17;
	conf.ipv6.hoplmt		= hoplimit;

	//conf.conf_tcp_flag = TH_RST;
	if (ctagif) {
		conf.l2Flag 		= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 	= cvid;
		conf.cvlan.cprio	= cpri;
	}

	if (stagif) {
		conf.l2Flag 		= conf.l2Flag | L2_SVLAN;
		conf.svlan.svid 	= svid;
		conf.svlan.sprio	= spri;
		if(tpid)
			conf.svlan.tpid 	 = tpid;
		else
			conf.svlan.tpid = 0x88a8;
	}
	
	if (pppoetagif==1) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8864;
		conf.pppoe.session	= pppoe_session_id;
	}else if(pppoetagif==2) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8863;
		conf.pppoe.session	= pppoe_session_id;
	}

	memcpy((void *)&conf.conf_smac[0],&smac[0],6);
	memcpy((void *)&conf.conf_dmac[0],&dmac[0],6);
	memcpy(conf.ipv6.src_ip,sip,16);
	memcpy(conf.ipv6.dst_ip,dip,16);
	conf.conf_sport = sport;
	conf.conf_dport = dport;

	*pktLen = pktGen(&conf, (int8*)pktBuf); 		 /* generate pkt in buff */

	*pktLen += 4;	// packet CRC

	return 0;
}


/* v6sip v6dip by buffe[16]*/
int v6TcpUdpPktGen3(
	uint8 *pktBuf,uint8 *pktData,int *pktLen,int pktType,
	unsigned char *smac, unsigned char *dmac,
	unsigned char *sip, unsigned char *dip,
	unsigned short sport,unsigned short dport,
	int hoplimit, int tc,
	int ctagif,int cvid,int cpri,int cdei,
	int stagif,int tpid, int svid,int spri,int sdei,
	int pppoetagif,int pppoe_session_id)
{
	int i;
	rtl8651_PktConf_t conf;

	bzero(&conf, sizeof(conf));

	for (i=0;i<*pktLen;i++)
		pktData[i] = i&0xff;

	if(*pktLen > 8)
	{
		pktData[0] = 0x70;	//p
		pktData[1] = 0x61;	//a
		pktData[2] = 0x79;	//y
		pktData[3] = 0x6c;	//l
		pktData[4] = 0x6f;	//o
		pktData[5] = 0x61;	//a
		pktData[6] = 0x64;	//d
	}

	conf.pktType	= pktType;
	conf.ipv6.	 version = 6;
	conf.ipv6.priority		= tc;
	conf.ipv6.flowlbl[0]	= conf.ipv6.flowlbl[1]	= conf.ipv6.flowlbl[2] = 0;
	conf.payload.content	= pktData;	  
	conf.payload.length 	= *pktLen;
	//conf.ipv6.pldlen		= *pktLen;
	conf.ipv6.nxthdr			= 17;
	conf.ipv6.hoplmt		= hoplimit;

	//conf.conf_tcp_flag = TH_RST;
	if (ctagif) {
		conf.l2Flag 		= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 	= cvid;
		conf.cvlan.cprio	= cpri;
		conf.cvlan.cfi		= cdei;
	}

	if (stagif) {
		conf.l2Flag 		= conf.l2Flag | L2_SVLAN;
		conf.svlan.svid 	= svid;
		conf.svlan.sprio	= spri;
		conf.svlan.dei		= sdei;
		if(tpid)
			conf.svlan.tpid 	 = tpid;
		else
			conf.svlan.tpid = 0x88a8;
	}
	
	if (pppoetagif==1) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8864;
		conf.pppoe.session	= pppoe_session_id;
	}else if(pppoetagif==2) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8863;
		conf.pppoe.session	= pppoe_session_id;
	}

	memcpy((void *)&conf.conf_smac[0],&smac[0],6);
	memcpy((void *)&conf.conf_dmac[0],&dmac[0],6);
	memcpy(conf.ipv6.src_ip,sip,16);
	memcpy(conf.ipv6.dst_ip,dip,16);
	conf.conf_sport = sport;
	conf.conf_dport = dport;

	*pktLen = pktGen(&conf, (int8*)pktBuf); 		 /* generate pkt in buff */

	*pktLen += 4;	// packet CRC

	return 0;
}


/* v6sip v6dip by buffe[16]*/
int v6TcpUdpPktGen4(
	uint8 *pktBuf,uint8 *pktData,int *pktLen,int pktType,
	unsigned char *smac, unsigned char *dmac,
	unsigned char *sip, unsigned char *dip,
	unsigned short sport,unsigned short dport,
	int hoplimit, int tc,
	int ctagif,int ctpid, int cvid,int cpri,int cdei,
	int stagif,int stpid, int svid,int spri,int sdei,
	int pppoetagif,int pppoe_session_id)
{
	int i;
	rtl8651_PktConf_t conf;

	bzero(&conf, sizeof(conf));

	for (i=0;i<*pktLen;i++)
		pktData[i] = i&0xff;

	if(*pktLen > 8)
	{
		pktData[0] = 0x70;	//p
		pktData[1] = 0x61;	//a
		pktData[2] = 0x79;	//y
		pktData[3] = 0x6c;	//l
		pktData[4] = 0x6f;	//o
		pktData[5] = 0x61;	//a
		pktData[6] = 0x64;	//d
	}

	conf.pktType	= pktType;
	conf.ipv6.	 version = 6;
	conf.ipv6.priority		= tc;
	conf.ipv6.flowlbl[0]	= conf.ipv6.flowlbl[1]	= conf.ipv6.flowlbl[2] = 0;
	conf.payload.content	= pktData;	  
	conf.payload.length 	= *pktLen;
	//conf.ipv6.pldlen		= *pktLen;
	conf.ipv6.nxthdr			= 17;
	conf.ipv6.hoplmt		= hoplimit;

	//conf.conf_tcp_flag = TH_RST;
/*	
	if (ctagif) {
		conf.l2Flag 		= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 	= cvid;
		conf.cvlan.cprio	= cpri;
		conf.cvlan.cfi		= cdei;
		if(ctpid)
			conf.cvlan.tpid = ctpid;
		else
			conf.cvlan.tpid = 0x8100;		
	}

	if (stagif) {
		conf.l2Flag 		= conf.l2Flag | L2_SVLAN;
		conf.svlan.svid 	= svid;
		conf.svlan.sprio	= spri;
		conf.svlan.dei		= sdei;
		if(stpid)
			conf.svlan.tpid = stpid;
		else
			conf.svlan.tpid = 0x88a8;
	}
*/
	
	if (pppoetagif==1) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8864;
		conf.pppoe.session	= pppoe_session_id;
	}else if(pppoetagif==2) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8863;
		conf.pppoe.session	= pppoe_session_id;
	}

	memcpy((void *)&conf.conf_smac[0],&smac[0],6);
	memcpy((void *)&conf.conf_dmac[0],&dmac[0],6);
	memcpy(conf.ipv6.src_ip,sip,16);
	memcpy(conf.ipv6.dst_ip,dip,16);
	conf.conf_sport = sport;
	conf.conf_dport = dport;

	*pktLen = pktGen(&conf, (int8*)pktBuf); 		 /* generate pkt in buff */

	*pktLen += 4;	// packet CRC


	//inner vlan
	if(ctagif)
		insertTopVlanTags(pktBuf,pktLen,ctpid?ctpid:0x8100,cvid,cpri,cdei);
	//top vlan
	if(stagif)
		insertTopVlanTags(pktBuf,pktLen,stpid?stpid:0x88a8,svid,spri,sdei);




	return 0;
}



#define MAX_PKT_LEN				(65535 + 128)
unsigned char v6PktBuf[MAX_PKT_LEN];
unsigned char tmpData[MAX_PKT_LEN];

int v6rdPktGenByPatten(uint8 *pktBuf, uint8 *pktData, int *pktLen, uint32 pktType, uint8 tcpFlag,
		uint8* pktsmac, uint8* pktdmac,
		uint8* pktV6sip, uint8 *pktV6dip,
		uint32 pktsip, uint32 pktdip,
		uint16 sport, uint16 dport,
		int ctagif,int cvid,int cpri,
		int stagif,int svid,int spri, 
		uint8 pppoeTagif,uint16 pppoeSID, 
		uint16 ipid, uint8 ttl, uint8 hoplimit,	uint8 mf_frag, uint8 tos, uint8 tc) 
{

	int i;
	rtl8651_PktConf_t conf;
	uint32 ipv4Offset = 14; //da+sa+etherType
	int v4PktLen,v6PktLen,v6rdPktLen;

	
	for (i=0;i<*pktLen;i++)
		pktData[i] = i&0xff;

	if(*pktLen > 8)
	{
		pktData[0] = 0x70;	//p
		pktData[1] = 0x61;	//a
		pktData[2] = 0x79;	//y
		pktData[3] = 0x6c;	//l
		pktData[4] = 0x6f;	//o
		pktData[5] = 0x61;	//a
		pktData[6] = 0x64;	//d
	}

	//IPv6
	bzero(&conf,sizeof(conf));
	conf.pktType			= pktType;
	conf.conf_tcp_flag	= (pktType==_PKT_TYPE_V6TCP)?tcpFlag:0;
	conf.ipv6.version		= 6;
	conf.payload.content	= pktData;
	conf.payload.length 	= *pktLen;
	conf.rhdr.hdrlen		= 0;
	conf.ipv6.priority		= tc;
	conf.ipv6.flowlbl[0]	= conf.ipv6.flowlbl[1]	= conf.ipv6.flowlbl[2] = 0;
	conf.ipv6.pldlen		= *pktLen;
	//conf.ipv6.nxthdr			= 2;		//IGMPv2
	conf.ipv6.hoplmt		= hoplimit;
	
	//set sip & port
	memcpy(conf.conf_v6sip, pktV6sip, 16);
	conf.conf_sport 		= (sport==0)?0x200:sport;

	//set dip &port
	memcpy(conf.conf_v6dip, pktV6dip, 16);
	conf.conf_dport 		= (dport==0)?0x300:dport;

	memcpy(conf.conf_smac, pktsmac, 6);
	memcpy(conf.conf_dmac, pktdmac, 6);

	v6PktLen = pktGen(&conf, (int8*)v6PktBuf); 		 /* generate pkt in buff */
	//dump_packet(v6PktBuf, v6PktLen, "\033[1;33;40m[tmp v6 Packet]\033[0m");

	//IPv4
	memset(&tmpData[0],0,MAX_PKT_LEN);
	memcpy(&tmpData[0],&v6PktBuf[14],v6PktLen-14);
	
	conf.pktType			= _PKT_TYPE_IP;
	conf.ip.version 		= 0x4;
	conf.payload.content	= tmpData;
	conf.payload.length 	= *pktLen + 40;	// v6 header
	if(pktType==_PKT_TYPE_V6TCP)
		conf.payload.length += 20;
	else if(pktType==_PKT_TYPE_V6UDP)
		conf.payload.length += 8;
	conf.ip.id				= ipid;
	conf.ip.ttl 			= ttl;
	conf.ip.headerLen		= 20;
	if(mf_frag > 1)
		conf.ip.offset = mf_frag;
	else if(mf_frag==1)
		conf.ip.mf			= mf_frag;
	else
		conf.ip.mf			= 0;
	conf.ip.tos 			= tos;
	conf.ip.protocol		= 41;	//IPv6 encapsulation

	conf.pppoe.type 		= 0x0800;
	conf.pppoe.session		= 0;
	conf.cvlan.cprio		= 0;
	conf.l2Flag 			= 0;

	//set sip & port
	conf.conf_sip			= pktsip;
	conf.conf_sport 		= (sport==0)?0x200:sport;

	//set dip &port
	conf.conf_dip			= pktdip;
	conf.conf_dport 		= (dport==0)?0x300:dport;

	memcpy(conf.conf_smac, pktsmac, 6); //don't care
	memcpy(conf.conf_dmac, pktdmac, 6); //don't care

	if(ctagif)
	{
		conf.l2Flag 					= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 				= cvid;
		conf.cvlan.cprio				= cpri;
		ipv4Offset += 4;
	}
	if(stagif)
	{
		conf.l2Flag 					= conf.l2Flag | L2_SVLAN ;
		conf.svlan.tpid 				= 0x88a8;		//etherType
		conf.svlan.svid 				= svid;
		conf.svlan.sprio				= spri;
		ipv4Offset += 4;
	}

	if(pppoeTagif==0)
	{
		conf.pppoe.type 		= 0x0800;
		conf.pppoe.session		= pppoeSID;
	}
	else
	{
		conf.l2Flag 			= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 		= 0x8864;
		conf.pppoe.session		= pppoeSID;
		ipv4Offset += 8;
	}

	v6rdPktLen = v4PktLen = pktGen(&conf, (int8*)pktBuf); 		 /* generate pkt in buff */
	//dump_packet(pktBuf, v4PktLen, "\033[1;33;40m[tmp v4 Packet]\033[0m");
	//memcpy(pktBuf+(ipv4Offset+20),v6PktBuf+14,v6PktLen);

	*pktLen = v6rdPktLen;

	l2FormCrc(0 , pktBuf, *pktLen);
	*pktLen += 4;

	//dump_packet(pktBuf, *pktLen, "\033[1;33;40m[output Packet]\033[0m");
	
	return 0;
}

int v6rdTCPPktGenByPatten(uint8 *pktBuf, uint8 *pktData, int *pktLen, uint32 pktType, 
		uint8 tcpFlag, int data_offset, int seq, int ack,
		uint8* pktsmac, uint8* pktdmac,
		uint8* pktV6sip, uint8 *pktV6dip,
		uint32 pktsip, uint32 pktdip,
		uint16 sport, uint16 dport,
		int ctagif,int cvid,int cpri,
		int stagif,int svid,int spri, 
		uint8 pppoeTagif,uint16 pppoeSID, 
		uint16 ipid, uint8 ttl, uint8 hoplimit, uint8 mf_frag, uint8 tos, uint8 tc) 
{

	int i;
	rtl8651_PktConf_t conf;
	uint32 ipv4Offset = 14; //da+sa+etherType
	int v4PktLen,v6PktLen,v6rdPktLen;
	unsigned char *v6PktBuf=kmalloc(2048+40, GFP_ATOMIC | __GFP_ZERO);
	unsigned char *tmpData=kmalloc(2048, GFP_ATOMIC | __GFP_ZERO);


	
	for (i=0;i<*pktLen;i++)
		pktData[i] = i&0xff;

	if(*pktLen > 8)
	{
		pktData[0] = 0x70;	//p
		pktData[1] = 0x61;	//a
		pktData[2] = 0x79;	//y
		pktData[3] = 0x6c;	//l
		pktData[4] = 0x6f;	//o
		pktData[5] = 0x61;	//a
		pktData[6] = 0x64;	//d
	}

	//IPv6
	bzero(&conf,sizeof(conf));
	conf.pktType			= pktType;
	conf.ipv6.version		= 6;
	conf.payload.content	= pktData;
	conf.payload.length 	= *pktLen;
	conf.rhdr.hdrlen		= 0;
	conf.ipv6.priority		= tc;
	conf.ipv6.flowlbl[0]	= conf.ipv6.flowlbl[1]	= conf.ipv6.flowlbl[2] = 0;
	conf.ipv6.pldlen		= *pktLen;
	//conf.ipv6.nxthdr			= 2;		//IGMPv2
	conf.ipv6.hoplmt		= hoplimit;
	
	//set sip & port
	memcpy(conf.conf_v6sip, pktV6sip, 16);
	conf.conf_sport 		= (sport==0)?0x200:sport;

	//set dip &port
	memcpy(conf.conf_v6dip, pktV6dip, 16);
	conf.conf_dport 		= (dport==0)?0x300:dport;

	memcpy(conf.conf_smac, pktsmac, 6);
	memcpy(conf.conf_dmac, pktdmac, 6);

	//TCP
	conf.conf_tcp_flag	= (pktType==_PKT_TYPE_V6TCP)?tcpFlag:0;
	conf.conf_tcp_seq = seq;
	conf.conf_tcp_ack = ack;
	conf.conf_tcp_offset = data_offset;

	v6PktLen = pktGen(&conf, (int8*)v6PktBuf); 		 /* generate pkt in buff */
	//dump_packet(v6PktBuf, v6PktLen, "\033[1;33;40m[tmp v6 Packet]\033[0m");

	//IPv4
	memset(&tmpData[0],0,2048);
	memcpy(&tmpData[0],&v6PktBuf[14],v6PktLen-14);
	
	conf.pktType			= _PKT_TYPE_IP;
	conf.ip.version 		= 0x4;
	conf.payload.content	= tmpData;
	conf.payload.length 	= *pktLen + 40;	// v6 header
	if(pktType==_PKT_TYPE_V6TCP)
		conf.payload.length += 20;
	else if(pktType==_PKT_TYPE_V6UDP)
		conf.payload.length += 8;
	conf.ip.id				= ipid;
	conf.ip.ttl 			= ttl;
	conf.ip.headerLen		= 20;
	if(mf_frag > 1)
		conf.ip.offset = mf_frag;
	else if(mf_frag==1)
		conf.ip.mf			= mf_frag;
	else
		conf.ip.mf			= 0;
	conf.ip.tos 			= tos;
	conf.ip.protocol		= 41;	//IPv6 encapsulation

	conf.pppoe.type 		= 0x0800;
	conf.pppoe.session		= 0;
	conf.cvlan.cprio		= 0;
	conf.l2Flag 			= 0;

	//set sip & port
	conf.conf_sip			= pktsip;
	conf.conf_sport 		= (sport==0)?0x200:sport;

	//set dip &port
	conf.conf_dip			= pktdip;
	conf.conf_dport 		= (dport==0)?0x300:dport;

	memcpy(conf.conf_smac, pktsmac, 6); //don't care
	memcpy(conf.conf_dmac, pktdmac, 6); //don't care

	if(ctagif)
	{
		conf.l2Flag 					= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 				= cvid;
		conf.cvlan.cprio				= cpri;
		ipv4Offset += 4;
	}
	if(stagif)
	{
		conf.l2Flag 					= conf.l2Flag | L2_SVLAN ;
		conf.svlan.tpid 				= 0x88a8;		//etherType
		conf.svlan.svid 				= svid;
		conf.svlan.sprio				= spri;
		ipv4Offset += 4;
	}

	if(pppoeTagif==0)
	{
		conf.pppoe.type 		= 0x0800;
		conf.pppoe.session		= pppoeSID;
	}
	else
	{
		conf.l2Flag 			= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 		= 0x8864;
		conf.pppoe.session		= pppoeSID;
		ipv4Offset += 8;
	}

	v6rdPktLen = v4PktLen = pktGen(&conf, (int8*)pktBuf); 		 /* generate pkt in buff */
	//dump_packet(pktBuf, v4PktLen, "\033[1;33;40m[tmp v4 Packet]\033[0m");
	//memcpy(pktBuf+(ipv4Offset+20),v6PktBuf+14,v6PktLen);

	*pktLen = v6rdPktLen;

	l2FormCrc(0 , pktBuf, *pktLen);
	*pktLen += 4;

	//dump_packet(pktBuf, *pktLen, "\033[1;33;40m[output Packet]\033[0m");
	kfree(v6PktBuf);
	kfree(tmpData);
	return 0;
}

int v6rdFragmentPktGenByPatten(uint8 *pktBuf, uint8 *pktData, int *pktLen, uint32 pktType, uint8 tcpFlag,
		uint8* pktsmac, uint8* pktdmac,
		uint8* pktV6sip, uint8 *pktV6dip,
		uint32 pktsip, uint32 pktdip,
		uint16 sport, uint16 dport,
		int ctagif,int cvid,int cpri,
		int stagif,int svid,int spri, 
		uint8 pppoeTagif,uint16 pppoeSID, 
		uint16 ipid, uint8 ttl, uint8 hoplimit, uint8 mf_frag, uint16 v6_mf_frag, uint16 v6_frag_id, uint8 tos, uint8 tc) 
{

	int i;
	rtl8651_PktConf_t conf;
	uint32 ipv4Offset = 14; //da+sa+etherType
	int v4PktLen,v6PktLen,v6rdPktLen;
	unsigned char *v6PktBuf=kmalloc(2048+40, GFP_ATOMIC | __GFP_ZERO);
	unsigned char *tmpData=kmalloc(2048, GFP_ATOMIC | __GFP_ZERO);

	
	for (i=0;i<*pktLen;i++)
		pktData[i] = i&0xff;

	if(*pktLen > 8)
	{
		pktData[0] = 0x70;	//p
		pktData[1] = 0x61;	//a
		pktData[2] = 0x79;	//y
		pktData[3] = 0x6c;	//l
		pktData[4] = 0x6f;	//o
		pktData[5] = 0x61;	//a
		pktData[6] = 0x64;	//d
	}

	//IPv6
	bzero(&conf,sizeof(conf));
	conf.pktType			= pktType;
	conf.conf_tcp_flag	= (pktType==_PKT_TYPE_V6TCP)?tcpFlag:0;
	conf.conf_tcp_seq = 0x1234;
	conf.conf_tcp_ack = 0x2345;
	conf.ipv6.version		= 6;
	conf.payload.content	= pktData;
	conf.payload.length 	= *pktLen;
	conf.rhdr.hdrlen		= 0;
	conf.ipv6.priority		= tc;
	conf.ipv6.flowlbl[0]	= conf.ipv6.flowlbl[1]	= conf.ipv6.flowlbl[2] = 0;
	conf.ipv6.pldlen		= *pktLen;
	//conf.ipv6.nxthdr			= 2;		//IGMPv2
	conf.ipv6.hoplmt		= hoplimit;
	
	//set sip & port
	memcpy(conf.conf_v6sip, pktV6sip, 16);
	conf.conf_sport 		= (sport==0)?0x200:sport;

	//set dip &port
	memcpy(conf.conf_v6dip, pktV6dip, 16);
	conf.conf_dport 		= (dport==0)?0x300:dport;

	memcpy(conf.conf_smac, pktsmac, 6);
	memcpy(conf.conf_dmac, pktdmac, 6);

	if(v6_mf_frag > 0)
	{
		conf.extHdr = FraHdr;
		if(v6_mf_frag == 1)
			conf.frghdr.fragment = 0x1;
		else
			conf.frghdr.fragment = (v6_mf_frag-1)<<3;
		conf.frghdr.id = v6_frag_id;
	}

	v6PktLen = pktGen(&conf, (int8*)v6PktBuf);		 /* generate pkt in buff */
	//dump_packet(v6PktBuf, v6PktLen, "\033[1;33;40m[tmp v6 Packet]\033[0m");

	//IPv4
	memset(&tmpData[0],0,2048);
	memcpy(&tmpData[0],&v6PktBuf[14],v6PktLen-14);
	
	conf.pktType			= _PKT_TYPE_IP;
	conf.ip.version 		= 0x4;
	conf.payload.content	= tmpData;
	conf.payload.length 	= v6PktLen - 14; // v6 header
	if(pktType==_PKT_TYPE_V6TCP)
		conf.payload.length += 20;
	else if(pktType==_PKT_TYPE_V6UDP)
		conf.payload.length += 8;
	conf.ip.id				= ipid;
	conf.ip.ttl 			= ttl;
	conf.ip.headerLen		= 20;
	if(mf_frag > 1)
		conf.ip.offset = mf_frag-1;
	else if(mf_frag==1)
		conf.ip.mf			= mf_frag;
	else
		conf.ip.mf			= 0;
	conf.ip.tos 			= tos;
	conf.ip.protocol		= 41;	//IPv6 encapsulation

	conf.pppoe.type 		= 0x0800;
	conf.pppoe.session		= 0;
	conf.cvlan.cprio		= 0;
	conf.l2Flag 			= 0;

	//set sip & port
	conf.conf_sip			= pktsip;
	conf.conf_sport 		= (sport==0)?0x200:sport;

	//set dip &port
	conf.conf_dip			= pktdip;
	conf.conf_dport 		= (dport==0)?0x300:dport;

	memcpy(conf.conf_smac, pktsmac, 6); //don't care
	memcpy(conf.conf_dmac, pktdmac, 6); //don't care

	if(ctagif)
	{
		conf.l2Flag 					= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 				= cvid;
		conf.cvlan.cprio				= cpri;
		ipv4Offset += 4;
	}
	if(stagif)
	{
		conf.l2Flag 					= conf.l2Flag | L2_SVLAN ;
		conf.svlan.tpid 				= 0x88a8;		//etherType
		conf.svlan.svid 				= svid;
		conf.svlan.sprio				= spri;
		ipv4Offset += 4;
	}

	if(pppoeTagif==0)
	{
		conf.pppoe.type 		= 0x0800;
		conf.pppoe.session		= pppoeSID;
	}
	else
	{
		conf.l2Flag 			= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 		= 0x8864;
		conf.pppoe.session		= pppoeSID;
		ipv4Offset += 8;
	}

	v6rdPktLen = v4PktLen = pktGen(&conf, (int8*)pktBuf);		 /* generate pkt in buff */
	//dump_packet(pktBuf, v4PktLen, "\033[1;33;40m[tmp v4 Packet]\033[0m");
	//memcpy(pktBuf+(ipv4Offset+20),v6PktBuf+14,v6PktLen);

	*pktLen = v6rdPktLen;

	l2FormCrc(0 , pktBuf, *pktLen);
	*pktLen += 4;

	//dump_packet(pktBuf, *pktLen, "\033[1;33;40m[output Packet]\033[0m");
	kfree(v6PktBuf);
	kfree(tmpData);
	return 0;
}


int dslitePktGenByPatten(uint8 *pktBuf, uint8 *pktData, int *pktLen, uint32 pktType, uint8 tcpFlag,
		uint8* pktsmac, uint8* pktdmac,
		uint8* pktV6sip, uint8 *pktV6dip,
		uint32 pktsip, uint32 pktdip,
		uint16 sport, uint16 dport,
		int ctagif,int cvid,int cpri,
		int stagif,int svid,int spri, 
		uint8 pppoeTagif,uint8 pppoeSID, 
		uint8 ttl, uint8 hoplimit,  uint8 mf, uint8 tos ,uint8 tc) 
{

	int i, payloadLen;
	rtl8651_PktConf_t conf;
	struct ipv6hdr2 *v6h;
	char *pppoeh; 
	uint32 ipv6Offset = 14; //da+sa+etherType
	
	bzero(&conf,sizeof(conf));
	
	for(i=0;i<*pktLen;i++)
		pktData[i] = i&0xff;

	if(*pktLen > 8)
	{
		pktData[0] = 0x70;	//p
		pktData[1] = 0x61;	//a
		pktData[2] = 0x79;	//y
		pktData[3] = 0x6c;	//l
		pktData[4] = 0x6f;	//o
		pktData[5] = 0x61;	//a
		pktData[6] = 0x64;	//d
	}

	conf.pktType			= pktType;
	conf.conf_tcp_flag		= (pktType==_PKT_TYPE_TCP)?tcpFlag:0;
	conf.ip.version 		= 0x4;
	conf.payload.content	= pktData;	  // system default content (< 2048)
	conf.payload.length 	= *pktLen;	  // must < 2048
	conf.ip.id				= 0x123;
	conf.ip.ttl 			= ttl;
	conf.ip.headerLen		= 20;
	conf.ip.mf				= mf;
	conf.ip.offset			= 0;
	conf.ip.tos 			= tos;

	conf.pppoe.type 		= 0x0800;
	conf.pppoe.session		= 0;
	conf.cvlan.cprio		= 0;
	conf.l2Flag 			= 0;

	//set sip & port
	conf.conf_sip			= pktsip;
	conf.conf_sport 		= sport;

	//set dip &port
	conf.conf_dip			= pktdip;
	conf.conf_dport 		= dport;

	memcpy(conf.conf_smac, pktsmac, 6); //don't care
	memcpy(conf.conf_dmac, pktdmac, 6); //don't care

	*pktLen = pktGen(&conf, (int8*)pktBuf); 		 /* generate pkt in buff */

	//remove l2 header
	*pktLen -= 14;
	memmove(pktData, pktBuf+14, *pktLen);

	payloadLen = *pktLen;

	//dump_packet(pktData, payloadLen, "\033[1;33;40m[tmp Packet]\033[0m");

	//1 add ipv6 header
	bzero(&conf,sizeof(conf));

	conf.pktType			= _PKT_TYPE_V6TCP;
	conf.ipv6.version		= 6;
	conf.payload.content	= NULL;   // system default content (< 2048)
	conf.payload.length 	= 0;	  // must < 2048
	conf.rhdr.hdrlen		= 0;
	conf.ipv6.priority		= 0;
	conf.ipv6.flowlbl[0]	= conf.ipv6.flowlbl[1]	= conf.ipv6.flowlbl[2] = 0;
	conf.ipv6.pldlen		= payloadLen;
	//conf.ipv6.nxthdr			= 2;		//IGMPv2
	conf.ipv6.hoplmt		= hoplimit;
	conf.ipv6.priority		= tc;
	
	if(pppoeTagif==0)
	{
		conf.pppoe.type 		= 0x0800;
		conf.pppoe.session		= pppoeSID;
	}
	else
	{
		conf.l2Flag 			= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 		= 0x8864;
		conf.pppoe.session		= pppoeSID;
		ipv6Offset += 8;
	}


	//set sip & port
	memcpy(conf.conf_v6sip, pktV6sip, 16);
	conf.conf_sport 		= sport;

	//set dip &port
	memcpy(conf.conf_v6dip, pktV6dip, 16);
	conf.conf_dport 		= dport;

	memcpy(conf.conf_smac, pktsmac, 6);
	memcpy(conf.conf_dmac, pktdmac, 6);
		
	if(ctagif)
	{
		conf.l2Flag 					= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 				= cvid;
		conf.cvlan.cprio				= cpri;
		ipv6Offset += 4;
	}
	if(stagif)
	{
		conf.l2Flag 					= conf.l2Flag | L2_SVLAN ;
		conf.svlan.tpid 				= 0x88a8;		//etherType
		conf.svlan.svid 				= svid;
		conf.svlan.sprio				= spri;
		ipv6Offset += 4;
	}
		
	*pktLen = pktGen(&conf, (int8*)pktBuf); 		 /* generate pkt in buff */

	*pktLen -= 20;
	memmove(pktBuf+(*pktLen), pktData, payloadLen);
	*pktLen += payloadLen;

	//modify outer V6 header
	v6h = (struct ipv6hdr2*)((uint8 *)pktBuf + ipv6Offset);
	v6h->payload_len = htons(payloadLen&0xffff);
	v6h->nexthdr = 0x4; 

	//modify PPPoE Length
	if(pppoeTagif){
		pppoeh = ((uint8 *)pktBuf + ipv6Offset -8);
		*((uint16 *)(/*Length field*/pppoeh+4)) = htons(payloadLen + 42/*IPv6 header + PPP protocal*/);
	}

	l2FormCrc(0 , pktBuf, *pktLen);
	*pktLen += 4;

	//dump_packet(pktBuf, *pktLen, "\033[1;33;40m[output Packet]\033[0m");
	
	return 0;
}



int MAPE_PktGenByPatten(uint8 *pktBuf, uint8 *pktData, int *pktLen, uint32 pktType, uint8 tcpFlag,
		uint8* pktsmac, uint8* pktdmac,
		uint8* pktV6sip, uint8 *pktV6dip,
		uint32 pktsip, uint32 pktdip,
		uint16 sport, uint16 dport,
		int ctagif,int cvid,int cpri,
		int stagif,int svid,int spri, 
		uint8 pppoeTagif,uint8 pppoeSID, 
		uint8 ttl, uint8 hoplimit,	uint8 mf, uint8 tos,uint8 tc) 
{

	int i, payloadLen;
	rtl8651_PktConf_t conf;
	struct ipv6hdr2 *v6h;
	char *pppoeh; 
	uint32 ipv6Offset = 14; //da+sa+etherType
	uint32 L4Len=0;
	
	bzero(&conf,sizeof(conf));
	//TEST_WARNING("*pktLen=%d",*pktLen);
	
	for(i=0;i<*pktLen;i++)
		pktData[i] = i&0xff;

	if(*pktLen > 8)
	{
		pktData[0] = 0x70;	//p
		pktData[1] = 0x61;	//a
		pktData[2] = 0x79;	//y
		pktData[3] = 0x6c;	//l
		pktData[4] = 0x6f;	//o
		pktData[5] = 0x61;	//a
		pktData[6] = 0x64;	//d
	}

	conf.pktType			= pktType;
	conf.conf_tcp_flag		= (pktType==_PKT_TYPE_TCP)?tcpFlag:0;
	conf.ip.version 		= 0x4;
	conf.payload.content	= pktData;	  // system default content (< 2048)
	conf.payload.length 	= *pktLen;	  // must < 2048
	conf.ip.id				= 0x123;
	conf.ip.ttl 			= ttl;
	conf.ip.headerLen		= 20;
	conf.ip.mf				= mf;
	conf.ip.offset			= 0;
	conf.ip.tos 			= tos;

	conf.pppoe.type 		= 0x0800;
	conf.pppoe.session		= 0;
	conf.cvlan.cprio		= 0;
	conf.l2Flag 			= 0;

	//set sip & port
	conf.conf_sip			= pktsip;
	conf.conf_sport 		= sport;

	//set dip &port
	conf.conf_dip			= pktdip;
	conf.conf_dport 		= dport;

	memcpy(conf.conf_smac, pktsmac, 6); //don't care
	memcpy(conf.conf_dmac, pktdmac, 6); //don't care

	*pktLen = pktGen(&conf, (int8*)pktBuf); 		 /* generate pkt in buff */

	//remove l2 header
	*pktLen -= 14;
	memmove(pktData, pktBuf+14, *pktLen);

	if(conf.pktType==_PKT_TYPE_TCP || conf.pktType==_PKT_TYPE_TCP_SHORT_HDR)
	{
		L4Len=20;
		if(tcpFlag&TH_SYN)
			L4Len+=4;
	}
	else if(conf.pktType==_PKT_TYPE_UDP)
	{
		L4Len=8;
	}
	
	if(L4Len)
		payloadLen = conf.payload.length + conf.ip.headerLen /*v4HdrLen*/ + L4Len; /* for padding packet */
	else
		payloadLen = *pktLen; /*if padding packet payloadLen will get error length*/

	//if(payloadLen != *pktLen)
		//TEST_WARNING("Error payloadLen(%d)!=*pktLen(%d)",payloadLen,*pktLen);

	//dump_packet(pktData, payloadLen, "\033[1;33;40m[tmp Packet]\033[0m");

	//1 add ipv6 header
	bzero(&conf,sizeof(conf));

	conf.pktType			= _PKT_TYPE_V6TCP;
	conf.ipv6.version		= 6;
	conf.payload.content	= NULL;   // system default content (< 2048)
	conf.payload.length 	= 0;	  // must < 2048
	conf.rhdr.hdrlen		= 0;
	conf.ipv6.priority		= 0;
	conf.ipv6.flowlbl[0]	= conf.ipv6.flowlbl[1]	= conf.ipv6.flowlbl[2] = 0;
	conf.ipv6.pldlen		= payloadLen;
	//conf.ipv6.nxthdr			= 2;		//IGMPv2
	conf.ipv6.hoplmt		= hoplimit;
	conf.ipv6.priority		= tc;
	
	if(pppoeTagif==0)
	{
		conf.pppoe.type 		= 0x0800;
		conf.pppoe.session		= pppoeSID;
	}
	else
	{
		conf.l2Flag 			= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 		= 0x8864;
		conf.pppoe.session		= pppoeSID;
		ipv6Offset += 8;
	}


	//set sip & port
	memcpy(conf.conf_v6sip, pktV6sip, 16);
	conf.conf_sport 		= sport;

	//set dip &port
	memcpy(conf.conf_v6dip, pktV6dip, 16);
	conf.conf_dport 		= dport;

	memcpy(conf.conf_smac, pktsmac, 6);
	memcpy(conf.conf_dmac, pktdmac, 6);
		
	if(ctagif)
	{
		conf.l2Flag 					= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 				= cvid;
		conf.cvlan.cprio				= cpri;
		ipv6Offset += 4;
	}
	if(stagif)
	{
		conf.l2Flag 					= conf.l2Flag | L2_SVLAN ;
		conf.svlan.tpid 				= 0x88a8;		//etherType
		conf.svlan.svid 				= svid;
		conf.svlan.sprio				= spri;
		ipv6Offset += 4;
	}
		
	*pktLen = pktGen(&conf, (int8*)pktBuf); 		 /* generate pkt in buff */

	*pktLen -= 20;
	memmove(pktBuf+(*pktLen), pktData, payloadLen);
	*pktLen += payloadLen;

	//modify outer V6 header
	v6h = (struct ipv6hdr2*)((uint8 *)pktBuf + ipv6Offset);
	v6h->payload_len = htons(payloadLen&0xffff);
	v6h->nexthdr = 0x4; 

	//modify PPPoE Length
	if(pppoeTagif){
		pppoeh = ((uint8 *)pktBuf + ipv6Offset -8);
		*((uint16 *)(/*Length field*/pppoeh+4)) = htons(payloadLen + 42/*IPv6 header + PPP protocal*/);
	}

	l2FormCrc(0 , pktBuf, *pktLen);
	*pktLen += 4;

	//dump_packet(pktBuf, *pktLen, "\033[1;33;40m[output Packet]\033[0m");
	
	return 0;
}


int MAPE_PktGenByPattenContentZero(uint8 *pktBuf, uint8 *pktData, int *pktLen, uint32 pktType, uint8 tcpFlag,
		uint8* pktsmac, uint8* pktdmac,
		uint8* pktV6sip, uint8 *pktV6dip,
		uint32 pktsip, uint32 pktdip,
		uint16 sport, uint16 dport,
		int ctagif,int cvid,int cpri,
		int stagif,int svid,int spri, 
		uint8 pppoeTagif,uint8 pppoeSID, 
		uint8 ttl, uint8 hoplimit,	uint8 mf, uint8 tos,uint8 tc) 
{

	int i, payloadLen;
	rtl8651_PktConf_t conf;
	struct ipv6hdr2 *v6h;
	char *pppoeh; 
	uint32 ipv6Offset = 14; //da+sa+etherType
	uint32 L4Len=0;
	
	bzero(&conf,sizeof(conf));
	//TEST_WARNING("*pktLen=%d",*pktLen);
	
	for(i=0;i<*pktLen;i++)
		pktData[i] = 0x0;


	conf.pktType			= pktType;
	conf.conf_tcp_flag		= (pktType==_PKT_TYPE_TCP)?tcpFlag:0;
	conf.ip.version 		= 0x4;
	conf.payload.content	= pktData;	  // system default content (< 2048)
	conf.payload.length 	= *pktLen;	  // must < 2048
	conf.ip.id				= 0x123;
	conf.ip.ttl 			= ttl;
	conf.ip.headerLen		= 20;
	conf.ip.mf				= mf;
	conf.ip.offset			= 0;
	conf.ip.tos 			= tos;

	conf.pppoe.type 		= 0x0800;
	conf.pppoe.session		= 0;
	conf.cvlan.cprio		= 0;
	conf.l2Flag 			= 0;

	//set sip & port
	conf.conf_sip			= pktsip;
	conf.conf_sport 		= sport;

	//set dip &port
	conf.conf_dip			= pktdip;
	conf.conf_dport 		= dport;

	memcpy(conf.conf_smac, pktsmac, 6); //don't care
	memcpy(conf.conf_dmac, pktdmac, 6); //don't care

	*pktLen = pktGen(&conf, (int8*)pktBuf); 		 /* generate pkt in buff */

	//remove l2 header
	*pktLen -= 14;
	memmove(pktData, pktBuf+14, *pktLen);

	if(conf.pktType==_PKT_TYPE_TCP || conf.pktType==_PKT_TYPE_TCP_SHORT_HDR)
	{
		L4Len=20;
		if(tcpFlag&TH_SYN)
			L4Len+=4;
	}
	else if(conf.pktType==_PKT_TYPE_UDP)
	{
		L4Len=8;
	}
	
	if(L4Len)
		payloadLen = conf.payload.length + conf.ip.headerLen /*v4HdrLen*/ + L4Len; /* for padding packet */
	else
		payloadLen = *pktLen; /*if padding packet payloadLen will get error length*/

	//if(payloadLen != *pktLen)
		//TEST_WARNING("Error payloadLen(%d)!=*pktLen(%d)",payloadLen,*pktLen);

	//dump_packet(pktData, payloadLen, "\033[1;33;40m[tmp Packet]\033[0m");

	//1 add ipv6 header
	bzero(&conf,sizeof(conf));

	conf.pktType			= _PKT_TYPE_V6TCP;
	conf.ipv6.version		= 6;
	conf.payload.content	= NULL;   // system default content (< 2048)
	conf.payload.length 	= 0;	  // must < 2048
	conf.rhdr.hdrlen		= 0;
	conf.ipv6.priority		= 0;
	conf.ipv6.flowlbl[0]	= conf.ipv6.flowlbl[1]	= conf.ipv6.flowlbl[2] = 0;
	conf.ipv6.pldlen		= payloadLen;
	//conf.ipv6.nxthdr			= 2;		//IGMPv2
	conf.ipv6.hoplmt		= hoplimit;
	conf.ipv6.priority		= tc;
	
	if(pppoeTagif==0)
	{
		conf.pppoe.type 		= 0x0800;
		conf.pppoe.session		= pppoeSID;
	}
	else
	{
		conf.l2Flag 			= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 		= 0x8864;
		conf.pppoe.session		= pppoeSID;
		ipv6Offset += 8;
	}


	//set sip & port
	memcpy(conf.conf_v6sip, pktV6sip, 16);
	conf.conf_sport 		= sport;

	//set dip &port
	memcpy(conf.conf_v6dip, pktV6dip, 16);
	conf.conf_dport 		= dport;

	memcpy(conf.conf_smac, pktsmac, 6);
	memcpy(conf.conf_dmac, pktdmac, 6);
		
	if(ctagif)
	{
		conf.l2Flag 					= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 				= cvid;
		conf.cvlan.cprio				= cpri;
		ipv6Offset += 4;
	}
	if(stagif)
	{
		conf.l2Flag 					= conf.l2Flag | L2_SVLAN ;
		conf.svlan.tpid 				= 0x88a8;		//etherType
		conf.svlan.svid 				= svid;
		conf.svlan.sprio				= spri;
		ipv6Offset += 4;
	}
		
	*pktLen = pktGen(&conf, (int8*)pktBuf); 		 /* generate pkt in buff */

	*pktLen -= 20;
	memmove(pktBuf+(*pktLen), pktData, payloadLen);
	*pktLen += payloadLen;

	//modify outer V6 header
	v6h = (struct ipv6hdr2*)((uint8 *)pktBuf + ipv6Offset);
	v6h->payload_len = htons(payloadLen&0xffff);
	v6h->nexthdr = 0x4; 

	//modify PPPoE Length
	if(pppoeTagif){
		pppoeh = ((uint8 *)pktBuf + ipv6Offset -8);
		*((uint16 *)(/*Length field*/pppoeh+4)) = htons(payloadLen + 42/*IPv6 header + PPP protocal*/);
	}

	l2FormCrc(0 , pktBuf, *pktLen);
	*pktLen += 4;

	//dump_packet(pktBuf, *pktLen, "\033[1;33;40m[output Packet]\033[0m");
	
	return 0;
}


int MAPE_PktGenByPatten2(uint8 *pktBuf, uint8 *pktData, int *pktLen, uint32 pktType, uint8 tcpFlag,
		uint8* pktsmac, uint8* pktdmac,
		uint8* pktV6sip, uint8 *pktV6dip,
		uint32 pktsip, uint32 pktdip,
		uint16 sport, uint16 dport,
		int ctagif,int cvid,int cpri,
		int stagif,int svid,int spri, 
		uint8 pppoeTagif,uint8 pppoeSID, 
		uint8 ttl, uint8 hoplimit,	uint8 inner_mf ,uint8 outer_mf, uint8 tos,uint8 tc,
		uint16 outer_ipfragOffset, uint16 inner_ipfragOffset, uint16 tcpseq, uint8 smallTcpOffset)
{

	int i, payloadLen;
	rtl8651_PktConf_t conf;
	struct ipv6hdr2 *v6h;
	char *pppoeh; 
	uint32 ipv6Offset = 14; //da+sa+etherType
	uint32 v6HdrLen=40;
	uint32 extenv6HdrLen=0;
	uint32 L4Len=0;
	bzero(&conf,sizeof(conf));

	//TEST_WARNING("*pktLen=%d",*pktLen);
	
	for(i=0;i<*pktLen;i++)
		pktData[i] = i&0xff;


	if(*pktLen > 8)
	{
		pktData[0] = 0x70;	//p
		pktData[1] = 0x61;	//a
		pktData[2] = 0x79;	//y
		pktData[3] = 0x6c;	//l
		pktData[4] = 0x6f;	//o
		pktData[5] = 0x61;	//a
		pktData[6] = 0x64;	//d
	}


	if(smallTcpOffset)
		conf.pktType			= _PKT_TYPE_TCP_SHORT_HDR;
	else
		conf.pktType			= pktType;
	conf.l4hdr.tc.seq		= tcpseq;
	conf.conf_tcp_flag		= (pktType==_PKT_TYPE_TCP)?tcpFlag:0;
	conf.ip.version 		= 0x4;
	conf.payload.content	= pktData;	  // system default content (< 2048)
	conf.payload.length 	= *pktLen;	  // must < 2048
	conf.ip.id				= 0x123;
	conf.ip.ttl 			= ttl;
	conf.ip.headerLen		= 20;
	conf.ip.mf				= inner_mf;
	conf.ip.offset			= inner_ipfragOffset*8;
	conf.ip.tos 			= tos;


	conf.pppoe.type 		= 0x0800;
	conf.pppoe.session		= 0;
	conf.cvlan.cprio		= 0;
	conf.l2Flag 			= 0;


	//set sip & port
	conf.conf_sip			= pktsip;
	conf.conf_sport 		= sport;

	//set dip &port
	conf.conf_dip			= pktdip;
	conf.conf_dport 		= dport;

	memcpy(conf.conf_smac, pktsmac, 6); //don't care
	memcpy(conf.conf_dmac, pktdmac, 6); //don't care

	*pktLen = pktGen(&conf, (int8*)pktBuf); 		 /* generate pkt in buff */

	//remove l2 header
	*pktLen -= 14;
	memmove(pktData, pktBuf+14, *pktLen);


	if(conf.pktType==_PKT_TYPE_TCP || conf.pktType==_PKT_TYPE_TCP_SHORT_HDR)
	{
		L4Len=20;
		if(tcpFlag&TH_SYN)
			L4Len+=4;
	}
	else if(conf.pktType==_PKT_TYPE_UDP)
	{
		L4Len=8;
	}
	
	if(L4Len)
		payloadLen = conf.payload.length + conf.ip.headerLen /*v4HdrLen*/ + L4Len; /* for padding packet */
	else
		payloadLen = *pktLen; /*if padding packet payloadLen will get error length*/

	//if(payloadLen != *pktLen)
		//TEST_WARNING("Error L4Len=%d payloadLen(%d)!=*pktLen(%d)",L4Len,payloadLen,*pktLen);

	//dump_packet(pktData, payloadLen, "\033[1;33;40m[tmp Packet]\033[0m");

	//1 add ipv6 header
	bzero(&conf,sizeof(conf));

	conf.pktType			= _PKT_TYPE_V6TCP;
	conf.ipv6.version		= 6;
	conf.payload.content	= NULL;   // system default content (< 2048)
	conf.payload.length 	= 0;	  // must < 2048
	conf.rhdr.hdrlen		= 0;
	conf.ipv6.priority		= 0;
	conf.ipv6.flowlbl[0]	= conf.ipv6.flowlbl[1]	= conf.ipv6.flowlbl[2] = 0;
	conf.ipv6.pldlen		= payloadLen;
	//conf.ipv6.nxthdr			= 2;		//IGMPv2
	conf.ipv6.hoplmt		= hoplimit;
	conf.ipv6.priority		= tc;
	if(outer_ipfragOffset || outer_mf)
	{
		conf.extHdr = FraHdr;
		conf.frghdr.fragment = ((outer_ipfragOffset)<<3) | outer_mf;
		conf.frghdr.id = 0x2379;
		extenv6HdrLen+=8;
		//printk("@@@@@conf.frghdr.fragment=%x  outer_ipfragOffset=%x outer_mf=%x \n",conf.frghdr.fragment,outer_ipfragOffset,outer_mf);
	}
	
	if(pppoeTagif==0)
	{
		conf.pppoe.type 		= 0x0800;
		conf.pppoe.session		= pppoeSID;
	}
	else
	{
		conf.l2Flag 			= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 		= 0x8864;
		conf.pppoe.session		= pppoeSID;
		ipv6Offset += 8;
	}


	//set sip & port
	memcpy(conf.conf_v6sip, pktV6sip, 16);
	conf.conf_sport 		= sport;

	//set dip &port
	memcpy(conf.conf_v6dip, pktV6dip, 16);
	conf.conf_dport 		= dport;

	memcpy(conf.conf_smac, pktsmac, 6);
	memcpy(conf.conf_dmac, pktdmac, 6);
		
	if(ctagif)
	{
		conf.l2Flag 					= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 				= cvid;
		conf.cvlan.cprio				= cpri;
		ipv6Offset += 4;
	}
	if(stagif)
	{
		conf.l2Flag 					= conf.l2Flag | L2_SVLAN ;
		conf.svlan.tpid 				= 0x88a8;		//etherType
		conf.svlan.svid 				= svid;
		conf.svlan.sprio				= spri;
		ipv6Offset += 4;
	}

	//No content
	*pktLen = pktGen(&conf, (int8*)pktBuf); 		 /* generate pkt in buff */

	
	*pktLen -= 20; // TCP Len
	
	memmove(pktBuf+(*pktLen), pktData, payloadLen);
	*pktLen += payloadLen;

	//modify outer V6 header
	v6h = (struct ipv6hdr2*)((uint8 *)pktBuf + ipv6Offset);

	if(outer_ipfragOffset || outer_mf)
	{
		*(pktBuf+ipv6Offset+40)/*v6hdrLen*/ = 0x4;
	}
	else
		v6h->nexthdr = 0x4; 
	v6h->payload_len = htons((payloadLen&0xffff) + extenv6HdrLen);
	//printk("v6h->payload_len=%x 0x%x  payloadLen=%d extenv6HdrLen=%d " ,v6h->payload_len,(payloadLen&0xffff + extenv6HdrLen),payloadLen,extenv6HdrLen);


	//modify PPPoE Length
	if(pppoeTagif){
		pppoeh = ((uint8 *)pktBuf + ipv6Offset -8);
		*((uint16 *)(/*Length field*/pppoeh+4)) = htons(payloadLen + v6HdrLen + extenv6HdrLen +2/*IPv6 header + PPP protocal*/);
	}

	l2FormCrc(0 , pktBuf, *pktLen);
	*pktLen += 4;

	//dump_packet(pktBuf, *pktLen, "MPAE Gen");
	
	return 0;
}


int MAPE_PktGenByPatten3(uint8 *pktBuf, uint8 *pktData, int *pktLen, uint32 pktType, uint8 tcpFlag,
		uint8* pktsmac, uint8* pktdmac,
		uint8* pktV6sip, uint8 *pktV6dip,
		uint32 pktsip, uint32 pktdip,
		uint16 sport, uint16 dport,
		int ctagif,int cvid,int cpri,
		int stagif,int svid,int spri, 
		uint8 pppoeTagif,uint8 pppoeSID, 
		uint8 ttl, uint8 hoplimit,	uint8 inner_mf ,uint8 outer_mf, uint8 tos,uint8 tc,
		uint16 outer_ipfragOffset, uint16 inner_ipfragOffset, uint16 tcpseq, uint8 smallTcpOffset)
{

	int i, payloadLen;
	rtl8651_PktConf_t conf;
	struct ipv6hdr2 *v6h;
	char *pppoeh; 
	uint32 ipv6Offset = 14; //da+sa+etherType
	uint32 v6HdrLen=40;
	uint32 extenv6HdrLen=0;
	uint32 L4Len=0;
	bzero(&conf,sizeof(conf));

	
	for(i=0;i<*pktLen;i++)
		pktData[i] = i&0xff;

	if(*pktLen > 8)
	{
		pktData[0] = 0x70;	//p
		pktData[1] = 0x61;	//a
		pktData[2] = 0x79;	//y
		pktData[3] = 0x6c;	//l
		pktData[4] = 0x6f;	//o
		pktData[5] = 0x61;	//a
		pktData[6] = 0x64;	//d
	}


	if(smallTcpOffset)
		conf.pktType			= _PKT_TYPE_TCP_SHORT_HDR;
	else
		conf.pktType			= pktType;
	conf.l4hdr.tc.seq		= tcpseq;
	conf.conf_tcp_flag		= (pktType==_PKT_TYPE_TCP)?tcpFlag:0;
	conf.ip.version 		= 0x4;
	conf.payload.content	= pktData;	  // system default content (< 2048)
	conf.payload.length 	= *pktLen;	  // must < 2048
	conf.ip.id				= 0x123;
	conf.ip.ttl 			= ttl;
	conf.ip.headerLen		= 20;
	conf.ip.mf				= inner_mf;
	conf.ip.offset			= inner_ipfragOffset*8;
	conf.ip.tos 			= tos;


	conf.pppoe.type 		= 0x0800;
	conf.pppoe.session		= 0;
	conf.cvlan.cprio		= 0;
	conf.l2Flag 			= 0;


	//set sip & port
	conf.conf_sip			= pktsip;
	conf.conf_sport 		= sport;

	//set dip &port
	conf.conf_dip			= pktdip;
	conf.conf_dport 		= dport;

	memcpy(conf.conf_smac, pktsmac, 6); //don't care
	memcpy(conf.conf_dmac, pktdmac, 6); //don't care

	*pktLen = pktGen(&conf, (int8*)pktBuf); 		 /* generate pkt in buff */

	//remove l2 header
	*pktLen -= 14;
	memmove(pktData, pktBuf+14, *pktLen);


	if(conf.pktType==_PKT_TYPE_TCP || conf.pktType==_PKT_TYPE_TCP_SHORT_HDR)
	{
		L4Len=20;
		if(tcpFlag&TH_SYN)
			L4Len+=4;
	}
	else if(conf.pktType==_PKT_TYPE_UDP)
	{
		L4Len=8;
	}

	
	if(L4Len)
		payloadLen = conf.payload.length + conf.ip.headerLen /*v4HdrLen*/ + L4Len; /* for padding packet */
	else
		payloadLen = *pktLen; /*if padding packet payloadLen will get error length*/

	//if(payloadLen != *pktLen)
		//TEST_WARNING("Error payloadLen(%d)!=*pktLen(%d)",payloadLen,*pktLen);

	//dump_packet(pktData, payloadLen, "\033[1;33;40m[tmp Packet]\033[0m");

	//1 add ipv6 header
	bzero(&conf,sizeof(conf));

	conf.pktType			= _PKT_TYPE_V6TCP;
	conf.ipv6.version		= 6;
	conf.payload.content	= NULL;   // system default content (< 2048)
	conf.payload.length 	= 0;	  // must < 2048
	conf.rhdr.hdrlen		= 0;
	conf.ipv6.priority		= 0;
	conf.ipv6.flowlbl[0]	= conf.ipv6.flowlbl[1]	= conf.ipv6.flowlbl[2] = 0;
	conf.ipv6.pldlen		= payloadLen;
	//conf.ipv6.nxthdr			= 2;		//IGMPv2
	conf.ipv6.hoplmt		= hoplimit;
	conf.ipv6.priority		= tc;
	if(outer_ipfragOffset || outer_mf)
	{
		conf.extHdr = FraHdr;
		conf.frghdr.fragment = ((outer_ipfragOffset)<<3) | outer_mf;
		conf.frghdr.id = 0x2379;
		extenv6HdrLen+=8;
		//printk("@@@@@conf.frghdr.fragment=%x	outer_ipfragOffset=%x outer_mf=%x \n",conf.frghdr.fragment,outer_ipfragOffset,outer_mf);
	}
	
	if(pppoeTagif==0)
	{
		conf.pppoe.type 		= 0x0800;
		conf.pppoe.session		= pppoeSID;
	}
	else
	{
		conf.l2Flag 			= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 		= 0x8864;
		conf.pppoe.session		= pppoeSID;
		ipv6Offset += 8;
	}


	//set sip & port
	memcpy(conf.conf_v6sip, pktV6sip, 16);
	//conf.conf_sport 		= (sport==0)?0x200:sport;

	//set dip &port
	memcpy(conf.conf_v6dip, pktV6dip, 16);
	//conf.conf_dport 		= (dport==0)?0x300:dport;

	memcpy(conf.conf_smac, pktsmac, 6);
	memcpy(conf.conf_dmac, pktdmac, 6);
		
	if(ctagif)
	{
		conf.l2Flag 					= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 				= cvid;
		conf.cvlan.cprio				= cpri;
		ipv6Offset += 4;
	}
	if(stagif)
	{
		conf.l2Flag 					= conf.l2Flag | L2_SVLAN ;
		conf.svlan.tpid 				= 0x88a8;		//etherType
		conf.svlan.svid 				= svid;
		conf.svlan.sprio				= spri;
		ipv6Offset += 4;
	}

	//No content
	*pktLen = pktGen(&conf, (int8*)pktBuf); 		 /* generate pkt in buff */

	
	*pktLen -= 20; // TCP Len
	
	memmove(pktBuf+(*pktLen), pktData, payloadLen);
	*pktLen += payloadLen;

	//modify outer V6 header
	v6h = (struct ipv6hdr2*)((uint8 *)pktBuf + ipv6Offset);

	if(outer_ipfragOffset || outer_mf)
	{
		*(pktBuf+ipv6Offset+40)/*v6hdrLen*/ = 0x4;
	}
	else
		v6h->nexthdr = 0x4; 
	v6h->payload_len = htons((payloadLen&0xffff) + extenv6HdrLen);
	//printk("v6h->payload_len=%x 0x%x	payloadLen=%d extenv6HdrLen=%d " ,v6h->payload_len,(payloadLen&0xffff + extenv6HdrLen),payloadLen,extenv6HdrLen);


	//modify PPPoE Length
	if(pppoeTagif){
		pppoeh = ((uint8 *)pktBuf + ipv6Offset -8);
		*((uint16 *)(/*Length field*/pppoeh+4)) = htons(payloadLen + v6HdrLen + extenv6HdrLen +2/*IPv6 header + PPP protocal*/);
	}

	l2FormCrc(0 , pktBuf, *pktLen);
	*pktLen += 4;

	//dump_packet(pktBuf, *pktLen, "MPAE Gen");
	
	return 0;
}

int l2PktGen(uint8 *pktBuf,uint8 *pktData,int *pktLen,
	unsigned char *smac, unsigned char *dmac, int etype,
	int ctagif,int cvid,int cpri,
	int stagif,int tpid, int svid,int spri,
	cputag_t *cputag, cputagTx_t *cputagTx,
	int pppoetagif,int pppoe_session_id)
{
	int i;
	rtl8651_PktConf_t conf;

 	bzero(&conf, sizeof(conf));

	for (i=0;i<*pktLen;i++)
		pktData[i] = i&0xff;

	if(*pktLen > 8)
	{
		pktData[0] = 0x70;	//p
		pktData[1] = 0x61;	//a
		pktData[2] = 0x79;	//y
		pktData[3] = 0x6c;	//l
		pktData[4] = 0x6f;	//o
		pktData[5] = 0x61;	//a
		pktData[6] = 0x64;	//d
	}

	conf.pktType	= _PKT_TYPE_ETHER;
	conf.payload.length 	= *pktLen;
	conf.payload.content	= pktData;
	conf.conf_ethtype = etype;
	conf.pppoe.session	= 0;
	conf.cvlan.cprio	= 1;
	conf.l2Flag 		= 0;

	//conf.conf_tcp_flag = TH_RST;
	if (ctagif) {
		conf.l2Flag 		= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 	= cvid;
		conf.cvlan.cprio	= cpri;
	}

	if (stagif) {
		conf.l2Flag 		= conf.l2Flag | L2_SVLAN;
		conf.svlan.svid 	= svid;
		conf.svlan.sprio	= spri;
		if(tpid)
			conf.svlan.tpid		 = tpid;
		else
			conf.svlan.tpid = 0x88a8;
	}

	if(inner_insert_len_snap_after_vlan || inner_insert_len_snap_before_vlan)
	{
		conf.l2Flag |= L2_SNAP;
		conf.llc.dsap = 0xaa;
		conf.llc.ssap = 0xaa;
	}

	if (cputag!=NULL) {
		conf.l2Flag 		= conf.l2Flag | L2_CPUTAG_OTHER;
		conf.conf_ctag_protocol	= 0x4;
		conf.conf_ctag_reason	= cputag->res;
		conf.conf_ctag_priority	= cputag->pri;
		conf.conf_ctag_streamid = cputag->streamid;
//		conf.conf_ctag_ttlde	= cputag->ttl_1;
		conf.conf_ctag_l3r	= cputag->l3r;
		conf.conf_ctag_org	= cputag->org;
		conf.conf_ctag_spa	= cputag->spa;
		conf.conf_ctag_extspa	= cputag->extspa;
		conf.conf_ctag_extdpmsk	= cputag->epmsk;
		conf.conf_ctag_fbi = cputag->fbi;
		conf.conf_ctag_fb_index = cputag->fbindex;
	}else if (cputagTx != NULL) {
		conf.l2Flag 		= conf.l2Flag | L2_CPUTAG_TX2;
		conf.conf_ctag_protocol	= 0x4;
		conf.ctag_tx2.txmsk = cputagTx->txmsk;
		conf.ctag_tx2.prisel = cputagTx->prisel;
		conf.ctag_tx2.pri = cputagTx->pri;
		conf.ctag_tx2.keep = cputagTx->keep;
		conf.ctag_tx2.dislrn = cputagTx->dislrn;
		conf.ctag_tx2.psel = cputagTx->psel;
		conf.ctag_tx2.directTx = cputagTx->directTx;
		conf.ctag_tx2.extspa = cputagTx->extspa;
		conf.ctag_tx2.pppoeact = cputagTx->pppoeact;
		conf.ctag_tx2.pppoeidx = cputagTx->pppoeidx;
		conf.ctag_tx2.streamid = cputagTx->streamid;
	}

	if (pppoetagif==1) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8864;
		conf.pppoe.session 	= pppoe_session_id;
	}else if(pppoetagif==2) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8863;
		conf.pppoe.session 	= pppoe_session_id;
	}

	strtomac((void *)&conf.conf_smac[0], smac);
	strtomac((void *)&conf.conf_dmac[0], dmac);


	pppoe_length_exclude_padding = inner_ppp_length_exclude_padding;
	pppoe_length_include_padding = inner_ppp_length_include_padding;
	//printk("pppoe_length_exclude_padding:%d @ %s %d\n",pppoe_length_exclude_padding,__func__,__LINE__);
	ipv4_length_exclude_padding = inner_ipv4_len_exclude_padding;
	//printk("ipv4_length_exclude_padding:%d @ %s %d\n",ipv4_length_exclude_padding,__func__,__LINE__);
	skip_minPktLen_check = inner_skip_minPktLen_check;
	//printk("skip_minPktLen_check:%d @ %s %d\n",skip_minPktLen_check,__func__,__LINE__);
	llc_len_including_padding = inner_llc_len_include_ingress_padding;
	*pktLen = pktGen(&conf, (int8*)pktBuf);			 /* generate pkt in buff */
	llc_len_including_padding = 0;
	/* generate pkt in buff */
	if(inner_tail_padding_len > 0)
	{
		memset(pktBuf+*pktLen,0,inner_tail_padding_len);
		*pktLen += inner_tail_padding_len;
	}
	pppoe_length_exclude_padding = 0;
	pppoe_length_include_padding = 0;
	ipv4_length_exclude_padding = 0;
	skip_minPktLen_check = 0;

	 if(inner_insert_len_snap_before_vlan && (ctagif || stagif))
	 {
		 unsigned char lenSnap[8];
		 unsigned char tag[8];
		 unsigned short l2Len;
		 int lenTag = (ctagif?4:0) + (stagif?4:0);
		 
		 memcpy(&tag[0], pktBuf+12, lenTag);
		 memcpy(&lenSnap[0], pktBuf+12+lenTag, 8);
		 l2Len = (lenSnap[0]<<8|lenSnap[1]);
		 l2Len += lenTag;
		 lenSnap[0] = (l2Len>>8)&0xff;
		 lenSnap[1] = l2Len&0xff;
		 memcpy(pktBuf + 12, &lenSnap[0], 8);
		 memcpy(pktBuf + 20, &tag[0], lenTag);
		 //dump_packet(pktBuf,*pktLen,"Packet with snap");
	 }

         *pktLen += 4;
	//dump_packet(pktBuf, *pktLen, "\033[1;33;40m[Pure L2l packet]\033[0m");

	return 0;
}


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
	int pppoetagif,int pppoe_session_id)
{
	int i;
	rtl8651_PktConf_t conf;
        unsigned char *innerPktContentPtr = (uint8 *)&innerPktContent[0];
        int innerPktContentLen;

 	bzero(&conf, sizeof(conf));

	for (i=0;i<*pktLen;i++)
		pktData[i] = i&0xff;

	if(*pktLen > 8)
	{
		pktData[0] = 0x70;	//p
		pktData[1] = 0x61;	//a
		pktData[2] = 0x79;	//y
		pktData[3] = 0x6c;	//l
		pktData[4] = 0x6f;	//o
		pktData[5] = 0x61;	//a
		pktData[6] = 0x64;	//d
	}

	conf.pktType	= _PKT_TYPE_ETHER;
	conf.payload.length 	= *pktLen;
	conf.payload.content	= pktData;
	conf.conf_ethtype	= etype;
	conf.pppoe.session	= 0;
	conf.cvlan.cprio	= 1;
	conf.l2Flag 		= 0;

	//conf.conf_tcp_flag = TH_RST;
	if (ctagif) {
		conf.l2Flag 		= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 	= cvid;
		conf.cvlan.cprio	= cpri;
	}

	if (stagif) {
		conf.l2Flag 		= conf.l2Flag | L2_SVLAN;
		conf.svlan.svid 	= svid;
		conf.svlan.sprio	= spri;
		if(tpid)
			conf.svlan.tpid		 = tpid;
		else
			conf.svlan.tpid = 0x88a8;
	}

	if(inner_insert_len_snap_after_vlan || inner_insert_len_snap_before_vlan)
	{
		conf.l2Flag |= L2_SNAP;
		conf.llc.dsap = 0xaa;
		conf.llc.ssap = 0xaa;
	}

	if (cputag!=NULL) {
		conf.l2Flag 		= conf.l2Flag | L2_CPUTAG_OTHER;
		conf.conf_ctag_protocol	= 0x4;
		conf.conf_ctag_reason	= cputag->res;
		conf.conf_ctag_priority	= cputag->pri;
		conf.conf_ctag_streamid = cputag->streamid;
//		conf.conf_ctag_ttlde	= cputag->ttl_1;
		conf.conf_ctag_l3r	= cputag->l3r;
		conf.conf_ctag_org	= cputag->org;
		conf.conf_ctag_spa	= cputag->spa;
		conf.conf_ctag_extspa	= cputag->extspa;
		conf.conf_ctag_extdpmsk	= cputag->epmsk;
		conf.conf_ctag_fbi = cputag->fbi;
		conf.conf_ctag_fb_index = cputag->fbindex;
	}else if (cputagTx != NULL) {
		conf.l2Flag 		= conf.l2Flag | L2_CPUTAG_TX2;
		conf.conf_ctag_protocol	= 0x4;
		conf.ctag_tx2.txmsk = cputagTx->txmsk;
		conf.ctag_tx2.prisel = cputagTx->prisel;
		conf.ctag_tx2.pri = cputagTx->pri;
		conf.ctag_tx2.keep = cputagTx->keep;
		conf.ctag_tx2.dislrn = cputagTx->dislrn;
		conf.ctag_tx2.psel = cputagTx->psel;
		conf.ctag_tx2.directTx = cputagTx->directTx;
		conf.ctag_tx2.extspa = cputagTx->extspa;
		conf.ctag_tx2.pppoeact = cputagTx->pppoeact;
		conf.ctag_tx2.pppoeidx = cputagTx->pppoeidx;
		conf.ctag_tx2.streamid = cputagTx->streamid;
	}

	if (pppoetagif==1) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8864;
		conf.pppoe.session 	= pppoe_session_id;
	}else if(pppoetagif==2) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8863;
		conf.pppoe.session 	= pppoe_session_id;
	}

	strtomac((void *)&conf.conf_smac[0], smac);
	strtomac((void *)&conf.conf_dmac[0], dmac);

	pppoe_length_exclude_padding = inner_ppp_length_exclude_padding;
	skip_minPktLen_check = inner_skip_minPktLen_check;
	llc_len_including_padding = inner_llc_len_include_ingress_padding;
	innerPktContentLen = pktGen(&conf, (int8*)innerPktContentPtr);			 /* generate pkt in buff */
	if(inner_tail_padding_len > 0)
	{
		memset(innerPktContent+innerPktContentLen,0,inner_tail_padding_len);
		innerPktContentLen += inner_tail_padding_len;
	}
	llc_len_including_padding = 0;
	pppoe_length_exclude_padding = 0;
	skip_minPktLen_check = 0;

	if(inner_insert_len_snap_before_vlan && (ctagif || stagif))
	{
		unsigned char lenSnap[8];
		unsigned char tag[8];
		unsigned short l2Len;
		int lenTag = (ctagif?4:0) + (stagif?4:0);
		
		memcpy(&tag[0], innerPktContent+12, lenTag);
		memcpy(&lenSnap[0], innerPktContent+12+lenTag, 8);
		l2Len = (lenSnap[0]<<8|lenSnap[1]);
		l2Len += lenTag;
		lenSnap[0] = (l2Len>>8)&0xff;
		lenSnap[1] = l2Len&0xff;
		memcpy(innerPktContent + 12, &lenSnap[0], 8);
		memcpy(innerPktContent + 20, &tag[0], lenTag);
		//dump_packet(innerPktContent,*pktLen,"Packet with snap");
	}

	//dump_packet(innerPktContent,innerPktContentLen,"Inner Packet");

    /* SRv6 L2 tunnel part */
     do{
        //IPv6
        bzero(&conf,sizeof(conf));
        conf.pktType                            = _PKT_TYPE_IPV6;
        conf.conf_tcp_flag                      = 0;
        conf.ipv6.version                       = 6;
         conf.payload.content               = innerPktContentPtr;
         conf.payload.length                = innerPktContentLen;
	conf.rhdr.hdrlen			   = srhHdrLen;
	conf.rhdr.segments_left 		= srhSegmentLeft;
         conf.ipv6.priority                     = l2tun_tc;
         conf.ipv6.flowlbl[0]  = (flow_label>>16)&0xf;
         conf.ipv6.flowlbl[1]  = (flow_label>>8)&0xff;
         conf.ipv6.flowlbl[2] =  (flow_label>>0)&0xff;
         conf.ipv6.pldlen        = *pktLen;
         conf.ipv6.nxthdr          = l2tun_protocol;
         conf.ipv6.hoplmt        = l2tun_hoplimit;
	if(l2tun_pppoetagif == 1)
	{
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8864;
		conf.pppoe.session 	= l2tun_pppoe_session_id;
	}
	else if(l2tun_pppoetagif == 2)
	{
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8863;
		conf.pppoe.session 	= l2tun_pppoe_session_id;
	}
  	if (l2tun_ctagif) {
		conf.l2Flag 		= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 	= l2tun_cvid;
		conf.cvlan.cprio	= l2tun_cpri;
	}

	if (l2tun_stagif) {
		conf.l2Flag 		= conf.l2Flag | L2_SVLAN;
		conf.svlan.svid 	= l2tun_svid;
		conf.svlan.sprio	= l2tun_spri;
		if(l2tun_tpid)
			conf.svlan.tpid		 = l2tun_tpid;
		else
			conf.svlan.tpid = 0x88a8;
	}
	if(srhHdrLen)
		conf.l2Flag |= (RouHdr | SRv6Hdr);

	if(outer_insert_len_snap)
		conf.l2Flag |= L2_SNAP;
	
	in6_pton(l2tun_sip,-1,(unsigned char *)&conf.ipv6.src_ip[0],-1,NULL);
       in6_pton(l2tun_dip,-1,(unsigned char *)&conf.ipv6.dst_ip[0],-1,NULL);

	strtomac((void *)&conf.conf_smac[0], l2tun_smac);
	strtomac((void *)&conf.conf_dmac[0], l2tun_dmac);
         
	 pppoe_length_exclude_padding = outer_ppp_length_exclude_padding;
	 pppoe_length_include_padding = outer_ppp_length_include_padding;
	 ipv6_length_exclude_padding = outer_ipv6_len_exclude_padding;
	 ipv6_length_include_padding = outer_ipv6_len_include_padding;
	 llc_len_including_padding = outer_llc_len_include_ingress_padding;
	 llc_len_excluding_padding = outer_llc_len_exclude_ingress_padding;
         *pktLen = pktGen(&conf, (int8*)pktBuf);       /* generate pkt in buff */
	llc_len_including_padding = 0;
	llc_len_excluding_padding = 0;
	 pppoe_length_exclude_padding = 0;
	 pppoe_length_include_padding = 0;
	 ipv6_length_exclude_padding = 0;
	 ipv6_length_include_padding = 0;

         *pktLen += 4;
	//dump_packet(pktBuf, *pktLen, "\033[1;33;40m[SRv6 L2tunnl packet]\033[0m");
      }while(0);

	return 0;
}

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
	int pppoetagif,int pppoe_session_id)
{
	int i;
	rtl8651_PktConf_t conf;
        unsigned char *innerPktContentPtr = (uint8 *)&innerPktContent[0];
        int innerPktContentLen;

 	bzero(&conf, sizeof(conf));

	for (i=0;i<*pktLen;i++)
		pktData[i] = i&0xff;

	if(*pktLen > 8)
	{
		pktData[0] = 0x70;	//p
		pktData[1] = 0x61;	//a
		pktData[2] = 0x79;	//y
		pktData[3] = 0x6c;	//l
		pktData[4] = 0x6f;	//o
		pktData[5] = 0x61;	//a
		pktData[6] = 0x64;	//d
	}

	if (protocol == IPPROTO_UDP)
		conf.pktType	= _PKT_TYPE_UDP;
	else
		conf.pktType	= _PKT_TYPE_TCP;

	conf.ip.id					= 0x123;
	conf.ip.ttl 				= ttl;
	conf.ip.tos 				= iptos;
	conf.payload.length 		= *pktLen;
	conf.ip.version				= 0x4;
	conf.ip.headerLen			= 20;
	conf.payload.content		= pktData;
	conf.pppoe.type 			= 0x0800;
	conf.pppoe.session			= 0;
	conf.cvlan.cprio			= 1;
	conf.l2Flag 				= 0;

	//conf.conf_tcp_flag = TH_RST;
	if (ctagif) {
		conf.l2Flag 		= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 	= cvid;
		conf.cvlan.cprio	= cpri;
	}

	if (stagif) {
		conf.l2Flag 		= conf.l2Flag | L2_SVLAN;
		conf.svlan.svid 	= svid;
		conf.svlan.sprio	= spri;
		if(tpid)
			conf.svlan.tpid		 = tpid;
		else
			conf.svlan.tpid = 0x88a8;
	}
	
	if(inner_insert_len_snap_after_vlan || inner_insert_len_snap_before_vlan)
		conf.l2Flag |= L2_SNAP;

	if (cputag!=NULL) {
		conf.l2Flag 		= conf.l2Flag | L2_CPUTAG_OTHER;
		conf.conf_ctag_protocol	= 0x4;
		conf.conf_ctag_reason	= cputag->res;
		conf.conf_ctag_priority	= cputag->pri;
		conf.conf_ctag_streamid = cputag->streamid;
//		conf.conf_ctag_ttlde	= cputag->ttl_1;
		conf.conf_ctag_l3r	= cputag->l3r;
		conf.conf_ctag_org	= cputag->org;
		conf.conf_ctag_spa	= cputag->spa;
		conf.conf_ctag_extspa	= cputag->extspa;
		conf.conf_ctag_extdpmsk	= cputag->epmsk;
		conf.conf_ctag_fbi = cputag->fbi;
		conf.conf_ctag_fb_index = cputag->fbindex;
	}else if (cputagTx != NULL) {
		conf.l2Flag 		= conf.l2Flag | L2_CPUTAG_TX2;
		conf.conf_ctag_protocol	= 0x4;
		conf.ctag_tx2.txmsk = cputagTx->txmsk;
		conf.ctag_tx2.prisel = cputagTx->prisel;
		conf.ctag_tx2.pri = cputagTx->pri;
		conf.ctag_tx2.keep = cputagTx->keep;
		conf.ctag_tx2.dislrn = cputagTx->dislrn;
		conf.ctag_tx2.psel = cputagTx->psel;
		conf.ctag_tx2.directTx = cputagTx->directTx;
		conf.ctag_tx2.extspa = cputagTx->extspa;
		conf.ctag_tx2.pppoeact = cputagTx->pppoeact;
		conf.ctag_tx2.pppoeidx = cputagTx->pppoeidx;
		conf.ctag_tx2.streamid = cputagTx->streamid;

	}

	if (pppoetagif==1) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8864;
		conf.pppoe.session 	= pppoe_session_id;
	}else if(pppoetagif==2) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8863;
		conf.pppoe.session 	= pppoe_session_id;
	}

	strtomac((void *)&conf.conf_smac[0], smac);
	strtomac((void *)&conf.conf_dmac[0], dmac);
	conf.conf_sip	= ntohl(inet_addr(sip));
	conf.conf_dip	= ntohl(inet_addr(dip));
	conf.conf_sport = sport;
	conf.conf_dport = dport;

	pppoe_length_exclude_padding = inner_ppp_length_exclude_padding;
	pppoe_length_include_padding = inner_ppp_length_include_padding;
	//printk("pppoe_length_exclude_padding:%d @ %s %d\n",pppoe_length_exclude_padding,__func__,__LINE__);
	ipv4_length_exclude_padding = inner_ipv4_len_exclude_padding;
	//printk("ipv4_length_exclude_padding:%d @ %s %d\n",ipv4_length_exclude_padding,__func__,__LINE__);
	skip_minPktLen_check = inner_skip_minPktLen_check;
	//printk("skip_minPktLen_check:%d @ %s %d\n",skip_minPktLen_check,__func__,__LINE__);
	llc_len_including_padding = inner_llc_len_include_ingress_padding;
	innerPktContentLen = pktGen(&conf, (int8*)innerPktContentPtr);
	/* generate pkt in buff */
	//printk("inner_tail_padding_len:%d @ %s %d\n",inner_tail_padding_len,__func__,__LINE__);
	llc_len_including_padding = 0;
	if(inner_tail_padding_len > 0)
	{
		memset(innerPktContent+innerPktContentLen,0,inner_tail_padding_len);
		innerPktContentLen += inner_tail_padding_len;
	}
	pppoe_length_exclude_padding = 0;
	pppoe_length_include_padding = 0;
	ipv4_length_exclude_padding = 0;
	skip_minPktLen_check = 0;

	if(inner_insert_len_snap_before_vlan && (ctagif || stagif))
	{
		unsigned char lenSnap[8];
		unsigned char tag[8];
		unsigned short l2Len;
		int lenTag = (ctagif?4:0) + (stagif?4:0);
		
		memcpy(&tag[0], innerPktContent+12, lenTag);
		memcpy(&lenSnap[0], innerPktContent+12+lenTag, 8);
		l2Len = (lenSnap[0]<<8|lenSnap[1]);
		l2Len += lenTag;
		lenSnap[0] = (l2Len>>8)&0xff;
		lenSnap[1] = l2Len&0xff;
		memcpy(innerPktContent + 12, &lenSnap[0], 8);
		memcpy(innerPktContent + 20, &tag[0], lenTag);
		//dump_packet(innerPktContent,*pktLen,"Packet with snap");
	}

	//dump_packet(innerPktContent,innerPktContentLen,"Inner Packet");

    /* SRv6 L2 tunnel part */
     do{
        //IPv6
        bzero(&conf,sizeof(conf));
        conf.pktType                            = _PKT_TYPE_IPV6;
        conf.conf_tcp_flag                      = 0;
        conf.ipv6.version                       = 6;
         conf.payload.content               = innerPktContentPtr;
         conf.payload.length                = innerPktContentLen;
	 conf.rhdr.hdrlen		= srhHdrLen;
	 conf.rhdr.nexthdr			 = 143;
	 conf.rhdr.segments_left 	 = srhSegmentLeft;
         conf.ipv6.priority                     = l2tun_tc;
         conf.ipv6.flowlbl[0]  = (flow_label>>16)&0xf;
         conf.ipv6.flowlbl[1]  = (flow_label>>8)&0xff;
         conf.ipv6.flowlbl[2] =  (flow_label>>0)&0xff;
         conf.ipv6.pldlen        = *pktLen;
         conf.ipv6.nxthdr          = l2tun_protocol;
         conf.ipv6.hoplmt        = l2tun_hoplimit;
	if(l2tun_pppoetagif == 1)
	{
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8864;
		conf.pppoe.session 	= l2tun_pppoe_session_id;
	}
	else if(l2tun_pppoetagif == 2)
	{
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8863;
		conf.pppoe.session 	= l2tun_pppoe_session_id;
	}
  	if (l2tun_ctagif) {
		conf.l2Flag 		= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 	= l2tun_cvid;
		conf.cvlan.cprio	= l2tun_cpri;
	}

	if (l2tun_stagif) {
		conf.l2Flag 		= conf.l2Flag | L2_SVLAN;
		conf.svlan.svid 	= l2tun_svid;
		conf.svlan.sprio	= l2tun_spri;
		if(l2tun_tpid)
			conf.svlan.tpid		 = l2tun_tpid;
		else
			conf.svlan.tpid = 0x88a8;
	}      

	if(outer_insert_len_snap)
		conf.l2Flag |= L2_SNAP;

	in6_pton(l2tun_sip,-1,(unsigned char *)&conf.ipv6.src_ip[0],-1,NULL);
       in6_pton(l2tun_dip,-1,(unsigned char *)&conf.ipv6.dst_ip[0],-1,NULL);

	 strtomac((void *)&conf.conf_smac[0], l2tun_smac);
	 strtomac((void *)&conf.conf_dmac[0], l2tun_dmac);
         
	 pppoe_length_exclude_padding = outer_ppp_length_exclude_padding;
	 pppoe_length_include_padding = outer_ppp_length_include_padding;
	 ipv6_length_exclude_padding = outer_ipv6_len_exclude_padding;
	 ipv6_length_include_padding = outer_ipv6_len_include_padding;
	 llc_len_excluding_padding = outer_llc_len_exclude_ingress_padding;
	 //printk("pppoe_length_exclude_padding:%d @ %s %d\n",pppoe_length_exclude_padding,__func__,__LINE__);
	//printk("ipv6_length_exclude_padding:%d @ %s %d\n",ipv6_length_exclude_padding,__func__,__LINE__);
	 //printk("ipv6_length_include_padding:%d @ %s %d\n",ipv6_length_include_padding,__func__,__LINE__);
	 //printk("skip_minPktLen_check:%d @ %s %d\n",skip_minPktLen_check,__func__,__LINE__);
         *pktLen = pktGen(&conf, (int8*)pktBuf);       /* generate pkt in buff */
	 llc_len_excluding_padding = 0;
	pppoe_length_exclude_padding = 0;
	pppoe_length_include_padding = 0;
	ipv6_length_exclude_padding = 0;
	ipv6_length_include_padding = 0;
 
         *pktLen += 4;
	//dump_packet(pktBuf, *pktLen, "\033[1;33;40m[SRv6 L2tunnl packet]\033[0m");
      }while(0);

	return 0;
}

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
	int pppoetagif,int pppoe_session_id)
{
	int i;
	rtl8651_PktConf_t conf;
        unsigned char *innerPktContentPtr = (uint8 *)&innerPktContent[0];
        int innerPktContentLen;

 	bzero(&conf, sizeof(conf));

	for (i=0;i<*pktLen;i++)
		pktData[i] = i&0xff;

	if(*pktLen > 8)
	{
		pktData[0] = 0x70;	//p
		pktData[1] = 0x61;	//a
		pktData[2] = 0x79;	//y
		pktData[3] = 0x6c;	//l
		pktData[4] = 0x6f;	//o
		pktData[5] = 0x61;	//a
		pktData[6] = 0x64;	//d
	}

	if (protocol == IPPROTO_UDP)
		conf.pktType	= _PKT_TYPE_UDP;
	else if (protocol == IPPROTO_TCP)
		conf.pktType	= _PKT_TYPE_TCP;
	else if (protocol == IPPROTO_ICMP)
		conf.pktType	= _PKT_TYPE_ICMP;
	else
		conf.pktType	= _PKT_TYPE_IP;

	conf.conf_tcp_flag			= (conf.pktType == _PKT_TYPE_TCP)?tcpFlag:0;
	conf.ip.id					= 0x123;
	conf.ip.ttl 				= ttl;
	conf.ip.tos 				= iptos;
	conf.payload.length 		= *pktLen;
	conf.ip.version				= 0x4;
	conf.ip.headerLen			= 20;
	conf.payload.content		= pktData;
	conf.pppoe.type 			= 0x0800;
	conf.pppoe.session			= 0;
	conf.cvlan.cprio			= 1;
	conf.l2Flag 				= 0;

	//conf.conf_tcp_flag = TH_RST;
	if (ctagif) {
		conf.l2Flag 		= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 	= cvid;
		conf.cvlan.cprio	= cpri;
	}

	if (stagif) {
		conf.l2Flag 		= conf.l2Flag | L2_SVLAN;
		conf.svlan.svid 	= svid;
		conf.svlan.sprio	= spri;
		if(tpid)
			conf.svlan.tpid		 = tpid;
		else
			conf.svlan.tpid = 0x88a8;
	}

	if (cputag!=NULL) {
		conf.l2Flag 		= conf.l2Flag | L2_CPUTAG_OTHER;
		conf.conf_ctag_protocol	= 0x4;
		conf.conf_ctag_reason	= cputag->res;
		conf.conf_ctag_priority	= cputag->pri;
		conf.conf_ctag_streamid = cputag->streamid;
//		conf.conf_ctag_ttlde	= cputag->ttl_1;
		conf.conf_ctag_l3r	= cputag->l3r;
		conf.conf_ctag_org	= cputag->org;
		conf.conf_ctag_spa	= cputag->spa;
		conf.conf_ctag_extspa	= cputag->extspa;
		conf.conf_ctag_extdpmsk	= cputag->epmsk;
		conf.conf_ctag_fbi = cputag->fbi;
		conf.conf_ctag_fb_index = cputag->fbindex;
	}else if (cputagTx != NULL) {
		conf.l2Flag 		= conf.l2Flag | L2_CPUTAG_TX2;
		conf.conf_ctag_protocol	= 0x4;
		conf.ctag_tx2.txmsk = cputagTx->txmsk;
		conf.ctag_tx2.prisel = cputagTx->prisel;
		conf.ctag_tx2.pri = cputagTx->pri;
		conf.ctag_tx2.keep = cputagTx->keep;
		conf.ctag_tx2.dislrn = cputagTx->dislrn;
		conf.ctag_tx2.psel = cputagTx->psel;
		conf.ctag_tx2.directTx = cputagTx->directTx;
		conf.ctag_tx2.extspa = cputagTx->extspa;
		conf.ctag_tx2.pppoeact = cputagTx->pppoeact;
		conf.ctag_tx2.pppoeidx = cputagTx->pppoeidx;
		conf.ctag_tx2.streamid = cputagTx->streamid;

	}

	if (pppoetagif==1) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8864;
		conf.pppoe.session 	= pppoe_session_id;
	}else if(pppoetagif==2) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8863;
		conf.pppoe.session 	= pppoe_session_id;
	}

	strtomac((void *)&conf.conf_smac[0], smac);
	strtomac((void *)&conf.conf_dmac[0], dmac);
	conf.conf_sip	= ntohl(inet_addr(sip));
	conf.conf_dip	= ntohl(inet_addr(dip));
	conf.conf_sport = sport;
	conf.conf_dport = dport;
	conf.ip.offset = ipfost;
	conf.ip.mf	 = ipmf;

	pppoe_length_exclude_padding = inner_ppp_length_exclude_padding;
	//printk("pppoe_length_exclude_padding:%d @ %s %d\n",pppoe_length_exclude_padding,__func__,__LINE__);
	ipv4_length_exclude_padding = inner_ipv4_len_exclude_padding;
	//printk("ipv4_length_exclude_padding:%d @ %s %d\n",ipv4_length_exclude_padding,__func__,__LINE__);
	skip_minPktLen_check = inner_skip_minPktLen_check;
	//printk("skip_minPktLen_check:%d @ %s %d\n",skip_minPktLen_check,__func__,__LINE__);
	innerPktContentLen = pktGen(&conf, (int8*)innerPktContentPtr);			 /* generate pkt in buff */
	if(inner_tail_padding_len > 0)
	{
		memset(innerPktContent+innerPktContentLen,0,inner_tail_padding_len);
		innerPktContentLen += inner_tail_padding_len;
	}
	pppoe_length_exclude_padding = 0;
	ipv4_length_exclude_padding = 0;
	skip_minPktLen_check = 0;

	//dump_packet(innerPktContent,innerPktContentLen,"Inner Packet");

    /* SRv6 L2 tunnel part */
     do{
        //IPv6
        bzero(&conf,sizeof(conf));
        conf.pktType                            = _PKT_TYPE_IPV6;
        conf.conf_tcp_flag                      = 0;
        conf.ipv6.version                       = 6;
         conf.payload.content               = innerPktContentPtr;
         conf.payload.length                = innerPktContentLen - outer_ipv6_len_exclude_padding;
	 conf.rhdr.hdrlen			= srhHdrLen;
	 conf.rhdr.nexthdr			 = 143;
	 conf.rhdr.segments_left 	 = srhSegmentLeft;
         conf.ipv6.priority                     = l2tun_tc;
         conf.ipv6.flowlbl[0]  = (flow_label>>16)&0xf;
         conf.ipv6.flowlbl[1]  = (flow_label>>8)&0xff;
         conf.ipv6.flowlbl[2] =  (flow_label>>0)&0xff;
         conf.ipv6.pldlen        = *pktLen;
         conf.ipv6.nxthdr          = l2tun_protocol;
         conf.ipv6.hoplmt        = l2tun_hoplimit;
	if(l2tun_pppoetagif == 1)
	{
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8864;
		conf.pppoe.session 	= l2tun_pppoe_session_id;
	}
	else if(l2tun_pppoetagif == 2)
	{
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8863;
		conf.pppoe.session 	= l2tun_pppoe_session_id;
	}
  	if (l2tun_ctagif) {
		conf.l2Flag 		= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 	= l2tun_cvid;
		conf.cvlan.cprio	= l2tun_cpri;
	}

	if (l2tun_stagif) {
		conf.l2Flag 		= conf.l2Flag | L2_SVLAN;
		conf.svlan.svid 	= l2tun_svid;
		conf.svlan.sprio	= l2tun_spri;
		if(l2tun_tpid)
			conf.svlan.tpid		 = l2tun_tpid;
		else
			conf.svlan.tpid = 0x88a8;
	}      
	in6_pton(l2tun_sip,-1,(unsigned char *)&conf.ipv6.src_ip[0],-1,NULL);
         in6_pton(l2tun_dip,-1,(unsigned char *)&conf.ipv6.dst_ip[0],-1,NULL);

	 strtomac((void *)&conf.conf_smac[0], l2tun_smac);
	 strtomac((void *)&conf.conf_dmac[0], l2tun_dmac);
         
	 pppoe_length_exclude_padding = outer_ppp_length_exclude_padding;
	 //printk("pppoe_length_exclude_padding:%d @ %s %d\n",pppoe_length_exclude_padding,__func__,__LINE__);
	 ipv6_length_exclude_padding = outer_ipv6_len_exclude_padding;
	//printk("ipv6_length_exclude_padding:%d @ %s %d\n",ipv6_length_exclude_padding,__func__,__LINE__);
	 //printk("skip_minPktLen_check:%d @ %s %d\n",skip_minPktLen_check,__func__,__LINE__);
	llc_len_excluding_padding = outer_llc_len_exclude_ingress_padding;
         *pktLen = pktGen(&conf, (int8*)pktBuf);       /* generate pkt in buff */
	 llc_len_excluding_padding = 0;
	 pppoe_length_exclude_padding = 0;
	 ipv6_length_exclude_padding = 0;
 
         *pktLen += 4;
	//dump_packet(pktBuf, *pktLen, "\033[1;33;40m[SRv6 L2tunnl packet]\033[0m");
      }while(0);

	return 0;
}

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
	int pppoetagif,int pppoe_session_id)
{
	int i;
	rtl8651_PktConf_t conf;
        unsigned char *innerPktContentPtr = (uint8 *)&innerPktContent[0];
        int innerPktContentLen;

 	bzero(&conf, sizeof(conf));

	for (i=0;i<*pktLen;i++)
		pktData[i] = i&0xff;

	if(*pktLen > 8)
	{
		pktData[0] = 0x70;	//p
		pktData[1] = 0x61;	//a
		pktData[2] = 0x79;	//y
		pktData[3] = 0x6c;	//l
		pktData[4] = 0x6f;	//o
		pktData[5] = 0x61;	//a
		pktData[6] = 0x64;	//d
	}

	if (nextheader == IPPROTO_UDP)
		conf.pktType	= _PKT_TYPE_V6UDP;
	else if (nextheader == IPPROTO_TCP)
		conf.pktType	= _PKT_TYPE_V6TCP;
	else if (nextheader == IPPROTO_ICMP)
		conf.pktType	= _PKT_TYPE_V6ICMP;
	else
		conf.pktType	= _PKT_TYPE_IPV6;

	conf.l2Flag 			= 0;
	conf.ipv6.version 		= 6;
	conf.ipv6.priority		= tc;
	conf.ipv6.flowlbl[0]  	= (flowlabel>>16)&0xf;
	conf.ipv6.flowlbl[1]  	= (flowlabel>>8)&0xff;
	conf.ipv6.flowlbl[2] 		= (flowlabel>>0)&0xff;
	conf.payload.content	= pktData;	  
	conf.payload.length 	= *pktLen;
	conf.ipv6.nxthdr		= nextheader;
	conf.ipv6.hoplmt		= hoplimit;
	conf.ip.mf				= ipmf;
	conf.ip.offset			= ipfost;
	conf.conf_tcp_flag		= tcpFlag;

	strtomac((void *)&conf.conf_smac[0], smac);
	strtomac((void *)&conf.conf_dmac[0], dmac);
	in6_pton(sip,-1,(unsigned char *)&conf.ipv6.src_ip[0],-1,NULL);
	in6_pton(dip,-1,(unsigned char *)&conf.ipv6.dst_ip[0],-1,NULL);
	conf.conf_sport = sport;
	conf.conf_dport = dport;

	//conf.conf_tcp_flag = TH_RST;
	if (ctagif) {
		conf.l2Flag 		= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 	= cvid;
		conf.cvlan.cprio	= cpri;
	}

	if (stagif) {
		conf.l2Flag 		= conf.l2Flag | L2_SVLAN;
		conf.svlan.svid 	= svid;
		conf.svlan.sprio	= spri;
		if(tpid)
			conf.svlan.tpid		 = tpid;
		else
			conf.svlan.tpid = 0x88a8;
	}

	if(inner_insert_len_snap_after_vlan || inner_insert_len_snap_before_vlan)
		conf.l2Flag |= L2_SNAP;

	if (cputag!=NULL) {
		conf.l2Flag 		= conf.l2Flag | L2_CPUTAG_OTHER;
		conf.conf_ctag_protocol	= 0x4;
		conf.conf_ctag_reason	= cputag->res;
		conf.conf_ctag_priority	= cputag->pri;
		conf.conf_ctag_streamid = cputag->streamid;
//		conf.conf_ctag_ttlde	= cputag->ttl_1;
		conf.conf_ctag_l3r	= cputag->l3r;
		conf.conf_ctag_org	= cputag->org;
		conf.conf_ctag_spa	= cputag->spa;
		conf.conf_ctag_extspa	= cputag->extspa;
		conf.conf_ctag_extdpmsk	= cputag->epmsk;
		conf.conf_ctag_fbi = cputag->fbi;
		conf.conf_ctag_fb_index = cputag->fbindex;
	}else if (cputagTx != NULL) {
		conf.l2Flag 		= conf.l2Flag | L2_CPUTAG_TX2;
		conf.conf_ctag_protocol	= 0x4;
		conf.ctag_tx2.txmsk = cputagTx->txmsk;
		conf.ctag_tx2.prisel = cputagTx->prisel;
		conf.ctag_tx2.pri = cputagTx->pri;
		conf.ctag_tx2.keep = cputagTx->keep;
		conf.ctag_tx2.dislrn = cputagTx->dislrn;
		conf.ctag_tx2.psel = cputagTx->psel;
		conf.ctag_tx2.directTx = cputagTx->directTx;
		conf.ctag_tx2.extspa = cputagTx->extspa;
		conf.ctag_tx2.pppoeact = cputagTx->pppoeact;
		conf.ctag_tx2.pppoeidx = cputagTx->pppoeidx;
		conf.ctag_tx2.streamid = cputagTx->streamid;

	}

	if (pppoetagif==1) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8864;
		conf.pppoe.session 	= pppoe_session_id;
	}else if(pppoetagif==2) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8863;
		conf.pppoe.session 	= pppoe_session_id;
	}

	 pppoe_length_exclude_padding = inner_ppp_length_exclude_padding;
	 //printk("pppoe_length_exclude_padding:%d @ %s %d\n",pppoe_length_exclude_padding,__func__,__LINE__);
	 ipv6_length_exclude_padding = inner_ipv6_len_exclude_padding;
	 //printk("ipv6_length_exclude_padding:%d @ %s %d\n",ipv6_length_exclude_padding,__func__,__LINE__);
	 skip_minPktLen_check = inner_skip_minPktLen_check;
	 //printk("skip_minPktLen_check:%d @ %s %d\n",skip_minPktLen_check,__func__,__LINE__);
	 llc_len_including_padding = inner_llc_len_include_ingress_padding;
 	innerPktContentLen = pktGen(&conf, (int8*)innerPktContentPtr);			 /* generate pkt in buff */
	llc_len_including_padding = 0;
	 memset(innerPktContent+innerPktContentLen,0,inner_tail_padding_len);
	 innerPktContentLen += inner_tail_padding_len;
	skip_minPktLen_check = 0;
	pppoe_length_exclude_padding = 0;
	ipv6_length_exclude_padding = 0;

	if(inner_insert_len_snap_before_vlan && (ctagif || stagif))
	{
		unsigned char lenSnap[8];
		unsigned char tag[8];
		unsigned short l2Len;
		int lenTag = (ctagif?4:0) + (stagif?4:0);
		
		memcpy(&tag[0], innerPktContent+12, lenTag);
		memcpy(&lenSnap[0], innerPktContent+12+lenTag, 8);
		l2Len = (lenSnap[0]<<8|lenSnap[1]);
		l2Len += lenTag;
		lenSnap[0] = (l2Len>>8)&0xff;
		lenSnap[1] = l2Len&0xff;
		memcpy(innerPktContent + 12, &lenSnap[0], 8);
		memcpy(innerPktContent + 20, &tag[0], lenTag);
//		dump_packet(innerPktContent,*pktLen,"Packet with snap");
	}

	//dump_packet(innerPktContent,innerPktContentLen,"Inner Packet");

    /* SRv6 L2 tunnel part */
     do{
        //IPv6
        bzero(&conf,sizeof(conf));
        conf.pktType                            = _PKT_TYPE_IPV6;
        conf.conf_tcp_flag                      = 0;
        conf.ipv6.version                       = 6;
         conf.payload.content               = innerPktContentPtr;
         conf.payload.length                = innerPktContentLen;
	 conf.rhdr.hdrlen			= srhHdrLen;
	 conf.rhdr.nexthdr			 = 143;
	 conf.rhdr.segments_left 	 = srhSegmentLeft;
         conf.ipv6.priority                     = l2tun_tc;
         conf.ipv6.flowlbl[0]  = (l2tun_flowlabel>>16)&0xf;
         conf.ipv6.flowlbl[1]  = (l2tun_flowlabel>>8)&0xff;
         conf.ipv6.flowlbl[2] =  (l2tun_flowlabel>>0)&0xff;
         conf.ipv6.pldlen        = *pktLen;
         conf.ipv6.nxthdr          = l2tun_protocol;
         conf.ipv6.hoplmt        = l2tun_hoplimit;

	if(l2tun_pppoetagif == 1)
	{
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8864;
		conf.pppoe.session 	= l2tun_pppoe_session_id;
	}
	else if(l2tun_pppoetagif == 2)
	{
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8863;
		conf.pppoe.session 	= l2tun_pppoe_session_id;
	}
  	if (l2tun_ctagif) {
		conf.l2Flag 		= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 	= l2tun_cvid;
		conf.cvlan.cprio	= l2tun_cpri;
	}

	if (l2tun_stagif) {
		conf.l2Flag 		= conf.l2Flag | L2_SVLAN;
		conf.svlan.svid 	= l2tun_svid;
		conf.svlan.sprio	= l2tun_spri;
		if(l2tun_tpid)
			conf.svlan.tpid		 = l2tun_tpid;
		else
			conf.svlan.tpid = 0x88a8;
	}      

	if(outer_insert_len_snap)
		conf.l2Flag	|= L2_SNAP;

	in6_pton(l2tun_sip,-1,(unsigned char *)&conf.ipv6.src_ip[0],-1,NULL);
       in6_pton(l2tun_dip,-1,(unsigned char *)&conf.ipv6.dst_ip[0],-1,NULL);

	strtomac((void *)&conf.conf_smac[0], l2tun_smac);
	strtomac((void *)&conf.conf_dmac[0], l2tun_dmac);
         
	pppoe_length_exclude_padding = outer_ppp_length_exclude_padding;
	ipv6_length_exclude_padding = outer_ipv6_len_exclude_padding;
	llc_len_excluding_padding = outer_llc_len_exclude_ingress_padding;
      *pktLen = pktGen(&conf, (int8*)pktBuf);       /* generate pkt in buff */
	  llc_len_excluding_padding = 0;
	pppoe_length_exclude_padding = 0;
	ipv6_length_exclude_padding = 0;

#if 0
	 if(outer_insert_len_snap)
	 {
		 int offset = 12;
		 unsigned short *l2Len;
		 offset += ((l2tun_ctagif?4:0) + (l2tun_stagif?4:0));
		 memmove(pktBuf + offset + 2, pktBuf + offset, *pktLen -offset);
		 l2Len = (unsigned short *)(pktBuf + offset);
		 *l2Len = (*pktLen - offset);
		 *pktLen += 2;
		 dump_packet(pktBuf,*pktLen,"Outer packet with snap");
	 }
#endif

         *pktLen += 4;
	//dump_packet(pktBuf, *pktLen, "\033[1;33;40m[SRv6 L2tunnl packet]\033[0m");
      }while(0);

	return 0;
}

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
	int pppoetagif,int pppoe_session_id)
{
	int i;
	rtl8651_PktConf_t conf;
        unsigned char *innerPktContentPtr = (uint8 *)&innerPktContent[0];
        int innerPktContentLen;

 	bzero(&conf, sizeof(conf));

	for (i=0;i<*pktLen;i++)
		pktData[i] = i&0xff;

	if(*pktLen > 8)
	{
		pktData[0] = 0x70;	//p
		pktData[1] = 0x61;	//a
		pktData[2] = 0x79;	//y
		pktData[3] = 0x6c;	//l
		pktData[4] = 0x6f;	//o
		pktData[5] = 0x61;	//a
		pktData[6] = 0x64;	//d
	}

	if (nextheader == IPPROTO_UDP)
		conf.pktType	= _PKT_TYPE_V6UDP;
	else if(nextheader == IPPROTO_TCP)
		conf.pktType	= _PKT_TYPE_V6TCP;
	else if(nextheader == IPPROTO_ICMP)
		conf.pktType	= _PKT_TYPE_V6ICMP;
	else
		conf.pktType	= _PKT_TYPE_IPV6;

	conf.l2Flag 			= 0;
	conf.ipv6.version 		= 6;
	conf.ipv6.priority		= tc;
	conf.ipv6.flowlbl[0]  	= (flowlabel>>16)&0xf;
	conf.ipv6.flowlbl[1]  	= (flowlabel>>8)&0xff;
	conf.ipv6.flowlbl[2] 		= (flowlabel>>0)&0xff;
	conf.payload.content	= pktData;	  
	conf.payload.length 	= *pktLen;
	conf.ipv6.nxthdr		= nextheader;
	conf.ipv6.hoplmt		= hoplimit;
	if(mf_frag)
	{
		conf.ipv6.nxthdr = 44;
		conf.frghdr.nexthdr = conf.pktType;
		conf.frghdr.fragment = mf_frag;
		conf.frghdr.id = 0x123;
	}

	strtomac((void *)&conf.conf_smac[0], smac);
	strtomac((void *)&conf.conf_dmac[0], dmac);
	in6_pton(sip,-1,(unsigned char *)&conf.ipv6.src_ip[0],-1,NULL);
	in6_pton(dip,-1,(unsigned char *)&conf.ipv6.dst_ip[0],-1,NULL);
	conf.conf_sport = sport;
	conf.conf_dport = dport;

	//conf.conf_tcp_flag = TH_RST;
	if (ctagif) {
		conf.l2Flag 		= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 	= cvid;
		conf.cvlan.cprio	= cpri;
	}

	if (stagif) {
		conf.l2Flag 		= conf.l2Flag | L2_SVLAN;
		conf.svlan.svid 	= svid;
		conf.svlan.sprio	= spri;
		if(tpid)
			conf.svlan.tpid		 = tpid;
		else
			conf.svlan.tpid = 0x88a8;
	}

	if (cputag!=NULL) {
		conf.l2Flag 		= conf.l2Flag | L2_CPUTAG_OTHER;
		conf.conf_ctag_protocol	= 0x4;
		conf.conf_ctag_reason	= cputag->res;
		conf.conf_ctag_priority	= cputag->pri;
		conf.conf_ctag_streamid = cputag->streamid;
//		conf.conf_ctag_ttlde	= cputag->ttl_1;
		conf.conf_ctag_l3r	= cputag->l3r;
		conf.conf_ctag_org	= cputag->org;
		conf.conf_ctag_spa	= cputag->spa;
		conf.conf_ctag_extspa	= cputag->extspa;
		conf.conf_ctag_extdpmsk	= cputag->epmsk;
		conf.conf_ctag_fbi = cputag->fbi;
		conf.conf_ctag_fb_index = cputag->fbindex;
	}else if (cputagTx != NULL) {
		conf.l2Flag 		= conf.l2Flag | L2_CPUTAG_TX2;
		conf.conf_ctag_protocol	= 0x4;
		conf.ctag_tx2.txmsk = cputagTx->txmsk;
		conf.ctag_tx2.prisel = cputagTx->prisel;
		conf.ctag_tx2.pri = cputagTx->pri;
		conf.ctag_tx2.keep = cputagTx->keep;
		conf.ctag_tx2.dislrn = cputagTx->dislrn;
		conf.ctag_tx2.psel = cputagTx->psel;
		conf.ctag_tx2.directTx = cputagTx->directTx;
		conf.ctag_tx2.extspa = cputagTx->extspa;
		conf.ctag_tx2.pppoeact = cputagTx->pppoeact;
		conf.ctag_tx2.pppoeidx = cputagTx->pppoeidx;
		conf.ctag_tx2.streamid = cputagTx->streamid;
	}

	if (pppoetagif==1) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8864;
		conf.pppoe.session 	= pppoe_session_id;
	}else if(pppoetagif==2) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8863;
		conf.pppoe.session 	= pppoe_session_id;
	}

	 pppoe_length_exclude_padding = inner_ppp_length_exclude_padding;
	 //printk("pppoe_length_exclude_padding:%d @ %s %d\n",pppoe_length_exclude_padding,__func__,__LINE__);
	 ipv6_length_exclude_padding = inner_ipv6_len_exclude_padding;
	 //printk("ipv6_length_exclude_padding:%d @ %s %d\n",ipv6_length_exclude_padding,__func__,__LINE__);
	 skip_minPktLen_check = inner_skip_minPktLen_check;
	 //printk("skip_minPktLen_check:%d @ %s %d\n",skip_minPktLen_check,__func__,__LINE__);
 	innerPktContentLen = pktGen(&conf, (int8*)innerPktContentPtr);			 /* generate pkt in buff */
	 memset(innerPktContent+innerPktContentLen,0,inner_tail_padding_len);
	 innerPktContentLen += inner_tail_padding_len;
	skip_minPktLen_check = 0;
	pppoe_length_exclude_padding = 0;
	ipv6_length_exclude_padding = 0;
	

	//dump_packet(innerPktContent,innerPktContentLen,"Inner Packet");

    /* SRv6 L2 tunnel part */
     do{
        //IPv6
        bzero(&conf,sizeof(conf));
        conf.pktType                            = _PKT_TYPE_IPV6;
        conf.conf_tcp_flag                      = 0;
        conf.ipv6.version                       = 6;
         conf.payload.content               = innerPktContentPtr;
         conf.payload.length                = innerPktContentLen - outer_ipv6_len_exclude_padding;
	 conf.rhdr.hdrlen			= srhHdrLen;
	 conf.rhdr.nexthdr			 = 143;
	 conf.rhdr.segments_left 	 = srhSegmentLeft;
         conf.ipv6.priority                     = l2tun_tc;
         conf.ipv6.flowlbl[0]  = (l2tun_flowlabel>>16)&0xf;
         conf.ipv6.flowlbl[1]  = (l2tun_flowlabel>>8)&0xff;
         conf.ipv6.flowlbl[2] =  (l2tun_flowlabel>>0)&0xff;
         conf.ipv6.pldlen        = *pktLen;
         conf.ipv6.nxthdr          = l2tun_protocol;
         conf.ipv6.hoplmt        = l2tun_hoplimit;
	if(l2tun_pppoetagif == 1)
	{
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8864;
		conf.pppoe.session 	= l2tun_pppoe_session_id;
	}
	else if(l2tun_pppoetagif == 2)
	{
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8863;
		conf.pppoe.session 	= l2tun_pppoe_session_id;
	}
  	if (l2tun_ctagif) {
		conf.l2Flag 		= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 	= l2tun_cvid;
		conf.cvlan.cprio	= l2tun_cpri;
	}

	if (l2tun_stagif) {
		conf.l2Flag 		= conf.l2Flag | L2_SVLAN;
		conf.svlan.svid 	= l2tun_svid;
		conf.svlan.sprio	= l2tun_spri;
		if(l2tun_tpid)
			conf.svlan.tpid		 = l2tun_tpid;
		else
			conf.svlan.tpid = 0x88a8;
	}      
	in6_pton(l2tun_sip,-1,(unsigned char *)&conf.ipv6.src_ip[0],-1,NULL);
         in6_pton(l2tun_dip,-1,(unsigned char *)&conf.ipv6.dst_ip[0],-1,NULL);

	 strtomac((void *)&conf.conf_smac[0], l2tun_smac);
	 strtomac((void *)&conf.conf_dmac[0], l2tun_dmac);
         
	 pppoe_length_exclude_padding = outer_ppp_length_exclude_padding;
	 ipv6_length_exclude_padding = outer_ipv6_len_exclude_padding;
	 llc_len_excluding_padding = outer_llc_len_exclude_ingress_padding;
      *pktLen = pktGen(&conf, (int8*)pktBuf);       /* generate pkt in buff */
	  llc_len_excluding_padding = 0;
	  pppoe_length_exclude_padding = 0;
	  ipv6_length_exclude_padding = 0;

         *pktLen += 4;
	//dump_packet(pktBuf, *pktLen, "\033[1;33;40m[SRv6 L2tunnl packet]\033[0m");
      }while(0);

	return 0;
}

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
	int pppoetagif,int pppoe_session_id)
{
	int i;
	rtl8651_PktConf_t conf;
        unsigned char *innerPktContentPtr = (uint8 *)&innerPktContent[0];
        int innerPktContentLen;

 	bzero(&conf, sizeof(conf));

	for (i=0;i<*pktLen;i++)
		pktData[i] = i&0xff;

	if(*pktLen > 8)
	{
		pktData[0] = 0x70;	//p
		pktData[1] = 0x61;	//a
		pktData[2] = 0x79;	//y
		pktData[3] = 0x6c;	//l
		pktData[4] = 0x6f;	//o
		pktData[5] = 0x61;	//a
		pktData[6] = 0x64;	//d
	}

	conf.pktType	= _PKT_TYPE_ETHER;
	conf.payload.length 	= *pktLen;
	conf.payload.content	= pktData;
	conf.conf_ethtype	= etype;
	conf.pppoe.session	= 0;
	conf.cvlan.cprio	= 1;
	conf.l2Flag 		= 0;

	//conf.conf_tcp_flag = TH_RST;
	if (ctagif) {
		conf.l2Flag 		= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 	= cvid;
		conf.cvlan.cprio	= cpri;
	}

	if (stagif) {
		conf.l2Flag 		= conf.l2Flag | L2_SVLAN;
		conf.svlan.svid 	= svid;
		conf.svlan.sprio	= spri;
		if(tpid)
			conf.svlan.tpid		 = tpid;
		else
			conf.svlan.tpid = 0x88a8;
	}

	if(inner_insert_len_snap_after_vlan || inner_insert_len_snap_before_vlan)
	{
		conf.l2Flag |= L2_SNAP;
		conf.llc.dsap = 0xaa;
		conf.llc.ssap = 0xaa;
	}

	if (cputag!=NULL) {
		conf.l2Flag 		= conf.l2Flag | L2_CPUTAG_OTHER;
		conf.conf_ctag_protocol	= 0x4;
		conf.conf_ctag_reason	= cputag->res;
		conf.conf_ctag_priority	= cputag->pri;
		conf.conf_ctag_streamid = cputag->streamid;
//		conf.conf_ctag_ttlde	= cputag->ttl_1;
		conf.conf_ctag_l3r	= cputag->l3r;
		conf.conf_ctag_org	= cputag->org;
		conf.conf_ctag_spa	= cputag->spa;
		conf.conf_ctag_extspa	= cputag->extspa;
		conf.conf_ctag_extdpmsk	= cputag->epmsk;
		conf.conf_ctag_fbi = cputag->fbi;
		conf.conf_ctag_fb_index = cputag->fbindex;
	}else if (cputagTx != NULL) {
		conf.l2Flag 		= conf.l2Flag | L2_CPUTAG_TX2;
		conf.conf_ctag_protocol	= 0x4;
		conf.ctag_tx2.txmsk = cputagTx->txmsk;
		conf.ctag_tx2.prisel = cputagTx->prisel;
		conf.ctag_tx2.pri = cputagTx->pri;
		conf.ctag_tx2.keep = cputagTx->keep;
		conf.ctag_tx2.dislrn = cputagTx->dislrn;
		conf.ctag_tx2.psel = cputagTx->psel;
		conf.ctag_tx2.directTx = cputagTx->directTx;
		conf.ctag_tx2.extspa = cputagTx->extspa;
		conf.ctag_tx2.pppoeact = cputagTx->pppoeact;
		conf.ctag_tx2.pppoeidx = cputagTx->pppoeidx;
		conf.ctag_tx2.streamid = cputagTx->streamid;
	}

	if (pppoetagif==1) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8864;
		conf.pppoe.session 	= pppoe_session_id;
	}else if(pppoetagif==2) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8863;
		conf.pppoe.session 	= pppoe_session_id;
	}

	strtomac((void *)&conf.conf_smac[0], smac);
	strtomac((void *)&conf.conf_dmac[0], dmac);

	pppoe_length_exclude_padding = inner_ppp_length_exclude_padding;
	skip_minPktLen_check = inner_skip_minPktLen_check;
	llc_len_including_padding = inner_llc_len_include_ingress_padding;
	innerPktContentLen = pktGen(&conf, (int8*)innerPktContentPtr);			 /* generate pkt in buff */
	//dump_packet(innerPktContentPtr,innerPktContentLen,"Original inner packet");
	if(inner_tail_padding_len > 0)
	{
		memset(innerPktContent+innerPktContentLen,0,inner_tail_padding_len);
		innerPktContentLen += inner_tail_padding_len;
		//dump_packet(innerPktContentPtr,innerPktContentLen,"Original inner packet with padding");
	}
	llc_len_including_padding = 0;
	pppoe_length_exclude_padding = 0;
	skip_minPktLen_check = 0;

	if(inner_insert_len_snap_before_vlan && (ctagif || stagif))
	{
		unsigned char lenSnap[8];
		unsigned char tag[8];
		unsigned short l2Len;
		int lenTag = (ctagif?4:0) + (stagif?4:0);
		
		memcpy(&tag[0], innerPktContent+12, lenTag);
		memcpy(&lenSnap[0], innerPktContent+12+lenTag, 8);
		l2Len = (lenSnap[0]<<8|lenSnap[1]);
		l2Len += lenTag;
		lenSnap[0] = (l2Len>>8)&0xff;
		lenSnap[1] = l2Len&0xff;
		memcpy(innerPktContent + 12, &lenSnap[0], 8);
		memcpy(innerPktContent + 20, &tag[0], lenTag);
		//dump_packet(innerPktContent,*pktLen,"Packet with snap");
	}

	//dump_packet(innerPktContent,innerPktContentLen,"Inner Packet");

    /* SRv6 L2 tunnel part */
     do{
        //IPv6
        bzero(&conf,sizeof(conf));
        conf.pktType                            = _PKT_TYPE_IPV6;
        conf.conf_tcp_flag                      = 0;
        conf.ipv6.version                       = 6;
         conf.payload.content               = innerPktContentPtr;
         conf.payload.length                = innerPktContentLen;
	conf.rhdr.hdrlen			   = srhHdrLen;
	conf.rhdr.segments_left 		= srhSegmentLeft;
         conf.ipv6.priority                     = l2tun_tc;
         conf.ipv6.flowlbl[0]  = (flow_label>>16)&0xf;
         conf.ipv6.flowlbl[1]  = (flow_label>>8)&0xff;
         conf.ipv6.flowlbl[2] =  (flow_label>>0)&0xff;
         conf.ipv6.pldlen        = *pktLen;
         conf.ipv6.nxthdr          = l2tun_protocol;
         conf.ipv6.hoplmt        = l2tun_hoplimit;
	if(l2tun_pppoetagif == 1)
	{
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8864;
		conf.pppoe.session 	= l2tun_pppoe_session_id;
	}
	else if(l2tun_pppoetagif == 2)
	{
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8863;
		conf.pppoe.session 	= l2tun_pppoe_session_id;
	}
  	if (l2tun_ctagif) {
		conf.l2Flag 		= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 	= l2tun_cvid;
		conf.cvlan.cprio	= l2tun_cpri;
	}

	if (l2tun_stagif) {
		conf.l2Flag 		= conf.l2Flag | L2_SVLAN;
		conf.svlan.svid 	= l2tun_svid;
		conf.svlan.sprio	= l2tun_spri;
		if(l2tun_tpid)
			conf.svlan.tpid		 = l2tun_tpid;
		else
			conf.svlan.tpid = 0x88a8;
	}
	if(srhHdrLen)
		conf.l2Flag |= (RouHdr | SRv6Hdr);

	if(outer_insert_len_snap)
		conf.l2Flag |= L2_SNAP;
	
	in6_pton(l2tun_sip,-1,(unsigned char *)&conf.ipv6.src_ip[0],-1,NULL);
       in6_pton(l2tun_dip,-1,(unsigned char *)&conf.ipv6.dst_ip[0],-1,NULL);

	strtomac((void *)&conf.conf_smac[0], l2tun_smac);
	strtomac((void *)&conf.conf_dmac[0], l2tun_dmac);
         
	 pppoe_length_exclude_padding = outer_ppp_length_exclude_padding;
	 pppoe_length_include_padding = outer_ppp_length_include_padding;
	 ipv6_length_exclude_padding = outer_ipv6_len_exclude_padding;
	 ipv6_length_include_padding = outer_ipv6_len_include_padding;
	 llc_len_including_padding = outer_llc_len_include_ingress_padding;
	 llc_len_excluding_padding = outer_llc_len_exclude_ingress_padding;
         *pktLen = pktGen(&conf, (int8*)pktBuf);       /* generate pkt in buff */
	llc_len_including_padding = 0;
	llc_len_excluding_padding = 0;
	 pppoe_length_exclude_padding = 0;
	 pppoe_length_include_padding = 0;
	 ipv6_length_exclude_padding = 0;
	 ipv6_length_include_padding = 0;

	if(outer_insert_len_snap && outer_insert_len_snap_before_vlan && (l2tun_ctagif || l2tun_stagif))
	{
		unsigned char lenSnap[8];
		unsigned char tag[8];
		unsigned short l2Len;
		int lenTag = (l2tun_ctagif?4:0) + (l2tun_stagif?4:0);
		
		memcpy(&tag[0], pktBuf+12, lenTag);
		memcpy(&lenSnap[0], pktBuf+12+lenTag, 8);
		l2Len = (lenSnap[0]<<8|lenSnap[1]);
		l2Len += lenTag;
		lenSnap[0] = (l2Len>>8)&0xff;
		lenSnap[1] = l2Len&0xff;
		memcpy(pktBuf + 12, &lenSnap[0], 8);
		memcpy(pktBuf + 20, &tag[0], lenTag);
		//dump_packet(innerPktContent,*pktLen,"Packet with snap");
	}

	 
	// dump_packet(pktBuf, *pktLen, "\033[1;33;40m[SRv6 L2tunnl packet without sids]\033[0m");
	if(inner_tail_padding_len > 0 && outer_ipv6_len_exclude_padding > 0)
	{
		memset(pktBuf+*pktLen,0,inner_tail_padding_len);
		*pktLen += inner_tail_padding_len;
		//dump_packet(innerPktContentPtr,innerPktContentLen,"Original inner packet with padding");
	}

	 //SRH segment list
	 if(*pktLen >= (innerPktContentLen + (srhHdrLen<<3)))
	 {
	 	int nSeg = (srhLastEntry+1);
	 	for(i=0; i<nSeg ; i++)
			in6_pton(sid[11-nSeg+i],-1,(unsigned char *)&pktBuf[*pktLen-innerPktContentLen-(srhHdrLen<<3)+(i<<4)],-1,NULL);

		//SRv6 Capabilities TLV
		if(srhHdrLen > ((srhLastEntry+1)<<1))
		{
			int startTLV = *pktLen-innerPktContentLen-(srhHdrLen<<3)+(nSeg<<4);
			pktBuf[startTLV] = 1038&0xff; //Type
			pktBuf[startTLV+1] = 6; //Length
			pktBuf[startTLV+2] = 0x80; //Flags
		}
	 }

         *pktLen += 4;
	//dump_packet(pktBuf, *pktLen, "\033[1;33;40m[SRv6 L2tunnl packet]\033[0m");
      }while(0);

	return 0;
}

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
	int pppoetagif,int pppoe_session_id)
{
	int i;
	rtl8651_PktConf_t conf;
        unsigned char *innerPktContentPtr = (uint8 *)&innerPktContent[0];
        int innerPktContentLen;

 	bzero(&conf, sizeof(conf));

	for (i=0;i<*pktLen;i++)
		pktData[i] = i&0xff;

	if(*pktLen > 8)
	{
		pktData[0] = 0x70;	//p
		pktData[1] = 0x61;	//a
		pktData[2] = 0x79;	//y
		pktData[3] = 0x6c;	//l
		pktData[4] = 0x6f;	//o
		pktData[5] = 0x61;	//a
		pktData[6] = 0x64;	//d
	}

	if (protocol == IPPROTO_UDP)
		conf.pktType	= _PKT_TYPE_UDP;
	else if (protocol == IPPROTO_TCP)
		conf.pktType	= _PKT_TYPE_TCP;
	else if (protocol == IPPROTO_ICMP)
		conf.pktType	= _PKT_TYPE_ICMP;
	else
		conf.pktType	= _PKT_TYPE_IP;

	conf.ip.id					= 0x123;
	conf.ip.ttl 				= ttl;
	conf.ip.tos 				= iptos;
	conf.payload.length 		= *pktLen;
	conf.ip.version				= 0x4;
	conf.ip.headerLen			= 20;
	conf.ip.mf					= ipmf;
	conf.ip.offset				= ipfost;
	conf.conf_tcp_flag			= tcpFlag;
	conf.payload.content		= pktData;
	conf.pppoe.type 			= 0x0800;
	conf.pppoe.session			= 0;
	conf.cvlan.cprio			= 1;
	conf.l2Flag 				= 0;

	//conf.conf_tcp_flag = TH_RST;
	if (ctagif) {
		conf.l2Flag 		= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 	= cvid;
		conf.cvlan.cprio	= cpri;
	}

	if (stagif) {
		conf.l2Flag 		= conf.l2Flag | L2_SVLAN;
		conf.svlan.svid 	= svid;
		conf.svlan.sprio	= spri;
		if(tpid)
			conf.svlan.tpid		 = tpid;
		else
			conf.svlan.tpid = 0x88a8;
	}
	
	if(inner_insert_len_snap_after_vlan || inner_insert_len_snap_before_vlan)
	{
		conf.l2Flag |= L2_SNAP;
		conf.llc.dsap = 0xaa;
		conf.llc.ssap = 0xaa;
	}

	if (cputag!=NULL) {
		conf.l2Flag 		= conf.l2Flag | L2_CPUTAG_OTHER;
		conf.conf_ctag_protocol	= 0x4;
		conf.conf_ctag_reason	= cputag->res;
		conf.conf_ctag_priority	= cputag->pri;
		conf.conf_ctag_streamid = cputag->streamid;
//		conf.conf_ctag_ttlde	= cputag->ttl_1;
		conf.conf_ctag_l3r	= cputag->l3r;
		conf.conf_ctag_org	= cputag->org;
		conf.conf_ctag_spa	= cputag->spa;
		conf.conf_ctag_extspa	= cputag->extspa;
		conf.conf_ctag_extdpmsk	= cputag->epmsk;
		conf.conf_ctag_fbi = cputag->fbi;
		conf.conf_ctag_fb_index = cputag->fbindex;
	}else if (cputagTx != NULL) {
		conf.l2Flag 		= conf.l2Flag | L2_CPUTAG_TX2;
		conf.conf_ctag_protocol	= 0x4;
		conf.ctag_tx2.txmsk = cputagTx->txmsk;
		conf.ctag_tx2.prisel = cputagTx->prisel;
		conf.ctag_tx2.pri = cputagTx->pri;
		conf.ctag_tx2.keep = cputagTx->keep;
		conf.ctag_tx2.dislrn = cputagTx->dislrn;
		conf.ctag_tx2.psel = cputagTx->psel;
		conf.ctag_tx2.directTx = cputagTx->directTx;
		conf.ctag_tx2.extspa = cputagTx->extspa;
		conf.ctag_tx2.pppoeact = cputagTx->pppoeact;
		conf.ctag_tx2.pppoeidx = cputagTx->pppoeidx;
		conf.ctag_tx2.streamid = cputagTx->streamid;
	}

	if (pppoetagif==1) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8864;
		conf.pppoe.session 	= pppoe_session_id;
	}else if(pppoetagif==2) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8863;
		conf.pppoe.session 	= pppoe_session_id;
	}

	strtomac((void *)&conf.conf_smac[0], smac);
	strtomac((void *)&conf.conf_dmac[0], dmac);
	conf.conf_sip	= ntohl(inet_addr(sip));
	conf.conf_dip	= ntohl(inet_addr(dip));
	conf.conf_sport = sport;
	conf.conf_dport = dport;

	pppoe_length_exclude_padding = inner_ppp_length_exclude_padding;
	pppoe_length_include_padding = inner_ppp_length_include_padding;
	//printk("pppoe_length_exclude_padding:%d @ %s %d\n",pppoe_length_exclude_padding,__func__,__LINE__);
	ipv4_length_exclude_padding = inner_ipv4_len_exclude_padding;
	//printk("ipv4_length_exclude_padding:%d @ %s %d\n",ipv4_length_exclude_padding,__func__,__LINE__);
	skip_minPktLen_check = inner_skip_minPktLen_check;
	//printk("skip_minPktLen_check:%d @ %s %d\n",skip_minPktLen_check,__func__,__LINE__);
	llc_len_including_padding = inner_llc_len_include_ingress_padding;
	innerPktContentLen = pktGen(&conf, (int8*)innerPktContentPtr);
	/* generate pkt in buff */
	//printk("inner_tail_padding_len:%d @ %s %d\n",inner_tail_padding_len,__func__,__LINE__);
	llc_len_including_padding = 0;
	if(inner_tail_padding_len > 0)
	{
		memset(innerPktContent+innerPktContentLen,0,inner_tail_padding_len);
		innerPktContentLen += inner_tail_padding_len;
	}
	pppoe_length_exclude_padding = 0;
	pppoe_length_include_padding = 0;
	ipv4_length_exclude_padding = 0;
	skip_minPktLen_check = 0;

	if(inner_insert_len_snap_before_vlan && (ctagif || stagif))
	{
		unsigned char lenSnap[8];
		unsigned char tag[8];
		unsigned short l2Len;
		int lenTag = (ctagif?4:0) + (stagif?4:0);
		
		memcpy(&tag[0], innerPktContent+12, lenTag);
		memcpy(&lenSnap[0], innerPktContent+12+lenTag, 8);
		l2Len = (lenSnap[0]<<8|lenSnap[1]);
		l2Len += lenTag;
		lenSnap[0] = (l2Len>>8)&0xff;
		lenSnap[1] = l2Len&0xff;
		memcpy(innerPktContent + 12, &lenSnap[0], 8);
		memcpy(innerPktContent + 20, &tag[0], lenTag);
		//dump_packet(innerPktContent,*pktLen,"Packet with snap");
	}

	//dump_packet(innerPktContent,innerPktContentLen,"Inner Packet");

    /* SRv6 L2 tunnel part */
     do{
        //IPv6
        bzero(&conf,sizeof(conf));
        conf.pktType                            = _PKT_TYPE_IPV6;
        conf.conf_tcp_flag                      = 0;
        conf.ipv6.version                       = 6;
         conf.payload.content               = innerPktContentPtr;
         conf.payload.length                = innerPktContentLen;
	 conf.rhdr.hdrlen		= srhHdrLen;
	 conf.rhdr.nexthdr			 = 143;
	 conf.rhdr.segments_left 	 = srhSegmentLeft;
         conf.ipv6.priority                     = l2tun_tc;
         conf.ipv6.flowlbl[0]  = (flow_label>>16)&0xf;
         conf.ipv6.flowlbl[1]  = (flow_label>>8)&0xff;
         conf.ipv6.flowlbl[2] =  (flow_label>>0)&0xff;
         conf.ipv6.pldlen        = *pktLen;
         conf.ipv6.nxthdr          = l2tun_protocol;
         conf.ipv6.hoplmt        = l2tun_hoplimit;
	if(l2tun_pppoetagif == 1)
	{
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8864;
		conf.pppoe.session 	= l2tun_pppoe_session_id;
	}
	else if(l2tun_pppoetagif == 2)
	{
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8863;
		conf.pppoe.session 	= l2tun_pppoe_session_id;
	}
  	if (l2tun_ctagif) {
		conf.l2Flag 		= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 	= l2tun_cvid;
		conf.cvlan.cprio	= l2tun_cpri;
	}

	if (l2tun_stagif) {
		conf.l2Flag 		= conf.l2Flag | L2_SVLAN;
		conf.svlan.svid 	= l2tun_svid;
		conf.svlan.sprio	= l2tun_spri;
		if(l2tun_tpid)
			conf.svlan.tpid		 = l2tun_tpid;
		else
			conf.svlan.tpid = 0x88a8;
	}      
	if(srhHdrLen)
		conf.l2Flag |= (RouHdr | SRv6Hdr);

	if(outer_insert_len_snap)
		conf.l2Flag |= L2_SNAP;

	in6_pton(l2tun_sip,-1,(unsigned char *)&conf.ipv6.src_ip[0],-1,NULL);
       in6_pton(l2tun_dip,-1,(unsigned char *)&conf.ipv6.dst_ip[0],-1,NULL);

	 strtomac((void *)&conf.conf_smac[0], l2tun_smac);
	 strtomac((void *)&conf.conf_dmac[0], l2tun_dmac);
         
	 pppoe_length_exclude_padding = outer_ppp_length_exclude_padding;
	 pppoe_length_include_padding = outer_ppp_length_include_padding;
	 ipv6_length_exclude_padding = outer_ipv6_len_exclude_padding;
	 ipv6_length_include_padding = outer_ipv6_len_include_padding;
	 llc_len_excluding_padding = outer_llc_len_exclude_ingress_padding;
	 //printk("pppoe_length_exclude_padding:%d @ %s %d\n",pppoe_length_exclude_padding,__func__,__LINE__);
	//printk("ipv6_length_exclude_padding:%d @ %s %d\n",ipv6_length_exclude_padding,__func__,__LINE__);
	 //printk("ipv6_length_include_padding:%d @ %s %d\n",ipv6_length_include_padding,__func__,__LINE__);
	 //printk("skip_minPktLen_check:%d @ %s %d\n",skip_minPktLen_check,__func__,__LINE__);
         *pktLen = pktGen(&conf, (int8*)pktBuf);       /* generate pkt in buff */
	 llc_len_excluding_padding = 0;
	pppoe_length_exclude_padding = 0;
	pppoe_length_include_padding = 0;
	ipv6_length_exclude_padding = 0;
	ipv6_length_include_padding = 0;


 
	 if(outer_insert_len_snap && outer_insert_len_snap_before_vlan && (l2tun_ctagif || l2tun_stagif))
	 {
		 unsigned char lenSnap[8];
		 unsigned char tag[8];
		 unsigned short l2Len;
		 int lenTag = (l2tun_ctagif?4:0) + (l2tun_stagif?4:0);
		 
		 memcpy(&tag[0], pktBuf+12, lenTag);
		 memcpy(&lenSnap[0], pktBuf+12+lenTag, 8);
		 l2Len = (lenSnap[0]<<8|lenSnap[1]);
		 l2Len += lenTag;
		 lenSnap[0] = (l2Len>>8)&0xff;
		 lenSnap[1] = l2Len&0xff;
		 memcpy(pktBuf + 12, &lenSnap[0], 8);
		 memcpy(pktBuf + 20, &tag[0], lenTag);
		 //dump_packet(innerPktContent,*pktLen,"Packet with snap");
	 }
	  
	 // dump_packet(pktBuf, *pktLen, "\033[1;33;40m[SRv6 L2tunnl packet without sids]\033[0m");
	 if(inner_tail_padding_len > 0 && outer_ipv6_len_exclude_padding > 0)
	 {
		 memset(pktBuf+*pktLen,0,inner_tail_padding_len);
		 *pktLen += inner_tail_padding_len;
		 //dump_packet(innerPktContentPtr,innerPktContentLen,"Original inner packet with padding");
	 }

	 //SRH segment list
	 if(srhHdrLen > 0  && *pktLen >= (innerPktContentLen + (srhHdrLen<<3)))
	 {
	 	int nSeg = (srhLastEntry+1);
	 	for(i=0; i<nSeg ; i++)
			in6_pton(sid[11-nSeg+i],-1,(unsigned char *)&pktBuf[*pktLen-innerPktContentLen-(srhHdrLen<<3)+(i<<4)],-1,NULL);

		//SRv6 Capabilities TLV
		if(srhHdrLen > ((srhLastEntry+1)<<1))
		{
			int startTLV = *pktLen-innerPktContentLen-(srhHdrLen<<3)+(nSeg<<4);
			pktBuf[startTLV] = 1038&0xff; //Type
			pktBuf[startTLV+1] = 6; //Length
			pktBuf[startTLV+2] = 0x80; //Flags
		}
	 }
	 
         *pktLen += 4;
	//dump_packet(pktBuf, *pktLen, "\033[1;33;40m[SRv6 L2tunnl packet]\033[0m");
      }while(0);

	return 0;
}

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
	int pppoetagif,int pppoe_session_id)
{
	int i;
	rtl8651_PktConf_t conf;
        unsigned char *innerPktContentPtr = (uint8 *)&innerPktContent[0];
        int innerPktContentLen;

 	bzero(&conf, sizeof(conf));

	for (i=0;i<*pktLen;i++)
		pktData[i] = i&0xff;

	if(*pktLen > 8)
	{
		pktData[0] = 0x70;	//p
		pktData[1] = 0x61;	//a
		pktData[2] = 0x79;	//y
		pktData[3] = 0x6c;	//l
		pktData[4] = 0x6f;	//o
		pktData[5] = 0x61;	//a
		pktData[6] = 0x64;	//d
	}

	if (nextheader == IPPROTO_UDP)
		conf.pktType	= _PKT_TYPE_V6UDP;
	else if (nextheader == IPPROTO_TCP)
		conf.pktType	= _PKT_TYPE_V6TCP;
	else if (nextheader == IPPROTO_ICMP)
		conf.pktType	= _PKT_TYPE_V6ICMP;
	else
		conf.pktType	= _PKT_TYPE_IPV6;

	conf.l2Flag 			= 0;
	conf.ipv6.version 		= 6;
	conf.ipv6.priority		= tc;
	conf.rhdr.hdrlen	   = srhHdrLen;
	conf.rhdr.nexthdr			= 143;
	conf.rhdr.segments_left 	= srhSegmentLeft;
	conf.ipv6.flowlbl[0]  	= (flowlabel>>16)&0xf;
	conf.ipv6.flowlbl[1]  	= (flowlabel>>8)&0xff;
	conf.ipv6.flowlbl[2] 		= (flowlabel>>0)&0xff;
	conf.payload.content	= pktData;	  
	conf.payload.length 	= *pktLen;
	conf.ipv6.nxthdr		= nextheader;
	conf.ipv6.hoplmt		= hoplimit;
	conf.conf_tcp_flag		= tcpFlag;

	strtomac((void *)&conf.conf_smac[0], smac);
	strtomac((void *)&conf.conf_dmac[0], dmac);
	in6_pton(sip,-1,(unsigned char *)&conf.ipv6.src_ip[0],-1,NULL);
	in6_pton(dip,-1,(unsigned char *)&conf.ipv6.dst_ip[0],-1,NULL);
	conf.conf_sport = sport;
	conf.conf_dport = dport;

	//conf.conf_tcp_flag = TH_RST;
	if (ctagif) {
		conf.l2Flag 		= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 	= cvid;
		conf.cvlan.cprio	= cpri;
	}

	if (stagif) {
		conf.l2Flag 		= conf.l2Flag | L2_SVLAN;
		conf.svlan.svid 	= svid;
		conf.svlan.sprio	= spri;
		if(tpid)
			conf.svlan.tpid		 = tpid;
		else
			conf.svlan.tpid = 0x88a8;
	}

	if(inner_insert_len_snap_after_vlan || inner_insert_len_snap_before_vlan)
		conf.l2Flag |= L2_SNAP;

	if (cputag!=NULL) {
		conf.l2Flag 		= conf.l2Flag | L2_CPUTAG_OTHER;
		conf.conf_ctag_protocol	= 0x4;
		conf.conf_ctag_reason	= cputag->res;
		conf.conf_ctag_priority	= cputag->pri;
		conf.conf_ctag_streamid = cputag->streamid;
//		conf.conf_ctag_ttlde	= cputag->ttl_1;
		conf.conf_ctag_l3r	= cputag->l3r;
		conf.conf_ctag_org	= cputag->org;
		conf.conf_ctag_spa	= cputag->spa;
		conf.conf_ctag_extspa	= cputag->extspa;
		conf.conf_ctag_extdpmsk	= cputag->epmsk;
		conf.conf_ctag_fbi = cputag->fbi;
		conf.conf_ctag_fb_index = cputag->fbindex;
	}else if (cputagTx != NULL) {
		conf.l2Flag 		= conf.l2Flag | L2_CPUTAG_TX2;
		conf.conf_ctag_protocol	= 0x4;
		conf.ctag_tx2.txmsk = cputagTx->txmsk;
		conf.ctag_tx2.prisel = cputagTx->prisel;
		conf.ctag_tx2.pri = cputagTx->pri;
		conf.ctag_tx2.keep = cputagTx->keep;
		conf.ctag_tx2.dislrn = cputagTx->dislrn;
		conf.ctag_tx2.psel = cputagTx->psel;
		conf.ctag_tx2.directTx = cputagTx->directTx;
		conf.ctag_tx2.extspa = cputagTx->extspa;
		conf.ctag_tx2.pppoeact = cputagTx->pppoeact;
		conf.ctag_tx2.pppoeidx = cputagTx->pppoeidx;
		conf.ctag_tx2.streamid = cputagTx->streamid;

	}

	if (pppoetagif==1) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8864;
		conf.pppoe.session 	= pppoe_session_id;
	}else if(pppoetagif==2) {
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8863;
		conf.pppoe.session 	= pppoe_session_id;
	}

	 pppoe_length_exclude_padding = inner_ppp_length_exclude_padding;
	 //printk("pppoe_length_exclude_padding:%d @ %s %d\n",pppoe_length_exclude_padding,__func__,__LINE__);
	 ipv6_length_exclude_padding = inner_ipv6_len_exclude_padding;
	 //printk("ipv6_length_exclude_padding:%d @ %s %d\n",ipv6_length_exclude_padding,__func__,__LINE__);
	 skip_minPktLen_check = inner_skip_minPktLen_check;
	 //printk("skip_minPktLen_check:%d @ %s %d\n",skip_minPktLen_check,__func__,__LINE__);
	 llc_len_including_padding = inner_llc_len_include_ingress_padding;
	 memset(&innerPktContent[0],0,2048);
 	innerPktContentLen = pktGen(&conf, (int8*)innerPktContentPtr);			 /* generate pkt in buff */
	llc_len_including_padding = 0;
	 memset(innerPktContent+innerPktContentLen,0,inner_tail_padding_len);
	 innerPktContentLen += inner_tail_padding_len;
	skip_minPktLen_check = 0;
	pppoe_length_exclude_padding = 0;
	ipv6_length_exclude_padding = 0;

	if(inner_insert_len_snap_before_vlan && (ctagif || stagif))
	{
		unsigned char lenSnap[8];
		unsigned char tag[8];
		unsigned short l2Len;
		int lenTag = (ctagif?4:0) + (stagif?4:0);
		
		memcpy(&tag[0], innerPktContent+12, lenTag);
		memcpy(&lenSnap[0], innerPktContent+12+lenTag, 8);
		l2Len = (lenSnap[0]<<8|lenSnap[1]);
		l2Len += lenTag;
		lenSnap[0] = (l2Len>>8)&0xff;
		lenSnap[1] = l2Len&0xff;
		memcpy(innerPktContent + 12, &lenSnap[0], 8);
		memcpy(innerPktContent + 20, &tag[0], lenTag);
//		dump_packet(innerPktContent,*pktLen,"Packet with snap");
	}

	//dump_packet(innerPktContent,innerPktContentLen,"Inner Packet");

    /* SRv6 L2 tunnel part */
     do{
        //IPv6
        bzero(&conf,sizeof(conf));
        conf.pktType                            = _PKT_TYPE_IPV6;
        conf.conf_tcp_flag                      = 0;
        conf.ipv6.version                       = 6;
         conf.payload.content               = innerPktContentPtr;
         conf.payload.length                = innerPktContentLen;
	 conf.rhdr.hdrlen			= srhHdrLen;
	 conf.rhdr.nexthdr			 = 143;
	 conf.rhdr.segments_left 	 = srhSegmentLeft;
	 in6_pton(sid[10-srhLastEntry],-1,(&conf.rhdr.dip[0]),-1,NULL);
         conf.ipv6.priority                     = l2tun_tc;
         conf.ipv6.flowlbl[0]  = (l2tun_flowlabel>>16)&0xf;
         conf.ipv6.flowlbl[1]  = (l2tun_flowlabel>>8)&0xff;
         conf.ipv6.flowlbl[2] =  (l2tun_flowlabel>>0)&0xff;
         conf.ipv6.pldlen        = *pktLen;
         conf.ipv6.nxthdr          = l2tun_protocol;
         conf.ipv6.hoplmt        = l2tun_hoplimit;

	if(l2tun_pppoetagif == 1)
	{
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8864;
		conf.pppoe.session 	= l2tun_pppoe_session_id;
	}
	else if(l2tun_pppoetagif == 2)
	{
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8863;
		conf.pppoe.session 	= l2tun_pppoe_session_id;
	}
  	if (l2tun_ctagif) {
		conf.l2Flag 		= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 	= l2tun_cvid;
		conf.cvlan.cprio	= l2tun_cpri;
	}

	if (l2tun_stagif) {
		conf.l2Flag 		= conf.l2Flag | L2_SVLAN;
		conf.svlan.svid 	= l2tun_svid;
		conf.svlan.sprio	= l2tun_spri;
		if(l2tun_tpid)
			conf.svlan.tpid		 = l2tun_tpid;
		else
			conf.svlan.tpid = 0x88a8;
	}      

	if(srhHdrLen)
		conf.l2Flag |= (RouHdr | SRv6Hdr);

	if(outer_insert_len_snap)
		conf.l2Flag	|= L2_SNAP;

	in6_pton(l2tun_sip,-1,(unsigned char *)&conf.ipv6.src_ip[0],-1,NULL);
       in6_pton(l2tun_dip,-1,(unsigned char *)&conf.ipv6.dst_ip[0],-1,NULL);

	strtomac((void *)&conf.conf_smac[0], l2tun_smac);
	strtomac((void *)&conf.conf_dmac[0], l2tun_dmac);
         
	pppoe_length_exclude_padding = outer_ppp_length_exclude_padding;
	ipv6_length_exclude_padding = outer_ipv6_len_exclude_padding;
	llc_len_excluding_padding = outer_llc_len_exclude_ingress_padding;
      *pktLen = pktGen(&conf, (int8*)pktBuf);       /* generate pkt in buff */
	  llc_len_excluding_padding = 0;
	pppoe_length_exclude_padding = 0;
	ipv6_length_exclude_padding = 0;

#if 0
	 if(outer_insert_len_snap)
	 {
		 int offset = 12;
		 unsigned short *l2Len;
		 offset += ((l2tun_ctagif?4:0) + (l2tun_stagif?4:0));
		 memmove(pktBuf + offset + 2, pktBuf + offset, *pktLen -offset);
		 l2Len = (unsigned short *)(pktBuf + offset);
		 *l2Len = (*pktLen - offset);
		 *pktLen += 2;
		 dump_packet(pktBuf,*pktLen,"Outer packet with snap");
	 }
#endif

	 //SRH segment list
	 if(srhHdrLen > 0  && *pktLen >= (innerPktContentLen + (srhHdrLen<<3)))
	 {
	 	int nSeg = (srhLastEntry+1);
	 	for(i=0; i<nSeg ; i++)
			in6_pton(sid[11-nSeg+i],-1,(unsigned char *)&pktBuf[*pktLen-innerPktContentLen-(srhHdrLen<<3)+(i<<4)],-1,NULL);

		//SRv6 Capabilities TLV
		if(srhHdrLen > ((srhLastEntry+1)<<1))
		{
			int startTLV = *pktLen-innerPktContentLen-(srhHdrLen<<3)+(nSeg<<4);
			pktBuf[startTLV] = 1038&0xff; //Type
			pktBuf[startTLV+1] = 6; //Length
			pktBuf[startTLV+2] = 0x80; //Flags
		}
	 }

         *pktLen += 4;
	//dump_packet(pktBuf, *pktLen, "\033[1;33;40m[SRv6 L2tunnl packet]\033[0m");
      }while(0);

	return 0;
}

int SRv6innerV4TcpUdpPktGen(uint8 *pktBuf,uint8 *pktData,int *pktLen,
	unsigned char *smac, unsigned char *dmac,
	unsigned char *outer_sip,unsigned char *outer_dip,
	int outer_protocol,int outer_hoplimit, int outer_tc, int outer_flow_label,
	int srhHdrLen, int srhSegmentLeft, int srhLastEntry,
	unsigned char *sid[],
	unsigned char *sip, unsigned short sport,
	unsigned char *dip, unsigned short dport,
	int protocol,int ttl, int iptos,int ipmf,int ipfost,int tcpFlag,
	int ctagif,int cvid,int cpri,
	int stagif,int tpid, int svid,int spri,
	cputag_t *cputag, cputagTx_t *cputagTx,
	int pppoetagif,int pppoe_session_id)
{
	int i;
	rtl8651_PktConf_t conf;
        int innerPktContentLen;

 	bzero(&conf, sizeof(conf));

	for (i=0;i<*pktLen;i++)
		pktData[i] = i&0xff;

	if(*pktLen > 8)
	{
		pktData[0] = 0x70;	//p
		pktData[1] = 0x61;	//a
		pktData[2] = 0x79;	//y
		pktData[3] = 0x6c;	//l
		pktData[4] = 0x6f;	//o
		pktData[5] = 0x61;	//a
		pktData[6] = 0x64;	//d
	}

//printk("- protocol:%d - [%s %d]\n",protocol,__func__,__LINE__);
	if (protocol == IPPROTO_UDP)
		conf.pktType	= _PKT_TYPE_UDP;
	else if (protocol == IPPROTO_TCP)
		conf.pktType	= _PKT_TYPE_TCP;
	else if (protocol == IPPROTO_ICMP)
		conf.pktType	= _PKT_TYPE_ICMP;
	else
		conf.pktType	= _PKT_TYPE_IP;

	conf.ip.id					= 0x123;
	conf.ip.ttl 				= ttl;
	conf.ip.tos 				= iptos;
	conf.payload.length 		= *pktLen;
	conf.ip.version				= 0x4;
	conf.ip.mf					= ipmf;
	conf.ip.offset				= ipfost;
	conf.ip.headerLen			= 20;
	conf.payload.content		= pktData;
	conf.pppoe.type 			= 0x0800;
	conf.pppoe.session			= 0;
	conf.cvlan.cprio			= 1;
	conf.conf_tcp_flag			= tcpFlag;
	conf.l2Flag 				= 0;
	if(inner_insert_len_snap_after_vlan || inner_insert_len_snap_before_vlan)
		conf.l2Flag |= L2_SNAP;

	strtomac((void *)&conf.conf_smac[0], smac);
	strtomac((void *)&conf.conf_dmac[0], dmac);
	conf.conf_sip	= ntohl(inet_addr(sip));
	conf.conf_dip	= ntohl(inet_addr(dip));
	conf.conf_sport = sport;
	conf.conf_dport = dport;

	skip_minPktLen_check = inner_skip_minPktLen_check;
	memset(&innerPktContent[0],0,2048);
	innerPktContentLen = pktGen(&conf, (int8*)&innerPktContent[0]);
	skip_minPktLen_check = 0;
	//dump_packet(innerPktContent,innerPktContentLen,"Original SRv6innerV4TcpUdpPktGen Inner Packet");
	if(inner_tail_padding_len > 0)
	{
		memset(innerPktContent+innerPktContentLen,0,inner_tail_padding_len);
		innerPktContentLen += inner_tail_padding_len;
	}

	if(inner_insert_len_snap_before_vlan && (ctagif || stagif))
	{
		unsigned char lenSnap[8];
		unsigned char tag[8];
		unsigned short l2Len;
		int lenTag = (ctagif?4:0) + (stagif?4:0);
		
		memcpy(&tag[0], &innerPktContent[12], lenTag);
		memcpy(&lenSnap[0], &innerPktContent[12+lenTag], 8);
		l2Len = (lenSnap[0]<<8|lenSnap[1]);
		l2Len += lenTag;
		lenSnap[0] = (l2Len>>8)&0xff;
		lenSnap[1] = l2Len&0xff;
		memcpy(&innerPktContent[12], &lenSnap[0], 8);
		memcpy(&innerPktContent[20], &tag[0], lenTag);
		//dump_packet(pktBuf,*pktLen,"Packet with snap");
	}


	innerPktContentLen -= 14;
	
	memset(&tmpData[0],0,2048);
	memcpy(&tmpData[0],&innerPktContent[14],innerPktContentLen);
	/* generate pkt in buff */
	//printk("inner_tail_padding_len:%d @ %s %d\n",inner_tail_padding_len,__func__,__LINE__);

	//dump_packet(innerPktContent,innerPktContentLen,"SRv6innerV4TcpUdpPktGen Inner Packet");

    /* SRv6 L2 tunnel part */
     do{
        //IPv6
        bzero(&conf,sizeof(conf));
        conf.pktType                            = _PKT_TYPE_IPV6;
        conf.conf_tcp_flag                      = 0;
        conf.ipv6.version                       = 6;
         conf.payload.content               = tmpData;
         conf.payload.length                = innerPktContentLen;
	 conf.rhdr.hdrlen		= srhHdrLen;
	 //conf.rhdr.nexthdr			 = 143;
	 conf.rhdr.segments_left 	 = srhSegmentLeft;
	 if(srhLastEntry >= 10)
		in6_pton(sid[14-srhLastEntry],-1,(&conf.rhdr.dip[0]),-1,NULL);
	 else
	 	in6_pton(sid[10-srhLastEntry],-1,(&conf.rhdr.dip[0]),-1,NULL);
         conf.ipv6.priority                     = outer_tc;
         conf.ipv6.flowlbl[0]  = (outer_flow_label>>16)&0xf;
         conf.ipv6.flowlbl[1]  = (outer_flow_label>>8)&0xff;
         conf.ipv6.flowlbl[2] =  (outer_flow_label>>0)&0xff;
         conf.ipv6.pldlen        = *pktLen;
         conf.ipv6.nxthdr          = 4;
         conf.ipv6.hoplmt        = outer_hoplimit;
	if(pppoetagif == 1)
	{
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8864;
		conf.pppoe.session 	= pppoe_session_id;
	}
	else if(pppoetagif == 2)
	{
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8863;
		conf.pppoe.session 	= pppoe_session_id;
	}
  	if (ctagif) {
		conf.l2Flag 		= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 	= cvid;
		conf.cvlan.cprio	= cpri;
	}

	if (stagif) {
		conf.l2Flag 		= conf.l2Flag | L2_SVLAN;
		conf.svlan.svid 	= svid;
		conf.svlan.sprio	= spri;
		if(tpid)
			conf.svlan.tpid		 = tpid;
		else
			conf.svlan.tpid = 0x88a8;
	}      
	if(srhHdrLen)
		conf.l2Flag |= (RouHdr | SRv6Hdr);

	in6_pton(outer_sip,-1,(unsigned char *)&conf.ipv6.src_ip[0],-1,NULL);
       in6_pton(outer_dip,-1,(unsigned char *)&conf.ipv6.dst_ip[0],-1,NULL);

	 strtomac((void *)&conf.conf_smac[0], smac);
	 strtomac((void *)&conf.conf_dmac[0], dmac);
         
         *pktLen = pktGen(&conf, (int8*)pktBuf);       /* generate pkt in buff */
 
	 //SRH segment list
	 if(srhHdrLen > 0  && *pktLen >= (innerPktContentLen + (srhHdrLen<<3)))
	 {
	 	int nSeg = (srhLastEntry+1);
	 	for(i=0; i<nSeg ; i++)
	 	{
			if(srhLastEntry >= 10)
				in6_pton(sid[15-nSeg+i],-1,(unsigned char *)&pktBuf[*pktLen-innerPktContentLen-(srhHdrLen<<3)+(i<<4)],-1,NULL);
			else
				in6_pton(sid[11-nSeg+i],-1,(unsigned char *)&pktBuf[*pktLen-innerPktContentLen-(srhHdrLen<<3)+(i<<4)],-1,NULL);
	 	}

		//SRv6 Capabilities TLV
		if(srhHdrLen > ((srhLastEntry+1)<<1))
		{
			int startTLV = *pktLen-innerPktContentLen-(srhHdrLen<<3)+(nSeg<<4);
			pktBuf[startTLV] = 1038&0xff; //Type
			pktBuf[startTLV+1] = 6; //Length
			pktBuf[startTLV+2] = 0x80; //Flags
		}
	 }

	 
         *pktLen += 4;
	//dump_packet(pktBuf, *pktLen, "\033[1;33;40m[SRv6innerV4TcpUdpPktGen]\033[0m");
      }while(0);

	return 0;
}

int SRv6innerV6TcpUdpPktGen(uint8 *pktBuf,uint8 *pktData,int *pktLen,
	unsigned char *smac, unsigned char *dmac,
	unsigned char *outer_sip,unsigned char *outer_dip,
	int outer_protocol,int outer_hoplimit, int outer_tc, int outer_flow_label,
	int srhHdrLen, int srhSegmentLeft, int srhLastEntry,
	unsigned char *sid[],
	unsigned char *sip, unsigned short sport,
	unsigned char *dip, unsigned short dport,
	int nextheader, int flowlabel, int hoplimit, int tc,int ipmf,int ipfost,int tcpFlag,
	int ctagif,int cvid,int cpri,
	int stagif,int tpid, int svid,int spri,
	cputag_t *cputag, cputagTx_t *cputagTx,
	int pppoetagif,int pppoe_session_id)
{
	int i;
	rtl8651_PktConf_t conf;
        unsigned char *innerPktContentPtr = (uint8 *)&innerPktContent[0];
        int innerPktContentLen;

 	bzero(&conf, sizeof(conf));

	for (i=0;i<*pktLen;i++)
		pktData[i] = i&0xff;

	if(*pktLen > 8)
	{
		pktData[0] = 0x70;	//p
		pktData[1] = 0x61;	//a
		pktData[2] = 0x79;	//y
		pktData[3] = 0x6c;	//l
		pktData[4] = 0x6f;	//o
		pktData[5] = 0x61;	//a
		pktData[6] = 0x64;	//d
	}

	if (nextheader == IPPROTO_UDP)
		conf.pktType	= _PKT_TYPE_V6UDP;
	else if (nextheader == IPPROTO_TCP)
		conf.pktType	= _PKT_TYPE_V6TCP;
	else if (nextheader == IPPROTO_ICMP)
		conf.pktType	= _PKT_TYPE_V6ICMP;
	else
		conf.pktType	= _PKT_TYPE_IPV6;

	conf.l2Flag 			= 0;
	conf.ipv6.version 		= 6;
	conf.ipv6.priority		= tc;
	conf.ipv6.flowlbl[0]  	= (flowlabel>>16)&0xf;
	conf.ipv6.flowlbl[1]  	= (flowlabel>>8)&0xff;
	conf.ipv6.flowlbl[2] 		= (flowlabel>>0)&0xff;
	conf.payload.content	= pktData;	  
	conf.payload.length 	= *pktLen;
	conf.ipv6.nxthdr		= nextheader;
	conf.ipv6.hoplmt		= hoplimit;
	conf.ip.mf				= ipmf;
	conf.ip.offset			= ipfost;
	conf.conf_tcp_flag		= tcpFlag;

	strtomac((void *)&conf.conf_smac[0], smac);
	strtomac((void *)&conf.conf_dmac[0], dmac);
	in6_pton(sip,-1,(unsigned char *)&conf.ipv6.src_ip[0],-1,NULL);
	in6_pton(dip,-1,(unsigned char *)&conf.ipv6.dst_ip[0],-1,NULL);
	conf.conf_sport = sport;
	conf.conf_dport = dport;

 	innerPktContentLen = pktGen(&conf, (int8*)innerPktContentPtr);			 /* generate pkt in buff */
	innerPktContentLen -= 14;
	memcpy((uint8 *)&innerPktContent[0],(uint8 *)&innerPktContent[14],innerPktContentLen);

	//dump_packet(innerPktContent,innerPktContentLen,"Inner Packet");

    /* SRv6 L2 tunnel part */
     do{
        //IPv6
        bzero(&conf,sizeof(conf));
        conf.pktType                            = _PKT_TYPE_IPV6;
        conf.conf_tcp_flag                      = 0;
        conf.ipv6.version                       = 6;
         conf.payload.content               = innerPktContentPtr;
         conf.payload.length                = innerPktContentLen;
	 conf.rhdr.hdrlen			= srhHdrLen;
	 //conf.rhdr.nexthdr			 = 41;
	 conf.rhdr.segments_left 	 = srhSegmentLeft;
	 in6_pton(sid[10-srhLastEntry],-1,(&conf.rhdr.dip[0]),-1,NULL);
         conf.ipv6.priority                     = outer_tc;
         conf.ipv6.flowlbl[0]  = (outer_flow_label>>16)&0xf;
         conf.ipv6.flowlbl[1]  = (outer_flow_label>>8)&0xff;
         conf.ipv6.flowlbl[2] =  (outer_flow_label>>0)&0xff;
         conf.ipv6.pldlen        = *pktLen;
         conf.ipv6.nxthdr          = 41;
         conf.ipv6.hoplmt        = outer_hoplimit;

	if(pppoetagif == 1)
	{
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8864;
		conf.pppoe.session 	= pppoe_session_id;
	}
	else if(pppoetagif == 2)
	{
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8863;
		conf.pppoe.session 	= pppoe_session_id;
	}
  	if (ctagif) {
		conf.l2Flag 		= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 	= cvid;
		conf.cvlan.cprio	= cpri;
	}

	if (stagif) {
		conf.l2Flag 		= conf.l2Flag | L2_SVLAN;
		conf.svlan.svid 	= svid;
		conf.svlan.sprio	= spri;
		if(tpid)
			conf.svlan.tpid		 = tpid;
		else
			conf.svlan.tpid = 0x88a8;
	}      

	if(srhHdrLen)
		conf.l2Flag |= (RouHdr | SRv6Hdr);

	in6_pton(outer_sip,-1,(unsigned char *)&conf.ipv6.src_ip[0],-1,NULL);
       in6_pton(outer_dip,-1,(unsigned char *)&conf.ipv6.dst_ip[0],-1,NULL);

	strtomac((void *)&conf.conf_smac[0], smac);
	strtomac((void *)&conf.conf_dmac[0], dmac);
         
      *pktLen = pktGen(&conf, (int8*)pktBuf);       /* generate pkt in buff */

	 //SRH segment list
	 if(srhHdrLen > 0  &&  *pktLen >= (innerPktContentLen + (srhHdrLen<<3)))
	 {
	 	int nSeg = (srhLastEntry+1);
	 	for(i=0; i<nSeg ; i++)
			in6_pton(sid[11-nSeg+i],-1,(unsigned char *)&pktBuf[*pktLen-innerPktContentLen-(srhHdrLen<<3)+(i<<4)],-1,NULL);

		//SRv6 Capabilities TLV
		if(srhHdrLen > ((srhLastEntry+1)<<1))
		{
			int startTLV = *pktLen-innerPktContentLen-(srhHdrLen<<3)+(nSeg<<4);
			pktBuf[startTLV] = 1038&0xff; //Type
			pktBuf[startTLV+1] = 6; //Length
			pktBuf[startTLV+2] = 0x80; //Flags
		}
	 }

         *pktLen += 4;
	//dump_packet(pktBuf, *pktLen, "\033[1;33;40m[SRv6 L2tunnl packet]\033[0m");
      }while(0);

	return 0;
}

int SRv6TcpUdpPktGen(uint8 *pktBuf,uint8 *pktData,int *pktLen,
	unsigned char *smac, unsigned char *dmac,
	int srhHdrLen, int srhSegmentLeft,int srhLastEntry,
	unsigned char *sid[],unsigned char *srh_dip,
	unsigned char *sip, unsigned short sport,
	unsigned char *dip, unsigned short dport,
	int nextheader, int flowlabel, int hoplimit, int tc, int ipmf,int ipfost,int tcpFlag,
	int ctagif,int cvid,int cpri,
	int stagif,int tpid, int svid,int spri,
	int pppoetagif,int pppoe_session_id)
{
	int i;
	rtl8651_PktConf_t conf;
	int srv6Offset = 14+40;

 	bzero(&conf, sizeof(conf));

	for (i=0;i<*pktLen;i++)
		pktData[i] = i&0xff;

	if(*pktLen > 8)
	{
		pktData[0] = 0x70;	//p
		pktData[1] = 0x61;	//a
		pktData[2] = 0x79;	//y
		pktData[3] = 0x6c;	//l
		pktData[4] = 0x6f;	//o
		pktData[5] = 0x61;	//a
		pktData[6] = 0x64;	//d
	}

	if (nextheader == IPPROTO_UDP)
		conf.pktType	= _PKT_TYPE_V6UDP;
	else if (nextheader == IPPROTO_TCP)
		conf.pktType	= _PKT_TYPE_V6TCP;
	else if (nextheader == IPPROTO_ICMP)
		conf.pktType	= _PKT_TYPE_V6ICMP;
	else
		conf.pktType	= _PKT_TYPE_IPV6;

	conf.l2Flag 			= 0;
	conf.ipv6.version 		= 6;
	conf.ipv6.priority		= tc;
	conf.ipv6.flowlbl[0]  	= (flowlabel>>16)&0xf;
	conf.ipv6.flowlbl[1]  	= (flowlabel>>8)&0xff;
	conf.ipv6.flowlbl[2] 		= (flowlabel>>0)&0xff;
	conf.payload.content	= pktData;	  
	conf.payload.length 	= *pktLen;
	conf.ipv6.nxthdr		= nextheader;
	conf.ipv6.hoplmt		= hoplimit;
	conf.conf_tcp_flag			= tcpFlag;
	 conf.rhdr.hdrlen			= srhHdrLen;
	 //conf.rhdr.nexthdr			 = 41;
	 conf.rhdr.segments_left 	 = srhSegmentLeft;
	 in6_pton(srh_dip,-1,(&conf.rhdr.dip[0]),-1,NULL);

	if(pppoetagif == 1)
	{
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8864;
		conf.pppoe.session 	= pppoe_session_id;
		srv6Offset += 8;
	}
	else if(pppoetagif == 2)
	{
		conf.l2Flag 		= conf.l2Flag | L2_PPPoE ;
		conf.pppoe.type 	= 0x8863;
		conf.pppoe.session 	= pppoe_session_id;
		srv6Offset += 8;
	}
  	if (ctagif) {
		conf.l2Flag 		= conf.l2Flag | L2_CVLAN;
		conf.cvlan.cvid 	= cvid;
		conf.cvlan.cprio	= cpri;
		srv6Offset += 4;
	}

	if (stagif) {
		conf.l2Flag 		= conf.l2Flag | L2_SVLAN;
		conf.svlan.svid 	= svid;
		conf.svlan.sprio	= spri;
		if(tpid)
			conf.svlan.tpid		 = tpid;
		else
			conf.svlan.tpid = 0x88a8;
		srv6Offset += 4;
	}      

	if(srhHdrLen)
		conf.l2Flag |= (RouHdr | SRv6Hdr);

	in6_pton(sip,-1,(unsigned char *)&conf.ipv6.src_ip[0],-1,NULL);
	in6_pton(dip,-1,(unsigned char *)&conf.ipv6.dst_ip[0],-1,NULL);
	conf.conf_sport = sport;
	conf.conf_dport = dport;

	strtomac((void *)&conf.conf_smac[0], smac);
	strtomac((void *)&conf.conf_dmac[0], dmac);
        
      *pktLen = pktGen(&conf, (int8*)pktBuf);       /* generate pkt in buff */
	//dump_packet(pktBuf, *pktLen, "\033[1;33;40m[SRv6TcpUdpPktGen]\033[0m");

	 //SRH segment list
	 if(srhHdrLen > 0  &&  *pktLen >= (srhHdrLen<<3))
	 {
	 	int nSeg = (srhLastEntry+1);
	 	for(i=0; i<nSeg ; i++)
	 	{
			in6_pton(sid[11-nSeg+i],-1,(unsigned char *)&pktBuf[srv6Offset+8+(i<<4)],-1,NULL);
	 	}

		//SRv6 Capabilities TLV
		if(srhHdrLen > ((srhLastEntry+1)<<1))
		{
			int startTLV = srv6Offset + 8 + (nSeg<<4);
			pktBuf[startTLV] = 1038&0xff; //Type
			pktBuf[startTLV+1] = 6; //Length
			pktBuf[startTLV+2] = 0x80; //Flags
		}
	 }

         *pktLen += 4;
	//dump_packet(pktBuf, *pktLen, "\033[1;33;40m[SRv6 L2tunnl packet]\033[0m");

	return 0;
}

