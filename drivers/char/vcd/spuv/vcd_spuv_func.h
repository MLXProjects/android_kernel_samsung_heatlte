/* vcd_spuv_func.h
 *
 * Copyright (C) 2012-2013 Renesas Mobile Corp.
 * All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef __VCD_SPUV_FUNC_H__
#define __VCD_SPUV_FUNC_H__

/*
 * constant definition
 */

/* hwspin_lock time out value */
#define VCD_SPUV_FUNC_MAX_LOCK_TIME		(62)
#define VCD_SPUV_FUNC_IPC_MAX_LOCK_TIME		(15)

/* spuv use power domain */
#define VCD_SPUV_FUNC_POWER_DOMAIN_MAX		2

/* spuv if time out value */
#define VCD_SPUV_FUNC_MAX_WAIT_TIME		(3000)
#define VCD_SPUV_FUNC_TIMEOUT			0

/* spuv max retry value */
#define VCD_SPUV_FUNC_MAX_RETRY			3

/* word to byte */
#define VCD_SPUV_FUNC_WORD_TO_BYTE		4

/* spuv memory bit shift */
#define VCD_SPUV_FUNC_BIT_SHIFT			8

/* spuv memory bit shift */
#define VCD_SPUV_FUNC_NEXT_ADDR			4

/* cpu-dsp communication registers 0 to 7 (COM0 to 7) */
#define VCD_SPUV_FUNC_COM2_MASK			0x00FFFFFF
#define VCD_SPUV_FUNC_COM3_MASK			0x00FF0000

/* binary file name */
#define VCD_SPUV_FUNC_SPUV_FILE_NAME		"spuv.bin"
#define VCD_SPUV_FUNC_PCM_PROC_FILE_NAME	"pcm_proc.bin"
#define VCD_SPUV_FUNC_DIAMOND_FILE_NAME		"diamond.bin"

/* sd path */
#define VCD_SPUV_FUNC_SPUV_SD_PATH		"/mnt/sdcard/spuv/spuv.bin"
#define VCD_SPUV_FUNC_PCM_SD_PATH		"/mnt/sdcard/spuv/pcm_proc.bin"

/* read files */
#define VCD_SPUV_FUNC_OFFSET_GLOBAL_SIZE	0x5
#define VCD_SPUV_FUNC_OFFSET_PAGE_NUM		0x8
#define VCD_SPUV_FUNC_OFFSET_MEMORY_TYPE	0xA
#define VCD_SPUV_FUNC_OFFSET_PAGE_SIZE		0xC
#define VCD_SPUV_FUNC_OFFSET_PAGE_DATA		0x10
#define VCD_SPUV_FUNC_UNIT_PAGE_SIZE		4
#define VCD_SPUV_FUNC_UNIT_GLOBAL_SIZE		1024

#define VCD_SPUV_FUNC_MEMORY_TYPE_NONE_00	0x00
#define VCD_SPUV_FUNC_MEMORY_TYPE_NONE_FF	0xFF
#define VCD_SPUV_FUNC_MEMORY_TYPE_PRAM		0x50
#define VCD_SPUV_FUNC_MEMORY_TYPE_XRAM		0x58
#define VCD_SPUV_FUNC_MEMORY_TYPE_YRAM		0x59

#define VCD_SPUV_FUNC_PAGE_SIZE			16

#define VCD_SPUV_FUNC_CPU_TO_DSP_BIT_SHIFT	8

/* cache memory control register (CCTL) */
#define VCD_SPUV_FUNC_CCTL_LWSWAP_LW		0x00000000
#define VCD_SPUV_FUNC_CCTL_LWSWAP_NLW		0x80000000
#define VCD_SPUV_FUNC_CCTL_FORM_LOWER		0x00000000
#define VCD_SPUV_FUNC_CCTL_FORM_UPPER		0x10000000
#define VCD_SPUV_FUNC_CCTL_P0C_NORMAL		0x00000000
#define VCD_SPUV_FUNC_CCTL_P0C_ENABLE		0x00000004
#define VCD_SPUV_FUNC_CCTL_X0C_NORMAL		0x00000000
#define VCD_SPUV_FUNC_CCTL_X0C_ENABLE		0x00000002
#define VCD_SPUV_FUNC_CCTL_Y0C_NORMAL		0x00000000
#define VCD_SPUV_FUNC_CCTL_Y0C_ENABLE		0x00000001
#define VCD_SPUV_FUNC_CCTL_CACHE_ON	(VCD_SPUV_FUNC_CCTL_P0C_ENABLE | \
					 VCD_SPUV_FUNC_CCTL_X0C_ENABLE | \
					 VCD_SPUV_FUNC_CCTL_Y0C_ENABLE)
#define VCD_SPUV_FUNC_CCTL_CACHE_OFF	(VCD_SPUV_FUNC_CCTL_P0C_NORMAL | \
					VCD_SPUV_FUNC_CCTL_X0C_NORMAL | \
					VCD_SPUV_FUNC_CCTL_Y0C_NORMAL)
/* cache memory status register (SPUMSTS) */
#define VCD_SPUV_FUNC_SPUMSTS_IDL_OP		0x00000000
#define VCD_SPUV_FUNC_SPUMSTS_IDL_NOP		0x00000001
#define VCD_SPUV_FUNC_SPUMSTS_WAIT_MAX		512

/* voiceif clock and reset control register (CRMU_VOICEIF) */
#define VCD_SPUV_FUNC_CRMU_VOICEIF_RST_RLS	0x00000010
#define VCD_SPUV_FUNC_CRMU_VOICEIF_SPUV_EN_REG	0x00000002
#define VCD_SPUV_FUNC_CRMU_VOICEIF_SUB_EN_REG	0x00000001
#define VCD_SPUV_FUNC_CRMU_VOICEIF_ON		\
			(VCD_SPUV_FUNC_CRMU_VOICEIF_RST_RLS | \
			VCD_SPUV_FUNC_CRMU_VOICEIF_SPUV_EN_REG | \
			VCD_SPUV_FUNC_CRMU_VOICEIF_SUB_EN_REG)

/* p/x/y ram global memory size control register (GADDR_CTRL_P/X/Y) */
#define VCD_SPUV_FUNC_GADDR_CTRL_1KW		0x00000009
#define VCD_SPUV_FUNC_GADDR_CTRL_2KW		0x0000000a
#define VCD_SPUV_FUNC_GADDR_CTRL_4KW		0x0000000b
#define VCD_SPUV_FUNC_GADDR_CTRL_8KW		0x0000000c
#define VCD_SPUV_FUNC_GADDR_CTRL_16KW		0x0000000d
#define VCD_SPUV_FUNC_GADDR_CTRL_32KW		0x0000000e
#define VCD_SPUV_FUNC_GADDR_CTRL_64KW		0x0000000f

/* dsp full reset register (DSPRST) */
#define VCD_SPUV_FUNC_DSPRST_ACTIVE		0x00000000
#define VCD_SPUV_FUNC_DSPRST_RESET		0x00000001

/* message send mask (COM0 to 7) */
#define VCD_SPUV_FUNC_COM2_MASK			0x00FFFFFF
#define VCD_SPUV_FUNC_COM3_MASK			0x00FF0000

/* interrupt message control register for CPU (AMSGIT) */
#define VCD_SPUV_FUNC_AMSGIT_MSG_REQ		0x00000001
#define VCD_SPUV_FUNC_AMSGIT_MSG_ACK		0x00000002

/* dsp core reset register (DSPCORERST) */
#define VCD_SPUV_FUNC_DSPCORERST_ACTIVE		0x00000000
#define VCD_SPUV_FUNC_DSPCORERST_RESET		0x00000001

/* cpu interrupt source mask register (IEMASKC) */
#define VCD_SPUV_FUNC_IEMASKC_DISABLE		0x00000111
#define VCD_SPUV_FUNC_IEMASKC_ENABLE		0x00001111

/* cpu interrupt signal mask register (IMASKC) */
#define VCD_SPUV_FUNC_IMASKC_DSP_DISABLE	0x00000111
#define VCD_SPUV_FUNC_IMASKC_DSP_ENABLE		0x00001111

/* soft ware reset register (SRCR2) */
#define VCD_SPUV_FUNC_SRCR2_SPU2V_RESET		0x00100000

/* HPB HPBCTRL2 register set bit */
#define VCD_SPUV_FUNC_HPBCTRL2_DMSENMSH		0x00000800
#define VCD_SPUV_FUNC_HPBCTRL2_DMSENMRAM	0x00008000

/* CPG MPMODE set bit */
#define VCD_SPUV_FUNC_MPMODE_MPSWMSK		0x00000080

/* fw static buffer size */
#define VCD_SPUV_FUNC_FW_BUFFER_SIZE		0x00180000

/* mixing parameter */
#define	VCD_SPUV_FUNC_FR10_MIX_MAX		60
#define	VCD_SPUV_FUNC_FR10_MIX_MAX_VAL		32767
#define	VCD_SPUV_FUNC_FR10_MIX_MIN_VAL		-32768

/* SRC mode */
#define VCD_SPUV_FUNC_NO_SRC			0x88
#define VCD_SPUV_FUNC_UNKNOWN_SRC		0xFF

/* sdram static area */
#define SPUV_FUNC_SDRAM_NON_CACHE_AREA_SIZE	0x740000
#define SPUV_FUNC_SDRAM_CACHE_AREA_SIZE		0x080000
#define SPUV_FUNC_SDRAM_DIAMOND_AREA_SIZE	0x040000

#define SPUV_FUNC_SDRAM_AREA_TOP_PHY		\
		g_spuv_func_sdram_static_area_top_phy
#define SPUV_FUNC_SDRAM_NON_CACHE_AREA_TOP	\
		g_spuv_func_sdram_static_non_cache_area_top
#define SPUV_FUNC_SDRAM_CACHE_AREA_TOP		\
		g_spuv_func_sdram_static_cache_area_top
/* non cache area */
#define SPUV_FUNC_SDRAM_FIRMWARE_BUFFER		( \
			SPUV_FUNC_SDRAM_NON_CACHE_AREA_TOP + 0x00000000)
#define SPUV_FUNC_SDRAM_BINARY_READ_BUFFER	( \
			SPUV_FUNC_SDRAM_NON_CACHE_AREA_TOP + 0x00000000)
#define SPUV_FUNC_SDRAM_SPUV_SEND_MSG_BUFFER	( \
			SPUV_FUNC_SDRAM_NON_CACHE_AREA_TOP + 0x00700000)
#define SPUV_FUNC_SDRAM_RECORD_BUFFER_0		( \
			SPUV_FUNC_SDRAM_NON_CACHE_AREA_TOP + 0x00701000)
#define SPUV_FUNC_SDRAM_RECORD_BUFFER_1		( \
			SPUV_FUNC_SDRAM_NON_CACHE_AREA_TOP + 0x00701280)
#define SPUV_FUNC_SDRAM_PLAYBACK_BUFFER_0	( \
			SPUV_FUNC_SDRAM_NON_CACHE_AREA_TOP + 0x00702000)
#define SPUV_FUNC_SDRAM_PLAYBACK_BUFFER_1	( \
			SPUV_FUNC_SDRAM_NON_CACHE_AREA_TOP + 0x00702280)
#define SPUV_FUNC_SDRAM_VOIP_DL_TEMP_BUFFER_0	( \
			SPUV_FUNC_SDRAM_NON_CACHE_AREA_TOP + 0x00703000)
#define SPUV_FUNC_SDRAM_VOIP_DL_TEMP_BUFFER_1	( \
			SPUV_FUNC_SDRAM_NON_CACHE_AREA_TOP + 0x00703280)
#define SPUV_FUNC_SDRAM_VOIP_UL_TEMP_BUFFER_0	( \
			SPUV_FUNC_SDRAM_NON_CACHE_AREA_TOP + 0x00703800)
#define SPUV_FUNC_SDRAM_VOIP_UL_TEMP_BUFFER_1	( \
			SPUV_FUNC_SDRAM_NON_CACHE_AREA_TOP + 0x00703A80)
#define SPUV_FUNC_SDRAM_PT_PLAYBACK_BUFFER_0	( \
			SPUV_FUNC_SDRAM_NON_CACHE_AREA_TOP + 0x00703D00)
#define SPUV_FUNC_SDRAM_PT_PLAYBACK_BUFFER_1	( \
			SPUV_FUNC_SDRAM_NON_CACHE_AREA_TOP + 0x00704480)

/* cache area */
#define SPUV_FUNC_SDRAM_PROC_MSG_BUFFER		( \
			SPUV_FUNC_SDRAM_CACHE_AREA_TOP + 0x00000000)
#define SPUV_FUNC_SDRAM_SPUV_CRASHLOG		( \
			SPUV_FUNC_SDRAM_CACHE_AREA_TOP + 0x00002000)
#define SPUV_FUNC_SDRAM_SPUV_MSG_BUFFER		( \
			SPUV_FUNC_SDRAM_CACHE_AREA_TOP + 0x00030000)
#define SPUV_FUNC_SDRAM_FW_RESULT_BUFFER	( \
			SPUV_FUNC_SDRAM_CACHE_AREA_TOP + 0x00031000)
#define SPUV_FUNC_SDRAM_SYSTEM_INFO_BUFFER	( \
			SPUV_FUNC_SDRAM_CACHE_AREA_TOP + 0x00033000)
#define SPUV_FUNC_SDRAM_VOIP_DL_BUFFER_0	( \
			SPUV_FUNC_SDRAM_CACHE_AREA_TOP + 0x00035000)
#define SPUV_FUNC_SDRAM_VOIP_DL_BUFFER_1	( \
			SPUV_FUNC_SDRAM_CACHE_AREA_TOP + 0x00035800)
#define SPUV_FUNC_SDRAM_VOIP_UL_BUFFER_0	( \
			SPUV_FUNC_SDRAM_CACHE_AREA_TOP + 0x00036000)
#define SPUV_FUNC_SDRAM_VOIP_UL_BUFFER_1	( \
			SPUV_FUNC_SDRAM_CACHE_AREA_TOP + 0x00036800)

/* spuv physical memory configuration mapping */
#define SPUV_FUNC_DATA_RAM_SIZE		0x00040000
#define SPUV_FUNC_PRAM0_PHY		0xECB00000
#define SPUV_FUNC_XRAM0_PHY		0xECB40000
#define SPUV_FUNC_YRAM0_PHY		0xECB80000
#define SPUV_FUNC_DSPIO_PHY		0xECBC0000

/* spuv crash log */
#define SPUV_FUNC_DUMP_XMEM_SIZE	0x10000
#define SPUV_FUNC_DUMP_YMEM_SIZE	0x10000

/* HPB registers */
#define SPUV_FUNC_HPB_REG_SIZE		0x8
#define SPUV_FUNC_HPB_REG_TOP_PHY	(HPB_BASE_PHYS + 0x1014)
#define SPUV_FUNC_HPB_REG_TOP		g_spuv_func_hpb_register_top
#define SPUV_FUNC_RW_32_HPB_HPBCTRL2	(SPUV_FUNC_HPB_REG_TOP + 0x00000004)

/* CPG registers */
#define SPUV_FUNC_CPG_REG_SIZE		0x81CC
#define SPUV_FUNC_CPG_REG_TOP_PHY	CPG_BASE_PHYS
#define SPUV_FUNC_CPG_REG_TOP		g_spuv_func_cpg_register_top
#define SPUV_FUNC_RW_32_CPG_VCLKCR1	(SPUV_FUNC_CPG_REG_TOP + 0x00000008)
#define SPUV_FUNC_RW_32_CPG_VCLKCR2	(SPUV_FUNC_CPG_REG_TOP + 0x0000000C)
#define SPUV_FUNC_RW_32_CPG_VCLKCR3	(SPUV_FUNC_CPG_REG_TOP + 0x00000014)
#define SPUV_FUNC_RW_32_CPG_VCLKCR4	(SPUV_FUNC_CPG_REG_TOP + 0x0000001C)
#define SPUV_FUNC_RW_32_CPG_VCLKCR5	(SPUV_FUNC_CPG_REG_TOP + 0x00000034)
#define SPUV_FUNC_RW_32_CPG_FSIACKCR	(SPUV_FUNC_CPG_REG_TOP + 0x00000018)
#define SPUV_FUNC_RW_32_CPG_FSIBCKCR	(SPUV_FUNC_CPG_REG_TOP + 0x00000090)
#define SPUV_FUNC_RW_32_CPG_SPUACKCR	(SPUV_FUNC_CPG_REG_TOP + 0x00000084)
#define SPUV_FUNC_RW_32_CPG_SPUVCKCR	(SPUV_FUNC_CPG_REG_TOP + 0x00000094)
#define SPUV_FUNC_RW_32_CPG_HSICKCR	(SPUV_FUNC_CPG_REG_TOP + 0x0000008C)
#define SPUV_FUNC_RW_32_CPG_MPMODE	(SPUV_FUNC_CPG_REG_TOP + 0x000000CC)
#define SPUV_FUNC_RO_32_CPG_MSTPSR0	(SPUV_FUNC_CPG_REG_TOP + 0x00000030)
#define SPUV_FUNC_RO_32_CPG_MSTPSR1	(SPUV_FUNC_CPG_REG_TOP + 0x00000038)
#define SPUV_FUNC_RO_32_CPG_MSTPSR2	(SPUV_FUNC_CPG_REG_TOP + 0x00000040)
#define SPUV_FUNC_RO_32_CPG_MSTPSR3	(SPUV_FUNC_CPG_REG_TOP + 0x00000048)
#define SPUV_FUNC_RO_32_CPG_MSTPSR4	(SPUV_FUNC_CPG_REG_TOP + 0x0000004C)
#define SPUV_FUNC_RO_32_CPG_MSTPSR5	(SPUV_FUNC_CPG_REG_TOP + 0x0000003C)
#define SPUV_FUNC_RO_32_CPG_MSTPSR6	(SPUV_FUNC_CPG_REG_TOP + 0x000001C0)
#define SPUV_FUNC_RW_32_CPG_RMSTPCR0	(SPUV_FUNC_CPG_REG_TOP + 0x00000110)
#define SPUV_FUNC_RW_32_CPG_RMSTPCR1	(SPUV_FUNC_CPG_REG_TOP + 0x00000114)
#define SPUV_FUNC_RW_32_CPG_RMSTPCR2	(SPUV_FUNC_CPG_REG_TOP + 0x00000118)
#define SPUV_FUNC_RW_32_CPG_RMSTPCR3	(SPUV_FUNC_CPG_REG_TOP + 0x0000011C)
#define SPUV_FUNC_RW_32_CPG_RMSTPCR4	(SPUV_FUNC_CPG_REG_TOP + 0x00000120)
#define SPUV_FUNC_RW_32_CPG_RMSTPCR5	(SPUV_FUNC_CPG_REG_TOP + 0x00000124)
#define SPUV_FUNC_RW_32_CPG_RMSTPCR6	(SPUV_FUNC_CPG_REG_TOP + 0x00000128)
#define SPUV_FUNC_RW_32_CPG_SMSTPCR0	(SPUV_FUNC_CPG_REG_TOP + 0x00000130)
#define SPUV_FUNC_RW_32_CPG_SMSTPCR1	(SPUV_FUNC_CPG_REG_TOP + 0x00000134)
#define SPUV_FUNC_RW_32_CPG_SMSTPCR2	(SPUV_FUNC_CPG_REG_TOP + 0x00000138)
#define SPUV_FUNC_RW_32_CPG_SMSTPCR3	(SPUV_FUNC_CPG_REG_TOP + 0x0000013C)
#define SPUV_FUNC_RW_32_CPG_SMSTPCR4	(SPUV_FUNC_CPG_REG_TOP + 0x00000140)
#define SPUV_FUNC_RW_32_CPG_SMSTPCR5	(SPUV_FUNC_CPG_REG_TOP + 0x00000144)
#define SPUV_FUNC_RW_32_CPG_SMSTPCR6	(SPUV_FUNC_CPG_REG_TOP + 0x00000148)
#define SPUV_FUNC_RW_32_CPG_MMSTPCR0	(SPUV_FUNC_CPG_REG_TOP + 0x00000150)
#define SPUV_FUNC_RW_32_CPG_MMSTPCR1	(SPUV_FUNC_CPG_REG_TOP + 0x00000154)
#define SPUV_FUNC_RW_32_CPG_MMSTPCR2	(SPUV_FUNC_CPG_REG_TOP + 0x00000158)
#define SPUV_FUNC_RW_32_CPG_MMSTPCR3	(SPUV_FUNC_CPG_REG_TOP + 0x0000015C)
#define SPUV_FUNC_RW_32_CPG_MMSTPCR4	(SPUV_FUNC_CPG_REG_TOP + 0x00000160)
#define SPUV_FUNC_RW_32_CPG_MMSTPCR5	(SPUV_FUNC_CPG_REG_TOP + 0x00000164)
#define SPUV_FUNC_RW_32_CPG_MMSTPCR6	(SPUV_FUNC_CPG_REG_TOP + 0x00000168)
#define SPUV_FUNC_RW_32_CPG_SRCR0	(SPUV_FUNC_CPG_REG_TOP + 0x000080A0)
#define SPUV_FUNC_RW_32_CPG_SRCR1	(SPUV_FUNC_CPG_REG_TOP + 0x000080A8)
#define SPUV_FUNC_RW_32_CPG_SRCR2	(SPUV_FUNC_CPG_REG_TOP + 0x000080B0)
#define SPUV_FUNC_RW_32_CPG_SRCR3	(SPUV_FUNC_CPG_REG_TOP + 0x000080B8)
#define SPUV_FUNC_RW_32_CPG_SRCR4	(SPUV_FUNC_CPG_REG_TOP + 0x000080BC)
#define SPUV_FUNC_RW_32_CPG_SRCR5	(SPUV_FUNC_CPG_REG_TOP + 0x000080C4)
#define SPUV_FUNC_RW_32_CPG_SRCR6	(SPUV_FUNC_CPG_REG_TOP + 0x000081C8)
#define SPUV_FUNC_RW_32_CPG_CKSCR	(SPUV_FUNC_CPG_REG_TOP + 0x000000C0)
/* CRMU registers */
#define SPUV_FUNC_CRMU_REG_SIZE		0xC
#define SPUV_FUNC_CRMU_REG_TOP_PHY	0xECBF9800
#define SPUV_FUNC_CRMU_REG_TOP		g_spuv_func_crmu_register_top
#define SPUV_FUNC_RW_32_CRMU_GTU	(SPUV_FUNC_CRMU_REG_TOP + 0x00000000)
#define SPUV_FUNC_RW_32_CRMU_VOICEIF	(SPUV_FUNC_CRMU_REG_TOP + 0x00000004)
#define SPUV_FUNC_RW_32_CRMU_INTCVO	(SPUV_FUNC_CRMU_REG_TOP + 0x00000008)
/* GTU registers */
#define SPUV_FUNC_GTU_REG_SIZE		0x44
#define SPUV_FUNC_GTU_REG_TOP_PHY	0xECBF9C00
#define SPUV_FUNC_GTU_REG_TOP		g_spuv_func_gtu_register_top
#define SPUV_FUNC_RW_32_SEL_20_PLS	(SPUV_FUNC_GTU_REG_TOP + 0x00000000)
#define SPUV_FUNC_RW_32_PG_LOOP		(SPUV_FUNC_GTU_REG_TOP + 0x00000004)
#define SPUV_FUNC_RW_32_PG_RELOAD	(SPUV_FUNC_GTU_REG_TOP + 0x00000008)
#define SPUV_FUNC_RW_32_PG_MIN		(SPUV_FUNC_GTU_REG_TOP + 0x0000000C)
#define SPUV_FUNC_RW_32_PG_MAX		(SPUV_FUNC_GTU_REG_TOP + 0x00000010)
#define SPUV_FUNC_RW_32_PG_20_EN	(SPUV_FUNC_GTU_REG_TOP + 0x00000014)
#define SPUV_FUNC_RO_32_PC_PH_STS	(SPUV_FUNC_GTU_REG_TOP + 0x00000018)
#define SPUV_FUNC_RW_32_PC_DET_EN	(SPUV_FUNC_GTU_REG_TOP + 0x0000001C)
#define SPUV_FUNC_RW_32_PC_MIN_TH	(SPUV_FUNC_GTU_REG_TOP + 0x00000020)
#define SPUV_FUNC_RW_32_PC_MAX_TH	(SPUV_FUNC_GTU_REG_TOP + 0x00000024)
#define SPUV_FUNC_RW_32_EN_TRG		(SPUV_FUNC_GTU_REG_TOP + 0x00000028)
#define SPUV_FUNC_RW_32_DLY_P_UL	(SPUV_FUNC_GTU_REG_TOP + 0x0000002C)
#define SPUV_FUNC_RW_32_DLY_P_DL	(SPUV_FUNC_GTU_REG_TOP + 0x00000030)
#define SPUV_FUNC_RW_32_DLY_B_UL	(SPUV_FUNC_GTU_REG_TOP + 0x00000034)
#define SPUV_FUNC_RW_32_DLY_SP_0	(SPUV_FUNC_GTU_REG_TOP + 0x00000038)
#define SPUV_FUNC_RW_32_DLY_SP_1	(SPUV_FUNC_GTU_REG_TOP + 0x0000003C)
#define SPUV_FUNC_RW_32_PC_RL_VL	(SPUV_FUNC_GTU_REG_TOP + 0x00000040)
/* VOICEIF registers */
#define SPUV_FUNC_VOICEIF_REG_SIZE	0x1C0C
#define SPUV_FUNC_VOICEIF_REG_TOP_PHY	0xECBC0000
#define SPUV_FUNC_VOICEIF_REG_TOP	g_spuv_func_voiceif_register_top
#define SPUV_FUNC_RO_32_UL1_BUF		(SPUV_FUNC_VOICEIF_REG_TOP + 0x00000000)
#define SPUV_FUNC_RO_32_UL2_BUF		(SPUV_FUNC_VOICEIF_REG_TOP + 0x00000600)
#define SPUV_FUNC_RO_32_UL3_BUF		(SPUV_FUNC_VOICEIF_REG_TOP + 0x00000C00)
#define SPUV_FUNC_RO_32_UL4_BUF		(SPUV_FUNC_VOICEIF_REG_TOP + 0x00001200)
#define SPUV_FUNC_RO_32_UL5_BUF		(SPUV_FUNC_VOICEIF_REG_TOP + 0x00001800)
#define SPUV_FUNC_WO_32_DL_BUF		(SPUV_FUNC_VOICEIF_REG_TOP + 0x00003000)
#define SPUV_FUNC_RW_32_UL_EN1		(SPUV_FUNC_VOICEIF_REG_TOP + 0x0003B800)
#define SPUV_FUNC_RW_32_UL_EN2		(SPUV_FUNC_VOICEIF_REG_TOP + 0x0003B804)
#define SPUV_FUNC_RO_32_U1_P_LT		(SPUV_FUNC_VOICEIF_REG_TOP + 0x0003B808)
#define SPUV_FUNC_RO_32_U2_P_LT		(SPUV_FUNC_VOICEIF_REG_TOP + 0x0003B80C)
#define SPUV_FUNC_RW_32_D_BUF_FL	(SPUV_FUNC_VOICEIF_REG_TOP + 0x0003B810)
#define SPUV_FUNC_RO_32_D_P_LT		(SPUV_FUNC_VOICEIF_REG_TOP + 0x0003B814)
#define SPUV_FUNC_RW_32_UL_EN3		(SPUV_FUNC_VOICEIF_REG_TOP + 0x0003B818)
#define SPUV_FUNC_RW_32_UL_EN4		(SPUV_FUNC_VOICEIF_REG_TOP + 0x0003B81C)
#define SPUV_FUNC_RW_32_UL_EN5		(SPUV_FUNC_VOICEIF_REG_TOP + 0x0003B820)
#define SPUV_FUNC_RO_32_U3_P_LT		(SPUV_FUNC_VOICEIF_REG_TOP + 0x0003B824)
#define SPUV_FUNC_RO_32_U4_P_LT		(SPUV_FUNC_VOICEIF_REG_TOP + 0x0003B828)
#define SPUV_FUNC_RO_32_U5_P_LT		(SPUV_FUNC_VOICEIF_REG_TOP + 0x0003B82C)
/* INTCVO registers */
#define SPUV_FUNC_INTCVO_REG_SIZE	0x24
#define SPUV_FUNC_INTCVO_REG_TOP_PHY	0xECBFBA40
#define SPUV_FUNC_INTCVO_REG_TOP	g_spuv_func_intcvo_register_top
#define SPUV_FUNC_RW_32_DINTEN		(SPUV_FUNC_INTCVO_REG_TOP + 0x00000000)
#define SPUV_FUNC_RW_32_DINTMASK	(SPUV_FUNC_INTCVO_REG_TOP + 0x00000004)
#define SPUV_FUNC_RW_32_DINTCLR		(SPUV_FUNC_INTCVO_REG_TOP + 0x00000008)
#define SPUV_FUNC_RO_32_DINTSTS		(SPUV_FUNC_INTCVO_REG_TOP + 0x0000000C)
#define SPUV_FUNC_RW_32_AMSGIT		(SPUV_FUNC_INTCVO_REG_TOP + 0x00000010)
#define SPUV_FUNC_RW_32_AINTEN		(SPUV_FUNC_INTCVO_REG_TOP + 0x00000014)
#define SPUV_FUNC_RW_32_AINTMASK	(SPUV_FUNC_INTCVO_REG_TOP + 0x00000018)
#define SPUV_FUNC_RW_32_AINTCLR		(SPUV_FUNC_INTCVO_REG_TOP + 0x0000001C)
#define SPUV_FUNC_RO_32_AINTSTS		(SPUV_FUNC_INTCVO_REG_TOP + 0x00000020)
#define SPUV_FUNC_RW_32_BBINTSET	(SPUV_FUNC_INTCVO_REG_TOP + 0x00000024)
#define SPUV_FUNC_RW_32_SHINTSET	(SPUV_FUNC_INTCVO_REG_TOP + 0x00000028)
#define SPUV_FUNC_RW_32_V20MSITEN	(SPUV_FUNC_INTCVO_REG_TOP + 0x0000002C)
#define SPUV_FUNC_RW_32_V20MSITCLR	(SPUV_FUNC_INTCVO_REG_TOP + 0x00000030)
#define SPUV_FUNC_RO_32_V20MSITSTAT	(SPUV_FUNC_INTCVO_REG_TOP + 0x00000034)
#define SPUV_FUNC_RW_32_BBITCLR		(SPUV_FUNC_INTCVO_REG_TOP + 0x00000038)
#define SPUV_FUNC_RW_32_SHITCLR		(SPUV_FUNC_INTCVO_REG_TOP + 0x0000003C)
#define SPUV_FUNC_RO_32_BBITSTAT	(SPUV_FUNC_INTCVO_REG_TOP + 0x00000040)
#define SPUV_FUNC_RO_32_SHITSTAT	(SPUV_FUNC_INTCVO_REG_TOP + 0x00000044)
/* SPUV registers */
#define SPUV_FUNC_SPUV_REG_SIZE		0x1C80
#define SPUV_FUNC_SPUV_REG_TOP_PHY	0xECBFE000
#define SPUV_FUNC_SPUV_REG_TOP		g_spuv_func_spuv_register_top
#define SPUV_FUNC_RW_32_CCTL		(SPUV_FUNC_SPUV_REG_TOP + 0x00000004)
#define SPUV_FUNC_RW_32_P0BASE0		(SPUV_FUNC_SPUV_REG_TOP + 0x00000010)
#define SPUV_FUNC_RW_32_X0BASE0		(SPUV_FUNC_SPUV_REG_TOP + 0x00000014)
#define SPUV_FUNC_RW_32_Y0BASE0		(SPUV_FUNC_SPUV_REG_TOP + 0x00000018)
#define SPUV_FUNC_RO_32_SPUMSTS		(SPUV_FUNC_SPUV_REG_TOP + 0x00000028)
#define SPUV_FUNC_RW_32_P0BASE1		(SPUV_FUNC_SPUV_REG_TOP + 0x00000058)
#define SPUV_FUNC_RW_32_X0BASE1		(SPUV_FUNC_SPUV_REG_TOP + 0x0000005C)
#define SPUV_FUNC_RW_32_Y0BASE1		(SPUV_FUNC_SPUV_REG_TOP + 0x00000060)
#define SPUV_FUNC_RW_32_P0BASE2		(SPUV_FUNC_SPUV_REG_TOP + 0x00000070)
#define SPUV_FUNC_RW_32_X0BASE2		(SPUV_FUNC_SPUV_REG_TOP + 0x00000074)
#define SPUV_FUNC_RW_32_Y0BASE2		(SPUV_FUNC_SPUV_REG_TOP + 0x00000078)
#define SPUV_FUNC_RW_32_P0BASE3		(SPUV_FUNC_SPUV_REG_TOP + 0x00000088)
#define SPUV_FUNC_RW_32_X0BASE3		(SPUV_FUNC_SPUV_REG_TOP + 0x0000008C)
#define SPUV_FUNC_RW_32_Y0BASE3		(SPUV_FUNC_SPUV_REG_TOP + 0x00000090)
#define SPUV_FUNC_RW_32_P0BASE4		(SPUV_FUNC_SPUV_REG_TOP + 0x000000A0)
#define SPUV_FUNC_RW_32_X0BASE4		(SPUV_FUNC_SPUV_REG_TOP + 0x000000A4)
#define SPUV_FUNC_RW_32_Y0BASE4		(SPUV_FUNC_SPUV_REG_TOP + 0x000000A8)
#define SPUV_FUNC_RW_32_P0BASE5		(SPUV_FUNC_SPUV_REG_TOP + 0x000000B8)
#define SPUV_FUNC_RW_32_X0BASE5		(SPUV_FUNC_SPUV_REG_TOP + 0x000000BC)
#define SPUV_FUNC_RW_32_Y0BASE5		(SPUV_FUNC_SPUV_REG_TOP + 0x000000C0)
#define SPUV_FUNC_RW_32_P0BASE6		(SPUV_FUNC_SPUV_REG_TOP + 0x000000D0)
#define SPUV_FUNC_RW_32_X0BASE6		(SPUV_FUNC_SPUV_REG_TOP + 0x000000D4)
#define SPUV_FUNC_RW_32_Y0BASE6		(SPUV_FUNC_SPUV_REG_TOP + 0x000000D8)
#define SPUV_FUNC_RW_32_P0BASE7		(SPUV_FUNC_SPUV_REG_TOP + 0x000000E8)
#define SPUV_FUNC_RW_32_X0BASE7		(SPUV_FUNC_SPUV_REG_TOP + 0x000000EC)
#define SPUV_FUNC_RW_32_Y0BASE7		(SPUV_FUNC_SPUV_REG_TOP + 0x000000F0)
#define SPUV_FUNC_RW_32_P0BASE8		(SPUV_FUNC_SPUV_REG_TOP + 0x00000100)
#define SPUV_FUNC_RW_32_X0BASE8		(SPUV_FUNC_SPUV_REG_TOP + 0x00000104)
#define SPUV_FUNC_RW_32_Y0BASE8		(SPUV_FUNC_SPUV_REG_TOP + 0x00000108)
#define SPUV_FUNC_RW_32_P0BASE9		(SPUV_FUNC_SPUV_REG_TOP + 0x00000118)
#define SPUV_FUNC_RW_32_X0BASE9		(SPUV_FUNC_SPUV_REG_TOP + 0x0000011C)
#define SPUV_FUNC_RW_32_Y0BASE9		(SPUV_FUNC_SPUV_REG_TOP + 0x00000120)
#define SPUV_FUNC_RW_32_P0BASE10	(SPUV_FUNC_SPUV_REG_TOP + 0x00000130)
#define SPUV_FUNC_RW_32_X0BASE10	(SPUV_FUNC_SPUV_REG_TOP + 0x00000134)
#define SPUV_FUNC_RW_32_Y0BASE10	(SPUV_FUNC_SPUV_REG_TOP + 0x00000138)
#define SPUV_FUNC_RW_32_P0BASE11	(SPUV_FUNC_SPUV_REG_TOP + 0x00000148)
#define SPUV_FUNC_RW_32_X0BASE11	(SPUV_FUNC_SPUV_REG_TOP + 0x0000014C)
#define SPUV_FUNC_RW_32_Y0BASE11	(SPUV_FUNC_SPUV_REG_TOP + 0x00000150)
#define SPUV_FUNC_RW_32_P0BASE12	(SPUV_FUNC_SPUV_REG_TOP + 0x00000160)
#define SPUV_FUNC_RW_32_X0BASE12	(SPUV_FUNC_SPUV_REG_TOP + 0x00000164)
#define SPUV_FUNC_RW_32_Y0BASE12	(SPUV_FUNC_SPUV_REG_TOP + 0x00000168)
#define SPUV_FUNC_RW_32_P0BASE13	(SPUV_FUNC_SPUV_REG_TOP + 0x00000178)
#define SPUV_FUNC_RW_32_X0BASE13	(SPUV_FUNC_SPUV_REG_TOP + 0x0000017C)
#define SPUV_FUNC_RW_32_Y0BASE13	(SPUV_FUNC_SPUV_REG_TOP + 0x00000180)
#define SPUV_FUNC_RW_32_P0BASE14	(SPUV_FUNC_SPUV_REG_TOP + 0x00000190)
#define SPUV_FUNC_RW_32_X0BASE14	(SPUV_FUNC_SPUV_REG_TOP + 0x00000194)
#define SPUV_FUNC_RW_32_Y0BASE14	(SPUV_FUNC_SPUV_REG_TOP + 0x00000198)
#define SPUV_FUNC_RW_32_P0BASE15	(SPUV_FUNC_SPUV_REG_TOP + 0x000001A8)
#define SPUV_FUNC_RW_32_X0BASE15	(SPUV_FUNC_SPUV_REG_TOP + 0x000001AC)
#define SPUV_FUNC_RW_32_Y0BASE15	(SPUV_FUNC_SPUV_REG_TOP + 0x000001B0)
#define SPUV_FUNC_RW_32_CMOD		(SPUV_FUNC_SPUV_REG_TOP + 0x00000200)
#define SPUV_FUNC_RW_32_SPUSRST		(SPUV_FUNC_SPUV_REG_TOP + 0x00001C24)
#define SPUV_FUNC_RO_32_SPUADR		(SPUV_FUNC_SPUV_REG_TOP + 0x00001C28)
#define SPUV_FUNC_RO_32_ENDIAN		(SPUV_FUNC_SPUV_REG_TOP + 0x00001C2C)
#define SPUV_FUNC_RW_32_GCOM0		(SPUV_FUNC_SPUV_REG_TOP + 0x00001C40)
#define SPUV_FUNC_RW_32_GCOM1		(SPUV_FUNC_SPUV_REG_TOP + 0x00001C44)
#define SPUV_FUNC_RW_32_GCOM2		(SPUV_FUNC_SPUV_REG_TOP + 0x00001C48)
#define SPUV_FUNC_RW_32_GCOM3		(SPUV_FUNC_SPUV_REG_TOP + 0x00001C4C)
#define SPUV_FUNC_RW_32_GCOM4		(SPUV_FUNC_SPUV_REG_TOP + 0x00001C50)
#define SPUV_FUNC_RW_32_GCOM5		(SPUV_FUNC_SPUV_REG_TOP + 0x00001C54)
#define SPUV_FUNC_RW_32_GCOM6		(SPUV_FUNC_SPUV_REG_TOP + 0x00001C58)
#define SPUV_FUNC_RW_32_GCOM7		(SPUV_FUNC_SPUV_REG_TOP + 0x00001C5C)
#define SPUV_FUNC_RW_32_GCLK_CTRL	(SPUV_FUNC_SPUV_REG_TOP + 0x00001C7C)
/* DSP0 registers */
#define SPUV_FUNC_DSP0_REG_SIZE		0x1E0
#define SPUV_FUNC_DSP0_REG_TOP_PHY	0xECBFFD00
#define SPUV_FUNC_DSP0_REG_TOP		g_spuv_func_dsp0_register_top
#define SPUV_FUNC_RW_32_DSPRST		(SPUV_FUNC_DSP0_REG_TOP + 0x00000000)
#define SPUV_FUNC_RW_32_DSPCORERST	(SPUV_FUNC_DSP0_REG_TOP + 0x00000004)
#define SPUV_FUNC_RO_32_DSPHOLD		(SPUV_FUNC_DSP0_REG_TOP + 0x00000008)
#define SPUV_FUNC_RW_32_DSPRESTART	(SPUV_FUNC_DSP0_REG_TOP + 0x0000000C)
#define SPUV_FUNC_RW_32_IEMASKC		(SPUV_FUNC_DSP0_REG_TOP + 0x00000018)
#define SPUV_FUNC_RW_32_IMASKC		(SPUV_FUNC_DSP0_REG_TOP + 0x0000001C)
#define SPUV_FUNC_RW_32_IEVENTC		(SPUV_FUNC_DSP0_REG_TOP + 0x00000020)
#define SPUV_FUNC_OO_32_IEMASKD		(SPUV_FUNC_DSP0_REG_TOP + 0x00000024)
#define SPUV_FUNC_OO_32_IMASKD		(SPUV_FUNC_DSP0_REG_TOP + 0x00000028)
#define SPUV_FUNC_RW_32_IESETD		(SPUV_FUNC_DSP0_REG_TOP + 0x0000002C)
#define SPUV_FUNC_OO_32_IECLRD		(SPUV_FUNC_DSP0_REG_TOP + 0x00000030)
#define SPUV_FUNC_RW_32_OR		(SPUV_FUNC_DSP0_REG_TOP + 0x00000034)
#define SPUV_FUNC_RW_32_COM0		(SPUV_FUNC_DSP0_REG_TOP + 0x00000038)
#define SPUV_FUNC_RW_32_COM1		(SPUV_FUNC_DSP0_REG_TOP + 0x0000003C)
#define SPUV_FUNC_RW_32_COM2		(SPUV_FUNC_DSP0_REG_TOP + 0x00000040)
#define SPUV_FUNC_RW_32_COM3		(SPUV_FUNC_DSP0_REG_TOP + 0x00000044)
#define SPUV_FUNC_RW_32_COM4		(SPUV_FUNC_DSP0_REG_TOP + 0x00000048)
#define SPUV_FUNC_RW_32_COM5		(SPUV_FUNC_DSP0_REG_TOP + 0x0000004C)
#define SPUV_FUNC_RW_32_COM6		(SPUV_FUNC_DSP0_REG_TOP + 0x00000050)
#define SPUV_FUNC_RW_32_COM7		(SPUV_FUNC_DSP0_REG_TOP + 0x00000054)
#define SPUV_FUNC_RW_32_BTADRU		(SPUV_FUNC_DSP0_REG_TOP + 0x00000058)
#define SPUV_FUNC_RW_32_BTADRL		(SPUV_FUNC_DSP0_REG_TOP + 0x0000005C)
#define SPUV_FUNC_RW_32_WDATU		(SPUV_FUNC_DSP0_REG_TOP + 0x00000060)
#define SPUV_FUNC_RW_32_WDATL		(SPUV_FUNC_DSP0_REG_TOP + 0x00000064)
#define SPUV_FUNC_RO_32_RDATU		(SPUV_FUNC_DSP0_REG_TOP + 0x00000068)
#define SPUV_FUNC_RO_32_RDATL		(SPUV_FUNC_DSP0_REG_TOP + 0x0000006C)
#define SPUV_FUNC_RO_32_BTCTRL		(SPUV_FUNC_DSP0_REG_TOP + 0x00000070)
#define SPUV_FUNC_RO_32_SPUSTS		(SPUV_FUNC_DSP0_REG_TOP + 0x00000074)
#define SPUV_FUNC_RW_32_GPI0_INTBIT	(SPUV_FUNC_DSP0_REG_TOP + 0x00000078)
#define SPUV_FUNC_RW_32_GPI1_INTBIT	(SPUV_FUNC_DSP0_REG_TOP + 0x0000007C)
#define SPUV_FUNC_RO_32_GPI0		(SPUV_FUNC_DSP0_REG_TOP + 0x00000080)
#define SPUV_FUNC_RO_32_GPI1		(SPUV_FUNC_DSP0_REG_TOP + 0x00000084)
/* #define SPUV_FUNC_--_--_GO_TO_SLEEP */
#define SPUV_FUNC_RW_32_SBAR0		(SPUV_FUNC_DSP0_REG_TOP + 0x00000100)
#define SPUV_FUNC_RW_32_SAR0		(SPUV_FUNC_DSP0_REG_TOP + 0x00000104)
#define SPUV_FUNC_RW_32_DBAR0		(SPUV_FUNC_DSP0_REG_TOP + 0x00000108)
#define SPUV_FUNC_RW_32_DAR0		(SPUV_FUNC_DSP0_REG_TOP + 0x0000010C)
#define SPUV_FUNC_RW_32_TCR0		(SPUV_FUNC_DSP0_REG_TOP + 0x00000110)
#define SPUV_FUNC_RW_32_SHPRI0		(SPUV_FUNC_DSP0_REG_TOP + 0x00000114)
#define SPUV_FUNC_RW_32_CHCR0		(SPUV_FUNC_DSP0_REG_TOP + 0x00000118)
#define SPUV_FUNC_RW_32_SBAR1		(SPUV_FUNC_DSP0_REG_TOP + 0x00000120)
#define SPUV_FUNC_RW_32_SAR1		(SPUV_FUNC_DSP0_REG_TOP + 0x00000124)
#define SPUV_FUNC_RW_32_DBAR1		(SPUV_FUNC_DSP0_REG_TOP + 0x00000128)
#define SPUV_FUNC_RW_32_DAR1		(SPUV_FUNC_DSP0_REG_TOP + 0x0000012C)
#define SPUV_FUNC_RW_32_TCR1		(SPUV_FUNC_DSP0_REG_TOP + 0x00000130)
#define SPUV_FUNC_RW_32_SHPRI1		(SPUV_FUNC_DSP0_REG_TOP + 0x00000134)
#define SPUV_FUNC_RW_32_CHCR1		(SPUV_FUNC_DSP0_REG_TOP + 0x00000138)
#define SPUV_FUNC_RW_32_SBAR2		(SPUV_FUNC_DSP0_REG_TOP + 0x00000140)
#define SPUV_FUNC_RW_32_SAR2		(SPUV_FUNC_DSP0_REG_TOP + 0x00000144)
#define SPUV_FUNC_RW_32_DBAR2		(SPUV_FUNC_DSP0_REG_TOP + 0x00000148)
#define SPUV_FUNC_RW_32_DAR2		(SPUV_FUNC_DSP0_REG_TOP + 0x0000014C)
#define SPUV_FUNC_RW_32_TCR2		(SPUV_FUNC_DSP0_REG_TOP + 0x00000150)
#define SPUV_FUNC_RW_32_SHPRI2		(SPUV_FUNC_DSP0_REG_TOP + 0x00000154)
#define SPUV_FUNC_RW_32_CHCR2		(SPUV_FUNC_DSP0_REG_TOP + 0x00000158)
#define SPUV_FUNC_RW_32_LSA0		(SPUV_FUNC_DSP0_REG_TOP + 0x00000180)
#define SPUV_FUNC_RW_32_LEA0		(SPUV_FUNC_DSP0_REG_TOP + 0x00000184)
#define SPUV_FUNC_RW_32_LSA1		(SPUV_FUNC_DSP0_REG_TOP + 0x00000190)
#define SPUV_FUNC_RW_32_LEA1		(SPUV_FUNC_DSP0_REG_TOP + 0x00000194)
#define SPUV_FUNC_RW_32_LSA2		(SPUV_FUNC_DSP0_REG_TOP + 0x000001A0)
#define SPUV_FUNC_RW_32_LEA2		(SPUV_FUNC_DSP0_REG_TOP + 0x000001A4)
#define SPUV_FUNC_RW_32_ASID_DSP	(SPUV_FUNC_DSP0_REG_TOP + 0x000001C0)
#define SPUV_FUNC_RW_32_ASID_CPU	(SPUV_FUNC_DSP0_REG_TOP + 0x000001C4)
#define SPUV_FUNC_RW_32_ASID_DMA_CH0	(SPUV_FUNC_DSP0_REG_TOP + 0x000001C8)
#define SPUV_FUNC_RW_32_ASID_DMA_CH1	(SPUV_FUNC_DSP0_REG_TOP + 0x000001CC)
#define SPUV_FUNC_RW_32_ASID_DMA_CH2	(SPUV_FUNC_DSP0_REG_TOP + 0x000001D0)
#define SPUV_FUNC_RW_32_GADDR_CTRL_P	(SPUV_FUNC_DSP0_REG_TOP + 0x000001D4)
#define SPUV_FUNC_RW_32_GADDR_CTRL_X	(SPUV_FUNC_DSP0_REG_TOP + 0x000001D8)
#define SPUV_FUNC_RW_32_GADDR_CTRL_Y	(SPUV_FUNC_DSP0_REG_TOP + 0x000001DC)

/* pm runtime message log */
#define SPUV_FUNC_PM_RUNTIME_GET_LOG		\
		"[ -> PM   ] PM_RUNTIME_GET\n"
#define SPUV_FUNC_PM_RUNTIME_PUT_LOG		\
		"[ -> PM   ] PM_RUNTIME_PUT\n"

/*
 * define macro declaration
 */

/* sdram address change (logical -> physical) */
#define vcd_spuv_func_sdram_logical_to_physical(logical, ret) \
{ \
	if ((logical >= g_spuv_func_sdram_static_non_cache_area_top) && \
		(logical < (g_spuv_func_sdram_static_non_cache_area_top \
		+ SPUV_FUNC_SDRAM_NON_CACHE_AREA_SIZE))) { \
		/* non cache address */ \
		ret = (SPUV_FUNC_SDRAM_AREA_TOP_PHY + \
			(logical - \
			g_spuv_func_sdram_static_non_cache_area_top)); \
	} else { \
		/* cache address */ \
		ret = ((SPUV_FUNC_SDRAM_AREA_TOP_PHY + \
			SPUV_FUNC_SDRAM_NON_CACHE_AREA_SIZE) + \
			(logical - \
			g_spuv_func_sdram_static_cache_area_top)); \
	} \
}

/* ceiling macro */
#define vcd_spuv_func_ceiling128(x)	((x + 127) & (~127))

/* register control macro */
#define vcd_spuv_func_register(reg, ret) \
		{ \
			unsigned long flags; \
			flags = pm_get_spinlock(); \
			if (g_spuv_func_is_spuv_clk) \
				ret = ioread32(IOMEM(reg)); \
			pm_release_spinlock(flags); \
		}
#define vcd_spuv_func_set_register(set_bit, reg) \
		{ \
			unsigned long flags; \
			flags = pm_get_spinlock(); \
			if (g_spuv_func_is_spuv_clk) \
				iowrite32(set_bit, IOMEM(reg)); \
			pm_release_spinlock(flags); \
		}
#include <mach/common.h>
#define vcd_spuv_func_modify_register(clear_bit, set_bit, reg) \
		{ \
			unsigned long flags; \
			flags = pm_get_spinlock(); \
			if (g_spuv_func_is_spuv_clk) \
				sh_modify_register32( \
					IOMEM(reg), clear_bit, set_bit); \
			pm_release_spinlock(flags); \
		}

/*
 * enum declaration
 */
enum VCD_SPUV_FUNC_GLOBAL_AREA_SIZE {
	VCD_SPUV_FUNC_GLOBAL_AREA_SIZE_1KW	= (4 * 1024),
	VCD_SPUV_FUNC_GLOBAL_AREA_SIZE_2KW	= (8 * 1024),
	VCD_SPUV_FUNC_GLOBAL_AREA_SIZE_4KW	= (16 * 1024),
	VCD_SPUV_FUNC_GLOBAL_AREA_SIZE_8KW	= (32 * 1024),
	VCD_SPUV_FUNC_GLOBAL_AREA_SIZE_16KW	= (64 * 1024),
	VCD_SPUV_FUNC_GLOBAL_AREA_SIZE_32KW	= (128 * 1024),
	VCD_SPUV_FUNC_GLOBAL_AREA_SIZE_64KW	= (256 * 1024),
};

enum VCD_SPUV_FUNC_SAMPLING_RATE {
	VCD_SPUV_FUNC_SAMPLING_RATE_8KHZ = 0,
	VCD_SPUV_FUNC_SAMPLING_RATE_11KHZ,
	VCD_SPUV_FUNC_SAMPLING_RATE_12KHZ,
	VCD_SPUV_FUNC_SAMPLING_RATE_16KHZ,
	VCD_SPUV_FUNC_SAMPLING_RATE_22KHZ,
	VCD_SPUV_FUNC_SAMPLING_RATE_24KHZ,
	VCD_SPUV_FUNC_SAMPLING_RATE_32KHZ,
	VCD_SPUV_FUNC_SAMPLING_RATE_44KHZ,
	VCD_SPUV_FUNC_SAMPLING_RATE_48KHZ,
	VCD_SPUV_FUNC_SAMPLING_RATE_MAX,
};

enum VCD_SPUV_FUNC_RESAMPLE_TABLE {
	VCD_SPUV_FUNC_RESAMPLE_TABLE_SAMPLING_RATE = 0,
	VCD_SPUV_FUNC_RESAMPLE_TABLE_BUFFER_SIZE,
	VCD_SPUV_FUNC_RESAMPLE_TABLE_MAX,
};

/*
 * structure declaration
 */

/* spuv firmware infomation */
struct vcd_spuv_func_read_fw_info {
	unsigned int *pram_addr[VCD_SPUV_FUNC_PAGE_SIZE];
	unsigned int *xram_addr[VCD_SPUV_FUNC_PAGE_SIZE];
	unsigned int *yram_addr[VCD_SPUV_FUNC_PAGE_SIZE];

	unsigned int pram_global_size;
	unsigned int xram_global_size;
	unsigned int yram_global_size;

	unsigned int pram_page_size[VCD_SPUV_FUNC_PAGE_SIZE];
	unsigned int xram_page_size[VCD_SPUV_FUNC_PAGE_SIZE];
	unsigned int yram_page_size[VCD_SPUV_FUNC_PAGE_SIZE];
};

struct vcd_spuv_func_fw_info {
	void *base_addr[VCD_SPUV_FUNC_PAGE_SIZE];
	void *reg_addr[VCD_SPUV_FUNC_PAGE_SIZE];
	phys_addr_t reg_addr_physcal[VCD_SPUV_FUNC_PAGE_SIZE];
	unsigned int page_size[VCD_SPUV_FUNC_PAGE_SIZE];
	unsigned int global_size;
};


/*
 * table declaration
 */


/*
 * extern declaration
 */
extern bool g_spuv_func_is_spuv_clk;

extern phys_addr_t g_spuv_func_sdram_static_area_top_phy;
extern void *g_spuv_func_sdram_static_non_cache_area_top;
extern void *g_spuv_func_sdram_static_cache_area_top;
extern void __iomem *g_spuv_func_hpb_register_top;
extern void __iomem *g_spuv_func_cpg_register_top;
extern void __iomem *g_spuv_func_crmu_register_top;
extern void __iomem *g_spuv_func_gtu_register_top;
extern void __iomem *g_spuv_func_voiceif_register_top;
extern void __iomem *g_spuv_func_intcvo_register_top;
extern void __iomem *g_spuv_func_spuv_register_top;
extern void __iomem *g_spuv_func_dsp0_register_top;

/*
 * prototype declaration
 */
/* Internal public functions */
extern void vcd_spuv_func_cacheflush_sdram(const void *logical_addr,
	unsigned int size);
extern void vcd_spuv_func_cacheflush(phys_addr_t physical_addr,
	const void *logical_addr, unsigned int size);
extern void vcd_spuv_func_ipc_semaphore_init(void);
extern int vcd_spuv_func_control_power_supply(int effective);
extern int vcd_spuv_func_check_power_supply(void);
extern int vcd_spuv_func_set_fw(void);
extern void vcd_spuv_func_release_firmware(void);
extern void vcd_spuv_func_dsp_core_reset(void);

extern void vcd_spuv_func_send_msg(int *param, int length);
extern void vcd_spuv_func_send_ack(int is_log_enable);
extern void vcd_spuv_func_get_fw_request(void);

extern void vcd_spuv_func_set_hpb_register(void);
extern void vcd_spuv_func_set_cpg_register(void);

extern void *vcd_spuv_func_get_spuv_static_buffer(void);
extern void *vcd_spuv_func_get_pcm_static_buffer(void);
extern void *vcd_spuv_func_get_diamond_sdram_buffer(void);

/* Synchronous conversion functions */
extern void vcd_spuv_func_start_wait(void);
extern void vcd_spuv_func_end_wait(void);

/* Memory functions */
extern int vcd_spuv_func_ioremap(void);
extern void vcd_spuv_func_iounmap(void);
extern int vcd_spuv_func_get_fw_buffer(void);
extern void vcd_spuv_func_free_fw_buffer(void);

/* SRC functions */
extern int vcd_spuv_func_resampler_init
(int alsa_rate, int spuv_rate);
extern int vcd_spuv_func_resampler_set
(int alsa_ul_rate, int alsa_dl_rate, int spuv_rate);
extern int vcd_spuv_func_resampler_close(void);

extern void vcd_spuv_func_pt_playback(void);

extern void vcd_spuv_func_voip_ul(unsigned int *buf_size);
extern void vcd_spuv_func_voip_dl(unsigned int *buf_size);

extern void vcd_spuv_func_init_playback_buffer_id(void);
extern void vcd_spuv_func_init_record_buffer_id(void);

extern void vcd_spuv_func_init_voip_ul_buffer_id(void);
extern void vcd_spuv_func_init_voip_dl_buffer_id(void);

extern void vcd_spuv_func_set_plaback_buffer_id(void);
extern void vcd_spuv_func_set_record_buffer_id(void);

extern void vcd_spuv_func_set_voip_ul_buffer_id(void);
extern void vcd_spuv_func_set_voip_dl_buffer_id(void);
/* Register dump functions */
extern void vcd_spuv_func_dump_hpb_registers(void);
extern void vcd_spuv_func_dump_cpg_registers(void);
extern void vcd_spuv_func_dump_crmu_registers(void);
extern void vcd_spuv_func_dump_gtu_registers(void);
extern void vcd_spuv_func_dump_voiceif_registers(void);
extern void vcd_spuv_func_dump_intcvo_registers(void);
extern void vcd_spuv_func_dump_spuv_registers(void);
extern void vcd_spuv_func_dump_dsp0_registers(void);
extern void vcd_spuv_func_dump_pram0_memory(void);
extern void vcd_spuv_func_dump_xram0_memory(void);
extern void vcd_spuv_func_dump_yram0_memory(void);
extern void vcd_spuv_func_dump_dspio_memory(void);
extern void vcd_spuv_func_dump_sdram_static_area_memory(void);
extern void vcd_spuv_func_dump_fw_static_buffer_memory(void);
extern void vcd_spuv_func_dump_spuv_crashlog(void);
extern void vcd_spuv_func_dump_diamond_memory(void);

#endif /* __VCD_SPUV_FUNC_H__ */