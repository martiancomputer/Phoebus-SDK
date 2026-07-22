

//#include <net/netfilter/nf_conntrack_helper.h>


#include "rtk_fc_struct.h"
#include "rtk_fc_debug.h"
#include "rtk_fc_internal.h"
#include "rtk_fc_assistant.h"
#include "rtk_fc_multicast.h"
#include "rtk_fc_external.h"
#include "rtk_fc_acl.h"


#if defined(CONFIG_RTK_L34_G3_PLATFORM)
#if defined(CONFIG_FC_CA8277AB_SERIES)
#include "rtk_rg_g3_internal.h"
#endif
#include <flow.h>
#include <vlan.h>
#include <mcast.h>
#include <ca_ni_tx.h>
#endif
#include <rt_acl_ext.h>		//for aal_l3_cls_add 

#if defined(CONFIG_FC_CA8277B_SERIES)
extern rtk_scfg_t rtkScfg;
#endif
extern rtk_fc_pathDecision_global_var_t pathDecision_var;

#if defined(CONFIG_RTK_L34_G3_PLATFORM)

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
//CONFIG_FC_RTL8277C_SERIES || CONFIG_FC_RTL9607F_SERIES

#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
static rtk_fc_ret_t RTK_RG_ASIC_AMSDU_DBG_MODE_VOQ_SEL(uint8 ldpid, uint8 cos, uint8 pol_id, rtk_rg_asic_flow_config_t *pFlowConfig)
{
	int i;
	int8 fwd_id = SIGNED_INVALID;
	uint8 voq_id, voq_ldpid, voq_cos;

	for(i = 0 ; i < AMSDU_PE_DBG_MODE_INFO_SIZE ; i++)
	{
		if((fc_db.controlFuc.amsdu_pe_dbg_info_tbl[i].egress_port == ldpid) &&
				(fc_db.controlFuc.amsdu_pe_dbg_info_tbl[i].egress_cos == cos) &&
				(fc_db.controlFuc.amsdu_pe_dbg_info_tbl[i].egress_pol_id == pol_id))
		{
			fwd_id = i;
			break;
		}
	}
	if(fwd_id == SIGNED_INVALID)
	{
		WARNING("Strange, invalid FWD_ID, do not change flow (ldpid 0x%2x, cos %u, pol_id %u)", ldpid, cos, pol_id);
		return RTK_FC_RET_ERR;
	}
	else if(_rtk_fc_amsdu_pe_dbg_mode_amsdu_pe_dbg_mode_voq_id_get(ldpid, cos, pol_id, &voq_id))
	{
		WARNING("Strange, failed to get voq_id, do not change flow (ldpid 0x%2x, cos %u, pol_id %u)", ldpid, cos, pol_id);
		return RTK_FC_RET_ERR;
	}
	voq_ldpid = fc_db.wifi_amsdu_pe_offload_voq_tbl[voq_id].ldpid;
	voq_cos = fc_db.wifi_amsdu_pe_offload_voq_tbl[voq_id].cos;

	if((pFlowConfig->in_path == FB_PATH_12) && (fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH12_PROTOCOL] == ENABLED) && (pFlowConfig->path12.key.ethertype == 0x0806))
	{
		TRACE("Path1 enable ethertype, set forceOriPath for ARP packet to prevent long latency");
		return RTK_FC_RET_OK;
	}

	DEBUG("ldpid 0x%2x, cos %u, pol_id %u match AMSDU debug mode FWD_ID[%02u], send to AMSDU VOQ[%d] (ldpid 0x%2x, cos %u)", ldpid, cos, pol_id, fwd_id, voq_id, voq_ldpid, voq_cos);
	if(pFlowConfig->in_path == FB_PATH_5) {
		pFlowConfig->path5.action.is_amsdu_pe_offload_wifiTx = TRUE;
		pFlowConfig->path5.action.wifi_ssid = fwd_id;
		pFlowConfig->path5.action.cos_update_en = 1;
		pFlowConfig->path5.action.cos = voq_cos;
		pFlowConfig->path5.action.ldpid = voq_ldpid;
		pFlowConfig->path5.action.gemId_mapping_mode = FALSE; //disable GEM mapping mode

#if defined(CONFIG_FC_RTL9607F_SERIES)
		pFlowConfig->path5.action.l2format_act_vld = TRUE;
		pFlowConfig->path5.action.l2format_act = RTK_ASIC_L2_FORMAT_ACT_SNAP;
#endif
	}else if(pFlowConfig->in_path == FB_PATH_34) {
		pFlowConfig->path34.action.is_amsdu_pe_offload_wifiTx = TRUE;
		pFlowConfig->path34.action.wifi_ssid = fwd_id;
		pFlowConfig->path34.action.cos_update_en = 1;
		pFlowConfig->path34.action.cos = voq_cos;
		pFlowConfig->path34.action.ldpid = voq_ldpid;
		pFlowConfig->path34.action.gemId_mapping_mode = FALSE; //disable GEM mapping mode

#if defined(CONFIG_FC_RTL9607F_SERIES)
		pFlowConfig->path34.action.l2format_act_vld = TRUE;
		pFlowConfig->path34.action.l2format_act = RTK_ASIC_L2_FORMAT_ACT_SNAP;
#endif
	}else if(pFlowConfig->in_path == FB_PATH_12) {
		pFlowConfig->path12.action.is_amsdu_pe_offload_wifiTx = TRUE;
		pFlowConfig->path12.action.wifi_ssid = fwd_id;
		pFlowConfig->path12.action.cos_update_en = 1;
		pFlowConfig->path12.action.cos = voq_cos;
		pFlowConfig->path12.action.ldpid = voq_ldpid;
		pFlowConfig->path12.action.gemId_mapping_mode = FALSE; //disable GEM mapping mode

#if defined(CONFIG_FC_RTL9607F_SERIES)
		pFlowConfig->path12.action.l2format_act_vld = TRUE;
		pFlowConfig->path12.action.l2format_act = RTK_ASIC_L2_FORMAT_ACT_SNAP;
#endif
	}

	return RTK_FC_RET_OK;
}

static rtk_fc_ret_t RTK_RG_ASIC_WIFI_AMSDU_VOQ_SEL(rtk_rg_asic_path1_entry_t *flow, rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, rtk_rg_asic_flow_config_t *pFlowConfig, bool is_wifi_tx, void *wfo_flow_mib_trx_info)
{
	uint8 _ldpid, _cos, toAmsduPE;
	uint16 _wifi_ssid;
	uint8 wifi_pri = 0;
	int16 lutDaIdx = pG3IgrExtraInfo->lutEgrDaIdx;
	rtk_fc_wlan_devidx_t egrWlanDevIdx;
	uint16 *pWfo_flow_mib_idx = NULL;
#if defined(CONFIG_RTK_FC_WFO_PER_HW_FLOW_MIB)
	int16 lutSaIdx = pG3IgrExtraInfo->lutIgrSaIdx;
	uint16 wfo_flow_mib_idx = RTK_ASIC_WFO_PER_FLOW_MIB_DISABLE;

	pWfo_flow_mib_idx = &wfo_flow_mib_idx;
#endif

	if(is_wifi_tx)
	{
		/* wifi TX*/
#if defined(CONFIG_RTK_FC_WIFI_MULTIBAND_ACC_BY_ONE_LUT)
		egrWlanDevIdx = pG3IgrExtraInfo->egrWlanDevIdx;
#else
		egrWlanDevIdx = fc_db.lutTbl[pG3IgrExtraInfo->lutEgrDaIdx]->wlan_dev_idx;
#endif

		wifi_pri = flow->out_user_priority%8; // flow->out_user_priority is wifi priority for wifi TX packet

		_rtk_fc_wifi_get_amsduMacLutIdx(pG3IgrExtraInfo->lutEgrDaIdx,egrWlanDevIdx,&lutDaIdx);
	
		toAmsduPE = _rtk_fc_wifi_toAmsdu_check(lutDaIdx,wifi_pri);

		if((pFlowConfig->in_path == FB_PATH_12) && (fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH12_PROTOCOL] == ENABLED) && (pFlowConfig->path12.key.ethertype == 0x0806))
		{
			TRACE("Path1 enable ethertype, set forceOriPath for ARP packet to prevent long latency");
			toAmsduPE = FALSE;
		}


		if(toAmsduPE) 
		{
			if(_rtk_fc_wifi_amsdu_voq_decision(fc_db.lutTbl[lutDaIdx]->wifiMacId,wifi_pri,&_ldpid, &_cos,&_wifi_ssid, TRUE/*wifi TX*/, pWfo_flow_mib_idx)==RTK_FC_RET_OK)
			{
#if defined(CONFIG_RTK_FC_WFO_PER_HW_FLOW_MIB)
				rtk_fc_wfo_flow_mib_tx_info_t *pFlow_mib_tx_info = wfo_flow_mib_trx_info;

				if(pFlow_mib_tx_info == NULL)
					WARNING("NULL wfo_flow_mib_trx_info");

				flow->out_wfo_flow_mib_idx = wfo_flow_mib_idx;

				pFlow_mib_tx_info->mac_id = fc_db.lutTbl[lutDaIdx]->wifiMacId;
				pFlow_mib_tx_info->wifi_pri = wifi_pri;
#endif

				DEBUG("dest mac id %d set ldpid %d cos %d", fc_db.lutTbl[lutDaIdx]->wifiMacId, _ldpid, _cos);
				if(pFlowConfig->in_path == FB_PATH_5) {
					pFlowConfig->path5.action.is_amsdu_pe_offload_wifiTx = TRUE;
					pFlowConfig->path5.action.wifi_ssid = _wifi_ssid;
					pFlowConfig->path5.action.cos_update_en = 1;
					pFlowConfig->path5.action.cos = _cos;
					pFlowConfig->path5.action.ldpid = _ldpid;
				
#if defined(CONFIG_FC_RTL9607F_SERIES)
					pFlowConfig->path5.action.l2format_act_vld = TRUE;
					pFlowConfig->path5.action.l2format_act = RTK_ASIC_L2_FORMAT_ACT_SNAP;
#endif			
				}else if(pFlowConfig->in_path == FB_PATH_34) {
					pFlowConfig->path34.action.is_amsdu_pe_offload_wifiTx = TRUE;
					pFlowConfig->path34.action.wifi_ssid = _wifi_ssid;
					pFlowConfig->path34.action.cos_update_en = 1;
					pFlowConfig->path34.action.cos = _cos;
					pFlowConfig->path34.action.ldpid = _ldpid;
				
#if defined(CONFIG_FC_RTL9607F_SERIES)
					pFlowConfig->path34.action.l2format_act_vld = TRUE;
					pFlowConfig->path34.action.l2format_act = RTK_ASIC_L2_FORMAT_ACT_SNAP;
#endif		
				}else if(pFlowConfig->in_path == FB_PATH_12) {
					pFlowConfig->path12.action.is_amsdu_pe_offload_wifiTx = TRUE;
					pFlowConfig->path12.action.wifi_ssid = _wifi_ssid;
					pFlowConfig->path12.action.cos_update_en = 1;
					pFlowConfig->path12.action.cos = _cos;
					pFlowConfig->path12.action.ldpid = _ldpid;
				
#if defined(CONFIG_FC_RTL9607F_SERIES)
					pFlowConfig->path12.action.l2format_act_vld = TRUE;
					pFlowConfig->path12.action.l2format_act = RTK_ASIC_L2_FORMAT_ACT_SNAP;
#endif		
				}
			}
		}
	}
#if defined(CONFIG_RTK_FC_WFO_PER_HW_FLOW_MIB)
	else
	{
		/* wifi RX*/
		if(_rtk_fc_wifi_amsdu_voq_decision(fc_db.lutTbl[lutSaIdx]->wifiMacId, 0 /*wifi_pri*/,&_ldpid, &_cos,&_wifi_ssid, FALSE/*wifi TX*/, pWfo_flow_mib_idx)==RTK_FC_RET_OK)
		{
			rtk_fc_wfo_flow_mib_rx_info_t *pFlow_mib_rx_info = wfo_flow_mib_trx_info;

			if(pFlow_mib_rx_info == NULL)
				WARNING("NULL wfo_flow_mib_trx_info");

			flow->out_wfo_flow_mib_idx = wfo_flow_mib_idx;

			DEBUG("src mac id %d set ldpid %d cos %d", fc_db.lutTbl[lutSaIdx]->wifiMacId, _ldpid, _cos);
			if(pFlowConfig->in_path == FB_PATH_5) {
				if(pFlowConfig->path5.action.gemId_mapping_mode == RTK_GEM_MAPPING_MODE)
				{
					pFlow_mib_rx_info->ldpid =  pFlowConfig->path5.action.tcont_id+0x20;
					pFlow_mib_rx_info->pol_id =  pFlowConfig->path5.action.gem_id;
					pFlowConfig->path5.action.gemId_mapping_mode = FALSE; //disable GEM mapping mode
				}
				else
					pFlow_mib_rx_info->ldpid = pFlowConfig->path5.action.ldpid;
				pFlow_mib_rx_info->cos = pFlowConfig->path5.action.cos;

				pFlowConfig->path5.action.is_amsdu_pe_offload_wifiRx = TRUE;
				pFlowConfig->path5.action.wifi_ssid = _wifi_ssid;
				pFlowConfig->path5.action.cos_update_en = 1;
				pFlowConfig->path5.action.cos = _cos;
				pFlowConfig->path5.action.ldpid = _ldpid;
			}else if(pFlowConfig->in_path == FB_PATH_34) {
				if(pFlowConfig->path34.action.gemId_mapping_mode == RTK_GEM_MAPPING_MODE)
				{
					pFlow_mib_rx_info->ldpid =  pFlowConfig->path34.action.tcont_id+0x20;
					pFlow_mib_rx_info->pol_id =  pFlowConfig->path34.action.gem_id;
					pFlowConfig->path34.action.gemId_mapping_mode = FALSE; //disable GEM mapping mode
				}
				else
					pFlow_mib_rx_info->ldpid = pFlowConfig->path34.action.ldpid;
				pFlow_mib_rx_info->cos = pFlowConfig->path34.action.cos;

				pFlowConfig->path34.action.is_amsdu_pe_offload_wifiRx = TRUE;
				pFlowConfig->path34.action.wifi_ssid = _wifi_ssid;
				pFlowConfig->path34.action.cos_update_en = 1;
				pFlowConfig->path34.action.cos = _cos;
				pFlowConfig->path34.action.ldpid = _ldpid;
			}else if(pFlowConfig->in_path == FB_PATH_12) {
				if(pFlowConfig->path12.action.gemId_mapping_mode == RTK_GEM_MAPPING_MODE)
				{
					pFlow_mib_rx_info->ldpid =  pFlowConfig->path12.action.tcont_id+0x20;
					pFlow_mib_rx_info->pol_id =  pFlowConfig->path12.action.gem_id;
					pFlowConfig->path12.action.gemId_mapping_mode = FALSE; //disable GEM mapping mode
				}
				else
					pFlow_mib_rx_info->ldpid = pFlowConfig->path12.action.ldpid;
				pFlow_mib_rx_info->cos = pFlowConfig->path12.action.cos;

				pFlowConfig->path12.action.is_amsdu_pe_offload_wifiRx = TRUE;
				pFlowConfig->path12.action.wifi_ssid = _wifi_ssid;
				pFlowConfig->path12.action.cos_update_en = 1;
				pFlowConfig->path12.action.cos = _cos;
				pFlowConfig->path12.action.ldpid = _ldpid;
			}
		}
	}
#endif

	return RTK_FC_RET_OK;
}
#endif

static int32 RTK_RG_G3_FLOW_DEL(uint32 idx)
{
	asic_ret_t ret;

	FC_PARAM_CHK(idx >= fc_db.flowHwTableSize, CA_E_OK);

	ret = rtk_rg_asic_flow_del(idx);

	if(ret == ASIC_RET_SUCCESS){
		TABLE(">>>>> Delete Main Hash flow[%d]", idx);
	}else{
		TABLE(">>>>> Delete Main Hash flow[%d] fail, ret = 0x%x", idx, ret);
	}
	
	return ret;
}
//CONFIG_FC_RTL8277C_SERIES || CONFIG_FC_RTL9607F_SERIES
static int32 RTK_RG_G3_FLOW_SET(rtk_rg_asic_path1_entry_t *flow, uint32 *mainHashIdx, rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, uint8 swOnly, uint32 flowIdx, rtk_fc_g3_flow_add_state_t *flow_add_state)
{
	int ret = CA_E_OK;
	rtk_rg_asic_flow_config_t flowConfig;
	uint16 outterTPID = fc_db.systemGlobal.stagTPID[pG3IgrExtraInfo->egr_svlan_tpid_sel];
	bool flowIdActMask = FALSE;
	bool hybridFwd = (!!fc_db.controlFuc.hw_sw_hybrid_fwd && !!fc_db.controlFuc.shortcut_earlycheck_en && (pG3IgrExtraInfo->dualHdrType == RTK_FC_DUALTYPE_NONE));
	uint16 flowId;
	rtk_rg_asic_flow_hash_crc_t flow_hash_crc;
	uint8 flow_spa, flow_dpa;
	rtk_fc_wlan_devidx_t flow_igrWlanDevIdx, flow_egrWlanDevIdx;
#if defined(CONFIG_RTK_FC_WFO_PER_HW_FLOW_MIB)
	bool if_wifi_tx = FALSE, if_wifi_rx = FALSE, if_wifiTx_wo_amsdu = FALSE, if_wifiRx_wo_amsdu = FALSE;
	rtk_fc_wfo_flow_mib_tx_info_t flow_mib_rx_info = {0};
	rtk_fc_wfo_flow_mib_tx_info_t flow_mib_tx_info = {0};
#endif
#if defined(CONFIG_RTK_FC_PKT_QUEUE_OFFLOAD_BY_PE)
	int32 pe_tc_queue_idx = FAIL;
	rt_pe_tc_queue_test_request_t pe_tc_queue_test_req={0};
	rt_pe_tc_queue_test_rsv_cls_info_t pe_tc_queue_test_rsv_cls_info={0};
	pe_tc_queue_test_rsv_cls_info.lspid = FAIL;
#endif

	*flow_add_state = G3_FLOW_ADD_STATE_FAIL;
	if((swOnly) || (flowIdx >= fc_db.flowHwTableSize))
	{
		/*swOnly entry: not add to HW or index > HW table size*/
		*mainHashIdx = G3_FLOWIDX_INVALID;
		*flow_add_state = G3_FLOW_ADD_STATE_PURE_SW;
		return CA_E_OK;
	}

	if (flow->in_path == FB_PATH_12 || flow->in_path == FB_PATH_34 || flow->in_path == FB_PATH_5
#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)
		|| flow->in_path == FB_PATH_ESP_SPI_DS
#endif
	) {
		if(fc_db.lutTbl[pG3IgrExtraInfo->lutIgrSaIdx]==NULL)
		{
			WARNING("ingress SMAC: lutTbl[%d] is NULL!", pG3IgrExtraInfo->lutIgrSaIdx);
			goto ADD_HW_FLOW_FAIL;
		}

		if(fc_db.lutTbl[pG3IgrExtraInfo->lutEgrDaIdx]==NULL)
		{
			WARNING("path3 ingress DMAC: lutTbl[%d] is NULL!", pG3IgrExtraInfo->lutEgrDaIdx);
			goto ADD_HW_FLOW_FAIL;
		}

		flow_spa = fc_db.lutTbl[pG3IgrExtraInfo->lutIgrSaIdx]->spa;
		flow_dpa = fc_db.lutTbl[pG3IgrExtraInfo->lutEgrDaIdx]->spa;

	#if defined(CONFIG_RTK_FC_ETHPORT_TRUNKING_BY_ONE_LUT)
		if (IS_ETH_TRUNKING_PORT(pG3IgrExtraInfo->igrspa))
			flow_spa = pG3IgrExtraInfo->igrspa;

		if (IS_ETH_TRUNKING_PORT(pG3IgrExtraInfo->egrdpa))
			flow_dpa = pG3IgrExtraInfo->egrdpa;
	#endif

	#if defined(CONFIG_RTK_FC_WIFI_MULTIBAND_ACC_BY_ONE_LUT)
		flow_igrWlanDevIdx = pG3IgrExtraInfo->igrWlanDevIdx;
		flow_egrWlanDevIdx = pG3IgrExtraInfo->egrWlanDevIdx;
	#else
		flow_igrWlanDevIdx = fc_db.lutTbl[pG3IgrExtraInfo->lutIgrSaIdx]->wlan_dev_idx;
		flow_egrWlanDevIdx = fc_db.lutTbl[pG3IgrExtraInfo->lutEgrDaIdx]->wlan_dev_idx;
	#endif
	}

	TRACE("Translate flow in_path %d to G3 format\n", flow->in_path);

	memset(&flowConfig, 0, sizeof(flowConfig));

	if(flow->in_path == FB_PATH_5)
	{
		rtk_rg_asic_path5_entry_t *pFlowPath5 = (rtk_rg_asic_path5_entry_t *)flow;
		flowConfig.in_path = FB_PATH_5;

		/* === key === */
		// key: L2
		flowConfig.path5.key.orig_lspid = flow_spa;
		if(pFlowPath5->in_stagif)
		{
			flowConfig.path5.key.stag_if = TRUE;
			flowConfig.path5.key.svlan_tpid = fc_db.systemGlobal.stagTPID[pG3IgrExtraInfo->igr_svlan_tpid_sel];
			flowConfig.path5.key.svlan_id = pG3IgrExtraInfo->svlan_id;
			flowConfig.path5.key.svlan_pri = pG3IgrExtraInfo->svlan_pri;
			flowConfig.path5.key.svlan_dei = pG3IgrExtraInfo->igr_svlan_dei;
		}
		if(pFlowPath5->in_ctagif)
		{
			flowConfig.path5.key.ctag_if = TRUE;
			flowConfig.path5.key.cvlan_tpid = CVLAN_TPID;
			flowConfig.path5.key.cvlan_id = pG3IgrExtraInfo->cvlan_id;
			flowConfig.path5.key.cvlan_pri = pFlowPath5->in_cvlan_pri;
			flowConfig.path5.key.cvlan_cfi = pG3IgrExtraInfo->igr_cvlan_cfi;
		}

		if(pFlowPath5->in_pppoeif)
		{
			flowConfig.path5.key.pppoetag_if = TRUE;
			flowConfig.path5.key.pppoe_sid = pG3IgrExtraInfo->pppoe_session_id;
		}

		memcpy(&flowConfig.path5.key.src_mac[0], &fc_db.lutTbl[pG3IgrExtraInfo->lutIgrSaIdx]->l2Addr, ETH_ALEN);

		if(fc_db.lutTbl[fc_db.netifTbl[NETIF_HWTOSW(pFlowPath5->in_intf_idx)].lutIdx]==NULL)
		{
			WARNING("path5 ingress DMAC: lutTbl[%d] is NULL!", fc_db.netifTbl[NETIF_HWTOSW(pFlowPath5->in_intf_idx)].lutIdx);
			goto ADD_HW_FLOW_FAIL;
		}
		else
			memcpy(&flowConfig.path5.key.dst_mac[0], &fc_db.lutTbl[fc_db.netifTbl[NETIF_HWTOSW(pFlowPath5->in_intf_idx)].lutIdx]->l2Addr, ETH_ALEN);

		// key: L3
		if(pFlowPath5->in_ipv4_or_ipv6)
		{
			flowConfig.path5.key.ipv4_or_ipv6 = TRUE; // 1: IPv6, 0: IPv4
			flowConfig.path5.key.ip_sa[0] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Saddr.s6_addr[0]);
			flowConfig.path5.key.ip_sa[1] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Saddr.s6_addr[4]);
			flowConfig.path5.key.ip_sa[2] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Saddr.s6_addr[8]);
			flowConfig.path5.key.ip_sa[3] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Saddr.s6_addr[12]);
			flowConfig.path5.key.ip_da[0] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Daddr.s6_addr[0]);
			flowConfig.path5.key.ip_da[1] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Daddr.s6_addr[4]);
			flowConfig.path5.key.ip_da[2] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Daddr.s6_addr[8]);
			flowConfig.path5.key.ip_da[3] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Daddr.s6_addr[12]);
		}
		else
		{
			flowConfig.path5.key.ipv4_or_ipv6 = FALSE; // 1: IPv6, 0: IPv4
			flowConfig.path5.key.ip_sa[3] = pFlowPath5->in_src_ipv4_addr;
			if(pFlowPath5->out_l4_act)
			{
				// NAPT
				if(pFlowPath5->out_l4_direction)
					flowConfig.path5.key.ip_da[3] = pFlowPath5->in_dst_ipv4_addr;// outbound
				else
					flowConfig.path5.key.ip_da[3] = fc_db.netifTbl[NETIF_HWTOSW(pFlowPath5->in_intf_idx)].intf.gateway_ipv4_addr; // inbound
			}
			else
			{
				// Routing
				flowConfig.path5.key.ip_da[3] = pFlowPath5->in_dst_ipv4_addr;
			}
		}
		if(pFlowPath5->in_tos_check)
		{
			flowConfig.path5.key.ip_dscp = (pFlowPath5->in_tos >>2)&0x3f;
			flowConfig.path5.key.ip_ecn = pFlowPath5->in_tos&0x3;
		}
		flowConfig.path5.key.l4proto_num = pFlowPath5->in_l4proto_num;
		// key: L4
		flowConfig.path5.key.l4_src_port = pFlowPath5->in_l4_src_port;
		flowConfig.path5.key.l4_dst_port = pFlowPath5->in_l4_dst_port;
		// from PON
		if(flow_spa == RTK_FC_MAC_PORT_PON)
		{
			flowConfig.path5.key.is_from_ponRx_wifi_rx =  RTK_ASIC_FLOW_FROM_PON;
			flowConfig.path5.key.pon_streamId_or_wifi_devIdx = pG3IgrExtraInfo->pon_stream_id; // WAN rx stream id or wifi RX SSID
		}
#if defined(CONFIG_RTK_FC_FORWARDING_AWARE_LAN_SWID)
		if(fc_db.lutTbl[pG3IgrExtraInfo->lutIgrSaIdx]->spa < RTK_FC_MAC_PORT_PON) {
			flowConfig.path5.key.is_from_ponRx_wifi_rx =  RTK_ASIC_FLOW_FROM_PON;
			flowConfig.path5.key.pon_streamId_or_wifi_devIdx = pG3IgrExtraInfo->pon_stream_id;	// LAN rx swid(extspa)
		}
#endif
		// from wifi
		if((1 << flow_spa) & RTK_FC_ALL_MAC_WLANCPU_PORTMASK)
		{
			flowConfig.path5.key.is_from_ponRx_wifi_rx =  RTK_ASIC_FLOW_FROM_WIFI;
			flowConfig.path5.key.pon_streamId_or_wifi_devIdx = flow_igrWlanDevIdx; // WAN rx stream id or wifi RX SSID
#if defined(CONFIG_FC_RTL9607F_SERIES) && (defined(CONFIG_RTK_NIC_HWLOOKUP_SNAP_TRANSFORM_BY_HW) || defined(CONFIG_RTK_FC_WIFI_DRIVER_OFFLOAD_BY_PE))
			/* wifi RX: DMA LSO to splict AMSDU packet and HW hash to tranfer packet from SNAP to ETH*/
			flowConfig.path5.action.l2format_act_vld = TRUE;
			flowConfig.path5.action.l2format_act = RTK_ASIC_L2_FORMAT_ACT_ETHERNET;
#endif
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
			if(RTK_FC_HELPER_WLAN_IS_CLIENT_MODE(flow_igrWlanDevIdx)) {
				flowConfig.path5.action.disable_smac_pppoe_trans = TRUE; // ds to keep smac
			}
#endif
#if defined(CONFIG_RTK_FC_WFO_PER_HW_FLOW_MIB)
			if_wifi_rx = TRUE;
#endif
		}

		/* === action === */
		// action: L2
		flowConfig.path5.action.mac_da_idx = pG3IgrExtraInfo->lutEgrDaIdx;
		flowConfig.path5.action.vlan_act = TRUE;
		if((pFlowPath5->out_svid_format_act + pFlowPath5->out_cvid_format_act) == 2)
		{
			// double tag
			flowConfig.path5.action.vlan_cnt = 2;
			flowConfig.path5.action.outer_tpid = outterTPID;
			flowConfig.path5.action.outer_vlanid = pFlowPath5->out_svlan_id;
			flowConfig.path5.action.outer_vlanpri = pFlowPath5->out_spri;
			flowConfig.path5.action.outer_dei = pG3IgrExtraInfo->egr_svlan_dei;
			flowConfig.path5.action.inner_tpid = CVLAN_TPID;
			flowConfig.path5.action.inner_vlanid = pFlowPath5->out_cvlan_id;
			flowConfig.path5.action.inner_vlanpri = pFlowPath5->out_cpri;
			flowConfig.path5.action.inner_dei = pG3IgrExtraInfo->egr_cvlan_cfi;
		}
		else if(pFlowPath5->out_svid_format_act)
		{
			// single tag
			flowConfig.path5.action.vlan_cnt = 1;
			flowConfig.path5.action.outer_tpid = outterTPID;
			flowConfig.path5.action.outer_vlanid = pFlowPath5->out_svlan_id;
			flowConfig.path5.action.outer_vlanpri = pFlowPath5->out_spri;
			flowConfig.path5.action.outer_dei = pG3IgrExtraInfo->egr_svlan_dei;
		}
		else if(pFlowPath5->out_cvid_format_act)
		{
			// single tag
			flowConfig.path5.action.vlan_cnt = 1;
			flowConfig.path5.action.outer_tpid = CVLAN_TPID;
			flowConfig.path5.action.outer_vlanid = pFlowPath5->out_cvlan_id;
			flowConfig.path5.action.outer_vlanpri = pFlowPath5->out_cpri;
			flowConfig.path5.action.outer_dei = pG3IgrExtraInfo->egr_cvlan_cfi;
		}
		else
		{
			// un-tag
			flowConfig.path5.action.vlan_cnt = 0;
		}

		// action: L3/L4
		flowConfig.path5.action.ip_dscp_update_en = pFlowPath5->out_dscp_act;
		flowConfig.path5.action.ip_dscp = pFlowPath5->out_dscp;

		if(pFlowPath5->out_l4_act)
		{
			if(pG3IgrExtraInfo->natloopback_act_info.is_ipv4_natloopback)
			{
				// NAP LOOPBACK
				flowConfig.path5.action.ip_action_mode = RTK_ASIC_IP_ACTION_MODE_V4_NAT_LOOPBACK;
				if(pFlowPath5->in_ipv4_or_ipv6==0)
				{
					//IPv4
					if(pFlowPath5->out_l4_direction)
					{
						// outbound
						flowConfig.path5.action.nat_loopback_cfg.ipv4_sip = fc_db.netifTbl[NETIF_HWTOSW(pFlowPath5->out_intf_idx)].intf.gateway_ipv4_addr;
						flowConfig.path5.action.nat_loopback_cfg.l4_sport = pFlowPath5->out_l4_port;
						flowConfig.path5.action.nat_loopback_cfg.ipv4_dip = pG3IgrExtraInfo->natloopback_act_info.l3_ip;
						flowConfig.path5.action.nat_loopback_cfg.l4_dport = pG3IgrExtraInfo->natloopback_act_info.l4_port;
					}
					else
					{
						// inbound
						flowConfig.path5.action.nat_loopback_cfg.ipv4_sip = pG3IgrExtraInfo->natloopback_act_info.l3_ip;
						flowConfig.path5.action.nat_loopback_cfg.l4_sport = pG3IgrExtraInfo->natloopback_act_info.l4_port;
						flowConfig.path5.action.nat_loopback_cfg.ipv4_dip = pFlowPath5->out_dst_ipv4_addr;
						flowConfig.path5.action.nat_loopback_cfg.l4_dport = pFlowPath5->out_l4_port;
					}
				}
				else if(pFlowPath5->in_ipv4_or_ipv6==1)
				{
					WARNING("Do not support IPv6 NAT LOOPBACK!");
					goto ADD_HW_FLOW_FAIL;
				}
			}
			else
			{
				// NAPT
				flowConfig.path5.action.ip_action_mode = RTK_ASIC_IP_ACTION_MODE_V4_V6_NAPT;// 0: v4_v6_routing, 1: v4_v6_NAPT, 2: v6_NPT (rtk_8277c_asic_flow_config_ip_action_mode_t)
				if(pFlowPath5->in_ipv4_or_ipv6==0)
				{
					//IPv4
					if(pFlowPath5->out_l4_direction)
					{
						// outbound
						flowConfig.path5.action.ip_type = 0; // 0: replace ip_sa/l4_sport according to ip_addr configuration (NAPT); 1: replace ip_da/l4_dport according to ip_addr configuration (NAPTR)
						flowConfig.path5.action.ip[3] = fc_db.netifTbl[NETIF_HWTOSW(pFlowPath5->out_intf_idx)].intf.gateway_ipv4_addr;
					}
					else
					{
						// inbound
						flowConfig.path5.action.ip_type = 1; // 0: replace ip_sa/l4_sport according to ip_addr configuration (NAPT); 1: replace ip_da/l4_dport according to ip_addr configuration (NAPTR)
						flowConfig.path5.action.ip[3] = pFlowPath5->out_dst_ipv4_addr;
					}
					flowConfig.path5.action.l4_port = pFlowPath5->out_l4_port;
				}
				else if(pFlowPath5->in_ipv4_or_ipv6==1)
				{
					//IPv6
					if(pFlowPath5->out_l4_direction)
					{
						// outbound
						flowConfig.path5.action.ip_type = 0; // 0: replace ip_sa/l4_sport according to ip_addr configuration (NAPT); 1: replace ip_da/l4_dport according to ip_addr configuration (NAPTR)
					}
					else
					{
						// inbound
						flowConfig.path5.action.ip_type = 1; // 0: replace ip_sa/l4_sport according to ip_addr configuration (NAPT); 1: replace ip_da/l4_dport according to ip_addr configuration (NAPTR)
					}
					flowConfig.path5.action.ip[0] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_nat_addr.s6_addr[0]);
					flowConfig.path5.action.ip[1] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_nat_addr.s6_addr[4]);
					flowConfig.path5.action.ip[2] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_nat_addr.s6_addr[8]);
					flowConfig.path5.action.ip[3] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_nat_addr.s6_addr[12]);
					flowConfig.path5.action.l4_port = pFlowPath5->out_l4_port;
				}
			}
		}
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		else if(pFlowPath5->out_change_l4_port_only == 1)
		{
			// change SPORT only
			flowConfig.path5.action.ip_action_mode = RTK_ASIC_IP_ACTION_MODE_V4_V6_NAPT;// 0: v4_v6_routing, 1: v4_v6_NAPT, 2: v6_NPT (rtk_8277c_asic_flow_config_ip_action_mode_t)
			flowConfig.path5.action.ip_type = 0; // 0: replace ip_sa/l4_sport according to ip_addr configuration (NAPT); 1: replace ip_da/l4_dport according to ip_addr configuration (NAPTR)
			flowConfig.path5.action.l4_port = pFlowPath5->out_l4_port;
			if(pFlowPath5->in_ipv4_or_ipv6==0)
			{
				//IPv4: keep
				flowConfig.path5.action.ip[3] = pFlowPath5->in_src_ipv4_addr;
			}
			else if(pFlowPath5->in_ipv4_or_ipv6==1)
			{
				//IPv6: keep
				flowConfig.path5.action.ip[0] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Saddr.s6_addr[0]);
				flowConfig.path5.action.ip[1] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Saddr.s6_addr[4]);
				flowConfig.path5.action.ip[2] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Saddr.s6_addr[8]);
				flowConfig.path5.action.ip[3] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Saddr.s6_addr[12]);
			}
		}
		else if(pFlowPath5->out_change_l4_port_only == 2)
		{
			// change DPORT only
			flowConfig.path5.action.ip_action_mode = RTK_ASIC_IP_ACTION_MODE_V4_V6_NAPT;// 0: v4_v6_routing, 1: v4_v6_NAPT, 2: v6_NPT (rtk_8277c_asic_flow_config_ip_action_mode_t)
			flowConfig.path5.action.ip_type = 1; // 0: replace ip_sa/l4_sport according to ip_addr configuration (NAPT); 1: replace ip_da/l4_dport according to ip_addr configuration (NAPTR)
			flowConfig.path5.action.l4_port = pFlowPath5->out_l4_port;
			if(pFlowPath5->in_ipv4_or_ipv6==0)
			{
				//IPv4: keep
				flowConfig.path5.action.ip[3] = pFlowPath5->in_dst_ipv4_addr;
			}
			else if(pFlowPath5->in_ipv4_or_ipv6==1)
			{
				//IPv6: keep
				flowConfig.path5.action.ip[0] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Daddr.s6_addr[0]);
				flowConfig.path5.action.ip[1] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Daddr.s6_addr[4]);
				flowConfig.path5.action.ip[2] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Daddr.s6_addr[8]);
				flowConfig.path5.action.ip[3] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Daddr.s6_addr[12]);
			}
		}
#endif

		// action: Others
		flowConfig.path5.action.cos_update_en = pFlowPath5->out_user_pri_act;
		flowConfig.path5.action.cos = pFlowPath5->out_user_priority;
		flowConfig.path5.action.ldpid = flow_dpa;
		flowConfig.path5.action.mapeMapT_fmr_idx_vld = pFlowPath5->out_fmr_idx_act;
		flowConfig.path5.action.mapeMapT_fmr_idx = pFlowPath5->out_fmr_idx;
		// to Wifi
		if((1<<(flowConfig.path5.action.ldpid))&RTK_FC_ALL_MAC_WLANCPU_PORTMASK)
		{
			flowConfig.path5.action.wifi_ssid = flow_egrWlanDevIdx;
#if defined(CONFIG_SMP)
			{
				uint16 wifi_tx_dispatch_mode;
				uint8 cpu_idx;
				uint band;
				band = RTK_FC_HELPER_WLAN_DEVID_TO_BANDIDX(flow_egrWlanDevIdx);
				if(RTK_FC_MGR_DISPATCH_SMP_MODE_WIFI_TX_CPUID_GET(&cpu_idx, &wifi_tx_dispatch_mode, flowIdx>>1, (RTK_FC_MGR_DISPATCH_WLAN0_TX+band)) == SUCCESS)
				{
					if((wifi_tx_dispatch_mode == RTK_FC_DISPATCH_MODE_SMP_BY_HASH) || (wifi_tx_dispatch_mode == RTK_FC_DISPATCH_MODE_SMP_BY_RR))
					{
						flowConfig.path5.action.ldpid = RTK_FC_MAC_PORT_CPU + cpu_idx;
						TRACE("WIFI TX dispatch is SMP mode, decide WIFI TX CPU port ID 0x%x by %s", flowConfig.path5.action.ldpid, (wifi_tx_dispatch_mode == RTK_FC_DISPATCH_MODE_SMP_BY_HASH)?"HASH":"RR");
					}
				}
			}
#endif
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
#if defined(CONFIG_RTK_FC_WFO_PER_HW_FLOW_MIB)
			if(fc_db.pe_offload_wifi_amsdu_en)
				RTK_RG_ASIC_WIFI_AMSDU_VOQ_SEL(flow, pG3IgrExtraInfo, &flowConfig, TRUE/*wifi TX*/, &flow_mib_tx_info);

			if_wifi_tx = TRUE;
			if(fc_db.wfo_per_flow_mib_init_done && (flowConfig.path5.action.is_amsdu_pe_offload_wifiTx == FALSE))
				if_wifiTx_wo_amsdu = TRUE;
#else
			if(fc_db.pe_offload_wifi_amsdu_en)
				RTK_RG_ASIC_WIFI_AMSDU_VOQ_SEL(flow, pG3IgrExtraInfo, &flowConfig, TRUE/*wifi TX*/, NULL);
#endif
#endif
		}
		flowConfig.path5.action.ingress_intf_idx = _rtk_fc_flow_intf_mapping_idx_get(pFlowPath5->in_intf_idx);
		flowConfig.path5.action.egress_intf_idx = _rtk_fc_flow_intf_mapping_idx_get(pFlowPath5->out_intf_idx);
		flowConfig.path5.action.l3_if_vld1	= TRUE;
		_rtk_fc_g3FlowIdDecision(flow, pG3IgrExtraInfo, &flowIdActMask, &flowId);
		// policer1 is host policing
		if(flowIdActMask)
		{
			flowConfig.path5.action.pol_en = TRUE;
			flowConfig.path5.action.pol_id = flowId;
		}

		// policer_2: flow meter (+ flow mib (flow_meter_mib_conf_dependence enabled))
		if(pFlowPath5->out_share_meter_act)
		{
			flowConfig.path5.action.pol2_en = TRUE;
			flowConfig.path5.action.pol2_id = pFlowPath5->out_share_meter_idx + G3_FLOW_POLICER_IDXSHIFT_FLOWMTR;
		}

		// policer_3: flow mib (flow_meter_mib_conf_dependence disabled)
		if(!fc_db.controlFuc.flow_meter_mib_conf_dependence)
		{
			if(fc_db.controlFuc.flow_mib_mode == RT_STAT_FLOW_MIB_MODE_DEFAULT)
			{
				if(pFlowPath5->out_flow_counter_act)
				{
					flowConfig.path5.action.pol3_en = TRUE;
					flowConfig.path5.action.pol3_id = pFlowPath5->out_flow_counter_idx + G3_FLOW_POLICER_IDXSHIFT_FLOWMIB;
				}
			}
			else
			{
				//RT_STAT_FLOW_MIB_MODE_EXTRA
				if(pFlowPath5->out_flow_counter_act)
				{
					flowConfig.path5.action.pol2_en = TRUE;
					flowConfig.path5.action.pol2_id = pFlowPath5->out_flow_counter_idx + G3_FLOW_POLICER_IDXSHIFT_FLOWMIB;
				}
			}
		}

		if(pFlowPath5->out_flow_counter2_act)
		{
			if(fc_db.controlFuc.flow_mib_mode == RT_STAT_FLOW_MIB_MODE_DEFAULT)
			{
				//RT_STAT_FLOW_MIB_MODE_EXTRA
				WARNING("Not support flow mib2 in RT_STAT_FLOW_MIB_MODE_DEFAULT mode");
				 goto ADD_HW_FLOW_FAIL;
			}
			else
			{
				//RT_STAT_FLOW_MIB_MODE_EXTRA
				flowConfig.path5.action.pol3_en = TRUE;
				flowConfig.path5.action.pol3_id = pFlowPath5->out_flow_counter2_idx + G3_FLOW_POLICER_IDXSHIFT_FLOWMIB2;
			}
		}

#if defined(CONFIG_RG_G3_WAN_PORT_INDEX)&&(CONFIG_RG_G3_WAN_PORT_INDEX==7)
		if(pFlowPath5->out_stream_idx_act && fc_db.streamidTbl[pFlowPath5->out_stream_idx].valid &&
			(fc_db.wanPortMask.portmask & (1 << flowConfig.path5.action.ldpid)))
		{
			flowConfig.path5.action.gemId_mapping_mode = RTK_GEM_MAPPING_MODE;
			flowConfig.path5.action.gem_id = fc_db.streamidTbl[pFlowPath5->out_stream_idx].gemid;
			flowConfig.path5.action.tcont_id = fc_db.streamidTbl[pFlowPath5->out_stream_idx].ldpid&0x1f;
			if(fc_db.streamidTbl[pFlowPath5->out_stream_idx].cos)
			{
				flowConfig.path5.action.cos_update_en = TRUE;
				flowConfig.path5.action.cos = fc_db.streamidTbl[pFlowPath5->out_stream_idx].cos;
			}
			else
				flowConfig.path5.action.cos_update_en = FALSE;// always disable cos update to prevent packet is sent to invalid PON queue
		}
#endif
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
		if(((1U << fc_db.lutTbl[pG3IgrExtraInfo->lutIgrSaIdx]->spa) & fc_db.controlFuc.amsdu_pe_dbg_mode_rx_portmask) && 
			((1U << fc_db.lutTbl[pG3IgrExtraInfo->lutEgrDaIdx]->spa) & fc_db.controlFuc.amsdu_pe_dbg_mode_tx_portmask))
		{
			if(flowConfig.path5.action.gemId_mapping_mode == RTK_GEM_MAPPING_MODE)
			{
				if((1U << flowConfig.path5.action.tcont_id) & fc_db.controlFuc.amsdu_pe_dbg_mode_tx_tcontIdmask)
					RTK_RG_ASIC_AMSDU_DBG_MODE_VOQ_SEL(flowConfig.path5.action.tcont_id+0x20, flowConfig.path5.action.cos_update_en?flowConfig.path5.action.cos:0, flowConfig.path5.action.gem_id, &flowConfig);
			}
			else
				RTK_RG_ASIC_AMSDU_DBG_MODE_VOQ_SEL(flowConfig.path5.action.ldpid, flowConfig.path5.action.cos_update_en?flowConfig.path5.action.cos:0, 0, &flowConfig);
		}
#endif
#if defined(CONFIG_RTK_FC_WFO_PER_HW_FLOW_MIB)
		if((if_wifi_tx == FALSE) && if_wifi_rx)
		{
			// should be done after ldpid/stream_id decision
			if(fc_db.pe_offload_wifi_amsdu_en)
				RTK_RG_ASIC_WIFI_AMSDU_VOQ_SEL(flow, pG3IgrExtraInfo, &flowConfig, FALSE/*wifi RX*/, &flow_mib_rx_info);
			if(fc_db.wfo_per_flow_mib_init_done && (flowConfig.path5.action.is_amsdu_pe_offload_wifiRx == FALSE))
				if_wifiRx_wo_amsdu = TRUE;
			if(flowConfig.path5.action.is_amsdu_pe_offload_wifiRx == TRUE)
				flowConfig.path5.action.gemId_mapping_mode = 0; // disable gem mapping mode for upstream
		}
#endif
		if(pG3IgrExtraInfo->dualHdrType == RTK_FC_DUALTYPE_VXLAN)
		{
			if(pG3IgrExtraInfo->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM)
			{
				DEBUG("VXLAN upstream");	
				flowConfig.path5.action.vxlan_sport_update_en = TRUE;
				flowConfig.path5.action.vxlan_sport = pG3IgrExtraInfo->vxlan_info.outer_srcPort;
				
			}
		}

		if(pG3IgrExtraInfo->naptv6_info.isNPTv6 ==1)
		{
			
			flowConfig.path5.action.nptv6_cfg.ipv6_prefix_index_aft = pG3IgrExtraInfo->naptv6_info.prefix_idx;
			if(pFlowPath5->out_l4_direction)
			{
				flowConfig.path5.action.nptv6_cfg.ipv6_prefix_len_aft = pG3IgrExtraInfo->naptv6_info.wan_prefix_len;
				flowConfig.path5.action.nptv6_cfg.ipv6_prefix_len_ori = pG3IgrExtraInfo->naptv6_info.lan_prefix_len;
			}
			else
			{
				flowConfig.path5.action.nptv6_cfg.ipv6_prefix_len_aft = pG3IgrExtraInfo->naptv6_info.lan_prefix_len;
				flowConfig.path5.action.nptv6_cfg.ipv6_prefix_len_ori = pG3IgrExtraInfo->naptv6_info.wan_prefix_len;
			}
			flowConfig.path5.action.ip_action_mode = RTK_ASIC_IP_ACTION_MODE_V6_NPT;
			flowConfig.path5.action.nptv6_cfg.if_ipv6_add_hw_recal = 1;

		}

		if(pFlowPath5->sw_shaper_en) {
			if(hybridFwd) {
				// redirect to main cpu instead of original dest port
				flowConfig.path5.action.ldpid = RTK_FC_MAC_PORT_MAINCPU;
				flowConfig.path5.action.wifi_ssid = RTK_FC_SWID_HW_SW_HYBRID_FWD;
				flowConfig.path5.action.pol_en = 0;
				flowConfig.path5.action.pol2_en = 0;
				flowConfig.path5.action.pol3_en = 0;
				flowConfig.path5.action.gemId_mapping_mode = 0;
#if defined(CONFIG_FC_RTL9607F_SERIES)
				if(flowConfig.path5.action.l2format_act_vld == TRUE && 
					flowConfig.path5.action.l2format_act == RTK_ASIC_L2_FORMAT_ACT_SNAP &&
					!fc_db.controlFuc.hw_sw_hybrid_fwd_to_pe) {	// to ARM wifi driver, disable snap translation
					flowConfig.path5.action.l2format_act_vld = FALSE;
					flowConfig.path5.action.l2format_act = RTK_ASIC_L2_FORMAT_ACT_ETHERNET;
				}
#endif
			}else
				flowConfig.path5.action.sw_shaper_en = pFlowPath5->sw_shaper_en;
				
		}

#if defined(CONFIG_RTK_FC_PKT_QUEUE_OFFLOAD_BY_PE)
		if(fc_db.controlFuc.pe_offload_pkt_queue_portmask)
		{
			int32 do_pe_offload=0, pe_cpu_port, pe_queue_idx=0, connection_idx=FAIL;
			if((fc_db.controlFuc.pe_offload_pkt_queue_portmask & (0x1<<flowConfig.path5.key.orig_lspid))
				&& (fc_db.wanPortMask.portmask & (0x1<<flowConfig.path5.action.ldpid)))
			{
				if(fc_db.pe_queue_tc_mode==0)
				{
					TRACE("[PE QUEUE] Offload upstream lspid %d and pon streamId %d to PE", flowConfig.path5.key.orig_lspid, pFlowPath5->out_stream_idx);
					pe_cpu_port = RT_PE_QUEUE_TEST_CPU_PORT;
					do_pe_offload = 1;
				}
				else if(fc_db.pe_queue_tc_mode_only_for_small_pkt==0 || pG3IgrExtraInfo->pe_tc_queue_flow_lspid!=FAIL) //tc mode
				{
					if(fc_db.pe_tc_queue_pe_used_num==MAX_PE_TC_QUEUE_TEST_PE_USED_NUM)
					{
						pe_queue_idx = (fc_db.pe_queue_tc_mode_slave_portmask & (0x1<<flowConfig.path5.key.orig_lspid)) ? (fc_db.pe_tc_queue_main_queue_idx^0x1) : fc_db.pe_tc_queue_main_queue_idx ;
					}
					pe_cpu_port = (pe_queue_idx==0) ? RT_PE_TC_QUEUE_0_TEST_CPU_PORT : RT_PE_TC_QUEUE_1_TEST_CPU_PORT ;

					connection_idx = rtk_fc_pe_tc_queue_test_free_connection_index_get(pe_queue_idx, flowIdx);
					if(connection_idx != FAIL)
					{
						memset(&pe_tc_queue_test_req, 0, sizeof(rt_pe_tc_queue_test_request_t));
						pe_tc_queue_test_req.req_cmd = RT_PE_TC_QUEUE_TEST_CMD_CONNECTION_ADD;
						pe_tc_queue_test_req.conn_info.connection_idx = connection_idx;
						pe_tc_queue_test_req.conn_info.ipv4_en 	= (flowConfig.path5.key.ipv4_or_ipv6) ? 0 : 1;
						pe_tc_queue_test_req.conn_info.ipv6_en 	= (flowConfig.path5.key.ipv4_or_ipv6) ? 1 : 0;
						pe_tc_queue_test_req.conn_info.tcp_en 	= (flowConfig.path5.key.l4proto_num==IPPROTO_TCP) ? 1 : 0;
						pe_tc_queue_test_req.conn_info.udp_en 	= (flowConfig.path5.key.l4proto_num==IPPROTO_UDP) ? 1 : 0;
						pe_tc_queue_test_req.conn_info.lspid 	= RTK_FC_PE_TC_QUEUE_TEST_DMA_LSO_LSPID;
						pe_tc_queue_test_req.conn_info.pol_grp_id 	= 0;
						pe_tc_queue_test_req.conn_info.fe_bypass 	= 1;
						if(pFlowPath5->out_stream_idx_act && fc_db.streamidTbl[pFlowPath5->out_stream_idx].valid)
						{
							pe_tc_queue_test_req.conn_info.ldpid 	= fc_db.streamidTbl[pFlowPath5->out_stream_idx].ldpid;
							pe_tc_queue_test_req.conn_info.cos 		= fc_db.streamidTbl[pFlowPath5->out_stream_idx].cos;
							pe_tc_queue_test_req.conn_info.flowid 	= fc_db.streamidTbl[pFlowPath5->out_stream_idx].gemid;
						}
						else
						{
							pe_tc_queue_test_req.conn_info.ldpid 	= flowConfig.path5.action.ldpid;
						}
						if(pG3IgrExtraInfo->dualHdrType==RTK_FC_DUALTYPE_NONE)
						{
							TRACE("[PE TC QUEUE][Single] Offload upstream lspid %d and pon streamId %d to PE cpu port 0x%x", flowConfig.path5.key.orig_lspid, pFlowPath5->out_stream_idx, pe_cpu_port);
							do_pe_offload = 1;
							
						}
						else //dual
						{
							if(fc_db.netifTbl[NETIF_HWTOSW(pFlowPath5->out_intf_idx)].extra_available && fc_db.netifTbl[NETIF_HWTOSW(pFlowPath5->out_intf_idx)].hwIdx_for_pe!=SIGNED_INVALID)
							{
								TRACE("[PE TC QUEUE][Dual] Offload upstream lspid %d and pon streamId %d to PE cpu port 0x%x", flowConfig.path5.key.orig_lspid, pFlowPath5->out_stream_idx, pe_cpu_port);
								do_pe_offload = 1;
								pe_tc_queue_test_req.conn_info.lspid 	= RTK_FC_DUAL_EXTRA_HWLOOKUP_LSPID_BASE + pG3IgrExtraInfo->dualHdrType;
								pe_tc_queue_test_req.conn_info.pol_grp_id 	= fc_db.netifTbl[NETIF_HWTOSW(pFlowPath5->out_intf_idx)].extra_netifId_map;
								pe_tc_queue_test_req.conn_info.ldpid 	= RT_PE_TC_QUEUE_TEST_HWLOOKUP_LDPID;
								pe_tc_queue_test_req.conn_info.fe_bypass 	= 0;
								//change egr_intf to single
								flowConfig.path5.action.egress_intf_idx = fc_db.netifTbl[NETIF_HWTOSW(pFlowPath5->out_intf_idx)].hwIdx_for_pe;
							}
							else
							{
								TRACE("[PE TC QUEUE][Dual] Fail to redirect to PE ... due to this dualHdrType %d does not support extra_available to do HWLOOKUP !!", pG3IgrExtraInfo->dualHdrType);
							}
						}
					}
				}
			}
			if(do_pe_offload)
			{
				flowConfig.path5.action.ldpid = pe_cpu_port;
				flowConfig.path5.action.gemId_mapping_mode = 0;
				flowConfig.path5.action.cos_update_en = TRUE;
				flowConfig.path5.action.cos = 0;
				if(fc_db.pe_queue_tc_mode==0)
				{
					flowConfig.path5.action.wifi_ssid = pFlowPath5->out_stream_idx + RT_PE_QUEUE_TEST_SW_ID_BASE;
				}
				else
				{
					if(pG3IgrExtraInfo->pe_tc_queue_flow_lspid != FAIL)
					{
						//for rsv cls
						pe_tc_queue_test_rsv_cls_info.lspid = flowConfig.path5.key.orig_lspid;
						memcpy(pe_tc_queue_test_rsv_cls_info.smac, flowConfig.path5.key.src_mac, ETH_ALEN);
						memcpy(pe_tc_queue_test_rsv_cls_info.dmac, flowConfig.path5.key.dst_mac, ETH_ALEN);
						if(pFlowPath5->in_ipv4_or_ipv6==0)
						{
							pe_tc_queue_test_rsv_cls_info.isIPv4OrIpv6 = 0;
							pe_tc_queue_test_rsv_cls_info.ipv4_dip_addr = pFlowPath5->in_dst_ipv4_addr;
						}
						else
						{
							pe_tc_queue_test_rsv_cls_info.isIPv4OrIpv6 = 1;
							memcpy(&pe_tc_queue_test_rsv_cls_info.ipv6_dip_addr, &pG3IgrExtraInfo->igr_ipv6_dip, sizeof(struct in6_addr));
						}
						pe_tc_queue_test_rsv_cls_info.action_lspid = pG3IgrExtraInfo->pe_tc_queue_flow_lspid;
						//for redirect flow key
						flowConfig.path5.key.orig_lspid = pG3IgrExtraInfo->pe_tc_queue_flow_lspid;	
					}
					flowConfig.path5.action.wifi_ssid = connection_idx + RT_PE_TC_QUEUE_TEST_SW_ID_BASE;
					pe_tc_queue_idx = pe_queue_idx;
					flow->pe_tc_queue_valid = 1; 
					flow->pe_tc_queue_idx = pe_tc_queue_idx; 
					flow->pe_tc_queue_connection_idx = connection_idx;
				}
			}
		}
#endif
#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)
		if(pG3IgrExtraInfo->ipsec_pe_offload ==1)
		{
			//disable mtu checking
			flowConfig.path5.action.disable_mtu_check = TRUE;
			// un-tag
			flowConfig.path5.action.vlan_cnt = 0;
			flowConfig.path5.action.wifi_ssid = fc_db.fc_ipsec_info[pG3IgrExtraInfo->ipsec_hook_table_index].pe_crypto_sw_id;
			flowConfig.path5.action.gemId_mapping_mode = 0;
			flowConfig.path5.action.ldpid = RT_PE_IPSEC_CPU_PORT;
			flowConfig.path5.action.cos_update_en = TRUE;
			flowConfig.path5.action.cos = 0;
			if(fc_db.fc_ipsec_info[pG3IgrExtraInfo->ipsec_hook_table_index].direction==RTK_FC_IPSEC_DIR_ENCRYT)
			{
				flowConfig.path5.action.disable_smac_pppoe_trans = TRUE; // for pppoe wan without pppoe tag
				flowConfig.path5.action.wifi_ssid |= ((pFlowPath5->in_ipv4_or_ipv6) ? RT_PE_IPSEC_SW_ID_INNER_IPV6_BIT : 0x0 );
			}
		}
#endif	
#if defined(CONFIG_RTK_FC_SRV6_OFFLOAD_BY_PE)
		if(pG3IgrExtraInfo->dualHdrType == RTK_FC_DUALTYPE_SRV6 && pG3IgrExtraInfo->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM && pG3IgrExtraInfo->srv6_connection_idx!=FAIL)
		{
			flowConfig.path5.action.wifi_ssid = (pG3IgrExtraInfo->srv6_connection_idx + RT_PE_SRV6_SW_ID_CONNECTION_IDX_BASE);
			flowConfig.path5.action.ldpid = RT_PE_SRV6_CPU_PORT;
			flowConfig.path5.action.gemId_mapping_mode = 0;
			flowConfig.path5.action.cos_update_en = TRUE;
			flowConfig.path5.action.cos = 0;
		}
#endif
	}
	else if(flow->in_path == FB_PATH_34)
	{
		rtk_rg_asic_path3_entry_t *pFlowPath3 = (rtk_rg_asic_path3_entry_t *)flow;
		flowConfig.in_path = FB_PATH_34;

		/* === key === */
		// key: L2
		flowConfig.path34.key.orig_lspid = flow_spa;
		if(pFlowPath3->in_stagif)
		{
			flowConfig.path34.key.stag_if = TRUE;
			flowConfig.path34.key.svlan_tpid = fc_db.systemGlobal.stagTPID[pG3IgrExtraInfo->igr_svlan_tpid_sel];
			flowConfig.path34.key.svlan_id = pG3IgrExtraInfo->svlan_id;
			flowConfig.path34.key.svlan_pri = pG3IgrExtraInfo->svlan_pri;
			flowConfig.path34.key.svlan_dei = pG3IgrExtraInfo->igr_svlan_dei;
		}
		if(pFlowPath3->in_ctagif)
		{
			flowConfig.path34.key.ctag_if = TRUE;
			flowConfig.path34.key.cvlan_tpid = CVLAN_TPID;
			flowConfig.path34.key.cvlan_id = pG3IgrExtraInfo->cvlan_id;
			flowConfig.path34.key.cvlan_pri = pFlowPath3->in_cvlan_pri;
			flowConfig.path34.key.cvlan_cfi = pG3IgrExtraInfo->igr_cvlan_cfi;
		}

		if(pFlowPath3->in_pppoeif)
		{
			flowConfig.path34.key.pppoetag_if = TRUE;
			flowConfig.path34.key.pppoe_sid = pG3IgrExtraInfo->pppoe_session_id;
		}

		memcpy(&flowConfig.path34.key.src_mac[0], &fc_db.lutTbl[pG3IgrExtraInfo->lutIgrSaIdx]->l2Addr, ETH_ALEN);
		memcpy(&flowConfig.path34.key.dst_mac[0], &fc_db.lutTbl[pG3IgrExtraInfo->lutEgrDaIdx]->l2Addr, ETH_ALEN);

		// key: L3
		flowConfig.path34.key.ipv4_or_ipv6 = pFlowPath3->in_ipv4_or_ipv6; // 1: IPv6, 0: IPv4
		if(pFlowPath3->in_ipv4_or_ipv6)
		{
			flowConfig.path34.key.ipv4_or_ipv6 = TRUE; // 1: IPv6, 0: IPv4
			flowConfig.path34.key.ip_sa[0] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Saddr.s6_addr[0]);
			flowConfig.path34.key.ip_sa[1] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Saddr.s6_addr[4]);
			flowConfig.path34.key.ip_sa[2] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Saddr.s6_addr[8]);
			flowConfig.path34.key.ip_sa[3] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Saddr.s6_addr[12]);
			flowConfig.path34.key.ip_da[0] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Daddr.s6_addr[0]);
			flowConfig.path34.key.ip_da[1] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Daddr.s6_addr[4]);
			flowConfig.path34.key.ip_da[2] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Daddr.s6_addr[8]);
			flowConfig.path34.key.ip_da[3] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Daddr.s6_addr[12]);
		}
		else
		{
			flowConfig.path34.key.ipv4_or_ipv6 = FALSE; // 1: IPv6, 0: IPv4
			flowConfig.path34.key.ip_sa[3] = pFlowPath3->in_src_ipv4_addr;
			flowConfig.path34.key.ip_da[3] = pFlowPath3->in_dst_ipv4_addr;
		}

		if(pFlowPath3->in_tos_check)
		{
			flowConfig.path34.key.ip_dscp = (pFlowPath3->in_tos >>2)&0x3f;
			flowConfig.path34.key.ip_ecn = pFlowPath3->in_tos&0x3;
		}
		flowConfig.path34.key.l4proto_num = pFlowPath3->in_l4proto_num;

		// key: L4
		flowConfig.path34.key.l4_src_port = pFlowPath3->in_l4_src_port;
		flowConfig.path34.key.l4_dst_port = pFlowPath3->in_l4_dst_port;
		// from PON
		if(flow_spa == RTK_FC_MAC_PORT_PON)
		{
			flowConfig.path34.key.is_from_ponRx_wifi_rx =  RTK_ASIC_FLOW_FROM_PON;
			flowConfig.path34.key.pon_streamId_or_wifi_devIdx = pG3IgrExtraInfo->pon_stream_id; // WAN rx stream id or wifi RX SSID
		}
#if defined(CONFIG_RTK_FC_FORWARDING_AWARE_LAN_SWID)
		if(fc_db.lutTbl[pG3IgrExtraInfo->lutIgrSaIdx]->spa < RTK_FC_MAC_PORT_PON) {
			flowConfig.path34.key.is_from_ponRx_wifi_rx =  RTK_ASIC_FLOW_FROM_PON;
			flowConfig.path34.key.pon_streamId_or_wifi_devIdx = pG3IgrExtraInfo->pon_stream_id;	// LAN rx swid(extspa)
		}
#endif
		// from wifi
		if((1 << flow_spa) & RTK_FC_ALL_MAC_WLANCPU_PORTMASK)
		{
			flowConfig.path34.key.is_from_ponRx_wifi_rx =  RTK_ASIC_FLOW_FROM_WIFI;
			flowConfig.path34.key.pon_streamId_or_wifi_devIdx = flow_igrWlanDevIdx; // WAN rx stream id or wifi RX SSID
#if defined(CONFIG_FC_RTL9607F_SERIES) && (defined(CONFIG_RTK_NIC_HWLOOKUP_SNAP_TRANSFORM_BY_HW) || defined(CONFIG_RTK_FC_WIFI_DRIVER_OFFLOAD_BY_PE))
			/* wifi RX: DMA LSO to splict AMSDU packet and HW hash to tranfer packet from SNAP to ETH*/
			flowConfig.path34.action.l2format_act_vld = TRUE;
			flowConfig.path34.action.l2format_act = RTK_ASIC_L2_FORMAT_ACT_ETHERNET;
#endif
#if defined(CONFIG_RTK_FC_WFO_PER_HW_FLOW_MIB)
			if_wifi_rx = TRUE;
#endif
		}

		/* === action === */
		// action: L2
		flowConfig.path34.action.vlan_act = TRUE;
		if((pFlowPath3->out_svid_format_act + pFlowPath3->out_cvid_format_act) == 2)
		{
			// double tag
			flowConfig.path34.action.vlan_cnt = 2;
			flowConfig.path34.action.outer_tpid = outterTPID;
			flowConfig.path34.action.outer_vlanid = pFlowPath3->out_svlan_id;
			flowConfig.path34.action.outer_vlanpri = pFlowPath3->out_spri;
			flowConfig.path34.action.outer_dei = pG3IgrExtraInfo->egr_svlan_dei;
			flowConfig.path34.action.inner_tpid = CVLAN_TPID;
			flowConfig.path34.action.inner_vlanid = pFlowPath3->out_cvlan_id;
			flowConfig.path34.action.inner_vlanpri = pFlowPath3->out_cpri;
			flowConfig.path34.action.inner_dei = pG3IgrExtraInfo->egr_cvlan_cfi;
		}
		else if(pFlowPath3->out_svid_format_act)
		{
			// single tag
			flowConfig.path34.action.vlan_cnt = 1;
			flowConfig.path34.action.outer_tpid = outterTPID;
			flowConfig.path34.action.outer_vlanid = pFlowPath3->out_svlan_id;
			flowConfig.path34.action.outer_vlanpri = pFlowPath3->out_spri;
			flowConfig.path34.action.outer_dei = pG3IgrExtraInfo->egr_svlan_dei;
		}
		else if(pFlowPath3->out_cvid_format_act)
		{
			// single tag
			flowConfig.path34.action.vlan_cnt = 1;
			flowConfig.path34.action.outer_tpid = CVLAN_TPID;
			flowConfig.path34.action.outer_vlanid = pFlowPath3->out_cvlan_id;
			flowConfig.path34.action.outer_vlanpri = pFlowPath3->out_cpri;
			flowConfig.path34.action.outer_dei = pG3IgrExtraInfo->egr_cvlan_cfi;
		}
		else
		{
			// un-tag
			flowConfig.path34.action.vlan_cnt = 0;
		}

		if(pFlowPath3->out_smac_trans)
			flowConfig.path34.action.out_smac_trans = TRUE;

		if(pFlowPath3->out_dmac_trans)
		{
			flowConfig.path34.action.out_dmac_trans = TRUE;
			flowConfig.path34.action.mac_da_idx = pG3IgrExtraInfo->lutEgrDaIdx;
		}

		// action: L3
		flowConfig.path34.action.ip_dscp_update_en = pFlowPath3->out_dscp_act;
		flowConfig.path34.action.ip_dscp = pFlowPath3->out_dscp;

		// action: Others
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		if(pFlowPath3->out_change_l4_port_only == 1)
		{
			// change SPORT only
			flowConfig.path34.action.change_l4_port_vld = TRUE;
			flowConfig.path34.action.ip_type = 0; // 0: replace ip_sa/l4_sport according to ip_addr configuration (NAPT); 1: replace ip_da/l4_dport according to ip_addr configuration (NAPTR)
			flowConfig.path34.action.l4_port = pFlowPath3->out_change_l4_port;
			if(pFlowPath3->in_ipv4_or_ipv6==0)
			{
				//IPv4: keep
				flowConfig.path34.action.ip[3] = pFlowPath3->in_src_ipv4_addr;
			}
			else if(pFlowPath3->in_ipv4_or_ipv6==1)
			{
				//IPv6: keep
				flowConfig.path34.action.ip[0] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Saddr.s6_addr[0]);
				flowConfig.path34.action.ip[1] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Saddr.s6_addr[4]);
				flowConfig.path34.action.ip[2] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Saddr.s6_addr[8]);
				flowConfig.path34.action.ip[3] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Saddr.s6_addr[12]);
			}
		}
		else if(pFlowPath3->out_change_l4_port_only == 2)
		{
			// change DPORT only
			flowConfig.path34.action.change_l4_port_vld = TRUE;
			flowConfig.path34.action.ip_type = 1; // 0: replace ip_sa/l4_sport according to ip_addr configuration (NAPT); 1: replace ip_da/l4_dport according to ip_addr configuration (NAPTR)
			flowConfig.path34.action.l4_port = pFlowPath3->out_change_l4_port;
			if(pFlowPath3->in_ipv4_or_ipv6==0)
			{
				//IPv4: keep
				flowConfig.path34.action.ip[3] = pFlowPath3->in_dst_ipv4_addr;
			}
			else if(pFlowPath3->in_ipv4_or_ipv6==1)
			{
				//IPv6: keep
				flowConfig.path34.action.ip[0] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Daddr.s6_addr[0]);
				flowConfig.path34.action.ip[1] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Daddr.s6_addr[4]);
				flowConfig.path34.action.ip[2] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Daddr.s6_addr[8]);
				flowConfig.path34.action.ip[3] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Daddr.s6_addr[12]);
			}
		}
#endif
		flowConfig.path34.action.cos_update_en = pFlowPath3->out_user_pri_act;
		flowConfig.path34.action.cos = pFlowPath3->out_user_priority;
		if(pFlowPath3->out_uc_lut_lookup)
			flowConfig.path34.action.ldpid = flow_dpa;
		else
		{
			if(pFlowPath3->out_portmask)
				flowConfig.path34.action.ldpid = __ffs(pFlowPath3->out_portmask);
			else if(pFlowPath3->out_ext_portmask_idx)
			{
				uint32 macPort, macExtPort;

				// In G3 platform out_ext_portmask_idx is wlanIdMask
				RTK_FC_HELPER_WLAN_DEVID_TO_PORTID(__ffs(fc_db.extPortTbl[pFlowPath3->out_ext_portmask_idx].extPortEnt.extpmask), &macPort, &macExtPort);
				flowConfig.path34.action.ldpid = macPort;
			}
			else
				flowConfig.path34.action.ldpid = flow_dpa;
		}

		// to Wifi
		if((1<<(flowConfig.path34.action.ldpid))&RTK_FC_ALL_MAC_WLANCPU_PORTMASK)
		{
			rtk_fc_wlan_devidx_t wlanDevIdx;
			if(pFlowPath3->out_uc_lut_lookup)
				wlanDevIdx = flow_egrWlanDevIdx;
			else
			{
				if(pFlowPath3->out_ext_portmask_idx)
				{
					// In G3 platform out_ext_portmask_idx is wlanIdMask
					wlanDevIdx = __ffs(fc_db.extPortTbl[pFlowPath3->out_ext_portmask_idx].extPortEnt.extpmask);
				}
				else
					wlanDevIdx = flow_egrWlanDevIdx;
			}
			flowConfig.path34.action.wifi_ssid = wlanDevIdx;

	#if defined(CONFIG_SMP)
			{
				uint16 wifi_tx_dispatch_mode;
				uint8 cpu_idx;
				uint band;
				band = RTK_FC_HELPER_WLAN_DEVID_TO_BANDIDX(wlanDevIdx);
				if(RTK_FC_MGR_DISPATCH_SMP_MODE_WIFI_TX_CPUID_GET(&cpu_idx, &wifi_tx_dispatch_mode, flowIdx>>1, (RTK_FC_MGR_DISPATCH_WLAN0_TX+band)) == SUCCESS)
				{
					if((wifi_tx_dispatch_mode == RTK_FC_DISPATCH_MODE_SMP_BY_HASH) || (wifi_tx_dispatch_mode == RTK_FC_DISPATCH_MODE_SMP_BY_RR))
					{
						flowConfig.path34.action.ldpid = RTK_FC_MAC_PORT_CPU + cpu_idx;
						TRACE("WIFI TX dispatch is SMP mode, decide WIFI TX CPU port ID 0x%x by %s", flowConfig.path34.action.ldpid, (wifi_tx_dispatch_mode == RTK_FC_DISPATCH_MODE_SMP_BY_HASH)?"HASH":"RR");
					}
				}
			}
	#endif
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
#if defined(CONFIG_RTK_FC_WFO_PER_HW_FLOW_MIB)
			if(fc_db.pe_offload_wifi_amsdu_en)
				RTK_RG_ASIC_WIFI_AMSDU_VOQ_SEL(flow, pG3IgrExtraInfo, &flowConfig, TRUE/*wifi TX*/, &flow_mib_tx_info);

			if_wifi_tx = TRUE;
			if(fc_db.wfo_per_flow_mib_init_done && (flowConfig.path34.action.is_amsdu_pe_offload_wifiTx == FALSE))
				if_wifiTx_wo_amsdu = TRUE;
#else
			if(fc_db.pe_offload_wifi_amsdu_en)
				RTK_RG_ASIC_WIFI_AMSDU_VOQ_SEL(flow, pG3IgrExtraInfo, &flowConfig, TRUE/*wifi TX*/, NULL);
#endif
#endif
		}
		flowConfig.path34.action.ingress_intf_idx = _rtk_fc_flow_intf_mapping_idx_get(pFlowPath3->in_intf_idx);
		flowConfig.path34.action.egress_intf_idx = _rtk_fc_flow_intf_mapping_idx_get(pFlowPath3->out_intf_idx);

		flowConfig.path34.action.l3_if_vld1	= TRUE;
		_rtk_fc_g3FlowIdDecision(flow, pG3IgrExtraInfo, &flowIdActMask, &flowId);
		// policer1 is host policing
		if(flowIdActMask)
		{
			flowConfig.path34.action.pol_en = TRUE;
			flowConfig.path34.action.pol_id = flowId;
		}

		// policer_2: flow meter (+ flow mib (flow_meter_mib_conf_dependence enabled))
		if(pFlowPath3->out_share_meter_act)
		{
			flowConfig.path34.action.pol2_en = TRUE;
			flowConfig.path34.action.pol2_id = pFlowPath3->out_share_meter_idx + G3_FLOW_POLICER_IDXSHIFT_FLOWMTR;
		}

		// policer_3: flow mib (flow_meter_mib_conf_dependence disabled)
		if(!fc_db.controlFuc.flow_meter_mib_conf_dependence)
		{
			if(fc_db.controlFuc.flow_mib_mode == RT_STAT_FLOW_MIB_MODE_DEFAULT)
			{
				if(pFlowPath3->out_flow_counter_act)
				{
					flowConfig.path34.action.pol3_en = TRUE;
					flowConfig.path34.action.pol3_id = pFlowPath3->out_flow_counter_idx+ G3_FLOW_POLICER_IDXSHIFT_FLOWMIB;
				}
			}
			else
			{
				//RT_STAT_FLOW_MIB_MODE_EXTRA
				if(pFlowPath3->out_flow_counter_act)
				{
					flowConfig.path34.action.pol2_en = TRUE;
					flowConfig.path34.action.pol2_id = pFlowPath3->out_flow_counter_idx + G3_FLOW_POLICER_IDXSHIFT_FLOWMIB;
				}
			}
			
		}

		if(pFlowPath3->out_flow_counter2_act)
		{
			if(fc_db.controlFuc.flow_mib_mode == RT_STAT_FLOW_MIB_MODE_DEFAULT)
			{
				//RT_STAT_FLOW_MIB_MODE_EXTRA
				WARNING("Not support flow mib2 in RT_STAT_FLOW_MIB_MODE_DEFAULT mode");
				goto ADD_HW_FLOW_FAIL;
			}
			else
			{
				//RT_STAT_FLOW_MIB_MODE_EXTRA
				flowConfig.path34.action.pol3_en = TRUE;
				flowConfig.path34.action.pol3_id = pFlowPath3->out_flow_counter2_idx + G3_FLOW_POLICER_IDXSHIFT_FLOWMIB2;

			}
		}

#if defined(CONFIG_RG_G3_WAN_PORT_INDEX)&&(CONFIG_RG_G3_WAN_PORT_INDEX==7)
		if(pFlowPath3->out_stream_idx_act && fc_db.streamidTbl[pFlowPath3->out_stream_idx].valid &&
			(fc_db.wanPortMask.portmask & (1 << flowConfig.path34.action.ldpid)))
		{
			flowConfig.path34.action.gemId_mapping_mode = RTK_GEM_MAPPING_MODE;
			flowConfig.path34.action.gem_id = fc_db.streamidTbl[pFlowPath3->out_stream_idx].gemid;
			flowConfig.path34.action.tcont_id = fc_db.streamidTbl[pFlowPath3->out_stream_idx].ldpid&0x1f;
			if(fc_db.streamidTbl[pFlowPath3->out_stream_idx].cos)
			{
				flowConfig.path34.action.cos_update_en = TRUE;
				flowConfig.path34.action.cos = fc_db.streamidTbl[pFlowPath3->out_stream_idx].cos;
			}
			else
				flowConfig.path34.action.cos_update_en = FALSE;// always disable cos update to prevent packet is sent to invalid PON queue
		}
#endif
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
		if(((1U << fc_db.lutTbl[pG3IgrExtraInfo->lutIgrSaIdx]->spa) & fc_db.controlFuc.amsdu_pe_dbg_mode_rx_portmask) && 
			((1U << fc_db.lutTbl[pG3IgrExtraInfo->lutEgrDaIdx]->spa) & fc_db.controlFuc.amsdu_pe_dbg_mode_tx_portmask))
		{
			if(flowConfig.path34.action.gemId_mapping_mode == RTK_GEM_MAPPING_MODE)
			{
				if((1U << flowConfig.path34.action.tcont_id) & fc_db.controlFuc.amsdu_pe_dbg_mode_tx_tcontIdmask)
					RTK_RG_ASIC_AMSDU_DBG_MODE_VOQ_SEL(flowConfig.path34.action.tcont_id+0x20, flowConfig.path34.action.cos_update_en?flowConfig.path34.action.cos:0, flowConfig.path34.action.gem_id, &flowConfig);
			}
			else
				RTK_RG_ASIC_AMSDU_DBG_MODE_VOQ_SEL(flowConfig.path34.action.ldpid, flowConfig.path34.action.cos_update_en?flowConfig.path34.action.cos:0, 0, &flowConfig);
		}
#endif
#if defined(CONFIG_RTK_FC_WFO_PER_HW_FLOW_MIB)
		if((if_wifi_tx == FALSE) && if_wifi_rx)
		{
			// should be done after ldpid/stream_id decision
			if(fc_db.pe_offload_wifi_amsdu_en)
				RTK_RG_ASIC_WIFI_AMSDU_VOQ_SEL(flow, pG3IgrExtraInfo, &flowConfig, FALSE/*wifi RX*/, &flow_mib_rx_info);
			if(fc_db.wfo_per_flow_mib_init_done && (flowConfig.path34.action.is_amsdu_pe_offload_wifiRx == FALSE))
				if_wifiRx_wo_amsdu = TRUE;
			if(flowConfig.path34.action.is_amsdu_pe_offload_wifiRx == TRUE)
				flowConfig.path34.action.gemId_mapping_mode = 0; // disable gem mapping mode for upstream
		}
#endif

		if(pG3IgrExtraInfo->dualHdrType == RTK_FC_DUALTYPE_VXLAN)
		{
			if(pG3IgrExtraInfo->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM)
			{
				//DEBUG("VXLAN upstream");
				flowConfig.path34.action.vxlan_sport_update_en = TRUE;
				flowConfig.path34.action.vxlan_sport = pG3IgrExtraInfo->vxlan_info.outer_srcPort;
			}
		}

		if(pFlowPath3->sw_shaper_en) {
			if(hybridFwd) {
				// redirect to main cpu instead of original dest port
				flowConfig.path34.action.ldpid = RTK_FC_MAC_PORT_MAINCPU;
				flowConfig.path34.action.wifi_ssid = RTK_FC_SWID_HW_SW_HYBRID_FWD;
				flowConfig.path34.action.pol_en = 0;
				flowConfig.path34.action.pol2_en = 0;
				flowConfig.path34.action.pol3_en = 0;
				flowConfig.path34.action.gemId_mapping_mode = 0;
#if defined(CONFIG_FC_RTL9607F_SERIES)
				if(flowConfig.path34.action.l2format_act_vld == TRUE &&
					flowConfig.path34.action.l2format_act == RTK_ASIC_L2_FORMAT_ACT_SNAP &&
					!fc_db.controlFuc.hw_sw_hybrid_fwd_to_pe) {	// to ARM wifi driver, disable snap translation
					flowConfig.path34.action.l2format_act_vld = FALSE;
					flowConfig.path34.action.l2format_act = RTK_ASIC_L2_FORMAT_ACT_ETHERNET;
				}
#endif
			}else
				flowConfig.path34.action.sw_shaper_en = pFlowPath3->sw_shaper_en;
		}
		if(fc_db.controlFuc.bridge_5tuple_flow_keep_mac_addr_mode)
		{
			flowConfig.path34.action.smac_keep = TRUE;
			flowConfig.path34.action.out_dmac_trans = TRUE;
			flowConfig.path34.action.mac_da_idx = pG3IgrExtraInfo->lutEgrDaIdx;
		}

#if defined(CONFIG_RTK_FC_PKT_QUEUE_OFFLOAD_BY_PE)
		if(fc_db.controlFuc.pe_offload_pkt_queue_portmask)
		{
			int32 do_pe_offload=0, pe_cpu_port, pe_queue_idx=0, connection_idx=FAIL;
			if((fc_db.controlFuc.pe_offload_pkt_queue_portmask & (0x1<<flowConfig.path34.key.orig_lspid))
				&& (fc_db.wanPortMask.portmask & (0x1<<flowConfig.path34.action.ldpid)))
			{
				if(fc_db.pe_queue_tc_mode==0)
				{
					TRACE("[PE QUEUE] Offload upstream lspid %d and pon streamId %d to PE", flowConfig.path34.key.orig_lspid, pFlowPath3->out_stream_idx);
					pe_cpu_port = RT_PE_QUEUE_TEST_CPU_PORT;
					do_pe_offload = 1;
				}
				else if(fc_db.pe_queue_tc_mode_only_for_small_pkt==0 || pG3IgrExtraInfo->pe_tc_queue_flow_lspid!=FAIL) //tc mode
				{
					if(fc_db.pe_tc_queue_pe_used_num==MAX_PE_TC_QUEUE_TEST_PE_USED_NUM)
					{
						pe_queue_idx = (fc_db.pe_queue_tc_mode_slave_portmask & (0x1<<flowConfig.path34.key.orig_lspid)) ? (fc_db.pe_tc_queue_main_queue_idx^0x1) : fc_db.pe_tc_queue_main_queue_idx ;
					}
					pe_cpu_port = (pe_queue_idx==0) ? RT_PE_TC_QUEUE_0_TEST_CPU_PORT : RT_PE_TC_QUEUE_1_TEST_CPU_PORT ;

					connection_idx = rtk_fc_pe_tc_queue_test_free_connection_index_get(pe_queue_idx, flowIdx);
					if(connection_idx != FAIL)
					{
						memset(&pe_tc_queue_test_req, 0, sizeof(rt_pe_tc_queue_test_request_t));
						pe_tc_queue_test_req.req_cmd = RT_PE_TC_QUEUE_TEST_CMD_CONNECTION_ADD;
						pe_tc_queue_test_req.conn_info.connection_idx = connection_idx;
						pe_tc_queue_test_req.conn_info.ipv4_en	= (flowConfig.path34.key.ipv4_or_ipv6) ? 0 : 1;
						pe_tc_queue_test_req.conn_info.ipv6_en	= (flowConfig.path34.key.ipv4_or_ipv6) ? 1 : 0;
						pe_tc_queue_test_req.conn_info.tcp_en	= (flowConfig.path34.key.l4proto_num==IPPROTO_TCP) ? 1 : 0;
						pe_tc_queue_test_req.conn_info.udp_en	= (flowConfig.path34.key.l4proto_num==IPPROTO_UDP) ? 1 : 0;
						pe_tc_queue_test_req.conn_info.lspid	= RTK_FC_PE_TC_QUEUE_TEST_DMA_LSO_LSPID;
						pe_tc_queue_test_req.conn_info.pol_grp_id	= 0;
						pe_tc_queue_test_req.conn_info.fe_bypass 	= 1;
						if(pFlowPath3->out_stream_idx_act && fc_db.streamidTbl[pFlowPath3->out_stream_idx].valid)
						{
							pe_tc_queue_test_req.conn_info.ldpid	= fc_db.streamidTbl[pFlowPath3->out_stream_idx].ldpid;
							pe_tc_queue_test_req.conn_info.cos		= fc_db.streamidTbl[pFlowPath3->out_stream_idx].cos;
							pe_tc_queue_test_req.conn_info.flowid	= fc_db.streamidTbl[pFlowPath3->out_stream_idx].gemid;
						}
						else
						{
							pe_tc_queue_test_req.conn_info.ldpid	= flowConfig.path34.action.ldpid;
						}
						if(pG3IgrExtraInfo->dualHdrType==RTK_FC_DUALTYPE_NONE)
						{
							TRACE("[PE TC QUEUE][Single] Offload upstream lspid %d and pon streamId %d to PE cpu port 0x%x", flowConfig.path34.key.orig_lspid, pFlowPath3->out_stream_idx, pe_cpu_port);
							do_pe_offload = 1;
							
						}
						else //dual
						{
							if(fc_db.netifTbl[NETIF_HWTOSW(pFlowPath3->out_intf_idx)].extra_available && fc_db.netifTbl[NETIF_HWTOSW(pFlowPath3->out_intf_idx)].hwIdx_for_pe!=SIGNED_INVALID)
							{
								TRACE("[PE TC QUEUE][Dual] Offload upstream lspid %d and pon streamId %d to PE cpu port 0x%x", flowConfig.path34.key.orig_lspid, pFlowPath3->out_stream_idx, pe_cpu_port);
								do_pe_offload = 1;
								pe_tc_queue_test_req.conn_info.lspid	= RTK_FC_DUAL_EXTRA_HWLOOKUP_LSPID_BASE + pG3IgrExtraInfo->dualHdrType;
								pe_tc_queue_test_req.conn_info.pol_grp_id	= fc_db.netifTbl[NETIF_HWTOSW(pFlowPath3->out_intf_idx)].extra_netifId_map;
								pe_tc_queue_test_req.conn_info.ldpid	= RT_PE_TC_QUEUE_TEST_HWLOOKUP_LDPID;
								pe_tc_queue_test_req.conn_info.fe_bypass 	= 0;
								//change egr_intf to single
								flowConfig.path34.action.egress_intf_idx = fc_db.netifTbl[NETIF_HWTOSW(pFlowPath3->out_intf_idx)].hwIdx_for_pe;
							}
							else
							{
								TRACE("[PE TC QUEUE][Dual] Fail to redirect to PE ... due to this dualHdrType %d does not support extra_available to do HWLOOKUP !!", pG3IgrExtraInfo->dualHdrType);
							}
						}
					}
				}
			}
			if(do_pe_offload)
			{
				flowConfig.path34.action.ldpid = pe_cpu_port;
				flowConfig.path34.action.gemId_mapping_mode = 0;
				flowConfig.path34.action.cos_update_en = TRUE;
				flowConfig.path34.action.cos = 0;
				if(fc_db.pe_queue_tc_mode==0)
				{
					flowConfig.path34.action.wifi_ssid = pFlowPath3->out_stream_idx + RT_PE_QUEUE_TEST_SW_ID_BASE;
				}
				else
				{
					if(pG3IgrExtraInfo->pe_tc_queue_flow_lspid != FAIL)
					{
						//for rsv cls
						pe_tc_queue_test_rsv_cls_info.lspid = flowConfig.path34.key.orig_lspid;
						memcpy(pe_tc_queue_test_rsv_cls_info.smac, flowConfig.path34.key.src_mac, ETH_ALEN);
						memcpy(pe_tc_queue_test_rsv_cls_info.dmac, flowConfig.path34.key.dst_mac, ETH_ALEN);
						if(pFlowPath3->in_ipv4_or_ipv6==0)
						{
							pe_tc_queue_test_rsv_cls_info.isIPv4OrIpv6 = 0;
							pe_tc_queue_test_rsv_cls_info.ipv4_dip_addr = pFlowPath3->in_dst_ipv4_addr;
						}
						else
						{
							pe_tc_queue_test_rsv_cls_info.isIPv4OrIpv6 = 1;
							memcpy(&pe_tc_queue_test_rsv_cls_info.ipv6_dip_addr, &pG3IgrExtraInfo->igr_ipv6_dip, sizeof(struct in6_addr));
						}
						pe_tc_queue_test_rsv_cls_info.action_lspid = pG3IgrExtraInfo->pe_tc_queue_flow_lspid;
						//for redirect flow key
						flowConfig.path34.key.orig_lspid = pG3IgrExtraInfo->pe_tc_queue_flow_lspid;	
					}
					flowConfig.path34.action.wifi_ssid = connection_idx + RT_PE_TC_QUEUE_TEST_SW_ID_BASE;
					pe_tc_queue_idx = pe_queue_idx;
					flow->pe_tc_queue_valid = 1; 
					flow->pe_tc_queue_idx = pe_tc_queue_idx; 
					flow->pe_tc_queue_connection_idx = connection_idx;
				}
			}
		}
#endif
#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)
		if(pG3IgrExtraInfo->ipsec_pe_offload ==1)
		{
			// un-tag
			flowConfig.path34.action.vlan_cnt = 0;
			flowConfig.path34.action.wifi_ssid = fc_db.fc_ipsec_info[pG3IgrExtraInfo->ipsec_hook_table_index].pe_crypto_sw_id;
			flowConfig.path34.action.gemId_mapping_mode = 0;
			flowConfig.path34.action.ldpid = RT_PE_IPSEC_CPU_PORT;
			flowConfig.path34.action.cos_update_en = TRUE;
			flowConfig.path34.action.cos = 0;
			if(fc_db.fc_ipsec_info[pG3IgrExtraInfo->ipsec_hook_table_index].direction==RTK_FC_IPSEC_DIR_ENCRYT)
			{
				flowConfig.path34.action.wifi_ssid |= ((pFlowPath3->in_ipv4_or_ipv6) ? RT_PE_IPSEC_SW_ID_INNER_IPV6_BIT : 0x0 );
			}
		}
#endif		
#if defined(CONFIG_RTK_FC_SRV6_OFFLOAD_BY_PE)
		if(pG3IgrExtraInfo->dualHdrType == RTK_FC_DUALTYPE_SRV6 && pG3IgrExtraInfo->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM && pG3IgrExtraInfo->srv6_connection_idx!=FAIL)
		{
			flowConfig.path34.action.wifi_ssid = (pG3IgrExtraInfo->srv6_connection_idx + RT_PE_SRV6_SW_ID_CONNECTION_IDX_BASE);
			flowConfig.path34.action.ldpid = RT_PE_SRV6_CPU_PORT;
			flowConfig.path34.action.gemId_mapping_mode = 0;
			flowConfig.path34.action.cos_update_en = TRUE;
			flowConfig.path34.action.cos = 0;
		}
#endif
	}
	else if(flow->in_path == FB_PATH_12)
	{
		rtk_rg_asic_path1_entry_t *pFlowPath1 = (rtk_rg_asic_path1_entry_t *)flow;
		flowConfig.in_path = FB_PATH_12;

		/* === key === */
		// key: L2
		flowConfig.path12.key.orig_lspid = pFlowPath1->in_spa;
		if(pFlowPath1->in_stagif)
		{
			flowConfig.path12.key.stag_if = TRUE;
			flowConfig.path12.key.svlan_tpid = fc_db.systemGlobal.stagTPID[pG3IgrExtraInfo->igr_svlan_tpid_sel];
			flowConfig.path12.key.svlan_id = pG3IgrExtraInfo->svlan_id;
			flowConfig.path12.key.svlan_pri = pG3IgrExtraInfo->svlan_pri;
			flowConfig.path12.key.svlan_dei = pG3IgrExtraInfo->igr_svlan_dei;
		}
		if(pFlowPath1->in_ctagif)
		{
			flowConfig.path12.key.ctag_if = TRUE;
			flowConfig.path12.key.cvlan_tpid = CVLAN_TPID;
			flowConfig.path12.key.cvlan_id = pG3IgrExtraInfo->cvlan_id;
			flowConfig.path12.key.cvlan_pri = pFlowPath1->in_cvlan_pri;
			flowConfig.path12.key.cvlan_cfi = pG3IgrExtraInfo->igr_cvlan_cfi;
		}

		if(fc_db.lutTbl[pFlowPath1->in_smac_lut_idx]==NULL)
		{
			WARNING("lutTbl[%d] is NULL!", pFlowPath1->in_smac_lut_idx);
			goto ADD_HW_FLOW_FAIL;
		}
		else
			memcpy(&flowConfig.path12.key.src_mac[0], &fc_db.lutTbl[pFlowPath1->in_smac_lut_idx]->l2Addr, ETH_ALEN);

		if(fc_db.lutTbl[pFlowPath1->in_dmac_lut_idx]==NULL)
		{
			WARNING("lutTbl[%d] is NULL!", pFlowPath1->in_smac_lut_idx);
			goto ADD_HW_FLOW_FAIL;
		}
		else
			memcpy(&flowConfig.path12.key.dst_mac[0], &fc_db.lutTbl[pFlowPath1->in_dmac_lut_idx]->l2Addr, ETH_ALEN);

#if defined(CONFIG_FC_RTL8277C_SERIES)
		flowConfig.path12.key.len_encoded = pFlowPath1->in_len_encoded;
#endif
		if(fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH12_PROTOCOL])
		{
#if defined(CONFIG_FC_RTL8277C_SERIES)
			flowConfig.path12.key.ethertype = (pFlowPath1->in_ethertype_msb << 8) | (pFlowPath1->in_ethertype_lsb7_4 << 4) | pFlowPath1->in_ethertype_lsb3_0;
#else
			flowConfig.path12.key.ethertype = pFlowPath1->in_ethertype;
#endif
		}

		// key: L3
		flowConfig.path12.key.ip_vld = pFlowPath1->in_iphdrif;
		if(pFlowPath1->in_tos_check)
		{
			flowConfig.path12.key.ip_dscp = (pFlowPath1->in_tos >>2)&0x3f;
			flowConfig.path12.key.ip_ecn = pFlowPath1->in_tos&0x3;
		}

		// from PON
		if(flow_spa == RTK_FC_MAC_PORT_PON)
		{
			flowConfig.path12.key.is_from_ponRx_wifi_rx =  RTK_ASIC_FLOW_FROM_PON;
			flowConfig.path12.key.pon_streamId_or_wifi_devIdx = pG3IgrExtraInfo->pon_stream_id; // WAN rx stream id or wifi RX SSID
		}
#if defined(CONFIG_RTK_FC_FORWARDING_AWARE_LAN_SWID)
		if(fc_db.lutTbl[pG3IgrExtraInfo->lutIgrSaIdx]->spa < RTK_FC_MAC_PORT_PON) {
			flowConfig.path12.key.is_from_ponRx_wifi_rx =  RTK_ASIC_FLOW_FROM_PON;
			flowConfig.path12.key.pon_streamId_or_wifi_devIdx = pG3IgrExtraInfo->pon_stream_id;	// LAN rx swid(extspa)
		}
#endif
		// from wifi
		if((1 << flow_spa) & RTK_FC_ALL_MAC_WLANCPU_PORTMASK)
		{
			flowConfig.path12.key.is_from_ponRx_wifi_rx =  RTK_ASIC_FLOW_FROM_WIFI;
			flowConfig.path12.key.pon_streamId_or_wifi_devIdx = flow_igrWlanDevIdx; // WAN rx stream id or wifi RX SSID
#if defined(CONFIG_FC_RTL9607F_SERIES) && (defined(CONFIG_RTK_NIC_HWLOOKUP_SNAP_TRANSFORM_BY_HW) || defined(CONFIG_RTK_FC_WIFI_DRIVER_OFFLOAD_BY_PE))
			/* wifi RX: DMA LSO to splict AMSDU packet and HW hash to tranfer packet from SNAP to ETH*/
			flowConfig.path12.action.l2format_act_vld = TRUE;
			flowConfig.path12.action.l2format_act = RTK_ASIC_L2_FORMAT_ACT_ETHERNET;
#endif
#if defined(CONFIG_RTK_FC_WFO_PER_HW_FLOW_MIB)
			if_wifi_rx = TRUE;
#endif
		}

		/* === action === */
		// action: L2
		flowConfig.path12.action.vlan_act = TRUE;
		if((pFlowPath1->out_svid_format_act + pFlowPath1->out_cvid_format_act) == 2)
		{
			// double tag
			flowConfig.path12.action.vlan_cnt = 2;
			flowConfig.path12.action.outer_tpid = outterTPID;
			flowConfig.path12.action.outer_vlanid = pFlowPath1->out_svlan_id;
			flowConfig.path12.action.outer_vlanpri = pFlowPath1->out_spri;
			flowConfig.path12.action.outer_dei = pG3IgrExtraInfo->egr_svlan_dei;
			flowConfig.path12.action.inner_tpid = CVLAN_TPID;
			flowConfig.path12.action.inner_vlanid = pFlowPath1->out_cvlan_id;
			flowConfig.path12.action.inner_vlanpri = pFlowPath1->out_cpri;
			flowConfig.path12.action.inner_dei = pG3IgrExtraInfo->egr_cvlan_cfi;
		}
		else if(pFlowPath1->out_svid_format_act)
		{
			// single tag
			flowConfig.path12.action.vlan_cnt = 1;
			flowConfig.path12.action.outer_tpid = outterTPID;
			flowConfig.path12.action.outer_vlanid = pFlowPath1->out_svlan_id;
			flowConfig.path12.action.outer_vlanpri = pFlowPath1->out_spri;
			flowConfig.path12.action.outer_dei = pG3IgrExtraInfo->egr_svlan_dei;
		}
		else if(pFlowPath1->out_cvid_format_act)
		{
			// single tag
			flowConfig.path12.action.vlan_cnt = 1;
			flowConfig.path12.action.outer_tpid = CVLAN_TPID;
			flowConfig.path12.action.outer_vlanid = pFlowPath1->out_cvlan_id;
			flowConfig.path12.action.outer_vlanpri = pFlowPath1->out_cpri;
			flowConfig.path12.action.outer_dei = pG3IgrExtraInfo->egr_cvlan_cfi;
		}
		else
		{
			// un-tag
			flowConfig.path12.action.vlan_cnt = 0;
		}

		if(pFlowPath1->out_smac_trans)
			flowConfig.path12.action.out_smac_trans = TRUE;

		if(pFlowPath1->out_dmac_trans)
		{
			flowConfig.path12.action.out_dmac_trans = TRUE;
			flowConfig.path12.action.mac_da_idx = pG3IgrExtraInfo->lutEgrDaIdx;
		}

		// action: L3
		flowConfig.path12.action.ip_dscp_update_en = pFlowPath1->out_dscp_act;
		flowConfig.path12.action.ip_dscp = pFlowPath1->out_dscp;

		// action: Others
		flowConfig.path12.action.cos_update_en = pFlowPath1->out_user_pri_act;
		flowConfig.path12.action.cos = pFlowPath1->out_user_priority;
		if(pFlowPath1->out_uc_lut_lookup)
			flowConfig.path12.action.ldpid = flow_dpa;
		else
		{
			if(pFlowPath1->out_portmask)
				flowConfig.path12.action.ldpid = __ffs(pFlowPath1->out_portmask);
			else if(pFlowPath1->out_ext_portmask_idx)
			{
				uint32 macPort, macExtPort;

				// In G3 platform out_ext_portmask_idx is wlanIdMask
				RTK_FC_HELPER_WLAN_DEVID_TO_PORTID(__ffs(fc_db.extPortTbl[pFlowPath1->out_ext_portmask_idx].extPortEnt.extpmask), &macPort, &macExtPort);
				flowConfig.path12.action.ldpid = macPort;
			}
			else
				flowConfig.path12.action.ldpid = flow_dpa;
		}

		// to Wifi
		if((1<<(flowConfig.path12.action.ldpid))&RTK_FC_ALL_MAC_WLANCPU_PORTMASK)
		{
			rtk_fc_wlan_devidx_t wlanDevIdx;
			if(pFlowPath1->out_uc_lut_lookup)
				wlanDevIdx = flow_egrWlanDevIdx;
			else
			{
				if(pFlowPath1->out_ext_portmask_idx)
				{
					// In G3 platform out_ext_portmask_idx is wlanIdMask
					wlanDevIdx = __ffs(fc_db.extPortTbl[pFlowPath1->out_ext_portmask_idx].extPortEnt.extpmask);
				}
				else
					wlanDevIdx = flow_egrWlanDevIdx;
			}
			flowConfig.path12.action.wifi_ssid = wlanDevIdx;
#if defined(CONFIG_SMP)
			{
				uint16 wifi_tx_dispatch_mode;
				uint8 cpu_idx;
				uint band;
				band = RTK_FC_HELPER_WLAN_DEVID_TO_BANDIDX(wlanDevIdx);
				if(RTK_FC_MGR_DISPATCH_SMP_MODE_WIFI_TX_CPUID_GET(&cpu_idx, &wifi_tx_dispatch_mode, flowIdx>>1, (RTK_FC_MGR_DISPATCH_WLAN0_TX+band)) == SUCCESS)
				{
					if((wifi_tx_dispatch_mode == RTK_FC_DISPATCH_MODE_SMP_BY_HASH) || (wifi_tx_dispatch_mode == RTK_FC_DISPATCH_MODE_SMP_BY_RR))
					{
						flowConfig.path12.action.ldpid = RTK_FC_MAC_PORT_CPU + cpu_idx;
						TRACE("WIFI TX dispatch is SMP mode, decide WIFI TX CPU port ID 0x%x by %s", flowConfig.path12.action.ldpid, (wifi_tx_dispatch_mode == RTK_FC_DISPATCH_MODE_SMP_BY_HASH)?"HASH":"RR");
					}
				}
			}

#endif
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
#if defined(CONFIG_RTK_FC_WFO_PER_HW_FLOW_MIB)
			if(fc_db.pe_offload_wifi_amsdu_en)
				RTK_RG_ASIC_WIFI_AMSDU_VOQ_SEL(flow, pG3IgrExtraInfo, &flowConfig, TRUE/*wifi TX*/, &flow_mib_tx_info);

			if_wifi_tx = TRUE;
			if(fc_db.wfo_per_flow_mib_init_done && (flowConfig.path12.action.is_amsdu_pe_offload_wifiTx == FALSE))
				if_wifiTx_wo_amsdu = TRUE;
#else
			if(fc_db.pe_offload_wifi_amsdu_en)
				RTK_RG_ASIC_WIFI_AMSDU_VOQ_SEL(flow, pG3IgrExtraInfo, &flowConfig, TRUE/*wifi TX*/, NULL);
#endif
#endif
		}
		flowConfig.path12.action.ingress_intf_idx = _rtk_fc_flow_intf_mapping_idx_get(pFlowPath1->in_intf_idx);
		flowConfig.path12.action.egress_intf_idx = _rtk_fc_flow_intf_mapping_idx_get(pFlowPath1->out_intf_idx);


		flowConfig.path12.action.l3_if_vld1	= TRUE;
		_rtk_fc_g3FlowIdDecision(flow, pG3IgrExtraInfo, &flowIdActMask, &flowId);
		// policer1 is host policing
		if(flowIdActMask)
		{
			flowConfig.path12.action.pol_en = TRUE;
			flowConfig.path12.action.pol_id = flowId;
		}

		// policer_2: flow meter (+ flow mib (flow_meter_mib_conf_dependence enabled))
		if(pFlowPath1->out_share_meter_act)
		{
			flowConfig.path12.action.pol2_en = TRUE;
			flowConfig.path12.action.pol2_id = pFlowPath1->out_share_meter_idx + G3_FLOW_POLICER_IDXSHIFT_FLOWMTR;
		}

		// policer_3: flow mib (flow_meter_mib_conf_dependence disabled)
		if(!fc_db.controlFuc.flow_meter_mib_conf_dependence)
		{
			if(fc_db.controlFuc.flow_mib_mode == RT_STAT_FLOW_MIB_MODE_DEFAULT)
			{
				if(pFlowPath1->out_flow_counter_act)
				{
					flowConfig.path12.action.pol3_en = TRUE;
					flowConfig.path12.action.pol3_id = pFlowPath1->out_flow_counter_idx + G3_FLOW_POLICER_IDXSHIFT_FLOWMIB;
				}
			}
			else
			{
				//RT_STAT_FLOW_MIB_MODE_EXTRA
				if(pFlowPath1->out_flow_counter_act)
				{
					flowConfig.path12.action.pol2_en = TRUE;
					flowConfig.path12.action.pol2_id = pFlowPath1->out_flow_counter_idx + G3_FLOW_POLICER_IDXSHIFT_FLOWMIB;
				}
			}
		}

		if(pFlowPath1->out_flow_counter2_act)
		{
			if(fc_db.controlFuc.flow_mib_mode == RT_STAT_FLOW_MIB_MODE_DEFAULT)
			{
				//RT_STAT_FLOW_MIB_MODE_EXTRA
				WARNING("Not support flow mib2 in RT_STAT_FLOW_MIB_MODE_DEFAULT mode");
				goto ADD_HW_FLOW_FAIL;
			}
			else
			{
				//RT_STAT_FLOW_MIB_MODE_EXTRA
				flowConfig.path12.action.pol3_en = TRUE;
				flowConfig.path12.action.pol3_id = pFlowPath1->out_flow_counter2_idx + G3_FLOW_POLICER_IDXSHIFT_FLOWMIB2;

			}
		}

		if(pG3IgrExtraInfo->dualHdrType == RTK_FC_DUALTYPE_VXLAN)
		{
			if(pG3IgrExtraInfo->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM)
			{
				DEBUG("VXLAN path1 upstream");	
				flowConfig.path12.action.vxlan_sport_update_en = TRUE;
				flowConfig.path12.action.vxlan_sport = pG3IgrExtraInfo->vxlan_info.outer_srcPort;
				
			}
		}

		if(pFlowPath1->sw_shaper_en) {
			if(hybridFwd) {
				// redirect to main cpu instead of original dest port
				flowConfig.path12.action.ldpid = RTK_FC_MAC_PORT_MAINCPU;
				flowConfig.path12.action.wifi_ssid = RTK_FC_SWID_HW_SW_HYBRID_FWD;
				flowConfig.path12.action.pol_en = 0;
				flowConfig.path12.action.pol2_en = 0;
				flowConfig.path12.action.pol3_en = 0;
				flowConfig.path12.action.gemId_mapping_mode = 0;
#if defined(CONFIG_FC_RTL9607F_SERIES)
				if(flowConfig.path12.action.l2format_act_vld == TRUE &&
					flowConfig.path12.action.l2format_act == RTK_ASIC_L2_FORMAT_ACT_SNAP &&
					!fc_db.controlFuc.hw_sw_hybrid_fwd_to_pe) {	// to ARM wifi driver, disable snap translation
					flowConfig.path12.action.l2format_act_vld = FALSE;
					flowConfig.path12.action.l2format_act = RTK_ASIC_L2_FORMAT_ACT_ETHERNET;
				}
#endif
			}else
				flowConfig.path12.action.sw_shaper_en = pFlowPath1->sw_shaper_en;
		}

#if defined(CONFIG_RG_G3_WAN_PORT_INDEX)&&(CONFIG_RG_G3_WAN_PORT_INDEX==7)
		if(((pFlowPath1->in_spa != RTK_FC_MAC_PORT_PON) && pFlowPath1->in_out_stream_idx_check_act) && fc_db.streamidTbl[pFlowPath1->in_out_stream_idx].valid &&
			(fc_db.wanPortMask.portmask & (1 << flowConfig.path12.action.ldpid)))
		{
			flowConfig.path12.action.gemId_mapping_mode = RTK_GEM_MAPPING_MODE;
			flowConfig.path12.action.gem_id = fc_db.streamidTbl[pFlowPath1->in_out_stream_idx].gemid;
			flowConfig.path12.action.tcont_id = fc_db.streamidTbl[pFlowPath1->in_out_stream_idx].ldpid&0x1f;
			if(fc_db.streamidTbl[pFlowPath1->in_out_stream_idx].cos)
			{
				flowConfig.path12.action.cos_update_en = TRUE;
				flowConfig.path12.action.cos = fc_db.streamidTbl[pFlowPath1->in_out_stream_idx].cos;
			}
			else
				flowConfig.path12.action.cos_update_en = FALSE;// always disable cos update to prevent packet is sent to invalid PON queue
		}
#endif
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
		if(((1U << fc_db.lutTbl[pG3IgrExtraInfo->lutIgrSaIdx]->spa) & fc_db.controlFuc.amsdu_pe_dbg_mode_rx_portmask) && 
			((1U << fc_db.lutTbl[pG3IgrExtraInfo->lutEgrDaIdx]->spa) & fc_db.controlFuc.amsdu_pe_dbg_mode_tx_portmask))
		{
			if(flowConfig.path12.action.gemId_mapping_mode == RTK_GEM_MAPPING_MODE)
			{
				if((1U << flowConfig.path12.action.tcont_id) & fc_db.controlFuc.amsdu_pe_dbg_mode_tx_tcontIdmask)
					RTK_RG_ASIC_AMSDU_DBG_MODE_VOQ_SEL(flowConfig.path12.action.tcont_id+0x20, flowConfig.path12.action.cos_update_en?flowConfig.path12.action.cos:0, flowConfig.path12.action.gem_id, &flowConfig);
			}
			else
				RTK_RG_ASIC_AMSDU_DBG_MODE_VOQ_SEL(flowConfig.path12.action.ldpid, flowConfig.path12.action.cos_update_en?flowConfig.path12.action.cos:0, 0, &flowConfig);
		}
#endif
#if defined(CONFIG_RTK_FC_WFO_PER_HW_FLOW_MIB)
		if((if_wifi_tx == FALSE) && if_wifi_rx)
		{
			// should be done after ldpid/stream_id decision
			if(fc_db.pe_offload_wifi_amsdu_en)
				RTK_RG_ASIC_WIFI_AMSDU_VOQ_SEL(flow, pG3IgrExtraInfo, &flowConfig, FALSE/*wifi RX*/, &flow_mib_rx_info);
			if(fc_db.wfo_per_flow_mib_init_done && (flowConfig.path12.action.is_amsdu_pe_offload_wifiRx == FALSE))
				if_wifiRx_wo_amsdu = TRUE;
			if(flowConfig.path12.action.is_amsdu_pe_offload_wifiRx == TRUE)
				flowConfig.path12.action.gemId_mapping_mode = 0; // disable gem mapping mode for upstream
		}
#endif
#if defined(CONFIG_RTK_FC_PKT_QUEUE_OFFLOAD_BY_PE)
		if(fc_db.controlFuc.pe_offload_pkt_queue_portmask)
		{
			int32 do_pe_offload=0, pe_cpu_port, pe_queue_idx=0, connection_idx=FAIL;
			if((fc_db.controlFuc.pe_offload_pkt_queue_portmask & (0x1<<flowConfig.path12.key.orig_lspid))
				&& (fc_db.wanPortMask.portmask & (0x1<<flowConfig.path12.action.ldpid)))
			{
				if(fc_db.pe_queue_tc_mode==0)
				{
					TRACE("[PE QUEUE] Offload upstream lspid %d and pon streamId %d to PE", flowConfig.path12.key.orig_lspid, pFlowPath1->in_out_stream_idx);
					pe_cpu_port = RT_PE_QUEUE_TEST_CPU_PORT;
					do_pe_offload = 1;
				}
				else if(fc_db.pe_queue_tc_mode_only_for_small_pkt==0 || pG3IgrExtraInfo->pe_tc_queue_flow_lspid!=FAIL) //tc mode
				{
					if(fc_db.pe_tc_queue_pe_used_num==MAX_PE_TC_QUEUE_TEST_PE_USED_NUM)
					{
						pe_queue_idx = (fc_db.pe_queue_tc_mode_slave_portmask & (0x1<<flowConfig.path12.key.orig_lspid)) ? (fc_db.pe_tc_queue_main_queue_idx^0x1) : fc_db.pe_tc_queue_main_queue_idx ;
					}
					pe_cpu_port = (pe_queue_idx==0) ? RT_PE_TC_QUEUE_0_TEST_CPU_PORT : RT_PE_TC_QUEUE_1_TEST_CPU_PORT ;

					connection_idx = rtk_fc_pe_tc_queue_test_free_connection_index_get(pe_queue_idx, flowIdx);
					if(connection_idx != FAIL)
					{
						memset(&pe_tc_queue_test_req, 0, sizeof(rt_pe_tc_queue_test_request_t));
						pe_tc_queue_test_req.req_cmd = RT_PE_TC_QUEUE_TEST_CMD_CONNECTION_ADD;
						pe_tc_queue_test_req.conn_info.connection_idx = connection_idx;
						pe_tc_queue_test_req.conn_info.ipv4_en 	= pG3IgrExtraInfo->igr_ipv4_valid;
						pe_tc_queue_test_req.conn_info.ipv6_en 	= pG3IgrExtraInfo->igr_ipv6_valid;
						pe_tc_queue_test_req.conn_info.tcp_en 	= pG3IgrExtraInfo->igr_tcp_valid;
						pe_tc_queue_test_req.conn_info.udp_en 	= pG3IgrExtraInfo->igr_udp_valid;
						pe_tc_queue_test_req.conn_info.lspid 	= RTK_FC_PE_TC_QUEUE_TEST_DMA_LSO_LSPID;
						pe_tc_queue_test_req.conn_info.pol_grp_id 	= 0;
						pe_tc_queue_test_req.conn_info.fe_bypass 	= 1;
						if(pFlowPath1->in_out_stream_idx_check_act && fc_db.streamidTbl[pFlowPath1->in_out_stream_idx].valid)
						{
							pe_tc_queue_test_req.conn_info.ldpid 	= fc_db.streamidTbl[pFlowPath1->in_out_stream_idx].ldpid;
							pe_tc_queue_test_req.conn_info.cos 		= fc_db.streamidTbl[pFlowPath1->in_out_stream_idx].cos;
							pe_tc_queue_test_req.conn_info.flowid 	= fc_db.streamidTbl[pFlowPath1->in_out_stream_idx].gemid;
						}
						else
						{
							pe_tc_queue_test_req.conn_info.ldpid 	= flowConfig.path12.action.ldpid;
						}
						if(pG3IgrExtraInfo->dualHdrType==RTK_FC_DUALTYPE_NONE)
						{
							TRACE("[PE TC QUEUE][Single] Offload upstream lspid %d and pon streamId %d to PE cpu port 0x%x", flowConfig.path12.key.orig_lspid, pFlowPath1->in_out_stream_idx, pe_cpu_port);
							do_pe_offload = 1;
							
						}
						else //dual
						{
							if(fc_db.netifTbl[NETIF_HWTOSW(pFlowPath1->out_intf_idx)].extra_available && fc_db.netifTbl[NETIF_HWTOSW(pFlowPath1->out_intf_idx)].hwIdx_for_pe!=SIGNED_INVALID)
							{
								TRACE("[PE TC QUEUE][Dual] Offload upstream lspid %d and pon streamId %d to PE cpu port 0x%x", flowConfig.path12.key.orig_lspid, pFlowPath1->in_out_stream_idx, pe_cpu_port);
								do_pe_offload = 1;
								pe_tc_queue_test_req.conn_info.lspid 	= RTK_FC_DUAL_EXTRA_HWLOOKUP_LSPID_BASE + pG3IgrExtraInfo->dualHdrType;
								pe_tc_queue_test_req.conn_info.pol_grp_id 	= fc_db.netifTbl[NETIF_HWTOSW(pFlowPath1->out_intf_idx)].extra_netifId_map;
								pe_tc_queue_test_req.conn_info.ldpid 	= RT_PE_TC_QUEUE_TEST_HWLOOKUP_LDPID;
								pe_tc_queue_test_req.conn_info.fe_bypass 	= 0;
								//change egr_intf to single
								flowConfig.path12.action.egress_intf_idx = fc_db.netifTbl[NETIF_HWTOSW(pFlowPath1->out_intf_idx)].hwIdx_for_pe;
							}
							else
							{
								TRACE("[PE TC QUEUE][Dual] Fail to redirect to PE ... due to this dualHdrType %d does not support extra_available to do HWLOOKUP !!", pG3IgrExtraInfo->dualHdrType);
							}
						}
					}
				}
			}
			if(do_pe_offload)
			{
				flowConfig.path12.action.ldpid = pe_cpu_port;
				flowConfig.path12.action.gemId_mapping_mode = 0;
				flowConfig.path12.action.cos_update_en = TRUE;
				flowConfig.path12.action.cos = 0;
				if(fc_db.pe_queue_tc_mode==0)
				{
					flowConfig.path12.action.wifi_ssid = pFlowPath1->in_out_stream_idx + RT_PE_QUEUE_TEST_SW_ID_BASE;
				}
				else
				{
					if(pG3IgrExtraInfo->pe_tc_queue_flow_lspid != FAIL)
					{
						if(pG3IgrExtraInfo->igr_ipv4_valid || pG3IgrExtraInfo->igr_ipv6_valid)
						{
							//for rsv cls
							pe_tc_queue_test_rsv_cls_info.lspid = flowConfig.path12.key.orig_lspid;
							memcpy(pe_tc_queue_test_rsv_cls_info.smac, flowConfig.path12.key.src_mac, ETH_ALEN);
							memcpy(pe_tc_queue_test_rsv_cls_info.dmac, flowConfig.path12.key.dst_mac, ETH_ALEN);
							if(pG3IgrExtraInfo->igr_ipv4_valid)
							{
								pe_tc_queue_test_rsv_cls_info.isIPv4OrIpv6 = 0;
								pe_tc_queue_test_rsv_cls_info.ipv4_dip_addr = pG3IgrExtraInfo->igr_ipv4_dip;
							}
							else
							{
								pe_tc_queue_test_rsv_cls_info.isIPv4OrIpv6 = 1;
								memcpy(&pe_tc_queue_test_rsv_cls_info.ipv6_dip_addr, &pG3IgrExtraInfo->igr_ipv6_dip, sizeof(struct in6_addr));
							}
							pe_tc_queue_test_rsv_cls_info.action_lspid = pG3IgrExtraInfo->pe_tc_queue_flow_lspid;
							//for redirect flow key
							flowConfig.path12.key.orig_lspid = pG3IgrExtraInfo->pe_tc_queue_flow_lspid;
						}				
						else
						{
							TRACE("[PE TC QUEUE] Fail to add redirect flow to PE ... due to flow is non-ip path12  !!");
							return CA_E_ERROR;
						}	
					}
					flowConfig.path12.action.wifi_ssid = connection_idx + RT_PE_TC_QUEUE_TEST_SW_ID_BASE;
					pe_tc_queue_idx = pe_queue_idx;
					flow->pe_tc_queue_valid = 1;
					flow->pe_tc_queue_idx = pe_tc_queue_idx; 
					flow->pe_tc_queue_connection_idx = connection_idx;
				}
			}
		}
#endif
#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)
		if(pG3IgrExtraInfo->ipsec_pe_offload ==1)
		{
#if 1
			TRACE("[PE IPSEC] Do not support path12 !!!");
#else
			// un-tag
			flowConfig.path12.action.vlan_cnt = 0;
			flowConfig.path12.action.wifi_ssid = fc_db.fc_ipsec_info[pG3IgrExtraInfo->ipsec_hook_table_index].pe_crypto_sw_id;
			flowConfig.path12.action.gemId_mapping_mode = 0;
			flowConfig.path12.action.ldpid = RT_PE_IPSEC_CPU_PORT;
			flowConfig.path12.action.cos_update_en = TRUE;
			flowConfig.path12.action.cos = 0;
			if(fc_db.fc_ipsec_info[pG3IgrExtraInfo->ipsec_hook_table_index].direction==RTK_FC_IPSEC_DIR_ENCRYT)
			{
				flowConfig.path12.action.wifi_ssid |= ((pFlowPath1->in_ipv4_or_ipv6) ? RT_PE_IPSEC_SW_ID_INNER_IPV6_BIT : 0x0 );
			}
#endif			
		}
#endif		
#if defined(CONFIG_RTK_FC_SRV6_OFFLOAD_BY_PE)
		if(pG3IgrExtraInfo->dualHdrType == RTK_FC_DUALTYPE_SRV6 && pG3IgrExtraInfo->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM && pG3IgrExtraInfo->srv6_connection_idx!=FAIL)
		{
			flowConfig.path12.action.wifi_ssid = (pG3IgrExtraInfo->srv6_connection_idx + RT_PE_SRV6_SW_ID_CONNECTION_IDX_BASE);
			flowConfig.path12.action.ldpid = RT_PE_SRV6_CPU_PORT;
			flowConfig.path12.action.gemId_mapping_mode = 0;
			flowConfig.path12.action.cos_update_en = TRUE;
			flowConfig.path12.action.cos = 0;
		}
#endif
	}
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
	else if(flow->in_path == FB_PATH_MC_WIFI_AMSDU_TX)
	{
		rtk_rg_asic_pathMc_wifi_amsdu_tx_entry_t *pathMc_wifi_amsdu_tx = (rtk_rg_asic_pathMc_wifi_amsdu_tx_entry_t *)flow;
		flowConfig.in_path = FB_PATH_MC_WIFI_AMSDU_TX;

		/* === key === */
		// key: L2
		flowConfig.pathMc_wifi_amsdu_tx.key.wifi_mac_id = pathMc_wifi_amsdu_tx->in_wifi_mac_id;

		if(fc_db.lutTbl[pathMc_wifi_amsdu_tx->in_dmac_lut_idx] == NULL)
		{
			WARNING("lutTbl[%d] is NULL!", pathMc_wifi_amsdu_tx->in_dmac_lut_idx);
			goto ADD_HW_FLOW_FAIL;
		}
		else
			memcpy(&flowConfig.pathMc_wifi_amsdu_tx.key.dst_mac[0], &fc_db.lutTbl[pathMc_wifi_amsdu_tx->in_dmac_lut_idx]->l2Addr, ETH_ALEN);

		/* === action === */
		flowConfig.pathMc_wifi_amsdu_tx.action.mac_da_idx_act = pathMc_wifi_amsdu_tx->out_dmac_lut_act;
		flowConfig.pathMc_wifi_amsdu_tx.action.mac_da_idx = pathMc_wifi_amsdu_tx->out_dmac_lut_idx;
		flowConfig.pathMc_wifi_amsdu_tx.action.cos_update_en = pathMc_wifi_amsdu_tx->out_cos_act;
		flowConfig.pathMc_wifi_amsdu_tx.action.cos = pathMc_wifi_amsdu_tx->out_cos;
		flowConfig.pathMc_wifi_amsdu_tx.action.ldpid_act = pathMc_wifi_amsdu_tx->out_ldpid_act;
		flowConfig.pathMc_wifi_amsdu_tx.action.ldpid = pathMc_wifi_amsdu_tx->out_ldpid;
		flowConfig.pathMc_wifi_amsdu_tx.action.sw_id_act = pathMc_wifi_amsdu_tx->out_sw_id_act;
		flowConfig.pathMc_wifi_amsdu_tx.action.sw_id = pathMc_wifi_amsdu_tx->out_sw_id;
#if defined(CONFIG_FC_RTL9607F_SERIES)
		flowConfig.pathMc_wifi_amsdu_tx.action.l2format_act_vld = pathMc_wifi_amsdu_tx->out_l2format_act_vld;
		flowConfig.pathMc_wifi_amsdu_tx.action.l2format_act = pathMc_wifi_amsdu_tx->out_l2format_act;
#endif
		flowConfig.pathMc_wifi_amsdu_tx.action.is_amsdu_pe_offload_wifiTx = pathMc_wifi_amsdu_tx->is_amsdu_pe_offload_wifiTx;
#if defined(CONFIG_RTK_FC_WFO_PER_HW_FLOW_MIB)
		flow_mib_tx_info.wifi_pri = pathMc_wifi_amsdu_tx->out_wfo_tx_wifi_pri;
		flow_mib_tx_info.mac_id = pathMc_wifi_amsdu_tx->out_wfo_tx_mac_id ;
		if_wifi_tx = TRUE;
#endif
	}
#endif
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	else if(flow->in_path == FB_PATH_VXLAN_US_EXTRA_TX)
	{
		rtk_rg_asic_pathVxlan_up_extra_tx_entry_t *pFlowVxlan_up_frag_tx = (rtk_rg_asic_pathVxlan_up_extra_tx_entry_t *)flow;
		flowConfig.in_path = FB_PATH_VXLAN_US_EXTRA_TX;
		
		//VXLAN("flowIdx = %d(0x%x) g3 info: %d(0x%x)",flowIdx, flowIdx, pG3IgrExtraInfo->crc16, pG3IgrExtraInfo->crc16);

		/* === key === */
		// key: L2
		flowConfig.pathVxlan_up_extra_tx.key.orig_lspid		=  pFlowVxlan_up_frag_tx->lspid;
		flowConfig.pathVxlan_up_extra_tx.key.netif_id = pFlowVxlan_up_frag_tx->netif_id;

		/* === action === */
		flowConfig.pathVxlan_up_extra_tx.action.cos_update_en = pFlowVxlan_up_frag_tx->out_user_pri_act;
		flowConfig.pathVxlan_up_extra_tx.action.cos = pFlowVxlan_up_frag_tx->out_user_priority;
		flowConfig.pathVxlan_up_extra_tx.action.ldpid = pFlowVxlan_up_frag_tx->ldpid;
		flowConfig.pathVxlan_up_extra_tx.action.egress_intf_idx = _rtk_fc_flow_intf_mapping_idx_get(pFlowVxlan_up_frag_tx->out_intf_idx);

#if defined(CONFIG_RG_G3_WAN_PORT_INDEX)&&(CONFIG_RG_G3_WAN_PORT_INDEX==7)
		if((pFlowVxlan_up_frag_tx->out_stream_idx_act) && fc_db.streamidTbl[pFlowVxlan_up_frag_tx->out_stream_idx].valid &&
			(fc_db.wanPortMask.portmask & (1 << flowConfig.pathVxlan_up_extra_tx.action.ldpid)))
		{
			/*Don't Use Gem mapping mode, gem-id and cos will be set in HDR CPU when dma lso doing hw-lookup.*/
			flowConfig.pathVxlan_up_extra_tx.action.ldpid = fc_db.streamidTbl[pFlowVxlan_up_frag_tx->out_stream_idx].ldpid;
			flowConfig.pathVxlan_up_extra_tx.action.to_pon = 1;
		}
#endif
		flowConfig.pathVxlan_up_extra_tx.action.vxlan_sport_update_en = pFlowVxlan_up_frag_tx->out_vxlan_sport_update_en;
		flowConfig.pathVxlan_up_extra_tx.action.vxlan_sport = pFlowVxlan_up_frag_tx->out_vxlan_sport;
	}
#endif
#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)
	else if(flow->in_path == FB_PATH_ESP_SPI_DS)
	{
		rtk_rg_asic_esp_ds_spi_entry_t *pFlowPath_ESP = (rtk_rg_asic_esp_ds_spi_entry_t *)flow;
		flowConfig.in_path = FB_PATH_ESP_SPI_DS;

		/* === key === */
		// key: L2
		flowConfig.path_esp_ds_spi.key.orig_lspid = flow_spa;
		TRACE("flowConfig.path_esp_ds_spi.key.orig_lspid: %d",flowConfig.path_esp_ds_spi.key.orig_lspid);
		if(pFlowPath_ESP->in_stagif)
		{
			flowConfig.path_esp_ds_spi.key.stag_if = TRUE;
			flowConfig.path_esp_ds_spi.key.svlan_tpid = fc_db.systemGlobal.stagTPID[pG3IgrExtraInfo->igr_svlan_tpid_sel];
			flowConfig.path_esp_ds_spi.key.svlan_id = pG3IgrExtraInfo->svlan_id;
			flowConfig.path_esp_ds_spi.key.svlan_pri = pG3IgrExtraInfo->svlan_pri;
			flowConfig.path_esp_ds_spi.key.svlan_dei = pG3IgrExtraInfo->igr_svlan_dei;
		}
		if(pFlowPath_ESP->in_ctagif)
		{
			flowConfig.path_esp_ds_spi.key.ctag_if = TRUE;
			flowConfig.path_esp_ds_spi.key.cvlan_tpid = CVLAN_TPID;
			flowConfig.path_esp_ds_spi.key.cvlan_id = pG3IgrExtraInfo->cvlan_id;
			flowConfig.path_esp_ds_spi.key.cvlan_pri = pFlowPath_ESP->in_cvlan_pri;
			flowConfig.path_esp_ds_spi.key.cvlan_cfi = pG3IgrExtraInfo->igr_cvlan_cfi;
		}

		if(pFlowPath_ESP->in_pppoeif)
		{
			flowConfig.path_esp_ds_spi.key.pppoetag_if = TRUE;
			flowConfig.path_esp_ds_spi.key.pppoe_sid = pG3IgrExtraInfo->pppoe_session_id;
		}

		memcpy(&flowConfig.path_esp_ds_spi.key.src_mac[0], &fc_db.lutTbl[pG3IgrExtraInfo->lutIgrSaIdx]->l2Addr, ETH_ALEN);

		if(fc_db.lutTbl[fc_db.netifTbl[NETIF_HWTOSW(pFlowPath_ESP->in_intf_idx)].lutIdx]==NULL)
		{
			WARNING("path_esp_ds_spi ingress DMAC: lutTbl[%d] is NULL!", fc_db.netifTbl[NETIF_HWTOSW(pFlowPath_ESP->in_intf_idx)].lutIdx);
			goto ADD_HW_FLOW_FAIL;
		}
		else
			memcpy(&flowConfig.path_esp_ds_spi.key.dst_mac[0], &fc_db.lutTbl[fc_db.netifTbl[NETIF_HWTOSW(pFlowPath_ESP->in_intf_idx)].lutIdx]->l2Addr, ETH_ALEN);

		// key: L3
		if(pFlowPath_ESP->in_ipv4_or_ipv6)
		{
			flowConfig.path_esp_ds_spi.key.ipv4_or_ipv6 = TRUE; // 1: IPv6, 0: IPv4
			flowConfig.path_esp_ds_spi.key.ip_sa[0] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Saddr.s6_addr[0]);
			flowConfig.path_esp_ds_spi.key.ip_sa[1] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Saddr.s6_addr[4]);
			flowConfig.path_esp_ds_spi.key.ip_sa[2] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Saddr.s6_addr[8]);
			flowConfig.path_esp_ds_spi.key.ip_sa[3] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Saddr.s6_addr[12]);
			flowConfig.path_esp_ds_spi.key.ip_da[0] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Daddr.s6_addr[0]);
			flowConfig.path_esp_ds_spi.key.ip_da[1] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Daddr.s6_addr[4]);
			flowConfig.path_esp_ds_spi.key.ip_da[2] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Daddr.s6_addr[8]);
			flowConfig.path_esp_ds_spi.key.ip_da[3] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Daddr.s6_addr[12]);
		}
		else
		{
			flowConfig.path_esp_ds_spi.key.ipv4_or_ipv6 = FALSE; // 1: IPv6, 0: IPv4
			flowConfig.path_esp_ds_spi.key.ip_sa[3] = pFlowPath_ESP->in_src_ipv4_addr;
			flowConfig.path_esp_ds_spi.key.ip_da[3] = fc_db.netifTbl[NETIF_HWTOSW(pFlowPath_ESP->in_intf_idx)].intf.gateway_ipv4_addr; // inbound
#if 0
			if(pFlowPath_ESP->out_l4_act)
			{
				// NAPT
				if(pFlowPath_ESP->out_l4_direction)
					flowConfig.path_esp_ds_spi.key.ip_da[3] = pFlowPath_ESP->in_dst_ipv4_addr;// outbound
				else
					flowConfig.path_esp_ds_spi.key.ip_da[3] = fc_db.netifTbl[NETIF_HWTOSW(pFlowPath_ESP->in_intf_idx)].intf.gateway_ipv4_addr; // inbound
			}
			else
			{
				// Routing
				flowConfig.path_esp_ds_spi.key.ip_da[3] = pFlowPath_ESP->in_dst_ipv4_addr;
			}
#endif			
		}
		if(pFlowPath_ESP->in_tos_check)
		{
			flowConfig.path_esp_ds_spi.key.ip_dscp = (pFlowPath_ESP->in_tos >>2)&0x3f;
			flowConfig.path_esp_ds_spi.key.ip_ecn = pFlowPath_ESP->in_tos&0x3;
		}
		flowConfig.path_esp_ds_spi.key.l4proto_num = pFlowPath_ESP->in_l4proto_num;
		// key: L4
		flowConfig.path_esp_ds_spi.key.l4_src_port = pFlowPath_ESP->in_l4_src_port;
		flowConfig.path_esp_ds_spi.key.l4_dst_port = pFlowPath_ESP->in_l4_dst_port;
		// from PON
		if(flow_spa == RTK_FC_MAC_PORT_PON)
		{
			flowConfig.path_esp_ds_spi.key.is_from_ponRx_wifi_rx =  RTK_ASIC_FLOW_FROM_PON;
			flowConfig.path_esp_ds_spi.key.pon_streamId_or_wifi_devIdx = pG3IgrExtraInfo->pon_stream_id; // WAN rx stream id or wifi RX SSID
		}
		flowConfig.path_esp_ds_spi.key.spi =  pG3IgrExtraInfo->ipsec_spi;
		TRACE("flowConfig.path_esp_ds_spi.key.spi = 0x%x",flowConfig.path_esp_ds_spi.key.spi);

		/* === action === */
		// action: L2
		flowConfig.path_esp_ds_spi.action.mac_da_idx = pG3IgrExtraInfo->lutEgrDaIdx;
		flowConfig.path_esp_ds_spi.action.vlan_act = TRUE;
		if((pFlowPath_ESP->out_svid_format_act + pFlowPath_ESP->out_cvid_format_act) == 2)
		{
			// double tag
			flowConfig.path_esp_ds_spi.action.vlan_cnt = 2;
			flowConfig.path_esp_ds_spi.action.outer_tpid = outterTPID;
			flowConfig.path_esp_ds_spi.action.outer_vlanid = pFlowPath_ESP->out_svlan_id;
			flowConfig.path_esp_ds_spi.action.outer_vlanpri = pFlowPath_ESP->out_spri;
			flowConfig.path_esp_ds_spi.action.outer_dei = pG3IgrExtraInfo->egr_svlan_dei;
			flowConfig.path_esp_ds_spi.action.inner_tpid = CVLAN_TPID;
			flowConfig.path_esp_ds_spi.action.inner_vlanid = pFlowPath_ESP->out_cvlan_id;
			flowConfig.path_esp_ds_spi.action.inner_vlanpri = pFlowPath_ESP->out_cpri;
			flowConfig.path_esp_ds_spi.action.inner_dei = pG3IgrExtraInfo->egr_cvlan_cfi;
		}
		else if(pFlowPath_ESP->out_svid_format_act)
		{
			// single tag
			flowConfig.path_esp_ds_spi.action.vlan_cnt = 1;
			flowConfig.path_esp_ds_spi.action.outer_tpid = outterTPID;
			flowConfig.path_esp_ds_spi.action.outer_vlanid = pFlowPath_ESP->out_svlan_id;
			flowConfig.path_esp_ds_spi.action.outer_vlanpri = pFlowPath_ESP->out_spri;
			flowConfig.path_esp_ds_spi.action.outer_dei = pG3IgrExtraInfo->egr_svlan_dei;
		}
		else if(pFlowPath_ESP->out_cvid_format_act)
		{
			// single tag
			flowConfig.path_esp_ds_spi.action.vlan_cnt = 1;
			flowConfig.path_esp_ds_spi.action.outer_tpid = CVLAN_TPID;
			flowConfig.path_esp_ds_spi.action.outer_vlanid = pFlowPath_ESP->out_cvlan_id;
			flowConfig.path_esp_ds_spi.action.outer_vlanpri = pFlowPath_ESP->out_cpri;
			flowConfig.path_esp_ds_spi.action.outer_dei = pG3IgrExtraInfo->egr_cvlan_cfi;
		}
		else
		{
			// un-tag
			flowConfig.path_esp_ds_spi.action.vlan_cnt = 0;
		}

		// action: L3/L4
		flowConfig.path_esp_ds_spi.action.ip_dscp_update_en = pFlowPath_ESP->out_dscp_act;
		flowConfig.path_esp_ds_spi.action.ip_dscp = pFlowPath_ESP->out_dscp;
#if 0
		if(pFlowPath_ESP->out_l4_act) // always 0
		{
			{
				// NAPT
				flowConfig.path_esp_ds_spi.action.ip_action_mode = RTK_ASIC_IP_ACTION_MODE_V4_V6_NAPT;// 0: v4_v6_routing, 1: v4_v6_NAPT, 2: v6_NPT (rtk_8277c_asic_flow_config_ip_action_mode_t)
				if(pFlowPath_ESP->in_ipv4_or_ipv6==0)
				{
					//IPv4
					if(pFlowPath_ESP->out_l4_direction)
					{
						// outbound
						flowConfig.path_esp_ds_spi.action.ip_type = 0; // 0: replace ip_sa/l4_sport according to ip_addr configuration (NAPT); 1: replace ip_da/l4_dport according to ip_addr configuration (NAPTR)
						flowConfig.path_esp_ds_spi.action.ip[3] = fc_db.netifTbl[NETIF_HWTOSW(pFlowPath_ESP->out_intf_idx)].intf.gateway_ipv4_addr;
					}
					else
					{
						// inbound
						flowConfig.path_esp_ds_spi.action.ip_type = 1; // 0: replace ip_sa/l4_sport according to ip_addr configuration (NAPT); 1: replace ip_da/l4_dport according to ip_addr configuration (NAPTR)
						flowConfig.path_esp_ds_spi.action.ip[3] = pFlowPath_ESP->out_dst_ipv4_addr;
					}
					flowConfig.path_esp_ds_spi.action.l4_port = pFlowPath_ESP->out_l4_port;
				}
				else if(pFlowPath_ESP->in_ipv4_or_ipv6==1)
				{
					//IPv6
					if(pFlowPath_ESP->out_l4_direction)
					{
						// outbound
						flowConfig.path_esp_ds_spi.action.ip_type = 0; // 0: replace ip_sa/l4_sport according to ip_addr configuration (NAPT); 1: replace ip_da/l4_dport according to ip_addr configuration (NAPTR)
					}
					else
					{
						// inbound
						flowConfig.path_esp_ds_spi.action.ip_type = 1; // 0: replace ip_sa/l4_sport according to ip_addr configuration (NAPT); 1: replace ip_da/l4_dport according to ip_addr configuration (NAPTR)
					}
					flowConfig.path_esp_ds_spi.action.ip[0] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_nat_addr.s6_addr[0]);
					flowConfig.path_esp_ds_spi.action.ip[1] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_nat_addr.s6_addr[4]);
					flowConfig.path_esp_ds_spi.action.ip[2] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_nat_addr.s6_addr[8]);
					flowConfig.path_esp_ds_spi.action.ip[3] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_nat_addr.s6_addr[12]);
					flowConfig.path_esp_ds_spi.action.l4_port = pFlowPath_ESP->out_l4_port;
				}
			}
		}
#endif
		// action: Others
		flowConfig.path_esp_ds_spi.action.cos_update_en = pFlowPath_ESP->out_user_pri_act;
		flowConfig.path_esp_ds_spi.action.cos = pFlowPath_ESP->out_user_priority;
		flowConfig.path_esp_ds_spi.action.ldpid = flow_dpa;
		flowConfig.path_esp_ds_spi.action.mapeMapT_fmr_idx_vld = pFlowPath_ESP->out_fmr_idx_act;
		flowConfig.path_esp_ds_spi.action.mapeMapT_fmr_idx = pFlowPath_ESP->out_fmr_idx;

		flowConfig.path_esp_ds_spi.action.ingress_intf_idx = _rtk_fc_flow_intf_mapping_idx_get(pFlowPath_ESP->in_intf_idx);
		flowConfig.path_esp_ds_spi.action.egress_intf_idx = _rtk_fc_flow_intf_mapping_idx_get(pFlowPath_ESP->out_intf_idx);
		flowConfig.path_esp_ds_spi.action.l3_if_vld1	= TRUE;
		_rtk_fc_g3FlowIdDecision(flow, pG3IgrExtraInfo, &flowIdActMask, &flowId);
		// policer1 is host policing
		if(flowIdActMask)
		{
			flowConfig.path_esp_ds_spi.action.pol_en = TRUE;
			flowConfig.path_esp_ds_spi.action.pol_id = flowId;
		}

		// policer_2: flow meter (+ flow mib (flow_meter_mib_conf_dependence enabled))
		if(pFlowPath_ESP->out_share_meter_act)
		{
			flowConfig.path_esp_ds_spi.action.pol2_en = TRUE;
			flowConfig.path_esp_ds_spi.action.pol2_id = pFlowPath_ESP->out_share_meter_idx + G3_FLOW_POLICER_IDXSHIFT_FLOWMTR;
		}


#if 0//defined(CONFIG_RG_G3_WAN_PORT_INDEX)&&(CONFIG_RG_G3_WAN_PORT_INDEX==7)
		if(pFlowPath_ESP->out_stream_idx_act && fc_db.streamidTbl[pFlowPath_ESP->out_stream_idx].valid &&
			(fc_db.wanPortMask.portmask & (1 << flowConfig.path_esp_ds_spi.action.ldpid)))
		{
			flowConfig.path_esp_ds_spi.action.gemId_mapping_mode = RTK_GEM_MAPPING_MODE;
			flowConfig.path_esp_ds_spi.action.gem_id = fc_db.streamidTbl[pFlowPath_ESP->out_stream_idx].gemid;
			flowConfig.path_esp_ds_spi.action.tcont_id = fc_db.streamidTbl[pFlowPath_ESP->out_stream_idx].ldpid&0x1f;
			if(fc_db.streamidTbl[pFlowPath_ESP->out_stream_idx].cos)
			{
				flowConfig.path_esp_ds_spi.action.cos_update_en = TRUE;
				flowConfig.path_esp_ds_spi.action.cos = fc_db.streamidTbl[pFlowPath_ESP->out_stream_idx].cos;
			}
			else
				flowConfig.path_esp_ds_spi.action.cos_update_en = FALSE;// always disable cos update to prevent packet is sent to invalid PON queue
		}
#endif

		if(pG3IgrExtraInfo->naptv6_info.isNPTv6 ==1)
		{
			
			flowConfig.path_esp_ds_spi.action.nptv6_cfg.ipv6_prefix_index_aft = pG3IgrExtraInfo->naptv6_info.prefix_idx;
			if(pFlowPath_ESP->out_l4_direction)
			{
				flowConfig.path_esp_ds_spi.action.nptv6_cfg.ipv6_prefix_len_aft = pG3IgrExtraInfo->naptv6_info.wan_prefix_len;
				flowConfig.path_esp_ds_spi.action.nptv6_cfg.ipv6_prefix_len_ori = pG3IgrExtraInfo->naptv6_info.lan_prefix_len;
			}
			else
			{
				flowConfig.path_esp_ds_spi.action.nptv6_cfg.ipv6_prefix_len_aft = pG3IgrExtraInfo->naptv6_info.lan_prefix_len;
				flowConfig.path_esp_ds_spi.action.nptv6_cfg.ipv6_prefix_len_ori = pG3IgrExtraInfo->naptv6_info.wan_prefix_len;
			}
			flowConfig.path_esp_ds_spi.action.ip_action_mode = RTK_ASIC_IP_ACTION_MODE_V6_NPT;
			flowConfig.path_esp_ds_spi.action.nptv6_cfg.if_ipv6_add_hw_recal = 1;

		}


		if(pG3IgrExtraInfo->ipsec_pe_offload ==1)
		{
			//disable mtu checking
			flowConfig.path_esp_ds_spi.action.disable_mtu_check = TRUE;
			// un-tag
			flowConfig.path_esp_ds_spi.action.vlan_cnt = 0;
			flowConfig.path_esp_ds_spi.action.wifi_ssid = fc_db.fc_ipsec_info[pG3IgrExtraInfo->ipsec_hook_table_index].pe_crypto_sw_id;
			flowConfig.path_esp_ds_spi.action.gemId_mapping_mode = 0;
			flowConfig.path_esp_ds_spi.action.ldpid = RT_PE_IPSEC_CPU_PORT;
			flowConfig.path_esp_ds_spi.action.cos_update_en = TRUE;
			flowConfig.path_esp_ds_spi.action.cos = 0;
			if(fc_db.fc_ipsec_info[pG3IgrExtraInfo->ipsec_hook_table_index].direction==RTK_FC_IPSEC_DIR_ENCRYT)
			{
				flowConfig.path_esp_ds_spi.action.disable_smac_pppoe_trans = TRUE; // for pppoe wan without pppoe tag
				flowConfig.path_esp_ds_spi.action.wifi_ssid |= ((pFlowPath_ESP->in_ipv4_or_ipv6) ? RT_PE_IPSEC_SW_ID_INNER_IPV6_BIT : 0x0 );
			}
		}
		else
		{
			// redirect to main cpu instead of original dest port
			flowConfig.path_esp_ds_spi.action.ldpid = RTK_FC_MAC_PORT_MAINCPU;
			flowConfig.path_esp_ds_spi.action.wifi_ssid = RTK_FC_SWID_HW_SW_HYBRID_FWD;
			
			flowConfig.path_esp_ds_spi.action.pol_en = 0;
			flowConfig.path_esp_ds_spi.action.pol2_en = 0;
			flowConfig.path_esp_ds_spi.action.pol3_en = 0;
			flowConfig.path_esp_ds_spi.action.gemId_mapping_mode = 0;
			flowConfig.path_esp_ds_spi.action.vlan_cnt = 0; // always untag
#if defined(CONFIG_FC_RTL9607F_SERIES)
			if(flowConfig.path_esp_ds_spi.action.l2format_act_vld == TRUE && 
				flowConfig.path_esp_ds_spi.action.l2format_act == RTK_ASIC_L2_FORMAT_ACT_SNAP &&
				!fc_db.controlFuc.hw_sw_hybrid_fwd_to_pe) {	// to ARM wifi driver, disable snap translation
				flowConfig.path_esp_ds_spi.action.l2format_act_vld = FALSE;
				flowConfig.path_esp_ds_spi.action.l2format_act = RTK_ASIC_L2_FORMAT_ACT_ETHERNET;
			}
#endif
		}
	}
#endif
	else
	{
		WARNING("not support\n");
		return CA_E_NOT_SUPPORT;
	}

#if defined(CONFIG_RTK_FC_WFO_PER_HW_FLOW_MIB)
	if(fc_db.wfo_per_flow_mib_init_done)
	{
		/*	if_wifi rx (if_wifiRx_wo_amsdu)	| if_wifi tx (if_wifiTx_wo_amsdu)	| flow
			--------------------------------------------------------------------------------------------------------------
			0 (0)							| 0	(0)								| HW		(no need to AMSDU)
			0 (0)							| 1	(0)								| HW		(by TX AMSDU)
			0 (0)							| 1	(1)								| pure SW
			1 (0)							| 0	(0)								| HW		(by RX AMSDU)
			1 (1)							| 0	(0)								| pure SW
			1 (0)							| 1	(0)								| HW		(by TX AMSDU)
			1 (0)							| 1	(1)								| pure SW (wifiRx mib table to LAN/WAN only)
			1 (1)							| 1	(0)								| HW     	(by TX AMSDU)
			1 (1)							| 1	(1)								| pure SW
		*/
		if(if_wifi_rx && if_wifi_tx)
		{
			// wifi rx -> wifi tx
			if(if_wifiTx_wo_amsdu)
			{
				TRACE("Wifi TX without AMSDU. Do not to add HW flow due to CONFIG_RTK_FC_WFO_PER_HW_FLOW_MIB");
				*mainHashIdx = G3_FLOWIDX_INVALID;
				*flow_add_state = G3_FLOW_ADD_STATE_PURE_SW;
				return CA_E_OK;
			}
		}
		else
		{
			if(if_wifiTx_wo_amsdu || if_wifiRx_wo_amsdu)
			{
				TRACE("Wifi %s without AMSDU. Do not to add HW flow due to CONFIG_RTK_FC_WFO_PER_HW_FLOW_MIB", if_wifiTx_wo_amsdu?"TX":"RX");
				*mainHashIdx = G3_FLOWIDX_INVALID;
				*flow_add_state = G3_FLOW_ADD_STATE_PURE_SW;
				return CA_E_OK;
			}
		}
		if(if_wifi_rx || if_wifi_tx)
		{
			//update share memory table
			if( _rtk_fc_wifi_amsdu_wfo_flow_mib_tbl_write(flowIdx, flow, pG3IgrExtraInfo, if_wifi_tx?TRUE:FALSE, if_wifi_tx?&flow_mib_tx_info:&flow_mib_rx_info))
			{
				WARNING("Write WFO FLOW MIB TABLE FAIL");
				goto ADD_HW_FLOW_FAIL;
			}
		}
	}

#endif
#if defined(CONFIG_RTK_FC_PKT_QUEUE_OFFLOAD_BY_PE)
	if(pG3IgrExtraInfo->pe_tc_queue_flow_lspid!=FAIL && pe_tc_queue_idx==FAIL)
	{
		TRACE("[PE TC QUEUE] Fail to add redirect flow to PE ... pe_tc_queue_flow_lspid %u", pG3IgrExtraInfo->pe_tc_queue_flow_lspid);
		return CA_E_ERROR;
	}
#endif	

	{
		if(unlikely(fc_db.controlFuc.hash_crc_debug))
		{
			rtk_rg_asic_flow_hash_cal_info_t flowHashCalInfo = {0};
			flowHashCalInfo.in_path = flowConfig.in_path;
			switch(flowHashCalInfo.in_path)
			{
				case FB_PATH_12:
					memcpy(&flowHashCalInfo.path12_key, &flowConfig.path12.key, sizeof(flowHashCalInfo.path12_key));
					break;
				case FB_PATH_34:
					memcpy(&flowHashCalInfo.path34_key, &flowConfig.path34.key, sizeof(flowHashCalInfo.path34_key));
					break;
				case FB_PATH_5:
					memcpy(&flowHashCalInfo.path5_key, &flowConfig.path5.key, sizeof(flowHashCalInfo.path5_key));
					break;
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
				case FB_PATH_MC_WIFI_AMSDU_TX:
					memcpy(&flowHashCalInfo.pathMc_wifi_amsdu_tx_key, &flowConfig.pathMc_wifi_amsdu_tx.key, sizeof(flowHashCalInfo.pathMc_wifi_amsdu_tx_key));
					break;
#endif
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
				case FB_PATH_VXLAN_US_EXTRA_TX:
					memcpy(&flowHashCalInfo.pathVxlan_up_extra_tx_key, &flowConfig.pathVxlan_up_extra_tx.key, sizeof(flowHashCalInfo.pathVxlan_up_extra_tx_key));
					break;
#endif
#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)
				case FB_PATH_ESP_SPI_DS:
					memcpy(&flowHashCalInfo.path_esp_ds_spi_key, &flowConfig.path_esp_ds_spi.key, sizeof(flowHashCalInfo.path_esp_ds_spi_key));
					break;
#endif
				default:
					break;
			}
			rtk_rg_asic_flow_hash_crc_cal(&flowHashCalInfo, &flow_hash_crc);
			if((flow_hash_crc.crc16 != pG3IgrExtraInfo->crc16) || (flow_hash_crc.crc32 != pG3IgrExtraInfo->crc32))
				WARNING("[hash_crc_debug] flow add CRC check failed!!! CRC16/CRC32 (flow re-cal): 0x%04x/0x%08x, CRC16/CRC32 (pG3IgrExtraInfo): 0x%04x/0x%08x", flow_hash_crc.crc16, flow_hash_crc.crc32, pG3IgrExtraInfo->crc16, pG3IgrExtraInfo->crc32);
			else
				TRACE("[hash_crc_debug] CRC matched! CRC16/CRC32 (flow re-cal): 0x%04x/0x%08x, CRC16/CRC32 (pG3IgrExtraInfo): 0x%04x/0x%08x", flow_hash_crc.crc16, flow_hash_crc.crc32, pG3IgrExtraInfo->crc16, pG3IgrExtraInfo->crc32);
		}
		flow_hash_crc.crc16 = pG3IgrExtraInfo->crc16;
		flow_hash_crc.crc32 = pG3IgrExtraInfo->crc32;
		TRACE("Add crc 16: %x crc32: %x",flow_hash_crc.crc16,flow_hash_crc.crc32);
		ret = rtk_rg_asic_flow_add_with_no_crc_calulate_by_idx(&flowConfig, flowIdx, flow_hash_crc);
	}

#if defined(CONFIG_RTK_FC_PKT_QUEUE_OFFLOAD_BY_PE)
	if(pe_tc_queue_idx!=FAIL && (ret==ASIC_RET_SUCCESS || ret==ASIC_RET_EXIST))
	{
		rt_pe_ret_t pe_ret;
		pe_ret = rtk_fc_pe_queue_test_connection_add(pe_tc_queue_idx, pe_tc_queue_test_req, flowIdx, &pe_tc_queue_test_rsv_cls_info);
		if(pe_ret!=RT_PE_RET_OK)
			WARNING("[PE TC QUEUE %d] Fail to add tc_queue connection[%d], pe_ret=%d", pe_tc_queue_idx, pe_tc_queue_test_req.conn_info.connection_idx, pe_ret);
	}
#endif

	if(ret == ASIC_RET_SUCCESS)
	{
		*mainHashIdx = flowIdx;
		*flow_add_state = G3_FLOW_ADD_STATE_HW;
		TABLE(">>>>> Add flow[%d] to Main Hash flow[%d]", flowIdx, *mainHashIdx);
		return CA_E_OK;
	}
	else if(ret == ASIC_RET_EXIST)
	{
		*mainHashIdx = flowIdx;
		*flow_add_state = G3_FLOW_ADD_STATE_HW;
		TABLE(">>>>> Add flow[%d] to Main Hash flow[%d], but the entry already exist", flowIdx, *mainHashIdx);
		return CA_E_OK;
	}
	else
	{
		TABLE(">>>>> Add flow[%d] to Main Hash flow failed, ret = 0x%x", flowIdx, ret);
	}
ADD_HW_FLOW_FAIL:
#if defined(CONFIG_RTK_FC_WFO_PER_HW_FLOW_MIB)
	_rtk_fc_wfo_cache_mib_list_Del(flow->out_wfo_flow_mib_idx);
#endif
	return CA_E_ERROR;
}
#else
//NOT (CONFIG_FC_RTL8277C_SERIES || CONFIG_FC_RTL9607F_SERIES)
static int32 RTK_RG_G3_FLOW_DEL(uint32 idx)
{
	ca_status_t ret = CA_E_OK;

	FC_PARAM_CHK(idx == G3_FLOWIDX_INVALID, CA_E_OK);

	ret = ca_flow_delete(G3_DEF_DEVID, idx);

	if(ret == CA_E_OK){
		TABLE(">>>>> Delete Main Hash flow[%d]", idx);
	}else{
		TABLE(">>>>> Delete Main Hash flow[%d] fail, ret = 0x%x", idx, ret);
	}
	return ret;
}
//NOT (CONFIG_FC_RTL8277C_SERIES || CONFIG_FC_RTL9607F_SERIES)
static int32 RTK_RG_G3_FLOW_SET(rtk_rg_asic_path1_entry_t *flow, uint32 *mainHashIdx, rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, uint8 swOnly, uint32 flowIdx, rtk_fc_g3_flow_add_state_t *flow_add_state)
{
	ca_status_t ret = CA_E_OK;
	ca_flow_t flow_config;
	bool outerSVlanEn = FALSE, flowIdActMask = FALSE;
	uint16 outerSVlanId = 0, outerSVlanPri = 0, outerSVlanDei = 0, flowId = 0;
	uint16 outterTPID = fc_db.systemGlobal.stagTPID[pG3IgrExtraInfo->egr_svlan_tpid_sel];
	bool fromLanPort;
	uint8 flow_spa = fc_db.lutTbl[pG3IgrExtraInfo->lutIgrSaIdx]->spa;
	uint8 flow_dpa = fc_db.lutTbl[pG3IgrExtraInfo->lutEgrDaIdx]->spa;
	rtk_fc_wlan_devidx_t flow_igrWlanDevIdx, flow_egrWlanDevIdx;

	*flow_add_state = G3_FLOW_ADD_STATE_FAIL;
	if(swOnly)
	{
		/*swOnly entry: not add to HW*/
		*mainHashIdx = G3_FLOWIDX_INVALID;
		*flow_add_state = G3_FLOW_ADD_STATE_PURE_SW;
		return CA_E_OK;
	}

#if defined(CONFIG_RTK_FC_ETHPORT_TRUNKING_BY_ONE_LUT)
	if (IS_ETH_TRUNKING_PORT(pG3IgrExtraInfo->igrspa))
		flow_spa = pG3IgrExtraInfo->igrspa;

	if (IS_ETH_TRUNKING_PORT(pG3IgrExtraInfo->egrdpa))
		flow_dpa = pG3IgrExtraInfo->egrdpa;
#endif

#if defined(CONFIG_RTK_FC_WIFI_MULTIBAND_ACC_BY_ONE_LUT)
	flow_igrWlanDevIdx = pG3IgrExtraInfo->igrWlanDevIdx;
	flow_egrWlanDevIdx = pG3IgrExtraInfo->egrWlanDevIdx;
#else
	flow_igrWlanDevIdx = fc_db.lutTbl[pG3IgrExtraInfo->lutIgrSaIdx]->wlan_dev_idx;
	flow_egrWlanDevIdx = fc_db.lutTbl[pG3IgrExtraInfo->lutEgrDaIdx]->wlan_dev_idx;
#endif

	fromLanPort = ((G3_LOOPBACK_P_START <= flow_spa) && (flow_spa <= G3_LOOPBACK_P_END)) ? TRUE : FALSE;

	TRACE("Translate flow in_path %d to G3 format\n", flow->in_path );

	// unicast: main hash flow
	memset(&flow_config, 0, sizeof(flow_config));
	// common setting
	flow_config.aging_time = fc_db.flowSyncPeriod_unit1s;
	flow_config.actions.forward = CA_CLASSIFIER_FORWARD_PORT;

#if defined(CONFIG_FC_CA8277B_SERIES)
	// from PON
	if(flow_spa == RTK_FC_MAC_PORT_PON)
		flow_config.key.flow_id = pG3IgrExtraInfo->pon_stream_id; // WAN rx stream id or wifi RX SSID

	// from wifi
	if((1 << flow_spa) & RTK_FC_ALL_MAC_WLANCPU_PORTMASK)
	{
#if 0	//NIC TX for wifi HW lookup always carry header_a in current configuration

		rtk_fc_wlan_devmap_t *wlanDevMap = NULL;
		if(RTK_FC_HELPER_WLAN_DEVMAP_GET(flow_igrWlanDevIdx, &wlanDevMap) == SUCCESS)
		{
			// if shareCpuPort == 0, DMA LSO will not carry header_a+header_cpu, and pol_id will be 0
			if(wlanDevMap && RTK_FC_HELPER_WLAN_DEVMAP_SHAREEXTPORT_GET(wlanDevMap))
				flow_config.key.flow_id = flow_igrWlanDevIdx;
		}
		else
			WARNING("get wlanDevMap[%d] failed", flow_igrWlanDevIdx);
#else
		flow_config.key.flow_id = flow_igrWlanDevIdx;
#endif
	}
#endif
	if(flow->in_path == FB_PATH_5){
		rtk_rg_asic_path5_entry_t *pFlowPath5 = (rtk_rg_asic_path5_entry_t *)flow;
		if( (1<<flow_spa) & fc_db.wanPortMask.portmask)
			flow_config.key_type = RG_CA_FLOW_UC5TUPLE_DS;	// naptr or routing
		else
			flow_config.key_type = RG_CA_FLOW_UC5TUPLE_US;	// napt or routing

		// Key: L2

		if(fc_db.controlFuc.loopbackMode_is_enabled && fromLanPort)
		{
			flow_config.key.l2_key.o_lspid = G3_LOOPBACK_P_NEWSPA;
			flow_config.key.l2_key.outer_vlan.vid = flow_spa + G3_LOOPBACK_UPSTREAM_VID_BASE;
			flow_config.key.l2_key.outer_vlan.pri = 0;
			flow_config.key.l2_key.outer_vlan.dei = 0;
			flow_config.key.l2_key.outer_vlan.tpid = CVLAN_TPID;
			if((pFlowPath5->in_stagif + pFlowPath5->in_ctagif) == 2){		// double tag
				flow_config.key.l2_key.inner_vlan.vid = pG3IgrExtraInfo->svlan_id;
				flow_config.key.l2_key.inner_vlan.pri = pG3IgrExtraInfo->svlan_pri;
				flow_config.key.l2_key.inner_vlan.dei = pG3IgrExtraInfo->igr_svlan_dei;
				flow_config.key.l2_key.inner_vlan.tpid = fc_db.systemGlobal.stagTPID[pG3IgrExtraInfo->igr_svlan_tpid_sel];
			}else{													// single tag or untag
				if(pFlowPath5->in_stagif){
					flow_config.key.l2_key.inner_vlan.vid = pG3IgrExtraInfo->svlan_id;
					flow_config.key.l2_key.inner_vlan.pri = pG3IgrExtraInfo->svlan_pri;
					flow_config.key.l2_key.inner_vlan.dei = pG3IgrExtraInfo->igr_svlan_dei;
					flow_config.key.l2_key.inner_vlan.tpid = fc_db.systemGlobal.stagTPID[pG3IgrExtraInfo->igr_svlan_tpid_sel];
				}
				if(pFlowPath5->in_ctagif){
					flow_config.key.l2_key.inner_vlan.vid = pG3IgrExtraInfo->cvlan_id;
					flow_config.key.l2_key.inner_vlan.pri = pFlowPath5->in_cvlan_pri;
					flow_config.key.l2_key.inner_vlan.dei = pG3IgrExtraInfo->igr_cvlan_cfi;
					flow_config.key.l2_key.inner_vlan.tpid = CVLAN_TPID;
				}
			}
		}
		else
		{
			flow_config.key.l2_key.o_lspid = flow_spa;	// force enabled to support unknown sa trap by main hash

			if((pFlowPath5->in_stagif + pFlowPath5->in_ctagif) == 2){		// double tag
				flow_config.key.l2_key.outer_vlan.vid = pG3IgrExtraInfo->svlan_id;
				flow_config.key.l2_key.outer_vlan.pri = pG3IgrExtraInfo->svlan_pri;
				flow_config.key.l2_key.outer_vlan.dei = pG3IgrExtraInfo->igr_svlan_dei;
				flow_config.key.l2_key.outer_vlan.tpid = fc_db.systemGlobal.stagTPID[pG3IgrExtraInfo->igr_svlan_tpid_sel];
				flow_config.key.l2_key.inner_vlan.vid = pG3IgrExtraInfo->cvlan_id;
				flow_config.key.l2_key.inner_vlan.pri = pFlowPath5->in_cvlan_pri;
				flow_config.key.l2_key.inner_vlan.dei = pG3IgrExtraInfo->igr_cvlan_cfi;
				flow_config.key.l2_key.inner_vlan.tpid = CVLAN_TPID;
			}else{													// single tag or untag
				if(pFlowPath5->in_stagif){
					flow_config.key.l2_key.outer_vlan.vid = pG3IgrExtraInfo->svlan_id;
					flow_config.key.l2_key.outer_vlan.pri = pG3IgrExtraInfo->svlan_pri;
					flow_config.key.l2_key.outer_vlan.dei = pG3IgrExtraInfo->igr_svlan_dei;
					flow_config.key.l2_key.outer_vlan.tpid = fc_db.systemGlobal.stagTPID[pG3IgrExtraInfo->igr_svlan_tpid_sel];
				}
				if(pFlowPath5->in_ctagif){
					flow_config.key.l2_key.outer_vlan.vid = pG3IgrExtraInfo->cvlan_id;
					flow_config.key.l2_key.outer_vlan.pri = pFlowPath5->in_cvlan_pri;
					flow_config.key.l2_key.outer_vlan.dei = pG3IgrExtraInfo->igr_cvlan_cfi;
					flow_config.key.l2_key.outer_vlan.tpid = CVLAN_TPID;
				}
			}
		}
		if(pFlowPath5->in_pppoeif)
		{
			flow_config.key.l2_key.pppoe_type = 2; //session
			flow_config.key.l2_key.pppoe_session_id = pG3IgrExtraInfo->pppoe_session_id;
		}
		// Key: L3
		flow_config.key.l3_key.ip_valid = TRUE;
		flow_config.key.l3_key.ip_version = pFlowPath5->in_ipv4_or_ipv6?6:4;
		flow_config.key.l3_key.ip_protocol = pFlowPath5->in_l4proto_num;
		if(pFlowPath5->in_tos_check)
		{
			flow_config.key.l3_key.dscp = (pFlowPath5->in_tos >>2)&0x3f;
			flow_config.key.l3_key.ecn = pFlowPath5->in_tos&0x3;
		}

		if(pFlowPath5->in_ipv4_or_ipv6){
			flow_config.key.l3_key.ip_sa.afi = CA_IPV6;
			flow_config.key.l3_key.ip_da.afi = CA_IPV6;
			flow_config.key.l3_key.ip_sa.addr_len = 128;
			flow_config.key.l3_key.ip_da.addr_len = 128;
			flow_config.key.l3_key.ip_sa.ip_addr.ipv6_addr[0] =  ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Saddr.s6_addr[0]);
			flow_config.key.l3_key.ip_sa.ip_addr.ipv6_addr[1] =  ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Saddr.s6_addr[4]);
			flow_config.key.l3_key.ip_sa.ip_addr.ipv6_addr[2] =  ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Saddr.s6_addr[8]);
			flow_config.key.l3_key.ip_sa.ip_addr.ipv6_addr[3] =  ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Saddr.s6_addr[12]);
			flow_config.key.l3_key.ip_da.ip_addr.ipv6_addr[0] =  ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Daddr.s6_addr[0]);
			flow_config.key.l3_key.ip_da.ip_addr.ipv6_addr[1] =  ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Daddr.s6_addr[4]);
			flow_config.key.l3_key.ip_da.ip_addr.ipv6_addr[2] =  ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Daddr.s6_addr[8]);
			flow_config.key.l3_key.ip_da.ip_addr.ipv6_addr[3] =  ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Daddr.s6_addr[12]);

		}else{
			flow_config.key.l3_key.ip_sa.afi = CA_IPV4;
			flow_config.key.l3_key.ip_da.afi = CA_IPV4;
			flow_config.key.l3_key.ip_sa.ip_addr.ipv4_addr = pFlowPath5->in_src_ipv4_addr;
			flow_config.key.l3_key.ip_sa.addr_len = 32;

			if(pFlowPath5->out_l4_act){
				// NAPT
				if(pFlowPath5->out_l4_direction){
					// outbound
					flow_config.key.l3_key.ip_da.ip_addr.ipv4_addr = pFlowPath5->in_dst_ipv4_addr;
					flow_config.key.l3_key.ip_da.addr_len = 32;
				}else{
					// inbound
					flow_config.key.l3_key.ip_da.ip_addr.ipv4_addr = fc_db.netifTbl[NETIF_HWTOSW(pFlowPath5->in_intf_idx)].intf.gateway_ipv4_addr;
					flow_config.key.l3_key.ip_da.addr_len = 32;
				}
			}else{
				// Routing
				flow_config.key.l3_key.ip_da.ip_addr.ipv4_addr = pFlowPath5->in_dst_ipv4_addr;
				flow_config.key.l3_key.ip_da.addr_len = 32;
			}
		}

		// Key: L4
		flow_config.key.l4_key.src_l4_port = pFlowPath5->in_l4_src_port;
		flow_config.key.l4_key.dst_l4_port = pFlowPath5->in_l4_dst_port;

		// Action: L2
		{
			// routing
			flow_config.actions.options.masks.decrement_ttl = TRUE;

			flow_config.actions.options.masks.mac_sa = TRUE;
			memcpy(&flow_config.actions.options.mac_sa, &fc_db.netifTbl[NETIF_HWTOSW(pFlowPath5->out_intf_idx)].intf.gateway_mac_addr, ETH_ALEN);

			flow_config.actions.options.masks.mac_da = TRUE;
			memcpy(&flow_config.actions.options.mac_da, &fc_db.lutTbl[pG3IgrExtraInfo->lutEgrDaIdx]->l2Addr[0], ETH_ALEN);

		}

#if defined(CONFIG_FC_CAG3_SERIES)
		if(pFlowPath5->out_stream_idx_act){
			// special case: support stream idx by svlan id
			outerSVlanEn = TRUE;
			outerSVlanId = pFlowPath5->out_stream_idx;
			outerSVlanPri = pFlowPath5->out_cpri;
			outerSVlanDei = pG3IgrExtraInfo->egr_svlan_dei;
		}
		else if(fc_db.controlFuc.loopbackMode_is_enabled && flow_dpa <= G3_LOOPBACK_P_END)
		{
			outerSVlanEn = TRUE;
			outerSVlanId = G3_LOOPBACK_DOWNSTREAM_VID(flow_dpa, pFlowPath5->out_user_priority);
			outerSVlanPri = 0;
			outerSVlanDei = 0;
			outterTPID = CVLAN_TPID;
		}
		else
#endif
		{
			//normal case: support double tag
			if(pFlowPath5->out_svid_format_act){
				outerSVlanEn = TRUE;
				outerSVlanId = pFlowPath5->out_svlan_id;
				outerSVlanPri = pFlowPath5->out_spri;
				outerSVlanDei = pG3IgrExtraInfo->egr_svlan_dei;
			}
		}
		
		if((outerSVlanEn + pFlowPath5->out_cvid_format_act) == 2){		// double tag
			flow_config.actions.options.masks.outer_vlan_act = TRUE;
			flow_config.actions.options.masks.outer_dot1p = TRUE;
			flow_config.actions.options.masks.outer_dei = TRUE;
			flow_config.actions.options.masks.outer_tpid = TRUE;
			flow_config.actions.options.outer_vlan_act = CA_FLOW_VLAN_ACTION_SET;
			flow_config.actions.options.outer_vid = outerSVlanId;
			flow_config.actions.options.outer_dot1p = outerSVlanPri;
			flow_config.actions.options.outer_dei = outerSVlanDei;
			flow_config.actions.options.outer_tpid = outterTPID;

			flow_config.actions.options.masks.inner_vlan_act = TRUE;
			flow_config.actions.options.masks.inner_dot1p = TRUE;
			flow_config.actions.options.masks.inner_dei = TRUE;
			flow_config.actions.options.masks.inner_tpid = TRUE;
			flow_config.actions.options.inner_vlan_act = CA_FLOW_VLAN_ACTION_SET;
			flow_config.actions.options.inner_vid = pFlowPath5->out_cvlan_id;
			flow_config.actions.options.inner_dot1p = pFlowPath5->out_cpri;
			flow_config.actions.options.inner_dei = pG3IgrExtraInfo->egr_cvlan_cfi;
			flow_config.actions.options.inner_tpid = CVLAN_TPID;
		}else{																		// single tag or untag
			if(outerSVlanEn){
				flow_config.actions.options.masks.outer_vlan_act = TRUE;
				flow_config.actions.options.masks.outer_dot1p = TRUE;
				flow_config.actions.options.masks.outer_dei = TRUE;
				flow_config.actions.options.masks.outer_tpid = TRUE;
				flow_config.actions.options.outer_vlan_act = CA_FLOW_VLAN_ACTION_SET;
				flow_config.actions.options.outer_vid = outerSVlanId;
				flow_config.actions.options.outer_dot1p = outerSVlanPri;
				flow_config.actions.options.outer_dei = outerSVlanDei;
				flow_config.actions.options.outer_tpid = outterTPID;
			}else if(pFlowPath5->out_cvid_format_act){
				flow_config.actions.options.masks.outer_vlan_act = TRUE;
				flow_config.actions.options.masks.outer_dot1p = TRUE;
				flow_config.actions.options.masks.outer_dei = TRUE;
				flow_config.actions.options.masks.outer_tpid = TRUE;
				flow_config.actions.options.outer_vlan_act = CA_FLOW_VLAN_ACTION_SET;
				flow_config.actions.options.outer_vid = pFlowPath5->out_cvlan_id;
				flow_config.actions.options.outer_dot1p = pFlowPath5->out_cpri;
				flow_config.actions.options.outer_dei = pG3IgrExtraInfo->egr_cvlan_cfi;
				flow_config.actions.options.outer_tpid = CVLAN_TPID;
			}else{
				flow_config.actions.options.masks.outer_vlan_act = TRUE;
				flow_config.actions.options.outer_vlan_act = CA_FLOW_VLAN_ACTION_POP;
			}
		}

		
		if(fc_db.netifTbl[NETIF_HWTOSW(pFlowPath5->out_intf_idx)].intf.valid && fc_db.netifTbl[NETIF_HWTOSW(pFlowPath5->out_intf_idx)].intf.out_pppoe_act == FB_NETIFPPPOE_ACT_ADD){
			flow_config.actions.options.masks.egress_pppoe_action = TRUE;
			flow_config.actions.options.pppoe_session_id =  fc_db.netifTbl[NETIF_HWTOSW(pFlowPath5->out_intf_idx)].intf.out_pppoe_sid;
		}else if (fc_db.netifTbl[NETIF_HWTOSW(pFlowPath5->in_intf_idx)].intf.valid && fc_db.netifTbl[NETIF_HWTOSW(pFlowPath5->in_intf_idx)].intf.out_pppoe_act == FB_NETIFPPPOE_ACT_ADD &&
				fc_db.netifTbl[NETIF_HWTOSW(pFlowPath5->out_intf_idx)].intf.valid&& fc_db.netifTbl[NETIF_HWTOSW(pFlowPath5->out_intf_idx)].intf.out_pppoe_act == FB_NETIFPPPOE_ACT_REMOVE){
			flow_config.actions.options.masks.egress_pppoe_action = TRUE;
			flow_config.actions.options.pppoe_session_id =  0;
		}
#if defined(CONFIG_FC_CA8277B_SERIES)
		if(fc_db.controlFuc.special_fast_forward_mode==1)
		{
			//For vxlan ds outter header
			if(pG3IgrExtraInfo->dualHdrType == RTK_FC_DUALTYPE_VXLAN && pG3IgrExtraInfo->direction == RTK_FC_FLOW_DIRECTION_DOWNSTREAM && pG3IgrExtraInfo->vxlan_info.is_vxlanDS_outerFlow)
			{
				if((outerSVlanEn + pFlowPath5->out_cvid_format_act) == 2)
				{
					flow_config.actions.options.outer_vlan_act = CA_FLOW_VLAN_ACTION_POP;
					flow_config.actions.options.inner_vlan_act = CA_FLOW_VLAN_ACTION_POP;
				}else
					flow_config.actions.options.outer_vlan_act = CA_FLOW_VLAN_ACTION_POP;

				flow_config.actions.options.masks.egress_pppoe_action = TRUE;
				flow_config.actions.options.pppoe_session_id =  0;
			}
			// VXLAN DOWN STREAM INNER FLOW!!
			if (pG3IgrExtraInfo->dualHdrType == RTK_FC_DUALTYPE_VXLAN && pG3IgrExtraInfo->direction == RTK_FC_FLOW_DIRECTION_DOWNSTREAM && !pG3IgrExtraInfo->vxlan_info.is_vxlanDS_outerFlow)
			{
				//For vxlan ds inner header
				flow_config.key.l2_key.o_lspid = 0; // For No HeaderA
				flow_config.key.flow_id = 0;
				//TRACE("[VXLAN]Round-robin downstream inner-flow use cpu-port 0x%x for o_lspid",flow_config.key.l2_key.o_lspid);


			}

			if (pG3IgrExtraInfo->dualHdrType == RTK_FC_DUALTYPE_VXLAN && pG3IgrExtraInfo->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM && pG3IgrExtraInfo->vxlan_info.is_vxlanUS_outerFlow)
			{
				flow_config.key.l2_key.o_lspid = 0; // For No HeaderA
				//TRACE("[VXLAN]Round-robin upstream outer-flow use cpu-port %d for o_lspid",flow_config.key.l2_key.o_lspid);
			}
		}
		
#endif

		// Action: L3 L4
		flow_config.actions.options.masks.dscp = pFlowPath5->out_dscp_act;
		flow_config.actions.options.dscp = pFlowPath5->out_dscp;

		if(pFlowPath5->out_l4_act){
			// NAPT
			if(pFlowPath5->in_ipv4_or_ipv6==0) //V4
			{
				if(pFlowPath5->out_l4_direction){
					// outbound
					flow_config.actions.options.masks.ip_sa = TRUE;
					flow_config.actions.options.masks.src_l4_port = TRUE;
					flow_config.actions.options.ip_sa.afi = CA_IPV4;
					flow_config.actions.options.ip_sa.ip_addr.ipv4_addr = fc_db.netifTbl[NETIF_HWTOSW(pFlowPath5->out_intf_idx)].intf.gateway_ipv4_addr;
					flow_config.actions.options.ip_sa.addr_len = 32;
					flow_config.actions.options.src_l4_port = pFlowPath5->out_l4_port;
				}else{
					// inbound
					flow_config.actions.options.masks.ip_da = TRUE;
					flow_config.actions.options.masks.dst_l4_port = TRUE;
					flow_config.actions.options.ip_da.afi = CA_IPV4;
					flow_config.actions.options.ip_da.ip_addr.ipv4_addr = pFlowPath5->out_dst_ipv4_addr;
					flow_config.actions.options.ip_da.addr_len = 32;
					flow_config.actions.options.dst_l4_port = pFlowPath5->out_l4_port;
				}
			}
#if defined(CONFIG_FC_CA8277B_SERIES)
			else if(pFlowPath5->in_ipv4_or_ipv6==1) //V6
			{
				if(pFlowPath5->out_l4_direction){
					// outbound
					flow_config.actions.options.masks.ip_sa = TRUE;
					flow_config.actions.options.masks.src_l4_port = TRUE;
					flow_config.actions.options.src_l4_port = pFlowPath5->out_l4_port;
					
					flow_config.actions.options.ip_sa.afi = CA_IPV6;
					flow_config.actions.options.ip_sa.ip_addr.ipv6_addr[0] =ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_nat_addr.s6_addr[0]);
					flow_config.actions.options.ip_sa.ip_addr.ipv6_addr[1] =ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_nat_addr.s6_addr[4]);
					flow_config.actions.options.ip_sa.ip_addr.ipv6_addr[2] =ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_nat_addr.s6_addr[8]);
					flow_config.actions.options.ip_sa.ip_addr.ipv6_addr[3] =ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_nat_addr.s6_addr[12]);
					
				}else{
					// inbound
					flow_config.actions.options.masks.ip_da = TRUE;
					flow_config.actions.options.masks.dst_l4_port = TRUE;
					flow_config.actions.options.ip_da.afi = CA_IPV6;
					flow_config.actions.options.dst_l4_port = pFlowPath5->out_l4_port;
					
					flow_config.actions.options.ip_da.ip_addr.ipv6_addr[0] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_nat_addr.s6_addr[0]);
					flow_config.actions.options.ip_da.ip_addr.ipv6_addr[1] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_nat_addr.s6_addr[4]);
					flow_config.actions.options.ip_da.ip_addr.ipv6_addr[2] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_nat_addr.s6_addr[8]);
					flow_config.actions.options.ip_da.ip_addr.ipv6_addr[3] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_nat_addr.s6_addr[12]);
				}
			}
#endif
			
		}


		// Action: Others
		flow_config.actions.options.masks.cos = pFlowPath5->out_user_pri_act;
		flow_config.actions.options.cos = pFlowPath5->out_user_priority;

		if(fc_db.controlFuc.loopbackMode_is_enabled && flow_dpa <= G3_LOOPBACK_P_END)
			flow_config.actions.dest.port = G3_LOOPBACK_P_NEWSPA;
		else
		{
			flow_config.actions.dest.port = flow_dpa;
			// to Wifi
			if((1<<(flow_config.actions.dest.port))&RTK_FC_ALL_MAC_WLANCPU_PORTMASK){
				flow_config.actions.options.masks.sw_id = TRUE;
				flow_config.actions.options.sw_id[0] = flow_egrWlanDevIdx;

			#if defined(CONFIG_SMP)
				{
					uint16 wifi_tx_dispatch_mode;
					uint8 cpu_idx;
					uint band;
					band = RTK_FC_HELPER_WLAN_DEVID_TO_BANDIDX(flow_egrWlanDevIdx);
					if(RTK_FC_MGR_DISPATCH_SMP_MODE_WIFI_TX_CPUID_GET(&cpu_idx, &wifi_tx_dispatch_mode, flowIdx>>1, (RTK_FC_MGR_DISPATCH_WLAN0_TX+band)) == SUCCESS)
					{
						if((wifi_tx_dispatch_mode == RTK_FC_DISPATCH_MODE_SMP_BY_HASH) || (wifi_tx_dispatch_mode == RTK_FC_DISPATCH_MODE_SMP_BY_RR))
						{
							flow_config.actions.dest.port = RTK_FC_MAC_PORT_CPU + cpu_idx;
							TRACE("WIFI TX dispatch is SMP mode, decide WIFI TX CPU port ID 0x%x by %s", flow_config.actions.dest.port, (wifi_tx_dispatch_mode == RTK_FC_DISPATCH_MODE_SMP_BY_HASH)?"HASH":"RR");
						}
					}
				}
			#endif
			}
		}
		
#if defined(CONFIG_FC_CA8277B_SERIES)
		if(fc_db.controlFuc.special_fast_forward_mode==1)
		{
			//For vxlan upstream
			if((pG3IgrExtraInfo->dualHdrType == RTK_FC_DUALTYPE_VXLAN && pG3IgrExtraInfo->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM && pG3IgrExtraInfo->vxlan_info.is_vxlanUS_innerFlow) || 
				(pG3IgrExtraInfo->dualHdrType == RTK_FC_DUALTYPE_VXLAN && pG3IgrExtraInfo->direction == RTK_FC_FLOW_DIRECTION_DOWNSTREAM && pG3IgrExtraInfo->vxlan_info.is_vxlanDS_outerFlow) 
				)
			{

				if(pG3IgrExtraInfo->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM)
				{
					int acl_ret;

					// Round-Robin Mode
					
					flow_config.actions.dest.port = fc_db.vxlan_fastFwd_RoundRobin_cpuPort_array[pG3IgrExtraInfo->vxlan_info.cpuPortIdx];
					DEBUG("[VXLAN]Upstream R-R index = %d use CPU port 0x%x\n",pG3IgrExtraInfo->vxlan_info.cpuPortIdx, flow_config.actions.dest.port);

					//  Extra
					if(pG3IgrExtraInfo->vxlan_info.useExtra==1)
					{
						DEBUG("[VXLAN][UPSTREAM]Use extra %d voq = %d",pG3IgrExtraInfo->vxlan_info.extraIndex, pG3IgrExtraInfo->vxlan_info.extraVOQ);
						flow_config.actions.options.masks.sw_id = TRUE;
						flow_config.actions.options.sw_id[0] = pG3IgrExtraInfo->vxlan_info.extraIndex;
						flow_config.actions.options.masks.cos = TRUE;
						flow_config.actions.options.cos = pG3IgrExtraInfo->vxlan_info.extraVOQ;
					}
					else
					{
						flow_config.actions.options.sw_id[0] = 0;
					}
					
					if(!pG3IgrExtraInfo->vxlan_info.useExtra && fc_db.vxlan_upStream_record[pG3IgrExtraInfo->vxlan_info.cpuPortIdx].isSet ==0)
					{
						ret = ca_ni_dma_lso_special_fastFwd_outerSport_setting(flow_config.actions.dest.port, pG3IgrExtraInfo->vxlan_info.outer_srcPort);
						if(ret == FAIL)
						{
							WARNING("[VXLAN]port %d set outer Sport failed ! swOnly!",flow_config.actions.dest.port);
							return CA_E_NOT_SUPPORT;
						}
					}
					
					if(pG3IgrExtraInfo->vxlan_info.useExtra ==0)
					{
						fc_db.vxlan_upStream_record[pG3IgrExtraInfo->vxlan_info.cpuPortIdx].cpuPort = flow_config.actions.dest.port;
					}
					else
					{
						fc_db.vxlan_extra_upStream_record[(pG3IgrExtraInfo->vxlan_info.extraIndex)].cpuPort = flow_config.actions.dest.port;
					}

					
					if(!pG3IgrExtraInfo->vxlan_info.useExtra && fc_db.vxlan_l3cls_upstream_isSet[pG3IgrExtraInfo->vxlan_info.cpuPortIdx] == 0)
					{
						rtk_fc_aclAndCf_reserved_vxlan_ffd_t vxlan_ffd;
						memset(&vxlan_ffd, 0, sizeof(vxlan_ffd));
						vxlan_ffd.lan_port_idx = pG3IgrExtraInfo->vxlan_info.cpuPortIdx;
						vxlan_ffd.l4_sport = pFlowPath5->in_l4_src_port;
						vxlan_ffd.l4_dport = pFlowPath5->in_l4_dst_port;
						vxlan_ffd.pkt_len_start = rtkScfg.smallest_packet_size;
						acl_ret = _rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CA_CLS_TYPE_VXLAN_FFD_US, &vxlan_ffd);
						if(acl_ret == FAILED){
							fc_db.vxlan_specialFastFwd_setup_l3cls=1;
							_rtk_rg_aclAndCfReservedRuleAdd(RTK_FC_ACLANDCF_RESERVED_TAIL_END, NULL);
							acl_ret = _rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CA_CLS_TYPE_VXLAN_FFD_US, &vxlan_ffd);
							if(acl_ret == FAILED)
								WARNING("fail to add vxlan ffd us cls");
						}
					}
					

				}
				else if(pG3IgrExtraInfo->direction == RTK_FC_FLOW_DIRECTION_DOWNSTREAM)
				{
					int acl_ret;
					int extra_len=0;

					flow_config.actions.dest.port = fc_db.vxlan_fastFwd_RoundRobin_cpuPort_array[pG3IgrExtraInfo->vxlan_info.cpuPortIdx];
					DEBUG("[VXLAN]Downstream RR index = %d use CPU port 0x%x\n",pG3IgrExtraInfo->vxlan_info.cpuPortIdx, flow_config.actions.dest.port);
					
					
					if(pG3IgrExtraInfo->vxlan_info.useExtra ==0)
					{
						fc_db.vxlan_downStream_record[pG3IgrExtraInfo->vxlan_info.cpuPortIdx].cpuPort = flow_config.actions.dest.port;
					}
					else
					{
						fc_db.vxlan_extra_downStream_record[(pG3IgrExtraInfo->vxlan_info.extraIndex)].cpuPort = flow_config.actions.dest.port;
					}
					
					if(pG3IgrExtraInfo->vxlan_info.ingress_inner_stag)
						extra_len += 4;
					if(pG3IgrExtraInfo->vxlan_info.ingress_inner_ctag)
						extra_len += 4;
					if(pG3IgrExtraInfo->vxlan_info.ingress_inner_pppoeTag)
						extra_len += 8;
					

					if(!pG3IgrExtraInfo->vxlan_info.useExtra && fc_db.vxlan_l3cls_downstream_isSet[pG3IgrExtraInfo->vxlan_info.cpuPortIdx] == 0)
					{
						rtk_fc_aclAndCf_reserved_vxlan_ffd_t vxlan_ffd;
						memset(&vxlan_ffd, 0, sizeof(vxlan_ffd));
						vxlan_ffd.lan_port_idx = pG3IgrExtraInfo->vxlan_info.cpuPortIdx;
						vxlan_ffd.l4_sport = pFlowPath5->in_l4_src_port;
						vxlan_ffd.l4_dport = pFlowPath5->in_l4_dst_port;
						vxlan_ffd.pkt_len_start = rtkScfg.smallest_packet_size+pG3IgrExtraInfo->vxlan_info.outer_tag_len+extra_len;
						acl_ret = _rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CA_CLS_TYPE_VXLAN_FFD_DS, &vxlan_ffd);
						if(acl_ret == FAILED){
							fc_db.vxlan_specialFastFwd_setup_l3cls=1;	//skip l2tp function
							_rtk_rg_aclAndCfReservedRuleAdd(RTK_FC_ACLANDCF_RESERVED_TAIL_END, NULL);
							acl_ret = _rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CA_CLS_TYPE_VXLAN_FFD_DS, &vxlan_ffd);
							if(acl_ret == FAILED)
								WARNING("fail to add vxlan ffd ds cls");
						}
					}
					// Extra
					// For all Extra vxlan downstream will go voq 1
					if(fc_db.vxlan_ds_extraCLS_isSet==0)
					{
						DEBUG("[VXLAN][DOWNSTREAM]Use extra and now will set downstream cls for voq 1");
						acl_ret = _rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CA_CLS_TYPE_VXLAN_FFD_EXTRA_DS, NULL);
						if(acl_ret == FAILED){
							fc_db.vxlan_specialFastFwd_setup_l3cls=1;	//skip l2tp function
							_rtk_rg_aclAndCfReservedRuleAdd(RTK_FC_ACLANDCF_RESERVED_TAIL_END, NULL);
							acl_ret = _rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CA_CLS_TYPE_VXLAN_FFD_EXTRA_DS, NULL);
							if(acl_ret == FAILED)
								WARNING("fail to add vxlan ffd ds cls");
						}
					}


									
				}
				

				
			}
		}

		
#endif
		flow_config.actions.options.masks.sw_id = TRUE;
		flow_config.actions.options.sw_id[1] = (pFlowPath5->in_intf_idx<<8 | pFlowPath5->out_intf_idx);


		_rtk_fc_g3FlowIdDecision(flow, pG3IgrExtraInfo, &flowIdActMask, &flowId);
#if defined(CONFIG_FC_CA8277B_SERIES)
		// policer1 is for upstream GemId, there should no policer function use flow policer1 action

		// policer_2: flow meter + flow mib (flow_meter_mib_conf_dependence must be enabled)
		flow_config.actions.options.masks.flow_id2 = pFlowPath5->out_share_meter_act;
		flow_config.actions.options.flow_id2 = pFlowPath5->out_share_meter_idx + G3_FLOW_POLICER_IDXSHIFT_FLOWMTR;

		// policer_3: host pure logging (decide by _rtk_fc_g3FlowIdDecision)
		flow_config.actions.options.masks.flow_id3 = flowIdActMask;
		flow_config.actions.options.flow_id3 = flowId;

#else
		flow_config.actions.options.masks.flow_id = flowIdActMask;
		flow_config.actions.options.flow_id = flowId;
#endif

#if defined(CONFIG_FC_CA8277B_SERIES)
#if defined(CONFIG_RG_G3_WAN_PORT_INDEX)&&(CONFIG_RG_G3_WAN_PORT_INDEX==7)
		if(pFlowPath5->out_stream_idx_act && fc_db.streamidTbl[pFlowPath5->out_stream_idx].valid
			&& (fc_db.wanPortMask.portmask & (1 << flow_config.actions.dest.port))) {
			flow_config.actions.options.masks.gemId_mapping_mode = TRUE;
			flow_config.actions.options.masks.tcont_id = TRUE;
			flow_config.actions.options.masks.gem_id = TRUE;
			flow_config.actions.options.masks.cos = TRUE;
			
			flow_config.actions.options.gemId_mapping_mode = 0;
			flow_config.actions.options.tcont_id = fc_db.streamidTbl[pFlowPath5->out_stream_idx].ldpid;
			flow_config.actions.options.gem_id = fc_db.streamidTbl[pFlowPath5->out_stream_idx].gemid;
			flow_config.actions.options.cos = fc_db.streamidTbl[pFlowPath5->out_stream_idx].cos;
		}
#endif
#endif

	}
	else if(flow->in_path == FB_PATH_34){
		rtk_rg_asic_path3_entry_t *pFlowPath3 = (rtk_rg_asic_path3_entry_t *)flow;
		if( (1<<flow_spa) & fc_db.wanPortMask.portmask)
			flow_config.key_type = RG_CA_FLOW_UC5TUPLE_DS;
		else
			flow_config.key_type = RG_CA_FLOW_UC5TUPLE_US;

		// Key: L2
		if(fc_db.controlFuc.loopbackMode_is_enabled && fromLanPort)
		{
			flow_config.key.l2_key.o_lspid = G3_LOOPBACK_P_NEWSPA;

			flow_config.key.l2_key.outer_vlan.vid = flow_spa + G3_LOOPBACK_UPSTREAM_VID_BASE;
			flow_config.key.l2_key.outer_vlan.pri = 0;
			flow_config.key.l2_key.outer_vlan.dei = 0;
			flow_config.key.l2_key.outer_vlan.tpid = CVLAN_TPID;
			if((pFlowPath3->in_stagif + pFlowPath3->in_ctagif) == 2){		// double tag
				flow_config.key.l2_key.inner_vlan.vid = pG3IgrExtraInfo->svlan_id;
				flow_config.key.l2_key.inner_vlan.pri = pG3IgrExtraInfo->svlan_pri;
				flow_config.key.l2_key.inner_vlan.dei = pG3IgrExtraInfo->igr_svlan_dei;
				flow_config.key.l2_key.inner_vlan.tpid = fc_db.systemGlobal.stagTPID[pG3IgrExtraInfo->igr_svlan_tpid_sel];
			}else{													// single tag or untag
				if(pFlowPath3->in_stagif){
					flow_config.key.l2_key.inner_vlan.vid = pG3IgrExtraInfo->svlan_id;
					flow_config.key.l2_key.inner_vlan.pri = pG3IgrExtraInfo->svlan_pri;
					flow_config.key.l2_key.inner_vlan.dei = pG3IgrExtraInfo->igr_svlan_dei;
					flow_config.key.l2_key.inner_vlan.tpid = fc_db.systemGlobal.stagTPID[pG3IgrExtraInfo->igr_svlan_tpid_sel];
				}
				if(pFlowPath3->in_ctagif){
					flow_config.key.l2_key.inner_vlan.vid = pG3IgrExtraInfo->cvlan_id;
					flow_config.key.l2_key.inner_vlan.pri = pFlowPath3->in_cvlan_pri;
					flow_config.key.l2_key.inner_vlan.dei = pG3IgrExtraInfo->igr_cvlan_cfi;
					flow_config.key.l2_key.inner_vlan.tpid = CVLAN_TPID;
				}
			}
		}
		else
		{
			flow_config.key.l2_key.o_lspid = flow_spa;	// force enabled to support unknown sa trap by main hash

			if((pFlowPath3->in_stagif + pFlowPath3->in_ctagif) == 2){		// double tag
				flow_config.key.l2_key.outer_vlan.vid = pG3IgrExtraInfo->svlan_id;
				flow_config.key.l2_key.outer_vlan.pri = pG3IgrExtraInfo->svlan_pri;
				flow_config.key.l2_key.outer_vlan.dei = pG3IgrExtraInfo->igr_svlan_dei;
				flow_config.key.l2_key.outer_vlan.tpid = fc_db.systemGlobal.stagTPID[pG3IgrExtraInfo->igr_svlan_tpid_sel];
				flow_config.key.l2_key.inner_vlan.vid = pG3IgrExtraInfo->cvlan_id;
				flow_config.key.l2_key.inner_vlan.pri = pFlowPath3->in_cvlan_pri;
				flow_config.key.l2_key.inner_vlan.dei = pG3IgrExtraInfo->igr_cvlan_cfi;
				flow_config.key.l2_key.inner_vlan.tpid = CVLAN_TPID;
			}else{													// single tag or untag
				if(pFlowPath3->in_stagif){
					flow_config.key.l2_key.outer_vlan.vid = pG3IgrExtraInfo->svlan_id;
					flow_config.key.l2_key.outer_vlan.pri = pG3IgrExtraInfo->svlan_pri;
					flow_config.key.l2_key.outer_vlan.dei = pG3IgrExtraInfo->igr_svlan_dei;
					flow_config.key.l2_key.outer_vlan.tpid = fc_db.systemGlobal.stagTPID[pG3IgrExtraInfo->igr_svlan_tpid_sel];
				}
				if(pFlowPath3->in_ctagif){
					flow_config.key.l2_key.outer_vlan.vid = pG3IgrExtraInfo->cvlan_id;
					flow_config.key.l2_key.outer_vlan.pri = pFlowPath3->in_cvlan_pri;
					flow_config.key.l2_key.outer_vlan.dei = pG3IgrExtraInfo->igr_cvlan_cfi;
					flow_config.key.l2_key.outer_vlan.tpid = CVLAN_TPID;
				}
			}
		}
		if(pFlowPath3->in_pppoeif)
		{
			flow_config.key.l2_key.pppoe_type = 2; //session
			flow_config.key.l2_key.pppoe_session_id = pG3IgrExtraInfo->pppoe_session_id;
		}
		// Key: L3
		flow_config.key.l3_key.ip_valid = TRUE;
		flow_config.key.l3_key.ip_version = pFlowPath3->in_ipv4_or_ipv6?6:4;
		flow_config.key.l3_key.ip_protocol = pFlowPath3->in_l4proto_num;
		if(pFlowPath3->in_tos_check)
		{
			flow_config.key.l3_key.dscp = (pFlowPath3->in_tos >>2)&0x3f;
			flow_config.key.l3_key.ecn = pFlowPath3->in_tos&0x3;
		}

		if(pFlowPath3->in_ipv4_or_ipv6){
			flow_config.key.l3_key.ip_sa.afi = CA_IPV6;
			flow_config.key.l3_key.ip_da.afi = CA_IPV6;
			flow_config.key.l3_key.ip_sa.addr_len = 128;
			flow_config.key.l3_key.ip_da.addr_len = 128;
			flow_config.key.l3_key.ip_sa.ip_addr.ipv6_addr[0] =  ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Saddr.s6_addr[0]);
			flow_config.key.l3_key.ip_sa.ip_addr.ipv6_addr[1] =  ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Saddr.s6_addr[4]);
			flow_config.key.l3_key.ip_sa.ip_addr.ipv6_addr[2] =  ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Saddr.s6_addr[8]);
			flow_config.key.l3_key.ip_sa.ip_addr.ipv6_addr[3] =  ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Saddr.s6_addr[12]);
			flow_config.key.l3_key.ip_da.ip_addr.ipv6_addr[0] =  ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Daddr.s6_addr[0]);
			flow_config.key.l3_key.ip_da.ip_addr.ipv6_addr[1] =  ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Daddr.s6_addr[4]);
			flow_config.key.l3_key.ip_da.ip_addr.ipv6_addr[2] =  ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Daddr.s6_addr[8]);
			flow_config.key.l3_key.ip_da.ip_addr.ipv6_addr[3] =  ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Daddr.s6_addr[12]);

		}else{
			flow_config.key.l3_key.ip_sa.afi = CA_IPV4;
			flow_config.key.l3_key.ip_da.afi = CA_IPV4;
			flow_config.key.l3_key.ip_sa.ip_addr.ipv4_addr = pFlowPath3->in_src_ipv4_addr;
			flow_config.key.l3_key.ip_da.ip_addr.ipv4_addr = pFlowPath3->in_dst_ipv4_addr;
			flow_config.key.l3_key.ip_sa.addr_len = 32;
			flow_config.key.l3_key.ip_da.addr_len = 32;
		}

		// Key: L4
		flow_config.key.l4_key.src_l4_port = pFlowPath3->in_l4_src_port;
		flow_config.key.l4_key.dst_l4_port = pFlowPath3->in_l4_dst_port;


		// Action: L2
#if defined(CONFIG_FC_CAG3_SERIES)
		if(pFlowPath3->out_stream_idx_act){
			// special case: support stream idx by svlan id
			outerSVlanEn = TRUE;
			outerSVlanId = pFlowPath3->out_stream_idx;
			outerSVlanPri = pFlowPath3->out_cpri;
			outerSVlanDei = pG3IgrExtraInfo->egr_svlan_dei;
		}
		else if(fc_db.controlFuc.loopbackMode_is_enabled && flow_dpa <= G3_LOOPBACK_P_END)
		{
			outerSVlanEn = TRUE;
			outerSVlanId = G3_LOOPBACK_DOWNSTREAM_VID(flow_dpa, pFlowPath3->out_user_priority);
			outerSVlanPri = 0;
			outerSVlanDei = 0;
			outterTPID = CVLAN_TPID;
		}
		else
#endif
		{
			//normal case: support double tag
			if(pFlowPath3->out_svid_format_act){
				outerSVlanEn = TRUE;
				outerSVlanId = pFlowPath3->out_svlan_id;
				outerSVlanPri = pFlowPath3->out_spri;
				outerSVlanDei = pG3IgrExtraInfo->egr_svlan_dei;
			}
		}

		if((outerSVlanEn + pFlowPath3->out_cvid_format_act) == 2){		// double tag
			flow_config.actions.options.masks.outer_vlan_act = TRUE;
			flow_config.actions.options.masks.outer_dot1p = TRUE;
			flow_config.actions.options.masks.outer_dei = TRUE;
			flow_config.actions.options.masks.outer_tpid = TRUE;
			flow_config.actions.options.outer_vlan_act = CA_FLOW_VLAN_ACTION_SET;
			flow_config.actions.options.outer_vid = outerSVlanId;
			flow_config.actions.options.outer_dot1p = outerSVlanPri;
			flow_config.actions.options.outer_dei = outerSVlanDei;
			flow_config.actions.options.outer_tpid = outterTPID;

			flow_config.actions.options.masks.inner_vlan_act = TRUE;
			flow_config.actions.options.masks.inner_dot1p = TRUE;
			flow_config.actions.options.masks.inner_dei = TRUE;
			flow_config.actions.options.masks.inner_tpid = TRUE;
			flow_config.actions.options.inner_vlan_act = CA_FLOW_VLAN_ACTION_SET;
			flow_config.actions.options.inner_vid = pFlowPath3->out_cvlan_id;
			flow_config.actions.options.inner_dot1p = pFlowPath3->out_cpri;
			flow_config.actions.options.inner_dei = pG3IgrExtraInfo->egr_cvlan_cfi;
			flow_config.actions.options.inner_tpid = CVLAN_TPID;
		}else{
			if(outerSVlanEn){
				flow_config.actions.options.masks.outer_vlan_act = TRUE;
				flow_config.actions.options.masks.outer_dot1p = TRUE;
				flow_config.actions.options.masks.outer_dei = TRUE;
				flow_config.actions.options.masks.outer_tpid = TRUE;
				flow_config.actions.options.outer_vlan_act = CA_FLOW_VLAN_ACTION_SET;
				flow_config.actions.options.outer_vid = outerSVlanId;
				flow_config.actions.options.outer_dot1p = outerSVlanPri;
				flow_config.actions.options.outer_dei = outerSVlanDei;
				flow_config.actions.options.outer_tpid = outterTPID;
			}else if(pFlowPath3->out_cvid_format_act){
				flow_config.actions.options.masks.outer_vlan_act = TRUE;
				flow_config.actions.options.masks.outer_dot1p = TRUE;
				flow_config.actions.options.masks.outer_dei = TRUE;
				flow_config.actions.options.masks.outer_tpid = TRUE;
				flow_config.actions.options.outer_vlan_act = CA_FLOW_VLAN_ACTION_SET;
				flow_config.actions.options.outer_vid = pFlowPath3->out_cvlan_id;
				flow_config.actions.options.outer_dot1p = pFlowPath3->out_cpri;
				flow_config.actions.options.outer_dei = pG3IgrExtraInfo->egr_cvlan_cfi;
				flow_config.actions.options.outer_tpid = CVLAN_TPID;
			}else{
				flow_config.actions.options.masks.outer_vlan_act = TRUE;
				flow_config.actions.options.outer_vlan_act = CA_FLOW_VLAN_ACTION_POP;
			}
		}
		
#if defined(CONFIG_FC_CA8277B_SERIES)
		if(fc_db.controlFuc.special_fast_forward_mode==1)
		{
			//For vxlan ds outter header
			if(pG3IgrExtraInfo->dualHdrType == RTK_FC_DUALTYPE_VXLAN && pG3IgrExtraInfo->direction == RTK_FC_FLOW_DIRECTION_DOWNSTREAM && pG3IgrExtraInfo->vxlan_info.is_vxlanDS_outerFlow)
			{
				if((outerSVlanEn + pFlowPath3->out_cvid_format_act) == 2)
				{
					flow_config.actions.options.outer_vlan_act = CA_FLOW_VLAN_ACTION_POP;
					flow_config.actions.options.inner_vlan_act = CA_FLOW_VLAN_ACTION_POP;
				}else
					flow_config.actions.options.outer_vlan_act = CA_FLOW_VLAN_ACTION_POP;

				flow_config.actions.options.masks.egress_pppoe_action = TRUE;
				flow_config.actions.options.pppoe_session_id =	0;
			}
			// VXLAN DOWN STREAM INNER FLOW!!
			if (pG3IgrExtraInfo->dualHdrType == RTK_FC_DUALTYPE_VXLAN && pG3IgrExtraInfo->direction == RTK_FC_FLOW_DIRECTION_DOWNSTREAM && !pG3IgrExtraInfo->vxlan_info.is_vxlanDS_outerFlow)
			{
				//For vxlan ds inner header
				flow_config.key.l2_key.o_lspid = 0; // For No HeaderA
				flow_config.key.flow_id = 0;
				//TRACE("[VXLAN]Round-robin downstream inner-flow use cpu-port 0x%x for o_lspid",flow_config.key.l2_key.o_lspid);


			}

			if (pG3IgrExtraInfo->dualHdrType == RTK_FC_DUALTYPE_VXLAN && pG3IgrExtraInfo->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM && pG3IgrExtraInfo->vxlan_info.is_vxlanUS_outerFlow)
			{
				flow_config.key.l2_key.o_lspid = 0; // For No HeaderA
				//TRACE("[VXLAN]Round-robin upstream outer-flow use cpu-port %d for o_lspid",flow_config.key.l2_key.o_lspid);
			}
		}
		
#endif

		if(pFlowPath3->out_smac_trans){
			flow_config.actions.options.masks.decrement_ttl = TRUE;
			flow_config.actions.options.masks.mac_sa = TRUE;
			memcpy(&flow_config.actions.options.mac_sa, &fc_db.netifTbl[NETIF_HWTOSW(pFlowPath3->out_intf_idx)].intf.gateway_mac_addr, ETH_ALEN);
		}else{
			flow_config.actions.options.masks.decrement_ttl = FALSE;
		}

		if(pFlowPath3->out_dmac_trans){

			flow_config.actions.options.masks.mac_da = TRUE;
			memcpy(&flow_config.actions.options.mac_da, &fc_db.lutTbl[pG3IgrExtraInfo->lutEgrDaIdx]->l2Addr, ETH_ALEN);
		}

		// Action: L3
		flow_config.actions.options.masks.dscp = pFlowPath3->out_dscp_act;
		flow_config.actions.options.dscp = pFlowPath3->out_dscp;


		// Action: Others
		flow_config.actions.options.masks.cos = pFlowPath3->out_user_pri_act;
		flow_config.actions.options.cos = pFlowPath3->out_user_priority;

		if(fc_db.controlFuc.loopbackMode_is_enabled && flow_dpa <= G3_LOOPBACK_P_END)
			flow_config.actions.dest.port = G3_LOOPBACK_P_NEWSPA;
		else
		{
			if(pFlowPath3->out_uc_lut_lookup)
				flow_config.actions.dest.port = flow_dpa;
			else
			{
				if(pFlowPath3->out_portmask)
					flow_config.actions.dest.port = __ffs(pFlowPath3->out_portmask);
				else if(pFlowPath3->out_ext_portmask_idx)
				{
					uint32 macPort, macExtPort;

					// In G3 platform out_ext_portmask_idx is wlanIdMask
					RTK_FC_HELPER_WLAN_DEVID_TO_PORTID(__ffs(fc_db.extPortTbl[pFlowPath3->out_ext_portmask_idx].extPortEnt.extpmask), &macPort, &macExtPort);
					flow_config.actions.dest.port = macPort;
				}
				else
					flow_config.actions.dest.port = flow_dpa;
			}

			// to Wifi
			if((1<<(flow_config.actions.dest.port))&RTK_FC_ALL_MAC_WLANCPU_PORTMASK){
				rtk_fc_wlan_devidx_t wlanDevIdx;
				if(pFlowPath3->out_uc_lut_lookup)
					wlanDevIdx = flow_egrWlanDevIdx;
				else
				{
					if(pFlowPath3->out_ext_portmask_idx)
					{
						// In G3 platform out_ext_portmask_idx is wlanIdMask
						wlanDevIdx = __ffs(fc_db.extPortTbl[pFlowPath3->out_ext_portmask_idx].extPortEnt.extpmask);
					}
					else
						wlanDevIdx = flow_egrWlanDevIdx;
				}
				flow_config.actions.options.masks.sw_id = TRUE;
				flow_config.actions.options.sw_id[0] = wlanDevIdx;

			#if defined(CONFIG_SMP)
				{
					uint16 wifi_tx_dispatch_mode;
					uint8 cpu_idx;
					uint band;
					band = RTK_FC_HELPER_WLAN_DEVID_TO_BANDIDX(wlanDevIdx);
					if(RTK_FC_MGR_DISPATCH_SMP_MODE_WIFI_TX_CPUID_GET(&cpu_idx, &wifi_tx_dispatch_mode, flowIdx>>1, (RTK_FC_MGR_DISPATCH_WLAN0_TX+band)) == SUCCESS)
					{
						if((wifi_tx_dispatch_mode == RTK_FC_DISPATCH_MODE_SMP_BY_HASH) || (wifi_tx_dispatch_mode == RTK_FC_DISPATCH_MODE_SMP_BY_RR))
						{
							flow_config.actions.dest.port = RTK_FC_MAC_PORT_CPU + cpu_idx;
							TRACE("WIFI TX dispatch is SMP mode, decide WIFI TX CPU port ID 0x%x by %s", flow_config.actions.dest.port, (wifi_tx_dispatch_mode == RTK_FC_DISPATCH_MODE_SMP_BY_HASH)?"HASH":"RR");
						}
					}
				}
			#endif
			}
		}
	
		flow_config.actions.options.masks.sw_id = TRUE;
		flow_config.actions.options.sw_id[1] = (pFlowPath3->in_intf_idx<<8 | pFlowPath3->out_intf_idx);
		
#if defined(CONFIG_FC_CA8277B_SERIES)
		// L2 VXLAN Setting
		if(fc_db.controlFuc.special_fast_forward_mode==1)
		{
			//For vxlan trap to CPU flow setting
			if((pG3IgrExtraInfo->dualHdrType == RTK_FC_DUALTYPE_VXLAN && pG3IgrExtraInfo->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM && pG3IgrExtraInfo->vxlan_info.is_vxlanUS_innerFlow) || 
				(pG3IgrExtraInfo->dualHdrType == RTK_FC_DUALTYPE_VXLAN && pG3IgrExtraInfo->direction == RTK_FC_FLOW_DIRECTION_DOWNSTREAM && pG3IgrExtraInfo->vxlan_info.is_vxlanDS_outerFlow) 
				)
			{

				if(pG3IgrExtraInfo->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM)
				{
					// Round-Robin Mode
					
					flow_config.actions.dest.port = fc_db.vxlan_fastFwd_RoundRobin_cpuPort_array[pG3IgrExtraInfo->vxlan_info.cpuPortIdx];
					DEBUG("[L2 VXLAN]Upstream R-R index = %d use CPU port 0x%x  voq = %d\n",pG3IgrExtraInfo->vxlan_info.cpuPortIdx, flow_config.actions.dest.port, pG3IgrExtraInfo->vxlan_info.extraVOQ);

					//L2 VXLAN always use Extra
					flow_config.actions.options.masks.sw_id = TRUE;
					flow_config.actions.options.sw_id[0] = pG3IgrExtraInfo->vxlan_info.extraIndex;
					DEBUG("[L2 VXLAN][UPSTREAM]Use extra %d",pG3IgrExtraInfo->vxlan_info.extraIndex);
					flow_config.actions.options.masks.cos = TRUE;
					flow_config.actions.options.cos = pG3IgrExtraInfo->vxlan_info.extraVOQ;
				
					
					fc_db.vxlan_extra_upStream_record[(pG3IgrExtraInfo->vxlan_info.extraIndex)].cpuPort = flow_config.actions.dest.port;
					
				}
				else if(pG3IgrExtraInfo->direction == RTK_FC_FLOW_DIRECTION_DOWNSTREAM)
				{
					int acl_ret;

					flow_config.actions.dest.port = fc_db.vxlan_fastFwd_RoundRobin_cpuPort_array[pG3IgrExtraInfo->vxlan_info.cpuPortIdx];
					DEBUG("[VXLAN]Downstream RR index = %d use CPU port 0x%x\n",pG3IgrExtraInfo->vxlan_info.cpuPortIdx, flow_config.actions.dest.port);
					
					fc_db.vxlan_extra_downStream_record[(pG3IgrExtraInfo->vxlan_info.extraIndex)].cpuPort = flow_config.actions.dest.port;
					
					// Extra
					// For all Extra vxlan downstream will go voq 1
					if(fc_db.vxlan_ds_extraCLS_isSet==0)
					{
						DEBUG("[VXLAN][DOWNSTREAM]Use extra and now will set downstream cls for voq 1");
						acl_ret = _rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CA_CLS_TYPE_VXLAN_FFD_EXTRA_DS, NULL);
						if(acl_ret == FAILED){
							fc_db.vxlan_specialFastFwd_setup_l3cls=1;	//skip l2tp function
							_rtk_rg_aclAndCfReservedRuleAdd(RTK_FC_ACLANDCF_RESERVED_TAIL_END, NULL);
							acl_ret = _rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CA_CLS_TYPE_VXLAN_FFD_EXTRA_DS, NULL);
							if(acl_ret == FAILED)
								WARNING("fail to add vxlan ffd ds cls");
						}
					}
				}
			}
		}

				
#endif

		_rtk_fc_g3FlowIdDecision(flow, pG3IgrExtraInfo, &flowIdActMask, &flowId);
#if defined(CONFIG_FC_CA8277B_SERIES)
		// policer1 is for upstream GemId, there should no policer function use flow policer1 action

		// policer_2: flow meter + flow mib (flow_meter_mib_conf_dependence must be enabled)
		flow_config.actions.options.masks.flow_id2 = pFlowPath3->out_share_meter_act;
		flow_config.actions.options.flow_id2 = pFlowPath3->out_share_meter_idx + G3_FLOW_POLICER_IDXSHIFT_FLOWMTR;

		// policer_3: host pure logging (decide by _rtk_fc_g3FlowIdDecision)
		flow_config.actions.options.masks.flow_id3 = flowIdActMask;
		flow_config.actions.options.flow_id3 = flowId;

#else
		flow_config.actions.options.masks.flow_id = flowIdActMask;
		flow_config.actions.options.flow_id = flowId;
#endif

#if defined(CONFIG_FC_CA8277B_SERIES)
#if defined(CONFIG_RG_G3_WAN_PORT_INDEX)&&(CONFIG_RG_G3_WAN_PORT_INDEX==7)
		if(pFlowPath3->out_stream_idx_act && fc_db.streamidTbl[pFlowPath3->out_stream_idx].valid
			&& (fc_db.wanPortMask.portmask & (1 << flow_config.actions.dest.port))) {
			flow_config.actions.options.masks.gemId_mapping_mode = TRUE;
			flow_config.actions.options.masks.tcont_id = TRUE;
			flow_config.actions.options.masks.gem_id = TRUE;
			flow_config.actions.options.masks.cos = TRUE;
			
			flow_config.actions.options.gemId_mapping_mode = 0;
			flow_config.actions.options.tcont_id = fc_db.streamidTbl[pFlowPath3->out_stream_idx].ldpid;
			flow_config.actions.options.gem_id = fc_db.streamidTbl[pFlowPath3->out_stream_idx].gemid;
			flow_config.actions.options.cos = fc_db.streamidTbl[pFlowPath3->out_stream_idx].cos;
		}
#endif
#endif

	}
	else if(flow->in_path == FB_PATH_12){
		rtk_rg_asic_path1_entry_t *pFlowPath1 = (rtk_rg_asic_path1_entry_t *)flow;
		flow_config.key_type = RG_CA_FLOW_UC2TUPLE_BRIDGE;

		// Key: L2
		if(fc_db.controlFuc.loopbackMode_is_enabled && fromLanPort)
		{
			flow_config.key.l2_key.outer_vlan.vid = flow_spa + G3_LOOPBACK_UPSTREAM_VID_BASE;
			flow_config.key.l2_key.outer_vlan.pri = 0;
			flow_config.key.l2_key.outer_vlan.dei = 0;
			flow_config.key.l2_key.outer_vlan.tpid = CVLAN_TPID;
			if((pFlowPath1->in_stagif + pFlowPath1->in_ctagif) == 2){		// double tag
				flow_config.key.l2_key.inner_vlan.vid = pG3IgrExtraInfo->svlan_id;
				flow_config.key.l2_key.inner_vlan.pri = pG3IgrExtraInfo->svlan_pri;
				flow_config.key.l2_key.inner_vlan.dei = pG3IgrExtraInfo->igr_svlan_dei;
				flow_config.key.l2_key.inner_vlan.tpid = fc_db.systemGlobal.stagTPID[pG3IgrExtraInfo->igr_svlan_tpid_sel];
			}else{													// single tag or untag
				if(pFlowPath1->in_stagif){
					flow_config.key.l2_key.inner_vlan.vid = pG3IgrExtraInfo->svlan_id;
					flow_config.key.l2_key.inner_vlan.pri = pG3IgrExtraInfo->svlan_pri;
					flow_config.key.l2_key.inner_vlan.dei = pG3IgrExtraInfo->igr_svlan_dei;
					flow_config.key.l2_key.inner_vlan.tpid = fc_db.systemGlobal.stagTPID[pG3IgrExtraInfo->igr_svlan_tpid_sel];
				}
				if(pFlowPath1->in_ctagif){
					flow_config.key.l2_key.inner_vlan.vid = pG3IgrExtraInfo->cvlan_id;
					flow_config.key.l2_key.inner_vlan.pri = pFlowPath1->in_cvlan_pri;
					flow_config.key.l2_key.inner_vlan.dei = pG3IgrExtraInfo->igr_cvlan_cfi;
					flow_config.key.l2_key.inner_vlan.tpid = CVLAN_TPID;
				}
			}
			flow_config.key.l2_key.o_lspid = G3_LOOPBACK_P_NEWSPA;
		}
		else
		{
			if((pFlowPath1->in_stagif + pFlowPath1->in_ctagif) == 2){		// double tag
				flow_config.key.l2_key.outer_vlan.vid = pG3IgrExtraInfo->svlan_id;
				flow_config.key.l2_key.outer_vlan.pri = pG3IgrExtraInfo->svlan_pri;
				flow_config.key.l2_key.outer_vlan.dei = pG3IgrExtraInfo->igr_svlan_dei;
				flow_config.key.l2_key.outer_vlan.tpid = fc_db.systemGlobal.stagTPID[pG3IgrExtraInfo->igr_svlan_tpid_sel];
				flow_config.key.l2_key.inner_vlan.vid = pG3IgrExtraInfo->cvlan_id;
				flow_config.key.l2_key.inner_vlan.pri = pFlowPath1->in_cvlan_pri;
				flow_config.key.l2_key.inner_vlan.dei = pG3IgrExtraInfo->igr_cvlan_cfi;
				flow_config.key.l2_key.inner_vlan.tpid = CVLAN_TPID;
			}else{													// single tag or untag
				if(pFlowPath1->in_stagif){
					flow_config.key.l2_key.outer_vlan.vid = pG3IgrExtraInfo->svlan_id;
					flow_config.key.l2_key.outer_vlan.pri = pG3IgrExtraInfo->svlan_pri;
					flow_config.key.l2_key.outer_vlan.dei = pG3IgrExtraInfo->igr_svlan_dei;
					flow_config.key.l2_key.outer_vlan.tpid = fc_db.systemGlobal.stagTPID[pG3IgrExtraInfo->igr_svlan_tpid_sel];
				}
				if(pFlowPath1->in_ctagif){
					flow_config.key.l2_key.outer_vlan.vid = pG3IgrExtraInfo->cvlan_id;
					flow_config.key.l2_key.outer_vlan.pri = pFlowPath1->in_cvlan_pri;
					flow_config.key.l2_key.outer_vlan.dei = pG3IgrExtraInfo->igr_cvlan_cfi;
					flow_config.key.l2_key.outer_vlan.tpid = CVLAN_TPID;
				}
			}
		//if(pFlowPath1->in_spa_check)	// force enabled to support unknown sa trap by main hash
			flow_config.key.l2_key.o_lspid = pFlowPath1->in_spa;
		}
		if(fc_db.lutTbl[pFlowPath1->in_smac_lut_idx]==NULL)
		{
			WARNING("lutTbl[%d] is NULL!", pFlowPath1->in_smac_lut_idx);
			return CA_E_ERROR;
		}
		else
			memcpy(&flow_config.key.l2_key.mac_sa, &fc_db.lutTbl[pFlowPath1->in_smac_lut_idx]->l2Addr, ETH_ALEN);
		
		if(fc_db.lutTbl[pFlowPath1->in_dmac_lut_idx]==NULL)
		{
			WARNING("lutTbl[%d] is NULL!", pFlowPath1->in_smac_lut_idx);
			return CA_E_ERROR;
		}
		else
			memcpy(&flow_config.key.l2_key.mac_da, &fc_db.lutTbl[pFlowPath1->in_dmac_lut_idx]->l2Addr, ETH_ALEN);

		// Key: L3
		if(pFlowPath1->in_tos_check)
		{
			flow_config.key.l3_key.dscp = (pFlowPath1->in_tos >>2)&0x3f;
			flow_config.key.l3_key.ecn = pFlowPath1->in_tos&0x3;
		}

		// Action: L2
#if defined(CONFIG_FC_CAG3_SERIES)
		if((pFlowPath1->in_spa!=RTK_FC_MAC_PORT_PON) && pFlowPath1->in_out_stream_idx_check_act){
			// special case: support stream idx by svlan id
			outerSVlanEn = TRUE;
			outerSVlanId = pFlowPath1->in_out_stream_idx;
			outerSVlanPri = pFlowPath1->out_cpri;
			outerSVlanDei = pG3IgrExtraInfo->egr_svlan_dei;
		}
		else if(fc_db.controlFuc.loopbackMode_is_enabled && flow_dpa <= G3_LOOPBACK_P_END)
		{
			outerSVlanEn = TRUE;
			outerSVlanId = G3_LOOPBACK_DOWNSTREAM_VID(flow_dpa, pFlowPath1->out_user_priority);
			outerSVlanPri = 0;
			outerSVlanDei = 0;
			outterTPID = CVLAN_TPID;
		}
		else
#endif
		{
			//normal case: support double tag
			if(pFlowPath1->out_svid_format_act){
				outerSVlanEn = TRUE;
				outerSVlanId = pFlowPath1->out_svlan_id;
				outerSVlanPri = pFlowPath1->out_spri;
				outerSVlanDei = pG3IgrExtraInfo->egr_svlan_dei;
			}
		}
		if( (outerSVlanEn + pFlowPath1->out_cvid_format_act) == 2){		// double tag
			flow_config.actions.options.masks.outer_vlan_act = TRUE;
			flow_config.actions.options.masks.outer_dot1p = TRUE;
			flow_config.actions.options.masks.outer_dei = TRUE;
			flow_config.actions.options.masks.outer_tpid = TRUE;
			flow_config.actions.options.outer_vlan_act = CA_FLOW_VLAN_ACTION_SET;
			flow_config.actions.options.outer_vid = outerSVlanId;
			flow_config.actions.options.outer_dot1p = outerSVlanPri;
			flow_config.actions.options.outer_dei = outerSVlanDei;
			flow_config.actions.options.outer_tpid = outterTPID;

			flow_config.actions.options.masks.inner_vlan_act = TRUE;
			flow_config.actions.options.masks.inner_dot1p = TRUE;
			flow_config.actions.options.masks.inner_dei = TRUE;
			flow_config.actions.options.masks.inner_tpid = TRUE;
			flow_config.actions.options.inner_vlan_act = CA_FLOW_VLAN_ACTION_SET;
			flow_config.actions.options.inner_vid = pFlowPath1->out_cvlan_id;
			flow_config.actions.options.inner_dot1p = pFlowPath1->out_cpri;
			flow_config.actions.options.inner_dei = pG3IgrExtraInfo->egr_cvlan_cfi;
			flow_config.actions.options.inner_tpid = CVLAN_TPID;
		}else{
			if(outerSVlanEn){
				flow_config.actions.options.masks.outer_vlan_act = TRUE;
				flow_config.actions.options.masks.outer_dot1p = TRUE;
				flow_config.actions.options.masks.outer_dei = TRUE;
				flow_config.actions.options.masks.outer_tpid = TRUE;
				flow_config.actions.options.outer_vlan_act = CA_FLOW_VLAN_ACTION_SET;
				flow_config.actions.options.outer_vid = outerSVlanId;
				flow_config.actions.options.outer_dot1p = outerSVlanPri;
				flow_config.actions.options.outer_dei = outerSVlanDei;
				flow_config.actions.options.outer_tpid = outterTPID;
			}else if(pFlowPath1->out_cvid_format_act){
				flow_config.actions.options.masks.outer_vlan_act = TRUE;
				flow_config.actions.options.masks.outer_dot1p = TRUE;
				flow_config.actions.options.masks.outer_dei = TRUE;
				flow_config.actions.options.masks.outer_tpid = TRUE;
				flow_config.actions.options.outer_vlan_act = CA_FLOW_VLAN_ACTION_SET;
				flow_config.actions.options.outer_vid = pFlowPath1->out_cvlan_id;
				flow_config.actions.options.outer_dot1p = pFlowPath1->out_cpri;
				flow_config.actions.options.outer_dei = pG3IgrExtraInfo->egr_cvlan_cfi;
				flow_config.actions.options.outer_tpid = CVLAN_TPID;
			}else{
				flow_config.actions.options.masks.outer_vlan_act = TRUE;
				flow_config.actions.options.outer_vlan_act = CA_FLOW_VLAN_ACTION_POP;
			}
		}

		if(pFlowPath1->out_smac_trans){
			flow_config.actions.options.masks.decrement_ttl = TRUE;
			flow_config.actions.options.masks.mac_sa = TRUE;
			memcpy(&flow_config.actions.options.mac_sa, &fc_db.netifTbl[NETIF_HWTOSW(pFlowPath1->out_intf_idx)].intf.gateway_mac_addr, ETH_ALEN);
		}else{
			flow_config.actions.options.masks.decrement_ttl = FALSE;
		}
		if(pFlowPath1->out_dmac_trans){
			flow_config.actions.options.masks.mac_da = TRUE;
			memcpy(&flow_config.actions.options.mac_da, &fc_db.lutTbl[pG3IgrExtraInfo->lutEgrDaIdx]->l2Addr, ETH_ALEN);
		}

		// Action: L3
		flow_config.actions.options.masks.dscp = pFlowPath1->out_dscp_act;
		flow_config.actions.options.dscp = pFlowPath1->out_dscp;

		// Action: Others
		flow_config.actions.options.masks.cos = pFlowPath1->out_user_pri_act;
		flow_config.actions.options.cos = pFlowPath1->out_user_priority;

		if(fc_db.controlFuc.loopbackMode_is_enabled && flow_dpa <= G3_LOOPBACK_P_END)
			flow_config.actions.dest.port = G3_LOOPBACK_P_NEWSPA;
		else
		{
			if(pFlowPath1->out_uc_lut_lookup)
				flow_config.actions.dest.port = flow_dpa;
			else
			{
				if(pFlowPath1->out_portmask)
					flow_config.actions.dest.port = __ffs(pFlowPath1->out_portmask);
				else if(pFlowPath1->out_ext_portmask_idx)
				{
					uint32 macPort, macExtPort;

					// In G3 platform out_ext_portmask_idx is wlanIdMask
					RTK_FC_HELPER_WLAN_DEVID_TO_PORTID(__ffs(fc_db.extPortTbl[pFlowPath1->out_ext_portmask_idx].extPortEnt.extpmask), &macPort, &macExtPort);
					flow_config.actions.dest.port = macPort;
				}
				else
					flow_config.actions.dest.port = flow_dpa;
			}

			// to Wifi
			if((1<<(flow_config.actions.dest.port))&RTK_FC_ALL_MAC_WLANCPU_PORTMASK){
				rtk_fc_wlan_devidx_t wlanDevIdx;
				if(pFlowPath1->out_uc_lut_lookup)
					wlanDevIdx = flow_egrWlanDevIdx;
				else
				{
					if(pFlowPath1->out_ext_portmask_idx)
					{
						// In G3 platform out_ext_portmask_idx is wlanIdMask
						wlanDevIdx = __ffs(fc_db.extPortTbl[pFlowPath1->out_ext_portmask_idx].extPortEnt.extpmask);
					}
					else
						wlanDevIdx = flow_egrWlanDevIdx;
				}
				flow_config.actions.options.masks.sw_id = TRUE;
				flow_config.actions.options.sw_id[0] = wlanDevIdx;

			#if defined(CONFIG_SMP)
				{
					uint16 wifi_tx_dispatch_mode;
					uint8 cpu_idx;
					uint band;
					band = RTK_FC_HELPER_WLAN_DEVID_TO_BANDIDX(wlanDevIdx);
					if(RTK_FC_MGR_DISPATCH_SMP_MODE_WIFI_TX_CPUID_GET(&cpu_idx, &wifi_tx_dispatch_mode, flowIdx>>1, (RTK_FC_MGR_DISPATCH_WLAN0_TX+band)) == SUCCESS)
					{
						if((wifi_tx_dispatch_mode == RTK_FC_DISPATCH_MODE_SMP_BY_HASH) || (wifi_tx_dispatch_mode == RTK_FC_DISPATCH_MODE_SMP_BY_RR))
						{
							flow_config.actions.dest.port = RTK_FC_MAC_PORT_CPU + cpu_idx;
							TRACE("WIFI TX dispatch is SMP mode, decide WIFI TX CPU port ID 0x%x by %s", flow_config.actions.dest.port, (wifi_tx_dispatch_mode == RTK_FC_DISPATCH_MODE_SMP_BY_HASH)?"HASH":"RR");
						}
					}
				}
			#endif
			}
		}

		flow_config.actions.options.masks.sw_id = TRUE;
		flow_config.actions.options.sw_id[1] = (pFlowPath1->in_intf_idx<<8 | pFlowPath1->out_intf_idx);

		_rtk_fc_g3FlowIdDecision(flow, pG3IgrExtraInfo, &flowIdActMask, &flowId);
#if defined(CONFIG_FC_CA8277B_SERIES)
		// policer1 is for upstream GemId, there should no policer function use flow policer1 action

		// policer_2: flow meter + flow mib (flow_meter_mib_conf_dependence must be enabled)
		flow_config.actions.options.masks.flow_id2 = pFlowPath1->out_share_meter_act;
		flow_config.actions.options.flow_id2 = pFlowPath1->out_share_meter_idx + G3_FLOW_POLICER_IDXSHIFT_FLOWMTR;

		// policer_3: host pure logging (decide by _rtk_fc_g3FlowIdDecision)
		flow_config.actions.options.masks.flow_id3 = flowIdActMask;
		flow_config.actions.options.flow_id3 = flowId;

#else
		flow_config.actions.options.masks.flow_id = flowIdActMask;
		flow_config.actions.options.flow_id = flowId;
#endif

#if defined(CONFIG_FC_CA8277B_SERIES)
#if defined(CONFIG_RG_G3_WAN_PORT_INDEX)&&(CONFIG_RG_G3_WAN_PORT_INDEX==7)
		if(((pFlowPath1->in_spa!=RTK_FC_MAC_PORT_PON) && pFlowPath1->in_out_stream_idx_check_act) && fc_db.streamidTbl[pFlowPath1->in_out_stream_idx].valid
			&& (fc_db.wanPortMask.portmask & (1 << flow_config.actions.dest.port))) {
			flow_config.actions.options.masks.gemId_mapping_mode = TRUE;
			flow_config.actions.options.masks.tcont_id = TRUE;
			flow_config.actions.options.masks.gem_id = TRUE;
			flow_config.actions.options.masks.cos = TRUE;
			
			flow_config.actions.options.gemId_mapping_mode = 0; //use gem mapping mode 0
			flow_config.actions.options.tcont_id = fc_db.streamidTbl[pFlowPath1->in_out_stream_idx].ldpid;
			flow_config.actions.options.gem_id = fc_db.streamidTbl[pFlowPath1->in_out_stream_idx].gemid;
			flow_config.actions.options.cos = fc_db.streamidTbl[pFlowPath1->in_out_stream_idx].cos;
		}
#endif
#endif
	}
	else{
		WARNING("not support\n");
		return CA_E_NOT_SUPPORT;
	}

	ret = ca_flow_add(0, &flow_config);

	if(ret == CA_E_OK){
		*mainHashIdx = flow_config.index;
		*flow_add_state = G3_FLOW_ADD_STATE_HW;
		TABLE(">>>>> Add Main Hash flow[%d]", flow_config.index);
		ca_flow_age_set(G3_DEF_DEVID, flow_config.index, G3_FLOW_AGE_STATE_IDLE);	// the aging time starts from flowSyncPeriod_unit1s
	}else if(ret==CA_E_FULL){
		*mainHashIdx = G3_FLOWIDX_INVALID;
		*flow_add_state = G3_FLOW_ADD_STATE_PURE_SW;
		ret = CA_E_OK;
		TABLE(">>>>> Main Hash table is full");
	}else{
		TABLE(">>>>> Add Main Hash  flow type[%d] fail, ret = 0x%x", flow_config.key_type, ret);
	}


	return ret;
}
#endif

rtk_fc_ret_t RTK_FC_ASIC_PONRX_TO_L3FE_SET(uint8 enable)
{

#if defined(CONFIG_FC_CA8277B_SERIES)
	rtk_8277b_asic_ni_p7_rxsel_l3fe(enable);

#elif defined(CONFIG_FC_RTL8277C_SERIES)

	ca_ni_rx_l3fe_demux_t demux={0};
	aal_ni_hv_glb_internal_port_id_cfg_mask_t niGlbCfgMask={0};
	aal_ni_hv_glb_internal_port_id_cfg_t niGlbCfg={0};

	if(enable) {
		/* 
		 * PON->L3FE->L2FE->L3FE(hash CRC offload)->L2TM->L3QM->CPU
		 * take care the configuration sequence: make sure l2 cls rule ready first.
		 */

		niGlbCfgMask.wrd = 0;
		niGlbCfgMask.bf.wan_rxsel = TRUE;
		niGlbCfg.wan_rxsel = 0;
		aal_ni_hv_glb_internal_port_id_cfg_set(CA_DEF_DEVID, niGlbCfgMask, &niGlbCfg);

		// L2 rule ready
		RTK_FC_HELPER_RT_CLS_FWDPORT_SET(RTK_FC_MAC_PORT_PON_LSPID_TO_L2FE, AAL_LPORT_L3_WAN);

		// deepq=0 to CPU directly
		//demux.wrd = rtk_ne_reg_read(NI_HV_GLB_NIRX_L3FE_DEMUX_CFG1);
		//demux.bf.ldpid_1	= NIRX_L3FE_DEMUX_ID_L2FE;		
		//rtk_ne_reg_write(demux.wrd, NI_HV_GLB_NIRX_L3FE_DEMUX_CFG1);

		// deepq=1 to L2FE for storm control
		demux.wrd = rtk_ne_reg_read(NI_HV_GLB_NIRX_L3FE_DPQ_DEMUX_CFG1);
		demux.bf.ldpid_1	= NIRX_L3FE_DEMUX_ID_L2FE;
		rtk_ne_reg_write(demux.wrd, NI_HV_GLB_NIRX_L3FE_DPQ_DEMUX_CFG1);
		
	}else{
		/* 
		 * PON->L2FE->L2TM->L3FE->L3QM->CPU
		 * take care the configuration sequence: make sure l2 cls rule ready first.
		 */
		//demux.wrd = rtk_ne_reg_read(NI_HV_GLB_NIRX_L3FE_DEMUX_CFG1);
		//demux.bf.ldpid_1	= NIRX_L3FE_DEMUX_ID_L3QM;		
		//rtk_ne_reg_write(demux.wrd, NI_HV_GLB_NIRX_L3FE_DEMUX_CFG1);
		
		demux.wrd = rtk_ne_reg_read(NI_HV_GLB_NIRX_L3FE_DPQ_DEMUX_CFG1);
		demux.bf.ldpid_1	= NIRX_L3FE_DEMUX_ID_L3QM;
		rtk_ne_reg_write(demux.wrd, NI_HV_GLB_NIRX_L3FE_DPQ_DEMUX_CFG1);

		
		niGlbCfgMask.wrd = 0;
		niGlbCfgMask.bf.wan_rxsel = TRUE;
		niGlbCfg.wan_rxsel = 2;
		aal_ni_hv_glb_internal_port_id_cfg_set(CA_DEF_DEVID, niGlbCfgMask, &niGlbCfg);
		
	}
#endif

	return RTK_FC_RET_OK;
}

#endif


rtk_fc_ret_t RTK_RG_ASIC_GLOBALSTATE_SET(rtk_rg_asic_globalStateType_t stateType, rtk_enable_t state)
{
	int ret=RTK_FC_RET_OK;
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	if(stateType == FB_GLOBAL_PATHALL_SKIP_SPRI ||
		stateType == FB_GLOBAL_PATHALL_SKIP_VLAN_DEICFI ) {
		// sw support
		ret=RT_ERR_OK;
	}else {
		if(rtk_rg_asic_globalState_set(stateType, state) == RT_ERR_RG_OK)
			ret = RTK_FC_RET_OK;
		else
			ret = RTK_FC_RET_ERR;
	}
#endif
	if(ret==RT_ERR_OK)
	{
		fc_db.systemGlobal.fbGlobalState[stateType]=state;
	}
	return ret;
}	
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
rtk_fc_ret_t RTK_RG_ASIC_PREHASHPTN_SET(rtk_rg_asic_preHashPtn_t ptnType, uint32 code)
{
	return RTK_FC_RET_OK;
}
#else
rtk_fc_ret_t RTK_RG_ASIC_PREHASHPTN_SET(rtk_rg_asic_preHashPtn_t ptnType, uint32 code)
{
	int ret=RTK_FC_RET_OK;

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	if(rtk_rg_asic_preHashPtn_set(ptnType, code)==RT_ERR_RG_OK)
		ret = RTK_FC_RET_OK;
	else
		ret = RTK_FC_RET_ERR;
#endif
	if(ret==RT_ERR_OK)
	{
		fc_db.systemGlobal.preHashPtn[ptnType]=code;
	}
	return ret;
}
#endif
rtk_fc_ret_t RTK_RG_ASIC_FLOWPATH_INVALID(uint32 idx)
{
	// SW entry has been updated, now we only need to update HW.
	rtk_fc_ret_t ret=RTK_FC_RET_OK;
	
#if defined(CONFIG_RTK_L34_CANDIDATE_FLOW)
	if(fc_db.flowTbl[idx].candidateState == CANDIDATE_STATE_NONE)
		return (RTK_FC_RET_OK);
#endif

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	if(idx<fc_db.flowHwTableSize)
	{
		if(rtk_rg_asic_flowPath1_set(idx, (rtk_rg_asic_path1_entry_t *)&fc_db.flowTbl[idx].pFlowEntry->path1)==RT_ERR_RG_OK)
			ret = RTK_FC_RET_OK;
		else
			ret = RTK_FC_RET_ERR;
	}
	
#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
	{
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		uint32 hwFlowIdx = idx;
#else
		uint32 hwFlowIdx = fc_db.flowTbl[idx].mainHashIdx;
#endif
		if(fc_db.flowTbl[idx].pFlowEntry->path1.valid == 0)
			ret = RTK_RG_G3_FLOW_DEL(hwFlowIdx);
	}
#endif

	_rtk_fc_trace_filter_flowTracker(idx);

	return ret;
}

rtk_fc_ret_t RTK_RG_ASIC_FLOWPATH_DEL(uint32 idx)
{
	rtk_fc_ret_t ret=RTK_FC_RET_OK;

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	if(idx<fc_db.flowHwTableSize)
	{

		if(_rtk_rg_fbMode_get()==FB_MODE_4K)
		{
			// delete SRAM entry
			if(rtk_rg_asic_flowPath_del(idx)==RT_ERR_RG_OK)
				ret = RTK_FC_RET_OK;
			else
				ret = RTK_FC_RET_ERR;

			// delete SW entry
			if(ret==RTK_FC_RET_OK)
			{
				memset(&fc_db.flowTbl[idx].pFlowEntry->path1, 0, sizeof(rtk_rg_asic_path1_entry_t));
			}
		}
		else
		{
			// delete DRAM entry - the entry was pointed by rg_db.flow[idx].pFlowEntry
			if(rtk_rg_asic_flowPath_del(idx)==RT_ERR_RG_OK)
				ret = RTK_FC_RET_OK;
			else
				ret = RTK_FC_RET_ERR;

		}
	}
	else
	{
		ret=RTK_FC_RET_OK;
		memset(&fc_db.flowTbl[idx].pFlowEntry->path1, 0, sizeof(rtk_rg_asic_path1_entry_t));
	}
#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
	{
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		uint32_t hwFlowIdx = idx;
#else
		uint32_t hwFlowIdx = fc_db.flowTbl[idx].mainHashIdx;
#endif
		if((ret = RTK_RG_G3_FLOW_DEL(hwFlowIdx))==RTK_FC_RET_OK)
		{
			memset(&fc_db.flowTbl[idx].pFlowEntry->path1, 0, sizeof(rtk_rg_asic_path1_entry_t));
		}
	}
#if 0 //defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES) //FIX_ME_DUAL
	if(fc_db.flowTbl[idx].mainHashIdx != G3_FLOWIDX_INVALID)
	{
		rtk_rg_asic_path1_entry_t *pPathData;
		l3pe_dual_tbl_entry_t dual_buff_entry;
		l3pe_dual_control_tbl_entry_t dual_ctrl_entry;

		pPathData = (rtk_rg_asic_path1_entry_t *)&fc_db.flowTbl[idx].pFlowEntry->path1;

		memset(&dual_buff_entry,0,sizeof(l3pe_dual_tbl_entry_t));
		ret = aal_pe_dual_content_entry_add(L3PE_DUAL_MAP_SINGLE_INTF(pPathData->out_intf_idx), &dual_buff_entry);
		if(ret != RTK_FC_RET_OK)
		{
			TRACE("Add dual content failed!(ret:0x%x)",ret);
			return ret;
		}

		memset(&dual_ctrl_entry,0,sizeof(l3pe_dual_control_tbl_entry_t));
		ret = aal_pe_dual_control_entry_add (L3PE_DUAL_MAP_SINGLE_INTF(pPathData->out_intf_idx), &dual_ctrl_entry);
		if(ret != RTK_FC_RET_OK)
		{
			TRACE("Add dual control failed!(ret:0x%x)",ret);
			return ret;
		}
	}
#endif
#endif

	_rtk_fc_trace_filter_flowTracker(idx);

	return ret;
}

rtk_fc_ret_t RTK_RG_ASIC_FLOWPATH_SET(uint32 *idx, void *pData, rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, uint8 swOnly)
{
	rtk_fc_ret_t ret=RTK_FC_RET_OK;
	rtk_rg_asic_path1_entry_t pathData;
	rtk_rg_asic_path1_entry_t *pPathData=&pathData;
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	uint32 mainHashIdx = G3_FLOWIDX_INVALID;
	rtk_rg_asic_path3_entry_t *pP3 = (rtk_rg_asic_path3_entry_t *)pPathData;
	bool bFlowUpdate = FALSE;
	rtk_fc_g3_flow_add_state_t flow_add_state = G3_FLOW_ADD_STATE_FAIL;
#endif

	memcpy(pPathData, pData, sizeof(rtk_rg_asic_path1_entry_t));

#if defined(CONFIG_RTL9607C_SERIES) && !defined(__LITTLE_ENDIAN)
			if(ASIC_CHIP_ID==RTL9603CVD_CHIP_ID)
			{
				if((pPathData->in_path==FB_PATH_12) || (pPathData->in_path==FB_PATH_34))
					
				if((uint32)fc_db.flowTbl[*idx].pFlowEntry == (uint32)pData )
					return RTK_FC_RET_ERR_INVALID_PARAM;
			}
#endif

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	if(*idx<fc_db.flowHwTableSize)
	{
		if(rtk_rg_asic_flowPath1_set(*idx, (rtk_rg_asic_path1_entry_t *)pPathData) == RT_ERR_RG_OK)
			ret = RTK_FC_RET_OK;
		else
			ret = RTK_FC_RET_ERR;
		
		if(_rtk_rg_fbMode_get()==FB_MODE_4K)
		{
			if(ret==RTK_FC_RET_OK)
			{
				memcpy(&fc_db.flowTbl[*idx].pFlowEntry->path1, pPathData, sizeof(rtk_rg_asic_path1_entry_t));
			}
		}
	}
	else
	{
		// In order to share an image between 9607C and 9603CVD
#if defined(CONFIG_RTL9607C_SERIES) && !defined(__LITTLE_ENDIAN)
		if(ASIC_CHIP_ID==RTL9603CVD_CHIP_ID)
		{
			rtk_rg_asic_path1_entry_t *pP1Data = (rtk_rg_asic_path1_entry_t *)pPathData;
			if(pP1Data->in_path==FB_PATH_12) // path 1, 2
			{
				pP1Data->in_spa <<= 1;
				pP1Data->out_portmask <<= 5;
			}
			else if(pP1Data->in_path==FB_PATH_34)	// path 3, 4
			{
				pP1Data->out_portmask <<= 5;
			}
		}
#endif
		ret=RTK_FC_RET_OK;
		memcpy(&fc_db.flowTbl[*idx].pFlowEntry->path1, pPathData, sizeof(rtk_rg_asic_path1_entry_t));
	}

#elif defined(CONFIG_RTK_L34_G3_PLATFORM)

	if(!pP3 || !pG3IgrExtraInfo)
		return RTK_FC_RET_ERR_NULL_POINTER;
	if(pP3->in_path==FB_PATH_6)
	{
		WARNING("FCv3 DO NOT ADD PATH6 IN G3 PLATFORM");
		return RTK_FC_RET_ERR_INVALID_PARAM;
	}


#if !(defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES))
#if defined(CONFIG_RTK_L34_CANDIDATE_FLOW)
	if(fc_db.flowTbl[*idx].candidateState==CANDIDATE_STATE_NONE)
#else
	if(fc_db.flowTbl[*idx].pFlowEntry->path1.valid==0)
#endif
		fc_db.flowTbl[*idx].mainHashIdx = G3_FLOWIDX_INVALID; // initial mainHashIdx first
#endif

	if(pP3->valid==FALSE){
#if defined(CONFIG_RTK_L34_CANDIDATE_FLOW)
		bFlowUpdate = TRUE;
#endif
		if(pP3->in_path==FB_PATH_6)
			bFlowUpdate = TRUE;
	}
	else{
		if((ret = RTK_RG_G3_FLOW_SET(pPathData, &mainHashIdx, pG3IgrExtraInfo, swOnly, *idx, &flow_add_state))==RTK_FC_RET_OK)
		{
			bFlowUpdate = TRUE;
			if(flow_add_state == G3_FLOW_ADD_STATE_HW){
#if defined(CONFIG_FC_RTL8277C_SERIES)
				if(fc_db.flowTbl[*idx].pFlowEntry->path1.flow_cache_mib_en)
				{
					uint32 flow_cache_mib_idx;
					if( (*idx<RTK_FC_TABLESIZE_HASH_FLOW) && aal_hash_cache_aqm_static_entry_add(*idx, &flow_cache_mib_idx)==0 )
					{
						fc_db.flowTbl[*idx].pFlowEntry->path1.flow_cache_mib_idx_or_in_outerVlan = flow_cache_mib_idx;
					}
					DEBUG("[flow_cache_mib en] flow_cache_mib_idx=%d", fc_db.flowTbl[*idx].pFlowEntry->path1.flow_cache_mib_idx_or_in_outerVlan);
				}
#elif defined(CONFIG_FC_RTL9607F_SERIES)
				//TODO: 9607F flow cache mib
#else
				fc_db.flowTbl[*idx].mainHashIdx = mainHashIdx;
#endif
				fc_db.g3_mHashTbl_validBitsArray[(mainHashIdx >> 5)] |= (0x1 << (mainHashIdx&0x1f));
			}
#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
			{
				/*
					For unicast pure SW flow, support DMA TX VLAN/PPPoE offload
					(multicast may have multiple flow to support multi-action, not support DMA TX VLAN/PPPoE offload currently)
				*/
				if(!( fc_db.netifTbl[NETIF_HWTOSW(pPathData->out_intf_idx)].dualType == RTK_FC_DUALTYPE_VXLAN || fc_db.netifTbl[NETIF_HWTOSW(pPathData->out_intf_idx)].dualType == RTK_FC_DUALTYPE_GRE_ETH_BR))
					_rtk_fc_flow_dmaAftAction_update((rtk_rg_asic_path3_entry_t *)pPathData, *idx, pG3IgrExtraInfo);
			}
#endif
		}
		else
		{
			DEBUG("Add HW flow failed, ret = %d", ret);
		}
	}
	if(pG3IgrExtraInfo && pG3IgrExtraInfo->dualHdrType == RTK_FC_DUALTYPE_VXLAN && pG3IgrExtraInfo->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM)
		bFlowUpdate = TRUE;
	if(bFlowUpdate)
		memcpy(&fc_db.flowTbl[*idx].pFlowEntry->path1, pPathData, sizeof(rtk_rg_asic_path1_entry_t));
#endif


	_rtk_fc_trace_filter_flowTracker(*idx);

	return ret;
}

rtk_fc_ret_t RTK_RG_ASIC_EXTPORTMASKTABLE_ADD(uint32 extPMask, int32 *extPMaskIdx)
{
	rtk_fc_ret_t ret=RTK_FC_RET_OK;

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	rtk_rg_asic_extPortMask_entry_t extPMaskEnt;
#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
	rtk_fc_wifi_dev_mask_t extPMaskEnt;
#endif
	uint32 idx;

	// find an available entry in indirect access table
	if((ret = _rtk_fc_extPMaskEntry_find(extPMask, &idx)) == RTK_FC_RET_ERR_ENTRY_FULL)
	{
		WARNING("FAIL to add indirect mac table because of entry full");
		return RT_ERR_RG_ENTRY_FULL;
	}

	if(fc_db.extPortTbl[idx].extPortEnt.extpmask == extPMask)
	{
		// increase referenced entry
		fc_db.extPortTbl[idx].extPortRefCount++;
	}else
	{
		TABLE("Add extPMask entry[%d]=0x%x", idx, extPMask);
		// configure a new one
		extPMaskEnt.extpmask = extPMask;

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
		if(rtk_rg_asic_extPortMaskTable_add(idx, &extPMaskEnt)==RT_ERR_RG_OK)
			ret=RTK_FC_RET_OK;
		else
			ret=RTK_FC_RET_ERR;	
#endif

		if(ret==RTK_FC_RET_OK)
		{
			//Copy to software table
			fc_db.extPortTbl[idx].extPortEnt.extpmask = extPMask;
			fc_db.extPortTbl[idx].extPortRefCount = 1;
		}
	}

	//DEBUG("target extpmaskidx = %d, refcount = %d, mask = 0x%x", idx, rg_db.extPortTbl[idx].extPortRefCount, rg_db.extPortTbl[idx].extPortEnt.extpmask);

	*extPMaskIdx = idx;

	return ret;
}

rtk_fc_ret_t RTK_RG_ASIC_EXTPORTMASKTABLE_DEL(int32 extPMaskIdx)
{
	rtk_fc_ret_t ret=RTK_FC_RET_OK;

	if(extPMaskIdx >= EXTPTBL_RSVED_ENTRY )
	{
		if(fc_db.extPortTbl[extPMaskIdx].extPortRefCount > 0)
		{
			fc_db.extPortTbl[extPMaskIdx].extPortRefCount -= 1;
			//TABLE("Del(keep by refCount) extPMask entry[%d]=0x%x ref=%d", extPMaskIdx, fc_db.extPortTbl[extPMaskIdx].extPortEnt.extpmask,fc_db.extPortTbl[extPMaskIdx].extPortRefCount);
		}
		if(fc_db.extPortTbl[extPMaskIdx].extPortRefCount == 0)
		{
			TABLE("Del extPMask entry[%d]", extPMaskIdx);

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
			if(rtk_rg_asic_extPortMaskTable_del(extPMaskIdx) == RT_ERR_RG_OK)
				ret=RTK_FC_RET_OK;
			else
				ret=RTK_FC_RET_ERR;				
#endif

			if(ret==RTK_FC_RET_OK)
			{
				fc_db.extPortTbl[extPMaskIdx].extPortEnt.extpmask = 0;
			}
		}

		//DEBUG("target extpmaskidx = %d, refcount = %d, mask = 0x%x", extPMaskIdx, rg_db.extPortTbl[extPMaskIdx].extPortRefCount, rg_db.extPortTbl[extPMaskIdx].extPortEnt.extpmask);
	}else
	{
		// entry[0] was reserved, do nothing
	}

	return ret;

}

rtk_fc_ret_t RTK_RG_ASIC_EXTRATAGACTION_ADD(uint32 actionList, uint32 actionIdx, rtk_rg_asic_extraTagAction_t *pExtraTagAction)
{
	rtk_fc_ret_t ret=RTK_FC_RET_OK;

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	if(rtk_rg_asic_extraTagAction_add(actionList, actionIdx, pExtraTagAction)==RT_ERR_RG_OK)
		ret=RTK_FC_RET_OK;
	else
		ret=RTK_FC_RET_ERR;
#endif

	if(ret==RTK_FC_RET_OK)
	{
		if(actionList>=RTK_FC_TABLESIZE_EXTRATAG) {
			WARNING("invalid extra tag actionList: %d", actionList);
			return RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE;
		}
		//Copy to software table
		memcpy(&fc_db.extraTagList[(actionList)].actions[actionIdx], pExtraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));

		if(pExtraTagAction->type1.act_bit == FB_EXTG_ACTBIT_1){
			fc_db.extraTagList[(actionList)].valid = TRUE;
			fc_db.extraTagList[(actionList)].bufferOff = pExtraTagAction->type1.src_addr_offset;
			fc_db.extraTagList[(actionList)].contentLen = pExtraTagAction->type1.length;
		}
	}

	return ret;
}

rtk_fc_ret_t RTK_RG_ASIC_EXTRATAGACTION_DEL(uint32 actionList, uint32 actionIdx)
{
	rtk_fc_ret_t ret=RTK_FC_RET_OK;

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	if(rtk_rg_asic_extraTagAction_del(actionList, actionIdx) == RT_ERR_RG_OK)
		ret=RTK_FC_RET_OK;
	else
		ret=RTK_FC_RET_ERR;		
#endif

	if(ret==RTK_FC_RET_OK)
	{
		memset(&fc_db.extraTagList[(actionList)].actions[actionIdx], 0, sizeof(rtk_rg_asic_extraTagAction_t));
	}

	return ret;
}

rtk_fc_ret_t RTK_RG_ASIC_EXTRATAGACTION_GET(uint32 actionList, uint32 actionIdx, rtk_rg_asic_extraTagAction_t *pExtraTagAction)
{
	rtk_fc_ret_t ret=RTK_FC_RET_OK;

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	if(rtk_rg_asic_extraTagAction_get(actionList, actionIdx, pExtraTagAction)==RT_ERR_RG_OK)
		ret=RTK_FC_RET_OK;
	else
		ret=RTK_FC_RET_ERR;
#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
	// software support
	memcpy(pExtraTagAction, &fc_db.extraTagList[(actionList)].actions[actionIdx], sizeof(rtk_rg_asic_extraTagAction_t));

#endif

	return ret;
}

rtk_fc_ret_t RTK_RG_ASIC_EXTRATAGCONTENTBUFFER_SET(uint32 actionList, char *pContentBuffer)
{
	rtk_fc_ret_t ret=RTK_FC_RET_OK;

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	if(rtk_rg_asic_extraTagContentBuffer_set(actionList, pContentBuffer) == RT_ERR_RG_OK)
		ret = RTK_FC_RET_OK;
	else
		ret = RTK_FC_RET_ERR;
#endif

	if(ret==RTK_FC_RET_OK)
	{
		memcpy(&fc_db.extraTagContentBuffer[fc_db.extraTagList[(actionList)].bufferOff], pContentBuffer, fc_db.extraTagList[(actionList)].contentLen);
	}

	return ret;

}

rtk_fc_ret_t RTK_RG_ASIC_EXTRATAGCONTENTBUFFER_GET(uint32 offset, uint32 len, char *pContentBuffer)
{
	rtk_fc_ret_t ret=RTK_FC_RET_OK;

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	if(rtk_rg_asic_extraTagContentBuffer_get(offset, len, pContentBuffer) == RT_ERR_RG_OK)
		ret = RTK_FC_RET_OK;
	else
		ret = RTK_FC_RET_ERR;

#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
	// software support
	memcpy(pContentBuffer, &fc_db.extraTagContentBuffer[offset], len);

#endif


	return ret;

}


rtk_fc_ret_t RTK_RG_ASIC_NETIFTABLE_ADD(uint32 hwIdx, rtk_rg_asic_netif_entry_t *intfEntry)
{
	int ret ;

	if(rtk_rg_asic_netifTable_add(hwIdx, intfEntry))
		ret = RTK_FC_RET_ERR;
	else
		ret =RTK_FC_RET_OK;

	return ret;
}

rtk_fc_ret_t RTK_RG_ASIC_NETIFTABLE_DEL(uint32 hwIdx)
{
	rtk_fc_ret_t ret = RTK_FC_RET_OK;
	uint32 temp_ret = RTK_FC_RET_OK;

	if(fc_db.netifHwTblVaild[hwIdx].hwNetifValid)
	{
		if(rtk_rg_asic_netifTable_del(hwIdx))
			ret = RTK_FC_RET_ERR;
		else
			ret = RTK_FC_RET_OK;

		rtk_fc_netif_dummy_packet_clear(hwIdx);

		fc_db.netifHwTblVaild[hwIdx].hwNetifValid=0;

		/*Clear HW netif mib*/
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
		temp_ret = rtk_rg_asic_netifMib_reset(hwIdx);
		if(temp_ret != RT_ERR_RG_OK)
			WARNING("clear hw netif mib failed, ret=%d", temp_ret);

#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
		{
			l3pe_cntr_tx_t tx_cntr;
			l3pe_cntr_rx_t rx_cntr;
			int aal_ret=AAL_E_OK;

			memset(&tx_cntr, 0, sizeof(tx_cntr));
			memset(&rx_cntr, 0, sizeof(rx_cntr));

			temp_ret = aal_l3pe_cntr_tx_set(G3_DEF_DEVID, hwIdx, &tx_cntr);
			if(temp_ret != AAL_E_OK)
				WARNING("clear hw netif[%d] tx mib failed, ret=%d", hwIdx, ret);
			temp_ret = aal_l3pe_cntr_rx_set(G3_DEF_DEVID, hwIdx, &rx_cntr);
			if(temp_ret != AAL_E_OK)
				WARNING("clear hw netif[%d] rx mib failed, ret=%d", hwIdx, ret);
#if defined(CONFIG_FC_CA8277B_SERIES)
			// clear MC netif counter
			aal_ret = aal_l3pe_cntr_tx_set(G3_DEF_DEVID, hwIdx + RTK_FC_MC_HW_NETIF_IDXSHIFT, &tx_cntr);
			if(aal_ret != AAL_E_OK)
				WARNING("clear hw netif[%d] (real hw index %d) tx mib for mc packet failed, ret=%d", hwIdx, hwIdx + RTK_FC_MC_HW_NETIF_IDXSHIFT, aal_ret);
			aal_ret = aal_l3pe_cntr_rx_set(G3_DEF_DEVID, hwIdx + RTK_FC_MC_HW_NETIF_IDXSHIFT, &rx_cntr);
			if(aal_ret != AAL_E_OK)
				WARNING("clear hw netif[%d] (real hw index %d) rx mib for mc packet failed, ret=%d", hwIdx, hwIdx + RTK_FC_MC_HW_NETIF_IDXSHIFT, aal_ret);
#elif defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
			// clear MC netif counter (care rx counter only)
			{
				aal_l3_te_pm_policer_t l3_pm_data;
				aal_ret = aal_l3_te_pm_policer_flow_get(G3_DEF_DEVID, hwIdx + G3_FLOW_POLICER_IDXSHIFT_MC_NETIF_RXMIB, &l3_pm_data); //read clear
				if(aal_ret != AAL_E_OK)
					WARNING("clear hw netif[%d] (real hw pol_idx %d) rx mib for mc packet failed, ret=%d", hwIdx, hwIdx + G3_FLOW_POLICER_IDXSHIFT_MC_NETIF_RXMIB, ret);
			}
#endif
			if(aal_ret==AAL_E_OK)
				ret = RTK_FC_RET_OK;
			else
				ret = RTK_FC_RET_ERR;

		}
#endif
	}

	return ret;
}

rtk_fc_ret_t RTK_RG_ASIC_ETHTYPE_ADD(uint32 swIdx, uint16 ethtype)
{
	rtk_fc_ret_t ret = RTK_FC_RET_OK;
	uint32 hwIdx = 0;
	
	if(swIdx>=RTK_FC_TABLESIZE_ETHERTYPE) {
		WARNING("fail to add ethtype since idx %d >= table size %d", swIdx, RTK_FC_TABLESIZE_ETHERTYPE);
		return RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE;
	}
	if(fc_db.ethertypeTbl[swIdx].valid) {
		WARNING("fail to add ethtype 0x%x since swidx[%d] is occupied by 0x%x", ethtype, swIdx,
						fc_db.ethertypeTbl[swIdx].ethType);
		return RTK_FC_RET_ERR_ENTRY_EXIST;
	}
	
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	{
		rtk_rg_asic_etherType_entry_t ethType={0};

		ethType.ethertype = ethtype;
		hwIdx = swIdx;
		rtk_rg_asic_etherTypeTable_add(hwIdx, &ethType);
	}

#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
	{
		l3_cam_ethertype_tbl_entry_t cam_eth={0};

		cam_eth.vld = TRUE;
		cam_eth.ethertype = ethtype;

		if (aal_entry_add(AAL_TABLE_L3_CAM_ETHERTYPE, &cam_eth, &hwIdx)!=AAL_E_OK) {
			WARNING("fail to add ethtype 0x%x to hw table, ret = %d ", ethtype, ret);
			return RTK_FC_RET_ERR;
		}
		else
			ret=RTK_FC_RET_OK;
	}

#endif

	fc_db.ethertypeTbl[swIdx].valid = TRUE;
	fc_db.ethertypeTbl[swIdx].ethType = ethtype;
	fc_db.ethertypeTbl[swIdx].hwIdx = hwIdx;
	
	return ret;
}

rtk_fc_ret_t RTK_RG_ASIC_ETHTYPE_DEL(uint32 swIdx)
{
	rtk_fc_ret_t ret = RTK_FC_RET_OK;

	if(swIdx>=RTK_FC_TABLESIZE_ETHERTYPE)
		return RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE;
	if(!fc_db.ethertypeTbl[swIdx].valid)
		return RTK_FC_RET_ERR_ENTRY_NOT_FOUND;

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	if(rtk_rg_asic_etherTypeTable_del(fc_db.ethertypeTbl[swIdx].hwIdx)==RT_ERR_RG_OK)
		ret = RTK_FC_RET_OK;
	else
		ret = RTK_FC_RET_ERR;
#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
	if(aal_entry_del(AAL_TABLE_L3_CAM_ETHERTYPE, fc_db.ethertypeTbl[swIdx].hwIdx) == AAL_E_OK)
		ret = RTK_FC_RET_OK;
	else
		ret = RTK_FC_RET_ERR;
#endif

	if(ret == RTK_FC_RET_OK)
		bzero(&fc_db.ethertypeTbl[swIdx], sizeof(rtk_fc_tableEthType_t));

	return ret;
}

rtk_fc_ret_t RTK_RG_ASIC_ETHTYPE_GET_HW_IDX(uint16 ethtype, uint32 *hwIdx)
{
	/*	check if specific ethtype value is cared, 
	 *	if yes return the hw entry index
	 *	if no return don't care
	*/

	int i = 0;

	*hwIdx=RTK_FC_ETHERTYPE_DONT_CARE;

	if(fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH12_PROTOCOL]==FALSE) {
		*hwIdx = RTK_FC_ETHERTYPE_DONT_CARE;		
	}else {
		for(i = 0; i < RTK_FC_TABLESIZE_ETHERTYPE; i++) {
			if(!fc_db.ethertypeTbl[i].valid) 
				continue;
			if(fc_db.ethertypeTbl[i].ethType == ethtype) {
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
				*hwIdx = fc_db.ethertypeTbl[i].hwIdx;
#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
				*hwIdx = (fc_db.ethertypeTbl[i].hwIdx+1);	// ethertype_enc; fib assign N+1 if target is N.
#endif
				break;
			}
		}
	}

	return RTK_FC_RET_OK;
}

rtk_fc_ret_t RTK_L2_PORTLIMITLEARNINGCNT_SET(rtk_port_t port, uint32 macCnt)
{
	int ret=RTK_FC_RET_OK;
	if(_rtk_fc_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	return rtk_l2_portLimitLearningCnt_set(port, macCnt);
}
rtk_fc_ret_t RTK_L2_PORTLIMITLEARNINGCNTACTION_SET(rtk_port_t port, rtk_l2_limitLearnCntAction_t learningAction)
{
	int ret=RTK_FC_RET_OK;
	if(_rtk_fc_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	return rtk_l2_portLimitLearningCntAction_set(port, learningAction);
}

rtk_fc_ret_t RTK_L2_PORTLOOKUPMISSACTION_SET(rtk_port_t port, rtk_l2_lookupMissType_t type, rtk_action_t action)
{
	int ret=RTK_FC_RET_OK;
	if(_rtk_fc_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	return rtk_l2_portLookupMissAction_set(port, type, action);
}

rtk_fc_ret_t RTK_L2_ILLEGALPORTMOVEACTION_SET(rtk_port_t port, rtk_action_t fwdAction)
{
	int ret=RTK_FC_RET_OK;
	if(_rtk_fc_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	return rtk_l2_illegalPortMoveAction_set(port, fwdAction);
}

rtk_fc_ret_t RTK_L2_PORTAGINGENABLE_SET(rtk_port_t port, rtk_enable_t enable)
{
	int ret=RTK_FC_RET_OK;
	if(_rtk_fc_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	return rtk_l2_portAgingEnable_set(port, enable);
}

rtk_fc_ret_t RTK_L2_VIDUNMATCHACTION_SET(rtk_port_t port, rtk_action_t fwdAction)
{
	int ret=RTK_FC_RET_OK;
	if(_rtk_fc_checkPortNotExistByPhy(port))return ret;     //just return without changing anything
	return  rtk_l2_vidUnmatchAction_set(port,fwdAction);
}

rtk_fc_ret_t RTK_VLAN_PORTPVID_SET(rtk_port_t port,uint32 pvid)
{
	int ret=RTK_FC_RET_OK;
	if(_rtk_fc_checkPortNotExistByPhy(port))return ret;     //just return without changing anything
	return rtk_vlan_portPvid_set(port,pvid);
}

rtk_fc_ret_t RTK_QOS_PORTPRIMAP_SET(rtk_port_t port, uint32 group)
{
	int ret=RTK_FC_RET_OK;
	if(_rtk_fc_checkPortNotExistByPhy(port))return ret;     //just return without changing anything
	return rtk_qos_portPriMap_set(port, group);
}

rtk_fc_ret_t RTK_QOS_PORTPRISELGROUP_SET(rtk_port_t port, uint32 priSelGrpIdx)
{
	int ret=RTK_FC_RET_OK;
	if(_rtk_fc_checkPortNotExistByPhy(port))return ret;     //just return without changing anything
	return rtk_qos_portPriSelGroup_set(port, priSelGrpIdx);
}

rtk_fc_ret_t RTK_QOS_PORTPRI_SET(rtk_port_t port, rtk_pri_t intPri)
{
	int ret=RTK_FC_RET_OK;
	if(_rtk_fc_checkPortNotExistByPhy(port))return ret;     //just return without changing anything
	return rtk_qos_portPri_set(port, intPri);
}

rtk_fc_ret_t RTK_SVLAN_SERVICEPORT_SET(rtk_port_t port, rtk_enable_t enable)
{
	int ret=RTK_FC_RET_OK;
	if(_rtk_fc_checkPortNotExistByPhy(port))return ret;     //just return without changing anything
	return rtk_svlan_servicePort_set(port,enable);
}

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
rtk_fc_ret_t RTK_ACL_IGRRULEENTRY_ADD(rtk_acl_ingress_entry_t *pAclRule, int ref_info)
{
	int ret=RTK_FC_RET_OK;

	if( (ref_info != FAIL) && (pAclRule->act.enableAct[ACL_IGR_FORWARD_ACT] == ENABLED) &&
		(pAclRule->act.forwardAct.act == ACL_IGR_FORWARD_TRAP_ACT) &&
		(pAclRule->act.enableAct[ACL_IGR_CVLAN_ACT] != ENABLED)){

		pAclRule->act.enableAct[ACL_IGR_CVLAN_ACT] = ENABLED;
		pAclRule->act.cvlanAct.act = ACL_IGR_CVLAN_IGR_CVLAN_ACT;
		if(ref_info >= RTK_FC_ACLANDCF_RESERVED_TAIL_END){
			pAclRule->act.cvlanAct.cvid = ref_info - RTK_FC_ACLANDCF_RESERVED_TAIL_END;
			pAclRule->act.cvlanAct.cvid |= RXINFO_REF_ACL_RSN_CTRL_USER;
		}
		else{
			pAclRule->act.cvlanAct.cvid = ref_info;
			pAclRule->act.cvlanAct.cvid |= RXINFO_REF_ACL_RSN_CTRL_RSV;
		}
		pAclRule->act.cvlanAct.cvid |= RXINFO_REF_ACL_RSN_CTRL_TRAP;
	}

	ret=rtk_acl_igrRuleEntry_add(pAclRule);
	return ret;
}

rtk_fc_ret_t RTK_ACL_IGRSTATE_SET(rtk_port_t port, rtk_enable_t state)
{
	int ret=RTK_FC_RET_OK;
	if(_rtk_fc_checkPortNotExistByPhy(port))return ret;     //just return without changing anything
	return rtk_acl_igrState_set(port, state);
}

rtk_fc_ret_t RTK_ACL_IGRUNMATCHACTION_SET(rtk_port_t port, rtk_filter_unmatch_action_type_t action)
{
	int ret=RTK_FC_RET_OK;
	if(_rtk_fc_checkPortNotExistByPhy(port))return ret;     //just return without changing anything
	return rtk_acl_igrUnmatchAction_set(port, action);
}
#endif

rtk_fc_ret_t RTK_PORT_MACFORCEABILITY_GET(rtk_port_t port, rtk_port_macAbility_t * pMacAbility)
{
	int ret=RTK_FC_RET_OK;
	if(_rtk_fc_checkPortNotExistByPhy(port))return ret;     //just return without changing anything
	return rtk_port_macForceAbility_get(port, pMacAbility);
}

rtk_fc_ret_t RTK_PORT_MACFORCEABILITY_SET(rtk_port_t port,rtk_port_macAbility_t macAbility)
{
	int ret=RTK_FC_RET_OK;
	if(_rtk_fc_checkPortNotExistByPhy(port))return ret;     //just return without changing anything
	return rtk_port_macForceAbility_set(port, macAbility);
}

rtk_fc_ret_t RTK_PORT_MACFORCEABILITYSTATE_SET(rtk_port_t port,rtk_enable_t state)
{
	int ret=RTK_FC_RET_OK;
	if(_rtk_fc_checkPortNotExistByPhy(port))return ret;     //just return without changing anything
	return rtk_port_macForceAbilityState_set(port, state);
}

