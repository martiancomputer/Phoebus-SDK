
#include <net/net_namespace.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/kmod.h>

#include <common/type.h>
#include <uapi/linux/types.h>
#include <linux/byteorder/generic.h>
#include <linux/byteorder/little_endian.h>
#include <uapi/linux/byteorder/little_endian.h>
//#include <ioal/mem32.h>		/*xpon define*/
#include <uapi/linux/random.h>

#include <rtl_glue.h>
#include <pktGen.h>
#include <rtl_glue.h>
#include <test_define.h>
#include "l34testmodule.h"

#include <rtk_fc_api.h>


#define GRP_ALL             0x0fffffff
#define GRP_SKIP_INIT		0x99900100

extern char procfs_buffer[PROCFS_MAX_SIZE];
#define INFO_PAIR_SIZE 8
rtk_fcTesting_case_t fcTestInfo;
#define BUF_SIZE 1600
unsigned char igrData[BUF_SIZE];
unsigned char egrData[BUF_SIZE];
unsigned char igrPayload[BUF_SIZE];
unsigned char egrPayload[BUF_SIZE];
HEADER_A_T ni_header_a;


typedef struct MODEL_TESTCASE_REGIST_S
{	
	int caseNum;
	char * caseName;
	int32 (*fp)(rtk_fcTesting_case_t *);
	uint32 group;
}MODEL_TESTCASE_REGIST_T;

#define MODEL_TESTCASE_REGIST(num,func,groupmask) \
	{ \
		caseNum: num, \
		caseName: #func, \
		fp: func, \
		group: groupmask,\
	}


enum
{
	PARSE_DUMP_FAILED = -4,
	PARSE_TESTCASE_FAILED = -3,
	PARSE_FAILED= -2,
	PARSE_EXIT = -1,
	PARSE_SUCCESS=0,
};


//fucntion define
int call_testOneCase(int num);
int call_testMultipleCase(int numStart, int numEnd);
int call_testGroupCase(int group);
int rtk_send_withHdra_by_info(rtk_fcTesting_case_t *info);



MODEL_TESTCASE_REGIST_T  fcTestCaseRegist[]=
{
	MODEL_TESTCASE_REGIST(1, dslite_upstream, 				GRP_ALL),
	MODEL_TESTCASE_REGIST(2, dslite_downstream_dualInLan, 			GRP_ALL),
	MODEL_TESTCASE_REGIST(3, dslite_upstream_dualInLan, 			GRP_ALL),	
	MODEL_TESTCASE_REGIST(4, gre_downstream_dualInLan, 			GRP_ALL),	

#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
	MODEL_TESTCASE_REGIST(10, wfo_callback_igmp_testing, 			GRP_ALL),	
	MODEL_TESTCASE_REGIST(11, wfo_callback_mcdata_testing, 			GRP_ALL),
#endif

};

static int bufInit(void)
{
	memset(&fcTestInfo,0,sizeof(fcTestInfo));
	memset(&igrData[0],0,sizeof(igrData));
	memset(&egrData[0],0,sizeof(egrData));
	memset(&igrPayload[0],0,sizeof(igrPayload));
	memset(&egrPayload[0],0,sizeof(egrPayload));
	fcTestInfo.egrInfo.egrData=&egrData[0];
	fcTestInfo.igrInfo.igrData=&igrData[0];
	return 0;
}


static int configEnvInit(void)
{

	return 0;
}

int dump_testCase(void)
{
	
	u32 i = 0, caseNum = sizeof(fcTestCaseRegist)/sizeof(MODEL_TESTCASE_REGIST_T);
	int strLen = 0;
	rtlglue_printf("\r\n  <<FC Test Case List>>  Total Case Num: %d\r\n\n", caseNum);

	for(i = 0; i < caseNum; i++)
	{
		rtlglue_printf(KERN_CONT "\t- case %d\t%s", fcTestCaseRegist[i].caseNum, fcTestCaseRegist[i].caseName);
		strLen = 50 - strlen(fcTestCaseRegist[i].caseName);
		if(strLen<0) strLen = 0;
		do{
			rtlglue_printf(KERN_CONT " ");
			strLen--;
		}while(strLen>0);
		rtlglue_printf(KERN_CONT "mask:  0x%x\r\n", fcTestCaseRegist[i].group);
	}

	return 0;
}



int ConvStrToInt( char **s)
{
	int i = 0, k = 0, isHex = 0;

	if ( (*s[0] == '0') && (*s[1] == 'x') )
		isHex=1;

	for (k=0; **s ; (*s)++,k++ )
	{
		if (isHex)
		{
			if(k<2) continue;
			i = (i<<4) + (**s-'0');
		} 
		else
		{
			if((**s < '0') || (**s > '9')) break;
			i = (i*10) + (**s-'0');
		}
	}
		
	return i;
}

int single_test_dump_case_table(struct seq_file *s, void *data)
{
	dump_testCase();

	return 0;
}

int single_test(struct file *file, const char __user *buffer, unsigned long count, void *data)
{
	/* get buffer size */
	int procfs_buffer_size = count;
	int caseNo = 0;
	int caseNo2 = 0;


	if (procfs_buffer_size > PROCFS_MAX_SIZE ) {
		procfs_buffer_size = PROCFS_MAX_SIZE;
	}

	//memset(procfs_buffer, 0, PROCFS_MAX_SIZE);
	
	/* write data to the buffer */
	//if ( copy_from_user(procfs_buffer, buffer, procfs_buffer_size) ) {
	//	return -EFAULT;
	//}
	//procfs_buffer[procfs_buffer_size] = '\0';

	if(procfs_buffer[0]=='a' && procfs_buffer[1]=='l' && procfs_buffer[2]=='l')
		call_testGroupCase(GRP_ALL);
	else
	{
		char *t = &procfs_buffer[0];
		char *split_str;
		//caseNo=ConvStrToInt(&procfs_buffer);

		if(t!=NULL){
			split_str=strsep(&t," ");		
			caseNo=ConvStrToInt(&split_str);
		}
		if(t!=NULL){
			split_str=strsep(&t," ");
			caseNo2=ConvStrToInt(&split_str);
		}
		if(caseNo2 > caseNo)
			call_testMultipleCase(caseNo, caseNo2);
		else
			call_testOneCase(caseNo);
	}

	return procfs_buffer_size;
}


int rtk_test_user_pipe_cmd(const char *comment, ...)
{
	char cmd_buf[256];

	char * envp[]={
		"HOME=/",
		NULL
	};
	char * argv[]={
		"/bin/ash",
		"-c",
		cmd_buf,
		NULL
	};
	int retval;
	va_list argList;
	va_start(argList, comment);
	//sprintf( cmd_buff, comment, ##arg);
	vsprintf(cmd_buf, comment, argList);

	//TRACE("Exec: %s", cmd_buf);

	retval=call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);

	va_end(argList);
	return retval;
}


int call_testOneCase(int num){
	int i;
	int ret=PARSE_TESTCASE_FAILED;
	
	for(i=0;i<sizeof(fcTestCaseRegist)/sizeof(fcTestCaseRegist[0]);i++)
	{
		if(fcTestCaseRegist[i].caseNum==num)
		{
			rtlglue_printf("\n\n");
			//TRACE("*****************************************************************");
			//TRACE("* Run Test Case #%d: %s\t\t*", fcTestCaseRegist[i].caseNum, fcTestCaseRegist[i].caseName);
			//TRACE("*****************************************************************");
			bufInit();

			ret = fcTestCaseRegist[i].fp(&fcTestInfo);
			ret = rtk_fcTesting(&fcTestInfo);
			
			if(ret!=SUCCESS){
				rtlglue_printf(COLOR_H "RUN TESTCASE[%d] FAILED!"COLOR_NM"\n",num);
				return ret;
			}else{
				ret=PARSE_SUCCESS;
				break;
			}
		}
	}

	if(ret==PARSE_TESTCASE_FAILED){
		rtlglue_printf(COLOR_H "TESTCASE[%d] NOT FOUND!"COLOR_NM"\n",num);
	}
	return ret;
}

int call_testOneCase_withoutInit(int num){
	int i;
	int ret=PARSE_TESTCASE_FAILED;
	
	for(i=0;i<sizeof(fcTestCaseRegist)/sizeof(fcTestCaseRegist[0]);i++)
	{
		if(fcTestCaseRegist[i].caseNum==num)
		{
			rtlglue_printf("\n\n");
			//TRACE("*****************************************************************");
			//TRACE("* Run UnInit Test Case #%d: %s\t\t*", fcTestCaseRegist[i].caseNum, fcTestCaseRegist[i].caseName);
			//TRACE("*****************************************************************");
							
			ret = fcTestCaseRegist[i].fp(&fcTestInfo);
			ret = rtk_fcTesting(&fcTestInfo);

			if(ret!=SUCCESS){
				rtlglue_printf(COLOR_H "RUN TESTCASE[%d] FAILED!"COLOR_NM"\n",num);
				return ret;
			}else{
				ret=PARSE_SUCCESS;
				break;
			}
		}
	}

	if(ret==PARSE_TESTCASE_FAILED){
		rtlglue_printf(COLOR_H "TESTCASE[%d] NOT FOUND!"COLOR_NM"\n",num);
	}
	return ret;
}

int call_testMultipleCase(int numStart, int numEnd){
	int i;
	int ret=PARSE_TESTCASE_FAILED;
	int grpRet = SUCCESS;
	int allCaseNum = (sizeof(fcTestCaseRegist))/(sizeof(MODEL_TESTCASE_REGIST_T));
	int passCaseNum = 0, failCaseNum = 0;
	int failCaseReport[(sizeof(fcTestCaseRegist))/(sizeof(MODEL_TESTCASE_REGIST_T))];

	unsigned long int startTime,elapsedTime;
	startTime = jiffies;


	//WARNING("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
	//WARNING("* Run Test Cases from #%d to #%d\t\t", numStart, numEnd);
	//WARNING("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
	
	for(i=0;i<allCaseNum;i++)
	{
		if((fcTestCaseRegist[i].caseNum>=numStart) && (fcTestCaseRegist[i].caseNum<=numEnd))
		{
			rtlglue_printf("\n\n");
			//TRACE("*****************************************************************");
			//TRACE("* Run Test Case #%d: %s\t\t*", fcTestCaseRegist[i].caseNum, fcTestCaseRegist[i].caseName);
			//TRACE("*****************************************************************");

			configEnvInit();
				
			ret = fcTestCaseRegist[i].fp(&fcTestInfo);
			ret = rtk_fcTesting(&fcTestInfo);

			
			if(ret!=SUCCESS){
				rtlglue_printf(COLOR_H "[M]RUN TESTCASE[%d] FAILED!"COLOR_NM"\n", fcTestCaseRegist[i].caseNum);
				failCaseReport[failCaseNum] = i;
				failCaseNum++;
				grpRet = ret;
			}else{
				passCaseNum++;
			}

			//Report
			{
				int j,k;
				rtlglue_printf("\r\n  <<TEST REPORT>>  Total Case: %d,  Pass: %d,  Fail: %d\r\n\n", passCaseNum+failCaseNum, passCaseNum, failCaseNum);
				
				for(k=0; k<=i; k++)
				{
					if(fcTestCaseRegist[k].caseNum < numStart)
						continue;
					if(fcTestCaseRegist[k].caseNum > numEnd)
						break;
										
					for(j=0;j<failCaseNum;j++) 	// belongs to fail case?
					{
						if(fcTestCaseRegist[failCaseReport[j]].caseNum == fcTestCaseRegist[k].caseNum)
						{
							rtlglue_printf("\033[1;31m%-10s\033[m", "[FAIL]");
							break;
						}
					}
					if(j==failCaseNum)			// didn't match fail case list, so it is pass
						rtlglue_printf("\033[1;37;42m%-10s\033[m",  "[PASS]");
					
					rtlglue_printf(KERN_CONT "- case %-6d  %s\r\n", fcTestCaseRegist[k].caseNum, fcTestCaseRegist[k].caseName);
				}
			}
		}
	}
	elapsedTime = jiffies - startTime;
	rtlglue_printf("\n\n ** Total elasped %ld hours %ld mins %ld secs. **\n\n",(elapsedTime/HZ)/3600,((elapsedTime/HZ)/60)%60,(elapsedTime/HZ)%60);


	return grpRet;
}

int call_testGroupCase(int group){
	int i;
	int ret=PARSE_TESTCASE_FAILED;
	int grpRet = SUCCESS;
	int allCaseNum = (sizeof(fcTestCaseRegist))/(sizeof(MODEL_TESTCASE_REGIST_T));
	int passCaseNum = 0, failCaseNum = 0, runningCaseNum = 0;
	int failCaseReport[ (sizeof(fcTestCaseRegist))/(sizeof(MODEL_TESTCASE_REGIST_T))];
	int runningCaseReport[ (sizeof(fcTestCaseRegist))/(sizeof(MODEL_TESTCASE_REGIST_T))];

	unsigned long int startTime,elapsedTime;
	startTime = jiffies;


//printk("#### group : 0x%08x\n",group);	
	for(i=0;i<allCaseNum;i++)
	{
//printk(">>> (fcTestCaseRegist[i].group:0x%08x & group):0x%08x\n",fcTestCaseRegist[i].group,(fcTestCaseRegist[i].group & group));
		if(((fcTestCaseRegist[i].group & group) == group) || (GRP_ALL == group))
		{
			rtlglue_printf("\n\n");
			//TRACE("*****************************************************************");
			//TRACE("* Run Test Case #%d: %s\t\t*", fcTestCaseRegist[i].caseNum, fcTestCaseRegist[i].caseName);
			//TRACE("*****************************************************************");

			runningCaseReport[runningCaseNum] = i;
			runningCaseNum++;

			configEnvInit();

			ret = fcTestCaseRegist[i].fp(&fcTestInfo);
			ret = rtk_fcTesting(&fcTestInfo);

			if(ret!=SUCCESS){
				rtlglue_printf(COLOR_H "RUN TESTCASE[%d] FAILED!"COLOR_NM"\n", fcTestCaseRegist[i].caseNum);
				failCaseReport[failCaseNum] = i;
				failCaseNum++;
				grpRet = ret;
			}else{
					passCaseNum++;
			}

			//Report
			{
				int j,k;
				rtlglue_printf("\r\n  <<TEST REPORT>>  Total Case: %d,  Pass: %d,  Fail: %d\r\n\n", passCaseNum+failCaseNum, passCaseNum, failCaseNum);
		
				for(k=0; k<=i; k++)
				{
					int jj = 0;
					for(jj=0;jj<runningCaseNum;jj++)
					if(fcTestCaseRegist[runningCaseReport[jj]].caseNum == fcTestCaseRegist[k].caseNum)
					{
						for(j=0;j<failCaseNum;j++)
						{
							if(fcTestCaseRegist[failCaseReport[j]].caseNum == fcTestCaseRegist[k].caseNum)
							{
								rtlglue_printf("\033[1;31m [FAILED] \033[m");
								break;
							}
						}
						if(j==failCaseNum)
							rtlglue_printf("\033[1;37;42m [PASS] \033[m");
						rtlglue_printf(KERN_CONT"\t- case %d\t%s\r\n", fcTestCaseRegist[k].caseNum, fcTestCaseRegist[k].caseName);
					}
				}
			}
		}
	}

	if(ret==PARSE_TESTCASE_FAILED){
		rtlglue_printf(COLOR_H "TESTCASE with group mask 0x%x NOT FOUND!"COLOR_NM"\n", group);
	}

	/* Show Test Report */
	rtlglue_printf("\r\n  <<TEST REPORT>>  Total Case: %d,  Pass: %d,  Fail: %d\r\n\n", passCaseNum+failCaseNum, passCaseNum, failCaseNum);
	
	for(i=0; i<failCaseNum; i++){
		rtlglue_printf(KERN_CONT"\t- case %d\t%s\r\n", fcTestCaseRegist[failCaseReport[i]].caseNum, fcTestCaseRegist[failCaseReport[i]].caseName);
	}
	
	elapsedTime = jiffies - startTime;
	rtlglue_printf("\n\n ** Total elasped %ld hours %ld mins %ld secs. **\n\n",(elapsedTime/HZ)/3600,((elapsedTime/HZ)/60)%60,(elapsedTime/HZ)%60);


	return grpRet;
}



int getRandom(uint32 modulo)
{
	uint32 randNum = 0;
	get_random_bytes(&randNum, sizeof(randNum));

	if(modulo==0)
		return randNum;
	else
		return (randNum & (modulo-1));            
}

uint32 getRandom_range(uint32 range_start, uint32 range_end)
{
	uint32 randNum = 0;
	uint32 modulo = range_end - range_start + 1;

	get_random_bytes(&randNum, sizeof(randNum));

	return range_start + (randNum % modulo);       
}

/* this is main testing fucntion*/
#define TEST_SKB_BUF_SIZE 1800

#define PORT_MAX 10
char* testPort_dev_mapping[PORT_MAX]=
{
	"eth0.2",/* portID 0*/
	"eth0.3",/* portID 1*/
	"eth0.4",/* portID 2*/
	"eth0.5",/* portID 3*/
	"eth0.7",/* portID 4*/
	"eth0.8",/* portID 5*/	
	"eth0.6",/* portID 6*/
	"nas0",	 /* portID 7*/
	"wlan0",
	"wlan1",
};

//call by rtk_fc_skb_netif_rx
int rtk_fcTesting_netif_rx(struct sk_buff *skb)
{

	int ret;

	if(fcTestInfo.testStatus)
		TEST_DEBUG("into testStatus:%d",fcTestInfo.testStatus);
	
	switch(fcTestInfo.testStatus)
	{
		case TESTING_INIT:
			return RTK_FC_NIC_RX_CONTINUE;
			break;
		case TESTING_STEP1_TX:
		{
			if(fcTestInfo.egrInfo.egr_to==NETIF_RX_PROTOCOL_STACK)
			{	
				fcTestInfo.testStatus = TESTING_INIT;
				return RTK_FC_NIC_RX_CONTINUE;
			}
			else
			{
				//handle skb data/len/tail
				if((fcTestInfo.egrInfo.egrDataLen - fcTestInfo.igrInfo.igrDataLen)>0)
				{
					skb_put(skb,(fcTestInfo.egrInfo.egrDataLen - fcTestInfo.igrInfo.igrDataLen));
				}
				else
				{
					skb_pull(skb,(fcTestInfo.igrInfo.igrDataLen-fcTestInfo.egrInfo.egrDataLen ));
				}
				memcpy(skb->data,fcTestInfo.egrInfo.egrData,fcTestInfo.egrInfo.egrDataLen);	


				//config skb->dev
				if(fcTestInfo.egrInfo.egr_to==WLAN_RTK_FC_FASTFWD_DEV_XMIT)
				{
					//put wlan0-vap0/wlan0-vap1 ...
					if(fcTestInfo.egrInfo.egrSkbEgrDev)
					{
						skb->dev = fcTestInfo.egrInfo.egrSkbEgrDev;
					}
					else
					{
						TEST_FIXME("FIXME");
					}
				}
				else if (fcTestInfo.egrInfo.egr_to==PHY_RTK_FC_SKB_TX)
				{
					if(fcTestInfo.egrInfo.egrSkbEgrDev)
					{
						skb->dev = fcTestInfo.egrInfo.egrSkbEgrDev;
					}
					else
					{
						//phy port
						TEST_DEBUG("egr port:%d  dev:%s",fcTestInfo.egrInfo.egrPort,testPort_dev_mapping[fcTestInfo.egrInfo.egrPort]);
						skb->dev = __dev_get_by_name(&init_net,testPort_dev_mapping[fcTestInfo.egrInfo.egrPort]); /*I don't care but ni will check dev*/
					}
				}


				/*
					below field should check
				*/
				//skb -> eth/l3hdr/l4hdr
				//skb -> from_dev
				//skb -> priority

				//fc data cache ingress/egress logical 
				rtk_fc_decision_ingress_interface(skb,fcTestInfo.egrInfo.igrLogicalDev[0],fcTestInfo.egrInfo.igrLogicalDev[1],fcTestInfo.egrInfo.igrLogicalDev[2]);
				rtk_fc_decision_egress_interface(skb,fcTestInfo.egrInfo.egrLogicalDev[0],fcTestInfo.egrInfo.egrLogicalDev[1],fcTestInfo.egrInfo.egrLogicalDev[2]);

				
				fcTestInfo.testStatus = TESTING_STEP2_NETIF_RX_TO_FC;

				if(fcTestInfo.dischk_slowPath_pkt)
				{

				}

				//call fc egress learning
				if(fcTestInfo.egrInfo.egr_to==WLAN_RTK_FC_FASTFWD_DEV_XMIT)
				{
					ret = rtk_fc_fastfwd_dev_xmit(skb,skb->dev);
				}
				else if (fcTestInfo.egrInfo.egr_to==PHY_RTK_FC_SKB_TX)
				{
					ret =  rtk_fc_skb_tx(skb,skb->dev);
				}	
				break;
			}
		}

		case TESTING_STEP2_NETIF_RX_TO_FC:
		{
			break;
		}

		default:
		{
			TEST_WARNING("status:%d",fcTestInfo.testStatus);
			dev_kfree_skb(skb);
			break;
		}
	}


	return RTK_FC_NIC_RX_CONTINUE;
}


int rtk_send_withHdra_by_info(rtk_fcTesting_case_t *info)
{
	struct sk_buff *igr_skb;
	
	//checking 
	if(info==NULL || 
		info->igrInfo.igr_from>=RTK_IGR_MAX || info->igrInfo.igrData==NULL || 
		info->egrInfo.egr_to>=RTK_EGR_MAX ||info->egrInfo.egrData==NULL )
	{	
		rtlglue_printf("Error parameter\n");
		return FAIL;
	}


	/*
		Fc ingress
	*/
	igr_skb=dev_alloc_skb(TEST_SKB_BUF_SIZE);
	memcpy(igr_skb->data,info->igrInfo.igrData,info->igrInfo.igrDataLen);
	skb_put(igr_skb,info->igrInfo.igrDataLen);
	igr_skb->dev = __dev_get_by_name(&init_net,"nas0"); /*I don't care but ni will check dev*/


	if(info->igrInfo.igr_from == RTK_FC_SKB_PHY_RX)
	{
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
		rtlglue_printf("not support yet\n");
#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
		ca_ni_tx_config_t tx_config;
		bzero(&tx_config,sizeof(tx_config));
		tx_config.flow_id = info->igrInfo.flowid;
		tx_config.core_config.bf.is_from_ca_tx=1;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		tx_config.stream_id_en = info->igrInfo.streamid_en;
		tx_config.stream_id = info->igrInfo.streamid;
#endif
		tx_config.force_def_tx_en=1;
		tx_config.p_ni_header_a = &ni_header_a;
		memset(&ni_header_a,0,sizeof(ni_header_a));
		ni_header_a.bits.lspid=info->igrInfo.igrPort;
		ni_header_a.bits.ldpid=RTK_FC_MAC_PORT_L3_LAN;
		ni_header_a.bits.fe_bypass=1;
		ni_header_a.bits.bits_32_63.pkt_info.pol_id = info->igrInfo.pol_id;
		nic_egress_start_xmit_for_fc_dirTx(igr_skb,NULL,&tx_config);
#endif
	}

	return SUCCESS;
}

int rtk_fcTesting(rtk_fcTesting_case_t *info)
{
	rtk_send_withHdra_by_info(info);

	info->testStatus = TESTING_STEP1_TX;

	//I think need a delay to avoid race-condition
	mdelay(500);


	return SUCCESS;
}


