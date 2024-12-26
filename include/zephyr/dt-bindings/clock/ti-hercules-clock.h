/**
 * Copyrights (c) 2024 Rahul Arasikere <arasikere.rahul@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INCLUDE_ZEPHYR_DT_BINDINGS_CLOCK_TI_HERCULES_CLOCK_H_
#define INCLUDE_ZEPHYR_DT_BINDINGS_CLOCK_TI_HERCULES_CLOCK_H_

#define CLOCK_SRC_OSCILLATOR    0x0U
#define CLOCK_SRC_PLL1          0x1U
#define CLOCK_SRC_RESERVED      0x2U
#define CLOCK_SRC_EXTCLKIN      0x3U
#define CLOCK_SRC_LF_LPO        0x4U
#define CLOCK_SRC_HF_LPO        0x5U
#define CLOCK_SRC_PLL2          0x6U
#define CLOCK_SRC_EXTCLKIN2     0x7U
#define CLOCK_SRC_VCLK          0x9U
#define CLOCK_SRC_PLL2_ODCLK_8  0xEU
#define CLOCK_SRC_PLL2_ODCLK_16 0xFU
#define CLOCK_SRC_NONE          0x10U

#define CLOCK_DOM_GCLK1   0x0U
#define CLOCK_DOM_HCLK    0x1U
#define CLOCK_DOM_VCLK    0x2U
#define CLOCK_DOM_VCLK2   0x3U
#define CLOCK_DOM_VCLKA1  0x4U
#define CLOCK_DOM_VCLKA2  0x5U
#define CLOCK_DOM_RTICLK1 0x6U
#define CLOCK_DOM_VCLK3   0x8U
#define CLOCK_DOM_VCLKA4  0xBU
#define CLOCK_DOM_NONE    0xCU

#define CLOCK_ON_WAKEUP           0x2U
#define CLOCK_ON_WAKEUP_GCLK1_OFF 0x1U
#define CLOCK_ON_NORMAL           0x0U

#define RTICLK_DIV_1 0x0U
#define RTICLK_DIV_2 0x1U
#define RTICLK_DIV_4 0x2U
#define RTICLK_DIV_8 0x3U

#define VCLK4AR_DIV_1 (0x0U << 24)
#define VCLK4AR_DIV_2 (0x1U << 24)
#define VCLK4AR_DIV_3 (0x2U << 24)
#define VCLK4AR_DIV_4 (0x3U << 24)
#define VCLK4AR_DIV_5 (0x4U << 24)
#define VCLK4AR_DIV_6 (0x5U << 24)
#define VCLK4AR_DIV_7 (0x6U << 24)
#define VCLK4AR_DIV_8 (0x7U << 24)

#define VCLKA4_DIV_CDDIS (0x1U << 20) /*Disable the VCLKA4 divider output.*/

#endif /* INCLUDE_ZEPHYR_DT_BINDINGS_CLOCK_TI_HERCULES_CLOCK_H_ */
