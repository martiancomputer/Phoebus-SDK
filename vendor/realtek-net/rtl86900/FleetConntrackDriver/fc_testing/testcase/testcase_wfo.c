#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)

#include <test_define.h>
#include <pktGen.h>
#include <rtl_util.h>
#include <rtl_glue.h>
#include <dump.h>
#include <rt_igmp_ext.h>
#include <rtk_fc_wfo.h>
#include <rtk_fc_api.h>



#define WLAN_MAC0 "00-01-02-03-04-05"		//1
#define WLAN_MAC1 "00-11-12-13-14-15"		//2
#define WLAN_MAC2 "00-21-22-23-24-25"		//3

#define A4_WLAN_MAC0 "00-01-02-03-A4-05"	//4
#define A4_WLAN_MAC1 "00-11-12-13-A4-15"	//5
#define A4_WLAN_MAC2 "00-21-22-23-A4-25"	//6

#define A4_WLAN_MAC10 "00-21-22-23-A4-AA"


#define CASE 4
int egressPreFC_Callback(struct net_device *dev, struct sk_buff *skb ,rtk_egrPreFc_t *param)
{
	static int repeatNum=0;

	//TEST_DEBUG("egressPreFC_Callback");
	param->egrPreFc_cbAct |=RTK_FC_CB_MC2UC;

	if(repeatNum%CASE==0)
	{
		param->macListCnt=5;
		strtomac((void *)&param->macList[0].mac[0], WLAN_MAC0);
		strtomac((void *)&param->macList[1].mac[0], WLAN_MAC1);
		param->macList[2].is_a4=1;
		strtomac((void *)&param->macList[2].mac[0], A4_WLAN_MAC0);
		param->macList[3].is_a4=1;
		strtomac((void *)&param->macList[3].mac[0], A4_WLAN_MAC1);
		param->macList[4].is_a4=1;
		strtomac((void *)&param->macList[4].mac[0], A4_WLAN_MAC10);			
	}
	else if(repeatNum%CASE==1)
	{

		param->macListCnt=5;
		strtomac((void *)&param->macList[0].mac[0], WLAN_MAC0);
		strtomac((void *)&param->macList[1].mac[0], WLAN_MAC1);
		strtomac((void *)&param->macList[2].mac[0], WLAN_MAC2);
		param->macList[3].is_a4=1;
		strtomac((void *)&param->macList[3].mac[0], A4_WLAN_MAC0);
		param->macList[4].is_a4=1;
		strtomac((void *)&param->macList[4].mac[0], A4_WLAN_MAC10);		

	}
	else if(repeatNum%CASE==2)
	{
		param->macListCnt=4;
		strtomac((void *)&param->macList[0].mac[0], WLAN_MAC1);
		strtomac((void *)&param->macList[1].mac[0], WLAN_MAC2);
		param->macList[2].is_a4=1;
		strtomac((void *)&param->macList[2].mac[0], A4_WLAN_MAC1);
		param->macList[3].is_a4=1;
		strtomac((void *)&param->macList[3].mac[0], A4_WLAN_MAC2);	
	}
	else if(repeatNum%CASE==3)
	{
		param->macListCnt=0;
	}

	repeatNum++;

	return SUCCESS;
}

int igressPreFC_Callback(struct net_device *dev, struct sk_buff *skb ,rtk_igrPreFc_t *param)
{
	//TEST_DEBUG("igressPreFC_Callback");
	param->igrPreFc_cbAct |=RTK_FC_CB_IGMP_LRN;
	return SUCCESS;
}

int igressPostFC_Callback(struct net_device *dev, struct sk_buff *skb ,rtk_igrPostFc_t *param)
{
	//TEST_DEBUG("igressPostFC_Callback");

	return SUCCESS;
}


static int isCallbackReg=0;

//	info->egrInfo.igrLogicalDev[0] =__dev_get_by_name(&init_net,"nas0_0");
//	info->egrInfo.egrLogicalDev[0] =__dev_get_by_name(&init_net,"eth0.2.0");
int wfo_callback_igmp_testing(rtk_fcTesting_case_t *info)
{
	struct net_device *egrWlanDev=NULL;
	rtk_fc_wifi_dev_attr_t attr;
	uint32 groupip,sourceip;
	uint8 igr_Pktsmac[6];
	memset(&attr,0,sizeof(attr));

	egrWlanDev=__dev_get_by_name(&init_net,"wlan0");
	if(isCallbackReg==0)
	{
		attr.egrPreFc_cb = egressPreFC_Callback;
		attr.igrPostFc_cb = igressPostFC_Callback;
		attr.igrPreFc_cb = igressPreFC_Callback;
		rtk_fc_wifi_dev_attr_set(egrWlanDev, attr);
		isCallbackReg=1;
{
		rtk_fc_wifi_amsdu_pe_offload_sta_conf_sel_t sta_conf_sel={0};
		rtk_fc_wifi_amsdu_pe_offload_sta_info_t sta_conf={0};
		uint8 wlanMac[6];

		rtk_fc_internal_wifi_amsdu_pe_offload_en(RTK_PE_AMSDU_MODE_DISABLE_FLOW,TRUE);

		sta_conf_sel.p_wifi_dev=1;
		sta_conf.p_wifi_dev = egrWlanDev;

		TEST_DEBUG("Config MacID");		
		strtomac((void *)&wlanMac[0], WLAN_MAC0);
		rtk_fc_wifi_amsdu_pe_offload_mac_id_set(1,sta_conf_sel,sta_conf,wlanMac);
		strtomac((void *)&wlanMac[0], WLAN_MAC1);
		rtk_fc_wifi_amsdu_pe_offload_mac_id_set(2,sta_conf_sel,sta_conf,wlanMac);
		strtomac((void *)&wlanMac[0], WLAN_MAC2);
		rtk_fc_wifi_amsdu_pe_offload_mac_id_set(3,sta_conf_sel,sta_conf,wlanMac);
		strtomac((void *)&wlanMac[0], A4_WLAN_MAC0);
		rtk_fc_wifi_amsdu_pe_offload_mac_id_set(4,sta_conf_sel,sta_conf,wlanMac);	
		strtomac((void *)&wlanMac[0], A4_WLAN_MAC1);
		rtk_fc_wifi_amsdu_pe_offload_mac_id_set(5,sta_conf_sel,sta_conf,wlanMac);
		strtomac((void *)&wlanMac[0], A4_WLAN_MAC2);
		rtk_fc_wifi_amsdu_pe_offload_mac_id_set(6,sta_conf_sel,sta_conf,wlanMac);		
}		
	}


	//info->igrInfo.igr_from
	//rtk_fc_skb_rx
	info->igrInfo.igr_from=RTK_FC_SKB_PHY_RX;
	info->igrInfo.pol_id = RTK_FC_WLAN0_ROOT_INTF;
	info->igrInfo.flowid = RTK_FC_WLAN0_ROOT_INTF;
	info->igrInfo.igrPort = 0x10;

	strtoip(&groupip,"224.1.2.3");
	strtoip(&sourceip,"192.168.1.10");
	strtomac((void *)&igr_Pktsmac[0], LAN_HOST_MAC_0);

	IGMPv2_pktGen(info->igrInfo.igrData,&info->igrInfo.igrDataLen,IGMPv2_REPORT,igr_Pktsmac,sourceip,groupip);

	info->egrInfo.egr_to = NETIF_RX_PROTOCOL_STACK;


	return SUCCESS;

}

#define REALTEK_MAGIC_NUMBER 0xB7
int wfo_callback_mcdata_testing(rtk_fcTesting_case_t *info)
{
	struct net_device *egrWlanDev=NULL;
	rtk_fc_wifi_dev_attr_t attr;
	uint32 groupip,sourceip;
	uint8 igr_Pktsmac[6],igr_Pktdmac[6],wlanMac[6];
	extern int isCallbackReg;
	memset(&attr,0,sizeof(attr));

	egrWlanDev=__dev_get_by_name(&init_net,"wlan0");
	if(isCallbackReg==0)
	{

		attr.egrPreFc_cb = egressPreFC_Callback;
		attr.igrPostFc_cb = igressPostFC_Callback;
		attr.igrPreFc_cb = igressPreFC_Callback;
		rtk_fc_wifi_dev_attr_set(egrWlanDev, attr);
		isCallbackReg=1;

{
		rtk_fc_wifi_amsdu_pe_offload_sta_conf_sel_t sta_conf_sel={0};
		rtk_fc_wifi_amsdu_pe_offload_sta_info_t sta_conf={0};

		rtk_fc_internal_wifi_amsdu_pe_offload_en(RTK_PE_AMSDU_MODE_DISABLE_FLOW,TRUE);

		sta_conf_sel.p_wifi_dev=1;
		sta_conf.p_wifi_dev = egrWlanDev;

		TEST_DEBUG("Config MacID");		
		strtomac((void *)&wlanMac[0], WLAN_MAC0);
		rtk_fc_wifi_amsdu_pe_offload_mac_id_set(1,sta_conf_sel,sta_conf,wlanMac);
		strtomac((void *)&wlanMac[0], WLAN_MAC1);
		rtk_fc_wifi_amsdu_pe_offload_mac_id_set(2,sta_conf_sel,sta_conf,wlanMac);
		strtomac((void *)&wlanMac[0], WLAN_MAC2);
		rtk_fc_wifi_amsdu_pe_offload_mac_id_set(3,sta_conf_sel,sta_conf,wlanMac);		
}		
	}


	//rtk_fc_skb_rx
	info->igrInfo.igr_from=RTK_FC_SKB_PHY_RX;
	info->igrInfo.igrPort = RTK_FC_MAC_PORT1;

	strtoip(&groupip,"224.1.2.3");
	strtoip(&sourceip,"192.168.1.10");
	strtomac((void *)&igr_Pktsmac[0], REMOTE_HOST_MAC_1);
	strtomac((void *)&igr_Pktdmac[0], "01:00:5e:01:02:03" );
	info->igrInfo.igrDataLen=20;


	v4TcpUdpPktGen2(info->igrInfo.igrData,&egrPayload[0],&info->igrInfo.igrDataLen,_PKT_TYPE_UDP,
		igr_Pktsmac,igr_Pktdmac,
		sourceip,groupip,
		0x1234,0x4321,
		10,0,
		0,0,0,
		0,0,0,0,
		0,0);

#if 0
	memcpy(info->egrInfo.egrData,info->igrInfo.igrData,info->igrInfo.igrDataLen);
	info->egrInfo.egrSkbEgrDev = egrWlanDev;
	info->egrInfo.egr_to = WLAN_RTK_FC_FASTFWD_DEV_XMIT;
	info->egrInfo.egrDataLen = info->igrInfo.igrDataLen;


	rt_igmp_multicastMode_set(RT_IGMP_MULTICAST_SYNC_MODE_USER);

{
	/* config fc multicast api */
	rt_igmp_group_devPort_cfg_t mcDevConfig;
	memset(&mcDevConfig,0,sizeof(mcDevConfig));
	mcDevConfig.egrInfoType=EGRINFO_DEVIFIDX;
	mcDevConfig.cntEgrInfo = 1;
	mcDevConfig.egrInfo.devIfidx[0].ifindex = egrWlanDev->ifindex;
	mcDevConfig.groupBehavior = RT_MC_BEHAVIOR_FLOW_FORWAED;
	mcDevConfig.group_addr.ipv4 = groupip;
	mcDevConfig.is_ipv6 = 0;
	rt_igmp_multicastGroupDev_set(mcDevConfig);
}
#else
	//using slow path testing mc
	info->egrInfo.egr_to = NETIF_RX_PROTOCOL_STACK;
#endif

	return SUCCESS;

}


#endif
