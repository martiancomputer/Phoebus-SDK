#ifndef _TEST_DEFINE_
#define _TEST_DEFINE_

#include "l34testmodule.h"


#define BUF_SIZE 1600
extern unsigned char igrPayload[BUF_SIZE];
extern unsigned char egrPayload[BUF_SIZE];

#define SESSION_ID_11	0x11
#define SESSION_ID_22	0x11

#define WAN_CVLAN_0		1000
#define WAN_CPRI_0		2
#define WAN_CVLAN_1		1001
#define WAN_CPRI_1		3
#define WAN_CVLAN_2		1002
#define WAN_CPRI_2		4

#define WAN_SVLAN_0		2000
#define WAN_SPRI_0		1
#define WAN_SVLAN_1		2001
#define WAN_SPRI_1		2
#define WAN_SVLAN_2		2002
#define WAN_SPRI_2		3

#define LAN_CVLAN_0		3000
#define LAN_CPRI_0		5
#define LAN_CVLAN_1		3001
#define LAN_CPRI_1		6
#define LAN_CVLAN_2		3002
#define LAN_CPRI_2		7

#define WAN_NAS0_0_MAC "00:e0:4c:86:70:02"

#define DSLITE_LOCAL_B4v6IP "2001:0db8:85a3:08d3:1319:8a2e:0001:b4b4"


#define WAN_B4v6IP_1 "2001:0db8:85a3:08d3:1319:8a2e:0002:b4b4"


#define REMOTE_HOST_IP4_3	"8.8.8.8"
#define REMOTE_HOST_IP6_3	"2001:3388:0808:0808:0001:2345:1010:1010"
#define REMOTE_HOST_MAC_3	"00-03-10-10-10-10"

#define DSLITE_REMOTE_AFTR_V6IP			"2001:0db8:85a3:08d3:1319:8a2e:0000:afaf"
#define AFTR_V4IP			"10.0.175.175"
#define AFTR_V6IPMIP 		"2001:0db8:85a3:08d3:1319:8a2e:c0a8:0203" 
#define BAD_AFTR_V6IPMIP	"2001:0db8:85a3:08d3:1319:8a2e:ffff:ffff" 

#define REMOTE_HOST_IP4_0	"210.254.254.254"
#define REMOTE_HOST_IP6_0	"2001:0db8:85a3:08d3:1319:8a2e:1010:1010"
#define REMOTE_HOST_MAC_0	"00-00-10-10-10-10"

#define REMOTE_HOST_IP4_1	"10.10.10.11"
#define REMOTE_HOST_IP6_1	"2001:0db8:85a3:08d3:1319:8a2e:1010:1011"
#define REMOTE_HOST_MAC_1	"00-00-10-10-10-11"

#define LAN0_GATEWAY_IP0	"192.168.1.1"
#define LAN_GW_MAC_0		"00:E0:4C:86:70:01"
#define LAN0_GATEWAY_IP1	"192.168.10.1"
#define LAN_GW_MAC_1		"00-aa-aa-aa-a1-00"
#define LAN0_GATEWAY_IP2	"192.168.2.1"
#define LAN_GW_MAC_2		"00-aa-aa-aa-a2-00"


#define LAN_HOST_IP4_0		"192.168.1.100"
#define LAN_HOST_IP6_0		"2001:0db8:85a3:08d3:0:0:0:0064"
#define LAN_HOST_IP6_1		"2007:0000:1111:2222:0:0:0:8888"
#define LAN_HOST_MAC_0 		"12-34-56-78-9a-bc"
#define LAN_HOST_IP4_1		"192.168.250.101"
#define LAN_HOST_MAC_1	 	"00-88-88-88-01-01"
#define LAN_HOST_IP4_2		"192.168.250.102"
#define LAN_HOST_MAC_2 		"00-88-88-88-01-02"


int dslite_upstream(rtk_fcTesting_case_t *info);
int dslite_downstream_dualInLan(rtk_fcTesting_case_t *info);
int dslite_upstream_dualInLan(rtk_fcTesting_case_t *info);
int gre_downstream_dualInLan(rtk_fcTesting_case_t *info);
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
int wfo_callback_igmp_testing(rtk_fcTesting_case_t *info);
int wfo_callback_mcdata_testing(rtk_fcTesting_case_t *info);
#endif

#endif
