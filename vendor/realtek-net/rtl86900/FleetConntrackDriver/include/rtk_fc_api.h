#ifndef __RTK_FC_API_H__
#define __RTK_FC_API_H__

/****************************************
*	EXPORT_SYMBOL from FC (core) driver	*
*****************************************/

#include <rtk_fc_mgr.h>
#include <rtk_fc_helper_wlan.h>


#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)

/* Function Name:
* 	 rtk_fc_mapet_hwfmr_add
* Description:
* 	 Map-E/Map-T fmr table add for hardware accelerate
* Input:
* 	 fmrhwInfo		
* Output:
* 	 N/A
* Return:
* 	 0		 - SUCCESS
* 	 others  - error code
*/
int rtk_fc_mapet_hwfmr_add(rtk_fc_mapet_hwfmrInfo_t *fmrhwInfo);

/* Function Name:
* 	 rtk_fc_mapet_hwfmr_del
* Description:
* 	Map-E/Map-T fmr table del for hardware accelerate
* Input:
* 	 fmrhwInfo
* Output:
* 	  N/A
* Return:
* 	 0		 - SUCCESS
* 	 others  - error code
*/
int rtk_fc_mapet_hwfmr_del(rtk_fc_mapet_hwfmrInfo_t *fmrhwInfo);
#endif


 /* Function Name:
 *      rtk_fc_dual_devInfoConfig
 * Description:
 *      get dualInfo from api
 * Input:
 *      devName		- device Name
 *      dualType	- dual-hdr type
 *		dualDevInfo - dual device Info defined by per dualType
 * Output:
 *      N/A
 * Return:
 *	  0 	  - SUCCESS
 *	  others  - error code
 */

 int rtk_fc_dual_devInfoConfig(char* devName,rtk_fc_api_dualHdrtype_t dualType,rtk_fc_dual_devInfoConfig_t *dualDevInfo);



 /* Function Name:
 *      rtk_fc_ingress_flowLearning
 * Description:
 *      Flow-based HWNAT ingress learning module
 * Input:
 *      rtskb		- packet buffer translated from sk_buff struct
 *      pRxInfo		- provide ingress hw info
 * Output:
 *      N/A
 * Return:
 *      RE8670_RX_STOP			
 *      RE8670_RX_CONTINUE 
 *      RE8670_RX_STOP_SKBNOFREE
 */
int rtk_fc_ingress_flowLearning(struct rt_skbuff *rtskb, fc_rx_info_t *pRxInfo);
 
 /* Function Name:
 *      rtk_fc_egress_flowLearning
 * Description:
 *      Flow-based HWNAT egress learning module
 * Input:
 *      rtskb			- packet buffer translated from sk_buff struct
 *      dev			- net dev
 *      portMask 		- ethernet dev to tx port
 *      egrWlanDevIdx	- wlan dev index
 * Output:
 *      N/A
 * Return:
 *      0		- SUCCESS
 *      others 	- error code
 */
int rtk_fc_egress_flowLearning(struct rt_skbuff *rtskb, struct net_device *dev, rtk_fc_egrInfo_t *p_egrExtraInfo);

 /* Function Name:
 *      rtk_fc_fastfwd_ingress_rcv
 * Description:
 *      Flow-based HWNAT ingress learning module
 * Input:
 *      rtskb			- packet buffer translated from sk_buff struct
 * Output:
 *      N/A
 * Return:
 *      0		- SUCCESS
 *      others 	- error code
 */
int rtk_fc_fastfwd_ingress_rcv(struct rt_skbuff *rtskb);
 
 /* Function Name:
 *      rtk_fc_fastfwd_egress_xmit
 * Description:
 *      Flow-based HWNAT egress learning module for wlan traffic
 * Input:
 *      rtskb			- packet buffer translated from sk_buff struct
 *      dev			- net dev
 * Output:
 *      N/A
 * Return:
 *      0		- SUCCESS
 *      others 	- error code
 */
int rtk_fc_fastfwd_egress_xmit(struct rt_skbuff *rtskb, struct net_device *dev, rtk_fc_wlan_devidx_t wlanDevIdx);

 /* Function Name:
 *      rtk_fc_fastfwd_directXmit
 * Description:
 *      wifi fastforward: receiving skb from nic and forwarding to wifi driver tx without any processing
 * Input:
 *      skb				- skb data
 *      pRxDesc		- nic rx info
 * Output:
 *      N/A
 * Return:
 *      0		- SUCCESS
 *      others 	- error code
 */
 int rtk_fc_fastfwd_directXmit(struct sk_buff *skb, fc_rx_info_t *pRxInfo);

 /* Function Name:
 *      rtk_fc_helper_register
 * Description:
 *      manager module register helper function to core module
 * Input:
 *      type		- helper function sub-type
 *      api			- pointer for different helper functions
 * Output:
 *      N/A
 * Return:
 *      0		- SUCCESS
 *      others 	- error code
 */
int rtk_fc_helper_register(rtk_fc_helper_api_t type, void *api);


/* Function Name:
 *      rtk_fc_helper_unregister
 * Description:
 *      force remarking packet format according to specific patterns.
 * Input:
 *      type		- helper function sub-type
 * Output:
 *      N/A
 * Return:
 *      0		- SUCCESS
 *      others 	- error code
 */
int rtk_fc_helper_unregister(rtk_fc_helper_api_t type);

/* Function Name:
 *      rtk_fc_coreDBInfo_get
 * Description:
 *      Get fc db info from mgr module.
 * Input:
 *      opCode		  - info type
 *      pOpData         - the data from core module
 * Output:
 *      N/A
 * Return:
 *      0		- SUCCESS
 *      others 	- error code
 */

int rtk_fc_coreDBInfo_get(rtk_fc_coreDB_ops_t opCode, rtk_fc_coreDB_info_t* pOpData);

/* Function Name:
 *      rtk_fc_l2Info_get
 * Description:
 *      Get fc l2 mac information
 * Input:
 *      mac		- target mac address
 * Output:
 *      l2info		- port and other information 
 * Return:
 *      0		- SUCCESS
 *      others 	- error code
 */
int rtk_fc_l2Info_get(uint8 *mac, rtk_fc_mgr_l2_info_t *l2info);

#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
/* Function Name:
*	   rtk_fc_internal_wifi_amsdu_pe_offload_mac_id_set
* Description:
*	   set mac_id and its related info for Wifi PE offload.
* Input:
*	mac_id		- mac_id
*   sta_conf_sel- wifi STA configuration selection
*   sta_conf    - wifi STA information
*	mac_addr	- MAC address of mac_id
* Output:
*	  N/A
* Return:
*	   0			  - SUCCESS
*	   others		  - error code
*/
int rtk_fc_internal_wifi_amsdu_pe_offload_mac_id_set(unsigned int mac_id, rtk_fc_wifi_amsdu_pe_offload_sta_conf_sel_t sta_conf_sel, rtk_fc_wifi_amsdu_pe_offload_sta_info_t sta_conf, unsigned char* mac_addr);

/* Function Name:
*	   rtk_fc_internal_wifi_amsdu_pe_offload_mac_id_del
* Description:
*	   delete mac_id and its related info for Wifi PE offload.
* Input:
*	   mac_id		- mac_id
* Output:
*	  N/A
* Return:
*	   0			  - SUCCESS
*	   others		  - error code
*/
int rtk_fc_internal_wifi_amsdu_pe_offload_mac_id_del(unsigned int mac_id);

/* Function Name:
*	rtk_fc_internal_wifi_amsdu_pe_offload_mac_id_flush
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
int rtk_fc_internal_wifi_amsdu_pe_offload_mac_id_flush(void);

/* Function Name:
*   rtk_fc_internal_wifi_amsdu_pe_offload_mac_id_flow_flush
* Description:
*   clear all mac_id flows.
* Input:
*   N/A
* Output:
*   N/A
* Return:
*   0       - SUCCESS
*   others      - error code
*/
int rtk_fc_internal_wifi_amsdu_pe_offload_mac_id_flow_flush(void);

/* Function Name:
*   rtk_fc_internal_wifi_amsdu_pe_offload_en
* Description:
*   enable/disable wifi amsdu pe offload.
* Input:
*   mode - switching or not
*   pe_offload_wifi_amsdu_en - enable/disable wifi amsdu pe offload.
* Output:
*   N/A
* Return:
*   0       - SUCCESS
*   others      - error code
*/
int rtk_fc_internal_wifi_amsdu_pe_offload_en(rtk_pe_amsdu_mode_t mode, int pe_offload_wifi_amsdu_en);

/* Function Name:
*	rtk_fc_internal_wifi_amsdu_pe_offload_mode_set
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
int rtk_fc_internal_wifi_amsdu_pe_offload_mode_set(rtk_fc_wifi_amsdu_pe_offload_mode_t mode);

/* Function Name:
*	rtk_fc_internal_wifi_amsdu_pe_offload_mode_set
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
int rtk_fc_internal_wifi_amsdu_pe_offload_mode_get(rtk_fc_wifi_amsdu_pe_offload_mode_t *mode);

int rtk_fc_internal_wifi_dev_attr_set(rtk_fc_wlan_devidx_t wlanDevIdx, rtk_fc_wifi_dev_attr_t ori_attr, rtk_fc_wifi_dev_attr_t new_attr);

int rtk_fc_internal_amsdu_pe_offload_mc2uc(bool enable);

int rtk_fc_internal_amsdu_pe_offload_perDev_mc2uc(struct net_device *dev,bool enable);

/* Function Name:
*	rtk_fc_internal_wifi_amsdu_pe_offload_wifiPri_to_amsduQ_set
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
int rtk_fc_internal_wifi_amsdu_pe_offload_wifiPri_to_amsduQ_set(unsigned int wifi_pri, rtk_fc_wifi_amsdu_pe_queueid_t amsdu_qid);

/* Function Name:
*	rtk_fc_internal_wifi_amsdu_pe_offload_wifiPri_to_amsduQ_get
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
int rtk_fc_internal_wifi_amsdu_pe_offload_wifiPri_to_amsduQ_get(unsigned int wifi_pri, rtk_fc_wifi_amsdu_pe_queueid_t *amsdu_qid);
#endif

/* Function Name:
*	rtk_fc_internal_wifi_ipDscp_to_wifiPri_set
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
int rtk_fc_internal_wifi_ipDscp_to_wifiPri_set(unsigned int ip_dscp, unsigned int wifi_pri);

/* Function Name:
*	rtk_fc_internal_wifi_ipDscp_to_wifiPri_get
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
int rtk_fc_internal_wifi_ipDscp_to_wifiPri_get(unsigned int ip_dscp, unsigned int *wifi_pri);

int rtk_fc_devGwMacIdx_get(struct net_device *dev);
unsigned int rtk_fc_devGwPrivFlag_getByFcIdx(int entryIdx);

int rtk_fc_decision_ingress_interface(struct sk_buff *skb,struct net_device *dev0,struct net_device *dev1,struct net_device *dev2);
int rtk_fc_decision_egress_interface(struct sk_buff *skb,struct net_device *dev0,struct net_device *dev1,struct net_device *dev2);

typedef void (* fc_l2_refresh_notify)(unsigned char *mac, unsigned short vid, unsigned char anyflowestablish);
void rtk_fc_reg_lut_notify(fc_l2_refresh_notify notify_func);

int rtk_fc_core_init(void);
void rtk_fc_core_exit(void);

#if defined(CONFIG_RTK_SOC_RTL8198D)
uint rtk_fc_ext_lut_learning(uint8 *srcMac, rtk_fc_pmap_t igrPort, uint16 vid,
					rtk_fc_wlan_devidx_t wlan_dev_idx, uint8 verify_fdb, uint8 isStatic, int16 *macL2Idx, uint8 learning_state);
uint _rtk_fc_lut_find(uint8 *mac, int16 *lutIdx);
int _rtk_fc_lut_staticEntry_del(uint32 lutIdx);
#endif

#if defined(CONFIG_RTK_FC_PER_HW_FLOW_MIB)
 /* Function Name:
 *	rtk_fc_wlan_rx_flow_counter_by_skb
 * Description:
 *      counter hw flow packets and bytes
 * Input:
 *      skb		- skb
 *	wlan_dev_idx	- wlan_dev_idx
 *	path_mask	- flow path mask
 * Output:
 *      N/A
 * Return:
 *      0		- SUCCESS
 *      others 		- error code
 */
int rtk_fc_wlan_rx_flow_counter_by_skb(struct sk_buff *skb, unsigned int wlan_dev_idx, uint8 path_mask);

/* Function Name:
*	rtk_fc_wlan_tx_flow_counter_by_hash_crc
* Description:
*	counter hw flow packets and bytes
* Input:
*	skb		- skb
*	spa		- spa
*	crc16		- crc16 from nh_priv
*	crc32		- crc32 from nh_priv
*	path_mask	- flow path mask
* Output:
*	N/A
* Return:
*	0	   	- SUCCESS
*	others  	- error code
*/
int rtk_fc_wlan_tx_flow_counter_by_hash_crc(struct sk_buff *skb, int8 spa, uint16 crc16, uint32 crc32, uint8 path_mask);
#endif

#if defined(CONFIG_RTL8198X_SERIES)
/* Function Name:
*	rtk_fc_set_lanhost_routing_logging
* Description:
*	set lanhost_routing_logging control of host holicing
* Input:
*	value		- value
* Output:
*	N/A
* Return:
*	0		- SUCCESS
*/
uint8 rtk_fc_set_lanhost_routing_logging(uint8 value);

#if defined(CONFIG_RTK_FC_WFO_PER_HW_FLOW_MIB)
/* Function Name:
*	rtk_fc_wfo_clear_per_flow_mib_count_by_lut
* Description:
*	clear wfo per flow mib count by lut index
* Input:
*	lut_idx		- lut index
* Output:
*	N/A
* Return:
*	0		- SUCCESS
*/
int rtk_fc_wfo_clear_per_flow_mib_count_by_lut(int16 lut_idx);

/* Function Name:
*	_rtk_fc_wfo_get_ext_flow_mib_current_count_by_lut
* Description:
*	get ext_flowmib current count by lut index (no lock)
* Input:
*	lut_idx		- lut index
* Output:
*	p_counter	- current alive HW flow mib
*	p_sw_counter	- current alive SW flow mib
* Return:
*	0		- SUCCESS
*/
int _rtk_fc_wfo_get_ext_flow_mib_current_count_by_lut(int16 lut_idx, rtk_fc_ext_flow_mib_counter_t *p_counter, rtk_fc_ext_flow_mib_counter_t *p_sw_counter);

/* Function Name:
*	rtk_fc_wfo_get_ext_flow_mib_current_count_by_lut
* Description:
*	get ext_flowmib current count by lut index  (global lock)
* Input:
*	lut_idx		- lut index
* Output:
*	p_counter	- current alive HW flow mib
*	p_sw_counter	- current alive SW flow mib
* Return:
*	0		- SUCCESS
*/
int rtk_fc_wfo_get_ext_flow_mib_current_count_by_lut(int16 lut_idx, rtk_fc_ext_flow_mib_counter_t *p_counter, rtk_fc_ext_flow_mib_counter_t *p_sw_counter);

/* Function Name:
*	rtk_fc_lutExtFlowmibIdxUpdate
* Description:
*	update lut and ext flow_mib entry index reference
* Input:
*	mac		- mac addr
*	add		- add or delete operation
* Output:
*	N/A
* Return:
*	0		- SUCCESS
*/
int rtk_fc_lutExtFlowmibIdxUpdate(uint8 *mac, bool add);
#endif
#endif
#endif
