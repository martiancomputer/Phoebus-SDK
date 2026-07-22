#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/utsname.h>
#include <linux/version.h>

#include <linux/module.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/slab.h>

#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/io.h>

#include <ca_types.h>

#define MAGIC_NUM		 0x7533967
#if 0
#if defined(CONFIG_ARCH_CORTINA_VENUS) || defined(CONFIG_ARCH_REALTEK_TAURUS)
#define PELOG_CPU_NUM 2
#elif defined(CONFIG_ARCH_REALTEK_9607F)
#define PELOG_CPU_NUM 3
#else
#errro "no support"
#endif
#endif
uint32_t PELOG_CPU_NUM = 0;

#define BUF_SIZE 16384
struct pe_seq_context {
	int32_t pe_id;
	char *log_ptr;
	int32_t log_size;
	int32_t data_len;
	int32_t show_state;
	int32_t done;
};

typedef struct {
	uint32_t magicNum;
	uint32_t logOverwrite;
	uint32_t write_offset;
//#ifdef CONFIG_ARCH_REALTEK_9607F
	uint32_t last_wb_offset;
	char resvd[16];//for cache line align
//#endif
	char data[];
}__attribute__ ((__packed__)) pelog_header_t;


typedef struct pelog_module {
	void __iomem* pelog_shm_vaddr;
	resource_size_t pelog_shm_paddr;
	resource_size_t shm_size;
	uint32_t u32ReadOffset;
	char*		rebuild_msgBuf;
}pelog_module_t;


pelog_module_t pelog_module0,pelog_module1;
//#if PELOG_CPU_NUM >= 3
pelog_module_t pelog_module2;
//#endif

MODULE_LICENSE("GPL");
#ifdef CONFIG_OF
/* Match table for of_platform binding */
static const struct of_device_id cortina_pelog_of_match[] = {
        { .compatible = "cortina,pelog", },
        {},
};
MODULE_DEVICE_TABLE(of, cortina_pelog_of_match);
#endif

int isPelogInitSucess0 = 0,isPelogInitSucess1 = 0;
//#if PELOG_CPU_NUM >= 3
int isPelogInitSucess2 = 0;
//#endif

static int ca_pelog_probe(struct platform_device *pdev)
{
	pelog_header_t *pPelog;
	struct device_node *np;
	struct resource mem_resource;
	const struct of_device_id *match;
	int ret=-ENOMEM;

	printk("%s \n", __func__);
	///* assign DT node pointer */
	//np = pdev->dev.of_node;

		/* search DT for a match */
	match = of_match_device(cortina_pelog_of_match, &pdev->dev);
	if (!match){
		return -EINVAL;
	}

	np = of_parse_phandle(pdev->dev.of_node, "memory-region", 0);
	if (!np) {
  		printk("No %s specified\n", "memory-region");
  		return ret;
	}

	/* get "pelog0 shm" from DT and convert to platform mem address resource */
	ret = of_address_to_resource(np, 0, &mem_resource);
	if (ret) {
		printk("%s:pelog of_address_to_resource return %d\n", __func__, ret);
		return ret;
	}

	/* get max pe */
	if (of_property_read_u32(pdev->dev.of_node, "max-pe", &PELOG_CPU_NUM)) {
		//module_context->max_pe = 2;
		printk(KERN_ERR "IPC: missing reg property 'max-pe'.\n");
		return -EINVAL;
	}

	//pelog_module0.shm_size = resource_size(&mem_resource)/PELOG_CPU_NUM;
	pelog_module0.shm_size = resource_size(&mem_resource);
	do_div(pelog_module0.shm_size, PELOG_CPU_NUM);
	pelog_module0.pelog_shm_vaddr =
		devm_ioremap(&pdev->dev, mem_resource.start, pelog_module0.shm_size);

	if (!pelog_module0.pelog_shm_vaddr) {
		printk("pelog0: devm_ioremap fail\n");
		return -ENOMEM;
	}

	pelog_module0.pelog_shm_paddr = mem_resource.start;
	pelog_module0.u32ReadOffset = 0;

	printk("pelog0: shm physical address : 0x%lx\n", (unsigned long)pelog_module0.pelog_shm_paddr);
	printk("pelog0: shm virtual address : 0x%lx\n", (unsigned long)pelog_module0.pelog_shm_vaddr);
	printk("pelog0: shm size: 0x%lx\n", (unsigned long) pelog_module0.shm_size);
	printk("pelog0: reset magicNum\n");

	pPelog = (pelog_header_t*) pelog_module0.pelog_shm_vaddr;
	pPelog->magicNum = 0;

	pelog_module0.rebuild_msgBuf = kmalloc(pelog_module0.shm_size, GFP_KERNEL);

	if (!pelog_module0.rebuild_msgBuf) {
		printk("pelog0: kmalloc fail\n");
		return -ENOMEM;
	}

	isPelogInitSucess0 = 1;

		/* get "pelog1 shm" from DT and convert to platform mem address resource */
	//ret = of_address_to_resource(np, 1, &mem_resource);
	//if (ret) {
	//	printk("%s:pelog1 of_address_to_resource return %d\n", __func__, ret);
	//	return ret;
	//}

	//pelog_module1.shm_size = resource_size(&mem_resource)/PELOG_CPU_NUM;
	pelog_module1.shm_size = resource_size(&mem_resource);
	do_div(pelog_module1.shm_size, PELOG_CPU_NUM);

	pelog_module1.pelog_shm_vaddr =
		devm_ioremap(&pdev->dev, (mem_resource.start+pelog_module0.shm_size), pelog_module1.shm_size);

	if (!pelog_module1.pelog_shm_vaddr) {
		printk("pelog1: devm_ioremap fail\n");
		return -ENOMEM;
	}

	pelog_module1.pelog_shm_paddr = mem_resource.start+pelog_module0.shm_size;
	pelog_module1.u32ReadOffset = 0;

	printk("pelog1: shm physical address : 0x%lx\n", (unsigned long)pelog_module1.pelog_shm_paddr);
	printk("pelog1: shm virtual address : 0x%lx\n", (unsigned long)pelog_module1.pelog_shm_vaddr);
	printk("pelog1: shm size: 0x%lx\n", (unsigned long) pelog_module1.shm_size);
	printk("pelog1: reset magicNum\n");

	pPelog = (pelog_header_t*) pelog_module1.pelog_shm_vaddr;
	pPelog->magicNum = 0;

	pelog_module1.rebuild_msgBuf = kmalloc(pelog_module1.shm_size, GFP_KERNEL);

	if (!pelog_module1.rebuild_msgBuf) {
		printk(KERN_INFO "pelog1: kmalloc fail\n");
		return -ENOMEM;
	}

	isPelogInitSucess1 = 1;

//#if PELOG_CPU_NUM >= 3
	if (PELOG_CPU_NUM >= 3) {
		//pelog_module2.shm_size = resource_size(&mem_resource)/PELOG_CPU_NUM;
		pelog_module2.shm_size = resource_size(&mem_resource);
		do_div(pelog_module2.shm_size, PELOG_CPU_NUM);

		pelog_module2.pelog_shm_vaddr =
			devm_ioremap(&pdev->dev, (mem_resource.start+pelog_module1.shm_size+pelog_module0.shm_size), pelog_module2.shm_size);

		if (!pelog_module2.pelog_shm_vaddr) {
			printk(KERN_INFO "pelog2: devm_ioremap fail\n");
			return -ENOMEM;
		}

		pelog_module2.pelog_shm_paddr = mem_resource.start+pelog_module0.shm_size+pelog_module1.shm_size;
		pelog_module2.u32ReadOffset = 0;

		printk(KERN_INFO "pelog2: shm physical address : 0x%lx\n", (unsigned long)pelog_module2.pelog_shm_paddr);
		printk(KERN_INFO "pelog2: shm virtual address : 0x%lx\n", (unsigned long)pelog_module2.pelog_shm_vaddr);
		printk(KERN_INFO "pelog2: shm size: 0x%lx\n", (unsigned long) pelog_module2.shm_size);
		printk(KERN_INFO "pelog2: reset magicNum\n");

		pPelog = (pelog_header_t *) pelog_module2.pelog_shm_vaddr;
		pPelog->magicNum = 0;

		pelog_module2.rebuild_msgBuf = kmalloc(pelog_module2.shm_size, GFP_KERNEL);

		if (!pelog_module2.rebuild_msgBuf) {
			printk(KERN_INFO "pelog2: kmalloc fail\n");
			return -ENOMEM;
		}

		isPelogInitSucess2 = 1;
	}
//#endif

	return 0;
}

static int ca_pelog_remove(struct platform_device *op)
{
	return 0;
}


static struct platform_driver ca_pelog_driver = {
	.probe = ca_pelog_probe,
	.remove = ca_pelog_remove,
	.driver = {
		   .owner = THIS_MODULE,
		   .name = "ca_pelog",
		   .of_match_table = of_match_ptr(cortina_pelog_of_match),
		   },
};


static int __init ca_pelog_init(void)
{
	//return init_procfs_pelog();
	return platform_driver_register(&ca_pelog_driver);
}

static void __exit ca_pelog_exit(void)
{
	platform_driver_unregister(&ca_pelog_driver);
}

module_init(ca_pelog_init);
module_exit(ca_pelog_exit);

char *pelog_get_log(int pe_id, uint32_t *size)
{
	pelog_module_t *pmodule;
	pelog_header_t *pelog;
	uint64_t header_size;
	uint64_t msgBuf_size;
	uint32_t logOffset;
	uint32_t logOverwrite;

	if (pe_id == 0) {
		if (isPelogInitSucess0 == 0) {
			pr_err("Pelog0 init fail\n");
			return NULL;
		}
		pmodule = &pelog_module0;
	} else if (pe_id == 1) {
		if (isPelogInitSucess1 == 0) {
			pr_err("Pelog1 init fail\n");
			return NULL;
		}
		pmodule = &pelog_module1;
	}
//#if PELOG_CPU_NUM >= 3
	else if (pe_id == 2 && PELOG_CPU_NUM >= 3) {
		if (isPelogInitSucess2 == 0) {
			pr_err("Pelog2 init fail\n");
			return NULL;
		}
		pmodule = &pelog_module2;
	}
//#endif
	else {
		pr_err("pe_id %d is wrong.\n", pe_id);
		return NULL;
	}

	pelog = (pelog_header_t *)pmodule->pelog_shm_vaddr;
	header_size = (uint64_t)(pelog->data) - (uint64_t)pelog;
	msgBuf_size = (uint64_t)pmodule->shm_size - (uint64_t)header_size;
#if 0
#ifdef CONFIG_ARCH_REALTEK_9607F
	logOffset = pelog->last_wb_offset;
#else
	logOffset = pelog->write_offset;
#endif
#endif
	if (PELOG_CPU_NUM >= 3) {
		logOffset = pelog->last_wb_offset;
	} else {
		logOffset = pelog->write_offset;
	}
	logOverwrite = pelog->logOverwrite;

	if (pelog->magicNum != MAGIC_NUM) {
		pr_err("PEDSP%d's Pelog not enable\n", pe_id);
		return NULL;
	}

	if (logOffset == pmodule->u32ReadOffset && logOverwrite == 0) {
		pr_info("PEDSP%d's Pelog no log\n", pe_id);
		return NULL;
	}

	memset(pmodule->rebuild_msgBuf, 0, pmodule->shm_size);
	if (logOverwrite == 0) {
		//printk("%s %d\n",__FUNCTION__,__LINE__);
		if (pmodule->u32ReadOffset > logOffset) { //DSP reboot
			pr_info("PEDSP%d Pelog log reset\n", pe_id);
			memcpy_fromio(pmodule->rebuild_msgBuf, (pelog->data), logOffset);
			*size = logOffset;
		} else {
			memcpy_fromio(pmodule->rebuild_msgBuf, (pelog->data) + pmodule->u32ReadOffset, logOffset - pmodule->u32ReadOffset);
			*size = logOffset - pmodule->u32ReadOffset;
		}
	} else if (logOverwrite == 1) {
		if (pmodule->u32ReadOffset > logOffset) {
			memcpy_fromio(pmodule->rebuild_msgBuf, (pelog->data) + pmodule->u32ReadOffset, (msgBuf_size - pmodule->u32ReadOffset));
			*size = msgBuf_size - pmodule->u32ReadOffset;
			memcpy_fromio((pmodule->rebuild_msgBuf)+(msgBuf_size-pmodule->u32ReadOffset), (pelog->data), logOffset);
			*size += logOffset;
			pelog->logOverwrite = 0;
		} else {
			pr_info("PEDSP%d Pelog log overwrite\n", pe_id);
			memcpy_fromio(pmodule->rebuild_msgBuf, (pelog->data)+logOffset, (msgBuf_size-logOffset));
			*size = msgBuf_size-logOffset;
			memcpy_fromio((pmodule->rebuild_msgBuf)+(msgBuf_size-logOffset), (pelog->data), logOffset);
			*size += logOffset;
			pelog->logOverwrite = 0;
		}

		//memcpy(pelog_module.rebuild_msgBuf,(pelog->data)+logOffset,(msgBuf_size-logOffset));
		//memcpy((pelog_module.rebuild_msgBuf)+(msgBuf_size-logOffset),(pelog->data),logOffset);
		//printk("%s %d\n",__FUNCTION__,__LINE__);
	} else {
		pr_info("PEDSP%d Pelog log overwrite more then once\n", pe_id);
		memcpy_fromio(pmodule->rebuild_msgBuf, (pelog->data)+logOffset, (msgBuf_size-logOffset));
		*size = msgBuf_size-logOffset;
		memcpy_fromio((pmodule->rebuild_msgBuf)+(msgBuf_size-logOffset), (pelog->data), logOffset);
		*size += logOffset;
		pelog->logOverwrite = 0;
	}
	pmodule->u32ReadOffset = logOffset;
	return (char *)pmodule->rebuild_msgBuf;
}

static void *pe_seq_start(struct seq_file *s, loff_t *pos)
{
	struct pe_seq_context *p = (struct pe_seq_context *)s->private;

	if (p->done)
		return NULL;

	if (p->show_state == 1)
		return p->log_ptr;

	if ((*pos) == 0) {
		uint32_t size;

		p->log_ptr = pelog_get_log(p->pe_id, &size);
		p->log_size = (int32_t)size;
		if (p->log_ptr == NULL)
			return NULL;
	} else
		return p->log_ptr;

	if (p->log_size > (BUF_SIZE - 1))
		p->data_len = BUF_SIZE - 1;
	else
		p->data_len = p->log_size;
	p->show_state = 1;
	return p->log_ptr;
}

static void *pe_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	struct pe_seq_context *p = (struct pe_seq_context *)s->private;

	(*pos)++;
	if (p->done)
		return NULL;

	if (p->show_state == 1)
		return p->log_ptr;

	if (p->log_size > (BUF_SIZE - 1))
		p->data_len = BUF_SIZE - 1;
	else
		p->data_len = p->log_size;
	p->show_state = 1;
	return p->log_ptr;
}

static void pe_seq_stop(struct seq_file *s, void *v)
{
}

static int pe_seq_show(struct seq_file *s, void *v)
{
	struct pe_seq_context *p = (struct pe_seq_context *)s->private;

	//seq_printf(s, (char *)v);
	if (seq_write(s, p->log_ptr, p->data_len) == 0) {
		p->log_ptr = p->log_ptr + p->data_len;
		p->log_size = p->log_size - p->data_len;
		p->show_state = 0;
		p->data_len = 0;
		if (p->log_size <= 0)
			p->done = 1;
	}
	return 0;
}

static const struct seq_operations pe_seq_ops = {
	.start = pe_seq_start,
	.next = pe_seq_next,
	.stop = pe_seq_stop,
	.show = pe_seq_show
};

static int pelog0_proc_open(struct inode *inode, struct file *file)
{
	struct pe_seq_context *p = __seq_open_private(file, &pe_seq_ops, sizeof(*p));

	if (!p)
		return -ENOMEM;
	memset(p, 0, sizeof(*p));
	p->pe_id = 0;
	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
static const struct proc_ops pelog0_proc_fops = {
	.proc_open		= pelog0_proc_open,
	.proc_read		= seq_read,
	.proc_lseek		= seq_lseek,
	.proc_release	= seq_release_private,
};
#else
static const struct file_operations pelog0_proc_fops = {
	.open		= pelog0_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release_private,
};
#endif

static int pelog1_proc_open(struct inode *inode, struct file *file)
{
	struct pe_seq_context *p = __seq_open_private(file, &pe_seq_ops, sizeof(*p));

	if (!p)
		return -ENOMEM;
	memset(p, 0, sizeof(*p));
	p->pe_id = 1;
	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
static const struct proc_ops pelog1_proc_fops = {
	.proc_open		= pelog1_proc_open,
	.proc_read		= seq_read,
	.proc_lseek		= seq_lseek,
	.proc_release	= seq_release_private,
};
#else
static const struct file_operations pelog1_proc_fops = {
	.open		= pelog1_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release_private,
};
#endif

//#if PELOG_CPU_NUM >= 3
static int pelog2_proc_open(struct inode *inode, struct file *file)
{
	struct pe_seq_context *p = __seq_open_private(file, &pe_seq_ops, sizeof(*p));

	if (!p)
		return -ENOMEM;
	memset(p, 0, sizeof(*p));
	p->pe_id = 2;
	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
static const struct proc_ops pelog2_proc_fops = {
	.proc_open		= pelog2_proc_open,
	.proc_read		= seq_read,
	.proc_lseek		= seq_lseek,
	.proc_release	= seq_release_private,
};
#else
static const struct file_operations pelog2_proc_fops = {
	.open		= pelog2_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release_private,
};
#endif
//#endif

static int __init proc_pelog_init(void)
{
	proc_create("pelog0", 0, NULL, &pelog0_proc_fops);
	proc_create("pelog1", 0, NULL, &pelog1_proc_fops);
//#if PELOG_CPU_NUM >= 3
	proc_create("pelog2", 0, NULL, &pelog2_proc_fops);
//#endif
	return 0;
}
fs_initcall(proc_pelog_init);
