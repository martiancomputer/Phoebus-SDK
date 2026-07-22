#ifndef __RTK_FC_WIFI_AMSDU_OFFLOAD__
#define __RTK_FC_WIFI_AMSDU_OFFLOAD__

#include <linux/skbuff.h>
#include <linux/netdevice.h>

#define RTK_WIFI_DFT_AMSDU_PKT_SIZE				3568
#define RTK_WIFI_AMSDU_OFFLOAD_INVALID_VOQ_ID	0x3F

#define AMSDU_PE_OFFLD_WIFI_PRI_OFFLD_EN_BITMSK_SUPPORT 1
#if defined(CONFIG_RTK_FC_WFO_PER_HW_FLOW_MIB)
#define AMSDU_PE_OFFLD_WIFI_PER_HW_FLOW_MIB_SUPPORT		1
#endif
#define RTK_FC_WFO_A4_STA_M2U_REFINE 1

#define RTK_FC_WFO_MC_ADDBA_BY_CALLBACK 1
#define RTK_FC_WFO_MESH_VLAN_FUNCTION_SUPPORT 1

typedef struct rtk_fc_wifi_amsdu_pe_offload_sta_conf_sel_s
{
	unsigned int power_saving				:1;	// If 1, update power saving status
	unsigned int wifi_pri_offld_en_bitmsk	:1;	// If 1, update wifi_pri_offld_en_bitmsk
	unsigned int amsdu_pkt_size				:1;	// If 1, update amsdu packet size
	unsigned int p_wifi_dev					:1;
	unsigned int voq_id						:1;	// valid only in RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_MULTI_STA_TC_EPP64 and RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_MULTI_STA_WMM_EPP64 mode
	unsigned int sta_id						:1;	// valid only in RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_MULTI_STA_TC_EPP64 and RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_MULTI_STA_WMM_EPP64 mode
}rtk_fc_wifi_amsdu_pe_offload_sta_conf_sel_t;

typedef struct rtk_fc_wifi_amsdu_pe_offload_sta_info_s
{
	unsigned int power_saving				:1;	//power saving status, 1: on; 0: off
	unsigned int wifi_pri_offld_en_bitmsk	:8; //bit[n]: if wifi priority n enable wifi amsdu offlad, 1: enable ; 0: disable
	unsigned int amsdu_pkt_size;				//amsdu packet size
	struct net_device *p_wifi_dev;
	unsigned int voq_id						:6;	// valid only in RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_MULTI_STA_XXX
	unsigned int sta_id						:5;	// valid only in RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_MULTI_STA_XXX
}rtk_fc_wifi_amsdu_pe_offload_sta_info_t;

typedef enum rtk_fc_wifi_amsdu_pe_offload_mode_e
{
	RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_TC_EPP64 = 0,			// 1 mac_id mapping to 1 voq, total 40 voq
	RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_WMM_EPP64,			// 1 mac_id mapping to 1 voq, total 10 voq
	RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_MULTI_STA_TC_EPP64,	// n mac_id mapping to 1 voq, total 40 voq
	RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_MULTI_STA_WMM_EPP64,	// n mac_id mapping to 1 voq, total 10 voq
	RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_DISABLE,
	RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_EPP64_END = RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_DISABLE,
	RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_TC_EPP256,			// 1 mac_id mapping to 1 voq, total XX voq
	RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_WMM_EPP256,			// 1 mac_id mapping to 1 voq, total XX voq
	RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_MULTI_STA_TC_EPP256,	// n mac_id mapping to 1 voq, total XX voq
	RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_MULTI_STA_WMM_EPP256,	// n mac_id mapping to 1 voq, total XX voq
	RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_END,
}rtk_fc_wifi_amsdu_pe_offload_mode_t;

#define	RTK_FC_WIFI_AMSDU_PE_OFFLOAD_NSTA_TO_1VOQ(mode)	(((mode == RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_MULTI_STA_TC_EPP64) || (mode == RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_MULTI_STA_WMM_EPP64) || (mode == RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_MULTI_STA_TC_EPP256) || (mode == RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_MULTI_STA_WMM_EPP256))?TRUE:FALSE)
#define	RTK_FC_WIFI_AMSDU_PE_OFFLOAD_IF_EPP64_TC_MODE(mode)	(((mode == RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_TC_EPP64) || (mode == RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_MULTI_STA_TC_EPP64))?TRUE:FALSE)
#define	RTK_FC_WIFI_AMSDU_PE_OFFLOAD_IF_EPP64_WMM_MODE(mode)	(((mode == RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_WMM_EPP64) || (mode == RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_MULTI_STA_WMM_EPP64))?TRUE:FALSE)
#define	RTK_FC_WIFI_AMSDU_PE_OFFLOAD_IF_EPP256_TC_MODE(mode)	(((mode == RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_TC_EPP256) || (mode == RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_MULTI_STA_TC_EPP256))?TRUE:FALSE)
#define	RTK_FC_WIFI_AMSDU_PE_OFFLOAD_IF_EPP256_WMM_MODE(mode)	(((mode == RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_WMM_EPP256) || (mode == RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_MULTI_STA_WMM_EPP256))?TRUE:FALSE)
typedef enum rtk_fc_wifi_amsdu_pe_queueid_e
{
	RTK_FC_WIFI_AMSDU_PE_QUEUE0 = 0,
	RTK_FC_WIFI_AMSDU_PE_QUEUE1 = 1,
	RTK_FC_WIFI_AMSDU_PE_QUEUE2 = 2,
	RTK_FC_WIFI_AMSDU_PE_QUEUE3 = 3,

	RTK_FC_WIFI_AMSDU_PE_QUEUE_MAX_TC_EPP64_MODE = RTK_FC_WIFI_AMSDU_PE_QUEUE0,
	RTK_FC_WIFI_AMSDU_PE_QUEUE_MAX_WMM_EPP64_MODE = RTK_FC_WIFI_AMSDU_PE_QUEUE3,
}rtk_fc_wifi_amsdu_pe_queueid_t;

typedef enum rtk_fc_wifi_amsdu_pe_wmm_queueid_mapping_e
{
	RTK_FC_WIFI_AMSDU_PE_WMM_QUEUE_BK = RTK_FC_WIFI_AMSDU_PE_QUEUE1,
	RTK_FC_WIFI_AMSDU_PE_WMM_QUEUE_BE = RTK_FC_WIFI_AMSDU_PE_QUEUE0,
	RTK_FC_WIFI_AMSDU_PE_WMM_QUEUE_VI = RTK_FC_WIFI_AMSDU_PE_QUEUE2,
	RTK_FC_WIFI_AMSDU_PE_WMM_QUEUE_VO = RTK_FC_WIFI_AMSDU_PE_QUEUE3,
}rtk_fc_wifi_amsdu_pe_wmm_queueid_mapping_t;

typedef enum rtk_fc_wifi_cb_e
{
    RTK_FC_CB_NON = (0),
    RTK_FC_CB_IGMP_LRN = (1<<1),
    RTK_FC_CB_UC2MC = (1<<2),
    RTK_FC_CB_MC2UC = (1<<3),
    RTK_FC_CB_NAT25_SMAC_TRANS = (1<<4),
    RTK_FC_CB_NAT25_DMAC_TRANS = (1<<5),
    RTK_FC_CB_LOOPBACK_DETECT_DROP = (1<<6),
    RTK_FC_CB_VLANFILTER_DROP = (1<<7),
    RTK_FC_CB_ADD_BA = (1<<8),		//for wifi dirver notify sta to receive agg amsdu
}rtk_fc_wifi_cb_t;


#define MAX_STA_MAC 64
typedef struct rtk_fc_macList_s
{
    unsigned char mac[6];
    unsigned short ctagif:1;
    unsigned short cvlan:12;
    unsigned short cpri_vld:1;
    unsigned short cpri:3;
	unsigned short is_a4:1;		//for multicast A4 device do not do M2U,but need using mac enqueue to right PE queue 
}rtk_fc_macList_t;
 

typedef struct rtk_igrPostFc_s
{
	rtk_fc_wifi_cb_t igrPostFc_cbAct;
}rtk_igrPostFc_t;

typedef struct rtk_igrPreFc_s
{
	rtk_fc_wifi_cb_t igrPreFc_cbAct;
}rtk_igrPreFc_t;

typedef struct rtk_egrPreFc_s
{
    rtk_fc_wifi_cb_t egrPreFc_cbAct;
    short macListCnt;
    rtk_fc_macList_t macList[MAX_STA_MAC];
}rtk_egrPreFc_t;

#if defined(CONFIG_RTK_FC_WFO_PER_HW_FLOW_MIB)
typedef struct rtk_fc_wfo_per_flow_mib_info_s
{
	uint8_t rx_entry_per_voq;
	uint8_t tx_entry_per_voq;
	uint32_t dmemWifiRxFlowMibCacheTbl_phyAddr;	//physical address of wifi rx cache table
	uint32_t dmemWifiTxFlowMibCacheTbl_phyAddr;	//physical address of wifi tx cache table
	uint32_t ddrWifiFlowMibNonCacheTbl_phyAddr;	//physical address of wifi non cache table
}rtk_fc_wfo_per_flow_mib_info_t;
typedef struct rtk_fc_wfo_rx_cache_ent_s
{
	uint32_t pkt_cnt;
	uint32_t byte_cnt_l;
	uint32_t byte_cnt_h;
	union{
		uint32_t raw_data;
		struct{
			uint32_t cos		:3;
			uint32_t pol_id		:8;
			uint32_t ldpid		:6;
			uint32_t reserve	:15;
		}bits;
	}bit32_field;
}__attribute__((packed))rtk_fc_wfo_rx_cache_ent_t;
typedef struct rtk_fc_wfo_tx_cache_ent_s
{
	uint32_t pkt_cnt;
	uint32_t byte_cnt_l;
	uint32_t byte_cnt_h;
	union{
		uint32_t raw_data;
		struct{
			uint32_t wifi_pri	:3;
			uint32_t mac_id		:8;
			uint32_t reserve	:21;
		}bits;
	}bit32_field;
}__attribute__((packed))rtk_fc_wfo_tx_cache_ent_t;

typedef struct rtk_fc_wfo_non_cache_crc32_s
{
	uint32_t crc32_0;
	uint32_t crc32_1;
	uint32_t crc32_2;
	uint32_t crc32_3;
	uint32_t crc32_4;
	uint32_t crc32_5;
	uint32_t crc32_6;
	uint32_t crc32_7;
}__attribute__((packed))rtk_fc_wfo_non_cache_crc32_t;

typedef struct rtk_fc_wfo_non_cache_ent_s
{
	uint32_t pkt_cnt;
	uint32_t byte_cnt_l;
	uint32_t byte_cnt_h;
	uint32_t wifi_pri	:3;
	uint32_t mac_id		:8;
	uint32_t cos		:3;
	uint32_t pol_id		:8;
	uint32_t ldpid		:6;
	uint32_t reserve	:4;
}__attribute__((packed))rtk_fc_wfo_non_cache_ent_t;
#endif

#if defined(CONFIG_RTK_FC_WFO_INTEGRATE_QOS)
#define RTK_FC_WFO_INTEGRATE_QOS_CPU_PORT	0x11
typedef enum rtk_fc_wfo_amsdu_pe_cpu_port_e
{
	RTK_FC_WFO_AMSDU_PE_CPU_P_0x13	= 0,
	RTK_FC_WFO_AMSDU_PE_CPU_P_0x14	= 1,
	RTK_FC_WFO_AMSDU_PE_CPU_P_0x15	= 2,
	RTK_FC_WFO_AMSDU_PE_CPU_P_0x16	= 3,
	RTK_FC_WFO_AMSDU_PE_CPU_P_0x17	= 4,
	RTK_FC_WFO_AMSDU_PE_LDPID_CNT	= 5,
}rtk_fc_wfo_amsdu_pe_cpu_port_t;

typedef struct rtk_fc_wfo_integrate_qos_info_s
{
	uint8_t amsdu_cpu_to_lspid_mapping[RTK_FC_WFO_AMSDU_PE_LDPID_CNT];
}rtk_fc_wfo_integrate_qos_info_t;
#endif

typedef struct rtk_fc_wifi_dev_attr_s
{
	unsigned int block_relay:1;			// If 1, drop same ssid forwarding pkt
	unsigned int client_mode:1;		// If 1, hwnat driver call wifi api to modify pkt include mac and arp, dhcp content.
	/*--------------------------------------------
		repeater	: client_mode(1) + do_nat25(1) 
		mesh	: client_mode(1) + do_nat25(0) 
	--------------------------------------------*/
	unsigned int do_nat25:1;

#if 1 //Will be removed and replace by (igrPostFc_cb/igrPreFc_cb/egrPreFc_cb)
	/*--------------------------------------------
		repeater	: client_mode(1) + nat25_cb(TRUE) 
		mesh	: client_mode(1) + nat25_cb(FALSE) 
	--------------------------------------------*/
	int (*nat25_cb)(struct net_device *dev, struct sk_buff *skb, bool rx);	// register for NAT2.5 translation in client mode (repeater only)
	int (*uc2mc_cb)(struct net_device *dev, struct sk_buff *skb);		// register for UC to MC translation in client mode (repeater only in wifi rx)
	int (*lb_cb)(struct net_device *dev, struct sk_buff *skb);			// register for loopback detection & drop in client mode (mesh/multi-ap only in wifi rx); return 1 if expect to drop pkt.
#endif

	/*
		igrPreFc_cb
			-igmp_learning()	        =>learning igmp packet		
			-loopback_detect()			=>detect loopback
	*/
	int (*igrPreFc_cb)(struct net_device *dev, struct sk_buff *skb ,rtk_igrPreFc_t *param);


	/*
		igrPostFc_cb
			-nat25()			=>modify dmac
			-uc2mc()			=>modify dmac from mc to uc
	*/
	int (*igrPostFc_cb)(struct net_device *dev, struct sk_buff *skb ,rtk_igrPostFc_t *param);
	
	
	/*
		egr_preFc_cb
			-nat25()					=>modify smac
			-igmp_get_mc2uc_macList()	=>get mc2ucList for hardware add
			-add_ba()					=>for wifi dirver notify sta to receive agg amsdu
	*/
	int (*egrPreFc_cb)(struct net_device *dev, struct sk_buff *skb ,rtk_egrPreFc_t *param);
	
}rtk_fc_wifi_dev_attr_t;

typedef enum rtk_fc_wifi_event_code_e
{
	RTK_FC_WIFI_EVENT_DISABLE_ACC = 0,
	RTK_FC_WIFI_EVENT_ENABLE_ACC,
	RTK_FC_WIFI_EVENT_END,
}rtk_fc_wifi_event_code_t;

typedef struct rtk_fc_wifi_event_s
{
	rtk_fc_wifi_event_code_t code;
}rtk_fc_wifi_event_t;

#if defined(CONFIG_RTK_L34_G3_PLATFORM)
typedef struct rtk_fc_wlan_devCfg_s
{
	uint8_t macPortIdx;		// rtk_fc_mac_port_idx_t
	uint8_t initBand;		// rtk_fc_wlan_init_id_t
}rtk_fc_wlan_devMapCfg_t;

/* Function Name:
*	rtk_fc_wifi_wlanDevMap_add
* Description:
*	Add Wifi dev into FC wlanDevMap table.
* Input:
*	wifiDev_name- wifi device name
*   wlanDevMapCfg- wlanDevMap configuration for wifi device
* Output:
*	wlanDevIdx  - wlanDevIdx for new added wifi device (please refer to rtk_fc_wlan_devidx_t)
* Return:
*	0	   	- SUCCESS
*	others  	- error code
*/
int rtk_fc_wifi_wlanDevMap_add(char* wifiDev_name, rtk_fc_wlan_devMapCfg_t wlanDevMapCfg, uint8_t *wlanDevIdx);

/* Function Name:
*	rtk_fc_wifi_wlanDevMap_del
* Description:
*	Delete Wifi dev from FC wlanDevMap table by device name.
* Input:
*	wifiDev_name- wifi device name
* Output:
*	N/A
* Return:
*	0	   	- SUCCESS
*	others  	- error code
*/
int rtk_fc_wifi_wlanDevMap_del(char* wifiDev_name);

/* Function Name:
*	rtk_fc_wifi_wlanDevMap_flush
* Description:
*	Flush FC wlanDevMap table
* Input:
*	N/A
* Output:
*	N/A
* Return:
*	0	   	- SUCCESS
*	others  	- error code
*/
int rtk_fc_wifi_wlanDevMap_flush(void);
#endif

/* Function Name:
*	rtk_fc_wifi_amsdu_pe_offload_mac_id_set
* Description:
*	set mac_id and its related info for Wifi PE offload.
* Input:
*	mac_id		- mac_id
*   sta_conf_sel- wifi STA configuration selection
*   sta_conf    - wifi STA information
*	mac_addr	- MAC address of mac_id
* Output:
*	N/A
* Return:
*	0	   	- SUCCESS
*	others  	- error code
*/
int rtk_fc_wifi_amsdu_pe_offload_mac_id_set(unsigned int mac_id, rtk_fc_wifi_amsdu_pe_offload_sta_conf_sel_t sta_conf_sel, rtk_fc_wifi_amsdu_pe_offload_sta_info_t sta_conf, unsigned char* mac_addr);

/* Function Name:
*	rtk_fc_wifi_amsdu_pe_offload_mac_id_del
* Description:
*	delete mac_id and its related info for Wifi PE offload.
* Input:
*	mac_id		- mac_id
* Output:
*	N/A
* Return:
*	0	   	- SUCCESS
*	others  	- error code
*/
int rtk_fc_wifi_amsdu_pe_offload_mac_id_del(unsigned int mac_id);

/* Function Name:
*	rtk_fc_wifi_amsdu_pe_offload_mac_id_flush
* Description:
*	clear all mac_id and its related info for Wifi PE offload.
* Input:
*	N/A
* Output:
*	N/A
* Return:
*	0	   	- SUCCESS
*	others  	- error code
*/
int rtk_fc_wifi_amsdu_pe_offload_mac_id_flush(void);

/* Function Name:
*	rtk_fc_wifi_amsdu_pe_offload_mode_set
* Description:
*	set wifi amsdu offload mode
* Input:
*	mode		- mode
* Output:
*	N/A
* Return:
*	0	   	- SUCCESS
*	others  	- error code
*/
int rtk_fc_wifi_amsdu_pe_offload_mode_set(rtk_fc_wifi_amsdu_pe_offload_mode_t mode);

/* Function Name:
*	rtk_fc_wifi_amsdu_pe_offload_mode_get
* Description:
*	get wifi amsdu offload mode
* Input:
*	mode		- mode
* Output:
*	N/A
* Return:
*	0	   	- SUCCESS
*	others  	- error code
*/
int rtk_fc_wifi_amsdu_pe_offload_mode_get(rtk_fc_wifi_amsdu_pe_offload_mode_t *mode);

/* Function Name:
*	rtk_fc_wifi_dev_attr_set
* Description:
*	set dev attribute according to dev ptr, caller should call get first and then set what you want to change.
* Input:
*	dev			- net_dev pointer
*	attr			- attribute
* Output:
*	N/A
* Return:
*	0	   		- SUCCESS
*	others  		- error code
*/
int rtk_fc_wifi_dev_attr_set(struct net_device *dev, rtk_fc_wifi_dev_attr_t attr);

/* Function Name:
*	rtk_fc_wifi_dev_attr_get
* Description:
*	get dev attribute according to dev ptr
* Input:
*	dev			- net_dev pointer
* Output:
*	attr			- attribute
* Return:
*	0	   		- SUCCESS
*	others  		- error code
*/
int rtk_fc_wifi_dev_attr_get(struct net_device *dev, rtk_fc_wifi_dev_attr_t *attr);

/* Function Name:
*	rtk_fc_wifi_dev_to_devidx_get
* Description:
*	get fc wifi dev index
* Input:
*	dev			- net_dev pointer
* Output:
*	wlan_dev_idx	- wifi dev index
* Return:
*	0	   		- SUCCESS
*	others  		- error code
*/
int rtk_fc_wifi_dev_to_devidx_get(struct net_device *dev, unsigned int *wlan_dev_idx);

/* Function Name:
*	rtk_fc_wifi_event_handling_cb_register
* Description:
*	wifi driver event handling callback
* Input:
*	rtk_fc_wifi_event_handling_callback	- cb func
* Output:
*	N/A
* Return:
*	0	   		- SUCCESS
*	others  		- error code
*/
typedef int (*rtk_fc_wifi_event_handling_callback)(rtk_fc_wifi_event_t *event);
int rtk_fc_wifi_event_handling_cb_register(rtk_fc_wifi_event_handling_callback pfunc);

/* Function Name:
*	rtk_fc_wifi_client_mode_cb_register
* Description:
*	wifi provide pkt modificaton callback
* Input:
*	rtk_fc_wifi_callback	- cb func
* Output:
*	N/A
* Return:
*	0	   		- SUCCESS
*	others  		- error code
*/
typedef int (*rtk_fc_wifi_callback)(struct net_device *dev, struct sk_buff *skb, bool rx);
int rtk_fc_wifi_client_mode_cb_register(rtk_fc_wifi_callback pfunc);

/* Function Name:
*	rtk_fc_wifi_dmacHandle_cb_register
* Description:
*	wifi provide pkt modificaton callback
* Input:
*	rtk_fc_wfo_callback	- cb func
* Output:
*	N/A
* Return:
*	0	   		- SUCCESS
*	others  		- error code
*/
typedef int (*rtk_fc_wifi_dmacHandle_callback)(struct net_device *dev, struct sk_buff *skb);
int rtk_fc_wifi_dmacHandle_cb_register(rtk_fc_wifi_dmacHandle_callback pfunc);

/* Function Name:
*	rtk_fc_wifi_amsdu_pe_offload_wifiPri_to_amsduQ_set
* Description:
*	Set wifi priority to amdsu queue id mapping
* Input:
*	wifi_pri	- wifi priority
*	amsdu_qid	- amsdu queue id
* Output:
*	N/A
* Return:
*	0	   		- SUCCESS
*	others  		- error code
*/
int rtk_fc_wifi_amsdu_pe_offload_wifiPri_to_amsduQ_set(unsigned int wifi_pri, rtk_fc_wifi_amsdu_pe_queueid_t amsdu_qid);

/* Function Name:
*	rtk_fc_wifi_amsdu_pe_offload_wifiPri_to_amsduQ_get
* Description:
*	Get wifi priority to amdsu queue id mapping
* Input:
*	wifi_pri	- wifi priority
* Output:
*	amsdu_qid	- amsdu queue id
* Return:
*	0	   		- SUCCESS
*	others  		- error code
*/
int rtk_fc_wifi_amsdu_pe_offload_wifiPri_to_amsduQ_get(unsigned int wifi_pri, rtk_fc_wifi_amsdu_pe_queueid_t *amsdu_qid);

/* Function Name:
*	rtk_fc_wifi_ipDscp_to_wifiPri_set
* Description:
*	Set IP DSCP value to wifi priority mapping
* Input:
*	ip_dscp		- IP DSCP value
*	wifi_pri	- wifi priority
* Output:
*	N/A
* Return:
*	0	   		- SUCCESS
*	others  		- error code
*/
int rtk_fc_wifi_ipDscp_to_wifiPri_set(unsigned int ip_dscp, unsigned int wifi_pri);

/* Function Name:
*	rtk_fc_wifi_ipDscp_to_wifiPri_get
* Description:
*	Get IP DSCP value to wifi priority mapping
* Input:
*	ip_dscp		- IP DSCP value
* Output:
*	wifi_pri	- wifi priority
* Return:
*	0	   		- SUCCESS
*	others  		- error code
*/
int rtk_fc_wifi_ipDscp_to_wifiPri_get(unsigned int ip_dscp, unsigned int *wifi_pri);

#if defined(CONFIG_RTK_FC_WFO_PER_HW_FLOW_MIB)
/* Function Name:
*	rtk_fc_wfo_per_flow_mib_info_init
* Description:
*	Initial wfo_per_flow_mib_info
* Input:
*	info		- wfo_per_flow_mib_info
* Output:
*	N/A
* Return:
*	0	   		- SUCCESS
*	others  		- error code
*/
int rtk_fc_wfo_per_flow_mib_info_init(rtk_fc_wfo_per_flow_mib_info_t info);

/* Function Name:
*	rtk_fc_wfo_per_flow_mib_info_clear
* Description:
*	Clear wfo_per_flow_mib_info
* Input:
*	N/A
* Output:
*	N/A
* Return:
*	0	   		- SUCCESS
*	others  		- error code
*/
int rtk_fc_wfo_per_flow_mib_info_clear(void);
#endif
#if defined(CONFIG_RTK_FC_WFO_INTEGRATE_QOS)

/* Function Name:
*	rtk_fc_wfo_integrate_qos_set
* Description:
*	Enable or disable wfo integrate qos data path and get integrate_qos_info
* Input:
*	if_enable			- Enable or disable wfo integrate qos data path
* Output:
*	integrate_qos_info	- integrate_qos_info (amsdu cpu to lspid mapping table)
* Return:
*	0	   		- SUCCESS
*	others  		- error code
*/
int rtk_fc_wfo_integrate_qos_set(bool if_enable, rtk_fc_wfo_integrate_qos_info_t *integrate_qos_info);
#endif
#endif

