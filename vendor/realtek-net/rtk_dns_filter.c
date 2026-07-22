#include <linux/module.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/kernel.h>
#include <linux/etherdevice.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/skbuff.h>
#include <linux/version.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/inetdevice.h>
#include <net/checksum.h>
#include <net/udp.h>
#include <linux/ctype.h>

#include <soc/cortina/rtk_common_utility.h>
#include "rtk_dns_filter.h"
#include <soc/cortina/rtk_dns_trap.h>

#ifdef CONFIG_IPV6
#include <linux/in6.h>
#include <net/if_inet6.h>
#include <net/addrconf.h>
#endif

#if defined(CONFIG_CTC_AP) || defined(CONFIG_CMCC_AP) || defined(CONFIG_CU_AP)
#define DBG_DNS_FILTER

#if defined(DBG_DNS_FILTER)
	#define DBGP_DNS_FILTER(format, arg...) 	  \
		do {if (dns_filter_dbg) {printk("[%s %d]"format , __func__, __LINE__, ##arg);}} while(0)
#else
	#define DBGP_DNS_FILTER(format, arg...)
#endif
#endif

int dns_filter_mode = 0;
int rtk_action_for_whitelist = 0;
int rtk_dns_filter_enable = 0;
int dns_filter_rule_num=0;
#if defined(CONFIG_CTC_AP) || defined(CONFIG_CMCC_AP) || defined(CONFIG_CU_AP)
int dns_filter_dbg = 0;
#endif

static LIST_HEAD(rule_list_head);
static DEFINE_RWLOCK(filter_lock);

static int rtk_calc_filter_num(int  *filter_num)
{
	struct list_head *lh;
	rtk_dns_filter_entry *entry=NULL;
	int num = 0;

	if(filter_num == NULL){
		return -1;
	}
	
	read_lock(&filter_lock);
	list_for_each(lh, &rule_list_head)
	{		
		entry=list_entry(lh, rtk_dns_filter_entry, rule_list);
		if(entry){
			num++;		
		}
	}
	read_unlock(&filter_lock);

	*filter_num = num;

	return 0;
}

#if defined(CONFIG_CTC_AP) || defined(CONFIG_CMCC_AP) || defined(CONFIG_CU_AP)
void do_parse_skb_info(const struct sk_buff *skb, _PACKET_INFO *_p)
{
	struct ethhdr  *eth_h=NULL;
	struct vlan_hdr *vlan_h=NULL;
	struct iphdr   *ip4_h=NULL;
	struct ipv6hdr *ip6_h=NULL;
	struct udphdr  *udp_h=NULL;
	struct tcphdr  *tcp_h=NULL;
	struct dnshdr  *dns_h=NULL;
	int offset=0;
	uint16_t protocol;
	
	eth_h = eth_hdr(skb);

	protocol = skb->protocol;
	if (protocol == htons(ETH_P_8021Q)) {
		offset += 4;
		vlan_h = (struct vlan_hdr *)(skb->data);
		protocol = vlan_h->h_vlan_encapsulated_proto;
	}
	if (protocol == htons(ETH_P_IP)) {
		ip4_h = (struct iphdr *)(skb->data+offset);
		if (ip4_h->protocol == IPPROTO_UDP) {
			udp_h = (void *)((char *) ip4_h + ip4_h->ihl * 4);
		}
		else if(ip4_h->protocol == IPPROTO_TCP) {
			tcp_h = (void *)((char *) ip4_h + ip4_h->ihl * 4);
		}
	}
#ifdef CONFIG_IPV6	
	else if(protocol == htons(ETH_P_IPV6)) {
		__u8 nexthdr;
		__be16 frag_off;
		int ptr;
		
		ip6_h = (struct ipv6hdr *)(skb->data+offset);
		ptr = (u8 *)(ip6_h + 1) - skb->data;
		nexthdr = ip6_h->nexthdr;
		ptr = ipv6_skip_exthdr(skb, ptr, &nexthdr, &frag_off);
		if(ptr > 0){
			if (NEXTHDR_UDP == nexthdr) {
				udp_h = (struct udphdr*)(skb->data + ptr);
			} else if (NEXTHDR_TCP == nexthdr) {
				tcp_h = (struct tcphdr*)(skb->data + ptr);
			} 
		}
	}
#endif	

	if((udp_h && (udp_h->source == DNS_PORT || udp_h->dest == DNS_PORT))) {
		dns_h = (struct dnshdr*)((void *)udp_h + sizeof(struct udphdr));
	} else if(tcp_h && (tcp_h->ack & tcp_h->psh) && (tcp_h->source == DNS_PORT || tcp_h->dest == DNS_PORT)) {
		dns_h = (struct dnshdr*)((void *)tcp_h + tcp_h->doff * 4);
	}
	
	if(dns_h) DBGP_DNS_FILTER("dns_h %p qdcount %hu ancount %hu opcode %hhu rcode %hhu\n", 
									dns_h, ntohs(dns_h->qdcount), ntohs(dns_h->ancount), dns_h->opcode, dns_h->rcode);
	
	if(_p) {
		_p->eth_h = eth_h;
		_p->vlan_h = vlan_h;
		_p->ip4_h = ip4_h;
#ifdef CONFIG_IPV6
		_p->ip6_h = ip6_h;
#endif
		_p->udp_h = udp_h;
		_p->tcp_h = tcp_h;
		_p->dns_h = dns_h;	
	}
	return;
}

static inline char* get_qname(const char *data, int len, char *qname, int size)
{
	unsigned char i=0, j, s;
	s = (unsigned char)*(data++);
	while(s != 0 && (i+1+s)<=len && i < (size-1)) {
		for(j=0; j<s; j++)
			qname[i++] = *(data++);
		s = (unsigned char) *(data++);
		if(s != 0) qname[i++] = '.';
	}
	qname[i] = '\0';
	return qname;
}

static inline int
get_next_qname (const struct sk_buff *skb, struct dnshdr *dns_h, 
					int *offset, int offset_max, struct dns_qd_s *q)
{
	char *_c, *_e;
	unsigned char i;
	unsigned short len;
	
	if((*offset)+6 > offset_max) return -1;
	q->qname = _c = (char*)((void*)dns_h + *offset);
	_e = (char*)((void*)dns_h + offset_max);
	while((i=*_c)!=0 && _c<=_e) {
		_c++; 
		while(i>0 && *_c!=0 && _c<=_e) {_c++; i--;}
	}
	if(!(i==0 && (_c+5)<=_e && *_c == 0)) return -1;
	len = _c - q->qname;
	*offset += len + 1;
	q->qtype = *((unsigned short*)((void*)dns_h + *offset));
	*offset += 2;
	q->qclass = *((unsigned short*)((void*)dns_h + *offset));
	*offset += 2;
	return len;
}

static int domain_match(char *ruleStr, char *qname)
{
	char *tokptr, *ptr;
	
	while ((tokptr = strsep(&ruleStr, ",")) != NULL) {
		//remove prefix http:// or https:// and www.
		if (!strncmp(tokptr, "http://", 7)) {
			tokptr += 7;
		}
		else if (!strncmp(tokptr, "https://", 8)) {
			tokptr += 8;
		}
		if (!strncmp(tokptr, "www.", 4)) {
			tokptr += 4;
		}
		//remove suffix .com or .com.cn
		if ((ptr = strstr((const char *)tokptr, (const char *)".com")) && ((*(ptr+4) == '\0') || (*(ptr+4) == '.')))
			*ptr = '\0';
		
		if(strstr(qname, tokptr)){
			return 1;
		}
	}
	return 0;
}
/*
 * @FUNCTION: should_block_domain_name
 * @PARAMETER:
 *    @input: skb
 *            domain_name
 *    @output:
 *            mode: 0 - blacklist        1- whilelist
 *            action: 0：不处理，不代理该DNS请求；1：回复路由器LAN IP地址；2：DNS回复Name Error
 * @RETURN:
 *            0 - unmatch       1 - match
 */
static int should_block_domain_name(struct sk_buff *skb, char* domain_name, int *mode, int *action)
{
	struct list_head *lh;
	rtk_dns_filter_entry *entry=NULL;
	struct ethhdr  *eth_h=eth_hdr(skb);
	int found = 0;
	int rule_mode = 0;
	int domain_len;
	char *ptr, *tokptr;
	int whitelistAction=-1;
	char dnsAddr[RTK_DNS_FILTER_CONTENT_LEN*RTK_DNS_FILTER_MAX_NUM];
	
	unsigned char *skb_mac = NULL;
	char smacAddr[3*ETH_ADDR_LEN+1]={'\0'};

	if(domain_name == NULL || skb==NULL || action==NULL || mode==NULL){
		return 0;
	}

	skb_mac = eth_h->h_source;
	snprintf(smacAddr,sizeof(smacAddr),"%02X:%02X:%02X:%02X:%02X:%02X",
				skb_mac[0],skb_mac[1],skb_mac[2],
				skb_mac[3],skb_mac[4],skb_mac[5]);

	if (!strncmp(domain_name, "http://", 7)) {
		domain_name += 7;
	}
	else if (!strncmp(domain_name, "https://", 8)) {
		domain_name += 8;
	}
	if (!strncmp(domain_name, "www.", 4)) {
		domain_name += 4;
	}
	if ((ptr = strstr((const char *)domain_name, (const char *)".com")) != NULL) {
		/*For most commen case, if domain_name contain ".com", parse it to get hostname.
                * e.g. "sp1s0.a.baidu.com" -> "baidu"
                * then search "baidu" in DNS list.
                */
		*ptr = '\0';
	}

	read_lock(&filter_lock);
	list_for_each(lh, &rule_list_head)
	{		
		entry=list_entry(lh, rtk_dns_filter_entry, rule_list);	
		if(entry){
			if((!strncmp(entry->macAddr,smacAddr, 17))||(!strncmp(entry->macAddr,"all",3)))
			{
				if (entry->listmode && (-1 == whitelistAction))
					whitelistAction = entry->action;
				DBGP_DNS_FILTER("%s %d entryname:%s domain_name:%s\n",__FUNCTION__, __LINE__,entry->data,domain_name);
				#if 0
				while ((tokptr = strsep(&domain_name, ".")) != NULL) {
					if(strstr(entry->data, tokptr)){
						DBGP_DNS_FILTER("%s %d hit:%s\n",__FUNCTION__, __LINE__,tokptr);
						*action = entry->action;
						rule_mode = entry->listmode;
						found =1;
						break;
					}
				}
				if (found)
					break;
				#else
				if (strstr(entry->data, domain_name)) {
					DBGP_DNS_FILTER("hit...\n");
					*action = entry->action;
					rule_mode = entry->listmode;
					found =1;
					break;
				}

				snprintf(dnsAddr, sizeof(dnsAddr), "%s", entry->data);
				if (domain_match(dnsAddr, domain_name)) {
					DBGP_DNS_FILTER("hit...\n");
					*action = entry->action;
					rule_mode = entry->listmode;
					found =1;
					break;
				}
				#endif
			}
		}
	}
	read_unlock(&filter_lock);

	if(found)
	{
		*mode = rule_mode;
		if (rule_mode != 0)/*whitelist, pass*/
			*action = -1;
	}
	else{
		*mode = dns_filter_mode;

		*action = -1;
		if(dns_filter_mode && (whitelistAction != -1)){/* whitelist, block*/
			//whitelist but not match rule need modify
			*action = whitelistAction;
			found = 1;
		}
	}
	return (found);
}

static int do_domain_name_match(const struct sk_buff *skb, _PACKET_INFO *_p, int *mode, int *action)
{
	int ret = 0, i, offset, offset_max, qcount, acount, len;
	struct dnshdr *dns_h;
	struct dns_qd_s q_sec;
	char qname[256]={0};

	dns_h = _p->dns_h;
	qcount = ntohs(dns_h->qdcount);
	acount = ntohs(dns_h->ancount);
	if ((qcount <= 0) || _p->dns_h->qr) {
		DBGP_DNS_FILTER("[%s:%d] It isnt DNS request message, ignore\n",__FUNCTION__,__LINE__);
		goto out;
	}
	
	i = 0;
	offset = sizeof(struct dnshdr);
	offset_max = skb_headlen(skb) - (unsigned int)((void*)dns_h - (void*)skb->data);
	while(qcount > 0 && ret <=0 && offset < offset_max) {
		if((len = get_next_qname(skb, dns_h, &offset, offset_max, &q_sec)) < 0) break;

		get_qname(q_sec.qname, len, qname, sizeof(qname));
		DBGP_DNS_FILTER("[Query %d] %s, TYPE %hu, CLASS %hu\n", ++i, 
							qname, ntohs(q_sec.qtype), ntohs(q_sec.qclass));
				
		ret = should_block_domain_name(skb, qname, mode, action);
		
		qcount--;
	}
out:
	return ret;
}

#define DNS_MAX_SIZE 600
static int do_dns_reply(const struct sk_buff *oldskb, _PACKET_INFO *_p, int action)
{
	int len = 0, ret = 1, offset, offset_max;
	unsigned short qcount, acount, i; 
	struct sk_buff *nskb=NULL;
	__u8 protocol;
	struct ethhdr  *ethh = NULL;
	struct vlan_hdr *vlanh = NULL;
	struct iphdr   *ip4h = NULL;
#ifdef CONFIG_IPV6
	struct ipv6hdr *ip6h = NULL;
#endif
	struct udphdr  *udph = NULL;
	//struct tcphdr  *tcph = NULL;
	struct dnshdr  *dnsh = NULL;
	struct dns_qd_s q_sec;
	void *ptr;

	if (T_DNS_IGNORE == action) {
		return 0;
	}
	if(!(nskb = alloc_skb(DNS_MAX_SIZE+LL_MAX_HEADER, GFP_ATOMIC))) {
		DBGP_DNS_FILTER("No memory to allocate SKB\n");
		goto error;
	}
	
	len = LL_MAX_HEADER;
	if(_p->vlan_h)
		len += sizeof(struct vlan_hdr);
	if(_p->ip4_h)
		len += sizeof(struct iphdr);
#ifdef CONFIG_IPV6
	else if(_p->ip6_h)
		len += sizeof(struct ipv6hdr);
#endif
	skb_reserve(nskb, len);
	
	if(_p->udp_h) {
		skb_reset_transport_header(nskb);
		udph = (struct udphdr *)skb_put(nskb, sizeof(struct udphdr));
		memset(udph, 0, sizeof(struct udphdr));
		udph->source = _p->udp_h->dest;
		udph->dest = _p->udp_h->source;
		udph->len = 0;
		udph->check = 0;
		protocol = IPPROTO_UDP;
	} 
#if 0 //not support yet
	else if(_p->tcp_h) {
		skb_reset_transport_header(nskb);
		tcph = (struct tcphdr *)skb_put(nskb, sizeof(struct tcphdr));
		memset(tcph, 0, sizeof(struct tcphdr));
		tcph->source = _p->tcp_h->dest;
		tcph->dest = _p->tcp_h->source;
		tcph->doff	= sizeof(struct tcphdr) / 4;
		if (_p->tcp_h->ack) {
			tcph->seq = _p->tcp_h->ack_seq;
		} else {
			tcph->ack_seq = htonl(ntohl(_p->tcp_h->seq) + _p->tcp_h->syn + _p->tcp_h->fin +
						  oldskb->len - ip_hdrlen(oldskb) -
						  (_p->tcp_h->doff << 2));
			tcph->ack = 1;
		}
		protocol = IPPROTO_TCP;
	}
#endif
	else {
		DBGP_DNS_FILTER("Error packet L4 header\n");
		goto error;
	}
	
	dnsh = (struct dnshdr *)skb_put(nskb, sizeof(struct dnshdr));
	memcpy(dnsh, _p->dns_h, sizeof(struct dnshdr));
	dnsh->qdcount = 0;
	dnsh->ancount = 0;
	dnsh->nscount = 0;
	dnsh->arcount = 0;
	dnsh->qr = 1;
	
	i = 0;
	qcount = ntohs(_p->dns_h->qdcount);
	offset = sizeof(struct dnshdr);
	offset_max = skb_headlen(oldskb) - (unsigned int)((void*)_p->dns_h - (void*)oldskb->data);
	if(qcount > 0 && offset < offset_max && 
		(len = get_next_qname(oldskb, _p->dns_h, &offset, offset_max, &q_sec)) > 0) 
	{
		ptr = skb_put(nskb, len+5);
		memcpy(ptr, q_sec.qname, len+1);
		ptr += len+1;
		*((unsigned short*)ptr) = q_sec.qtype;
		*((unsigned short*)(ptr+2)) = q_sec.qclass;
		i++;
	}
	dnsh->qdcount = htons(i);
	
	if(T_DNS_NAME_ERR == action) {
		dnsh->rcode = DNS_RCODE_SERVFAIL;
	}
	else {
		struct net *net = dev_net(oldskb->dev);
		struct net_device *dev = dev_get_by_name(net, "br0");
		struct dns_an_s *a_sec;
		acount = 0;
		if(dev) {
			if(q_sec.qtype == DNS_TYPE_A) {
				struct in_device *indev;	
				if((indev = in_dev_get(dev))) {
					struct in_ifaddr *p = indev->ifa_list;
					if(p) {
						a_sec = (struct dns_an_s *)skb_put(nskb, sizeof(struct dns_an_s)+sizeof(p->ifa_address));
						a_sec->name = htons(0xc000 + sizeof(struct dnshdr));
						a_sec->type = q_sec.qtype;
						a_sec->rclass = q_sec.qclass;
						a_sec->ttl = htonl(120);
						a_sec->rdlength = htons(sizeof(p->ifa_address));
						memcpy(a_sec->rdata, &p->ifa_address, sizeof(p->ifa_address));
						acount++;
						offset += sizeof(struct dns_an_s)+sizeof(p->ifa_address);
					}
					in_dev_put(indev);
				}
			}
#ifdef CONFIG_IPV6
			else if(q_sec.qtype == DNS_TYPE_AAAA) {
				struct inet6_dev *idev;
				struct inet6_ifaddr *ifp;
				if((idev = in6_dev_get(dev))) {
					read_lock_bh(&idev->lock);
					list_for_each_entry(ifp, &idev->addr_list, if_list) {
						if (ifp->scope == IFA_LINK && !(ifp->flags & IFA_F_TENTATIVE)) {
							a_sec = (struct dns_an_s *)skb_put(nskb, sizeof(struct dns_an_s)+sizeof(ifp->addr));
							a_sec->name = htons(0xc000 + sizeof(struct dnshdr));
							a_sec->type = q_sec.qtype;
							a_sec->rclass = q_sec.qclass;
							a_sec->ttl = htonl(120);
							a_sec->rdlength = htons(sizeof(ifp->addr));
							memcpy(a_sec->rdata, &ifp->addr, sizeof(ifp->addr));
							acount++;
							offset += sizeof(struct dns_an_s)+sizeof(ifp->addr);
							break;
						}
					}
					read_unlock_bh(&idev->lock);
					in6_dev_put(idev);
				}
			}
#endif
			dev_put(dev);
		} 
		if(acount <= 0) dnsh->rcode = DNS_RCODE_SERVFAIL;
		dnsh->ancount = htons(acount);
	}
	
	len = offset;
	if(udph){
		len += sizeof(struct udphdr);
		udph->len = htons(len);
	}
	
	if(_p->ip4_h) {
		skb_reset_network_header(nskb);
		ip4h = (struct iphdr *)skb_push(nskb, sizeof(struct iphdr));
		memset(ip4h, 0, sizeof(struct iphdr));
		ip4h->version	= 4;
		ip4h->ihl	= sizeof(struct iphdr) / 4;
		ip4h->frag_off	= htons(IP_DF);
		ip4h->protocol	= protocol;
		ip4h->saddr	= _p->ip4_h->daddr;
		ip4h->daddr	= _p->ip4_h->saddr;
		ip4h->ttl	= 64;
		ip4h->tot_len = htons((len+sizeof(struct iphdr)));
		nskb->protocol = htons(ETH_P_IP);
		nskb->csum = skb_checksum(nskb, sizeof(struct iphdr), len, 0);
		ip4h->check = ip_fast_csum(ip4h, ip4h->ihl);
	}
#ifdef CONFIG_IPV6
	else if(_p->ip6_h){
		skb_reset_network_header(nskb);
		ip6h = (struct ipv6hdr*)skb_push(nskb, sizeof(struct ipv6hdr));
		memset(ip6h, 0, sizeof(struct ipv6hdr));
		ip6_flow_hdr(ip6h, 0, 0);
		ip6h->hop_limit = 64;
		ip6h->nexthdr = protocol;
		ip6h->saddr = _p->ip6_h->daddr;
		ip6h->daddr = _p->ip6_h->saddr;
		ip6h->payload_len = htons(len);
		nskb->protocol = htons(ETH_P_IPV6);
		nskb->csum = skb_checksum(nskb, sizeof(struct ipv6hdr), len, 0);
	}
#endif
	else {
		DBGP_DNS_FILTER("Error packet L3 header\n");
		goto error;
	}
 
	if(udph) {
		if(ip4h)
			udph->check = csum_tcpudp_magic(ip4h->saddr, ip4h->daddr, len, IPPROTO_UDP, nskb->csum);
#ifdef CONFIG_IPV6		
		else if(ip6h)
			udph->check = csum_ipv6_magic(&ip6h->saddr, &ip6h->daddr, len, IPPROTO_UDP, nskb->csum);
#endif
	}

	if (_p->vlan_h) {
		vlanh = (struct vlan_hdr *)skb_push(nskb, sizeof(struct vlan_hdr));
		memcpy((void *)vlanh, (const void *)_p->vlan_h, sizeof(struct vlan_hdr));
	}
	
	if(_p->eth_h) {
		ethh = (struct ethhdr*)skb_push(nskb, ETH_HLEN);
		memcpy(ethh->h_source, _p->eth_h->h_dest, ETH_ALEN);
		memcpy(ethh->h_dest, _p->eth_h->h_source, ETH_ALEN);
        if(ip4h) ethh->h_proto = htons(ETH_P_IP);
		else if(ip6h) ethh->h_proto = htons(ETH_P_IPV6);
	}
	
	nskb->dev = oldskb->dev;
	nskb->ip_summed = CHECKSUM_NONE;
	nskb->pkt_type = PACKET_OUTGOING;
	nskb->priority = 0;
	
	if(ip4h)
		DBGP_DNS_FILTER("Send DNS reply to %pI4\n", &ip4h->daddr);
#ifdef CONFIG_IPV6
	else if(ip6h)
		DBGP_DNS_FILTER("Send DNS reply to %pI6c\n", &ip6h->daddr);
#endif
	
	if (dev_queue_xmit(nskb) < 0) {
		DBGP_DNS_FILTER("Fail to send DNS reply\n");
		goto error;
	}
	
	ret = 0;
	return ret;
	
error:
	if(nskb) kfree_skb(nskb);
	return ret;
}

int do_dns_action(const struct sk_buff *skb, _PACKET_INFO *_p, int action)
{
	int ret = RET_DNS_FILTER_PASS, qcount;
	struct dnshdr *dns_h;
	char qname[256]={0}, dns_qname[256]={0}, *comment;
	
	dns_h = _p->dns_h;
	qcount = ntohs(dns_h->qdcount);

	if ((action==T_DNS_IGNORE) || (action==T_DNS_REPLY) || (action==T_DNS_NAME_ERR)) {
		if(qcount <= 0) {
			DBGP_DNS_FILTER("It isnt DNS request message, ignore\n");
			goto out;
		}
		
		if (!do_dns_reply(skb, _p, action)) {
			dev_kfree_skb(skb);
			ret = RET_DNS_FILTER_DROP;
		}
	}
	
out:
	return ret;
}

int rtk_filter_dns_query(struct sk_buff **pskb)
{
	_PACKET_INFO _p;
	struct sk_buff *skb = NULL;
	int filter_number = dns_filter_rule_num;
	int mode=0, action=0;
	int ret;

	//dns filter disable and blacklist case don't filter dns packets
	if(rtk_dns_filter_enable!=1 || (filter_number <= 0 && dns_filter_mode == DNS_FILTER_MODE_BLACK))
		return RET_DNS_FILTER_PASS;

	skb = *pskb;
	do_parse_skb_info(skb, &_p);

 	if (_p.dns_h)
	{
		ret = do_domain_name_match(skb, &_p, &mode, &action);
		if (ret) {//match
			if (0 == mode) {//blacklist
				return do_dns_action(skb, &_p, action);
			}
			else {//whitelist
				if (action != -1) {
					/* unmatch dns, but match mac, follow action */
					return do_dns_action(skb, &_p, action);
				}
				else {
					/* no rule match, do nothing */
				}
			}
			return (RET_DNS_FILTER_PASS);
		}
#if 0
		else {//not match
			if (0 == mode) //blacklist but not match rule dont need modify
				return RET_DNS_FILTER_PASS;
			else {//whitelist but not match rule need modify
				/* no rule match, so how about the action??? */
				return do_dns_action(skb, &_p, T_DNS_IGNORE);
			}
		}
#endif
	}
	
	return RET_DNS_FILTER_PASS;
}
#else
static int get_dev_ip_mask(const char * name, unsigned int *ip, unsigned int *mask)
{
	struct in_device *in_dev = NULL;
	struct net_device *landev = NULL;
	struct in_ifaddr *ifap = NULL;
	
	if((name == NULL) || (ip==NULL) || (mask == NULL)) 
	{
		return -1;
	}
	
	if ((landev = __dev_get_by_name(&init_net, name)) != NULL)
	{
		in_dev=(struct in_device *)(landev->ip_ptr);
		if (in_dev != NULL) 
		{
			for (ifap=in_dev->ifa_list; ifap != NULL; ifap=ifap->ifa_next)
			{
				if (strcmp(name, ifap->ifa_label) == 0)
				{
					*ip = ifap->ifa_address;
					*mask = ifap->ifa_mask;
					return 0;
				}
			}

		}
	}

	return -1;
}

static int should_block_domain_name(struct sk_buff *skb,char* domain_name,int *ctcapd_flag)
{	
	struct list_head *lh;
	rtk_dns_filter_entry *entry=NULL;
	int found = 0;
	int domain_len = 0;
	int rule_mode = 0;
	
	unsigned char *skb_mac = NULL;
	char dmacAddr[3*ETH_ADDR_LEN+1]={'\0'};

	if(domain_name == NULL || skb==NULL || ctcapd_flag==NULL){
		return 0;
	}

	skb_mac = skb_mac_header(skb);

	if(skb_mac==NULL){
		return 0;
	}
	if(strstr((char*)domain_name, ".com")){
		/*For most commen case, if domain_name contain ".com", parse it to get hostname.
                * e.g. "sp1s0.a.baidu.com" -> "baidu"
                * then search "baidu" in DNS list.
                */
		domain_len = strlen(strstr(domain_name, ".com"));
		domain_len = strlen(domain_name) - domain_len;
		domain_name [domain_len] = '\0';
		if (strchr(domain_name, '.') !=NULL)	domain_name = strrchr(domain_name, '.')+1;
	}
	snprintf(dmacAddr,sizeof(dmacAddr),"%02X:%02X:%02X:%02X:%02X:%02X",
		skb_mac[0],skb_mac[1],skb_mac[2],
		skb_mac[3],skb_mac[4],skb_mac[5]);

	read_lock(&filter_lock);
	list_for_each(lh, &rule_list_head)
	{		
		entry=list_entry(lh, rtk_dns_filter_entry, rule_list);	
		
		if(entry){
			if((strstr(entry->macAddr,dmacAddr)!=0)||(!strncmp(entry->macAddr,"all",3)))
			{
				//printk("111 %s %d entryname:%s domain_name:%s\n",__FUNCTION__, __LINE__,entry->data,domain_name);
				if(strstr(entry->data,domain_name)){
					*ctcapd_flag = entry->action;
					rule_mode = entry->listmode;
					found =1;
					break;
				}
			}
		}
	}
	read_unlock(&filter_lock);

	if(found)
	{
		if(!rule_mode)
		{
			//match blacklist rule need modify
			return 1;
		}
		else
		{	
			//match whitelist rule dont need modify
			return 0;
		}
	}
	else{
		if(!dns_filter_mode){
			//blacklist but not match rule dont need modify
			return 0;
		}
		else{
			//whitelist but not match rule need modify
			return 1;
		}
	}
	
}

int modifyDnsReply(struct sk_buff *skb, int ctcapd_flag)
{
    struct iphdr *iph = NULL;
    struct udphdr *udph = NULL;
    unsigned char *data = {'\0'};
    int record_num = 0;
    unsigned char change_flag = 0;
    int i = 0;
    unsigned int br0_ip = 0;
    unsigned int br0_mask = 0;
    unsigned short dns_hdr_flag = 0;
    int offset = ETH_ADDR_LEN*2+2;
    unsigned short typeT = 0;
    unsigned short data_len = 0;
    dnsheader_t *dns_hdr = NULL;

	if(skb == NULL){
		return -1;
	}

	if(skb->protocol==htons(ETH_P_8021Q))
		offset+=4;
	
	iph=(struct iphdr *)(skb->data+offset);

	if(iph==NULL)
		return -1;
	if (iph->version == 4){
    	udph=(void *) iph + iph->ihl*4;
	}else{
		udph = ipv6_find_udp_hdr(skb,offset);
	}
	if(udph==NULL)	
		return -1;
	
	dns_hdr = (dnsheader_t*)((void*)udph + sizeof(struct udphdr));

    data = (void *)udph + sizeof(struct udphdr) + 12;

	if(dns_hdr==NULL || data==NULL)
		return -1;

    if (ntohs(udph->source) != 53) // DNS response
	{
		return 0;
    }
	if(dns_hdr == NULL)
	{
		return -1;
	}
	if(((dns_hdr->u & 0x8000)>>15) != 1)/*Not an answer*/
	{
		return 0;
	}
	record_num = ntohs(dns_hdr->ancount);
	if(record_num <= 0)
	{
		return -1;
	}
	while(*data){
		data ++;
	}//query domain name end
	data += 5;//(1:to answer,type:2byte ,class:2byte) to answer
	do 
	{
		data += 2;//(domain name pointer,2byte)current:type
		typeT = ntohs((*(unsigned short*)data));
		data += 8;//type:2bytes+class:2bytes+timetolive:4bytes,current:data_len
		data_len = ntohs(*(unsigned short*)data);
		data += 2;//(datalen:2byte),current:data
		//printk("%s %d ctcapd_flag:%d typeT:%d data_len:%d \n",__FUNCTION__, __LINE__, ctcapd_flag,typeT, data_len);
		if(typeT == 0x01 && data_len == 4)//IPV4 address
		{
			if(ctcapd_flag == 1 || (dns_filter_mode == 1 && rtk_action_for_whitelist == 1)){
				//action :1 reply modify ip->dut ip
				if(!get_dev_ip_mask(RTL_PS_BR0_DEV_NAME, &br0_ip, &br0_mask)){
					data[0] = (br0_ip&0xff000000)>>24;
					data[1] = (br0_ip&0x00ff0000)>>16;
					data[2] = (br0_ip&0x0000ff00)>>8;
					data[3] = (br0_ip&0x000000ff)>>0;
				}
				else{
					printk("get ip fail!\n");
					return -1;
				}
			}	
			else if(ctcapd_flag == 2 || (dns_filter_mode == 1 && rtk_action_for_whitelist == 2))
			{
				//action :2 reply dnshdr->rcode=3 name error
				dns_hdr_flag = ntohs(dns_hdr->u);
				dns_hdr_flag |= 0x3;
				dns_hdr->u = htons(dns_hdr_flag);
			}
			else{
				//action :0 drop pkts
				memset(data,0,4);
			}

			#if 0
			printk("dut ip:%x ip:%x.%x.%x.%x flag:%d dns_hdr:%x\n",br0_ip,
									data[0],
									data[1],
									data[2],
									data[3],ctcapd_flag,dns_hdr->u);
			#endif
			
			change_flag = 1;
		}
		else if(typeT == 0x1c && data_len == 16)//IPV6 address
		{
			memset(data,0,16);
			change_flag = 1;
		}
		if (data + data_len == NULL) break;
		data += data_len;//to next record
		i ++;
	}while(i < record_num);
	if(change_flag)
	{
		/* ip checksum */
		skb->ip_summed = CHECKSUM_NONE;
		iph->check = 0;
		if (iph->version ==4) iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl);

		/* udp checksum */
		udph->check = 0;
		udph->check = csum_tcpudp_magic(iph->saddr, iph->daddr,
						ntohs(udph->len), IPPROTO_UDP,
						csum_partial((char *)udph,
									 ntohs(udph->len), 0));
	}
    return 0;
}

int rtk_filter_dns_query(struct sk_buff **pskb){
	struct iphdr *iph = NULL;
#ifdef CONFIG_IPV6
	struct ipv6hdr *ipv6h = NULL;
#endif
	struct udphdr *udph = NULL;
	struct sk_buff *skb = NULL;
	unsigned char *data = NULL;
	int filter_number = 0;
	int offset = ETH_ADDR_LEN*2+2;
	int datalen = 0;
	int ctcapd_filter_flag = 0;
	char name[256] = {'\0'};

	if (!pskb || !*pskb) return -1;
	rtk_calc_filter_num(&filter_number);
	//dns filter disable and blacklist case don't filter dns packets
	if(rtk_dns_filter_enable!=1 || (filter_number <= 0 && dns_filter_mode == DNS_FILTER_MODE_BLACK)) return 0;

	skb = *pskb;
	if(skb->protocol == htons(ETH_P_8021Q)) {
		offset += 4; // VLAN offset.
        }

	iph=(struct iphdr *)(skb->data+offset);
#ifdef CONFIG_IPV6
	ipv6h=(struct ipv6hdr *)(skb->data+offset);
#endif
	if(iph==NULL) return -1;

	if(iph->version == 4
#ifdef CONFIG_IPV6
           || ipv6h->version == 6
#endif
          ){
		if(iph->version == 4  && iph->protocol == IPPROTO_UDP){
			//ipv4 udp
			udph = (void *)iph + iph->ihl*4;}
#ifdef CONFIG_IPV6
		else{
			//ipv6 udp
			if(ipv6h->version == 6 && ipv6h->nexthdr== IPPROTO_UDP){
				udph = ipv6_find_udp_hdr(skb,offset);
                          if (ntohs(ipv6h->payload_len-udph->len)<0) return -1;
			}
		}
#endif
		if(udph==NULL || udph -> dest == NULL) return -1;
		if (ntohs(udph->dest) != 53 && ntohs(udph->source) != 53){
			return 0;
		}
		/*************************************************************************************
 
	    RFC 1035 ( Chap 4.1 , p.25 ) :
	    All communications inside of domain protocol are carried in a single format called a MESSAGE,
	    The top level format of MESSAGE is divided into 5 sections shown below:

	            +---------------------+
	            |             Header              |
	            +---------------------+
	            |            Question             | the question for the name server
	            +---------------------+
	            |             Answer              | RRs answering the question
	            +---------------------+
	            |           Authority              | RRs pointing toward an authority
	            +---------------------+
	            |           Additional             | RRs holding additional information
	            +---------------------+

	    We would parse these sections step by step.
	 
	   	*************************************************************************************/
	 
	 
	    /*************************************************************************************
            [HEADER]

            RFC 1035 ( 4.1.1. , p.26-p.27 ) :

            The header contains the following fields:

                                                                            1    1    1    1    1    1
                   0    1    2    3    4    5    6    7    8    9    0    1    2    3    4    5
                +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
                |                                            ID                                           |
                +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
                |QR|      Opcode     |AA|TC|RD|RA|       Z      |      RCODE     |
                +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
                |                                       QDCOUNT                                     |
                +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
                |                                       ANCOUNT                                      |
                +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
                |                                       NSCOUNT                                      |
                +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
                |                                       ARCOUNT                                      |
                +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

            where:

            ID                      Identifier
            QR                      query (0), response (1).
            OPCODE          Define the kind of this message.
            AA                      Indicate if this message is sent by the AUTHORITY server for first replied query or not.
            TC                      Indicate if this packet is truncated or not.
            RD                      Request DNS server to pursue this query recursively
            RA                      Indicate if the DNS server support recursive process or not.
            Z                       reserved, must be 0.
            RCODE           return value: 0 means NO ERROR, others mean something wrong.
            QDCOUNT # of entries in the question section.
            ANCOUNT # of resource records in the answer section.
            NSCOUNT # of name server resource records in the authority records section.
            ARCOUNT # of resource records in the additional records section.

    ***************************************************************************

    IPv4, IPv6, UDP header are seperately defined at source/include/uapi/linux/
    ip.h, ipv6.h, udp.h.

RFC 1883 Page 4
3.  IPv6 Header Format:

   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |Version| Prio. |                   Flow Label                  |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |         Payload Length        |  Next Header  |   Hop Limit   |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   +                         Source Address                        +
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   +                      Destination Address                      +
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   +                                                               +
   +                             Data                              +
   +                                                               +
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

	in this function,
        first take ETH_HLEN (14) to gain IPv4 and IPv6 header in skb data filed.
        version, and next header = IPPROTO_UDP (17) ,
        to find IPv6 UDP packet.
        header length for ipv6 is fixed as 40, then get the UPD header.
        check port 53 to find DNS packet

        Then get hostname and do the name check,
        set response according to action code.

*************************************************************************************/
		data = (void *)udph + sizeof(struct udphdr) + 12;
		if(data==NULL) return -1;
		//parse data
		datalen = ntohs(udph->len) - sizeof(struct udphdr) - 12; // 12 is ID+Flags+QDCOUNT+ANCOUNT+NSCOUNT+ARCOUNT
		memset(name, 0, sizeof(name));
		while (datalen > 0) {
		        if (*data) {
				if((strlen(name) + (*data)) >= (sizeof(name)-1)) return 0;
		                strncpy(name+strlen(name), data+1, *data);
		                datalen -= (*data + 1);
		                data += (*data + 1);
		                if (*data) {
		                        strcat(name, ".");
		            }
		        }
		        else
		                break;
		}
		if(should_block_domain_name(skb,name,&ctcapd_filter_flag)){
			modifyDnsReply(skb,ctcapd_filter_flag);
			return 0;
		}
	}
	return 0;
}
#endif

int rtk_filter_table_head_init(void)
{
	dns_filter_mode = 0;
	
	return 0;
}

int rtk_filter_table_regist(struct list_head * new_item)
{
	if(new_item ==NULL)
		return 0;
	
	write_lock_bh(&filter_lock);
	list_add(new_item, &rule_list_head);
	dns_filter_rule_num++;
	write_unlock_bh(&filter_lock);
	
	return 0;
}

int rtk_filter_table_del(char *domain,char *macaddr)
{
	struct list_head *lh=NULL;
	struct list_head *lhp=NULL;
	rtk_dns_filter_entry *entry=NULL; 

	if(domain == NULL|| macaddr ==NULL)
		return 0;

	write_lock_bh(&filter_lock);
	list_for_each_safe(lh, lhp,&rule_list_head)
	{	
		entry = list_entry(lh, rtk_dns_filter_entry, rule_list);
		if(entry){
			if((!strncmp(entry->macAddr,macaddr,sizeof(entry->macAddr))) && (!strncmp(entry->data,domain,sizeof(entry->data)))){
				list_del(&entry->rule_list);
				kfree(entry);
				dns_filter_rule_num--;
			}
		}
	}
	write_unlock_bh(&filter_lock);

	return 0;
	
}

int rtk_filter_table_flush(void)
{
	struct list_head *lh=NULL;
	struct list_head *lhp=NULL;
	rtk_dns_filter_entry *entry=NULL; 

	write_lock_bh(&filter_lock);
	list_for_each_safe(lh, lhp,&rule_list_head)
	{	
		entry = list_entry(lh, rtk_dns_filter_entry, rule_list);
		if(entry){
				list_del(&entry->rule_list);
				kfree(entry);
		}
	}
	write_unlock_bh(&filter_lock);

	dns_filter_rule_num=0;
	dns_filter_mode=0;
	
	return 0;
}

unsigned long str2hexnum(unsigned char *p)
{
	unsigned long val=0;
	unsigned long c=0;

	if(p == NULL)
		return -1;
	
	for(val=0;*p!='\0';val=(val<< 4 ) + c, p++)
	{
		c=*p;
		if(c >= '0' && c <='9')
			c=c-'0';
		else if(c >='a' && c <='f')
			c=c-'a'+10;
		else if(c >='A' &&  c<='F')
			c=c-'A'+10;
		else
			return -1;
	}
	
	return val;

}

static int rtk_dns_filter_table_write_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
  	//char *tokptr=NULL,*tokptr1=NULL;
  	char *tokptr=NULL;
	char *tmpbuf=NULL;
	int ctcapd_filter_flag = 0;
	rtk_dns_filter_entry *entry;
	//int listmode = 0,i = 0;
	int listmode = 0;
	char ctcapd_dns_filter_mac[RTK_DNS_FILTER_MAC_LENTH*RTK_DNS_FILTER_MAC_NUM];
	//char dns_filter_domain_tmp[RTK_DNS_FILTER_CONTENT_LEN];
	unsigned char totalmode[2];

#if 1	// fix -Werror=frame-larger-than=
	unsigned int info_size = RTK_DNS_FILTER_RULE_TOTAL_LEN * sizeof(unsigned char);
	unsigned int domain_size = RTK_DNS_FILTER_CONTENT_LEN * RTK_DNS_FILTER_MAX_NUM * sizeof(char);

	unsigned char *ctcapd_get_info_from_usr_space = kmalloc(info_size, GFP_KERNEL);
	char *ctcapd_dns_filter_domain = kmalloc(domain_size, GFP_KERNEL);
#else
	unsigned char ctcapd_get_info_from_usr_space[RTK_DNS_FILTER_RULE_TOTAL_LEN];
	char ctcapd_dns_filter_domain[RTK_DNS_FILTER_CONTENT_LEN*RTK_DNS_FILTER_MAX_NUM];
#endif

	if(ctcapd_get_info_from_usr_space == NULL || ctcapd_dns_filter_domain == NULL)
		goto _err;
	
	memset(ctcapd_get_info_from_usr_space,0, info_size);
	memset(ctcapd_dns_filter_domain,0, domain_size);


	if(count>RTK_DNS_FILTER_RULE_TOTAL_LEN)
		goto _err;

    if (buffer && !copy_from_user(ctcapd_get_info_from_usr_space, buffer, count))
 	{
	    ctcapd_get_info_from_usr_space[count]='\0';	
		tmpbuf = ctcapd_get_info_from_usr_space;
		if(memcmp(tmpbuf,"init", strlen("init")) == 0)
		{
			rtk_filter_table_head_init();
		}
		else if(memcmp(tmpbuf,"flush", strlen("flush")) == 0)
		{	
			rtk_filter_table_flush();		
		}
		else if(memcmp(tmpbuf,"white", strlen("white")) == 0)
		{
			dns_filter_mode=DNS_FILTER_MODE_WHITE;
		}
		else if(memcmp(tmpbuf,"black", strlen("black")) == 0)
		{
			dns_filter_mode=DNS_FILTER_MODE_BLACK;
		}
		else if(memcmp(tmpbuf,"totalwhitemode:", strlen("totalwhitemode:")) == 0)
		{
			tmpbuf+=strlen("totalwhitemode:");
			if(!tmpbuf) goto _err;

			snprintf(totalmode,sizeof(totalmode),"%s",tmpbuf);
			if((listmode = str2hexnum(totalmode))==-1){
				printk("%s %d totalmode=%s\n",__FUNCTION__,__LINE__,totalmode);
				goto _err;
			}

			rtk_action_for_whitelist = listmode;
			//printk("%s %d rtk_action_for_whitelist=%d listmode:%d\n",__FUNCTION__,__LINE__,rtk_action_for_whitelist,listmode);
		}
		else if(memcmp(tmpbuf,"add:", strlen("add:")) == 0)
		{
			//printk("1:%s %d tmpbuf=%s\n",__FUNCTION__,__LINE__,tmpbuf);
			//add:1 2 www.baidu.com,http://www.douban.com c0:a8:01:65:27:a3
			tmpbuf+=strlen("add:");
			if(!tmpbuf) goto _err;
			
			//printk("2:%s %d tmpbuf=%s\n",__FUNCTION__,__LINE__,tmpbuf);
			//1 2 www.baidu.com,http://www.douban.com c0:a8:01:65:27:a3
			tokptr = strsep(&tmpbuf," ");
			if(!tmpbuf) goto _err;
			if((listmode= str2hexnum(tokptr))==-1)
				goto _err;

			//printk("3:%s %d tmpbuf=%s\n",__FUNCTION__,__LINE__,tmpbuf);
			//2 www.baidu.com,http://www.douban.com c0:a8:01:65:27:a3
			tokptr = strsep(&tmpbuf," ");
			if(!tmpbuf) goto _err;
			if((ctcapd_filter_flag= str2hexnum(tokptr))==-1)
				goto _err;

			//printk("4:%s %d tmpbuf=%s\n",__FUNCTION__,__LINE__,tmpbuf);
			//www.baidu.com,http://www.douban.com c0:a8:01:65:27:a3
			tokptr = strsep(&tmpbuf," ");
			if(!tmpbuf) goto _err;
			if(!snprintf(ctcapd_dns_filter_domain, domain_size,"%s",tokptr))
				goto _err;
			#if 0
			for (tokptr1 = strsep(&tokptr,","); tokptr1 != NULL;
			tokptr1 = strsep(&tokptr,","))
			{
				memset(dns_filter_domain_tmp,0,sizeof(dns_filter_domain_tmp));
				snprintf(dns_filter_domain_tmp,sizeof(dns_filter_domain_tmp),"%s",tokptr1);

				if(!strncmp(dns_filter_domain_tmp,"http://",7))
					for(i=7;i<sizeof(dns_filter_domain_tmp);i++)
						dns_filter_domain_tmp[i-7]=dns_filter_domain_tmp[i];
				if(!strncmp(dns_filter_domain_tmp,"https://",8))
					for(i=8;i<sizeof(dns_filter_domain_tmp);i++)
						dns_filter_domain_tmp[i-8]=dns_filter_domain_tmp[i];
				if(!strncmp(dns_filter_domain_tmp,"www.",4))
					for(i=4;i<sizeof(dns_filter_domain_tmp);i++)
						dns_filter_domain_tmp[i-4]=dns_filter_domain_tmp[i];

				strcat(ctcapd_dns_filter_domain,dns_filter_domain_tmp);
			}
			#endif

			//c0:a8:01:65:27:a3
			//printk("4:%s %d tmpbuf=%s\n",__FUNCTION__,__LINE__,tmpbuf);
			if(!snprintf(ctcapd_dns_filter_mac,sizeof(entry->macAddr),"%s",tmpbuf))
				goto _err;

			entry = kmalloc(sizeof(rtk_dns_filter_entry),GFP_KERNEL);		
			if(entry == NULL)
				goto _err;

			INIT_LIST_HEAD(&entry->rule_list);

			entry->listmode=listmode;
			entry->action	= ctcapd_filter_flag;
			snprintf(entry->data,sizeof(entry->data),"%s",ctcapd_dns_filter_domain);
			snprintf(entry->macAddr,sizeof(entry->macAddr),"%s",ctcapd_dns_filter_mac);

			//printk("=========%s %d data=%s macAddr=%s\n",__FUNCTION__,__LINE__,entry->data,entry->macAddr);
			rtk_filter_table_regist(&entry->rule_list);
		}
		else if(memcmp(tmpbuf,"del:", strlen("del:")) == 0){
			//printk("%s %d tmpbuf:%s\n",__FUNCTION__,__LINE__,tmpbuf);
			tmpbuf+=strlen("del:");
			if(!tmpbuf) goto _err;

			tokptr = strsep(&tmpbuf," ");
			if(!tmpbuf) goto _err;
			if(!snprintf(ctcapd_dns_filter_domain, domain_size,"%s",tokptr))
				goto _err;
			
			if(!snprintf(ctcapd_dns_filter_mac,sizeof(ctcapd_dns_filter_mac),"%s",tmpbuf))
				goto _err;

			rtk_filter_table_del(ctcapd_dns_filter_domain, ctcapd_dns_filter_mac);
		}

		if (ctcapd_get_info_from_usr_space)
			kfree(ctcapd_get_info_from_usr_space);

		if (ctcapd_dns_filter_domain)
			kfree(ctcapd_dns_filter_domain);

		return count;
	}
_err:	

	if (ctcapd_get_info_from_usr_space)
		kfree(ctcapd_get_info_from_usr_space);

	if (ctcapd_dns_filter_domain)
		kfree(ctcapd_dns_filter_domain);

	printk("write filter rule error\n");
	return -1;
}

static int rtk_dns_filter_table_read_proc(struct seq_file *s, void *v)
{
	struct list_head *lh;
	rtk_dns_filter_entry *entry=NULL;
	int filter_number = 0;

	rtk_calc_filter_num(&filter_number);
	
	if(dns_filter_mode){
		seq_printf(s, "total mode=%d total_action:%d rule_num=%d\n",dns_filter_mode, rtk_action_for_whitelist, filter_number);
	}
	else{
		seq_printf(s, "total mode=%d rule_num=%d\n",dns_filter_mode,filter_number);
	}

	read_lock(&filter_lock);
	list_for_each(lh, &rule_list_head)
	{		
		entry=list_entry(lh, rtk_dns_filter_entry, rule_list);
		if(entry){	
			seq_printf(s, "listmode=%d action=%d domain=%s mac=%s\n",entry->listmode,entry->action,entry->data,entry->macAddr);
		}
	}
	read_unlock(&filter_lock);
	
    return 0;
}

static int rtk_dns_filter_enable_write_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
  	unsigned char ctcapd_get_info_from_usr_space[32];
	unsigned char *tmpbuf=NULL;

	memset(ctcapd_get_info_from_usr_space,0,sizeof(ctcapd_get_info_from_usr_space));
	
    if (buffer && !copy_from_user(&ctcapd_get_info_from_usr_space, buffer, count))
 	{
	    ctcapd_get_info_from_usr_space[count]='\0';	
		tmpbuf = ctcapd_get_info_from_usr_space;
		if(memcmp(tmpbuf,"enable", strlen("enable")) == 0)
		{
			rtk_dns_filter_enable = 1;
		}
		else if(memcmp(tmpbuf,"disable", strlen("disable")) == 0){
			rtk_dns_filter_enable = 0;
		}

		return count;
	}
	
	return -1;
}

static int rtk_dns_filter_enable_read_proc(struct seq_file *s, void *v)
{

	if(rtk_dns_filter_enable == 1)
		seq_printf(s, "dns_filter_enable:%s\n", "enable");
	else
		seq_printf(s, "dns_filter_enable:%s\n", "disable");

    return 0;
}

int rtk_dns_filter_table_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, rtk_dns_filter_table_read_proc, NULL));
}

int rtk_dns_filter_table_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	return rtk_dns_filter_table_write_proc(file, userbuf,count, off);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
struct proc_ops rtk_dns_filter_table_proc_fops = {
        .proc_open           = rtk_dns_filter_table_single_open,
	 	.proc_write		= rtk_dns_filter_table_single_write,
        .proc_read           = seq_read,
        .proc_lseek         = seq_lseek,
        .proc_release        = single_release,
};
#else
struct file_operations rtk_dns_filter_table_proc_fops = {
        .open           = rtk_dns_filter_table_single_open,
	 	.write		= rtk_dns_filter_table_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif

int rtk_dns_filter_enable_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, rtk_dns_filter_enable_read_proc, NULL));
}

int rtk_dns_filter_enable_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	return rtk_dns_filter_enable_write_proc(file, userbuf,count, off);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
struct proc_ops rtk_dns_filter_enable_proc_fops = {
        .proc_open           = rtk_dns_filter_enable_single_open,
	 	.proc_write		= rtk_dns_filter_enable_single_write,
        .proc_read           = seq_read,
        .proc_lseek         = seq_lseek,
        .proc_release        = single_release,
};
#else
struct file_operations rtk_dns_filter_enable_proc_fops = {
        .open           = rtk_dns_filter_enable_single_open,
	 	.write		= rtk_dns_filter_enable_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif

#if defined(CONFIG_CTC_AP) || defined(CONFIG_CMCC_AP) || defined(CONFIG_CU_AP)
static int rtk_dns_filter_dbg_read_proc(struct seq_file *s, void *v)
{

	if(dns_filter_dbg == 1)
		seq_printf(s, "dns_filter_dbg:%s\n", "enable");
	else
		seq_printf(s, "dns_filter_dbg:%s\n", "disable");

    return 0;
}

static int rtk_dns_filter_dbg_write_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
  	unsigned char tmpbuf[32];

    if (buffer && !copy_from_user(&tmpbuf, buffer, count))
 	{
 		dns_filter_dbg = simple_strtoul(tmpbuf, NULL, 0);

		return count;
	}
	
	return -1;
}

int rtk_dns_filter_dbg_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, rtk_dns_filter_dbg_read_proc, NULL));
}

int rtk_dns_filter_dbg_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	return rtk_dns_filter_dbg_write_proc(file, userbuf,count, off);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
struct proc_ops rtk_dns_filter_dbg_proc_fops = {
        .proc_open           = rtk_dns_filter_dbg_single_open,
	 	.proc_write		= rtk_dns_filter_dbg_single_write,
        .proc_read           = seq_read,
        .proc_lseek         = seq_lseek,
        .proc_release        = single_release,
};
#else
struct file_operations rtk_dns_filter_enable_proc_fops = {
        .open           = rtk_dns_filter_dbg_single_open,
	 	.write		= rtk_dns_filter_dbg_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif
#endif

static void rtk_dns_filter_create_proc(void)
{
	if(rtk_proc_dir==NULL)
		rtk_proc_dir=proc_mkdir("driver/realtek", NULL);

	rtk_filter_table_head_init();
	if(rtk_proc_dir)
	{
		proc_create(RTK_DNS_FILTER_LIST, 0, rtk_proc_dir, &rtk_dns_filter_table_proc_fops);	
		proc_create(RTK_DNS_FILTER_ENABLE, 0, rtk_proc_dir, &rtk_dns_filter_enable_proc_fops);
#if defined(CONFIG_CTC_AP) || defined(CONFIG_CMCC_AP) || defined(CONFIG_CU_AP)
		proc_create(RTK_DNS_FILTER_DBG, 0, rtk_proc_dir, &rtk_dns_filter_dbg_proc_fops);
#endif
	}	
}
static void rtk_dns_filter_destroy_proc(void)
{
	if(rtk_proc_dir)
	{	
		remove_proc_entry(RTK_DNS_FILTER_LIST, rtk_proc_dir);	
		remove_proc_entry(RTK_DNS_FILTER_ENABLE, rtk_proc_dir);
	}
}

int __init rtk_dns_filter_init(void)
{
	rtk_dns_filter_create_proc();

	dns_filter_mode = 0;

	return 0;
}

void __exit rtk_dns_filter_exit(void)
{
	rtk_dns_filter_destroy_proc();
}


