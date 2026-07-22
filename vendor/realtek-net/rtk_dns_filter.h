#ifndef __RTK_DNS_FILTER_H__
#define __RTK_DNS_FILTER_H__

#define DNS_KEY_TABLE	1
#define RTK_DNS_FILTER_CONTENT_LEN 32
#define RTK_DNS_FILTER_MAX_NUM 32
#define RTK_DNS_FILTER_MAC_LENTH 3*6+1
#define RTK_DNS_FILTER_MAC_NUM 3
#define RTK_DNS_FILTER_RULE_TOTAL_LEN 60+RTK_DNS_FILTER_MAC_LENTH*RTK_DNS_FILTER_MAC_NUM+RTK_DNS_FILTER_CONTENT_LEN*RTK_DNS_FILTER_MAX_NUM

#define DNS_FILTER_MODE_BLACK	0
#define DNS_FILTER_MODE_WHITE	1
#define ETH_ADDR_LEN 6

#define RTK_DNS_FILTER_LIST "rtk_dns_filter_list"
#define RTK_DNS_FILTER_ENABLE "rtk_dns_filter_enable"
#define RTK_DNS_FILTER_DBG "rtk_dns_filter_dbg"

#define RTL_PS_BR0_DEV_NAME "br0"

#if defined(CONFIG_CTC_AP) || defined(CONFIG_CMCC_AP) || defined(CONFIG_CU_AP)
#include <asm/byteorder.h>
#include <linux/if_vlan.h>
#include <linux/ip.h>
#ifdef CONFIG_IPV6
#include <net/ipv6.h>
#endif
#include <net/tcp.h>
#include <net/udp.h>

enum {
	T_DNS_IGNORE=0,
	T_DNS_REPLY,
	T_DNS_NAME_ERR
};

enum {
	DNS_RCODE_NOERROR,
	DNS_RCODE_FORMERR,
	DNS_RCODE_SERVFAIL,
	DNS_RCODE_NXDOMAIN,
	DNS_RCODE_NOTIMP,
	DNS_RCODE_REFUSED,
	DNS_RCODE_YXDOMAIN,
	DNS_RCODE_XRRSET,
	DNS_RCODE_NOTAUTH,
	DNS_RCODE_NOTZONE,
	DNS_RCODE_MAX,
};

enum {
	RET_DNS_FILTER_PASS=0,
	RET_DNS_FILTER_DROP,
};

#if defined(__LITTLE_ENDIAN_BITFIELD)
#define DNS_PORT 0x3500
#define DNS_TYPE_CNAME  0x0500
#define DNS_TYPE_A		0x0100
#define DNS_TYPE_AAAA	0x1C00
#elif defined(__BIG_ENDIAN_BITFIELD)
#define DNS_PORT 0x0035
#define DNS_TYPE_CNAME  0x0005
#define DNS_TYPE_A		0x0001
#define DNS_TYPE_AAAA	0x001C
#endif

struct dnshdr {
	unsigned short id : 16;
#if defined(__LITTLE_ENDIAN_BITFIELD)
	unsigned char rd : 1;
	unsigned char tc : 1;
	unsigned char aa : 1;
	unsigned char opcode : 4;
	unsigned char qr : 1;
	unsigned char rcode : 4;
	unsigned char cd : 1;
	unsigned char ad : 1;
	unsigned char z : 1;
	unsigned char ra : 1;
#elif defined(__BIG_ENDIAN_BITFIELD)
	unsigned char qr : 1;
	unsigned char opcode : 4;
	unsigned char aa : 1;
	unsigned char tc : 1;
	unsigned char rd : 1;
	unsigned char ra : 1;
	unsigned char z : 1;
	unsigned char ad : 1;
	unsigned char cd : 1;
	unsigned char rcode : 4;
#else
#error "Adjust your <asm/byteorder.h> defines"
#endif
	unsigned short qdcount : 16;
	unsigned short ancount : 16;
	unsigned short nscount : 16;
	unsigned short arcount : 16;
} __attribute__((packed));;

struct dns_qd_s {
	char *qname;
	unsigned short qtype;
	unsigned short qclass;
} __attribute__((packed));

struct dns_an_s {
	unsigned short name;
	unsigned short type;
	unsigned short rclass;
	unsigned int ttl;
	unsigned short rdlength;
	char rdata[];
} __attribute__((packed));

typedef struct {
	struct ethhdr   *eth_h;
	struct vlan_hdr *vlan_h;
	struct iphdr    *ip4_h;
#ifdef CONFIG_IPV6
	struct ipv6hdr  *ip6_h;
#endif
	struct udphdr   *udp_h;
	struct tcphdr   *tcp_h;
	struct dnshdr   *dns_h;
} _PACKET_INFO;
#endif

typedef struct _rtk_dns_filter
{
	struct list_head rule_list;
	unsigned int listmode;//0:blacklist 1:whitelist
	unsigned int action; //0:drop ;1:response br0 ip; 2:rcode:no server name
	char macAddr[RTK_DNS_FILTER_MAC_LENTH];
	char data[RTK_DNS_FILTER_CONTENT_LEN*RTK_DNS_FILTER_MAX_NUM];
}rtk_dns_filter_entry;

int rtk_dns_filter_init(void);
void rtk_dns_filter_exit(void);
int rtk_filter_dns_query(struct sk_buff **pskb);
#ifdef CONFIG_IPV6
struct udphdr *ipv6_find_udp_hdr(struct sk_buff *skb,int offset);
#endif
#endif

