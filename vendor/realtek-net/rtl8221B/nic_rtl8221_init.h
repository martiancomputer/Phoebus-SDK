
#ifndef __NIC_RTL8221_INIT_H__
#define __NIC_RTL8221_INIT_H__

BOOLEAN
Rtl8221_phy_init(
    IN HANDLE hDevice,
    IN PHY_LINK_ABILITY *pphylinkability,
    IN BOOL singlephy
    );

#endif




