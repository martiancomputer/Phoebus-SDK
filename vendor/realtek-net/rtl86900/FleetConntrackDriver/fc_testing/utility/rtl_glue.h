#ifndef _RTL_GLUE_
#define _RTL_GLUE_


//#include <linux/kernel.h>
#include <common/type.h>
#include <uapi/linux/types.h>
//#include <linux/byteorder/little_endian.h>
//#include <uapi/linux/byteorder/little_endian.h>
//#include <linux/byteorder/generic.h>
#include <rtl_types.h>
#include <common/rt_type.h>
#include <linux/string.h>
#include <linux/printk.h>


#define COLOR_NM "\033[0m"
#define COLOR_Y "\033[1;33m"
#define COLOR_H "\033[1;37;41m"
#define COLOR_B "\033[1;34m"
#define COLOR_G "\033[1;32m"
#define COLOR_R "\033[1;31m"
#define COLOR_M "\033[1;35m"
#define COLOR_C "\033[1;36m"


typedef enum rtk_test_debug_level_e
{
	RTK_TEST_DEBUG_LEVEL_DEBUG=0x1,
	RTK_TEST_DEBUG_LEVEL_FIXME=0x2,
	RTK_TEST_DEBUG_LEVEL_CALLBACK=0x4,
	RTK_TEST_DEBUG_LEVEL_TRACE=0x8,
	RTK_TEST_DEBUG_LEVEL_ACL=0x10,
	RTK_TEST_DEBUG_LEVEL_WARN=0x20,
	RTK_TEST_DEBUG_LEVEL_TRACE_DUMP=0x40,
	RTK_TEST_DEBUG_LEVEL_WMUX=0x80,
	RTK_TEST_DEBUG_LEVEL_MACLERAN=0x100,
	RTK_TEST_DEBUG_LEVEL_TABLE=0x200,
	RTK_TEST_DEBUG_LEVEL_ALG=0x400,
	RTK_TEST_DEBUG_LEVEL_IGMP=0x800,
	RTK_TEST_DEBUG_LEVEL_ACL_RRESERVED=0x1000,
	RTK_TEST_DEBUG_LEVEL_RG_API=0x2000,
	RTK_TEST_DEBUG_LEVEL_FLOW=0x4000,
	RTK_TEST_DEBUG_LEVEL_ASIC=0x80000000,
	RTK_TEST_DEBUG_LEVEL_ALL=0xffffffff,
} rtk_test_debug_level_t;



// free and NULL pointer to prevent double free
#define rtlglue_free(APTR) rtlglue_freeAndNull((void*)&APTR)


void *rtlglue_malloc(uint32 NBYTES);
void rtlglue_freeAndNull(void **APTR);
//void rtlglue_free(void *APTR);
#define rtlglue_printf printk


#ifndef bzero
#define bzero( p, s ) memset( p, 0, s )
#endif

#ifndef ASSERT64_EQ
#define ASSERT64_EQ(value1,value2)\
do {\
		int64 v1;\
		int64 v2;\
		v1=value1;\
		v2=value2;\
        if ((v1) != (v2)) {\
                rtlglue_printf("\033[31;43m%s(%d): value1=%#llX value2=%#llX, fail, so abort!\033[m\n", __FUNCTION__, __LINE__,(v1),(v2));\
                return FAILED; \
        }\
}while (0)
#endif

#ifndef ASSERT_EQ_JUMP
#define ASSERT_EQ_JUMP(v1,v2)\
do {\
	int value1, value2; \
	value1 = (v1); \
	value2 = (v2); \
	if ((value1) != (value2)) {\
		rtlglue_printf("\033[31;43m%s(%d): value1=0x%x value2=0x%x, fail, so abort!\033[m\n", __FUNCTION__, __LINE__,(value1),(value2));\
		ret = value1; \
		goto TERMINATE_TEST; \
	}\
}while (0)
#endif

#ifndef ASSERT_EQ
#define ASSERT_EQ(value1,value2)\
do {\
		int v1;\
		int v2;\
		v1=value1;\
		v2=value2;\
        if ((v1) != (v2)) {\
                rtlglue_printf("\033[31;43m%s(%d): value1=0x%x value2=0x%x, fail, so abort!\033[m\n", __FUNCTION__, __LINE__,(v1),(v2));\
                return FAILED; \
        }\
}while (0)
#endif
#ifndef ASSERT_EQ_WITH_MSG
#define ASSERT_EQ_WITH_MSG(result,expected,msg)\
do {\
	int value1, value2; \
	value1 = (result); \
	value2 = (expected); \
	if ((value1) != (value2)) {\
		rtlglue_printf("\033[31;43m%s(%d) %s: result=0x%x expected=0x%x, fail, so abort!\033[m\n", __FUNCTION__, __LINE__,msg,(value1),(value2));\
              return FAILED; \
	}\
}while (0)
#endif

#ifndef ASSERT
#define ASSERT(value)\
do {\
	if (!((int32)(value))) {\
		rtlglue_printf("\t%s(%d): Fail, so abort!\n", __FUNCTION__, __LINE__);\
		return FAILED; \
	}\
}while (0)
#endif

#ifndef ASSERT_RT
#define ASSERT_RT(val,ret)\
do {\
	int v1 = val;\
	int r1 = ret;\
	if (!((int32)(v1))) {\
		rtlglue_printf("\033[31;43m%s(%d): val=0x%x ret=0x%x, fail, so abort!\033[m\n", __FUNCTION__, __LINE__,(v1),(r1));\
		return r1; \
	}\
}while (0)
#endif

#ifndef ASSERT_EQ_TERM
#define ASSERT_EQ_TERM(v1,v2)\
do {\
	int value1, value2; \
	value1 = (v1); \
	value2 = (v2); \
	if ((value1) != (value2)) {\
		rtlglue_printf("\033[31;43m%s(%d): value1=0x%x value2=0x%x, fail, so abort!\033[m\n", __FUNCTION__, __LINE__,(value1),(value2));\
		if(value1==0)\
			ret = value2;\
		else\
			ret = value1; \
		goto TERMINATE_TEST; \
	}\
}while (0)
#endif

#ifndef ASSERT_EQ_TERM_WITH_MSG
#define ASSERT_EQ_TERM_WITH_MSG(result,expected,msg)\
do {\
	int value1, value2; \
	value1 = (result); \
	value2 = (expected); \
	if ((value1) != (value2)) {\
		rtlglue_printf("\033[31;43m%s(%d) %s: result=0x%x expected=0x%x, fail, so abort!\033[m\n", __FUNCTION__, __LINE__,msg,(value1),(value2));\
		ret = FAILED; \
		goto TERMINATE_TEST; \
	}\
}while (0)
#endif



#ifndef ASSERT_EQ_CONT
#define ASSERT_EQ_CONT(v1,v2)\
do {\
	int value1, value2; \
	value1 = (v1); \
	value2 = (v2); \
	if ((value1) != (value2)) {\
		rtlglue_printf("\033[31;43m%s(%d): value1=0x%x value2=0x%x, fail, so abort!\033[m\n", __FUNCTION__, __LINE__,(value1),(value2));\
		if(value1==0)\
			ret = value2;\
		else\
			ret = value1; \
	}\
}while (0)
#endif


#ifdef TEST_DEBUG
#undef TEST_DEBUG
#endif
#ifndef TEST_DEBUG
#define TEST_DEBUG( comment ,arg...) do { if(test_debug_level&RTK_TEST_DEBUG_LEVEL_DEBUG) common_dump_fctest(RTK_TEST_DEBUG_LEVEL_DEBUG,"TEST_DEBUG",33,40,__FUNCTION__,__LINE__,comment,##arg); }while(0)
#endif

#ifndef TEST_FIXME
#define TEST_FIXME( comment ,arg...) do { if(test_debug_level&RTK_TEST_DEBUG_LEVEL_FIXME) common_dump_fctest(RTK_TEST_DEBUG_LEVEL_FIXME,"TEST_FIXME",34,40,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef TEST_CBACK
#define TEST_CBACK( comment ,arg...) do { if(test_debug_level&RTK_TEST_DEBUG_LEVEL_CALLBACK) common_dump_fctest(RTK_TEST_DEBUG_LEVEL_CALLBACK,"TEST_CALLBACK",34,40,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef TEST_TRACE
#define TEST_TRACE( comment ,arg...)	do { if(test_debug_level&RTK_TEST_DEBUG_LEVEL_TRACE) common_dump_fctest(RTK_TEST_DEBUG_LEVEL_TRACE,"TEST_TRACE",35,40,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef TEST_ACL
#define TEST_ACL( comment ,arg...) do { if(test_debug_level&RTK_TEST_DEBUG_LEVEL_ACL) common_dump_fctest(RTK_TEST_DEBUG_LEVEL_ACL,"TEST_ACL",36,40,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef TEST_ACL_RSV
#define TEST_ACL_RSV( comment ,arg...) do { if(test_debug_level&RTK_TEST_DEBUG_LEVEL_ACL_RRESERVED) common_dump_fctest(RTK_TEST_DEBUG_LEVEL_ACL_RRESERVED,"ACL_RSV  ",36,40,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif


#ifndef TEST_WARNING
#define TEST_WARNING( comment ,arg...) do { if(test_debug_level&RTK_TEST_DEBUG_LEVEL_WARN) common_dump_fctest(RTK_TEST_DEBUG_LEVEL_WARN,"TEST_WARNING",33,41,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef TEST_WMUX
#define TEST_WMUX( comment ,arg...) do { if(test_debug_level&RTK_TEST_DEBUG_LEVEL_WMUX) common_dump_fctest(RTK_TEST_DEBUG_LEVEL_WMUX,"TEST_WMUX",32,40,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef TEST_MACLN
#define TEST_MACLN( comment ,arg...) do { if(test_debug_level&RTK_TEST_DEBUG_LEVEL_MACLERAN) common_dump_fctest(RTK_TEST_DEBUG_LEVEL_MACLERAN,"TEST_MACLEARN",31,40,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef TEST_TABLE
#define TEST_TABLE( comment ,arg...) do { if(test_debug_level&RTK_TEST_DEBUG_LEVEL_TABLE) common_dump_fctest(RTK_TEST_DEBUG_LEVEL_TABLE,"TEST_TABLE",31,40,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef TEST_ALG
#define TEST_ALG( comment ,arg...) do { if(test_debug_level&RTK_TEST_DEBUG_LEVEL_ALG) common_dump_fctest(RTK_TEST_DEBUG_LEVEL_ALG,"TEST_ALG",31,40,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef TEST_IGMP
#define TEST_IGMP( comment ,arg...) do { if(test_debug_level&RTK_TEST_DEBUG_LEVEL_IGMP) common_dump_fctest(RTK_TEST_DEBUG_LEVEL_IGMP,"TEST_IGMP",31,40,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef TEST_FLOW
#define TEST_FLOW( comment ,arg...) do { if(test_debug_level&RTK_TEST_DEBUG_LEVEL_FLOW) common_dump_fctest(RTK_TEST_DEBUG_LEVEL_FLOW,"TEST_FLOW",32,40,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef TEST_ASIC
#define TEST_ASIC( comment ,arg...) do { if(test_debug_level&RTK_TEST_DEBUG_LEVEL_ASIC) common_dump_fctest(RTK_TEST_DEBUG_LEVEL_ASIC,"TEST_ASIC",94,40,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

extern rtk_test_debug_level_t test_debug_level;

void common_dump_fctest(rtk_test_debug_level_t level, char *string, int color, int bgcolor,const char *funcs,int line, char *comment,...);


#endif /*endif _RTL_GLUE_*/
