#ifndef _RTL_TYPES_
#define _RTL_TYPES_



/* ==================
		Type definition
    ================== */

#ifndef uint64
#define  uint64  unsigned long long
#endif
#ifndef int64
#define int64 long long
#endif
#ifndef uint32
#define uint32 unsigned int
#endif
#ifndef int32
#define int32	int
#endif
#ifndef uint16
#define uint16 unsigned short
#endif
#ifndef int16
#define int16 short
#endif
#ifndef uint8
#define uint8 unsigned char
#endif
#ifndef int8
#define int8	char
#endif

#ifndef __KERNEL__
#ifndef u8
typedef uint8 u8;
#endif
#ifndef u16
typedef uint16 u16;
#endif
#ifndef u32
typedef uint32 u32;
#endif
#ifndef u64
typedef uint64 u64;
#endif
#endif //__KERNEL__

#ifndef ipaddr_t
#define ipaddr_t unsigned int
#endif

typedef unsigned long	memaddr;	
typedef struct {
    uint16      mac47_32;
    uint16      mac31_16;
    uint16      mac15_0;
} macaddr_t;

#define ETHER_ADDR_LEN				6
typedef struct ether_addr_s {
	uint8 octet[ETHER_ADDR_LEN];
} ether_addr_t;

typedef struct svlan_header_s {
#ifdef _LITTLE_ENDIAN
	uint16 spri:7;
	uint16 dei:1;
	uint16 svid:12;
#else
	uint16 svid:12;
	uint16 dei:1;
	uint16 spri:7;
#endif
} svlan_header_t;

typedef struct cvlan_header_s {
#ifdef _LITTLE_ENDIAN
	uint16 cpri:7;
	uint16 cfi:1;
	uint16 cvid:12;
#else
	uint16 cvid:12;
	uint16 cfi:1;
	uint16 cpri:7;
#endif
} cvlan_header_t;



typedef struct pppoe_header_s {
#ifdef _LITTLE_ENDIAN
	uint8 type:4;
	uint8 version:4;
#else
	uint8 version:4;
	uint8 type:4;
#endif
	uint8 code;
	uint16 sid;
	uint16 length;
} pppoe_header_t;

typedef struct ipv4_header_s {
#ifdef _LITTLE_ENDIAN
	uint8 header_length:4;
	uint8 version:4;
#else
	uint8 version:4;
	uint8 header_length:4;
#endif
	uint8 tos;
	uint16 total_length;
	uint16 id;
#ifdef _LITTLE_ENDIAN
	uint16 frag_off:13;
	uint16 frag:3;
#else
	uint16 frag:3;
	uint16 frag_off:13;
#endif
	uint8 ttl;
	uint8 protocal;
	uint16 checksum;
	uint32 sip;
	uint32 dip;
} ipv4_header_t;


#include <common/rt_type.h>

typedef struct ipv6_header_first_dword_s {
#ifdef _LITTLE_ENDIAN
		uint32 flow_label:20;
		uint32 traffic_class:8; //tos
		uint32 version:4;
#else
		uint32 version:4;
		uint32 traffic_class:8; //tos
		uint32 flow_label:20;
#endif
} ipv6_header_first_dword_t;
typedef struct ipv6_header_s {
	ipv6_header_first_dword_t first_dword;
	uint16 payload_length;
	uint8 next_header;
	uint8 hop_limit;
	rtk_ipv6_addr_t sip;
	rtk_ipv6_addr_t dip;
} ipv6_header_t;

typedef struct tcp_header_offset_s {
#ifdef _LITTLE_ENDIAN
	uint16 flags:6;
	uint16 reserve:6;
	uint16 offset:4;
#else
	uint16 offset:4;
	uint16 reserve:6;
	uint16 flags:6;
#endif
} tcp_header_offset_t;

typedef struct tcp_header_s {
	uint16 sport;
	uint16 dport; 
	uint32 seq_num;
	uint32 ack_num;
	tcp_header_offset_t offset;
	uint16 window;
	uint16 checksum;
	uint16 urgent;
} tcp_header_t;

typedef struct udp_header_s {
	uint16 sport;
	uint16 dport; 
	uint16 length; 
	uint16 checksum; 
} udp_header_t;

#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef SUCCESS
#define SUCCESS 0
#endif
#ifndef FAILED
#define FAILED -1
#endif
#ifndef FAIL
#define FAIL -1
#endif

#define NOPACKET -2

#if 0 //define in type.h
#ifndef ENABLED
#define ENABLED 1
#endif
#ifndef DISABLED
#define DISABLED 0
#endif
#endif

#ifndef CLEARBITS
#define CLEARBITS(a,b)	((a) &= ~(b))
#endif

#ifndef SETBITS
#define SETBITS(a,b)		((a) |= (b))
#endif

#ifndef ISSET
#define ISSET(a,b)		(((a) & (b))!=0)
#endif

#ifndef ISCLEARED
#define ISCLEARED(a,b)	(((a) & (b))==0)
#endif

//#ifndef max
//#define max(a,b)  (((a) > (b)) ? (a) : (b))
//#endif			   /* max */

//#ifndef min
//#define min(a,b)  (((a) < (b)) ? (a) : (b))
//#endif			   /* min */

#if 0
#define __used			__attribute__((used))
#define __unused		__attribute__((unused))
#define __deprecated	__attribute__((deprecated))
#define __aligned		__attribute__((aligned))
#define __packed		__attribute__((packed))
#define __noreturn		__attribute__((noreturn))
#endif
#endif


