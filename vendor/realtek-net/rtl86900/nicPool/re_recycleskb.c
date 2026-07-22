// SPDX-License-Identifier: Unlicense
#ifdef CONFIG_RTL_ETH_RECYCLED_SKB

#include "re_recycleconf.h"
#include "re_recycleskb.h"
#include <common/rt_type.h>
#include "module/intr_bcaster/intr_bcaster.h"

#ifdef RTK_QUE
struct ring_que {
	int qlen;
	int qmax;
	int head;
	int tail;
	struct sk_buff *ring[MAX_PRE_ALLOC_RX_SKB + 1];
};

static struct ring_que rx_skb_queue;
#else
static struct sk_buff_head rx_skb_queue;
#endif

#if defined(RTL_RING_BUFFER_RECYCLE_SKB)
static __cacheline_aligned_in_smp struct ring_buffer_recycle_t rbr;
#endif
#if defined(RTL_DOUBLE_LINK_LIST_RECYCLE_SKB) || defined(RTL_SINGLE_LINK_LIST_RECYCLE_SKB)
static __cacheline_aligned_in_smp struct sk_buff_head recycle_skb_queue[1 << RTL_POOL_NUM_SHIFT_BITS];
#endif
#if defined(RTL_STRAIGHT_ARRAY_RECYCLE_SKB)
static __cacheline_aligned_in_smp struct straight_recycle_t sra;
#endif
#if defined(RTL_LL_LINK_LIST_RECYCLE_SKB)
static __cacheline_aligned_in_smp spinlock_t llll_read_lock[1 << RTL_POOL_NUM_SHIFT_BITS];
static __cacheline_aligned_in_smp struct llist_head llll_skb_head[1 << RTL_POOL_NUM_SHIFT_BITS];
static __cacheline_aligned_in_smp atomic_t llll_num[1 << RTL_POOL_NUM_SHIFT_BITS];
#endif

static unsigned int eth_skb_total_num[1 << RTL_POOL_NUM_SHIFT_BITS];
static unsigned int eth_skb_free_num[1 << RTL_POOL_NUM_SHIFT_BITS];
static unsigned int eth_skb_max_size[1 << RTL_POOL_NUM_SHIFT_BITS] = {0};
#if defined(CONFIG_RTL_ETH_RECYCLED_SKB_ALLOC_MAX)
static unsigned int eth_skb_max_allocate[1 << RTL_POOL_NUM_SHIFT_BITS] = {0};
#endif
static unsigned int eth_skb_pool[1 << RTL_POOL_NUM_SHIFT_BITS] = {0};
unsigned int switch_off_recycle_skb = RTL_RECYCLE_SWITCH_ON;
EXPORT_SYMBOL(switch_off_recycle_skb);

#if	defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
static int rtl_recycle_skb_cb_value = -1;
static DEFINE_SPINLOCK(rtl_recycle_skb_cb_lock);
static unsigned char rtl_recycle_skb_cb_check[RTL_SKB_CB_SECTION_MAX] = {0};

static inline void __init_skb_recy_eth_debug(struct sk_buff *skb)
{
	rtl_recycle_skb_debug_t rtl_skb_debug_parameter;

	skb->double_check_free = 0;

	rtl_skb_debug_parameter.cb = rtl_recycle_skb_cb_value;
	rtl_skb_debug_parameter.check_skb = skb;
	rtl_skb_debug_parameter.skb_pointer_off = -1;
	rtl_skb_debug_parameter.headroom = -1;
	rtl_skb_debug_parameter.dump_stack_en = 1;
	rtl_recycle_skb_cb_section_enter(&rtl_skb_debug_parameter);
}
#else
static inline void __init_skb_recy_eth_debug(struct sk_buff *skb)
{
}
#endif

#if defined(RTL_RING_BUFFER_RECYCLE_SKB)
unsigned int rtl_free_eth_skb_num(void)
{
	if (rbr.state == RTL_RING_BUF_RECYCLE_FULL)
		return rbr.ring_size_mask + 1;
	else if (rbr.state == RTL_RING_BUF_RECYCLE_EMPTY)
		return 0;
	else if (rbr.wr_idx > rbr.rd_idx)
		return rbr.wr_idx - rbr.rd_idx;

	return rbr.ring_size_mask + 1 - rbr.rd_idx + rbr.wr_idx;
}

unsigned int rtl_alloc_eth_skb_num(void)
{
	if (rbr.state == RTL_RING_BUF_RECYCLE_FULL)
		return 0;
	else if (rbr.state == RTL_RING_BUF_RECYCLE_EMPTY)
		return rbr.ring_size_mask + 1;
	else if (rbr.wr_idx > rbr.rd_idx)
		return rbr.ring_size_mask + 1 - rbr.wr_idx + rbr.rd_idx;

	return rbr.rd_idx - rbr.wr_idx;
}

__always_inline
void rtl_eth_skb_queue_head(struct sk_buff *newsk)
{
	RECYCLE_SPINLOCK(&rbr.lock);
	if (likely(rbr.state != RTL_RING_BUF_RECYCLE_FULL)) {
		rbr.ring_buffer_recycle_array[rbr.wr_idx] = newsk;
		rbr.wr_idx = (rbr.wr_idx + 1) & rbr.ring_size_mask;
		smp_mb();
		if (unlikely(rbr.wr_idx == rbr.rd_idx))
			rbr.state = RTL_RING_BUF_RECYCLE_FULL;
		else
			rbr.state = RTL_RING_BUF_RECYCLE_NORMAL;
	} else {
		pr_err("abnormal!! free the skb as dynamically one..\n");
		newsk->recyclable = 0;
		dev_kfree_skb_any(newsk);
	}
	RECYCLE_SPINUNLOCK(&rbr.lock);
}

__always_inline
struct sk_buff *rtl_eth_skb_dequeue(void)
{
	struct sk_buff *skb = NULL;

	RECYCLE_SPINLOCK(&rbr.lock);
	if (likely(rbr.state != RTL_RING_BUF_RECYCLE_EMPTY)) {
		skb = rbr.ring_buffer_recycle_array[rbr.rd_idx];
		rbr.rd_idx = (rbr.rd_idx + 1) & rbr.ring_size_mask;
		smp_mb();
		if (unlikely(rbr.wr_idx == rbr.rd_idx))
			rbr.state = RTL_RING_BUF_RECYCLE_EMPTY;
		else
			rbr.state = RTL_RING_BUF_RECYCLE_NORMAL;
	}
	RECYCLE_SPINUNLOCK(&rbr.lock);
	return skb;
}
#endif
#if defined(RTL_DOUBLE_LINK_LIST_RECYCLE_SKB)
unsigned int rtl_free_eth_skb_num(int pool)
{
	return recycle_skb_queue[pool].qlen;
}

unsigned int rtl_alloc_eth_skb_num(int pool)
{
	return eth_skb_free_num[pool] - recycle_skb_queue[pool].qlen;
}

__always_inline
void rtl_eth_skb_queue_head(struct sk_buff *newsk, int pool)
{
	RECYCLE_SPINLOCK(&recycle_skb_queue[pool].lock);
#if	defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
	__skb_queue_tail(&recycle_skb_queue[pool], newsk);
#else
	__skb_queue_head(&recycle_skb_queue[pool], newsk);
#endif
	RECYCLE_SPINUNLOCK(&recycle_skb_queue[pool].lock);
}

__always_inline
struct sk_buff *rtl_eth_skb_dequeue(int pool)
{
	struct sk_buff *skb = NULL;

	RECYCLE_SPINLOCK(&recycle_skb_queue[pool].lock);
	skb = __skb_dequeue(&recycle_skb_queue[pool]);
	RECYCLE_SPINUNLOCK(&recycle_skb_queue[pool].lock);
	return skb;
}
#endif
#if defined(RTL_SINGLE_LINK_LIST_RECYCLE_SKB)
unsigned int rtl_free_eth_skb_num(void)
{
	return recycle_skb_queue.qlen;
}

unsigned int rtl_alloc_eth_skb_num(void)
{
	return eth_skb_free_num - recycle_skb_queue.qlen;
}

__always_inline
void rtl_eth_skb_queue_head(struct sk_buff *newsk)
{
	RECYCLE_SPINLOCK(&recycle_skb_queue.lock);
	newsk->next = recycle_skb_queue.next;
	newsk->prev = (struct sk_buff *)&recycle_skb_queue;
	recycle_skb_queue.next = newsk;
	recycle_skb_queue.qlen++;
	RECYCLE_SPINUNLOCK(&recycle_skb_queue.lock);
}

__always_inline
void __recycle_skb_peek(struct sk_buff *skb, struct sk_buff_head *list)
{
	struct sk_buff *next, *prev;

	list->qlen--;
	next = skb->next;
	prev = skb->prev;
	next->prev = prev;
	prev->next = next;
}

__always_inline
struct sk_buff *rtl_eth_skb_dequeue(void)
{
	struct sk_buff *skb = NULL;

	RECYCLE_SPINLOCK(&recycle_skb_queue.lock);
	skb = recycle_skb_queue.next;
	if (likely(skb != (struct sk_buff *)&recycle_skb_queue))
		__recycle_skb_peek(skb, &recycle_skb_queue);
	else
		skb = NULL;
	RECYCLE_SPINUNLOCK(&recycle_skb_queue.lock);
	return skb;
}
#endif
#if defined(RTL_STRAIGHT_ARRAY_RECYCLE_SKB)
unsigned int rtl_free_eth_skb_num(void)
{
	return sra.idx;
}

unsigned int rtl_alloc_eth_skb_num(void)
{
	return eth_skb_free_num - sra.idx;
}

__always_inline
void rtl_eth_skb_queue_head(struct sk_buff *newsk)
{
	RECYCLE_SPINLOCK(&sra.lock);
	sra.array[sra.idx] = newsk;
	sra.idx++;
	RECYCLE_SPINUNLOCK(&sra.lock);
}

__always_inline
struct sk_buff *rtl_eth_skb_dequeue(void)
{
	struct sk_buff *skb = NULL;

	RECYCLE_SPINLOCK(&sra.lock);
	if (likely(sra.idx)) {
		sra.idx--;
		skb = sra.array[sra.idx];
	}
	RECYCLE_SPINUNLOCK(&sra.lock);
	return skb;
}
#endif
#if defined(RTL_LL_LINK_LIST_RECYCLE_SKB) && !defined(CONFIG_RTL_FBM_STACKING_RECYCLE_SKB)
unsigned int rtl_free_eth_skb_num(int pool)
{
	return atomic_read(&llll_num[pool]);
}

unsigned int rtl_alloc_eth_skb_num(int pool)
{
	return eth_skb_free_num[pool] - atomic_read(&llll_num[pool]);
}

__always_inline
void rtl_eth_skb_queue_head(struct sk_buff *newsk, int pool)
{
	llist_add(&newsk->recy_llnode, &llll_skb_head[pool]);
	atomic_inc(&llll_num[pool]);
}

__always_inline
struct sk_buff *rtl_eth_skb_dequeue(int pool)
{
	struct llist_node *llll_node;
	struct sk_buff *skb;

	RECYCLE_SPINLOCK(&llll_read_lock[pool]);
	llll_node = llist_del_first(&llll_skb_head[pool]);
	RECYCLE_SPINUNLOCK(&llll_read_lock[pool]);
	if (!llll_node)
		return NULL;
	skb = llist_entry(llll_node, struct sk_buff, recy_llnode);
	atomic_dec(&llll_num[pool]);
	return skb;
}
#endif
#if defined(CONFIG_RTL_FBM_STACKING_RECYCLE_SKB)
unsigned int (*rtl_free_eth_skb_num)(int pool);
unsigned int (*rtl_alloc_eth_skb_num)(int pool);
void (*rtl_eth_skb_queue_head)(struct sk_buff *newsk, int pool);
struct sk_buff *(*rtl_eth_skb_dequeue)(int pool);
void (*rtl_eth_skb_prepare_before_free)(struct sk_buff *skb);

unsigned int rtl_free_eth_skb_num_llll(int pool)
{
	return atomic_read(&llll_num[pool]);
}

unsigned int rtl_alloc_eth_skb_num_llll(int pool)
{
	return eth_skb_free_num[pool] - atomic_read(&llll_num[pool]);
}

__always_inline
void rtl_eth_skb_queue_head_llll(struct sk_buff *newsk, int pool)
{
	llist_add(&newsk->recy_llnode, &llll_skb_head[pool]);
	atomic_inc(&llll_num[pool]);
}

__always_inline
struct sk_buff *rtl_eth_skb_dequeue_llll(int pool)
{
	struct llist_node *llll_node;
	struct sk_buff *skb;

	RECYCLE_SPINLOCK(&llll_read_lock[pool]);
	llll_node = llist_del_first(&llll_skb_head[pool]);
	RECYCLE_SPINUNLOCK(&llll_read_lock[pool]);
	if (!llll_node)
		return NULL;
	skb = llist_entry(llll_node, struct sk_buff, recy_llnode);
	atomic_dec(&llll_num[pool]);
	return skb;
}

unsigned int rtl_alloc_eth_skb_num_fbm(int pool)
{
	return eth_skb_free_num[pool] - rtl_free_eth_skb_num(pool);
}

int rtl_recycle_register_callback_function(void *free_num, void *alloc_num, void *queue_skb, void *dequeue_skb, void *prepare_before_free)
{
	if (free_num)
		rtl_free_eth_skb_num = free_num;
	if (alloc_num)
		rtl_alloc_eth_skb_num = alloc_num;
	if (queue_skb)
		rtl_eth_skb_queue_head = queue_skb;
	if (dequeue_skb)
		rtl_eth_skb_dequeue = dequeue_skb;
	if (prepare_before_free)
		rtl_eth_skb_prepare_before_free = prepare_before_free;

	return 0;
}
EXPORT_SYMBOL(rtl_recycle_register_callback_function);
#endif

/*default set 0, let userspace to adjust it*/
static unsigned long min_limit_pages;
module_param(min_limit_pages, ulong, 0644);
static unsigned int mem_usage_status; /*0: normal, 1: no memory*/

#define K(x) ((x) << (PAGE_SHIFT - 10))
static struct sk_buff *rtl_netdev_alloc_skb_limit(unsigned int size)
{
	unsigned long free_pages, limit_pages;
	limit_pages = (min_free_kbytes > min_limit_pages) ? min_free_kbytes : min_limit_pages;
	limit_pages += (size / 512);
	free_pages = global_zone_page_state(NR_FREE_PAGES);

#ifdef CONFIG_HIGHMEM
	free_pages -= nr_free_highpages();
#endif
	free_pages = K(free_pages);
	if (free_pages > limit_pages) {
		if (mem_usage_status == 1) {
			mem_usage_status = 0;
			queue_broadcast(MSG_TYPE_NIC_EVENT, NIC_EVENT_TYPE_MEM_RECOVERD, 0, ENABLED);
		}
		return __recycle_rtl_dev_alloc_skb(size);
	}

	printk_ratelimited("[%s %d] %lu, %d\n", __func__, __LINE__, free_pages, min_free_kbytes);
	if (mem_usage_status == 0) {
		mem_usage_status = 1;
		queue_broadcast(MSG_TYPE_NIC_EVENT, NIC_EVENT_TYPE_NO_MEM, 0, ENABLED);
	}

	return NULL;
}

#ifdef ETH_RECYCLE_SKB_PROC
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
struct proc_dir_entry *rtl_recycle_pe;

static int rtl_recycleskb_read(struct seq_file *seq, void *v)
{
	int i;

	/*if recycle skb is disabled, nothing to be displayed.*/
	if (switch_off_recycle_skb) {
		seq_puts(seq, "\nUsage:\n");
		seq_puts(seq, "echo on > /proc/realtek/recycle_skb\n - Switch on recycle skb mechanism.\n");
		return 0;
	}

#if	defined(RTL_RING_BUFFER_RECYCLE_SKB)
	seq_puts(seq, "\nRecycle SKB: ring-buffer");
#endif
#if	defined(RTL_DOUBLE_LINK_LIST_RECYCLE_SKB)
	seq_puts(seq, "\nRecycle SKB: double-link-list");
#endif
#if	defined(RTL_SINGLE_LINK_LIST_RECYCLE_SKB)
	seq_puts(seq, "\nRecycle SKB: single-link-list");
#endif
#if defined(RTL_STRAIGHT_ARRAY_RECYCLE_SKB)
	seq_puts(seq, "\nRecycle SKB: straight-array");
#endif
#if defined(RTL_LL_LINK_LIST_RECYCLE_SKB) && !defined(CONFIG_RTL_FBM_STACKING_RECYCLE_SKB)
	seq_puts(seq, "\nRecycle SKB: lock_less-link-list");
#endif
#if defined(CONFIG_RTL_FBM_STACKING_RECYCLE_SKB)
	if (rtl_free_eth_skb_num == rtl_free_eth_skb_num_llll)
		seq_puts(seq, "\nRecycle SKB: FBM disable use LLLL");
	else
		seq_puts(seq, "\nRecycle SKB: FBM hw stack");
#endif
	for (i = 0; i < (0x1 << RTL_POOL_NUM_SHIFT_BITS); i++) {
		if (eth_skb_pool[i] == 0)
			continue;
		seq_printf(seq, "\npool[%d]", i);
		seq_printf(seq, "\neth_skb_total_num = %u eth_skb_free_num = %u, eth_skb_max_size = %u", eth_skb_total_num[i], eth_skb_free_num[i], eth_skb_max_size[i]);
		seq_printf(seq, "\nrecycle_skb_free_num:\t\t%10d", rtl_free_eth_skb_num(i));
		seq_printf(seq, "\nrecycle_skb_alloc_num:\t\t%10d", rtl_alloc_eth_skb_num(i));
#if defined(CONFIG_RTL_ETH_RECYCLED_SKB_ALLOC_MAX)
		seq_printf(seq, "\nrecycle_skb_alloc_max:\t\t%10d", eth_skb_max_allocate[i]);
		eth_skb_max_allocate[i] = 0;
#endif
	}
#if	defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
	{
		int i;

		seq_printf(seq, "\nSKB CB range: %d ~ %d", RTL_SKB_CB_SECTION_LOW, RTL_SKB_CB_SECTION_HIGH);
		seq_puts(seq, "\nSKB CB section registered: ");
		for (i = RTL_SKB_CB_SECTION_LOW; i <= RTL_SKB_CB_SECTION_HIGH; i++) {
			if (rtl_recycle_skb_cb_check[i])
				seq_printf(seq, "%d ", i);
		}
	}
#endif
#if defined(CONFIG_RTL_FBM_STACKING_RECYCLE_SKB)
	if (rtl_free_eth_skb_num == rtl_free_eth_skb_num_llll) {
#endif
		seq_puts(seq, "\nUsage:\n");
		seq_puts(seq, "echo on > /proc/realtek/recycle_skb\n - Switch on recycle skb mechanism.\n");
		seq_puts(seq, "echo off > /proc/realtek/recycle_skb\n - Switch off recycle skb mechanism.\n");
#if defined(RTL_DOUBLE_LINK_LIST_RECYCLE_SKB) || defined(RTL_SINGLE_LINK_LIST_RECYCLE_SKB) || defined(RTL_LL_LINK_LIST_RECYCLE_SKB)
		seq_puts(seq, "echo inc <pool> <count> > /proc/realtek/recycle_skb\n - Increase free skb number of pool, if new skb is available.\n");
		seq_puts(seq, "echo dec <pool> <count> > /proc/realtek/recycle_skb\n - Decrease free skb number of pool, if free skb enough.\n");
#endif
#if defined(CONFIG_RTL_FBM_STACKING_RECYCLE_SKB)
	} else {
		seq_puts(seq, "\n");
	}
#endif
	return 0;
}

static ssize_t rtl_recycleskb_write(struct file *filp, const char __user *buf, size_t count, loff_t *offp)
{
	int i, operator = 0;
	unsigned int original_state = switch_off_recycle_skb;
	unsigned long operand = 0, flags, pool;
	char tmpbuf[64];
	char *strptr;
	char *tokptr;
	struct sk_buff *skb;

	/*echo inc poolid 100 > /proc/recycle_skb
	 *echo dec poolid 100 > /proc/recycle_skb
	 *echo off > /proc/recycle_skb
	 *echo on > /proc/recycle_skb
	 */

	if (count >= sizeof(tmpbuf))
		return -EINVAL;

	if (buf && !copy_from_user(tmpbuf, buf, count)) {
		for (i = 0; i < count; i++) {
			if (tmpbuf[i] < 0 || tmpbuf[i] > 255)
				goto errout;
		}
		tmpbuf[count] = '\0';
		strptr = tmpbuf;
		tokptr = strsep(&strptr, "\n ");
		if (!tokptr) {
			goto errout;
		} else if (strcasecmp(tokptr, "on") == 0) {
#if defined(CONFIG_RTL_FBM_STACKING_RECYCLE_SKB)
			if (rtl_free_eth_skb_num == rtl_free_eth_skb_num_llll) {
#endif
				if (original_state == RTL_RECYCLE_SWITCH_OFF) {
					local_irq_save(flags);
					for (pool = 0; pool < (1 << RTL_POOL_NUM_SHIFT_BITS); pool++) {
						if (eth_skb_pool[pool]) {
							for (i = 0; i < eth_skb_total_num[pool]; i++) {
								skb = rtl_netdev_alloc_skb_limit(eth_skb_max_size[pool]);
								if (skb) {
									__init_skb_recy_eth(skb);
									skb->recyclable = pool;
									__init_skb_recy_eth_debug(skb);
#if defined(CONFIG_RTL_FBM_STACKING_RECYCLE_SKB)
									if (rtl_eth_skb_prepare_before_free)
										rtl_eth_skb_prepare_before_free(skb);
#endif
									rtl_eth_skb_queue_head(skb, pool);
								}
							}
							eth_skb_free_num[pool] = eth_skb_total_num[pool];
						}
					}
					switch_off_recycle_skb = RTL_RECYCLE_SWITCH_ON;
					local_irq_restore(flags);
				}
				pr_info("switch on!\n");
#if defined(CONFIG_RTL_FBM_STACKING_RECYCLE_SKB)
			}
#endif
			return count;
		} else if (strcasecmp(tokptr, "off") == 0) {
#if defined(CONFIG_RTL_FBM_STACKING_RECYCLE_SKB)
			if (rtl_free_eth_skb_num == rtl_free_eth_skb_num_llll) {
#endif
				if (original_state == RTL_RECYCLE_SWITCH_ON) {
					switch_off_recycle_skb = RTL_RECYCLE_SWITCH_OFF;
					/*drain all free skb*/
					local_irq_save(flags);
					for (pool = 0; pool < (1 << RTL_POOL_NUM_SHIFT_BITS); pool++) {
						if (eth_skb_pool[pool]) {
							while (rtl_free_eth_skb_num(pool)) {
								skb = rtl_eth_skb_dequeue(pool);
								if (skb) {
									skb->recyclable = 0;
									dev_kfree_skb_any(skb);
								}
							}
						}
					}
					local_irq_restore(flags);
				}
				pr_info("switch off!\n");
#if defined(CONFIG_RTL_FBM_STACKING_RECYCLE_SKB)
			}
#endif
			return count;
		}
#if defined(RTL_DOUBLE_LINK_LIST_RECYCLE_SKB) || defined(RTL_SINGLE_LINK_LIST_RECYCLE_SKB) || defined(RTL_LL_LINK_LIST_RECYCLE_SKB)
		else if (switch_off_recycle_skb == RTL_RECYCLE_SWITCH_OFF) {
			/*when switch off should not operate free skb number.*/
			return count;
		} else if (strcasecmp(tokptr, "inc") == 0)
			operator = 1;
		else if (strcasecmp(tokptr, "dec") == 0)
			operator = -1;
		else
			goto errout;
#if defined(CONFIG_RTL_FBM_STACKING_RECYCLE_SKB)
		if (rtl_free_eth_skb_num == rtl_free_eth_skb_num_llll) {
#endif
			tokptr = strsep(&strptr, " ");
			if (!tokptr)
				goto errout;

			if (kstrtoul(tokptr, 0, &pool))
				goto errout;

			tokptr = strsep(&strptr, " ");
			if (!tokptr)
				goto errout;

			if (kstrtoul(tokptr, 0, &operand))
				goto errout;

			local_irq_save(flags);
			if (operator < 0 && operand > rtl_free_eth_skb_num(pool)) {
				local_irq_restore(flags);
				pr_err("dec over free skb num (%lu)\n", operand);
				goto errout;
			}

			for (i = 0; i < operand; i++) {
				if (operator > 0) {
					skb = rtl_netdev_alloc_skb_limit(eth_skb_max_size[pool]);
					if (skb) {
						__init_skb_recy_eth(skb);
						skb->recyclable = pool;
						__init_skb_recy_eth_debug(skb);
#if defined(CONFIG_RTL_FBM_STACKING_RECYCLE_SKB)
						if (rtl_eth_skb_prepare_before_free)
							rtl_eth_skb_prepare_before_free(skb);
#endif
						rtl_eth_skb_queue_head(skb, pool);
						eth_skb_free_num[pool]++;
					}
				} else {
					skb = rtl_eth_skb_dequeue(pool);
					if (skb) {
						skb->recyclable = 0;
						dev_kfree_skb_any(skb);
						eth_skb_free_num[pool]--;
					}
				}
			}
			local_irq_restore(flags);
			pr_info("recycle_skb_free_num[%lu]:%d\n", pool, rtl_free_eth_skb_num(pool));
#if defined(CONFIG_RTL_FBM_STACKING_RECYCLE_SKB)
		}
#endif
#endif
	}
errout:
	return count;
}

static int rtl_recycleskb_open(struct inode *inode, struct file *file)
{
	return single_open(file, rtl_recycleskb_read, inode->i_private);
}

static const struct proc_ops rtl_recycleskb_ops = {
	.proc_open           = rtl_recycleskb_open,
	.proc_write          = rtl_recycleskb_write,
	.proc_read           = seq_read,
	.proc_lseek          = seq_lseek,
	.proc_release        = single_release,
};
#endif /*ETH_RECYCLE_SKB_PROC*/

int init_recycle_eth_skb_buf(unsigned int ring_size, unsigned int max_size, int forced_ring_size, int forced_new_pool)
{
	int i, pool = -1;
	int first_invalid = -1;
	struct sk_buff *skb;

	for (i = 1; i < (0x1 << RTL_POOL_NUM_SHIFT_BITS); i++) {
		if (eth_skb_pool[i] == 0) {
			if (first_invalid < 0)
				first_invalid = i;
			continue;
		} else if (eth_skb_max_size[i] == max_size && forced_new_pool == 0) {
			pool = i;
			break;
		}
	}
	if (pool < 0)
		pool = first_invalid;

	if (forced_ring_size) {
		eth_skb_free_num[pool] = ring_size;
	} else {
#if defined(CONFIG_RTL865X_ETH_PRIV_SKB_NUM) && (CONFIG_RTL865X_ETH_PRIV_SKB_NUM > 0)
		/*forced recycle pool size from preconfig or menuconfig*/
		eth_skb_free_num[pool] = CONFIG_RTL865X_ETH_PRIV_SKB_NUM;
#elif defined(CONFIG_RTL_ETH_RECYCLED_SKB_BY_MEM)
		struct sysinfo si;

		si_meminfo(&si);
		if (K(si.totalram) >= CONFIG_RTL_ETH_RECYCLED_SKB_THRESHOLD)
			eth_skb_free_num[pool] = CONFIG_RTL_ETH_RECYCLED_SKB_BY_MEM_HIGH;
		else
			eth_skb_free_num[pool] = CONFIG_RTL_ETH_RECYCLED_SKB_BY_MEM_LOW;

		if (eth_skb_free_num[pool] < ring_size)
			eth_skb_free_num[pool] = MAX_ETH_SKB_NUM + ring_size;
#else
		eth_skb_free_num[pool] = MAX_ETH_SKB_NUM + ring_size;
#endif
	}
	/*keep original total number*/
	eth_skb_total_num[pool] = eth_skb_free_num[pool];

#if defined(RTL_RING_BUFFER_RECYCLE_SKB)
	rbr.rd_idx = 0;
	rbr.wr_idx = 0;
	rbr.state = RTL_RING_BUF_RECYCLE_EMPTY;
	spin_lock_init(&rbr.lock);

	for (i = 0; (0x1 << i) < eth_skb_free_num; i++)
		;
	if (i >= 32) {
		pr_err("pool is oversized...failed!\n");
		return;
	}

	eth_skb_free_num = (0x1 << i);
	rbr.ring_size_mask = eth_skb_free_num - 1;

	rbr.ring_buffer_recycle_array = kmalloc_array(eth_skb_free_num, sizeof(struct sk_buff *), GFP_ATOMIC);
	if (!rbr.ring_buffer_recycle_array) {
		/*pr_err("ring buffer recycle array allocate failed!\n");*/
		return;
	}
#endif
#if defined(RTL_DOUBLE_LINK_LIST_RECYCLE_SKB) || defined(RTL_SINGLE_LINK_LIST_RECYCLE_SKB)
	skb_queue_head_init(&recycle_skb_queue[pool]);
#endif
#if defined(RTL_STRAIGHT_ARRAY_RECYCLE_SKB)
	sra.idx = 0;
	spin_lock_init(&sra.lock);

	sra.array = kmalloc_array(eth_skb_free_num, sizeof(struct sk_buff *), GFP_ATOMIC);
	if (!sra.array) {
		/*pr_err("straight recycle array allocate failed!\n");*/
		return;
	}
#endif
#if defined(RTL_LL_LINK_LIST_RECYCLE_SKB) && !defined(CONFIG_RTL_FBM_STACKING_RECYCLE_SKB)
	init_llist_head(&llll_skb_head[pool]);
	spin_lock_init(&llll_read_lock[pool]);
	atomic_set(&llll_num[pool], 0);
#endif
#if defined(CONFIG_RTL_FBM_STACKING_RECYCLE_SKB)
	/*FBM not enable, use LLLL as default*/
	if (forced_new_pool < 0xF) {
		init_llist_head(&llll_skb_head[pool]);
		spin_lock_init(&llll_read_lock[pool]);
		atomic_set(&llll_num[pool], 0);

		rtl_recycle_register_callback_function(rtl_free_eth_skb_num_llll, rtl_alloc_eth_skb_num_llll, rtl_eth_skb_queue_head_llll, rtl_eth_skb_dequeue_llll, NULL);
	} else {
		/*use forced_new_pool as FBM pool id*/
		rtl_recycle_register_callback_function(NULL, rtl_alloc_eth_skb_num_fbm, NULL, NULL, NULL);
		if (!rtl_free_eth_skb_num || !rtl_eth_skb_queue_head || !rtl_eth_skb_dequeue || !rtl_eth_skb_prepare_before_free)
			pr_err("ERROR! not register FBM callback function!!\n");
	}
#endif

	if (max_size < CROSS_LAN_MBUF_LEN)
		eth_skb_max_size[pool] = CROSS_LAN_MBUF_LEN;
	else
		eth_skb_max_size[pool] = max_size;
	eth_skb_pool[pool] = 1;
#if	defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
	if (rtl_recycle_skb_cb_value < 0)
		rtl_recycle_skb_cb_value = rtl_recycle_skb_cb_section_register();
#endif
	for (i = 0; i < eth_skb_free_num[pool]; i++) {
		skb = rtl_netdev_alloc_skb_limit(eth_skb_max_size[pool]);
		if (skb) {
			__init_skb_recy_eth(skb);
			skb->recyclable = pool;
			__init_skb_recy_eth_debug(skb);
#if defined(CONFIG_RTL_FBM_STACKING_RECYCLE_SKB)
			if (rtl_eth_skb_prepare_before_free)
				rtl_eth_skb_prepare_before_free(skb);
#endif
			rtl_eth_skb_queue_head(skb, pool);
		}
	}

	/*init rx_skb_queue*/
	memset(&rx_skb_queue, 0, sizeof(rx_skb_queue));
#ifdef ETH_RECYCLE_SKB_PROC
	if (!rtl_recycle_pe) {
		rtl_recycle_pe = proc_create_data("recycle_skb", 0644,
						  realtek_proc, &rtl_recycleskb_ops, NULL);
		if (!rtl_recycle_pe)
			pr_err("can't create proc entry for recycle_skb\n");
	}
#endif

	return pool;
}
EXPORT_SYMBOL(init_recycle_eth_skb_buf);

void deinit_recycle_eth_skb_buf(int pool)
{
	int i;
	struct sk_buff *skb;

	while (rtl_free_eth_skb_num(pool)) {
		skb = rtl_eth_skb_dequeue(pool);
		if (skb) {
			skb->recyclable = 0;
			dev_kfree_skb_any(skb);
		}
	}

	eth_skb_total_num[pool] = 0;
	eth_skb_free_num[pool] = 0;
	eth_skb_max_size[pool] = 0;
	eth_skb_pool[pool] = 0;

#ifdef ETH_RECYCLE_SKB_PROC
	if (rtl_recycle_pe) {
		for (i = ((0x1 << RTL_POOL_NUM_SHIFT_BITS) - 1); i >= 1; i--) {
			if (eth_skb_pool[i] > 0)
				break;
		}
		if (i < 0 && rtl_recycle_pe)
			proc_remove(rtl_recycle_pe);
	}
#endif
}
EXPORT_SYMBOL(deinit_recycle_eth_skb_buf);

int lookup_recycle_eth_skb_pool(unsigned int size)
{
	int i, max = 1 << RTL_POOL_NUM_SHIFT_BITS;

	for (i = 0; i < max; i++) {
		if (eth_skb_pool[i] && eth_skb_max_size[i] >= size)
			return i;
	}

	return -1;
}
EXPORT_SYMBOL(lookup_recycle_eth_skb_pool);

void recycle_skb_clean(struct sk_buff *skb)
{
	struct skb_shared_info *shinfo;
	bool				pfmemalloc;
	sk_buff_data_t		end;
	unsigned char		*head;
	unsigned int		truesize;
	unsigned char		recyclable;
#if	defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
	unsigned char		double_check_free;
#endif

	/*keep these fields before clean*/
	pfmemalloc = skb->pfmemalloc;
	head = skb->head;
	end = skb->end;
	truesize = skb->truesize;
	recyclable = skb->recyclable;
#if	defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
	double_check_free = skb->double_check_free;
#endif

	memset(skb, 0, offsetof(struct sk_buff, tail));
	/*clear RTK private header*/
	memset(&skb->__private_header_start, 0, offsetof(struct sk_buff, __private_header_end) -
											offsetof(struct sk_buff, __private_header_start));

	/* Account for allocated memory : skb + skb->head */
	skb->truesize = truesize;
	refcount_set(&skb->users, 1);
	skb->head = head;
	skb->end = end;
	skb->pfmemalloc = pfmemalloc;
	skb->mac_header = (typeof(skb->mac_header))~0U;
	skb->transport_header = (typeof(skb->transport_header))~0U;
	skb->recyclable = recyclable;
#if	defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
	skb->double_check_free = double_check_free;
	skb->skb_bak = skb;
	skb->head_bak = head;
#endif

	/* make sure we initialize shinfo sequentially */
	shinfo = skb_shinfo(skb);
	memset(shinfo, 0, offsetof(struct skb_shared_info, dataref));
	atomic_set(&shinfo->dataref, 1);
}
EXPORT_SYMBOL(recycle_skb_clean);

int recycle_rtl_eth_buf(struct sk_buff *skb)
{
	if (likely(switch_off_recycle_skb == RTL_RECYCLE_SWITCH_ON)) {
		__u16 pool = skb->recyclable;

		if (likely(pool > 0)) {
			if (likely(eth_skb_pool[pool])) {
				__recycle_frag_list(skb, rtl_eth_skb_queue_head, pool);
				__recycle_rtl_eth_buf(skb);
#if defined(CONFIG_RTL_FBM_STACKING_RECYCLE_SKB)
				if (rtl_eth_skb_prepare_before_free)
					rtl_eth_skb_prepare_before_free(skb);
#endif
				rtl_eth_skb_queue_head(skb, pool);
				return 1;
			} else {
				printk(KERN_ERR "[NIC] recycle skb but pool[%d] deinit!!\n", pool);
				/* clear recyclable and let kernel free it */
				skb->recyclable = 0;
			}
		}
	}
	return 0;
}
EXPORT_SYMBOL(recycle_rtl_eth_buf);

struct sk_buff *dev_alloc_skb_recy_eth(unsigned int size, int pool)
{
	struct sk_buff *skb;

	/* first argument is not used */
	if (unlikely(size > eth_skb_max_size[pool] || switch_off_recycle_skb))
		return NULL;

	/*dequeue one skb or null if empty*/
	skb = rtl_eth_skb_dequeue(pool);
	if (likely(skb)) {
#if defined(CONFIG_RTL_ETH_RECYCLED_SKB_ALLOC_MAX)
		unsigned int alloc_num = rtl_alloc_eth_skb_num(pool);

		if (alloc_num > eth_skb_max_allocate[pool])
			eth_skb_max_allocate[pool] = alloc_num;
#endif
#if	defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
		if (skb->double_check_free)
			pr_err("%s %d ERROR!!! double check free should be zero...skb=%llx\n", __func__, __LINE__, skb);
		else
			skb->double_check_free = 1;
		{
			rtl_recycle_skb_debug_t rtl_skb_debug_parameter;

			rtl_skb_debug_parameter.cb = rtl_recycle_skb_cb_value;
			rtl_skb_debug_parameter.check_skb = skb;
			rtl_skb_debug_parameter.skb_pointer_off = -1;
			rtl_skb_debug_parameter.headroom = -1;
			rtl_skb_debug_parameter.dump_stack_en = 1;
			rtl_recycle_skb_cb_section_leave(&rtl_skb_debug_parameter);
		}
#endif
		__init_skb_recy_eth(skb);
	}
	return skb;
}
EXPORT_SYMBOL(dev_alloc_skb_recy_eth);

struct sk_buff *recycle_skb_swap(struct sk_buff *skb)
{
	/*allocate new skb,
	 *swap skb, enqueue old_skb back to queue,
	 *continue to protocol stack
	 */
	struct sk_buff *new_skb = NULL;
	int pool = skb->recyclable;

	/*check for jumbo packet*/
	if (skb_shinfo(skb)->frag_list) {
		struct sk_buff *segs = skb_shinfo(skb)->frag_list;

		new_skb = rtl_netdev_alloc_skb_limit(eth_skb_max_size[pool]);
		if (new_skb) {
			struct sk_buff *frag_skb = rtl_netdev_alloc_skb_limit(eth_skb_max_size[pool]);

			if (frag_skb) {
				struct sk_buff *tmp_skb = skb_shinfo(skb)->frag_list->next;

				if (tmp_skb) {
					/*loop for next*/
					struct sk_buff *swap_skb = rtl_netdev_alloc_skb_limit(eth_skb_max_size[pool]);

					if (swap_skb) {
						struct sk_buff *next_skb = tmp_skb->next;

						frag_skb->next = swap_skb;
						while (next_skb) {
							struct sk_buff *next = next_skb->next;
							struct sk_buff *alloc = NULL;

							alloc = rtl_netdev_alloc_skb_limit(eth_skb_max_size[pool]);
							if (alloc) {
								swap_skb->next = alloc;
								swap_skb = alloc;
							} else {
								swap_skb = NULL;
								break;
							}
							next_skb = next;
						}
						if (swap_skb) {
							/*copy new_skb,frag_skb and all next_skb*/
							swap_skb = frag_skb->next;
							next_skb = skb_shinfo(skb)->frag_list->next;
							while (swap_skb) {
								struct sk_buff *swap_next = swap_skb->next;
								struct sk_buff *next_skb_next = next_skb->next;

								__recycle_skb_copy(next_skb, swap_skb);
								swap_skb = swap_next;
								next_skb = next_skb_next;
							}
							new_skb->data_len = skb->data_len;
							__recycle_skb_copy(skb, new_skb);
							__recycle_skb_copy(skb_shinfo(new_skb)->frag_list, frag_skb);
							skb_shinfo(skb)->frag_list = skb_shinfo(new_skb)->frag_list;
							skb_shinfo(new_skb)->frag_list = frag_skb;
						} else {
							/*no memory for swap*/
							swap_skb = frag_skb->next;
							while (swap_skb) {
								struct sk_buff *next = swap_skb->next;

								dev_kfree_skb_any(swap_skb);
								swap_skb = next;
							}
							dev_kfree_skb_any(frag_skb);
							dev_kfree_skb_any(new_skb);
							new_skb = NULL;
						}
					} else {
						/*no memory for swap*/
						dev_kfree_skb_any(frag_skb);
						dev_kfree_skb_any(new_skb);
						new_skb = NULL;
					}
				} else {
					/*copy new_skb and frag_skb*/
					new_skb->data_len = skb->data_len;
					__recycle_skb_copy(skb, new_skb);
					__recycle_skb_copy(skb_shinfo(new_skb)->frag_list, frag_skb);
					skb_shinfo(skb)->frag_list = skb_shinfo(new_skb)->frag_list;
					skb_shinfo(new_skb)->frag_list = frag_skb;
				}
			} else {
				/*no memory for frag_skb*/
				dev_kfree_skb_any(new_skb);
				new_skb = NULL;
			}
		}
		/*free old skb*/
		while (segs) {
			struct sk_buff *next = segs->next;
#if defined(CONFIG_RTL_FBM_STACKING_RECYCLE_SKB)
			segs->data = segs->head + RTL_RESERVED_HEADER_SIZE;
#endif
			dev_kfree_skb_any(segs);
			segs = next;
		}
		skb_shinfo(skb)->frag_list = NULL;
	} else {
		new_skb = rtl_netdev_alloc_skb_limit(eth_skb_max_size[pool]);
		if (new_skb)
			__recycle_skb_copy(skb, new_skb);
	}
#if defined(CONFIG_RTL_FBM_STACKING_RECYCLE_SKB)
	skb->data = skb->head + RTL_RESERVED_HEADER_SIZE;
#endif
	dev_kfree_skb_any(skb);
	return new_skb;
}
EXPORT_SYMBOL(recycle_skb_swap);

#ifdef CONFIG_WIRELESS_EXT
struct sk_buff *priv_skb_copy(struct sk_buff *skb)
{
	struct sk_buff *n;
	unsigned char recyclable;

	if (rx_skb_queue.qlen == 0) {
#if defined(CONFIG_LUNA_G3_SERIES)
		n = dev_alloc_skb(CA_NI_SKB_ALLOC_DATA_SIZE);
#else
		n = re8670_getAlloc(CROSS_LAN_MBUF_LEN);
#endif
	} else {
#ifdef RTK_QUE
		n = rtk_dequeue(&rx_skb_queue);
#else
		n = __skb_dequeue(&rx_skb_queue);
#endif
	}

	if (!n)
		return NULL;

	/*keep original recyclable*/
	recyclable = n->recyclable;

	/* Set the tail pointer and length */
	n->len = 0;
	skb_put(n, skb->len);
	n->csum = skb->csum;
	n->ip_summed = skb->ip_summed;
	memcpy(n->data, skb->data, skb->len);

	skb_copy_header(n, skb);

	/*keep original recyclable*/
	n->recyclable = recyclable;
	return n;
}
#endif /* CONFIG_NET_RADIO */

#if	defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
int rtl_recycle_skb_cb_section_register(void)
{
	int i;

	spin_lock(&rtl_recycle_skb_cb_lock);
	for (i = RTL_SKB_CB_SECTION_LOW; i <= RTL_SKB_CB_SECTION_HIGH; i++) {
		if (rtl_recycle_skb_cb_check[i] == 0) {
			rtl_recycle_skb_cb_check[i] = 1;
			spin_unlock(&rtl_recycle_skb_cb_lock);
			pr_info("%s :skb cb section register=>%d\n", __func__, i);
			return i;
		}
	}
	spin_unlock(&rtl_recycle_skb_cb_lock);
	return -1;
}
EXPORT_SYMBOL(rtl_recycle_skb_cb_section_register);

int rtl_recycle_skb_cb_section_unregister(unsigned int cb)
{
	if (cb < RTL_SKB_CB_SECTION_LOW || cb > RTL_SKB_CB_SECTION_HIGH)
		return -1;
	spin_lock(&rtl_recycle_skb_cb_lock);
	rtl_recycle_skb_cb_check[cb] = 0;
	spin_unlock(&rtl_recycle_skb_cb_lock);
	pr_info("%s :skb cb section unregister=>%d\n", __func__, cb);
	return 0;
}
EXPORT_SYMBOL(rtl_recycle_skb_cb_section_unregister);

int rtl_recycle_skb_cb_section(rtl_recycle_skb_debug_t *rtl_skb_debug, int section_value)
{
	int i, err = 0;

	if (!rtl_skb_debug->check_skb) {
		pr_err("%s: skb null..\n", __func__);
		return -1;
	}
	if (rtl_skb_debug->check_skb->recyclable == 0) {
		/* pr_err("%s: not recycle skb..\n", __func__); */
		return -1;
	}
	if (rtl_skb_debug->cb < RTL_SKB_CB_SECTION_LOW || rtl_skb_debug->cb > RTL_SKB_CB_SECTION_HIGH) {
		pr_err("%s: skb cb[%d] out of range(%d~%d)..\n", __func__, rtl_skb_debug->cb, RTL_SKB_CB_SECTION_LOW, RTL_SKB_CB_SECTION_HIGH);
		return -1;
	}
	if (rtl_recycle_skb_cb_check[rtl_skb_debug->cb] == 0) {
		pr_err("%s: skb cb[%d] unregistered..\n", __func__, rtl_skb_debug->cb);
		return -1;
	}

	for (i = RTL_SKB_CB_SECTION_LOW; i <= RTL_SKB_CB_SECTION_HIGH; i++) {
		if (i == rtl_skb_debug->cb) {
			if (section_value < 0 && rtl_skb_debug->check_skb->cb[i] == 0) {
				pr_err("%s: skb->cb[%d] section inconsistent!!skb=%llx skb->data=%llx skb->head=%llx\n", __func__, i, rtl_skb_debug->check_skb, rtl_skb_debug->check_skb->data, rtl_skb_debug->check_skb->head);
			} else if (rtl_skb_debug->check_skb->cb[i] == section_value) {
				pr_err("%s: skb->cb[%d] section %s!!skb=%llx skb->data=%llx skb->head=%llx\n", __func__, i, section_value ? "re-enter" : "re-leave", rtl_skb_debug->check_skb, rtl_skb_debug->check_skb->data, rtl_skb_debug->check_skb->head);
				err++;
			}
		} else if (rtl_skb_debug->check_skb->cb[i]) {
			pr_err("%s: skb->cb[%d] section overlap!!skb=%llx skb->data=%llx skb->head=%llx\n", __func__, i, rtl_skb_debug->check_skb, rtl_skb_debug->check_skb->data, rtl_skb_debug->check_skb->head);
			err++;
		}
	}
	if (section_value >= 0)
		rtl_skb_debug->check_skb->cb[rtl_skb_debug->cb] = section_value;

	if (rtl_skb_debug->headroom >= 0 && skb_headroom(rtl_skb_debug->check_skb) < rtl_skb_debug->headroom) {
		pr_err("%s %d: skb data offset unmatch!! skb->data=%llx skb->head=%llx skb_headroom=%d < %d\n", __func__, __LINE__, rtl_skb_debug->check_skb->data, rtl_skb_debug->check_skb->head, skb_headroom(rtl_skb_debug->check_skb), rtl_skb_debug->headroom);
		err++;
	}
	if (rtl_skb_debug->skb_pointer_off >= 0 && rtl_skb_debug->check_skb->skb_bak != (struct sk_buff *)(*((ca_uint_t *)(rtl_skb_debug->check_skb->head + rtl_skb_debug->skb_pointer_off)))) {
		pr_err("%s %d: skb data unmatch!! offset=%d skb_bak=%llx skb->data=%llx skb=%llx skb->head=%llx\n", __func__, __LINE__, rtl_skb_debug->skb_pointer_off, rtl_skb_debug->check_skb->skb_bak, rtl_skb_debug->check_skb->data, rtl_skb_debug->check_skb, rtl_skb_debug->check_skb->head);
		print_hex_dump(KERN_INFO, "", DUMP_PREFIX_ADDRESS, 16, 1, (rtl_skb_debug->check_skb->head + rtl_skb_debug->skb_pointer_off) - 16, 32, true);
		err++;
	}
	if (rtl_skb_debug->check_skb->skb_bak != rtl_skb_debug->check_skb) {
		pr_err("%s %d: skb bak unmatch!! skb_bak=%llx skb=%llx\n", __func__, __LINE__, rtl_skb_debug->check_skb->skb_bak, rtl_skb_debug->check_skb);
		err++;
	}
	if (rtl_skb_debug->check_skb->head_bak != rtl_skb_debug->check_skb->head) {
		pr_err("%s %d: skb head bak unmatch!! skb->head_bak=%llx skb->head=%llx\n", __func__, __LINE__, rtl_skb_debug->check_skb->head_bak, rtl_skb_debug->check_skb->head);
		err++;
	}

	if (err)
		goto ERR_RET;

	return 0;

ERR_RET:
	if (rtl_skb_debug->dump_stack_en > 0)
		dump_stack();
	return -1;
}

int rtl_recycle_skb_cb_section_enter(rtl_recycle_skb_debug_t *rtl_skb_debug)
{
	return rtl_recycle_skb_cb_section(rtl_skb_debug, RTL_SKB_CB_SECTION_ENTER);
}
EXPORT_SYMBOL(rtl_recycle_skb_cb_section_enter);

int rtl_recycle_skb_cb_section_check(rtl_recycle_skb_debug_t *rtl_skb_debug)
{
	return rtl_recycle_skb_cb_section(rtl_skb_debug, -1);
}
EXPORT_SYMBOL(rtl_recycle_skb_cb_section_check);

int rtl_recycle_skb_cb_section_leave(rtl_recycle_skb_debug_t *rtl_skb_debug)
{
	return rtl_recycle_skb_cb_section(rtl_skb_debug, RTL_SKB_CB_SECTION_LEAVE);
}
EXPORT_SYMBOL(rtl_recycle_skb_cb_section_leave);
#endif

#endif /* CONFIG_RTL_ETH_RECYCLED_SKB */
