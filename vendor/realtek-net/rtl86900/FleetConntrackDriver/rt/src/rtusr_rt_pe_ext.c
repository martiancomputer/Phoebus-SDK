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




/*
 * Include Files
 */
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <osal/lib.h>
#include <rtk/rtusr/include/rtusr_util.h>
#include <rtdrv_ext_netfilter.h>
#include <common/rt_type.h>

/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */


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
int rt_pe_http_test(rt_pe_http_test_request_t http_test_req, rt_pe_http_test_result_t *pHttp_test_result)
{
    rtdrv_rt_peCfg_ext_t rt_pe_cfg;

    /* function body */
    osal_memcpy(&rt_pe_cfg.http_test_request, &http_test_req, sizeof(rt_pe_http_test_request_t));
    GETSOCKOPT(RTDRV_RT_PE_HTTP_TEST_GET, &rt_pe_cfg, rtdrv_rt_peCfg_ext_t, 1);
	osal_memcpy(pHttp_test_result, &rt_pe_cfg.http_test_result, sizeof(rt_pe_http_test_result_t));

    return RT_ERR_OK;
}   /* end of rt_pe_http_test */

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
int rt_pe_iperf_test(rt_pe_iperf_request_t iperf_req, rt_pe_iperf_result_t *pIperf_result)
{
	rtdrv_rt_peCfg_ext_t rt_pe_cfg;

	/* function body */
	osal_memcpy(&rt_pe_cfg.iperf_request, &iperf_req, sizeof(rt_pe_iperf_request_t));
	GETSOCKOPT(RTDRV_RT_PE_IPERF_TEST_GET, &rt_pe_cfg, rtdrv_rt_peCfg_ext_t, 1);
	osal_memcpy(pIperf_result, &rt_pe_cfg.iperf_result, sizeof(rt_pe_iperf_result_t));

	return RT_ERR_OK;
}   /* end of rt_pe_iperf_test */

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
int rt_pe_generic_fwd_test(rt_pe_generic_fwd_request_t fwd_req, rt_pe_generic_fwd_result_t *pFwd_ret)
{
	rtdrv_rt_peCfg_ext_t rt_pe_cfg;

	/* function body */
	osal_memcpy(&rt_pe_cfg.generic_fwd_request, &fwd_req, sizeof(rt_pe_generic_fwd_request_t));
	GETSOCKOPT(RTDRV_RT_PE_GENERIC_FWD_TEST_GET, &rt_pe_cfg, rtdrv_rt_peCfg_ext_t, 1);
	osal_memcpy(pFwd_ret, &rt_pe_cfg.generic_fwd_result, sizeof(rt_pe_generic_fwd_result_t));

	return RT_ERR_OK;
}   /* end of rt_pe_generic_fwd_test */

