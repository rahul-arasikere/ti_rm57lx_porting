/**
 * Copyrights (c) 2024 Rahul Arasikere <arasikere.rahul@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT ti_hercules_gcm

#include <soc.h>
#include <zephyr/drivers/clock_control.h>
#include <zephyr/dt-bindings/clock/ti-hercules-clock.h>
#include <zephyr/device.h>
#include <zephyr/sys/util.h>

#include <errno.h>

#define CLOCKS_NODE       DT_NODELABEL(clocks)
#define OSCIN_CLOCK_NODE  DT_CHILD(CLOCKS_NODE, oscin)
#define EXT_CLKIN1_NODE   DT_CHILD(CLOCKS_NODE, ext_clkin1)
#define EXT_CLKIN2_NODE   DT_CHILD(CLOCKS_NODE, ext_clkin2)
#define PLL1_NODE         DT_CHILD(CLOCKS_NODE, pll1)
#define PLL2_NODE         DT_CHILD(CLOCKS_NODE, pll2)
#define LF_LPO_CLOCK_NODE DT_CHILD(CLOCKS_NODE, lf_lpo)
#define HF_LPO_CLOCK_NODE DT_CHILD(CLOCKS_NODE, hf_lpo)
#define GCM_NODE          DT_NODELABEL(gcm)

/* Helper Macro Functions */
#define z_plldiv(x)    DT_PROP_BY_IDX(x, r)
#define z_refclkdiv(x) DT_PROP_BY_IDX(x, nr)
#define z_pllmul(x)    DT_PROP_BY_IDX(x, nf)
#define z_odpll(x)     DT_PROP_BY_IDX(x, od)

/* Check whether frequency modulation is enabled. */
#define FMENA DT_PROP_BY_IDX(CLOCKS_NODE, enable_freq_modulation)

#define z_pll1_mulmod()          DT_PROP_BY_IDX(PLL1_NODE, mulmod)
#define z_pll1_spreadingrate()   DT_PROP_BY_IDX(PLL1_NODE, ns)
#define z_pll1_spreadingamount() DT_PROP_BY_IDX(PLL1_NODE, nv)

#define FBSLIP  BIT(9)
#define RFSLIP  BIT(8)
#define OSCFAIL BIT(0)

#define PENA             BIT(8)
#define ESM_SRx_PLLxSLIP BIT(10)

/* Fixed clock frequencies */
#define CLOCK_OSCIN_FREQ      DT_PROP_BY_IDX(OSCIN_CLOCK_NODE, clock_frequency)
#define CLOCK_EXT_CLKIN1_FREQ DT_PROP_BY_IDX(EXT_CLKIN1_NODE, clock_frequency)
#define CLOCK_EXT_CLKIN2_FREQ DT_PROP_BY_IDX(EXT_CLKIN2_NODE, clock_frequency)

static uint32_t _errata_disable_plls(uint32_t plls)
{
	uint32_t timeout = 0x10, fail_code = 0;
	volatile struct hercules_syscon_1_regs *sys_regs_1 = (void *)DT_REG_ADDR(SYS1_NODE);
	volatile struct hercules_syscon_2_regs *sys_regs_2 = (void *)DT_REG_ADDR(SYS2_NODE);
	volatile struct hercules_esm_regs *esm_regs = (void *)DT_REG_ADDR(ESM_NODE);
	sys_regs_1->CSDISSET = plls;
	while (((sys_regs_1->CSVSTAT & plls) != 0) && timeout-- != 0) {
		/* Clear ESM and GLBSTAT PLL slip flags */
		sys_regs_1->GBLSTAT = FBSLIP | RFSLIP;

		if ((plls & BIT(CLOCK_SRC_PLL1)) == BIT(CLOCK_SRC_PLL1)) {
			esm_regs->SR1[0] = ESM_SRx_PLLxSLIP;
		}
		if ((plls & BIT(CLOCK_SRC_PLL2)) == BIT(CLOCK_SRC_PLL2)) {
			esm_regs->SR4[0] = ESM_SRx_PLLxSLIP;
		}
	}
	if (timeout == 0) {
		fail_code = 4;
	}
	return fail_code;
}

/** @fn static uint32_t _errata_SSWF021_45_both_plls(uint32_t count)
*   @brief This handles the errata for CLOCK_SRC_PLL1 and CLOCK_SRC_PLL2. This function is called in
device startup
*
*   @param[in] count : Number of retries until both PLLs are locked successfully
*                      Minimum value recommended is 5
*
*   @return 0 = Success (the PLL or both PLLs have successfully locked and then been disabled)
*           1 = CLOCK_SRC_PLL1 failed to successfully lock in "count" tries
*           2 = CLOCK_SRC_PLL2 failed to successfully lock in "count" tries
*           3 = Neither CLOCK_SRC_PLL1 nor CLOCK_SRC_PLL2 successfully locked in "count" tries
*           4 = The workaround function was not able to disable at least one of the PLLs. The most
likely reason is that a PLL is already being used as a clock source. This can be caused by the
workaround function being called from the wrong place in the code.
 */
static uint32_t _errata_SSWF021_45_both_plls(uint32_t count)
{
	uint32_t fail_code = 0;
	uint32_t retries;
	uint32_t clock_control_save;
	volatile struct hercules_syscon_1_regs *sys_regs_1 = (void *)DT_REG_ADDR(SYS1_NODE);
	volatile struct hercules_syscon_2_regs *sys_regs_2 = (void *)DT_REG_ADDR(SYS2_NODE);
	volatile struct hercules_esm_regs *esm_regs = (void *)DT_REG_ADDR(ESM_NODE);
	clock_control_save = sys_regs_1->CLKCNTL;
	/* First set VCLK2 = HCLK */
	sys_regs_1->CLKCNTL = clock_control_save & (PENA | (0xf << 16));
	/* Now set VCLK = HCLK and enable peripherals */
	sys_regs_1->CLKCNTL = PENA;
	for (retries = 0; retries < count; retries++) {
		fail_code = _errata_disable_plls(BIT(CLOCK_SRC_PLL1) | BIT(CLOCK_SRC_PLL2));
		if (fail_code != 0) {
			break;
		}
		/* Clear Global Status Register */
		sys_regs_1->GBLSTAT = FBSLIP | RFSLIP | OSCFAIL;
		/* Clear the ESM PLL slip flags */
		esm_regs->SR1[0] = ESM_SRx_PLLxSLIP;
		esm_regs->SR4[0] = ESM_SRx_PLLxSLIP;

		/* set both PLLs to OSCIN/1*27/(2*1) */
		sys_regs_1->PLLCTL1 = 0x20001A00;
		sys_regs_1->PLLCTL2 = 0x3FC0723D;
		sys_regs_2->PLLCTL3 = 0x20001A00;
		sys_regs_1->CSDISCLR = BIT(CLOCK_SRC_PLL1) | BIT(CLOCK_SRC_PLL2);

		/* Check for (CLOCK_SRC_PLL1 valid or CLOCK_SRC_PLL1 slip) and (CLOCK_SRC_PLL2 valid
		 * or CLOCK_SRC_PLL2 slip) */
		while ((((sys_regs_1->CSVSTAT & BIT(CLOCK_SRC_PLL1)) == 0) &&
			((esm_regs->SR1[0] & ESM_SRx_PLLxSLIP) == 0)) ||
		       (((sys_regs_1->CSVSTAT & BIT(CLOCK_SRC_PLL2)) == 0) &&
			((esm_regs->SR4[0] & ESM_SRx_PLLxSLIP) == 0))) {
			/* Wait */
		}
	}
	return fail_code;
}
struct ti_herc_periph_clk {
	uint8_t domain;
	uint8_t source;
	uint8_t clock_mode;
	uint8_t arg; /* divider argument (optional) */
};

struct ti_hercules_gcm_clock_data {
};

static int ti_hercules_gcm_clock_on(const struct device *dev, clock_control_subsys_t sys)
{
	struct ti_herc_periph_clk *periph_clk = (struct ti_herc_periph_clk *)sys;
	if (!IN_RANGE(periph_clk->source, CLOCK_SRC_OSCILLATOR, CLOCK_SRC_EXTCLKIN2) &&
	    !IN_RANGE(periph_clk->domain, CLOCK_DOM_GCLK1, CLOCK_DOM_VCLKA4)) {
		return -EINVAL;
	}
	volatile struct hercules_syscon_1_regs *sys_regs_1 = (void *)DT_REG_ADDR(SYS1_NODE);
	if (IN_RANGE(periph_clk->source, CLOCK_SRC_OSCILLATOR, CLOCK_SRC_EXTCLKIN2)) {
		/* enable clock source if not enabled */
		sys_regs_1->CSDIS |= BIT(periph_clk->source);
	}
	if (IN_RANGE(periph_clk->domain, CLOCK_DOM_GCLK1, CLOCK_DOM_VCLKA4)) {
		/* enable clock domain if not enabled */
		sys_regs_1->CDDIS &= ~BIT(periph_clk->domain);
	}
	return 0;
}

static int ti_hercules_gcm_clock_off(const struct device *dev, clock_control_subsys_t sys)
{

	volatile struct hercules_syscon_1_regs *sys_regs_1 = (void *)DT_REG_ADDR(SYS1_NODE);
	struct ti_herc_periph_clk *periph_clk = (struct ti_herc_periph_clk *)sys;
	if (!IN_RANGE(periph_clk->source, CLOCK_SRC_OSCILLATOR, CLOCK_SRC_EXTCLKIN2) &&
	    !IN_RANGE(periph_clk->domain, CLOCK_DOM_GCLK1, CLOCK_DOM_VCLKA4)) {
		return -EINVAL;
	}
	if (IN_RANGE(periph_clk->domain, CLOCK_DOM_GCLK1, CLOCK_DOM_VCLKA4)) {
		sys_regs_1->CDDIS |= BIT(periph_clk->domain);
	}
	if (IN_RANGE(periph_clk->source, CLOCK_SRC_OSCILLATOR, CLOCK_SRC_EXTCLKIN2)) {
		sys_regs_1->CSDIS |= BIT(periph_clk->source);
	}
	return 0;
}

static enum clock_control_status ti_hercules_gcm_clock_get_status(const struct device *dev,
								  clock_control_subsys_t sys)
{
	volatile struct hercules_syscon_1_regs *sys_regs_1 = (void *)DT_REG_ADDR(SYS1_NODE);
	struct ti_herc_periph_clk *periph_clk = (struct ti_herc_periph_clk *)sys;
	uint32_t csv_stat = 0;
	uint32_t csdis_val = 0;
	uint32_t cddis_val = 0;
	/* Only clock source status is checked */
	if (!IN_RANGE(periph_clk->source, CLOCK_SRC_OSCILLATOR, CLOCK_SRC_EXTCLKIN2) &&
	    !IN_RANGE(periph_clk->domain, CLOCK_DOM_GCLK1, CLOCK_DOM_VCLKA4)) {
		return CLOCK_CONTROL_STATUS_UNKNOWN;
	} else if (IN_RANGE(periph_clk->source, CLOCK_SRC_OSCILLATOR, CLOCK_SRC_EXTCLKIN2)) {
		sys_read32(sys_regs_1->CSVSTAT, &csv_stat);
		sys_read32(sys_regs_1->CSDIS, &csdis_val);
		csv_stat = FIELD_GET(BIT(periph_clk->source), csv_stat);
		csdis_val = FIELD_GET(BIT(periph_clk->source), csdis_val);
		if (csdis_val) {
			return CLOCK_CONTROL_STATUS_OFF;
		} else if (csv_stat) {
			return CLOCK_CONTROL_STATUS_ON;
		} else if (!csdis_val) {
			return CLOCK_CONTROL_STATUS_STARTING;
		} else {
			return CLOCK_CONTROL_STATUS_UNKNOWN;
		}
	} else {
		sys_read32(sys_regs_1->CDDIS, &cddis_val);
		cddis_val = FIELD_GET(BIT(periph_clk->domain), cddis_val);
		return (cddis_val) ? CLOCK_CONTROL_STATUS_OFF : CLOCK_CONTROL_STATUS_ON;
	}
}

static int ti_hercules_gcm_clock_get_rate(const struct device *dev, clock_control_subsys_t sys,
					  uint32_t *rate)
{
	struct ti_herc_periph_clk *periph_clk = (struct ti_herc_periph_clk *)sys;
	*rate = 0;
	switch (periph_clk->source) {
	case CLOCK_SRC_OSCILLATOR:
		*rate = CLOCK_OSCIN_FREQ;
		break;

	case CLOCK_SRC_EXTCLKIN:
		*rate = CLOCK_EXT_CLKIN1_FREQ;
		break;

	case CLOCK_SRC_EXTCLKIN2:
		*rate = CLOCK_EXT_CLKIN2_FREQ;
		break;

	case CLOCK_SRC_PLL1:
		*rate = ((CLOCK_OSCIN_FREQ / z_refclkdiv(PLL1_NODE)) * z_pllmul(PLL1_NODE)) /
			z_odpll(PLL1_NODE) / z_plldiv(PLL1_NODE);
		break;

	case CLOCK_SRC_PLL2:
		*rate = ((CLOCK_OSCIN_FREQ / z_refclkdiv(PLL2_NODE)) * z_pllmul(PLL2_NODE)) /
			z_odpll(PLL2_NODE) / z_plldiv(PLL2_NODE);
		break;

	default:
		return -EINVAL;
	}
	return 0;
}

static int ti_hercules_gcm_clock_configure(const struct device *dev, clock_control_subsys_t sys,
					   void *data)
{
	ARG_UNUSED(data);
	static struct ti_herc_periph_clk vclk_sys = {
		.source = CLOCK_SRC_VCLK,
	};
	struct ti_herc_periph_clk *periph_clk = (struct ti_herc_periph_clk *)sys;
	volatile struct hercules_syscon_1_regs *sys_regs_1 = (void *)DT_REG_ADDR(SYS1_NODE);
	volatile struct hercules_syscon_2_regs *sys_regs_2 = (void *)DT_REG_ADDR(SYS2_NODE);
	struct ti_herc_periph_clk *periph_clk = (struct ti_herc_periph_clk *)sys;
	uint32_t vclk_rate, src_clk_rate;
	if (!IN_RANGE(periph_clk->source, CLOCK_SRC_OSCILLATOR, CLOCK_SRC_EXTCLKIN2)) {
		/* Invalid source for input */
		return -EINVAL;
	}
	if (!IN_RANGE(periph_clk->clock_mode, CLOCK_ON_NORMAL, CLOCK_ON_WAKEUP_GCLK1_OFF)) {
		return -EINVAL;
	}
	switch (periph_clk->domain) {
	case CLOCK_DOM_GCLK1:
	case CLOCK_DOM_HCLK:
	case CLOCK_DOM_VCLK:
	case CLOCK_DOM_VCLK2:
	case CLOCK_DOM_VCLK3:
		/* All the above domains use the same source for clocks */
		sys_regs_1->GHVSRC |= (0xF & periph_clk->source) << (8 * periph_clk->clock_mode);
		break;
	case CLOCK_DOM_VCLKA1:
		sys_regs_1->VCLKASRC |= (0xF & periph_clk->source);
		break;

	case CLOCK_DOM_VCLKA2:
		sys_regs_1->VCLKASRC |= (0xF & periph_clk->source) << 8;
		break;
	case CLOCK_DOM_VCLKA4:
		sys_regs_2->VCLKACON1 |= (0xF & periph_clk->source) | periph_clk->arg;
		break;
	case CLOCK_DOM_RTICLK1:
		if (!IN_RANGE(periph_clk->arg, RTICLK_DIV_1, RTICLK_DIV_8)) {
			return -EINVAL;
		}
		if (periph_clk->source != CLOCK_SRC_VCLK) {
			if (!clock_control_get_rate(DEVICE_DT_GET(GCM_NODE), sys, &src_clk_rate)) {
				return -EINVAL;
			}
			if (!clock_control_get_rate(DEVICE_DT_GET(GCM_NODE),
						    (clock_control_subsys_t)&vclk_sys,
						    &vclk_rate)) {
				return -EINVAL;
			}
			if (src_clk_rate < vclk_rate / (1 << periph_clk->arg)) {
				return -EINVAL;
			}
		}
		sys_regs_1->RCLKSRC = ((1 << periph_clk->arg) << 8) /* Set divider */
				      | (0xF & periph_clk->source); /* set source */

		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int ti_hercules_gcm_clock_init(const struct device *dev)
{
	_errata_SSWF021_45_both_plls(5);
	volatile struct hercules_syscon_1_regs *sys_regs_1 = (void *)DT_REG_ADDR(SYS1_NODE);
	volatile struct hercules_syscon_2_regs *sys_regs_2 = (void *)DT_REG_ADDR(SYS2_NODE);

	/* Configure PLL control registers and enable PLLs.
	 * The PLL takes (127 + 1024 * NR) oscillator cycles to acquire lock.
	 * This initialization sequence performs all the tasks that are not
	 * required to be done at full application speed while the PLL locks.
	 */
	sys_regs_1->CSDISSET = BIT(CLOCK_SRC_PLL1) | BIT(CLOCK_SRC_PLL2);
	while (sys_regs_1->CSDIS & (BIT(CLOCK_SRC_PLL1) | BIT(CLOCK_SRC_PLL2)) !=
					   (BIT(CLOCK_SRC_PLL1) | BIT(CLOCK_SRC_PLL2))) {
		/*nop*/;
	}

	/* Clear Global Status Flags */
	sys_regs_1->GBLSTAT = FBSLIP | RFSLIP | OSCFAIL;

#if IS_ENABLED(PLL1_NODE)
	uint32_t pllctl1_conf = 0;
	pllctl1_conf |= DT_PROP_BY_IDX(PLL1_NODE, reset_on_pll_slip) << 31;
#if DT_PROP_BY_IDX(PLL1_NODE, bypass_on_pll_slip)
	/* Enable Bypass on Slip*/
	pllctl1_conf |= 0b11 << 29;
#else
	/* Disable Bypass on Slip */
	pllctl1_conf |= 0x2 << 29;
#endif /* Bypass-On-PLL-Slip */
	BUILD_ASSERT(IN_RANGE(z_plldiv(PLL1_NODE), 1, 32), "R out of range! (1 -32)");
	pllctl1_conf |= (z_plldiv(PLL1_NODE) - 1) << 24;
	pllctl1_conf |= DT_PROP_BY_IDX(PLL1_NODE, reset_on_oscillator_fail) << 23;
	BUILD_ASSERT(IN_RANGE(z_refclkdiv(PLL1_NODE), 1, 64), "NR out of range! (1 - 64)");
	pllctl1_conf |= (z_refclkdiv(PLL1_NODE) - 1) << 16;
	BUILD_ASSERT(IN_RANGE(z_pllmul(PLL1_NODE), 1, 256), "NF out of range! (1 - 256)");
	pllctl1_conf |= (z_pllmul(PLL1_NODE) - 1) << 8;

	sys_regs_1->PLLCTL1 = pllctl1_conf;

	uint32_t pllctl2_conf = 0;
#if FMENA
	pllctl2_conf |= BIT(31);
	BUILD_ASSERT(IN_RANGE(z_pll1_spreadingrate(), 1, 512), "NS out of range! (1-512)");
	pllctl2_conf |= (z_pll1_spreadingrate() - 1) << 22;
	BUILD_ASSERT(IN_RANGE(z_pll1_mulmod(), 8, 511), "MULMOD out of range! (8 - 511)");
	pllctl2_conf |= z_pll1_mulmod() << 12;
	BUILD_ASSERT(IN_RANGE(z_pll1_spreadingamount(), 1, 512), "NV out of range! (1 - 512)");
	pllctl2_conf |= (z_pll1_spreadingamount() - 1);
#endif /* FMENA */
	BUILD_ASSERT(IN_RANGE(z_odpll(PLL1_NODE), 1, 8), "OD out of range! (1 - 8)");
	pllctl2_conf |= (z_odpll(PLL1_NODE) - 1) << 9;
	sys_regs_1->PLLCTL2 = pllctl2_conf;
#endif /* PLL1_NODE */

#if IS_ENABLED(PLL2_NODE)
	uint32_t pllctl3_conf = 0;
	BUILD_ASSERT(IN_RANGE(z_odpll(PLL2_NODE), 1, 8), "OD out of range! (1 - 8)");
	pllctl3_conf |= (z_odpll(PLL2_NODE) - 1) << 29;
	BUILD_ASSERT(IN_RANGE(z_plldiv(PLL2_NODE), 1, 32), "R out of range! (1 -32)");
	pllctl3_conf |= (z_plldiv(PLL2_NODE) - 1) << 24;
	BUILD_ASSERT(IN_RANGE(z_refclkdiv(PLL2_NODE), 1, 64), "NR out of range! (1 - 64)");
	pllctl3_conf |= (z_refclkdiv(PLL2_NODE) - 1) << 16;
	BUILD_ASSERT(IN_RANGE(z_pllmul(PLL2_NODE), 1, 256), "NF out of range! (1 - 256)");
	pllctl3_conf |= (z_pllmul(PLL2_NODE) - 1) << 8;
	sys_regs_2->PLLCTL3 = pllctl3_conf;
#endif /* PLL2_NODE */

#if IS_ENABLED(OSCIN_CLOCK_NODE)
	sys_regs_1->CSDIS |= BIT(CLOCK_SRC_OSCILLATOR);
#endif
#if IS_ENABLED(PLL1_NODE)
	sys_regs_1->CSDIS |= BIT(CLOCK_SRC_PLL1);
#endif
#if IS_ENABLED(EXT_CLKIN1_NODE)
	sys_regs_1->CSDIS |= BIT(CLOCK_SRC_EXTCLKIN);
#endif
#if IS_ENABLED(LF_LPO_CLOCK_NODE)
	sys_regs_1->CSDIS |= BIT(CLOCK_SRC_LF_LPO);
#endif
#if IS_ENABLED(HF_LPO_CLOCK_NODE)
	sys_regs_1->CSDIS |= BIT(CLOCK_SRC_HF_LPO);
#endif
#if IS_ENABLED(PLL2_NODE)
	sys_regs_1->CSDIS |= BIT(CLOCK_SRC_PLL2);
#endif
#if IS_ENABLED(EXT_CLKIN2_NODE)
	sys_regs_1->CSDIS |= BIT(CLOCK_SRC_EXTCLKIN2);
#endif
	return 0;
}

static DEVICE_API(clock_control,
		  ti_hercules_gcm_clock_api) = {.on = ti_hercules_gcm_clock_on,
						.off = ti_hercules_gcm_clock_off,
						.get_status = ti_hercules_gcm_clock_get_status,
						.get_rate = ti_hercules_gcm_clock_get_rate,
						.configure = ti_hercules_gcm_clock_configure};

DEVICE_DT_DEFINE(DT_NODELABEL(gcm), ti_hercules_gcm_clock_init, NULL, NULL, NULL, PRE_KERNEL_1,
		 CONFIG_CLOCK_CONTROL_INIT_PRIORITY, &ti_hercules_gcm_clock_api);
