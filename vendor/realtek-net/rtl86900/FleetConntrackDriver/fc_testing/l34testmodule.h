#ifndef __L34TESTMODULE__
#define __L34TESTMODULE__

#include <linux/fs.h>
#include <linux/list.h>
#include <linux/version.h>

#include <rtk_fc_mgr.h>
#include <rtk_fc_helper_wlan.h>
#include <rtk_fc_assistant.h>
#include <rtk_fc_mgrTRx.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
#include <linux/proc_fs.h>
#endif

#define TEST_MODULE_NAME "fc_testing"

/* PROC GROUP */
#define RTK_FCTESTING_MAX_PROC_GROUP 1
#define PROCFS_MAX_SIZE 1024


typedef struct rtk_fcTesting_proc_s
{
	char *name;
	int (*get) (struct seq_file *s, void *v);
	int (*set) ( struct file *, const char *,unsigned long, void *);
	unsigned int inode_id[RTK_FCTESTING_MAX_PROC_GROUP];
	unsigned int unlockBefortWrite;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
	struct proc_ops proc_fops;
#else
	struct file_operations proc_fops;
#endif

}rtk_fcTesting_proc_t;


typedef enum rtk_egr_to_e{
	/* egr_To */
	PHY_RTK_FC_SKB_TX = 0, 				/* rtk_fc_skb_tx */
	WLAN_RTK_FC_FASTFWD_DEV_XMIT ,		/* rtk_fc_fastfwd_dev_xmit */
	NETIF_RX_PROTOCOL_STACK ,					/* continue send to PS slowpath*/
	RTK_EGR_MAX,					//below RTK_EGR_MAX is not support yet
}rtk_egr_to_t;

typedef enum rtk_igr_from_e{
	/* igr_from */
	RTK_FC_SKB_PHY_RX = 0,				/* rtk_fc_skb_rx */
	RTK_IGR_MAX,						//below RTK_IGR_MAX is not support yet
	RTK_FC_SKB_WIFI_RX ,				/* rtk_fc_skb_wifi_rx */
	RTK_FC_FASTFWD_NETIF_RX ,			/* rtk_fc_fastfwd_netif_rx */
	RTK_FC_FASTFWD_NAPI_GRO_RECEIVE,	/* rtk_fc_fastfwd_napi_gro_receive */
}rtk_igr_from_t;

typedef enum rtk_fcTesting_status_e{
	TESTING_INIT=0,					
	TESTING_STEP1_TX,		/* prepare packet and direct Tx by hdr-a*/
	TESTING_STEP2_NETIF_RX_TO_FC,	/* receive by netif_rx and prepare egress packet into fc_egress_learning*/
	TESTING_STEP3,
}rtk_fcTesting_status_t;


typedef struct rtk_fcTesting_IgrInfo_s
{
	rtk_igr_from_t igr_from;
	int igrDataLen;
	unsigned char *igrData;
	unsigned char igrPort;
	unsigned char streamid_en;
	unsigned char streamid;
	unsigned short flowid;
	unsigned short pol_id;
}rtk_fcTesting_IgrInfo_t;

typedef struct rtk_fcTesting_egrInfo_s
{
	rtk_egr_to_t egr_to;
	int egrDataLen;
	unsigned char *egrData;

	struct net_device *egrSkbEgrDev;
	unsigned char egrIsWlanPort;
	unsigned char egrPort;		// phy port or rtk_fc_wlan_devidx_t by isWlanPort

	//stacking device in netfilter
	struct net_device *igrLogicalDev[4];
	struct net_device *egrLogicalDev[4];

	/* skb mark */
	uint32 skbMark;
	uint64 skbMark2;

	
}rtk_fcTesting_egrInfo_t;



typedef struct rtk_fcTesting_case_s
{
	rtk_fcTesting_status_t testStatus;
	rtk_fcTesting_IgrInfo_t 	igrInfo;
	rtk_fcTesting_egrInfo_t 	egrInfo;

	uint32 valid:1;
	uint32 dischk_egr_pkt;
	uint32 dischk_slowPath_pkt:1;
	uint32 dischk_shortcut_pkt:1;
	uint32 dischk_earlyChkShortCut_pkt:1;
	uint32 dischk_hardware_pkt:1;
	
	//igrFC_check function!?
	//egr_FC_chekc function!?
	//egrSlowPathSkbModify function;
}rtk_fcTesting_case_t;


int rtk_test_user_pipe_cmd(const char *comment, ...);

int single_test_dump_case_table(struct seq_file *s, void *data);
int single_test(struct file *file, const char __user *buffer, unsigned long count, void *data);

int rtk_fcTesting(rtk_fcTesting_case_t *info);
int rtk_fcTesting_netif_rx(struct sk_buff *skb);

#endif
