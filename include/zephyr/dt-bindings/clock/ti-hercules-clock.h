/**
 * Copyrights (c) 2024 Rahul Arasikere <arasikere.rahul@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INCLUDE_ZEPHYR_DT_BINDINGS_CLOCK_TI_HERCULES_CLOCK_H_
#define INCLUDE_ZEPHYR_DT_BINDINGS_CLOCK_TI_HERCULES_CLOCK_H_

#define CLOCK_SRC_OSCILLATOR 0
#define CLOCK_SRC_PLL1       1
#define CLOCK_SRC_RESERVED   2
#define CLOCK_SRC_EXTCLKIN   3
#define CLOCK_SRC_LF_LPO     4
#define CLOCK_SRC_HF_LPO     5
#define CLOCK_SRC_PLL2       6
#define CLOCK_SRC_EXTCLKIN2  7
#define CLOCK_SRC_DEFAULT    8

#define CLOCK_DOM_GCLK1   0
#define CLOCK_DOM_HCLK    1
#define CLOCK_DOM_VCLK    2
#define CLOCK_DOM_VCLK2   3
#define CLOCK_DOM_VCLKA1  4
#define CLOCK_DOM_VCLKA2  5
#define CLOCK_DOM_RTICLK1 6
#define CLOCK_DOM_VCLK3   8
#define CLOCK_DOM_VCLKA4  11

#endif /* INCLUDE_ZEPHYR_DT_BINDINGS_CLOCK_TI_HERCULES_CLOCK_H_ */
