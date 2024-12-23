/**
 * Copyrights (c) 2024 Rahul Arasikere <arasikere.rahul@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT ti_hercules_vim

#include <soc.h>

#include <zephyr/arch/arm/irq.h>
#include <zephyr/arch/cpu.h>

#include <zephyr/devicetree.h>

#include <zephyr/kernel.h>

#include <zephyr/linker/linker-defs.h>

#include <zephyr/sys/util.h>

#include <zephyr/types.h>

#define VIM_NODE DT_NODELABEL(vim)

#define EDAC_MODE_ENABLE  (uint32_t)(0xA << 16)
#define EDAC_MODE_DISABLE (uint32_t)(0x5 << 16)

#define VIM_ECC_ENABLE  (uint32_t)0xAU
#define VIM_ECC_DISABLE (uint32_t)0x5U

#define VIM_RAM_ADDR (void *)0xFFF82000

struct ti_hercules_vim_registers {
	uint32_t rsvd1[59U];    /* 0x0000 - 0x00E8 Reserved */
	uint32_t ECCSTAT;       /* 0x00EC        */
	uint32_t ECCCTL;        /* 0x00F0        */
	uint32_t UERRADDR;      /* 0x00F4        */
	uint32_t FBVECADDR;     /* 0x00F8        */
	uint32_t SBERRADDR;     /* 0x00FC        */
	uint32_t IRQINDEX;      /* 0x0100        */
	uint32_t FIQINDEX;      /* 0x0104        */
	uint32_t rsvd2;         /* 0x0108        */
	uint32_t rsvd3;         /* 0x010C        */
	uint32_t FIRQPR0;       /* 0x0110        */
	uint32_t FIRQPR1;       /* 0x0114        */
	uint32_t FIRQPR2;       /* 0x0118        */
	uint32_t FIRQPR3;       /* 0x011C        */
	uint32_t INTREQ0;       /* 0x0120        */
	uint32_t INTREQ1;       /* 0x0124        */
	uint32_t INTREQ2;       /* 0x0128        */
	uint32_t INTREQ3;       /* 0x012C        */
	uint32_t REQMASKSET0;   /* 0x0130        */
	uint32_t REQMASKSET1;   /* 0x0134        */
	uint32_t REQMASKSET2;   /* 0x0138        */
	uint32_t REQMASKSET3;   /* 0x013C        */
	uint32_t REQMASKCLR0;   /* 0x0140        */
	uint32_t REQMASKCLR1;   /* 0x0144        */
	uint32_t REQMASKCLR2;   /* 0x0148        */
	uint32_t REQMASKCLR3;   /* 0x014C        */
	uint32_t WAKEMASKSET0;  /* 0x0150        */
	uint32_t WAKEMASKSET1;  /* 0x0154        */
	uint32_t WAKEMASKSET2;  /* 0x0158        */
	uint32_t WAKEMASKSET3;  /* 0x015C        */
	uint32_t WAKEMASKCLR0;  /* 0x0160        */
	uint32_t WAKEMASKCLR1;  /* 0x0164        */
	uint32_t WAKEMASKCLR2;  /* 0x0168        */
	uint32_t WAKEMASKCLR3;  /* 0x016C        */
	uint32_t IRQVECREG;     /* 0x0170        */
	uint32_t FIQVECREG;     /* 0x0174        */
	uint32_t CAPEVT;        /* 0x0178        */
	uint32_t rsvd4;         /* 0x017C        */
	uint32_t CHANCTRL[32U]; /* 0x0180-0x02FC */
};

void z_soc_irq_init(void)
{
	struct ti_hercules_vim_registers *reg = (void *)DT_REG_ADDR(VIM_NODE);
	size_t irq_count = (size_t)_vector_end - (size_t)_vector_start;
	/* Enable ECC for VIM RAM */
	/* Errata VIM#28 Workaround: Disable Single Bit error correction */
	reg->ECCCTL = VIM_ECC_ENABLE | EDAC_MODE_DISABLE;
	/* Copy IRQ handlers into VIM RAM */
	(void)memcpy(VIM_RAM_ADDR, _vector_start, irq_count);
}
