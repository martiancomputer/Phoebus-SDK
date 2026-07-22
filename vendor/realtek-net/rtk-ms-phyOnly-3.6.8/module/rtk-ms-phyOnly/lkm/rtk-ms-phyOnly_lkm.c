/*
 * Copyright (C) 2009-2015 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 71708 $
 * $Date: 2016-09-19 11:31:17 +0800 (Mon, 19 Sep 2016) $
 *
 * Purpose : Create stacking linux kernel module
 *
 */

/*
 * Include Files
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <fs/proc/internal.h>
#include <linux/uaccess.h>
#include <linux/delay.h>	/* mdelay() */
#include <linux/kthread.h>
#include <common/rt_error.h>
//#if IS_BUILTIN(CONFIG_RTK_EXT_GPHY) || IS_MODULE(CONFIG_RTK_EXT_GPHY)
#include <rtk_ext_gphy.h>
//#endif
#include <phy_hal.h>
#include <phy_init.h>
#include <soc/type.h>
#include <hal/phy/phydef.h>
#include <rtk/phy.h>
#include <example/src/test_phyApi.h>
#include <common/rt_version.h>	/* Get correct RT_VERSION_SDK */
#include <hal/mac/miim_common_drv.h>
#ifdef CONFIG_LUNA_G3_SERIES
#include <aal_phy.h>
#endif

//#define RTKMSPHY_DEBUG_ENABLE

#ifdef RTKMSPHY_DEBUG_ENABLE
#include <hal/phy/phy_rtl8224_patch.h>
#endif
#include <hal/phy/phy_rtl8224.h>

/*
 * Symbol Definition
 */
#define RTKMSPHY_PRINT(fmt,args...)	printk(KERN_INFO fmt"\n", ##args)
#define RTKMSPHY_MSG(fmt,args...)	printk(KERN_INFO "rtkMSphy: "fmt"\n", ##args)
#define RTKMSPHY_INFO(fmt,args...)	printk(KERN_INFO "\033[1;33;46m""rtkMSphy: "fmt"\033[m""\n", ##args)
#define RTKMSPHY_ERROR(fmt,args...)	printk(KERN_INFO "\033[1;33;41m""rtkMSphy: "fmt"\033[m""\n", ##args)

#ifdef RTKMSPHY_DEBUG_ENABLE
#define RTKMSPHY_DEBUG			RTKMSPHY_INFO
#else
#define RTKMSPHY_DEBUG(fmt,args...)
#endif

#define RTL8261_CA_XFI_PORT		(6)
#define RTL8261_CA_PON_PORT		(7)

//#define RTL8261_MMD_1_3_VALUE		(0xCAF1)
//#define RTL8261_CCUT_MMD_1_3_VALUE	(0xCAF2)

/*
 * Data Declaration
 */
#define RTKMSPHY_DEFAULT_UNIT_ID	(0)
#define RTKMSPHY_MAX_PHY_NUMBER		(2)
#define RTKMSPHY_MAX_PORT_NUMBER	(4)
#define RTKMSPHY_RTL8224_PORT_NUMBER	(4)

static int rtkmsphy_init = 0;
static int rtkmsphy_phy_count = 0;
static int rtkmsphy_port_count = 0;

/*
 * This table is used to describe your hardware board design, especially for mapping relation between port and phy.
 * Port related information
 * .mac_id      = port id.
 * .phy_idex    = used to indicate which PHY entry is used by this port in glued_phy_Descp[].
 * .eth         = Ethernet speed type (refer to rt_port_ethType_t).
 * .medi        = Port media type (refer to rt_port_medium_t).
 */
phy_hwp_portDescp_t  my_port_descp[RTKMSPHY_MAX_PORT_NUMBER + 1];
//phy_hwp_portDescp_t  my_port_descp[] = {
//        { .mac_id = HWP_END, .phy_idx = 0, .eth = HWP_XGE, .medi = HWP_COPPER, .smi = 0, .phy_addr = 0},
//        { .mac_id = HWP_END, .phy_idx = 1, .eth = HWP_XGE, .medi = HWP_COPPER, .smi = 0, .phy_addr = 0},
//        { .mac_id = HWP_END },
//    };

/*
 * PHY related information
 * .chip        = PHY Chip model (refer to phy_type_t).
 * .mac_id      = The first port id of this PHY. For example, the 8218D is connected to
 *                port 0 ~ 7, then the .mac_id  = 0.
 * .phy_max     = The MAX port number of this PHY. For examplem the 8218D is an octet PHY,
 *                so this number is 8.
 */
phy_hwp_phyDescp_t     my_phy_Descp[RTKMSPHY_MAX_PHY_NUMBER + 1];
//phy_hwp_phyDescp_t     my_phy_Descp[] = {
//        [0] = { .chip = HWP_END, .mac_id = 0, .phy_max = 1 },
//        [1] = { .chip = HWP_END, .mac_id = 0, .phy_max = 1 },
//        [2] = { .chip = HWP_END },
//    };

/*
 * Macro Definition
 */

/*
 * rtkmsphy proc definition
 */
struct proc_dir_entry *rtkmsphy_proc_dir = NULL;

#define PROC_DIR_RTKMSPHY		"rtkmsphy"
#define PROC_FILE_HELP			"help"
#define PROC_FILE_PHY			"phy"
#define PROC_FILE_LINKSTATUS		"link_status"
#define PROC_FILE_CTRL			"ctrl"
#define PROC_FILE_TEMPERATURE		"temperature"
//#define PROC_FILE_CNT			"cnt"
#define PROC_FILE_FORCEMODEABILITY	"force_speed"
#define PROC_FILE_SDSTESTMODE		"sds_test"
#define PROC_FILE_IEEETESTMODE		"ieee_test"
#define PROC_FILE_SDSEYEPARAM		"sds_eye_param"
#define PROC_FILE_PHYEYEMONITOR		"phy_eye_monitor"

#define INVALID_VALUE			99
#define MAX_COMMAND_LEN			32

#if 0
int _rtk_rtl8261_proc_pasring_string_to_integer(const char *buff,unsigned long len)
{
	char tmpbuf[MAX_COMMAND_LEN] = {0};

	if(len >= MAX_COMMAND_LEN)
		len = MAX_COMMAND_LEN;

	if (buff) {
		/* copy data to the buffer */
		strncpy(tmpbuf, buff, len);
		tmpbuf[len - 1] = '\0';
		return simple_strtol(tmpbuf, NULL, 0);
	}

	return INVALID_VALUE;
}
#endif

u32
rtkmsphy_get_phy_Descp_index_by_mac_id(u8 mac_id)
{
	int i = 0;

	for (i = 0 ; my_phy_Descp[i].chip != HWP_END ; i++)
		if ((mac_id >= my_phy_Descp[i].mac_id) && (mac_id < my_phy_Descp[i].mac_id + my_phy_Descp[i].phy_max))
			return i;

	RTKMSPHY_ERROR("%s(%d): Can not find phy_Descp index by this mac_id %d", __FUNCTION__, __LINE__, mac_id);
	return INVALID_VALUE;
}

u8
rtkmsphy_get_phy_addr_by_mac_id(u8 mac_id)
{
	int i = 0;

	for (i = 0 ; my_port_descp[i].mac_id != HWP_END ; i++)
		if (my_port_descp[i].mac_id == mac_id)		return my_port_descp[i].phy_addr;

	RTKMSPHY_ERROR("%s(%d): Can not find this mac_id %d", __FUNCTION__, __LINE__, mac_id);
	return INVALID_VALUE;
}
EXPORT_SYMBOL(rtkmsphy_get_phy_addr_by_mac_id);

u8
rtkmsphy_get_mac_id_by_phy_addr(u8 phy_addr)
{
	int i = 0;

	for (i = 0 ; my_port_descp[i].mac_id != HWP_END ; i++)
		if (my_port_descp[i].phy_addr == phy_addr)	return my_port_descp[i].mac_id;

	RTKMSPHY_DEBUG("%s(%d): Can not find this phy_addr %d", __FUNCTION__, __LINE__, phy_addr);
	return INVALID_VALUE;
}
EXPORT_SYMBOL(rtkmsphy_get_mac_id_by_phy_addr);

#ifdef RTKMSPHY_DEBUG_ENABLE
/*
 * Function Declaration
 */
uint32
rtkmsphy_8224_mask(uint8 msb, uint8 lsb)
{
    uint32  val = 0;
    uint8   i = 0;

    for (i = lsb; i <= msb; i++)
    {
        val |= (1 << i);
    }
    return val;
}

int32
rtkmsphy_8224_mask_get(uint8 msb, uint8 lsb, uint32 *mask)
{
    if ((msb > 31) || (lsb > 31) || (msb < lsb))
    {
        return RT_ERR_FAILED;
    }
    *mask = rtkmsphy_8224_mask(msb, lsb);

    return RT_ERR_OK;
}

static int32 rtkmsphy_8224_sdsRegField_set(uint32 unit, rtk_port_t port, uint32 sdsPage, uint32 sdsReg, uint8 msb, uint8 lsb, uint32 data)
{
    int32           ret;
    rtk_port_t      base_port = 0;
    uint32          reg_data = 0;
    uint32          mask = 0;

    if ((msb > 15) || (lsb > 15) || (msb < lsb))
    {
        return RT_ERR_FAILED;
    }

    base_port = HWP_PHY_BASE_MACID(unit, port);

    phy_rtl8224_sdsReg_get(unit, base_port, sdsPage, sdsReg, &reg_data);

    ret = rtkmsphy_8224_mask_get(msb, lsb, &mask);
    if(ret != RT_ERR_OK)
        return ret;

    reg_data &= ~(mask);
    reg_data |= (((data) << lsb) & mask);

    phy_rtl8224_sdsReg_set(unit, base_port, sdsPage, sdsReg, reg_data);

    return RT_ERR_OK;
}

static int32 rtkmsphy_8224_sdsRegField_get(uint32 unit, rtk_port_t port, uint32 sdsPage, uint32 sdsReg, uint8 msb, uint8 lsb, uint32 *pData)
{
    int32           ret;
    rtk_port_t      base_port = 0;
    uint32          reg_data = 0;
    uint32          mask = 0;

    if ((msb > 15) || (lsb > 15) || (msb < lsb))
    {
        return RT_ERR_FAILED;
    }

    base_port = HWP_PHY_BASE_MACID(unit, port);

    phy_rtl8224_sdsReg_get(unit, base_port, sdsPage, sdsReg, &reg_data);

    ret = rtkmsphy_8224_mask_get(msb, lsb, &mask);
    if(ret != RT_ERR_OK)
        return ret;

    *pData = (((reg_data) & (mask)) >> lsb);

    return RT_ERR_OK;
}
#endif

void rtkmsphy_proc_usage(const char *filename)
{
	RTKMSPHY_MSG("[Usage]");

	if((strcasecmp(filename, PROC_FILE_PHY) == 0) || (strcasecmp(filename, PROC_FILE_HELP) == 0)) {
		RTKMSPHY_MSG("echo [$action] > /proc/%s/%s", PROC_DIR_RTKMSPHY, PROC_FILE_PHY);
		RTKMSPHY_MSG("\t$action: {init,reset}");
		if(strcasecmp(filename, PROC_FILE_PHY) == 0)		return;
	}

	if((strcasecmp(filename, PROC_FILE_LINKSTATUS) == 0) || (strcasecmp(filename, PROC_FILE_HELP) == 0)) {
		RTKMSPHY_MSG("cat /proc/%s/%s", PROC_DIR_RTKMSPHY, PROC_FILE_LINKSTATUS);
		if(strcasecmp(filename, PROC_FILE_LINKSTATUS) == 0)		return;
	}

	if((strcasecmp(filename, PROC_FILE_CTRL) == 0) || (strcasecmp(filename, PROC_FILE_HELP) == 0)) {
		RTKMSPHY_MSG("cat /proc/%s/%s", PROC_DIR_RTKMSPHY, PROC_FILE_CTRL);
		RTKMSPHY_MSG("echo [$action] [option] > /proc/%s/%s", PROC_DIR_RTKMSPHY, PROC_FILE_CTRL);
		RTKMSPHY_MSG("\t$action: {serdes}");
		if(strcasecmp(filename, PROC_FILE_LINKSTATUS) == 0)		return;
	}

	return;
}

static int help_fops(struct seq_file *s, void *v)
{
	rtkmsphy_proc_usage(PROC_FILE_HELP);
	return 0;
}

#if 0
static int Rtl8261_phy_fops(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	int32 ret;
	char tmpbuf[MAX_COMMAND_LEN] = {0};
	int len = (count > MAX_COMMAND_LEN) ? (MAX_COMMAND_LEN - 1) : count;

	rtk_phy_initInfo_t initInfo;

	initInfo.port_desc = my_port_descp;
	initInfo.phy_desc = my_phy_Descp;

	RTKMSPHY_DEBUG("%s - %s: %s (%d)", file->f_path.dentry->d_iname, __FUNCTION__, buffer, count);
	if (buffer) {
		char *strptr, *split_str;

		/* copy data to the buffer */
		strncpy(tmpbuf, buffer, len);
		tmpbuf[len] = '\0';

		strptr = tmpbuf;
		RTKMSPHY_DEBUG("strptr: [%s]", strptr);

		/*parse command*/
		split_str = strsep(&strptr," ");
		RTKMSPHY_DEBUG("split_str [%s]", split_str);

		if(strcasecmp(split_str, "init") == 0)
		{
			if ((ret = rtk_init(&initInfo)) == RT_ERR_OK)
				RTKMSPHY_MSG("rtk_init OK");
			else
				RTKMSPHY_ERROR("rtk_init Fail (0x%x) !!!", ret);

			if((ret = rtk_port_adminEnable_set(RTKMSPHY_DEFAULT_UNIT_ID, CONFIG_RTL8261_PORT7_PHY_ADDR, ENABLED)) == RT_ERR_OK)
				RTKMSPHY_MSG("rtk_port_adminEnable_set OK");
			else
				RTKMSPHY_ERROR("rtk_port_adminEnable_set ERROR(0x%x) !!!", ret);
		}
		else if(strcasecmp(split_str, "reset") == 0)
		{
			if ((ret = rtk_port_phyReset_set(RTKMSPHY_DEFAULT_UNIT_ID, CONFIG_RTL8261_PORT7_PHY_ADDR)) == RT_ERR_OK)
				RTKMSPHY_MSG("rtk_port_phyReset_set OK");
			else
				RTKMSPHY_ERROR("rtk_port_phyReset_set Fail (0x%x) !!!", ret);
		}else
		{
			goto phy_error;
		}
	}
	return count;

phy_error:
	RTKMSPHY_ERROR("%s() FAIL ......", __FUNCTION__);
	rtkmsphy_proc_usage(file->f_path.dentry->d_iname);
	return -EFAULT;
}
#endif

uint64 old_cnt_rx[RTKMSPHY_MAX_PORT_NUMBER + 1];
uint64 old_cnt_rx_err[RTKMSPHY_MAX_PORT_NUMBER + 1];
uint64 old_cnt_rx_crcerr[RTKMSPHY_MAX_PORT_NUMBER + 1];
uint64 old_cnt_ldpc_err[RTKMSPHY_MAX_PORT_NUMBER + 1];

static int rtkmsphy_link_status_fops(struct seq_file *s, void *v)
{
	int32 ret, i, j, sds_num;
	uint32 value;
	uint64 cnt = 0;
	rtk_enable_t enable;
	rtk_port_linkStatus_t status;
	rtk_port_media_t media;
	rtk_port_speed_t  speed;
	rtk_port_duplex_t duplex;
	rtk_phy_macIntfSdsLinkStatus_t sstatus;
#ifdef RTKMSPHY_DEBUG_ENABLE
	uint8 msb = 0, lsb = 0;
	rtk_port_t base_port = 0;
	uint32 sdsPage = 0, sdsReg = 0, reg_data = 0;
	uint32 mmdAddr = 0, mmdReg = 0;
#endif
	const char *status_info[] = {
		"DOWN",
		"UP",
		NULL
	};
	const char *media_info[] = {
		"COPPER",
		"FIBER",
		NULL
	};
	const char *speed_info[] = {
		"10Mbps",
		"100Mbps",
		"1Gbps",
		"500Mbpd",
		"2Gbps",   /* Applicable to 8380 */
		"2.5Gbps",
		"5Gbps",
		"10Gbps",
		"2.5Gbps Lite",
		"5Gbps Lite",
		"10Gbos Lite",
		NULL
	};
	const char *duplex_info[] = {
		"HALF",
		"FULL",
		NULL
	};

	for (i = 0 ; my_port_descp[i].mac_id != HWP_END ; i++) {
		RTKMSPHY_PRINT("----------------------------------------------------------------");
		switch (my_phy_Descp[rtkmsphy_get_phy_Descp_index_by_mac_id(my_port_descp[i].mac_id)].chip) {
		case RTK_PHYTYPE_RTL8261:
			RTKMSPHY_PRINT("Port %d  [RTL8261] [Addr %d]", my_port_descp[i].mac_id, my_port_descp[i].phy_addr);
			break;
		case RTK_PHYTYPE_RTL8261I:
			RTKMSPHY_PRINT("Port %d  [RTL8261I] [Addr %d]", my_port_descp[i].mac_id, my_port_descp[i].phy_addr);
			break;
		case RTK_PHYTYPE_RTL8261BE:
			RTKMSPHY_PRINT("Port %d  [RTL8261N(BE)] [Addr %d]", my_port_descp[i].mac_id, my_port_descp[i].phy_addr);
			break;
		case RTK_PHYTYPE_RTL8261B:
			RTKMSPHY_PRINT("Port %d  [RTL8261N(B)] [Addr %d]", my_port_descp[i].mac_id, my_port_descp[i].phy_addr);
			break;
		case RTK_PHYTYPE_RTL8251L_REV3:
			RTKMSPHY_PRINT("Port %d  [RTL8251L_REV3] [Addr %d]", my_port_descp[i].mac_id, my_port_descp[i].phy_addr);
			break;
		case RTK_PHYTYPE_RTL8224:
			RTKMSPHY_PRINT("Port %d  [RTL8224] [Addr %d]", my_port_descp[i].mac_id, my_port_descp[i].phy_addr);
			break;
		case RTK_PHYTYPE_NONE:
		default:
			RTKMSPHY_PRINT("Port %d  [UNKNOWN PHYTYPE] [Addr %d]", my_port_descp[i].mac_id, my_port_descp[i].phy_addr);
		}

		/* Power */
		if((ret = rtk_port_adminEnable_get(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, &enable)) == RT_ERR_OK)
			RTKMSPHY_PRINT("\tPower\t\t: %s", status_info[enable]);
		else
			RTKMSPHY_ERROR("rtk_port_phyLinkStatus_get ERROR(0x%x) !!!", ret);

		/* Link */
		/* WORKAROUND: Read twice for real link status ??? */
		//rtk_port_phyLinkStatus_get(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, &status);
		if((ret = rtk_port_phyLinkStatus_get(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, &status)) == RT_ERR_OK)
			RTKMSPHY_PRINT("\tLink\t\t: %s", status_info[status]);
		else
			RTKMSPHY_ERROR("rtk_port_phyLinkStatus_get ERROR(0x%x) !!!", ret);

		/* Media/Speed/Duplex */
		if (status == PORT_LINKUP) {
			if ((ret = rtk_port_linkMedia_get(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, &status, &media)) == RT_ERR_OK)
				RTKMSPHY_PRINT("\tMedia\t\t: %s", media_info[media]);
			else
				RTKMSPHY_ERROR("rtk_port_linkMedia_get ERROR(0x%x) !!!", ret);

			if ((ret = rtk_port_speedDuplex_get(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, &speed, &duplex)) == RT_ERR_OK) {
				RTKMSPHY_PRINT("\tSpeed\t\t: %s", speed_info[speed]);
				RTKMSPHY_PRINT("\tDuplex\t\t: %s", duplex_info[duplex]);
			}
			else
				RTKMSPHY_ERROR("rtk_port_speedDuplex_get ERROR(0x%x) !!!", ret);
		}
		else {
			RTKMSPHY_PRINT("\tMedia\t\t: %s", "N/A");
			RTKMSPHY_PRINT("\tSpeed\t\t: %s", "N/A");
			RTKMSPHY_PRINT("\tDuplex\t\t: %s", "N/A");
		}

		/* Serdes Mode */
		if ((ret = rtk_port_phyCtrl_get(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, RTK_PHY_CTRL_SERDES_MODE, &value)) == RT_ERR_OK) {
			switch (value) {
			case RTK_PHY_CTRL_SERDES_MODE_USXGMII:						/* 0x0000: USXGMII */
				RTKMSPHY_PRINT("\tSerdes Mode\t: USXGMII (0x%04x) %s", value, RTK_PHY_CTRL_SERDES_MODE_IS_AUTO(value) ? "FIX" : "AUTO");
				break;
			case RTK_PHY_CTRL_SERDES_MODE_10GR_5GX_2P5GX_SGMII:				/* 0x1001: 10G-R/5G-X/2.5G-X/SGMII */
				RTKMSPHY_PRINT("\tSerdes Mode\t: 10G-R/5G-X/2.5G-X/SGMII (0x%04x) %s", value, RTK_PHY_CTRL_SERDES_MODE_IS_AUTO(value) ? "FIX" : "AUTO");
				break;
			case RTK_PHY_CTRL_SERDES_MODE_10GR_XFI5GADAPT_XFI2P5GADAPT_SGMII:		/* 0x1002: 10G-R/XFI-5G-ADAPT/XFI-2.5G-ADAPT/SGMII */
				RTKMSPHY_PRINT("\tSerdes Mode\t: 10G-R/XFI-5G-ADAPT/XFI-2.5G-ADAPT/SGMII (0x%04x) %s", value, RTK_PHY_CTRL_SERDES_MODE_IS_AUTO(value) ? "FIX" : "AUTO");
				break;
			case RTK_PHY_CTRL_SERDES_MODE_10GR_5GR_2P5GX_SGMII:				/* 0x1101: 10G-R/5G-R/2.5G-X/SGMII */
				RTKMSPHY_PRINT("\tSerdes Mode\t: 10G-R/5G-R/2.5G-X/SGMII (0x%04x) %s", value, RTK_PHY_CTRL_SERDES_MODE_IS_AUTO(value) ? "FIX" : "AUTO");
				break;
			case RTK_PHY_CTRL_SERDES_MODE_10GR_XFI5GCPRI_2P5GX_SGMII:			/* 0x1102: 10G-R/XFI-5G-CPRI/2.5G-X/SGMII */
				RTKMSPHY_PRINT("\tSerdes Mode\t: 10G-R/XFI-5G-CPRI/2.5G-X/SGMII (0x%04x) %s", value, RTK_PHY_CTRL_SERDES_MODE_IS_AUTO(value) ? "FIX" : "AUTO");
				break;
			case RTK_PHY_CTRL_SERDES_MODE_UNKNOWN:						/* 0xFFFF */
			default:
				RTKMSPHY_PRINT("\tSerdes Mode\t: UNKNOWN (%d)", ret);
				break;
			}
		}
		else
			RTKMSPHY_PRINT("\tSerdes Mode\t: N/A (0x%04X)", ret);

		/* Serdes Link */
		if((ret = rtk_port_phyMacIntfSerdesLinkStatus_get(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, &sstatus)) == RT_ERR_OK) {
			//RTKMSPHY_PRINT("\tSerDes Num\t: %d",sstatus.sds_num);
			if (sstatus.sds_num > RTK_PHY_MAC_INTF_SDS_MAX)		sds_num = RTK_PHY_MAC_INTF_SDS_MAX;
			else							sds_num = sstatus.sds_num;

			for (j = 0 ; j < sds_num ; j++)
				RTKMSPHY_PRINT("\tSerdes Link\t: #%d %s", j, status_info[sstatus.link_status[j]]);
		}
		else
			RTKMSPHY_ERROR("rtk_port_phyMacIntfSerdesLinkStatus_get ERROR(0x%x) !!!", ret);

		/* RX */
		if ((ret = rtk_port_phyDbgCounter_get(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, PHY_DBG_CNT_RX, &cnt)) == RT_ERR_OK) {
			RTKMSPHY_PRINT("\tRX\t\t: 0x%016llx\t[0x%016llx]", (cnt - old_cnt_rx[i]), cnt);
			old_cnt_rx[i] = cnt;
		} else
			RTKMSPHY_ERROR("rtk_port_phyDbgCounter_get ERROR: PHY_DBG_CNT_RX (0x%x) !!!", ret);

		/* RX_ERR */
		if ((ret = rtk_port_phyDbgCounter_get(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, PHY_DBG_CNT_RX_ERR, &cnt)) == RT_ERR_OK) {
			RTKMSPHY_PRINT("\tRX_ERR\t\t: 0x%016llx\t[0x%016llx]", (cnt - old_cnt_rx_err[i]), cnt);
			old_cnt_rx_err[i] = cnt;
		} else
			RTKMSPHY_ERROR("rtk_port_phyDbgCounter_get ERROR: PHY_DBG_CNT_RX_ERR (0x%x) !!!", ret);

		/* RX_CRCERR */
		if ((ret = rtk_port_phyDbgCounter_get(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, PHY_DBG_CNT_RX_CRCERR, &cnt)) == RT_ERR_OK) {
			RTKMSPHY_PRINT("\tRX_CRCERR\t: 0x%016llx\t[0x%016llx]", (cnt - old_cnt_rx_crcerr[i]), cnt);
			old_cnt_rx_crcerr[i] = cnt;
		} else
			RTKMSPHY_ERROR("rtk_port_phyDbgCounter_get ERROR: PHY_DBG_CNT_RX_CRCERR (0x%x) !!!", ret);

		/* LDPC_ERR */
		if ((ret = rtk_port_phyDbgCounter_get(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, PHY_DBG_CNT_LDPC_ERR, &cnt)) == RT_ERR_OK) {
			RTKMSPHY_PRINT("\tLDPC_ERR\t: 0x%016llx\t[0x%016llx]", (cnt - old_cnt_ldpc_err[i]),cnt);
			old_cnt_ldpc_err[i] = cnt;
		} else
			RTKMSPHY_ERROR("rtk_port_phyDbgCounter_get ERROR: PHY_DBG_CNT_LDPC_ERR (0x%x) !!!", ret);


	}
	RTKMSPHY_PRINT("----------------------------------------------------------------");

#ifdef RTKMSPHY_DEBUG_ENABLE
	printk(KERN_INFO "\n\n\n");
	base_port = 8;

	//_phy_8224_sdsRegField_get(0, base_port, PHY_8224_AM_PERIOD_PAGE, PHY_8224_AM_PERIOD_REG, PHY_8224_AM_PERIOD_HIGH_BIT, PHY_8224_AM_PERIOD_LOW_BIT, pAmPeriod);
	sdsPage = PHY_8224_AM_PERIOD_PAGE; sdsReg = PHY_8224_AM_PERIOD_REG; msb = PHY_8224_AM_PERIOD_HIGH_BIT; lsb = PHY_8224_AM_PERIOD_LOW_BIT;
	rtkmsphy_8224_sdsRegField_get(0, base_port, sdsPage, sdsReg, msb, lsb, &reg_data);
	RTKMSPHY_PRINT("[AM_PERIOD]\t\tphy_8224_sdsRegField_get(0, %d, %d, %d, %d, %d): %#x", base_port, sdsPage, sdsReg, msb, lsb, reg_data);

	sdsPage = 6; sdsReg = 29; msb = 11; lsb = 10;
	rtkmsphy_8224_sdsRegField_get(0, base_port, sdsPage, sdsReg, msb, lsb, &reg_data);
	RTKMSPHY_PRINT("[sync head]\t\tphy_8224_sdsRegField_get(0, %d, %d, %d, %d, %d): %#x", base_port, sdsPage, sdsReg, msb, lsb, reg_data);

	sdsPage = PHY_8224_NWAY_OPCODE_PAGE; sdsReg = PHY_8224_NWAY_OPCODE_REG; msb = PHY_8224_NWAY_OPCODE_HIGH_BIT; lsb = PHY_8224_NWAY_OPCODE_LOW_BIT;
	rtkmsphy_8224_sdsRegField_get(0, base_port, sdsPage, sdsReg, msb, lsb, &reg_data);
	RTKMSPHY_PRINT("[NWAYOP code]\t\tphy_8224_sdsRegField_get(0, %d, %d, %d, %d, %d): %#x", base_port, sdsPage, sdsReg, msb, lsb, reg_data);

	sdsPage = 7; sdsReg = 6; msb = 15; lsb = 0;
	rtkmsphy_8224_sdsRegField_get(0, base_port, sdsPage, sdsReg, msb, lsb, &reg_data);
	RTKMSPHY_PRINT("[tx config code]\tphy_8224_sdsRegField_get(0, %d, %d, %d, %d, %d): %#x", base_port, sdsPage, sdsReg, msb, lsb, reg_data);

	printk(KERN_INFO "\n\n\n");

	for (i = 0 ; i < 2 ; i++) {
		RTKMSPHY_PRINT("%d. Checking Serdes Status", i);
		sdsPage = 4; sdsReg = 1;
		phy_rtl8224_sdsReg_get(0, base_port, sdsPage, sdsReg, &reg_data);
		RTKMSPHY_PRINT("port get phy-sds-reg port %d sds-page %d sds-reg %d:\t\t%#x", base_port, sdsPage, sdsReg, reg_data);
		sdsPage = 7; sdsReg = 21;
		phy_rtl8224_sdsReg_get(0, base_port, sdsPage, sdsReg, &reg_data);
		RTKMSPHY_PRINT("port get phy-sds-reg port %d sds-page %d sds-reg %d:\t\t%#x", base_port, sdsPage, sdsReg, reg_data);
		sdsPage = 5; sdsReg = 0;
		phy_rtl8224_sdsReg_get(0, base_port, sdsPage, sdsReg, &reg_data);
		RTKMSPHY_PRINT("port get phy-sds-reg port %d sds-page %d sds-reg %d:\t\t%#x", base_port, sdsPage, sdsReg, reg_data);
		sdsPage = 5; sdsReg = 1;
		phy_rtl8224_sdsReg_get(0, base_port, sdsPage, sdsReg, &reg_data);
		RTKMSPHY_PRINT("port get phy-sds-reg port %d sds-page %d sds-reg %d:\t\t%#x", base_port, sdsPage, sdsReg, reg_data);
		sdsPage = 7; sdsReg = 27;
		phy_rtl8224_sdsReg_get(0, base_port, sdsPage, sdsReg, &reg_data);
		RTKMSPHY_PRINT("port get phy-sds-reg port %d sds-page %d sds-reg %d:\t\t%#x", base_port, sdsPage, sdsReg, reg_data);
		sdsPage = 7; sdsReg = 26;
		phy_rtl8224_sdsReg_get(0, base_port, sdsPage, sdsReg, &reg_data);
		RTKMSPHY_PRINT("port get phy-sds-reg port %d sds-page %d sds-reg %d:\t\t%#x", base_port, sdsPage, sdsReg, reg_data);
		sdsPage = 7; sdsReg = 23;
		phy_rtl8224_sdsReg_get(0, base_port, sdsPage, sdsReg, &reg_data);
		RTKMSPHY_PRINT("port get phy-sds-reg port %d sds-page %d sds-reg %d:\t\t%#x", base_port, sdsPage, sdsReg, reg_data);
		sdsPage = 7; sdsReg = 24;
		phy_rtl8224_sdsReg_get(0, base_port, sdsPage, sdsReg, &reg_data);
		RTKMSPHY_PRINT("port get phy-sds-reg port %d sds-page %d sds-reg %d:\t\t%#x", base_port, sdsPage, sdsReg, reg_data);
		sdsPage = 7; sdsReg = 25;
		phy_rtl8224_sdsReg_get(0, base_port, sdsPage, sdsReg, &reg_data);
		RTKMSPHY_PRINT("port get phy-sds-reg port %d sds-page %d sds-reg %d:\t\t%#x", base_port, sdsPage, sdsReg, reg_data);
		sdsPage = 7; sdsReg = 22;
		phy_rtl8224_sdsReg_get(0, base_port, sdsPage, sdsReg, &reg_data);
		RTKMSPHY_PRINT("port get phy-sds-reg port %d sds-page %d sds-reg %d:\t\t%#x", base_port, sdsPage, sdsReg, reg_data);
		mmdAddr = 30; mmdReg = 0x7b25;
		phy_hal_mmd_read(0, base_port, mmdAddr, mmdReg, &reg_data);
		RTKMSPHY_PRINT("port get phy-mmd-reg port %d mmd-addr %d mmd-reg %#x:\t\t%#x", base_port, mmdAddr, mmdReg, reg_data);
		mmdAddr = 30; mmdReg = 0x7b24;
		phy_hal_mmd_read(0, base_port, mmdAddr, mmdReg, &reg_data);
		RTKMSPHY_PRINT("port get phy-mmd-reg port %d mmd-addr %d mmd-reg %#x:\t\t%#x", base_port, mmdAddr, mmdReg, reg_data);
		/* USXGMII sub mode, should be 0xbed for 8224 */
		mmdAddr = 30; mmdReg = 0x7b20;
		phy_hal_mmd_read(0, base_port, mmdAddr, mmdReg, &reg_data);
		RTKMSPHY_PRINT("port get phy-mmd-reg port %d mmd-addr %d mmd-reg %#x:\t\t%#x", base_port, mmdAddr, mmdReg, reg_data);
		mdelay(10000);
	}
#endif
	return 0;
}

static int rtkmsphy_temperature_get_fops(struct seq_file *s, void *v)
{
	int32 ret, i;
	uint32 value, temperature;

	for (i = 0 ; my_port_descp[i].mac_id != HWP_END ; i++) {
		RTKMSPHY_PRINT("----------------------------------------------------------------");
		switch (my_phy_Descp[rtkmsphy_get_phy_Descp_index_by_mac_id(my_port_descp[i].mac_id)].chip) {
		case RTK_PHYTYPE_RTL8261:
			RTKMSPHY_PRINT("Port %d  [RTL8261] [Addr %d]", my_port_descp[i].mac_id, my_port_descp[i].phy_addr);
			break;
		case RTK_PHYTYPE_RTL8261I:
			RTKMSPHY_PRINT("Port %d  [RTL8261I] [Addr %d]", my_port_descp[i].mac_id, my_port_descp[i].phy_addr);
			break;
		case RTK_PHYTYPE_RTL8261BE:
			RTKMSPHY_PRINT("Port %d  [RTL8261N(BE)] [Addr %d]", my_port_descp[i].mac_id, my_port_descp[i].phy_addr);
			break;
		case RTK_PHYTYPE_RTL8261B:
			RTKMSPHY_PRINT("Port %d  [RTL8261N(B)] [Addr %d]", my_port_descp[i].mac_id, my_port_descp[i].phy_addr);
			break;
		case RTK_PHYTYPE_RTL8251L_REV3:
			RTKMSPHY_PRINT("Port %d  [RTL8251L_REV3] [Addr %d]", my_port_descp[i].mac_id, my_port_descp[i].phy_addr);
			break;
		case RTK_PHYTYPE_RTL8224:
			RTKMSPHY_PRINT("Port %d  [RTL8224] [Addr %d]", my_port_descp[i].mac_id, my_port_descp[i].phy_addr);
			break;
		case RTK_PHYTYPE_NONE:
		default:
			RTKMSPHY_PRINT("Port %d  [UNKNOWN PHYTYPE] [Addr %d]", my_port_descp[i].mac_id, my_port_descp[i].phy_addr);
		}

		/* Temperature */
		if ((ret = rtk_port_phyCtrl_get(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, RTK_PHY_CTRL_TEMP, &value)) == RT_ERR_OK) {
			if(value == 262144)
				temperature = -((2 << 19) - value) / 1024;
			else
				temperature = value / 1024;
			RTKMSPHY_PRINT("\tTemperature\t\t: %d C", temperature);
		}
		else
			RTKMSPHY_PRINT("\tTemperature\t\t: ERROR(%d)", ret);
	}

	return 0;
}

static int rtkmsphy_ctrl_get_fops(struct seq_file *s, void *v)
{
	int32 ret, i;
	uint32 value, temperature;

	for (i = 0 ; my_port_descp[i].mac_id != HWP_END ; i++) {
		RTKMSPHY_PRINT("----------------------------------------------------------------");
		switch (my_phy_Descp[rtkmsphy_get_phy_Descp_index_by_mac_id(my_port_descp[i].mac_id)].chip) {
		case RTK_PHYTYPE_RTL8261:
			RTKMSPHY_PRINT("Port %d  [RTL8261] [Addr %d]", my_port_descp[i].mac_id, my_port_descp[i].phy_addr);
			break;
		case RTK_PHYTYPE_RTL8261I:
			RTKMSPHY_PRINT("Port %d  [RTL8261I] [Addr %d]", my_port_descp[i].mac_id, my_port_descp[i].phy_addr);
			break;
		case RTK_PHYTYPE_RTL8261BE:
			RTKMSPHY_PRINT("Port %d  [RTL8261N(BE)] [Addr %d]", my_port_descp[i].mac_id, my_port_descp[i].phy_addr);
			break;
		case RTK_PHYTYPE_RTL8261B:
			RTKMSPHY_PRINT("Port %d  [RTL8261N(B)] [Addr %d]", my_port_descp[i].mac_id, my_port_descp[i].phy_addr);
			break;
		case RTK_PHYTYPE_RTL8251L_REV3:
			RTKMSPHY_PRINT("Port %d  [RTL8251L_REV3] [Addr %d]", my_port_descp[i].mac_id, my_port_descp[i].phy_addr);
			break;
		case RTK_PHYTYPE_RTL8224:
			RTKMSPHY_PRINT("Port %d  [RTL8224] [Addr %d]", my_port_descp[i].mac_id, my_port_descp[i].phy_addr);
			break;
		case RTK_PHYTYPE_NONE:
		default:
			RTKMSPHY_PRINT("Port %d  [UNKNOWN PHYTYPE] [Addr %d]", my_port_descp[i].mac_id, my_port_descp[i].phy_addr);
		}

		/* Serdes Mode */
		if ((ret = rtk_port_phyCtrl_get(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, RTK_PHY_CTRL_SERDES_MODE, &value)) == RT_ERR_OK) {
			switch (value) {
			case RTK_PHY_CTRL_SERDES_MODE_USXGMII:						/* 0x0000: USXGMII */
				RTKMSPHY_PRINT("\tSerdes Mode\t\t: USXGMII (0x%04x) %s", value, RTK_PHY_CTRL_SERDES_MODE_IS_AUTO(value) ? "FIX" : "AUTO");
				break;
			case RTK_PHY_CTRL_SERDES_MODE_10GR_5GX_2P5GX_SGMII:				/* 0x1001: 10G-R/5G-X/2.5G-X/SGMII */
				RTKMSPHY_PRINT("\tSerdes Mode\t\t: 10G-R/5G-X/2.5G-X/SGMII (0x%04x) %s", value, RTK_PHY_CTRL_SERDES_MODE_IS_AUTO(value) ? "FIX" : "AUTO");
				break;
			case RTK_PHY_CTRL_SERDES_MODE_10GR_XFI5GADAPT_XFI2P5GADAPT_SGMII:		/* 0x1002: 10G-R/XFI-5G-ADAPT/XFI-2.5G-ADAPT/SGMII */
				RTKMSPHY_PRINT("\tSerdes Mode\t\t: 10G-R/XFI-5G-ADAPT/XFI-2.5G-ADAPT/SGMII (0x%04x) %s", value, RTK_PHY_CTRL_SERDES_MODE_IS_AUTO(value) ? "FIX" : "AUTO");
				break;
			case RTK_PHY_CTRL_SERDES_MODE_10GR_5GR_2P5GX_SGMII:				/* 0x1101: 10G-R/5G-R/2.5G-X/SGMII */
				RTKMSPHY_PRINT("\tSerdes Mode\t\t: 10G-R/5G-R/2.5G-X/SGMII (0x%04x) %s", value, RTK_PHY_CTRL_SERDES_MODE_IS_AUTO(value) ? "FIX" : "AUTO");
				break;
			case RTK_PHY_CTRL_SERDES_MODE_10GR_XFI5GCPRI_2P5GX_SGMII:			/* 0x1102: 10G-R/XFI-5G-CPRI/2.5G-X/SGMII */
				RTKMSPHY_PRINT("\tSerdes Mode\t\t: 10G-R/XFI-5G-CPRI/2.5G-X/SGMII (0x%04x) %s", value, RTK_PHY_CTRL_SERDES_MODE_IS_AUTO(value) ? "FIX" : "AUTO");
				break;
			case RTK_PHY_CTRL_SERDES_MODE_UNKNOWN:						/* 0xFFFF */
			default:
				RTKMSPHY_PRINT("\tSerdes Mode\t\t: UNKNOWN (%d)", ret);
				break;
			}
		}
		else
			RTKMSPHY_PRINT("\tSerdes Mode\t\t: N/A (0x%04X)", ret);

		/* RTK_PHY_CTRL_SERDES_UPDTAE */
#if 0
		RTKMSPHY_INFO("[RTK_PHY_CTRL_SERDES_UPDTAE]");
		if ((ret = rtk_port_phyCtrl_get(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, RTK_PHY_CTRL_SERDES_UPDTAE, &value)) == RT_ERR_OK)
			RTKMSPHY_MSG("rtk_port_phyCtrl_get OK: value = 0x%X");
		else
			RTKMSPHY_ERROR("rtk_port_phyCtrl_get (0x%x) !!!", ret);
#endif
		/* Temperature */
		if ((ret = rtk_port_phyCtrl_get(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, RTK_PHY_CTRL_TEMP, &value)) == RT_ERR_OK) {
			if(value == 262144)
				temperature = -((2 << 19) - value) / 1024;
			else
				temperature = value / 1024;
			RTKMSPHY_PRINT("\tTemperature\t\t: %d C", temperature);
		}
		else
			RTKMSPHY_PRINT("\tTemperature\t\t: ERROR(%d)", ret);

		/* SERDES_RX_POLARITY */
		if ((ret = rtk_port_phyCtrl_get(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, RTK_PHY_CTRL_SERDES_RX_POLARITY, &value)) == RT_ERR_OK)
			RTKMSPHY_PRINT("\tSERDES_RX_POLARITY\t: %d", value);
		else
			RTKMSPHY_PRINT("\tSERDES_RX_POLARITY\t: ERROR(%d)", ret);

		/* SERDES_RX_POLARITY */
		if ((ret = rtk_port_phyCtrl_get(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, RTK_PHY_CTRL_SERDES_RX_POLARITY, &value)) == RT_ERR_OK)
			RTKMSPHY_PRINT("\tSERDES_RX_POLARITY\t: %d", value);
		else
			RTKMSPHY_PRINT("\tSERDES_RX_POLARITY\t: ERROR(%d)", ret);

		/* SERDES_TX_POLARITY */
		if ((ret = rtk_port_phyCtrl_get(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, RTK_PHY_CTRL_SERDES_TX_POLARITY, &value)) == RT_ERR_OK)
			RTKMSPHY_PRINT("\tSERDES_TX_POLARITY\t: %d", value);
		else
			RTKMSPHY_PRINT("\tSERDES_TX_POLARITY\t: ERROR(%d)", ret);

		/* LOOPBACK_REMOTE */
		if ((ret = rtk_port_phyCtrl_get(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, RTK_PHY_CTRL_LOOPBACK_REMOTE, &value)) == RT_ERR_OK)
			RTKMSPHY_PRINT("\tLOOPBACK_REMOTE\t\t: %d", value);
		else
			RTKMSPHY_PRINT("\tLOOPBACK_REMOTE\t\t: ERROR(%d)", ret);

		/* LOOPBACK_INTERNAL_PMA */
		if ((ret = rtk_port_phyCtrl_get(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, RTK_PHY_CTRL_LOOPBACK_INTERNAL_PMA, &value)) == RT_ERR_OK)
			RTKMSPHY_PRINT("\tLOOPBACK_INTERNAL_PMA\t: %d", value);
		else
			RTKMSPHY_PRINT("\tLOOPBACK_INTERNAL_PMA\t: ERROR(%d)", ret);

		/* MDI_POLARITY_SWAP */
		if ((ret = rtk_port_phyCtrl_get(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, RTK_PHY_CTRL_MDI_POLARITY_SWAP, &value)) == RT_ERR_OK)
			RTKMSPHY_PRINT("\tMDI_POLARITY_SWAP\t: %d", value);
		else
			RTKMSPHY_PRINT("\tMDI_POLARITY_SWAP\t: ERROR(%d)", ret);

		/* CTRL_MDI_INVERSE */
		if ((ret = rtk_port_phyCtrl_get(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, RTK_PHY_CTRL_MDI_INVERSE, &value)) == RT_ERR_OK)
			RTKMSPHY_PRINT("\tCTRL_MDI_INVERSE\t: %d", value);
		else
			RTKMSPHY_PRINT("\tCTRL_MDI_INVERSE\t: ERROR(%d)", ret);
		/* CTRL_EEE_PHYMODE */
		if ((ret = rtk_port_phyCtrl_get(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, RTK_PHY_CTRL_EEE_PHYMODE, &value)) == RT_ERR_OK)
			RTKMSPHY_PRINT("\tCTRL_EEE_PHYMODE\t: %d", value);
		else
			RTKMSPHY_PRINT("\tCTRL_EEE_PHYMODE\t: ERROR(%d)", ret);
	}

	return 0;
}

#define  RTK_PHY_CTRL_SERDES_MODE_OPT			"serdes"
#define  RTK_PHY_CTRL_SERDES_RX_POLARITY_OPT		"srp"
#define  RTK_PHY_CTRL_SERDES_TX_POLARITY_OPT		"stp"
#define  RTK_PHY_CTRL_LOOPBACK_REMOTE_OPT		"lr"
#define  RTK_PHY_CTRL_LOOPBACK_INTERNAL_PMA_OPT		"li"
#define  RTK_PHY_CTRL_MDI_POLARITY_SWAP_OPT		"mps"
#define  RTK_PHY_CTRL_MDI_INVERSE_OPT			"mi"
#define  RTK_PHY_CTRL_SERDES_UPDTAE_OPT			"su"
#define  RTK_PHY_CTRL_EEE_OPT				"eee"

#define  RTK_PHY_CTRL_VALUE_DONT_CARE			(0x0)

static int rtkmsphy_ctrl_set_fops(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	int32 ret;
	char tmpbuf[MAX_COMMAND_LEN] = {0};
	int len = (count > MAX_COMMAND_LEN) ? (MAX_COMMAND_LEN -1) : count;
	uint32 port, value;
	rtk_phy_ctrl_t ctrl_type;

	uint32 serdes_mode;	//= _rtk_rtl8261_proc_pasring_string_to_integer(buffer, count);
	const char *serdes_mode_info[] = {
		"USXGMII",
		"10G-R/5G-X/2.5G-X/SGMII",
		"10G-R/XFI-5G-ADAPT/XFI-2.5G-ADAPT/SGMII",
		"10G-R/5G-R/2.5G-X/SGMII",
		"10G-R/XFI-5G-CPRI/2.5G-X/SGMII",
		NULL
	};

	RTKMSPHY_DEBUG("%s - %s: %s (%d)", file->f_path.dentry->d_iname, __FUNCTION__, buffer, count);
	if (buffer) {
		char *strptr, *split_str, *ctrl_str;

		/* copy data to the buffer */
		strncpy(tmpbuf, buffer, len);
		tmpbuf[len] = '\0';

		strptr = tmpbuf;
		RTKMSPHY_MSG("Input: [%s]", strptr);

		/* parse command */
		ctrl_str = strsep(&strptr," ");
		split_str = strsep(&strptr," ");
		port = simple_strtoul(split_str, NULL, 0);
		split_str = strsep(&strptr," ");
		value = simple_strtoul(split_str, NULL, 0);

		RTKMSPHY_MSG("Parsing result: [%s][port %d][value %d]", ctrl_str, port, value);
		RTKMSPHY_MSG("================ [PORT %d] PHY_ADDR %d ================", port, rtkmsphy_get_phy_addr_by_mac_id(port));

		if(strcasecmp(ctrl_str, RTK_PHY_CTRL_SERDES_MODE_OPT) == 0) {
			ctrl_type = RTK_PHY_CTRL_SERDES_MODE;
			serdes_mode = value;	/* Save user's input and transfer to phy ctrl's serdes define */
			switch (value) {
			case 0:
				value = RTK_PHY_CTRL_SERDES_MODE_USXGMII;				/* 0x0000: USXGMII */
				break;
			case 1:
				value = RTK_PHY_CTRL_SERDES_MODE_10GR_5GX_2P5GX_SGMII;			/* 0x1001: 10G-R/5G-X/2.5G-X/SGMII */
				break;
			case 2:
				value = RTK_PHY_CTRL_SERDES_MODE_10GR_XFI5GADAPT_XFI2P5GADAPT_SGMII;	/* 0x1002: 10G-R/XFI-5G-ADAPT/XFI-2.5G-ADAPT/SGMII */
				break;
			case 3:
				value = RTK_PHY_CTRL_SERDES_MODE_10GR_5GR_2P5GX_SGMII;			/* 0x1101: 10G-R/5G-R/2.5G-X/SGMII */
				break;
			case 4:
				value = RTK_PHY_CTRL_SERDES_MODE_10GR_XFI5GCPRI_2P5GX_SGMII;		/* 0x1102: 10G-R/XFI-5G-CPRI/2.5G-X/SGMII */
				break;
			default:
				RTKMSPHY_ERROR("Unknown serdes mode %d, set to default serdes mode as 0", value);
				value = RTK_PHY_CTRL_SERDES_MODE_USXGMII;				/* 0x0000: USXGMII */
				value = 0;
				break;
			}
			RTKMSPHY_INFO("[RTK_PHY_CTRL_SERDES_MODE]: %s %s", serdes_mode_info[serdes_mode], RTK_PHY_CTRL_SERDES_MODE_IS_AUTO(value) ? "FIX" : "AUTO");
		}
		/* RTK_PHY_CTRL_SERDES_UPDTAE */
		else if(strcasecmp(ctrl_str, RTK_PHY_CTRL_SERDES_UPDTAE_OPT) == 0) {
			ctrl_type = RTK_PHY_CTRL_SERDES_UPDTAE;
			value = RTK_PHY_CTRL_VALUE_DONT_CARE;
			RTKMSPHY_INFO("[RTK_PHY_CTRL_SERDES_UPDTAE]");
		}
		/* RTK_PHY_CTRL_SERDES_RX_POLARITY */
		else if(strcasecmp(ctrl_str, RTK_PHY_CTRL_SERDES_RX_POLARITY_OPT) == 0) {
			ctrl_type = RTK_PHY_CTRL_SERDES_RX_POLARITY;
			RTKMSPHY_INFO("[RTK_PHY_CTRL_SERDES_RX_POLARITY]");
		}
		/* RTK_PHY_CTRL_SERDES_TX_POLARITY */
		else if(strcasecmp(ctrl_str, RTK_PHY_CTRL_SERDES_TX_POLARITY_OPT) == 0) {
			ctrl_type = RTK_PHY_CTRL_SERDES_TX_POLARITY;
			RTKMSPHY_INFO("[RTK_PHY_CTRL_SERDES_TX_POLARITY]");
		}
		/* RTK_PHY_CTRL_LOOPBACK_REMOTE */
		else if(strcasecmp(ctrl_str, RTK_PHY_CTRL_LOOPBACK_REMOTE_OPT) == 0) {
			ctrl_type = RTK_PHY_CTRL_LOOPBACK_REMOTE;
			RTKMSPHY_INFO("[RTK_PHY_CTRL_LOOPBACK_REMOTE]");
		}
		/* RTK_PHY_CTRL_LOOPBACK_INTERNAL_PMA */
		else if(strcasecmp(ctrl_str, RTK_PHY_CTRL_LOOPBACK_INTERNAL_PMA_OPT) == 0) {
			ctrl_type = RTK_PHY_CTRL_LOOPBACK_INTERNAL_PMA;
			RTKMSPHY_INFO("[RTK_PHY_CTRL_LOOPBACK_INTERNAL_PMA]");
		}
		/* RTK_PHY_CTRL_MDI_POLARITY_SWAP */
		else if(strcasecmp(ctrl_str, RTK_PHY_CTRL_MDI_POLARITY_SWAP_OPT) == 0) {
			ctrl_type = RTK_PHY_CTRL_MDI_POLARITY_SWAP;
			RTKMSPHY_INFO("[RTK_PHY_CTRL_MDI_POLARITY_SWAP]");
		}
		/* RTK_PHY_CTRL_MDI_INVERSE */
		else if(strcasecmp(ctrl_str, RTK_PHY_CTRL_MDI_INVERSE_OPT) == 0) {
			ctrl_type = RTK_PHY_CTRL_MDI_INVERSE;
			RTKMSPHY_INFO("[RTK_PHY_CTRL_MDI_INVERSE]");
		}
		/* RTK_PHY_CTRL_EEE */
		else if(strcasecmp(ctrl_str, RTK_PHY_CTRL_EEE_OPT) == 0) {
			ctrl_type = RTK_PHY_CTRL_EEE_PHYMODE;
			/* value 0 : DISABLED ; value 1 : ENABLED */
			RTKMSPHY_INFO("[RTK_PHY_CTRL_EEE]");
		}
		//else if(strcasecmp(ctrl_str, "ugan") == 0) {
		//}
		//else if(strcasecmp(ctrl_str, "sgan") == 0) {
		//}
		//else if(strcasecmp(ctrl_str, "ugan") == 0) {
		//}
		else {
			RTKMSPHY_ERROR("Unknown ctrl command [%s] !!!", ctrl_str);
		}

		/* Execution */
		if ((ret = rtk_port_phyCtrl_set(RTKMSPHY_DEFAULT_UNIT_ID, port, ctrl_type, value)) == RT_ERR_OK)
			RTKMSPHY_MSG(" - rtk_port_phyCtrl_set OK");
		else
			RTKMSPHY_ERROR(" - rtk_port_phyCtrl_set (0x%x) !!!", ret);
	}
	return count;
}

#if 0
uint64 old_p6_cnt_rx = 0;
uint64 old_p6_cnt_rx_err = 0;
uint64 old_p6_cnt_rx_crcerr = 0;
uint64 old_p6_cnt_ldpc_err = 0;

uint64 old_p7_cnt_rx = 0;
uint64 old_p7_cnt_rx_err = 0;
uint64 old_p7_cnt_rx_crcerr = 0;
uint64 old_p7_cnt_ldpc_err = 0;

static int rtkmsphy_cnt_get_fops(struct seq_file *s, void *v)
{
	int32 ret, i;
	uint64 cnt;
	//uint32 data0, data1, data2, data3;

	for (i = 0 ; my_port_descp[i].mac_id != HWP_END ; i++) {
		RTKMSPHY_MSG("================ [PORT %d] PHY_ADDR %d ================", my_port_descp[i].mac_id, rtkmsphy_get_phy_addr_by_mac_id(my_port_descp[i].mac_id));

		if ((ret = rtk_port_phyDbgCounter_get(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, PHY_DBG_CNT_RX, &cnt)) == RT_ERR_OK) {
			if (my_port_descp[i].mac_id == RTL8261_CA_XFI_PORT) {
				RTKMSPHY_MSG("PHY_DBG_CNT_RX :\t0x%016llx\t[0x%016llx]", (cnt - old_p6_cnt_rx), cnt);
				old_p6_cnt_rx = cnt;
			}
			else if (my_port_descp[i].mac_id == RTL8261_CA_PON_PORT) {
				RTKMSPHY_MSG("PHY_DBG_CNT_RX :\t0x%016llx\t[0x%016llx]", (cnt - old_p7_cnt_rx), cnt);
				old_p7_cnt_rx = cnt;
			}
		} else
			RTKMSPHY_ERROR("rtk_port_phyCtrl_get ERROR: PHY_DBG_CNT_RX (0x%x) !!!", ret);

		if ((ret = rtk_port_phyDbgCounter_get(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, PHY_DBG_CNT_RX_ERR, &cnt)) == RT_ERR_OK) {
			if (my_port_descp[i].mac_id == RTL8261_CA_XFI_PORT) {
				RTKMSPHY_MSG("PHY_DBG_CNT_RX_ERR :\t0x%016llx\t[0x%016llx]", (cnt - old_p6_cnt_rx_err), cnt);
				old_p6_cnt_rx_err = cnt;
			}
			else if (my_port_descp[i].mac_id == RTL8261_CA_PON_PORT) {
				RTKMSPHY_MSG("PHY_DBG_CNT_RX_ERR :\t0x%016llx\t[0x%016llx]", (cnt - old_p7_cnt_rx_err), cnt);
				old_p7_cnt_rx_err = cnt;
			}
		} else
			RTKMSPHY_ERROR("rtk_port_phyCtrl_get ERROR: PHY_DBG_CNT_RX_ERR (0x%x) !!!", ret);

		if ((ret = rtk_port_phyDbgCounter_get(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, PHY_DBG_CNT_RX_CRCERR, &cnt)) == RT_ERR_OK) {
			if (my_port_descp[i].mac_id == RTL8261_CA_XFI_PORT) {
				RTKMSPHY_MSG("PHY_DBG_CNT_RX_CRCERR : 0x%016llx\t[0x%016llx]", (cnt - old_p6_cnt_rx_crcerr), cnt);
				old_p6_cnt_rx_crcerr = cnt;
			}
			else if (my_port_descp[i].mac_id == RTL8261_CA_PON_PORT) {
				RTKMSPHY_MSG("PHY_DBG_CNT_RX_CRCERR : 0x%016llx\t[0x%016llx]", (cnt - old_p7_cnt_rx_crcerr), cnt);
				old_p7_cnt_rx_crcerr = cnt;
			}
		} else
			RTKMSPHY_ERROR("rtk_port_phyCtrl_get ERROR: PHY_DBG_CNT_RX_CRCERR (0x%x) !!!", ret);

		if ((ret = rtk_port_phyDbgCounter_get(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, PHY_DBG_CNT_LDPC_ERR, &cnt)) == RT_ERR_OK) {
			if (my_port_descp[i].mac_id == RTL8261_CA_XFI_PORT) {
				RTKMSPHY_MSG("PHY_DBG_CNT_LDPC_ERR :\t0x%016llx\t[0x%016llx]", (cnt - old_p6_cnt_ldpc_err),cnt);
				old_p6_cnt_ldpc_err = cnt;
			}
			else if (my_port_descp[i].mac_id == RTL8261_CA_PON_PORT) {
				RTKMSPHY_MSG("PHY_DBG_CNT_LDPC_ERR :\t0x%016llx\t[0x%016llx]", (cnt - old_p7_cnt_ldpc_err),cnt);
				old_p7_cnt_ldpc_err = cnt;
			}
		} else
			RTKMSPHY_ERROR("rtk_port_phyCtrl_get ERROR: PHY_DBG_CNT_LDPC_ERR (0x%x) !!!", ret);
#if 0
		/* enable */
		phy_hal_mmd_write(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 30, 0x210, 0x2);
		phy_hal_mmd_write(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 30, 0x230, 0x2);
		phy_hal_mmd_write(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 30, 0x250, 0x2);

		/* clear */
		phy_hal_mmd_write(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 30, 0x210, 0x3C2);
		phy_hal_mmd_write(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 30, 0x230, 0x3C2);
		phy_hal_mmd_write(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 30, 0x250, 0x3C2);

		/* read */
		/* RX0 */
		phy_hal_mmd_read(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 30, 0x247, &data0);
		phy_hal_mmd_read(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 30, 0x246, &data1);
		phy_hal_mmd_read(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 30, 0x245, &data2);

		RTKMSPHY_MSG("RX0 : 30.0x247 = %08x; 30.0x246 = %08x, 30.0x245 = %08x", data0, data1 , data2);

		/* RX1 */
		phy_hal_mmd_read(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 30, 0x267, &data0);
		phy_hal_mmd_read(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 30, 0x266, &data1);
		phy_hal_mmd_read(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 30, 0x265, &data2);

		RTKMSPHY_MSG("RX1 : 30.0x267 = %08x; 30.0x266 = %08x, 30.0x265 = %08x", data0, data1 , data2);

		/* TX0 */
		phy_hal_mmd_read(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 30, 0x227, &data0);
		phy_hal_mmd_read(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 30, 0x226, &data1);
		phy_hal_mmd_read(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 30, 0x225, &data2);

		RTKMSPHY_MSG("TX0 : 30.0x227 = %08x; 30.0x226 = %08x, 30.0x225 = %08x", data0, data1 , data2);

		phy_hal_mmd_read(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 31, 0xb202, &data0);
		phy_hal_mmd_read(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 31, 0xb282, &data1);
		phy_hal_mmd_read(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 31, 0xb302, &data2);
		phy_hal_mmd_read(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 31, 0xb382, &data3);

		RTKMSPHY_MSG("TX0 : 31.0xb202 = %08x; 31.0xb282 = %08x, 31.0xb302 = %08x, 31.0xb382 = %08x", data0, data1 , data2, data3);
#endif
	}

	return 0;
}

static int rtkmsphy_cnt_set_fops(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	RTKMSPHY_INFO("%s(%d): TODO", __FUNCTION__, __LINE__);

	return count;
}
#endif

#define RTK_PHY_PORT_SPEED_100M		"100M"
#define RTK_PHY_PORT_SPEED_1000M	"1G"
#define RTK_PHY_PORT_SPEED_2_5G		"2.5G"
#define RTK_PHY_PORT_SPEED_5G		"5G"
#define RTK_PHY_PORT_SPEED_10G		"10G"

static int rtkmsphy_ForceModeAbility_get_fops(struct seq_file *s, void *v)
{
	RTKMSPHY_INFO("%s(%d): TODO", __FUNCTION__, __LINE__);

	return 0;
}

static int rtkmsphy_ForceModeAbility_set_fops (struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	int32 ret;
	char tmpbuf[MAX_COMMAND_LEN] = {0};
	int len = (count > MAX_COMMAND_LEN) ? (MAX_COMMAND_LEN -1) : count;
	uint32 mac_id = 0;
	rtk_port_speed_t    speed;

	RTKMSPHY_DEBUG("%s - %s: %s (%d)", file->f_path.dentry->d_iname, __FUNCTION__, buffer, count);
	if (buffer) {
		char *strptr, *split_str, *port_speed;

		/* copy data to the buffer */
		strncpy(tmpbuf, buffer, len);
		tmpbuf[len] = '\0';

		strptr = tmpbuf;
		RTKMSPHY_MSG("Input: [%s]", strptr);

		/*parse command*/
		split_str = strsep(&strptr," ");
		mac_id = simple_strtoul(split_str, NULL, 0);
		port_speed = strsep(&strptr," ");
		RTKMSPHY_MSG("Parsing result: [port %d][speed %s]", mac_id, port_speed);

		RTKMSPHY_MSG("================ [PORT %d] PHY_ADDR %d ================", mac_id, rtkmsphy_get_phy_addr_by_mac_id(mac_id));
		if(strcasecmp(port_speed, RTK_PHY_PORT_SPEED_100M) == 0)		speed = PORT_SPEED_100M;
		else if(strcasecmp(port_speed, RTK_PHY_PORT_SPEED_1000M) == 0)		speed = PORT_SPEED_1000M;
		else if(strcasecmp(port_speed, RTK_PHY_PORT_SPEED_2_5G) == 0)		speed = PORT_SPEED_2_5G;
		else if(strcasecmp(port_speed, RTK_PHY_PORT_SPEED_5G) == 0)		speed = PORT_SPEED_5G;
		else if(strcasecmp(port_speed, RTK_PHY_PORT_SPEED_10G) == 0)		speed = PORT_SPEED_10G;
		else {
			RTKMSPHY_ERROR("UNKNOWN port speed %s", port_speed);
			return count;
		}

		if((ret = rtk_port_phyForceModeAbility_set(RTKMSPHY_DEFAULT_UNIT_ID, mac_id, speed, PORT_FULL_DUPLEX, ENABLED)) == RT_ERR_OK)
			RTKMSPHY_MSG("rtk_port_phyForceModeAbility_set OK");
		else
			RTKMSPHY_ERROR("rtk_port_phyForceModeAbility_set(0x%x) !!!", ret);
	}
	return count;
}

static int rtkmsphy_sdstestmode_get_fops(struct seq_file *s, void *v)
{
	RTKMSPHY_INFO("%s(%d): TODO", __FUNCTION__, __LINE__);

	return 0;
}

static int rtkmsphy_sdstestmode_set_fops(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	int32 ret;
	char tmpbuf[MAX_COMMAND_LEN] = {0};
	int len = (count > MAX_COMMAND_LEN) ? (MAX_COMMAND_LEN - 1) : count;
	uint32 sdsId, testMode;
	uint32 mac_id = 0;

	const char *serdes_test_mode_info[] = {
		"DISABLE",
		"PRBS7",
		"PRBS9",
		"PRBS11",
		"PRBS15",
		"PRBS23",
		"PRBS31",
		"SQUARE8",
		NULL
	};

	RTKMSPHY_DEBUG("%s - %s: %s (%d)", file->f_path.dentry->d_iname, __FUNCTION__, buffer, count);
	if (buffer) {
		char *strptr, *split_str;

		/* copy data to the buffer */
		strncpy(tmpbuf, buffer, len);
		tmpbuf[len] = '\0';

		strptr = tmpbuf;
		RTKMSPHY_DEBUG("strptr: [%s]", strptr);

		/*parse command*/
		split_str = strsep(&strptr," ");
		mac_id = simple_strtoul(split_str, NULL, 0);
		split_str = strsep(&strptr," ");
		sdsId = simple_strtoul(split_str, NULL, 0);
		split_str = strsep(&strptr," ");
		testMode = simple_strtoul(split_str, NULL, 0);
		RTKMSPHY_MSG("Parsing result: [port %d][sdsId %d][testMode %d]", mac_id, sdsId, testMode);
		RTKMSPHY_MSG("[Serdes ID]: Should be 0");
		RTKMSPHY_MSG("[Serdes Tese Mode]:\n\t0: %s\n\t1: %s\n\t2: %s\n\t3: %s\n\t4: %s\n\t5: %s\n\t6: %s\n\t7: %s",
			serdes_test_mode_info[0], serdes_test_mode_info[1], serdes_test_mode_info[2], serdes_test_mode_info[3],
			serdes_test_mode_info[4], serdes_test_mode_info[5], serdes_test_mode_info[6], serdes_test_mode_info[7]);

		RTKMSPHY_MSG("================ [PORT %d] PHY_ADDR %d ================", mac_id, rtkmsphy_get_phy_addr_by_mac_id(mac_id));

		if ((ret = rtk_port_phySdsTestMode_set(RTKMSPHY_DEFAULT_UNIT_ID, mac_id, sdsId, testMode)) == RT_ERR_OK) {
			RTKMSPHY_MSG("rtk_port_phySdsTestMode_set(%d, %d, %d, %d) [Seredes Mode: %s] OK",
				RTKMSPHY_DEFAULT_UNIT_ID, mac_id, sdsId, testMode, serdes_test_mode_info[testMode]);
		}
		else
			RTKMSPHY_ERROR("rtk_port_phySdsTestMode_set ERROR (ret = 0x%x) !!!", ret);
	}

	return count;
}

static int rtkmsphy_ieeetestmode_get_fops(struct seq_file *s, void *v)
{
	RTKMSPHY_INFO("%s(%d): TODO", __FUNCTION__, __LINE__);

	return 0;
}

static int rtkmsphy_ieeetestmode_set_fops(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	int32 ret;
	char tmpbuf[MAX_COMMAND_LEN] = {0};
	int len = (count > MAX_COMMAND_LEN) ? (MAX_COMMAND_LEN - 1) : count;
	uint32 testMode;
	uint32 mac_id = 0;
	rtk_port_phyTestMode_t phyTestMode;

	const char *phy_test_mode_info[] = {
		"1G_TEST_MODE_NONE",
		"1G_TEST_MODE1",
		"1G_TEST_MODE2",
		"1G_TEST_MODE3",
		"1G_TEST_MODE4",
		"2.5G/5G/10G_TEST_MODE_NONE",
		"2.5G/5G/10G_TEST_MODE1",
		"2.5G/5G/10G_TEST_MODE2",
		"2.5G/5G/10G_TEST_MODE3",
		"2.5G/5G/10G_TEST_MODE4_1",
		"2.5G/5G/10G_TEST_MODE4_2",
		"2.5G/5G/10G_TEST_MODE4_4",
		"2.5G/5G/10G_TEST_MODE4_5",
		"2.5G/5G/10G_TEST_MODE4_6",
		"2.5G/5G/10G_TEST_MODE4_DUALTONE1",
		"2.5G/5G/10G_TEST_MODE4_DUALTONE2",
		"2.5G/5G/10G_TEST_MODE4_DUALTONE3",
		"2.5G/5G/10G_TEST_MODE4_DUALTONE4",
		"2.5G/5G/10G_TEST_MODE4_DUALTONE5",
		"2.5G/5G/10G_TEST_MODE5",
		"2.5G/5G/10G_TEST_MODE6",
		"2.5G/5G/10G_TEST_MODE7",
		"100M_TEST_MODE_NONE",
		"100M_TEST_MODE_IEEE",
		"100M_TEST_MODE_ANSI_JITTER",
		"100M_TEST_MODE_ANSI_DROOP",
		NULL
	};

	RTKMSPHY_DEBUG("%s - %s: %s (%d)", file->f_path.dentry->d_iname, __FUNCTION__, buffer, count);
	if (buffer) {
		char *strptr, *split_str;

		/* copy data to the buffer */
		strncpy(tmpbuf, buffer, len);
		tmpbuf[len] = '\0';

		strptr = tmpbuf;
		RTKMSPHY_DEBUG("strptr: [%s]", strptr);

		/*parse command*/
		split_str = strsep(&strptr," ");
		mac_id = simple_strtoul(split_str, NULL, 0);
		split_str = strsep(&strptr," ");
		testMode = simple_strtoul(split_str, NULL, 0);
		RTKMSPHY_MSG("Parsing result: [port %d][testMode %d]", mac_id, testMode);
		RTKMSPHY_MSG("[PHY Test Mode]:");
		printk("\n\t00: %s\n\t01: %s\n\t02: %s\n\t03: %s\n\t04: %s\n\t05: %s"
			"\n\t06: %s\n\t07: %s\n\t08: %s\n\t09: %s\n\t10: %s\n\t11: %s\n\t12: %s\n\t13: %s\n\t14: %s\n\t15: %s"
			"\n\t16: %s\n\t17: %s\n\t18: %s\n\t19: %s\n\t20: %s\n\t21: %s\n\t22: %s\n\t23: %s\n\t24: %s\n\t25: %s",
			phy_test_mode_info[0], phy_test_mode_info[1], phy_test_mode_info[2], phy_test_mode_info[3],
			phy_test_mode_info[4], phy_test_mode_info[5], phy_test_mode_info[6], phy_test_mode_info[7],
			phy_test_mode_info[8], phy_test_mode_info[9], phy_test_mode_info[10], phy_test_mode_info[11],
			phy_test_mode_info[12], phy_test_mode_info[13], phy_test_mode_info[14], phy_test_mode_info[15],
			phy_test_mode_info[16], phy_test_mode_info[17], phy_test_mode_info[18], phy_test_mode_info[19],
			phy_test_mode_info[20], phy_test_mode_info[21], phy_test_mode_info[22], phy_test_mode_info[23],
			phy_test_mode_info[24], phy_test_mode_info[25]);

		if (testMode == 0)		phyTestMode.mode = RTK_PORT_PHY_1G_TEST_MODE_NONE;
		else if (testMode == 1)		phyTestMode.mode = RTK_PORT_PHY_1G_TEST_MODE1;
		else if (testMode == 2)		phyTestMode.mode = RTK_PORT_PHY_1G_TEST_MODE2;
		else if (testMode == 3)		phyTestMode.mode = RTK_PORT_PHY_1G_TEST_MODE3;
		else if (testMode == 4)		phyTestMode.mode = RTK_PORT_PHY_1G_TEST_MODE4;
		else if (testMode == 5)		phyTestMode.mode = RTK_PORT_PHY_10G_TEST_MODE_NONE;
		else if (testMode == 6)		phyTestMode.mode = RTK_PORT_PHY_10G_TEST_MODE1;
		else if (testMode == 7)		phyTestMode.mode = RTK_PORT_PHY_10G_TEST_MODE2;
		else if (testMode == 8)		phyTestMode.mode = RTK_PORT_PHY_10G_TEST_MODE3;
		else if (testMode == 9)		phyTestMode.mode = RTK_PORT_PHY_10G_TEST_MODE4_1;
		else if (testMode == 10)		phyTestMode.mode = RTK_PORT_PHY_10G_TEST_MODE4_2;
		else if (testMode == 11)		phyTestMode.mode = RTK_PORT_PHY_10G_TEST_MODE4_4;
		else if (testMode == 12)		phyTestMode.mode = RTK_PORT_PHY_10G_TEST_MODE4_5;
		else if (testMode == 13)		phyTestMode.mode = RTK_PORT_PHY_10G_TEST_MODE4_6;
		else if (testMode == 14)		phyTestMode.mode = RTK_PORT_PHY_10G_TEST_MODE4_DUALTONE1;
		else if (testMode == 15)		phyTestMode.mode = RTK_PORT_PHY_10G_TEST_MODE4_DUALTONE2;
		else if (testMode == 16)		phyTestMode.mode = RTK_PORT_PHY_10G_TEST_MODE4_DUALTONE3;
		else if (testMode == 17)		phyTestMode.mode = RTK_PORT_PHY_10G_TEST_MODE4_DUALTONE4;
		else if (testMode == 18)		phyTestMode.mode = RTK_PORT_PHY_10G_TEST_MODE4_DUALTONE5;
		else if (testMode == 19)		phyTestMode.mode = RTK_PORT_PHY_10G_TEST_MODE5;
		else if (testMode == 20)		phyTestMode.mode = RTK_PORT_PHY_10G_TEST_MODE6;
		else if (testMode == 21)		phyTestMode.mode = RTK_PORT_PHY_10G_TEST_MODE7;
		else if (testMode == 22)		phyTestMode.mode = RTK_PORT_PHY_100M_TEST_MODE_NONE;
		else if (testMode == 23)		phyTestMode.mode = RTK_PORT_PHY_100M_TEST_MODE_IEEE;
		else if (testMode == 24)		phyTestMode.mode = RTK_PORT_PHY_100M_TEST_MODE_ANSI_JITTER;
		else if (testMode == 25)		phyTestMode.mode = RTK_PORT_PHY_100M_TEST_MODE_ANSI_DROOP;
		else {
			RTKMSPHY_ERROR("UNKNOWN test mode %d", testMode);
			return count;
		}

		RTKMSPHY_MSG("================ [PORT %d] PHY_ADDR %d ================", mac_id, rtkmsphy_get_phy_addr_by_mac_id(mac_id));

		if ((ret = rtk_port_phyIeeeTestMode_set(RTKMSPHY_DEFAULT_UNIT_ID, mac_id, &phyTestMode)) == RT_ERR_OK) {
			RTKMSPHY_MSG("rtk_port_phyIeeeTestMode_set(%d, %d, 0x%04x) [PHY Test Mode: %s] OK",
				RTKMSPHY_DEFAULT_UNIT_ID, mac_id, phyTestMode.mode, phy_test_mode_info[testMode]);
		}
		else
			RTKMSPHY_ERROR("rtk_port_phyIeeeTestMode_set ERROR (ret = 0x%x) !!!", ret);
	}

	return count;
}


static int rtkmsphy_SdsEyeParam_get_fops(struct seq_file *s, void *v)
{
	int32 i, ret, sdsId = 0;
	rtk_sds_eyeParam_t eyeParam;

	for (i = 0 ; my_port_descp[i].mac_id != HWP_END ; i++) {
		RTKMSPHY_MSG("================ [PORT %d] PHY_ADDR %d ================", my_port_descp[i].mac_id, my_port_descp[i].mac_id);
		if ((ret = rtk_port_phySdsEyeParam_get(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, sdsId, &eyeParam)) == RT_ERR_OK){
			RTKMSPHY_MSG("rtk_port_phySdsEyeParam_get OK: unit [%d], phyAddr [%d], sdsId [%d]", RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, sdsId);
			RTKMSPHY_MSG("\timpedance : 0x%0x", eyeParam.impedance);
			RTKMSPHY_MSG("\tpre_amp : 0x%0x", eyeParam.pre_amp);
			RTKMSPHY_MSG("\tmain_amp : 0x%0x", eyeParam.main_amp);
			RTKMSPHY_MSG("\tpost_amp : 0x%0x", eyeParam.post_amp);
			RTKMSPHY_MSG("\tpost2_amp : 0x%0x", eyeParam.post2_amp);
			RTKMSPHY_MSG("\tpre_en : %s", eyeParam.pre_en ? "Enabled" : "Disabled");
			RTKMSPHY_MSG("\tpost_en : %s", eyeParam.post_en ? "Enabled" : "Disabled");
			RTKMSPHY_MSG("\tpost2_en : %s", eyeParam.post2_en ? "Enabled" : "Disabled");
			RTKMSPHY_MSG("\tmain_en : %s", eyeParam.main_en ? "Enabled" : "Disabled");
		}
		else
			RTKMSPHY_ERROR("rtk_port_phySdsEyeParam_get ERROR (ret = 0x%x) !!!", ret);
	}

	return 0;
}

static struct task_struct *rtl261_port_phy_sds_eye_task;
uint32 eye_port, eye_sdsId, eye_frameNum = 0;
rtk_sds_eyeParam_t eyeParam;
int32 rtkmsphy_port_phy_sds_eye_status = 1; //0: stop, 1:start

static int rtkmsphy_port_phy_sds_eye(void *data)
{
	int32 i, ret;

	for (i = 0 ; my_port_descp[i].mac_id != HWP_END ; i++) {
		if (my_port_descp[i].mac_id == eye_port) {
			RTKMSPHY_MSG("================ [PORT %d] PHY_ADDR %d ================", my_port_descp[i].mac_id, my_port_descp[i].phy_addr);
			if ((ret = rtk_port_phySdsEyeParam_set(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, eye_sdsId, &eyeParam)) == RT_ERR_OK){
				RTKMSPHY_MSG("rtk_port_phySdsEyeParam_set OK: unit [%d], phyAddr [%d], sdsId [%d]", RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, eye_sdsId);
			}
			else {
				RTKMSPHY_ERROR("rtk_port_phySdsEyeParam_set ERROR (ret = 0x%x) !!!", ret);
				rtkmsphy_port_phy_sds_eye_status = 0;
			}

			if((ret = rtk_port_phyEyeMonitor_start(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, eye_sdsId, eye_frameNum)) == RT_ERR_OK)
				RTKMSPHY_MSG("rtk_port_phyEyeMonitor_start(%d, %d, %d, %d)", RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, eye_sdsId, eye_frameNum);
			else {
				RTKMSPHY_ERROR("rtk_port_phyEyeMonitor_start ERROR(0x%x) !!!", ret);
				rtkmsphy_port_phy_sds_eye_status = 0;
			}
		}
		if (rtkmsphy_port_phy_sds_eye_status == 0) {
			RTKMSPHY_ERROR("eye monitor stop.");
			return RT_ERR_OK;
		}
	}

	RTKMSPHY_MSG("eye monitor done.");
	return RT_ERR_OK;
}

static int rtkmsphy_SdsEyeParam_set_fops(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	//int32 i, ret;
	//uint32 sdsId, frameNum = 0;
	char tmpbuf[MAX_COMMAND_LEN] = {0};
	int len = (count > MAX_COMMAND_LEN) ? (MAX_COMMAND_LEN -1) : count;
	//rtk_sds_eyeParam_t eyeParam;

	RTKMSPHY_DEBUG("%s - %s: %s (%d)", file->f_path.dentry->d_iname, __FUNCTION__, buffer, count);
	if (buffer) {
		char *strptr, *split_str;

		/* copy data to the buffer */
		strncpy(tmpbuf, buffer, len);
		tmpbuf[len] = '\0';

		strptr = tmpbuf;
		RTKMSPHY_MSG("Input: [%s]", strptr);

		/*parse command*/
		split_str = strsep(&strptr," ");
		eye_port = simple_strtoul(split_str, NULL, 0);
		split_str = strsep(&strptr," ");
		eye_sdsId = simple_strtoul(split_str, NULL, 0);
		split_str = strsep(&strptr," ");
		eye_frameNum = simple_strtoul(split_str, NULL, 0);

		split_str = strsep(&strptr," ");
		eyeParam.impedance = simple_strtoul(split_str, NULL, 0);
		split_str = strsep(&strptr," ");
		eyeParam.pre_amp = simple_strtoul(split_str, NULL, 0);
		split_str = strsep(&strptr," ");
		eyeParam.main_amp = simple_strtoul(split_str, NULL, 0);
		split_str = strsep(&strptr," ");
		eyeParam.post_amp = simple_strtoul(split_str, NULL, 0);
		split_str = strsep(&strptr," ");
		eyeParam.post2_amp = simple_strtoul(split_str, NULL, 0);

		split_str = strsep(&strptr," ");
		eyeParam.pre_en = simple_strtoul(split_str, NULL, 0);
		split_str = strsep(&strptr," ");
		eyeParam.post_en = simple_strtoul(split_str, NULL, 0);
		split_str = strsep(&strptr," ");
		eyeParam.post2_en = simple_strtoul(split_str, NULL, 0);
		split_str = strsep(&strptr," ");
		eyeParam.main_en = simple_strtoul(split_str, NULL, 0);

		RTKMSPHY_MSG("Parsing result:");
		RTKMSPHY_MSG("\tport : 0x%0x", eye_port);
		RTKMSPHY_MSG("\tsdsId : 0x%0x", eye_sdsId);
		RTKMSPHY_MSG("\tframeNum : 0x%0x", eye_frameNum);
		RTKMSPHY_MSG("\timpedance : 0x%0x", eyeParam.impedance);
		RTKMSPHY_MSG("\tpre_amp : 0x%0x", eyeParam.pre_amp);
		RTKMSPHY_MSG("\tmain_amp : 0x%0x", eyeParam.main_amp);
		RTKMSPHY_MSG("\tpost_amp : 0x%0x", eyeParam.post_amp);
		RTKMSPHY_MSG("\tpost2_amp : 0x%0x", eyeParam.post2_amp);
		RTKMSPHY_MSG("\tpre_en : %s", eyeParam.pre_en ? "Enabled" : "Disabled");
		RTKMSPHY_MSG("\tpost_en : %s", eyeParam.post_en ? "Enabled" : "Disabled");
		RTKMSPHY_MSG("\tpost2_en : %s", eyeParam.post2_en ? "Enabled" : "Disabled");
		RTKMSPHY_MSG("\tmain_en : %s", eyeParam.main_en ? "Enabled" : "Disabled");

		rtl261_port_phy_sds_eye_task = kthread_create(rtkmsphy_port_phy_sds_eye, NULL, "rtkmsphy_port_phy_sds_eye");
		if (WARN_ON(!rtl261_port_phy_sds_eye_task)) {
			RTKMSPHY_ERROR("error: create rtl261_port_phy_sds_eye_task thread fail.");
			return count;
		}
		kthread_bind(rtl261_port_phy_sds_eye_task, 0);
		wake_up_process(rtl261_port_phy_sds_eye_task);

		rtkmsphy_port_phy_sds_eye_status = 1;

		RTKMSPHY_MSG("eye monitor started.");
	}

	return count;
}

#if 0
static int rtkmsphy_phyEyeMonitor_get_fops(struct seq_file *s, void *v)
{

	RTKMSPHY_ERROR("rtkmsphy_phyEyeMonitor_get_fops is NOT implemented !!!");
#if 0
	int32 i, ret;
	uint32 sdsId = 0, frameNum = 0;

	for (i = 0 ; my_port_descp[i].mac_id != HWP_END ; i++) {
		RTKMSPHY_MSG("================ [PORT %d] PHY_ADDR %d ================", my_port_descp[i].mac_id, my_port_descp[i].phy_addr);
		if((ret = rtk_port_phyEyeMonitor_start(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, sdsId, frameNum)) == RT_ERR_OK)
			RTKMSPHY_MSG("rtk_port_phyEyeMonitor_start(%d, %d, %d, %d)", RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, sdsId, frameNum);
		else
			RTKMSPHY_ERROR("rtk_port_phyEyeMonitor_start ERROR(0x%x) !!!", ret);
	}
#endif
	return 0;
}
#endif

static int rtkmsphy_phyEyeMonitor_set_fops(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	int32 i, ret;
	uint32 mac_id = 0, sdsId = 0, frameNum = 0;

	char tmpbuf[MAX_COMMAND_LEN] = {0};
	int len = (count > MAX_COMMAND_LEN) ? (MAX_COMMAND_LEN -1) : count;
	//rtk_sds_eyeParam_t eyeParam;
	rtk_sds_eyeMonInfo_t info;

	RTKMSPHY_DEBUG("%s - %s: %s (%d)", file->f_path.dentry->d_iname, __FUNCTION__, buffer, count);
	if (buffer) {
		char *strptr, *split_str;

		/* copy data to the buffer */
		strncpy(tmpbuf, buffer, len);
		tmpbuf[len] = '\0';

		strptr = tmpbuf;
		RTKMSPHY_MSG("Input: [%s]", strptr);

		/*parse command*/
		split_str = strsep(&strptr," ");
		mac_id = simple_strtoul(split_str, NULL, 0);
		split_str = strsep(&strptr," ");
		sdsId = simple_strtoul(split_str, NULL, 0);
		split_str = strsep(&strptr," ");
		frameNum = simple_strtoul(split_str, NULL, 0);

		RTKMSPHY_MSG("Parsing result:");
		RTKMSPHY_MSG("\tport : 0x%0x", mac_id);
		RTKMSPHY_MSG("\tsdsId : 0x%0x", sdsId);
		RTKMSPHY_MSG("\tframeNum : 0x%0x", frameNum);

		for (i = 0 ; my_port_descp[i].mac_id != HWP_END ; i++) {
			if (my_port_descp[i].mac_id == mac_id) {
				RTKMSPHY_MSG("================ [PORT %d] PHY_ADDR %d ================", my_port_descp[i].mac_id, rtkmsphy_get_phy_addr_by_mac_id(my_port_descp[i].mac_id));
				if ((ret = rtk_port_phyEyeMonitorInfo_get(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, sdsId, frameNum, &info)) == RT_ERR_OK)
					RTKMSPHY_MSG("rtk_port_phyEyeMonitorInfo_get (%d, %d, %d, %d) OK", RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, sdsId, frameNum);
				else
					RTKMSPHY_ERROR("rtk_port_phyEyeMonitorInfo_get ERROR (ret = 0x%x) !!!", ret);

				RTKMSPHY_MSG("PHY Eye Monitor Info:");
				RTKMSPHY_MSG("\tHeight : 0x%0x", info.height);
				RTKMSPHY_MSG("\tWidth  : 0x%0x", info.width);
			}
		}
	}

	return count;
}


typedef enum rtkmsphy_procDir_e
{
	PROC_DIR_RTKMSPHY_ROOT,
	PROC_DIR_RTKMSPHY_LEEF //this field must put at last.
} rtkmsphy_procDir_t;

typedef struct rtkmsphy_proc_s
{
	char *name;
	int (*get) (struct seq_file *s, void *v);
	int (*set) (struct file *file, const char __user *buffer, size_t count, loff_t *ppos);
	unsigned int inode_id;
	unsigned char unlockBefortWrite;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
	struct proc_ops proc_fops;
#else
	struct file_operations proc_fops;
#endif
	rtkmsphy_procDir_t dir;
}rtkmsphy_proc_t;

rtkmsphy_proc_t rtkmsphyProc[]=
{
	{
		.name= PROC_FILE_HELP,
		.get = help_fops,
		.set = NULL,
		.dir = PROC_DIR_RTKMSPHY_ROOT,
	},
#if 0
	{
		.name= PROC_FILE_PHY,
		.get = NULL,
		.set = Rtkmsphy_phy_fops,
		.dir = PROC_DIR_RTKMSPHY_ROOT,
	},
#endif
	{
		.name= PROC_FILE_LINKSTATUS,
		.get = rtkmsphy_link_status_fops,
		.set = NULL,
		.dir = PROC_DIR_RTKMSPHY_ROOT,
	},
	{
		.name= PROC_FILE_TEMPERATURE,
		.get = rtkmsphy_temperature_get_fops,
		.set = NULL,
		.dir = PROC_DIR_RTKMSPHY_ROOT,
	},
	{
		.name= PROC_FILE_CTRL,
		.get = rtkmsphy_ctrl_get_fops,
		.set = rtkmsphy_ctrl_set_fops,
		.dir = PROC_DIR_RTKMSPHY_ROOT,
	},
#if 0
	{
		.name= PROC_FILE_CNT,
		.get = rtkmsphy_cnt_get_fops,
		.set = rtkmsphy_cnt_set_fops,
		.dir = PROC_DIR_RTKMSPHY_ROOT,
	},
#endif
	{
		.name= PROC_FILE_FORCEMODEABILITY,
		.get = rtkmsphy_ForceModeAbility_get_fops,
		.set = rtkmsphy_ForceModeAbility_set_fops,
		.dir = PROC_DIR_RTKMSPHY_ROOT,
	},
	{
		.name= PROC_FILE_SDSTESTMODE,
		.get = rtkmsphy_sdstestmode_get_fops,
		.set = rtkmsphy_sdstestmode_set_fops,
		.dir = PROC_DIR_RTKMSPHY_ROOT,
	},
	{
		.name= PROC_FILE_IEEETESTMODE,
		.get = rtkmsphy_ieeetestmode_get_fops,
		.set = rtkmsphy_ieeetestmode_set_fops,
		.dir = PROC_DIR_RTKMSPHY_ROOT,
	},
	{
		.name= PROC_FILE_SDSEYEPARAM,
		.get = rtkmsphy_SdsEyeParam_get_fops,
		.set = rtkmsphy_SdsEyeParam_set_fops,
		.dir = PROC_DIR_RTKMSPHY_ROOT,
	},
	{
		.name= PROC_FILE_PHYEYEMONITOR,
		.get = NULL,	// rtkmsphy_phyEyeMonitor_get_fops,
		.set = rtkmsphy_phyEyeMonitor_set_fops,
		.dir = PROC_DIR_RTKMSPHY_ROOT,
	},
};

/* rtkmsphy common proc function */
static void *rtkmsphy_single_start(struct seq_file *p, loff_t *pos)
{
	return NULL + (*pos == 0);
}

static void *rtkmsphy_single_next(struct seq_file *p, void *v, loff_t *pos)
{
	++*pos;
	return NULL;
}

static void rtkmsphy_single_stop(struct seq_file *p, void *v)
{
}

int rtkmsphy_seq_open(struct file *file, const struct seq_operations *op)
{
	struct seq_file *p = file->private_data;

	if (!p) {
		p = kmalloc(sizeof(*p), GFP_ATOMIC);
		if (!p){
			return -ENOMEM;
		}
		file->private_data = p;
	}
	memset(p, 0, sizeof(*p));
	mutex_init(&p->lock);
	p->op = op;
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 4, 79)
	p->file = file;
#else
#ifdef CONFIG_USER_NS
	p->user_ns = file->f_cred->user_ns;
#endif
#endif
	/*
	 * Wrappers around seq_open(e.g. swaps_open) need to be
	 * aware of this. If they set f_version themselves, they
	 * should call seq_open first and then set f_version.
	 */
	file->f_version = 0;

	/*
	 * seq_files support lseek() and pread().  They do not implement
	 * write() at all, but we clear FMODE_PWRITE here for historical
	 * reasons.
	 *
	 * If a client of seq_files a) implements file.write() and b) wishes to
	 * support pwrite() then that client will need to implement its own
	 * file.open() which calls seq_open() and then sets FMODE_PWRITE.
	 */
	file->f_mode &= ~FMODE_PWRITE;
	return 0;
}


int rtkmsphy_single_open(struct file *file, int (*show) (struct seq_file *m, void *v), void *data)
{
	struct seq_operations *op = kmalloc(sizeof(*op), GFP_ATOMIC);
	int res = -ENOMEM;

	if (op) {
		op->start = rtkmsphy_single_start;
		op->next = rtkmsphy_single_next;
		op->stop = rtkmsphy_single_stop;
		op->show = show;
		res = rtkmsphy_seq_open(file, op);
		if (!res)
			((struct seq_file *)file->private_data)->private = data;
		else
			kfree(op);
	}
	return res;
}

static int rtkmsphy_nullDebugGet(struct seq_file *s, void *v)
{
	return 0;
}

static int rtkmsphy_nullDebugSingleOpen(struct inode *inode, struct file *file)
{
       return(single_open(file, rtkmsphy_nullDebugGet, NULL));
}

static int rtkmsphy_commonDebugSingleOpen(struct inode *inode, struct file *file)
{
	int i, ret = -1;

	//rtkmsphy_spin_lock_bh(rtkmsphySysdb.lock_rtkmsphy);
	//========================= Critical Section Start =========================//
	for( i = 0; i < (sizeof(rtkmsphyProc) / sizeof(rtkmsphy_proc_t)) ; i++) {
		//RTKMSPHY_MSG("common_single_open inode_id=%u i_ino=%u", rtkmsphyProc[i].inode_id,(unsigned int)inode->i_ino);

		if(rtkmsphyProc[i].inode_id == (unsigned int)inode->i_ino) {
			ret = rtkmsphy_single_open(file, rtkmsphyProc[i].get, NULL);
			break;
		}
	}
	//========================= Critical Section End =========================//
	//rtkmsphy_spin_unlock_bh(rtkmsphySysdb.lock_rtkmsphy);

	return ret;
}

static ssize_t rtkmsphy_commonDebugSingleWrite(struct file * file, const char __user * userbuf,
				size_t count, loff_t * off)
{
	int i, ret = -1;
	char procBuffer[count];
	char *pBuffer = NULL;

	/* write data to the buffer */
	memset(procBuffer, 0, sizeof(procBuffer));
	if (copy_from_user(procBuffer, userbuf, count)) {
		return -EFAULT;
	}
	procBuffer[count-1] = '\0';
	pBuffer = procBuffer;

	//rtkmsphy_spin_lock_bh(rtkmsphySysdb.lock_rtkmsphy);
	//========================= Critical Section Start =========================//
	for( i = 0; i < (sizeof(rtkmsphyProc) / sizeof(rtkmsphy_proc_t)) ; i++) {
		//RTKMSPHY_MSG("common_single_write inode_id=%u i_ino=%u",rtkmsphyProc[i].inode_id,(unsigned int)file->f_dentry->d_inode->i_ino);

		if(rtkmsphyProc[i].inode_id == (unsigned int)file->f_inode->i_ino) {
			//if(rtkmsphyProc[i].unlockBefortWrite)
			//	rtkmsphy_spin_unlock_bh(rtkmsphySysdb.lock_rtkmsphy);
			ret = rtkmsphyProc[i].set(file, pBuffer, count, off);
			break;
		}
	}
	//========================= Critical Section End =========================//
	//if((i!=(sizeof(rtkmsphyProc)/sizeof(rtkmsphy_proc_t))) && !rtkmsphyProc[i].unlockBefortWrite)
	//	rtkmsphy_spin_unlock_bh(rtkmsphySysdb.lock_rtkmsphy);

	return ret;
}

int rtkmsphy_proc_init(void)
{
	int i = 0;
	struct proc_dir_entry *p = NULL;

	rtkmsphy_proc_dir = proc_mkdir(PROC_DIR_RTKMSPHY, NULL);
	if (rtkmsphy_proc_dir == NULL) {
		RTKMSPHY_ERROR("create /proc/%s failed!", PROC_DIR_RTKMSPHY);
		return 0;
	}

	for(i = 0; i < (sizeof(rtkmsphyProc)/sizeof(rtkmsphy_proc_t)) ; i++)
	{
		struct proc_dir_entry *parentDir = NULL;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
		if(rtkmsphyProc[i].get == NULL)
			rtkmsphyProc[i].proc_fops.proc_open = rtkmsphy_nullDebugSingleOpen;
		else
			rtkmsphyProc[i].proc_fops.proc_open = rtkmsphy_commonDebugSingleOpen;

		if(rtkmsphyProc[i].set == NULL)
			rtkmsphyProc[i].proc_fops.proc_write = NULL;
		else
			rtkmsphyProc[i].proc_fops.proc_write = rtkmsphy_commonDebugSingleWrite;

		rtkmsphyProc[i].proc_fops.proc_read = seq_read;
		rtkmsphyProc[i].proc_fops.proc_lseek = seq_lseek;
		rtkmsphyProc[i].proc_fops.proc_release = single_release;
#else
		if(rtkmsphyProc[i].get == NULL)
			rtkmsphyProc[i].proc_fops.open = rtkmsphy_nullDebugSingleOpen;
		else
			rtkmsphyProc[i].proc_fops.open = rtkmsphy_commonDebugSingleOpen;

		if(rtkmsphyProc[i].set == NULL)
			rtkmsphyProc[i].proc_fops.write = NULL;
		else
			rtkmsphyProc[i].proc_fops.write = rtkmsphy_commonDebugSingleWrite;

		rtkmsphyProc[i].proc_fops.read = seq_read;
		rtkmsphyProc[i].proc_fops.llseek = seq_lseek;
		rtkmsphyProc[i].proc_fops.release = single_release;
#endif

		switch(rtkmsphyProc[i].dir)
		{
			case PROC_DIR_RTKMSPHY_ROOT:
				parentDir = rtkmsphy_proc_dir;
				break;
			default:
				break;
		}

		p = proc_create_data(rtkmsphyProc[i].name, S_IRUGO, parentDir, &(rtkmsphyProc[i].proc_fops),NULL);
		if(!p){
			RTKMSPHY_ERROR("create proc %s failed!", rtkmsphyProc[i].name);
		}
		rtkmsphyProc[i].inode_id = p->low_ino;
	}
	RTKMSPHY_MSG("Creat %d proc entry.", i);

	return 1;
}

void rtkmsphy_proc_exit(void)
{
	int i = 0;

	for(i = 0 ; i < (sizeof(rtkmsphyProc) / sizeof(rtkmsphy_proc_t)) ; i++)
	{
		struct proc_dir_entry *parentDir = NULL;

		switch(rtkmsphyProc[i].dir)
		{
			case PROC_DIR_RTKMSPHY_ROOT:
				parentDir = rtkmsphy_proc_dir;
				break;
			default:
				break;
		}

		remove_proc_entry(rtkmsphyProc[i].name, parentDir);
	}

	proc_remove(rtkmsphy_proc_dir);
}


/* Function Name:
 *      _phy_rtk_oui_chk
 * Description:
 *      Identify the OUI is the realtek OUI or not?
 * Input:
 *      reg2_data - device identifier reg 2 data
 *      reg3_data - device identifier reg 3 data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - Realtek OUI
 *      RT_ERR_FAILED - not Realtek OUI
 * Note:
 *      None
 */
int32
__phy_rtk_oui_chk(uint32 reg2_data, uint32 reg3_data)
{
	if ((reg2_data != PHY_IDENT_OUI_03_18) ||
		((reg3_data >> OUI_19_24_OFFSET) != PHY_IDENT_OUI_19_24))
	{
		return RT_ERR_FAILED;
	}

	return RT_ERR_OK;
}

/* Function Name:
 *      _phy_identify_default_c45
 * Description:
 *      Identify the port is match input PHY information or not for clause 45 PHY
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      model_id - model id
 *      rev_id   - revision id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - is match the PHY information
 *      RT_ERR_FAILED           - access failure or others
 *      RT_ERR_PHY_NOT_MATCH    - is not match the PHY information
 * Note:
 *      None
 */
int32
__phy_identify_default_c45(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id)
{
	int32       ret;
	uint32      reg2_data = 0, reg3_data = 0;
	uint32      chip_model_id, chip_rev_id;
	//hal_control_t   *pHalCtrl = NULL;

	//RT_PARAM_CHK((NULL == (pHalCtrl = hal_ctrlInfo_get(unit))), RT_ERR_FAILED);
	//RT_PARAM_CHK((NULL == MACDRV(pHalCtrl)->fMdrv_miim_read), RT_ERR_FAILED);

	//MACDRV(pHalCtrl)->fMdrv_miim_mmd_read(unit, port, 1, 2, &reg2_data);
	//ret = MACDRV(pHalCtrl)->fMdrv_miim_mmd_read(unit, port, 1, 3, &reg3_data);

#ifdef CONFIG_LUNA_G3_SERIES
	//phy_hal_mmd_read(unit, port, 1, 0x2, &reg2_data);
	aal_mdio_read(unit, 0, port, 1, 0x2, (ca_uint16_t *)&reg2_data);
	RTKMSPHY_DEBUG(" - PhyAddr-%d: Read MMD1.0x2 = 0x%0X", port, reg2_data);
	//phy_hal_mmd_read(unit, port, 1, 0x3, &reg3_data);
	aal_mdio_read(unit, 0, port, 1, 0x3, (ca_uint16_t *)&reg3_data);
	RTKMSPHY_DEBUG(" - PhyAddr-%d: Read MMD1.0x3 = 0x%0X", port, reg3_data);
#else
	RTKMSPHY_ERROR("Only Support G3 platform!!");
	return RT_ERR_PORT_NOT_SUPPORTED;
#endif
	if ((ret = __phy_rtk_oui_chk(reg2_data, reg3_data)) != RT_ERR_OK)
	{
		RTKMSPHY_ERROR("Compare OUI reg2_data failed(unit %d, port %d): reg2_data= 0x%x; PHY_IDENT_OUI_03_18 = 0x%x!!", unit, port, reg2_data, PHY_IDENT_OUI_03_18);
		RTKMSPHY_ERROR("Compare OUI reg3_data failed(unit %d, port %d): reg3_data.b[24:19]= 0x%x; PHY_IDENT_OUI_19_24 = 0x%x!!", unit, port, (reg3_data >> OUI_19_24_OFFSET), PHY_IDENT_OUI_19_24);
		return ret;
	}

	chip_model_id = PHY_IDENT_MODEL_NUM(reg3_data);
	chip_rev_id   = PHY_IDENT_REV_NUM(reg3_data);

	if ((chip_model_id == model_id) && (chip_rev_id >= rev_id))
	{
		return RT_ERR_OK;
	}

	return RT_ERR_PHY_NOT_MATCH;
}

static int32
__phy_identify_8261(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id)
{
	//hal_control_t   *pHalCtrl = NULL;
	int32           ret;
	uint32          reg_data1 = 0;
	uint32          reg_data2 = 0;

	//RT_PARAM_CHK((NULL == (pHalCtrl = hal_ctrlInfo_get(unit))), RT_ERR_FAILED);

	if (port >= RTK_MAX_PORT_PER_UNIT)
	{
		return RT_ERR_PHY_NOT_MATCH;
	}

	if ((ret = __phy_identify_default_c45(unit, port, model_id, rev_id)) != RT_ERR_OK)
	{
		return ret;
	}

#if 1
	//if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_mmd_read(unit, port, 30, 0x103, &reg_data1)) != RT_ERR_OK)
	//    return ret;
	//if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_mmd_read(unit, port, 30, 0x104, &reg_data2)) != RT_ERR_OK)
	//    return ret;

#ifdef CONFIG_LUNA_G3_SERIES
	//phy_hal_mmd_read(unit, port, 30, 0x103, &reg_data1);
	aal_mdio_read(unit, 0, port, 30, 0x103, (ca_uint16_t *)&reg_data1);
	RTKMSPHY_DEBUG(" - PhyAddr-%d: Read MMD30.0x103  = 0x%0X", port, reg_data1);
	//phy_hal_mmd_read(unit, port, 30, 0x104, &reg_data2);
	aal_mdio_read(unit, 0, port, 30, 0x104, (ca_uint16_t *)&reg_data2);
	RTKMSPHY_DEBUG(" - PhyAddr-%d: Read MMD30.0x104  = 0x%0X", port, reg_data2);
#else
	RTKMSPHY_ERROR("Only Support G3 platform!!");
	return RT_ERR_PORT_NOT_SUPPORTED;
#endif

	reg_data2 = REG32_FIELD_GET(reg_data2, 11, 0xF800);

	if (reg_data1 == 0x8291 && reg_data2 == 0x1F)
	{
		return RT_ERR_OK;
	}
	else
	{
		return RT_ERR_PHY_NOT_MATCH;
	}
#else
	if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_mmd_read(unit, port, 30, 0x101, &reg_data)) != RT_ERR_OK)
		return ret;
	if ((reg_data & 0xF000) != 0xB000)
	{
		reg_data &= (~(0xF000));
		reg_data |= (0xB000);
		if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_mmd_write(unit, port, 30, 0x101, reg_data)) != RT_ERR_OK)
			return ret;
	}
	if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_mmd_read(unit, port, 30, 0x102, &reg_data)) != RT_ERR_OK)
	{
		return ret;
	}
	if ((reg_data & 0x60) != 0)
	{
		return RT_ERR_PHY_NOT_MATCH;
	}
#endif

	return RT_ERR_OK;
}

static int32
__phy_identify_8261I(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id)
{
	//hal_control_t   *pHalCtrl = NULL;
	int32           ret;
	uint32          reg_data1 = 0;
	uint32          reg_data2 = 0;

	//RT_PARAM_CHK((NULL == (pHalCtrl = hal_ctrlInfo_get(unit))), RT_ERR_FAILED);

	if (port >= RTK_MAX_PORT_PER_UNIT)
	{
		return RT_ERR_PHY_NOT_MATCH;
	}

	if ((ret = __phy_identify_default_c45(unit, port, model_id, rev_id)) != RT_ERR_OK)
	{
		return ret;
	}

	//if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_mmd_read(unit, port, 30, 0x103, &reg_data1)) != RT_ERR_OK)
	//    return ret;
	//if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_mmd_read(unit, port, 30, 0x104, &reg_data2)) != RT_ERR_OK)
	//    return ret;

#ifdef CONFIG_LUNA_G3_SERIES
	//phy_hal_mmd_read(unit, port, 30, 0x103, &reg_data1);
	aal_mdio_read(unit, 0, port, 30, 0x103, (ca_uint16_t *)&reg_data1);
	RTKMSPHY_DEBUG(" - PhyAddr-%d: Read MMD30.0x103  = 0x%0X", port, reg_data1);
	//phy_hal_mmd_read(unit, port, 30, 0x104, &reg_data2);
	aal_mdio_read(unit, 0, port, 30, 0x104, (ca_uint16_t *)&reg_data2);
	RTKMSPHY_DEBUG(" - PhyAddr-%d: Read MMD30.0x104  = 0x%0X", port, reg_data2);
#else
	RTKMSPHY_ERROR("Only Support G3 platform!!");
	return RT_ERR_PORT_NOT_SUPPORTED;
#endif

	reg_data2 = REG32_FIELD_GET(reg_data2, 11, 0xF800);

	if (reg_data1 == 0x8291 && reg_data2 == 0x0)
	{
		return RT_ERR_OK;
	}
	else
	{
		return RT_ERR_PHY_NOT_MATCH;
	}

	return RT_ERR_OK;
}

static int32
__phy_identify_8261BE(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id)
{
    int32           ret;
    uint32          reg_data1 = 0;
    uint32          reg_data2 = 0, model_c1 = 0, model_c2 = 0;

    if (port >= RTK_MAX_PORT_PER_UNIT)
    {
        return RT_ERR_PHY_NOT_MATCH;
    }

    if ((ret = __phy_identify_default_c45(unit, port, model_id, rev_id)) != RT_ERR_OK)
    {
        return ret;
    }

    //if ((ret = hal_miim_mmd_read(unit, port, 30, 0x103, &reg_data1)) != RT_ERR_OK)
    //    return ret;
    //if ((ret = hal_miim_mmd_read(unit, port, 30, 0x104, &reg_data2)) != RT_ERR_OK)
    //    return ret;
#ifdef CONFIG_LUNA_G3_SERIES
	aal_mdio_read(unit, 0, port, 30, 0x103, (ca_uint16_t *)&reg_data1);
	RTKMSPHY_DEBUG(" - PhyAddr-%d: Read MMD30.0x103  = 0x%0X", port, reg_data1);
	aal_mdio_read(unit, 0, port, 30, 0x104, (ca_uint16_t *)&reg_data2);
	RTKMSPHY_DEBUG(" - PhyAddr-%d: Read MMD30.0x104  = 0x%0X", port, reg_data2);
#else
	RTKMSPHY_ERROR("Only Support G3 platform!!");
	return RT_ERR_PORT_NOT_SUPPORTED;
#endif

    model_c1 = REG32_FIELD_GET(reg_data2, 11, 0xF800);
    model_c2 = REG32_FIELD_GET(reg_data2,  6, 0x07C0);

    if (reg_data1 == 0x8261 && (model_c1 == 0x2 && model_c2 == 0x5))
    {
        return RT_ERR_OK;
    }
    else
    {
        return RT_ERR_PHY_NOT_MATCH;
    }
}

static int32
__phy_identify_8261B(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id)
{
	int32           ret;
	uint32          reg_data1 = 0;
	uint32          reg_data2 = 0, model_c1 = 0, model_c2 = 0;

	if (port >= RTK_MAX_PORT_PER_UNIT)
	{
		return RT_ERR_PHY_NOT_MATCH;
	}

	if ((ret = __phy_identify_default_c45(unit, port, model_id, rev_id)) != RT_ERR_OK)
	{
		return ret;
	}

	//if ((ret = hal_miim_mmd_read(unit, port, 30, 0x103, &reg_data1)) != RT_ERR_OK)
	//	return ret;
	//if ((ret = hal_miim_mmd_read(unit, port, 30, 0x104, &reg_data2)) != RT_ERR_OK)
	//	return ret;
#ifdef CONFIG_LUNA_G3_SERIES
	aal_mdio_read(unit, 0, port, 30, 0x103, (ca_uint16_t *)&reg_data1);
	RTKMSPHY_DEBUG(" - PhyAddr-%d: Read MMD30.0x103  = 0x%0X", port, reg_data1);
	aal_mdio_read(unit, 0, port, 30, 0x104, (ca_uint16_t *)&reg_data2);
	RTKMSPHY_DEBUG(" - PhyAddr-%d: Read MMD30.0x104  = 0x%0X", port, reg_data2);
#else
	RTKMSPHY_ERROR("Only Support G3 platform!!");
	return RT_ERR_PORT_NOT_SUPPORTED;
#endif
	model_c1 = REG32_FIELD_GET(reg_data2, 11, 0xF800);
	model_c2 = REG32_FIELD_GET(reg_data2,  6, 0x07C0);

	if (reg_data1 == 0x8261 && (model_c1 != 0x2 || model_c2 != 0x5))
	{
		return RT_ERR_OK;
	}
	else
	{
		return RT_ERR_PHY_NOT_MATCH;
	}
}

static int32
__phy_identify_8251Lrev3(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id)
{
	int32           ret;
	uint32          reg_data1 = 0;
	uint32          reg_data2 = 0, model_c1 = 0, model_c2 = 0;

	if (port >= RTK_MAX_PORT_PER_UNIT)
	{
		//RTKMSPHY_ERROR("%s(%d): port = %d", __FUNCTION__, __LINE__, port);
		return RT_ERR_PHY_NOT_MATCH;
	}

	if ((ret = __phy_identify_default_c45(unit, port, model_id, rev_id)) != RT_ERR_OK)
	{
		//RTKMSPHY_ERROR("%s(%d)", __FUNCTION__, __LINE__);
		return ret;
	}

	//if ((ret = hal_miim_mmd_read(unit, port, 30, 0x103, &reg_data1)) != RT_ERR_OK) {
	//	RTKMSPHY_ERROR("%s(%d) ret = %d", __FUNCTION__, __LINE__, ret);
	//	return ret;
	//}
	//if ((ret = hal_miim_mmd_read(unit, port, 30, 0x104, &reg_data2)) != RT_ERR_OK) {
	//	RTKMSPHY_ERROR("%s(%d)", __FUNCTION__, __LINE__);
	//	return ret;
	//}
#ifdef CONFIG_LUNA_G3_SERIES
	aal_mdio_read(unit, 0, port, 30, 0x103, (ca_uint16_t *)&reg_data1);
	RTKMSPHY_DEBUG(" - PhyAddr-%d: Read MMD30.0x103  = 0x%0X", port, reg_data1);
	aal_mdio_read(unit, 0, port, 30, 0x104, (ca_uint16_t *)&reg_data2);
	RTKMSPHY_DEBUG(" - PhyAddr-%d: Read MMD30.0x104  = 0x%0X", port, reg_data2);
#else
	RTKMSPHY_ERROR("Only Support G3 platform!!");
	return RT_ERR_PORT_NOT_SUPPORTED;
#endif
	model_c1 = REG32_FIELD_GET(reg_data2, 11, 0xF800);
	model_c2 = REG32_FIELD_GET(reg_data2,  6, 0x07C0);

	if (reg_data1 == 0x8251 && (model_c1 == 0xC && model_c2 == 0x0))
	{
		return RT_ERR_OK;
	}
	else
	{
		return RT_ERR_PHY_NOT_MATCH;
	}
}

static int32
__phy_identify_8224(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id)
{
	//hal_control_t   *pHalCtrl = NULL;
	int32           ret;
	uint32          reg_data1 = 0;
	uint32          reg_data2 = 0;
	rtk_port_t      base_port = 0;

	//RT_PARAM_CHK((NULL == (pHalCtrl = hal_ctrlInfo_get(unit))), RT_ERR_FAILED);

	if (port >= RTK_MAX_PORT_PER_UNIT)
	{
		return RT_ERR_PHY_NOT_MATCH;
	}

	if ((ret = __phy_identify_default_c45(unit, port, model_id, rev_id)) != RT_ERR_OK)
	{
		return ret;
	}

	base_port = HWP_PHY_BASE_MACID(unit, port);

	//if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_mmd_read(unit, base_port, 30, 0x0004, &reg_data1)) != RT_ERR_OK)
	//    return ret;
	//if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_mmd_read(unit, base_port, 30, 0x0005, &reg_data2)) != RT_ERR_OK)
	//    return ret;
#ifdef CONFIG_LUNA_G3_SERIES
	//phy_hal_mmd_read(unit, port, 30, 0x0004, &reg_data1);
	aal_mdio_read(unit, 0, port, 30, 0x0004, (ca_uint16_t *)&reg_data1);
	RTKMSPHY_DEBUG(" - PhyAddr-%d: Read MMD30.0x0004  = 0x%0X", port, reg_data1);
	//phy_hal_mmd_read(unit, port, 30, 0x0005, &reg_data2);
	aal_mdio_read(unit, 0, port, 30, 0x0005, (ca_uint16_t *)&reg_data2);
	RTKMSPHY_DEBUG(" - PhyAddr-%d: Read MMD30.0x0005  = 0x%0X", port, reg_data2);
#else
	RTKMSPHY_ERROR("Only Support G3 platform!!");
	return RT_ERR_PORT_NOT_SUPPORTED;
#endif

	if (reg_data1 == 0x0 && reg_data2 == 0x8224)
	{
		return RT_ERR_OK;
	}
	else
	{
		return RT_ERR_PHY_NOT_MATCH;
	}

	return RT_ERR_OK;
}

int __eth_phy_rtkmsphy_reg_get(u8 phy_addr, rtk_extgphy_reg_t *pReg)
{
	RTKMSPHY_DEBUG("%s(%d): phy_addr = %d", __FUNCTION__, __LINE__, phy_addr);
	return rtk_port_phyMmdReg_get(RTKMSPHY_DEFAULT_UNIT_ID, rtkmsphy_get_mac_id_by_phy_addr(phy_addr), pReg->page, pReg->reg, &pReg->data);
}
EXPORT_SYMBOL(__eth_phy_rtkmsphy_reg_get);

int __eth_phy_rtkmsphy_reg_set(u8 phy_addr, rtk_extgphy_reg_t *pReg)
{
	RTKMSPHY_DEBUG("%s(%d): phy_addr = %d", __FUNCTION__, __LINE__, phy_addr);
	return rtk_port_phyMmdReg_set(RTKMSPHY_DEFAULT_UNIT_ID, rtkmsphy_get_mac_id_by_phy_addr(phy_addr), pReg->page, pReg->reg, pReg->data);
}
EXPORT_SYMBOL(__eth_phy_rtkmsphy_reg_set);

int __eth_phy_rtkmsphy_add_phy(u8 port, u8 phy_addr)
{
	int i = 0;
	RTKMSPHY_DEBUG("%s(%d): port = %d, phy_dev = %d", __FUNCTION__, __LINE__, port, phy_addr);

	for (i = 0 ; my_port_descp[i].mac_id != HWP_END ; i++) {
		if (my_port_descp[i].phy_addr == phy_addr) {
			RTKMSPHY_MSG("phyAddr %u existed on PORT %u", phy_addr, my_port_descp[i].mac_id);
			return 1;
		}
	}

	if (rtkmsphy_phy_count >= RTKMSPHY_MAX_PHY_NUMBER) {
		RTKMSPHY_ERROR("Unable add phy, reach max number for PORT %u ; phyAddr %u", port, phy_addr);
		return 1;
	}
	/* Use same value in mac_id and phy_addr */
	if (__phy_identify_8261(RTKMSPHY_DEFAULT_UNIT_ID, phy_addr, PHY_MODEL_ID_RTL826X, PHY_REV_NO_A) == RT_ERR_OK) {
		my_phy_Descp[rtkmsphy_phy_count].chip = RTK_PHYTYPE_RTL8261;
		my_phy_Descp[rtkmsphy_phy_count].mac_id = port;
		my_phy_Descp[rtkmsphy_phy_count].phy_max = 1;
		RTKMSPHY_INFO("RTL8261 identified [Port %d] [phyAddr %d]", port, phy_addr);
	} else if (__phy_identify_8261I(RTKMSPHY_DEFAULT_UNIT_ID, phy_addr, PHY_MODEL_ID_RTL826X, PHY_REV_NO_A) == RT_ERR_OK) {
		my_phy_Descp[rtkmsphy_phy_count].chip = RTK_PHYTYPE_RTL8261I;
		my_phy_Descp[rtkmsphy_phy_count].mac_id = port;
		my_phy_Descp[rtkmsphy_phy_count].phy_max = 1;
		RTKMSPHY_INFO("RTL8261I identified [Port %d] [phyAddr %d]", port, phy_addr);
	} else if (__phy_identify_8261BE(RTKMSPHY_DEFAULT_UNIT_ID, phy_addr, PHY_MODEL_ID_RTL826X, PHY_REV_NO_D) == RT_ERR_OK) {
		my_phy_Descp[rtkmsphy_phy_count].chip = RTK_PHYTYPE_RTL8261BE;
		my_phy_Descp[rtkmsphy_phy_count].mac_id = port;
		my_phy_Descp[rtkmsphy_phy_count].phy_max = 1;
		RTKMSPHY_INFO("RTL8261N(BE) identified [Port %d] [phyAddr %d]", port, phy_addr);
	} else if (__phy_identify_8261B(RTKMSPHY_DEFAULT_UNIT_ID, phy_addr, PHY_MODEL_ID_RTL826X, PHY_REV_NO_D) == RT_ERR_OK) {
		my_phy_Descp[rtkmsphy_phy_count].chip = RTK_PHYTYPE_RTL8261B;
		my_phy_Descp[rtkmsphy_phy_count].mac_id = port;
		my_phy_Descp[rtkmsphy_phy_count].phy_max = 1;
		RTKMSPHY_INFO("RTL8261N(B) identified [Port %d] [phyAddr %d]", port, phy_addr);
	} else if (__phy_identify_8251Lrev3(RTKMSPHY_DEFAULT_UNIT_ID, phy_addr, PHY_MODEL_ID_RTL826X, PHY_REV_NO_D) == RT_ERR_OK) {
		my_phy_Descp[rtkmsphy_phy_count].chip = RTK_PHYTYPE_RTL8251L_REV3;
		my_phy_Descp[rtkmsphy_phy_count].mac_id = port;
		my_phy_Descp[rtkmsphy_phy_count].phy_max = 1;
		RTKMSPHY_INFO("RTL8251L_REV3 identified [Port %d] [phyAddr %d]", port, phy_addr);
	} else if (__phy_identify_8224(RTKMSPHY_DEFAULT_UNIT_ID, phy_addr, PHY_MODEL_ID_RTL8224, PHY_REV_NO_A) == RT_ERR_OK) {
		my_phy_Descp[rtkmsphy_phy_count].chip = RTK_PHYTYPE_RTL8224;
		my_phy_Descp[rtkmsphy_phy_count].mac_id = port;
		my_phy_Descp[rtkmsphy_phy_count].phy_max = RTKMSPHY_RTL8224_PORT_NUMBER;
		RTKMSPHY_INFO("RTL8224 identified [Port %d] [phyAddr %d]", port, phy_addr);
	} else {
		RTKMSPHY_ERROR("UNKNOWN Model ID and Rev No. on port %d", port);
		return 1;
	}

	if ((rtkmsphy_port_count + my_phy_Descp[rtkmsphy_phy_count].phy_max) > RTKMSPHY_MAX_PORT_NUMBER) {
		RTKMSPHY_ERROR("Unable add %d phyPorts for PORT %u (phyAddr %u), Reach MAX Number (%d)!!!", my_phy_Descp[rtkmsphy_phy_count].phy_max, port, phy_addr, RTKMSPHY_MAX_PORT_NUMBER);
		return 1;
	}
	for (i = 0 ; i < my_phy_Descp[rtkmsphy_phy_count].phy_max ; i++) {
		my_port_descp[rtkmsphy_port_count].mac_id = port + i;
		my_port_descp[rtkmsphy_port_count].phy_idx = rtkmsphy_phy_count;
		switch (my_phy_Descp[rtkmsphy_phy_count].chip) {
		case RTK_PHYTYPE_RTL8261:
		case RTK_PHYTYPE_RTL8261I:
		case RTK_PHYTYPE_RTL8261BE:
		case RTK_PHYTYPE_RTL8261B:
			my_port_descp[rtkmsphy_port_count].eth = HWP_XGE;
			break;
		case RTK_PHYTYPE_RTL8251L_REV3:
			my_port_descp[rtkmsphy_port_count].eth = HWP_5GE;
			break;
		case RTK_PHYTYPE_RTL8224:
			my_port_descp[rtkmsphy_port_count].eth = HWP_2_5GE;
			break;
		default:
			/* Actually never reach here */
			RTKEXTGPHY_ERROR("Port%d ethernet type is NOT recognized !!!", port + i);
		}
		my_port_descp[rtkmsphy_port_count].medi = HWP_COPPER;
		my_port_descp[rtkmsphy_port_count].smi = 0;
		my_port_descp[rtkmsphy_port_count].phy_addr = phy_addr + i;
		rtkmsphy_port_count++;
	}

	rtkmsphy_phy_count++;
	return 0;
}
EXPORT_SYMBOL(__eth_phy_rtkmsphy_add_phy);

static void rtk_ms_phy_display_all(void)
{
	int i = 0;
	RTKMSPHY_MSG("[port_descp]");
	for (i = 0 ; my_port_descp[i].mac_id != HWP_END ; i++) {
		RTKMSPHY_MSG("\t- #%d: .mac_id = %2u, .phy_idx = %u, .eth = %u, .medi = %u, .smi = %u, .phy_addr = %2u", i,
			my_port_descp[i].mac_id,
			my_port_descp[i].phy_idx,
			my_port_descp[i].eth,
			my_port_descp[i].medi,
			my_port_descp[i].smi,
			my_port_descp[i].phy_addr);
	}

	RTKMSPHY_MSG("[phy_Descp]");
	for (i = 0 ; my_phy_Descp[i].chip != HWP_END ; i++) {
		RTKMSPHY_MSG("\t- #%d .chip = %2u, .mac_id = %2u, .phy_max = %u", i,
			my_phy_Descp[i].chip,
			my_phy_Descp[i].mac_id,
			my_phy_Descp[i].phy_max);

	}

	return;
}

int __eth_phy_rtkmsphy_init(u8 phy_addr)
{
	int ret, i = 0;
	uint32 data;
	rtk_phy_initInfo_t initInfo;

	RTKMSPHY_DEBUG("%s(%d): phy_addr = %d", __FUNCTION__, __LINE__, phy_addr);
	if (rtkmsphy_init) {
		RTKMSPHY_MSG("All PHYs are init.");
		return 0;
	}

	rtk_ms_phy_display_all();
	initInfo.port_desc = my_port_descp;
	initInfo.phy_desc = my_phy_Descp;

	RTKMSPHY_DEBUG("PHY lkm init");
	if ((ret = rtk_init(&initInfo)) == RT_ERR_OK)
		RTKMSPHY_INFO("rtk_init OK");
	else
		RTKMSPHY_ERROR("rtk_init Fail (0x%x) !!!", ret);

	for (i = 0 ; my_port_descp[i].mac_id != HWP_END ; i++) {
		/* Hard Code Setting for 8261[N] */
		if ((my_phy_Descp[my_port_descp[i].phy_idx].chip == RTK_PHYTYPE_RTL8261) ||
		    (my_phy_Descp[my_port_descp[i].phy_idx].chip == RTK_PHYTYPE_RTL8261I) ||
		    (my_phy_Descp[my_port_descp[i].phy_idx].chip == RTK_PHYTYPE_RTL8261BE) ||
		    (my_phy_Descp[my_port_descp[i].phy_idx].chip == RTK_PHYTYPE_RTL8261B) ||
		    (my_phy_Descp[my_port_descp[i].phy_idx].chip == RTK_PHYTYPE_RTL8251L_REV3)) {
			RTKMSPHY_MSG("[PORT %d] Setting RTL8261(I/N)/RTL8251L", my_port_descp[i].mac_id);

			data = RTK_PHY_CTRL_SERDES_MODE_USXGMII;
			RTKMSPHY_MSG(" - Set serdes mode as RTK_PHY_CTRL_SERDES_MODE_USXGMII");
			if ((ret = rtk_port_phyCtrl_set(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, RTK_PHY_CTRL_SERDES_MODE, data)) != RT_ERR_OK)
				RTKMSPHY_ERROR(" - rtk_port_phyCtrl_set (0x%x) !!!", ret);


			if (my_port_descp[i].mac_id == RTL8261_CA_XFI_PORT) {
#ifdef CONFIG_LUNA_G3_SERIES
				/* Patch for port 6 on luna G3 Series */
				RTKMSPHY_INFO(" - Set USXGMII CFW as 0xaa");
				phy_hal_mmd_read(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 30, 0x141, &data);
				//RTKMSPHY_MSG(" - PhyAddr-%d: Read MMD30.0x141  = 0x%0X", my_port_descp[i].mac_id, data);
				data = 0x80aa;
				phy_hal_mmd_write(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 30, 0x141, data);
				//RTKMSPHY_MSG(" - PhyAddr-%d: Set  MMD30.0x141  = 0x%0X", my_port_descp[i].mac_id, data);

				phy_hal_mmd_read(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 30, 0x143, &data);
				//RTKMSPHY_MSG(" - PhyAddr-%d: Read MMD30.0x143  = 0x%0X", my_port_descp[i].mac_id, data);
				data = 0x8C07;
				phy_hal_mmd_write(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 30, 0x143, data);
				//RTKMSPHY_MSG(" - PhyAddr-%d: Set  MMD30.0x143  = 0x%0X", my_port_descp[i].mac_id, data);
#endif
			}
			else if (my_port_descp[i].mac_id == RTL8261_CA_PON_PORT) {
#if 0
				RTKMSPHY_MSG(" - FORCE 10G");
				if((ret = rtk_port_phyForceModeAbility_set(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, PORT_SPEED_10G, PORT_FULL_DUPLEX, ENABLED)) != RT_ERR_OK)
					RTKMSPHY_ERROR("rtk_port_phyForceModeAbility_set(0x%x) !!!", ret);

				RTKMSPHY_MSG(" - Disable AN");
				if((ret = rtk_port_phyAutoNegoEnable_set(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, DISABLED)) != RT_ERR_OK)
					RTKMSPHY_ERROR("rtk_port_phyAutoNegoEnable_set ERROR(0x%x) !!!", ret);
#endif
#ifdef CONFIG_CA8277B_SERIES
				RTKMSPHY_MSG(" - Disable USXGMII NWAY");
				if ((ret = rtk_port_phyCtrl_set(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, RTK_PHY_CTRL_SERDES_USXGMII_AN, DISABLED)) != RT_ERR_OK)
					RTKMSPHY_ERROR(" - rtk_port_phyCtrl_set(%d): ret = 0x%x !!!", RTK_PHY_CTRL_SERDES_USXGMII_AN, ret);
#endif
#if 0
				RTKMSPHY_MSG(" - Disable SGMII NWAY !!!");
				if ((ret = rtk_port_phyCtrl_set(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, RTK_PHY_CTRL_SERDES_SGMII_AN, DISABLED)) != RT_ERR_OK)
					RTKMSPHY_ERROR(" - rtk_port_phyCtrl_set(%d): ret = 0x%x !!!", RTK_PHY_CTRL_SERDES_SGMII_AN, ret);
				RTKMSPHY_MSG(" - Disable BASEX NWAY !!!");
				if ((ret = rtk_port_phyCtrl_set(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, RTK_PHY_CTRL_SERDES_BASEX_AN, DISABLED)) != RT_ERR_OK)
					RTKMSPHY_ERROR(" - rtk_port_phyCtrl_set(%d): ret = 0x%x !!!", RTK_PHY_CTRL_SERDES_BASEX_AN, ret);
#endif
#ifdef CONFIG_LUNA_G3_SERIES
#ifdef CONFIG_RTL8277C_SERIES
				/* Patch for port 7 on luna G3 Series */
				RTKMSPHY_INFO(" - Set USXGMII CFW as 0xaa");
				phy_hal_mmd_read(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 30, 0x141, &data);
				//RTKMSPHY_MSG(" - PhyAddr-%d: Read MMD30.0x141  = 0x%0X", my_port_descp[i].mac_id, data);
				data = 0x80aa;
				phy_hal_mmd_write(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 30, 0x141, data);
				//RTKMSPHY_MSG(" - PhyAddr-%d: Set  MMD30.0x141  = 0x%0X", my_port_descp[i].mac_id, data);

				phy_hal_mmd_read(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 30, 0x143, &data);
				//RTKMSPHY_MSG(" - PhyAddr-%d: Read MMD30.0x143  = 0x%0X", my_port_descp[i].mac_id, data);
				data = 0x8C07;
				phy_hal_mmd_write(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 30, 0x143, data);
				//RTKMSPHY_MSG(" - PhyAddr-%d: Set  MMD30.0x143  = 0x%0X", my_port_descp[i].mac_id, data);
#endif
#endif
			}
		}

		/* Hard Code Setting for 8261N(BE) */
		if (my_phy_Descp[my_port_descp[i].phy_idx].chip == RTK_PHYTYPE_RTL8261BE) {
			RTKMSPHY_MSG(" - Set TX Polarity as 1 for RTL8261N(BE)");
			rtk_port_phyCtrl_set(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, RTK_PHY_CTRL_SERDES_TX_POLARITY, 1);
			//RTKMSPHY_MSG(" - Set MACSEC Bypass as 1 for RTL8261N");
			//rtk_port_phyCtrl_set(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, RTK_PHY_CTRL_MACSEC_BYPASS, 1);
		}

		/* Hard Code Setting for 8224 */
		if (my_phy_Descp[my_port_descp[i].phy_idx].chip == RTK_PHYTYPE_RTL8224) {
#if (defined(CONFIG_RTL8277C_SERIES) || defined(CONFIG_MERCURY_SERIES))
			RTKMSPHY_INFO(" - Set NWAYOP code as 0xaa for addr %d", my_port_descp[i].mac_id);
			if ((ret = phy_8224_sdsOpCode_set(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 0xaa)) != RT_ERR_OK)
				RTKMSPHY_ERROR("   - Set NWAYOP code (%d)", ret);
			RTKMSPHY_INFO(" - MDI reversef for addr %d", my_port_descp[i].mac_id);
			phy_hal_mmd_write(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 30, 0xa90, 0xc);
			RTKMSPHY_INFO(" - Power up PHY for addr %d", my_port_descp[i].mac_id);
			phy_hal_mmd_write(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 31, 0xa610, 0x2058);
#endif
		}

		ret = rtk_port_phyCtrl_set(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, RTK_PHY_CTRL_EEE_PHYMODE, ENABLED);
		RTKMSPHY_MSG(" - Set EEE modefor addr %d (%d)", my_port_descp[i].mac_id, ret);
	}

	rtkmsphy_init = 1;
	return 0;
}
EXPORT_SYMBOL(__eth_phy_rtkmsphy_init);

int __eth_phy_rtkmsphy_admin_enable_set(u8 phy_addr, bool power_up)
{
	return rtk_port_adminEnable_set(RTKMSPHY_DEFAULT_UNIT_ID, rtkmsphy_get_mac_id_by_phy_addr(phy_addr), power_up);
}
EXPORT_SYMBOL(__eth_phy_rtkmsphy_admin_enable_set);

int __eth_phy_rtkmsphy_admin_enable_get(u8 phy_addr, bool *power_up)
{
	int32 ret;
	rtk_enable_t enable;
	if((ret = rtk_port_adminEnable_get(RTKMSPHY_DEFAULT_UNIT_ID, rtkmsphy_get_mac_id_by_phy_addr(phy_addr), &enable)) != RT_ERR_OK)
		return ret;
	else
		*power_up = enable;
	return 0;
}
EXPORT_SYMBOL(__eth_phy_rtkmsphy_admin_enable_get);

int __eth_phy_rtkmsphy_auto_neg_set(u8 phy_addr, rtk_extgphy_ability_t *pAbility)
{
	rtk_port_phy_ability_t  ability;
	RTKMSPHY_DEBUG("%s(%d): phy_addr = %d", __FUNCTION__, __LINE__, phy_addr);

	ability.Half_10 = pAbility->Half_10;
	ability.Full_10 = pAbility->Full_10;
	ability.Half_100 = pAbility->Half_100;
	ability.Full_100 = pAbility->Full_100;
	ability.Half_1000 = pAbility->Half_1000;
	ability.Full_1000 = pAbility->Full_1000;
#if (defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL9300))
	ability.Half_10G = 0;
	ability.Full_10G = pAbility->Full_10000;
#endif
	ability.adv_2_5G = pAbility->Full_2500;
	ability.adv_5G = 0;
	ability.adv_10GBase_T = pAbility->Full_10000;
	ability.FC = pAbility->FC;
	ability.AsyFC = pAbility->AsyFC;

	return rtk_port_phyAutoNegoAbility_set(RTKMSPHY_DEFAULT_UNIT_ID, rtkmsphy_get_mac_id_by_phy_addr(phy_addr), &ability);
}
EXPORT_SYMBOL(__eth_phy_rtkmsphy_auto_neg_set);

int __eth_phy_rtkmsphy_auto_neg_get(u8 phy_addr, rtk_extgphy_ability_t *pAbility)
{
	int32 ret;
	rtk_port_phy_ability_t  ability;
	RTKMSPHY_DEBUG("%s(%d): phy_addr = %d", __FUNCTION__, __LINE__, phy_addr);

	if((ret = rtk_port_phyAutoNegoAbility_get(RTKMSPHY_DEFAULT_UNIT_ID, rtkmsphy_get_mac_id_by_phy_addr(phy_addr), &ability)) != RT_ERR_OK)
		return ret;

	pAbility->Half_10 = ability.Half_10;
	pAbility->Full_10 = ability.Full_10;
	pAbility->Half_100 = ability.Half_100;
	pAbility->Full_100 = ability.Full_100;
	pAbility->Half_1000 = ability.Half_1000;
	pAbility->Full_1000 = ability.Full_1000;
#if (defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL9300))
	pAbility->Full_10000 = ability.Full_10G;
#endif
	pAbility->Full_2500 = ability.adv_2_5G;
	pAbility->Full_10000 = ability.adv_10GBase_T;
	pAbility->FC = ability.FC;
	pAbility->AsyFC = ability.AsyFC;

	return 0;
}
EXPORT_SYMBOL(__eth_phy_rtkmsphy_auto_neg_get);

int __eth_phy_rtkmsphy_link_status_get(u8 phy_addr, ext_phy_link_status_t *link_status)
{
	int32 ret;
	rtk_port_linkStatus_t	status;
	rtk_port_speed_t	speed;
	rtk_port_duplex_t	duplex;
	RTKMSPHY_DEBUG("%s(%d): phy_addr = %d", __FUNCTION__, __LINE__, phy_addr);

	/* WORKAROUND: Read twice for real link status ??? */
	rtk_port_phyLinkStatus_get(RTKMSPHY_DEFAULT_UNIT_ID, rtkmsphy_get_mac_id_by_phy_addr(phy_addr), &status);
	if((ret = rtk_port_phyLinkStatus_get(RTKMSPHY_DEFAULT_UNIT_ID, rtkmsphy_get_mac_id_by_phy_addr(phy_addr), &status)) != RT_ERR_OK)
		return ret;

	link_status->link_up = (status == PORT_LINKUP) ? 1 : 0 ;

	if (status == PORT_LINKUP) {
		if((ret = rtk_port_speedDuplex_get(RTKMSPHY_DEFAULT_UNIT_ID, rtkmsphy_get_mac_id_by_phy_addr(phy_addr), &speed, &duplex)) != RT_ERR_OK)
			return ret;

		switch(speed) {
		case PORT_SPEED_10M:
			link_status->speed = EXT_PHY_SPEED_10;
			break;
		case PORT_SPEED_100M:
			link_status->speed = EXT_PHY_SPEED_100;
			break;
		case PORT_SPEED_1000M:
			link_status->speed = EXT_PHY_SPEED_1000;
			break;
		case PORT_SPEED_500M:
			link_status->speed = EXT_PHY_SPEED_500;
			break;
		case PORT_SPEED_2G:
			link_status->speed = EXT_PHY_SPEED_2000;
			break;
		case PORT_SPEED_2_5G:
			link_status->speed = EXT_PHY_SPEED_2500;
			break;
		case PORT_SPEED_5G:
			link_status->speed = EXT_PHY_SPEED_5000;
			break;
		case PORT_SPEED_10G:
			link_status->speed = EXT_PHY_SPEED_10000;
			break;
		case PORT_SPEED_2_5G_LITE:
			link_status->speed = EXT_PHY_SPEED_2500LITE;
			break;
		case PORT_SPEED_5G_LITE:
			link_status->speed = EXT_PHY_SPEED_5000LITE;
			break;
		case PORT_SPEED_10G_LITE:
			link_status->speed = EXT_PHY_SPEED_10000LITE;
			break;
		case PORT_SPEED_END:
		default:
			link_status->speed = EXT_PHY_SPEED_MAX;
			break;
		}

		if (duplex == PORT_HALF_DUPLEX)
			link_status->duplex = EXT_PHY_DUPLEX_HALF;
		else if (duplex == PORT_FULL_DUPLEX)
			link_status->duplex = EXT_PHY_DUPLEX_FULL;
		else
			link_status->duplex = EXT_PHY_DUPLEX_INVALID;
	}

	return 0;
}
EXPORT_SYMBOL(__eth_phy_rtkmsphy_link_status_get);

/* Implement rtk_port_phyCtrl_set(RTK_PHY_CTRL_SERDES_RX_RESET) for phy_rtl826xb */
u32 __eth_phy_rtkmsphy_sds_rx_reset(u8 phy_addr)
{
	RTKMSPHY_DEBUG("%s(%d): phy_addr = %d", __FUNCTION__, __LINE__, phy_addr);

	/* Hard Code Setting for 8261N(BE) */
	if (my_phy_Descp[my_port_descp[rtkmsphy_get_mac_id_by_phy_addr(phy_addr)].phy_idx].chip == RTK_PHYTYPE_RTL8261BE) {
		RTKMSPHY_MSG(" - Reset sds rx for RTL8261N(BE)");
		phy_hal_mmd_write(RTKMSPHY_DEFAULT_UNIT_ID, rtkmsphy_get_mac_id_by_phy_addr(phy_addr), 30, 0x141, 0x10);
		phy_hal_mmd_write(RTKMSPHY_DEFAULT_UNIT_ID, rtkmsphy_get_mac_id_by_phy_addr(phy_addr), 30, 0x143, 0x8820);
		phy_hal_mmd_write(RTKMSPHY_DEFAULT_UNIT_ID, rtkmsphy_get_mac_id_by_phy_addr(phy_addr), 30, 0x141, 0x30);
		phy_hal_mmd_write(RTKMSPHY_DEFAULT_UNIT_ID, rtkmsphy_get_mac_id_by_phy_addr(phy_addr), 30, 0x143, 0x8820);
	}
	return 0;
}
EXPORT_SYMBOL(__eth_phy_rtkmsphy_sds_rx_reset);

u32 __eth_phy_rtkmsphy_cdr_reset(u8 phy_addr)
{
	RTKMSPHY_DEBUG("%s(%d): phy_addr = %d", __FUNCTION__, __LINE__, phy_addr);

	if ((my_phy_Descp[my_port_descp[rtkmsphy_get_mac_id_by_phy_addr(phy_addr)].phy_idx].chip == RTK_PHYTYPE_RTL8261) ||
		(my_phy_Descp[my_port_descp[rtkmsphy_get_mac_id_by_phy_addr(phy_addr)].phy_idx].chip == RTK_PHYTYPE_RTL8261I) ||
		(my_phy_Descp[my_port_descp[rtkmsphy_get_mac_id_by_phy_addr(phy_addr)].phy_idx].chip == RTK_PHYTYPE_RTL8261BE) ||
		(my_phy_Descp[my_port_descp[rtkmsphy_get_mac_id_by_phy_addr(phy_addr)].phy_idx].chip == RTK_PHYTYPE_RTL8261B) ||
		(my_phy_Descp[my_port_descp[rtkmsphy_get_mac_id_by_phy_addr(phy_addr)].phy_idx].chip == RTK_PHYTYPE_RTL8251L_REV3)) {
		RTKMSPHY_MSG(" - Do CDR Reset for port %d (phyAddr %d)", rtkmsphy_get_mac_id_by_phy_addr(phy_addr), phy_addr);
		phy_hal_mmd_write(RTKMSPHY_DEFAULT_UNIT_ID, rtkmsphy_get_mac_id_by_phy_addr(phy_addr), 30, 0x141, 0x40);
		phy_hal_mmd_write(RTKMSPHY_DEFAULT_UNIT_ID, rtkmsphy_get_mac_id_by_phy_addr(phy_addr), 30, 0x143, 0x88a0);
		phy_hal_mmd_write(RTKMSPHY_DEFAULT_UNIT_ID, rtkmsphy_get_mac_id_by_phy_addr(phy_addr), 30, 0x141, 0xC0);
		phy_hal_mmd_write(RTKMSPHY_DEFAULT_UNIT_ID, rtkmsphy_get_mac_id_by_phy_addr(phy_addr), 30, 0x143, 0x88a0);
	}
	return 0;
}
EXPORT_SYMBOL(__eth_phy_rtkmsphy_cdr_reset);

int __eth_phy_rtkmsphy_flow_ctrl_get(u8 phy_addr, u8 *tx_pause, u8 *rx_pause)
{
	int32 ret;
	rtk_enable_t reg0_auto_nego;
	rtk_port_phy_ability_t my_ability, lp_ability;
	rtk_port_t port = rtkmsphy_get_mac_id_by_phy_addr(phy_addr);

	*tx_pause = DISABLED;
	*rx_pause = DISABLED;

	ret = rtk_port_phyAutoNegoEnable_get(RTKMSPHY_DEFAULT_UNIT_ID, port, &reg0_auto_nego);
	if (ret != RT_ERR_OK)
		goto exit;

	ret = rtk_port_phyAutoNegoAbility_get(RTKMSPHY_DEFAULT_UNIT_ID, port, &my_ability);
	if (ret != RT_ERR_OK)
		goto exit;

	ret = rtk_port_phyPeerAutoNegoAbility_get(RTKMSPHY_DEFAULT_UNIT_ID, port, &lp_ability);
	if (ret != RT_ERR_OK)
		goto exit;

	if (reg0_auto_nego) {
		/* refer to the table 37-4 of IEEE802.3 */
		if (my_ability.FC && lp_ability.FC) {
			*tx_pause = ENABLED;
			*rx_pause = ENABLED;
		}
		else if (my_ability.AsyFC && lp_ability.AsyFC) {
			if (my_ability.FC)
				*rx_pause = ENABLED;
			else if (lp_ability.FC)
				*tx_pause = ENABLED;
		}
	}
	else {
		if (my_ability.FC) {
			*tx_pause = ENABLED;
			*rx_pause = ENABLED;
		}
	}
exit:
	return ret;
}
EXPORT_SYMBOL(__eth_phy_rtkmsphy_flow_ctrl_get);

static void my_data_init(void)
{
	int i = 0;
	
	RTKMSPHY_MSG("Data Init.");
	for (i = 0 ; i <= RTKMSPHY_MAX_PORT_NUMBER ; i++) {
		my_port_descp[i].mac_id = HWP_END;
		my_port_descp[i].phy_idx = 0;
		my_port_descp[i].eth = HWP_ETHTYPE_END;
		my_port_descp[i].medi = HWP_MEDIUM_END;
		my_port_descp[i].smi = 0;
		my_port_descp[i].phy_addr = 0;

		old_cnt_rx[i] = 0;
		old_cnt_rx_err[i] = 0;
		old_cnt_rx_crcerr[i] = 0;
		old_cnt_ldpc_err[i] = 0;
	}

	for (i = 0 ; i <= RTKMSPHY_MAX_PHY_NUMBER ; i++) {
		my_phy_Descp[i].chip = HWP_END;
		my_phy_Descp[i].mac_id = 0;
		my_phy_Descp[i].phy_max = 0;
	}

	return;
}

/*
 * Function Declaration
 */
static int __init phy_lkm_init(void)
{
        RTKMSPHY_INFO("rtk-ms-phyOnly-%s", RT_VERSION_SDK);
	my_data_init();

#if IS_BUILTIN(CONFIG_RTK_EXT_GPHY) || IS_MODULE(CONFIG_RTK_EXT_GPHY)
        RTKMSPHY_INFO("Phys init. will be done by extGphy");
#else
	//int32 ret, i;
	//rtk_phy_initInfo_t initInfo;
	//uint32 data;

#if defined(CONFIG_RTL8261_PORT6_PHY_ENABLE)
	__eth_phy_rtkmsphy_add_phy(RTL8261_CA_XFI_PORT, CONFIG_RTL8261_PORT6_PHY_ADDR);
#endif
#if defined(CONFIG_RTL8261_PORT7_PHY_ENABLE)
	__eth_phy_rtkmsphy_add_phy(RTL8261_CA_PON_PORT, CONFIG_RTL8261_PORT6_PHY_ADDR);
#endif
	__eth_phy_rtkmsphy_init(0);





#if 0
	for (i = 0 ; my_port_descp[i].mac_id != HWP_END ; i++) {
		RTKMSPHY_MSG("[PORT %d] Detecting RTL8261",  my_port_descp[i].mac_id);
		phy_hal_mmd_read(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 1, 3, &data);
		if (data == RTL8261_MMD_1_3_VALUE) {
			RTKMSPHY_MSG(" - Found it, UNIT %d/PHYADDR %d/MMD %d/REG 0x%X = 0x%0X",
				RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 1, 3, data);
		}
		else if (data == RTL8261_CCUT_MMD_1_3_VALUE) {
			RTKMSPHY_MSG(" - Found it (C CUT), UNIT %d/PHYADDR %d/MMD %d/REG 0x%X = 0x%0X",
				RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 1, 3, data);
		}
		else {
			RTKMSPHY_ERROR(" - Not RTL8261!!!, UNIT %d/PHYADDR %d/MMD %d/REG 0x%X = 0x%0X (should be 0x%04X or 0x%04X)",
				RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 1, 3, data, RTL8261_MMD_1_3_VALUE, RTL8261_CCUT_MMD_1_3_VALUE);
			return 0;
		}
	}

	RTKMSPHY_DEBUG("PHY lkm init");
	initInfo.port_desc = my_port_descp;
	initInfo.phy_desc = my_phy_Descp;

	if ((ret = rtk_init(&initInfo)) == RT_ERR_OK)
		RTKMSPHY_INFO("rtk_init OK");
	else
		RTKMSPHY_ERROR("rtk_init Fail (0x%x) !!!", ret);

	for (i = 0 ; my_port_descp[i].mac_id != HWP_END ; i++) {
		RTKMSPHY_MSG("[PORT %d] Setting RTL8261",  my_port_descp[i].mac_id);

		data = RTK_PHY_CTRL_SERDES_MODE_USXGMII;
		RTKMSPHY_MSG(" - Set serdes mode as RTK_PHY_CTRL_SERDES_MODE_USXGMII");
		if ((ret = rtk_port_phyCtrl_set(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, RTK_PHY_CTRL_SERDES_MODE, data)) != RT_ERR_OK)
			RTKMSPHY_ERROR(" - rtk_port_phyCtrl_set (0x%x) !!!", ret);

		if (my_port_descp[i].mac_id == RTL8261_CA_XFI_PORT) {
#ifdef CONFIG_LUNA_G3_SERIES
			/* Patch for port 6 on luna G3 Series */
			RTKMSPHY_INFO(" - Set USXGMII CFW as 0xaa");
			phy_hal_mmd_read(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 30, 0x141, &data);
			//RTKMSPHY_MSG(" - PhyAddr-%d: Read MMD30.0x141  = 0x%0X", my_port_descp[i].mac_id, data);
			data = 0x80aa;
			phy_hal_mmd_write(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 30, 0x141, data);
			//RTKMSPHY_MSG(" - PhyAddr-%d: Set  MMD30.0x141  = 0x%0X", my_port_descp[i].mac_id, data);

			phy_hal_mmd_read(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 30, 0x143, &data);
			//RTKMSPHY_MSG(" - PhyAddr-%d: Read MMD30.0x143  = 0x%0X", my_port_descp[i].mac_id, data);
			data = 0x8C07;
			phy_hal_mmd_write(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 30, 0x143, data);
			//RTKMSPHY_MSG(" - PhyAddr-%d: Set  MMD30.0x143  = 0x%0X", my_port_descp[i].mac_id, data);
#endif
		}
		else if (my_port_descp[i].mac_id == RTL8261_CA_PON_PORT) {
#if 0
			RTKMSPHY_MSG(" - FORCE 10G");
			if((ret = rtk_port_phyForceModeAbility_set(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, PORT_SPEED_10G, PORT_FULL_DUPLEX, ENABLED)) != RT_ERR_OK)
				RTKMSPHY_ERROR("rtk_port_phyForceModeAbility_set(0x%x) !!!", ret);

			RTKMSPHY_MSG(" - Disable AN");
			if((ret = rtk_port_phyAutoNegoEnable_set(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, DISABLED)) != RT_ERR_OK)
				RTKMSPHY_ERROR("rtk_port_phyAutoNegoEnable_set ERROR(0x%x) !!!", ret);
#endif
#ifdef CONFIG_CA8277B_SERIES
			RTKMSPHY_MSG(" - Disable USXGMII NWAY");
			if ((ret = rtk_port_phyCtrl_set(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, RTK_PHY_CTRL_SERDES_USXGMII_AN, DISABLED)) != RT_ERR_OK)
				RTKMSPHY_ERROR(" - rtk_port_phyCtrl_set(%d): ret = 0x%x !!!", RTK_PHY_CTRL_SERDES_USXGMII_AN, ret);
#endif
#if 0
			RTKMSPHY_MSG(" - Disable SGMII NWAY !!!");
			if ((ret = rtk_port_phyCtrl_set(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, RTK_PHY_CTRL_SERDES_SGMII_AN, DISABLED)) != RT_ERR_OK)
				RTKMSPHY_ERROR(" - rtk_port_phyCtrl_set(%d): ret = 0x%x !!!", RTK_PHY_CTRL_SERDES_SGMII_AN, ret);
			RTKMSPHY_MSG(" - Disable BASEX NWAY !!!");
			if ((ret = rtk_port_phyCtrl_set(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, RTK_PHY_CTRL_SERDES_BASEX_AN, DISABLED)) != RT_ERR_OK)
				RTKMSPHY_ERROR(" - rtk_port_phyCtrl_set(%d): ret = 0x%x !!!", RTK_PHY_CTRL_SERDES_BASEX_AN, ret);
#endif
#ifdef CONFIG_LUNA_G3_SERIES
#ifdef CONFIG_RTL8277C_SERIES
			/* Patch for port 7 on luna G3 Series */
			RTKMSPHY_INFO(" - Set USXGMII CFW as 0xaa");
			phy_hal_mmd_read(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 30, 0x141, &data);
			//RTKMSPHY_MSG(" - PhyAddr-%d: Read MMD30.0x141  = 0x%0X", my_port_descp[i].mac_id, data);
			data = 0x80aa;
			phy_hal_mmd_write(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 30, 0x141, data);
			//RTKMSPHY_MSG(" - PhyAddr-%d: Set  MMD30.0x141  = 0x%0X", my_port_descp[i].mac_id, data);

			phy_hal_mmd_read(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 30, 0x143, &data);
			//RTKMSPHY_MSG(" - PhyAddr-%d: Read MMD30.0x143  = 0x%0X", my_port_descp[i].mac_id, data);
			data = 0x8C07;
			phy_hal_mmd_write(RTKMSPHY_DEFAULT_UNIT_ID, my_port_descp[i].mac_id, 30, 0x143, data);
			//RTKMSPHY_MSG(" - PhyAddr-%d: Set  MMD30.0x143  = 0x%0X", my_port_descp[i].mac_id, data);
#endif
#endif
		}
	}
#endif
#endif
	rtkmsphy_proc_init();

	return 0;
}

static void __exit phy_lkm_exit(void)
{
	RTKMSPHY_MSG("Exit PHY lkm");
}

module_init(phy_lkm_init);
module_exit(phy_lkm_exit);

MODULE_AUTHOR("Realtek.com");
MODULE_DESCRIPTION("phy lkm");
MODULE_LICENSE("GPL");

#if 0
EXPORT_SYMBOL(rtk_init);
EXPORT_SYMBOL(rtk_port_linkMedia_get);
EXPORT_SYMBOL(rtk_port_speedDuplex_get);
EXPORT_SYMBOL(rtk_port_adminEnable_set);
EXPORT_SYMBOL(rtk_port_phyAutoNegoEnable_get);
EXPORT_SYMBOL(rtk_port_phyAutoNegoEnable_set);
EXPORT_SYMBOL(rtk_port_phyAutoNegoAbilityLocal_get);
EXPORT_SYMBOL(rtk_port_phyAutoNegoAbility_get);
EXPORT_SYMBOL(rtk_port_phyAutoNegoAbility_set);
EXPORT_SYMBOL(rtk_port_phyForceModeAbility_get);
EXPORT_SYMBOL(rtk_port_phyForceModeAbility_set);
EXPORT_SYMBOL(rtk_port_phyForceFlowctrlMode_get);
EXPORT_SYMBOL(rtk_port_phyForceFlowctrlMode_set);
EXPORT_SYMBOL(rtk_port_phyMasterSlave_get);
EXPORT_SYMBOL(rtk_port_phyMasterSlave_set);
EXPORT_SYMBOL(rtk_port_phyReg_get);
EXPORT_SYMBOL(rtk_port_phyReg_set);
EXPORT_SYMBOL(rtk_port_phyExtParkPageReg_get);
EXPORT_SYMBOL(rtk_port_phyExtParkPageReg_set);
EXPORT_SYMBOL(rtk_port_phymaskExtParkPageReg_set);
EXPORT_SYMBOL(rtk_port_phyMmdReg_get);
EXPORT_SYMBOL(rtk_port_phyMmdReg_set);
EXPORT_SYMBOL(rtk_port_phymaskMmdReg_set);
EXPORT_SYMBOL(rtk_port_phyComboPortMedia_get);
EXPORT_SYMBOL(rtk_port_phyComboPortMedia_set);
EXPORT_SYMBOL(rtk_port_greenEnable_get);
EXPORT_SYMBOL(rtk_port_greenEnable_set);
EXPORT_SYMBOL(rtk_port_phyCrossOverMode_get);
EXPORT_SYMBOL(rtk_port_phyCrossOverMode_set);
EXPORT_SYMBOL(rtk_port_phyCrossOverStatus_get);
EXPORT_SYMBOL(rtk_port_phyComboPortFiberMedia_get);
EXPORT_SYMBOL(rtk_port_phyComboPortFiberMedia_set);
EXPORT_SYMBOL(rtk_port_linkDownPowerSavingEnable_get);
EXPORT_SYMBOL(rtk_port_linkDownPowerSavingEnable_set);
EXPORT_SYMBOL(rtk_port_gigaLiteEnable_get);
EXPORT_SYMBOL(rtk_port_gigaLiteEnable_set);
EXPORT_SYMBOL(rtk_port_2pt5gLiteEnable_get);
EXPORT_SYMBOL(rtk_port_2pt5gLiteEnable_set);
EXPORT_SYMBOL(rtk_port_phyReconfig_register);
EXPORT_SYMBOL(rtk_port_phyReconfig_unregister);
EXPORT_SYMBOL(rtk_port_downSpeedEnable_get);
EXPORT_SYMBOL(rtk_port_downSpeedEnable_set);
EXPORT_SYMBOL(rtk_port_downSpeedStatus_get);
EXPORT_SYMBOL(rtk_port_fiberNwayForceLinkEnable_get);
EXPORT_SYMBOL(rtk_port_fiberNwayForceLinkEnable_set);
EXPORT_SYMBOL(rtk_port_phyLoopBackEnable_get);
EXPORT_SYMBOL(rtk_port_phyLoopBackEnable_set);
EXPORT_SYMBOL(rtk_port_10gMedia_set);
EXPORT_SYMBOL(rtk_port_10gMedia_get);
EXPORT_SYMBOL(rtk_port_phyFiberTxDis_set);
EXPORT_SYMBOL(rtk_port_phyFiberTxDisPin_set);
EXPORT_SYMBOL(rtk_port_fiberRxEnable_get);
EXPORT_SYMBOL(rtk_port_fiberRxEnable_set);
EXPORT_SYMBOL(rtk_port_phyIeeeTestMode_set);
EXPORT_SYMBOL(rtk_port_phyPolar_get);
EXPORT_SYMBOL(rtk_port_phyPolar_set);
EXPORT_SYMBOL(rtk_port_phyEyeMonitor_start);
EXPORT_SYMBOL(rtk_port_phyEyeMonitorInfo_get);
EXPORT_SYMBOL(rtk_port_imageFlash_load);
EXPORT_SYMBOL(rtk_port_phySdsRxCaliStatus_get);
EXPORT_SYMBOL(rtk_port_phyReset_set);
EXPORT_SYMBOL(rtk_port_phyLinkStatus_get);
EXPORT_SYMBOL(rtk_port_phyPeerAutoNegoAbility_get);
EXPORT_SYMBOL(rtk_port_phyMacIntfSerdesMode_get);
EXPORT_SYMBOL(rtk_port_phyLedMode_set);
EXPORT_SYMBOL(rtk_port_phyLedCtrl_get);
EXPORT_SYMBOL(rtk_port_phyLedCtrl_set);
EXPORT_SYMBOL(rtk_port_phyMacIntfSerdesLinkStatus_get);
EXPORT_SYMBOL(rtk_port_phySdsEyeParam_get);
EXPORT_SYMBOL(rtk_port_phySdsEyeParam_set);
EXPORT_SYMBOL(rtk_port_phyMdiLoopbackEnable_get);
EXPORT_SYMBOL(rtk_port_phyMdiLoopbackEnable_set);
EXPORT_SYMBOL(rtk_port_phyIntr_init);
EXPORT_SYMBOL(rtk_port_phyIntrEnable_get);
EXPORT_SYMBOL(rtk_port_phyIntrEnable_set);
EXPORT_SYMBOL(rtk_port_phyIntrStatus_get);
EXPORT_SYMBOL(rtk_port_phyIntrMask_get);
EXPORT_SYMBOL(rtk_port_phyIntrMask_set);
EXPORT_SYMBOL(rtk_port_phySdsTestMode_set);
EXPORT_SYMBOL(rtk_port_phySdsTestModeCnt_get);
EXPORT_SYMBOL(rtk_port_phySdsLeq_get);
EXPORT_SYMBOL(rtk_port_phySdsLeq_set);
EXPORT_SYMBOL(rtk_port_phySds_get);
EXPORT_SYMBOL(rtk_port_phySds_set);
EXPORT_SYMBOL(rtk_port_phyCtrl_get);
EXPORT_SYMBOL(rtk_port_phyCtrl_set);
EXPORT_SYMBOL(rtk_port_phyLiteEnable_get);
EXPORT_SYMBOL(rtk_port_phyLiteEnable_set);
EXPORT_SYMBOL(rtk_port_phyDbgCounter_get);
EXPORT_SYMBOL(rtk_eee_portEnable_get);
EXPORT_SYMBOL(rtk_eee_portEnable_set);
EXPORT_SYMBOL(rtk_diag_rtctEnable_set);
#endif

