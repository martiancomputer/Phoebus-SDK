#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
//#include <linux/mutex.h>
#include <linux/kthread.h>
//#include <linux/gpio.h>
#include <linux/delay.h>	/* mdelay() */
#include <fs/proc/internal.h>
#include <linux/rtnetlink.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#ifdef CONFIG_RTL_MULTI_LAN_DEV
#include <uapi/linux/if.h>
#include <linux/netdevice.h>
#endif
#include <linux/inet.h>

#include <rtk_types.h>
#include <port.h>
#include <cpu.h>
#include <svlan.h>
#include <vlan.h>
#include <stat.h>
#include <l2.h>
#include <mirror.h>
#include <igmp.h>
#include <leds-rtk-hw.h>

#include <rtk_switch.h>
#include <rtl8367_ioctl.h>

#ifdef CONFIG_DAL_RTL8367C
#include <dal/rtl8367c/rtl8367c_smi.h>
#include <dal/rtl8367c/rtl8367c_asicdrv.h>
#include <dal/rtl8367c/rtl8367c_asicdrv_phy.h>
#endif

#ifdef CONFIG_DAL_RTL8367D
#include <dal/rtl8367d/rtl8367d_smi.h>
#include <dal/rtl8367d/rtl8367d_asicdrv.h>
#endif

#ifdef CONFIG_EXTERNAL_SWITCH
#include <rtl8367_mapper.h>
#ifdef CONFIG_SDK_MODULES
#include <module/intr_bcaster/intr_bcaster.h>
#endif
#endif

#if defined(CONFIG_RTL9607C_SERIES)
#include <rtk/gpio.h>
#include <rtk/mdio.h>
#include <common/rt_error.h>
#include <rt_mdio.h>

#define IO_MODE_EN_REG			0xBB023014
#define MDIO_MASTER_EN			(1 << 10)
#define EXT_MDX_M_EN			(1 << 11)

#define RTL9607C_SET0_MDC_PIN       6
#define RTL9607C_SET0_MDIO_PIN      7
#endif

#ifdef CONFIG_RTL_MULTI_LAN_DEV
#define RTL8367_MAX_LAN_PORT_NUM	10

struct rtl8367_dev_table_entry {
	u8	ifname[IFNAMSIZ];
	struct net_device_ops *orig_ops;
	struct net_device_ops replace_ops;
};

static struct rtl8367_dev_table_entry rtl8367_dev_table[RTL8367_MAX_LAN_PORT_NUM];

struct rtl8367_port_isolation_mapping_table_entry {
	rtksw_port_t extPort;
	rtksw_uint32 setFid;
};

static struct rtl8367_port_isolation_mapping_table_entry rtl8367_port_isolation_mapping_table[RTL8367_MAX_LAN_PORT_NUM];

#define RE8637_PORT_LINK_SCAN_INTERVAL  3000
static struct task_struct *rtl8367_port_scan_task;
unsigned char rtl8367_port_scan_status = 1; //0: stop, 1:start

rtksw_api_ret_t rtk_rtl8367_port_scan_start(void);
rtksw_api_ret_t rtk_rtl8367_port_scan_stop(void);
#endif

struct proc_dir_entry *rtl8367_proc_dir = NULL;

#define PROC_DIR_RTL8367		"rtl8367"
#define PROC_FILE_HELP			"help"
#define PROC_FILE_LINK_STATUS		"link_status"
#define PROC_FILE_SVLAN					"svlan"
#define PROC_FILE_PORT_ISOLATION_MAPPING	"port_isolation_mapping"
#define PROC_FILE_STAT					"stat"
#ifdef CONFIG_RTL_MULTI_LAN_DEV
#define PROC_FILE_PORT_SCAN				"port_scan"
#define PROC_FILE_DEV_PORT_MAPPING		"dev_port_mapping"
#endif
#define PROC_FILE_L2					"l2"
#define PROC_FILE_ASIC_REG					"asic_reg"
#define PROC_FILE_PORT_PHY_REG				"port_phy_reg"
#define PROC_FILE_PORT_PHY_OCP_REG			"port_phy_ocp_reg"
#define PROC_FILE_MIRROR			"mirror"
#define PROC_FILE_IGMP			"igmp"
#define PROC_FILE_L2MCAST			"l2mcast"

static int svlan_vid_mcast = -1;
module_param(svlan_vid_mcast, int, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC (svlan_vid_mcast, "8367S multicast SVLAN VID");

void rtl8367_usage(const char *filename)
{
	RTL8367_MSG("[Usage]");

	if((strcasecmp(filename, PROC_FILE_LINK_STATUS) == 0) || (strcasecmp(filename, PROC_FILE_HELP) == 0)) {
		RTL8367_MSG("cat /proc/%s/%s", PROC_DIR_RTL8367, PROC_FILE_LINK_STATUS);
		if(strcasecmp(filename, PROC_FILE_LINK_STATUS) == 0)		return;
	}

	if((strcasecmp(filename, PROC_FILE_SVLAN) == 0) || (strcasecmp(filename, PROC_FILE_HELP) == 0)) {
		RTL8367_MSG("cat /proc/%s/%s", PROC_DIR_RTL8367, PROC_FILE_SVLAN);
		RTL8367_MSG("echo [MCAST] [SVID] > /proc/%s/%s", PROC_DIR_RTL8367, PROC_FILE_SVLAN);
		if(strcasecmp(filename, PROC_FILE_SVLAN) == 0)		return;
	}

	if((strcasecmp(filename, PROC_FILE_STAT) == 0) || (strcasecmp(filename, PROC_FILE_HELP) == 0)) {
		RTL8367_MSG("cat /proc/%s/%s", PROC_DIR_RTL8367, PROC_FILE_STAT);
		if(strcasecmp(filename, PROC_FILE_STAT) == 0)		return;
	}

#ifdef CONFIG_RTL_MULTI_LAN_DEV
	if((strcasecmp(filename, PROC_FILE_PORT_SCAN) == 0) || (strcasecmp(filename, PROC_FILE_HELP) == 0)) {
		RTL8367_MSG("cat /proc/%s/%s", PROC_DIR_RTL8367, PROC_FILE_PORT_SCAN);
		RTL8367_MSG("echo status [START/STOP] > /proc/%s/%s", PROC_DIR_RTL8367, PROC_FILE_PORT_SCAN);
		if(strcasecmp(filename, PROC_FILE_PORT_SCAN) == 0)		return;
	}

	if((strcasecmp(filename, PROC_FILE_DEV_PORT_MAPPING) == 0) || (strcasecmp(filename, PROC_FILE_HELP) == 0)) {
		RTL8367_MSG("cat /proc/%s/%s", PROC_DIR_RTL8367, PROC_FILE_DEV_PORT_MAPPING);
		RTL8367_MSG("echo num_of_port dev_name /proc/%s/%s", PROC_DIR_RTL8367, PROC_FILE_DEV_PORT_MAPPING);
		if(strcasecmp(filename, PROC_FILE_DEV_PORT_MAPPING) == 0)		return;
	}
#endif
	
	if((strcasecmp(filename, PROC_FILE_L2) == 0) || (strcasecmp(filename, PROC_FILE_HELP) == 0)) {
		RTL8367_MSG("cat /proc/%s/%s", PROC_DIR_RTL8367, PROC_FILE_L2);
		RTL8367_MSG("echo 1 > /proc/%s/%s (clear all port counter)", PROC_DIR_RTL8367, PROC_FILE_L2);
		if(strcasecmp(filename, PROC_FILE_L2) == 0)		return;
	}
	
	if((strcasecmp(filename, PROC_FILE_ASIC_REG) == 0) || (strcasecmp(filename, PROC_FILE_HELP) == 0)) {
		RTL8367_MSG("echo [get/set] regaddr_hex [regdata_hex(for set operation)] > /proc/%s/%s", PROC_DIR_RTL8367, PROC_FILE_ASIC_REG);
		if(strcasecmp(filename, PROC_FILE_ASIC_REG) == 0)		return;
	}
	
	if((strcasecmp(filename, PROC_FILE_PORT_PHY_REG) == 0) || (strcasecmp(filename, PROC_FILE_HELP) == 0)) {
		RTL8367_MSG("echo [get/set] port regid [regdata_hex(for set operation)] > /proc/%s/%s", PROC_DIR_RTL8367, PROC_FILE_PORT_PHY_REG);
		if(strcasecmp(filename, PROC_FILE_PORT_PHY_REG) == 0)		return;
	}
	
	if((strcasecmp(filename, PROC_FILE_PORT_PHY_OCP_REG) == 0) || (strcasecmp(filename, PROC_FILE_HELP) == 0)) {
		RTL8367_MSG("echo [get/set] port regaddr_hex [regdata_hex(for set operation)] > /proc/%s/%s", PROC_DIR_RTL8367, PROC_FILE_PORT_PHY_OCP_REG);
		if(strcasecmp(filename, PROC_FILE_PORT_PHY_OCP_REG) == 0)		return;
	}

	if((strcasecmp(filename, PROC_FILE_MIRROR) == 0) || (strcasecmp(filename, PROC_FILE_HELP) == 0)) {
		RTL8367_MSG("echo mirroring_port mirrored_port > /proc/%s/%s", PROC_DIR_RTL8367, PROC_FILE_MIRROR);
		if(strcasecmp(filename, PROC_FILE_MIRROR) == 0)		return;
	}

	if((strcasecmp(filename, PROC_FILE_PORT_ISOLATION_MAPPING) == 0) || (strcasecmp(filename, PROC_FILE_HELP) ==0)) {
		RTL8367_MSG("cat /proc/%s/%s", PROC_DIR_RTL8367, PROC_FILE_PORT_ISOLATION_MAPPING);
		RTL8367_MSG("echo num_of_port ext_port_name /proc/%s/%s", PROC_DIR_RTL8367, PROC_FILE_PORT_ISOLATION_MAPPING);
		if(strcasecmp(filename, PROC_FILE_PORT_ISOLATION_MAPPING) == 0)		return;
	}
	
	if((strcasecmp(filename, PROC_FILE_IGMP) == 0) || (strcasecmp(filename, PROC_FILE_HELP) == 0)) {
		RTL8367_MSG("cat /proc/%s/%s", PROC_DIR_RTL8367, PROC_FILE_IGMP);
		RTL8367_MSG("echo init > /proc/%s/%s", PROC_DIR_RTL8367, PROC_FILE_IGMP);
		RTL8367_MSG("echo status [ENABLE/DISABLE] > /proc/%s/%s", PROC_DIR_RTL8367, PROC_FILE_IGMP);
		if(strcasecmp(filename, PROC_FILE_MIRROR) == 0)		return;
	}

	if((strcasecmp(filename, PROC_FILE_L2MCAST) == 0) || (strcasecmp(filename, PROC_FILE_HELP) == 0)) {
		RTL8367_MSG("cat /proc/%s/%s", PROC_DIR_RTL8367, PROC_FILE_L2MCAST);
		RTL8367_MSG("echo [ipMcastAddrLOOKUP_MAC/ipMcastAddrLOOKUP_IP/ipMcastAddrLOOKUP_IP_VID] > /proc/%s/%s", PROC_DIR_RTL8367, PROC_FILE_L2MCAST);
		RTL8367_MSG("echo [addl2mcast/dell2mcast] [address] [mcastMAC] [portmask] [fid] > /proc/%s/%s", PROC_DIR_RTL8367, PROC_FILE_L2MCAST);
		RTL8367_MSG("echo [addl2ipmcast/dell2ipmcast] [sip] [dip] [portmask] > /proc/%s/%s", PROC_DIR_RTL8367, PROC_FILE_L2MCAST);
		if(strcasecmp(filename, PROC_FILE_MIRROR) == 0)		return;
	}
	
	return;
}

static void help_fops(struct seq_file *s, void *v)
{
	rtl8367_usage(PROC_FILE_HELP);
	return;
}

static rtksw_api_ret_t Rtl8367_link_status_fops(struct seq_file *s, void *v)
{
	rtksw_port_t port = UTP_PORT0;
	rtksw_port_linkStatus_t linkStatus;
	rtksw_port_speed_t speed;
	rtksw_port_duplex_t duplex;

	const char *link_status_info[] = {
		"DOWN",
		"UP  ",
		NULL
	};
	const char *speed_info[] = {
		"10M  ",
		"100M ",
		"1000M",
		"500M ",
		"2500M",
		NULL
	};
	const char *duplex_info[] = {
		"HALF",
		"FULL",
		NULL
	};

	for (port = UTP_PORT0 ; port <= UTP_PORT4 ; port++) {
		if (rtksw_port_phyStatus_get(port, &linkStatus, &speed, &duplex) != RT_ERR_OK)	goto link_status_error;
		RTL8367_MSG("[port %d] %s - %s - %s", port, link_status_info[linkStatus], linkStatus ? speed_info[speed] : "     ", linkStatus ? duplex_info[duplex]: "    ");
	}

	return RT_ERR_OK;

link_status_error:
	RTL8367_ERROR("%s() FAIL ......", __FUNCTION__);
	rtl8367_usage(PROC_FILE_LINK_STATUS);
	return RT_ERR_FAILED;
}

static int Rtl8367_svlan_write_fops(struct file *filp, char *buf, size_t count, loff_t *offp)
{
	char option[64];
	int value;
	char *strptr;
	char *tokptr;
	int ret;

	if (buf)
	{
		buf[count] = '\0';
		strptr=buf;
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto svlan_error;
		}
		sprintf(option, "%s", tokptr);
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto svlan_error;
		}
		if(tokptr[strlen(tokptr) - 1] == 0x0a)
			tokptr[strlen(tokptr) - 1] = 0x00;

		if(!strcmp(option, "MCAST"))
		{
			value = simple_strtol(tokptr, NULL, 0);
			ret=RT_ERR_OK;
			svlan_vid_mcast = value;
			RTL8367_MSG("Set MCAST SVLAN VID=%d", svlan_vid_mcast);
		}
		else
		{
			RTL8367_ERROR("invalid option.\n");
			goto svlan_error;
		}
	}
	else
	{
svlan_error:
		rtl8367_usage(PROC_FILE_SVLAN);
		return -EFAULT;
	}

	return count;
}

static rtksw_api_ret_t Rtl8367_svlan_fops(struct seq_file *s, void *v)
{
	rtksw_portmask_t svlan_portmask;
	rtksw_port_t port = UTP_PORT0;
	rtksw_vlan_t svid;
	rtksw_svlan_memberCfg_t svlan_cfg;
	
	if(rtksw_svlan_servicePort_get(&svlan_portmask) != RT_ERR_OK)	goto svlan_error;
	RTL8367_MSG("SVLAN service port=0x%x", svlan_portmask);

	RTL8367_MSG("==========================default SVLAN==========================");
	for (port = UTP_PORT0 ; port < UTP_PORT4 ; port++)
	{
		if (rtksw_svlan_defaultSvlan_get(port, &svid) != RT_ERR_OK)	goto svlan_error;
		RTL8367_MSG("port=%d svid=%d", port, svid);
	}
	RTL8367_MSG("=================================================================");

	RTL8367_MSG("=========================SVLAN member port=========================");
	for (port = UTP_PORT0 ; port < UTP_PORT4 ; port++)
	{
		if (rtksw_svlan_defaultSvlan_get(port, &svid) != RT_ERR_OK)	goto svlan_error;
		if (rtksw_svlan_memberPortEntry_get(svid, &svlan_cfg) != RT_ERR_OK)	goto svlan_error;
		RTL8367_MSG("[port svid=%d]", svlan_cfg.svid);
		RTL8367_MSG("	memberport=0x%x", svlan_cfg.memberport);
		RTL8367_MSG("	untagport=0x%x", svlan_cfg.untagport);
		RTL8367_MSG("	fiden=%d", svlan_cfg.fiden);
		RTL8367_MSG("	fid=%d", svlan_cfg.fid);
		RTL8367_MSG("	priority=%d", svlan_cfg.priority);
		RTL8367_MSG("	efiden=%d", svlan_cfg.efiden);
		RTL8367_MSG("	efid=%d", svlan_cfg.efid);
		RTL8367_MSG("	chk_ivl_svl=%d", svlan_cfg.chk_ivl_svl);
		RTL8367_MSG("	ivl_svl=%d", svlan_cfg.ivl_svl);
	}

	if (svlan_vid_mcast != -1)
	{
		svid = svlan_vid_mcast;
		if (rtksw_svlan_memberPortEntry_get(svid, &svlan_cfg) != RT_ERR_OK)	goto svlan_error;
		RTL8367_MSG("[mcast svid=%d]", svlan_cfg.svid);
		RTL8367_MSG("	memberport=0x%x", svlan_cfg.memberport);
		RTL8367_MSG("	untagport=0x%x", svlan_cfg.untagport);
		RTL8367_MSG("	fiden=%d", svlan_cfg.fiden);
		RTL8367_MSG("	fid=%d", svlan_cfg.fid);
		RTL8367_MSG("	priority=%d", svlan_cfg.priority);
		RTL8367_MSG("	efiden=%d", svlan_cfg.efiden);
		RTL8367_MSG("	efid=%d", svlan_cfg.efid);
		RTL8367_MSG("	chk_ivl_svl=%d", svlan_cfg.chk_ivl_svl);
		RTL8367_MSG("	ivl_svl=%d", svlan_cfg.ivl_svl);
	}
	RTL8367_MSG("==================================================================");

	return RT_ERR_OK;

svlan_error:
	RTL8367_ERROR("%s() FAIL ......", __FUNCTION__);
	rtl8367_usage(PROC_FILE_SVLAN);
	return RT_ERR_FAILED;
}

rtksw_api_ret_t rtk_rtl8367_port_isolation_change(void);
static int Rtl8367_port_isolation_mapping_write_fops(struct file *filp, char *buf, size_t count, loff_t *offp)
{
	unsigned int port_num;
	char *strptr;
	char *tokptr;

	if (buf)
	{
		buf[count] = '\0';
		strptr=buf;
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto port_isolation_error;
		}
		port_num = simple_strtol(tokptr, NULL, 0);
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto port_isolation_error;
		}
		if(tokptr[strlen(tokptr) - 1] == 0x0a)
			tokptr[strlen(tokptr) - 1] = 0x00;

		if(port_num>=RTL8367_MAX_LAN_PORT_NUM)
		{
			RTL8367_MSG("port %d is out of range.\n", port_num);
			goto port_isolation_error;
		}

		if (!strcmp(tokptr, "EXT_PORT0"))
			rtl8367_port_isolation_mapping_table[port_num].extPort = EXT_PORT0;
		else if (!strcmp(tokptr, "EXT_PORT1"))
			rtl8367_port_isolation_mapping_table[port_num].extPort = EXT_PORT1;
		else if (!strcmp(tokptr, "EXT_PORT2"))
			rtl8367_port_isolation_mapping_table[port_num].extPort = EXT_PORT2;
		else if (!strcmp(tokptr, "FID_ENABLE"))
			rtl8367_port_isolation_mapping_table[port_num].setFid = 1;
		else if (!strcmp(tokptr, "FID_DISABLE"))
			rtl8367_port_isolation_mapping_table[port_num].setFid = 0;
		else
			goto port_isolation_error;

		RTL8367_MSG("port %d group to %s\n", port_num, tokptr);
		rtk_rtl8367_port_isolation_change();
	}
	else
	{
port_isolation_error:
		rtl8367_usage(PROC_FILE_PORT_ISOLATION_MAPPING);
		return -EFAULT;
	}
	return count;
}

static rtksw_api_ret_t Rtl8367_port_isolation_mapping_fops(struct seq_file *s, void *v)
{
	int i;

	RTL8367_MSG("=========================port isolation table=========================\n");
	for(i=0 ; i<RTL8367_MAX_LAN_PORT_NUM ; i++)
	{
		if (rtl8367_port_isolation_mapping_table[i].extPort != EXT_PORT0 
		&& rtl8367_port_isolation_mapping_table[i].extPort != EXT_PORT1 
		&& rtl8367_port_isolation_mapping_table[i].extPort != EXT_PORT2)
			continue;

		RTL8367_MSG("port %d <-> ext %s, setFid=%d", i, rtl8367_port_isolation_mapping_table[i].extPort==EXT_PORT0?"EXT_PORT0":((rtl8367_port_isolation_mapping_table[i].extPort==EXT_PORT1)?"EXT_PORT1":"EXT_PORT2"), rtl8367_port_isolation_mapping_table[i].setFid);
	}
	RTL8367_MSG("===========================================================================\n");

	return RT_ERR_OK;
}

static int Rtl8367_stat_write_fops(struct file *filp, char *buf, size_t count, loff_t *offp)
{	
	char tmpbuf[32] = "\0";
	int mode=0;
	rtksw_port_t port = UTP_PORT0;
	
	if (buf)
	{
		buf[count] = '\0';
		mode = simple_strtol(buf, NULL, 0);
	
		if(mode == 1){
			for (port = UTP_PORT0 ; port < RTKSW_PORT_MAX ; port++)
			{
				if(rtksw_stat_port_reset(port) != RT_ERR_OK)	continue;
			}
			printk("All port counter clear!!\n");
		}
	}
	return count;
}

static rtksw_api_ret_t Rtl8367_stat_fops(struct seq_file *s, void *v)
{
	rtksw_stat_port_cntr_t port_cntrs;
	rtksw_port_t port = UTP_PORT0;
	
	RTL8367_MSG("==========================STATISTIC==========================");
	for (port = UTP_PORT0 ; port < RTKSW_PORT_MAX ; port++)
	{
		if(rtksw_stat_port_getAll(port, &port_cntrs) != RT_ERR_OK)	continue;
		RTL8367_MSG("port %d:", port);
		if(port_cntrs.ifInOctets) RTL8367_MSG("	ifInOctets=%d", port_cntrs.ifInOctets);
		if(port_cntrs.dot3StatsFCSErrors) RTL8367_MSG("	dot3StatsFCSErrors=%d", port_cntrs.dot3StatsFCSErrors);
		if(port_cntrs.dot3StatsSymbolErrors) RTL8367_MSG("	dot3StatsSymbolErrors=%d", port_cntrs.dot3StatsSymbolErrors);
		if(port_cntrs.dot3InPauseFrames) RTL8367_MSG("	dot3InPauseFrames=%d", port_cntrs.dot3InPauseFrames);
		if(port_cntrs.dot3ControlInUnknownOpcodes) RTL8367_MSG("	dot3ControlInUnknownOpcodes=%d", port_cntrs.dot3ControlInUnknownOpcodes);
		if(port_cntrs.etherStatsFragments) RTL8367_MSG("	etherStatsFragments=%d", port_cntrs.etherStatsFragments);
		if(port_cntrs.etherStatsJabbers) RTL8367_MSG("	etherStatsJabbers=%d", port_cntrs.etherStatsJabbers);
		if(port_cntrs.ifInUcastPkts) RTL8367_MSG("	ifInUcastPkts=%d", port_cntrs.ifInUcastPkts);
		if(port_cntrs.etherStatsDropEvents) RTL8367_MSG("	etherStatsDropEvents=%d", port_cntrs.etherStatsDropEvents);
		if(port_cntrs.etherStatsOctets) RTL8367_MSG("	etherStatsOctets=%d", port_cntrs.etherStatsOctets);
		if(port_cntrs.etherStatsUndersizePkts) RTL8367_MSG("	etherStatsUndersizePkts=%d", port_cntrs.etherStatsUndersizePkts);
		if(port_cntrs.etherStatsOversizePkts) RTL8367_MSG("	etherStatsOversizePkts=%d", port_cntrs.etherStatsOversizePkts);
		if(port_cntrs.etherStatsPkts64Octets) RTL8367_MSG("	etherStatsPkts64Octets=%d", port_cntrs.etherStatsPkts64Octets);
		if(port_cntrs.etherStatsPkts65to127Octets) RTL8367_MSG("	etherStatsPkts65to127Octets=%d", port_cntrs.etherStatsPkts65to127Octets);
		if(port_cntrs.etherStatsPkts128to255Octets) RTL8367_MSG("	etherStatsPkts128to255Octets=%d", port_cntrs.etherStatsPkts128to255Octets);
		if(port_cntrs.etherStatsPkts256to511Octets) RTL8367_MSG("	etherStatsPkts256to511Octets=%d", port_cntrs.etherStatsPkts256to511Octets);
		if(port_cntrs.etherStatsPkts512to1023Octets) RTL8367_MSG("	etherStatsPkts512to1023Octets=%d", port_cntrs.etherStatsPkts512to1023Octets);
		if(port_cntrs.etherStatsPkts1024toMaxOctets) RTL8367_MSG("	etherStatsPkts1024toMaxOctets=%d", port_cntrs.etherStatsPkts1024toMaxOctets);
		if(port_cntrs.etherStatsMcastPkts) RTL8367_MSG("	etherStatsMcastPkts=%d", port_cntrs.etherStatsMcastPkts);
		if(port_cntrs.etherStatsBcastPkts) RTL8367_MSG("	etherStatsBcastPkts=%d", port_cntrs.etherStatsBcastPkts);
		if(port_cntrs.ifOutOctets) RTL8367_MSG("	ifOutOctets=%d", port_cntrs.ifOutOctets);
		if(port_cntrs.dot3StatsSingleCollisionFrames) RTL8367_MSG("	dot3StatsSingleCollisionFrames=%d", port_cntrs.dot3StatsSingleCollisionFrames);
		if(port_cntrs.dot3StatsMultipleCollisionFrames) RTL8367_MSG("	dot3StatsMultipleCollisionFrames=%d", port_cntrs.dot3StatsMultipleCollisionFrames);
		if(port_cntrs.dot3StatsDeferredTransmissions) RTL8367_MSG("	dot3StatsDeferredTransmissions=%d", port_cntrs.dot3StatsDeferredTransmissions);
		if(port_cntrs.dot3StatsLateCollisions) RTL8367_MSG("	dot3StatsLateCollisions=%d", port_cntrs.dot3StatsLateCollisions);
		if(port_cntrs.etherStatsCollisions) RTL8367_MSG("	etherStatsCollisions=%d", port_cntrs.etherStatsCollisions);
		if(port_cntrs.dot3StatsExcessiveCollisions) RTL8367_MSG("	dot3StatsExcessiveCollisions=%d", port_cntrs.dot3StatsExcessiveCollisions);
		if(port_cntrs.dot3OutPauseFrames) RTL8367_MSG("	dot3OutPauseFrames=%d", port_cntrs.dot3OutPauseFrames);
		if(port_cntrs.dot1dBasePortDelayExceededDiscards) RTL8367_MSG("	dot1dBasePortDelayExceededDiscards=%d", port_cntrs.dot1dBasePortDelayExceededDiscards);
		if(port_cntrs.dot1dTpPortInDiscards) RTL8367_MSG("	dot1dTpPortInDiscards=%d", port_cntrs.dot1dTpPortInDiscards);
		if(port_cntrs.ifOutUcastPkts) RTL8367_MSG("	ifOutUcastPkts=%d", port_cntrs.ifOutUcastPkts);
		if(port_cntrs.ifOutMulticastPkts) RTL8367_MSG("	ifOutMulticastPkts=%d", port_cntrs.ifOutMulticastPkts);
		if(port_cntrs.ifOutBrocastPkts) RTL8367_MSG("	ifOutBrocastPkts=%d", port_cntrs.ifOutBrocastPkts);
		if(port_cntrs.outOampduPkts) RTL8367_MSG("	outOampduPkts=%d", port_cntrs.outOampduPkts);
		if(port_cntrs.inOampduPkts) RTL8367_MSG("	inOampduPkts=%d", port_cntrs.inOampduPkts);
		if(port_cntrs.pktgenPkts) RTL8367_MSG("	pktgenPkts=%d", port_cntrs.pktgenPkts);
		if(port_cntrs.inMldChecksumError) RTL8367_MSG("	inMldChecksumError=%d", port_cntrs.inMldChecksumError);
		if(port_cntrs.inIgmpChecksumError) RTL8367_MSG("	inIgmpChecksumError=%d", port_cntrs.inIgmpChecksumError);
		if(port_cntrs.inMldSpecificQuery) RTL8367_MSG("	inMldSpecificQuery=%d", port_cntrs.inMldSpecificQuery);
		if(port_cntrs.inMldGeneralQuery) RTL8367_MSG("	inMldGeneralQuery=%d", port_cntrs.inMldGeneralQuery);
		if(port_cntrs.inIgmpSpecificQuery) RTL8367_MSG("	inIgmpSpecificQuery=%d", port_cntrs.inIgmpSpecificQuery);
		if(port_cntrs.inIgmpGeneralQuery) RTL8367_MSG("	inIgmpGeneralQuery=%d", port_cntrs.inIgmpGeneralQuery);
		if(port_cntrs.inMldLeaves) RTL8367_MSG("	inMldLeaves=%d", port_cntrs.inMldLeaves);
		if(port_cntrs.inIgmpLeaves) RTL8367_MSG("	inIgmpLeaves=%d", port_cntrs.inIgmpLeaves);
		if(port_cntrs.inIgmpJoinsSuccess) RTL8367_MSG("	inIgmpJoinsSuccess=%d", port_cntrs.inIgmpJoinsSuccess);
		if(port_cntrs.inIgmpJoinsFail) RTL8367_MSG("	inIgmpJoinsFail=%d", port_cntrs.inIgmpJoinsFail);
		if(port_cntrs.inMldJoinsSuccess) RTL8367_MSG("	inMldJoinsSuccess=%d", port_cntrs.inMldJoinsSuccess);
		if(port_cntrs.inMldJoinsFail) RTL8367_MSG("	inMldJoinsFail=%d", port_cntrs.inMldJoinsFail);
		if(port_cntrs.inReportSuppressionDrop) RTL8367_MSG("	inReportSuppressionDrop=%d", port_cntrs.inReportSuppressionDrop);
		if(port_cntrs.inLeaveSuppressionDrop) RTL8367_MSG("	inLeaveSuppressionDrop=%d", port_cntrs.inLeaveSuppressionDrop);
		if(port_cntrs.outIgmpReports) RTL8367_MSG("	outIgmpReports=%d", port_cntrs.outIgmpReports);
		if(port_cntrs.outIgmpLeaves) RTL8367_MSG("	outIgmpLeaves=%d", port_cntrs.outIgmpLeaves);
		if(port_cntrs.outIgmpGeneralQuery) RTL8367_MSG("	outIgmpGeneralQuery=%d", port_cntrs.outIgmpGeneralQuery);
		if(port_cntrs.outIgmpSpecificQuery) RTL8367_MSG("	outIgmpSpecificQuery=%d", port_cntrs.outIgmpSpecificQuery);
		if(port_cntrs.outMldReports) RTL8367_MSG("	outMldReports=%d", port_cntrs.outMldReports);
		if(port_cntrs.outMldLeaves) RTL8367_MSG("	outMldLeaves=%d", port_cntrs.outMldLeaves);
		if(port_cntrs.outMldGeneralQuery) RTL8367_MSG("	outMldGeneralQuery=%d", port_cntrs.outMldGeneralQuery);
		if(port_cntrs.outMldSpecificQuery) RTL8367_MSG("	outMldSpecificQuery=%d", port_cntrs.outMldSpecificQuery);
		if(port_cntrs.inKnownMulticastPkts) RTL8367_MSG("	inKnownMulticastPkts=%d", port_cntrs.inKnownMulticastPkts);
		if(port_cntrs.ifInMulticastPkts) RTL8367_MSG("	ifInMulticastPkts=%d", port_cntrs.ifInMulticastPkts);
		if(port_cntrs.ifInBroadcastPkts) RTL8367_MSG("	ifInBroadcastPkts=%d", port_cntrs.ifInBroadcastPkts);
		if(port_cntrs.ifOutDiscards) RTL8367_MSG("	ifOutDiscards=%d", port_cntrs.ifOutDiscards);
	}
	RTL8367_MSG("=============================================================");

	return RT_ERR_OK;

stat_error:
	RTL8367_ERROR("%s() FAIL ......", __FUNCTION__);
	rtl8367_usage(PROC_FILE_STAT);
	return RT_ERR_FAILED;
}

#ifdef CONFIG_RTL_MULTI_LAN_DEV
static rtksw_api_ret_t Rtl8367_port_scan_read(struct seq_file *s, void *v)
{
	if(rtl8367_port_scan_status==1)
		RTL8367_MSG("status=START\n");
	else
		RTL8367_MSG("status=STOP\n");
	
	return RT_ERR_OK;
}

static int Rtl8367_port_scan_write(struct file *filp, char *buf, size_t count, loff_t *offp)
{	
	char option[64], value[64];
	char *strptr;
	char *tokptr;
	int ret;
	
	if (buf)
	{
		buf[count] = '\0';
		strptr=buf;
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto port_scan_error;
		}
		sprintf(option, "%s", tokptr);
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto port_scan_error;
		}
		if(tokptr[strlen(tokptr) - 1] == 0x0a)
			tokptr[strlen(tokptr) - 1] = 0x00;

		if(!strcmp(option, "status"))
		{
			sprintf(value, "%s", tokptr);
			ret=RT_ERR_OK;
			if(!strcasecmp(value, "START"))
				ret=rtk_rtl8367_port_scan_start();
			else if(!strcasecmp(value, "STOP"))
				ret=rtk_rtl8367_port_scan_stop();
			else
			{
				RTL8367_ERROR("invalid value.\n");
				goto port_scan_error;
			}

			if(ret != RT_ERR_OK)
			{
				RTL8367_ERROR("set port scan status FAIL, ret=%d.\n", ret);
				goto port_scan_error;
			}
		}
		else
		{
			RTL8367_ERROR("invalid option.\n");
			goto port_scan_error;
		}
	}
	else
	{
port_scan_error:
		rtl8367_usage(PROC_FILE_PORT_SCAN);
		return -EFAULT;
	}
		
	return count;
}

static rtksw_api_ret_t Rtl8367_dev_port_mapping_read(struct seq_file *s, void *v)
{
	struct net_device *dev = NULL;
	int i;

	RTL8367_MSG("=========================device port mapping table=========================\n");
	for(i=0 ; i<RTL8367_MAX_LAN_PORT_NUM ; i++)
	{
		dev = NULL;
		dev = __dev_get_by_name(&init_net, rtl8367_dev_table[i].ifname);
		if(dev)
			RTL8367_MSG("port %d <-> dev %s", i, rtl8367_dev_table[i].ifname);
		else if(rtl8367_dev_table[i].ifname[0] != '\0')
			RTL8367_MSG("port %d <-> dev %s (device not found !)", i, rtl8367_dev_table[i].ifname);
	}
	RTL8367_MSG("===========================================================================\n");
	
	return RT_ERR_OK;
}

static struct rtnl_link_stats64 *Rtl8367_dev_get_stats64(struct net_device *dev, struct rtnl_link_stats64 *stats) 
{
	rtksw_stat_port_cntr_t port_cntrs;
	rtksw_port_t port = UTP_PORT0;
//RTL8367_MSG("%s:%d\n", __FUNCTION__, __LINE__);

	if(dev){
		for (port = UTP_PORT0 ; port < RTKSW_PORT_MAX ; port++)
		{
			if(strcmp(rtl8367_dev_table[port].ifname, dev->name)==0 && netif_carrier_ok(dev)){
				//RTL8367_MSG("%s:%d port %d, dev->name %s, rtl8367_dev_table[port].ifname %s\n", __FUNCTION__, __LINE__, port, dev->name, rtl8367_dev_table[port].ifname);
				
				if(rtksw_stat_port_getAll(port, &port_cntrs) != RT_ERR_OK)	break;
				
				//RTL8367_MSG("port %d, dev->name %s", port, dev->name);
				stats->rx_packets = port_cntrs.ifInUcastPkts + port_cntrs.ifInMulticastPkts + port_cntrs.ifInBroadcastPkts;
				stats->tx_packets = port_cntrs.ifOutUcastPkts + port_cntrs.ifOutMulticastPkts + port_cntrs.ifOutBrocastPkts;
				stats->rx_bytes = port_cntrs.ifInOctets;
				stats->tx_bytes = port_cntrs.ifOutOctets;
				stats->rx_errors = port_cntrs.dot3StatsFCSErrors + port_cntrs.dot3StatsSymbolErrors + port_cntrs.dot3ControlInUnknownOpcodes + port_cntrs.etherStatsUndersizePkts + port_cntrs.etherStatsOversizePkts;
				stats->tx_errors = port_cntrs.dot1dBasePortDelayExceededDiscards;
				stats->rx_dropped = port_cntrs.dot1dTpPortInDiscards;
				stats->tx_dropped = port_cntrs.ifOutDiscards;
				stats->multicast = port_cntrs.etherStatsMcastPkts;
				stats->collisions = port_cntrs.etherStatsCollisions;
				
				/* detailed rx_errors: */
				stats->rx_length_errors = port_cntrs.etherStatsUndersizePkts + port_cntrs.etherStatsOversizePkts;
				//stats->rx_over_errors = port_cntrs.
				stats->rx_crc_errors = port_cntrs.dot3StatsFCSErrors;
				stats->rx_frame_errors = port_cntrs.dot3StatsSymbolErrors + port_cntrs.dot3ControlInUnknownOpcodes;
				//stats->rx_fifo_errors = port_cntrs.
				//stats->rx_missed_errors = port_cntrs.
				
				/* detailed tx_errors */
				//stats->tx_aborted_errors = port_cntrs.
				//stats->tx_carrier_errors = port_cntrs.
				//stats->tx_fifo_errors = port_cntrs.
				//stats->tx_heartbeat_errors = port_cntrs.
				//stats->tx_window_errors = port_cntrs.
				
				/* for cslip etc */
				//stats->rx_compressed = port_cntrs.
				//stats->tx_compressed = port_cntrs.
				break;
			}
		}
	}
	
	return &stats;
}

static int Rtl8367_dev_stop(struct net_device *dev)
{
	rtksw_port_t port = UTP_PORT0;
	memset(&dev->stats, 0 ,sizeof(dev->stats));
	
	for (port = UTP_PORT0 ; port < RTKSW_PORT_MAX ; port++)
	{
		if(strcmp(rtl8367_dev_table[port].ifname, dev->name)==0){
			rtksw_stat_port_reset(port);
			break;
		}
	}

	return 0;
}

static int Rtl8367_dev_port_mapping_write(struct file *filp, char *buf, size_t count, loff_t *offp)
{
	unsigned int port_num;
	char *strptr;
	char *tokptr;
	struct net_device *dev = NULL;
#ifdef RTK_NETDEV_PRIV_FLAGS
	u32 flags;
#endif
	
	if (buf)
	{
		buf[count] = '\0';
		strptr=buf;
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto dev_port_mapping_error;
		}
		port_num = simple_strtol(tokptr, NULL, 0);
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto dev_port_mapping_error;
		}
		if(tokptr[strlen(tokptr) - 1] == 0x0a)
			tokptr[strlen(tokptr) - 1] = 0x00;

		if(port_num>=RTL8367_MAX_LAN_PORT_NUM)
		{
			RTL8367_MSG("port %d is out of range.\n", port_num);
			goto dev_port_mapping_error;
		}

		sprintf(rtl8367_dev_table[port_num].ifname, "%s", tokptr);
		RTL8367_MSG("port %d assign to %s\n", port_num, tokptr);
		
		dev = __dev_get_by_name(&init_net, rtl8367_dev_table[port_num].ifname);
		if(dev){
#ifdef RTK_NETDEV_PRIV_FLAGS
			flags = rtk_netdev_get_flags(dev);
			flags |= RTK_IFF_DOMAIN_EXT_ELAN;
			rtk_netdev_set_flags(dev, flags);
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
			dev->priv_flags |= RTK_IFF_DOMAIN_EXT_ELAN;
#else
			dev->priv_flags |= IFF_DOMAIN_EXT_ELAN;
#endif
#endif
			dev->dev_port = CONFIG_EXTERNAL_SWITCH_PORT_OFFSET+port_num;
		}
	}
	else
	{
dev_port_mapping_error:
		rtl8367_usage(PROC_FILE_DEV_PORT_MAPPING);
		return -EFAULT;
	}
		
	return count;
}
#endif

static rtksw_api_ret_t Rtl8367_l2_fops(struct seq_file *s, void *v)
{
	rtksw_uint32  address = 0;
	rtksw_l2_ucastAddr_t l2_data;
	rtksw_port_t port = UTP_PORT0;
	rtksw_api_ret_t retVal;

	RTL8367_MSG("==========================L2==========================");
	while (1)
	{   
		if((retVal=rtksw_l2_addr_next_get(READMETHOD_NEXT_L2UC, UTP_PORT0, &address, &l2_data))!=RT_ERR_OK)
		{
			break;
		}

		RTL8367_MSG("	mac=%02x:%02x:%02x:%02x:%02x:%02x", l2_data.mac.octet[0], l2_data.mac.octet[1], l2_data.mac.octet[2], l2_data.mac.octet[3], l2_data.mac.octet[4], l2_data.mac.octet[5]);
		RTL8367_MSG("	ivl=%u", l2_data.ivl);
		RTL8367_MSG("	cvid=%u", l2_data.cvid);
		RTL8367_MSG("	fid=%u", l2_data.fid);
		RTL8367_MSG("	efid=%u", l2_data.efid);
		RTL8367_MSG("	port=%u", l2_data.port);
		RTL8367_MSG("	sa_block=%u", l2_data.sa_block);
		RTL8367_MSG("	da_block=%u", l2_data.da_block);
		RTL8367_MSG("	auth=%u", l2_data.auth);
		RTL8367_MSG("	is_static=%u", l2_data.is_static);
		RTL8367_MSG("	priority=%u", l2_data.priority);
		RTL8367_MSG("	sa_pri_en=%u", l2_data.sa_pri_en);
		RTL8367_MSG("	fwd_pri_en=%u", l2_data.fwd_pri_en);
		RTL8367_MSG("	address=%u", l2_data.address);
		RTL8367_MSG("=============================================================");
		address++; 
	}
	

	return RT_ERR_OK;

l2_error:
	RTL8367_ERROR("%s() FAIL ......", __FUNCTION__);
	rtl8367_usage(PROC_FILE_L2);
	return RT_ERR_FAILED;
}

static int Rtl8367_asic_reg_write(struct file *filp, char *buf, size_t count, loff_t *offp)
{
	unsigned int operation=0, regaddr_hex=0, regdata_hex=0;
	char *strptr;
	char *tokptr;

	
	if (buf)
	{
		buf[count] = '\0';
		strptr=buf;
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto asic_reg_error;
		}
		if(strcmp(tokptr, "get")==0){
			operation=0;
		}
		else{
			operation=1;
		}
		
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto asic_reg_error;
		}
		if(tokptr[strlen(tokptr) - 1] == 0x0a)
			tokptr[strlen(tokptr) - 1] = 0x00;
		
		regaddr_hex = simple_strtol(tokptr, NULL, 16);
		printk("regaddr_hex is 0x%04x\n", regaddr_hex);
		
		if(operation==0){ //get
#ifdef CONFIG_DAL_RTL8367C
			if(rtl8367c_getAsicReg(regaddr_hex, &regdata_hex)!=RT_ERR_OK){
				goto asic_reg_error;
			}
			printk("data is 0x%04x\n", regdata_hex);
#else
			if(rtl8367d_getAsicReg(regaddr_hex, &regdata_hex)!=RT_ERR_OK){
				goto asic_reg_error;
			}
			printk("data is 0x%04x\n", regdata_hex);
#endif
		}
		else{
			//set
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto asic_reg_error;
			}
			if(tokptr[strlen(tokptr) - 1] == 0x0a)
				tokptr[strlen(tokptr) - 1] = 0x00;
			
			regdata_hex = simple_strtol(tokptr, NULL, 16);
			printk("regdata_hex is 0x%04x\n", regdata_hex);
			
#ifdef CONFIG_DAL_RTL8367C
			if(rtl8367c_setAsicReg(regaddr_hex, regdata_hex)!=RT_ERR_OK){
				goto asic_reg_error;
			}
#else
			if(rtl8367d_setAsicReg(regaddr_hex, regdata_hex)!=RT_ERR_OK){
				goto asic_reg_error;
			}
#endif	
		}
	}
	else
	{
asic_reg_error:
		rtl8367_usage(PROC_FILE_ASIC_REG);
		return -EFAULT;
	}
		
	return count;
}

static int Rtl8367_port_phy_reg_write(struct file *filp, char *buf, size_t count, loff_t *offp)
{
	unsigned int operation=0, port=0, regid=0, regdata_hex=0;
	char *strptr;
	char *tokptr;

	
	if (buf)
	{
		buf[count] = '\0';
		strptr=buf;
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto port_phy_reg_error;
		}
		if(strcmp(tokptr, "get")==0){
			operation=0;
		}
		else{
			operation=1;
		}
		
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto port_phy_reg_error;
		}
		
		port = simple_strtol(tokptr, NULL, 10);
		printk("port is 0x%04x\n", port);
		
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto port_phy_reg_error;
		}
		if(tokptr[strlen(tokptr) - 1] == 0x0a)
			tokptr[strlen(tokptr) - 1] = 0x00;
		
		regid = simple_strtol(tokptr, NULL, 10);
		printk("regid is 0x%04x\n", regid);
		
		if(operation==0){ //get
			if(rtksw_port_phyReg_get(port, regid, &regdata_hex)!=RT_ERR_OK){
				goto port_phy_reg_error;
			}
			printk("data is 0x%04x\n", regdata_hex);
		}
		else{
			//set
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto port_phy_reg_error;
			}
			if(tokptr[strlen(tokptr) - 1] == 0x0a)
				tokptr[strlen(tokptr) - 1] = 0x00;
			
			regdata_hex = simple_strtol(tokptr, NULL, 16);
			printk("regdata_hex is 0x%04x\n", regdata_hex);
			
			if(rtksw_port_phyReg_set(port, regid, regdata_hex)!=RT_ERR_OK){
				goto port_phy_reg_error;
			}
		}
	}
	else
	{
port_phy_reg_error:
		rtl8367_usage(PROC_FILE_PORT_PHY_REG);
		return -EFAULT;
	}
		
	return count;
}

static int Rtl8367_port_phy_ocp_reg_write(struct file *filp, char *buf, size_t count, loff_t *offp)
{
	unsigned int operation=0, port=0, regaddr_hex=0, regdata_hex=0;
	char *strptr;
	char *tokptr;

	
	if (buf)
	{
		buf[count] = '\0';
		strptr=buf;
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto port_phy_ocp_reg_error;
		}
		if(strcmp(tokptr, "get")==0){
			operation=0;
		}
		else{
			operation=1;
		}
		
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto port_phy_ocp_reg_error;
		}
		
		port = simple_strtol(tokptr, NULL, 10);
		printk("port is 0x%04x\n", port);
		
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto port_phy_ocp_reg_error;
		}
		if(tokptr[strlen(tokptr) - 1] == 0x0a)
			tokptr[strlen(tokptr) - 1] = 0x00;
		
		regaddr_hex = simple_strtol(tokptr, NULL, 16);
		printk("regaddr_hex is 0x%04x\n", regaddr_hex);
		
		if(operation==0){ //get
#ifdef CONFIG_DAL_RTL8367C
			if(rtl8367c_getAsicPHYOCPReg(port, regaddr_hex, &regdata_hex)!=RT_ERR_OK){
				goto port_phy_ocp_reg_error;
			}
			printk("data is 0x%04x\n", regdata_hex);
#else
#if 0
			if(rtl8367d_getAsicPHYOCPReg(port, regaddr_hex, &regdata_hex)!=RT_ERR_OK){
				goto port_phy_ocp_reg_error;
			}
			printk("data is 0x%04x\n", regdata_hex);
#endif
#endif
		}
		else{
			//set
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto port_phy_ocp_reg_error;
			}
			if(tokptr[strlen(tokptr) - 1] == 0x0a)
				tokptr[strlen(tokptr) - 1] = 0x00;
			
			regdata_hex = simple_strtol(tokptr, NULL, 16);
			printk("regdata_hex is 0x%04x\n", regdata_hex);
			
#ifdef CONFIG_DAL_RTL8367C
			if(rtl8367c_setAsicPHYOCPReg(port, regaddr_hex, regdata_hex)!=RT_ERR_OK){
				goto port_phy_ocp_reg_error;
			}
#else
#if 0
			if(rtl8367d_setAsicPHYOCPReg(port, regaddr_hex, regdata_hex)!=RT_ERR_OK){
				goto port_phy_ocp_reg_error;
			}
#endif
#endif	
		}
	}
	else
	{
port_phy_ocp_reg_error:
		rtl8367_usage(PROC_FILE_PORT_PHY_OCP_REG);
		return -EFAULT;
	}
		
	return count;
}

static int Rtl8367_port_mirror(struct file *filp, char *buf, size_t count, loff_t *offp)
{
	rtksw_port_t mirroring_port=0;
	unsigned int mirrored_port=0;
	rtksw_portmask_t mirrored_rxmask, mirrored_txmask;
	char *strptr;
	char *tokptr;

	
	if (buf)
	{
		buf[count] = '\0';
		strptr=buf;
		
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto port_mirror_error;
		}
		
		mirroring_port = simple_strtol(tokptr, NULL, 10);
		printk("mirroring_port is 0x%04x\n", mirroring_port);
		
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto port_mirror_error;
		}
		mirrored_port = simple_strtol(tokptr, NULL, 10);
		printk("mirrored_port is 0x%04x\n", mirrored_port);

		rtksw_mirror_override_set(ENABLED, ENABLED, DISABLED);
		rtksw_mirror_isolationLeaky_set(ENABLED, ENABLED);
		rtksw_mirror_vlanLeaky_set(ENABLED, ENABLED);
		RTKSW_PORTMASK_CLEAR(mirrored_rxmask);
    	RTKSW_PORTMASK_PORT_SET(mirrored_rxmask, mirrored_port);
		RTKSW_PORTMASK_CLEAR(mirrored_txmask);
    	RTKSW_PORTMASK_PORT_SET(mirrored_txmask, mirrored_port);
	
		if(rtksw_mirror_portBased_set(mirroring_port, &mirrored_rxmask, &mirrored_txmask)!=RT_ERR_OK){
			goto port_mirror_error;
		}
	}
	else
	{
port_mirror_error:
		rtl8367_usage(PROC_FILE_MIRROR);
		return -EFAULT;
	}
		
	return count;
}

static rtksw_api_ret_t Rtl8367_igmp_read(struct seq_file *s, void *v)
{
	rtksw_enable_t enabled;
	rtksw_api_ret_t ret;

	ret=rtksw_igmp_state_get(&enabled);
	if (ret==RT_ERR_OK)
	{
		printk("status=");
		if (enabled)
			printk("Enabled\n");
		else
			printk("Disabled\n");
	}
	else
		printk("rtksw_igmp_state_get FAIL.\n");

	return RT_ERR_OK;
}

static int Rtl8367_igmp_write(struct file *filp, char *buf, size_t count, loff_t *offp)
{
	rtksw_enable_t enabled;
	char option[64], value[64];
	char *strptr;
	char *tokptr;
	rtksw_api_ret_t ret;

	if (buf)
	{
		buf[count] = '\0';
		strptr=buf;
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto igmp_error;
		}
		sprintf(option, "%s", tokptr);
		if(!strcmp(option, "init"))
		{
			ret=rtksw_igmp_init();
			if(ret != RT_ERR_OK)
			{
				RTL8367_ERROR("set igmp init FAIL, ret=%d.\n", ret);
				goto igmp_error;
			}
			printk("igmp init done.\n");
		}
		else if(!strcmp(option, "status"))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto igmp_error;
			}
			if(tokptr[strlen(tokptr) - 1] == 0x0a)
				tokptr[strlen(tokptr) - 1] = 0x00;

			sprintf(value, "%s", tokptr);
			ret=RT_ERR_OK;
			if(!strcasecmp(value, "ENABLE"))
			{
				enabled = RTKSW_ENABLED;
				ret=rtksw_igmp_state_set(enabled);
			}
			else if(!strcasecmp(value, "DISABLE"))
			{
				enabled = RTKSW_DISABLED;
				ret=rtksw_igmp_state_set(enabled);
			}
			else
			{
				RTL8367_ERROR("invalid value.\n");
				goto igmp_error;
			}

			if(ret != RT_ERR_OK)
			{
				RTL8367_ERROR("set igmp status FAIL, ret=%d.\n", ret);
				goto igmp_error;
			}
			printk("set igmp status %s.\n", (enabled==RTKSW_ENABLED)?"Enabled":"Disabled");
		}
		else
		{
			RTL8367_ERROR("invalid option.\n");
			goto igmp_error;
		}
	}
	else
	{
igmp_error:
		rtl8367_usage(PROC_FILE_IGMP);
		return -EFAULT;
	}

	return count;
}

static rtksw_api_ret_t Rtl8367_l2mcast_read(struct seq_file *s, void *v)
{
	rtksw_l2_ipmc_lookup_type_t type;
	rtksw_l2_mcastAddr_t mcasrAddr;
	rtksw_l2_ipMcastAddr_t ipMcastAddr;
	rtksw_uint32  address = 0;
	rtksw_api_ret_t ret;

	ret= rtksw_l2_ipMcastAddrLookup_get(&type);
	if (ret == RT_ERR_OK)
	{
		if (type == LOOKUP_MAC)
			printk("L2 lookup type=LOOKUP_MAC\n");
		else if (type == LOOKUP_IP)
			printk("L2 lookup type=LOOKUP_IP\n");
		else if (type == LOOKUP_IP_VID)
			printk("L2 lookup type=LOOKUP_IP_VID\n");
	}

	/* Get L2 multicast entry at whole system*/
	printk("===========L2 MCAST TABLE============\n");
	address = 0;
	while(1)
	{
		if((ret= rtksw_l2_mcastAddr_next_get(&address, &mcasrAddr)) != RT_ERR_OK)
		{
			break;
		}
		printk("[%d] mac=%02X:%02X:%02X:%02X:%02X:%02X, portmask=0x%x, fid=%d\n", address, mcasrAddr.mac.octet[0], mcasrAddr.mac.octet[1], mcasrAddr.mac.octet[2], mcasrAddr.mac.octet[3], mcasrAddr.mac.octet[4], mcasrAddr.mac.octet[5], mcasrAddr.portmask, mcasrAddr.fid);
		address++;
	}
	printk("=====================================\n");

	/* Get L3 multicast entry at whole system*/
	printk("==========L2 IP MCAST TABLE==========\n");
	address = 0;
	while(1)
	{
		if((ret= rtksw_l2_ipMcastAddr_next_get(&address, &ipMcastAddr)) != RT_ERR_OK)
		{
			break;
		}
		printk("[%d] sip=%pI4, dip=%pI4, portmask=0x%x\n", address, &(ipMcastAddr.sip), &(ipMcastAddr.dip), ipMcastAddr.portmask);
		address++;
	}
	printk("=====================================\n");

	return RT_ERR_OK;
}

static int Rtl8367_l2mcast_write(struct file *filp, char *buf, size_t count, loff_t *offp)
{
	rtksw_enable_t enabled;
	char option[64], mac[64];
	int address, portmask, fid;
	int macValues[6];
	char *strptr;
	char *tokptr;
	rtksw_api_ret_t ret;

	if (buf)
	{
		buf[count] = '\0';
		strptr=buf;
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto l2mcast_error;
		}
		sprintf(option, "%s", tokptr);
		if(!strcmp(option, "addl2mcast") || !strcmp(option, "dell2mcast"))
		{
			rtksw_l2_mcastAddr_t mcastAddr = {0};

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto l2mcast_error;
			}
			address = simple_strtol(tokptr, NULL, 10);
			mcastAddr.address = address;

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto l2mcast_error;
			}

			sprintf(mac, "%s", tokptr);
			if(sscanf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", &(macValues[0]), &(macValues[1]), &(macValues[2]), &(macValues[3]), &(macValues[4]), &(macValues[5])) == 6)
			{
				mcastAddr.mac.octet[0] = macValues[0];
				mcastAddr.mac.octet[1] = macValues[1];
				mcastAddr.mac.octet[2] = macValues[2];
				mcastAddr.mac.octet[3] = macValues[3];
				mcastAddr.mac.octet[4] = macValues[4];
				mcastAddr.mac.octet[5] = macValues[5];
			}
			else if (sscanf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", &(macValues[0]), &(macValues[1]), &(macValues[2]), &(macValues[3]), &(macValues[4]), &(macValues[5])) == 6)
			{
				mcastAddr.mac.octet[0] = macValues[0];
				mcastAddr.mac.octet[1] = macValues[1];
				mcastAddr.mac.octet[2] = macValues[2];
				mcastAddr.mac.octet[3] = macValues[3];
				mcastAddr.mac.octet[4] = macValues[4];
				mcastAddr.mac.octet[5] = macValues[5];
			}
			else
			{
				RTL8367_ERROR("should be xx:xx:xx:xx:xx:xx format\n");
				goto l2mcast_error;
			}

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto l2mcast_error;
			}

			portmask = simple_strtol(tokptr, NULL, 16);
			mcastAddr.portmask.bits[0] = portmask;

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto l2mcast_error;
			}

			if(tokptr[strlen(tokptr) - 1] == 0x0a)
				tokptr[strlen(tokptr) - 1] = 0x00;

			fid = simple_strtol(tokptr, NULL, 16);
			mcastAddr.fid  = fid;

			ret=RT_ERR_OK;
			if (!strcmp(option, "addl2mcast"))
			{
				ret=rtksw_l2_mcastAddr_add(&mcastAddr);
				if(ret != RT_ERR_OK)
				{
					RTL8367_ERROR("rtksw_l2_mcastAddr_add FAIL, ret=%d.\n", ret);
					goto l2mcast_error;
				}
				printk("set addl2mcast address=%d mac=%s portmask=0x%x fid=%d.\n", address, mac, portmask, fid);
			}
			else
			{
				ret=rtksw_l2_mcastAddr_del(&mcastAddr);
				if(ret != RT_ERR_OK)
				{
					RTL8367_ERROR("rtksw_l2_mcastAddr_del FAIL, ret=%d.\n", ret);
					goto l2mcast_error;
				}
				printk("set dell2mcast address=%d mac=%s.\n", address, mac);
			}
		}
		else if(!strcmp(option, "addl2ipmcast") || !strcmp(option, "dell2ipmcast"))
		{
			rtksw_l2_ipMcastAddr_t ipMcastAddr = {0};

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto l2mcast_error;
			}
			address = simple_strtol(tokptr, NULL, 10);
			ipMcastAddr.address = address;

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto l2mcast_error;
			}

			ipMcastAddr.sip = in_aton(tokptr);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto l2mcast_error;
			}

			ipMcastAddr.dip = in_aton(tokptr);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto l2mcast_error;
			}

			if(tokptr[strlen(tokptr) - 1] == 0x0a)
				tokptr[strlen(tokptr) - 1] = 0x00;

			portmask = simple_strtol(tokptr, NULL, 16);
			ipMcastAddr.portmask.bits[0] = portmask;

			ret=RT_ERR_OK;
			if (!strcmp(option, "addl2ipmcast"))
			{
				ret=rtksw_l2_ipMcastAddr_add(&ipMcastAddr);
				if(ret != RT_ERR_OK)
				{
					RTL8367_ERROR("rtksw_l2_ipMcastAddr_add FAIL, ret=%d.\n", ret);
					goto l2mcast_error;
				}
				printk("set addl2ipmcast address=%d sip=%pI4 dip=%pI4 portmask=0x%x.\n", address, &(ipMcastAddr.sip), &(ipMcastAddr.dip), ipMcastAddr.portmask.bits[0]);
			}
			else
			{
				ret=rtksw_l2_ipMcastAddr_del(&ipMcastAddr);
				if(ret != RT_ERR_OK)
				{
					RTL8367_ERROR("rtksw_l2_ipMcastAddr_del FAIL, ret=%d.\n", ret);
					goto l2mcast_error;
				}
				printk("set dell2ipmcast address=%d sip=%pI4 dip=%pI4.\n", address, &(ipMcastAddr.sip), &(ipMcastAddr.dip));
			}
		}
		else if (!strcmp(option, "ipMcastAddrLOOKUP_MAC"))
		{
			ret=rtksw_l2_ipMcastAddrLookup_set(LOOKUP_MAC);
			if(ret != RT_ERR_OK)
			{
				RTL8367_ERROR("rtksw_l2_ipMcastAddrLookup_set FAIL, ret=%d.\n", ret);
				goto l2mcast_error;
			}
			printk("set LOOKUP_MAC OK.\n", address, mac);
		}
		else if (!strcmp(option, "ipMcastAddrLOOKUP_IP"))
		{
			ret=rtksw_l2_ipMcastAddrLookup_set(LOOKUP_IP);
			if(ret != RT_ERR_OK)
			{
				RTL8367_ERROR("rtksw_l2_ipMcastAddrLookup_set FAIL, ret=%d.\n", ret);
				goto l2mcast_error;
			}
			printk("set LOOKUP_IP OK.\n", address, mac);
		}
		else if (!strcmp(option, "ipMcastAddrLOOKUP_IP_VID"))
		{
			ret=rtksw_l2_ipMcastAddrLookup_set(LOOKUP_IP_VID);
			if(ret != RT_ERR_OK)
			{
				RTL8367_ERROR("rtksw_l2_ipMcastAddrLookup_set FAIL, ret=%d.\n", ret);
				goto l2mcast_error;
			}
			printk("set LOOKUP_IP_VID OK.\n", address, mac);
		}
		else
		{
			RTL8367_ERROR("invalid option.\n");
			goto l2mcast_error;
		}
	}
	else
	{
l2mcast_error:
		rtl8367_usage(PROC_FILE_L2MCAST);
		return -EFAULT;
	}

	return count;
}

/*
 * rtl8367 proc definition
 */
typedef enum rtk_rtl8367_procDir_e
{
	PROC_DIR_RTL8367_ROOT,
	PROC_DIR_RTL8367_LEEF //this field must put at last.
} rtk_rtl8367_procDir_t;

typedef struct rtk_rtl8367_proc_s
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
	rtk_rtl8367_procDir_t dir;
}rtk_rtl8367_proc_t;

rtk_rtl8367_proc_t rtl8367Proc[]=
{
	{
		.name= PROC_FILE_HELP,
		.get = help_fops,
		.set = NULL,
		.dir = PROC_DIR_RTL8367_ROOT,
	},
	{
		.name= PROC_FILE_LINK_STATUS,
		.get = Rtl8367_link_status_fops,
		.set = NULL,
		.dir = PROC_DIR_RTL8367_ROOT,
	},
	{
		.name= PROC_FILE_SVLAN,
		.get = Rtl8367_svlan_fops,
		.set = Rtl8367_svlan_write_fops,
		.dir = PROC_DIR_RTL8367_ROOT,
	},
	{
		.name= PROC_FILE_PORT_ISOLATION_MAPPING,
		.get = Rtl8367_port_isolation_mapping_fops,
		.set = Rtl8367_port_isolation_mapping_write_fops,
		.dir = PROC_DIR_RTL8367_ROOT,
	},
	{
		.name= PROC_FILE_STAT,
		.get = Rtl8367_stat_fops,
		.set = Rtl8367_stat_write_fops,
		.dir = PROC_DIR_RTL8367_ROOT,
	},
#ifdef CONFIG_RTL_MULTI_LAN_DEV
	{
		.name= PROC_FILE_PORT_SCAN,
		.get = Rtl8367_port_scan_read,
		.set = Rtl8367_port_scan_write,
		.dir = PROC_DIR_RTL8367_ROOT,
	},
	{
		.name= PROC_FILE_DEV_PORT_MAPPING,
		.get = Rtl8367_dev_port_mapping_read,
		.set = Rtl8367_dev_port_mapping_write,
		.dir = PROC_DIR_RTL8367_ROOT,
	},
#endif
	{
		.name= PROC_FILE_L2,
		.get = Rtl8367_l2_fops,
		.set = NULL,
		.dir = PROC_DIR_RTL8367_ROOT,
	},
	{
		.name= PROC_FILE_ASIC_REG,
		.get = NULL,
		.set = Rtl8367_asic_reg_write,
		.dir = PROC_DIR_RTL8367_ROOT,
	},
	{
		.name= PROC_FILE_PORT_PHY_REG,
		.get = NULL,
		.set = Rtl8367_port_phy_reg_write,
		.dir = PROC_DIR_RTL8367_ROOT,
	},
	{
		.name= PROC_FILE_PORT_PHY_OCP_REG,
		.get = NULL,
		.set = Rtl8367_port_phy_ocp_reg_write,
		.dir = PROC_DIR_RTL8367_ROOT,
	},
	{
		.name= PROC_FILE_MIRROR,
		.get = NULL,
		.set = Rtl8367_port_mirror,
		.dir = PROC_DIR_RTL8367_ROOT,
	},
	{
		.name= PROC_FILE_IGMP,
		.get = Rtl8367_igmp_read,
		.set = Rtl8367_igmp_write,
		.dir = PROC_DIR_RTL8367_ROOT,
	},
	{
		.name= PROC_FILE_L2MCAST,
		.get = Rtl8367_l2mcast_read,
		.set = Rtl8367_l2mcast_write,
		.dir = PROC_DIR_RTL8367_ROOT,
	},
};

/*
 * rtl8367 common proc function
 */
static void *rtk_rtl8367_single_start(struct seq_file *p, loff_t *pos)
{
	return NULL + (*pos == 0);
}

static void *rtk_rtl8367_single_next(struct seq_file *p, void *v, loff_t *pos)
{
	++*pos;
	return NULL;
}

static void rtk_rtl8367_single_stop(struct seq_file *p, void *v)
{
}

int rtk_rtl8367_seq_open(struct file *file, const struct seq_operations *op)
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

int rtk_rtl8367_single_open(struct file *file, int (*show) (struct seq_file *m, void *v), void *data)
{
	struct seq_operations *op = kmalloc(sizeof(*op), GFP_ATOMIC);
	int res = -ENOMEM;

	if (op) {
		op->start = rtk_rtl8367_single_start;
		op->next = rtk_rtl8367_single_next;
		op->stop = rtk_rtl8367_single_stop;
		op->show = show;
		res = rtk_rtl8367_seq_open(file, op);
		if (!res)
			((struct seq_file *)file->private_data)->private = data;
		else
			kfree(op);
	}
	return res;
}

static int rtk_rtl8367_nullDebugGet(struct seq_file *s, void *v)
{
	return 0;
}

static int rtk_rtl8367_nullDebugSingleOpen(struct inode *inode, struct file *file)
{
       return(single_open(file, rtk_rtl8367_nullDebugGet, NULL));
}

static int rtk_rtl8367_commonDebugSingleOpen(struct inode *inode, struct file *file)
{
	int i, ret = -1;

	//rtl8367_spin_lock_bh(rtl8367Sysdb.lock_rtl8367);
	//========================= Critical Section Start =========================//
	for( i = 0; i < (sizeof(rtl8367Proc) / sizeof(rtk_rtl8367_proc_t)) ; i++) {
		//RTL8367_MSG("common_single_open inode_id=%u i_ino=%u", rtl8367Proc[i].inode_id,(unsigned int)inode->i_ino);

		if(rtl8367Proc[i].inode_id == (unsigned int)inode->i_ino) {
			ret = rtk_rtl8367_single_open(file, rtl8367Proc[i].get, NULL);
			break;
		}
	}
	//========================= Critical Section End =========================//
	//rtl8367_spin_unlock_bh(rtl8367Sysdb.lock_rtl8367);

	return ret;
}

static ssize_t rtk_rtl8367_commonDebugSingleWrite(struct file * file, const char __user * userbuf,
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

	//rtl8367_spin_lock_bh(rtl8367Sysdb.lock_rtl8367);
	//========================= Critical Section Start =========================//
	for( i = 0; i < (sizeof(rtl8367Proc) / sizeof(rtk_rtl8367_proc_t)) ; i++) {
		//RTL8367_MSG("common_single_write inode_id=%u i_ino=%u",rtl8367Proc[i].inode_id,(unsigned int)file->f_dentry->d_inode->i_ino);

		if(rtl8367Proc[i].inode_id == (unsigned int)file->f_inode->i_ino) {
			//if(rtl8367Proc[i].unlockBefortWrite)
			//	rtl8367_spin_unlock_bh(rtl8367Sysdb.lock_rtl8367);
			ret = rtl8367Proc[i].set(file, pBuffer, count, off);
			break;
		}
	}
	//========================= Critical Section End =========================//
	//if((i!=(sizeof(rtl8367Proc)/sizeof(rtk_rtl8367_proc_t))) && !rtl8367Proc[i].unlockBefortWrite)
	//	rtl8367_spin_unlock_bh(rtl8367Sysdb.lock_rtl8367);

	return ret;
}

rtksw_api_ret_t rtk_rtl8367_proc_init(void)
{
	int i = 0;
	struct proc_dir_entry *p = NULL;

	rtl8367_proc_dir = proc_mkdir(PROC_DIR_RTL8367, NULL);
	if (rtl8367_proc_dir == NULL) {
		RTL8367_ERROR("create /proc/%s failed!", PROC_DIR_RTL8367);
		return RT_ERR_FAILED;
	}

	for(i = 0; i < (sizeof(rtl8367Proc)/sizeof(rtk_rtl8367_proc_t)) ; i++)
	{
		struct proc_dir_entry *parentDir = NULL;

		if(rtl8367Proc[i].get == NULL){
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
			rtl8367Proc[i].proc_fops.proc_open = rtk_rtl8367_nullDebugSingleOpen;
#else
			rtl8367Proc[i].proc_fops.open = rtk_rtl8367_nullDebugSingleOpen;
#endif
		}
		else{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
			rtl8367Proc[i].proc_fops.proc_open = rtk_rtl8367_commonDebugSingleOpen;
#else
			rtl8367Proc[i].proc_fops.open = rtk_rtl8367_commonDebugSingleOpen;
#endif
		}

		if(rtl8367Proc[i].set == NULL){
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
			rtl8367Proc[i].proc_fops.proc_write = NULL;
#else
			rtl8367Proc[i].proc_fops.write = NULL;
#endif
		}
		else{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
			rtl8367Proc[i].proc_fops.proc_write = rtk_rtl8367_commonDebugSingleWrite;
#else
			rtl8367Proc[i].proc_fops.write = rtk_rtl8367_commonDebugSingleWrite;
#endif
		}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
		rtl8367Proc[i].proc_fops.proc_read = seq_read;
		rtl8367Proc[i].proc_fops.proc_lseek = seq_lseek;
		rtl8367Proc[i].proc_fops.proc_release = single_release;
#else
		rtl8367Proc[i].proc_fops.read = seq_read;
		rtl8367Proc[i].proc_fops.llseek = seq_lseek;
		rtl8367Proc[i].proc_fops.release = single_release;
#endif
		switch(rtl8367Proc[i].dir)
		{
			case PROC_DIR_RTL8367_ROOT:
				parentDir = rtl8367_proc_dir;
				break;
			default:
				break;
		}

		p = proc_create_data(rtl8367Proc[i].name, S_IRUGO, parentDir, &(rtl8367Proc[i].proc_fops),NULL);
		if(!p){
			RTL8367_ERROR("create proc %s failed!", rtl8367Proc[i].name);
		}
		rtl8367Proc[i].inode_id = p->low_ino;
	}
	RTL8367_MSG("Creat %d proc entry.", i);

	return RT_ERR_OK;
}

rtksw_api_ret_t rtk_rtl8367_test_init(void)
{
	rtksw_api_ret_t retVal;
	rtksw_svlan_memberCfg_t svlanCfg;

	/* SVLAN initialization */
	if ((retVal = rtksw_svlan_init()) != RT_ERR_OK)
	    return retVal;

	/* Setup uplink port = EXT_PORT0 */
	if ((retVal = rtksw_svlan_servicePort_add(EXT_PORT0)) != RT_ERR_OK)
	    return retVal;

	/* Setup SVLAN 4001 ~ 4004 */
	memset(&svlanCfg, 0x00, sizeof(rtksw_svlan_memberCfg_t));
	RTKSW_PORTMASK_PORT_SET(svlanCfg.memberport, EXT_PORT0);
	RTKSW_PORTMASK_PORT_SET(svlanCfg.memberport, UTP_PORT0);
	RTKSW_PORTMASK_PORT_SET(svlanCfg.untagport, UTP_PORT0);
	if ((retVal = rtksw_svlan_memberPortEntry_set(4001, &svlanCfg)) != RT_ERR_OK)
	    return retVal;

	memset(&svlanCfg, 0x00, sizeof(rtksw_svlan_memberCfg_t));
	RTKSW_PORTMASK_PORT_SET(svlanCfg.memberport, EXT_PORT0);
	RTKSW_PORTMASK_PORT_SET(svlanCfg.memberport, UTP_PORT1);
	RTKSW_PORTMASK_PORT_SET(svlanCfg.untagport, UTP_PORT1);
	if ((retVal = rtksw_svlan_memberPortEntry_set(4002, &svlanCfg)) != RT_ERR_OK)
	    return retVal;

	memset(&svlanCfg, 0x00, sizeof(rtksw_svlan_memberCfg_t));
	RTKSW_PORTMASK_PORT_SET(svlanCfg.memberport, EXT_PORT0);
	RTKSW_PORTMASK_PORT_SET(svlanCfg.memberport, UTP_PORT2);
	RTKSW_PORTMASK_PORT_SET(svlanCfg.untagport, UTP_PORT2);
	if ((retVal = rtksw_svlan_memberPortEntry_set(4003, &svlanCfg)) != RT_ERR_OK)
	    return retVal;

	memset(&svlanCfg, 0x00, sizeof(rtksw_svlan_memberCfg_t));
	RTKSW_PORTMASK_PORT_SET(svlanCfg.memberport, EXT_PORT0);
	RTKSW_PORTMASK_PORT_SET(svlanCfg.memberport, UTP_PORT3);
	RTKSW_PORTMASK_PORT_SET(svlanCfg.untagport, UTP_PORT3);
	if ((retVal = rtksw_svlan_memberPortEntry_set(4004, &svlanCfg)) != RT_ERR_OK)
	    return retVal;

	/* Setup UTP port default SVLAN */
	if ((retVal = rtksw_svlan_defaultSvlan_set(UTP_PORT0, 4001)) != RT_ERR_OK)
	    return retVal;

	if ((retVal = rtksw_svlan_defaultSvlan_set(UTP_PORT1, 4002)) != RT_ERR_OK)
	    return retVal;

	if ((retVal = rtksw_svlan_defaultSvlan_set(UTP_PORT2, 4003)) != RT_ERR_OK)
	    return retVal;

	if ((retVal = rtksw_svlan_defaultSvlan_set(UTP_PORT3, 4004)) != RT_ERR_OK)
	    return retVal;

	return RT_ERR_OK;
}

#ifdef CONFIG_RTL_MULTI_LAN_DEV
static int rtk_rtl8367_port_link_scan(void)
{
    struct net_device *dev = NULL;
    rtksw_port_t port = UTP_PORT0;
    rtksw_port_linkStatus_t linkStatus;
    rtksw_port_speed_t speed;
    rtksw_port_duplex_t duplex;

    for (port = UTP_PORT0 ; port < UTP_PORT4 ; port++)
    {
        if (rtksw_port_phyStatus_get(port, &linkStatus, &speed, &duplex) != RT_ERR_OK)  goto port_link_scan_error;

        dev = NULL;
        dev = dev_get_by_name(&init_net, rtl8367_dev_table[port].ifname);
        if(dev)
        {
            if(linkStatus && !netif_carrier_ok(dev))

            {
#ifdef CONFIG_EXTERNAL_SWITCH
#ifdef CONFIG_SDK_MODULES
                queue_broadcast(MSG_TYPE_LINK_CHANGE, INTR_STATUS_LINKUP, EXT_SWITCH_ORT_TO_RT_PORT(port), ENABLED);
#endif
#endif
                RTL8367_MSG("%s carrier ON.", dev->name);
                netif_carrier_on(dev);
            }
            else if(!linkStatus && netif_carrier_ok(dev))
            {
#ifdef CONFIG_EXTERNAL_SWITCH
#ifdef CONFIG_SDK_MODULES
                queue_broadcast(MSG_TYPE_LINK_CHANGE, INTR_STATUS_LINKDOWN, EXT_SWITCH_ORT_TO_RT_PORT(port), ENABLED);
#endif
#endif
                RTL8367_MSG("%s carrier OFF.", dev->name);
                netif_carrier_off(dev);
            }
            dev_put(dev);
        }
    }

	return RT_ERR_OK;
	
port_link_scan_error:
	RTL8367_ERROR("error: port_link_scan_error.");
	return RT_ERR_FAILED;
}

static int rtk_rtl8367_port_scan_timer_func(void *data)
{
	RTL8367_MSG("port scan thread create successfully.");

    while(rtl8367_port_scan_status)
	{
        msleep(RE8637_PORT_LINK_SCAN_INTERVAL);
        rtk_rtl8367_port_link_scan();
    }

	RTL8367_MSG("port scan thread exit successfully.");

	return RT_ERR_OK;
}

rtksw_api_ret_t rtk_rtl8367_port_scan_start(void)
{
    rtl8367_port_scan_task = kthread_create(rtk_rtl8367_port_scan_timer_func, NULL, "rtl8367_port_scan");
    if (WARN_ON(!rtl8367_port_scan_task))
	{
		RTL8367_ERROR("error: create rtl8367_port_scan thread fail.");
        return RT_ERR_FAILED;
    }
	kthread_bind(rtl8367_port_scan_task, 0);
	wake_up_process(rtl8367_port_scan_task);

	rtl8367_port_scan_status = 1;

	RTL8367_MSG("started.");

    return RT_ERR_OK;
}

rtksw_api_ret_t rtk_rtl8367_port_scan_stop(void)
{
	rtl8367_port_scan_status = 0;

	RTL8367_MSG("stoped.");
	kthread_stop(rtl8367_port_scan_task);
	
	return RT_ERR_OK;
}

rtksw_api_ret_t rtk_rtl8367_dev_port_mapping_init(void)
{
	memset(&rtl8367_dev_table, 0x00, sizeof(struct rtl8367_dev_table_entry)*RTL8367_MAX_LAN_PORT_NUM);
	return RT_ERR_OK;
}
#endif

rtksw_api_ret_t rtk_rtl8367_port_isolation_init(void)

 {
	rtksw_api_ret_t retVal;
	rtksw_portmask_t portmask;
        
#if defined(CONFIG_DAL_RTL8367D)
	RTL8367_INFO("rtk_rtl8367_port_isolation_init for RTL8367D\n");

	RTKSW_PORTMASK_CLEAR(portmask);
	RTKSW_PORTMASK_PORT_SET(portmask, EXT_PORT0);
	RTKSW_PORTMASK_PORT_SET(portmask, UTP_PORT0);
	rtksw_port_isolation_set(UTP_PORT0, &portmask);

	RTKSW_PORTMASK_CLEAR(portmask);
	RTKSW_PORTMASK_PORT_SET(portmask, EXT_PORT0);
	RTKSW_PORTMASK_PORT_SET(portmask, UTP_PORT1);
	rtksw_port_isolation_set(UTP_PORT1, &portmask);

	RTKSW_PORTMASK_CLEAR(portmask);
	RTKSW_PORTMASK_PORT_SET(portmask, EXT_PORT0);
	RTKSW_PORTMASK_PORT_SET(portmask, UTP_PORT2);
	rtksw_port_isolation_set(UTP_PORT2, &portmask);

	RTKSW_PORTMASK_CLEAR(portmask);
	RTKSW_PORTMASK_PORT_SET(portmask, EXT_PORT0);
	RTKSW_PORTMASK_PORT_SET(portmask, UTP_PORT3);
	rtksw_port_isolation_set(UTP_PORT3, &portmask);

	if ((retVal = rtksw_l2_limitLearningCnt_set(UTP_PORT0, 0)) != RT_ERR_OK)
	{
		RTL8367_ERROR("rtksw_l2_limitLearningCnt_set on port %d failed\n", UTP_PORT0);
		return retVal;
	}

	if ((retVal = rtksw_l2_limitLearningCnt_set(UTP_PORT1, 0)) != RT_ERR_OK)
	{
		RTL8367_ERROR("rtksw_l2_limitLearningCnt_set on port %d failed\n", UTP_PORT1);
		return retVal;
	}

	if ((retVal = rtksw_l2_limitLearningCnt_set(UTP_PORT2, 0)) != RT_ERR_OK)
	{
		RTL8367_ERROR("rtksw_l2_limitLearningCnt_set on port %d failed\n", UTP_PORT2);
		return retVal;
	}

	if ((retVal = rtksw_l2_limitLearningCnt_set(UTP_PORT3, 0)) != RT_ERR_OK)
	{
		RTL8367_ERROR("rtksw_l2_limitLearningCnt_set on port %d failed\n", UTP_PORT3);
		return retVal;
	}
	
	if ((retVal = rtksw_l2_limitLearningCnt_set(EXT_PORT0, 0)) != RT_ERR_OK)
	{
		RTL8367_ERROR("rtksw_l2_limitLearningCnt_set on port %d failed\n", EXT_PORT0);
		return retVal;
	}
#elif defined(CONFIG_DAL_RTL8367C)
	RTL8367_INFO("rtk_rtl8367_port_isolation_init for RTL8367C\n");

	RTKSW_PORTMASK_CLEAR(portmask);
    RTKSW_PORTMASK_PORT_SET(portmask, EXT_PORT0);
    if ((retVal = rtksw_port_isolation_set(UTP_PORT0, &portmask)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtksw_port_isolation_set(UTP_PORT1, &portmask)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtksw_port_isolation_set(UTP_PORT2, &portmask)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtksw_port_isolation_set(UTP_PORT3, &portmask)) != RT_ERR_OK)
        return retVal;

	if ((retVal = rtksw_vlan_portFid_set(UTP_PORT0, ENABLED, 1)) != RT_ERR_OK)
	{
		RTL8367_ERROR("rtksw_vlan_portFid_set on port %d failed\n", UTP_PORT0);
		return retVal;
	}

	if ((retVal = rtksw_vlan_portFid_set(UTP_PORT1, ENABLED, 2)) != RT_ERR_OK)
	{
		RTL8367_ERROR("rtksw_vlan_portFid_set on port %d failed\n", UTP_PORT1);
		return retVal;
	}
	
	if ((retVal = rtksw_vlan_portFid_set(UTP_PORT2, ENABLED, 3)) != RT_ERR_OK)
	{
		RTL8367_ERROR("rtksw_vlan_portFid_set on port %d failed\n", UTP_PORT2);
		return retVal;
	}
	
	if ((retVal = rtksw_vlan_portFid_set(UTP_PORT3, ENABLED, 4)) != RT_ERR_OK)
	{
		RTL8367_ERROR("rtksw_vlan_portFid_set on port %d failed\n", UTP_PORT3);
		return retVal;
	}
	
	if ((retVal = rtksw_vlan_portFid_set(EXT_PORT0, ENABLED, 5)) != RT_ERR_OK)
	{
		RTL8367_ERROR("rtksw_vlan_portFid_set on port %d failed\n", EXT_PORT0);
		return retVal;
	}

	if ((retVal = rtksw_l2_limitSystemLearningCntPortMask_set(&portmask)) != RT_ERR_OK)
	{
		RTL8367_ERROR("rtksw_l2_limitSystemLearningCntPortMask_set on port %d failed\n", EXT_PORT0);
		return retVal;
	}
	
	if ((retVal = rtksw_l2_limitSystemLearningCntAction_set(LIMIT_LEARN_CNT_ACTION_FORWARD)) != RT_ERR_OK)
	{
		RTL8367_ERROR("rtksw_l2_limitSystemLearningCntAction_set failed\n");
		return retVal;
	}

	if ((retVal = rtksw_l2_limitSystemLearningCnt_set(0)) != RT_ERR_OK)
	{
		RTL8367_ERROR("rtksw_l2_limitSystemLearningCntAction_set failed\n");
		return retVal;
	}

	/* Flush L2 table */
	if ((retVal = rtksw_l2_table_clear()) != RT_ERR_OK)
		return retVal;
#endif
        
	return RT_ERR_OK;
}

rtksw_api_ret_t rtk_rtl8367_port_isolation_change(void)
{
	rtksw_api_ret_t retVal;
	rtksw_portmask_t portmask;
	int i;

#if defined(CONFIG_DAL_RTL8367D)
	for(i=0 ; i<RTL8367_MAX_LAN_PORT_NUM ; i++)
	{
		if (rtl8367_port_isolation_mapping_table[i].extPort != EXT_PORT0 
			&& rtl8367_port_isolation_mapping_table[i].extPort != EXT_PORT1 
			&& rtl8367_port_isolation_mapping_table[i].extPort != EXT_PORT2)
			continue;

		RTKSW_PORTMASK_CLEAR(portmask);
		RTKSW_PORTMASK_PORT_SET(portmask, rtl8367_port_isolation_mapping_table[i].extPort);
		RTKSW_PORTMASK_PORT_SET(portmask, i);
		rtksw_port_isolation_set(i, &portmask);

		if ((retVal = rtksw_l2_limitLearningCnt_set(i, 0)) != RT_ERR_OK)
		{
			RTL8367_ERROR("rtksw_l2_limitLearningCnt_set on port %d failed\n", rtl8367_port_isolation_mapping_table[i].extPort);
			return retVal;
		}

		if ((retVal = rtksw_l2_limitLearningCnt_set(rtl8367_port_isolation_mapping_table[i].extPort, 0)) != RT_ERR_OK)
		{
			RTL8367_ERROR("rtksw_l2_limitLearningCnt_set on port %d failed\n", rtl8367_port_isolation_mapping_table[i].extPort);
			return retVal;
		}
	}
#elif defined(CONFIG_DAL_RTL8367C)
	for(i=0 ; i<RTL8367_MAX_LAN_PORT_NUM ; i++)
	{
		if (rtl8367_port_isolation_mapping_table[i].extPort != EXT_PORT0 
			&& rtl8367_port_isolation_mapping_table[i].extPort != EXT_PORT1 
			&& rtl8367_port_isolation_mapping_table[i].extPort != EXT_PORT2)
			continue;

		RTKSW_PORTMASK_CLEAR(portmask);
		RTKSW_PORTMASK_PORT_SET(portmask, rtl8367_port_isolation_mapping_table[i].extPort);
		RTKSW_PORTMASK_PORT_SET(portmask, i);
		rtksw_port_isolation_set(i, &portmask);
		if (rtl8367_port_isolation_mapping_table[i].setFid)
		{
			if ((retVal = rtksw_vlan_portFid_set(i, ENABLED, 1+i)) != RT_ERR_OK)
			{
				RTL8367_ERROR("rtksw_vlan_portFid_set on port %d failed\n", UTP_PORT0);
				return retVal;
			}
		}
		else
		{
			if ((retVal = rtksw_vlan_portFid_set(i, DISABLED, 0)) != RT_ERR_OK)
			{
				RTL8367_ERROR("rtksw_vlan_portFid_set on port %d failed\n", UTP_PORT0);
				return retVal;
			}
		}

		if ((retVal = rtksw_l2_limitLearningCnt_set(rtl8367_port_isolation_mapping_table[i].extPort, 0)) != RT_ERR_OK)
		{
			RTL8367_ERROR("rtksw_l2_limitLearningCnt_set on port %d failed\n", rtl8367_port_isolation_mapping_table[i].extPort);
			return retVal;
		}
	}
#endif

	/* Flush L2 table */
	if ((retVal = rtksw_l2_table_clear()) != RT_ERR_OK)
		return retVal;

	return RT_ERR_OK;
}

rtksw_api_ret_t rtk_rtl8367_port_isolation_mapping_init(void)
{
	memset(&rtl8367_port_isolation_mapping_table, 0x00, sizeof(struct rtl8367_port_isolation_mapping_table_entry)*RTL8367_MAX_LAN_PORT_NUM);
	return RT_ERR_OK;
}

void rtk_rtl8367_proc_exit(void)
{
	int i = 0;

	for(i = 0 ; i < (sizeof(rtl8367Proc) / sizeof(rtk_rtl8367_proc_t)) ; i++)
	{
		struct proc_dir_entry *parentDir = NULL;

		switch(rtl8367Proc[i].dir)
		{
			case PROC_DIR_RTL8367_ROOT:
				parentDir = rtl8367_proc_dir;
				break;
			default:
				break;
		}

		remove_proc_entry(rtl8367Proc[i].name, parentDir);
	}

	proc_remove(rtl8367_proc_dir);
}

static int rtk_rtl8367_notify_sys(struct notifier_block *this, unsigned long code, void *unused)
{
	rtksw_portmask_t portMask;
	
	memset(&portMask, 0, sizeof(rtksw_portmask_t));
	if (code == SYS_DOWN || code == SYS_HALT){
		/* reinit switch*/
		//printk("%s:%d\n", __FUNCTION__, __LINE__);
		rtksw_switch_init();
		/* clear port led*/
		rtksw_led_groupConfig_set(LED_GROUP_1, LED_CONFIG_LINK_ACT);
		rtksw_led_enable_set(LED_GROUP_1, &portMask);
	}
	
	return NOTIFY_DONE;
}

static struct notifier_block reboot_notifier = {
	.notifier_call  = rtk_rtl8367_notify_sys,
};

static int __init rtk_rtl8367_moudle_init(void)
{
	rtksw_uint32 rData;
	rtksw_api_ret_t ret;
	rtksw_port_mac_ability_t ability;
	rtksw_api_ret_t retVal;
	int i=0;
	int result=0;

	RTL8367_INFO("%s", __FUNCTION__);
#ifdef CONFIG_DAL_RTL8367C
	RTL8367_MSG("Support RTL8367C");
#endif
#ifdef CONFIG_DAL_RTL8367D
	RTL8367_MSG("Support RTL8367D");
#endif
#if defined(MDC_MDIO_OPERATION)
	RTL8367_MSG("Operated by MDC/MDIO");
#endif
	
#if defined(CONFIG_RTL9607C_SERIES)
	RTL8367_MSG("disable gpio %d/%d",RTL9607C_SET0_MDC_PIN,RTL9607C_SET0_MDIO_PIN);

	//the gpio must disable due share pin with mdc/mdio control
	result = rtk_gpio_state_set(RTL9607C_SET0_MDC_PIN, DISABLED);
	if(result != RT_ERR_OK)
		RTL8367_ERROR("rtk_gpio_state_set RTL9607C_SET0_MDC_PIN fail!\n");
	result = rtk_gpio_state_set(RTL9607C_SET0_MDIO_PIN, DISABLED);
	if(result != RT_ERR_OK)
		RTL8367_ERROR("rtk_gpio_state_set RTL9607C_SET0_MDIO_PIN fail!\n");
	RTL8367_MSG("rt_mdio_init");
	result = rt_mdio_init();
	if(result != RT_ERR_OK)
		RTL8367_ERROR("rt_mdio_init fail!\n");
	RTL8367_MSG("rt_mdio_cfg_set mdio port:%d phy-id:%d",DEFAULT_MDIO_PORT_NUM,MDC_MDIO_PHY_ID);
	result = rt_mdio_cfg_set(0, DEFAULT_MDIO_PORT_NUM, MDC_MDIO_PHY_ID, RTK_MDIO_FMT_C22);
	if(result != RT_ERR_OK)
		RTL8367_ERROR("rt_mdio_cfg_set fail!\n");
#endif
	
	rtl8367c_smi_read(0x1202, &rData);
	RTL8367_MSG("Test reading 0x1202 (should be 0x88a8) = 0x%0x", rData);
	if(rData==0x88a8){
		RTL8367_MSG("RTL8367C/D reset okay");
	}

#ifdef CONFIG_EXTERNAL_SWITCH
    rtl8367_mapper_init();
#endif

	/* switch init */
	ret = rtksw_switch_init();
	if (ret != RT_ERR_OK) {
		if (ret == RT_ERR_FAILED)
			RTL8367_ERROR("rtksw_switch_init - Failed (%d)", ret);
		else if (ret == RT_ERR_SMI)
			RTL8367_ERROR("rtksw_switch_init - SMI access error (%d)", ret);
		else
			RTL8367_ERROR("rtksw_switch_init - Unknown error (%d)", ret);
	}

#if 1
	RTL8367_INFO("Disable CPU port");
	rtksw_cpu_insert_t insert_mode;
	insert_mode = CPU_INSERT_TO_NONE;
	ret = rtksw_cpu_enable_set(RTKSW_DISABLED);
	if (ret != RT_ERR_OK)
		RTL8367_ERROR("rtksw_cpu_enable_set - Error (%d)", ret);
	ret = rtksw_cpu_tagPort_set(EXT_PORT0, insert_mode);
	if (ret != RT_ERR_OK)
		RTL8367_ERROR("rtksw_cpu_tagPort_set - Error (%d)", ret);
	ret = rtksw_cpu_tagPort_set(EXT_PORT1, insert_mode);
	if (ret != RT_ERR_OK)
		RTL8367_ERROR("rtksw_cpu_tagPort_set - Error (%d)", ret);
#endif

#if 1
	RTL8367_INFO("Setting Serdes");
	/* Serdes Nway è¨­å?, ?™å€‹è¨­å®šå??ˆè???CPU ??Serdes è¨­å?ä¸€?? */
	ret = rtksw_port_sgmiiNway_set(EXT_PORT0, RTKSW_DISABLED);
	if (ret != RT_ERR_OK)
		RTL8367_ERROR("rtksw_port_sgmiiNway_set - EXT_PORT0 Error (%d)", ret);

	ret = rtksw_port_sgmiiNway_set(EXT_PORT1, RTKSW_DISABLED);
	if (ret != RT_ERR_OK)
		RTL8367_ERROR("rtksw_port_sgmiiNway_set - EXT_PORT1 Error (%d)", ret);
	/* HSGMII 2.5G setting */
	memset(&ability, 0x00, sizeof(rtksw_port_mac_ability_t));
	ability.forcemode = 1;
	ability.speed = RTKSW_PORT_SPEED_2500M;
	//ability.speed = PORT_SPEED_1000M;
	ability.duplex = RTKSW_PORT_FULL_DUPLEX;
	ability.link = RTKSW_PORT_LINKUP;
	ability.nway = RTKSW_DISABLED;
	ability.txpause = RTKSW_DISABLED;
	ability.rxpause = RTKSW_DISABLED;
	ret = rtksw_port_macForceLinkExt_set(EXT_PORT0, MODE_EXT_HSGMII, &ability);
	if (ret != RT_ERR_OK)
		RTL8367_ERROR("rtksw_port_macForceLinkExt_set - EXT_PORT0 Error (%d)", ret);

	ret = rtksw_port_macForceLinkExt_set(EXT_PORT1, MODE_EXT_HSGMII, &ability);
	if (ret != RT_ERR_OK)
		RTL8367_ERROR("rtksw_port_macForceLinkExt_set - EXT_PORT1 Error (%d)", ret);
#endif
		
	/* L2 initialization */	
	ret = rtksw_l2_init();
	if (ret != RT_ERR_OK)
		RTL8367_ERROR("rtksw_l2_init - Error (%d)", ret);
#if 0
	ret = rtk_rtl8367_test_init();
	if (ret != RT_ERR_OK) {
		RTL8367_ERROR("rtk_rtl8367_test_init - Failed (%d)", ret);
	}
#endif

	/* Init proc device */
	rtk_rtl8367_proc_init();

	/* Init ioctl device */
	rtk_rtl8367_ioctl_init();

#ifdef CONFIG_RTL_MULTI_LAN_DEV
	rtk_rtl8367_dev_port_mapping_init();
	rtk_rtl8367_port_scan_start(); /* start the port scan thread */
	rtk_rtl8367_port_isolation_mapping_init();
#endif

    /* Port Isolation */
    rtk_rtl8367_port_isolation_init();
	
	/* led setup*/
	rtksw_led_groupConfig_set(LED_GROUP_1, LED_CONFIG_LINK_ACT);
	
	led_rtk_hw_rtl8367_led_register();
	
	result = register_reboot_notifier(&reboot_notifier);
	if (result) {
		pr_err("cannot register reboot notifier (err=%d)\n", result);
		return result;
	}

	return 0;
}

static void __exit rtk_rtl8367_module_exit(void)
{
	unsigned int port_num;
	int i = 0;
	struct net_device *dev = NULL;
	
#ifdef CONFIG_EXTERNAL_SWITCH
	rtl8367_mapper_uninit();
#endif
	
#ifdef CONFIG_RTL_MULTI_LAN_DEV
	rtk_rtl8367_port_scan_stop(); /* start the port scan thread */
#endif

	unregister_reboot_notifier(&reboot_notifier);
	led_rtk_hw_rtl8367_led_unregister();
	rtk_rtl8367_proc_exit();
	
#ifdef CONFIG_RTL_MULTI_LAN_DEV
	rtk_rtl8367_dev_port_mapping_init();
#endif
	RTL8367_INFO("%s", __FUNCTION__);
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("RTL8367 Module");
MODULE_AUTHOR("Realtek.com");

module_init(rtk_rtl8367_moudle_init);
module_exit(rtk_rtl8367_module_exit);

