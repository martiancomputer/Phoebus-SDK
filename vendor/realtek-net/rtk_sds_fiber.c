#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/kthread.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/delay.h>	/* mdelay() */
//#include <linux/platform_device.h>
//#include <linux/miscdevice.h>	/* misc_register */
//#include <fs/proc/internal.h>
#include <asm/io.h>
#include <common/rt_error.h>
#include <ca_event.h>
#include <aal_phy.h>
#include <aal_sds.h>
#include <aal_port.h>


#define PRINTHEADER	"sdsFiber"
#ifdef __KERNEL__
/*******************************
 * Only for Kernel Space       *
 *******************************/
#define RTKSDSFIBER_PRINT(fmt,args...)	printk(KERN_INFO fmt"\n", ##args)
#define RTKSDSFIBER_MSG(fmt,args...)	printk(KERN_INFO PRINTHEADER": "fmt"\n", ##args)
#define RTKSDSFIBER_INFO(fmt,args...)	printk(KERN_INFO "\033[1;33;46m"PRINTHEADER": "fmt"\033[m""\n", ##args)
#define RTKSDSFIBER_ERROR(fmt,args...)	printk(KERN_INFO "\033[1;33;41m"PRINTHEADER": "fmt" [%s(line %d)]""\033[m""\n", ##args, __FUNCTION__, __LINE__)

//#define RTKSDSFIBER_PHY_DEBUG
#ifdef RTKSDSFIBER_PHY_DEBUG
#define RTKSDSFIBER_DEBUG		RTKSDSFIBER_INFO
#else
#define RTKSDSFIBER_DEBUG(fmt,args...)
#endif
#endif

#define event_port_link_t	ca_event_port_link_t
static int  p4_link_up=0;
static int  p6_link_up=0;
static struct task_struct *rtk_sds_fiber_task;
#define RTK_SDS_FIBER_KTHREAD_CPU	0

static int rtk_sds_fiber_thread (void *data)
{
	uint32 status_reg;
#ifdef CONFIG_LUNA_G3_SERIES
	event_port_link_t	link_event;
	aal_sds_status_t     status;
#else
	int32 ret = RT_ERR_FAILED;
	rtk_port_macAbility_t portAbility;
#endif
	while(!kthread_should_stop()) {
		/* No need to wake up earlier */
		//set_current_state(TASK_UNINTERRUPTIBLE);

		
						/* Send link event, ref. __port_link_evt_send */					
						/* port 4 */
						aal_sds_status_get(0, CA_PORT_ID_NI4, &status);
						memset(&link_event, 0, sizeof(event_port_link_t));
						link_event.port_id = CA_PORT_ID(CA_PORT_TYPE_ETHERNET, CA_PORT_ID_NI4);

						if(status.signal_ok==1 && status.sync_ok==1 && status.link_ok==1){
							link_event.status = 1;
						}else{
							link_event.status = 0;
						}
						if(p4_link_up != link_event.status) {
							p4_link_up = link_event.status;
							ca_event_send(0, CA_EVENT_ETH_PORT_LINK, (void *)&link_event, sizeof(event_port_link_t));
							RTKSDSFIBER_INFO("PHY: Port %x link status = %d", CA_PORT_ID_NI4,link_event.status);
						}
					
						//port 6
						memset(&link_event, 0, sizeof(event_port_link_t));
						aal_sds_status_get(0, CA_PORT_ID_NI6, &status);
						if(status.signal_ok==1 && status.sync_ok==1 && status.link_ok==1){
							link_event.status = 1;
						}else{
							link_event.status = 0;
						}
	
						link_event.port_id = CA_PORT_ID(CA_PORT_TYPE_ETHERNET, CA_PORT_ID_NI6);

						if(p6_link_up != link_event.status) {
							p6_link_up = link_event.status;
							ca_event_send(0, CA_EVENT_ETH_PORT_LINK, (void *)&link_event, sizeof(event_port_link_t));
							RTKSDSFIBER_INFO("PHY: Port %x link status = %d", CA_PORT_ID_NI6,link_event.status);
						}

		//schedule_timeout(3 * HZ);
		msleep(200);
	}
	return 0;
}

/*
 * proc function
 */
#define PROC_FILE_RTK_SDS_FIBER		"sdsFiber"
extern struct proc_dir_entry		*realtek_proc;
static int rtk_sgmii_link_status_get(void)
{
	RTKSDSFIBER_PRINT("%d %d",p4_link_up,p6_link_up);
	return 0;
}

static int rtk_sds_fiber_read_proc(struct seq_file *seq, void *v)
{
	rtk_sgmii_link_status_get();
	return 0;
}

static int rtk_sds_fiber_open(struct inode *inode, struct file *file)
{
        return single_open(file, rtk_sds_fiber_read_proc, inode->i_private);
}

static const struct file_operations rtk_sds_fiber_fops = {
        .owner          = THIS_MODULE,
        .open           = rtk_sds_fiber_open,
        .read           = seq_read,
        .write          = NULL,
        .llseek         = seq_lseek,
        .release        = single_release,
};

bool rtk_sds_fiber_proc_init(void)
{
	struct proc_dir_entry *pe = NULL;

	pe = proc_create_data(PROC_FILE_RTK_SDS_FIBER,
		S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH,
		realtek_proc, &rtk_sds_fiber_fops, NULL);

	if (!pe)
		RTKSDSFIBER_ERROR("can't create proc entry for rtk_ext_gphy");

	RTKSDSFIBER_MSG("Creat proc entry.");

	return TRUE;
}

void rtk_sds_fiber_proc_exit(void)
{
	remove_proc_entry(PROC_FILE_RTK_SDS_FIBER, realtek_proc);
}

/*
 * module function
 */
static int __init rtk_sds_fiber_moudle_init(void)
{
	RTKSDSFIBER_MSG("RTK SDS FIBER Module Init V002.");


		aal_sds_mode_set(0, CA_PORT_ID_NI4, AAL_SDS_MODE_SGMII, AAL_SDS_SPEED_1G);
		aal_sds_mode_set(0, CA_PORT_ID_NI6, AAL_SDS_MODE_SGMII, AAL_SDS_SPEED_1G);
		

	/* Create rtk sds fiber link status polling kthread */
	rtk_sds_fiber_task = kthread_create(rtk_sds_fiber_thread, NULL, "rtk_sds_fiber/%d", RTK_SDS_FIBER_KTHREAD_CPU);
	if (WARN_ON(!rtk_sds_fiber_task)) {
		RTKSDSFIBER_ERROR("Create rtk_sds_fiber/%d failed!", RTK_SDS_FIBER_KTHREAD_CPU);
	}
	else {
		kthread_bind(rtk_sds_fiber_task, RTK_SDS_FIBER_KTHREAD_CPU);
		wake_up_process(rtk_sds_fiber_task);
	}
	
		/* Init proc device */
	rtk_sds_fiber_proc_init();
	
	return 0;
}

static void __exit rtk_sds_fiber_module_exit(void)
{
	kthread_stop(rtk_sds_fiber_task);
	rtk_sds_fiber_proc_exit();

	RTKSDSFIBER_MSG("%s", __FUNCTION__);
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("RTK SDS FIBER Module");
MODULE_AUTHOR("Realtek.com");

module_init(rtk_sds_fiber_moudle_init);
module_exit(rtk_sds_fiber_module_exit);

