// SPDX-License-Identifier: GPL-2.0-only
#define DEBUG
#include <linux/clk-provider.h>
#include <linux/clkdev.h>
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <soc/cortina/registers.h>
#include <soc/cortina/cortina-soc.h>

#define to_rtl9607f_pll_clk(_hw) container_of(_hw, struct rtl9607f_pll_clk, hw)
#define to_rtl9607f_cpu_daemon(_hw) \
		container_of(_hw, struct rtl9607f_cpu_daemon, hw)
#define clk_mask(width) ((1 << width) - 1)

#define RTK_DTS_CPU_SPEED_INIT 1
#define RTK_CLK_DIV_PATCH 1

enum {
	CA_CLK_REF,
	CA_CLK_PLL,
	CA_CLK_DIVIDER,
	CA_CLK_MUX,
	CA_CLK_GATE,
	CA_CLK_FIXED_FACTOR,
	CA_CLK_PE_DAEMON,
	CA_CLK_CPU_DAEMON,
};

enum {
	PED_REF,
	PED_FPLL,
	PED_DIV_F,
	PED_MUX_PE,
	PED_CLKS_NUM,
};

enum {
	CPUD_REF,
	CPUD_CPLL,
	CPUD_DIV_F2C,
	CPUD_MUX_CPU,
	CPUD_DIV_CORTEX,
	CPUD_CLKS_NUM,
};

#if defined( CONFIG_ARCH_REALTEK_9607F )
#define DIVN_BASE		3
#define DIVN_SHIFT		15
#define DIVN_WIDTH		9
#define PREDIV_BASE		2
#define PREDIV_SHIFT		12
#define PREDIV_WIDTH		2
#define DIV_PREDIV_BPS_SHIFT	14
#define DIV_PREDIV_BPS_WIDTH	1
#else
#error "Platform not support!"
#endif
#define PREDIV_VALUE		2
#define REF_40MHZ		40000000
#define REF_25MHZ		25000000
#define STRAP_XTAL_SEL	(1<<3)
#define GLOBAL_STRAP_SPEED_OFF		1
#define GLOBAL_STRAP_SPEED_MASK		0x3
// delay for pll lock should be more than 160us
#define MIN_DELAY_PLL_FOR_F2c		2000
#define MIN_DELAY_PLL_FOR_LOCK		500
#define MIN_DELAY_PLL_FOR_SWITCH	10

struct rtl9607f_clk_reg {
	void __iomem *regbase;
	spinlock_t regs_lock;	/* register access lock */
};

struct rtl9607f_clk_param {
	int type;
	u32 reg;
};

struct rtl9607f_pll_clk {
	void __iomem *reg;
	spinlock_t *lock;	/* register access lock */
	struct clk_hw hw;
};

struct rtl9607f_pll_param {
	unsigned int    pll;
	unsigned int    prediv;
	unsigned int    divn;
};

struct rtl9607f_cpu_strap_speed {
	unsigned long rate;
};

struct rtl9607f_pe_speed {
	unsigned int    pll;
	unsigned int    div;
};

struct rtl9607f_cpu_daemon {
	void __iomem *reg;
	struct mutex mlock;	/* daemon lock */
	spinlock_t *lock;	/* register access lock */
	struct clk_hw hw;
	struct clk *clks[CPUD_CLKS_NUM];
};

#define CPU_MAX_SPEED	1200000000
#define CPU_MIN_SPEED	 500000000

static const struct rtl9607f_cpu_strap_speed rtl9607f_cpu_strap_speed_table[] = {
	{ CPU_MIN_SPEED },
	{  600000000 },
	{ 1000000000 },
	{ CPU_MAX_SPEED }, /* 1200000000 */
	{ }
};

static struct rtl9607f_clk_param pll_ref      = {CA_CLK_REF, GLOBAL_STRAP};
static struct rtl9607f_clk_param pll_cpll     = {CA_CLK_PLL, GLOBAL_CPLL0};
static struct rtl9607f_clk_param pll_epll     = {CA_CLK_PLL, GLOBAL_EPLL0};
static struct rtl9607f_clk_param pll_fpll     = {CA_CLK_PLL, GLOBAL_FPLL0};
static struct rtl9607f_clk_param mux_cplldiv  = {CA_CLK_MUX, GLOBAL_CPLLDIV};
static struct rtl9607f_clk_param mux_pediv    = {CA_CLK_MUX, GLOBAL_PEDIV};
static struct rtl9607f_clk_param div_cplldiv  = {CA_CLK_DIVIDER, GLOBAL_CPLLDIV};
static struct rtl9607f_clk_param div_fplldiv  = {CA_CLK_DIVIDER, GLOBAL_PEDIV};
static struct rtl9607f_clk_param div_eplldiv  = {CA_CLK_DIVIDER, GLOBAL_EPLLDIV};
static struct rtl9607f_clk_param div_eplldiv2 = {CA_CLK_DIVIDER, GLOBAL_EPLLDIV2};
static struct rtl9607f_clk_param gate_config  = {CA_CLK_GATE, GLOBAL_GLOBAL_CONFIG};
static struct rtl9607f_clk_param fixed_factor = {CA_CLK_FIXED_FACTOR, 0};
static struct rtl9607f_clk_param pe_daemon   = {CA_CLK_PE_DAEMON, 0};
static struct rtl9607f_clk_param cpu_daemon   = {CA_CLK_CPU_DAEMON, 0};


static const struct of_device_id rtl9607f_soc_clks_ids[] __initconst = {
	{ .compatible = "realtek,rtl9607f-pll-ref-clk",
	  .data = (void *)&pll_ref, },
	{ .compatible = "realtek,rtl9607f-pll-cpll-clk",
	  .data = (void *)&pll_cpll, },
	{ .compatible = "realtek,rtl9607f-pll-fpll-clk",
	  .data = (void *)&pll_fpll, },
	{ .compatible = "realtek,rtl9607f-pll-epll-clk",
	  .data = (void *)&pll_epll, },
	{ .compatible = "realtek,mux-cpu-clk",
	  .data = (void *)&mux_cplldiv, },
	{ .compatible = "realtek,mux-pe-clk",
	  .data = (void *)&mux_pediv, },
	{ .compatible = "realtek,div-cpll-clk",
	  .data = (void *)&div_cplldiv, },
	{ .compatible = "realtek,div-fpll-clk",
	  .data = (void *)&div_fplldiv, },
	{ .compatible = "realtek,div-epll-clk",
	  .data = (void *)&div_eplldiv, },
	{ .compatible = "realtek,div-epll2-clk",
	  .data = (void *)&div_eplldiv2, },
	{ .compatible = "realtek,gate-config-clk",
	  .data = (void *)&gate_config, },
	{ .compatible = "realtek,fixed-factor-clk",
	  .data = (void *)&fixed_factor, },
	{ .compatible = "realtek,rtl9607f-cpu-daemon-clk",
	  .data = (void *)&cpu_daemon, },
	{ .compatible = "realtek,rtl9607f-pe-daemon-clk",
	  .data = (void *)&pe_daemon, },
	{},
};

static bool readonly;

static unsigned long rtl9607f_pll_recalc_rate(struct clk_hw *hw,
				     unsigned long parent_rate)
{
	struct rtl9607f_pll_clk *pll_clk = to_rtl9607f_pll_clk(hw);
	u32 val, divn, prediv;
#ifdef CONFIG_ARM
	u64 result = 0;
#endif

	val = readl(pll_clk->reg);

	if(parent_rate == REF_25MHZ){
			val |= (1 << DIV_PREDIV_BPS_SHIFT);
	}else{
			val &= ~(1 << DIV_PREDIV_BPS_SHIFT);
	}
	writel(val, pll_clk->reg);

	val = readl(pll_clk->reg) >> DIVN_SHIFT;
	val &= clk_mask(DIVN_WIDTH);
	divn = val + DIVN_BASE;

	val = readl(pll_clk->reg);
	if((val >> DIV_PREDIV_BPS_SHIFT) & clk_mask(DIV_PREDIV_BPS_WIDTH)){
		prediv = 1;
	}else{
		val = val >> PREDIV_SHIFT;
		val &= clk_mask(PREDIV_WIDTH);
		prediv = val + PREDIV_BASE;
	}

#ifdef CONFIG_ARM
	result = (u64)parent_rate * (u64)divn;
	do_div(result, prediv);
	return result;
#else
	return (parent_rate * divn / prediv);
#endif
}

static long rtl9607f_pll_round_rate(struct clk_hw *hw, unsigned long rate,
			   unsigned long *parent_rate)
{
	unsigned long mult;
	unsigned long divisor;

	if ((*parent_rate) == REF_40MHZ)
		divisor = ((*parent_rate)/PREDIV_VALUE);
	else
		divisor = (*parent_rate);

	if ((rate % divisor) != 0)
		goto NOT_SUPPORT;

	mult = rate / divisor;

	return mult*divisor;

NOT_SUPPORT:
	return *parent_rate;
}

static int rtl9607f_pll_set_rate(struct clk_hw *hw, unsigned long rate,
			unsigned long parent_rate)
{
	struct rtl9607f_pll_clk *pll_clk = to_rtl9607f_pll_clk(hw);
	unsigned long mult;
	unsigned long flags = 0;
	u32 val, divn;
	u32 divisor;

	if (parent_rate == REF_40MHZ)
		divisor = parent_rate / PREDIV_VALUE;
	else
		divisor = parent_rate;

	if ((rate % divisor) != 0)
		return 0;

	mult = rate / divisor;

	//REF_25MHZ, BPS=1, PREDIV_VALUE not effect
	divn = mult;

	spin_lock_irqsave(pll_clk->lock, flags);
	val = readl(pll_clk->reg);

	val &= ~(clk_mask(DIVN_WIDTH) << DIVN_SHIFT);
	val |= (divn - DIVN_BASE) << DIVN_SHIFT;
	val &= ~(clk_mask(PREDIV_WIDTH) << PREDIV_SHIFT);
/*
 * 9607F not set up DIV_PREDIV_SEL[1][0]
 *	val |= (PREDIV_VALUE - PREDIV_BASE) << PREDIV_SHIFT;
 */
	writel(val, pll_clk->reg);

	spin_unlock_irqrestore(pll_clk->lock, flags);

	return rate;
}

static const struct clk_ops rtl9607f_pll_ops = {
	.recalc_rate	= rtl9607f_pll_recalc_rate,
	.round_rate	= rtl9607f_pll_round_rate,
	.set_rate	= rtl9607f_pll_set_rate,
};


static int rtl9607f_clk_ref(struct device_node *np, struct rtl9607f_clk_reg *clk_reg,
		   struct rtl9607f_clk_param *clk_param)
{
	void __iomem *reg;
	struct clk *soc_clk;
	__maybe_unused  u32 reg_val;
	u32 rate = REF_25MHZ;

	reg = clk_reg->regbase + clk_param->reg - GLOBAL_JTAG_ID;

	reg_val = readl_relaxed(reg);

	if (reg_val & STRAP_XTAL_SEL)
		rate = REF_40MHZ;

	soc_clk = clk_register_fixed_rate(NULL, np->name, NULL, 0, rate);
	if (!IS_ERR(soc_clk))
		of_clk_add_provider(np, of_clk_src_simple_get, soc_clk);

	return IS_ERR(soc_clk) ? PTR_ERR(soc_clk) : 0;
}


static int rtl9607f_clk_pll(struct device_node *np, struct rtl9607f_clk_reg *clk_reg,
		   struct rtl9607f_clk_param *clk_param)
{
	const char *parent_name;
	void __iomem *reg;
	struct clk_init_data init;
	struct rtl9607f_pll_clk *pll_clk;
	struct clk *soc_clk;

	parent_name = of_clk_get_parent_name(np, 0);
	reg = clk_reg->regbase + clk_param->reg - GLOBAL_JTAG_ID;

	pll_clk = kzalloc(sizeof(*pll_clk), GFP_KERNEL);
	if (!pll_clk)
		return -ENOMEM;

	init.name = np->name;
	init.ops = &rtl9607f_pll_ops;
	init.parent_names = (parent_name ? &parent_name : NULL);
	init.num_parents = (parent_name ? 1 : 0);

	pll_clk->hw.init = &init;
	pll_clk->reg = reg;
	pll_clk->lock = &clk_reg->regs_lock;

	soc_clk = clk_register(NULL, &pll_clk->hw);

	if (!IS_ERR(soc_clk))
		of_clk_add_provider(np, of_clk_src_simple_get, soc_clk);
	else
		kfree(pll_clk);

	return IS_ERR(soc_clk) ? PTR_ERR(soc_clk) : 0;
}

static int rtl9607f_clk_divider(struct device_node *np, struct rtl9607f_clk_reg *clk_reg,
		       struct rtl9607f_clk_param *clk_param)
{
	u8 op_shift, op_width;
	const char *parent_name;
	void __iomem *reg;
	struct clk *soc_clk;
	u8 init_div = 0;
	u32 val;
	unsigned long flags = 0;

	if (of_property_read_u8(np, "operate-shift", &op_shift))
		return -EPERM;
	if (op_shift >= 32)
		return -EINVAL;
	if (of_property_read_u8(np, "operate-width", &op_width))
		return -EPERM;
	if (op_width >= 32)
		return -EINVAL;

	parent_name = of_clk_get_parent_name(np, 0);
	reg = clk_reg->regbase + clk_param->reg - GLOBAL_JTAG_ID;

	if (of_property_read_u8(np, "init-div", &init_div))
		init_div = 0;

	if (init_div) {
		spin_lock_irqsave(&clk_reg->regs_lock, flags);

		val = readl(reg);

		val &= ~(clk_mask(op_width) << op_shift);
		val |= (init_div << 1) << op_shift;

		writel(val, reg);

		spin_unlock_irqrestore(&clk_reg->regs_lock, flags);
	}


	soc_clk = clk_register_divider(NULL, np->name, parent_name, 0,
					       reg, op_shift + 1, op_width - 1,
					       CLK_DIVIDER_ONE_BASED |
					       CLK_DIVIDER_ALLOW_ZERO,
					       &clk_reg->regs_lock);

	if (!IS_ERR(soc_clk))
		of_clk_add_provider(np, of_clk_src_simple_get, soc_clk);

	return IS_ERR(soc_clk) ? PTR_ERR(soc_clk) : 0;
}

static int rtl9607f_clk_mux(struct device_node *np, struct rtl9607f_clk_reg *clk_reg,
		   struct rtl9607f_clk_param *clk_param)
{
	u8 op_shift, num_parents;
	void __iomem *reg;
	const char *parent_names[2];
	struct clk *soc_clk;
	int i;

	if (of_property_read_u8(np, "operate-shift", &op_shift))
		return -EPERM;
	if (op_shift >= 32)
		return -EINVAL;
	num_parents = of_clk_get_parent_count(np);
	if (num_parents != 2)
		return -EINVAL;

	for (i = 0; i < num_parents; i++)
		parent_names[i] = of_clk_get_parent_name(np, i);

	reg = clk_reg->regbase + clk_param->reg - GLOBAL_JTAG_ID;

	soc_clk = clk_register_mux(NULL, np->name, parent_names, num_parents,
				   0, reg, op_shift, 1, 0, &clk_reg->regs_lock);
	if (!IS_ERR(soc_clk))
		of_clk_add_provider(np, of_clk_src_simple_get, soc_clk);

	return IS_ERR(soc_clk) ? PTR_ERR(soc_clk) : 0;
}

static int rtl9607f_clk_gate(struct device_node *np, struct rtl9607f_clk_reg *clk_reg,
		    struct rtl9607f_clk_param *clk_param)
{
	u8 op_shift, gate_flags;
	void __iomem *reg;
	struct clk *soc_clk;

	if (of_property_read_u8(np, "operate-shift", &op_shift))
		return -EPERM;
	if (op_shift >= 32)
		return -EINVAL;

	if (of_property_read_bool(np, "active-low"))
		gate_flags = CLK_GATE_SET_TO_DISABLE;
	else
		gate_flags = 0;

	reg = clk_reg->regbase + clk_param->reg - GLOBAL_JTAG_ID;

	soc_clk = clk_register_gate(NULL, np->name, NULL, 0, reg, op_shift,
				    gate_flags, &clk_reg->regs_lock);
	if (!IS_ERR(soc_clk))
		of_clk_add_provider(np, of_clk_src_simple_get, soc_clk);

	return IS_ERR(soc_clk) ? PTR_ERR(soc_clk) : 0;
}

static int rtl9607f_clk_fixed_factor(struct device_node *np,
			    struct rtl9607f_clk_reg *clk_reg,
			    struct rtl9607f_clk_param *clk_param)
{
	unsigned int mult, div;
	const char *parent_name;
	struct clk *soc_clk;

	if (of_property_read_u32(np, "fixed-mult", &mult))
		mult = 1;
	if (of_property_read_u32(np, "fixed-div", &div))
		div = 1;

	parent_name = of_clk_get_parent_name(np, 0);

	soc_clk = clk_register_fixed_factor(NULL, np->name, parent_name, 0,
					    mult, div);
	if (!IS_ERR(soc_clk))
		of_clk_add_provider(np, of_clk_src_simple_get, soc_clk);

	return IS_ERR(soc_clk) ? PTR_ERR(soc_clk) : 0;
}
/*********** PE ***************************************/
#define	PE_MIN_FREQ 400000000
#define	PE_DEF_FREQ 800000000
#define	PE_MAX_FREQ 1200000000
static int rtl9607f_pe_set_rate(struct rtl9607f_clk_reg *clk_reg, unsigned long ref_rate, unsigned long target)
{
	GLOBAL_FPLL0_t fpll0;
	GLOBAL_PEDIV_t pediv;
	GLOBAL_FPLL1_t fpll1;
	u32 divn = 0;
	void __iomem *cpllmux_reg;
	void __iomem *fpll0_reg;
	void __iomem *pediv_reg;
	void __iomem *fpll1_reg;
	unsigned long result_rate;

	pediv_reg = clk_reg->regbase + GLOBAL_PEDIV - GLOBAL_JTAG_ID;
	fpll1_reg = clk_reg->regbase + GLOBAL_FPLL1 - GLOBAL_JTAG_ID;
	fpll0_reg = clk_reg->regbase + GLOBAL_FPLL0 - GLOBAL_JTAG_ID;

	//1. GLOBAL_CPLLMUX = 0xff000000 (cf_i1guard=15, cf_i0guard=15)
	cpllmux_reg = clk_reg->regbase + GLOBAL_CPLLMUX - GLOBAL_JTAG_ID;
	if (readl(cpllmux_reg) != 0xff000000)
		writel(0xff000000, cpllmux_reg);


	/* 2. before setting fpll, must change clk source to cpll
	 * switch pe clk to c2f_clk from CPLL(1.2G/2=600M)
	 */
	pediv.wrd = readl(pediv_reg);
	pediv.bf.offload_sel = 1;
	writel(pediv.wrd, pediv_reg);

	//wait 2ms
	udelay(2000);

	/* 3.power down FPLL:
	 *				FPLL1_OEB=0x3 ->
	 *				FPLL0_SCPU_RSTB=0 ->
	 *				FPLL0_SCPU_POW=0 (power down FPLL)
	 */
	fpll1.wrd = readl(fpll1_reg);
	fpll1.bf.OEB = 0x3;
	writel(fpll1.wrd, fpll1_reg);

	fpll0.wrd = readl(fpll0_reg);
	fpll0.bf.SCPU_RSTB = 0;
	writel(fpll0.wrd, fpll0_reg);

	// FPLL0_SCPU_POW=0 (power down FPLL)
	fpll0.bf.SCPU_POW = 0;
	writel(fpll0.wrd, fpll0_reg);

	/* 4. FPLL0_FPLL_mode_override = 0x1
	 *	(1=override the fpll settings with the FPLL0 pll parameters)
	 */
	fpll0.wrd = readl(fpll0_reg);
	fpll0.bf.fpll_mode_override = 0x1;
	writel(fpll0.wrd, fpll0_reg);

	/* 5.Set FPLL freq register(NOTE)
	 * (FPLL0_DIV_PREDIV_BPS / FPLL0_SCPU_DIV_DIVN / FPLL0_DIV_PREDIV_SEL)
	 */
	if (ref_rate == REF_25MHZ) {
		divn = (2*target/REF_25MHZ) - 3;
		fpll0.bf.DIV_PREDIV_BPS = 1;
		fpll0.bf.DIV_PREDIV_SEL = 0;
		fpll0.bf.SCPU_DIV_DIVN = divn;
		result_rate = ((divn + 3)*REF_25MHZ)/2;
	} else {
		divn = (2*target/REF_40MHZ/2) - 3;
		fpll0.bf.DIV_PREDIV_BPS = 0;
		fpll0.bf.DIV_PREDIV_SEL = 0;
		fpll0.bf.SCPU_DIV_DIVN = divn;
		result_rate = ((divn + 3)*REF_40MHZ);
	}
	pr_debug("DIV_PREDIV_BPS 0x%x, DIV_PREDIV_SEL 0x%x, SCPU_DIV_DIVN 0x%x\n", fpll0.bf.DIV_PREDIV_BPS, fpll0.bf.DIV_PREDIV_SEL, fpll0.bf.SCPU_DIV_DIVN);
	writel(fpll0.wrd, fpll0_reg);

	//7. FPLL0_SCPU_POW=1 -> FPLL0_SCPU_RSTB=1 (power up FPLL)
	fpll0.wrd = readl(fpll0_reg);
	fpll0.bf.SCPU_POW = 1;
	writel(fpll0.wrd, fpll0_reg);

	fpll0.bf.SCPU_RSTB = 1;
	writel(fpll0.wrd, fpll0_reg);

	pr_debug("GLOBAL_FPLL0 = 0x%08x\n", readl(fpll0_reg));

	//8. wait 500us then FPLL1_OEB=0x0 (PLL stable and enable clk output)
	udelay(500);
	fpll1.bf.OEB = 0x0;
	writel(fpll1.wrd, fpll1_reg);
	//9. wait 10us
	udelay(10);

	//10. switch pe clk to f_clk_noscan from FPLL
	pediv.bf.offload_sel = 0;
	writel(pediv.wrd, pediv_reg);
	pr_debug("GLOBAL_FPLL0(0x%08x) = 0x%08x\n", GLOBAL_FPLL0, readl(fpll0_reg));
	pr_debug("GLOBAL_FPLL1(0x%08x) = 0x%08x\n", GLOBAL_FPLL1, readl(fpll1_reg));
	pr_debug("GLOBAL_PEDIV(0x%08x) = 0x%08x\n", GLOBAL_PEDIV, readl(pediv_reg));

	pr_info("PE: Change frequency to %ld MHz\n", result_rate / 1000000);

	return result_rate;
}
static void rtl9607f_pe_restore_def_rate(struct rtl9607f_clk_reg *clk_reg, unsigned long ref_rate)
{
	GLOBAL_FPLL0_t fpll0;
	void __iomem *fpll0_reg;

	fpll0_reg = clk_reg->regbase + GLOBAL_FPLL0 - GLOBAL_JTAG_ID;
	fpll0.wrd = readl(fpll0_reg);
	if (fpll0.bf.fpll_mode_override == 0)
		return;
	/* Change to Default FREQ */
	rtl9607f_pe_set_rate(clk_reg, ref_rate, PE_DEF_FREQ);
}
static int rtl9607f_clk_pe_daemon(struct device_node *np,
			 struct rtl9607f_clk_reg *clk_reg,
			 struct rtl9607f_clk_param *clk_param)
{
	const char *parent_names[5];
	struct clk *ref_clk;
	unsigned long ref_rate;
	int i;
	u32 speed = 0;

	pr_debug("%s : init speed %u\n", __func__, speed);

	for (i = 0; i < CPUD_CLKS_NUM; i++)
		parent_names[i] = of_clk_get_parent_name(np, i);


	ref_clk = __clk_lookup(parent_names[CPUD_REF]);
	ref_rate = clk_get_rate(ref_clk);

	if (of_property_read_u32(np, "init-speed", &speed))
		speed = 0;

	if (speed) {
		if ((speed < PE_MIN_FREQ) && (speed > PE_MAX_FREQ))
			speed = 0;
	}

	if (speed) {
		pr_debug("%s : init speed %u\n", __func__, speed);
		pr_debug("%s : ref_rate %lu\n", __func__, ref_rate);
		rtl9607f_pe_set_rate(clk_reg, ref_rate, speed);
	} else
		rtl9607f_pe_restore_def_rate(clk_reg, ref_rate);

	return 0;
}
/********** ARM Coretex *****************/
static unsigned long rtl9607f_cpu_recalc_rate(struct clk_hw *hw,
				     unsigned long parent_rate)
{
	struct rtl9607f_cpu_daemon *cpud = to_rtl9607f_cpu_daemon(hw);
	unsigned long ref_rate;
	GLOBAL_CPLLDIV_t cplldiv;
	GLOBAL_CPLL0_t cpll0;
	void __iomem *cpll0_reg;
	void __iomem *cplldiv_reg;
	u8 prediv, mult, div;
#ifdef CONFIG_ARM
	u64 recalc_rate_result = 0;
#endif
	cpll0_reg = cpud->reg + GLOBAL_CPLL0 - GLOBAL_JTAG_ID;
	cplldiv_reg = cpud->reg + GLOBAL_CPLLDIV - GLOBAL_JTAG_ID;

	cpll0.wrd = readl(cpll0_reg);
	cplldiv.wrd = readl(cplldiv_reg);
	mult = cpll0.bf.SCPU_DIV_DIVN + DIVN_BASE;

	if(cpll0.bf.DIV_PREDIV_BPS == 0){
		prediv = cpll0.bf.DIV_PREDIV_SEL + PREDIV_BASE;
	}else{
		prediv = 1;
	}
	div = cplldiv.bf.cortex_divsel >> 1;

	// if bypass
	if (div == 0)
		div = 1;

	ref_rate = clk_get_rate(cpud->clks[CPUD_REF]);

#ifdef CONFIG_ARM
		recalc_rate_result = (u64)ref_rate * (u64)mult;
		do_div(recalc_rate_result, prediv);
		do_div(recalc_rate_result, div);

		return (u32)recalc_rate_result;
#else
	return ref_rate * mult / prediv / div;
#endif
}

static long rtl9607f_cpu_round_rate(struct clk_hw *hw, unsigned long rate,
			   unsigned long *parent_rate)
{
	struct rtl9607f_cpu_daemon *cpud = to_rtl9607f_cpu_daemon(hw);
	unsigned long ref_rate, target;

	ref_rate = clk_get_rate(cpud->clks[CPUD_REF]);
	if (ref_rate == REF_40MHZ) {
		target = (rate / (ref_rate/2));
		target = target * (ref_rate/2);
	} else {
		target = (rate/ref_rate);
		target = target * ref_rate;
	}

	return target;
}

static int rtl9607f_cpu_set_rate(struct clk_hw *hw, unsigned long rate,
			unsigned long parent_rate)
{
	struct rtl9607f_cpu_daemon *cpud = to_rtl9607f_cpu_daemon(hw);
	GLOBAL_CPLLDIV_t cplldiv;
	GLOBAL_CPLL0_t cpll0;
	GLOBAL_CPLL1_t cpll1;
	void __iomem *reg;
	unsigned long ref_rate;
	u32 mult, div, target;
	unsigned long flags = 0;

	if (readonly)
		return 0;

	ref_rate = clk_get_rate(cpud->clks[CPUD_REF]);
	div=0;

	if (ref_rate == REF_40MHZ) {
		mult = rate / (ref_rate / PREDIV_VALUE);
		target = mult * (ref_rate/PREDIV_VALUE);
	} else {
		mult = rate / ref_rate;
		target = mult * ref_rate;
	}

	mutex_lock(&cpud->mlock);

	/* unbind daemon <--> cortex temporarily to avoid loop */
	clk_set_parent(hw->clk, NULL);

	// before setting cpll, must change clk source to fpll
	clk_set_parent(cpud->clks[CPUD_MUX_CPU], cpud->clks[CPUD_DIV_F2C]);

	//wait 2ms
	udelay(MIN_DELAY_PLL_FOR_F2c);

	// must clr RSTB before setting cpll
	spin_lock_irqsave(cpud->lock, flags);

	// CPLL1_OEB=3
	reg = cpud->reg + GLOBAL_CPLL1 - GLOBAL_JTAG_ID;
	cpll1.wrd = readl(reg);
	cpll1.bf.OEB=3;
	writel(cpll1.wrd, reg);

	//CPLL0_SCPU_RSTB=0
	reg = cpud->reg + GLOBAL_CPLL0 - GLOBAL_JTAG_ID;
	cpll0.wrd = readl(reg);
	cpll0.bf.SCPU_RSTB = 0;
	writel(cpll0.wrd, reg);
	// CPLL0_SCPU_POW=0 (power down CPLL)
	cpll0.bf.SCPU_POW = 0;
	writel(cpll0.wrd, reg);

	//CPLLDIV_cpll_div_override = 0x1 (1=override the digital divider setting with the cortex_divsel)
	reg = cpud->reg + GLOBAL_CPLLDIV - GLOBAL_JTAG_ID;
	cplldiv.wrd = readl(reg);
	cplldiv.bf.cortex_divsel = div << 1;
	cplldiv.bf.cpll_div_override = 1;
	writel(cplldiv.wrd, reg);
	// CPLLDIV_cpll_mode_override = 0x1 (1=override the cpll settings with the CPLL0 pll parameters)
	cplldiv.bf.cpll_mode_override = 1;
	writel(cplldiv.wrd, reg);

	spin_unlock_irqrestore(cpud->lock, flags);

	clk_set_rate(cpud->clks[CPUD_CPLL], target);

	// PWR UP CPLL and reset RSTB to enable cpll
	spin_lock_irqsave(cpud->lock, flags);
	reg = cpud->reg + GLOBAL_CPLL0 - GLOBAL_JTAG_ID;
	cpll0.wrd = readl(reg);
	cpll0.bf.SCPU_POW = 1;
	writel(cpll0.wrd, reg);

	cpll0.bf.SCPU_RSTB = 1;
	writel(cpll0.wrd, reg);
	spin_unlock_irqrestore(cpud->lock, flags);
	udelay(MIN_DELAY_PLL_FOR_LOCK);

	// wait 500us then CPLL1_OEB=0 (PLL stable and enable clk output)
	spin_lock_irqsave(cpud->lock, flags);
	reg = cpud->reg + GLOBAL_CPLL1 - GLOBAL_JTAG_ID;
	cpll1.wrd = readl(reg);
	cpll1.bf.OEB = 0;
	writel(cplldiv.wrd, reg);
	spin_unlock_irqrestore(cpud->lock, flags);
	udelay(MIN_DELAY_PLL_FOR_SWITCH);
	// swich clock source back to cpll
	clk_set_parent(cpud->clks[CPUD_MUX_CPU], cpud->clks[CPUD_CPLL]);

	// bind again
	clk_set_parent(hw->clk, cpud->clks[CPUD_DIV_CORTEX]);

	mutex_unlock(&cpud->mlock);

	pr_info("Change CPU frequency to %ld MHz\n", rate / 1000000);

	return rate;
}

static int rtl9607f_cpu_init(struct clk_hw *hw)
{
	struct rtl9607f_cpu_daemon *cpud = to_rtl9607f_cpu_daemon(hw);
	void __iomem *cpllmux_reg;

	cpllmux_reg = cpud->reg + GLOBAL_CPLLMUX - GLOBAL_JTAG_ID;

	/* magic values received from HW team		*/
	/* prevent cpu stall while changing frequency	*/
	writel(0xff000000, cpllmux_reg);
	return 0;
}

static const struct clk_ops rtl9607f_cpu_ops = {
	.recalc_rate	= rtl9607f_cpu_recalc_rate,
	.round_rate	= rtl9607f_cpu_round_rate,
	.set_rate	= rtl9607f_cpu_set_rate,
	.init		= rtl9607f_cpu_init,
};

__weak struct clk *cpu_daemon_clk;

static int rtl9607f_clk_cpu_daemon(struct device_node *np,
			  struct rtl9607f_clk_reg *clk_reg,
			  struct rtl9607f_clk_param *clk_param)
{
	const char *parent_names[5];
	struct clk *ref_clk;
	unsigned long ref_rate, min_rate, max_rate;
	int i;
	struct clk *cpu_clk;
	void __iomem *reg;
	int strap;
	GLOBAL_CPLLDIV_t cplldiv;
	u64 rate;
#if RTK_DTS_CPU_SPEED_INIT
	u32 speed = 0;
#endif
	struct clk_init_data init;
	struct rtl9607f_cpu_daemon *cpud;

	readonly = of_property_read_bool(np, "read-only");

	cpud = kzalloc(sizeof(*cpud), GFP_KERNEL);
	if (!cpud)
		return -ENOMEM;

	for (i = 0; i < CPUD_CLKS_NUM; i++) {
		parent_names[i] = of_clk_get_parent_name(np, i);
		cpud->clks[i] = __clk_lookup(parent_names[i]);
	}

	ref_clk = __clk_lookup(parent_names[CPUD_REF]);
	ref_rate = clk_get_rate(ref_clk);

	min_rate = CPU_MIN_SPEED;
	max_rate = CPU_MAX_SPEED;
#if RTK_DTS_CPU_SPEED_INIT
	if (of_property_read_u32(np, "init-speed", &speed))
		speed = 0;

	if (speed) {
		if ((speed < min_rate) && (speed > max_rate))
			speed = 0;
	}

	if (speed) {
		pr_warn("%s : init speed %u\n", __func__, speed);
	} else
#endif
	{
		/*
		 * 1. If Bootloader use CPLL(SW), not change it.
		 * 2. If Use CPLL(HW), then adjust CPLL REG to match both for ca_soc->cpll info
		 */
		reg = clk_reg->regbase + GLOBAL_CPLLDIV - GLOBAL_JTAG_ID;
		cplldiv.wrd = readl(reg);
		if(cplldiv.bf.cpll_mode_override == 1){
			printk("SOC: CPLL Clock has inited. Skip CPLL Init.");
		}else{
			reg = clk_reg->regbase + GLOBAL_STRAP - GLOBAL_JTAG_ID;
			strap = (readl(reg) >> GLOBAL_STRAP_SPEED_OFF) & GLOBAL_STRAP_SPEED_MASK;

			rate = (u64)rtl9607f_cpu_strap_speed_table[strap].rate;

			clk_set_rate(cpud->clks[CPUD_CPLL], rate);

		}
	}

	init.name = np->name;
	init.ops = &rtl9607f_cpu_ops;
	init.parent_names = &parent_names[CPUD_DIV_CORTEX];
	init.num_parents = 1;

	mutex_init(&cpud->mlock);
	cpud->reg = clk_reg->regbase;
	cpud->lock = &clk_reg->regs_lock;
	cpud->hw.init = &init;

	cpu_clk = clk_register(NULL, &cpud->hw);
#if RTK_DTS_CPU_SPEED_INIT
	if (speed && cpud)
		rtl9607f_cpu_set_rate(&cpud->hw, speed, 0);
#endif
	if (!IS_ERR(cpu_clk)) {
		of_clk_add_provider(np, of_clk_src_simple_get, cpu_clk);
		clk_set_min_rate(cpu_clk, min_rate);
		clk_set_max_rate(cpu_clk, max_rate);
        cpu_daemon_clk = cpu_clk;
	} else {
		kfree(cpud);
	}

	return IS_ERR(cpu_clk) ? PTR_ERR(cpu_clk) : 0;
}

static int (*rtl9607f_clk_setup_fns[])(struct device_node *np,
			      struct rtl9607f_clk_reg *clk_reg,
			      struct rtl9607f_clk_param *clk_param) = {
	[CA_CLK_REF]          = rtl9607f_clk_ref,
	[CA_CLK_PLL]          = rtl9607f_clk_pll,
	[CA_CLK_DIVIDER]      = rtl9607f_clk_divider,
	[CA_CLK_MUX]          = rtl9607f_clk_mux,
	[CA_CLK_GATE]         = rtl9607f_clk_gate,
	[CA_CLK_FIXED_FACTOR] = rtl9607f_clk_fixed_factor,
	[CA_CLK_PE_DAEMON]    = rtl9607f_clk_pe_daemon,
	[CA_CLK_CPU_DAEMON]   = rtl9607f_clk_cpu_daemon,
};

static struct rtl9607f_clk_reg *soc_clk_reg;
static void __init rtl9607f_soc_clks_setup(struct device_node *np)
{
	struct device_node *childnp;
	const struct of_device_id *clk_id;
	struct rtl9607f_clk_param *clk_param;

	soc_clk_reg = kzalloc(sizeof(*soc_clk_reg), GFP_KERNEL);
	if (!soc_clk_reg)
		return;

	soc_clk_reg->regbase = of_iomap(np, 0);
	if (!soc_clk_reg->regbase) {
		kfree(soc_clk_reg);
		return;
	}

	spin_lock_init(&soc_clk_reg->regs_lock);

	for_each_child_of_node(np, childnp) {
		clk_id = of_match_node(rtl9607f_soc_clks_ids, childnp);
		if (!clk_id)
			continue;

		clk_param = (struct rtl9607f_clk_param *)clk_id->data;
		if (rtl9607f_clk_setup_fns[clk_param->type](childnp, soc_clk_reg,
							  clk_param))
			pr_err("rtl9607f_clk_setup_fns(%s) fail!\n", np->name);
	}
	printk("RTK SOC: soc_clks_setup finished.\n");
}

module_param(readonly, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(readonly, "CPU daemon is readonly");

CLK_OF_DECLARE(rtl9607f_soc_clks, "realtek,rtl9607f-soc-clks",
	       rtl9607f_soc_clks_setup);
