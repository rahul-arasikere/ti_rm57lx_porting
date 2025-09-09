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

#if IS_ENABLED(CONFIG_TICKLESS_KERNEL) && DT_INST_HAS_STATUS_OKAY(DT_NODELABEL(counter0))
#error counter0 is used for tickless capability, either disable TICKLESS_KERNEL or counter0.
#endif

#define RTI_NODE DT_NODELABEL(rti)

#define CNT1EN BIT(1)
#define CNT0EN BIT(0)

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
	return 0;
}

SYS_INIT(sys_clock_driver_init, PRE_KERNEL_2, CONFIG_SYSTEM_CLOCK_INIT_PRIORITY);
