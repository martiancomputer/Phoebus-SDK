
#include <rtk_types.h>
#include <rtl8367_mapper_port.h>
#include <port.h>


#include <osal/memory.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_bitop.h>
#include <common/util/rt_util.h>
#include <common/debug/rt_log.h>
#include <hal/common/halctrl.h>





/* Function Name:
 *      rtl8367_mapper_port_link_get
 * Description:
 *      Get the link status of the specific port
 * Input:
 *      port    - port id
 * Output:
 *      pStatus - pointer to the link status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The link status of the port is as following:
 *      - LINKDOWN
 *      - LINKUP
 */

int32
rtl8367_mapper_port_link_get(rtk_port_t port, rtk_port_linkStatus_t *pStatus)
{
    rtksw_port_t port_id;
    rtksw_api_ret_t retVal;
    rtksw_port_mac_ability_t portStatus;

 //   printk("%s port:%d \n", __FUNCTION__, port);


#if 1
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    //RT_INIT_CHK(port_init);

    /* parameter check */
    //RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pStatus), RT_ERR_NULL_POINTER);
#endif
    port_id = (rtksw_port_t)RT_PORT_TO_EXT_SWITCH_PORT(port);
    memset(&portStatus, 0x00, sizeof(rtksw_port_mac_ability_t));
    if ((retVal = rtksw_port_macStatus_get(port_id, &portStatus)) != RT_ERR_OK)
        return (int32)retVal;

    *pStatus = ((portStatus.link == 1) ? PORT_LINKUP : PORT_LINKDOWN);

   // printk("%s pStatus:%d \n", __FUNCTION__, *pStatus);

    return RT_ERR_OK;
}


/* Function Name:
 *      rtl8367_mapper_port_speedDuplex_get
 * Description:
 *      Get the negotiated port speed and duplex status of the specific port
 * Input:
 *      port    - port id
 * Output:
 *      pSpeed  - pointer to the port speed
 *      pDuplex - pointer to the port duplex
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 *      RT_ERR_PORT_LINKDOWN - link down port status
 * Note:
 *      (1) The speed type of the port is as following:
 *          - PORT_SPEED_10M
 *          - PORT_SPEED_100M
 *          - PORT_SPEED_1000M
 *
 *      (2) The duplex mode of the port is as following:
 *          - HALF_DUPLEX
 *          - FULL_DUPLEX
 */

int32
rtl8367_mapper_port_speedDuplex_get(
    rtk_port_t        port,
    rtk_port_speed_t  *pSpeed,
    rtk_port_duplex_t *pDuplex)
{
    rtksw_port_t port_id;
    rtksw_api_ret_t retVal;
    rtksw_port_mac_ability_t portStatus;
  //  printk("%s port:%d \n", __FUNCTION__, port);
#if 1
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    //RT_INIT_CHK(port_init);

    /* parameter check */
    //RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pSpeed), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pDuplex), RT_ERR_NULL_POINTER);
#endif
    port_id = (rtksw_port_t)RT_PORT_TO_EXT_SWITCH_PORT(port);
    memset(&portStatus, 0x00, sizeof(rtksw_port_mac_ability_t));
    if ((retVal = rtksw_port_macStatus_get(port_id, &portStatus)) != RT_ERR_OK)
        return (int32)retVal;

    switch(portStatus.speed)
    {
        case RTKSW_PORT_SPEED_10M:
            *pSpeed = PORT_SPEED_10M;
            break;
        case RTKSW_PORT_SPEED_100M:
            *pSpeed = PORT_SPEED_100M;
            break;
        case RTKSW_PORT_SPEED_1000M:
            *pSpeed = PORT_SPEED_1000M;
            break;
        case RTKSW_PORT_SPEED_500M:
            *pSpeed = PORT_SPEED_500M;
            break;

        default:
            return RT_ERR_FAILED;
            break;
    }

    *pDuplex = ((portStatus.duplex == RTKSW_PORT_FULL_DUPLEX) ? PORT_FULL_DUPLEX : PORT_HALF_DUPLEX);
//    printk("%s pSpeed:%d pDuplex:%d \n", __FUNCTION__, *pSpeed, *pDuplex);

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367_mapper_port_adminEnable_set
 * Description:
 *      Set port admin configuration of the specific port.
 * Input:
 *      port    - port id.
 *      enable  - Back pressure status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      This API can set port admin configuration of the specific port.
 *      The port admin configuration of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
int32 rtl8367_mapper_port_adminEnable_set(rtk_port_t port, rtk_enable_t enable)
{
	rtksw_port_t port_id;
    rtksw_api_ret_t retVal;
	rtksw_enable_t ext_enable;
	
	//  printk("%s port:%d \n", __FUNCTION__, port);
#if 1
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    //RT_INIT_CHK(port_init);

    /* parameter check */
    //RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    //RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
#endif	

	port_id = (rtksw_port_t)RT_PORT_TO_EXT_SWITCH_PORT(port);
    memset(&ext_enable, 0x00, sizeof(rtksw_enable_t));
	
	switch(enable)
    {
    case DISABLED:
		ext_enable = RTKSW_DISABLED;
		break;
	case ENABLED:
		ext_enable = RTKSW_ENABLED;
		break;
    default:
            return RT_ERR_FAILED;
            break;
    }

	if ((retVal = rtksw_port_adminEnable_set(port_id, ext_enable)) != RT_ERR_OK)
        return (int32)retVal;

	return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367_mapper_port_adminEnable_get
 * Description:
 *      Get port admin configurationof the specific port.
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - Back pressure status.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API can get port admin configuration of the specific port.
 *      The port admin configuration of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
int32 rtl8367_mapper_port_adminEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
	rtksw_port_t port_id;
    rtksw_api_ret_t retVal;
	rtksw_enable_t enable;

	//  printk("%s port:%d \n", __FUNCTION__, port);
#if 1
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    //RT_INIT_CHK(port_init);

    /* parameter check */
    //RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
#endif
    port_id = (rtksw_port_t)RT_PORT_TO_EXT_SWITCH_PORT(port);
    memset(&enable, 0x00, sizeof(rtksw_enable_t));
    if ((retVal = rtksw_port_adminEnable_get(port_id, &enable)) != RT_ERR_OK)
        return (int32)retVal;

	switch(enable)
    {
    case RTKSW_DISABLED:
		*pEnable = DISABLED;
		break;
	case RTKSW_ENABLED:
		*pEnable = ENABLED;
		break;
    default:
            return RT_ERR_FAILED;
            break;
    }
	
	return RT_ERR_OK;
}


/* Function Name:
 *      rtl8367_mapper_port_phyAutoNegoAbility_set
 * Description:
 *      Set ethernet PHY auto-negotiation desired ability.
 * Input:
 *      port        - port id.
 *      pAbility    - Ability structure
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_PHY_REG_ID       - Invalid PHY address
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      If Full_1000 bit is set to 1, the AutoNegotiation will be automatic set to 1. While both AutoNegotiation and Full_1000 are set to 0, the PHY speed and duplex selection will
 *      be set as following 100F > 100H > 10F > 10H priority sequence.
 */
int32 rtl8367_mapper_port_phyAutoNegoAbility_set(rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
	rtksw_port_t port_id;
    rtksw_api_ret_t retVal;
	rtksw_port_phy_ability_t portAbility;

	//  printk("%s port:%d \n", __FUNCTION__, port);
#if 1
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    //RT_INIT_CHK(port_init);

    /* parameter check */
    //RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pAbility), RT_ERR_NULL_POINTER);
#endif	
	port_id = (rtksw_port_t)RT_PORT_TO_EXT_SWITCH_PORT(port);
    memset(&portAbility, 0x00, sizeof(rtksw_port_phy_ability_t));
	portAbility.Half_10		= pAbility->Half_10;
	portAbility.Full_10		= pAbility->Full_10;
	portAbility.Half_100	= pAbility->Half_100;
	portAbility.Full_100	= pAbility->Full_100;
	portAbility.Full_1000	= pAbility->Full_1000;
	portAbility.FC			= pAbility->FC;
	portAbility.AsyFC		= pAbility->AsyFC;

	if ((retVal = rtksw_port_phyAutoNegoAbility_set(port_id, &portAbility)) != RT_ERR_OK)
        return (int32)retVal;

	return RT_ERR_OK;
}


/* Function Name:
 *      rtl8367_mapper_port_phyAutoNegoAbility_get
 * Description:
 *      Get PHY ability through PHY registers.
 * Input:
 *      port - Port id.
 * Output:
 *      pAbility - Ability structure
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_PHY_REG_ID       - Invalid PHY address
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      Get the capablity of specified PHY.
 */
int32 rtl8367_mapper_port_phyAutoNegoAbility_get(rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
	rtksw_port_t port_id;
    rtksw_api_ret_t retVal;
	rtksw_port_phy_ability_t portAbility;

	//  printk("%s port:%d \n", __FUNCTION__, port);
#if 1
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    //RT_INIT_CHK(port_init);

    /* parameter check */
    //RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pAbility), RT_ERR_NULL_POINTER);
#endif
    port_id = (rtksw_port_t)RT_PORT_TO_EXT_SWITCH_PORT(port);
    memset(&portAbility, 0x00, sizeof(rtksw_port_phy_ability_t));
    if ((retVal = rtksw_port_phyAutoNegoAbility_get(port_id, &portAbility)) != RT_ERR_OK)
        return (int32)retVal;

	pAbility->Half_10		= portAbility.Half_10;
	pAbility->Full_10		= portAbility.Full_10;
	pAbility->Half_100		= portAbility.Half_100;
	pAbility->Full_100		= portAbility.Full_100;
	pAbility->Full_1000		= portAbility.Full_1000;
	pAbility->FC			= portAbility.FC;
	pAbility->AsyFC			= portAbility.AsyFC;

	return RT_ERR_OK;
}

