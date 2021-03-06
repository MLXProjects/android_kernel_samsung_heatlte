/*
 * r8a7373 processor support
 *
 * Copyright (C) 2012  Renesas Electronics Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/platform_data/rmobile_hwsem.h>
#include <linux/platform_data/irq-renesas-irqc.h>
#include <linux/platform_data/sh_cpufreq.h>
#include <linux/platform_device.h>
#include <linux/i2c/i2c-sh_mobile.h>
#include <linux/i2c-gpio.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/memblock.h>
#include <linux/mmc/renesas_mmcif.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <sh/mdmloader/mdmloader.h>
#include <asm/delay.h>
#include <asm/io.h>
#include <asm/system_info.h>
#include <asm/hardware/cache-l2x0.h>
#include <asm/mach/map.h>
#include <asm/mach/time.h>
#include <asm/mach/arch.h>
#include <linux/memblock.h>
#include <mach/common.h>
#include <mach/gpio.h>
#include <mach/irqs.h>
#include <mach/r8a7373.h>
#include <mach/serial.h>
#include <mach/memory-r8a7373.h>
#include <mach/common.h>
#include <mach/pm.h>
#include <linux/gpio.h>
#include <linux/sched.h>
#include <mach/setup-u2audio.h>
#include <mach/setup-u2ion.h>
#include <mach/setup-u2sdhi.h>
#ifdef CONFIG_SH_RAMDUMP
#include <mach/ramdump.h>
#endif
#if defined CONFIG_CPU_IDLE || defined CONFIG_SUSPEND
#ifndef CONFIG_PM_HAS_SECURE
#include "pm_ram0.h"
#else /*CONFIG_PM_HAS_SECURE*/
#include "pm_ram0_tz.h"
#endif /*CONFIG_PM_HAS_SECURE*/
#endif

#ifdef CONFIG_RENESAS_BT
#include <mach/dev-renesas-bt.h>
#endif

#include <mach/board-bcm4334-bt.h>

#include <mach/sec_debug.h>
#if defined(CONFIG_SEC_DEBUG_INFORM_IOTABLE)
#include <mach/sec_debug_inform.h>
#endif

#include <linux/regulator/consumer.h>
#include <mach/setup-u2usb.h>
#include <mach/sbsc.h>
#include <linux/spi/sh_msiof.h>
#include <linux/tpu_pwm_board.h>
#include <video/sh_mobile_lcdc.h>
#include <linux/mmc/host.h>

#ifdef CONFIG_SH_THERMAL
#include <linux/thermal.h>
#include <linux/platform_data/sh_thermal.h>
#endif

#include "setup-u2sci.h"
#include "sh-pfc.h"
#include <mach/setup-u2fb.h>

#ifdef CONFIG_ARCH_SHMOBILE
void __iomem *dummy_write_mem;
#endif

void __init remove_block(char *name, phys_addr_t start, phys_addr_t end);

static unsigned int shmobile_revision;

static struct map_desc common_io_desc[] __initdata = {
#ifdef PM_FUNCTION_START
/* We arrange for some of ICRAM 0 to be MT_MEMORY_NONCACHED, so
 * it can be executed from, for the PM code; it is then Normal Uncached memory,
 * with the XN (eXecute Never) bit clear. However, the data area of the ICRAM
 * has to be MT_DEVICE, to satisfy data access size requirements of the ICRAM.
 */
	{
		.virtual	= __IO_ADDRESS(0xe6000000),
		.pfn		= __phys_to_pfn(0xe6000000),
		.length		= PM_FUNCTION_START-0xe6000000,
		.type		= MT_DEVICE
	},
	{
		.virtual	= __IO_ADDRESS(PM_FUNCTION_START),
		.pfn		= __phys_to_pfn(PM_FUNCTION_START),
		.length		= PM_FUNCTION_END-PM_FUNCTION_START,
		.type		= MT_MEMORY_NONCACHED
	},
	{
		.virtual	= __IO_ADDRESS(PM_FUNCTION_END),
		.pfn		= __phys_to_pfn(PM_FUNCTION_END),
		.length		= 0xe7000000-PM_FUNCTION_END,
		.type		= MT_DEVICE
	},
#else
	{
		.virtual	= __IO_ADDRESS(0xe6000000),
		.pfn		= __phys_to_pfn(0xe6000000),
		.length		= SZ_16M,
		.type		= MT_DEVICE
	},
#endif
#if defined(CONFIG_SEC_DEBUG_INFORM_IOTABLE)
	{
		.virtual	= SEC_DEBUG_INFORM_VIRT,
		.pfn		= __phys_to_pfn(SEC_DEBUG_INFORM_PHYS),
		.length		= SZ_4K,
		.type		= MT_UNCACHED,
	},
#endif
};

static struct map_desc r8a7373_io_desc[] __initdata = {
	{
		.virtual	= __IO_ADDRESS(0xf0000000),
		.pfn		= __phys_to_pfn(0xf0000000),
		.length		= SZ_2M,
		.type		= MT_DEVICE
	},
};

void __init r8a7373_map_io(void)
{
	debug_ll_io_init();
	iotable_init(common_io_desc, ARRAY_SIZE(common_io_desc));
	if (shmobile_is_u2())
		iotable_init(r8a7373_io_desc, ARRAY_SIZE(r8a7373_io_desc));

#if defined(CONFIG_SEC_DEBUG)
	sec_debug_init();
#endif
	shmobile_dt_smp_map_io();
}


static struct renesas_irqc_config irqc0_data = {
	.irq_base = irq_pin(0), /* IRQ0 -> IRQ31 */
};

static struct renesas_irqc_config irqc1_data = {
	.irq_base = irq_pin(32), /* IRQ32 -> IRQ63 */
};

static struct renesas_irqc_config irqc10_data = {
	.irq_base = rt_irq(0),
};

static struct renesas_irqc_config irqc12_data = {
	.irq_base = modem_irq(0),
};

static struct sh_plat_hp_data cpufreq_hp_data = {
	.hotplug_samples = 10,
	.hotplug_es_samples = 2,
	.thresholds = {
	{.thresh_plugout = 0, .thresh_plugin = 0},
	/* Secondary CPUs filled in by r8a7373_add_standard_devices */
	},
};

static struct sh_cpufreq_plat_data cpufreq_data = {
	.hp_data = &cpufreq_hp_data,
};

static struct platform_device sh_cpufreq_device = {
	.name = "shmobile-cpufreq",
	.id = 0,
	.dev = {
		.platform_data = &cpufreq_data,
	},
};

/* IICM */
static struct i2c_sh_mobile_platform_data i2c8_platform_data = {
	.bus_speed	= 400000,
	.bus_data_delay = MIN_SDA_DELAY,
};

static struct resource i2c8_resources[] = {
	DEFINE_RES_MEM_NAMED(0xe6d20000, 0x9, "IICM"),
	DEFINE_RES_IRQ(gic_spi(191)),
};

static struct platform_device i2c8_device = {
	.name		= "i2c-sh7730",
	.id		= 8,
	.resource	= i2c8_resources,
	.num_resources	= ARRAY_SIZE(i2c8_resources),
	.dev		= {
		.platform_data	= &i2c8_platform_data,
	},
};


/* Transmit sizes and respective CHCR register values */
enum {
	XMIT_SZ_8BIT		= 0,
	XMIT_SZ_16BIT		= 1,
	XMIT_SZ_32BIT		= 2,
	XMIT_SZ_64BIT		= 7,
	XMIT_SZ_128BIT		= 3,
	XMIT_SZ_256BIT		= 4,
	XMIT_SZ_512BIT		= 5,
};

/* log2(size / 8) - used to calculate number of transfers */
#define TS_SHIFT {			\
	[XMIT_SZ_8BIT]		= 0,	\
	[XMIT_SZ_16BIT]		= 1,	\
	[XMIT_SZ_32BIT]		= 2,	\
	[XMIT_SZ_64BIT]		= 3,	\
	[XMIT_SZ_128BIT]	= 4,	\
	[XMIT_SZ_256BIT]	= 5,	\
	[XMIT_SZ_512BIT]	= 6,	\
}

#define TS_INDEX2VAL(i) ((((i) & 3) << 3) | (((i) & 0xc) << (20 - 2)))
#define CHCR_TX(xmit_sz) (DM_FIX | SM_INC | 0x800 | TS_INDEX2VAL((xmit_sz)))
#define CHCR_RX(xmit_sz) (DM_INC | SM_FIX | 0x800 | TS_INDEX2VAL((xmit_sz)))

static const struct sh_dmae_slave_config r8a7373_dmae_slaves[] = {
	{
		.slave_id	= SHDMA_SLAVE_SCIF0_TX,
		.addr		= 0xe6450020,
		.chcr		= CHCR_TX(XMIT_SZ_8BIT),
		.mid_rid	= 0x21,
	}, {
		.slave_id	= SHDMA_SLAVE_SCIF0_RX,
		.addr		= 0xe6450024,
		.chcr		= CHCR_RX(XMIT_SZ_8BIT),
		.mid_rid	= 0x22,
	}, {
		.slave_id	= SHDMA_SLAVE_SCIF1_TX,
		.addr		= 0xe6c50020,
		.chcr		= CHCR_TX(XMIT_SZ_8BIT),
		.mid_rid	= 0x25,
	}, {
		.slave_id	= SHDMA_SLAVE_SCIF1_RX,
		.addr		= 0xe6c50024,
		.chcr		= CHCR_RX(XMIT_SZ_8BIT),
		.mid_rid	= 0x26,
	}, {
		.slave_id	= SHDMA_SLAVE_SCIF2_TX,
		.addr		= 0xe6c60020,
		.chcr		= CHCR_TX(XMIT_SZ_8BIT),
		.mid_rid	= 0x29,
	}, {
		.slave_id	= SHDMA_SLAVE_SCIF2_RX,
		.addr		= 0xe6c60024,
		.chcr		= CHCR_RX(XMIT_SZ_8BIT),
		.mid_rid	= 0x2a,
	}, {
		.slave_id	= SHDMA_SLAVE_SCIF3_TX,
		.addr		= 0xe6c70020,
		.chcr		= CHCR_TX(XMIT_SZ_8BIT),
		.mid_rid	= 0x2d,
	}, {
		.slave_id	= SHDMA_SLAVE_SCIF3_RX,
		.addr		= 0xe6c70024,
		.chcr		= CHCR_RX(XMIT_SZ_8BIT),
		.mid_rid	= 0x2e,
	}, {
		.slave_id	= SHDMA_SLAVE_SCIF4_TX,
		.addr		= 0xe6c20040,
		.chcr		= CHCR_TX(XMIT_SZ_8BIT),
		.mid_rid	= 0x3d,
	}, {
		.slave_id	= SHDMA_SLAVE_SCIF4_RX,
		.addr		= 0xe6c20060,
		.chcr		= CHCR_RX(XMIT_SZ_8BIT),
		.mid_rid	= 0x3e,
	}, {
		.slave_id	= SHDMA_SLAVE_SCIF5_TX,
		.addr		= 0xe6c30040,
		.chcr		= CHCR_TX(XMIT_SZ_8BIT),
		.mid_rid	= 0x19,
	}, {
		.slave_id	= SHDMA_SLAVE_SCIF5_RX,
		.addr		= 0xe6c30060,
		.chcr		= CHCR_RX(XMIT_SZ_8BIT),
		.mid_rid	= 0x1a,
	}, {
		.slave_id	= SHDMA_SLAVE_SCIF6_TX,
		.addr		= 0xe6ce0040,
		.chcr		= CHCR_TX(XMIT_SZ_8BIT),
		.mid_rid	= 0x1d,
	}, {
		.slave_id	= SHDMA_SLAVE_SCIF6_RX,
		.addr		= 0xe6ce0060,
		.chcr		= CHCR_RX(XMIT_SZ_8BIT),
		.mid_rid	= 0x1e,
	}, {
		.slave_id	= SHDMA_SLAVE_SCIF7_TX,
		.addr		= 0xe6470040,
		.chcr		= CHCR_TX(XMIT_SZ_8BIT),
		.mid_rid	= 0x35,
	}, {
		.slave_id	= SHDMA_SLAVE_SCIF7_RX,
		.addr		= 0xe6470060,
		.chcr		= CHCR_RX(XMIT_SZ_8BIT),
		.mid_rid	= 0x36,
	}, {
		.slave_id	= SHDMA_SLAVE_SDHI0_TX,
		.addr		= 0xee100030,
		.chcr		= CHCR_TX(XMIT_SZ_16BIT),
		.mid_rid	= 0xc1,
		.burst_sizes	= (1 << 1) | (1 << 4) | (1 << 5),
	}, {
		.slave_id	= SHDMA_SLAVE_SDHI0_RX,
		.addr		= 0xee100030,
		.chcr		= CHCR_RX(XMIT_SZ_16BIT),
		.mid_rid	= 0xc2,
		.burst_sizes	= (1 << 1) | (1 << 4) | (1 << 5),
	}, {
		.slave_id	= SHDMA_SLAVE_SDHI1_TX,
		.addr		= 0xee120030,
		.chcr		= CHCR_TX(XMIT_SZ_16BIT),
		.mid_rid	= 0xc5,
		.burst_sizes	= (1 << 1) | (1 << 4) | (1 << 5),
	}, {
		.slave_id	= SHDMA_SLAVE_SDHI1_RX,
		.addr		= 0xee120030,
		.chcr		= CHCR_RX(XMIT_SZ_16BIT),
		.mid_rid	= 0xc6,
		.burst_sizes	= (1 << 1) | (1 << 4) | (1 << 5),
	}, {
		.slave_id	= SHDMA_SLAVE_SDHI2_TX,
		.addr		= 0xee140030,
		.chcr		= CHCR_TX(XMIT_SZ_16BIT),
		.mid_rid	= 0xc9,
		.burst_sizes	= (1 << 1) | (1 << 4) | (1 << 5),
	}, {
		.slave_id	= SHDMA_SLAVE_SDHI2_RX,
		.addr		= 0xee140030,
		.chcr		= CHCR_RX(XMIT_SZ_16BIT),
		.mid_rid	= 0xca,
		.burst_sizes	= (1 << 1) | (1 << 4) | (1 << 5),
	}, {
		.slave_id	= SHDMA_SLAVE_MMCIF0_TX,
		.addr		= 0xe6bd0034,
		.chcr		= CHCR_TX(XMIT_SZ_32BIT),
		.mid_rid	= 0xd1,
	}, {
		.slave_id	= SHDMA_SLAVE_MMCIF0_RX,
		.addr		= 0xe6bd0034,
		.chcr		= CHCR_RX(XMIT_SZ_32BIT),
		.mid_rid	= 0xd2,
	}, {
		.slave_id	= SHDMA_SLAVE_MMCIF1_TX,
		.addr		= 0xe6be0034,
		.chcr		= CHCR_TX(XMIT_SZ_32BIT),
		.mid_rid	= 0xe1,
	}, {
		.slave_id	= SHDMA_SLAVE_MMCIF1_RX,
		.addr		= 0xe6be0034,
		.chcr		= CHCR_RX(XMIT_SZ_32BIT),
		.mid_rid	= 0xe2,
	}, {
		.slave_id	= SHDMA_SLAVE_FSI2A_TX,
		.addr		= 0xec230024,
		.chcr		= CHCR_TX(XMIT_SZ_32BIT),
		.mid_rid	= 0xd5,
	}, {
		.slave_id	= SHDMA_SLAVE_FSI2A_RX,
		.addr		= 0xec230020,
		.chcr		= CHCR_RX(XMIT_SZ_16BIT),
		.mid_rid	= 0xd6,
	}, {
		.slave_id	= SHDMA_SLAVE_FSI2B_TX,
		.addr		= 0xec230064,
		.chcr		= CHCR_TX(XMIT_SZ_32BIT),
		.mid_rid	= 0xd9,
	}, {
		.slave_id	= SHDMA_SLAVE_FSI2B_RX,
		.addr		= 0xec230060,
		.chcr		= CHCR_RX(XMIT_SZ_16BIT),
		.mid_rid	= 0xda,
	}, {
		.slave_id	= SHDMA_SLAVE_SCUW_FFD_TX,
		.addr		= 0xec700708,
		.chcr		= CHCR_TX(XMIT_SZ_32BIT),
		.mid_rid	= 0x79,
	}, {
		.slave_id	= SHDMA_SLAVE_SCUW_FFU_RX,
		.addr		= 0xec700714,
		.chcr		= CHCR_RX(XMIT_SZ_32BIT),
		.mid_rid	= 0x7a,
	}, {
		.slave_id	= SHDMA_SLAVE_SCUW_CPUFIFO_0_TX,
		.addr		= 0xec700720,
		.chcr		= CHCR_TX(XMIT_SZ_32BIT),
		.mid_rid	= 0x7d,
	}, {
		.slave_id	= SHDMA_SLAVE_SCUW_CPUFIFO_2_RX,
		.addr		= 0xec700738,
		.chcr		= CHCR_RX(XMIT_SZ_32BIT),
		.mid_rid	= 0x7e,
	}, {
		.slave_id	= SHDMA_SLAVE_SCUW_CPUFIFO_1_TX,
		.addr		= 0xec70072c,
		.chcr		= CHCR_TX(XMIT_SZ_32BIT),
		.mid_rid	= 0x81,
	}, {
		.slave_id	= SHDMA_SLAVE_PCM2PWM_TX,
		.addr		= 0xec380080,
		.chcr		= CHCR_TX(XMIT_SZ_16BIT),
		.mid_rid	= 0x91,
	},
};

#define DMAE_CHANNEL(_offset)					\
	{							\
		.offset		= _offset - 0x20,		\
		.dmars		= _offset - 0x20 + 0x40,	\
	}

static const struct sh_dmae_channel r8a7373_dmae_channels[] = {
	DMAE_CHANNEL(0x8000),
	DMAE_CHANNEL(0x8080),
	DMAE_CHANNEL(0x8100),
	DMAE_CHANNEL(0x8180),
	DMAE_CHANNEL(0x8200),
	DMAE_CHANNEL(0x8280),
	DMAE_CHANNEL(0x8300),
	DMAE_CHANNEL(0x8380),
	DMAE_CHANNEL(0x8400),
	DMAE_CHANNEL(0x8480),
	DMAE_CHANNEL(0x8500),
	DMAE_CHANNEL(0x8580),
	DMAE_CHANNEL(0x8600),
	DMAE_CHANNEL(0x8680),
	DMAE_CHANNEL(0x8700),
	DMAE_CHANNEL(0x8780),
	DMAE_CHANNEL(0x8800),
	DMAE_CHANNEL(0x8880),
};

static const unsigned int ts_shift[] = TS_SHIFT;

static struct sh_dmae_pdata r8a7373_dmae_platform_data = {
	.slave		= r8a7373_dmae_slaves,
	.slave_num	= ARRAY_SIZE(r8a7373_dmae_slaves),
	.channel	= r8a7373_dmae_channels,
	.channel_num	= ARRAY_SIZE(r8a7373_dmae_channels),
	.ts_low_shift	= 3,
	.ts_low_mask	= 0x18,
	.ts_high_shift	= (20 - 2),	/* 2 bits for shifted low TS */
	.ts_high_mask	= 0x00300000,
	.ts_shift	= ts_shift,
	.ts_shift_num	= ARRAY_SIZE(ts_shift),
	.dmaor_init	= DMAOR_DME,
};

static struct resource r8a7373_dmae_resources[] = {
	{
		/* DescriptorMEM */
		.start  = 0xFE00A000,
		.end    = 0xFE00A7FC,
		.flags  = IORESOURCE_MEM,
	},
	{
		/* Registers including DMAOR and channels including DMARSx */
		.start	= 0xfe000020,
		.end	= 0xfe008a00 - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		/* DMA error IRQ */
		.start	= gic_spi(167),
		.end	= gic_spi(167),
		.flags	= IORESOURCE_IRQ,
	},
	{
		/* IRQ for channels 0-17 */
		.start	= gic_spi(147),
		.end	= gic_spi(164),
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device dma0_device = {
	.name		= "sh-dma-engine",
	.id		= 0,
	.resource	= r8a7373_dmae_resources,
	.num_resources	= ARRAY_SIZE(r8a7373_dmae_resources),
	.dev		= {
		.platform_data	= &r8a7373_dmae_platform_data,
	},
};

/*
 * These three HPB semaphores will be requested at board-init timing,
 * and globally available (even for out-of-tree loadable modules).
 */
struct hwspinlock *r8a7373_hwlock_gpio;
EXPORT_SYMBOL(r8a7373_hwlock_gpio);
struct hwspinlock *r8a7373_hwlock_cpg;
EXPORT_SYMBOL(r8a7373_hwlock_cpg);
struct hwspinlock *r8a7373_hwlock_sysc;
EXPORT_SYMBOL(r8a7373_hwlock_sysc);

#ifdef CONFIG_SMECO
static struct resource smc_resources[] = {
	[0] = DEFINE_RES_IRQ(gic_spi(193)),
	[1] = DEFINE_RES_IRQ(gic_spi(194)),
	[2] = DEFINE_RES_IRQ(gic_spi(195)),
	[3] = DEFINE_RES_IRQ(gic_spi(196)),
};

static struct platform_device smc_netdevice0 = {
	.name		= "smc_net_device",
	.id		= 0,
	.resource	= smc_resources,
	.num_resources	= ARRAY_SIZE(smc_resources),
};

static struct platform_device smc_netdevice1 = {
	.name		= "smc_net_device",
	.id		= 1,
	.resource	= smc_resources,
	.num_resources	= ARRAY_SIZE(smc_resources),
};
#endif /* CONFIG_SMECO */

/* Bus Semaphores 0 */
static struct hwsem_desc r8a7373_hwsem0_descs[] = {
	HWSEM(SMGPIO, 0x20),
	HWSEM(SMCPG, 0x50),
	HWSEM(SMSYSC, 0x70),
};

static struct hwsem_pdata r8a7373_hwsem0_platform_data = {
	.base_id	= SMGPIO,
	.descs		= r8a7373_hwsem0_descs,
	.nr_descs	= ARRAY_SIZE(r8a7373_hwsem0_descs),
};

static struct resource r8a7373_hwsem0_resources[] = {
	{
		.start	= 0xe6001800,
		.end	= 0xe600187f,
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device hwsem0_device = {
	.name		= "rmobile_hwsem",
	.id		= 0,
	.resource	= r8a7373_hwsem0_resources,
	.num_resources	= ARRAY_SIZE(r8a7373_hwsem0_resources),
	.dev = {
		.platform_data = &r8a7373_hwsem0_platform_data,
	},
};

/* Bus Semaphores 1 */
static struct hwsem_desc r8a7373_hwsem1_descs[] = {
	HWSEM(SMGP000, 0x30), HWSEM(SMGP001, 0x30),
	HWSEM(SMGP002, 0x30), HWSEM(SMGP003, 0x30),
	HWSEM(SMGP004, 0x30), HWSEM(SMGP005, 0x30),
	HWSEM(SMGP006, 0x30), HWSEM(SMGP007, 0x30),
	HWSEM(SMGP008, 0x30), HWSEM(SMGP009, 0x30),
	HWSEM(SMGP010, 0x30), HWSEM(SMGP011, 0x30),
	HWSEM(SMGP012, 0x30), HWSEM(SMGP013, 0x30),
	HWSEM(SMGP014, 0x30), HWSEM(SMGP015, 0x30),
	HWSEM(SMGP016, 0x30), HWSEM(SMGP017, 0x30),
	HWSEM(SMGP018, 0x30), HWSEM(SMGP019, 0x30),
	HWSEM(SMGP020, 0x30), HWSEM(SMGP021, 0x30),
	HWSEM(SMGP022, 0x30), HWSEM(SMGP023, 0x30),
	HWSEM(SMGP024, 0x30), HWSEM(SMGP025, 0x30),
	HWSEM(SMGP026, 0x30), HWSEM(SMGP027, 0x30),
	HWSEM(SMGP028, 0x30), HWSEM(SMGP029, 0x30),
	HWSEM(SMGP030, 0x30), HWSEM(SMGP031, 0x30),
};

static struct hwsem_pdata r8a7373_hwsem1_platform_data = {
	.base_id	= SMGP000,
	.descs		= r8a7373_hwsem1_descs,
	.nr_descs	= ARRAY_SIZE(r8a7373_hwsem1_descs),
};

static struct resource r8a7373_hwsem1_resources[] = {
	{
		.start	= 0xe6001800,
		.end	= 0xe600187f,
		.flags	= IORESOURCE_MEM,
	},
	{
		/* software extension base */
		.start	= SDRAM_SOFT_SEMAPHORE_TVRF_START_ADDR,
		.end	= SDRAM_SOFT_SEMAPHORE_TVRF_END_ADDR,
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device hwsem1_device = {
	.name		= "rmobile_hwsem",
	.id		= 1,
	.resource	= r8a7373_hwsem1_resources,
	.num_resources	= ARRAY_SIZE(r8a7373_hwsem1_resources),
	.dev		= {
		.platform_data	= &r8a7373_hwsem1_platform_data,
	},
};

/* Bus Semaphores 2 */
static struct hwsem_desc r8a7373_hwsem2_descs[] = {
	HWSEM(SMGP100, 0x40), HWSEM(SMGP101, 0x40),
	HWSEM(SMGP102, 0x40), HWSEM(SMGP103, 0x40),
	HWSEM(SMGP104, 0x40), HWSEM(SMGP105, 0x40),
	HWSEM(SMGP106, 0x40), HWSEM(SMGP107, 0x40),
	HWSEM(SMGP108, 0x40), HWSEM(SMGP109, 0x40),
	HWSEM(SMGP110, 0x40), HWSEM(SMGP111, 0x40),
	HWSEM(SMGP112, 0x40), HWSEM(SMGP113, 0x40),
	HWSEM(SMGP114, 0x40), HWSEM(SMGP115, 0x40),
	HWSEM(SMGP116, 0x40), HWSEM(SMGP117, 0x40),
	HWSEM(SMGP118, 0x40), HWSEM(SMGP119, 0x40),
	HWSEM(SMGP120, 0x40), HWSEM(SMGP121, 0x40),
	HWSEM(SMGP122, 0x40), HWSEM(SMGP123, 0x40),
	HWSEM(SMGP124, 0x40), HWSEM(SMGP125, 0x40),
	HWSEM(SMGP126, 0x40), HWSEM(SMGP127, 0x40),
	HWSEM(SMGP128, 0x40), HWSEM(SMGP129, 0x40),
	HWSEM(SMGP130, 0x40), HWSEM(SMGP131, 0x40),
};

static struct hwsem_pdata r8a7373_hwsem2_platform_data = {
	.base_id	= SMGP100,
	.descs		= r8a7373_hwsem2_descs,
	.nr_descs	= ARRAY_SIZE(r8a7373_hwsem2_descs),
};

static struct resource r8a7373_hwsem2_resources[] = {
	{
		.start	= 0xe6001800,
		.end	= 0xe600187f,
		.flags	= IORESOURCE_MEM,
	},
	{
		/* software bit extension */
		.start	= SDRAM_SOFT_SEMAPHORE_E20_START_ADDR,
		.end	= SDRAM_SOFT_SEMAPHORE_E20_END_ADDR,
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device hwsem2_device = {
	.name		= "rmobile_hwsem",
	.id			= 2,
	.resource	= r8a7373_hwsem2_resources,
	.num_resources	= ARRAY_SIZE(r8a7373_hwsem2_resources),
	.dev = {
		.platform_data = &r8a7373_hwsem2_platform_data,
	},
};


static struct resource sgx_resources[] = {
	DEFINE_RES_MEM(0xfd000000, 0xc000),
	DEFINE_RES_IRQ(gic_spi(92)),
};

static struct platform_device sgx_device = {
	.name		= "pvrsrvkm",
	.id		= -1,
	.resource	= sgx_resources,
	.num_resources	= ARRAY_SIZE(sgx_resources),
};

#ifdef CONFIG_SH_RAMDUMP
static struct hw_register_range ramdump_res[] __initdata = {
	DEFINE_RES_RAMDUMP(0xE6180080, 0xE6180080, HW_REG_32BIT, 0, NULL, 0),
	DEFINE_RES_RAMDUMP(0xE6020000, 0xE6020000, HW_REG_16BIT, 0, NULL, 0),
	DEFINE_RES_RAMDUMP(0xE6020004, 0xE6020004, HW_REG_8BIT, 0, NULL, 0),
	DEFINE_RES_RAMDUMP(0xE6020008, 0xE6020008, HW_REG_8BIT, 0, NULL, 0),
	DEFINE_RES_RAMDUMP(0xE6030000, 0xE6030000, HW_REG_16BIT, 0, NULL, 0),
	DEFINE_RES_RAMDUMP(0xE6030004, 0xE6030004, HW_REG_8BIT, 0, NULL, 0),
	DEFINE_RES_RAMDUMP(0xE6030008, 0xE6030008, HW_REG_8BIT, 0, NULL, 0),
	DEFINE_RES_RAMDUMP(0xE6100020, 0xE6100020, HW_REG_32BIT, 0, NULL, 0),
	DEFINE_RES_RAMDUMP(0xE6100028, 0xE610002c, HW_REG_32BIT, 0, NULL, 0),
	DEFINE_RES_RAMDUMP(0xE6130500, 0xE6130500, HW_REG_32BIT, 0, NULL, 0),
	DEFINE_RES_RAMDUMP(0xE6130510, 0xE6130510, HW_REG_32BIT, 0, NULL, 0),
	DEFINE_RES_RAMDUMP(0xE6130514, 0xE6130514, HW_REG_32BIT, 0, NULL, 0),
	DEFINE_RES_RAMDUMP(0xE6130518, 0xE6130518, HW_REG_32BIT, 0, NULL, 0),
	DEFINE_RES_RAMDUMP(0xE6150000, 0xE6150200, HW_REG_32BIT, 0, NULL, 0),
	DEFINE_RES_RAMDUMP(0xE6151000, 0xE6151180, HW_REG_32BIT, 0, NULL, 0),
	DEFINE_RES_RAMDUMP(0xE6180000, 0xE61800FC, HW_REG_32BIT, 0, NULL, 0),
	DEFINE_RES_RAMDUMP(0xE6180200, 0xE618027C, HW_REG_32BIT, 0, NULL, 0),
	DEFINE_RES_RAMDUMP(0xE618801C, 0xE6188024, HW_REG_32BIT, 0, NULL, 0),
	DEFINE_RES_RAMDUMP(0xE61C0100, 0xE61C0104, HW_REG_32BIT, 0, NULL, 0),
	DEFINE_RES_RAMDUMP(0xE61C0300, 0xE61C0304, HW_REG_32BIT, 0, NULL, 0),
	DEFINE_RES_RAMDUMP(0xE61D0000, 0xE61D0000, HW_REG_16BIT, 0, NULL, 0),
	DEFINE_RES_RAMDUMP(0xE61D0040, 0xE61D0040, HW_REG_16BIT, 0, NULL, 0),
	DEFINE_RES_RAMDUMP(0xE61D0044, 0xE61D0048, HW_REG_32BIT, 0, NULL, 0),
	DEFINE_RES_RAMDUMP(0xE623000C, 0xE623000C, HW_REG_32BIT, 0, NULL, 0),
	DEFINE_RES_RAMDUMP(0xE623200C, 0xE623200C, HW_REG_32BIT, 0, NULL, 0),
	DEFINE_RES_RAMDUMP(0xE62A0004, 0xE62A0008, HW_REG_8BIT, 0, MSTPSR5,
			MSTPST525),
	DEFINE_RES_RAMDUMP(0xE62A002C, 0xE62A002C, HW_REG_8BIT, 0, MSTPSR5,
			MSTPST525),
	/*NOTE: at the moment address increment is done by 4 byte steps
	 * so this will read one byte from 004 and one byte form 008 */
	DEFINE_RES_RAMDUMP(0xE6820004, 0xE6820008, HW_REG_8BIT, POWER_A3SP,
			MSTPSR1, MSTPST116),
	DEFINE_RES_RAMDUMP(0xE682002C, 0xE682002C, HW_REG_8BIT, POWER_A3SP,
			MSTPSR1, MSTPST116),
	DEFINE_RES_RAMDUMP(0xF000010C, 0xF0000110, HW_REG_32BIT, 0, NULL, 0),
	DEFINE_RES_RAMDUMP(0xFE400000, 0xFE400000, HW_REG_32BIT, 0, NULL, 0),
	DEFINE_RES_RAMDUMP(0xFE400200, 0xFE400240, HW_REG_32BIT, 0, NULL, 0),
	DEFINE_RES_RAMDUMP(0xFE400358, 0xFE400358, HW_REG_32BIT, 0, NULL, 0),
	DEFINE_RES_RAMDUMP(0xFE401000, 0xFE401004, HW_REG_32BIT, 0, NULL, 0),
	DEFINE_RES_RAMDUMP(0xFE4011F4, 0xFE4011F4, HW_REG_32BIT, 0, NULL, 0),
	DEFINE_RES_RAMDUMP(0xFE401050, 0xFE4010BC, HW_REG_32BIT, 0, NULL, 0),
	DEFINE_RES_RAMDUMP(0xFE951000, 0xFE9510FC, HW_REG_32BIT, POWER_A3R,
			SMSTPCR0, MSTO007),
};

struct ramdump_plat_data ramdump_pdata __initdata = {
	.reg_dump_base = SDRAM_REGISTER_DUMP_AREA_START_ADDR,
	.reg_dump_size = SDRAM_REGISTER_DUMP_AREA_END_ADDR -
			SDRAM_REGISTER_DUMP_AREA_START_ADDR + 1,
	/* size of reg dump of each core */
	.core_reg_dump_size = SZ_1K,
	.num_resources = ARRAY_SIZE(ramdump_res),
	.hw_register_range = ramdump_res,
};

/* platform_device structure can not be marked as __initdata as
 * it is used by platform_uevent etc. That is why __refdata needs
 * to be used. platform_data pointer is nulled in probe */
static struct platform_device ramdump_device __refdata = {
	.name = "ramdump",
	.id = -1,
	.dev.platform_data = &ramdump_pdata,
};
#endif

/* Removed unused SCIF Ports getting initialized
 * to reduce BOOT UP time "JIRAID 1382" */
static struct platform_device *r8a7373_early_devices[] __initdata = {
#ifdef CONFIG_SH_RAMDUMP
	&ramdump_device,
#endif
};
static struct resource mtd_res[] = {
                [0] = {
                                .name   = "mtd_trace",
                                .start     = SDRAM_STM_TRACE_BUFFER_START_ADDR,
                                .end       = SDRAM_STM_TRACE_BUFFER_END_ADDR,
                                .flags     = IORESOURCE_MEM,
                },
};
static struct platform_device mtd_device = {
                .name = "mtd_trace",
                .num_resources               = ARRAY_SIZE(mtd_res),
                .resource             = mtd_res,
};


/* HS-- ES20 Specific late devices for Dialog */
static struct platform_device *r8a7373_late_devices_es20_d2153[] __initdata = {
	&i2c8_device, /* IICM  */
	&dma0_device,
#ifdef CONFIG_SMECO
	&smc_netdevice0,
	&smc_netdevice1,
#endif
	&sgx_device,
	&mtd_device,
};


/* For different STM muxing options 0, 1, or None, as given by
 * boot_command_line parameter stm=0/1/n
 */

static struct sh_mmcif_plat_data renesas_mmcif_plat = {
	.sup_pclk	= 0,
	.caps		= MMC_CAP_4_BIT_DATA | MMC_CAP_8_BIT_DATA |
		MMC_CAP_1_8V_DDR | MMC_CAP_UHS_DDR50 | MMC_CAP_NONREMOVABLE,
	.slave_id_tx	= SHDMA_SLAVE_MMCIF0_TX,
	.slave_id_rx	= SHDMA_SLAVE_MMCIF0_RX,
	.max_clk	= 52000000,
};

static struct resource lcdc_resources[] = {
	[0] = {
		.name	= "irq_generator",
		.start	= 0xe61c2000,
		.end	= 0xe61c203f,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= rt_irq(0),
		.flags	= IORESOURCE_IRQ,
	},
	[2] = {
		.name	= "panel_irq_port",
		.start	= GPIO_PORT27,
		.end	= GPIO_PORT27,
		.flags	= IORESOURCE_MEM,
	},
	[3] = {
		.name	= "panel_esd_irq_port",
		.start	= GPIO_PORT6,
		.end	= GPIO_PORT6,
		.flags	= IORESOURCE_MEM,
	},
};

static struct sh_mobile_lcdc_info lcdc_info = {
	.clock_source	= LCDC_CLK_PERIPHERAL,
	/* LCDC0 */
	.ch[0] = {
		.chan = LCDC_CHAN_MAINLCD,
#ifdef CONFIG_FB_SH_MOBILE_RGB888
		.bpp = 24,
#else
		.bpp = 32,
#endif
	/* Can be overridden by r8a7373_set_panel_reset_gpio() */
		.panelreset_gpio = GPIO_PORT31,
		.paneldsi_irq = -1,
	},
};

static struct platform_device lcdc_device = {
	.name		= "sh_mobile_lcdc_fb",
	.num_resources	= ARRAY_SIZE(lcdc_resources),
	.resource	= lcdc_resources,
	.dev	= {
		.platform_data		= &lcdc_info,
		.coherent_dma_mask	= DMA_BIT_MASK(32),
	},
};

static struct resource mfis_resources[] = {
	[0] = {
		.name   = "MFIS",
		.start  = gic_spi(126),
		.flags  = IORESOURCE_IRQ,
	},
};

static struct platform_device mfis_device = {
	.name           = "mfis",
	.id             = 0,
	.resource       = mfis_resources,
	.num_resources  = ARRAY_SIZE(mfis_resources),
};

static struct resource mdm_reset_resources[] = {
	[0] = {
		.name	= "MODEM_RESET",
		.start	= 0xE6190000,
		.end	= 0xE61900FF,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= gic_spi(219), /* EPMU_int1 */
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device mdm_reset_device = {
	.name		= "rmc_wgm_reset_int",
	.id			= 0,
	.resource	= mdm_reset_resources,
	.num_resources	= ARRAY_SIZE(mdm_reset_resources),
};

#ifdef CONFIG_SPI_SH_MSIOF
/* SPI */
static struct sh_msiof_spi_info sh_msiof0_info = {
	.rx_fifo_override = 256,
	.num_chipselect = 1,
};

static struct resource sh_msiof0_resources[] = {
	[0] = {
		.start  = 0xe6e20000,
		.end    = 0xe6e20064 - 1,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = gic_spi(109),
		.flags  = IORESOURCE_IRQ,
	},
};

static struct platform_device sh_msiof0_device = {
	.name = "spi_sh_msiof",
	.id   = 0,
	.dev  = {
		.platform_data  = &sh_msiof0_info,
	},
	.num_resources  = ARRAY_SIZE(sh_msiof0_resources),
	.resource       = sh_msiof0_resources,
};
#endif

#ifdef CONFIG_SH_THERMAL
static struct ths_trip ths_trips[] = {
	{.temp = 0, .max_freq = 1196000, .hotplug_mask = 0x3,
				.type = THERMAL_TRIP_ACTIVE,},
	{.temp = 75, .max_freq = 897000, .hotplug_mask = 0x3,
				.type = THERMAL_TRIP_ACTIVE,},
	{.temp = 85, .max_freq = 373750, .hotplug_mask = 0x1,
				.type = THERMAL_TRIP_ACTIVE,},
	{.temp = 95, .max_freq = 0, .hotplug_mask = 0,
				.type = THERMAL_TRIP_CRITICAL,},
};

/*  Add for Thermal Sensor driver*/
static struct thermal_sensor_data ths_platdata = {
	.hysteresis = 5,
	.flags = THS_ENABLE_RESET,
	.shutdown_temp = 100,
	.clk_name = "thermal_sensor",
	.trips = ths_trips,
	.trip_cnt = ARRAY_SIZE(ths_trips),
};

static struct resource ths_resources[] = {
	[0] = {
		.name	= "THS",
		.start	= 0xe61F0000,
		.end	= 0xe61F0238 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= gic_spi(73), /* SPI# of THS is 73 */
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device thermal_sensor_device = {
	.name			= "thermal_sensor",
	.id				= 0,
	.num_resources	= ARRAY_SIZE(ths_resources),
	.resource	= ths_resources,
	.dev		= {
		.platform_data	= &ths_platdata,
	},
};
/* End Add for Thermal Sensor driver */

#endif /* CONFIG_SH_THERMAL */

static struct resource	tpu_resources[] = {
	[TPU_MODULE_0] = {
		.name	= "tpu0_map",
		.start	= 0xe6600000,
		.end	= 0xe6600200,
		.flags	= IORESOURCE_MEM,
	},
};

/* GPIO Settings */
static struct portn_gpio_setting_info_tpu tpu0_gpio_setting_info[] = {
	[0] = { /* TPU CHANNEL */
		.flag = 0,
		.port = GPIO_PORT36,
		/* GPIO settings to be retained at resume state */
		.active = {
			/* GPIO_FN_TPUTO0 ,*//*Func 3*/
			.port_fn	= GPIO_FN_PORT36_TPU0TO0,
			.pull		= PORTn_CR_PULL_DOWN,
			.direction	= PORTn_CR_DIRECTION_NOT_SET,
			.output_level	= PORTn_OUTPUT_LEVEL_NOT_SET,
		},
		/* GPIO settings to be set at suspend state */
		.inactive = {
			.port_fn	= GPIO_PORT36, /*Func 0*/
			.pull		= PORTn_CR_PULL_DOWN,
			.direction	= PORTn_CR_DIRECTION_INPUT,
		}
	},
};

static struct port_info
tpu_pwm_pfc[TPU_MODULE_MAX][TPU_CHANNEL_MAX] = {
	[TPU_MODULE_0] = {
		[TPU_CHANNEL_0]	= {
			/* GPIO_FN_TPUTO0,*/
			.port_func	=  GPIO_PORT36,
			.func_name	= "pwm-tpu0to0",
			.port_count	= ARRAY_SIZE(tpu0_gpio_setting_info),
			.tpu_gpio_setting_info	= tpu0_gpio_setting_info,
		},
		[TPU_CHANNEL_1]	= {
			.port_func	=  GPIO_FN_TPU0TO1,
			.func_name	= "pwm-tpu0to1",
			.port_count	= 0,
			.tpu_gpio_setting_info	= NULL,
		},
		[TPU_CHANNEL_2]	= {
			.port_func	=  GPIO_FN_TPU0TO2,
			.func_name	= "pwm-tpu0to2",
			.port_count	= 0,
			.tpu_gpio_setting_info	= NULL,
		},
		[TPU_CHANNEL_3]	= {
			.port_func	=  GPIO_FN_TPU0TO3,
			.func_name	= "pwm-tpu0to3",
			.port_count	= 0,
			.tpu_gpio_setting_info	= NULL,
		},
	},
};

static struct platform_device	tpu_devices[] = {
	{
		.name		= "tpu-renesas-sh_mobile",
		.id		= TPU_MODULE_0,
		.num_resources	= 1,
		.resource	= &tpu_resources[TPU_MODULE_0],
		.dev	= {
			.platform_data = &tpu_pwm_pfc[TPU_MODULE_0],
		},
	},
};

static struct platform_device *u2_devices[] __initdata = {
	&sh_cpufreq_device,
	&lcdc_device,
	&mfis_device,
	&tpu_devices[TPU_MODULE_0],
	&mdm_reset_device,
#ifdef CONFIG_SPI_SH_MSIOF
	&sh_msiof0_device,
#endif
#ifdef CONFIG_SH_THERMAL
	&thermal_sensor_device,
#endif
};

static struct platform_device *r8a7373_hwsem_devices[] __initdata = {
	&hwsem0_device,
	&hwsem1_device,
	&hwsem2_device,
};

static const struct resource pfc_resources[] = {
	DEFINE_RES_MEM(0xe6050000, 0x9000),
};

static const struct of_dev_auxdata r8a7373_auxdata_lookup[] __initconst = {
	/* Have to pass pdata to get fixed IRQ numbering for non-DT drivers */
	OF_DEV_AUXDATA("renesas,irqc", 0xe61c0000, NULL, &irqc0_data),
	OF_DEV_AUXDATA("renesas,irqc", 0xe61c0200, NULL, &irqc1_data),
	OF_DEV_AUXDATA("renesas,irqc", 0xe61c1400, NULL, &irqc10_data),
	OF_DEV_AUXDATA("renesas,irqc", 0xe61c1800, NULL, &irqc12_data),
	/* mmcif pdata is required to manage platform callbacks */
	OF_DEV_AUXDATA("renesas,renesas-mmcif", 0xe6bd0000, NULL,
		       &renesas_mmcif_plat),
	OF_DEV_AUXDATA("renesas,sdhi-r8a7373", 0xee100000, NULL,
			&sdhi0_info),
	OF_DEV_AUXDATA("renesas,sdhi-r8a7373", 0xee120000, NULL,
			&sdhi1_info),
	OF_DEV_AUXDATA("renesas,r8a66597-usb", 0xe6890000, NULL,
			&usbhs_func_data_d2153),
	{},
};

void __init r8a7373_pinmux_init(void)
{
	sh_pfc_register_mappings(scif_pinctrl_map,
				 ARRAY_SIZE(scif_pinctrl_map));

	/* We need hwspinlocks ready now for the pfc driver */
	platform_add_devices(r8a7373_hwsem_devices,
			ARRAY_SIZE(r8a7373_hwsem_devices));
}

/* Bit odd having GPIO numbers in this file - boards need this override */
void __init r8a7373_set_panel_reset_gpio(int gpio)
{
	lcdc_info.ch[0].panelreset_gpio = gpio;
}

void __init r8a7373_set_panel_func(struct fb_panel_func (*fn)(int))
{
	lcdc_info.panel_func = fn;

}

static const struct of_device_id gic_matches[] __initconst = {
	{ .compatible ="arm,cortex-a9-gic" },
	{ .compatible ="arm,cortex-a7-gic" },
	{ .compatible ="arm,cortex-a15-gic" },
};

void __init r8a7373_patch_ramdump_addresses(void)
{
	int i;
	struct device_node *gic;
	struct resource dist, cpu;

	gic = of_find_matching_node(NULL, gic_matches);
	if (!gic)
		return;

	of_address_to_resource(gic, 0, &dist);
	of_address_to_resource(gic, 1, &cpu);
	of_node_put(gic);

	/* fix up ramdump register ranges for u2b */
	for (i=0; i< ARRAY_SIZE(ramdump_res); i++) {
		if ((ramdump_res[i].start &~ 0xFF) == 0xF0000100) {
			/* GIC CPU address in U2 */
			ramdump_res[i].start += cpu.start - 0xF0000100;
			ramdump_res[i].end += cpu.start - 0xF0000100;
		} else if ((ramdump_res[i].start &~ 0xFFF) == 0xF0001000) {
			/* GIC dist address in U2 */
			ramdump_res[i].start += dist.start - 0xF0001000;
			ramdump_res[i].end += dist.start - 0xF0001000;
		}
	}
}

void __init r8a7373_add_standard_devices(void)
{
	int cpu;

	of_platform_populate(NULL, of_default_bus_match_table,
				r8a7373_auxdata_lookup, NULL);
	r8a7373_irqc_init(); /* Actually just INTCS and FIQ init now... */

	r8a7373_register_scif(SCIFA0);
	r8a7373_register_scif(SCIFB0);
	r8a7373_register_scif(SCIFB1);

	r8a7373_patch_ramdump_addresses();

	platform_add_devices(r8a7373_early_devices,
			ARRAY_SIZE(r8a7373_early_devices));

	/* ES2 and onwards */
	if (!shmobile_is_older(U2_VERSION_2_0))
		platform_add_devices(r8a7373_late_devices_es20_d2153,
			ARRAY_SIZE(r8a7373_late_devices_es20_d2153));
/* ES2.0 change end */

	/* Fill in cpufreq hotplug thresholds */
	for_each_possible_cpu(cpu) {
		if (cpu == 0)
			continue;
		/* XXX U2B will probably need different numbers */
#ifdef CONFIG_CPUFREQ_OVERDRIVE
		cpufreq_hp_data.thresholds[cpu].thresh_plugout = 364000;
		cpufreq_hp_data.thresholds[cpu].thresh_plugin = 364000;
#else
		cpufreq_hp_data.thresholds[cpu].thresh_plugout = 373750;
		cpufreq_hp_data.thresholds[cpu].thresh_plugin = 373750;
#endif
	}

	platform_add_devices(u2_devices, ARRAY_SIZE(u2_devices));
	gpiopd_hwspinlock_init(r8a7373_hwlock_gpio);
}
/*Do Dummy write in L2 cache to avoid A2SL turned-off
	just after L2-sync operation */
#ifdef CONFIG_ARCH_SHMOBILE
void __init r8a7373_avoid_a2slpowerdown_afterL2sync(void)
{
	dummy_write_mem = __arm_ioremap(
	(unsigned long)(SDRAM_NON_SECURE_SPINLOCK_START_ADDR + 0x00000400),
	0x00000400/*1k*/, MT_UNCACHED);

	if (dummy_write_mem == NULL)
		printk(KERN_ERR "97373_a2slpowerdown_workaround Failed\n");
}
#endif
/* do nothing for !CONFIG_SMP or !CONFIG_HAVE_TWD */

/* Lock used while modifying register */
static DEFINE_SPINLOCK(io_lock);

void sh_modify_register8(void __iomem *addr, u8 clear, u8 set)
{
	unsigned long flags;
	u8 val;
	spin_lock_irqsave(&io_lock, flags);
	val = __raw_readb(addr);
	val &= ~clear;
	val |= set;
	__raw_writeb(val, addr);
	spin_unlock_irqrestore(&io_lock, flags);
}
EXPORT_SYMBOL_GPL(sh_modify_register8);

void sh_modify_register16(void __iomem *addr, u16 clear, u16 set)
{
	unsigned long flags;
	u16 val;
	spin_lock_irqsave(&io_lock, flags);
	val = __raw_readw(addr);
	val &= ~clear;
	val |= set;
	__raw_writew(val, addr);
	spin_unlock_irqrestore(&io_lock, flags);
}
EXPORT_SYMBOL_GPL(sh_modify_register16);

void sh_modify_register32(void __iomem *addr, u32 clear, u32 set)
{
	unsigned long flags;
	u32 val;
	spin_lock_irqsave(&io_lock, flags);
	val = __raw_readl(addr);
	val &= ~clear;
	val |= set;
	__raw_writel(val, addr);
	spin_unlock_irqrestore(&io_lock, flags);
}
EXPORT_SYMBOL_GPL(sh_modify_register32);

void __iomem *sbsc_sdmracr1a;

static void SBSC_Init_520Mhz(void)
{
	unsigned long work;

	printk(KERN_ALERT "START < %s >\n", __func__);

	/* Check PLL3 status */
	work = __raw_readl(PLLECR);
	if (!(work & PLLECR_PLL3ST)) {
		printk(KERN_ALERT "PLLECR_PLL3ST is 0\n");
		return;
	}

	/* Set PLL3 = 1040 Mhz*/
	__raw_writel(PLL3CR_1040MHZ, PLL3CR);

	/* Wait PLL3 status on */
	while (1) {
		work = __raw_readl(PLLECR);
		work &= PLLECR_PLL3ST;
		if (work == PLLECR_PLL3ST)
			break;
	}

	/* Dummy read */
	__raw_readl(sbsc_sdmracr1a);
}

void __init r8a7373_zq_calibration(void)
{
	/* ES2.02 / LPDDR2 ZQ Calibration Issue WA */
	void __iomem *sbsc_sdmra_28200 = NULL;
	void __iomem *sbsc_sdmra_38200 = NULL;
	u8 reg8 = __raw_readb(STBCHRB3);

	if ((reg8 & 0x80) && !shmobile_is_older(U2_VERSION_2_2)) {
		pr_err("< %s >Apply for ZQ calibration\n", __func__);
		pr_err("< %s > Before CPG_PLL3CR 0x%8x\n",
				__func__, __raw_readl(PLL3CR));
		sbsc_sdmracr1a   = ioremap(SBSC_BASE + 0x000088, 0x4);
		sbsc_sdmra_28200 = ioremap(SBSC_BASE + 0x128200, 0x4);
		sbsc_sdmra_38200 = ioremap(SBSC_BASE + 0x138200, 0x4);
		if (sbsc_sdmracr1a && sbsc_sdmra_28200 && sbsc_sdmra_38200) {
			SBSC_Init_520Mhz();
			__raw_writel(SBSC_SDMRACR1A_ZQ, sbsc_sdmracr1a);
			__raw_writel(SBSC_SDMRA_DONE, sbsc_sdmra_28200);
			__raw_writel(SBSC_SDMRA_DONE, sbsc_sdmra_38200);
		} else {
			pr_err("%s: ioremap failed.\n", __func__);
		}
		pr_err("< %s > After CPG_PLL3CR 0x%8x\n",
				__func__, __raw_readl(PLL3CR));
		if (sbsc_sdmracr1a)
			iounmap(sbsc_sdmracr1a);
		if (sbsc_sdmra_28200)
			iounmap(sbsc_sdmra_28200);
		if (sbsc_sdmra_38200)
			iounmap(sbsc_sdmra_38200);
	}
}

static void shmobile_check_rev(void)
{
	shmobile_revision = __raw_readl(IOMEM(CCCR));
}

inline unsigned int shmobile_rev(void)
{
	unsigned int chiprev;

	if (!shmobile_revision)
		shmobile_check_rev();
	chiprev = (shmobile_revision & CCCR_VERSION_MASK);
	return chiprev;
}
EXPORT_SYMBOL(shmobile_rev);

/*+ recovering legacy code to get a proper revision */
static int read_board_rev(void)
{
	int rev0, rev1, rev2, rev3, ret;
	int error;
	error = gpio_request(GPIO_PORT72, "HW_REV0");
	if (error < 0)
		goto ret_err;
	error = gpio_direction_input(GPIO_PORT72);
	if (error < 0)
		goto ret_err1;
	gpio_pull_down_port(GPIO_PORT72);

	rev0 = gpio_get_value(GPIO_PORT72);
	if (rev0 < 0) {
		error = rev0;
		goto ret_err1;
	}

	error = gpio_request(GPIO_PORT73, "HW_REV1");
	if (error < 0)
		goto ret_err1;
	error = gpio_direction_input(GPIO_PORT73);
	if (error < 0)
		goto ret_err2;
	gpio_pull_down_port(GPIO_PORT73);

	rev1 = gpio_get_value(GPIO_PORT73);
	if (rev1 < 0) {
		error = rev1;
		goto ret_err2;
	}

	error = gpio_request(GPIO_PORT74, "HW_REV2");
	if (error < 0)
		goto ret_err2;
	error = gpio_direction_input(GPIO_PORT74);
	if (error < 0)
		goto ret_err3;
	gpio_pull_down_port(GPIO_PORT74);

	rev2 = gpio_get_value(GPIO_PORT74);
	if (rev2 < 0) {
		error = rev2;
		goto ret_err3;
	}

	error = gpio_request(GPIO_PORT75, "HW_REV3");
	if (error < 0)
		goto ret_err3;
	error = gpio_direction_input(GPIO_PORT75);
	if (error < 0)
		goto ret_err4;
	gpio_pull_down_port(GPIO_PORT75);

	rev3 = gpio_get_value(GPIO_PORT75);
	if (rev3 < 0) {
		error = rev3;
		goto ret_err4;
	}

	ret =  rev3 << 3 | rev2 << 2 | rev1 << 1 | rev0;
	return ret;
ret_err4:
	 gpio_free(GPIO_PORT75);
ret_err3:
	 gpio_free(GPIO_PORT74);
ret_err2:
	 gpio_free(GPIO_PORT73);
ret_err1:
	 gpio_free(GPIO_PORT72);
ret_err:
	return error;
}
unsigned int u2_get_board_rev(void)
{
	static int board_rev_val = -1;
	unsigned int loop = 0;

	/*if Revision read is faild for 3 times genarate panic*/
	if (unlikely(board_rev_val < 0)) {
		for (loop = 0; loop < 3; loop++) {
			board_rev_val = read_board_rev();
			if (board_rev_val >= 0)
				break;
		}
		if (unlikely(loop == 3))
			panic("Board revision not found\n");
	}
	/*board revision is always be a +value*/

	return (unsigned int) board_rev_val;
}
EXPORT_SYMBOL_GPL(u2_get_board_rev);
/*- recovering legacy code to get a proper revision */

void __init r8a7373_l2cache_init(void)
{
#ifdef CONFIG_CACHE_L2X0
	/*
	 * [30] Early BRESP enable
	 * [27] Non-secure interrupt access control
	 * [26] Non-secure lockdown enable
	 * [22] Shared attribute override enable
	 * [19:17] Way-size: b011 = 64KB
	 * [16] Accosiativity: 0 = 8-way
	 */
	l2x0_of_init(0x4c460000, 0x820f0fff);
#endif
}


void __init r8a7373_init_early(void)
{
	sh_primary_pfc = "e6050000.pfc";

#ifdef CONFIG_ARM_TZ
	r8a7373_l2cache_init();
#endif
}

void __init r8a7373_init_late(void)
{
#ifdef CONFIG_PM
	rmobile_pm_late_init();
#endif
}

void __init remove_block(char *name, phys_addr_t start, phys_addr_t end)
{
	int ret;

	/* Assumption that we have to have address in SDRAM and
	 * we can have max 2 gigs of SDRAM (current limit on EOS2)
	 * Only do alloc if it's within the range kernel can see.
	 */
	if (start >= SDRAM_KERNEL_START_ADDR) {
		ret = memblock_remove(start, (end-start)+1);
		pr_alert("%s memblock_remove: 0x%x-0x%x = %d\n", name,
			start, end, ret);
		BUG_ON(ret < 0);
	} else {
		pr_alert("%s:0x%x-0x%x\n", name, start, end);
	}
}

/*
 * Common reserve for R8 A7373 - for memory carveouts
 */
void __init r8a7373_reserve(void)
{
	 /*
	 * Register whole sdram as split. We know that in R8A7373 physical ram
	 * always starts at SDRAM_START_ADDR (0x40000000) and the kernel knows
	 * all the ram at this point.
	 */

	ulong sdram_trace_end = SDRAM_STM_TRACE_BUFFER_END_ADDR;

	if (mdm_sdram_trace_small())
		sdram_trace_end = SDRAM_SMALL_STM_TRACE_BUFFER_END_ADDR;

	register_ramdump_split("SDRAM", SDRAM_START_ADDR,
			memblock_end_of_DRAM() - 1);

	/* Register icram 0 and 1 as split */
	register_ramdump_split("icram0", ICRAM0_START_ADDR, ICRAM0_END_ADDR);
	register_ramdump_split("icram1", ICRAM1_START_ADDR, ICRAM1_END_ADDR);

	/* Register splits for modem */
	register_ramdump_split("SDRAM_CP", SDRAM_MODEM_START_ADDR,
			SDRAM_MODEM_END_ADDR);
	register_ramdump_split("SDRAM_CP_MODEMTR",
			SDRAM_STM_TRACE_BUFFER_START_ADDR,
			sdram_trace_end);
	register_ramdump_split("SDRAM_CP_SHARED", SDRAM_SMC_START_ADDR,
			SDRAM_SMC_END_ADDR);

	register_ramdump_split("SDRAM_VOCODER",
			SDRAM_VOCODER_START_ADDR,
			SDRAM_VOCODER_END_ADDR);

	/* Register bootlog area for reset reason and boot loader debugging */
	register_ramdump_split("bootlog", SDRAM_BOOTLOG_START_ADDR,
			SDRAM_BOOTLOG_END_ADDR);

	/* Fixed reserves/removes start */

	u2evm_ion_adjust();
	u2vcd_reserve();

	remove_block("SDRAM_MFI",
		SDRAM_MFI_START_ADDR,
		SDRAM_MFI_END_ADDR);

	remove_block("SDRAM_SH-Firmware",
		SDRAM_SH_FIRM_START_ADDR,
		SDRAM_SH_FIRM_END_ADDR);

#ifdef CONFIG_MOBICORE_API
	/* Reserve memory for secure DRM buffer */
	remove_block("SDRAM_DRM",
		SDRAM_DRM_AREA_START_ADDR,
		SDRAM_DRM_AREA_END_ADDR);
#endif

	remove_block("SDRAM_MODEM",
		SDRAM_MODEM_START_ADDR,
		SDRAM_MODEM_END_ADDR);

#ifdef SDRAM_DIAMOND_START_ADDR
	remove_block("SDRAM_DIAMOND",
		SDRAM_DIAMOND_START_ADDR,
		SDRAM_DIAMOND_END_ADDR);
#endif

	remove_block("SDRAM_STM_TRACE",
		SDRAM_STM_TRACE_BUFFER_START_ADDR,
		sdram_trace_end);

	remove_block("SDRAM_NON_SEC_SPINLOCK",
		SDRAM_NON_SECURE_SPINLOCK_START_ADDR,
		SDRAM_NON_SECURE_SPINLOCK_END_ADDR);

	remove_block("SDRAM_SMC",
		SDRAM_SMC_START_ADDR,
		SDRAM_SMC_END_ADDR);

	remove_block("SDRAM_CRASHLOG",
		SDRAM_CRASHLOG_START_ADDR,
		SDRAM_CRASHLOG_END_ADDR);

	remove_block("SDRAM_TVRF",
		SDRAM_SOFT_SEMAPHORE_TVRF_START_ADDR,
		SDRAM_SOFT_SEMAPHORE_TVRF_END_ADDR);

	remove_block("SDRAM_FREQ",
		SDRAM_SOFT_SEMAPHORE_FREQ_START_ADDR,
		SDRAM_SOFT_SEMAPHORE_FREQ_END_ADDR);

	remove_block("SDRAM_E20",
		SDRAM_SOFT_SEMAPHORE_E20_START_ADDR,
		SDRAM_SOFT_SEMAPHORE_E20_END_ADDR);

	remove_block("SDRAM_NONVOL_FLAG",
		SDRAM_NON_VOLATILE_FLAG_AREA_START_ADDR,
		SDRAM_NON_VOLATILE_FLAG_AREA_END_ADDR);

	remove_block("SDRAM_SDTOC",
		SDRAM_SDTOC_START_ADDR,
		SDRAM_SDTOC_END_ADDR);

	remove_block("SDRAM_SEC_SPINLOCK",
		SDRAM_SECURE_SPINLOCK_AND_DATA_START_ADDR,
		SDRAM_SECURE_SPINLOCK_AND_DATA_END_ADDR);

	remove_block("SDRAM_ROTATION",
		SDRAM_ROTATION_BUFFER_START_ADDR,
		SDRAM_ROTATION_BUFFER_END_ADDR);

#if defined(CONFIG_SEC_DEBUG)
	sec_debug_magic_init();
#endif

	/* Dynamic reserves/removes start*/
	if (u2fb_reserve() < 0)
		pr_err("u2fb_reserve failed\n");
}

static const char *r8a7373_dt_compat[] __initdata = {
	"renesas,r8a7373",
	"renesas,r8a73a7",
	NULL,
};

DT_MACHINE_START(DT_R8A7373, "R8a7373 SOC DT Support")
	.dt_compat	= r8a7373_dt_compat,
MACHINE_END