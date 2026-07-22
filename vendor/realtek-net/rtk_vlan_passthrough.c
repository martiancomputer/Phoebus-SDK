#include <linux/module.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/netdevice.h>
#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/inetdevice.h>
#include <soc/cortina/rtk_vlan_common.h>
#include <soc/cortina/rtk_vlan_passthrough.h>
#include <soc/cortina/rtk_common_utility.h>
#if defined(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE)
#include "./rtl86900/FleetConntrackDriver/rtk_fc_skb.h"
#endif

#include <linux/version.h>
//#include <linux/fs.h>



int rtk_vlan_passthrough_enable = 0;

#ifdef CONFIG_RTL_SMUX_LEGACY
enum vlan_action{NONE, TAG_ADD};
#endif

rtl83xx_vlan_passthrough_set_hook_t rtl83xx_vlan_passthrough_set_hook;

#ifdef CONFIG_RTL_SMUX_LEGACY
extern int is_vlan_passthrough_wan(struct sk_buff *skb);
u32 smux_priv_flags_get(const struct net_device *dev, u32 flags_mask);

int rtk_set_vlan_passthrough_device_flag(char *dev_name, int flag)
{
	struct net_device *dev;

	if (dev_name == NULL)
		return FAIL;

	dev = dev_get_by_name(&init_net, dev_name);

	if (dev == NULL)
		return FAIL;

	if (flag == TAG_ADD)
		dev->dev_bridge = 1;
	else if (flag == NONE)
		dev->dev_bridge = 0;
	else {
		dev_put(dev);
		return FAIL;
	}

	dev_put(dev);
	return SUCCESS;
}
#endif

int rtk_vlan_passthrough_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, rtk_vlan_passthrough_read, NULL));
}
ssize_t rtk_vlan_passthrough_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return rtk_vlan_passthrough_write(file, userbuf, count, off);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
struct proc_ops rtk_vlan_passthrough_proc_fops= {
        .proc_open           = rtk_vlan_passthrough_single_open,
        .proc_write		    = rtk_vlan_passthrough_single_write,
        .proc_read           = seq_read,
        .proc_lseek         = seq_lseek,
        .proc_release        = single_release,
};
#else
struct file_operations rtk_vlan_passthrough_proc_fops= {
        .open           = rtk_vlan_passthrough_single_open,
        .write		    = rtk_vlan_passthrough_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif

int rtk_vlan_passthrough_read(struct seq_file *s, void *v)
{
	#ifdef CONFIG_RTL_SMUX_LEGACY
	struct net_device *dev = NULL;
	#endif

	seq_printf(s, "rtk_vlan_passthrough_enable=%d \n", rtk_vlan_passthrough_enable);
	seq_printf(s, "rtk_vlan_debug=0x%x \n", rtk_vlan_debug);

	#ifdef CONFIG_RTL_SMUX_LEGACY
	rtnl_lock();
	for_each_netdev(&init_net, dev) {
		if (NULL != dev) {
			seq_printf(s, "name=%s dev_bridge=0x%x\n", dev->name, dev->dev_bridge);
		}
	}
	rtnl_unlock();
	#endif

	return 0;
}

int rtk_vlan_passthrough_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char tmpbuf[128] = {'\0'};
	int enable = 0;
	unsigned char *strptr = NULL, *cmd_addr = NULL;
	unsigned debug_level = 0;
	#ifdef CONFIG_RTL_SMUX_LEGACY
	int ret = 0, num = 0;
	char dev_name[IFNAMSIZ] = {'\0'};
	char action[16] = {'\0'};
	int dev_vlan_passthru_flag = 0;
	#endif

	if (buff &&(len > 0) &&(len < sizeof(tmpbuf)) && !copy_from_user(tmpbuf, buff, len))
	{
		tmpbuf[len-1] = '\0';

		strptr = tmpbuf;
		cmd_addr = strsep(&strptr, " ");

		if (cmd_addr && !strncmp(cmd_addr, "vlan_passthrough", strlen("vlan_passthrough"))){

			if (strptr){
				sscanf(strptr, "%d", &enable);
				rtk_vlan_passthrough_enable = enable;
			}
			else{
				goto err_out;
			}
					
			VLAN_TRACE("%s %d len=%lu enable=%d strptr=%s \n", __FUNCTION__, __LINE__, len, enable, strptr);
			//to-do, configure external switch's vlan
			#if defined(CONFIG_RTL_83XX_SUPPORT)
			if (rtl83xx_vlan_passthrough_set_hook){
				int ret = (*rtl83xx_vlan_passthrough_set_hook)(rtk_vlan_passthrough_enable);
				VLAN_TRACE("ret=%d \n", ret);
			}
			else{
				VLAN_ERROR("rtl83xx_vlan_passthrough_set_hook is NULL!\n");
			}
			#endif
		}
		else if (cmd_addr && !strncmp(cmd_addr, "vlan_debug_level", strlen("vlan_debug_level"))){
			if (strptr){
				sscanf(strptr, "%x", &debug_level);
				rtk_vlan_debug = debug_level;
			}
			else{
				goto err_out;
			}
			
			VLAN_TRACE("%s %d rtk_vlan_debug=0x%x \n", __FUNCTION__, __LINE__, rtk_vlan_debug);
			
		}
		#ifdef CONFIG_RTL_SMUX_LEGACY
		else if (cmd_addr && !strncmp(cmd_addr, "dev", strlen("dev"))) {
			num = sscanf(strptr, "%s %s", action, dev_name);
			if (num < 2)
				goto err_out;
			if (!strncmp(action, "add", sizeof(action)))
				dev_vlan_passthru_flag = 1;
			else if (!strncmp(action, "remove", sizeof(action)))
				dev_vlan_passthru_flag = 0;
			else {
				goto err_out;
			}

			if (rtk_set_vlan_passthrough_device_flag(dev_name, dev_vlan_passthru_flag))
				goto err_out;
		}
		#endif
		else{
			goto err_out;
		}

		return len;
	}

err_out:
	
	printk("--------help--------\n");
	printk("echo vlan_passthrough [1/0] >/proc/driver/realtek/rtk_vlan_passthrough\n");
	printk("echo vlan_debug_level [level] >/proc/driver/realtek/rtk_vlan_passthrough\n");
	#ifdef CONFIG_RTL_SMUX_LEGACY
	printk("echo dev add/remove [dev_name] >/proc/driver/realtek/rtk_vlan_passthrough\n");
	#endif
	printk("[level]: trace=0x1, error=0x2, all=0xffffffff\n");
	
	return len;
}

static int __init rtk_vlan_passthrough_init(void)
{
	if (rtk_proc_dir)	
		printk("%s %d create rtk_vlan_passthrough proc file in /proc/driver/realtek/ \n", __func__, __LINE__);
	else
		printk("%s %d create rtk_vlan_passthrough proc file in /proc/ \n", __func__, __LINE__);
	
	if (proc_create_data("rtk_vlan_passthrough",0,rtk_proc_dir,&rtk_vlan_passthrough_proc_fops,NULL)==NULL){
		printk("%s %d create rtk_vlan_passthrough proc error !!\n", __func__, __LINE__);
	}
	
	return 0;
}

static void __exit rtk_vlan_passthrough_exit(void)
{
	VLAN_TRACE("%s %d \n", __func__, __LINE__);
	remove_proc_entry("rtk_vlan_passthrough", rtk_proc_dir);

	return;
}

int rtk_get_vlan_passthrough_enable(void)
{
	return rtk_vlan_passthrough_enable;
}


/*
* flag = 0, skb->data with ethernet header
* flag = 1, skb->data without ethernet header
*/
int rtk_vlan_passthrough_rx(struct sk_buff **pskb, int flag)
{
	struct sk_buff *skb = NULL;
	rtk_fc_ingress_data_t *ingress_data = NULL;
	unsigned char *pdata = NULL;
	unsigned int offset = 0;
	unsigned short protocol = 0;
	#ifdef CONFIG_RTL_SMUX_LEGACY
	unsigned short action = 0;
	#endif
	int ret = SUCCESS;
	struct vlan_hdr *vlan_header = NULL;

	if (!rtk_vlan_passthrough_enable)
		return SUCCESS;
	
	if (!pskb || !*pskb){
		VLAN_ERROR("parameter error! pskb or *psbk is NULL!");
		return FAIL;
	}

	skb = *pskb;

	#ifdef CONFIG_RTL_SMUX_LEGACY
	if (smux_priv_flags_get(skb->dev, PRIV_RSMUX)) {
		//packets from wan real dev check vlan passthrough flag
		if (DO_VLAN_PASSTHROUGH != is_vlan_passthrough_wan(skb))
			return SUCCESS;
	} else if (smux_priv_flags_get(skb->dev, PRIV_OSMUX)) {
		//pkt from smux dev needn't do vlan passthrough
		return SUCCESS;
	} else {
		//process pkts form lan/wlan check vlan passthrough flag
		action = skb->dev->dev_bridge ? TAG_ADD : NONE;
		if (action != TAG_ADD)
			return SUCCESS;
	}
	#endif

	if (!flag)
		offset += (ETH_ALEN<<1);

	pdata = skb->data + offset;
	ingress_data = &skb->fcIngressData;

	//unlikely...... do nothing......
	if (!pdata){
		VLAN_ERROR("packet is too short! skb->dev->name=%s, skb->len=%u, flag=%d", skb->dev->name, skb->len, flag);
		return SUCCESS;
	}
	
	//protocol
	if (!flag)
		protocol = *(unsigned short *)(pdata);
	else 
		protocol = skb->protocol;

	if (rtk_vlan_debug && net_ratelimit())
		VLAN_TRACE("skb->dev->name=%s, skb->len=%u, flag=%d protocol=0x%04x", skb->dev->name, skb->len, flag, protocol);

	//store vlan tag, currently donot consider STAG
	if (protocol == __constant_htons(ETH_P_8021Q)){
		if (!flag)
			pdata += 2;

		vlan_header = (struct vlan_hdr *)pdata;
		
		ingress_data->srcCVlanId = (ntohs(vlan_header->h_vlan_TCI)&VLAN_VID_MASK);
		ingress_data->srcCVlanPri = ((ntohs(vlan_header->h_vlan_TCI)&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT);
		ingress_data->ingressTagif |= RTK_CVLAN_TAGIF;
		
		if (rtk_vlan_debug && net_ratelimit())
			VLAN_TRACE("skb->dev->name=%s, skb->len=%u, flag=%d ingress_data->srcCVlanId=%u ingress_data->srcCVlanPri=%u ingress_data->ingressTagif=0x%x ret=%d", skb->dev->name, skb->len, flag, ingress_data->srcCVlanId,ingress_data->srcCVlanPri, ingress_data->ingressTagif, ret);

		//remove vlan tag ???
		if (!flag) {
			ret = remove_vlan_tag(pskb);
			
			if (rtk_vlan_debug && net_ratelimit())
				VLAN_TRACE("skb->dev->name=%s, skb->len=%u, flag=%d ingress_data->srcCVlanId=%u ingress_data->srcCVlanPri=%u ingress_data->ingressTagif=0x%x ret=%d", skb->dev->name, skb->len, flag, ingress_data->srcCVlanId,ingress_data->srcCVlanPri, ingress_data->ingressTagif, ret);
		} else {
			
			skb = skb_vlan_untag(skb);
			if (unlikely(!skb))
				return FAIL;
			
			//fix me ?
			skb->vlan_tci = 0;
			#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,1,0)
			skb->vlan_present = 0;
			#endif
			*pskb = skb;
			
			if (rtk_vlan_debug && net_ratelimit())
				VLAN_TRACE("skb->dev->name=%s, skb->len=%u, flag=%d ingress_data->srcCVlanId=%u ingress_data->srcCVlanPri=%u ingress_data->ingressTagif=0x%x", skb->dev->name, skb->len, flag, ingress_data->srcCVlanId,ingress_data->srcCVlanPri, ingress_data->ingressTagif);
		}
	}	
	
	return ret;	
}

/* 
  * NOTE!!!!!!
  * it's should call this function before add flow
*/
int rtk_vlan_passthrough_tx(struct sk_buff **pskb)
{
	struct sk_buff *skb = NULL;
	rtk_fc_ingress_data_t *ingress_data = NULL;
	int igr_stag_if = 0, igr_ctag_if = 0;
	int ret = 0;
	#ifdef CONFIG_RTL_SMUX_LEGACY
	int action = 0;
	#endif
	unsigned short vid = 0;
	unsigned char priority = 0;

	if (!rtk_vlan_passthrough_enable)
		return SUCCESS;
	
	if (!pskb || !*pskb){
		VLAN_ERROR("parameter error! pskb or *psbk is NULL!");
		return FAIL;
	}

	skb = *pskb;

	#ifdef CONFIG_RTL_SMUX_LEGACY
	action = skb->dev->dev_bridge ? TAG_ADD : NONE;
	if (action != TAG_ADD)
		return SUCCESS;
	#endif
	ingress_data = &skb->fcIngressData;
	if (*(unsigned short *)(&skb->data[ETH_ALEN<<1]) == __constant_htons(ETH_P_8021Q)) {
		//currently do nothing, maybe enable passthrough in gatewaymode ? fix me?
		if (rtk_vlan_debug && net_ratelimit())
			VLAN_TRACE("skb tagged! skb->dev->name=%s, skb->len=%u", skb->dev->name, skb->len);

		return SUCCESS;
	}

	if (ingress_data->ingressTagif & RTK_SVLAN_TAGIF)
		igr_stag_if = 1;

	if (ingress_data->ingressTagif & RTK_CVLAN_TAGIF)
		igr_ctag_if = 1;

	if (rtk_vlan_debug && net_ratelimit())
		VLAN_TRACE("skb->dev->name=%s, skb->len=%u igr_stag_if=0x%x igr_ctag_if=0x%x", skb->dev->name, skb->len, igr_stag_if, igr_ctag_if);
	
	//ingress packets untagged
	if (!igr_ctag_if){
		
		if (rtk_vlan_debug && net_ratelimit())
			VLAN_TRACE("ingress packets without tag! skb->dev->name=%s, skb->len=%u", skb->dev->name, skb->len);

		return SUCCESS;
	}

	vid = ingress_data->srcCVlanId;
	priority = ingress_data->srcCVlanPri;
	
	//insert vlan tag, currently donot consider STAG
	ret = rtk_insert_vlan_tag_and_priority(vid, priority, pskb);
	
	if (rtk_vlan_debug && net_ratelimit())
		VLAN_TRACE("insert vlan: skb->dev->name=%s, skb->len=%u vid=%u, priority=%u ret=%d", skb->dev->name, skb->len, vid, priority, ret);
	
	return ret;
}

module_init(rtk_vlan_passthrough_init);
module_exit(rtk_vlan_passthrough_exit);

EXPORT_SYMBOL(rtk_vlan_passthrough_init);
EXPORT_SYMBOL(rtk_vlan_passthrough_exit);
EXPORT_SYMBOL(rtk_get_vlan_passthrough_enable);
EXPORT_SYMBOL(rtk_vlan_passthrough_rx);
EXPORT_SYMBOL(rtk_vlan_passthrough_tx);
EXPORT_SYMBOL(rtl83xx_vlan_passthrough_set_hook);
#ifdef CONFIG_RTL_SMUX_LEGACY
EXPORT_SYMBOL(rtk_set_vlan_passthrough_device_flag);
#endif

