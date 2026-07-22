#ifndef _RTL8367_MAPPER_PORT_H_
#define _RTL8367_MAPPER_PORT_H_
#include <dal/dal_ext_switch_mapper.h>


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

extern int32
rtl8367_mapper_port_link_get(rtk_port_t port, rtk_port_linkStatus_t *pStatus);



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

extern int32
rtl8367_mapper_port_speedDuplex_get(
    rtk_port_t        port,
    rtk_port_speed_t  *pSpeed,
    rtk_port_duplex_t *pDuplex);


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
extern int32 rtl8367_mapper_port_adminEnable_set(rtk_port_t port, rtk_enable_t enable);

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
extern int32 rtl8367_mapper_port_adminEnable_get(rtk_port_t port, rtk_enable_t *pEnable);

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
extern int32 rtl8367_mapper_port_phyAutoNegoAbility_set(rtk_port_t port, rtk_port_phy_ability_t *pAbility);

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
extern int32 rtl8367_mapper_port_phyAutoNegoAbility_get(rtk_port_t port, rtk_port_phy_ability_t *pAbility);

#endif /* _RTL8367_MAPPER_PORT_H_ */

