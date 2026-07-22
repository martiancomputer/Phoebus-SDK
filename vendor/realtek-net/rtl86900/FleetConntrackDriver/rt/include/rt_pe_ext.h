/*
 * Copyright (C) 2012 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 68395 $
 * $Date: 2016-05-27 16:38:35 +0800 (Fri, 27 May 2016) $
 *
 * Purpose : Definition of PE API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Configuration of http test
 *
 */


#ifndef __RT_PE_EXT_H__
#define __RT_PE_EXT_H__


/*
 * Include Files
 */
#include <common/rt_type.h>
/*
 * Symbol Definition
 */
//tc queue
#define RT_PE_TC_QUEUE_TEST_PAGE_ORDER (20-PAGE_SHIFT) // 1MB buffer size
#define MAX_PE_TC_QUEUE_TEST_QUEUE_PKT_SIZE (300)
#define MAX_PE_TC_QUEUE_TEST_PE_USED_NUM (2)
#define MAX_PE_TC_QUEUE_TEST_CONNECTION_NUM (8)
#define MAX_PE_TC_QUEUE_TEST_TC_SIGNATURE_PAGE_NUM (160)
#define RT_PE_TC_QUEUE_TEST_TC_SIGNATURE_PAGE_BUF_ORDER (20) // 1MB
#define RT_PE_TC_QUEUE_TEST_TC_SIGNATURE_PAGE_BUF_SIZE (0x1<<RT_PE_TC_QUEUE_TEST_TC_SIGNATURE_PAGE_BUF_ORDER)
#define RT_PE_TC_QUEUE_TEST_TC_SIGNATURE_LEN (20)
#define RT_PE_TC_QUEUE_TEST_TC_SIGNATURE_BUF_LEN RT_PE_TC_QUEUE_TEST_TC_SIGNATURE_LEN//(32)
#define RT_PE_TC_QUEUE_TEST_TC_SIGNATURE_BUF_NUM_PER_PAGE (RT_PE_TC_QUEUE_TEST_TC_SIGNATURE_PAGE_BUF_SIZE/RT_PE_TC_QUEUE_TEST_TC_SIGNATURE_BUF_LEN - 20)
#define RT_PE_TC_QUEUE_TEST_SW_ID_BASE (1)
#define MAX_PE_TC_QUEUE_TEST_MBYTE_COST (354) //max cost 354MB
#define MIN_PE_TC_QUEUE_TEST_MBYTE_COST (24) //min cost 22MB
#define DEF_PE_TC_QUEUE_TEST_MBYTE_COST (178) //default cost 178MB
#define DEF_PE_TC_QUEUE_TEST_SLAVE_PORT_MASK (0x5) // port 0, 2
#define RT_PE_TC_QUEUE_TEST_HWLOOKUP_LDPID (0x19)
#if 0 //move to rtk_fc_define.h
#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES)
#define RT_PE_TC_QUEUE_TEST_CPU_PORT (0x13)
#define RT_PE_TC_QUEUE_TEST_DMA_LSO_CPU_VP_ID (11)
#else	//CONFIG_FC_RTL9607F_SERIES
#define RT_PE_TC_QUEUE_TEST_CPU_PORT (0x11)
#define RT_PE_TC_QUEUE_TEST_DMA_LSO_CPU_VP_ID (7)
#endif
#endif
//queue
#define MIN_PE_QUEUE_TEST_BUF_ORDER (PAGE_SHIFT)
#define RT_PE_QUEUE_TEST_BYTE_COST_PER_QUEUE (128+8+4)
#define MAX_PE_QUEUE_TEST_QUEUE_SIZE (14<<20)
#define MAX_PE_QUEUE_TEST_MBYTE_COST ((RT_PE_QUEUE_TEST_BYTE_COST_PER_QUEUE*MAX_PE_QUEUE_TEST_QUEUE_SIZE)>>20) //max cost 1960MB
#define MIN_PE_QUEUE_TEST_MBYTE_COST (35) //min cost 35MB
#define DEF_PE_QUEUE_TEST_MBYTE_COST (140) //default cost 140MB
#define MAX_PE_QUEUE_TEST_ALLOC_PAGE_NUM (((MAX_PE_QUEUE_TEST_QUEUE_SIZE*128)>>MIN_PE_QUEUE_TEST_BUF_ORDER)+3) //(5*1048576*128/4096)+3
#define MAX_PE_QUEUE_TEST_LAN_PORT_NUM (7)
#define MAX_PE_QUEUE_TEST_LAN_PORT_MASK (0x7f)
#define MAX_PE_QUEUE_TEST_IPC_BUF_PAGE_NUM (60)
#define RT_PE_QUEUE_TEST_SHAPER_IPG_PROFILE_IDX (2)
#define RT_PE_QUEUE_TEST_SHAPER_IPG_SIZE (16)	//means preamble+IPG+SFD
#define RT_PE_QUEUE_TEST_SHAPER_EGRESS_RATE (999999)
#define RT_PE_QUEUE_TEST_TM_PORT_PRVT_BUFF_NUM (100)
#define RT_PE_QUEUE_TEST_SW_ID_BASE (1)
#if 0 //move to rtk_fc_define.h
#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES)
#define RT_PE_QUEUE_TEST_CPU_PORT (0x13)
#define RT_PE_QUEUE_TEST_DMA_LSO_CPU_VP_ID (11)
#else	//CONFIG_FC_RTL9607F_SERIES
#define RT_PE_QUEUE_TEST_CPU_PORT (0x11)
#define RT_PE_QUEUE_TEST_DMA_LSO_CPU_VP_ID (7)
#endif
#endif
#if defined(CONFIG_FC_CA8277B_SERIES)
#define RT_PE_QUEUE_TEST_DMA_LSO_LSPID (0x20)
#else //CONFIG_FC_RTL8277C_SERIES || CONFIG_FC_RTL9607F_SERIES
#define RT_PE_QUEUE_TEST_DMA_LSO_LSPID (0x11)
#endif
#define FTTR_MASTER_DOWNSTREAM_TX_PORT (6)
//crypto
#define RTK_FC_CRYPTO_MAX_IPC_DESC_NUM 25
#define RTK_FC_CRYPTO_MAX_ARRAY_SIZE 128
#define RT_PE_IPSEC_PAGE_ORDER (20-PAGE_SHIFT) // 1MB buffer size
#define RT_PE_IPSEC_DATA_PAGE_ORDER (18-PAGE_SHIFT) // 256KB buffer size
#define RT_PE_IPSEC_DMA_LSO_DECRYPT_HWLOOKUP_LSPID RTK_FC_IPSEC_HWLOOKUP_LSPID
#define RT_PE_IPSEC_DMA_LSO_FIRST_ENCRYPT_HWLOOKUP_LSPID (RTK_FC_PE_IPSEC_FIRST_ENCRYPT_HWLOOKUP_LSPID)
#if 0 //move to rtk_fc_define.h
#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES)
#define RT_PE_IPSEC_CPU_PORT (0x13)
#else	//CONFIG_FC_RTL9607F_SERIES
#define RT_PE_IPSEC_CPU_PORT (0x11)
#endif
#endif
#define MAX_PE_IPSEC_HW_KEY_NUM (12)
#define MAX_PE_IPSEC_ENCRYPTION_CONNECTION_NUM (4)
#define MAX_PE_IPSEC_DECRYPTION_CONNECTION_NUM (4)
#define RT_PE_IPSEC_SW_ID_DECRYPTION_BIT (0x20)
#define RT_PE_IPSEC_SW_ID_INNER_IPV6_BIT (0x10)
#define RT_PE_IPSEC_SW_ID_CONNECTION_IDX_MASK (0xf)
#define RT_PE_IPSEC_SW_ID_CONNECTION_IDX_BASE (1)
#define RT_PE_IPSEC_IV_LEN (16)
#if	(MAX_PE_IPSEC_ENCRYPTION_CONNECTION_NUM > 31) || (MAX_PE_IPSEC_DECRYPTION_CONNECTION_NUM > 31)
#error "PE IPSEC encrypt/decrypt connection num is overflow"
#endif
//http 
#define RT_PE_HTTP_TEST_PAGE_ORDER (20-PAGE_SHIFT) // 1MB buffer size
#define RT_PE_HTTP_TEST_DATA_PAGE_ORDER (20-PAGE_SHIFT) // 1MB buffer size
#define MAX_PE_HTTP_DOWNLOAD_CONNECTION_NUM (20)
#define MAX_PE_HTTP_UPLOAD_CONNECTION_NUM (20)
#define MAX_PE_HTTP_CONNECTION_NUM ((MAX_PE_HTTP_DOWNLOAD_CONNECTION_NUM>MAX_PE_HTTP_UPLOAD_CONNECTION_NUM) ? MAX_PE_HTTP_DOWNLOAD_CONNECTION_NUM : MAX_PE_HTTP_UPLOAD_CONNECTION_NUM)
#define MAX_PE_HTTP_REQ_URL_STR_LENGTH (220)
#define MAX_PE_HTTP_HOST_STR_LENGTH (100)
#define MAX_PE_HTTP_USER_AGENT_STR_LENGTH (30)
#define MAX_PE_HTTP_VERSION_STR_LENGTH (10)
#if 0 //move to rtk_fc_define.h
#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES)
#define RT_PE_HTTP_TEST_CPU_PORT (0x13)
#define RT_PE_HTTP_TEST_DMA_LSO_CPU_VP_ID (11)
#else	//CONFIG_FC_RTL9607F_SERIES
#define RT_PE_HTTP_TEST_CPU_PORT (0x11)
#define RT_PE_HTTP_TEST_DMA_LSO_CPU_VP_ID (7)
#endif
#endif
#if defined(CONFIG_FC_CA8277B_SERIES)
#define RT_PE_HTTP_TEST_DMA_LSO_LSPID (0x20)
#else //CONFIG_FC_RTL8277C_SERIES || CONFIG_FC_RTL9607F_SERIES
#define RT_PE_HTTP_TEST_DMA_LSO_LSPID (0x11)
#endif
#define MAX_PE_HTTP_TEST_CLS_NUM (2) //1 for ip fragment packet
#define RT_PE_HTTP_TEST_SW_ID (8)
//iperf
#define MAX_PE_IPERF_FIRST_PAYLOAD_LENGTH (20)
#define MAX_PE_IPERF_PAYLOAD_LENGTH (2048)
#define MAX_PE_IPERF_TEST_TIME_SEC (1800)
#define RT_PE_IPERF_DEFAULT_TEST_TIME_SEC (300)
#define RT_PE_IPERF_TEST_SW_ID (9)
#define RT_PE_IPERF_TEST_IP_FRAG_SW_ID (10)
//generic fwd
#define MAX_PE_GENERIC_FWD_DOWNLOAD_CONNECTION_NUM MAX_PE_HTTP_DOWNLOAD_CONNECTION_NUM
#define MAX_PE_GENERIC_FWD_UPLOAD_CONNECTION_NUM MAX_PE_HTTP_UPLOAD_CONNECTION_NUM
#define MAX_PE_GENERIC_FWD_CONNECTION_NUM MAX_PE_HTTP_CONNECTION_NUM
//AMSDU
#define RT_PE_AMSDU_OFLD_BUF_SEC 2
#define RT_PE_AMSDU_OFLD_TOTAL_PAGE_ORDER_NUM 10

//srv6
#if 0 //move to rtk_fc_define.h
#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES)
#define RT_PE_SRV6_CPU_PORT (0x13)
#else
#define RT_PE_SRV6_CPU_PORT (0x11)
#endif
#endif
#if defined(CONFIG_FC_RTL9607F_SERIES)
#define RT_PE_SRV6_PAGE_ORDER (7) // 512KB buffer size
#define RT_PE_SRV6_FREEPOOL_PAGE_ORDER (7) // 512KB buffer size
#define RT_PE_SRV6_PAGEINFO_NUMBER (3)
#else
#define RT_PE_SRV6_PAGE_ORDER (8) // 1MB buffer size
#define RT_PE_SRV6_FREEPOOL_PAGE_ORDER (7) // 512KB buffer size
#define RT_PE_SRV6_PAGEINFO_NUMBER (10)
#endif
//#define RT_PE_SRV6_RX_INIT_NUMBER (2400)
//#define RT_PE_SRV6_RX_ARRAY_SIZE (16)
#define MAX_PE_SRV6_ENCAP_OUTER_HDR_LENGTH (256)
#define MAX_PE_SRV6_ENCAP_CONNECTION_NUM (1)
#define MAX_PE_SRV6_DECAP_CONNECTION_NUM (1)
#define RT_PE_SRV6_SW_ID_DECAP_BIT (0x20)
#define RT_PE_SRV6_SW_ID_CONNECTION_IDX_MASK (0x1f)
#define RT_PE_SRV6_SW_ID_CONNECTION_IDX_BASE (1)
//#define RT_PE_SRV6_DMA_LSO_LSPID (0x20)

#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
#define RT_PE_HTTP_TEST_DMA_LSO_BACKPRESSURE (1)
#endif
#define RT_PE_DMA_LSO_SHAPER_RATE_1DOT25G 	(0x0012C000)  //1200M
#define RT_PE_DMA_LSO_SHAPER_RATE_2DOT5G 	(0x00258000)  //2400M
#define RT_PE_DMA_LSO_SHAPER_RATE_10G 		(0x00992000)  //9800M

/* number of microseconds per millisecond */
#define RT_PE_USEC_PER_MSEC (1000)

#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES)
#define RT_PE_MAX_CPU_NUM (2)
#else	//CONFIG_FC_RTL9607F_SERIES
#define RT_PE_MAX_CPU_NUM (3)
#endif

/*
 * Data Declaration
 */
typedef enum rt_pe_func_num_e
{
	RT_PE_FUNC_NUM_WIFI_TX_AMSDU	= 0,
	RT_PE_FUNC_NUM_HTTP				= 1,
	RT_PE_FUNC_NUM_CRYPTO			= 2,
	RT_PE_FUNC_NUM_WFO				= 3,
	RT_PE_FUNC_NUM_SRV6				= 4,
	RT_PE_FUNC_NUM_PKT_QUEUE		= 5,
	RT_PE_FUNC_NUM_PKT_TC_QUEUE_0	= 6,
	RT_PE_FUNC_NUM_PKT_TC_QUEUE_1	= 7,
	RT_PE_FUNC_NUM_PE_DE_AMSDU	    = 8,
	RT_PE_FUNC_NUM_MAX
} rt_pe_func_num_t;

typedef enum rt_pe_ret_e
{
	RT_PE_RET_NON_INIT			= -2,
	RT_PE_RET_FAIL				= -1,
	RT_PE_RET_OK				= 0,
	RT_PE_RET_PARAM_SIZE_ERROR	= 1,
	RT_PE_RET_EXISTED			= 2,
	RT_PE_RET_NOT_FOUND 		= 3,
} rt_pe_ret_t;

typedef enum rt_pe_common_info_num_s
{
	RT_PE_CMN_INFO_NUM_DEFAULT			= 0,
	RT_PE_CMN_INFO_NUM_WIFI_TX_AMSDU	= 1,
	RT_PE_CMN_INFO_NUM_WIFI_WFO			= 2,
	RT_PE_CMN_INFO_NUM_MAX
} rt_pe_common_info_num_t;

typedef enum rt_pe_common_req_cmd_e
{
	RT_PE_CMN_REQ_CMD_PON_SID_SYNC = 0,
	RT_PE_CMN_REQ_CMD_MAX
} rt_pe_common_req_cmd_t;

typedef enum rt_pe_common_ret_cmd_e
{
	RT_PE_CMN_RET_CMD_INIT_DATA_GET = 0,
	RT_PE_CMN_RET_CMD_MAX
} rt_pe_common_ret_cmd_t;

typedef enum rt_pe_queue_test_cmd_e
{
	RT_PE_QUEUE_TEST_CMD_ENABLE 	= 0,
	RT_PE_QUEUE_TEST_CMD_DISABLE 	= 1,
	RT_PE_QUEUE_TEST_CMD_STATUS_GET = 2,
	RT_PE_QUEUE_TEST_CMD_MAX
} rt_pe_queue_test_cmd_t;

typedef enum rt_pe_queue_test_return_cmd_e
{
	RT_PE_QUEUE_TEST_RETURN_CMD_FINISH 	= 0,
	RT_PE_QUEUE_TEST_RETURN_CMD_GET_BUF = 1,	
	RT_PE_QUEUE_TEST_RETURN_CMD_MAX
} rt_pe_queue_test_return_cmd_t;

typedef enum rt_pe_tc_queue_test_cmd_e
{
	RT_PE_TC_QUEUE_TEST_CMD_ENABLE 			= 0,
	RT_PE_TC_QUEUE_TEST_CMD_DISABLE 		= 1,
	RT_PE_TC_QUEUE_TEST_CMD_STATUS_GET 		= 2,
	RT_PE_TC_QUEUE_TEST_CMD_CONNECTION_ADD 	= 3,
	RT_PE_TC_QUEUE_TEST_CMD_CONNECTION_DEL 	= 4,
	RT_PE_TC_QUEUE_TEST_CMD_MAX
} rt_pe_tc_queue_test_cmd_t;

typedef enum rt_pe_tc_queue_test_return_cmd_e
{
	RT_PE_TC_QUEUE_TEST_RETURN_CMD_FINISH 	= 0,
	RT_PE_TC_QUEUE_TEST_RETURN_CMD_GET_BUF 	= 1,
	RT_PE_TC_QUEUE_TEST_RETURN_CMD_MAX
} rt_pe_tc_queue_test_return_cmd_t;

typedef enum 
{
	RT_PE_CRYPTO_EALG_AES_128	= 0,
	RT_PE_CRYPTO_EALG_AES_192	= 1,
	RT_PE_CRYPTO_EALG_AES_256	= 2,
	RT_PE_CRYPTO_EALG_DES		= 3,
	RT_PE_CRYPTO_EALG_3DES		= 4,
	RT_PE_CRYPTO_EALG_MAX
} rt_pe_crypto_ealg_t;

typedef enum 
{
	RT_PE_CRYPTO_AALG_MD5HMAC 		= 0,
	RT_PE_CRYPTO_AALG_SHA1HMAC 		= 1,
	RT_PE_CRYPTO_AALG_SHA2_224HMAC 	= 2,
	RT_PE_CRYPTO_AALG_SHA2_256HMAC 	= 3,
	RT_PE_CRYPTO_AALG_MAX
} rt_pe_crypto_aalg_t;

typedef enum rt_pe_crypto_engine_cmd_e
{
	RT_PE_CRYPTO_ENGINE_CMD_ENABLE 					= 0,
	RT_PE_CRYPTO_ENGINE_CMD_DISABLE 				= 1,
	RT_PE_CRYPTO_ENGINE_CMD_ENCRYPT_CONNECTION_ADD 	= 2,
	RT_PE_CRYPTO_ENGINE_CMD_ENCRYPT_CONNECTION_DEL 	= 3,
	RT_PE_CRYPTO_ENGINE_CMD_DECRYPT_CONNECTION_ADD 	= 4,
	RT_PE_CRYPTO_ENGINE_CMD_DECRYPT_CONNECTION_DEL 	= 5,
	RT_PE_CRYPTO_ENGINE_CMD_STATUS_GET			 	= 6,
	RT_PE_CRYPTO_ENGINE_CMD_PS_SEND_DESC			= 7,
	RT_PE_CRYPTO_ENGINE_CMD_MAX
} rt_pe_crypto_engine_cmd_t;

typedef enum rt_pe_amsdu_ofld_cmd_t
{
	RT_PE_AMSDU_OFLD_CMD_GET_PAGE 					= 0,
	RT_PE_AMSDU_OFLD_CMD_MAX
} rt_pe_amsdu_ofld_cmd_t;

typedef enum rt_pe_crypto_engine_return_cmd_e
{
	RT_PE_CRYPTO_ENGINE_RETURN_CMD_FINISH	= 0,
	RT_PE_CRYPTO_ENGINE_HW_CRYPTO_READY		= 1,
	RT_PE_CRYPTO_ENGINE_RETURN_CMD_MAX
} rt_pe_crypto_engine_return_cmd_t;

typedef enum rt_pe_generic_fwd_req_cmd_e
{
	RT_PE_GENERIC_FWD_REQ_CMD_DOWNLOAD_START = 0,
	RT_PE_GENERIC_FWD_REQ_CMD_DOWNLOAD_STOP,
	RT_PE_GENERIC_FWD_REQ_CMD_DOWNLOAD_GET_CNT,
	RT_PE_GENERIC_FWD_REQ_CMD_UPLOAD_START,
	RT_PE_GENERIC_FWD_REQ_CMD_UPLOAD_STOP,
	RT_PE_GENERIC_FWD_REQ_CMD_UPLOAD_GET_CNT,
	RT_PE_GENERIC_FWD_REQ_CMD_MAX
} rt_pe_generic_fwd_req_cmd_t;

typedef enum rt_pe_generic_fwd_ret_cmd_e
{
	RT_PE_GENERIC_FWD_RET_CMD_DOWNLOAD_FINISH = 0,
	RT_PE_GENERIC_FWD_RET_CMD_DOWNLOAD_CNT,
	RT_PE_GENERIC_FWD_RET_CMD_UPLOAD_FINISH,
	RT_PE_GENERIC_FWD_RET_CMD_UPLOAD_CNT,
	RT_PE_GENERIC_FWD_RET_CMD_MAX
} rt_pe_generic_fwd_ret_cmd_t;

typedef enum rt_pe_generic_fwd_l4_proto_e
{
	RT_PE_GENERIC_FWD_L4_PROTO_TCP = 0,
	RT_PE_GENERIC_FWD_L4_PROTO_UDP,
	RT_PE_GENERIC_FWD_L4_PROTO_MAX
} rt_pe_generic_fwd_l4_proto_t;

typedef enum rt_pe_generic_fwd_state_e
{
	RT_PE_GENERIC_FWD_STATE_DISABLED = 0,
	RT_PE_GENERIC_FWD_STATE_COLLECT_INFO,
	RT_PE_GENERIC_FWD_STATE_PE_RUNNING
} rt_pe_generic_fwd_state_t;

typedef enum rt_pe_iperf_req_cmd_e
{
	RT_PE_IPERF_REQ_CMD_DOWNLOAD_START_UDP = 0,
	RT_PE_IPERF_REQ_CMD_DOWNLOAD_START_TCP,
	RT_PE_IPERF_REQ_CMD_DOWNLOAD_STOP,
	RT_PE_IPERF_REQ_CMD_DOWNLOAD_GET_CNT,
	RT_PE_IPERF_REQ_CMD_UPLOAD_START_UDP,
	RT_PE_IPERF_REQ_CMD_UPLOAD_START_TCP,
	RT_PE_IPERF_REQ_CMD_UPLOAD_STOP,
	RT_PE_IPERF_REQ_CMD_UPLOAD_GET_CNT,
	RT_PE_IPERF_REQ_CMD_MAX
} rt_pe_iperf_req_cmd_t;

typedef enum rt_pe_iperf_ret_cmd_e
{
	RT_PE_IPERF_RET_CMD_DOWNLOAD_FINISH = 0,
	RT_PE_IPERF_RET_CMD_DOWNLOAD_CNT 	= 1,
	RT_PE_IPERF_RET_CMD_UPLOAD_FINISH 	= 2,
	RT_PE_IPERF_RET_CMD_UPLOAD_CNT 		= 3,
	RT_PE_IPERF_RET_CMD_MAX
} rt_pe_iperf_ret_cmd_t;

typedef enum rt_pe_http_test_cmd_e
{
	RT_PE_HTTP_TEST_CMD_DOWNLOAD_START = 0,
	RT_PE_HTTP_TEST_CMD_DOWNLOAD_STOP,
	RT_PE_HTTP_TEST_CMD_DOWNLOAD_GET_CNT,
	RT_PE_HTTP_TEST_CMD_UPLOAD_START,
	RT_PE_HTTP_TEST_CMD_UPLOAD_STOP,
	RT_PE_HTTP_TEST_CMD_UPLOAD_GET_CNT,
	RT_PE_HTTP_TEST_CMD_LATENCY_START,
	RT_PE_HTTP_TEST_CMD_LATENCY_STOP,
	RT_PE_HTTP_TEST_CMD_LATENCY_GET_RESULT,
	RT_PE_HTTP_TEST_CMD_MAX
} rt_pe_http_test_cmd_t;

typedef enum rt_pe_http_test_return_cmd_e
{
	RT_PE_HTTP_TEST_RETURN_CMD_DOWNLOAD_FINISH 	= 0,
	RT_PE_HTTP_TEST_RETURN_CMD_DOWNLOAD_CNT 	= 1,
	RT_PE_HTTP_TEST_RETURN_CMD_UPLOAD_FINISH 	= 2,
	RT_PE_HTTP_TEST_RETURN_CMD_UPLOAD_CNT 		= 3,
	RT_PE_HTTP_TEST_RETURN_CMD_LATENCY_FINISH 	= 4,
	RT_PE_HTTP_TEST_RETURN_CMD_LATENCY_RESULT 	= 5,
	RT_PE_HTTP_TEST_RETURN_CMD_MAX
} rt_pe_http_test_return_cmd_t;

typedef enum {
	RTK_PE_HTTP_TEST_MODE_NORMAL 				= 0,
	RTK_PE_HTTP_TEST_MODE_SKIP_GET_POST 		= 1,	//skip get/post packet
	RTK_PE_HTTP_TEST_MODE_SKIP_ALL_CTRL 		= 2,	//skip all control packets
	RTK_PE_HTTP_TEST_MODE_MAX
} rtk_pe_http_test_mode_t;

typedef enum {
	RTK_PE_AMSDU_MODE_DISABLE_FLOW			= 0,
	RTK_PE_AMSDU_MODE_FUNC_SWITCHING 		= 1,
	RTK_PE_AMSDU_MODE_MAX
} rtk_pe_amsdu_mode_t;

typedef enum rt_pe_srv6_offload_cmd_e
{
	RT_PE_SRV6_OFFLOAD_CMD_ENABLE 					= 0,
	RT_PE_SRV6_OFFLOAD_CMD_DISABLE 					= 1,
	RT_PE_SRV6_OFFLOAD_CMD_ENCAP_CONNECTION_ADD 	= 2,
	RT_PE_SRV6_OFFLOAD_CMD_ENCAP_CONNECTION_DEL 	= 3,
	RT_PE_SRV6_OFFLOAD_CMD_DECAP_CONNECTION_ADD 	= 4,
	RT_PE_SRV6_OFFLOAD_CMD_DECAP_CONNECTION_DEL 	= 5,
	RT_PE_SRV6_OFFLOAD_CMD_STATUS_GET			 	= 6,
	RT_PE_SRV6_OFFLOAD_CMD_MAX
} rt_pe_srv6_offload_cmd_t;

typedef enum rt_pe_srv6_offload_return_cmd_e
{
	RT_PE_SRV6_OFFLOAD_RETURN_CMD_FINISH	= 0,
	RT_PE_SRV6_OFFLOAD_RETURN_CMD_MAX
} rt_pe_srv6_offload_return_cmd_t;

typedef struct rt_pe_page_info_s {
	uint32 order; 			/* each page size is 4KB, allocate 2^order consecutive pages => if order is 10, it allocates 4MB(4KB * 2^10) . */
	struct page *pPages;
}rt_pe_page_info_t;

typedef struct rt_pe_buf_info_s {
	uint32 buf_phy_addr; 	/* physical address of allocated buffer */
	uint32 buf_size;	 	/* total allocated buffer size */
}rt_pe_buf_info_t;

typedef struct rt_pe_status_s
{
	volatile int32 *pStatus;	// non-cached, refer to rt_pe_ret_t
	uint32 status_phy_addr;
}rt_pe_status_t;

typedef struct rt_pe_common_info_s {
	uint32 ni_cpu_port_id;
	uint32 ni_cpu_eqid0;
	uint32 ni_cpu_eqid1;
	uint32 ni_cpu_eq_profile_id;
	uint32 ni_cpu_epp_fifo_profile_id;
	uint32 ni_dmalso_vp_id;
}rt_pe_common_info_t;

typedef struct rt_pe_func_common_info_s {
	rt_pe_common_info_t cmn_info[RT_PE_CMN_INFO_NUM_MAX];
}rt_pe_func_common_info_t;

typedef struct rt_pe_common_info_req_s {
	uint32 pe_cpu_num;
}rt_pe_common_info_req_t;

typedef struct rt_pe_common_info_ret_s {
	rt_pe_ret_t ret_val;
	rt_pe_func_common_info_t func_cmn_info;
}rt_pe_common_info_ret_t;

typedef struct rt_stream_id_s
{
	//tx desc for stream id
	int32 sid;		// -1 invalid
	//header_a for stream id
	uint8 ldpid; 	//tcont, -1 invalid
	uint8 cos;
	uint16 flowid; 	//gemid
} rt_stream_id_t;

typedef struct rt_pe_tcp_info_s
{
	uint16 ipv4_id;
	uint8 client_window_scale;
	uint8 server_window_scale;
	uint32 seq_num;
	uint32 ack_num;
	uint32 mss_value;
}rt_pe_tcp_info_t;

typedef struct rt_pe_http_test_request_s
{
	rt_pe_http_test_cmd_t req_cmd;
		
	rtk_mac_t server_mac;
	rtk_mac_t client_mac;
	uint8 isIPv4OrIpv6; // 0: ipv4, 1: ipv6
	union{
		rtk_ip_addr_t ipv4_addr;
		rtk_ipv6_addr_t ipv6_addr;
	}client_ip;
	union{
		rtk_ip_addr_t ipv4_addr;
		rtk_ipv6_addr_t ipv6_addr;
	}server_ip;
	uint16 client_l4port;
	uint16 server_l4port;

	uint32 ldpid;
	int16 pppoe_sid;	// -1 invalid
	uint8 stpid_sel_en;
	uint8 stpid_sel;
	int16 svlan_vid;	// -1 invalid
	int16 cvlan_vid;	// -1 invalid
	int16 pon_streamId;	// -1 invalid

	uint32 connection_number;		// MAX_PE_HTTP_DOWNLOAD_CONNECTION_NUM, MAX_PE_HTTP_UPLOAD_CONNECTION_NUM

	// for download
	uint8 congestion_mode;
	uint8 non_congestion_completely_done;
	// for upload
	uint64 upload_content_length; 	// unit: bytes
	uint32 tcp_window_size; 		// unit: bytes, 0: means it uses default window size
	uint16 tcp_mss_size;			// unit: bytes, 0: means it uses default value

	rtk_pe_http_test_mode_t test_mode;
	// for test_mode is RTK_PE_HTTP_TEST_MODE_SKIP_ALL_CTRL
	rt_pe_tcp_info_t tcp_info[MAX_PE_HTTP_CONNECTION_NUM];
	// for test_mode is RTK_PE_HTTP_TEST_MODE_NORMAL
	uint8 http_req_url[MAX_PE_HTTP_REQ_URL_STR_LENGTH]; 	//e.g., "/garbage.php?ckSize=5000"
	uint8 http_host[MAX_PE_HTTP_HOST_STR_LENGTH];
	uint8 http_user_agent[MAX_PE_HTTP_USER_AGENT_STR_LENGTH];
	uint8 http_version[MAX_PE_HTTP_VERSION_STR_LENGTH]; 	//e.g., "HTTP/1.1"
	uint8 http_host_valid;
	uint8 http_user_agent_valid;
}rt_pe_http_test_request_t;

typedef struct rt_pe_http_test_result_s
{
	rt_pe_ret_t ret_val;
	uint32 pktCnt;
	uint64 byteCnt;					//payload only
	uint64 byteCnt_include_pktHdr;	//L2+L3+L4+payload
	//counter of each connection
	uint32 pktCnt_conn[MAX_PE_HTTP_CONNECTION_NUM];
	uint64 byteCnt_conn[MAX_PE_HTTP_CONNECTION_NUM];						//payload only
	uint64 byteCnt_include_pktHdr_conn[MAX_PE_HTTP_CONNECTION_NUM];			//L2+L3+L4+payload
	//the unit of following time is milliseconds 
	uint64 TCP_openRequestTime_ms; //It's relative time and is always zero
	uint64 TCP_openResponseTime_ms;
	uint64 HTTP_ROMTime_ms; //http get
	uint64 HTTP_BOMTime_ms; //start transmission
	uint64 HTTP_EOMTime_ms; //end transmission
	uint64 latency_us;	//microsecond
	uint64 jitter_us;	//microsecond
	uint16 pktLoss;
	uint16 pktRcv;
}rt_pe_http_test_result_t;

typedef struct rt_pe_crypto_encrypt_info_s
{
	int32 key_idx;
	int32 hash_key_idx;
	rt_pe_crypto_ealg_t cipher_mode;
	rt_pe_crypto_aalg_t hash_mode;
	uint32 iv_len;
	uint32 hash_icv_len;
	
	rtk_mac_t outer_dmac;
	rtk_mac_t outer_smac;
	uint8 outer_isIPv4OrIpv6; // 0: ipv4, 1: ipv6
	union{
		rtk_ip_addr_t ipv4_addr;
		rtk_ipv6_addr_t ipv6_addr;
	}outer_sip;
	union{
		rtk_ip_addr_t ipv4_addr;
		rtk_ipv6_addr_t ipv6_addr;
	}outer_dip;
	uint8 is_NATT;
	uint16 encap_sport;
	uint16 encap_dport;

	uint32 esp_spi;
	uint32 esp_seq_no;
	uint8 iv[RT_PE_IPSEC_IV_LEN];
	
	uint32 ldpid;
	union{
		struct{
			int16 pppoe_sid;	// -1 invalid
			int16 svlan_vid;	// -1 invalid
			int16 cvlan_vid;	// -1 invalid
			int16 pon_streamId;	// -1 invalid
			uint16 mtu_value;
		}external_used;
		struct{
			int32 dma_aft_idx; // -1 invalid, HW offload for PPPoE sid, svlan, cvlan 
			rt_stream_id_t streamId;
			uint32 segment_size; // for dmalso to do fragment
		}internal_used;
	}tag_info;
}rt_pe_crypto_encrypt_info_t;

typedef struct rt_pe_crypto_decrypt_info_s
{
	int32 key_idx;
	int32 hash_key_idx;
	rt_pe_crypto_ealg_t cipher_mode;
	rt_pe_crypto_aalg_t hash_mode;
	uint32 iv_len;
	uint32 hash_icv_len;
	
	uint8 outer_isIPv4OrIpv6; // 0: ipv4, 1: ipv6
	uint8 is_NATT;
	
	rtk_mac_t dmac;
	rtk_mac_t smac;

	uint32 hwlookup_swId;
	uint32 hwlookup_lspid;
}rt_pe_crypto_decrypt_info_t;

typedef struct rt_pe_crypto_request_s
{
	rt_pe_crypto_engine_cmd_t req_cmd;
	uint32 connection_idx;
	union{
		rt_pe_crypto_encrypt_info_t encrypt_info;
		rt_pe_crypto_decrypt_info_t decrypt_info;
	}connection;
	uint32 buf_phy_addr; 	/* physical address of allocated buffer */
	uint32 buf_size;	 	/* total allocated buffer size */
	uint32 data_info_phy_addr; 	/* physical address of data info */
	uint32 data_info_size;	 	/* total allocated data info size */ 
	uint32 ps_wait_desc_done_phy_addr;
	uint32 ps_wait_pop_done_phy_addr;
}rt_pe_crypto_request_t;

typedef struct rt_pe_queue_test_request_s
{
	rt_pe_queue_test_cmd_t req_cmd;
	uint32 queue_size;
	uint32 init_done_phy_addr;
	rt_pe_buf_info_t init_pkt_buf;
	rt_pe_buf_info_t ring_buf;
	rt_pe_buf_info_t sw_pool;
	rt_stream_id_t streamId;
	uint32 ni_pkt_buf_len;
	uint32 ni_pkt_buf_mask;
}rt_pe_queue_test_request_t;

typedef struct rt_pe_queue_test_buf_req_s
{
	uint32 buf_page_idx;
}rt_pe_queue_test_buf_req_t;

typedef struct rt_pe_queue_test_buf_ret_s
{
	uint32 buf_page_num;
	rt_pe_buf_info_t buf_page[MAX_PE_QUEUE_TEST_IPC_BUF_PAGE_NUM];
}rt_pe_queue_test_buf_ret_t;

typedef struct rt_pe_tc_queue_test_conn_info_s
{
	uint32 connection_idx;
	uint8 ipv4_en;
	uint8 ipv6_en;
	uint8 tcp_en;
	uint8 udp_en;
	uint8 lspid;
	uint8 pol_grp_id;
	uint8 ldpid;
	uint8 cos;
	uint8 fe_bypass;
	uint16 flowid;
}rt_pe_tc_queue_test_conn_info_t;

typedef struct rt_pe_tc_queue_test_conn_data_s
{
	int32 flow_idx; // -1 invalid
	uint32 cls_idx;
	rt_pe_tc_queue_test_conn_info_t conn_info;
}rt_pe_tc_queue_test_conn_data_t;

typedef struct rt_pe_tc_queue_test_rsv_cls_info_s
{
	uint32 lspid;
	uint8 smac[ETHER_ADDR_LEN];
	uint8 dmac[ETHER_ADDR_LEN];
	uint8 isIPv4OrIpv6; // 0: ipv4, 1: ipv6
	rtk_ip_addr_t ipv4_dip_addr;
	rtk_ipv6_addr_t ipv6_dip_addr;
	uint32 action_lspid;
}rt_pe_tc_queue_test_rsv_cls_info_t;

typedef struct rt_pe_tc_queue_test_request_s
{
	rt_pe_tc_queue_test_cmd_t req_cmd;
	uint32 queue_size;
	uint32 init_done_phy_addr;
	rt_pe_buf_info_t init_pkt_buf;
	rt_pe_buf_info_t ring_buf;
	rt_pe_tc_queue_test_conn_info_t conn_info;
	uint32 ldma_delay_us;
}rt_pe_tc_queue_test_request_t;

typedef rt_pe_queue_test_buf_req_t rt_pe_tc_queue_test_buf_req_t;

typedef rt_pe_queue_test_buf_ret_t rt_pe_tc_queue_test_buf_ret_t;

typedef struct rt_pe_iperf_request_s
{
	rt_pe_iperf_req_cmd_t req_cmd;
	
	rtk_mac_t server_mac;
	rtk_mac_t client_mac;
	uint8 generic_mode;
	uint8 server_mode;
	uint8 isIPv4OrIpv6; // 0: ipv4, 1: ipv6
	union{
		rtk_ip_addr_t ipv4_addr;
		rtk_ipv6_addr_t ipv6_addr;
	}client_ip;
	union{
		rtk_ip_addr_t ipv4_addr;
		rtk_ipv6_addr_t ipv6_addr;
	}server_ip;
	uint16 client_l4port[MAX_PE_HTTP_CONNECTION_NUM];
	uint16 server_l4port[MAX_PE_HTTP_CONNECTION_NUM];

	uint32 ldpid;
	int16 pppoe_sid;	// -1 invalid
	uint8 stpid_sel_en;
	uint8 stpid_sel;
	int16 svlan_vid;	// -1 invalid
	int16 cvlan_vid;	// -1 invalid
	int16 pon_streamId;	// -1 invalid

	uint32 test_time_sec;
	uint8 connection_number;
	//TCP 
	uint8 tcp_congestion_mode;			// for download
	uint16 tcp_mss_size;				// unit: bytes, 0: means it uses default value
	uint64 tcp_upload_content_length; 	// unit: bytes	
	//UDP
	uint16 udp_upload_payload_length; 			// unit: bytes
	uint16 udp_upload_first_payload_length; 	// unit: bytes
	uint8 udp_upload_first_payload[MAX_PE_IPERF_FIRST_PAYLOAD_LENGTH];
	uint32 udp_upload_bandwidth_mbps;
}rt_pe_iperf_request_t;

typedef struct rt_pe_iperf_udp_request_s
{
	rtk_mac_t server_mac;
	rtk_mac_t client_mac;
	uint8 generic_mode;
	uint8 server_mode;
	uint8 isIPv4OrIpv6; // 0: ipv4, 1: ipv6
	union{
		rtk_ip_addr_t ipv4_addr;
		rtk_ipv6_addr_t ipv6_addr;
	}client_ip;
	union{
		rtk_ip_addr_t ipv4_addr;
		rtk_ipv6_addr_t ipv6_addr;
	}server_ip;
	uint16 client_l4port[MAX_PE_HTTP_CONNECTION_NUM];
	uint16 server_l4port[MAX_PE_HTTP_CONNECTION_NUM];

	uint32 ldpid;
	int32 dma_aft_idx; // -1 invalid, HW offload for PPPoE sid, svlan, cvlan 
	uint32 dma_aft_hdr_offset;
	rt_stream_id_t streamId;

	uint32 test_time_sec;
	uint32 connection_number;	// MAX_PE_HTTP_DOWNLOAD_CONNECTION_NUM, MAX_PE_HTTP_UPLOAD_CONNECTION_NUM

	uint32 buf_phy_addr; 		/* physical address of allocated buffer */
	uint32 buf_size;	 		/* total allocated buffer size */ 
	uint32 data_info_phy_addr; 	/* physical address of data info */
	uint32 data_info_size;	 	/* total allocated data info size */ 
	uint32 status_phy_addr;
	
	uint16 upload_payload_length; 			// unit: bytes
	uint16 upload_first_payload_length; 	// unit: bytes
	uint8 upload_first_payload[MAX_PE_IPERF_FIRST_PAYLOAD_LENGTH];
}rt_pe_iperf_udp_request_t;

typedef struct rt_pe_iperf_result_s
{
	rt_pe_ret_t ret_val;
	uint32 pktCnt;
	uint64 byteCnt;							//payload only
	uint64 byteCnt_include_pktHdr;			//L2+L3+L4+payload
	//the unit of following time is milliseconds 
	uint64 cost_time_ms;
	//counter of each connection
	uint32 pktCnt_conn[MAX_PE_HTTP_CONNECTION_NUM];
	uint64 byteCnt_conn[MAX_PE_HTTP_CONNECTION_NUM];						//payload only
	uint64 byteCnt_include_pktHdr_conn[MAX_PE_HTTP_CONNECTION_NUM];			//L2+L3+L4+payload
}rt_pe_iperf_result_t;

typedef struct rt_pe_generic_pkt_pattern_s
{
	rtk_mac_t server_mac;
	rtk_mac_t client_mac;
	uint8 isIPv4OrIpv6; // 0: ipv4, 1: ipv6
	union{
		rtk_ip_addr_t ipv4_addr;
		rtk_ipv6_addr_t ipv6_addr;
	}client_ip;
	union{
		rtk_ip_addr_t ipv4_addr;
		rtk_ipv6_addr_t ipv6_addr;
	}server_ip;
	rt_pe_generic_fwd_l4_proto_t l4_proto; // 0: TCP, 1: UDP
	uint16 client_l4port;	// onu l4 port
	uint16 server_l4port;	// peer l4 port
}rt_pe_generic_pkt_pattern_t;

typedef struct rt_pe_generic_fwd_request_s
{
	rt_pe_generic_fwd_req_cmd_t req_cmd;
	
	rt_pe_generic_pkt_pattern_t pkt_pattern;

	uint8 server_mode;
	uint8 connection_number;
	//TCP 
	uint8 tcp_congestion_mode;				// only for download
	rt_pe_tcp_info_t tcp_info[MAX_PE_GENERIC_FWD_CONNECTION_NUM];
	uint64 tcp_upload_total_content_length; // unit: bytes
	//UDP
	uint16 udp_upload_payload_length; 		// unit: bytes
	uint16 udp_test_time_sec;
	uint32 udp_upload_bandwidth_mbps;
}rt_pe_generic_fwd_request_t;

typedef struct rt_pe_generic_fwd_result_s
{
	rt_pe_ret_t ret_val;
	uint32 pktCnt;
	uint64 byteCnt;							//payload only
	uint64 byteCnt_include_pktHdr;			//L2+L3+L4+payload
	//the unit of following time is milliseconds 
	uint64 cost_time_ms;
	//counter of each connection
	uint32 pktCnt_conn[MAX_PE_GENERIC_FWD_CONNECTION_NUM];
	uint64 byteCnt_conn[MAX_PE_GENERIC_FWD_CONNECTION_NUM];						//payload only
	uint64 byteCnt_include_pktHdr_conn[MAX_PE_GENERIC_FWD_CONNECTION_NUM];		//L2+L3+L4+payload
}rt_pe_generic_fwd_result_t;

typedef struct rt_pe_amsdu_ofld_request_s
{
	uint32 buf_phy_addr; 	/* physical address of allocated buffer */
	uint32 buf_size;	 	/* total allocated buffer size */
} rt_pe_amsdu_ofld_request_t;

/*
 * Function Declaration
 */

/* Function Name:
 *      rt_pe_http_test
 * Description:
 *      Set configuration of pe http test
 * Input:
 *      http_test_req	- Configuration of http test request 
 * Output:
 *		pHttp_test_result
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can set configuration of pe http test
 */
extern int rt_pe_http_test(rt_pe_http_test_request_t http_test_req, rt_pe_http_test_result_t *pHttp_test_result);
/* Function Name:
 *      rt_pe_iperf_test
 * Description:
 *      Set configuration of pe iperf test
 * Input:
 *      iperf_req	- Configuration of iperf test request 
 * Output:
 *		pIperf_result
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can set configuration of pe iperf test
 */
extern int rt_pe_iperf_test(rt_pe_iperf_request_t iperf_req, rt_pe_iperf_result_t *pIperf_result);
/* Function Name:
 *      rt_pe_generic_fwd_test
 * Description:
 *      Set configuration of pe generic fwd test
 * Input:
 *      fwd_req	- Configuration of pe generic fwd test request 
 * Output:
 *		pFwd_ret
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can set configuration of pe generic fwd test
 */
extern int rt_pe_generic_fwd_test(rt_pe_generic_fwd_request_t fwd_req, rt_pe_generic_fwd_result_t *pFwd_ret);

typedef struct rt_pe_srv6_encap_info_s
{
	uint8 srv6_type;
	uint8 outer_len;
	uint8 ipv6_payload_len_offset;
	uint8 ipv6_payload_base_value;

	uint8 ldpid;
	uint8 cos;
	uint16 flowid;

	uint8 outer_hdr[MAX_PE_SRV6_ENCAP_OUTER_HDR_LENGTH];
}rt_pe_srv6_encap_info_t;

typedef struct rt_pe_srv6_decap_info_s
{
	uint32 srv6_type;	//0:outer=da,sa,eth,ipv6 1:outer=da,sa,eth,ipv6,srh 2:outer=eth,ipv6
	uint32 outer_len;

	uint32 hwlookup_swId;
}rt_pe_srv6_decap_info_t;

typedef struct rt_pe_srv6_request_s
{
	rt_pe_srv6_offload_cmd_t req_cmd;
	uint8 connection_idx;
	uint8 hwlookup_lspid;
	union{
		rt_pe_srv6_encap_info_t encap_info;
		rt_pe_srv6_decap_info_t decap_info;
	}connection;
	uint32 buf_phy_addr[RT_PE_SRV6_PAGEINFO_NUMBER]; 	/* physical address of allocated buffer */
	uint32 buf_size[RT_PE_SRV6_PAGEINFO_NUMBER];	 	/* total allocated buffer size */
}rt_pe_srv6_request_t;

//pe de amsdu
#define PE_NI_DE_AMSDU_PAGE_ORDER 11
#define PE_NI_DE_AMSDU_PAGEINFO_NUMBER 1

typedef enum rt_pe_de_amsdu_cmd_e
{
	RT_PE_DE_AMSDU_CMD_ENABLE = 0,
	RT_PE_DE_AMSDU_CMD_DISABLE = 1,
	RT_PE_DE_AMSDU_CMD_TABLE_GET = 2,
	RT_PE_DE_AMSDU_CMD_MEMORY_GET = 3,
	RT_PE_DE_AMSDU_CMD_MEMORY_FREE = 4,
	RT_PE_DE_AMSDU_CMD_DEBUG_CNT_GET = 5,
	RT_PE_DE_AMSDU_CMD_MAX
} rt_pe_de_amsdu_cmd_e_t;

typedef struct rt_pe_de_amsdu_request_s
{
	uint32 buf_phy_addr[PE_NI_DE_AMSDU_PAGEINFO_NUMBER]; 	/* physical address of allocated buffer */
	uint32 buf_size[PE_NI_DE_AMSDU_PAGEINFO_NUMBER];	 	/* total allocated buffer size */
	uint32 var_offset;
} rt_pe_de_amsdu_request;

typedef struct rt_pe_de_amsdu_request_table_s
{
	uint32 lspid;
	uint32 cos;
	uint32 pol_id;
} rt_pe_de_amsdu_request_table;

typedef struct rt_pe_de_amsdu_table_s
{
	uint32 lspid;
	uint32 ldpid;
	uint32 cos;
	uint32 pol_id;
} rt_pe_de_amsdu_table;

//amsdu ofld
typedef enum rt_pe_amsdu_ofld_tmorq_e
{
	TMORQ_OP_ALL = 1,
	TMORQ_OP_MACID = 2,
	TMORQ_OP_SET = 3,
	TMORQ_OP_GET = 4,
	TMORQ_OP_MAX
} rt_pe_amsdu_ofld_tmorq_s;

typedef enum rt_pe_amsdu_ofld_cmd_e
{
	MAX_PKT_NUM_SETTING = 0,
	MAX_TX_PKT_SIZE_SETTING = 1,
	MAX_TX_PKT_SIZE_SETTING_BY_MAC_IDX = 2,
	MAX_TX_TIME_OUT_SETTING = 3,
	TX_AMSDU_RESET = 4,
	WIFI_MODE_SET = 5,
	MAX_PKT_NUM_GETTING = 6,
	MAX_TX_PKT_SIZE_GETTING = 7,
	MAX_TX_TIME_OUT_GETTING = 8,
	WIFI_PRI_INFO = 9,
	WIFI_WLAN_IDX_INFO = 10,
} rt_pe_amsdu_ofld_cmd_s;

typedef struct rt_pe_amsdu_ofld_tmorq_s
{
	uint16 op;
	uint16 macid;
	uint32 timeout;
} rt_pe_amsdu_ofld_tmorq_t;
#endif /* __RT_PE_EXT_H__ */
