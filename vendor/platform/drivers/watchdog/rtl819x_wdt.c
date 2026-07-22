#include <linux/bitops.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/watchdog.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/of.h>
#include <linux/of_platform.h>

#define DRIVER_NAME	 "rtl819x-wdt"

#define RTK_WDT_DBG	0
#define rtk_wdt_printk(fmt, arg...) \
	do { \
		if(RTK_WDT_DBG) \
			printk(fmt, ##arg); \
	} while(0)

static void __iomem *rtl819x_wdt_base;

#define DEFAULT_TIMEOUT	30
#define WDTCNTRR (0)
#define WDTINTRR (0x04)
#define WDTCTRLR (0x08)
#define WDT_E			      (1<<31)		/* Watchdog enable */
#define WDT_KICK		      (0x1 << 31)	/* Watchdog kick */
#define WDT_PH12_TO_MSK 	      31	       /* 11111b */
#define WDT_PH1_TO_SHIFT	      22
#define WDT_PH2_TO_SHIFT	      15
#define WDT_CLK_SC_SHIFT	      29

static unsigned int timeout_ph2 = 0;
static unsigned int clk_sc = 3;
static unsigned int rest_mode = 0;

//#define WDT_STOP_NOKICK

#if defined(WDT_STOP_NOKICK)
static unsigned char wdt_stopped = 0;
#endif


static bool nowayout = WATCHDOG_NOWAYOUT;
module_param(nowayout, bool, 0);
MODULE_PARM_DESC(nowayout,
		"Watchdog cannot be stopped once started (default="
		__MODULE_STRING(WATCHDOG_NOWAYOUT) ")");


static inline void rtk_wdt_w32(unsigned int reg, u32 val)
{
	iowrite32(val, (void __iomem *)((unsigned int)rtl819x_wdt_base + reg));
}

static inline u32 rtk_wdt_r32(unsigned int reg)
{
	return ioread32((void __iomem *)((unsigned int)rtl819x_wdt_base + reg));
}

#define RTK_WDG_EN()   (rtk_wdt_w32(WDTCTRLR, rtk_wdt_r32(WDTCTRLR) | WDT_E))
#define RTK_WDG_DIS()  (rtk_wdt_w32(WDTCTRLR , rtk_wdt_r32(WDTCTRLR) & (~WDT_E)))
#define RTK_WDG_KICK() (rtk_wdt_w32(WDTCNTRR, rtk_wdt_r32(WDTCNTRR) | WDT_KICK))

static void rtk_set_timer(unsigned int t)
{
	unsigned int reg_val = rtk_wdt_r32(WDTCTRLR);
	unsigned int res = 0;
	
	reg_val &= ~(WDT_PH12_TO_MSK << WDT_PH1_TO_SHIFT);

	rtk_wdt_printk("rtk_set_timer %u ", t);
	/* for LX bus 200MHz, time tick 1.34 s */
	res = (t*100)%134;
	t = (t*100)/134;
	if (res > (134/2))
		t++;
	if (t >= 1)
		t--;
	rtk_wdt_w32(WDTCTRLR,  reg_val | (t << WDT_PH1_TO_SHIFT));
	rtk_wdt_printk("0x%x\n", rtk_wdt_r32(WDTCTRLR));
}

static int rtl819x_wdt_ping(struct watchdog_device *w)
{	
#if defined(WDT_STOP_NOKICK)
	if(wdt_stopped) {
		printk("rtl819x_wdt_ping ignored, wdt stopped\n");
	} else 
#endif
	{
		RTK_WDG_KICK();
	}
	rtk_wdt_printk("rtl819x_wdt_ping \n");
	return 0;
}

static int rtl819x_wdt_set_timeout(struct watchdog_device *w, unsigned int t)
{
	if (t > 43)
		t = 43;
	w->timeout = t;	

	rtk_set_timer(t);	
	RTK_WDG_KICK();

	return 0;
}

static int rtl819x_wdt_start(struct watchdog_device *w)
{
	//printk("rtl819x_wdt_start\n");
	rtk_set_timer(w->timeout);
	RTK_WDG_EN();
	RTK_WDG_KICK();
	rtk_wdt_printk("%s\n", __func__);
	return 0;
}

static int rtl819x_wdt_stop(struct watchdog_device *w)
{
#if defined(WDT_STOP_NOKICK)
	if(w->timeout == 32) {
		wdt_stopped = 1;
		rtk_wdt_printk("rtl819x wdt stopped\n");
	}
#endif
	RTK_WDG_DIS();
	rtk_wdt_printk("rtl819x_wdt_stop\n");
	return 0;
}

static struct watchdog_info rtl819x_wdt_info = {
	.identity = DRIVER_NAME,
	.options = WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING | WDIOF_MAGICCLOSE,
};

static struct watchdog_ops rtl819x_wdt_ops = {
	.owner = THIS_MODULE,
	.start = rtl819x_wdt_start,
	.stop = rtl819x_wdt_stop,
	.ping = rtl819x_wdt_ping,
	.set_timeout = rtl819x_wdt_set_timeout,
};

static struct watchdog_device rtl819x_wdt_dev = {
	.info = &rtl819x_wdt_info,
	.ops = &rtl819x_wdt_ops,	
};

static void show_usage(void)
{
	unsigned int reg_val = rtk_wdt_r32(WDTCTRLR);

	printk("************ Watchdog Setting ****************\n");
	printk("WDT_E=%d, (1-enable, 0-disable)\n", ( (reg_val >> 31) & 0x1));
	//printk("LX(MHz)=%u\n", BSP_MHZ);
	printk("WDT_CLK_SC=%d\n", ( (reg_val >> 29) & 0x3));
	printk("PH1_TO=%d\n", ( (reg_val >> 22) & 0x1F));
	printk("PH2_TO=%d\n", ( (reg_val >> 15) & 0x1F));
	printk("WDT_RESET_MODE=%d\n", ( (reg_val ) & 0x3));
	printk("**********************************************\n");
}

static int rtl819x_wdt_clk_init(int sec)
{
	unsigned int reg = 0;

	reg = rtk_wdt_r32(WDTCTRLR);
	reg &= ~(WDT_PH12_TO_MSK << WDT_PH1_TO_SHIFT);

	reg &= ~(WDT_PH12_TO_MSK << WDT_PH2_TO_SHIFT);
	reg |= (timeout_ph2 << WDT_PH2_TO_SHIFT);

	reg |= (clk_sc << WDT_CLK_SC_SHIFT);
	reg |= rest_mode;

	if (sec == 0)
		rtk_wdt_w32(WDTCTRLR, reg | (31 << WDT_PH1_TO_SHIFT));
	else
	        rtk_wdt_w32(WDTCTRLR, reg | (sec << WDT_PH1_TO_SHIFT));

	show_usage();

	return 0;
}

static int rtl819x_wdt_probe(struct platform_device *pdev)
{
	struct resource *res;
	int ret;

	printk("rtl819x_wdt_probe!!\n");
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	rtl819x_wdt_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(rtl819x_wdt_base))
		return PTR_ERR(rtl819x_wdt_base);

	rtl819x_wdt_clk_init(DEFAULT_TIMEOUT);
	rtl819x_wdt_dev.parent = &pdev->dev;
	rtl819x_wdt_dev.timeout = DEFAULT_TIMEOUT;
	rtl819x_wdt_dev.max_timeout = 43;
	rtl819x_wdt_dev.min_timeout = 2;
	rtl819x_wdt_dev.timeout = 0;  

	watchdog_set_nowayout(&rtl819x_wdt_dev, nowayout);

	ret = watchdog_register_device(&rtl819x_wdt_dev);
	if (!ret)
		dev_info(&pdev->dev, "Initialized\n");

	return 0;
}

static int rtl819x_wdt_remove(struct platform_device *pdev)
{
	watchdog_unregister_device(&rtl819x_wdt_dev);

	return 0;
}

static void rtl819x_wdt_shutdown(struct platform_device *pdev)
{
	rtl819x_wdt_stop(&rtl819x_wdt_dev);
}

static struct platform_driver rtl819x_wdt_driver = {
	.probe		= rtl819x_wdt_probe,
	.remove		= rtl819x_wdt_remove,
	.shutdown	= rtl819x_wdt_shutdown,
	.driver		= {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
		//.of_match_table = of_match_ptr(rtl819x_wdt_match),
	},
};

module_platform_driver(rtl819x_wdt_driver);

MODULE_DESCRIPTION("Realtek RTL819X hardware watchdog driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" DRIVER_NAME);
MODULE_ALIAS_MISCDEV(WATCHDOG_MINOR);

