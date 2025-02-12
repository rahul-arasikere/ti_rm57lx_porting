/**
 * Copyrights (c) 2024 Rahul Arasikere <arasikere.rahul@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/drivers/timer/system_timer.h>
#include <zephyr/sys_clock.h>
#include <soc.h>

#define DT_DRV_COMPAT ti_hercules_rti_timer

#if DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT) > 1
#error Only one RTI driver instance should be enabled
#endif

static void rti_irq_handler(const struct device *dev) {

};

void sys_clock_announce(int32_t ticks)
{
}

uint32_t sys_clock_elapsed(void)
{
	return 0;
}

void sys_clock_disable(void)
{
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

static void sys_clock_driver_init(void)
{
}
