/**
 * Copyrights (c) 2024 Rahul Arasikere <arasikere.rahul@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INCLUDE_ZEPHYR_DT_BINDINGS_INTERRUPT_CONTROLLER_TI_HERCULES_VIM_H_
#define INCLUDE_ZEPHYR_DT_BINDINGS_INTERRUPT_CONTROLLER_TI_HERCULES_VIM_H_

/** @enum IRQ_CLOCK_SOURCE
 *   @brief Alias names for IRQ clock sources
 *
 *   This enumeration is used to provide alias names for the clock sources:
 *     - IRQ
 *     - FIQ
 */
enum IRQ_CLOCK_SOURCE {
	SYS_IRQ = 0U, /**< Alias for IRQ interrupt */
	SYS_FIQ = 1U, /**< Alias for FIQ interrupt */
};

#endif /* INCLUDE_ZEPHYR_DT_BINDINGS_INTERRUPT_CONTROLLER_TI_HERCULES_VIM_H_ */
