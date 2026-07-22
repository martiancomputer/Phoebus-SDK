

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/inet.h>
#include <linux/uaccess.h>
#include <fs/proc/internal.h>

#include "l34testmodule.h"
#include <rtl_glue.h>

int rtk_fcTesting_proc_debug_level_write(struct file *file, const char *buffer, unsigned long count, void *data);
int rtk_fcTesting_proc_debug_level_read(struct seq_file *s, void *data);

char procfs_buffer[PROCFS_MAX_SIZE];



rtk_fcTesting_proc_t l34testProc[]=
{
	{
		.name="testcase" ,
		.get = single_test_dump_case_table,
		.set = single_test,
	},

	{
		.name="test_debug_level" ,
		.get = rtk_fcTesting_proc_debug_level_read ,
		.set = rtk_fcTesting_proc_debug_level_write ,
	},

};


#define PROC_BUF_SIZE 256
int rtk_fcTesting_proc_debug_level_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	unsigned char tmpBuf[PROC_BUF_SIZE] = {0};
	int len = (count >= PROC_BUF_SIZE) ? PROC_BUF_SIZE : count;
	if (buffer)
	{
		/* copy data to the buffer */
		strncpy(tmpBuf, buffer, len);
		tmpBuf[len-1] = '\0';
		test_debug_level=simple_strtoul(tmpBuf, NULL, 16);
		return count;
	}
	return -EFAULT;
}

int rtk_fcTesting_proc_debug_level_read(struct seq_file *s, void *data)
{
	int bit = 0;
	char *name_of_debug_level[]={	//rtk_fc_debugLevel_t
		"DEBUG", "FIXME", "CALLBACK", "TRACE",
		"", "WARN", "TRACE_DUMP", "EVENT",
		"", "TABLE", "ALG", "IGMP",
		"ACL_RSV", "API", "TIMER", "PS",
		"IGR", "EGR", "WIFI", "ACL_CTRL",
		"DSLITE", "TOPS",
	};

	rtlglue_printf("Debug level=0x%x ", test_debug_level);
	for(bit=0; bit<(sizeof(name_of_debug_level)/sizeof(name_of_debug_level[0])); bit++){
		if(strlen(name_of_debug_level[bit]))
			rtlglue_printf("[0x%x:%s=%s]",(1<<bit), name_of_debug_level[bit], (test_debug_level&(1<<bit))?"\033[1;33mon\033[0m":"off");
	}
	rtlglue_printf("\n");

	return 0;
}


static int common_fpga_proc_get_null(struct seq_file *s, void *v){ return 0;}
static int common_fpga_single_open_null(struct inode *inode, struct file *file){return(single_open(file, common_fpga_proc_get_null, NULL));}
static int common_fpga_rg_single_open(struct inode *inode, struct file *file)
{
    int i;
    for( i=0; i< (sizeof(l34testProc)/sizeof(rtk_fcTesting_proc_t)) ;i++)
    {
		//printk("common_single_open inode_id=%u i_ino=%u, target proc: %s \n",l34testProc[i].inode_id[0],(unsigned int)inode->i_ino, l34testProc[i].name);
        if(l34testProc[i].inode_id[0]==(unsigned int)inode->i_ino)
        {
			return(single_open(file, l34testProc[i].get, NULL));
        }
    }
    return -1;       
}
static ssize_t common_fpga_rg_single_write(struct file * file, const char __user * userbuf,
				size_t count, loff_t * off)
{
    	int i;
	size_t len = (count >= (PROCFS_MAX_SIZE-1))?(PROCFS_MAX_SIZE-1):count;

		
	/* write data to the buffer */
	if ( copy_from_user(procfs_buffer, userbuf, len) ) {
		return -EFAULT;
	}
	procfs_buffer[len] = '\0';
	
    for( i=0; i< (sizeof(l34testProc)/sizeof(rtk_fcTesting_proc_t)) ;i++)
    {
		//printk("common_single_write inode_id=%u i_ino=%u, target proc: %s\n",l34testProc[i].inode_id[0],(unsigned int)file->f_inode->i_ino, l34testProc[i].name);
        if(l34testProc[i].inode_id[0]==(unsigned int)file->f_inode->i_ino)
        {
			return l34testProc[i].set(file,procfs_buffer,count,off);
        }
    }
    return -1;      
}
				

void rtk_modeltest_proc_init(void){

	struct proc_dir_entry *proc_rg = NULL;
	struct proc_dir_entry *p=NULL;
	int32 i;

	//
	// init /proc/fc_testing/...
	//
	proc_rg = proc_mkdir(TEST_MODULE_NAME, NULL); 
	
	printk("create proc/%s\n", TEST_MODULE_NAME);
	
	for( i=0; i< (sizeof(l34testProc)/sizeof(rtk_fcTesting_proc_t)) ;i++)
	{
	
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
		if(l34testProc[i].get==NULL)
			l34testProc[i].proc_fops.proc_open=common_fpga_single_open_null;
		else
			l34testProc[i].proc_fops.proc_open=common_fpga_rg_single_open;
		
		if(l34testProc[i].set==NULL)
			l34testProc[i].proc_fops.proc_write=NULL;
		else
			l34testProc[i].proc_fops.proc_write=common_fpga_rg_single_write;
		
		l34testProc[i].proc_fops.proc_read=seq_read;
		l34testProc[i].proc_fops.proc_lseek=seq_lseek;
		l34testProc[i].proc_fops.proc_release=single_release;
#else
		if(l34testProc[i].get==NULL)
			l34testProc[i].proc_fops.open=common_fpga_single_open_null;
		else
			l34testProc[i].proc_fops.open=common_fpga_rg_single_open;
		
		if(l34testProc[i].set==NULL)
			l34testProc[i].proc_fops.write=NULL;
		else
			l34testProc[i].proc_fops.write=common_fpga_rg_single_write;
		
		l34testProc[i].proc_fops.read=seq_read;
		l34testProc[i].proc_fops.llseek=seq_lseek;
		l34testProc[i].proc_fops.release=single_release;
#endif
		
		p = proc_create_data(l34testProc[i].name, S_IRUGO | S_IWUGO, proc_rg , &(l34testProc[i].proc_fops),NULL);

		if(!p)
		{
			printk("create proc/%s/%s failed!\n", TEST_MODULE_NAME, l34testProc[i].name);
		}
		else
		{
			printk("create proc/%s/%s p->low_ino = %u\n", TEST_MODULE_NAME, l34testProc[i].name, p->low_ino);
			l34testProc[i].inode_id[0] = p->low_ino;
		}
	}

}


__init int rtk_modeltest_init(void)
{
	printk("\r\n###  FC test module probe ... ### %s (%d)\r\n", __FUNCTION__, __LINE__);

	//debug_level = RTK_FB_DEBUG_LEVEL_TRACE | RTK_FB_DEBUG_LEVEL_WARN /*| RTK_FB_DEBUG_LEVEL_DEBUG*/;

	rtk_modeltest_proc_init();
	rtk_fc_skb_fctestHook_rxhook_register(rtk_fcTesting_netif_rx);
	rtk_test_user_pipe_cmd("echo 1 > proc/fc/ctrl/flow_skipAllPsTracking");
	rtk_test_user_pipe_cmd("echo 1 > proc/fc/ctrl/flow_l2_skipPsTracking");


	return SUCCESS;
}


static void __exit rtk_modeltest_exit(void)
{
	
	rtk_fc_skb_fctestHook_rxhook_register(NULL);
	return;
}


module_init(rtk_modeltest_init);
module_exit(rtk_modeltest_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Realtek Semiconductor Corp.");
MODULE_DESCRIPTION(TEST_MODULE_NAME" - FC test module");

