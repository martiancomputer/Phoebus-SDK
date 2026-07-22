/* SPDX-License-Identifier: Unlicense */
#ifndef _RE_RECYCLESKB_H_
#define _RE_RECYCLESKB_H_

#include <linux/skbuff.h>
#include <linux/interrupt.h>
#ifdef CONFIG_RTK_PTOOL_CPU_PERF
#include <linux/ptool.h>		/*performance*/
#endif
#include <linux/kernel.h>		/*dump_stack*/
#include <linux/mm.h>			/*get total memory size*/
#include <linux/vmstat.h>
#include <linux/highmem.h>

#if defined(CONFIG_LUNA_G3_SERIES)
#include <ca_ni.h>
#include <ca_ext.h>

#define MBUF_LEN	CA_NI_SKB_ALLOC_DATA_SIZE
#else
#include "../nicDriver/re8686_rtl9607c.h"

#define MBUF_LEN        SKB_BUF_SIZE
#endif

#if defined(RTL_PRIV_DATA_SIZE) && RTL_PRIV_DATA_SIZE > 0
/*define reserve 0,
 *because alloc_skb will append RTL_PRIV_DATA_SIZE,
 *so share it
 */
#define RTL_RESERVED_HEADER_SIZE	0
#else
#define RTL_RESERVED_HEADER_SIZE	128
#endif
#if defined(CONFIG_LUNA_G3_SERIES)
#define CROSS_LAN_MBUF_LEN		(MBUF_LEN)
#else
#define CROSS_LAN_MBUF_LEN		(MBUF_LEN + 16)
#endif

#if defined(CONFIG_CMCC)
#define MAX_PRE_ALLOC_RX_SKB	20000
#else
#define MAX_PRE_ALLOC_RX_SKB	5000
#endif

#define MAX_ETH_SKB_NUM	(MAX_PRE_ALLOC_RX_SKB + 600)
#define MAX_CRITICAL_ETH_SKB_NUM 256

#define ETH_RECYCLE_SKB_PROC	1

#if	defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
#define RTL_SKB_CB_SECTION_MAX		48
#define RTL_SKB_CB_SECTION_LOW		32
#define RTL_SKB_CB_SECTION_HIGH		40
#define RTL_SKB_CB_SECTION_ENTER	1
#define RTL_SKB_CB_SECTION_LEAVE	0
#endif

enum {
	RTL_RECYCLE_SWITCH_ON = 0x0,
	RTL_RECYCLE_SWITCH_OFF = 0x1,
};

#if defined(RTL_RING_BUFFER_RECYCLE_SKB)
enum {
	RTL_RING_BUF_RECYCLE_NORMAL = 0x0,
	RTL_RING_BUF_RECYCLE_EMPTY,
	RTL_RING_BUF_RECYCLE_FULL,
};

struct ring_buffer_recycle_t {
	unsigned int rd_idx;
	unsigned int wr_idx;
	unsigned int state;	/*0: normal, 1:empty, 2:full*/
	unsigned int ring_size_mask;

	struct sk_buff **ring_buffer_recycle_array;
	/*lock: lock to manipulate array*/
	spinlock_t	lock;
};
#endif
#if defined(RTL_STRAIGHT_ARRAY_RECYCLE_SKB)
struct straight_recycle_t {
	unsigned int idx;
	struct sk_buff **array;
	/*lock: lock to manipulate array*/
	spinlock_t	lock;
};
#endif
#if	defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
typedef struct rtl_recycle_skb_debug_s {
	struct sk_buff *check_skb;
	int cb;

	int skb_pointer_off;	/*-1 to disable check*/
	int headroom;			/*-1 to disable check*/
	int dump_stack_en;		/*-1 to disable check*/
}rtl_recycle_skb_debug_t;
#endif

#if defined(CONFIG_PREEMPT)
#define RECYCLE_SPINLOCK(x) spin_lock_bh(x)
#define RECYCLE_SPINUNLOCK(x) spin_unlock_bh(x)
#else
#define RECYCLE_SPINLOCK(x) spin_lock(x)
#define RECYCLE_SPINUNLOCK(x) spin_unlock(x)
#endif

int init_recycle_eth_skb_buf(unsigned int ring_size, unsigned int max_size, int forced_ring_size, int forced_new_pool);
void deinit_recycle_eth_skb_buf(int pool);
int lookup_recycle_eth_skb_pool(unsigned int size);
struct sk_buff *dev_alloc_skb_recy_eth(unsigned int size, int pool);
struct sk_buff *recycle_skb_swap(struct sk_buff *skb);

extern int min_free_kbytes;
extern struct proc_dir_entry *realtek_proc;

static inline void __init_skb_recy_eth(struct sk_buff *skb)
{
	/*reserve header room*/
	skb->data = skb->head + RTL_RESERVED_HEADER_SIZE;
	skb_reset_tail_pointer(skb);
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
	/*Clear identicated amsdu offload cb while refill skb.
	 *_SKB_CB_ETH_AGG=32 defined @ g6_wifi_driver/include/xmit_osdep.h
	 */
	skb->cb[32] = 0x0;
	skb->cb[33] = 0x0;
#endif
#ifdef CONFIG_SKB_EXTENSIONS
	/* only useable after checking ->active_extensions != 0 */
	skb->extensions = NULL;
	skb->active_extensions = 0;
#endif

#if	defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
	skb->skb_bak = skb;
	skb->head_bak = skb->head;
#endif
}

static inline void __recycle_skb_copy(struct sk_buff *skb, struct sk_buff *new_skb)
{
	unsigned char *head;
	sk_buff_data_t end;
	unsigned int truesize;
	bool pfmemalloc, cloned;
#if defined(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE)
	if (skb->fcIngressData.doLearning)
		memcpy(&new_skb->fcIngressData, &skb->fcIngressData, sizeof(rtk_fc_ingress_data_t));
	else
		new_skb->fcIngressData.traceFilterMatchShow = skb->fcIngressData.traceFilterMatchShow;
#endif

	head = new_skb->head;
	end = new_skb->end;
	truesize = new_skb->truesize;
	cloned = new_skb->cloned;
	pfmemalloc = new_skb->pfmemalloc;

	new_skb->head = skb->head;
	new_skb->data = skb->data;
	new_skb->tail = skb->tail;
	new_skb->end = skb->end;
	new_skb->truesize = skb->truesize;
	new_skb->cloned = skb->cloned;
	new_skb->pfmemalloc = skb->pfmemalloc;
	new_skb->protocol = skb->protocol;
	new_skb->mac_header = skb->mac_header;
	new_skb->len = skb->len;
	new_skb->dev = skb->dev;
#ifdef CONFIG_RTK_CONTROL_PACKET_PROTECTION
	new_skb->priority = skb->priority;
#endif
	new_skb->mark = skb->mark;
#ifdef CONFIG_RTK_SKB_MARK2
	new_skb->mark2 = skb->mark2;
#endif
#ifdef CONFIG_RTL8192CD
	new_skb->cb[0] = skb->cb[0];
#endif
#ifdef CONFIG_SKB_EXTENSIONS
	/* only useable after checking ->active_extensions != 0 */
	new_skb->extensions = skb->extensions;
	new_skb->active_extensions = skb->active_extensions;
#endif
	/* keep original ip checksum status to protocol stack */
	new_skb->ip_summed = skb->ip_summed;

	skb->head = head;
	skb->end = end;
	skb->truesize = truesize;
	skb->cloned = cloned;
	skb->pfmemalloc = pfmemalloc;
	/*for jumbo packet should clear it here*/
	skb->data_len = 0;
#if	defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
	skb->skb_bak = skb;
	skb->head_bak = head;
#endif
}

static inline void __recycle_rtl_eth_buf(struct sk_buff *skb)
{
	void recycle_skb_clean(struct sk_buff *skb);
	if (skb->cloned)
		recycle_skb_clean(skb);
#if	defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
	if (skb->double_check_free == 0)
		pr_err("%s %d ERROR!!! double check free should be one...skb=%llx\n", __func__, __LINE__, skb);
	else
		skb->double_check_free = 0;
	{
		rtl_recycle_skb_debug_t rtl_skb_debug_parameter;

		rtl_skb_debug_parameter.cb = rtl_recycle_skb_cb_value;
		rtl_skb_debug_parameter.check_skb = skb;
		rtl_skb_debug_parameter.skb_pointer_off = -1;
		rtl_skb_debug_parameter.headroom = -1;
		rtl_skb_debug_parameter.dump_stack_en = 1;
		rtl_recycle_skb_cb_section_enter(&rtl_skb_debug_parameter);
	}
#endif
}

static inline void __recycle_frag_list(struct sk_buff *skb, void (*skb_queue_head)(struct sk_buff *skb, int pool), __u16 pool)
{
	if (skb->frag_list_recycle) {
		/*free frag_list and it's next till NULL*/
		struct sk_buff *tmp_next, *NEXT_free = skb_shinfo(skb)->frag_list;

		while (NEXT_free) {
			tmp_next = NEXT_free->next;
			NEXT_free->next = NULL;
			__recycle_rtl_eth_buf(NEXT_free);
			skb_queue_head(NEXT_free, pool);
			NEXT_free = tmp_next;
		}
		skb_shinfo(skb)->frag_list = NULL;
		skb->data_len = 0;
		skb->frag_list_recycle = 0;
	}
}

#if defined(CONFIG_LUNA_G3_SERIES)
#define __recycle_rtl_dev_alloc_skb(size)		__netdev_alloc_skb(NULL, (size), GFP_ATOMIC | GFP_DMA)
#else
#define __recycle_rtl_dev_alloc_skb(size)		__netdev_alloc_skb(NULL, (size) + RTL_RESERVED_HEADER_SIZE, GFP_ATOMIC | GFP_DMA)
#endif

#if	defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
int rtl_recycle_skb_cb_section_register(void);
int rtl_recycle_skb_cb_section_unregister(unsigned int cb);
int rtl_recycle_skb_cb_section_enter(rtl_recycle_skb_debug_t *rtl_skb_debug);
int rtl_recycle_skb_cb_section_check(rtl_recycle_skb_debug_t *rtl_skb_debug);
int rtl_recycle_skb_cb_section_leave(rtl_recycle_skb_debug_t *rtl_skb_debug);
#endif

#endif /*_RE_RECYCLESKB_H_*/
