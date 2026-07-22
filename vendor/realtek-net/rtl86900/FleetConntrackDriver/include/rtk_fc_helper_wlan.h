#ifndef __RTK_FC_HELPER_WLAN__
#define __RTK_FC_HELPER_WLAN__

#include <uapi/linux/if.h>
#include <linux/netdevice.h>

#include <rtk_fc_port.h>
#include <rtk_fc_wfo.h>

#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
#if defined(CONFIG_REALTEK_IPC2RCPU)
typedef struct s_amsdu_setting_info {
	unsigned int mac_idx;
	unsigned int pkt_size;
	unsigned int wlan_dev_idx;
	unsigned int valid_bit;
} s_amsdu_setting_info;

#define RTK_FC_WIFI_PE_OFFLOAD_IPC_PKT_SIZE 1
#define RTK_FC_WIFI_PE_OFFLOAD_IPC_WLAN_IDX 2
#endif
#define RTK_FC_WIFI_PE_OFFLOAD_MAC_ID_SIZE					256
#define RTK_FC_WIFI_PE_OFFLOAD_VOQ_SIZE						40
#else
#define RTK_FC_WIFI_PE_OFFLOAD_MAC_ID_SIZE					0
#define RTK_FC_WIFI_PE_OFFLOAD_VOQ_SIZE						0
#endif

#if defined(CONFIG_RTK_FC_WIFI7_SUPPORT)
#ifdef CONFIG_MLO_INTF_NAME
#define WLANMLO_PREFIX				CONFIG_MLO_INTF_NAME
#else
#define WLANMLO_PREFIX				"wlanmlo"
#endif

#define WLAN_INTF_NAME2(prefix, postfix)			prefix#postfix
#define WLAN_INTF_NAME3(prefix, infix, postfix)		prefix#infix#postfix

#define WLANMLO_ROOT_INTF_NAME					WLANMLO_PREFIX
#define WLANMLO_VAP0_INTF_NAME					WLAN_INTF_NAME3(WLANMLO_ROOT_INTF_NAME, -vap, 0)
#define WLANMLO_VAP1_INTF_NAME					WLAN_INTF_NAME3(WLANMLO_ROOT_INTF_NAME, -vap, 1)
#define WLANMLO_VAP2_INTF_NAME					WLAN_INTF_NAME3(WLANMLO_ROOT_INTF_NAME, -vap, 2)
#define WLANMLO_VAP3_INTF_NAME					WLAN_INTF_NAME3(WLANMLO_ROOT_INTF_NAME, -vap, 3)
#define WLANMLO_VAP4_INTF_NAME					WLAN_INTF_NAME3(WLANMLO_ROOT_INTF_NAME, -vap, 4)
#define WLANMLO_VAP5_INTF_NAME					WLAN_INTF_NAME3(WLANMLO_ROOT_INTF_NAME, -vap, 5)
#define WLANMLO_VAP6_INTF_NAME					WLAN_INTF_NAME3(WLANMLO_ROOT_INTF_NAME, -vap, 6)
#define WLANMLO_VAP7_INTF_NAME					WLAN_INTF_NAME3(WLANMLO_ROOT_INTF_NAME, -vap, 7)
#define WLANMLO_CLIENT_INTF_NAME				WLAN_INTF_NAME2(WLANMLO_ROOT_INTF_NAME, -vxd)

#if 0
// wlanmlo.0
#define WLANMLO_0_ROOT_INTF_NAME				WLAN_INTF_NAME2(WLANMLO_PREFIX, .0)
#else
// wlanmlo1
#define WLANMLO_0_ROOT_INTF_NAME				WLAN_INTF_NAME2(WLANMLO_PREFIX, 1)
#endif

#define WLANMLO_0_VAP0_INTF_NAME				WLAN_INTF_NAME3(WLANMLO_0_ROOT_INTF_NAME, -vap, 0)
#define WLANMLO_0_VAP1_INTF_NAME				WLAN_INTF_NAME3(WLANMLO_0_ROOT_INTF_NAME, -vap, 1)
#define WLANMLO_0_VAP2_INTF_NAME				WLAN_INTF_NAME3(WLANMLO_0_ROOT_INTF_NAME, -vap, 2)
#define WLANMLO_0_VAP3_INTF_NAME				WLAN_INTF_NAME3(WLANMLO_0_ROOT_INTF_NAME, -vap, 3)
#define WLANMLO_0_VAP4_INTF_NAME				WLAN_INTF_NAME3(WLANMLO_0_ROOT_INTF_NAME, -vap, 4)
#define WLANMLO_0_VAP5_INTF_NAME				WLAN_INTF_NAME3(WLANMLO_0_ROOT_INTF_NAME, -vap, 5)
#define WLANMLO_0_VAP6_INTF_NAME				WLAN_INTF_NAME3(WLANMLO_0_ROOT_INTF_NAME, -vap, 6)
#define WLANMLO_0_VAP7_INTF_NAME				WLAN_INTF_NAME3(WLANMLO_0_ROOT_INTF_NAME, -vap, 7)
#define WLANMLO_0_CLIENT_INTF_NAME				WLAN_INTF_NAME2(WLANMLO_0_ROOT_INTF_NAME, -vxd)
#endif

typedef enum rtk_fc_wlanDev_e
{
#if defined(CONFIG_RTK_FC_WIFI7_SUPPORT)
	RTK_FC_WLAN_INVALID_INTF 		= (0),
	RTK_FC_WLAN0_ROOT_INTF 			= (1),
	RTK_FC_WLAN0_VAP0_INTF 			= (2),
	RTK_FC_WLAN0_VAP1_INTF 			= (3),
	RTK_FC_WLAN0_VAP2_INTF 			= (4),
	RTK_FC_WLAN0_VAP3_INTF 			= (5),
	RTK_FC_WLAN0_VAP4_INTF 			= (6),
	RTK_FC_WLAN0_VAP5_INTF 			= (7),
	RTK_FC_WLAN0_VAP6_INTF 			= (8),
	RTK_FC_WLAN0_VAP7_INTF 			= (9),
	RTK_FC_WLAN0_CLIENT_INTF 		= (10),	// e.g. vxd
	RTK_FC_WLAN0_MESH_INTF 			= (11),

	RTK_FC_WLAN1_ROOT_INTF 			= (12),
	RTK_FC_WLAN1_VAP0_INTF 			= (13),
	RTK_FC_WLAN1_VAP1_INTF 			= (14),
	RTK_FC_WLAN1_VAP2_INTF 			= (15),
	RTK_FC_WLAN1_VAP3_INTF 			= (16),
	RTK_FC_WLAN1_VAP4_INTF 			= (17),
	RTK_FC_WLAN1_VAP5_INTF 			= (18),
	RTK_FC_WLAN1_VAP6_INTF 			= (19),
	RTK_FC_WLAN1_VAP7_INTF 			= (20),
	RTK_FC_WLAN1_CLIENT_INTF 		= (21),
	RTK_FC_WLAN1_MESH_INTF 			= (22),

	RTK_FC_WLAN2_ROOT_INTF 			= (23),
	RTK_FC_WLAN2_VAP0_INTF 			= (24),
	RTK_FC_WLAN2_VAP1_INTF 			= (25),
	RTK_FC_WLAN2_VAP2_INTF 			= (26),
	RTK_FC_WLAN2_VAP3_INTF 			= (27),
	RTK_FC_WLAN2_VAP4_INTF 			= (28),
	RTK_FC_WLAN2_VAP5_INTF 			= (29),
	RTK_FC_WLAN2_VAP6_INTF 			= (30),
	RTK_FC_WLAN2_VAP7_INTF 			= (31),
	RTK_FC_WLAN2_CLIENT_INTF 		= (32),
	RTK_FC_WLAN2_MESH_INTF 			= (33),

	RTK_FC_WLANMLO_ROOT_INTF 		= (34),
	RTK_FC_WLANMLO_VAP0_INTF 		= (35),
	RTK_FC_WLANMLO_VAP1_INTF 		= (36),
	RTK_FC_WLANMLO_VAP2_INTF 		= (37),
	RTK_FC_WLANMLO_VAP3_INTF 		= (38),
	RTK_FC_WLANMLO_VAP4_INTF 		= (39),
	RTK_FC_WLANMLO_VAP5_INTF 		= (40),
	RTK_FC_WLANMLO_VAP6_INTF 		= (41),
	RTK_FC_WLANMLO_VAP7_INTF 		= (42),
	RTK_FC_WLANMLO_CLIENT_INTF 		= (43),

	RTK_FC_WLANMLO_0_ROOT_INTF 		= (44),
	RTK_FC_WLANMLO_0_VAP0_INTF 		= (45),
	RTK_FC_WLANMLO_0_VAP1_INTF 		= (46),
	RTK_FC_WLANMLO_0_VAP2_INTF 		= (47),
	RTK_FC_WLANMLO_0_VAP3_INTF 		= (48),
	RTK_FC_WLANMLO_0_VAP4_INTF 		= (49),
	RTK_FC_WLANMLO_0_VAP5_INTF 		= (50),
	RTK_FC_WLANMLO_0_VAP6_INTF 		= (51),
	RTK_FC_WLANMLO_0_VAP7_INTF 		= (52),
	RTK_FC_WLANMLO_0_CLIENT_INTF 	= (53),
#else
	RTK_FC_WLAN_INVALID_INTF = (0),
	RTK_FC_WLAN0_ROOT_INTF = (1),
	RTK_FC_WLAN0_VAP0_INTF,
	RTK_FC_WLAN0_VAP1_INTF,
	RTK_FC_WLAN0_VAP2_INTF,
	RTK_FC_WLAN0_VAP3_INTF,
	RTK_FC_WLAN0_VAP4_INTF ,
	RTK_FC_WLAN0_VAP5_INTF,
	RTK_FC_WLAN0_VAP6_INTF,
	RTK_FC_WLAN0_VAP7_INTF,
	RTK_FC_WLAN0_CLIENT_INTF,	// e.g. vxd
	RTK_FC_WLAN0_MESH_INTF,
	
	RTK_FC_WLAN1_ROOT_INTF = 16,
	RTK_FC_WLAN1_VAP0_INTF,
	RTK_FC_WLAN1_VAP1_INTF,
	RTK_FC_WLAN1_VAP2_INTF,
	RTK_FC_WLAN1_VAP3_INTF,
	RTK_FC_WLAN1_VAP4_INTF,
	RTK_FC_WLAN1_VAP5_INTF,
	RTK_FC_WLAN1_VAP6_INTF,
	RTK_FC_WLAN1_VAP7_INTF,
	RTK_FC_WLAN1_CLIENT_INTF,
	RTK_FC_WLAN1_MESH_INTF,
	
	RTK_FC_WLAN2_ROOT_INTF = 32,
	RTK_FC_WLAN2_VAP0_INTF,
	RTK_FC_WLAN2_VAP1_INTF,
	RTK_FC_WLAN2_VAP2_INTF,
	RTK_FC_WLAN2_VAP3_INTF,
	RTK_FC_WLAN2_VAP4_INTF,
	RTK_FC_WLAN2_VAP5_INTF,
	RTK_FC_WLAN2_VAP6_INTF,
	RTK_FC_WLAN2_VAP7_INTF,
	RTK_FC_WLAN2_CLIENT_INTF,
	RTK_FC_WLAN2_MESH_INTF,

	RTK_FC_WLANx_ATM_VC0_INTF = 46,
	RTK_FC_WLANx_ATM_VC1_INTF = 47,
	RTK_FC_WLANx_ATM_VC2_INTF = 48,
	RTK_FC_WLANx_ATM_VC3_INTF = 49,
	RTK_FC_WLANx_ATM_VC4_INTF = 50,
	RTK_FC_WLANx_ATM_VC5_INTF = 51,
	RTK_FC_WLANx_ATM_VC6_INTF = 52,
	RTK_FC_WLANx_ATM_VC7_INTF = 53,
#endif
	RTK_FC_WLANx_USB_INTF			= 54,
	RTK_FC_WLANx_PCIE_INTF			= 55,
	RTK_FC_COPY2CPU_INTF			= 56,
	RTK_FC_WLANX_END_INTF			= 60,
	RTK_FC_SWID_HW_SW_HYBRID_FWD	= 61,	// hw modify but to cpu for other features, e..g shaper, per flow mib cnt, ...
	RTK_FC_WLANX_MULTI_INTF			= 62,
	RTK_FC_WLANX_NOT_FOUND			= 63,	// to support 64 bits wlandevmask 
 }rtk_fc_wlan_devidx_t;

typedef unsigned long long  rtk_fc_wlan_devmask_t;

#if defined (CONFIG_RTK_L34_G3_PLATFORM)
typedef rtk_fc_wlan_devmask_t rtk_fc_ext_port_mask_t;

#define RTK_FC_WIFI_HWLOOKUP_ID_BURST_PKT 127
#endif

/************************* WLAN ******************************/
typedef enum rtk_fc_wlan_init_id_e
{
	RTK_FC_WLAN_INIT_ID_NONE=0,
	RTK_FC_WLAN_INIT_ID_0,
	RTK_FC_WLAN_INIT_ID_1,
	RTK_FC_WLAN_INIT_ID_2,
#if !defined(CONFIG_RTK_FC_WIFI7_SUPPORT)
	RTK_FC_WLAN_INIT_ID_MAX=4,
#else
	RTK_FC_WLAN_INIT_ID_MLO,
	RTK_FC_WLAN_INIT_ID_MLO_0,
	RTK_FC_WLAN_INIT_ID_MAX=6,
#endif
}rtk_fc_wlan_init_id_t;

typedef enum rtk_fc_wlan_id_e
{
	RTK_FC_WLAN_ID_0=0,
	RTK_FC_WLAN_ID_1=1,
	RTK_FC_WLAN_ID_2=2,
#if !defined(CONFIG_RTK_FC_WIFI7_SUPPORT)
	RTK_FC_WLAN_ID_AUTO=3,		// by wlan dex idx group
	RTK_FC_WLAN_ID_MAX=3,
#else
	RTK_FC_WLAN_ID_MLO=3,
	RTK_FC_WLAN_ID_MLO_0=4,
	RTK_FC_WLAN_ID_AUTO=5,		// by wlan dex idx group
	RTK_FC_WLAN_ID_MAX=5,
#endif
}rtk_fc_wlan_id_t;

#if !defined(CONFIG_RTK_FC_WIFI7_SUPPORT)
#define RTK_FC_WLAN_BAND_NUM	3           /* wlan0, wlan1, wlan2 */
#else
#define RTK_FC_WLAN_BAND_NUM	5           /* wlan0, wlan1, wlan2, wlanmlo, wlanmlo.0 */
#endif

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
#if defined(CONFIG_FC_RTL9607C_RTL9603CVD_HYBRID)
#define WLAN_CPU_BASIS_NUM	7
#elif defined(CONFIG_FC_RTL9607C_SERIES)
#define WLAN_CPU_BASIS_NUM	RTK_FC_MAC_PORT_SLAVECPU
#elif defined(CONFIG_FC_RTL9603CVD_SERIES)
#define WLAN_CPU_BASIS_NUM	RTK_FC_MAC_PORT_CPU
#endif
#elif defined (CONFIG_RTK_L34_G3_PLATFORM)
#define WLAN_CPU_BASIS_NUM	RTK_FC_MAC_PORT_WLAN_CPU0
#endif
#define WLAN_CPU_RANGE_NUM	(RTK_FC_MAC_PORT_MAX-WLAN_CPU_BASIS_NUM)
#define WLAN_EXTP_SIZE_SHIFT	3														/* support 8 ext port */

#define RTK_FC_WLAN_PORT_BUCKET_SIZE		(WLAN_CPU_RANGE_NUM<<WLAN_EXTP_SIZE_SHIFT)		/* idea: 3 cpu ports X 6 ext ports in ApolloPro, support 3*8=24 bucket*/
#define WLAN_MC_TO_UC_FF_MAX		8
#define WLAN_MC_TO_UC_MAC_POOL_MAX	64
#define WLAN_MC_SW_MACID_START		41
#define WLAN_MC_RESERVED_MACID0  (WLAN_MC_TO_UC_MAC_POOL_MAX+WLAN_MC_SW_MACID_START)
/************************************************************/

typedef struct rtk_fc_wlan_initmap_s
{
	rtk_fc_wlan_devidx_t wlanDevIdx;
	unsigned char ifname[IFNAMSIZ];
	rtk_fc_pmap_t portmap;
	unsigned char manuallyReg:1;
	unsigned char gmac_tx_trunking:1;
	rtk_fc_wlan_init_id_t initBand;
	rtk_fc_wifi_dev_attr_t attr;							// CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE
}rtk_fc_wlan_initmap_t;

typedef struct rtk_fc_wlan_devmap_s
{
	rtk_fc_wlan_devidx_t wlandevidx;
	// init by rtk_fc_wlan_portmap_register
	struct net_device *wlanDev;
	rtk_fc_pmap_t portmap;
	const struct net_device_ops *wlan_native_devops;

	rtk_fc_wlan_id_t band;

	unsigned char devHashIdx;
	rtk_fc_wifi_dev_attr_t attr;							// CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE
	unsigned char shareCpuPort:1;						//
	unsigned char shareExtPort:1;						// N wlan dev to 1 port
	unsigned char gmac_tx_trunking:1;					// use 3 gamc for wifi tx
	unsigned char valid:1;
	
	struct list_head ifidxDevList;
	struct list_head portDevList;
	// init by rtk_fc_wlan_txfunc_register
	struct net_device_ops wlan_fc_ops;
}rtk_fc_wlan_devmap_t;


typedef struct rtk_fc_wlan_ext_info_s
{
	union {
		rtk_fc_mac_ext_port_idx_t macExtPort;
		rtk_fc_wlan_devidx_t wlanDevIdx;
	};
}rtk_fc_wlan_ext_info_t;

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
typedef struct rtk_fc_wifi_flow_crtl_info_s
{
	uint32_t wlan0_accumulate_bit; //wlan0 accumulated packet bit count between wifi flow control detect interval
	uint32_t wlan1_accumulate_bit; //wlan1 accumulated packet bit count between wifi flow control detect interval
	uint32_t wifi_flow_ctrl_auto_en:1; //if enable wifi flow control auto enabling mechanism
}rtk_fc_wifi_flow_crtl_info_t;

typedef enum rtk_fc_wifi_gmac_sel_e
{
	RTK_FC_WIFI_GMAC_SEL_0 = 0,	// gmac 0, switch port 9
	RTK_FC_WIFI_GMAC_SEL_1,		// gmac 1, switch port 10
	RTK_FC_WIFI_GMAC_SEL_2,		// gmac 2, switch port 7
	RTK_FC_WIFI_GMAC_RR,			// gmac 0->1->(2)->0->1->(2)->...
	RTK_FC_WIFI_GMAC_TBD,
	RTK_FC_WIFI_GMAC_MAX,
}rtk_fc_wifi_gmac_sel_t;

#endif


int rtk_fc_wlan_init(void);


static inline uint32_t rtk_fc_wlan_ifidx_devlist_hash(struct net_device *dev)
{
	/* use dev memory address as unique key instead of ifindex */
#if defined(CONFIG_64BIT)
	uint32_t v = ((uint64_t)dev) & 0xffffffff;
#else
	uint32_t v = ((uint32_t)dev) & 0xffffffff;
#endif
	return (v%RTK_FC_WLANX_END_INTF);
}

static inline uint32_t rtk_fc_wlan_port_devlist_hash(rtk_fc_mac_port_idx_t macPort, rtk_fc_mac_ext_port_idx_t macExtPort)
{
	uint32_t basis, hvalue;

	basis = macPort - WLAN_CPU_BASIS_NUM;
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	hvalue = (basis << WLAN_EXTP_SIZE_SHIFT) + (macExtPort-RTK_FC_MAC_EXT_PORT0);
#elif defined (CONFIG_RTK_L34_G3_PLATFORM)
	hvalue = (basis << WLAN_EXTP_SIZE_SHIFT);
#endif


	hvalue %= RTK_FC_WLAN_PORT_BUCKET_SIZE;
	return hvalue;
}

static inline int rtk_fc_wlan_attr_cb_mask_val(rtk_fc_wlan_devmap_t *devmap)
{
	int mask = 0;
	if(!devmap) return mask;
	if(devmap->attr.nat25_cb) mask |= 0x1;
	if(devmap->attr.uc2mc_cb) mask |= 0x2;
	if(devmap->attr.lb_cb) mask |= 0x4;
	if(devmap->attr.igrPostFc_cb) mask |= 0x10;
	if(devmap->attr.igrPreFc_cb) mask |= 0x20;
	if(devmap->attr.egrPreFc_cb) mask |= 0x40;	
	return mask;
}

#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
int _rtk_fc_wifi_amsdu_pe_offload_mac_id_set(unsigned int mac_id, rtk_fc_wifi_amsdu_pe_offload_sta_conf_sel_t sta_conf_sel, rtk_fc_wifi_amsdu_pe_offload_sta_info_t sta_conf, unsigned char* mac_addr);
int _rtk_fc_wifi_amsdu_pe_offload_mac_id_del(unsigned int mac_id);
int _rtk_fc_wifi_amsdu_pe_offload_mac_id_flush(void);
int _rtk_fc_wifi_amsdu_pe_offload_mode_set(rtk_fc_wifi_amsdu_pe_offload_mode_t mode);
int _rtk_fc_wifi_amsdu_pe_offload_mode_get(rtk_fc_wifi_amsdu_pe_offload_mode_t *mode);
int _rtk_fc_wifi_dev_attr_set(struct net_device *dev, rtk_fc_wifi_dev_attr_t attr);
int _rtk_fc_wifi_dev_attr_get(struct net_device *dev, rtk_fc_wifi_dev_attr_t *attr);
int _rtk_fc_wifi_dev_to_devidx_get(struct net_device *dev, unsigned int *wlan_dev_idx);
int _rtk_fc_wifi_client_mode_cb_register(rtk_fc_wifi_callback pfunc);
int _rtk_fc_wifi_dmacHandle_cb_register(rtk_fc_wifi_dmacHandle_callback pfunc);
int _rtk_fc_wifi_amsdu_pe_offload_wifiPri_to_amsduQ_set(unsigned int wifi_pri, rtk_fc_wifi_amsdu_pe_queueid_t amsdu_qid);
int _rtk_fc_wifi_amsdu_pe_offload_wifiPri_to_amsduQ_get(unsigned int wifi_pri, rtk_fc_wifi_amsdu_pe_queueid_t *amsdu_qid);
#endif
int _rtk_fc_wifi_event_handling_cb_register(rtk_fc_wifi_event_handling_callback pfunc);
int _rtk_fc_wifi_ipDscp_to_wifiPri_set(unsigned int ip_dscp, unsigned int wifi_pri);
int _rtk_fc_wifi_ipDscp_to_wifiPri_get(unsigned int ip_dscp, unsigned int *wifi_pri);

#endif

