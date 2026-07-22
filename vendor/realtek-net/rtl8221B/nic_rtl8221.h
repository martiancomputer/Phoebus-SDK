/*
 * Copyright (C) 2024 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * Purpose : PHY RTL8226/RTL8221 Driver
 *
 * Feature : PHY RTL8226/RTL8221 Driver
 *
 */
#ifndef __NIC_RTL8221_H__
#define __NIC_RTL8221_H__

//#include <hal/phy/nic_rtl8221/rtl8221_typedef.h>
#include <rtl8221_typedef.h>

#define RELEASE_VER_M 1
//#define RELEASE_VER_SUB 0
#define _RELEASE_DATE 240327


BOOLEAN
Rtl8221_api_ver(IN HANDLE hDevice);

BOOLEAN
Rtl8221_ThermalSensor_get(
    IN HANDLE hDevice,
    OUT PHY_THERMAL_RESULT *pTsResult
    );

BOOLEAN
Rtl8221_ThermalSensor_resume_2P5G(
    IN HANDLE hDevice
    );

BOOLEAN
Rtl8221_wol_set(
    IN HANDLE hDevice,
    IN PHY_WOL_EVENT *pwolevent
    );

BOOLEAN
Rtl8221_wol_exit(
    IN HANDLE hDevice
    );

BOOLEAN
Rtl8221_phy_reset(
    IN HANDLE hDevice
    );

BOOLEAN
Rtl8221_autoNegoEnable_get(
    IN  HANDLE hDevice,
    OUT BOOL *pEnable
    );

BOOLEAN
Rtl8221_autoNegoEnable_set(
    IN HANDLE hDevice,
    IN BOOL Enable
    );

BOOLEAN
Rtl8221_autoNegoAbility_get(
    IN  HANDLE hDevice,
    OUT PHY_LINK_ABILITY *pPhyAbility
    );

BOOLEAN
Rtl8221_autoNegoAbility_set(
    IN HANDLE hDevice,
    IN PHY_LINK_ABILITY *pPhyAbility
    );

BOOLEAN
Rtl8221_duplex_get(
    IN  HANDLE hDevice,
    OUT BOOL *pEnable
    );

BOOLEAN
Rtl8221_duplex_set(
    IN HANDLE hDevice,
    IN BOOL Enable
    );

BOOLEAN
Rtl8221_is_link(
    IN  HANDLE hDevice,
    OUT BOOL *plinkok
    );

BOOLEAN
Rtl8221_speed_get(
    IN  HANDLE hDevice,
    OUT UINT16 *pSpeed
    );

BOOLEAN
Rtl8221_force_speed_set(
    IN HANDLE hDevice,
    IN UINT16 Speed
    );

BOOLEAN
Rtl8221_force_speed_get(
    IN  HANDLE hDevice,
    OUT UINT16 *force_speed
    );


BOOLEAN
Rtl8221_enable_set(
    IN HANDLE hDevice,
    IN BOOL Enable
    );

BOOLEAN
Rtl8221_eeeEnable_get(
    IN  HANDLE hDevice,
    OUT PHY_EEE_ENABLE *pEeeEnable
    );

BOOLEAN
Rtl8221_eeeEnable_set(
    IN HANDLE hDevice,
    IN PHY_EEE_ENABLE *pEeeEnable
    );

BOOLEAN
Rtl8221_PHYmodeEEE_set(
	IN HANDLE hDevice,
	int on_off
	);

BOOLEAN
Rtl8221_10M_PHYmodeEEEP_set(
	IN HANDLE hDevice,
	int on_off
	);


BOOLEAN
Rtl8221_crossOverMode_get(
    IN  HANDLE hDevice,
    OUT PHY_CROSSPVER_MODE *CrossOverMode
    );

BOOLEAN
Rtl8221_crossOverMode_set(
    IN HANDLE hDevice,
    IN PHY_CROSSPVER_MODE CrossOverMode
    );

BOOLEAN
Rtl8221_crossOverStatus_get(
    IN  HANDLE hDevice,
    OUT PHY_CROSSPVER_STATUS *pCrossOverStatus
    );

BOOLEAN
Rtl8221_masterSlave_get(
    IN  HANDLE hDevice,
    OUT PHY_MASTERSLAVE_MODE *MasterSlaveMode
    );

BOOLEAN
Rtl8221_masterSlave_set(
    IN HANDLE hDevice,
    IN PHY_MASTERSLAVE_MODE MasterSlaveMode
    );

BOOLEAN
Rtl8221_loopback_get(
    IN  HANDLE hDevice,
    OUT BOOL *pEnable
    );

BOOLEAN
Rtl8221_loopback_set(
    IN HANDLE hDevice,
    IN BOOL Enable
    );

BOOLEAN
Rtl8221_downSpeedEnable_get(
    IN  HANDLE hDevice,
    OUT BOOL *pEnable
    );

BOOLEAN
Rtl8221_downSpeedEnable_set(
    IN HANDLE hDevice,
    IN BOOL Enable
    );

BOOLEAN
Rtl8221_gigaLiteEnable_get(
    IN  HANDLE hDevice,
    OUT BOOL *pEnable
    );

BOOLEAN
Rtl8221_gigaLiteEnable_set(
    IN HANDLE hDevice,
    IN BOOL Enable
    );

BOOLEAN
Rtl8221_mdiSwapEnable_get(
    IN  HANDLE hDevice,
    OUT BOOL *pEnable
    );

BOOLEAN
Rtl8221_mdiSwapEnable_set(
    IN HANDLE hDevice,
    IN BOOL Enable
    );

BOOLEAN
Rtl8221_rtct_start(
    IN HANDLE hDevice
    );

BOOLEAN
Rtl8221_rtctResult_get(
    IN HANDLE hDevice,
    OUT PHY_RTCT_RESULT *pRtctResult
    );

BOOLEAN
Rtl8221_rtctdone_get(
    IN  HANDLE hDevice,
    OUT BOOL *prtct_done
    );


BOOLEAN
Rtl8221_linkDownPowerSavingEnable_get(
    IN  HANDLE hDevice,
    OUT BOOL *pEnable
    );

BOOLEAN
Rtl8221_linkDownPowerSavingEnable_set(
    IN HANDLE hDevice,
    IN BOOL Enable
    );

BOOLEAN
Rtl8221_2p5gLiteEnable_get(
    IN  HANDLE hDevice,
    OUT BOOL *pEnable
    );

BOOLEAN
Rtl8221_2p5gLiteEnable_set(
    IN HANDLE hDevice,
    IN BOOL Enable
    );

BOOLEAN
Rtl8221_ThermalSensorEnable_get(
    IN  HANDLE hDevice,
    OUT BOOL *pEnable
    );

BOOLEAN
Rtl8221_ThermalSensorEnable_set(
    IN HANDLE hDevice,
    IN BOOL Enable,
    IN UINT16 threshold
    );

BOOLEAN
Rtl8221_ieeeTestMode_set(
    IN HANDLE hDevice,
    IN UINT16 Speed,
    IN PHY_IEEE_TEST_MODE *pIEEEtestmode
    );

BOOLEAN
Rtl8221_serdes_link_get(
    IN  HANDLE hDevice,
    OUT BOOL *perdesLink,
    OUT PHY_SERDES_MODE *SerdesMode
    );

BOOLEAN
Rtl8221_serdes_option_set(
    IN HANDLE hDevice,
    IN UINT8 functioninput
    );

/*   serdes option set for initial state */
BOOLEAN
Rtl8221_serdes_option_set_for_init(
    IN HANDLE hDevice,
    IN UINT8 functioninput
    );

BOOLEAN
Rtl8221_serdes_option_get(
    IN HANDLE hDevice,
    OUT PHY_SERDES_OPTION *SerdesOption
    );

BOOLEAN
Rtl8221_serdes_polarity_swap(
    IN HANDLE hDevice,
    IN PHY_SERDES_POLARITY_SWAP *ppolarityswap
    );

BOOLEAN
Rtl8221_serdes_autoNego_set(
    IN HANDLE hDevice,
    IN BOOL Enable
    );

BOOLEAN
Rtl8221_fc_sts_get(
    IN  HANDLE hDevice,
    OUT UINT8 *tx_fc,
    OUT UINT8 *rx_fc
    );

BOOLEAN
Rtl8221_port_type_prefer_set(
    IN  HANDLE hDevice,
    IN  PHY_MASTERSLAVE_MODE mode
	);


#endif /* __NIC_RTL8226_H__ */

