#include <linux/platform_device.h>
#include <bspchip.h>

#define PADDR(addr)  ((addr) & 0x1FFFFFFF)

static int __init rtl8198d_register_wdt(void)
{
	struct resource res;

	memset(&res, 0, sizeof(res));

	res.flags = IORESOURCE_MEM;
	res.start = PADDR(BSP_WDCNTRR); // wdt clock control
	res.end = res.start + 0xc - 1;
	platform_device_register_simple("rtl819x-wdt", -1, &res, 1);
	return 0;

}
arch_initcall(rtl8198d_register_wdt);

