#ifdef __KERNEL__
#include <common/type.h>
#else
#include <rtl_types.h>
#endif


typedef struct MODEL_DUMP_REGIST_S
{	
	char* name;
	int32 (*fp)(void);
}MODEL_DUMP_REGIST_T;

#define MODEL_DUMP_TABLE(table,func) \
	{ \
		name: table, \
		fp: func, \
	}

void memDump (void *start, uint32 size, int8 * strHeader);
void DEBUG_PKT(uint8 *pkt,uint32 size,char *memo);

void test_dump_packet(uint8 *pkt,uint32 size,char *memo);
void dump_packet_header(uint8 *pkt,uint32 size,char *memo);
//void dump_compare_packet(uint8 *output,uint8 *expect,int out_size,int expect_size);


int32 dump_l34hs(void);
int32 dump_lut_table(void);
int32 dump_vlan_table(void);
int32 dump_netif(void);
int32 dump_camtag_table(void);
int32 dump_flowtag_table(void);
int32 dump_flow_table(void);
int32 dump_flowdram_table(void);
int32 dump_flowtrf_table(void);
#if defined(CONFIG_RG_APOLLOPRO_FEATURE_ENHANCE)
int32 dump_flow_mib(void);
#endif
int32 dump_macind_table(void);
int32 dump_ethtype_table(void);
int32 dump_extpmask_table(void);
#if defined(CONFIG_RG_RTL9607C_SERIES)
int32 dump_wanaccesslimit_table(void);
#endif
int32 dump_extratag_table(void);
int32 dump_port_isolation(void);
int32 dump_flow_p1Rawdata(int32 flowIdx, void *pFlowData);


//int dump_lut_table(void)

