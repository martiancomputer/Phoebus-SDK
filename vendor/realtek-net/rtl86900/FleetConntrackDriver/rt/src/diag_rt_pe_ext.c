#include <stdio.h>
#include <string.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <diag_util.h>
#include <diag_str.h>
#include <parser/cparser_priv.h>

#include <rt_pe_ext.h>

/*
 * rt_pe http_test download get_counter
 */
cparser_result_t
cparser_cmd_rt_pe_http_test_download_get_counter(
    cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED;
	rt_pe_http_test_request_t http_req;
	rt_pe_http_test_result_t http_result;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&http_req, 0x0, sizeof(rt_pe_http_test_request_t));
	http_req.req_cmd = RT_PE_HTTP_TEST_CMD_DOWNLOAD_GET_CNT;

	ret = rt_pe_http_test(http_req, &http_result);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to get counter of http download test ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to get counter of http download test ... RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		if(http_result.ret_val != RT_PE_RET_OK)
			diag_util_mprintf("Fail to get counter of http download test ... PE return 0x%x \n", http_result.ret_val);
		else
		{
			uint64 avg_speed_Mbps, duration_milliseconds=http_result.HTTP_EOMTime_ms-http_result.HTTP_BOMTime_ms;
			
			diag_util_mprintf("========== Result of http download test ========== \n");	
			diag_util_mprintf("TCP_openRequestTime	%llu ms \n", http_result.TCP_openRequestTime_ms);
			diag_util_mprintf("TCP_openResponseTime	%llu ms \n", http_result.TCP_openResponseTime_ms);
			diag_util_mprintf("HTTP_ROMTime %llu ms \n", http_result.HTTP_ROMTime_ms);
			diag_util_mprintf("HTTP_BOMTime %llu ms \n", http_result.HTTP_BOMTime_ms);
			diag_util_mprintf("HTTP_EOMTime %llu ms \n", http_result.HTTP_EOMTime_ms);
			diag_util_mprintf("Duration(HTTP_EOMTime - HTTP_BOMTime) %llu ms \n", duration_milliseconds);
			diag_util_mprintf("================ Test statistic =============== \n");
			diag_util_mprintf("PktCnt %u \n", http_result.pktCnt);
			diag_util_mprintf("ByteCnt(payload only) %llu \n", http_result.byteCnt);
			diag_util_mprintf("ByteCnt(whole packet) %llu \n", http_result.byteCnt_include_pktHdr);
			avg_speed_Mbps = (http_result.byteCnt*8*1000/duration_milliseconds)/1000000;
			diag_util_mprintf("Average speed(payload only) %u.%03u Gpbs \n", (uint32)(avg_speed_Mbps/1000), (uint32)(avg_speed_Mbps%1000));
			avg_speed_Mbps = (http_result.byteCnt_include_pktHdr*8*1000/duration_milliseconds)/1000000;
			diag_util_mprintf("Average speed(whole packet) %u.%03u Gpbs \n", (uint32)(avg_speed_Mbps/1000), (uint32)(avg_speed_Mbps%1000));
		}
	}	

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_pe_http_test_download_get_counter */

/*
 * rt_pe http_test download stop 
 */
cparser_result_t
cparser_cmd_rt_pe_http_test_download_stop(
    cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED;
	rt_pe_http_test_request_t http_req;
	rt_pe_http_test_result_t http_result;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&http_req, 0x0, sizeof(rt_pe_http_test_request_t));
	http_req.req_cmd = RT_PE_HTTP_TEST_CMD_DOWNLOAD_STOP;

	ret = rt_pe_http_test(http_req, &http_result);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to stop http download test ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to stop http download test ... RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		if(http_result.ret_val != RT_PE_RET_OK)
			diag_util_mprintf("Fail to stop http download test ... PE return 0x%x \n", http_result.ret_val);
		else
		{
			uint64 avg_speed_Mbps, duration_milliseconds=http_result.HTTP_EOMTime_ms-http_result.HTTP_BOMTime_ms;
			
			diag_util_mprintf("========== Result of http download test ========== \n");	
			diag_util_mprintf("TCP_openRequestTime	%llu ms \n", http_result.TCP_openRequestTime_ms);
			diag_util_mprintf("TCP_openResponseTime	%llu ms \n", http_result.TCP_openResponseTime_ms);
			diag_util_mprintf("HTTP_ROMTime %llu ms \n", http_result.HTTP_ROMTime_ms);
			diag_util_mprintf("HTTP_BOMTime %llu ms \n", http_result.HTTP_BOMTime_ms);
			diag_util_mprintf("HTTP_EOMTime %llu ms \n", http_result.HTTP_EOMTime_ms);
			diag_util_mprintf("Duration(HTTP_EOMTime - HTTP_BOMTime) %llu ms \n", duration_milliseconds);
			diag_util_mprintf("================ Test statistic ===============");
			diag_util_mprintf("PktCnt %u \n", http_result.pktCnt);
			diag_util_mprintf("ByteCnt(payload only) %llu \n", http_result.byteCnt);
			diag_util_mprintf("ByteCnt(whole packet) %llu \n", http_result.byteCnt_include_pktHdr);
			avg_speed_Mbps = (http_result.byteCnt*8*1000/duration_milliseconds)/1000000;
			diag_util_mprintf("Average speed(payload only) %u.%03u Gpbs \n", (uint32)(avg_speed_Mbps/1000), (uint32)(avg_speed_Mbps%1000));
			avg_speed_Mbps = (http_result.byteCnt_include_pktHdr*8*1000/duration_milliseconds)/1000000;
			diag_util_mprintf("Average speed(whole packet) %u.%03u Gpbs \n", (uint32)(avg_speed_Mbps/1000), (uint32)(avg_speed_Mbps%1000));
		}
	}	

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_pe_http_test_download_stop */

/*
 * rt_pe http_test download start connection_number <UINT:connection_number> client_mac <MACADDR:client_mac> server_mac <MACADDR:server_mac> IPv4 client_ip <IPV4ADDR:client_ip> server_ip <IPV4ADDR:server_ip> client_l4port <UINT:client_l4port> server_l4port <UINT:server_l4port> ldpid <UINT:ldpid> pppoe_sid <INT:pppoe_sid> svlan_vid <INT:svlan_vid> cvlan_vid <INT:cvlan_vid> pon_streamId <INT:pon_streamId> http_req_url <STRING:http_req_url> http_version <STRING:http_version>
 */
cparser_result_t
cparser_cmd_rt_pe_http_test_download_start_connection_number_connection_number_client_mac_client_mac_server_mac_server_mac_IPv4_client_ip_client_ip_server_ip_server_ip_client_l4port_client_l4port_server_l4port_server_l4port_ldpid_ldpid_pppoe_sid_pppoe_sid_svlan_vid_svlan_vid_cvlan_vid_cvlan_vid_pon_streamId_pon_streamId_http_req_url_http_req_url_http_version_http_version(
    cparser_context_t *context,
    uint32_t  *connection_number_ptr,
    cparser_macaddr_t  *client_mac_ptr,
    cparser_macaddr_t  *server_mac_ptr,
    uint32_t  *client_ip_ptr,
    uint32_t  *server_ip_ptr,
    uint32_t  *client_l4port_ptr,
    uint32_t  *server_l4port_ptr,
    uint32_t  *ldpid_ptr,
    int32_t  *pppoe_sid_ptr,
    int32_t  *svlan_vid_ptr,
    int32_t  *cvlan_vid_ptr,
    int32_t  *pon_streamId_ptr,
    char * *http_req_url_ptr,
    char * *http_version_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_pe_http_test_request_t http_req;
	rt_pe_http_test_result_t http_result;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&http_req, 0x0, sizeof(rt_pe_http_test_request_t));
	http_req.req_cmd = RT_PE_HTTP_TEST_CMD_DOWNLOAD_START;
	http_req.connection_number = *connection_number_ptr;
	osal_memcpy(http_req.client_mac.octet, client_mac_ptr->octet, ETHER_ADDR_LEN);
	osal_memcpy(http_req.server_mac.octet, server_mac_ptr->octet, ETHER_ADDR_LEN);	
	http_req.isIPv4OrIpv6 = 0;
	http_req.client_ip.ipv4_addr = (rtk_ip_addr_t)*client_ip_ptr;
	http_req.server_ip.ipv4_addr = (rtk_ip_addr_t)*server_ip_ptr;
	http_req.client_l4port = *client_l4port_ptr;
	http_req.server_l4port = *server_l4port_ptr;
	http_req.ldpid = *ldpid_ptr;
	http_req.pppoe_sid = *pppoe_sid_ptr;
	http_req.svlan_vid = *svlan_vid_ptr;
	http_req.cvlan_vid = *cvlan_vid_ptr;
	http_req.pon_streamId = *pon_streamId_ptr;
	osal_memcpy(http_req.http_req_url, *http_req_url_ptr, MAX_PE_HTTP_REQ_URL_STR_LENGTH);
	osal_memcpy(http_req.http_version, *http_version_ptr, MAX_PE_HTTP_VERSION_STR_LENGTH);
	
	ret = rt_pe_http_test(http_req, &http_result);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to start http download test ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to start http download test ... RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		if(http_result.ret_val != RT_PE_RET_OK)
			diag_util_mprintf("Fail to start http download test ... PE return 0x%x \n", http_result.ret_val);
		else
		{
			diag_util_mprintf("========== Success to start http download test ==========\n");
			diag_util_mprintf("connection_number %u \n", http_req.connection_number);
			diag_util_mprintf("client_mac %02x:%02x:%02x:%02x:%02x:%02x \n", http_req.client_mac.octet[0], http_req.client_mac.octet[1], http_req.client_mac.octet[2], http_req.client_mac.octet[3], http_req.client_mac.octet[4], http_req.client_mac.octet[5]);
			diag_util_mprintf("server_mac %02x:%02x:%02x:%02x:%02x:%02x \n", http_req.server_mac.octet[0], http_req.server_mac.octet[1], http_req.server_mac.octet[2], http_req.server_mac.octet[3], http_req.server_mac.octet[4], http_req.server_mac.octet[5]);
			diag_util_mprintf("client_ipv4 %u.%u.%u.%u \n", (http_req.client_ip.ipv4_addr>>24)&0xff, (http_req.client_ip.ipv4_addr>>16)&0xff, (http_req.client_ip.ipv4_addr>>8)&0xff, (http_req.client_ip.ipv4_addr)&0xff);
			diag_util_mprintf("server_ipv4 %u.%u.%u.%u \n", (http_req.server_ip.ipv4_addr>>24)&0xff, (http_req.server_ip.ipv4_addr>>16)&0xff, (http_req.server_ip.ipv4_addr>>8)&0xff, (http_req.server_ip.ipv4_addr)&0xff);
			diag_util_mprintf("client_l4port %u(0x%x) \n", http_req.client_l4port, http_req.client_l4port);
			diag_util_mprintf("server_l4port %u(0x%x) \n", http_req.server_l4port, http_req.server_l4port);
			diag_util_mprintf("ldpid %u\n", http_req.ldpid);
			diag_util_mprintf("pppoe_sid %d svlan_vid %d cvlan_vid %d \n", http_req.pppoe_sid, http_req.svlan_vid, http_req.cvlan_vid);
			diag_util_mprintf("pon_streamId %d \n", http_req.pon_streamId);
			diag_util_mprintf("http_req_url %s \n", http_req.http_req_url);
			diag_util_mprintf("http_version %s \n", http_req.http_version);
		}
	}	

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_pe_http_test_download_start_connection_number_connection_number_client_mac_client_mac_server_mac_server_mac_ipv4_client_ip_client_ip_server_ip_server_ip_client_l4port_client_l4port_server_l4port_server_l4port_ldpid_ldpid_pppoe_sid_pppoe_sid_svlan_vid_svlan_vid_cvlan_vid_cvlan_vid_pon_streamid_pon_streamid_http_req_url_http_req_url_http_version_http_version */

/*
 * rt_pe http_test download start connection_number <UINT:connection_number> client_mac <MACADDR:client_mac> server_mac <MACADDR:server_mac> IPv6 client_ip <IPV6ADDR:client_ip> server_ip <IPV6ADDR:server_ip> client_l4port <UINT:client_l4port> server_l4port <UINT:server_l4port> ldpid <UINT:ldpid> pppoe_sid <INT:pppoe_sid> svlan_vid <INT:svlan_vid> cvlan_vid <INT:cvlan_vid> pon_streamId <INT:pon_streamId> http_req_url <STRING:http_req_url> http_version <STRING:http_version>
 */
cparser_result_t
cparser_cmd_rt_pe_http_test_download_start_connection_number_connection_number_client_mac_client_mac_server_mac_server_mac_IPv6_client_ip_client_ip_server_ip_server_ip_client_l4port_client_l4port_server_l4port_server_l4port_ldpid_ldpid_pppoe_sid_pppoe_sid_svlan_vid_svlan_vid_cvlan_vid_cvlan_vid_pon_streamId_pon_streamId_http_req_url_http_req_url_http_version_http_version(
    cparser_context_t *context,
    uint32_t  *connection_number_ptr,
    cparser_macaddr_t  *client_mac_ptr,
    cparser_macaddr_t  *server_mac_ptr,
    char * *client_ip_ptr,
    char * *server_ip_ptr,
    uint32_t  *client_l4port_ptr,
    uint32_t  *server_l4port_ptr,
    uint32_t  *ldpid_ptr,
    int32_t  *pppoe_sid_ptr,
    int32_t  *svlan_vid_ptr,
    int32_t  *cvlan_vid_ptr,
    int32_t  *pon_streamId_ptr,
    char * *http_req_url_ptr,
    char * *http_version_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_pe_http_test_request_t http_req;
	rt_pe_http_test_result_t http_result;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&http_req, 0x0, sizeof(rt_pe_http_test_request_t));
	http_req.req_cmd = RT_PE_HTTP_TEST_CMD_DOWNLOAD_START;
	http_req.connection_number = *connection_number_ptr;
	osal_memcpy(http_req.client_mac.octet, client_mac_ptr->octet, ETHER_ADDR_LEN);
	osal_memcpy(http_req.server_mac.octet, server_mac_ptr->octet, ETHER_ADDR_LEN);	
	http_req.isIPv4OrIpv6 = 1;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(http_req.client_ip.ipv6_addr.ipv6_addr, TOKEN_STR(12)), ret);
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(http_req.server_ip.ipv6_addr.ipv6_addr, TOKEN_STR(14)), ret);
	http_req.client_l4port = *client_l4port_ptr;
	http_req.server_l4port = *server_l4port_ptr;
	http_req.ldpid = *ldpid_ptr;
	http_req.pppoe_sid = *pppoe_sid_ptr;
	http_req.svlan_vid = *svlan_vid_ptr;
	http_req.cvlan_vid = *cvlan_vid_ptr;
	http_req.pon_streamId = *pon_streamId_ptr;
	osal_memcpy(http_req.http_req_url, *http_req_url_ptr, MAX_PE_HTTP_REQ_URL_STR_LENGTH);
	osal_memcpy(http_req.http_version, *http_version_ptr, MAX_PE_HTTP_VERSION_STR_LENGTH);

	ret = rt_pe_http_test(http_req, &http_result);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to start http download test ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to start http download test ... RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		if(http_result.ret_val != RT_PE_RET_OK)
			diag_util_mprintf("Fail to start http download test ... PE return 0x%x \n", http_result.ret_val);
		else
		{
			diag_util_mprintf("========== Success to start http download test ==========\n");
			diag_util_mprintf("connection_number %u \n", http_req.connection_number);
			diag_util_mprintf("client_mac %02x:%02x:%02x:%02x:%02x:%02x \n", http_req.client_mac.octet[0], http_req.client_mac.octet[1], http_req.client_mac.octet[2], http_req.client_mac.octet[3], http_req.client_mac.octet[4], http_req.client_mac.octet[5]);
			diag_util_mprintf("server_mac %02x:%02x:%02x:%02x:%02x:%02x \n", http_req.server_mac.octet[0], http_req.server_mac.octet[1], http_req.server_mac.octet[2], http_req.server_mac.octet[3], http_req.server_mac.octet[4], http_req.server_mac.octet[5]);
			diag_util_mprintf("client_ipv6 %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n", 
								http_req.client_ip.ipv6_addr.ipv6_addr[0], http_req.client_ip.ipv6_addr.ipv6_addr[1], http_req.client_ip.ipv6_addr.ipv6_addr[2], http_req.client_ip.ipv6_addr.ipv6_addr[3],
								http_req.client_ip.ipv6_addr.ipv6_addr[4], http_req.client_ip.ipv6_addr.ipv6_addr[5], http_req.client_ip.ipv6_addr.ipv6_addr[6], http_req.client_ip.ipv6_addr.ipv6_addr[7],
								http_req.client_ip.ipv6_addr.ipv6_addr[8], http_req.client_ip.ipv6_addr.ipv6_addr[9], http_req.client_ip.ipv6_addr.ipv6_addr[10], http_req.client_ip.ipv6_addr.ipv6_addr[11],
								http_req.client_ip.ipv6_addr.ipv6_addr[12], http_req.client_ip.ipv6_addr.ipv6_addr[13], http_req.client_ip.ipv6_addr.ipv6_addr[14], http_req.client_ip.ipv6_addr.ipv6_addr[15]);
			diag_util_mprintf("server_ipv6 %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n", 
								http_req.server_ip.ipv6_addr.ipv6_addr[0], http_req.server_ip.ipv6_addr.ipv6_addr[1], http_req.server_ip.ipv6_addr.ipv6_addr[2], http_req.server_ip.ipv6_addr.ipv6_addr[3],
								http_req.server_ip.ipv6_addr.ipv6_addr[4], http_req.server_ip.ipv6_addr.ipv6_addr[5], http_req.server_ip.ipv6_addr.ipv6_addr[6], http_req.server_ip.ipv6_addr.ipv6_addr[7],
								http_req.server_ip.ipv6_addr.ipv6_addr[8], http_req.server_ip.ipv6_addr.ipv6_addr[9], http_req.server_ip.ipv6_addr.ipv6_addr[10], http_req.server_ip.ipv6_addr.ipv6_addr[11],
								http_req.server_ip.ipv6_addr.ipv6_addr[12], http_req.server_ip.ipv6_addr.ipv6_addr[13], http_req.server_ip.ipv6_addr.ipv6_addr[14], http_req.server_ip.ipv6_addr.ipv6_addr[15]);
			diag_util_mprintf("client_l4port %u(0x%x) \n", http_req.client_l4port, http_req.client_l4port);
			diag_util_mprintf("server_l4port %u(0x%x) \n", http_req.server_l4port, http_req.server_l4port);
			diag_util_mprintf("ldpid %u\n", http_req.ldpid);
			diag_util_mprintf("pppoe_sid %d svlan_vid %d cvlan_vid %d \n", http_req.pppoe_sid, http_req.svlan_vid, http_req.cvlan_vid);
			diag_util_mprintf("pon_streamId %d \n", http_req.pon_streamId);
			diag_util_mprintf("http_req_url %s \n", http_req.http_req_url);
			diag_util_mprintf("http_version %s \n", http_req.http_version);
		}
	}	

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_pe_http_test_download_start_connection_number_connection_number_client_mac_client_mac_server_mac_server_mac_ipv6_client_ip_client_ip_server_ip_server_ip_client_l4port_client_l4port_server_l4port_server_l4port_ldpid_ldpid_pppoe_sid_pppoe_sid_svlan_vid_svlan_vid_cvlan_vid_cvlan_vid_pon_streamid_pon_streamid_http_req_url_http_req_url_http_version_http_version */

/*
 * rt_pe http_test upload get_counter
 */
cparser_result_t
cparser_cmd_rt_pe_http_test_upload_get_counter(
    cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED;
	rt_pe_http_test_request_t http_req;
	rt_pe_http_test_result_t http_result;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&http_req, 0x0, sizeof(rt_pe_http_test_request_t));
	http_req.req_cmd = RT_PE_HTTP_TEST_CMD_UPLOAD_GET_CNT;

	ret = rt_pe_http_test(http_req, &http_result);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to get counter of http upload test ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to get counter of http upload test ... RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		if(http_result.ret_val != RT_PE_RET_OK)
			diag_util_mprintf("Fail to get counter of http upload test ... PE return 0x%x \n", http_result.ret_val);
		else
		{
			uint64 avg_speed_Mbps, duration_milliseconds=http_result.HTTP_EOMTime_ms-http_result.HTTP_BOMTime_ms;
			
			diag_util_mprintf("========== Result of http upload test ========== \n");	
			diag_util_mprintf("TCP_openRequestTime	%llu ms \n", http_result.TCP_openRequestTime_ms);
			diag_util_mprintf("TCP_openResponseTime	%llu ms \n", http_result.TCP_openResponseTime_ms);
			diag_util_mprintf("HTTP_ROMTime %llu ms \n", http_result.HTTP_ROMTime_ms);
			diag_util_mprintf("HTTP_BOMTime %llu ms \n", http_result.HTTP_BOMTime_ms);
			diag_util_mprintf("HTTP_EOMTime %llu ms \n", http_result.HTTP_EOMTime_ms);
			diag_util_mprintf("Duration(HTTP_EOMTime - HTTP_BOMTime) %llu ms \n", duration_milliseconds);
			diag_util_mprintf("================ Test statistic =============== \n");
			diag_util_mprintf("PktCnt %u \n", http_result.pktCnt);
			diag_util_mprintf("ByteCnt(payload only) %llu \n", http_result.byteCnt);
			diag_util_mprintf("ByteCnt(whole packet) %llu \n", http_result.byteCnt_include_pktHdr);
			avg_speed_Mbps = (http_result.byteCnt*8*1000/duration_milliseconds)/1000000;
			diag_util_mprintf("Average speed(payload only) %u.%03u Gpbs \n", (uint32)(avg_speed_Mbps/1000), (uint32)(avg_speed_Mbps%1000));
			avg_speed_Mbps = (http_result.byteCnt_include_pktHdr*8*1000/duration_milliseconds)/1000000;
			diag_util_mprintf("Average speed(whole packet) %u.%03u Gpbs \n", (uint32)(avg_speed_Mbps/1000), (uint32)(avg_speed_Mbps%1000));
		}
	}	

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_pe_http_test_upload_get_counter */

/*
 * rt_pe http_test upload stop 
 */
cparser_result_t
cparser_cmd_rt_pe_http_test_upload_stop(
    cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED;
	rt_pe_http_test_request_t http_req;
	rt_pe_http_test_result_t http_result;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&http_req, 0x0, sizeof(rt_pe_http_test_request_t));
	http_req.req_cmd = RT_PE_HTTP_TEST_CMD_UPLOAD_STOP;

	ret = rt_pe_http_test(http_req, &http_result);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to stop http upload test ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to stop http upload test ... RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		if(http_result.ret_val != RT_PE_RET_OK)
			diag_util_mprintf("Fail to stop http upload test ... PE return 0x%x \n", http_result.ret_val);
		else
		{
			uint64 avg_speed_Mbps, duration_milliseconds=http_result.HTTP_EOMTime_ms-http_result.HTTP_BOMTime_ms;
			
			diag_util_mprintf("========== Result of http upload test ========== \n");	
			diag_util_mprintf("TCP_openRequestTime	%llu ms \n", http_result.TCP_openRequestTime_ms);
			diag_util_mprintf("TCP_openResponseTime	%llu ms \n", http_result.TCP_openResponseTime_ms);
			diag_util_mprintf("HTTP_ROMTime %llu ms \n", http_result.HTTP_ROMTime_ms);
			diag_util_mprintf("HTTP_BOMTime %llu ms \n", http_result.HTTP_BOMTime_ms);
			diag_util_mprintf("HTTP_EOMTime %llu ms \n", http_result.HTTP_EOMTime_ms);
			diag_util_mprintf("Duration(HTTP_EOMTime - HTTP_BOMTime) %llu ms \n", duration_milliseconds);
			diag_util_mprintf("================ Test statistic =============== \n");
			diag_util_mprintf("PktCnt %u \n", http_result.pktCnt);
			diag_util_mprintf("ByteCnt(payload only) %llu \n", http_result.byteCnt);
			diag_util_mprintf("ByteCnt(whole packet) %llu \n", http_result.byteCnt_include_pktHdr);
			avg_speed_Mbps = (http_result.byteCnt*8*1000/duration_milliseconds)/1000000;
			diag_util_mprintf("Average speed(payload only) %u.%03u Gpbs \n", (uint32)(avg_speed_Mbps/1000), (uint32)(avg_speed_Mbps%1000));
			avg_speed_Mbps = (http_result.byteCnt_include_pktHdr*8*1000/duration_milliseconds)/1000000;
			diag_util_mprintf("Average speed(whole packet) %u.%03u Gpbs \n", (uint32)(avg_speed_Mbps/1000), (uint32)(avg_speed_Mbps%1000));
		}
	}	

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_pe_http_test_upload_stop */

/*
 * rt_pe http_test upload start connection_number <UINT:connection_number> client_mac <MACADDR:client_mac> server_mac <MACADDR:server_mac> IPv4 client_ip <IPV4ADDR:client_ip> server_ip <IPV4ADDR:server_ip> client_l4port <UINT:client_l4port> server_l4port <UINT:server_l4port> ldpid <UINT:ldpid> pppoe_sid <INT:pppoe_sid> svlan_vid <INT:svlan_vid> cvlan_vid <INT:cvlan_vid> pon_streamId <INT:pon_streamId> http_req_url <STRING:http_req_url> http_version <STRING:http_version> upload_content_length <UINT64:upload_content_length> upload_window_size <UINT:upload_window_size>
 */
cparser_result_t
cparser_cmd_rt_pe_http_test_upload_start_connection_number_connection_number_client_mac_client_mac_server_mac_server_mac_IPv4_client_ip_client_ip_server_ip_server_ip_client_l4port_client_l4port_server_l4port_server_l4port_ldpid_ldpid_pppoe_sid_pppoe_sid_svlan_vid_svlan_vid_cvlan_vid_cvlan_vid_pon_streamId_pon_streamId_http_req_url_http_req_url_http_version_http_version_upload_content_length_upload_content_length_upload_window_size_upload_window_size(
    cparser_context_t *context,
    uint32_t  *connection_number_ptr,
    cparser_macaddr_t  *client_mac_ptr,
    cparser_macaddr_t  *server_mac_ptr,
    uint32_t  *client_ip_ptr,
    uint32_t  *server_ip_ptr,
    uint32_t  *client_l4port_ptr,
    uint32_t  *server_l4port_ptr,
    uint32_t  *ldpid_ptr,
    int32_t  *pppoe_sid_ptr,
    int32_t  *svlan_vid_ptr,
    int32_t  *cvlan_vid_ptr,
    int32_t  *pon_streamId_ptr,
    char * *http_req_url_ptr,
    char * *http_version_ptr,
    uint64_t  *upload_content_length_ptr,
    uint32_t  *upload_window_size)
{
	int32 ret = RT_ERR_FAILED;
	rt_pe_http_test_request_t http_req;
	rt_pe_http_test_result_t http_result;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&http_req, 0x0, sizeof(rt_pe_http_test_request_t));
	http_req.req_cmd = RT_PE_HTTP_TEST_CMD_UPLOAD_START;
	http_req.connection_number = *connection_number_ptr;
	osal_memcpy(http_req.client_mac.octet, client_mac_ptr->octet, ETHER_ADDR_LEN);
	osal_memcpy(http_req.server_mac.octet, server_mac_ptr->octet, ETHER_ADDR_LEN);	
	http_req.isIPv4OrIpv6 = 0;
	http_req.client_ip.ipv4_addr = (rtk_ip_addr_t)*client_ip_ptr;
	http_req.server_ip.ipv4_addr = (rtk_ip_addr_t)*server_ip_ptr;
	http_req.client_l4port = *client_l4port_ptr;
	http_req.server_l4port = *server_l4port_ptr;
	http_req.ldpid = *ldpid_ptr;
	http_req.pppoe_sid = *pppoe_sid_ptr;
	http_req.svlan_vid = *svlan_vid_ptr;
	http_req.cvlan_vid = *cvlan_vid_ptr;
	http_req.pon_streamId = *pon_streamId_ptr;
	osal_memcpy(http_req.http_req_url, *http_req_url_ptr, MAX_PE_HTTP_REQ_URL_STR_LENGTH);
	osal_memcpy(http_req.http_version, *http_version_ptr, MAX_PE_HTTP_VERSION_STR_LENGTH);
	http_req.upload_content_length = *upload_content_length_ptr;
	http_req.tcp_window_size = *upload_window_size;

	ret = rt_pe_http_test(http_req, &http_result);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to start http upload test ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to start http upload test ... RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		if(http_result.ret_val != RT_PE_RET_OK)
			diag_util_mprintf("Fail to start http upload test ... PE return 0x%x \n", http_result.ret_val);
		else
		{
			diag_util_mprintf("========== Success to start http upload test ==========\n");
			diag_util_mprintf("connection_number %u \n", http_req.connection_number);
			diag_util_mprintf("client_mac %02x:%02x:%02x:%02x:%02x:%02x \n", http_req.client_mac.octet[0], http_req.client_mac.octet[1], http_req.client_mac.octet[2], http_req.client_mac.octet[3], http_req.client_mac.octet[4], http_req.client_mac.octet[5]);
			diag_util_mprintf("server_mac %02x:%02x:%02x:%02x:%02x:%02x \n", http_req.server_mac.octet[0], http_req.server_mac.octet[1], http_req.server_mac.octet[2], http_req.server_mac.octet[3], http_req.server_mac.octet[4], http_req.server_mac.octet[5]);
			diag_util_mprintf("client_ipv4 %u.%u.%u.%u \n", (http_req.client_ip.ipv4_addr>>24)&0xff, (http_req.client_ip.ipv4_addr>>16)&0xff, (http_req.client_ip.ipv4_addr>>8)&0xff, (http_req.client_ip.ipv4_addr)&0xff);
			diag_util_mprintf("server_ipv4 %u.%u.%u.%u \n", (http_req.server_ip.ipv4_addr>>24)&0xff, (http_req.server_ip.ipv4_addr>>16)&0xff, (http_req.server_ip.ipv4_addr>>8)&0xff, (http_req.server_ip.ipv4_addr)&0xff);
			diag_util_mprintf("client_l4port %u(0x%x) \n", http_req.client_l4port, http_req.client_l4port);
			diag_util_mprintf("server_l4port %u(0x%x) \n", http_req.server_l4port, http_req.server_l4port);
			diag_util_mprintf("ldpid %u\n", http_req.ldpid);
			diag_util_mprintf("pppoe_sid %d svlan_vid %d cvlan_vid %d \n", http_req.pppoe_sid, http_req.svlan_vid, http_req.cvlan_vid);
			diag_util_mprintf("pon_streamId %d \n", http_req.pon_streamId);
			diag_util_mprintf("http_req_url %s \n", http_req.http_req_url);
			diag_util_mprintf("http_version %s \n", http_req.http_version);
			diag_util_mprintf("upload_content_length %llu \n", http_req.upload_content_length);
			diag_util_mprintf("tcp_window_size %u \n", http_req.tcp_window_size);
		}
	}	

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_pe_http_test_upload_start_connection_number_connection_number_client_mac_client_mac_server_mac_server_mac_ipv4_client_ip_client_ip_server_ip_server_ip_client_l4port_client_l4port_server_l4port_server_l4port_ldpid_ldpid_pppoe_sid_pppoe_sid_svlan_vid_svlan_vid_cvlan_vid_cvlan_vid_pon_streamid_pon_streamid_http_req_url_http_req_url_http_version_http_version_upload_content_length_upload_content_length_upload_window_size_upload_window_size */

/*
 * rt_pe http_test upload start connection_number <UINT:connection_number> client_mac <MACADDR:client_mac> server_mac <MACADDR:server_mac> IPv6 client_ip <IPV6ADDR:client_ip> server_ip <IPV6ADDR:server_ip> client_l4port <UINT:client_l4port> server_l4port <UINT:server_l4port> ldpid <UINT:ldpid> pppoe_sid <INT:pppoe_sid> svlan_vid <INT:svlan_vid> cvlan_vid <INT:cvlan_vid> pon_streamId <INT:pon_streamId> http_req_url <STRING:http_req_url> http_version <STRING:http_version> upload_content_length <UINT64:upload_content_length> upload_window_size <UINT:upload_window_size>
 */
cparser_result_t
cparser_cmd_rt_pe_http_test_upload_start_connection_number_connection_number_client_mac_client_mac_server_mac_server_mac_IPv6_client_ip_client_ip_server_ip_server_ip_client_l4port_client_l4port_server_l4port_server_l4port_ldpid_ldpid_pppoe_sid_pppoe_sid_svlan_vid_svlan_vid_cvlan_vid_cvlan_vid_pon_streamId_pon_streamId_http_req_url_http_req_url_http_version_http_version_upload_content_length_upload_content_length_upload_window_size_upload_window_size(
    cparser_context_t *context,
    uint32_t  *connection_number_ptr,
    cparser_macaddr_t  *client_mac_ptr,
    cparser_macaddr_t  *server_mac_ptr,
    char * *client_ip_ptr,
    char * *server_ip_ptr,
    uint32_t  *client_l4port_ptr,
    uint32_t  *server_l4port_ptr,
    uint32_t  *ldpid_ptr,
    int32_t  *pppoe_sid_ptr,
    int32_t  *svlan_vid_ptr,
    int32_t  *cvlan_vid_ptr,
    int32_t  *pon_streamId_ptr,
    char * *http_req_url_ptr,
    char * *http_version_ptr,
    uint64_t  *upload_content_length_ptr,
    uint32_t  *upload_window_size)
{
	int32 ret = RT_ERR_FAILED;
	rt_pe_http_test_request_t http_req;
	rt_pe_http_test_result_t http_result;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&http_req, 0x0, sizeof(rt_pe_http_test_request_t));
	http_req.req_cmd = RT_PE_HTTP_TEST_CMD_UPLOAD_START;
	http_req.connection_number = *connection_number_ptr;
	osal_memcpy(http_req.client_mac.octet, client_mac_ptr->octet, ETHER_ADDR_LEN);
	osal_memcpy(http_req.server_mac.octet, server_mac_ptr->octet, ETHER_ADDR_LEN);	
	http_req.isIPv4OrIpv6 = 1;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(http_req.client_ip.ipv6_addr.ipv6_addr, TOKEN_STR(12)), ret);
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(http_req.server_ip.ipv6_addr.ipv6_addr, TOKEN_STR(14)), ret);
	http_req.client_l4port = *client_l4port_ptr;
	http_req.server_l4port = *server_l4port_ptr;
	http_req.ldpid = *ldpid_ptr;
	http_req.pppoe_sid = *pppoe_sid_ptr;
	http_req.svlan_vid = *svlan_vid_ptr;
	http_req.cvlan_vid = *cvlan_vid_ptr;
	http_req.pon_streamId = *pon_streamId_ptr;
	osal_memcpy(http_req.http_req_url, *http_req_url_ptr, MAX_PE_HTTP_REQ_URL_STR_LENGTH);
	osal_memcpy(http_req.http_version, *http_version_ptr, MAX_PE_HTTP_VERSION_STR_LENGTH);
	http_req.upload_content_length = *upload_content_length_ptr;
	http_req.tcp_window_size = *upload_window_size;

	ret = rt_pe_http_test(http_req, &http_result);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to start http upload test ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to start http upload test ... RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		if(http_result.ret_val != RT_PE_RET_OK)
			diag_util_mprintf("Fail to start http upload test ... PE return 0x%x \n", http_result.ret_val);
		else
		{
			diag_util_mprintf("========== Success to start http upload test ==========\n");
			diag_util_mprintf("connection_number %u \n", http_req.connection_number);
			diag_util_mprintf("client_mac %02x:%02x:%02x:%02x:%02x:%02x \n", http_req.client_mac.octet[0], http_req.client_mac.octet[1], http_req.client_mac.octet[2], http_req.client_mac.octet[3], http_req.client_mac.octet[4], http_req.client_mac.octet[5]);
			diag_util_mprintf("server_mac %02x:%02x:%02x:%02x:%02x:%02x \n", http_req.server_mac.octet[0], http_req.server_mac.octet[1], http_req.server_mac.octet[2], http_req.server_mac.octet[3], http_req.server_mac.octet[4], http_req.server_mac.octet[5]);
			diag_util_mprintf("client_ipv6 %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n", 
								http_req.client_ip.ipv6_addr.ipv6_addr[0], http_req.client_ip.ipv6_addr.ipv6_addr[1], http_req.client_ip.ipv6_addr.ipv6_addr[2], http_req.client_ip.ipv6_addr.ipv6_addr[3],
								http_req.client_ip.ipv6_addr.ipv6_addr[4], http_req.client_ip.ipv6_addr.ipv6_addr[5], http_req.client_ip.ipv6_addr.ipv6_addr[6], http_req.client_ip.ipv6_addr.ipv6_addr[7],
								http_req.client_ip.ipv6_addr.ipv6_addr[8], http_req.client_ip.ipv6_addr.ipv6_addr[9], http_req.client_ip.ipv6_addr.ipv6_addr[10], http_req.client_ip.ipv6_addr.ipv6_addr[11],
								http_req.client_ip.ipv6_addr.ipv6_addr[12], http_req.client_ip.ipv6_addr.ipv6_addr[13], http_req.client_ip.ipv6_addr.ipv6_addr[14], http_req.client_ip.ipv6_addr.ipv6_addr[15]);
			diag_util_mprintf("server_ipv6 %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n", 
								http_req.server_ip.ipv6_addr.ipv6_addr[0], http_req.server_ip.ipv6_addr.ipv6_addr[1], http_req.server_ip.ipv6_addr.ipv6_addr[2], http_req.server_ip.ipv6_addr.ipv6_addr[3],
								http_req.server_ip.ipv6_addr.ipv6_addr[4], http_req.server_ip.ipv6_addr.ipv6_addr[5], http_req.server_ip.ipv6_addr.ipv6_addr[6], http_req.server_ip.ipv6_addr.ipv6_addr[7],
								http_req.server_ip.ipv6_addr.ipv6_addr[8], http_req.server_ip.ipv6_addr.ipv6_addr[9], http_req.server_ip.ipv6_addr.ipv6_addr[10], http_req.server_ip.ipv6_addr.ipv6_addr[11],
								http_req.server_ip.ipv6_addr.ipv6_addr[12], http_req.server_ip.ipv6_addr.ipv6_addr[13], http_req.server_ip.ipv6_addr.ipv6_addr[14], http_req.server_ip.ipv6_addr.ipv6_addr[15]);
			diag_util_mprintf("client_l4port %u(0x%x) \n", http_req.client_l4port, http_req.client_l4port);
			diag_util_mprintf("server_l4port %u(0x%x) \n", http_req.server_l4port, http_req.server_l4port);
			diag_util_mprintf("ldpid %u\n", http_req.ldpid);
			diag_util_mprintf("pppoe_sid %d svlan_vid %d cvlan_vid %d \n", http_req.pppoe_sid, http_req.svlan_vid, http_req.cvlan_vid);
			diag_util_mprintf("pon_streamId %d \n", http_req.pon_streamId);
			diag_util_mprintf("http_req_url %s \n", http_req.http_req_url);
			diag_util_mprintf("http_version %s \n", http_req.http_version);
			diag_util_mprintf("upload_content_length %llu \n", http_req.upload_content_length);
			diag_util_mprintf("tcp_window_size %u \n", http_req.tcp_window_size);
		}
	}	

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_pe_http_test_upload_start_connection_number_connection_number_client_mac_client_mac_server_mac_server_mac_ipv6_client_ip_client_ip_server_ip_server_ip_client_l4port_client_l4port_server_l4port_server_l4port_ldpid_ldpid_pppoe_sid_pppoe_sid_svlan_vid_svlan_vid_cvlan_vid_cvlan_vid_pon_streamid_pon_streamid_http_req_url_http_req_url_http_version_http_version_upload_content_length_upload_content_length_upload_window_size_upload_window_size */


/*
 * rt_pe http_test latency_test get_counter
 */
cparser_result_t
cparser_cmd_rt_pe_http_test_latency_test_get_counter(
    cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED;
	rt_pe_http_test_request_t http_req;
	rt_pe_http_test_result_t http_result;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&http_req, 0x0, sizeof(rt_pe_http_test_request_t));
	http_req.req_cmd = RT_PE_HTTP_TEST_CMD_LATENCY_GET_RESULT;

	ret = rt_pe_http_test(http_req, &http_result);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to get counter of http latency test ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to get counter of http latency test ... RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		if(http_result.ret_val != RT_PE_RET_OK)
			diag_util_mprintf("Fail to get counter of http latency test ... PE return 0x%x \n", http_result.ret_val);
		else
		{
			diag_util_mprintf("========== Result of http latency test ========== \n");
			diag_util_mprintf("Latency %u.%03u ms \n", (uint32)(http_result.latency_us/RT_PE_USEC_PER_MSEC),  (uint32)(http_result.latency_us%RT_PE_USEC_PER_MSEC));
			diag_util_mprintf("Jitter %u.%03u ms \n", (uint32)(http_result.jitter_us/RT_PE_USEC_PER_MSEC),  (uint32)(http_result.jitter_us%RT_PE_USEC_PER_MSEC));
			diag_util_mprintf("pktLoss %u/%u \n", http_result.pktLoss, (http_result.pktLoss+http_result.pktRcv));
		}
	}	

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_pe_http_test_latency_test_get_counter */

/*
 * rt_pe http_test latency_test stop 
 */
cparser_result_t
cparser_cmd_rt_pe_http_test_latency_test_stop(
    cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED;
	rt_pe_http_test_request_t http_req;
	rt_pe_http_test_result_t http_result;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&http_req, 0x0, sizeof(rt_pe_http_test_request_t));
	http_req.req_cmd = RT_PE_HTTP_TEST_CMD_LATENCY_STOP;

	ret = rt_pe_http_test(http_req, &http_result);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to stop http latency test ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to stop http latency test ... RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		if(http_result.ret_val != RT_PE_RET_OK)
			diag_util_mprintf("Fail to stop http latency test ... PE return 0x%x \n", http_result.ret_val);
		else
		{
			diag_util_mprintf("========== Result of http latency test ==========  \n");
			diag_util_mprintf("Latency %u.%03u ms \n", (uint32)(http_result.latency_us/RT_PE_USEC_PER_MSEC),  (uint32)(http_result.latency_us%RT_PE_USEC_PER_MSEC));
			diag_util_mprintf("Jitter %u.%03u ms \n", (uint32)(http_result.jitter_us/RT_PE_USEC_PER_MSEC),  (uint32)(http_result.jitter_us%RT_PE_USEC_PER_MSEC));
			diag_util_mprintf("pktLoss %u/%u \n", http_result.pktLoss, (http_result.pktLoss+http_result.pktRcv));
		}
	}	

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_pe_http_test_latency_test_stop */

/*
 * rt_pe http_test latency_test start client_mac <MACADDR:client_mac> server_mac <MACADDR:server_mac> IPv4 client_ip <IPV4ADDR:client_ip> server_ip <IPV4ADDR:server_ip> client_l4port <UINT:client_l4port> server_l4port <UINT:server_l4port> ldpid <UINT:ldpid> pppoe_sid <INT:pppoe_sid> svlan_vid <INT:svlan_vid> cvlan_vid <INT:cvlan_vid> pon_streamId <INT:pon_streamId> http_req_url <STRING:http_req_url> http_version <STRING:http_version>
 */
cparser_result_t
cparser_cmd_rt_pe_http_test_latency_test_start_client_mac_client_mac_server_mac_server_mac_IPv4_client_ip_client_ip_server_ip_server_ip_client_l4port_client_l4port_server_l4port_server_l4port_ldpid_ldpid_pppoe_sid_pppoe_sid_svlan_vid_svlan_vid_cvlan_vid_cvlan_vid_pon_streamId_pon_streamId_http_req_url_http_req_url_http_version_http_version(
    cparser_context_t *context,
    cparser_macaddr_t  *client_mac_ptr,
    cparser_macaddr_t  *server_mac_ptr,
    uint32_t  *client_ip_ptr,
    uint32_t  *server_ip_ptr,
    uint32_t  *client_l4port_ptr,
    uint32_t  *server_l4port_ptr,
    uint32_t  *ldpid_ptr,
    int32_t  *pppoe_sid_ptr,
    int32_t  *svlan_vid_ptr,
    int32_t  *cvlan_vid_ptr,
    int32_t  *pon_streamId_ptr,
    char * *http_req_url_ptr,
    char * *http_version_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_pe_http_test_request_t http_req;
	rt_pe_http_test_result_t http_result;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&http_req, 0x0, sizeof(rt_pe_http_test_request_t));
	http_req.req_cmd = RT_PE_HTTP_TEST_CMD_LATENCY_START;
	http_req.connection_number = 1;
	osal_memcpy(http_req.client_mac.octet, client_mac_ptr->octet, ETHER_ADDR_LEN);
	osal_memcpy(http_req.server_mac.octet, server_mac_ptr->octet, ETHER_ADDR_LEN);	
	http_req.isIPv4OrIpv6 = 0;
	http_req.client_ip.ipv4_addr = (rtk_ip_addr_t)*client_ip_ptr;
	http_req.server_ip.ipv4_addr = (rtk_ip_addr_t)*server_ip_ptr;
	http_req.client_l4port = *client_l4port_ptr;
	http_req.server_l4port = *server_l4port_ptr;
	http_req.ldpid = *ldpid_ptr;
	http_req.pppoe_sid = *pppoe_sid_ptr;
	http_req.svlan_vid = *svlan_vid_ptr;
	http_req.cvlan_vid = *cvlan_vid_ptr;
	http_req.pon_streamId = *pon_streamId_ptr;
	osal_memcpy(http_req.http_req_url, *http_req_url_ptr, MAX_PE_HTTP_REQ_URL_STR_LENGTH);
	osal_memcpy(http_req.http_version, *http_version_ptr, MAX_PE_HTTP_VERSION_STR_LENGTH);
	
	ret = rt_pe_http_test(http_req, &http_result);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to start http latency test ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to start http latency test ... RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		if(http_result.ret_val != RT_PE_RET_OK)
			diag_util_mprintf("Fail to start http latency test ... PE return 0x%x \n", http_result.ret_val);
		else
		{
			diag_util_mprintf("========== Success to start http latency test ========== \n");
			diag_util_mprintf("connection_number %u \n", http_req.connection_number);
			diag_util_mprintf("client_mac %02x:%02x:%02x:%02x:%02x:%02x \n", http_req.client_mac.octet[0], http_req.client_mac.octet[1], http_req.client_mac.octet[2], http_req.client_mac.octet[3], http_req.client_mac.octet[4], http_req.client_mac.octet[5]);
			diag_util_mprintf("server_mac %02x:%02x:%02x:%02x:%02x:%02x \n", http_req.server_mac.octet[0], http_req.server_mac.octet[1], http_req.server_mac.octet[2], http_req.server_mac.octet[3], http_req.server_mac.octet[4], http_req.server_mac.octet[5]);
			diag_util_mprintf("client_ipv4 %u.%u.%u.%u \n", (http_req.client_ip.ipv4_addr>>24)&0xff, (http_req.client_ip.ipv4_addr>>16)&0xff, (http_req.client_ip.ipv4_addr>>8)&0xff, (http_req.client_ip.ipv4_addr)&0xff);
			diag_util_mprintf("server_ipv4 %u.%u.%u.%u \n", (http_req.server_ip.ipv4_addr>>24)&0xff, (http_req.server_ip.ipv4_addr>>16)&0xff, (http_req.server_ip.ipv4_addr>>8)&0xff, (http_req.server_ip.ipv4_addr)&0xff);
			diag_util_mprintf("client_l4port %u(0x%x) \n", http_req.client_l4port, http_req.client_l4port);
			diag_util_mprintf("server_l4port %u(0x%x) \n", http_req.server_l4port, http_req.server_l4port);
			diag_util_mprintf("ldpid %u\n", http_req.ldpid);
			diag_util_mprintf("pppoe_sid %d svlan_vid %d cvlan_vid %d \n", http_req.pppoe_sid, http_req.svlan_vid, http_req.cvlan_vid);
			diag_util_mprintf("pon_streamId %d \n", http_req.pon_streamId);
			diag_util_mprintf("http_req_url %s \n", http_req.http_req_url);
			diag_util_mprintf("http_version %s \n", http_req.http_version);
		}
	}	

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_pe_http_test_latency_test_start_client_mac_client_mac_server_mac_server_mac_ipv4_client_ip_client_ip_server_ip_server_ip_client_l4port_client_l4port_server_l4port_server_l4port_ldpid_ldpid_pppoe_sid_pppoe_sid_svlan_vid_svlan_vid_cvlan_vid_cvlan_vid_pon_streamid_pon_streamid_http_req_url_http_req_url_http_version_http_version */

/*
 * rt_pe http_test latency_test start client_mac <MACADDR:client_mac> server_mac <MACADDR:server_mac> IPv6 client_ip <IPV6ADDR:client_ip> server_ip <IPV6ADDR:server_ip> client_l4port <UINT:client_l4port> server_l4port <UINT:server_l4port> ldpid <UINT:ldpid> pppoe_sid <INT:pppoe_sid> svlan_vid <INT:svlan_vid> cvlan_vid <INT:cvlan_vid> pon_streamId <INT:pon_streamId> http_req_url <STRING:http_req_url> http_version <STRING:http_version>
 */
cparser_result_t
cparser_cmd_rt_pe_http_test_latency_test_start_client_mac_client_mac_server_mac_server_mac_IPv6_client_ip_client_ip_server_ip_server_ip_client_l4port_client_l4port_server_l4port_server_l4port_ldpid_ldpid_pppoe_sid_pppoe_sid_svlan_vid_svlan_vid_cvlan_vid_cvlan_vid_pon_streamId_pon_streamId_http_req_url_http_req_url_http_version_http_version(
    cparser_context_t *context,
    cparser_macaddr_t  *client_mac_ptr,
    cparser_macaddr_t  *server_mac_ptr,
    char * *client_ip_ptr,
    char * *server_ip_ptr,
    uint32_t  *client_l4port_ptr,
    uint32_t  *server_l4port_ptr,
    uint32_t  *ldpid_ptr,
    int32_t  *pppoe_sid_ptr,
    int32_t  *svlan_vid_ptr,
    int32_t  *cvlan_vid_ptr,
    int32_t  *pon_streamId_ptr,
    char * *http_req_url_ptr,
    char * *http_version_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_pe_http_test_request_t http_req;
	rt_pe_http_test_result_t http_result;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&http_req, 0x0, sizeof(rt_pe_http_test_request_t));
	http_req.req_cmd = RT_PE_HTTP_TEST_CMD_LATENCY_START;
	http_req.connection_number = 1;
	osal_memcpy(http_req.client_mac.octet, client_mac_ptr->octet, ETHER_ADDR_LEN);
	osal_memcpy(http_req.server_mac.octet, server_mac_ptr->octet, ETHER_ADDR_LEN);	
	http_req.isIPv4OrIpv6 = 1;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(http_req.client_ip.ipv6_addr.ipv6_addr, TOKEN_STR(10)), ret);
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(http_req.server_ip.ipv6_addr.ipv6_addr, TOKEN_STR(12)), ret);
	http_req.client_l4port = *client_l4port_ptr;
	http_req.server_l4port = *server_l4port_ptr;
	http_req.ldpid = *ldpid_ptr;
	http_req.pppoe_sid = *pppoe_sid_ptr;
	http_req.svlan_vid = *svlan_vid_ptr;
	http_req.cvlan_vid = *cvlan_vid_ptr;
	http_req.pon_streamId = *pon_streamId_ptr;
	osal_memcpy(http_req.http_req_url, *http_req_url_ptr, MAX_PE_HTTP_REQ_URL_STR_LENGTH);
	osal_memcpy(http_req.http_version, *http_version_ptr, MAX_PE_HTTP_VERSION_STR_LENGTH);

	ret = rt_pe_http_test(http_req, &http_result);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to start http latency test ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to start http latency test ... RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		if(http_result.ret_val != RT_PE_RET_OK)
			diag_util_mprintf("Fail to start http latency test ... PE return 0x%x \n", http_result.ret_val);
		else
		{
			diag_util_mprintf("========== Success to start http latency test ==========\n");
			diag_util_mprintf("connection_number %u \n", http_req.connection_number);
			diag_util_mprintf("client_mac %02x:%02x:%02x:%02x:%02x:%02x \n", http_req.client_mac.octet[0], http_req.client_mac.octet[1], http_req.client_mac.octet[2], http_req.client_mac.octet[3], http_req.client_mac.octet[4], http_req.client_mac.octet[5]);
			diag_util_mprintf("server_mac %02x:%02x:%02x:%02x:%02x:%02x \n", http_req.server_mac.octet[0], http_req.server_mac.octet[1], http_req.server_mac.octet[2], http_req.server_mac.octet[3], http_req.server_mac.octet[4], http_req.server_mac.octet[5]);
			diag_util_mprintf("client_ipv6 %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n", 
								http_req.client_ip.ipv6_addr.ipv6_addr[0], http_req.client_ip.ipv6_addr.ipv6_addr[1], http_req.client_ip.ipv6_addr.ipv6_addr[2], http_req.client_ip.ipv6_addr.ipv6_addr[3],
								http_req.client_ip.ipv6_addr.ipv6_addr[4], http_req.client_ip.ipv6_addr.ipv6_addr[5], http_req.client_ip.ipv6_addr.ipv6_addr[6], http_req.client_ip.ipv6_addr.ipv6_addr[7],
								http_req.client_ip.ipv6_addr.ipv6_addr[8], http_req.client_ip.ipv6_addr.ipv6_addr[9], http_req.client_ip.ipv6_addr.ipv6_addr[10], http_req.client_ip.ipv6_addr.ipv6_addr[11],
								http_req.client_ip.ipv6_addr.ipv6_addr[12], http_req.client_ip.ipv6_addr.ipv6_addr[13], http_req.client_ip.ipv6_addr.ipv6_addr[14], http_req.client_ip.ipv6_addr.ipv6_addr[15]);
			diag_util_mprintf("server_ipv6 %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n", 
								http_req.server_ip.ipv6_addr.ipv6_addr[0], http_req.server_ip.ipv6_addr.ipv6_addr[1], http_req.server_ip.ipv6_addr.ipv6_addr[2], http_req.server_ip.ipv6_addr.ipv6_addr[3],
								http_req.server_ip.ipv6_addr.ipv6_addr[4], http_req.server_ip.ipv6_addr.ipv6_addr[5], http_req.server_ip.ipv6_addr.ipv6_addr[6], http_req.server_ip.ipv6_addr.ipv6_addr[7],
								http_req.server_ip.ipv6_addr.ipv6_addr[8], http_req.server_ip.ipv6_addr.ipv6_addr[9], http_req.server_ip.ipv6_addr.ipv6_addr[10], http_req.server_ip.ipv6_addr.ipv6_addr[11],
								http_req.server_ip.ipv6_addr.ipv6_addr[12], http_req.server_ip.ipv6_addr.ipv6_addr[13], http_req.server_ip.ipv6_addr.ipv6_addr[14], http_req.server_ip.ipv6_addr.ipv6_addr[15]);
			diag_util_mprintf("client_l4port %u(0x%x) \n", http_req.client_l4port, http_req.client_l4port);
			diag_util_mprintf("server_l4port %u(0x%x) \n", http_req.server_l4port, http_req.server_l4port);
			diag_util_mprintf("ldpid %u\n", http_req.ldpid);
			diag_util_mprintf("pppoe_sid %d svlan_vid %d cvlan_vid %d \n", http_req.pppoe_sid, http_req.svlan_vid, http_req.cvlan_vid);
			diag_util_mprintf("pon_streamId %d \n", http_req.pon_streamId);
			diag_util_mprintf("http_req_url %s \n", http_req.http_req_url);
			diag_util_mprintf("http_version %s \n", http_req.http_version);
		}
	}	

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_pe_http_test_latency_test_start_client_mac_client_mac_server_mac_server_mac_ipv6_client_ip_client_ip_server_ip_server_ip_client_l4port_client_l4port_server_l4port_server_l4port_ldpid_ldpid_pppoe_sid_pppoe_sid_svlan_vid_svlan_vid_cvlan_vid_cvlan_vid_pon_streamid_pon_streamid_http_req_url_http_req_url_http_version_http_version */

/*
 * rt_pe iperf_test download get_counter
 */
cparser_result_t
cparser_cmd_rt_pe_iperf_test_download_get_counter(
	cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED;
	rt_pe_iperf_request_t iperf_req;
	rt_pe_iperf_result_t iperf_result;
	
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&iperf_req, 0x0, sizeof(rt_pe_iperf_request_t));
	iperf_req.req_cmd = RT_PE_IPERF_REQ_CMD_DOWNLOAD_GET_CNT;

	ret = rt_pe_iperf_test(iperf_req, &iperf_result);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to get counter of iperf download test ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to get counter of iperf download test ... RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		if(iperf_result.ret_val != RT_PE_RET_OK)
			diag_util_mprintf("Fail to get counter of iperf download test ... PE return 0x%x \n", iperf_result.ret_val);
		else
		{
			uint64 avg_speed_Mbps, duration_milliseconds=iperf_result.cost_time_ms;
			
			diag_util_mprintf("========== Result of iperf download test ========== \n"); 
			diag_util_mprintf("Duration(cost_time_ms) %llu ms \n", duration_milliseconds);
			diag_util_mprintf("================ Test statistic =============== \n");
			diag_util_mprintf("PktCnt %u \n", iperf_result.pktCnt);
			diag_util_mprintf("ByteCnt(payload only) %llu \n", iperf_result.byteCnt);
			diag_util_mprintf("ByteCnt(whole packet) %llu \n", iperf_result.byteCnt_include_pktHdr);
			avg_speed_Mbps = (iperf_result.byteCnt*8*1000/duration_milliseconds)/1000000;
			diag_util_mprintf("Average speed(payload only) %u.%03u Gpbs \n", (uint32)(avg_speed_Mbps/1000), (uint32)(avg_speed_Mbps%1000));
			avg_speed_Mbps = (iperf_result.byteCnt_include_pktHdr*8*1000/duration_milliseconds)/1000000;
			diag_util_mprintf("Average speed(whole packet) %u.%03u Gpbs \n", (uint32)(avg_speed_Mbps/1000), (uint32)(avg_speed_Mbps%1000));
		}
	}	

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_pe_iperf_test_download_get_counter */

/*
 * rt_pe iperf_test download stop 
 */
cparser_result_t
cparser_cmd_rt_pe_iperf_test_download_stop(
	cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED;
	rt_pe_iperf_request_t iperf_req;
	rt_pe_iperf_result_t iperf_result;
	
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&iperf_req, 0x0, sizeof(rt_pe_iperf_request_t));
	iperf_req.req_cmd = RT_PE_IPERF_REQ_CMD_DOWNLOAD_STOP;

	ret = rt_pe_iperf_test(iperf_req, &iperf_result);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to stop iperf download test ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to stop iperf download test ... RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		if(iperf_result.ret_val != RT_PE_RET_OK)
			diag_util_mprintf("Fail to stop iperf download test ... PE return 0x%x \n", iperf_result.ret_val);
		else
		{
			uint64 avg_speed_Mbps, duration_milliseconds=iperf_result.cost_time_ms;
			
			diag_util_mprintf("========== Result of iperf download test ========== \n"); 
			diag_util_mprintf("Duration(cost_time_ms) %llu ms \n", duration_milliseconds);
			diag_util_mprintf("================ Test statistic =============== \n");
			diag_util_mprintf("PktCnt %u \n", iperf_result.pktCnt);
			diag_util_mprintf("ByteCnt(payload only) %llu \n", iperf_result.byteCnt);
			diag_util_mprintf("ByteCnt(whole packet) %llu \n", iperf_result.byteCnt_include_pktHdr);
			avg_speed_Mbps = (iperf_result.byteCnt*8*1000/duration_milliseconds)/1000000;
			diag_util_mprintf("Average speed(payload only) %u.%03u Gpbs \n", (uint32)(avg_speed_Mbps/1000), (uint32)(avg_speed_Mbps%1000));
			avg_speed_Mbps = (iperf_result.byteCnt_include_pktHdr*8*1000/duration_milliseconds)/1000000;
			diag_util_mprintf("Average speed(whole packet) %u.%03u Gpbs \n", (uint32)(avg_speed_Mbps/1000), (uint32)(avg_speed_Mbps%1000));
		}
	}	

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_pe_iperf_test_download_stop */

/*
 * rt_pe iperf_test download start connection_number <UINT:connection_number> client_mac <MACADDR:client_mac> server_mac <MACADDR:server_mac> IPv4 client_ip <IPV4ADDR:client_ip> server_ip <IPV4ADDR:server_ip> UDP client_l4port <UINT:client_l4port> server_l4port <UINT:server_l4port> ldpid <UINT:ldpid> pppoe_sid <INT:pppoe_sid> svlan_vid <INT:svlan_vid> cvlan_vid <INT:cvlan_vid> pon_streamId <INT:pon_streamId> upload_first_payload_length <UINT:upload_first_payload_length> upload_first_payload <STRING:upload_first_payload>
 */
cparser_result_t
cparser_cmd_rt_pe_iperf_test_download_start_connection_number_connection_number_client_mac_client_mac_server_mac_server_mac_IPv4_client_ip_client_ip_server_ip_server_ip_UDP_client_l4port_client_l4port_server_l4port_server_l4port_ldpid_ldpid_pppoe_sid_pppoe_sid_svlan_vid_svlan_vid_cvlan_vid_cvlan_vid_pon_streamId_pon_streamId_upload_first_payload_length_upload_first_payload_length_upload_first_payload_upload_first_payload(
	cparser_context_t *context,
	uint32_t  *connection_number_ptr,
	cparser_macaddr_t  *client_mac_ptr,
	cparser_macaddr_t  *server_mac_ptr,
	uint32_t  *client_ip_ptr,
	uint32_t  *server_ip_ptr,
	uint32_t  *client_l4port_ptr,
	uint32_t  *server_l4port_ptr,
	uint32_t  *ldpid_ptr,
	int32_t  *pppoe_sid_ptr,
	int32_t  *svlan_vid_ptr,
	int32_t  *cvlan_vid_ptr,
	int32_t  *pon_streamId_ptr,
	uint32_t  *upload_first_payload_length,
	char * *upload_first_payload)
{
	int32 ret = RT_ERR_FAILED;
	rt_pe_iperf_request_t iperf_req;
	rt_pe_iperf_result_t iperf_result;
	
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&iperf_req, 0x0, sizeof(rt_pe_iperf_request_t));
	iperf_req.req_cmd = RT_PE_IPERF_REQ_CMD_DOWNLOAD_START_UDP;
	iperf_req.connection_number = *connection_number_ptr;
	osal_memcpy(iperf_req.client_mac.octet, client_mac_ptr->octet, ETHER_ADDR_LEN);
	osal_memcpy(iperf_req.server_mac.octet, server_mac_ptr->octet, ETHER_ADDR_LEN);	
	iperf_req.isIPv4OrIpv6 = 0;
	iperf_req.client_ip.ipv4_addr = (rtk_ip_addr_t)*client_ip_ptr;
	iperf_req.server_ip.ipv4_addr = (rtk_ip_addr_t)*server_ip_ptr;
	iperf_req.client_l4port[0] = *client_l4port_ptr;
	iperf_req.server_l4port[0] = *server_l4port_ptr;
	iperf_req.ldpid = *ldpid_ptr;
	iperf_req.pppoe_sid = *pppoe_sid_ptr;
	iperf_req.svlan_vid = *svlan_vid_ptr;
	iperf_req.cvlan_vid = *cvlan_vid_ptr;
	iperf_req.pon_streamId = *pon_streamId_ptr;
	iperf_req.udp_upload_first_payload_length = *upload_first_payload_length;
	osal_memcpy(iperf_req.udp_upload_first_payload, *upload_first_payload, MAX_PE_IPERF_FIRST_PAYLOAD_LENGTH);
	
	ret = rt_pe_iperf_test(iperf_req, &iperf_result);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to start iperf download test ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to start iperf download test ... RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		if(iperf_result.ret_val != RT_PE_RET_OK)
			diag_util_mprintf("Fail to start iperf download test ... PE return 0x%x \n", iperf_result.ret_val);
		else
		{
			diag_util_mprintf("========== Success to start iperf udp download test ==========\n");
			diag_util_mprintf("connection_number %u \n", iperf_req.connection_number);
			diag_util_mprintf("client_mac %02x:%02x:%02x:%02x:%02x:%02x \n", iperf_req.client_mac.octet[0], iperf_req.client_mac.octet[1], iperf_req.client_mac.octet[2], iperf_req.client_mac.octet[3], iperf_req.client_mac.octet[4], iperf_req.client_mac.octet[5]);
			diag_util_mprintf("server_mac %02x:%02x:%02x:%02x:%02x:%02x \n", iperf_req.server_mac.octet[0], iperf_req.server_mac.octet[1], iperf_req.server_mac.octet[2], iperf_req.server_mac.octet[3], iperf_req.server_mac.octet[4], iperf_req.server_mac.octet[5]);
			diag_util_mprintf("client_ipv4 %u.%u.%u.%u \n", (iperf_req.client_ip.ipv4_addr>>24)&0xff, (iperf_req.client_ip.ipv4_addr>>16)&0xff, (iperf_req.client_ip.ipv4_addr>>8)&0xff, (iperf_req.client_ip.ipv4_addr)&0xff);
			diag_util_mprintf("server_ipv4 %u.%u.%u.%u \n", (iperf_req.server_ip.ipv4_addr>>24)&0xff, (iperf_req.server_ip.ipv4_addr>>16)&0xff, (iperf_req.server_ip.ipv4_addr>>8)&0xff, (iperf_req.server_ip.ipv4_addr)&0xff);
			diag_util_mprintf("client_l4port %u(0x%x) \n", iperf_req.client_l4port, iperf_req.client_l4port);
			diag_util_mprintf("server_l4port %u(0x%x) \n", iperf_req.server_l4port, iperf_req.server_l4port);
			diag_util_mprintf("ldpid %u\n", iperf_req.ldpid);
			diag_util_mprintf("pppoe_sid %d svlan_vid %d cvlan_vid %d \n", iperf_req.pppoe_sid, iperf_req.svlan_vid, iperf_req.cvlan_vid);
			diag_util_mprintf("pon_streamId %d \n", iperf_req.pon_streamId);
			diag_util_mprintf("upload_first_payload_length %u\n", iperf_req.udp_upload_first_payload_length);
			diag_util_mprintf("upload_first_payload %s \n", iperf_req.udp_upload_first_payload);
		}
	}	

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_pe_iperf_test_download_start_connection_number_connection_number_client_mac_client_mac_server_mac_server_mac_IPv4_client_ip_client_ip_server_ip_server_ip_UDP_client_l4port_client_l4port_server_l4port_server_l4port_ldpid_ldpid_pppoe_sid_pppoe_sid_svlan_vid_svlan_vid_cvlan_vid_cvlan_vid_pon_streamId_pon_streamId_upload_first_payload_length_upload_first_payload_length_upload_first_payload_upload_first_payload */

/*
 * rt_pe iperf_test download start connection_number <UINT:connection_number> client_mac <MACADDR:client_mac> server_mac <MACADDR:server_mac> IPv6 client_ip <IPV6ADDR:client_ip> server_ip <IPV6ADDR:server_ip> UDP client_l4port <UINT:client_l4port> server_l4port <UINT:server_l4port> ldpid <UINT:ldpid> pppoe_sid <INT:pppoe_sid> svlan_vid <INT:svlan_vid> cvlan_vid <INT:cvlan_vid> pon_streamId <INT:pon_streamId> upload_first_payload_length <UINT:upload_first_payload_length> upload_first_payload <STRING:upload_first_payload>
 */
cparser_result_t
cparser_cmd_rt_pe_iperf_test_download_start_connection_number_connection_number_client_mac_client_mac_server_mac_server_mac_IPv6_client_ip_client_ip_server_ip_server_ip_UDP_client_l4port_client_l4port_server_l4port_server_l4port_ldpid_ldpid_pppoe_sid_pppoe_sid_svlan_vid_svlan_vid_cvlan_vid_cvlan_vid_pon_streamId_pon_streamId_upload_first_payload_length_upload_first_payload_length_upload_first_payload_upload_first_payload(
	cparser_context_t *context,
	uint32_t  *connection_number_ptr,
	cparser_macaddr_t  *client_mac_ptr,
	cparser_macaddr_t  *server_mac_ptr,
	char * *client_ip_ptr,
	char * *server_ip_ptr,
	uint32_t  *client_l4port_ptr,
	uint32_t  *server_l4port_ptr,
	uint32_t  *ldpid_ptr,
	int32_t  *pppoe_sid_ptr,
	int32_t  *svlan_vid_ptr,
	int32_t  *cvlan_vid_ptr,
	int32_t  *pon_streamId_ptr,
	uint32_t  *upload_first_payload_length,
	char * *upload_first_payload)
{
	int32 ret = RT_ERR_FAILED;
	rt_pe_iperf_request_t iperf_req;
	rt_pe_iperf_result_t iperf_result;
	
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&iperf_req, 0x0, sizeof(rt_pe_iperf_request_t));
	iperf_req.req_cmd = RT_PE_IPERF_REQ_CMD_DOWNLOAD_START_UDP;
	iperf_req.connection_number = *connection_number_ptr;
	osal_memcpy(iperf_req.client_mac.octet, client_mac_ptr->octet, ETHER_ADDR_LEN);
	osal_memcpy(iperf_req.server_mac.octet, server_mac_ptr->octet, ETHER_ADDR_LEN);	
	iperf_req.isIPv4OrIpv6 = 1;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(iperf_req.client_ip.ipv6_addr.ipv6_addr, TOKEN_STR(12)), ret);
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(iperf_req.server_ip.ipv6_addr.ipv6_addr, TOKEN_STR(14)), ret);
	iperf_req.client_l4port[0] = *client_l4port_ptr;
	iperf_req.server_l4port[0] = *server_l4port_ptr;
	iperf_req.ldpid = *ldpid_ptr;
	iperf_req.pppoe_sid = *pppoe_sid_ptr;
	iperf_req.svlan_vid = *svlan_vid_ptr;
	iperf_req.cvlan_vid = *cvlan_vid_ptr;
	iperf_req.pon_streamId = *pon_streamId_ptr;
	iperf_req.udp_upload_first_payload_length = *upload_first_payload_length;
	osal_memcpy(iperf_req.udp_upload_first_payload, *upload_first_payload, MAX_PE_IPERF_FIRST_PAYLOAD_LENGTH);

	ret = rt_pe_iperf_test(iperf_req, &iperf_result);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to start iperf download test ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to start iperf download test ... RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		if(iperf_result.ret_val != RT_PE_RET_OK)
			diag_util_mprintf("Fail to start iperf download test ... PE return 0x%x \n", iperf_result.ret_val);
		else
		{
			diag_util_mprintf("========== Success to start iperf udp download test ==========\n");
			diag_util_mprintf("connection_number %u \n", iperf_req.connection_number);
			diag_util_mprintf("client_mac %02x:%02x:%02x:%02x:%02x:%02x \n", iperf_req.client_mac.octet[0], iperf_req.client_mac.octet[1], iperf_req.client_mac.octet[2], iperf_req.client_mac.octet[3], iperf_req.client_mac.octet[4], iperf_req.client_mac.octet[5]);
			diag_util_mprintf("server_mac %02x:%02x:%02x:%02x:%02x:%02x \n", iperf_req.server_mac.octet[0], iperf_req.server_mac.octet[1], iperf_req.server_mac.octet[2], iperf_req.server_mac.octet[3], iperf_req.server_mac.octet[4], iperf_req.server_mac.octet[5]);
			diag_util_mprintf("client_ipv6 %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n", 
								iperf_req.client_ip.ipv6_addr.ipv6_addr[0], iperf_req.client_ip.ipv6_addr.ipv6_addr[1], iperf_req.client_ip.ipv6_addr.ipv6_addr[2], iperf_req.client_ip.ipv6_addr.ipv6_addr[3],
								iperf_req.client_ip.ipv6_addr.ipv6_addr[4], iperf_req.client_ip.ipv6_addr.ipv6_addr[5], iperf_req.client_ip.ipv6_addr.ipv6_addr[6], iperf_req.client_ip.ipv6_addr.ipv6_addr[7],
								iperf_req.client_ip.ipv6_addr.ipv6_addr[8], iperf_req.client_ip.ipv6_addr.ipv6_addr[9], iperf_req.client_ip.ipv6_addr.ipv6_addr[10], iperf_req.client_ip.ipv6_addr.ipv6_addr[11],
								iperf_req.client_ip.ipv6_addr.ipv6_addr[12], iperf_req.client_ip.ipv6_addr.ipv6_addr[13], iperf_req.client_ip.ipv6_addr.ipv6_addr[14], iperf_req.client_ip.ipv6_addr.ipv6_addr[15]);
			diag_util_mprintf("server_ipv6 %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n", 
								iperf_req.server_ip.ipv6_addr.ipv6_addr[0], iperf_req.server_ip.ipv6_addr.ipv6_addr[1], iperf_req.server_ip.ipv6_addr.ipv6_addr[2], iperf_req.server_ip.ipv6_addr.ipv6_addr[3],
								iperf_req.server_ip.ipv6_addr.ipv6_addr[4], iperf_req.server_ip.ipv6_addr.ipv6_addr[5], iperf_req.server_ip.ipv6_addr.ipv6_addr[6], iperf_req.server_ip.ipv6_addr.ipv6_addr[7],
								iperf_req.server_ip.ipv6_addr.ipv6_addr[8], iperf_req.server_ip.ipv6_addr.ipv6_addr[9], iperf_req.server_ip.ipv6_addr.ipv6_addr[10], iperf_req.server_ip.ipv6_addr.ipv6_addr[11],
								iperf_req.server_ip.ipv6_addr.ipv6_addr[12], iperf_req.server_ip.ipv6_addr.ipv6_addr[13], iperf_req.server_ip.ipv6_addr.ipv6_addr[14], iperf_req.server_ip.ipv6_addr.ipv6_addr[15]);
			diag_util_mprintf("client_l4port %u(0x%x) \n", iperf_req.client_l4port, iperf_req.client_l4port);
			diag_util_mprintf("server_l4port %u(0x%x) \n", iperf_req.server_l4port, iperf_req.server_l4port);
			diag_util_mprintf("ldpid %u\n", iperf_req.ldpid);
			diag_util_mprintf("pppoe_sid %d svlan_vid %d cvlan_vid %d \n", iperf_req.pppoe_sid, iperf_req.svlan_vid, iperf_req.cvlan_vid);
			diag_util_mprintf("pon_streamId %d \n", iperf_req.pon_streamId);
			diag_util_mprintf("upload_first_payload_length %u\n", iperf_req.udp_upload_first_payload_length);
			diag_util_mprintf("upload_first_payload %s \n", iperf_req.udp_upload_first_payload);
		}
	}	

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_pe_iperf_test_download_start_connection_number_connection_number_client_mac_client_mac_server_mac_server_mac_IPv6_client_ip_client_ip_server_ip_server_ip_UDP_client_l4port_client_l4port_server_l4port_server_l4port_ldpid_ldpid_pppoe_sid_pppoe_sid_svlan_vid_svlan_vid_cvlan_vid_cvlan_vid_pon_streamId_pon_streamId_upload_first_payload_length_upload_first_payload_length_upload_first_payload_upload_first_payload */

/*
 * rt_pe iperf_test upload get_counter
 */
cparser_result_t
cparser_cmd_rt_pe_iperf_test_upload_get_counter(
	cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED;
	rt_pe_iperf_request_t iperf_req;
	rt_pe_iperf_result_t iperf_result;
	
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&iperf_req, 0x0, sizeof(rt_pe_iperf_request_t));
	iperf_req.req_cmd = RT_PE_IPERF_REQ_CMD_UPLOAD_GET_CNT;

	ret = rt_pe_iperf_test(iperf_req, &iperf_result);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to get counter of iperf upload test ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to get counter of iperf upload test ... RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		if(iperf_result.ret_val != RT_PE_RET_OK)
			diag_util_mprintf("Fail to get counter of iperf upload test ... PE return 0x%x \n", iperf_result.ret_val);
		else
		{
			uint64 avg_speed_Mbps, duration_milliseconds=iperf_result.cost_time_ms;
			
			diag_util_mprintf("========== Result of iperf download test ========== \n"); 
			diag_util_mprintf("Duration(cost_time_ms) %llu ms \n", duration_milliseconds);
			diag_util_mprintf("================ Test statistic =============== \n");
			diag_util_mprintf("PktCnt %u \n", iperf_result.pktCnt);
			diag_util_mprintf("ByteCnt(payload only) %llu \n", iperf_result.byteCnt);
			diag_util_mprintf("ByteCnt(whole packet) %llu \n", iperf_result.byteCnt_include_pktHdr);
			avg_speed_Mbps = (iperf_result.byteCnt*8*1000/duration_milliseconds)/1000000;
			diag_util_mprintf("Average speed(payload only) %u.%03u Gpbs \n", (uint32)(avg_speed_Mbps/1000), (uint32)(avg_speed_Mbps%1000));
			avg_speed_Mbps = (iperf_result.byteCnt_include_pktHdr*8*1000/duration_milliseconds)/1000000;
			diag_util_mprintf("Average speed(whole packet) %u.%03u Gpbs \n", (uint32)(avg_speed_Mbps/1000), (uint32)(avg_speed_Mbps%1000));
		}
	}	

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_pe_iperf_test_upload_get_counter */

/*
 * rt_pe iperf_test upload stop 
 */
cparser_result_t
cparser_cmd_rt_pe_iperf_test_upload_stop(
	cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED;
	rt_pe_iperf_request_t iperf_req;
	rt_pe_iperf_result_t iperf_result;
	
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&iperf_req, 0x0, sizeof(rt_pe_iperf_request_t));
	iperf_req.req_cmd = RT_PE_IPERF_REQ_CMD_UPLOAD_STOP;

	ret = rt_pe_iperf_test(iperf_req, &iperf_result);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to stop iperf upload test ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to stop iperf upload test ... RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		if(iperf_result.ret_val != RT_PE_RET_OK)
			diag_util_mprintf("Fail to stop iperf upload test ... PE return 0x%x \n", iperf_result.ret_val);
		else
		{
			uint64 avg_speed_Mbps, duration_milliseconds=iperf_result.cost_time_ms;
			
			diag_util_mprintf("========== Result of iperf download test ========== \n"); 
			diag_util_mprintf("Duration(cost_time_ms) %llu ms \n", duration_milliseconds);
			diag_util_mprintf("================ Test statistic =============== \n");
			diag_util_mprintf("PktCnt %u \n", iperf_result.pktCnt);
			diag_util_mprintf("ByteCnt(payload only) %llu \n", iperf_result.byteCnt);
			diag_util_mprintf("ByteCnt(whole packet) %llu \n", iperf_result.byteCnt_include_pktHdr);
			avg_speed_Mbps = (iperf_result.byteCnt*8*1000/duration_milliseconds)/1000000;
			diag_util_mprintf("Average speed(payload only) %u.%03u Gpbs \n", (uint32)(avg_speed_Mbps/1000), (uint32)(avg_speed_Mbps%1000));
			avg_speed_Mbps = (iperf_result.byteCnt_include_pktHdr*8*1000/duration_milliseconds)/1000000;
			diag_util_mprintf("Average speed(whole packet) %u.%03u Gpbs \n", (uint32)(avg_speed_Mbps/1000), (uint32)(avg_speed_Mbps%1000));
		}
	}	

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_pe_iperf_test_upload_stop */

/*
 * rt_pe iperf_test upload start connection_number <UINT:connection_number> client_mac <MACADDR:client_mac> server_mac <MACADDR:server_mac> IPv4 client_ip <IPV4ADDR:client_ip> server_ip <IPV4ADDR:server_ip> UDP client_l4port <UINT:client_l4port> server_l4port <UINT:server_l4port> ldpid <UINT:ldpid> pppoe_sid <INT:pppoe_sid> svlan_vid <INT:svlan_vid> cvlan_vid <INT:cvlan_vid> pon_streamId <INT:pon_streamId> upload_payload_length <UINT:upload_payload_length> upload_first_payload_length <UINT:upload_first_payload_length> upload_first_payload <STRING:upload_first_payload>
 */
cparser_result_t
cparser_cmd_rt_pe_iperf_test_upload_start_connection_number_connection_number_client_mac_client_mac_server_mac_server_mac_IPv4_client_ip_client_ip_server_ip_server_ip_UDP_client_l4port_client_l4port_server_l4port_server_l4port_ldpid_ldpid_pppoe_sid_pppoe_sid_svlan_vid_svlan_vid_cvlan_vid_cvlan_vid_pon_streamId_pon_streamId_upload_payload_length_upload_payload_length_upload_first_payload_length_upload_first_payload_length_upload_first_payload_upload_first_payload(
	cparser_context_t *context,
	uint32_t  *connection_number_ptr,
	cparser_macaddr_t  *client_mac_ptr,
	cparser_macaddr_t  *server_mac_ptr,
	uint32_t  *client_ip_ptr,
	uint32_t  *server_ip_ptr,
	uint32_t  *client_l4port_ptr,
	uint32_t  *server_l4port_ptr,
	uint32_t  *ldpid_ptr,
	int32_t  *pppoe_sid_ptr,
	int32_t  *svlan_vid_ptr,
	int32_t  *cvlan_vid_ptr,
	int32_t  *pon_streamId_ptr,
	uint32_t  *upload_payload_length,
	uint32_t  *upload_first_payload_length,
	char * *upload_first_payload)
{
	int32 ret = RT_ERR_FAILED;
	rt_pe_iperf_request_t iperf_req;
	rt_pe_iperf_result_t iperf_result;
	
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&iperf_req, 0x0, sizeof(rt_pe_iperf_request_t));
	iperf_req.req_cmd = RT_PE_IPERF_REQ_CMD_UPLOAD_START_UDP;
	iperf_req.connection_number = *connection_number_ptr;
	osal_memcpy(iperf_req.client_mac.octet, client_mac_ptr->octet, ETHER_ADDR_LEN);
	osal_memcpy(iperf_req.server_mac.octet, server_mac_ptr->octet, ETHER_ADDR_LEN);	
	iperf_req.isIPv4OrIpv6 = 0;
	iperf_req.client_ip.ipv4_addr = (rtk_ip_addr_t)*client_ip_ptr;
	iperf_req.server_ip.ipv4_addr = (rtk_ip_addr_t)*server_ip_ptr;
	iperf_req.client_l4port[0] = *client_l4port_ptr;
	iperf_req.server_l4port[0] = *server_l4port_ptr;
	iperf_req.ldpid = *ldpid_ptr;
	iperf_req.pppoe_sid = *pppoe_sid_ptr;
	iperf_req.svlan_vid = *svlan_vid_ptr;
	iperf_req.cvlan_vid = *cvlan_vid_ptr;
	iperf_req.pon_streamId = *pon_streamId_ptr;
	iperf_req.udp_upload_payload_length = *upload_payload_length;
	iperf_req.udp_upload_first_payload_length = *upload_first_payload_length;
	osal_memcpy(iperf_req.udp_upload_first_payload, *upload_first_payload, MAX_PE_IPERF_FIRST_PAYLOAD_LENGTH);

	ret = rt_pe_iperf_test(iperf_req, &iperf_result);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to start iperf upload test ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to start iperf upload test ... RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		if(iperf_result.ret_val != RT_PE_RET_OK)
			diag_util_mprintf("Fail to start iperf upload test ... PE return 0x%x \n", iperf_result.ret_val);
		else
		{
			diag_util_mprintf("========== Success to start iperf udp upload test ==========\n");
			diag_util_mprintf("connection_number %u \n", iperf_req.connection_number);
			diag_util_mprintf("client_mac %02x:%02x:%02x:%02x:%02x:%02x \n", iperf_req.client_mac.octet[0], iperf_req.client_mac.octet[1], iperf_req.client_mac.octet[2], iperf_req.client_mac.octet[3], iperf_req.client_mac.octet[4], iperf_req.client_mac.octet[5]);
			diag_util_mprintf("server_mac %02x:%02x:%02x:%02x:%02x:%02x \n", iperf_req.server_mac.octet[0], iperf_req.server_mac.octet[1], iperf_req.server_mac.octet[2], iperf_req.server_mac.octet[3], iperf_req.server_mac.octet[4], iperf_req.server_mac.octet[5]);
			diag_util_mprintf("client_ipv4 %u.%u.%u.%u \n", (iperf_req.client_ip.ipv4_addr>>24)&0xff, (iperf_req.client_ip.ipv4_addr>>16)&0xff, (iperf_req.client_ip.ipv4_addr>>8)&0xff, (iperf_req.client_ip.ipv4_addr)&0xff);
			diag_util_mprintf("server_ipv4 %u.%u.%u.%u \n", (iperf_req.server_ip.ipv4_addr>>24)&0xff, (iperf_req.server_ip.ipv4_addr>>16)&0xff, (iperf_req.server_ip.ipv4_addr>>8)&0xff, (iperf_req.server_ip.ipv4_addr)&0xff);
			diag_util_mprintf("client_l4port %u(0x%x) \n", iperf_req.client_l4port, iperf_req.client_l4port);
			diag_util_mprintf("server_l4port %u(0x%x) \n", iperf_req.server_l4port, iperf_req.server_l4port);
			diag_util_mprintf("ldpid %u\n", iperf_req.ldpid);
			diag_util_mprintf("pppoe_sid %d svlan_vid %d cvlan_vid %d \n", iperf_req.pppoe_sid, iperf_req.svlan_vid, iperf_req.cvlan_vid);
			diag_util_mprintf("pon_streamId %d \n", iperf_req.pon_streamId);
			diag_util_mprintf("upload_payload_length %u\n", iperf_req.udp_upload_payload_length);
			diag_util_mprintf("upload_first_payload_length %u\n", iperf_req.udp_upload_first_payload_length);
			diag_util_mprintf("upload_first_payload %s \n", iperf_req.udp_upload_first_payload);
		}
	}	

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_pe_iperf_test_upload_start_connection_number_connection_number_client_mac_client_mac_server_mac_server_mac_IPv4_client_ip_client_ip_server_ip_server_ip_UDP_client_l4port_client_l4port_server_l4port_server_l4port_ldpid_ldpid_pppoe_sid_pppoe_sid_svlan_vid_svlan_vid_cvlan_vid_cvlan_vid_pon_streamId_pon_streamId_upload_payload_length_upload_payload_length_upload_first_payload_length_upload_first_payload_length_upload_first_payload_upload_first_payload */

/*
 * rt_pe iperf_test upload start connection_number <UINT:connection_number> client_mac <MACADDR:client_mac> server_mac <MACADDR:server_mac> IPv6 client_ip <IPV6ADDR:client_ip> server_ip <IPV6ADDR:server_ip> UDP client_l4port <UINT:client_l4port> server_l4port <UINT:server_l4port> ldpid <UINT:ldpid> pppoe_sid <INT:pppoe_sid> svlan_vid <INT:svlan_vid> cvlan_vid <INT:cvlan_vid> pon_streamId <INT:pon_streamId> upload_payload_length <UINT:upload_payload_length> upload_first_payload_length <UINT:upload_first_payload_length> upload_first_payload <STRING:upload_first_payload>
 */
cparser_result_t
cparser_cmd_rt_pe_iperf_test_upload_start_connection_number_connection_number_client_mac_client_mac_server_mac_server_mac_IPv6_client_ip_client_ip_server_ip_server_ip_UDP_client_l4port_client_l4port_server_l4port_server_l4port_ldpid_ldpid_pppoe_sid_pppoe_sid_svlan_vid_svlan_vid_cvlan_vid_cvlan_vid_pon_streamId_pon_streamId_upload_payload_length_upload_payload_length_upload_first_payload_length_upload_first_payload_length_upload_first_payload_upload_first_payload(
	cparser_context_t *context,
	uint32_t  *connection_number_ptr,
	cparser_macaddr_t  *client_mac_ptr,
	cparser_macaddr_t  *server_mac_ptr,
	char * *client_ip_ptr,
	char * *server_ip_ptr,
	uint32_t  *client_l4port_ptr,
	uint32_t  *server_l4port_ptr,
	uint32_t  *ldpid_ptr,
	int32_t  *pppoe_sid_ptr,
	int32_t  *svlan_vid_ptr,
	int32_t  *cvlan_vid_ptr,
	int32_t  *pon_streamId_ptr,
	uint32_t  *upload_payload_length,
	uint32_t  *upload_first_payload_length,
	char * *upload_first_payload)
{
	int32 ret = RT_ERR_FAILED;
	rt_pe_iperf_request_t iperf_req;
	rt_pe_iperf_result_t iperf_result;
	
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&iperf_req, 0x0, sizeof(rt_pe_iperf_request_t));
	iperf_req.req_cmd = RT_PE_IPERF_REQ_CMD_UPLOAD_START_UDP;
	iperf_req.connection_number = *connection_number_ptr;
	osal_memcpy(iperf_req.client_mac.octet, client_mac_ptr->octet, ETHER_ADDR_LEN);
	osal_memcpy(iperf_req.server_mac.octet, server_mac_ptr->octet, ETHER_ADDR_LEN);	
	iperf_req.isIPv4OrIpv6 = 1;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(iperf_req.client_ip.ipv6_addr.ipv6_addr, TOKEN_STR(12)), ret);
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(iperf_req.server_ip.ipv6_addr.ipv6_addr, TOKEN_STR(14)), ret);
	iperf_req.client_l4port[0] = *client_l4port_ptr;
	iperf_req.server_l4port[0] = *server_l4port_ptr;
	iperf_req.ldpid = *ldpid_ptr;
	iperf_req.pppoe_sid = *pppoe_sid_ptr;
	iperf_req.svlan_vid = *svlan_vid_ptr;
	iperf_req.cvlan_vid = *cvlan_vid_ptr;
	iperf_req.pon_streamId = *pon_streamId_ptr;
	iperf_req.udp_upload_payload_length = *upload_payload_length;
	iperf_req.udp_upload_first_payload_length = *upload_first_payload_length;
	osal_memcpy(iperf_req.udp_upload_first_payload, *upload_first_payload, MAX_PE_IPERF_FIRST_PAYLOAD_LENGTH);

	ret = rt_pe_iperf_test(iperf_req, &iperf_result);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to start iperf upload test ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to start iperf upload test ... RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		if(iperf_result.ret_val != RT_PE_RET_OK)
			diag_util_mprintf("Fail to start iperf upload test ... PE return 0x%x \n", iperf_result.ret_val);
		else
		{
			diag_util_mprintf("========== Success to start iperf udp upload test ==========\n");
			diag_util_mprintf("connection_number %u \n", iperf_req.connection_number);
			diag_util_mprintf("client_mac %02x:%02x:%02x:%02x:%02x:%02x \n", iperf_req.client_mac.octet[0], iperf_req.client_mac.octet[1], iperf_req.client_mac.octet[2], iperf_req.client_mac.octet[3], iperf_req.client_mac.octet[4], iperf_req.client_mac.octet[5]);
			diag_util_mprintf("server_mac %02x:%02x:%02x:%02x:%02x:%02x \n", iperf_req.server_mac.octet[0], iperf_req.server_mac.octet[1], iperf_req.server_mac.octet[2], iperf_req.server_mac.octet[3], iperf_req.server_mac.octet[4], iperf_req.server_mac.octet[5]);
			diag_util_mprintf("client_ipv6 %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n", 
								iperf_req.client_ip.ipv6_addr.ipv6_addr[0], iperf_req.client_ip.ipv6_addr.ipv6_addr[1], iperf_req.client_ip.ipv6_addr.ipv6_addr[2], iperf_req.client_ip.ipv6_addr.ipv6_addr[3],
								iperf_req.client_ip.ipv6_addr.ipv6_addr[4], iperf_req.client_ip.ipv6_addr.ipv6_addr[5], iperf_req.client_ip.ipv6_addr.ipv6_addr[6], iperf_req.client_ip.ipv6_addr.ipv6_addr[7],
								iperf_req.client_ip.ipv6_addr.ipv6_addr[8], iperf_req.client_ip.ipv6_addr.ipv6_addr[9], iperf_req.client_ip.ipv6_addr.ipv6_addr[10], iperf_req.client_ip.ipv6_addr.ipv6_addr[11],
								iperf_req.client_ip.ipv6_addr.ipv6_addr[12], iperf_req.client_ip.ipv6_addr.ipv6_addr[13], iperf_req.client_ip.ipv6_addr.ipv6_addr[14], iperf_req.client_ip.ipv6_addr.ipv6_addr[15]);
			diag_util_mprintf("server_ipv6 %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n", 
								iperf_req.server_ip.ipv6_addr.ipv6_addr[0], iperf_req.server_ip.ipv6_addr.ipv6_addr[1], iperf_req.server_ip.ipv6_addr.ipv6_addr[2], iperf_req.server_ip.ipv6_addr.ipv6_addr[3],
								iperf_req.server_ip.ipv6_addr.ipv6_addr[4], iperf_req.server_ip.ipv6_addr.ipv6_addr[5], iperf_req.server_ip.ipv6_addr.ipv6_addr[6], iperf_req.server_ip.ipv6_addr.ipv6_addr[7],
								iperf_req.server_ip.ipv6_addr.ipv6_addr[8], iperf_req.server_ip.ipv6_addr.ipv6_addr[9], iperf_req.server_ip.ipv6_addr.ipv6_addr[10], iperf_req.server_ip.ipv6_addr.ipv6_addr[11],
								iperf_req.server_ip.ipv6_addr.ipv6_addr[12], iperf_req.server_ip.ipv6_addr.ipv6_addr[13], iperf_req.server_ip.ipv6_addr.ipv6_addr[14], iperf_req.server_ip.ipv6_addr.ipv6_addr[15]);
			diag_util_mprintf("client_l4port %u(0x%x) \n", iperf_req.client_l4port, iperf_req.client_l4port);
			diag_util_mprintf("server_l4port %u(0x%x) \n", iperf_req.server_l4port, iperf_req.server_l4port);
			diag_util_mprintf("ldpid %u\n", iperf_req.ldpid);
			diag_util_mprintf("pppoe_sid %d svlan_vid %d cvlan_vid %d \n", iperf_req.pppoe_sid, iperf_req.svlan_vid, iperf_req.cvlan_vid);
			diag_util_mprintf("pon_streamId %d \n", iperf_req.pon_streamId);
			diag_util_mprintf("upload_payload_length %u\n", iperf_req.udp_upload_payload_length);
			diag_util_mprintf("upload_first_payload_length %u\n", iperf_req.udp_upload_first_payload_length);
			diag_util_mprintf("upload_first_payload %s \n", iperf_req.udp_upload_first_payload);
		}
	}	

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_pe_iperf_test_upload_start_connection_number_connection_number_client_mac_client_mac_server_mac_server_mac_IPv6_client_ip_client_ip_server_ip_server_ip_UDP_client_l4port_client_l4port_server_l4port_server_l4port_ldpid_ldpid_pppoe_sid_pppoe_sid_svlan_vid_svlan_vid_cvlan_vid_cvlan_vid_pon_streamId_pon_streamId_upload_payload_length_upload_payload_length_upload_first_payload_length_upload_first_payload_length_upload_first_payload_upload_first_payload */

/*
 * rt_pe generic_fwd_test download get_counter
 */
cparser_result_t
cparser_cmd_rt_pe_generic_fwd_test_download_get_counter(
	cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED;
	rt_pe_generic_fwd_request_t generic_fwd_req;
	rt_pe_generic_fwd_result_t generic_fwd_result;
	
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&generic_fwd_req, 0x0, sizeof(rt_pe_generic_fwd_request_t));
	generic_fwd_req.req_cmd = RT_PE_GENERIC_FWD_REQ_CMD_DOWNLOAD_GET_CNT;

	ret = rt_pe_generic_fwd_test(generic_fwd_req, &generic_fwd_result);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to get counter of generic_fwd download test ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to get counter of generic_fwd download test ... RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		if(generic_fwd_result.ret_val != RT_PE_RET_OK)
			diag_util_mprintf("Fail to get counter of generic_fwd download test ... PE return 0x%x \n", generic_fwd_result.ret_val);
		else
		{
			uint64 avg_speed_Mbps, duration_milliseconds=generic_fwd_result.cost_time_ms;
			
			diag_util_mprintf("========== Result of generic_fwd download test ========== \n"); 
			diag_util_mprintf("Duration(cost_time_ms) %llu ms \n", duration_milliseconds);
			diag_util_mprintf("================ Test statistic =============== \n");
			diag_util_mprintf("PktCnt %u \n", generic_fwd_result.pktCnt);
			diag_util_mprintf("ByteCnt(payload only) %llu \n", generic_fwd_result.byteCnt);
			diag_util_mprintf("ByteCnt(whole packet) %llu \n", generic_fwd_result.byteCnt_include_pktHdr);
			avg_speed_Mbps = (generic_fwd_result.byteCnt*8*1000/duration_milliseconds)/1000000;
			diag_util_mprintf("Average speed(payload only) %u.%03u Gpbs \n", (uint32)(avg_speed_Mbps/1000), (uint32)(avg_speed_Mbps%1000));
			avg_speed_Mbps = (generic_fwd_result.byteCnt_include_pktHdr*8*1000/duration_milliseconds)/1000000;
			diag_util_mprintf("Average speed(whole packet) %u.%03u Gpbs \n", (uint32)(avg_speed_Mbps/1000), (uint32)(avg_speed_Mbps%1000));
		}
	}	

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_pe_generic_fwd_test_download_get_counter */

/*
 * rt_pe generic_fwd_test download stop 
 */
cparser_result_t
cparser_cmd_rt_pe_generic_fwd_test_download_stop(
	cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED;
	rt_pe_generic_fwd_request_t generic_fwd_req;
	rt_pe_generic_fwd_result_t generic_fwd_result;
	
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&generic_fwd_req, 0x0, sizeof(rt_pe_generic_fwd_request_t));
	generic_fwd_req.req_cmd = RT_PE_GENERIC_FWD_REQ_CMD_DOWNLOAD_STOP;

	ret = rt_pe_generic_fwd_test(generic_fwd_req, &generic_fwd_result);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to stop generic_fwd download test ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to stop generic_fwd download test ... RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		if(generic_fwd_result.ret_val != RT_PE_RET_OK)
			diag_util_mprintf("Fail to stop generic_fwd download test ... PE return 0x%x \n", generic_fwd_result.ret_val);
		else
		{
			uint64 avg_speed_Mbps, duration_milliseconds=generic_fwd_result.cost_time_ms;
			
			diag_util_mprintf("========== Result of generic_fwd download test ========== \n"); 
			diag_util_mprintf("Duration(cost_time_ms) %llu ms \n", duration_milliseconds);
			diag_util_mprintf("================ Test statistic =============== \n");
			diag_util_mprintf("PktCnt %u \n", generic_fwd_result.pktCnt);
			diag_util_mprintf("ByteCnt(payload only) %llu \n", generic_fwd_result.byteCnt);
			diag_util_mprintf("ByteCnt(whole packet) %llu \n", generic_fwd_result.byteCnt_include_pktHdr);
			avg_speed_Mbps = (generic_fwd_result.byteCnt*8*1000/duration_milliseconds)/1000000;
			diag_util_mprintf("Average speed(payload only) %u.%03u Gpbs \n", (uint32)(avg_speed_Mbps/1000), (uint32)(avg_speed_Mbps%1000));
			avg_speed_Mbps = (generic_fwd_result.byteCnt_include_pktHdr*8*1000/duration_milliseconds)/1000000;
			diag_util_mprintf("Average speed(whole packet) %u.%03u Gpbs \n", (uint32)(avg_speed_Mbps/1000), (uint32)(avg_speed_Mbps%1000));
		}
	}	

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_pe_generic_fwd_test_download_stop */

/*
 * rt_pe generic_fwd_test download start connection_number <UINT:connection_number> client_mac <MACADDR:client_mac> server_mac <MACADDR:server_mac> IPv4 client_ip <IPV4ADDR:client_ip> server_ip <IPV4ADDR:server_ip> TCP client_l4port <UINT:client_l4port> server_l4port <UINT:server_l4port> congestion_mode <UINT:congestion_mode>
 */
cparser_result_t
cparser_cmd_rt_pe_generic_fwd_test_download_start_connection_number_connection_number_client_mac_client_mac_server_mac_server_mac_IPv4_client_ip_client_ip_server_ip_server_ip_TCP_client_l4port_client_l4port_server_l4port_server_l4port_congestion_mode_congestion_mode(
	cparser_context_t *context,
	uint32_t  *connection_number_ptr,
	cparser_macaddr_t  *client_mac_ptr,
	cparser_macaddr_t  *server_mac_ptr,
	uint32_t  *client_ip_ptr,
	uint32_t  *server_ip_ptr,
	uint32_t  *client_l4port_ptr,
	uint32_t  *server_l4port_ptr,
	uint32_t  *congestion_mode_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_pe_generic_fwd_request_t generic_fwd_req;
	rt_pe_generic_fwd_result_t generic_fwd_result;
	
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&generic_fwd_req, 0x0, sizeof(rt_pe_generic_fwd_request_t));
	generic_fwd_req.req_cmd = RT_PE_GENERIC_FWD_REQ_CMD_DOWNLOAD_START;
	generic_fwd_req.connection_number = *connection_number_ptr;
	osal_memcpy(generic_fwd_req.pkt_pattern.client_mac.octet, client_mac_ptr->octet, ETHER_ADDR_LEN);
	osal_memcpy(generic_fwd_req.pkt_pattern.server_mac.octet, server_mac_ptr->octet, ETHER_ADDR_LEN); 
	generic_fwd_req.pkt_pattern.isIPv4OrIpv6 = 0;
	generic_fwd_req.pkt_pattern.client_ip.ipv4_addr = (rtk_ip_addr_t)*client_ip_ptr;
	generic_fwd_req.pkt_pattern.server_ip.ipv4_addr = (rtk_ip_addr_t)*server_ip_ptr;
	generic_fwd_req.pkt_pattern.l4_proto = RT_PE_GENERIC_FWD_L4_PROTO_TCP;
	generic_fwd_req.pkt_pattern.client_l4port = *client_l4port_ptr;
	generic_fwd_req.pkt_pattern.server_l4port = *server_l4port_ptr;
	generic_fwd_req.tcp_congestion_mode = *congestion_mode_ptr;
	
	ret = rt_pe_generic_fwd_test(generic_fwd_req, &generic_fwd_result);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to start generic_fwd download test ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to start generic_fwd download test ... RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		if(generic_fwd_result.ret_val != RT_PE_RET_OK)
			diag_util_mprintf("Fail to start generic_fwd download test ... PE return 0x%x \n", generic_fwd_result.ret_val);
		else
		{
			diag_util_mprintf("========== Success to start generic_fwd TCP download test ==========\n");
			diag_util_mprintf("connection_number %u \n", generic_fwd_req.connection_number);
			diag_util_mprintf("client_mac %02x:%02x:%02x:%02x:%02x:%02x \n", generic_fwd_req.pkt_pattern.client_mac.octet[0], generic_fwd_req.pkt_pattern.client_mac.octet[1], generic_fwd_req.pkt_pattern.client_mac.octet[2], generic_fwd_req.pkt_pattern.client_mac.octet[3], generic_fwd_req.pkt_pattern.client_mac.octet[4], generic_fwd_req.pkt_pattern.client_mac.octet[5]);
			diag_util_mprintf("server_mac %02x:%02x:%02x:%02x:%02x:%02x \n", generic_fwd_req.pkt_pattern.server_mac.octet[0], generic_fwd_req.pkt_pattern.server_mac.octet[1], generic_fwd_req.pkt_pattern.server_mac.octet[2], generic_fwd_req.pkt_pattern.server_mac.octet[3], generic_fwd_req.pkt_pattern.server_mac.octet[4], generic_fwd_req.pkt_pattern.server_mac.octet[5]);
			diag_util_mprintf("client_ipv4 %u.%u.%u.%u \n", (generic_fwd_req.pkt_pattern.client_ip.ipv4_addr>>24)&0xff, (generic_fwd_req.pkt_pattern.client_ip.ipv4_addr>>16)&0xff, (generic_fwd_req.pkt_pattern.client_ip.ipv4_addr>>8)&0xff, (generic_fwd_req.pkt_pattern.client_ip.ipv4_addr)&0xff);
			diag_util_mprintf("server_ipv4 %u.%u.%u.%u \n", (generic_fwd_req.pkt_pattern.server_ip.ipv4_addr>>24)&0xff, (generic_fwd_req.pkt_pattern.server_ip.ipv4_addr>>16)&0xff, (generic_fwd_req.pkt_pattern.server_ip.ipv4_addr>>8)&0xff, (generic_fwd_req.pkt_pattern.server_ip.ipv4_addr)&0xff);
			diag_util_mprintf("l4_proto %u\n", generic_fwd_req.pkt_pattern.l4_proto);
			diag_util_mprintf("client_l4port %u(0x%x) \n", generic_fwd_req.pkt_pattern.client_l4port, generic_fwd_req.pkt_pattern.client_l4port);
			diag_util_mprintf("server_l4port %u(0x%x) \n", generic_fwd_req.pkt_pattern.server_l4port, generic_fwd_req.pkt_pattern.server_l4port);
			diag_util_mprintf("tcp_congestion_mode %u\n", generic_fwd_req.tcp_congestion_mode);
		}
	}	

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_pe_generic_fwd_test_download_start_connection_number_connection_number_client_mac_client_mac_server_mac_server_mac_IPv4_client_ip_client_ip_server_ip_server_ip_TCP_client_l4port_client_l4port_server_l4port_server_l4port_congestion_mode_congestion_mode */

/*
 * rt_pe generic_fwd_test download start connection_number <UINT:connection_number> client_mac <MACADDR:client_mac> server_mac <MACADDR:server_mac> IPv6 client_ip <IPV6ADDR:client_ip> server_ip <IPV6ADDR:server_ip> TCP client_l4port <UINT:client_l4port> server_l4port <UINT:server_l4port> congestion_mode <UINT:congestion_mode>
 */
cparser_result_t
cparser_cmd_rt_pe_generic_fwd_test_download_start_connection_number_connection_number_client_mac_client_mac_server_mac_server_mac_IPv6_client_ip_client_ip_server_ip_server_ip_TCP_client_l4port_client_l4port_server_l4port_server_l4port_congestion_mode_congestion_mode(
	cparser_context_t *context,
	uint32_t  *connection_number_ptr,
	cparser_macaddr_t  *client_mac_ptr,
	cparser_macaddr_t  *server_mac_ptr,
	char * *client_ip_ptr,
	char * *server_ip_ptr,
	uint32_t  *client_l4port_ptr,
	uint32_t  *server_l4port_ptr,
	uint32_t  *congestion_mode_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_pe_generic_fwd_request_t generic_fwd_req;
	rt_pe_generic_fwd_result_t generic_fwd_result;
	
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&generic_fwd_req, 0x0, sizeof(rt_pe_generic_fwd_request_t));
	generic_fwd_req.req_cmd = RT_PE_GENERIC_FWD_REQ_CMD_DOWNLOAD_START;
	generic_fwd_req.connection_number = *connection_number_ptr;
	osal_memcpy(generic_fwd_req.pkt_pattern.client_mac.octet, client_mac_ptr->octet, ETHER_ADDR_LEN);
	osal_memcpy(generic_fwd_req.pkt_pattern.server_mac.octet, server_mac_ptr->octet, ETHER_ADDR_LEN); 
	generic_fwd_req.pkt_pattern.isIPv4OrIpv6 = 1;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr, TOKEN_STR(12)), ret);
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr, TOKEN_STR(14)), ret);
	generic_fwd_req.pkt_pattern.l4_proto = RT_PE_GENERIC_FWD_L4_PROTO_TCP;
	generic_fwd_req.pkt_pattern.client_l4port = *client_l4port_ptr;
	generic_fwd_req.pkt_pattern.server_l4port = *server_l4port_ptr;
	generic_fwd_req.tcp_congestion_mode = *congestion_mode_ptr;

	ret = rt_pe_generic_fwd_test(generic_fwd_req, &generic_fwd_result);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to start generic_fwd download test ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to start generic_fwd download test ... RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		if(generic_fwd_result.ret_val != RT_PE_RET_OK)
			diag_util_mprintf("Fail to start generic_fwd download test ... PE return 0x%x \n", generic_fwd_result.ret_val);
		else
		{
			diag_util_mprintf("========== Success to start generic_fwd TCP download test ==========\n");
			diag_util_mprintf("connection_number %u \n", generic_fwd_req.connection_number);
			diag_util_mprintf("client_mac %02x:%02x:%02x:%02x:%02x:%02x \n", generic_fwd_req.pkt_pattern.client_mac.octet[0], generic_fwd_req.pkt_pattern.client_mac.octet[1], generic_fwd_req.pkt_pattern.client_mac.octet[2], generic_fwd_req.pkt_pattern.client_mac.octet[3], generic_fwd_req.pkt_pattern.client_mac.octet[4], generic_fwd_req.pkt_pattern.client_mac.octet[5]);
			diag_util_mprintf("server_mac %02x:%02x:%02x:%02x:%02x:%02x \n", generic_fwd_req.pkt_pattern.server_mac.octet[0], generic_fwd_req.pkt_pattern.server_mac.octet[1], generic_fwd_req.pkt_pattern.server_mac.octet[2], generic_fwd_req.pkt_pattern.server_mac.octet[3], generic_fwd_req.pkt_pattern.server_mac.octet[4], generic_fwd_req.pkt_pattern.server_mac.octet[5]);
			diag_util_mprintf("client_ipv6 %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n", 
								generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[0], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[1], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[2], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[3],
								generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[4], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[5], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[6], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[7],
								generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[8], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[9], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[10], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[11],
								generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[12], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[13], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[14], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[15]);
			diag_util_mprintf("server_ipv6 %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n", 
								generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[0], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[1], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[2], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[3],
								generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[4], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[5], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[6], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[7],
								generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[8], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[9], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[10], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[11],
								generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[12], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[13], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[14], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[15]);
			diag_util_mprintf("l4_proto %u\n", generic_fwd_req.pkt_pattern.l4_proto);
			diag_util_mprintf("client_l4port %u(0x%x) \n", generic_fwd_req.pkt_pattern.client_l4port, generic_fwd_req.pkt_pattern.client_l4port);
			diag_util_mprintf("server_l4port %u(0x%x) \n", generic_fwd_req.pkt_pattern.server_l4port, generic_fwd_req.pkt_pattern.server_l4port);
			diag_util_mprintf("tcp_congestion_mode %u\n", generic_fwd_req.tcp_congestion_mode);
		}	
	}	

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_pe_generic_fwd_test_download_start_connection_number_connection_number_client_mac_client_mac_server_mac_server_mac_IPv6_client_ip_client_ip_server_ip_server_ip_TCP_client_l4port_client_l4port_server_l4port_server_l4port_congestion_mode_congestion_mode */

/*
 * rt_pe generic_fwd_test download start connection_number <UINT:connection_number> client_mac <MACADDR:client_mac> server_mac <MACADDR:server_mac> IPv4 client_ip <IPV4ADDR:client_ip> server_ip <IPV4ADDR:server_ip> UDP client_l4port <UINT:client_l4port> server_l4port <UINT:server_l4port> test_time_sec <UINT:test_time_sec>
 */
cparser_result_t
cparser_cmd_rt_pe_generic_fwd_test_download_start_connection_number_connection_number_client_mac_client_mac_server_mac_server_mac_IPv4_client_ip_client_ip_server_ip_server_ip_UDP_client_l4port_client_l4port_server_l4port_server_l4port_test_time_sec_test_time_sec(
	cparser_context_t *context,
	uint32_t  *connection_number_ptr,
	cparser_macaddr_t  *client_mac_ptr,
	cparser_macaddr_t  *server_mac_ptr,
	uint32_t  *client_ip_ptr,
	uint32_t  *server_ip_ptr,
	uint32_t  *client_l4port_ptr,
	uint32_t  *server_l4port_ptr,
	uint32_t  *test_time_sec_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_pe_generic_fwd_request_t generic_fwd_req;
	rt_pe_generic_fwd_result_t generic_fwd_result;
	
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&generic_fwd_req, 0x0, sizeof(rt_pe_generic_fwd_request_t));
	generic_fwd_req.req_cmd = RT_PE_GENERIC_FWD_REQ_CMD_DOWNLOAD_START;
	generic_fwd_req.connection_number = *connection_number_ptr;
	osal_memcpy(generic_fwd_req.pkt_pattern.client_mac.octet, client_mac_ptr->octet, ETHER_ADDR_LEN);
	osal_memcpy(generic_fwd_req.pkt_pattern.server_mac.octet, server_mac_ptr->octet, ETHER_ADDR_LEN); 
	generic_fwd_req.pkt_pattern.isIPv4OrIpv6 = 0;
	generic_fwd_req.pkt_pattern.client_ip.ipv4_addr = (rtk_ip_addr_t)*client_ip_ptr;
	generic_fwd_req.pkt_pattern.server_ip.ipv4_addr = (rtk_ip_addr_t)*server_ip_ptr;
	generic_fwd_req.pkt_pattern.l4_proto = RT_PE_GENERIC_FWD_L4_PROTO_UDP;
	generic_fwd_req.pkt_pattern.client_l4port = *client_l4port_ptr;
	generic_fwd_req.pkt_pattern.server_l4port = *server_l4port_ptr;
	generic_fwd_req.udp_test_time_sec = *test_time_sec_ptr;
	
	ret = rt_pe_generic_fwd_test(generic_fwd_req, &generic_fwd_result);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to start generic_fwd download test ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to start generic_fwd download test ... RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		if(generic_fwd_result.ret_val != RT_PE_RET_OK)
			diag_util_mprintf("Fail to start generic_fwd download test ... PE return 0x%x \n", generic_fwd_result.ret_val);
		else
		{
			diag_util_mprintf("========== Success to start generic_fwd UDP download test ==========\n");
			diag_util_mprintf("connection_number %u \n", generic_fwd_req.connection_number);
			diag_util_mprintf("client_mac %02x:%02x:%02x:%02x:%02x:%02x \n", generic_fwd_req.pkt_pattern.client_mac.octet[0], generic_fwd_req.pkt_pattern.client_mac.octet[1], generic_fwd_req.pkt_pattern.client_mac.octet[2], generic_fwd_req.pkt_pattern.client_mac.octet[3], generic_fwd_req.pkt_pattern.client_mac.octet[4], generic_fwd_req.pkt_pattern.client_mac.octet[5]);
			diag_util_mprintf("server_mac %02x:%02x:%02x:%02x:%02x:%02x \n", generic_fwd_req.pkt_pattern.server_mac.octet[0], generic_fwd_req.pkt_pattern.server_mac.octet[1], generic_fwd_req.pkt_pattern.server_mac.octet[2], generic_fwd_req.pkt_pattern.server_mac.octet[3], generic_fwd_req.pkt_pattern.server_mac.octet[4], generic_fwd_req.pkt_pattern.server_mac.octet[5]);
			diag_util_mprintf("client_ipv4 %u.%u.%u.%u \n", (generic_fwd_req.pkt_pattern.client_ip.ipv4_addr>>24)&0xff, (generic_fwd_req.pkt_pattern.client_ip.ipv4_addr>>16)&0xff, (generic_fwd_req.pkt_pattern.client_ip.ipv4_addr>>8)&0xff, (generic_fwd_req.pkt_pattern.client_ip.ipv4_addr)&0xff);
			diag_util_mprintf("server_ipv4 %u.%u.%u.%u \n", (generic_fwd_req.pkt_pattern.server_ip.ipv4_addr>>24)&0xff, (generic_fwd_req.pkt_pattern.server_ip.ipv4_addr>>16)&0xff, (generic_fwd_req.pkt_pattern.server_ip.ipv4_addr>>8)&0xff, (generic_fwd_req.pkt_pattern.server_ip.ipv4_addr)&0xff);
			diag_util_mprintf("l4_proto %u\n", generic_fwd_req.pkt_pattern.l4_proto);
			diag_util_mprintf("client_l4port %u(0x%x) \n", generic_fwd_req.pkt_pattern.client_l4port, generic_fwd_req.pkt_pattern.client_l4port);
			diag_util_mprintf("server_l4port %u(0x%x) \n", generic_fwd_req.pkt_pattern.server_l4port, generic_fwd_req.pkt_pattern.server_l4port);
			diag_util_mprintf("udp_test_time_sec %u\n", generic_fwd_req.udp_test_time_sec);
		}
	}	

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_pe_generic_fwd_test_download_start_connection_number_connection_number_client_mac_client_mac_server_mac_server_mac_IPv4_client_ip_client_ip_server_ip_server_ip_UDP_client_l4port_client_l4port_server_l4port_server_l4port_test_time_sec_test_time_sec */

/*
 * rt_pe generic_fwd_test download start connection_number <UINT:connection_number> client_mac <MACADDR:client_mac> server_mac <MACADDR:server_mac> IPv6 client_ip <IPV6ADDR:client_ip> server_ip <IPV6ADDR:server_ip> UDP client_l4port <UINT:client_l4port> server_l4port <UINT:server_l4port> test_time_sec <UINT:test_time_sec>
 */
cparser_result_t
cparser_cmd_rt_pe_generic_fwd_test_download_start_connection_number_connection_number_client_mac_client_mac_server_mac_server_mac_IPv6_client_ip_client_ip_server_ip_server_ip_UDP_client_l4port_client_l4port_server_l4port_server_l4port_test_time_sec_test_time_sec(
	cparser_context_t *context,
	uint32_t  *connection_number_ptr,
	cparser_macaddr_t  *client_mac_ptr,
	cparser_macaddr_t  *server_mac_ptr,
	char * *client_ip_ptr,
	char * *server_ip_ptr,
	uint32_t  *client_l4port_ptr,
	uint32_t  *server_l4port_ptr,
	uint32_t  *test_time_sec_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_pe_generic_fwd_request_t generic_fwd_req;
	rt_pe_generic_fwd_result_t generic_fwd_result;
	
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&generic_fwd_req, 0x0, sizeof(rt_pe_generic_fwd_request_t));
	generic_fwd_req.req_cmd = RT_PE_GENERIC_FWD_REQ_CMD_DOWNLOAD_START;
	generic_fwd_req.connection_number = *connection_number_ptr;
	osal_memcpy(generic_fwd_req.pkt_pattern.client_mac.octet, client_mac_ptr->octet, ETHER_ADDR_LEN);
	osal_memcpy(generic_fwd_req.pkt_pattern.server_mac.octet, server_mac_ptr->octet, ETHER_ADDR_LEN); 
	generic_fwd_req.pkt_pattern.isIPv4OrIpv6 = 1;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr, TOKEN_STR(12)), ret);
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr, TOKEN_STR(14)), ret);
	generic_fwd_req.pkt_pattern.l4_proto = RT_PE_GENERIC_FWD_L4_PROTO_UDP;
	generic_fwd_req.pkt_pattern.client_l4port = *client_l4port_ptr;
	generic_fwd_req.pkt_pattern.server_l4port = *server_l4port_ptr;
	generic_fwd_req.udp_test_time_sec = *test_time_sec_ptr;

	ret = rt_pe_generic_fwd_test(generic_fwd_req, &generic_fwd_result);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to start generic_fwd download test ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to start generic_fwd download test ... RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		if(generic_fwd_result.ret_val != RT_PE_RET_OK)
			diag_util_mprintf("Fail to start generic_fwd download test ... PE return 0x%x \n", generic_fwd_result.ret_val);
		else
		{
			diag_util_mprintf("========== Success to start generic_fwd UDP download test ==========\n");
			diag_util_mprintf("connection_number %u \n", generic_fwd_req.connection_number);
			diag_util_mprintf("client_mac %02x:%02x:%02x:%02x:%02x:%02x \n", generic_fwd_req.pkt_pattern.client_mac.octet[0], generic_fwd_req.pkt_pattern.client_mac.octet[1], generic_fwd_req.pkt_pattern.client_mac.octet[2], generic_fwd_req.pkt_pattern.client_mac.octet[3], generic_fwd_req.pkt_pattern.client_mac.octet[4], generic_fwd_req.pkt_pattern.client_mac.octet[5]);
			diag_util_mprintf("server_mac %02x:%02x:%02x:%02x:%02x:%02x \n", generic_fwd_req.pkt_pattern.server_mac.octet[0], generic_fwd_req.pkt_pattern.server_mac.octet[1], generic_fwd_req.pkt_pattern.server_mac.octet[2], generic_fwd_req.pkt_pattern.server_mac.octet[3], generic_fwd_req.pkt_pattern.server_mac.octet[4], generic_fwd_req.pkt_pattern.server_mac.octet[5]);
			diag_util_mprintf("client_ipv6 %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n", 
								generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[0], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[1], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[2], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[3],
								generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[4], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[5], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[6], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[7],
								generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[8], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[9], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[10], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[11],
								generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[12], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[13], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[14], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[15]);
			diag_util_mprintf("server_ipv6 %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n", 
								generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[0], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[1], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[2], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[3],
								generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[4], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[5], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[6], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[7],
								generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[8], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[9], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[10], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[11],
								generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[12], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[13], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[14], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[15]);
			diag_util_mprintf("l4_proto %u\n", generic_fwd_req.pkt_pattern.l4_proto);
			diag_util_mprintf("client_l4port %u(0x%x) \n", generic_fwd_req.pkt_pattern.client_l4port, generic_fwd_req.pkt_pattern.client_l4port);
			diag_util_mprintf("server_l4port %u(0x%x) \n", generic_fwd_req.pkt_pattern.server_l4port, generic_fwd_req.pkt_pattern.server_l4port);
			diag_util_mprintf("udp_test_time_sec %u\n", generic_fwd_req.udp_test_time_sec);
		}
	}	

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_pe_generic_fwd_test_download_start_connection_number_connection_number_client_mac_client_mac_server_mac_server_mac_IPv6_client_ip_client_ip_server_ip_server_ip_UDP_client_l4port_client_l4port_server_l4port_server_l4port_test_time_sec_test_time_sec */

/*
 * rt_pe generic_fwd_test upload get_counter
 */
cparser_result_t
cparser_cmd_rt_pe_generic_fwd_test_upload_get_counter(
	cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED;
	rt_pe_generic_fwd_request_t generic_fwd_req;
	rt_pe_generic_fwd_result_t generic_fwd_result;
	
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&generic_fwd_req, 0x0, sizeof(rt_pe_generic_fwd_request_t));
	generic_fwd_req.req_cmd = RT_PE_GENERIC_FWD_REQ_CMD_UPLOAD_GET_CNT;

	ret = rt_pe_generic_fwd_test(generic_fwd_req, &generic_fwd_result);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to get counter of generic_fwd upload test ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to get counter of generic_fwd upload test ... RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		if(generic_fwd_result.ret_val != RT_PE_RET_OK)
			diag_util_mprintf("Fail to get counter of generic_fwd upload test ... PE return 0x%x \n", generic_fwd_result.ret_val);
		else
		{
			uint64 avg_speed_Mbps, duration_milliseconds=generic_fwd_result.cost_time_ms;
			
			diag_util_mprintf("========== Result of generic_fwd download test ========== \n"); 
			diag_util_mprintf("Duration(cost_time_ms) %llu ms \n", duration_milliseconds);
			diag_util_mprintf("================ Test statistic =============== \n");
			diag_util_mprintf("PktCnt %u \n", generic_fwd_result.pktCnt);
			diag_util_mprintf("ByteCnt(payload only) %llu \n", generic_fwd_result.byteCnt);
			diag_util_mprintf("ByteCnt(whole packet) %llu \n", generic_fwd_result.byteCnt_include_pktHdr);
			avg_speed_Mbps = (generic_fwd_result.byteCnt*8*1000/duration_milliseconds)/1000000;
			diag_util_mprintf("Average speed(payload only) %u.%03u Gpbs \n", (uint32)(avg_speed_Mbps/1000), (uint32)(avg_speed_Mbps%1000));
			avg_speed_Mbps = (generic_fwd_result.byteCnt_include_pktHdr*8*1000/duration_milliseconds)/1000000;
			diag_util_mprintf("Average speed(whole packet) %u.%03u Gpbs \n", (uint32)(avg_speed_Mbps/1000), (uint32)(avg_speed_Mbps%1000));
		}
	}	

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_pe_generic_fwd_test_upload_get_counter */

/*
 * rt_pe generic_fwd_test upload stop 
 */
cparser_result_t
cparser_cmd_rt_pe_generic_fwd_test_upload_stop(
	cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED;
	rt_pe_generic_fwd_request_t generic_fwd_req;
	rt_pe_generic_fwd_result_t generic_fwd_result;
	
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&generic_fwd_req, 0x0, sizeof(rt_pe_generic_fwd_request_t));
	generic_fwd_req.req_cmd = RT_PE_GENERIC_FWD_REQ_CMD_UPLOAD_STOP;

	ret = rt_pe_generic_fwd_test(generic_fwd_req, &generic_fwd_result);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to stop generic_fwd upload test ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to stop generic_fwd upload test ... RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		if(generic_fwd_result.ret_val != RT_PE_RET_OK)
			diag_util_mprintf("Fail to stop generic_fwd upload test ... PE return 0x%x \n", generic_fwd_result.ret_val);
		else
		{
			uint64 avg_speed_Mbps, duration_milliseconds=generic_fwd_result.cost_time_ms;
			
			diag_util_mprintf("========== Result of generic_fwd download test ========== \n"); 
			diag_util_mprintf("Duration(cost_time_ms) %llu ms \n", duration_milliseconds);
			diag_util_mprintf("================ Test statistic =============== \n");
			diag_util_mprintf("PktCnt %u \n", generic_fwd_result.pktCnt);
			diag_util_mprintf("ByteCnt(payload only) %llu \n", generic_fwd_result.byteCnt);
			diag_util_mprintf("ByteCnt(whole packet) %llu \n", generic_fwd_result.byteCnt_include_pktHdr);
			avg_speed_Mbps = (generic_fwd_result.byteCnt*8*1000/duration_milliseconds)/1000000;
			diag_util_mprintf("Average speed(payload only) %u.%03u Gpbs \n", (uint32)(avg_speed_Mbps/1000), (uint32)(avg_speed_Mbps%1000));
			avg_speed_Mbps = (generic_fwd_result.byteCnt_include_pktHdr*8*1000/duration_milliseconds)/1000000;
			diag_util_mprintf("Average speed(whole packet) %u.%03u Gpbs \n", (uint32)(avg_speed_Mbps/1000), (uint32)(avg_speed_Mbps%1000));
		}
	}	

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_pe_generic_fwd_test_upload_stop */

/*
 * rt_pe generic_fwd_test upload start connection_number <UINT:connection_number> client_mac <MACADDR:client_mac> server_mac <MACADDR:server_mac> IPv4 client_ip <IPV4ADDR:client_ip> server_ip <IPV4ADDR:server_ip> TCP client_l4port <UINT:client_l4port> server_l4port <UINT:server_l4port> upload_total_content_length <UINT64:upload_total_content_length>
 */
cparser_result_t
cparser_cmd_rt_pe_generic_fwd_test_upload_start_connection_number_connection_number_client_mac_client_mac_server_mac_server_mac_IPv4_client_ip_client_ip_server_ip_server_ip_TCP_client_l4port_client_l4port_server_l4port_server_l4port_upload_total_content_length_upload_total_content_length(
	cparser_context_t *context,
	uint32_t  *connection_number_ptr,
	cparser_macaddr_t  *client_mac_ptr,
	cparser_macaddr_t  *server_mac_ptr,
	uint32_t  *client_ip_ptr,
	uint32_t  *server_ip_ptr,
	uint32_t  *client_l4port_ptr,
	uint32_t  *server_l4port_ptr,
	uint64_t  *upload_total_content_length_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_pe_generic_fwd_request_t generic_fwd_req;
	rt_pe_generic_fwd_result_t generic_fwd_result;
	
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&generic_fwd_req, 0x0, sizeof(rt_pe_generic_fwd_request_t));
	generic_fwd_req.req_cmd = RT_PE_GENERIC_FWD_REQ_CMD_UPLOAD_START;
	generic_fwd_req.connection_number = *connection_number_ptr;
	osal_memcpy(generic_fwd_req.pkt_pattern.client_mac.octet, client_mac_ptr->octet, ETHER_ADDR_LEN);
	osal_memcpy(generic_fwd_req.pkt_pattern.server_mac.octet, server_mac_ptr->octet, ETHER_ADDR_LEN); 
	generic_fwd_req.pkt_pattern.isIPv4OrIpv6 = 0;
	generic_fwd_req.pkt_pattern.client_ip.ipv4_addr = (rtk_ip_addr_t)*client_ip_ptr;
	generic_fwd_req.pkt_pattern.server_ip.ipv4_addr = (rtk_ip_addr_t)*server_ip_ptr;
	generic_fwd_req.pkt_pattern.l4_proto = RT_PE_GENERIC_FWD_L4_PROTO_TCP;
	generic_fwd_req.pkt_pattern.client_l4port = *client_l4port_ptr;
	generic_fwd_req.pkt_pattern.server_l4port = *server_l4port_ptr;
	generic_fwd_req.tcp_upload_total_content_length = *upload_total_content_length_ptr;

	ret = rt_pe_generic_fwd_test(generic_fwd_req, &generic_fwd_result);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to start generic_fwd upload test ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to start generic_fwd upload test ... RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		if(generic_fwd_result.ret_val != RT_PE_RET_OK)
			diag_util_mprintf("Fail to start generic_fwd upload test ... PE return 0x%x \n", generic_fwd_result.ret_val);
		else
		{
			diag_util_mprintf("========== Success to start generic_fwd TCP upload test ==========\n");
			diag_util_mprintf("connection_number %u \n", generic_fwd_req.connection_number);
			diag_util_mprintf("client_mac %02x:%02x:%02x:%02x:%02x:%02x \n", generic_fwd_req.pkt_pattern.client_mac.octet[0], generic_fwd_req.pkt_pattern.client_mac.octet[1], generic_fwd_req.pkt_pattern.client_mac.octet[2], generic_fwd_req.pkt_pattern.client_mac.octet[3], generic_fwd_req.pkt_pattern.client_mac.octet[4], generic_fwd_req.pkt_pattern.client_mac.octet[5]);
			diag_util_mprintf("server_mac %02x:%02x:%02x:%02x:%02x:%02x \n", generic_fwd_req.pkt_pattern.server_mac.octet[0], generic_fwd_req.pkt_pattern.server_mac.octet[1], generic_fwd_req.pkt_pattern.server_mac.octet[2], generic_fwd_req.pkt_pattern.server_mac.octet[3], generic_fwd_req.pkt_pattern.server_mac.octet[4], generic_fwd_req.pkt_pattern.server_mac.octet[5]);
			diag_util_mprintf("client_ipv4 %u.%u.%u.%u \n", (generic_fwd_req.pkt_pattern.client_ip.ipv4_addr>>24)&0xff, (generic_fwd_req.pkt_pattern.client_ip.ipv4_addr>>16)&0xff, (generic_fwd_req.pkt_pattern.client_ip.ipv4_addr>>8)&0xff, (generic_fwd_req.pkt_pattern.client_ip.ipv4_addr)&0xff);
			diag_util_mprintf("server_ipv4 %u.%u.%u.%u \n", (generic_fwd_req.pkt_pattern.server_ip.ipv4_addr>>24)&0xff, (generic_fwd_req.pkt_pattern.server_ip.ipv4_addr>>16)&0xff, (generic_fwd_req.pkt_pattern.server_ip.ipv4_addr>>8)&0xff, (generic_fwd_req.pkt_pattern.server_ip.ipv4_addr)&0xff);
			diag_util_mprintf("l4_proto %u\n", generic_fwd_req.pkt_pattern.l4_proto);
			diag_util_mprintf("client_l4port %u(0x%x) \n", generic_fwd_req.pkt_pattern.client_l4port, generic_fwd_req.pkt_pattern.client_l4port);
			diag_util_mprintf("server_l4port %u(0x%x) \n", generic_fwd_req.pkt_pattern.server_l4port, generic_fwd_req.pkt_pattern.server_l4port);
			diag_util_mprintf("tcp_upload_total_content_length %llu\n", generic_fwd_req.tcp_upload_total_content_length);
		}
	}	

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_pe_generic_fwd_test_upload_start_connection_number_connection_number_client_mac_client_mac_server_mac_server_mac_IPv4_client_ip_client_ip_server_ip_server_ip_TCP_client_l4port_client_l4port_server_l4port_server_l4port_upload_total_content_length_upload_total_content_length */

/*
 * rt_pe generic_fwd_test upload start connection_number <UINT:connection_number> client_mac <MACADDR:client_mac> server_mac <MACADDR:server_mac> IPv6 client_ip <IPV6ADDR:client_ip> server_ip <IPV6ADDR:server_ip> TCP client_l4port <UINT:client_l4port> server_l4port <UINT:server_l4port> upload_total_content_length <UINT64:upload_total_content_length>
 */
cparser_result_t
cparser_cmd_rt_pe_generic_fwd_test_upload_start_connection_number_connection_number_client_mac_client_mac_server_mac_server_mac_IPv6_client_ip_client_ip_server_ip_server_ip_TCP_client_l4port_client_l4port_server_l4port_server_l4port_upload_total_content_length_upload_total_content_length(
	cparser_context_t *context,
	uint32_t  *connection_number_ptr,
	cparser_macaddr_t  *client_mac_ptr,
	cparser_macaddr_t  *server_mac_ptr,
	char * *client_ip_ptr,
	char * *server_ip_ptr,
	uint32_t  *client_l4port_ptr,
	uint32_t  *server_l4port_ptr,
	uint64_t  *upload_total_content_length_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_pe_generic_fwd_request_t generic_fwd_req;
	rt_pe_generic_fwd_result_t generic_fwd_result;
	
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&generic_fwd_req, 0x0, sizeof(rt_pe_generic_fwd_request_t));
	generic_fwd_req.req_cmd = RT_PE_GENERIC_FWD_REQ_CMD_UPLOAD_START;
	generic_fwd_req.connection_number = *connection_number_ptr;
	osal_memcpy(generic_fwd_req.pkt_pattern.client_mac.octet, client_mac_ptr->octet, ETHER_ADDR_LEN);
	osal_memcpy(generic_fwd_req.pkt_pattern.server_mac.octet, server_mac_ptr->octet, ETHER_ADDR_LEN); 
	generic_fwd_req.pkt_pattern.isIPv4OrIpv6 = 1;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr, TOKEN_STR(12)), ret);
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr, TOKEN_STR(14)), ret);
	generic_fwd_req.pkt_pattern.l4_proto = RT_PE_GENERIC_FWD_L4_PROTO_TCP;
	generic_fwd_req.pkt_pattern.client_l4port = *client_l4port_ptr;
	generic_fwd_req.pkt_pattern.server_l4port = *server_l4port_ptr;
	generic_fwd_req.tcp_upload_total_content_length = *upload_total_content_length_ptr;

	ret = rt_pe_generic_fwd_test(generic_fwd_req, &generic_fwd_result);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to start generic_fwd upload test ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to start generic_fwd upload test ... RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		if(generic_fwd_result.ret_val != RT_PE_RET_OK)
			diag_util_mprintf("Fail to start generic_fwd upload test ... PE return 0x%x \n", generic_fwd_result.ret_val);
		else
		{
			diag_util_mprintf("========== Success to start generic_fwd TCP upload test ==========\n");
			diag_util_mprintf("connection_number %u \n", generic_fwd_req.connection_number);
			diag_util_mprintf("client_mac %02x:%02x:%02x:%02x:%02x:%02x \n", generic_fwd_req.pkt_pattern.client_mac.octet[0], generic_fwd_req.pkt_pattern.client_mac.octet[1], generic_fwd_req.pkt_pattern.client_mac.octet[2], generic_fwd_req.pkt_pattern.client_mac.octet[3], generic_fwd_req.pkt_pattern.client_mac.octet[4], generic_fwd_req.pkt_pattern.client_mac.octet[5]);
			diag_util_mprintf("server_mac %02x:%02x:%02x:%02x:%02x:%02x \n", generic_fwd_req.pkt_pattern.server_mac.octet[0], generic_fwd_req.pkt_pattern.server_mac.octet[1], generic_fwd_req.pkt_pattern.server_mac.octet[2], generic_fwd_req.pkt_pattern.server_mac.octet[3], generic_fwd_req.pkt_pattern.server_mac.octet[4], generic_fwd_req.pkt_pattern.server_mac.octet[5]);
			diag_util_mprintf("client_ipv6 %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n", 
								generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[0], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[1], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[2], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[3],
								generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[4], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[5], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[6], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[7],
								generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[8], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[9], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[10], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[11],
								generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[12], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[13], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[14], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[15]);
			diag_util_mprintf("server_ipv6 %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n", 
								generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[0], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[1], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[2], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[3],
								generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[4], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[5], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[6], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[7],
								generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[8], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[9], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[10], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[11],
								generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[12], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[13], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[14], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[15]);
			diag_util_mprintf("l4_proto %u\n", generic_fwd_req.pkt_pattern.l4_proto);
			diag_util_mprintf("client_l4port %u(0x%x) \n", generic_fwd_req.pkt_pattern.client_l4port, generic_fwd_req.pkt_pattern.client_l4port);
			diag_util_mprintf("server_l4port %u(0x%x) \n", generic_fwd_req.pkt_pattern.server_l4port, generic_fwd_req.pkt_pattern.server_l4port);
			diag_util_mprintf("tcp_upload_total_content_length %llu\n", generic_fwd_req.tcp_upload_total_content_length);
		}
	}	

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_pe_generic_fwd_test_upload_start_connection_number_connection_number_client_mac_client_mac_server_mac_server_mac_IPv6_client_ip_client_ip_server_ip_server_ip_TCP_client_l4port_client_l4port_server_l4port_server_l4port_upload_total_content_length_upload_total_content_length */

/*
 * rt_pe generic_fwd_test upload start connection_number <UINT:connection_number> client_mac <MACADDR:client_mac> server_mac <MACADDR:server_mac> IPv4 client_ip <IPV4ADDR:client_ip> server_ip <IPV4ADDR:server_ip> UDP client_l4port <UINT:client_l4port> server_l4port <UINT:server_l4port> upload_payload_length <UINT:upload_payload_length> test_time_sec <UINT:test_time_sec>
 */
cparser_result_t
cparser_cmd_rt_pe_generic_fwd_test_upload_start_connection_number_connection_number_client_mac_client_mac_server_mac_server_mac_IPv4_client_ip_client_ip_server_ip_server_ip_UDP_client_l4port_client_l4port_server_l4port_server_l4port_upload_payload_length_upload_payload_length_test_time_sec_test_time_sec(
	cparser_context_t *context,
	uint32_t  *connection_number_ptr,
	cparser_macaddr_t  *client_mac_ptr,
	cparser_macaddr_t  *server_mac_ptr,
	uint32_t  *client_ip_ptr,
	uint32_t  *server_ip_ptr,
	uint32_t  *client_l4port_ptr,
	uint32_t  *server_l4port_ptr,
	uint32_t  *upload_payload_length_ptr,
	uint32_t  *test_time_sec_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_pe_generic_fwd_request_t generic_fwd_req;
	rt_pe_generic_fwd_result_t generic_fwd_result;
	
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&generic_fwd_req, 0x0, sizeof(rt_pe_generic_fwd_request_t));
	generic_fwd_req.req_cmd = RT_PE_GENERIC_FWD_REQ_CMD_UPLOAD_START;
	generic_fwd_req.connection_number = *connection_number_ptr;
	osal_memcpy(generic_fwd_req.pkt_pattern.client_mac.octet, client_mac_ptr->octet, ETHER_ADDR_LEN);
	osal_memcpy(generic_fwd_req.pkt_pattern.server_mac.octet, server_mac_ptr->octet, ETHER_ADDR_LEN); 
	generic_fwd_req.pkt_pattern.isIPv4OrIpv6 = 0;
	generic_fwd_req.pkt_pattern.client_ip.ipv4_addr = (rtk_ip_addr_t)*client_ip_ptr;
	generic_fwd_req.pkt_pattern.server_ip.ipv4_addr = (rtk_ip_addr_t)*server_ip_ptr;
	generic_fwd_req.pkt_pattern.l4_proto = RT_PE_GENERIC_FWD_L4_PROTO_UDP;
	generic_fwd_req.pkt_pattern.client_l4port = *client_l4port_ptr;
	generic_fwd_req.pkt_pattern.server_l4port = *server_l4port_ptr;
	generic_fwd_req.udp_upload_payload_length = *upload_payload_length_ptr;
	generic_fwd_req.udp_test_time_sec = *test_time_sec_ptr;

	ret = rt_pe_generic_fwd_test(generic_fwd_req, &generic_fwd_result);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to start generic_fwd upload test ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to start generic_fwd upload test ... RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		if(generic_fwd_result.ret_val != RT_PE_RET_OK)
			diag_util_mprintf("Fail to start generic_fwd upload test ... PE return 0x%x \n", generic_fwd_result.ret_val);
		else
		{
			diag_util_mprintf("========== Success to start generic_fwd UDP upload test ==========\n");
			diag_util_mprintf("connection_number %u \n", generic_fwd_req.connection_number);
			diag_util_mprintf("client_mac %02x:%02x:%02x:%02x:%02x:%02x \n", generic_fwd_req.pkt_pattern.client_mac.octet[0], generic_fwd_req.pkt_pattern.client_mac.octet[1], generic_fwd_req.pkt_pattern.client_mac.octet[2], generic_fwd_req.pkt_pattern.client_mac.octet[3], generic_fwd_req.pkt_pattern.client_mac.octet[4], generic_fwd_req.pkt_pattern.client_mac.octet[5]);
			diag_util_mprintf("server_mac %02x:%02x:%02x:%02x:%02x:%02x \n", generic_fwd_req.pkt_pattern.server_mac.octet[0], generic_fwd_req.pkt_pattern.server_mac.octet[1], generic_fwd_req.pkt_pattern.server_mac.octet[2], generic_fwd_req.pkt_pattern.server_mac.octet[3], generic_fwd_req.pkt_pattern.server_mac.octet[4], generic_fwd_req.pkt_pattern.server_mac.octet[5]);
			diag_util_mprintf("client_ipv4 %u.%u.%u.%u \n", (generic_fwd_req.pkt_pattern.client_ip.ipv4_addr>>24)&0xff, (generic_fwd_req.pkt_pattern.client_ip.ipv4_addr>>16)&0xff, (generic_fwd_req.pkt_pattern.client_ip.ipv4_addr>>8)&0xff, (generic_fwd_req.pkt_pattern.client_ip.ipv4_addr)&0xff);
			diag_util_mprintf("server_ipv4 %u.%u.%u.%u \n", (generic_fwd_req.pkt_pattern.server_ip.ipv4_addr>>24)&0xff, (generic_fwd_req.pkt_pattern.server_ip.ipv4_addr>>16)&0xff, (generic_fwd_req.pkt_pattern.server_ip.ipv4_addr>>8)&0xff, (generic_fwd_req.pkt_pattern.server_ip.ipv4_addr)&0xff);
			diag_util_mprintf("l4_proto %u\n", generic_fwd_req.pkt_pattern.l4_proto);
			diag_util_mprintf("client_l4port %u(0x%x) \n", generic_fwd_req.pkt_pattern.client_l4port, generic_fwd_req.pkt_pattern.client_l4port);
			diag_util_mprintf("server_l4port %u(0x%x) \n", generic_fwd_req.pkt_pattern.server_l4port, generic_fwd_req.pkt_pattern.server_l4port);
			diag_util_mprintf("udp_upload_payload_length %u\n", generic_fwd_req.udp_upload_payload_length);
			diag_util_mprintf("udp_test_time_sec %u\n", generic_fwd_req.udp_test_time_sec);
		}
	}	

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_pe_generic_fwd_test_upload_start_connection_number_connection_number_client_mac_client_mac_server_mac_server_mac_IPv4_client_ip_client_ip_server_ip_server_ip_UDP_client_l4port_client_l4port_server_l4port_server_l4port_upload_payload_length_upload_payload_length_test_time_sec_test_time_sec */

/*
 * rt_pe generic_fwd_test upload start connection_number <UINT:connection_number> client_mac <MACADDR:client_mac> server_mac <MACADDR:server_mac> IPv6 client_ip <IPV6ADDR:client_ip> server_ip <IPV6ADDR:server_ip> UDP client_l4port <UINT:client_l4port> server_l4port <UINT:server_l4port> upload_payload_length <UINT:upload_payload_length> test_time_sec <UINT:test_time_sec>
 */
cparser_result_t
cparser_cmd_rt_pe_generic_fwd_test_upload_start_connection_number_connection_number_client_mac_client_mac_server_mac_server_mac_IPv6_client_ip_client_ip_server_ip_server_ip_UDP_client_l4port_client_l4port_server_l4port_server_l4port_upload_payload_length_upload_payload_length_test_time_sec_test_time_sec(
	cparser_context_t *context,
	uint32_t  *connection_number_ptr,
	cparser_macaddr_t  *client_mac_ptr,
	cparser_macaddr_t  *server_mac_ptr,
	char * *client_ip_ptr,
	char * *server_ip_ptr,
	uint32_t  *client_l4port_ptr,
	uint32_t  *server_l4port_ptr,
	uint32_t  *upload_payload_length_ptr,
	uint32_t  *test_time_sec_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_pe_generic_fwd_request_t generic_fwd_req;
	rt_pe_generic_fwd_result_t generic_fwd_result;
	
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&generic_fwd_req, 0x0, sizeof(rt_pe_generic_fwd_request_t));
	generic_fwd_req.req_cmd = RT_PE_GENERIC_FWD_REQ_CMD_UPLOAD_START;
	generic_fwd_req.connection_number = *connection_number_ptr;
	osal_memcpy(generic_fwd_req.pkt_pattern.client_mac.octet, client_mac_ptr->octet, ETHER_ADDR_LEN);
	osal_memcpy(generic_fwd_req.pkt_pattern.server_mac.octet, server_mac_ptr->octet, ETHER_ADDR_LEN); 
	generic_fwd_req.pkt_pattern.isIPv4OrIpv6 = 1;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr, TOKEN_STR(12)), ret);
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr, TOKEN_STR(14)), ret);
	generic_fwd_req.pkt_pattern.l4_proto = RT_PE_GENERIC_FWD_L4_PROTO_UDP;
	generic_fwd_req.pkt_pattern.client_l4port = *client_l4port_ptr;
	generic_fwd_req.pkt_pattern.server_l4port = *server_l4port_ptr;
	generic_fwd_req.udp_upload_payload_length = *upload_payload_length_ptr;
	generic_fwd_req.udp_test_time_sec = *test_time_sec_ptr;

	ret = rt_pe_generic_fwd_test(generic_fwd_req, &generic_fwd_result);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to start generic_fwd upload test ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to start generic_fwd upload test ... RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		if(generic_fwd_result.ret_val != RT_PE_RET_OK)
			diag_util_mprintf("Fail to start generic_fwd upload test ... PE return 0x%x \n", generic_fwd_result.ret_val);
		else
		{
			diag_util_mprintf("========== Success to start generic_fwd UDP upload test ==========\n");
			diag_util_mprintf("connection_number %u \n", generic_fwd_req.connection_number);
			diag_util_mprintf("client_mac %02x:%02x:%02x:%02x:%02x:%02x \n", generic_fwd_req.pkt_pattern.client_mac.octet[0], generic_fwd_req.pkt_pattern.client_mac.octet[1], generic_fwd_req.pkt_pattern.client_mac.octet[2], generic_fwd_req.pkt_pattern.client_mac.octet[3], generic_fwd_req.pkt_pattern.client_mac.octet[4], generic_fwd_req.pkt_pattern.client_mac.octet[5]);
			diag_util_mprintf("server_mac %02x:%02x:%02x:%02x:%02x:%02x \n", generic_fwd_req.pkt_pattern.server_mac.octet[0], generic_fwd_req.pkt_pattern.server_mac.octet[1], generic_fwd_req.pkt_pattern.server_mac.octet[2], generic_fwd_req.pkt_pattern.server_mac.octet[3], generic_fwd_req.pkt_pattern.server_mac.octet[4], generic_fwd_req.pkt_pattern.server_mac.octet[5]);
			diag_util_mprintf("client_ipv6 %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n", 
								generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[0], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[1], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[2], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[3],
								generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[4], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[5], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[6], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[7],
								generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[8], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[9], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[10], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[11],
								generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[12], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[13], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[14], generic_fwd_req.pkt_pattern.client_ip.ipv6_addr.ipv6_addr[15]);
			diag_util_mprintf("server_ipv6 %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n", 
								generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[0], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[1], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[2], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[3],
								generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[4], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[5], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[6], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[7],
								generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[8], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[9], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[10], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[11],
								generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[12], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[13], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[14], generic_fwd_req.pkt_pattern.server_ip.ipv6_addr.ipv6_addr[15]);
			diag_util_mprintf("l4_proto %u\n", generic_fwd_req.pkt_pattern.l4_proto);
			diag_util_mprintf("client_l4port %u(0x%x) \n", generic_fwd_req.pkt_pattern.client_l4port, generic_fwd_req.pkt_pattern.client_l4port);
			diag_util_mprintf("server_l4port %u(0x%x) \n", generic_fwd_req.pkt_pattern.server_l4port, generic_fwd_req.pkt_pattern.server_l4port);
			diag_util_mprintf("udp_upload_payload_length %u\n", generic_fwd_req.udp_upload_payload_length);
			diag_util_mprintf("udp_test_time_sec %u\n", generic_fwd_req.udp_test_time_sec);
		}
	}	

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_pe_generic_fwd_test_upload_start_connection_number_connection_number_client_mac_client_mac_server_mac_server_mac_IPv6_client_ip_client_ip_server_ip_server_ip_UDP_client_l4port_client_l4port_server_l4port_server_l4port_upload_payload_length_upload_payload_length_test_time_sec_test_time_sec */

