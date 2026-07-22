#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv6.h>
#include <linux/tty.h>
#include <net/icmp.h>
#include <net/ip.h>     /*  for local_port_range[] */
#include <net/tcp.h>        /*  struct or_callable used in sock_rcv_skb */
#include <net/netlabel.h>
#include <linux/uaccess.h>
#include <asm/ioctls.h>
#include <linux/bitops.h>
#include <linux/interrupt.h>
#include <linux/netlink.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <net/ipv6.h>
#include <net/dsfield.h>
#include <linux/netfilter/xt_dscp.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 21)
#include <net/netfilter/nf_conntrack.h>
#else
#include <linux/netfilter_ipv4/ip_conntrack.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 31) && LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 33)
#include <net/netfilter/nf_conntrack_ecache.h>
#endif

#include "ctc_dpi_if.h"
#include "ctc_gw_if.h"



//#define LOG_DEBUG   printk

enum {
    NETFILTER_PRE_ROUTING,
    NETFILTER_FORWARD,
};

typedef struct netfilter_priv_info_
{
    int                 index;
    int                 hooknum;
    struct nf_hook_ops  *ops;
#ifdef CONFIG_IPV6
	struct nf_hook_ops  *opsv6;
#endif
} netfilter_priv_info_t;

unsigned long  g_forward_count = 0;
unsigned long  g_prerouting_count = 0;

static ctSgw_appGetInfoProc g_getinfo_func = NULL;
static ctSgw_appPreProc     g_prerouting_func[NETFILTER_MAX_HOOKS];
static ctSgw_appForwardProc g_forward_func[NETFILTER_MAX_HOOKS];

static appCtrlLogProc g_appCtrlLog_func = NULL;

void dump_hook_func(struct seq_file *seq)
{
	int i;
#if 1
	seq_printf(seq,"PreRoute Register Hook %08lu\n", g_prerouting_count);
	for (i = 0; i < NETFILTER_MAX_HOOKS; i++)
	{
		if(g_prerouting_func[i] != NULL)
		seq_printf(seq,"\tPrerouteFunc[%d]: %p\n", i, g_prerouting_func[i]);
	}
	seq_printf(seq,"Forword Register Hook %08lu\n", g_forward_count);
	for (i = 0; i < NETFILTER_MAX_HOOKS; i++)
	{
		if(g_forward_func[i] != NULL)
		seq_printf(seq,"\tForwordFunc[%d]: %p\n", i, g_forward_func[i]);
	}
#endif
}

void dump_tuple_info(tupleinfo_t *tuple_info)
{
	DPI_LOG(DPI_LOG_LEVEL_DEBUG,"=====================================================\n");
	DPI_LOG(DPI_LOG_LEVEL_DEBUG,"tuple_info->direct=%d\n",tuple_info->direct);
	DPI_LOG(DPI_LOG_LEVEL_DEBUG,"tuple_info->proto=%d\n",tuple_info->proto);
	DPI_LOG(DPI_LOG_LEVEL_DEBUG,"tuple_info->new_conntrack=%d\n",tuple_info->new_conntrack);
	DPI_LOG(DPI_LOG_LEVEL_DEBUG,"tuple_info->sipv4=0x%x\n",ntohl(tuple_info->sipv4));
	DPI_LOG(DPI_LOG_LEVEL_DEBUG,"tuple_info->dipv4=0x%x\n",ntohl(tuple_info->dipv4));
	DPI_LOG(DPI_LOG_LEVEL_DEBUG,"tuple_info->sipv6=%pI6\n",tuple_info->sipv6);
	DPI_LOG(DPI_LOG_LEVEL_DEBUG,"tuple_info->dipv6=%pI6\n",tuple_info->dipv6);
	DPI_LOG(DPI_LOG_LEVEL_DEBUG,"tuple_info->sport=0x%x\n",ntohs(tuple_info->sport));
	DPI_LOG(DPI_LOG_LEVEL_DEBUG,"tuple_info->dport=0x%x\n",ntohs(tuple_info->dport));
	DPI_LOG(DPI_LOG_LEVEL_DEBUG,"tuple_info->in_iif=0x%x\n",tuple_info->in_iif);
	DPI_LOG(DPI_LOG_LEVEL_DEBUG,"tuple_info->out_iif=0x%x\n",tuple_info->out_iif);
	DPI_LOG(DPI_LOG_LEVEL_DEBUG,"=====================================================\n");

}

static int __netfilter_process(int hooknum, unsigned char pf, struct sk_buff *skb, int iif, int oif)
{
    int i = 0;
    int fincnt = 0;
    int ret = MAXNET_DPI_CON;
	int ret1, ret2;
    uint8_t *layer3_data = NULL;
    uint8_t *layer2_data = NULL;
    uint8_t direct = 0;
    tupleinfo_t tuple_info;
    enum ip_conntrack_info ctinfo;
    dpi_gwinfo_t gwinfo;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 21)
    struct nf_conn *ct;
    
    ct = nf_ct_get(skb, &ctinfo);
    layer2_data = (uint8_t *) skb_mac_header(skb);
    layer3_data = (uint8_t *) skb_network_header(skb);
#else
    struct ip_conntrack *ct;
    
    ct = ip_conntrack_get(skb, &ctinfo);
    layer2_data = (uint8_t *)skb->mac.raw;
    layer3_data = (uint8_t *)skb->nh.raw;
#endif
    memset(&tuple_info, 0x0, sizeof(tupleinfo_t));

    if (unlikely(ct == NULL)) {
        return MAXNET_DPI_CON;
    }

    if (ctinfo == IP_CT_NEW || ctinfo == IP_CT_RELATED) {
        tuple_info.new_conntrack = 1;
    }
    else {
        tuple_info.new_conntrack = 0;
    }

    direct = CTINFO2DIR(ctinfo);
    tuple_info.direct = direct;
    tuple_info.proto = ct->tuplehash[direct].tuple.dst.protonum;
    /*
      here, we assume that the inside user original the session; and so the dstip is lan_ip when direct == 1
      but, as we know when it used the nat, we should get the "original srcip" when acting statistics
    */
    if (direct == IP_CT_DIR_ORIGINAL) {
		//if(pf == AF_INET)
		{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 21)
	        tuple_info.sipv4 = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip;
	        tuple_info.dipv4 = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip;
#else
	        tuple_info.sipv4 = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip;
	        tuple_info.dipv4 = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip;
#endif
		}
		//else if(pf == AF_INET6)
		{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 21)
	        memcpy(tuple_info.sipv6, ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip6, 16);
	        memcpy(tuple_info.dipv6, ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip6, 16);
#else
	        memcpy(tuple_info.sipv6, ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip6, 16);
	        memcpy(tuple_info.dipv6, ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip6, 16);
#endif
		}
        /* port using network order */
        tuple_info.sport = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all;
        tuple_info.dport = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all;
    }
    else {
		//if(pf == AF_INET)
		{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 21)
	        tuple_info.sipv4 = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip;
	        tuple_info.dipv4 = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip;
#else
	        tuple_info.sipv4 = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip;
	        tuple_info.dipv4 = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip;
#endif
		}
		//else if(pf == AF_INET6)
		{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 21)
	        memcpy(tuple_info.sipv6, ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip6, 16);
	        memcpy(tuple_info.dipv6, ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip6, 16);
#else
	        memcpy(tuple_info.sipv6, ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip6, 16);
	        memcpy(tuple_info.dipv6, ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip6, 16);
#endif
		}
        /* port using network order */
        tuple_info.sport = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all;
        tuple_info.dport = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all;
    }
    
    tuple_info.in_iif = iif;
    tuple_info.out_iif = oif;
    //dump_tuple_info(&tuple_info);


    memset(&gwinfo, 0, sizeof(gwinfo));

    for (i = 0; i < NETFILTER_MAX_HOOKS; i++) {
        if (hooknum == NETFILTER_PRE_ROUTING) {
            if (g_prerouting_func[i] != NULL) {
                ret = (g_prerouting_func[i])(layer2_data, &tuple_info, ct->dpi_context, &gwinfo, NULL);
                if (ret == MAXNET_FDS_TRANS) {
                    /* dnat */
#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 3, 0)
                    nf_nat_setup_info(ct, &(gwinfo.nat_range), NF_NAT_MANIP_DST);
#elif LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 24)
                    nf_nat_setup_info(ct, &(gwinfo.nat_range), IP_NAT_MANIP_DST);
#else
                    ip_nat_setup_info(ct, &(gwinfo.nat_range), NF_IP_PRE_ROUTING);
#endif
                }else if (MAXNET_DPI_CON == ret){
					if(g_fwdByPS >= 0)
					{
						if(g_fwdByPS_mark == 2)
						{
#if defined(CONFIG_RTK_SKB_MARK2)
							skb->mark2 |= (1<<g_fwdByPS);
							DPI_LOG(DPI_LOG_LEVEL_INFO,"%s-%d:g_prerouting_func[%d]= %p, ret=%d, skb->mark2=%llx\n", __func__,__LINE__,i,g_prerouting_func[i],ret,skb->mark2);
#endif
						}
						else
						{
							skb->mark |= (1<<g_fwdByPS);
							DPI_LOG(DPI_LOG_LEVEL_INFO,"%s-%d:g_prerouting_func[%d]= %p, ret=%d, skb->mark=%x\n", __func__,__LINE__,i,g_prerouting_func[i],ret,skb->mark);
						}
					}
                }
            }
        }
        else if (hooknum == NETFILTER_FORWARD) {
            if (g_forward_func[i] != NULL) {
                ret = (g_forward_func[i])(layer2_data, &tuple_info, ct->dpi_context, &gwinfo, NULL);
                if (gwinfo.layer7_id != 0) {
                    ct->layer7_id = gwinfo.layer7_id;
                }

				/* 1-8	b01返回值，代表不同的操作类型	0～7 */
				ret1 = ret&0xFF;
                if (ret1 == MAXNET_DPI_FIN) {
		    		DPI_LOG(DPI_LOG_LEVEL_INFO,"%s-%d:g_forward_func[%d]= %p, forward finished\n", __func__,__LINE__,i,g_forward_func[i]);
                    fincnt++;
		    		dump_tuple_info(&tuple_info);

					if(ct->dpi_dscp[direct] > 0)
					{
						if(pf == AF_INET)
							ipv4_change_dsfield(ip_hdr(skb),
										(__force __u8)(~XT_DSCP_MASK),
										((ct->dpi_dscp[direct])&XT_DSCP_MAX) << XT_DSCP_SHIFT);
						else if(pf == AF_INET6)
							ipv6_change_dsfield(ipv6_hdr(skb),
										(__force __u8)(~XT_DSCP_MASK),
										((ct->dpi_dscp[direct])&XT_DSCP_MAX) << XT_DSCP_SHIFT);
						DPI_LOG(DPI_LOG_LEVEL_INFO, "%s-%d, dscp continue remark:%d\n", __func__,__LINE__,((ct->dpi_dscp[direct])&XT_DSCP_MAX));
					}
                }
				else if (MAXNET_DPI_CON == ret1){
					if(g_fwdByPS >= 0)
					{
						if(g_fwdByPS_mark == 2)
						{
#if defined(CONFIG_RTK_SKB_MARK2)
							skb->mark2 |= (1<<g_fwdByPS);
							DPI_LOG(DPI_LOG_LEVEL_INFO, "%s-%d, continue ret=%d mark2=%llx\n", __func__,__LINE__,ret, skb->mark2);
#endif
						}
						else
						{
							skb->mark |= (1<<g_fwdByPS);
							DPI_LOG(DPI_LOG_LEVEL_INFO, "%s-%d, continue ret=%d mark=%x\n", __func__,__LINE__,ret, skb->mark);
						}
					}
				}
				else if (MAXNET_DPI_PQOS == ret1)
				{
					if(g_fwdQoS >= 0)
					{
						/* 9-16	根据不同操作类型，代表含义不一致	0～255 */
						ret2 = (ret>>8)&0xFF;
						if (ret2 <= 0)
							ret2 = g_fwdQoS_qNum-1;	//highest prio
						else if (ret2>g_fwdQoS_qNum)
							ret2 = 0;	//lowest prio
						else
							ret2 = g_fwdQoS_qNum-ret2;
						if(g_fwdQoS_mark == 2)
						{
#if defined(CONFIG_RTK_SKB_MARK2)
							skb->mark2 &= ~(0x7<<g_fwdQoS);
							skb->mark2 |= (ret2<<g_fwdQoS);
							DPI_LOG(DPI_LOG_LEVEL_INFO, "%s-%d, qos ret=%d mark2=%llx\n", __func__,__LINE__,ret, skb->mark2);
#endif
						}
						else
						{
							skb->mark &= ~(0x7<<g_fwdQoS);
							skb->mark |= (ret2<<g_fwdQoS);
							DPI_LOG(DPI_LOG_LEVEL_INFO, "%s-%d, qos ret=%d mark=%x\n", __func__,__LINE__,ret, skb->mark);
						}
					}
				}
				else if (MAXNET_DPI_PDSCP == ret1)
				{
					u_int8_t dscp=0;
					if(pf == AF_INET)
						dscp = ipv4_get_dsfield(ip_hdr(skb)) >> XT_DSCP_SHIFT;
					else if(pf == AF_INET6)
						dscp = ipv6_get_dsfield(ipv6_hdr(skb)) >> XT_DSCP_SHIFT;

					ret2 = (ret>>8)&0x3F;

					if (ret2 != dscp) {
						if(pf == AF_INET)
							ipv4_change_dsfield(ip_hdr(skb),
										(__force __u8)(~XT_DSCP_MASK),
										ret2 << XT_DSCP_SHIFT);
						else if(pf == AF_INET6)
							ipv6_change_dsfield(ipv6_hdr(skb),
										(__force __u8)(~XT_DSCP_MASK),
										ret2 << XT_DSCP_SHIFT);
						ct->dpi_dscp[direct] = ret2|(1<<7);

					}
					DPI_LOG(DPI_LOG_LEVEL_INFO, "%s-%d, dscp ret2=%d orig_dscp=%d\n", __func__,__LINE__,ret2, dscp);
#if defined(CONFIG_RTK_SKB_MARK2)
					// Set skbmark2 for RTK DSCP DS/US remark
					if (direct == IP_CT_DIR_ORIGINAL) {
						if (g_dpi_dscp_us_remark_start_bit > 0)
							skb->mark2 |= (1LL << g_dpi_dscp_us_remark_start_bit);
						if (g_dpi_dscp_us_remark_range_start_bit > 0)
							skb->mark2 |= ((unsigned long long)ret2 << g_dpi_dscp_us_remark_range_start_bit);
					} else {
						if (g_dpi_dscp_ds_remark_start_bit > 0)
							skb->mark2 |= (1LL << g_dpi_dscp_ds_remark_start_bit);
						if (g_dpi_dscp_ds_remark_range_start_bit > 0)
							skb->mark2 |= ((unsigned long long)ret2 << g_dpi_dscp_ds_remark_range_start_bit);
					} 

					DPI_LOG(DPI_LOG_LEVEL_INFO, "%s-%d, skbmark2=%lld, 0x%llx\n", __func__,__LINE__, skb->mark2, skb->mark2);
#endif
					if (g_appCtrlLog_func != NULL)
					{
						g_appCtrlLog_func(pf, 1, skb);
					}
				}
            }
        }
    }
    
    if (fincnt == g_forward_count) {
        /* TODO: all hook finish, help yourself */
    	DPI_LOG(DPI_LOG_LEVEL_INFO,"%s-%d all forward hook finish, help yourself g_forward_count=%lu\n", __func__,__LINE__,g_forward_count);
    }

    return ret;
}

static unsigned int __netfilter_core_process(struct sk_buff *skb,
        const struct net_device *in,
        const struct net_device *out,
        unsigned char proto, int hooknum)
{
    int ret = 0;

    if (hooknum == NETFILTER_PRE_ROUTING) {
        ret = __netfilter_process(hooknum, proto, skb, in->ifindex, 0);
    }
    else {
        ret = __netfilter_process(hooknum, proto, skb, in->ifindex, out->ifindex);
    }
    if (ret == MAXNET_DPI_DNY) {
		if (g_appCtrlLog_func != NULL)
		{
			g_appCtrlLog_func(proto, 2, skb);
		}
        return NF_DROP;
    }

    return NF_ACCEPT;
}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0)
static unsigned int __prerouting_process(void *priv,
    struct sk_buff *skb,
    const struct nf_hook_state *state)
{
	if (unlikely(g_dpi_func == 0)) {
        return NF_ACCEPT;
    }
	
	DPI_LOG(DPI_LOG_LEVEL_DEBUG, "%s-%d: pf=%u in->name=%s, in->ifindex=0x%x,out->name=%s, out->ifindex=0x%x\n", __func__,__LINE__,state->pf, state->in->name,state->in->ifindex,state->out->name,state->out->ifindex);
	return __netfilter_core_process(skb, state->in, state->out, state->pf, NETFILTER_PRE_ROUTING);

}
static unsigned int __forward_process(void *priv,
    struct sk_buff *skb,
    const struct nf_hook_state *state)
{
	if (unlikely(g_dpi_func == 0)) {
        return NF_ACCEPT;
    }
	
	DPI_LOG(DPI_LOG_LEVEL_DEBUG, "%s-%d: pf=%u in->name=%s, in->ifindex=0x%x,out->name=%s, out->ifindex=0x%x\n", __func__,__LINE__,state->pf, state->in->name,state->in->ifindex,state->out->name,state->out->ifindex);
    return __netfilter_core_process(skb, state->in, state->out, state->pf, NETFILTER_FORWARD);
}

#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 0)
static unsigned int __prerouting_process(const struct nf_hook_ops *ops,
        struct sk_buff *skb,
        const struct net_device *in,
        const struct net_device *out,
        int (*okfn)(struct sk_buff *))
{
	if (unlikely(g_dpi_func == 0)) {
        return NF_ACCEPT;
    }
	
    return __netfilter_core_process(skb, in, out, ops->pf, NETFILTER_PRE_ROUTING);
}

static unsigned int __forward_process(const struct nf_hook_ops *ops,
        struct sk_buff *skb,
        const struct net_device *in,
        const struct net_device *out,
        int (*okfn)(struct sk_buff *))
{
	if (unlikely(g_dpi_func == 0)) {
        return NF_ACCEPT;
    }
	
    return __netfilter_core_process(skb, in, out, ops->pf, NETFILTER_FORWARD);
}
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24)
static unsigned int __prerouting_ipv4_process(unsigned int hooknum,
        struct sk_buff *skb,
        const struct net_device *in,
        const struct net_device *out,
        int (*okfn)(struct sk_buff *))
{
	if (unlikely(g_dpi_func == 0)) {
        return NF_ACCEPT;
    }
	
    return __netfilter_core_process(skb, in, out, AF_INET, NETFILTER_PRE_ROUTING);
}

static unsigned int __forward_ipv4_process(unsigned int hooknum,
        struct sk_buff *skb,
        const struct net_device *in,
        const struct net_device *out,
        int (*okfn)(struct sk_buff *))
{
	if (unlikely(g_dpi_func == 0)) {
        return NF_ACCEPT;
    }

    return __netfilter_core_process(skb, in, out, AF_INET, NETFILTER_FORWARD);
}
#ifdef CONFIG_IPV6
static unsigned int __prerouting_ipv6_process(unsigned int hooknum,
        struct sk_buff *skb,
        const struct net_device *in,
        const struct net_device *out,
        int (*okfn)(struct sk_buff *))
{
	if (unlikely(g_dpi_func == 0)) {
        return NF_ACCEPT;
    }

    return __netfilter_core_process(skb, in, out, AF_INET6, NETFILTER_PRE_ROUTING);
}

static unsigned int __forward_ipv6_process(unsigned int hooknum,
        struct sk_buff *skb,
        const struct net_device *in,
        const struct net_device *out,
        int (*okfn)(struct sk_buff *))
{
	if (unlikely(g_dpi_func == 0)) {
        return NF_ACCEPT;
    }

    return __netfilter_core_process(skb, in, out, AF_INET6, NETFILTER_FORWARD);
}
#endif
#else
static unsigned int __prerouting_ipv4_process(unsigned int hooknum,
        struct sk_buff **pskb,
        const struct net_device *in,
        const struct net_device *out,
        int (*okfn)(struct sk_buff *))
{
    struct sk_buff *skb = *pskb;
	if (unlikely(g_dpi_func == 0)) {
        return NF_ACCEPT;
    }
    return __netfilter_core_process(skb, in, out, AF_INET, NETFILTER_PRE_ROUTING);
}

static unsigned int __forward_ipv4_process(unsigned int hooknum,
        struct sk_buff **pskb,
        const struct net_device *in,
        const struct net_device *out,
        int (*okfn)(struct sk_buff *))
{
    struct sk_buff *skb = *pskb;
	if (unlikely(g_dpi_func == 0)) {
        return NF_ACCEPT;
    }
    return __netfilter_core_process(skb, in, out, AF_INET, NETFILTER_FORWARD);
}
#ifdef CONFIG_IPV6
static unsigned int __prerouting_ipv6_process(unsigned int hooknum,
        struct sk_buff **pskb,
        const struct net_device *in,
        const struct net_device *out,
        int (*okfn)(struct sk_buff *))
{
    struct sk_buff *skb = *pskb;
	if (unlikely(g_dpi_func == 0)) {
        return NF_ACCEPT;
    }
    return __netfilter_core_process(skb, in, out, AF_INET6, NETFILTER_PRE_ROUTING);
}

static unsigned int __forward_ipv6_process(unsigned int hooknum,
        struct sk_buff **pskb,
        const struct net_device *in,
        const struct net_device *out,
        int (*okfn)(struct sk_buff *))
{
    struct sk_buff *skb = *pskb;
	if (unlikely(g_dpi_func == 0)) {
        return NF_ACCEPT;
    }
    return __netfilter_core_process(skb, in, out, AF_INET6, NETFILTER_FORWARD);
}

#endif
#endif

static int register_prerouting_hook(ctSgw_appPreProc hook, int priority, void **priv_data)
{
    int i = 0;
    netfilter_priv_info_t **info = NULL;
    
    if (hook == NULL) {
        return -1;
    }

    for (i = 0; i < NETFILTER_MAX_HOOKS; i++) {
        if (g_prerouting_func[i] == NULL) {
            break;
        }
    }

    if (i == NETFILTER_MAX_HOOKS) {
        DPI_LOG(DPI_LOG_LEVEL_ERROR, "%s: prerouting has max hooks\n", __func__);
        return -1;
    }

    if (!priv_data) {
        DPI_LOG(DPI_LOG_LEVEL_ERROR,"%s: priv data not init\n", __func__);
        return -1;
    }

    info = (netfilter_priv_info_t **)priv_data;
    *info = (netfilter_priv_info_t *)kmalloc(sizeof(netfilter_priv_info_t), GFP_ATOMIC);
    if (*info == NULL) {
        DPI_LOG(DPI_LOG_LEVEL_ERROR,"%s: malloc netfilter priv info failed\n", __func__);
        return -1;
    }

    memset(*info, 0, sizeof(netfilter_priv_info_t));
    (*info)->index = i + 1;
    (*info)->hooknum = NETFILTER_PRE_ROUTING;

    (*info)->ops = (struct nf_hook_ops *)kmalloc(sizeof(struct nf_hook_ops), GFP_ATOMIC);
    if ((*info)->ops == NULL) {
        DPI_LOG(DPI_LOG_LEVEL_ERROR,"%s: malloc nf_hook_ops failed\n", __func__);
        kfree(*info);
        *info = NULL;
        return -1;
    }

    memset((*info)->ops, 0, sizeof(struct nf_hook_ops));
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 0)
	(*info)->ops->hook = __prerouting_process;
#else
    (*info)->ops->hook = __prerouting_ipv4_process;
#endif
    //(*info)->ops->owner = THIS_MODULE;
    (*info)->ops->pf = PF_INET;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 21)
    (*info)->ops->hooknum = NF_INET_PRE_ROUTING;
#else
    (*info)->ops->hooknum = NF_IP_PRE_ROUTING;
#endif
    (*info)->ops->priority = priority;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,13,0)
	if (nf_register_net_hook(&init_net, (*info)->ops)) {
#else
	if (nf_register_hook((*info)->ops)) {
#endif
        DPI_LOG(DPI_LOG_LEVEL_ERROR,"%s: nf_register_hook failed\n", __func__);
        kfree((*info)->ops);
        kfree(*info);
        *info = NULL;
        g_prerouting_func[i] = NULL;
        return -1;
    }
#ifdef CONFIG_IPV6
	(*info)->opsv6 = (struct nf_hook_ops *)kmalloc(sizeof(struct nf_hook_ops), GFP_ATOMIC);
    if ((*info)->opsv6 == NULL) {
        DPI_LOG(DPI_LOG_LEVEL_ERROR,"%s: malloc nf_hook_ops failed for v6\n", __func__);
    }
	else
	{
	    memset((*info)->opsv6, 0, sizeof(struct nf_hook_ops));
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 0)
		(*info)->opsv6->hook = __prerouting_process;
#else
	    (*info)->opsv6->hook = __prerouting_ipv6_process;
#endif
	    //(*info)->ops->owner = THIS_MODULE;
	    (*info)->opsv6->pf = PF_INET6;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 21)
	    (*info)->opsv6->hooknum = NF_INET_PRE_ROUTING;
#else
	    (*info)->opsv6->hooknum = NF_IP_PRE_ROUTING;
#endif
	    (*info)->opsv6->priority = priority;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,13,0)
		if (nf_register_net_hook(&init_net, (*info)->opsv6)) {
#else
		if (nf_register_hook((*info)->opsv6)) {
#endif
	        DPI_LOG(DPI_LOG_LEVEL_ERROR,"%s: nf_register_hook failed for v6\n", __func__);
	        kfree((*info)->opsv6);
			(*info)->opsv6=NULL;
	    }
	}
#endif
	g_prerouting_func[i] = hook;
    g_prerouting_count++;
    DPI_LOG(DPI_LOG_LEVEL_INFO, "%s-%d registered prerouting hook priority:%d ok g_prerouting_count=%lu\n", __func__,__LINE__,priority,g_prerouting_count);

    return 0;
}

static int register_forward_hook(ctSgw_appForwardProc hook, int priority, void **priv_data)
{
    int i = 0;
    netfilter_priv_info_t **info = NULL;

    for (i = 0; i < NETFILTER_MAX_HOOKS; i++) {
        if (g_forward_func[i] == NULL) {
            break;
        }
    }

    if (i == NETFILTER_MAX_HOOKS) {
        DPI_LOG(DPI_LOG_LEVEL_ERROR,"%s: forward has max hooks\n", __func__);
        return -1;
    }

    if (!priv_data) {
        DPI_LOG(DPI_LOG_LEVEL_ERROR,"%s: priv data not init\n", __func__);
        return -1;
    }

    info = (netfilter_priv_info_t **)priv_data;
    *info = (netfilter_priv_info_t *)kmalloc(sizeof(netfilter_priv_info_t), GFP_ATOMIC);
    if (*info == NULL) {
        DPI_LOG(DPI_LOG_LEVEL_ERROR,"%s: malloc netfilter priv info failed\n", __func__);
        return -1;
    }

    memset(*info, 0, sizeof(netfilter_priv_info_t));
    (*info)->index = i + 1;
    (*info)->hooknum = NETFILTER_FORWARD;

    (*info)->ops = (struct nf_hook_ops *)kmalloc(sizeof(struct nf_hook_ops), GFP_ATOMIC);
    if ((*info)->ops == NULL) {
        DPI_LOG(DPI_LOG_LEVEL_ERROR,"%s: malloc nf_hook_ops failed\n", __func__);
        kfree(*info);
        *info = NULL;
        return -1;
    }

    memset((*info)->ops, 0, sizeof(struct nf_hook_ops));
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 0)
	(*info)->ops->hook = __forward_process;
#else
    (*info)->ops->hook = __forward_ipv4_process;
#endif
    //(*info)->ops->owner = THIS_MODULE;
    (*info)->ops->pf = PF_INET;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 21)
    (*info)->ops->hooknum = NF_INET_FORWARD;
#else
    (*info)->ops->hooknum = NF_IP_FORWARD;
#endif
    (*info)->ops->priority = priority;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,13,0)
	if (nf_register_net_hook(&init_net, (*info)->ops)) {
#else
	if (nf_register_hook((*info)->ops)) {
#endif
        DPI_LOG(DPI_LOG_LEVEL_ERROR,"%s: nf_register_hook failed\n", __func__);
        kfree((*info)->ops);
        kfree(*info);
        *info = NULL;
        return -1;
    }
	
#ifdef CONFIG_IPV6
	(*info)->opsv6 = (struct nf_hook_ops *)kmalloc(sizeof(struct nf_hook_ops), GFP_ATOMIC);
	if ((*info)->opsv6 == NULL) {
		DPI_LOG(DPI_LOG_LEVEL_ERROR,"%s: malloc nf_hook_ops failed for v6\n", __func__);
	}
	else
	{
		memset((*info)->opsv6, 0, sizeof(struct nf_hook_ops));
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 0)
		(*info)->opsv6->hook = __forward_process;
#else
		(*info)->opsv6->hook = __forward_ipv6_process;
#endif
		//(*info)->ops->owner = THIS_MODULE;
		(*info)->opsv6->pf = PF_INET6;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 21)
		(*info)->opsv6->hooknum = NF_INET_FORWARD;
#else
		(*info)->opsv6->hooknum = NF_IP_FORWARD;
#endif
		(*info)->opsv6->priority = priority;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,13,0)
		if (nf_register_net_hook(&init_net, (*info)->opsv6)) {
#else
		if (nf_register_hook((*info)->opsv6)) {
#endif
			DPI_LOG(DPI_LOG_LEVEL_ERROR,"%s: nf_register_hook failed for v6\n", __func__);
			kfree((*info)->opsv6);
			(*info)->opsv6=NULL;
		}
	}
#endif
	g_forward_func[i] = hook;
    g_forward_count++;
    DPI_LOG(DPI_LOG_LEVEL_INFO, "%s-%d registered forward hook priority:%d ok g_forward_count=%lu\n", __func__,__LINE__,priority,g_forward_count);

    return 0;
}

static void unregister_netfilter_hook(void **priv_data)
{
    netfilter_priv_info_t *info = NULL;
    
    if (!priv_data) {
        DPI_LOG(DPI_LOG_LEVEL_ERROR,"%s: priv data not init\n", __func__);
        return;
    }

    info = *((netfilter_priv_info_t **)priv_data);
    if (info == NULL) {
        DPI_LOG(DPI_LOG_LEVEL_ERROR,"%s: info is null\n", __func__);
        return;
    }

    if (info->ops == NULL 
#ifdef CONFIG_IPV6
		&& info->opsv6 == NULL
#endif
		) {
        DPI_LOG(DPI_LOG_LEVEL_ERROR,"%s: info ops is null\n", __func__);
        kfree(info);
        *priv_data = NULL;
        return;
    }

    if (info->index < 1 || info->index > NETFILTER_MAX_HOOKS) {
        DPI_LOG(DPI_LOG_LEVEL_ERROR,"%s: info index %d\n", __func__, info->index);
		if(info->ops)
		{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,13,0)
			nf_unregister_net_hook(&init_net, info->ops);
#else
			nf_unregister_hook(info->ops);
#endif
	        kfree(info->ops);
		}
#ifdef CONFIG_IPV6
		if(info->opsv6)
		{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,13,0)
			nf_unregister_net_hook(&init_net, info->opsv6);
#else
			nf_unregister_hook(info->opsv6);
#endif
	        kfree(info->opsv6);
		}
#endif
        kfree(info);
        *priv_data = NULL;
        return;
    }
    
    if (info->hooknum == NETFILTER_PRE_ROUTING) {
        g_prerouting_func[info->index - 1] = NULL;
	g_prerouting_count--;
    }
    else if (info->hooknum == NETFILTER_FORWARD) {
        g_forward_count--;
        g_forward_func[info->index - 1] = NULL;
    }
	if(info->ops)
	{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,13,0)
		nf_unregister_net_hook(&init_net, info->ops);
#else
	    nf_unregister_hook(info->ops);
#endif
	    kfree(info->ops);
	}
#ifdef CONFIG_IPV6
	if(info->opsv6)
	{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,13,0)
		nf_unregister_net_hook(&init_net, info->opsv6);
#else
        nf_unregister_hook(info->opsv6);
#endif
        kfree(info->opsv6);
	}
#endif
    kfree(info);

    *priv_data = NULL;
    return;
}

int ctSgw_appRegisterGetInfo(ctSgw_appGetInfoProc funcs)
{
    if (funcs == NULL) {
        return -1;
    }

    g_getinfo_func = funcs;
    return 0;
}

void ctSgw_appUnRegisterGetInfo(void)
{
    g_getinfo_func = NULL;
    return;
}

int ctSgw_appGetInfo(void *app_ctx, void *app_info)
{
    if (g_getinfo_func == NULL) {
        return -1;
    }

    return g_getinfo_func(app_ctx, app_info);
}

int ctSgw_appRegisterPreProc(CtSgwPreProcInfo *info)
{
    if (info == NULL) {
        return -1;
    }

    return register_prerouting_hook(info->hook, info->priority, &(info->priv_data));
}

void ctSgw_appUnRegisterPreProc(CtSgwPreProcInfo *info)
{
    if (info != NULL) {
        unregister_netfilter_hook(&(info->priv_data));
    }
    return;
}

int ctSgw_appRegisterForwardProc(CtSgwForwardProcInfo *info)
{
    if (info == NULL) {
        return -1;
    }

    return register_forward_hook(info->hook, info->priority, &(info->priv_data));
}

void ctSgw_appUnRegisterForwardProc(CtSgwForwardProcInfo *info)
{
    if (info != NULL) {
        unregister_netfilter_hook(&(info->priv_data));
    }
    return;
}

int appCtrlRegisterSaveLog(appCtrlLogProc funcs)
{
    if (funcs == NULL) {
        return -1;
    }

    g_appCtrlLog_func = funcs;
    return 0;
}

void appCtrlUnRegisterSaveLog(void)
{
    g_appCtrlLog_func = NULL;
    return;
}

int ctSgw_kapiVer(void)
{
	//support dscp and filter func
	return 1;
}

EXPORT_SYMBOL(ctSgw_kapiVer);

EXPORT_SYMBOL(ctSgw_appRegisterGetInfo);
EXPORT_SYMBOL(ctSgw_appUnRegisterGetInfo);
EXPORT_SYMBOL(ctSgw_appGetInfo);

EXPORT_SYMBOL(ctSgw_appRegisterPreProc);
EXPORT_SYMBOL(ctSgw_appUnRegisterPreProc);

EXPORT_SYMBOL(ctSgw_appRegisterForwardProc);
EXPORT_SYMBOL(ctSgw_appUnRegisterForwardProc);

EXPORT_SYMBOL(appCtrlRegisterSaveLog);
EXPORT_SYMBOL(appCtrlUnRegisterSaveLog);


MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("maxnet");


