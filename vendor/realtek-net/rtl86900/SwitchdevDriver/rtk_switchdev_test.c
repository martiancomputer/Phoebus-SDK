#include <generated/autoconf.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/compiler.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/in.h>
#include <linux/if_vlan.h>
#include <linux/crc32.h>
#include <linux/inet.h>
#include <linux/if_pppox.h>
#include <linux/ppp_defs.h>
#include <linux/icmp.h>
#include <linux/igmp.h>
#include <linux/of_net.h>
#include <linux/of_device.h>
#include <net/ipv6.h>
#include <net/tcp.h>
#include <net/udp.h>
#include <net/ip6_checksum.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <net/xfrm.h>
#include <linux/proc_fs.h>
#include <linux/if_bridge.h>
#include <net/switchdev.h>
#include "rtk_switchdev_hw.h"
#include "rtk_switchdev_ethtool.h"
#include "error.h"
#include "rt_l2.h"
#include "rt_stat.h"

#define PROC_FILENAME "rtk_switchdev_test"
#define CMD_LEN 8
#define MAC_ADDR_LEN 18

static struct proc_dir_entry *proc_file;

static ssize_t test_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos) {
	struct rtk_switchdev_hw_notifier_fdb_info info;
    char *kbuf, cmd_buf[8], mac_buf[18];
    int ret;
    unsigned char addr[ETH_ALEN];
    u16 vid;
    int port;

    // 分配內存
    kbuf = kmalloc(count + 1, GFP_KERNEL);
    if (!kbuf) {
        return -ENOMEM;
    }

    // 從用戶空間複製數據
    if (copy_from_user(kbuf, buffer, count)) {
        kfree(kbuf);
        return -EFAULT;
    }
    kbuf[count] = '\0';

   // 解析命令、MAC 地址、VID 和 PORT
    ret = sscanf(kbuf, "%7s %17s vid %hu port %d", cmd_buf, mac_buf, &vid, &port);
    if (ret != 4) {
        kfree(kbuf);
        return -EINVAL;
    }

    // 檢查命令
    if (strcmp(cmd_buf, "add_fdb") == 0 || strcmp(cmd_buf, "del_fdb") == 0) {
        // 解析 MAC 地址
        if (!mac_pton(mac_buf, addr)) {
            kfree(kbuf);
            return -EINVAL;
        }

        printk(KERN_INFO "RTK Switchdev Test: Command = %s, MAC Address = %pM, VID = %u, Port = %u\n", cmd_buf, addr, vid, port);

		if (strcmp(cmd_buf, "add_fdb") == 0) {
			info.addr = addr;
			info.vid = vid;
			call_rtk_switchdev_notifiers(RTK_SWITCHDEV_FDB_ADD_TO_PORT, port, &info.info, NULL);
		}

		if (strcmp(cmd_buf, "del_fdb") == 0) {
			info.addr = addr;
			info.vid = vid;
			call_rtk_switchdev_notifiers(RTK_SWITCHDEV_FDB_DEL_TO_PORT, port, &info.info, NULL);
		}
		
    } else {
        kfree(kbuf);
        return -EINVAL;
    }

    kfree(kbuf);
    return count;
}

static int test_show_help(struct seq_file *m, void *v)
{
	seq_printf(m, "\n echo add_fdb/del_fdb [MAC addr]\n");

	return 0;
}

int test_open(struct inode *inode, struct file *file)
{
	return single_open(file, test_show_help, (void *) file);
}

static const struct proc_ops fops_test __read_mostly = {
	.proc_open           = test_open,
	.proc_read           = seq_read,
	.proc_write          = test_write,
};

static int __init rtk_switchdev_test_init(void) {
    proc_file = proc_create(PROC_FILENAME, 0666, NULL, &fops_test);
    if (!proc_file) {
        return -ENOMEM;
    }
    printk(KERN_INFO "RTK Switchdev Test Module Initialized.\n");
    return 0;
}

static void __exit rtk_switchdev_test_exit(void) {
    proc_remove(proc_file);
    printk(KERN_INFO "RTK Switchdev Test Module Exited.\n");
}

module_init(rtk_switchdev_test_init);
module_exit(rtk_switchdev_test_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("RTK Switchdev Test Module");
MODULE_VERSION("1.0");

