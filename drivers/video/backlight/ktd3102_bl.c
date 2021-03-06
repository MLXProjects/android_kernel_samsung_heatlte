/*
 * linux/drivers/video/backlight/ktd_bl.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*******************************************************************************
* Copyright 2013 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/video/backlight/ktd_bl.c
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/regulator/consumer.h>
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/backlight.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/ktd_bl.h>
#include <linux/spinlock.h>
#include <linux/rtc.h>
#include <linux/lcd.h>
#include <linux/of_gpio.h>
#include <linux/of_device.h>
int current_intensity;
static int backlight_pin;
//static int backlight_pwm = 144;

static DEFINE_SPINLOCK(bl_ctrl_lock);

int real_level = 21;
EXPORT_SYMBOL(real_level);

static int backlight_mode=1;

#define CABC_FEATURE_ON 0
#define LCD_REGISTER 0

#define MAX_BRIGHTNESS_IN_BLU	33

#define DIMMING_VALUE		32

#define MAX_BRIGHTNESS_VALUE	255
#define MIN_BRIGHTNESS_VALUE	20

#define BACKLIGHT_SUSPEND 0
#define BACKLIGHT_RESUME 1
#define BACKLIGHT_DEBUG 1
#if BACKLIGHT_DEBUG
#define BLDBG(fmt, args...) printk(fmt, ## args)
#else
#define BLDBG(fmt, args...)
#endif

struct brt_value{
	int level;				/* Platform setting values */
	int tune_level;	      /* Chip Setting values */
};

struct ktd_bl_data {
	struct platform_device *pdev;
	unsigned int ctrl_pin;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend_desc;
#endif
};

struct lcd_info {
	struct device			*dev;
	struct spi_device		*spi;
	unsigned int			power;
	unsigned int			gamma_mode;
	unsigned int			current_gamma_mode;
	unsigned int			current_bl;
      unsigned int			bl;
#if CABC_FEATURE_ON      
	unsigned int			auto_brightness;
#endif	
	unsigned int			ldi_enable;
      unsigned int			acl_enable;
      unsigned int			cur_acl;
	struct mutex			lock;
	struct mutex			bl_lock;
	struct lcd_device		*ld;
	struct backlight_device		*bd;
	struct lcd_platform_data	*lcd_pd;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend		early_suspend;
#endif
#ifdef SMART_DIMMING
	unsigned char			id[3];
	struct str_smart_dim		smart;
#endif
};

struct brt_value brt_table_ktd[] = {
	{ MIN_BRIGHTNESS_VALUE,  29 },  /* Min pulse 32 */
	{ 27,  29 },
	{ 39,  28 },
	{ 51,  27 },
	{ 63,  26 },
	{ 75,  25 },
	{ 87,  24 },
	{ 99,  23 },
	{ 111,  22 },
	{ 123,  21 },  /* Default �� */
	{ 135,  20 },
	{ 147,  19 },
	{ 159,  18 },
	{ 171,  17 },
	{ 183,  16 },
	{ 195,  15 },
	{ 207,  14 },
	{ 220,  13 },
	{ 230,  12 },
	{ 235,  11 },
	{ 240,  10 },
	{ MAX_BRIGHTNESS_VALUE,  9 },
};


#define MAX_BRT_STAGE_KTD (int)(sizeof(brt_table_ktd)/sizeof(struct brt_value))

static void lcd_backlight_control(int num)
{
    volatile int limit;
    unsigned long flags;

    limit = num;

    spin_lock_irqsave(&bl_ctrl_lock, flags);

    for(;limit>0;limit--)
    {
       udelay(2);
       gpio_set_value(backlight_pin,0);
       udelay(2); 
       gpio_set_value(backlight_pin,1);
    }

   spin_unlock_irqrestore(&bl_ctrl_lock, flags);
}

/* input: intensity in percentage 0% - 100% */
static int ktd_backlight_update_status(struct backlight_device *bd)
{
    int user_intensity = bd->props.brightness;
    int tune_level = 0;
    int pulse;
    int i;

    BLDBG("[BACKLIGHT] update_status ==> user_intensity  : %d\n", user_intensity);

#if CABC_FEATURE_ON
    g_bl.props.brightness = user_intensity;
#endif    
    current_intensity = user_intensity;

	if (bd->props.power != FB_BLANK_UNBLANK)
		user_intensity = 0;

	if (bd->props.fb_blank != FB_BLANK_UNBLANK)
		user_intensity = 0;

	if (bd->props.state & BL_CORE_SUSPENDED)
		user_intensity = 0;

    if(backlight_mode!=BACKLIGHT_RESUME){
		BLDBG("[BACKLIGHT] suspend mode %d\n", backlight_mode);
		return 0;
    }
       
    if(user_intensity > 0) {
        if(user_intensity < MIN_BRIGHTNESS_VALUE) {
            tune_level = DIMMING_VALUE; /* DIMMING */
        }else if (user_intensity == MAX_BRIGHTNESS_VALUE) {
#if CABC_FEATURE_ON
             if(cabc_status){
               tune_level = brt_table_ktd_cabc[MAX_BRT_STAGE_KTD_CABC-1].tune_level;
            }else
#endif
            {
                tune_level = brt_table_ktd[MAX_BRT_STAGE_KTD-1].tune_level;
            }
        }else{
#if CABC_FEATURE_ON
            if(cabc_status){

               BLDBG("[BACKLIGHT] cabc ON!\n");
               for(i = 0; i < MAX_BRT_STAGE_KTD_CABC; i++) {
                   if(user_intensity <= brt_table_ktd_cabc[i].level ) {
                      tune_level = brt_table_ktd_cabc[i].tune_level;
                       break;
                    }
               }

            }else
#endif                
            {
             //   BLDBG("[BACKLIGHT] cabc OFF!\n");
                for(i = 0; i < MAX_BRT_STAGE_KTD; i++) {
                    if(user_intensity <= brt_table_ktd[i].level ) {
                        tune_level = brt_table_ktd[i].tune_level;
                        break;
                    }
                }
           }
        }
    }

    if (real_level==tune_level){
        return 0;
    }else{          
        if(tune_level<=0){
            gpio_set_value(backlight_pin,0);
            mdelay(3); 

      }else{
            if( real_level<=tune_level){
                pulse = tune_level - real_level;
            }else{
                pulse = 32 - (real_level - tune_level);
            }

            if (pulse==0){
                return 0;
            }

            BLDBG("[BACKLIGHT] update_status ==> tune_level : %d & pulse = %d\n", tune_level, pulse);

            lcd_backlight_control(pulse); 
        }

        real_level = tune_level;
        return 0;
    }

      
    return 0;
}


#if CABC_FEATURE_ON
void backlight_update_CABC()
{
   BLDBG("[BACKLIGHT] backlight_update_CABC\n");
   ktd_backlight_update_status(&g_bl);
}
EXPORT_SYMBOL(backlight_update_CABC);

extern void backlight_cabc_on(void);
extern void backlight_cabc_off(void);
#endif

static int ktd_backlight_get_brightness(struct backlight_device *bl)
{
    BLDBG("[BACKLIGHT] backlight_get_brightness\n");
    
    return current_intensity;
}

static struct backlight_ops ktd_backlight_ops = {
    .update_status	= ktd_backlight_update_status,
    .get_brightness	= ktd_backlight_get_brightness,
};

#ifdef CONFIG_HAS_EARLYSUSPEND
static void ktd_backlight_earlysuspend(struct early_suspend *desc)
{
    struct timespec ts;
    struct rtc_time tm;
 
    backlight_mode=BACKLIGHT_SUSPEND; 
    
    getnstimeofday(&ts);
    rtc_time_to_tm(ts.tv_sec, &tm);
    gpio_set_value(backlight_pin,0);

    real_level = 0;

    printk("[%02d:%02d:%02d.%03lu][BACKLIGHT] earlysuspend\n", tm.tm_hour, tm.tm_min, tm.tm_sec, ts.tv_nsec);
}

static void ktd_backlight_lateresume(struct early_suspend *desc)
{
    struct ktd_bl_data *ktd = container_of(desc, struct ktd_bl_data,
                                                                          early_suspend_desc);
    struct backlight_device *bl = platform_get_drvdata(ktd->pdev);
    struct timespec ts;
    struct rtc_time tm;
    
    
    getnstimeofday(&ts);
    rtc_time_to_tm(ts.tv_sec, &tm);
    backlight_mode=BACKLIGHT_RESUME;

#if CABC_FEATURE_ON
     if(cabc_status)
     {
        backlight_cabc_on();
     }
     else
     {
        backlight_cabc_off();
     }
#endif    
    printk("[%02d:%02d:%02d.%03lu][BACKLIGHT] late resume\n", tm.tm_hour, tm.tm_min, tm.tm_sec, ts.tv_nsec);
    backlight_update_status(bl);
}
#else
#ifdef CONFIG_PM
static int ktd_backlight_suspend(struct platform_device *pdev,
					pm_message_t state)
{
    struct backlight_device *bl = platform_get_drvdata(pdev);
    struct ktd_bl_data *ktd = dev_get_drvdata(&bl->dev);
    
    BLDBG("[BACKLIGHT] backlight_suspend\n");
        
    return 0;
}

static int ktd_backlight_resume(struct platform_device *pdev)
{
    struct backlight_device *bl = platform_get_drvdata(pdev);

    BLDBG("[BACKLIGHT] backlight_resume\n");
        
    backlight_update_status(bl);
        
    return 0;
}
#else
#define ktd_backlight_suspend  NULL
#define ktd_backlight_resume   NULL
#endif
#endif

#if CABC_FEATURE_ON
static ssize_t auto_brightness_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	char temp[3];

	sprintf(temp, "%d\n", lcd->auto_brightness);
	strcpy(buf, temp);

	return strlen(buf);
}

static ssize_t auto_brightness_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	int value;
	int rc;

	printk("auto_brightness_store called\n");

	rc = strict_strtoul(buf, (unsigned int)0, (unsigned long *)&value);
	if (rc < 0)
		return rc;
	else {
		if (lcd->auto_brightness != value) {
			dev_info(dev, "%s - %d, %d\n", __func__, lcd->auto_brightness, value);
			lcd->auto_brightness = value;

			if(lcd->auto_brightness == 0)
			{
                cabc_status = 0;
                backlight_cabc_off();
			}
			else if(lcd->auto_brightness >= 1 && lcd->auto_brightness < 5)
			{
                cabc_status = 1;
                backlight_cabc_on();
			}
			else if(lcd->auto_brightness >= 5)
			{
                cabc_status = 0;
                backlight_cabc_off();
			}
            
           
		}
	}
	return size; 
}

static DEVICE_ATTR(auto_brightness, 0644, auto_brightness_show, auto_brightness_store);
#endif

#if LCD_REGISTER
static int pulse_value;

static ssize_t brightness_pulse_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	char temp[3];

	sprintf(temp, "%d\n", pulse_value);
	strcpy(buf, temp);

	return strlen(buf);
}

static ssize_t brightness_pulse_setting(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);

	int rc;

	rc = strict_strtoul(buf, (unsigned int)0, (unsigned long *)&pulse_value);
	
	if (rc < 0)
		return rc;
	else {

		if(pulse_value <= 32 && pulse_value >= 1)	{
			printk("brightness_pulse_setting, pulse value=%d\n", pulse_value );

			gpio_set_value(backlight_pin,0);
			mdelay(3);

			lcd_backlight_control(pulse_value); 
		}
		else
			printk("out of range !!\n");

	}
	return size; 
}

static DEVICE_ATTR(manual_pulseset, 0644, brightness_pulse_show, brightness_pulse_setting);
#endif

static int get_platform_data(struct platform_device *pdev,
	struct platform_ktd_backlight_data *pdata)
{
	unsigned int val;
	if (of_property_read_u32(pdev->dev.of_node, "max-brightness", &val)) {
		dev_err(&pdev->dev, "max_brightness not getting from dts\n");
		return -1;
	}
	pdata->max_brightness = val;
	if (of_property_read_u32(pdev->dev.of_node,
			"default-brightness", &val)) {
		dev_err(&pdev->dev, "dft_brightness not getting from dts\n");
		return -1;
	}
	pdata->dft_brightness = val;
	pdata->ctrl_pin = of_get_gpio(pdev->dev.of_node, 0);
	pdev->dev.platform_data = pdata;
	return 0;
}

static const struct of_device_id ktd3102_of_match[] = {
	{.compatible = "kinetic,ktd3102",},
	{},
};

MODULE_DEVICE_TABLE(of, ktd3102_of_match);

static int ktd_backlight_probe(struct platform_device *pdev)
{
	struct platform_ktd_backlight_data *data = NULL;
	struct backlight_device *bl;
	struct ktd_bl_data *ktd;
    	struct backlight_properties props;
	int ret;

        BLDBG("[BACKLIGHT] backlight driver probe\n");
	if (pdev->dev.of_node) {
		dev_info(&pdev->dev, "Device Tree\n");
		data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
		if (!data) {
			dev_err(&pdev->dev, "no memory for state\n");
			return -ENOMEM;
		}
		if (get_platform_data(pdev, data)) {
			dev_err(&pdev->dev, "DTS_ktd:of_get_platform_data error\n");
			return -EINVAL;
		}
	} else {
		dev_info(&pdev->dev, "No Device Tree\n");
		data = pdev->dev.platform_data;
	}

	ktd = devm_kzalloc(&pdev->dev, sizeof(*ktd), GFP_KERNEL);
	if (!ktd) {
		dev_err(&pdev->dev, "no memory for state\n");
		return -ENOMEM;
	}

	backlight_pin = data->ctrl_pin;
	ret = devm_gpio_request_one(&pdev->dev, backlight_pin,
			GPIOF_OUT_INIT_HIGH, "Backlight");
	if (ret) {
		dev_err(&pdev->dev, "failed to get GPIO %d\n", backlight_pin);
		return ret;
	}

	memset(&props, 0, sizeof(struct backlight_properties));
	props.max_brightness = data->max_brightness;
	props.type = BACKLIGHT_PLATFORM;

	bl = backlight_device_register("panel", &pdev->dev,
			ktd, &ktd_backlight_ops, &props);
	if (IS_ERR(bl)) {
		dev_err(&pdev->dev, "failed to register backlight\n");
		return PTR_ERR(bl);
	}
	
#if CABC_FEATURE_ON
    lcd_device_register("panel", &pdev->dev, NULL, NULL);

    ret = device_create_file(&bl->dev, &dev_attr_auto_brightness);
	if (ret < 0)
	dev_err(&pdev->dev, "failed to add sysfs entries\n");
#endif
#if LCD_REGISTER
    lcd_device_register("panel", &pdev->dev, NULL, NULL);

    ret = device_create_file(&bl->dev, &dev_attr_manual_pulseset);
	if (ret < 0)
	dev_err(&pdev->dev, "failed to add sysfs entries\n");
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
	ktd->pdev = pdev;
	ktd->early_suspend_desc.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
	ktd->early_suspend_desc.suspend = ktd_backlight_earlysuspend;
	ktd->early_suspend_desc.resume = ktd_backlight_lateresume;
	register_early_suspend(&ktd->early_suspend_desc);
#endif

	bl->props.max_brightness = data->max_brightness;
	bl->props.brightness = data->dft_brightness;

	platform_set_drvdata(pdev, bl);

	return 0;
}

static int ktd_backlight_remove(struct platform_device *pdev)
{
    struct backlight_device *bl = platform_get_drvdata(pdev);
    struct ktd_bl_data *ktd = dev_get_drvdata(&bl->dev);

    backlight_device_unregister(bl);


#ifdef CONFIG_HAS_EARLYSUSPEND
    unregister_early_suspend(&ktd->early_suspend_desc);
#endif

    return 0;
}

static void ktd_backlight_shutdown(struct platform_device *pdev)
{
    printk("[BACKLIGHT] ktd_backlight_shutdown\n");
    gpio_set_value(backlight_pin,0);
    mdelay(3);

}


static struct platform_driver ktd_backlight_driver = {
	.driver		= {
		.name	= "ktd3102",
		.owner	= THIS_MODULE,
		.of_match_table =  ktd3102_of_match,
	},
	.probe		= ktd_backlight_probe,
	.remove		= ktd_backlight_remove,
	.shutdown       = ktd_backlight_shutdown,

#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend        = ktd_backlight_suspend,
	.resume         = ktd_backlight_resume,
#endif

};

module_platform_driver(ktd_backlight_driver);

MODULE_DESCRIPTION("ktd based Backlight Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:ktd-backlight");

