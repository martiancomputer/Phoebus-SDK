#ifndef _RTK_IGMP_NEC_SNOOPING_H
#define _RTK_IGMP_NEC_SNOOPING_H


#include <rtk_igmp_struct.h>

int rtk_igmp_set_wan_learn(int32 value);
int rtk_igmp_wanLearn_proc_write(struct file *file, char *buffer, unsigned long count, void *data);
int rtk_igmp_wanLearn_proc_read(struct seq_file *s, void *v);

#if defined(CONFIG_RTL_LAN_COMPATIBILITY) || defined(CONFIG_RTL_NEW_IGMP_REPORT_BEHAVIOR)
void rtk_igmp_SnoopingconfigInit(void);
#endif

#if defined(CONFIG_RTL_NEW_IGMP_REPORT_BEHAVIOR)
uint32 rtk_igmp_processQueries(rtk_igmp_pktHdr_t *pPkthdr, const struct net_device *SrcDev);
#endif

//proc function 
#if defined(CONFIG_RTL_NEW_IGMP_REPORT_BEHAVIOR)|| defined(CONFIG_RTL_LAN_COMPATIBILITY) || defined (CONFIG_RTL_PER_PORT_CLIENT_NUM)
int rtk_igmp_igmpSnoopingConfig_proc_write(struct file *file, char *buffer, unsigned long count, void *data);
int rtk_igmp_igmpSnoopingConfig_proc_read(struct seq_file *s, void *v);
int rtk_igmp_igmp6SnoopingConfig_proc_write(struct file *file, char *buffer, unsigned long count, void *data);
int rtk_igmp_igmp6SnoopingConfig_proc_read(struct seq_file *s, void *v);
#endif
#if defined(CONFIG_RTL_LAN_COMPATIBILITY)
int rtk_igmp_igmpCompatiblity_proc_read(struct seq_file *s, void *v);
int rtk_igmp_igmpCompatiblity_proc_write(struct file *file, char *buffer, unsigned long count, void *data);
#endif
#if defined(CONFIG_RTL_PER_PORT_CLIENT_NUM)
int rtk_igmp_brIgmpGeneralQueryVersion_proc_write(struct file *file,char *buffer, unsigned long count, void *data);
int rtk_igmp_brIgmpGeneralQueryVersion_proc_read(struct seq_file *s, void *v);
int rtk_igmp_brMldGeneralQueryVersion_proc_write(struct file *file,char *buffer, unsigned long count, void *data);
int rtk_igmp_brMldGeneralQueryVersion_proc_read(struct seq_file *s, void *v);
#endif

#if defined(CONFIG_RTL_NEW_IGMP_REPORT_BEHAVIOR)|| defined(CONFIG_RTL_LAN_COMPATIBILITY) || defined (CONFIG_RTL_PER_PORT_CLIENT_NUM)
#define DEFAULT_ROBUSTNESS	2
#define DEFAULT_QUERY_INTERVAL	125
#define DEFAULT_QUERY_RESPONSE_INTERVAL 10

#define GATEWAY_MODE			0
#define BRIDGE_MODE				1
#define DEFAULT_LAST_MEMBER_INTERVAL  1             	/* IGMP last member query time, default is 1 seconds */
extern int rtk_get_op_mode(void);
#endif

#if defined(CONFIG_RTL_LAN_COMPATIBILITY)
#define COMPATIBILITY_IGMPV3	0
#define COMPATIBILITY_IGMPV2	1
#define COMPATIBILITY_IGMPV1	2
void rtl_get_compatiblity_staus(int* status, uint32* timeout);
#endif

#if defined(CONFIG_RTL_PER_PORT_CLIENT_NUM)
#define DEFAULT_PER_PORT_MAX_NUM 64
#define NO_CLIENT_SPACE_MASK 0xFFFFFFFF
#define NO_CLIENT_SPACE_FLAG 0x01	//skb flag
#define FLAG_WAIT_STATUS 0x01		//igmpDb client entry flag
#define CLIENT_NUM_ID(ipVersion) ((ipVersion==IP_VERSION4)?0:1)

int rtk_igmp_brIgmpGeneralQueryVersion_proc_write(struct file *file,char *buffer, unsigned long count, void *data);
int rtk_igmp_brIgmpGeneralQueryVersion_proc_read(struct seq_file *s, void *v);
int rtk_igmp_brMldGeneralQueryVersion_proc_write(struct file *file,char *buffer, unsigned long count, void *data);
int rtk_igmp_brMldGeneralQueryVersion_proc_read(struct seq_file *s, void *v);
#endif

#if defined(CONFIG_RTL_PER_PORT_CLIENT_NUM)
int rtk_igmp_enterWaitStatus(int32 moduleIndex, uint32 ipVersion);
int rtk_igmp_recoverClientSpace(uint32 moduleIndex, uint32 ipVersion, uint32 inIfidx);
int rtk_igmp_checkClientEmptySpace(uint32 moduleIndex, uint32 ipVersion, uint32 inIfidx, uint32 *groupAddress, uint32 *clientAddress);
#endif



/* multicast configuration*/
struct rtl_mCastSnoopingGlobalConfig
{
#if defined(CONFIG_RTL_NEW_IGMP_REPORT_BEHAVIOR)	
	uint32 groupMemberAgingTime[2];                
	uint32 lastMemberAgingTime[2];			
	uint32 querierPresentInterval[2];                   
#else
	uint32 groupMemberAgingTime;                
	uint32 lastMemberAgingTime;			
	uint32 querierPresentInterval;                   
#endif	
#if defined(CONFIG_RTL_NEW_IGMP_REPORT_BEHAVIOR)
	char robustness[2];
	uint32 queryInterval[2];
	uint32 queryResponseInterval[2];
	uint32 lastMemberQueryInterval[2];
#endif
	uint8 igmpFastLeave;
	uint8 igmp6Fastleave;
	uint8 igmpCompatibilityEnable;
	uint8 igmp6CompatibilityEnable;
};


#endif

#if defined(CONFIG_RTL_NEW_IGMP_REPORT_BEHAVIOR)
/**********************IGMPv3 exponential field decoding ******************************/ 
#define	RTL_IGMPV3_MASK(value, nb)		((nb)>=32 ? (value) : ((1<<(nb))-1) & (value))
#define	RTL_IGMPV3_EXP(thresh, nbmant, nbexp, value) \
			((value) < (thresh) ? (value) : \
			((RTL_IGMPV3_MASK(value, nbmant) | (1<<(nbmant))) << \
			(RTL_IGMPV3_MASK((value) >> (nbmant), nbexp) + (nbexp))))		
			
#define	RTL_IGMPV3_QQIC(value)			RTL_IGMPV3_EXP(0x80, 4, 3, value)
#define	RTL_IGMPV3_MRC(value)			RTL_IGMPV3_EXP(0x80, 4, 3, value)
/********************************************************************************/


/**************************MLDv2 exponential field decoding ****************************/
#define   RTL_MLDV2_MASK(value, nb) 	((nb)>=32 ? (value) : ((1<<(nb))-1) & (value))
#define   RTL_MLDV2_EXP(thresh, nbmant, nbexp, value)\
	              ((value) < (thresh) ? (value) : \
			((RTL_MLDV2_MASK(value, nbmant) | (1<<(nbmant))) << \
			(RTL_MLDV2_MASK((value) >> (nbmant), nbexp) + (nbexp))))
			
#define   RTL_MLDV2_QQIC(value)			RTL_MLDV2_EXP(0x80, 4, 3, value)
#define   RTL_MLDV2_MRC(value)			RTL_MLDV2_EXP(0x8000, 12, 3, value)
/*------------------------------------------------------------------------------*/
#endif

