/* setup-u2audio.c
 *
 * Copyright (C) 2013 Renesas Mobile Corp.
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

#include <linux/proc_fs.h>
#include <linux/gpio.h>
#include <mach/gpio.h>
#include <linux/memblock.h>
#include <linux/bug.h>
#include <linux/seq_file.h>
#if defined(CONFIG_ARCH_R8A7373)
#include <mach/r8a7373.h>
#endif
#include <mach/setup-u2audio.h>
#ifdef CONFIG_MFD_D2153
#include <linux/d2153/core.h>
#include <linux/d2153/d2153_codec.h>
#include <linux/d2153/audio.h>
#ifdef CONFIG_SND_SOC_D2153_AAD
#include <linux/d2153/d2153_aad.h>
#endif	/* CONFIG_SND_SOC_D2153_AAD */
#endif	/* CONFIG_MFD_D2153 */

/* Proc read handler */
static int proc_read_u2audio_device(struct seq_file *file, void *v)
{
	int device_exist_p = d2153_pdata.audio.fm34_device ? DEVICE_EXIST :
							     DEVICE_NONE;

	return seq_printf(file, "%d", device_exist_p);
}

static int proc_open_u2audio(struct inode *inode, struct file *file)
{
	return single_open(file, proc_read_u2audio_device, NULL);
}

static const struct file_operations proc_u2audio_ops = {
	.open	= proc_open_u2audio,
	.read	= seq_read,
};

void u2audio_gpio_init(void)
{
#if defined(CONFIG_ARCH_R8A7373)
	gpio_request(GPIO_FN_FSIAISLD, "sound");
	gpio_request(GPIO_FN_FSIAOBT, "sound");
	gpio_request(GPIO_FN_FSIAOLR, "sound");
	gpio_request(GPIO_FN_FSIAOSLD, "sound");

	gpio_pull_off_port(GPIO_PORT263);
	gpio_pull_off_port(GPIO_PORT264);
	gpio_pull_off_port(GPIO_PORT265);

	gpio_request(GPIO_FN_FSIBISLD, "sound");
	gpio_request(GPIO_FN_FSIBOBT, "sound");
	gpio_request(GPIO_FN_FSIBOLR, "sound");
	gpio_request(GPIO_FN_FSIBOSLD, "sound");
#endif /* CONFIG_ARCH_R8A7373 */
}

#ifndef D2153_DEFAULT_SET_MICBIAS
void u2audio_codec_micbias_level_init(void)
{
	/* set headset mic bias */
	d2153_pdata.audio.micbias1_level = D2153_MICBIAS_LEVEL_2_6V;
	/* set sub mic bias */
	d2153_pdata.audio.micbias2_level = D2153_MICBIAS_LEVEL_2_1V;
	/* set main mic bias */
	d2153_pdata.audio.micbias3_level = D2153_MICBIAS_LEVEL_2_1V;
}
#endif	/* D2153_DEFAULT_SET_MICBIAS */

#ifdef CONFIG_SND_SOC_D2153_AAD
void u2audio_codec_aad_init(void)
{
	int res;

	d2153_pdata.audio.aad_jack_debounce_ms = d2153_pdata.audio.debounce_ms;
	d2153_pdata.audio.aad_jackout_debounce_ms =
						D2153_AAD_JACKOUT_DEBOUNCE_MS;
	d2153_pdata.audio.aad_button_debounce_ms =
						D2153_AAD_BUTTON_DEBOUNCE_MS;
	d2153_pdata.audio.aad_button_sleep_debounce_ms =
					D2153_AAD_BUTTON_SLEEP_DEBOUNCE_MS;
	d2153_pdata.audio.aad_gpio_detect_enable = true;
	d2153_pdata.audio.aad_gpio_port = GPIO_PORT7;

	/* GPIO Interrupt for G-DET */
	res = gpio_request(d2153_pdata.audio.aad_gpio_port,
			(d2153_pdata.audio.aad_codec_detect_enable ?
				"GPIO detect" : "Jack detect"));
	if (res < 0)
		printk(KERN_ERR "%s: gpio request failed[%d]\n",
				__func__, res);

	res = gpio_direction_input(d2153_pdata.audio.aad_gpio_port);
	if (res < 0)
		printk(KERN_ERR "%s: gpio direction input failed[%d]\n",
				__func__, res);

	gpio_pull_off_port(d2153_pdata.audio.aad_gpio_port);

	if (d2153_pdata.audio.aad_codec_detect_enable) {
		res = gpio_set_debounce(d2153_pdata.audio.aad_gpio_port,
					D2153_GPIO_DEBOUNCE_TIME_LONG);
		if (res < 0)
			printk(KERN_ERR "%s: gpio set debounce failed[%d]\n",
					__func__, res);
	} else {
		res = gpio_set_debounce(d2153_pdata.audio.aad_gpio_port,
					D2153_GPIO_DEBOUNCE_TIME_SHORT);
		if (res < 0)
			printk(KERN_ERR "%s: gpio set debounce failed[%d]\n",
					__func__, res);
	}
}
#endif	/* CONFIG_SND_SOC_D2153_AAD */

void u2audio_init(void)
{
	struct proc_dir_entry *root_audio;
	struct proc_dir_entry *root_device;

	u2audio_gpio_init();

	root_audio = proc_mkdir("audio", NULL);
	if (NULL != root_audio) {
		/* Create device entries */
		root_device = proc_mkdir("device", root_audio);
		if (NULL != root_device)
			proc_create("fm34", S_IRUGO, root_device,
				    &proc_u2audio_ops);
	} else {
		printk(KERN_ERR "%s Failed proc_mkdir\n", __func__);
	}

#ifndef D2153_DEFAULT_SET_MICBIAS
	u2audio_codec_micbias_level_init();
#endif	/* D2153_DEFAULT_SET_MICBIAS */

#ifdef CONFIG_SND_SOC_D2153_AAD
	u2audio_codec_aad_init();
#endif	/* CONFIG_SND_SOC_D2153_AAD */

	return;
}


/*
 * u2vcd_reserve - allocate memory for vocoder (size & alignment come from the struct)
 */
void u2vcd_reserve(void)
{
	int ret;

	/* At least in MP523x VOCODER has to be in the 1st 256 megabytes - it can' be beyond that */

	ret = memblock_remove(SDRAM_VOCODER_START_ADDR, (SDRAM_VOCODER_END_ADDR-SDRAM_VOCODER_START_ADDR)+1);
	pr_alert("u2vcd_reserve %d - VOCODER memblock allocation: 0x%x-0x%x\n", ret,
			SDRAM_VOCODER_START_ADDR,
			SDRAM_VOCODER_END_ADDR);
	BUG_ON(ret<0);
}



