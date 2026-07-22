#include <linux/igmp.h>
#include <net/mld.h>
//#include <uapi/linux/igmp.h>
//#include <linux/if_bridge.h>
#include <br_private.h>
#include <linux/proc_fs.h>
#include <fs/proc/internal.h>


#include <rtk_igmp_snooping.h>
#include <rtk_igmp_hook.h>
#include <rtk_igmp_debug.h>
#include <rtk_igmp_struct.h>
#include <rt_igmp_ext.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>


#if defined(CONFIG_RTL_LAN_COMPATIBILITY) || defined(CONFIG_RTL_NEW_IGMP_REPORT_BEHAVIOR)
#include <rtk_igmp_nec_snooping.h> 
#endif

#if defined(CONFIG_RTL_NEW_IGMP_REPORT_BEHAVIOR)|| defined(CONFIG_RTL_LAN_COMPATIBILITY) || defined (CONFIG_RTL_PER_PORT_CLIENT_NUM)
int igmpVersion=2;
int mldVersion=2;
struct rtl_mCastSnoopingGlobalConfig mCastSnoopingGlobalConfig;
extern uint32 rtk_igmp_startTime;
extern uint32 rtk_igmp_sysUpSeconds;
#define IGMP_HASH_TBL_SIZE 	(1<<IGMP_HASH_TBL_BIT)
#define IGMP_HASH_TBL_BIT 	5


extern struct rtk_igmp_multicastModule rtk_igmp_mCastModuleArray[IGMP_MAX_BR_MODULE_NUM];
extern rtk_igmp_multicastDeviceInfo_t  rtk_igmp_wanDevInfo[IGMP_MAX_WAN_DEV_NUM];
extern rtk_igmp_multicastDeviceInfo_t  rtk_igmp_lanDevInfo[IGMP_MAX_LAN_DEV_NUM];


#endif

#if defined(CONFIG_RTL_LAN_COMPATIBILITY)
int igmpcompatibilityStatus = COMPATIBILITY_IGMPV3;
uint32	oldVersionHostPresentTimeout;
uint32	oldVersionHostPresentTimeoutIpv6;

uint32	igmpV2Timer;
uint32  igmpV1Timer;
#endif

#if defined(CONFIG_RTL_PER_PORT_CLIENT_NUM)
uint8 enableMaxClientNumCheck = 0;
uint8 enableIpv6MaxClientNumCheck = 0;
uint32 maxClientNumPerPort[2] = {DEFAULT_PER_PORT_MAX_NUM, DEFAULT_PER_PORT_MAX_NUM}; 
uint32  currentClientNum[2][IGMP_MAX_DEV_NUM]={{0},{0}};
int rtk_igmp_updateCurrentClientNum(int ipVersion);
struct net_bridge *rtk_igmp_getBrDeviceByModuleIndex(int32 moduleIndex);
void rtk_igmpsendQuery(struct net_bridge *br);
void rtk_mldsendQuery(struct net_bridge *br);
int rtk_igmp_flushCurrentClientNum(int ipVersion);

#endif

int rtk_igmp_set_wan_learn(int32 value)
{
	int i = 0;
	
	for( i=0 ; i< IGMP_MAX_WAN_DEV_NUM; i++)
	{
		if(rtk_igmp_wanDevInfo[i].dev==NULL)
			continue;
		
		if(value == 0)
		{
			rtk_igmp_devConfig_set(rtk_igmp_wanDevInfo[i].ifindex, 0,RT_REPORT_DROP ,1);
			rtk_igmp_devConfig_set(rtk_igmp_wanDevInfo[i].ifindex, 1,RT_REPORT_DROP ,1);
			rtk_igmp_devConfig_set(rtk_igmp_wanDevInfo[i].ifindex, 0,RT_FAST_LEAVE ,0);
			rtk_igmp_devConfig_set(rtk_igmp_wanDevInfo[i].ifindex, 1,RT_FAST_LEAVE ,0);
			rtk_igmp_devConfig_set(rtk_igmp_wanDevInfo[i].ifindex, 0,RT_CARE_SOURCE ,0);
			rtk_igmp_devConfig_set(rtk_igmp_wanDevInfo[i].ifindex, 1,RT_CARE_SOURCE ,0);
		
		}else
		{
			rtk_igmp_devConfig_set(rtk_igmp_wanDevInfo[i].ifindex, 0,RT_REPORT_DROP ,0);
			rtk_igmp_devConfig_set(rtk_igmp_wanDevInfo[i].ifindex, 1,RT_REPORT_DROP ,0);
			rtk_igmp_devConfig_set(rtk_igmp_wanDevInfo[i].ifindex, 0,RT_FAST_LEAVE ,mCastSnoopingGlobalConfig.igmpFastLeave);
			rtk_igmp_devConfig_set(rtk_igmp_wanDevInfo[i].ifindex, 1,RT_FAST_LEAVE ,mCastSnoopingGlobalConfig.igmp6Fastleave);
			rtk_igmp_devConfig_set(rtk_igmp_wanDevInfo[i].ifindex, 0,RT_CARE_SOURCE ,1);
			rtk_igmp_devConfig_set(rtk_igmp_wanDevInfo[i].ifindex, 1,RT_CARE_SOURCE ,1);
		}
		
	}

	return 0;
		
}

int _rtk_igmp_pasring_string_to_integer(const char *buff,unsigned long len)
{
	char tmpbuf[256];
	int ret;

	memset(tmpbuf,0,sizeof(tmpbuf));
	if (buff)
	{
		/* copy data to the buffer */
		strncpy(tmpbuf, buff, len);

		tmpbuf[len-1] = '\0';
	}

	ret=simple_strtol(tmpbuf, NULL, 0);

	return ret;
}

int rtk_igmp_wanLearn_proc_write(struct file *file, char *buffer, unsigned long count, void *data)
{
	igmpSysdb.WanLearnIgmp = _rtk_igmp_pasring_string_to_integer(buffer,count);
	rtk_igmp_set_wan_learn(igmpSysdb.WanLearnIgmp);

	return count;

}

int rtk_igmp_wanLearn_proc_read(struct seq_file *s, void *v)
{
	PROC_PRINTF("status:%s",igmpSysdb.WanLearnIgmp==0?"disable":"enable\n");
	return SUCCESS;
}

void rtk_igmp_SnoopingconfigInit(void)
{
	rt_igmpHook_ignoreGroup_t patten;
	int32 index;

#if defined(CONFIG_RTL_NEW_IGMP_REPORT_BEHAVIOR)
	mCastSnoopingGlobalConfig.robustness[0]= DEFAULT_ROBUSTNESS;
	mCastSnoopingGlobalConfig.queryInterval[0]=DEFAULT_QUERY_INTERVAL;
	mCastSnoopingGlobalConfig.queryResponseInterval[0]= DEFAULT_QUERY_RESPONSE_INTERVAL;
	mCastSnoopingGlobalConfig.lastMemberQueryInterval[0]= DEFAULT_LAST_MEMBER_INTERVAL;
	mCastSnoopingGlobalConfig.robustness[1]= DEFAULT_ROBUSTNESS;
	mCastSnoopingGlobalConfig.queryInterval[1]=DEFAULT_QUERY_INTERVAL;
	mCastSnoopingGlobalConfig.queryResponseInterval[1]= DEFAULT_QUERY_RESPONSE_INTERVAL;
	mCastSnoopingGlobalConfig.lastMemberQueryInterval[1]= DEFAULT_LAST_MEMBER_INTERVAL;	  
	
	mCastSnoopingGlobalConfig.groupMemberAgingTime[0] = mCastSnoopingGlobalConfig.robustness[0]*mCastSnoopingGlobalConfig.queryInterval[0]+mCastSnoopingGlobalConfig.queryResponseInterval[0];
	mCastSnoopingGlobalConfig.lastMemberAgingTime[0] = mCastSnoopingGlobalConfig.robustness[0]*mCastSnoopingGlobalConfig.lastMemberQueryInterval[0];
	mCastSnoopingGlobalConfig.querierPresentInterval[0] = mCastSnoopingGlobalConfig.robustness[0]*mCastSnoopingGlobalConfig.queryInterval[0]+mCastSnoopingGlobalConfig.queryResponseInterval[0]/2;
	mCastSnoopingGlobalConfig.groupMemberAgingTime[1] = mCastSnoopingGlobalConfig.robustness[1]*mCastSnoopingGlobalConfig.queryInterval[1]+mCastSnoopingGlobalConfig.queryResponseInterval[1];
	mCastSnoopingGlobalConfig.lastMemberAgingTime[1] = mCastSnoopingGlobalConfig.robustness[1]*mCastSnoopingGlobalConfig.lastMemberQueryInterval[1];
	mCastSnoopingGlobalConfig.querierPresentInterval[1] = mCastSnoopingGlobalConfig.robustness[1]*mCastSnoopingGlobalConfig.queryInterval[1]+mCastSnoopingGlobalConfig.queryResponseInterval[1]/2;
#if defined(CONFIG_RTL_LAN_COMPATIBILITY)
	oldVersionHostPresentTimeout=mCastSnoopingGlobalConfig.groupMemberAgingTime[0];
	oldVersionHostPresentTimeoutIpv6=mCastSnoopingGlobalConfig.groupMemberAgingTime[1];	
#endif
#else
	oldVersionHostPresentTimeout = 260;
#endif

	mCastSnoopingGlobalConfig.igmp6Fastleave = 1;
	mCastSnoopingGlobalConfig.igmpFastLeave = 1;

	mCastSnoopingGlobalConfig.igmpCompatibilityEnable = 1;
	//mCastSnoopingGlobalConfig.igmp6CompatibilityEnable = 0;
	
	memset(&patten,0,sizeof(rt_igmpHook_ignoreGroup_t));
	patten.ignGroupIpStart[0]=0xeffffc84; // ADP group address:239.255.252.132
	patten.ignGroupIpEnd[0]=patten.ignGroupIpStart[0];
	rtk_igmp_groupToPsTblAdd( &patten, &index);

	return ;

}

#if defined(CONFIG_RTL_LAN_COMPATIBILITY)
void rtl_get_compatiblity_staus(int* status, uint32* timeout)
{
	*status = igmpcompatibilityStatus;
	
	if(COMPATIBILITY_IGMPV2 == igmpcompatibilityStatus)
		*timeout = igmpV2Timer-rtk_igmp_sysUpSeconds;
	else if(COMPATIBILITY_IGMPV1 == igmpcompatibilityStatus)
		*timeout = igmpV1Timer-rtk_igmp_sysUpSeconds;

	return;
}

#endif

#if defined(CONFIG_RTL_NEW_IGMP_REPORT_BEHAVIOR)

uint32 rtk_igmp_processQueries(rtk_igmp_pktHdr_t *pPkthdr, const struct net_device *SrcDev)
{

	uint32 groupAddress[4]={0,0,0,0};
	uint32 suppressFlag=0;
	uint32 *sourceAddr=NULL;
	uint32 numOfSrc=0;
	uint8 * pktBuf;
	uint8 qqic=0;
	uint32 qqi = 0;
		
	// seems to update multicast router. we need linux bridge multicast router feature.
	//so may be not need to update rtl_snoopQuerier
	/*querier timer update and election process*/
    //rtl_snoopQuerier(moduleIndex, ipVersion, portNum);

	if(!pPkthdr->iph)
	{
		
		IGMP_CTRL("Not support");
		return RTK_IGMP_NF_ACCEPT;
	}
	

	pktBuf=(uint8 *)pPkthdr->igmph;
		
	if(pPkthdr->iph->tot_len > 32) /*means igmpv3 query*/
	{
		groupAddress[0]=ntohl(((struct igmpv3Query*)pktBuf)->groupAddr);
		suppressFlag=((struct igmpv3Query*)pktBuf)->rsq & S_FLAG_MASK;
		sourceAddr=&(((struct igmpv3Query*)pktBuf)->srcList);
		numOfSrc=(uint32)ntohs(((struct igmpv3Query*)pktBuf)->numOfSrc);
		qqic = ((struct igmpv3Query*)pktBuf)->qqic;
		qqi = RTL_IGMPV3_QQIC(qqic);
	}
	else
	{
		groupAddress[0]=ntohl(((struct igmpv2Pkt *)pktBuf)->groupAddr);
	}
			
	if(groupAddress[0]==0) /*means general query*/
	{

		if ((rtk_get_op_mode()==BRIDGE_MODE)&&qqic>0)
		{
			IGMP_CTRL("qqic:%d, qqi:%d", qqic, qqi);
			mCastSnoopingGlobalConfig.queryInterval[0] = qqi;
			mCastSnoopingGlobalConfig.groupMemberAgingTime[0] = mCastSnoopingGlobalConfig.robustness[0]*mCastSnoopingGlobalConfig.queryInterval[0]+mCastSnoopingGlobalConfig.queryResponseInterval[0];
			mCastSnoopingGlobalConfig.querierPresentInterval[0] = mCastSnoopingGlobalConfig.robustness[0]*mCastSnoopingGlobalConfig.queryInterval[0]+mCastSnoopingGlobalConfig.queryResponseInterval[0]/2;
#if defined(CONFIG_RTL_LAN_COMPATIBILITY)
			oldVersionHostPresentTimeout = mCastSnoopingGlobalConfig.robustness[0]*mCastSnoopingGlobalConfig.queryInterval[0]+mCastSnoopingGlobalConfig.queryResponseInterval[0];
#endif
		}

		return RTK_IGMP_NF_ACCEPT;
	}
	return RTK_IGMP_NF_ACCEPT;
}
#endif


void rtk_igmp_igmpSnoopingTimeconfigSet(struct rtl_mCastSnoopingGlobalConfig config)
{
	mCastSnoopingGlobalConfig.robustness[0]= config.robustness[0];
	mCastSnoopingGlobalConfig.queryInterval[0]=config.queryInterval[0];
	mCastSnoopingGlobalConfig.queryResponseInterval[0]= config.queryResponseInterval[0];
	mCastSnoopingGlobalConfig.lastMemberQueryInterval[0]= config.lastMemberQueryInterval[0];
	
	mCastSnoopingGlobalConfig.groupMemberAgingTime[0] = mCastSnoopingGlobalConfig.robustness[0]*mCastSnoopingGlobalConfig.queryInterval[0]+mCastSnoopingGlobalConfig.queryResponseInterval[0];
	mCastSnoopingGlobalConfig.lastMemberAgingTime[0] = mCastSnoopingGlobalConfig.robustness[0]*mCastSnoopingGlobalConfig.lastMemberQueryInterval[0];
	mCastSnoopingGlobalConfig.querierPresentInterval[0] = mCastSnoopingGlobalConfig.robustness[0]*mCastSnoopingGlobalConfig.queryInterval[0]+mCastSnoopingGlobalConfig.queryResponseInterval[0]/2;
#if defined(CONFIG_RTL_LAN_COMPATIBILITY)
	oldVersionHostPresentTimeout=mCastSnoopingGlobalConfig.groupMemberAgingTime[0];
#endif

return ;

}

void rtk_igmp_igmp6SnoopingTimeconfigSet(struct rtl_mCastSnoopingGlobalConfig config)
{
	mCastSnoopingGlobalConfig.robustness[1]= config.robustness[1];
	mCastSnoopingGlobalConfig.queryInterval[1]=config.queryInterval[1];
	mCastSnoopingGlobalConfig.queryResponseInterval[1]= config.queryResponseInterval[1];
	mCastSnoopingGlobalConfig.lastMemberQueryInterval[1]= config.lastMemberQueryInterval[1];
	
	mCastSnoopingGlobalConfig.groupMemberAgingTime[1] = mCastSnoopingGlobalConfig.robustness[1]*mCastSnoopingGlobalConfig.queryInterval[1]+mCastSnoopingGlobalConfig.queryResponseInterval[1];
	mCastSnoopingGlobalConfig.lastMemberAgingTime[1] = mCastSnoopingGlobalConfig.robustness[1]*mCastSnoopingGlobalConfig.lastMemberQueryInterval[1];
	mCastSnoopingGlobalConfig.querierPresentInterval[1] = mCastSnoopingGlobalConfig.robustness[1]*mCastSnoopingGlobalConfig.queryInterval[1]+mCastSnoopingGlobalConfig.queryResponseInterval[1]/2;
#if defined(CONFIG_RTL_LAN_COMPATIBILITY)
	oldVersionHostPresentTimeoutIpv6=mCastSnoopingGlobalConfig.groupMemberAgingTime[1];
#endif

return ;

}


#if defined(CONFIG_RTL_NEW_IGMP_REPORT_BEHAVIOR)|| defined(CONFIG_RTL_LAN_COMPATIBILITY) || defined (CONFIG_RTL_PER_PORT_CLIENT_NUM)

void rtk_igmp_igmpSnoopingFastLeaveSet(uint8 enable)
{

	int i;

	mCastSnoopingGlobalConfig.igmpFastLeave = enable;

	//printk("set fastleave value is %d\n",mCastSnoopingGlobalConfig.igmpFastLeave);
	 
	for( i=0 ; i< IGMP_MAX_LAN_DEV_NUM; i++)
	{
		if(rtk_igmp_lanDevInfo[i].dev==NULL)
			continue;
	
		rtk_igmp_lanDevInfo[i].devConf.igmp.igmpfastLeave= enable;
	}

	for( i=0 ; i< IGMP_MAX_WAN_DEV_NUM; i++)
	{
		if(rtk_igmp_wanDevInfo[i].dev==NULL)
			continue;
		
		rtk_igmp_wanDevInfo[i].devConf.igmp.igmpfastLeave = enable;
	}


	return;
}

void rtk_igmp_igmp6SnoopingFastLeaveSet(uint8 enable)
{

	int i;

	mCastSnoopingGlobalConfig.igmp6Fastleave = enable;

	//printk("set fastleave value is %d\n",mCastSnoopingGlobalConfig.igmpFastLeave);
	 
	for( i=0 ; i< IGMP_MAX_LAN_DEV_NUM; i++)
	{
		if(rtk_igmp_lanDevInfo[i].dev==NULL)
			continue;
	
		rtk_igmp_lanDevInfo[i].devConf.igmp6.igmp6fastLeave= enable;
	}

	for( i=0 ; i< IGMP_MAX_WAN_DEV_NUM; i++)
	{
		if(rtk_igmp_wanDevInfo[i].dev==NULL)
			continue;
		
		rtk_igmp_wanDevInfo[i].devConf.igmp6.igmp6fastLeave = enable;
	}


	return;
}


int rtk_igmp_igmpSnoopingConfig_proc_read(struct seq_file *s, void *v)
{
#if defined(CONFIG_RTL_PER_PORT_CLIENT_NUM)
	int i;
#endif
#if defined(CONFIG_RTL_LAN_COMPATIBILITY)
	int igmpcompatibilityStatus;
	uint32 timeout = 0;
#endif

	PROC_PRINTF("igmp Config\n");
#if defined(CONFIG_RTL_LAN_COMPATIBILITY)

	rtl_get_compatiblity_staus(&igmpcompatibilityStatus, &timeout);

	PROC_PRINTF("-------------------------------------------------------\n");
	PROC_PRINTF("compatible :%s, oldVersionHostPresentTimeout=%d \n", mCastSnoopingGlobalConfig.igmpCompatibilityEnable ==1?"enabled\n":"disabled",oldVersionHostPresentTimeout);
	PROC_PRINTF("status:%s, timeout:%d\n", 
		(igmpcompatibilityStatus==COMPATIBILITY_IGMPV3)?"IGMP V3":((igmpcompatibilityStatus==COMPATIBILITY_IGMPV2)?"IGMP V2":"IGMP V1"),timeout);

	PROC_PRINTF("Usage:\n");
	PROC_PRINTF("echo compatible $compatible_value > /proc/igmp/ctrl/igmpSnoopingConfig \n");
	
#endif
#if defined(CONFIG_RTL_NEW_IGMP_REPORT_BEHAVIOR)
	PROC_PRINTF("-------------------------------------------------------\n");
	PROC_PRINTF("Time Config:\n");
	PROC_PRINTF("  %s %d\n  %s %d\n  %s %d\n  %s %d\n  %s %d\n  %s %d\n  %s %d\n\n",
		"Robustness value:", mCastSnoopingGlobalConfig.robustness[0],
		"Query Interval:", mCastSnoopingGlobalConfig.queryInterval[0],
		"Query Response Interval:",mCastSnoopingGlobalConfig.queryResponseInterval[0],
		"Group Member Interval:", mCastSnoopingGlobalConfig.groupMemberAgingTime[0],
		"Last Member Query Interval:", mCastSnoopingGlobalConfig.lastMemberQueryInterval[0],
		"Last Member Query Count:", mCastSnoopingGlobalConfig.robustness[0],
		"Last Member Interval:", mCastSnoopingGlobalConfig.lastMemberAgingTime[0]);
	PROC_PRINTF("Usage:\n");
	PROC_PRINTF("echo %s > /proc/igmp/ctrl/igmpSnoopingConfig\n","time robust $robust_value query $query_interval response $query_response_interval lastmember $last_member_query_interval");
#endif

	PROC_PRINTF("-------------------------------------------------------\n");
	PROC_PRINTF("Fastleave Config:\n");
	PROC_PRINTF("  Igmp fastleave is %s\n",(mCastSnoopingGlobalConfig.igmpFastLeave == 0)?"disable\n":"enable\n");
	PROC_PRINTF("Usage:\n");
	PROC_PRINTF("echo %s > /proc/igmp/ctrl/igmpSnoopingConfig\n","fastleave $enableFastleave");
	
	PROC_PRINTF("-------------------------------------------------------\n");
#if defined(CONFIG_RTL_PER_PORT_CLIENT_NUM)

	PROC_PRINTF("maxnumber Config:\n");
	PROC_PRINTF("  Igmp max number of registered membership per port is %s,maxClientNumPerPort=%d\n",(enableMaxClientNumCheck==1)?"enable":"disable",maxClientNumPerPort[0]);
	if(enableMaxClientNumCheck==1)
	{
		PROC_PRINTF("Client Number Info:\n");
		for(i=0;i<IGMP_MAX_LAN_DEV_NUM;i++)
		{
			if((rtk_igmp_lanDevInfo[i].dev!=NULL)&&(currentClientNum[0][rtk_igmp_lanDevInfo[i].ifindex]>0))
			{
				PROC_PRINTF("  device [%s] Igmp joined client number [%d]\n",rtk_igmp_lanDevInfo[i].dev->name,currentClientNum[0][rtk_igmp_lanDevInfo[i].ifindex]);
			}
		}

		if(igmpSysdb.WanLearnIgmp) 
		{
			for(i=0;i<IGMP_MAX_WAN_DEV_NUM;i++)
			{
				if((rtk_igmp_wanDevInfo[i].dev!=NULL)&&(currentClientNum[0][rtk_igmp_wanDevInfo[i].ifindex]>0))
				{
					PROC_PRINTF("  device [%s] Igmp joined client number [%d]\n",rtk_igmp_wanDevInfo[i].dev->name,currentClientNum[0][rtk_igmp_wanDevInfo[i].ifindex]);
				}
			}
		
		}
	}

	PROC_PRINTF("Usage:\n");
	PROC_PRINTF("echo %s > /proc/igmp/ctrl/igmpSnoopingConfig\n","maxnum $enable $num");
#endif			
	return 0;
}



int rtk_igmp_igmpSnoopingConfig_proc_write(struct file *file, char *buffer, unsigned long count, void *data)
{
	unsigned char tmpBuf[256] = {0};
	int len = (count >= 255) ? 255 : count;
	char *strptr = NULL;
	char *tokptr = NULL;
	int fastLeave=0;
	uint8 enable = 0;
	struct rtl_mCastSnoopingGlobalConfig config;
#if defined(CONFIG_RTL_PER_PORT_CLIENT_NUM)
	int maxNum=0, checkMaxNumEnable=0;
#endif


	if(buffer)
	{	
		/* copy data to the buffer */
		strncpy(tmpBuf, buffer, len);

		tmpBuf[len] = '\0';

		memset(&config,0,sizeof(config));

		strptr = tmpBuf;
		
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			printk("tokptr is null 1\n");
			goto ERR_INPUT;
		}


		if(!memcmp(tokptr, "fastleave", 9))
		{
			enable = 0;
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				printk("tokptr is null 2\n");
				goto ERR_INPUT;
			}
			
			fastLeave = simple_strtol(tokptr, NULL, 0);

			if(fastLeave)
			{
				enable=1;
			}
			
			rtk_igmp_igmpSnoopingFastLeaveSet(enable);
			return len;
		}
#if defined(CONFIG_RTL_LAN_COMPATIBILITY)
		else if(!memcmp(tokptr, "compatible", 10))
		{
			enable = 0;
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				printk("tokptr is null 2\n");
				goto ERR_INPUT;
			}
			
			enable = simple_strtol(tokptr, NULL, 0);

			
			if(enable)
				mCastSnoopingGlobalConfig.igmpCompatibilityEnable = 1;
			else
				mCastSnoopingGlobalConfig.igmpCompatibilityEnable = 0;

			return len;
		}
#endif

#if defined(CONFIG_RTL_NEW_IGMP_REPORT_BEHAVIOR)
		else if(!memcmp(tokptr, "time",4))
		{
			while(tokptr)
			{
				if(!memcmp(tokptr, "robust",6))
				{
					tokptr = strsep(&strptr," ");
					if (tokptr==NULL)
					{
						goto ERR_INPUT;
					}
					config.robustness[0] = simple_strtol(tokptr, NULL, 0);
				}
				else if(!memcmp(tokptr, "query",5))
				{
					tokptr = strsep(&strptr," ");
					if (tokptr==NULL)
					{
						goto ERR_INPUT;
					}
					config.queryInterval[0] = simple_strtol(tokptr, NULL, 0);
				}
				else if(!memcmp(tokptr, "response",8))
				{
					tokptr = strsep(&strptr," ");
					if (tokptr==NULL)
					{
						goto ERR_INPUT;
					}
					config.queryResponseInterval[0] = simple_strtol(tokptr, NULL, 0);
				}
				else if(!memcmp(tokptr, "lastmember",10))
				{
					tokptr = strsep(&strptr," ");
					if (tokptr==NULL)
					{
						goto ERR_INPUT;
					}
					config.lastMemberQueryInterval[0] = simple_strtol(tokptr, NULL, 0);
				}
				
				tokptr = strsep(&strptr," ");
			}
			
			rtk_igmp_igmpSnoopingTimeconfigSet(config);
			//rtk_igmp_igmpSnoopingConfig_proc_read(NULL,NULL);
			return len;
		}
	
#endif
#if defined(CONFIG_RTL_PER_PORT_CLIENT_NUM)
		else if(!memcmp(tokptr, "maxnum", 6))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto ERR_INPUT;
			}
						
			checkMaxNumEnable = simple_strtol(tokptr, NULL, 0);
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto ERR_INPUT;
			}
			maxNum = simple_strtol(tokptr, NULL, 0);
			if(checkMaxNumEnable)
			{
				if(!enableMaxClientNumCheck)
					rtk_igmp_updateCurrentClientNum(4);

				enableMaxClientNumCheck = 1;
				maxClientNumPerPort[CLIENT_NUM_ID(4)] = maxNum;
			}
			else
			{
				if(enableMaxClientNumCheck)
				{
					rtk_igmp_flushCurrentClientNum(4);
					enableMaxClientNumCheck = 0;
				}
			}				
		}
#endif

		else
		{
			goto ERR_INPUT;
		}
	}

ERR_INPUT:
	rtk_igmp_igmpSnoopingConfig_proc_read(NULL,NULL);
	return len;
}




int rtk_igmp_igmp6SnoopingConfig_proc_read(struct seq_file *s, void *v)
{
#if defined(CONFIG_RTL_PER_PORT_CLIENT_NUM)
	int i;
#endif
	PROC_PRINTF("Mld Config\n");
	
#if defined(CONFIG_RTL_NEW_IGMP_REPORT_BEHAVIOR)
	PROC_PRINTF("-------------------------------------------------------\n");
	PROC_PRINTF("Time Config:\n");
	PROC_PRINTF("  %s %d\n  %s %d\n  %s %d\n  %s %d\n  %s %d\n  %s %d\n  %s %d\n\n",
		"Robustness value:", mCastSnoopingGlobalConfig.robustness[1],
		"Query Interval:", mCastSnoopingGlobalConfig.queryInterval[1],
		"Query Response Interval:",mCastSnoopingGlobalConfig.queryResponseInterval[1],
		"Group Member Interval:", mCastSnoopingGlobalConfig.groupMemberAgingTime[1],
		"Last Member Query Interval:", mCastSnoopingGlobalConfig.lastMemberQueryInterval[1],
		"Last Member Query Count:", mCastSnoopingGlobalConfig.robustness[1],
		"Last Member Interval:", mCastSnoopingGlobalConfig.lastMemberAgingTime[1]);
	PROC_PRINTF("Usage:\n");
	PROC_PRINTF("echo %s > /proc/igmp/ctrl/igmp6SnoopingConfig\n","time robust $robust_value query $query_interval response $query_response_interval lastmember $last_member_query_interval");
#endif
	PROC_PRINTF("-------------------------------------------------------\n");
	PROC_PRINTF("Fastleave Config:\n");
	PROC_PRINTF("  Mld fastleave is %s\n",(mCastSnoopingGlobalConfig.igmp6Fastleave == 0)?"disable\n":"enable\n");
	PROC_PRINTF("Usage:\n");
	PROC_PRINTF("echo %s > /proc/igmp/ctrl/igmp6SnoopingConfig\n","fastleave $enableFastleave");
	PROC_PRINTF("-------------------------------------------------------\n");
#if defined(CONFIG_RTL_PER_PORT_CLIENT_NUM)

	PROC_PRINTF("maxnumber Config:\n");
	PROC_PRINTF("  Igmp max number of registered membership per port is %s,maxClientNumPerPort=%d\n",(enableIpv6MaxClientNumCheck==1)?"enable":"disable",maxClientNumPerPort[1]);
	if(enableIpv6MaxClientNumCheck==1)
	{
		PROC_PRINTF("Client Number Info:\n");
		for(i=0;i<IGMP_MAX_LAN_DEV_NUM;i++)
		{
			if((rtk_igmp_lanDevInfo[i].dev!=NULL)&&(currentClientNum[1][rtk_igmp_lanDevInfo[i].ifindex]>0))
			{
				PROC_PRINTF("  device [%s] Mld joined client number [%d]\n",rtk_igmp_lanDevInfo[i].dev->name,currentClientNum[1][rtk_igmp_lanDevInfo[i].ifindex]);
			}
		}

		if(igmpSysdb.WanLearnIgmp) 
		{
			for(i=0;i<IGMP_MAX_WAN_DEV_NUM;i++)
			{
				if((rtk_igmp_wanDevInfo[i].dev!=NULL)&&(currentClientNum[1][rtk_igmp_wanDevInfo[i].ifindex]>0))
				{
					PROC_PRINTF("  device [%s] Igmp joined client number [%d]\n",rtk_igmp_wanDevInfo[i].dev->name,currentClientNum[1][rtk_igmp_wanDevInfo[i].ifindex]);
				}
			}
		
		}
	}
	PROC_PRINTF("Usage:\n");
	PROC_PRINTF("echo %s > /proc/igmp/ctrl/igmp6SnoopingConfig\n","maxnum $enable $num");
#endif			
	return 0;
}


int rtk_igmp_igmp6SnoopingConfig_proc_write(struct file *file,char *buffer, unsigned long count, void *data)
{
	unsigned char tmpBuf[256] = {0};
	int len = (count >= 255) ? 255 : count;
	char *strptr = NULL;
	char *tokptr = NULL;
	int fastLeave=0;
	uint8 enable = 0;
	struct rtl_mCastSnoopingGlobalConfig config;
#if defined(CONFIG_RTL_PER_PORT_CLIENT_NUM)
	int maxNum=0, checkMaxNumEnable=0;
#endif


	if(buffer)
	{	
		//copy data to the buffer
		strncpy(tmpBuf, buffer, len);

		tmpBuf[len] = '\0';

		memset(&config,0,sizeof(config));

		strptr = tmpBuf;
		
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			printk("tokptr is null 1\n");
			goto ERR_INPUT;
		}

		if(!memcmp(tokptr, "fastleave", 9))
		{
			enable = 0;
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				printk("tokptr is null 2\n");
				goto ERR_INPUT;
			}
			
			fastLeave = simple_strtol(tokptr, NULL, 0);

			if(fastLeave)
			{
				enable=1;
			}
			
			rtk_igmp_igmp6SnoopingFastLeaveSet(enable);
			return len;
		}
#if defined(CONFIG_RTL_NEW_IGMP_REPORT_BEHAVIOR)
		else if(!memcmp(tokptr, "time",4))
		{
			while(tokptr)
			{
				if(!memcmp(tokptr, "robust",6))
				{
					tokptr = strsep(&strptr," ");
					if (tokptr==NULL)
					{
						goto ERR_INPUT;
					}
					config.robustness[1] = simple_strtol(tokptr, NULL, 0);
				}
				else if(!memcmp(tokptr, "query",5))
				{
					tokptr = strsep(&strptr," ");
					if (tokptr==NULL)
					{
						goto ERR_INPUT;
					}
					config.queryInterval[1] = simple_strtol(tokptr, NULL, 0);
				}
				else if(!memcmp(tokptr, "response",8))
				{
					tokptr = strsep(&strptr," ");
					if (tokptr==NULL)
					{
						goto ERR_INPUT;
					}
					config.queryResponseInterval[1] = simple_strtol(tokptr, NULL, 0);
				}
				else if(!memcmp(tokptr, "lastmember",10))
				{
					tokptr = strsep(&strptr," ");
					if (tokptr==NULL)
					{
						goto ERR_INPUT;
					}
					config.lastMemberQueryInterval[1] = simple_strtol(tokptr, NULL, 0);
				}
				
				tokptr = strsep(&strptr," ");
			}
			
			rtk_igmp_igmp6SnoopingTimeconfigSet(config);
			//rtk_igmp_igmpSnoopingConfig_proc_read(NULL,NULL);
			return len;
		}
#endif
#if defined(CONFIG_RTL_PER_PORT_CLIENT_NUM)
		else if(!memcmp(tokptr, "maxnum", 6))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto ERR_INPUT;
			}
						
			checkMaxNumEnable = simple_strtol(tokptr, NULL, 0);
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto ERR_INPUT;
			}
			maxNum = simple_strtol(tokptr, NULL, 0);
			if(checkMaxNumEnable)
			{
				if(!enableIpv6MaxClientNumCheck)
					rtk_igmp_updateCurrentClientNum(6);

				enableIpv6MaxClientNumCheck = 1;
				maxClientNumPerPort[CLIENT_NUM_ID(6)] = maxNum;
			}
			else
			{
				if(enableIpv6MaxClientNumCheck)
				{
					rtk_igmp_flushCurrentClientNum(6);
					enableIpv6MaxClientNumCheck = 0;
				}
			}				
		}
#endif

		else
		{
			goto ERR_INPUT;
		}
	}

ERR_INPUT:
	rtk_igmp_igmp6SnoopingConfig_proc_read(NULL,NULL);

	return len;


}


#endif //defined(CONFIG_RTL_NEW_IGMP_REPORT_BEHAVIOR)|| defined(CONFIG_RTL_LAN_COMPATIBILITY) || defined (CONFIG_RTL_PER_PORT_CLIENT_NUM)

#if defined(CONFIG_RTL_PER_PORT_CLIENT_NUM)

int rtk_igmp_brIgmpGeneralQueryVersion_proc_write(struct file *file,char *buffer, unsigned long count, void *data)
{
	if (buffer)
	{
		/* copy data to the buffer */
		igmpVersion = simple_strtol(buffer, NULL, 0);
		if(igmpVersion>=3)
		{
			igmpVersion=3;
		}
		else if (igmpVersion<=2)
		{
			igmpVersion=2;
		}
		else
		{
			igmpVersion=2;
		}
	}
	return count;
}

int rtk_igmp_brIgmpGeneralQueryVersion_proc_read(struct seq_file *s, void *v)
{
	
	PROC_PRINTF("igmp general query version is %s\n", 
			(igmpVersion==3)?"IGMP V3":"IGMP V2");
	PROC_PRINTF("Usage:\n");
	PROC_PRINTF("echo $version_value > /proc/igmp/ctrl/igmpQueryVersion \n");
	return 0;
}

int rtk_igmp_brMldGeneralQueryVersion_proc_write(struct file *file,char *buffer, unsigned long count, void *data)
{
	if (buffer)
	{
		/* copy data to the buffer */
		mldVersion = simple_strtol(buffer, NULL, 0);
		if(mldVersion>=2)
		{
			mldVersion=2;
		}
		else if (mldVersion<=1)
		{
			mldVersion=1;
		}
		else
		{
			mldVersion=2;
		}
	}
	return count;
}

int rtk_igmp_brMldGeneralQueryVersion_proc_read(struct seq_file *s, void *v)
{
	
	PROC_PRINTF("igmp general query version is %s\n", 
			(mldVersion==2)?"MLD V2":"MLD V1");
	PROC_PRINTF("Usage:\n");
	PROC_PRINTF("echo $version_value > /proc/igmp/ctrl/igmp6QueryVersion \n");
	return 0;
}



//enable maxnum --> disable maxnum
int rtk_igmp_flushCurrentClientNum(int ipVersion)
{
	int moduleIndex ;
	int32 hashIndex;
	struct rtk_igmp_groupEntry* groupEntryPtr=NULL;
	struct rtk_igmp_clientEntry* clientEntry=NULL;

	for(moduleIndex=0 ; moduleIndex<IGMP_MAX_BR_MODULE_NUM ;moduleIndex++ )
	{
		if( ! rtk_igmp_mCastModuleArray[moduleIndex].validBit) 
			continue;

		for (hashIndex=0;hashIndex<IGMP_HASH_TBL_SIZE;hashIndex++)
		{

			if(ipVersion==IP_VERSION4)
				groupEntryPtr=rtk_igmp_mCastModuleArray[moduleIndex].rtk_igmp_ipv4HashTable[hashIndex];
			else
				groupEntryPtr=rtk_igmp_mCastModuleArray[moduleIndex].rtk_igmp_ipv6HashTable[hashIndex];
		
			
			while (groupEntryPtr!=NULL)
			{

				clientEntry=groupEntryPtr->clientList;

				while (clientEntry!=NULL)
				{
					clientEntry->flag &= (~FLAG_WAIT_STATUS);
					clientEntry = clientEntry->next;
				}
				groupEntryPtr=groupEntryPtr->next;
			}

		}
		
	}

	memset(currentClientNum[CLIENT_NUM_ID(ipVersion)], 0, IGMP_MAX_DEV_NUM*sizeof(uint32));
	maxClientNumPerPort[CLIENT_NUM_ID(ipVersion)] = DEFAULT_PER_PORT_MAX_NUM; 	
	return SUCCESS;

}


int rtk_igmp_checkClientEmptySpace(uint32 moduleIndex, uint32 ipVersion, uint32 inIfidx, uint32 *groupAddress, uint32 *clientAddress)
{
	int portIndex=-1 , i;

	//if disable, just return True. 
	if((!(enableMaxClientNumCheck&&ipVersion==IP_VERSION4))&&(!(enableIpv6MaxClientNumCheck&&ipVersion==IP_VERSION6)))
		return TRUE;

	
	
	for(i=0;i<IGMP_MAX_LAN_DEV_NUM;i++)
	{
		if((rtk_igmp_lanDevInfo[i].dev!=NULL)&&(rtk_igmp_lanDevInfo[i].ifindex==inIfidx))
		{
			portIndex = inIfidx;
			break;
		}
	}

	if((igmpSysdb.WanLearnIgmp) && (portIndex ==-1))
	{
		for(i=0;i<IGMP_MAX_WAN_DEV_NUM;i++)
		{
			if((rtk_igmp_wanDevInfo[i].dev!=NULL)&&(rtk_igmp_wanDevInfo[i].ifindex==inIfidx))
			{
				portIndex = inIfidx;
				break;
			}
		}

	}

	if(portIndex!=-1)
	{
		if(currentClientNum[CLIENT_NUM_ID(ipVersion)][portIndex]>=maxClientNumPerPort[CLIENT_NUM_ID(ipVersion)])
			return FALSE;
		else
			currentClientNum[CLIENT_NUM_ID(ipVersion)][portIndex]++;
	
	}else
		return FALSE;

	return TRUE;
}

int rtk_igmp_updateCurrentClientNum(int ipVersion)
{
	int32 moduleIndex,i;
	int32 hashIndex;
	struct rtk_igmp_groupEntry *groupEntryPtr;
	struct rtk_igmp_clientEntry* clientEntry=NULL;
	uint32	currentClientNumTemp[IGMP_MAX_DEV_NUM]={0};

	for(moduleIndex=0; moduleIndex<IGMP_MAX_BR_MODULE_NUM ;moduleIndex++)
	{
		if(rtk_igmp_mCastModuleArray[moduleIndex].validBit == 0) 
		{
			continue;
		}	
		for(hashIndex=0;hashIndex<IGMP_HASH_TBL_SIZE;hashIndex++)
     	{
			if(ipVersion==IP_VERSION4)
				groupEntryPtr=rtk_igmp_mCastModuleArray[moduleIndex].rtk_igmp_ipv4HashTable[hashIndex];
			else
				groupEntryPtr=rtk_igmp_mCastModuleArray[moduleIndex].rtk_igmp_ipv6HashTable[hashIndex];

			
			while(groupEntryPtr!=NULL)
			{
				clientEntry=groupEntryPtr->clientList;
			
				while(clientEntry!=NULL)
				{

					for(i=0;i<IGMP_MAX_LAN_DEV_NUM;i++)
					{
						if((rtk_igmp_lanDevInfo[i].dev!=NULL)&&(rtk_igmp_lanDevInfo[i].ifindex==clientEntry->inIfidx))
						{
							if((clientEntry->flag&FLAG_WAIT_STATUS)==0)
								currentClientNumTemp[clientEntry->inIfidx]++;
							break;
						}
					}

					if(igmpSysdb.WanLearnIgmp)
					{
						for(i=0;i<IGMP_MAX_WAN_DEV_NUM;i++)
						{
							if((rtk_igmp_wanDevInfo[i].dev!=NULL)&&(rtk_igmp_wanDevInfo[i].ifindex==clientEntry->inIfidx))
							{
								if((clientEntry->flag&FLAG_WAIT_STATUS)==0)
									currentClientNumTemp[clientEntry->inIfidx]++;
								break;
							}

						}
					}
					
					clientEntry = clientEntry->next;
				}
				groupEntryPtr=groupEntryPtr->next;	
			}
		
       	}
		
	}

	memcpy(currentClientNum[CLIENT_NUM_ID(ipVersion)], currentClientNumTemp, IGMP_MAX_DEV_NUM*sizeof(uint32));
	
	return SUCCESS;	
}


int rtk_igmp_moveWaitEntrytoFwd(uint32 moduleIndex, uint32 ipVersion,uint32 inIfidx)
{
	int32 hashIndex;
	struct rtk_igmp_groupEntry *groupEntryPtr;
	struct rtk_igmp_clientEntry* clientEntry=NULL;
	//uint32 groupAddress[4]={0, 0, 0, 0};
	//uint32 clientAddress[4]={0, 0, 0, 0};
	int waitEntryExist = 0;
	
	
	//for(moduleIndex=0; moduleIndex<IGMP_MAX_BR_MODULE_NUM ;moduleIndex++)
	{
		if( ! rtk_igmp_mCastModuleArray[moduleIndex].validBit) 
			return waitEntryExist;

		for(hashIndex=0;hashIndex<IGMP_HASH_TBL_SIZE;hashIndex++)
	    {
	    	
			if(ipVersion==IP_VERSION4)
				groupEntryPtr=rtk_igmp_mCastModuleArray[moduleIndex].rtk_igmp_ipv4HashTable[hashIndex];
			else
				groupEntryPtr=rtk_igmp_mCastModuleArray[moduleIndex].rtk_igmp_ipv6HashTable[hashIndex];

			while(groupEntryPtr!=NULL)
			{
				clientEntry=groupEntryPtr->clientList;
			
				while(clientEntry!=NULL)
				{

					if((ipVersion==IP_VERSION4)&&(clientEntry->inIfidx==inIfidx) && (clientEntry->flag&FLAG_WAIT_STATUS))
					{
						/*
						groupAddress[0] = groupEntryPtr->groupAddr[0];
						clientAddress[0] = clientEntry->clientAddr[0];*/
						waitEntryExist = 1;
						clientEntry->flag &= (~FLAG_WAIT_STATUS);
						break;
					}
					else if((ipVersion==IP_VERSION6)&&(clientEntry->inIfidx==inIfidx) && (clientEntry->flag&FLAG_WAIT_STATUS))
					{
						/*
						groupAddress[0] = groupEntryPtr->groupAddr[0];
						groupAddress[1] = groupEntryPtr->groupAddr[1];
						groupAddress[2] = groupEntryPtr->groupAddr[2];
						groupAddress[3] = groupEntryPtr->groupAddr[3];
						clientAddress[0] = clientEntry->clientAddr[0];
						clientAddress[1] = clientEntry->clientAddr[1];
						clientAddress[2] = clientEntry->clientAddr[2];
						clientAddress[3] = clientEntry->clientAddr[3];*/
						waitEntryExist = 1;
						clientEntry->flag &= (~FLAG_WAIT_STATUS);
						break;
					}
					
					clientEntry = clientEntry->next;
				}
				
				if(waitEntryExist)
					break;
				groupEntryPtr=groupEntryPtr->next;	
			}

		}
	}


	/*if(waitEntryExist)
	{
		//in 97F port may exist br&nic moudule at same time. so need check all modules.
		//for 98D, it should not happens. 
		//find a wait entry, move the wait status of this client for all modules.
		for(moduleIndex=0; moduleIndex<IGMP_MAX_BR_MODULE_NUM ;moduleIndex++)
		{
			if( ! rtk_igmp_mCastModuleArray[moduleIndex].validBit) 
				continue;

			for(hashIndex=0;hashIndex<IGMP_HASH_TBL_SIZE;hashIndex++)
		    {
		    	
				if(ipVersion==IP_VERSION4)
					groupEntryPtr=rtk_igmp_mCastModuleArray[moduleIndex].rtk_igmp_ipv4HashTable[hashIndex];
				else
					groupEntryPtr=rtk_igmp_mCastModuleArray[moduleIndex].rtk_igmp_ipv6HashTable[hashIndex];

				while(groupEntryPtr!=NULL)
				{
					if((ipVersion==IP_VERSION4)&&(groupEntryPtr->groupAddr[0]==groupAddress[0]))
					{
						clientEntry=groupEntryPtr->clientList;
						while (clientEntry!=NULL)
						{					
							if(clientEntry->clientAddr[0]==clientAddress[0])
								clientEntry->flag &= (~FLAG_WAIT_STATUS);

							clientEntry = clientEntry->next;
						}
					}

	
					if((ipVersion==IP_VERSION4)&&(groupEntryPtr->groupAddr[0]==groupAddress[0])&&
						(groupEntryPtr->groupAddr[1]==groupAddress[1])&&
						(groupEntryPtr->groupAddr[2]==groupAddress[2])&&
						(groupEntryPtr->groupAddr[3]==groupAddress[3]))
					{
						clientEntry=groupEntryPtr->clientList;
						while (clientEntry!=NULL)
						{					
							if((clientEntry->clientAddr[0]==clientAddress[0])&&
								(clientEntry->clientAddr[1]==clientAddress[1])&&
								(clientEntry->clientAddr[2]==clientAddress[2])&&
								(clientEntry->clientAddr[3]==clientAddress[3]))
							clientEntry->flag &= (~FLAG_WAIT_STATUS);

							clientEntry = clientEntry->next;
						}
					}
				
					groupEntryPtr=groupEntryPtr->next;	
				}

			}
		
		}
		
	}*/

	return waitEntryExist;
}


//recover client entry status, and update currentClientNum
int rtk_igmp_recoverClientSpace(uint32 moduleIndex, uint32 ipVersion, uint32 inIfidx)
{
	int portIndex=-1,i;

	int waitEntryExist = 0;

	if(!(enableMaxClientNumCheck&&ipVersion==IP_VERSION4)
		&&!(enableIpv6MaxClientNumCheck &&(ipVersion==IP_VERSION6)))
		return FALSE;
		
	waitEntryExist = rtk_igmp_moveWaitEntrytoFwd(moduleIndex, ipVersion, inIfidx);
	if(!waitEntryExist)
	{
		
		for(i=0;i<IGMP_MAX_LAN_DEV_NUM;i++)
		{
			if((rtk_igmp_lanDevInfo[i].dev!=NULL)&&(rtk_igmp_lanDevInfo[i].ifindex==inIfidx))
			{
				portIndex = inIfidx;
				break;
			}
		}

		if((igmpSysdb.WanLearnIgmp) && (portIndex=-1))
		{
			for(i=0;i<IGMP_MAX_WAN_DEV_NUM;i++)
			{
				if((rtk_igmp_wanDevInfo[i].dev!=NULL)&&(rtk_igmp_wanDevInfo[i].ifindex==inIfidx))
				{
					portIndex = inIfidx;
					break;
				}

			}
		}
		
		if(portIndex!= -1)
			currentClientNum[CLIENT_NUM_ID(ipVersion)][portIndex]--;
	}
	return TRUE;

}

#define MAX_REPONSE_INTERVAL 2

int rtk_igmp_enterWaitStatus(int32 moduleIndex, uint32 ipVersion)
{
	int32 hashIndex;
	struct rtk_igmp_groupEntry* groupEntryPtr=NULL;
	struct rtk_igmp_clientEntry* clientEntry = NULL;
	struct rtk_igmp_sourceEntry* sourceEntryPtr=NULL;
	struct net_bridge *br =NULL;
	
#if defined(CONFIG_RTL_NEW_IGMP_REPORT_BEHAVIOR)
	int verId = CLIENT_NUM_ID(ipVersion);
#endif

	//set timer 
	if(rtk_igmp_mCastModuleArray[moduleIndex].validBit)
	{

		for(hashIndex=0;hashIndex<IGMP_HASH_TBL_SIZE;hashIndex++)
		{
			if((rtk_igmp_mCastModuleArray[moduleIndex].rtk_igmp_ipv4HashTable != NULL)&&(ipVersion==IP_VERSION4))
				groupEntryPtr=rtk_igmp_mCastModuleArray[moduleIndex].rtk_igmp_ipv4HashTable[hashIndex];
			else if((rtk_igmp_mCastModuleArray[moduleIndex].rtk_igmp_ipv6HashTable != NULL)&&(ipVersion==IP_VERSION6))
				groupEntryPtr=rtk_igmp_mCastModuleArray[moduleIndex].rtk_igmp_ipv6HashTable[hashIndex];
			else
				groupEntryPtr=NULL;
			
			while(groupEntryPtr!=NULL)
			{
				clientEntry=groupEntryPtr->clientList;
				while (clientEntry!=NULL)
				{	
					//igmp general query max response time is 0.1s
#if defined(CONFIG_RTL_NEW_IGMP_REPORT_BEHAVIOR)
					if(clientEntry->groupFilterTimer>rtk_igmp_sysUpSeconds+mCastSnoopingGlobalConfig.lastMemberAgingTime[verId]+MAX_REPONSE_INTERVAL)
					{
						clientEntry->groupFilterTimer = rtk_igmp_sysUpSeconds+mCastSnoopingGlobalConfig.lastMemberAgingTime[verId]+MAX_REPONSE_INTERVAL;
					}

#else
					if(clientEntry->groupFilterTimer>rtk_igmp_sysUpSeconds+mCastSnoopingGlobalConfig.lastMemberAgingTime+MAX_REPONSE_INTERVAL)
					{
						clientEntry->groupFilterTimer = rtk_igmp_sysUpSeconds+mCastSnoopingGlobalConfig.lastMemberAgingTime+MAX_REPONSE_INTERVAL;
					}
#endif
				
					sourceEntryPtr=clientEntry->sourceList;

					while(sourceEntryPtr!=NULL)
					{							
#if defined(CONFIG_RTL_NEW_IGMP_REPORT_BEHAVIOR)
						if(sourceEntryPtr->portTimer>rtk_igmp_sysUpSeconds+mCastSnoopingGlobalConfig.lastMemberAgingTime[verId]+MAX_REPONSE_INTERVAL)
						{
							sourceEntryPtr->portTimer = rtk_igmp_sysUpSeconds+mCastSnoopingGlobalConfig.lastMemberAgingTime[verId]+MAX_REPONSE_INTERVAL;
						}

#else
						if(sourceEntryPtr->portTimer>rtk_igmp_sysUpSeconds+mCastSnoopingGlobalConfig.lastMemberAgingTime+MAX_REPONSE_INTERVAL)
						{
							sourceEntryPtr->portTimer = rtk_igmp_sysUpSeconds+mCastSnoopingGlobalConfig.lastMemberAgingTime+MAX_REPONSE_INTERVAL;
						}
#endif
						
						sourceEntryPtr=sourceEntryPtr->next;
					}
					clientEntry = clientEntry->next;
				}
				groupEntryPtr=groupEntryPtr->next;	
			}				
		}
	}

	//send igmp/mld genral query  
	br = rtk_igmp_getBrDeviceByModuleIndex(moduleIndex);
	if(br)
	{
		if(ipVersion==IP_VERSION4)
			rtk_igmpsendQuery(br);
		else
			rtk_mldsendQuery(br);
	}
	
	return SUCCESS;
}
#endif


#if defined(CONFIG_RTL_PER_PORT_CLIENT_NUM)

/*igmpv3 general query*/
static unsigned char igmpV3QueryBuf[64]={	0x01,0x00,0x5e,0x00,0x00,0x01,		/*destination mac*/
									0x00,0x00,0x00,0x00,0x00,0x00,		/*offset:6*/
									0x08,0x00,						/*offset:12*/
									0x46,0x00,0x00,0x24,				/*offset:14*/
									0x00,0x00,0x40,0x00,				/*offset:18*/
									0x01,0x02,0x00,0x00,				/*offset:22*/
									0x00,0x00,0x00,0x00,				/*offset:26,source ip*/
									0xe0,0x00,0x00,0x01,				/*offset:30,destination ip*/
									0x94,0x04,0x00,0x00,				/*offset:34,router alert option*/
									0x11,0x01,0x00,0x00,				/*offset:38*/
									0x00,0x00,0x00,0x00,				/*offset:42,queried multicast ip address*/
									0x0a,0x3c,0x00,0x00,				/*offset:46*/
									0x00,0x00,0x00,0x00,				/*offset:50*/
									0x00,0x00,0x00,0x00,				/*offset:54*/
									0x00,0x00,0x00,0x00,				/*offset:58*/
									0x00,0x00							/*offset:62*/
									
								};			



/*igmpv2 general query*/
static unsigned char igmpV2QueryBuf[64]={	0x01,0x00,0x5e,0x00,0x00,0x01,		/*destination mac*/
									0x00,0x00,0x00,0x00,0x00,0x00,		/*offset:6*/
									0x08,0x00,						/*offset:12*/
									0x45,0x00,0x00,0x1c,				/*offset:14*/
									0x00,0x00,0x40,0x00,				/*offset:18*/
									0x01,0x02,0x00,0x00,				/*offset:22*/
									0x00,0x00,0x00,0x00,				/*offset:26*/
									0xe0,0x00,0x00,0x01,				/*offset:30*/
									0x11,0x01,0x0c,0xfa,				/*offset:34*/
									0x00,0x00,0x00,0x00,				/*offset:38*/
									0x00,0x00,0x00,0x00,				/*offset:42*/
									0x00,0x00,0x00,0x00,				/*offset:46*/
									0x00,0x00,0x00,0x00,				/*offset:50*/
									0x00,0x00,0x00,0x00,				/*offset:54*/
									0x00,0x00,0x00,0x00,				/*offset:58*/
									0x00,0x00							/*offset:62*/
									
								};	

static unsigned char mldQueryBuf[90]={	0x33,0x33,0x00,0x00,0x00,0x01,		/*destination mac*/
									0x00,0x00,0x00,0x00,0x00,0x00,		/*source mac*/	/*offset:6*/
									0x86,0xdd,						/*ether type*/	/*offset:12*/
									0x60,0x00,0x00,0x00,				/*version(1 byte)-traffic cliass(1 byte)- flow label(2 bytes)*/	/*offset:14*/
									0x00,0x20,0x00,0x01,				/*payload length(2 bytes)-next header(1 byte)-hop limit(value:1 1byte)*//*offset:18*/
									0xfe,0x80,0x00,0x00,				/*source address*/	/*offset:22*/
									0x00,0x00,0x00,0x00,				/*be zero*/	/*offset:26*/
									0x00,0x00,0x00,					/*upper 3 bytes mac address |0x02*/ /*offset:30*/
									0xff,0xfe,						/*fixed*/
									0x00,0x00,0x00,					/*lowert 3 bytes mac address*/	 /*offset:35*/
									0xff,0x02,0x00,0x00,				/*destination address is fixed as FF02::1*/	/*offset:38*/
									0x00,0x00,0x00,0x00,			
									0x00,0x00,0x00,0x00,			
									0x00,0x00,0x00,0x01,			
									0x3a,0x00,						/*icmp type(1 byte)-length(1 byte)*/	 /*offset:54*/
									0x05,0x02,0x00,0x00,				/*router alert option*/
									0x01,0x00,						/*padN*/
									0x82,0x00,						/*type(query:0x82)-code(0)*/	/*offset:62*/
									0x00,0x00,						/*checksum*/	/*offset:64*/
									0x00,0x0a,						/*maximum reponse code*/
									0x00,0x00,						/*reserved*/
									0x00,0x00,0x00,0x00,				/*multicast address,fixed as 0*/
									0x00,0x00,0x00,0x00,			
									0x00,0x00,0x00,0x00,			
									0x00,0x00,0x00,0x00,
									0x0a,0x3c,0x00,0x00
								};

static unsigned char ipv6PseudoHdrBuf[40]=	{	
									0xfe,0x80,0x00,0x00,				/*source address*/
									0x00,0x00,0x00,0x00,			
									0x00,0x00,0x00,0xff,			
									0xfe,0x00,0x00,0x00,			 	
									0xff,0x02,0x00,0x00,				/*destination address*/
									0x00,0x00,0x00,0x00,		
									0x00,0x00,0x00,0x00,			
									0x00,0x00,0x00,0x01,				
									0x00,0x00,0x00,0x18,				/*upper layer packet length*/
									0x00,0x00,0x00,0x3a					/*zero padding(3 bytes)-next header(1 byte)*/
									};		

static unsigned short rtk_ipv6Checksum(unsigned char *pktBuf, unsigned int pktLen, unsigned char  *ipv6PseudoHdrBuf)
{
	unsigned int  checksum=0;
	unsigned int count=pktLen;
	unsigned short   *ptr;

	/*compute ipv6 pseudo-header checksum*/
	ptr= (unsigned short  *) (ipv6PseudoHdrBuf);	
	for(count=0; count<20; count++) /*the pseudo header is 40 bytes long*/
	{
		checksum+= ntohs(*ptr);
		ptr++;
	}
	
	/*compute the checksum of mld buffer*/
	count=pktLen;
	ptr=(unsigned short  *) (pktBuf);	
	while(count>1)
	{
		checksum+= ntohs(*ptr);
		ptr++;
		count -= 2;
	}
	 
	if(count>0)
	{
		checksum+= *(pktBuf+pktLen-1)<<8; /*the last odd byte is treated as bit 15~8 of unsigned short*/
	}

	/* Roll over carry bits */
	checksum = (checksum >> 16) + (checksum & 0xffff);
	checksum += (checksum >> 16);

	/* Return checksum */
	return ((uint16) ~ checksum);
}


static unsigned short  rtk_ipv4Checksum(unsigned char *pktBuf, unsigned int pktLen)
{
	/*note: the first bytes of  packetBuf should be two bytes aligned*/
	unsigned int  checksum=0;
	unsigned int  count=pktLen;
	unsigned short   *ptr= (unsigned short *)pktBuf;	
	
	while(count>1)
	{
		checksum+= ntohs(*ptr);
		ptr++;
		count -= 2;
	}
	 
	if(count>0)
	{
		checksum+= *(pktBuf+pktLen-1)<<8; /*the last odd byte is treated as bit 15~8 of unsigned short*/
	}

	/* Roll over carry bits */
	checksum = (checksum >> 16) + (checksum & 0xffff);
	checksum += (checksum >> 16);

	/* Return checksum */
	return ((unsigned short) ~ checksum);

}


static unsigned char* rtk_generateIgmpQuery(struct net_bridge * br)
{
	struct net_device* brDev = NULL;
	unsigned short checkSum=0;
	struct in_device *in_dev;	
	struct net_device *landev;
	struct in_ifaddr *ifap = NULL;
	
      
	if(br==NULL)
	{
		return NULL;
	}
	
	brDev = br->dev;
	landev = brDev;
	if(igmpVersion==3)
	{
		memcpy(&igmpV3QueryBuf[6],brDev->dev_addr,6);			/*set source mac address*/
	}
	else
	{
		memcpy(&igmpV2QueryBuf[6],brDev->dev_addr,6);			/*set source mac address*/
	}
	
	/*set source ip address*/


	if (landev ){
		in_dev=(struct in_device*)(landev->ip_ptr);
		if(in_dev != NULL) {
			for (ifap=in_dev->ifa_list; ifap != NULL; ifap=ifap->ifa_next) {
				if (strcmp(br->dev->name, ifap->ifa_label) == 0){
					//printk("src ip:0x%x, [%s:%d]\n", ifap->ifa_address, __FUNCTION__, __LINE__);
					if(igmpVersion==3)
					{
						memcpy(&igmpV3QueryBuf[26],&ifap->ifa_address,4);
					}
					else
					{
						memcpy(&igmpV2QueryBuf[26],&ifap->ifa_address,4);
					}
				}
			}
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		return NULL;
	}


	if(igmpVersion==3)
	{
		igmpV3QueryBuf[24]=0;
		igmpV3QueryBuf[25]=0;
	}
	else
	{
		igmpV2QueryBuf[24]=0;
		igmpV2QueryBuf[25]=0;
	}
	
	if(igmpVersion==3)
	{
		checkSum=rtk_ipv4Checksum(&igmpV3QueryBuf[14],24);
	}
	else
	{
		checkSum=rtk_ipv4Checksum(&igmpV2QueryBuf[14],20);
	}

	if(igmpVersion==3)
	{
		igmpV3QueryBuf[24]=(checkSum&0xff00)>>8;
		igmpV3QueryBuf[25]=(checkSum&0x00ff);
	}
	else
	{
		igmpV2QueryBuf[24]=(checkSum&0xff00)>>8;
		igmpV2QueryBuf[25]=(checkSum&0x00ff);
	}
	

	if(igmpVersion==3)
	{
		igmpV3QueryBuf[40]=0;
		igmpV3QueryBuf[41]=0;
		checkSum=rtk_ipv4Checksum(&igmpV3QueryBuf[38],12);
		igmpV3QueryBuf[40]=(checkSum&0xff00)>>8;
		igmpV3QueryBuf[41]=(checkSum&0x00ff);
	}
	else
	{
		igmpV2QueryBuf[36]=0;
		igmpV2QueryBuf[37]=0;
		checkSum=rtk_ipv4Checksum(&igmpV2QueryBuf[34],8);
		igmpV2QueryBuf[36]=(checkSum&0xff00)>>8;
		igmpV2QueryBuf[37]=(checkSum&0x00ff);
	}

	if(igmpVersion==3)
	{
		return igmpV3QueryBuf;
	}
	else
	{
		return igmpV2QueryBuf;
	}
	
	return NULL;
}


void rtk_igmpsendQuery(struct net_bridge *br)
{
	unsigned char *igmpBuf=NULL;
	struct sk_buff *skb;
	struct sk_buff *skb2;
	struct net_bridge_port *p, *n;
	struct net_bridge_port *prev;
	unsigned int fwdCnt=0;

	skb=dev_alloc_skb(1024);
	if(skb==NULL)
	{
		return;
	}

	memset(skb->data,0,64);
	igmpBuf=rtk_generateIgmpQuery(br);
	if(igmpBuf==NULL)
	{
		return;
	}

	memcpy(skb->data,igmpBuf,64);

	skb->len = 0;
	if(igmpVersion==3)
	{
		skb_put(skb, 50);
	}
	else
	{
		skb_put(skb, 42);
	}
	
	skb->dev=br->dev;
	
	prev = NULL;
	fwdCnt=0;
	list_for_each_entry_safe(p, n, &br->port_list, list) 
	{ 
		if (p->state == BR_STATE_FORWARDING) 
		{
			if (prev != NULL) 
			{   
				if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) 
				{
					br->dev->stats.tx_dropped++;
					kfree_skb(skb);
					return;
				} 
				skb2->dev=prev->dev;
#if defined(CONFIG_COMPAT_NET_DEV_OPS)
				prev->dev->hard_start_xmit(skb2, prev->dev);
#else
				prev->dev->netdev_ops->ndo_start_xmit(skb2,prev->dev);
#endif					
				fwdCnt++;
			}
				                                                                               
			prev = p;
		}
	}

	if (prev != NULL) 
	{
		skb->dev=prev->dev;
#if defined(CONFIG_COMPAT_NET_DEV_OPS)
		prev->dev->hard_start_xmit(skb, prev->dev);
#else
		prev->dev->netdev_ops->ndo_start_xmit(skb,prev->dev);
#endif							  
		fwdCnt++;
	}

	if(fwdCnt==0)
	{
		/*to avoid memory leak*/
		kfree_skb(skb);
	}
	return;

}

static unsigned char* rtk_generateMldQuery(struct net_bridge * br)
{
	struct net_device* brDev = NULL;
	unsigned short checkSum=0;
	if(br==NULL)
	{
		return NULL;
	}
	
	brDev = br->dev;
	
	memcpy(&mldQueryBuf[6],brDev->dev_addr,6);			/*set source mac address*/
	
	memcpy(&mldQueryBuf[30],brDev->dev_addr,3);		/*set  mld query packet source ip address*/
	mldQueryBuf[30]=mldQueryBuf[30]|0x02;		
	memcpy(&mldQueryBuf[35],&brDev->dev_addr[3],3);		

	memcpy(ipv6PseudoHdrBuf,&mldQueryBuf[22],16);			/*set pseudo-header source ip*/
	if(mldVersion==2)
	{
		mldQueryBuf[19]=	0x24;
	}
	else
	{
		mldQueryBuf[19]=	0x20;
	}

	mldQueryBuf[64]=0;/*reset checksum*/
	mldQueryBuf[65]=0;
	if(mldVersion==2)
	{
		ipv6PseudoHdrBuf[35]=28;
		checkSum=rtk_ipv6Checksum(&mldQueryBuf[62],28,ipv6PseudoHdrBuf);
	}
	else
	{
		ipv6PseudoHdrBuf[35]=24;
		checkSum=rtk_ipv6Checksum(&mldQueryBuf[62],24,ipv6PseudoHdrBuf);
	}
	
	mldQueryBuf[64]=(checkSum&0xff00)>>8;
	mldQueryBuf[65]=(checkSum&0x00ff);
	
	return mldQueryBuf;
	
}


void rtk_mldsendQuery(struct net_bridge *br)
{
	struct sk_buff *skb;
	struct sk_buff *skb2;
	struct net_bridge_port *p, *n;
	struct net_bridge_port *prev;
	unsigned int fwdCnt=0;
	unsigned char *mldBuf=NULL;
	

	skb=dev_alloc_skb(1024);
	if(skb==NULL)
	{
		return;
	}
	
	memset(skb->data,0,86);
	mldBuf=rtk_generateMldQuery(br);
	if(mldBuf==NULL)
	{
		return;
	}
	
	if(mldVersion==2)
	{
		memcpy(skb->data,mldBuf,90);
		skb->len = 0;
		skb_put(skb, 90);
	}
	else
	{
		memcpy(skb->data,mldBuf,86);
		skb->len = 0;
		skb_put(skb, 86);
	}
	
	skb->dev=br->dev;
	
	prev = NULL;
	fwdCnt=0;
	list_for_each_entry_safe(p, n, &br->port_list, list) 
	{ 
		if(p->state == BR_STATE_FORWARDING)  
		{
			if (prev != NULL) 
			{																						
				if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) 
				{
					br->dev->stats.tx_dropped++;
					kfree_skb(skb);
					return;
				} 
				skb2->dev=prev->dev;
			#if defined(CONFIG_COMPAT_NET_DEV_OPS)
				prev->dev->hard_start_xmit(skb2, prev->dev);
			#else
				prev->dev->netdev_ops->ndo_start_xmit(skb2,prev->dev);
			#endif                  
				fwdCnt++;
			}
																							   
			prev = p;
		}
	}

	if (prev != NULL) 
	{
		skb->dev=prev->dev;
		
    #if defined(CONFIG_COMPAT_NET_DEV_OPS)
		prev->dev->hard_start_xmit(skb, prev->dev);
	#else
		prev->dev->netdev_ops->ndo_start_xmit(skb,prev->dev);
	#endif
		fwdCnt++;
	}

	if(fwdCnt==0)
	{
		/*to avoid memory leak*/
		kfree_skb(skb);
	}
	
	return;
}


struct net_bridge *rtk_igmp_getBrDeviceByModuleIndex(int32 moduleIndex)
{
	struct net_bridge *br =NULL;
	
	if(rtk_igmp_mCastModuleArray[moduleIndex].validBit)
	{
		if(rtk_igmp_mCastModuleArray[moduleIndex].brDevInfo.dev)
		{
			br = netdev_priv(rtk_igmp_mCastModuleArray[moduleIndex].brDevInfo.dev);
			if(br != NULL)
				return br;
			else
				return NULL;
		}
		else
			return NULL;
	}

	return NULL;
}

#endif




