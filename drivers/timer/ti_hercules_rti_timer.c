/**
 * Copyrights (c) 2024 Rahul Arasikere <arasikere.rahul@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * The systicks for this potentially tickless system is provided from the CNT0.
 * If tickless is enabled, compare 0 is used to provide the time keeping for the
 * kernel.
 */

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/drivers/timer/system_timer.h>
#include <zephyr/sys_clock.h>
#include <soc.h>

#include <zephyr/dt-bindings/clock/ti-hercules-clock.h>
#include <zephyr/dt-bindings/timer/ti-hercules-rti-timer.h>

#include <stdint.h>

#define DT_DRV_COMPAT ti_hercules_rti_timer

#define RTI_NODE DT_NODELABEL(rti)

#define TICKLESS IS_ENABLED(CONFIG_TICKLESS_KERNEL)

#if TICKLESS && DT_NODE_HAS_STATUS_OKAY(DT_NODELABEL(counter0))
#error counter0 is used for tickless capability, either disable TICKLESS_KERNEL or counter0.
#endif

#if DT_PROP(RTI_NODE, increment_on_failure) && DT_PROP(RTI_NODE, timebase_external)
#error timebase-external and increment-on-failure properties are mutually exclusive.
#endif

#define INC_ON_FAIL_EN BIT(1)
#define TBEXT_EN       BIT(0)

#define CNT1EN BIT(1)
#define CNT0EN BIT(0)

enum ntu_time_source {
	FLEXRAY_MACRO_TICK = 0,
	FLEXRAY_START_OF_CYCLE = 1,
	PLL2 = 2,
	EXTERNAL1 = 3,
};

static void rti_irq_handler(const struct device *dev) {

};

uint32_t sys_clock_elapsed(void)
{
	return 0;
}

void sys_clock_disable(void)
{
	volatile struct hercules_rti_regs *regs =
		(volatile struct hercules_rti_regs *)DT_REG_ADDR(RTI_NODE);
	regs->GCTRL &= ~(CNT0EN);
}

uint32_t sys_clock_cycle_get_32(void)
{
	return 0;
}

uint64_t sys_clock_cycle_get_64(void)
{
	return 0;
}

void sys_clock_idle_exit(void)
{
}

void sys_clock_set_timeout(int32_t ticks, bool idle)
{
}

static int sys_clock_driver_init(void)
{
	volatile struct hercules_rti_regs *regs =
		(volatile struct hercules_rti_regs *)DT_REG_ADDR(RTI_NODE);
	regs->GCTRL = 0;
	/* Setup NTU clock source if defined in dts or default to FLEXRAY_MACRO_TICK as in TRM */
	switch ((enum ntu_time_source)DT_ENUM_IDX_OR(RTI_NODE, ntu, FLEXRAY_MACRO_TICK)) {
	case EXTERNAL1:
		regs->GCTRL |= (0xFU << 16U);
		break;

	case FLEXRAY_START_OF_CYCLE:
		regs->GCTRL |= (0x5U << 16U);
		break;

	case PLL2:
		regs->GCTRL |= (0xAU << 16U);
		break;

	case FLEXRAY_MACRO_TICK:
	default:
		/* GCTRL is 0'd already*/
	}
	return 0;
#if DT_PROP(RTI_NODE, continue_on_suspend)
	regs->GCTRL |= BIT(15);
#endif
	regs->TBCTRL = 0;
#if DT_PROP(RTI_NODE, increment_on_failure)
	regs->TBCTRL |= INC_ON_FAIL_EN;
#endif
#if DT_PROP(RTI_NODE, timebase_external)
	regs->TBCTRL |= TBEXT_EN;
#endif

#if TICKLESS
	/* Disable external capture event for counter 0 */
	regs->CAPCTRL &= ~(BIT(0));
	/* Set counter0 as compare source for compare 0*/
	regs->COMPCTRL &= ~(BIT(0));

	/* Reset up counter and free running counter 0 */
	regs->CNT[0].UCx = 0;
	regs->CNT[0].FRCx = 0;
#endif

	return 0;
}

SYS_INIT(sys_clock_driver_init, PRE_KERNEL_2, CONFIG_SYSTEM_CLOCK_INIT_PRIORITY);
