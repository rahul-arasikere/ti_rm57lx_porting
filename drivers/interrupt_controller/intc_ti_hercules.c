/**
 * Copyrights (c) 2024 Rahul Arasikere <arasikere.rahul@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT ti_hercules_vim

#include <soc.h>

#include <zephyr/arch/arm/irq.h>
#include <zephyr/arch/cpu.h>
#include <zephyr/devicetree.h>
#include <zephyr/fatal.h>
#include <zephyr/kernel.h>
#include <zephyr/linker/linker-defs.h>
#include <zephyr/sw_isr_table.h>
#include <zephyr/sys/util.h>
#include <zephyr/types.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(vim, CONFIG_INTC_LOG_LEVEL);

#define VIM_NODE DT_NODELABEL(vim)

#define EDAC_MODE_ENABLE  (uint32_t)(0xA << 16)
#define EDAC_MODE_DISABLE (uint32_t)(0x5 << 16)

#define VIM_ECC_ENABLE  (uint32_t)0xAU
#define VIM_ECC_DISABLE (uint32_t)0x5U

#define VIM_RAM_ADDR     (void *)0xFFF82000U
#define VIM_RAM_ECC_ADDR (void *)0xFFF82400U

struct hercules_vim_regs {
	uint32_t rsvd1[59U];     /* 0x0000 - 0x00E8 Reserved */
	uint32_t ECCSTAT;        /* 0x00EC        */
	uint32_t ECCCTL;         /* 0x00F0        */
	uint32_t UERRADDR;       /* 0x00F4        */
	uint32_t FBVECADDR;      /* 0x00F8        */
	uint32_t SBERRADDR;      /* 0x00FC        */
	uint32_t IRQINDEX;       /* 0x0100        */
	uint32_t FIQINDEX;       /* 0x0104        */
	uint32_t rsvd2;          /* 0x0108        */
	uint32_t rsvd3;          /* 0x010C        */
	uint32_t FIRQPR[4];      /* 0x0110        */
	uint32_t INTREQ[4];      /* 0x0120        */
	uint32_t REQMASKSET[4];  /* 0x0130        */
	uint32_t REQMASKCLR[4];  /* 0x0140        */
	uint32_t WAKEMASKSET[4]; /* 0x0150        */
	uint32_t WAKEMASKCLR[4]; /* 0x0160        */
	uint32_t IRQVECREG;      /* 0x0170        */
	uint32_t FIQVECREG;      /* 0x0174        */
	uint32_t CAPEVT;         /* 0x0178        */
	uint32_t rsvd4;          /* 0x017C        */
	uint32_t CHANCTRL[32U];  /* 0x0180-0x02FC */
};

void vim_ecc_error_handle(void)
{
	volatile struct hercules_vim_regs *reg = (void *)DT_REG_ADDR(VIM_NODE);
	volatile struct hercules_esm_regs *esm_reg = (void *)DT_REG_ADDR(ESM_NODE);
	uint32_t vec = 0;
	uint32_t err_addr = reg->UERRADDR;
	uint32_t err_channel = ((err_addr & 0x3ff) >> 2);
	int idx = vec / 32;
	int rem = vec % 32;
	LOG_WRN("ECC error addr: %p, chan: %u, resetting", (void *)err_addr, err_channel);
	/* Reset the offending interrupt address. */
	// *(VIM_RAM_ADDR + err_channel) = (void *)_vector_start[err_channel];
	/* Clear the ECC Error */
	reg->ECCSTAT = 1U;
	/* Disable and enable the highest priority pending channel */
	if (reg->FIQINDEX != 0U) {
		vec = reg->FIQINDEX - 1U;
	} else {
		vec = reg->IRQINDEX - 1U;
	}
	if (vec == 0U) {
		reg->INTREQ[0] = 1U;
		vec = esm_reg->IOFFHR - 1U;
		/* This doesn't do anything yet as the ESM isn't enabled yet. */
		if (vec < 32U) {
			esm_reg->SR1[0U] = (uint32_t)1U << rem;
			/* Notify ESM Group 1 */
		} else if (vec < 64U) {
			esm_reg->SR1[1U] = (uint32_t)1U << rem;
			/* Notify ESM Group 2 */
		} else if (vec < 96U) {
			esm_reg->SR4[0U] = (uint32_t)1U << rem;
			/* Notify ESM Group 1 */
		} else if ((vec >= 128U) && (vec < 160U)) {
			esm_reg->SR7[0U] = (uint32_t)1U << rem;
			/* Notify ESM Group 2 */
		} else {
			esm_reg->SR7[0U] = 0xFFFFFFFFU;
			esm_reg->SR4[1U] = 0xFFFFFFFFU;
			esm_reg->SR4[0U] = 0xFFFFFFFFU;
			esm_reg->SR1[1U] = 0xFFFFFFFFU;
			esm_reg->SR1[0U] = 0xFFFFFFFFU;
		}
	} else {
		reg->REQMASKCLR[idx] = (uint32_t)1U << rem;
		reg->REQMASKSET[idx] = (uint32_t)1U << rem;
	}
}

void z_soc_irq_init(void)
{
	volatile struct hercules_vim_regs *reg = (void *)DT_REG_ADDR(VIM_NODE);
	size_t irq_count = (size_t)_vector_end - (size_t)_vector_start;
	/* Enable ECC for VIM RAM */
	/* Errata VIM#28 Workaround: Disable Single Bit error correction */
	reg->ECCCTL = VIM_ECC_ENABLE | EDAC_MODE_DISABLE;
	/* Copy IRQ handlers into VIM RAM */
	(void)memcpy(VIM_RAM_ADDR, _vector_start, irq_count);

	/* ECC related ERROR handler */
	reg->FBVECADDR = (uint32_t)&vim_ecc_error_handle;
	reg->FIRQPR[0] = 0x3; /* Channel 0 & 1 are FIQ only */
	/* Set all remaining channels to SYS_IRQ for now. */
	reg->FIRQPR[1] = 0;
	reg->FIRQPR[2] = 0;
	reg->FIRQPR[3] = 0;
	reg->REQMASKSET[0] = 0x3; /* Enable Channel 0 & 1 IRQs. */
	/* Disable all other IRQs for now. */
	reg->REQMASKSET[1] = 0;
	reg->REQMASKSET[2] = 0;
	reg->REQMASKSET[3] = 0;

	/* Set Capture Event Sources. */
	reg->CAPEVT = ((uint32_t)((uint32_t)0U << 0U) | (uint32_t)((uint32_t)0U << 16U));
}

void z_soc_irq_enable(unsigned int irq)
{
	volatile struct hercules_vim_regs *reg = (void *)DT_REG_ADDR(VIM_NODE);
	int idx = irq / 32;
	int rem = irq % 32;
	reg->REQMASKSET[idx] |= 1 << rem;
}

void z_soc_irq_disable(unsigned int irq)
{
	volatile struct hercules_vim_regs *reg = (void *)DT_REG_ADDR(VIM_NODE);
	int idx = irq / 32;
	int rem = irq % 32;
	reg->REQMASKCLR[idx] |= 1 << rem;
}

int z_soc_irq_is_enabled(unsigned int irq)
{
	volatile struct hercules_vim_regs *reg = (void *)DT_REG_ADDR(VIM_NODE);
	int idx = irq / 32;
	int rem = irq % 32;
	return ((reg->REQMASKSET[idx] >> rem) & 0x1);
}

void z_soc_irq_priority_set(unsigned int irq, unsigned int prio, unsigned int flags)
{
	volatile struct hercules_vim_regs *reg = (void *)DT_REG_ADDR(VIM_NODE);
	uint32_t idx = irq / 32;
	uint32_t rem = irq % 32;
	uint32_t offset_idx = prio / 4; /* Get channel priority map offset for irq channel */
	uint32_t offset_rem = prio % 4;
	offset_rem = 3 - offset_rem; /* Swap bytes */
	offset_rem *= 8;             /* Start bit to set */
	reg->CHANCTRL[offset_idx] &=
		~(uint32_t)((uint32_t)0xFFU << offset_rem); /* Clear previous mapping. */
	reg->CHANCTRL[offset_idx] |= (uint32_t)((uint32_t)0x7FU & irq) << offset_rem;
	/* flags is set to either SYS_IRQ or SYS_FIQ */
	WRITE_BIT(reg->FIRQPR[idx], rem, flags);
}

unsigned int z_soc_irq_get_active(void)
{
	volatile struct hercules_vim_regs *reg = (void *)DT_REG_ADDR(VIM_NODE);
	if (reg->FIQINDEX != 0U) {
		return reg->FIQINDEX - 1U;
	} else {
		return reg->IRQINDEX - 1U;
	}
}

void z_soc_irq_eoi(unsigned int irq)
{
	volatile struct hercules_vim_regs *reg = (void *)DT_REG_ADDR(VIM_NODE);
	int idx = irq / 32;
	int rem = irq % 32;
	reg->INTREQ[idx] |= 1 << rem; /* Clear pending IRQ */
}
