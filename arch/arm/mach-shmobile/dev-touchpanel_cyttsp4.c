#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <mach/irqs.h>
#include <linux/interrupt.h>
#include <mach/r8a7373.h>
#include <mach/gpio.h>
//#include "board-8930.h"

/* cyttsp */
#include <linux/cyttsp4_bus.h>
#include <linux/cyttsp4_core.h>
#include <linux/cyttsp4_btn.h>
#include <linux/cyttsp4_mt.h>
#include <linux/delay.h>
#include <linux/regulator/consumer.h>


#define CONFIG_TOUCHSCREEN_CYPRESS_CYTTSP4_INCLUDE_FW
#ifndef SAMSUNG_SYSINFO_DATA
#define SAMSUNG_SYSINFO_DATA
#endif

#if defined(CONFIG_TOUCHSCREEN_CYPRESS_CYTTSP4_INCLUDE_FW) && defined(SAMSUNG_SYSINFO_DATA)

#if defined(CONFIG_MACH_AFYONLTE)

#include "mach/afyonlte_cyttsp4_img.h"
static struct cyttsp4_touch_firmware cyttsp4_firmware[] = {
	// CYTTSP4_SILICON_REV_D
	{
		.img = cyttsp4_img,
		.size = ARRAY_SIZE(cyttsp4_img),
		.hw_version = 0x05,
		.fw_version = 0x0300,
		.config_version = 0x13,
	},

	// CYTTSP4_SILICON_REV_B
	{
		.img = cyttsp4_img,
		.size = ARRAY_SIZE(cyttsp4_img),
		.hw_version = 0x05,
		.fw_version = 0x0300,
		.config_version = 0x13,
	}
};

#else//defined(CONFIG_MACH_AFYONLTE)

#include "mach/HEAT_LTE_HW02_FW05.h"
#include "mach/HEAT_LTE_HW01_FW02.h"
static struct cyttsp4_touch_firmware cyttsp4_firmware[] = {
	// CYTTSP4_SILICON_REV_D
	{
		.img = cyttsp4_img_revD,
		.size = ARRAY_SIZE(cyttsp4_img_revD),
		.hw_version = 0x02,
		.fw_version = 0x0500,
		.config_version = 0x03,
	},

	// CYTTSP4_SILICON_REV_B
	{
		.img = cyttsp4_img_revB,
		.size = ARRAY_SIZE(cyttsp4_img_revB),
		.hw_version = 0x01,
		.fw_version = 0x0200,
		.config_version = 0x02,
	}
};

#endif//defined(CONFIG_MACH_AFYONLTE)

#else//defined(CONFIG_TOUCHSCREEN_CYPRESS_CYTTSP4_INCLUDE_FW) && defined(SAMSUNG_SYSINFO_DATA)
static struct cyttsp4_touch_firmware cyttsp4_firmware = {
	.img = NULL,
	.size = 0,
	.ver = NULL,
	.vsize = 0,
};
#endif//defined(CONFIG_TOUCHSCREEN_CYPRESS_CYTTSP4_INCLUDE_FW) && defined(SAMSUNG_SYSINFO_DATA)

#define CYTTSP4_USE_I2C

extern unsigned int system_rev;

#ifdef CYTTSP4_USE_I2C
#define CYTTSP4_I2C_NAME "cyttsp4_i2c_adapter"
#define CYTTSP4_I2C_TCH_ADR 0x24
#define CYTTSP4_LDR_TCH_ADR 0x24

#define CYTTSP4_I2C_IRQ_GPIO 32
#if defined(CONFIG_MACH_HEATLTE) || defined(CONFIG_MACH_VIVALTOLTE)
#define CYTTSP4_ENABLE_GPIO 26//21
#define CYTTSP4_ENABLE_GPIO_REV00 21
#endif
//#define CYTTSP4_I2C_RST_GPIO 14
#endif

#if defined(CONFIG_MACH_AFYONLTE)
#define CY_MAXX 540
#define CY_MAXY 960
#else //defined(CONFIG_MACH_LOGANLTE) for heat lte
#define CY_MAXX 480
#define CY_MAXY 800
#endif
#define CY_MINX 0
#define CY_MINY 0

#define CY_ABS_MIN_X CY_MINX
#define CY_ABS_MIN_Y CY_MINY
#define CY_ABS_MAX_X CY_MAXX
#define CY_ABS_MAX_Y CY_MAXY
#define CY_ABS_MIN_P 0
#define CY_ABS_MAX_P 255
#define CY_ABS_MIN_W 0
#define CY_ABS_MAX_W 255

#define CY_ABS_MIN_T 0

#define CY_ABS_MAX_T 15

#define CY_IGNORE_VALUE 0xFFFF

#define  NLSX4373_EN_GPIO    14

#define TSP_DEBUG_OPTION 0
int touch_debug_option = TSP_DEBUG_OPTION;

#define	CAPRI_TSP_DEBUG(fmt, args...)	\
	if (touch_debug_option)	\
		printk("[TSP %s:%4d] " fmt, \
		__func__, __LINE__, ## args);

//static int verg_en=0;

static int cyttsp4_hw_power(int on, int use_irq, int irq_gpio)
{
	int ret = 0;

	static struct regulator *reg_l31;

	pr_info("[TSP] power %s\n", on ? "on" : "off");

	if (!reg_l31) {
		reg_l31 = regulator_get(NULL, "vtsp_3v");
		if (IS_ERR(reg_l31)) {
			pr_err("%s: could not get 8917_l31, rc = %ld\n",
				__func__, PTR_ERR(reg_l31));
			return -1;
		}
		ret = regulator_set_voltage(reg_l31, 2850000, 2850000);
		if (ret) {
			pr_err("%s: unable to set l31 voltage to 3.3V\n",
				__func__);
			return -1;
		}
	}

	if (on) {
		ret = regulator_enable(reg_l31);
		if (ret) {
			pr_err("%s: enable l31 failed, rc=%d\n",
				__func__, ret);
			return -1;
		}
		pr_info("%s: tsp 3.3V on is finished.\n", __func__);

		/* Delay for tsp chip is ready for i2c before enable irq */
		msleep(20);

             #if defined(CONFIG_MACH_HEATLTE)		
		if(system_rev>=3)
		gpio_direction_output(CYTTSP4_ENABLE_GPIO, 1);
		else
		gpio_direction_output(CYTTSP4_ENABLE_GPIO_REV00, 1);	
             #endif		
		msleep(50);	
	} else {
             #if defined(CONFIG_MACH_HEATLTE)
		if(system_rev>=3)
		gpio_direction_output(CYTTSP4_ENABLE_GPIO, 0);
		else
		gpio_direction_output(CYTTSP4_ENABLE_GPIO_REV00, 0);		
             #endif
	        /* Delay for 20 msec */
	        msleep(20);

		if (regulator_is_enabled(reg_l31))
			ret = regulator_disable(reg_l31);
		else
			printk(KERN_ERR
				"%s: rugulator is(L31(3.3V) disabled\n",
					__func__);
		if (ret) {
			pr_err("%s: disable l31 failed, rc=%d\n",
				__func__, ret);
			return -1;
		}
		pr_info("%s: tsp 3.3V off is finished.\n", __func__);


        /* Delay for 100 msec */
        msleep(100);
	}

	return 0;
}



static int cyttsp4_xres(struct cyttsp4_core_platform_data *pdata,
		struct device *dev)
{
    int irq_gpio = pdata->irq_gpio;
	int rc = 0;

	cyttsp4_hw_power(0, true, irq_gpio);

	cyttsp4_hw_power(1, true, irq_gpio);

	return rc;
}

static int cyttsp4_init(struct cyttsp4_core_platform_data *pdata,
		int on, struct device *dev)
{
	int rc = 0;

	if (on) {
		cyttsp4_hw_power(1, false, 0);
	} else {
		cyttsp4_hw_power(0, false, 0);
	}

	return rc;
}

static int cyttsp4_wakeup(struct cyttsp4_core_platform_data *pdata,
		struct device *dev, atomic_t *ignore_irq)
{
	int irq_gpio = pdata->irq_gpio;

	return cyttsp4_hw_power(1, true, irq_gpio);
}

static int cyttsp4_sleep(struct cyttsp4_core_platform_data *pdata,
		struct device *dev, atomic_t *ignore_irq)
{
    int irq_gpio = pdata->irq_gpio;

	return cyttsp4_hw_power(0, true, irq_gpio);
}

static int cyttsp4_power(struct cyttsp4_core_platform_data *pdata,
		int on, struct device *dev, atomic_t *ignore_irq)
{
	if (on)
		return cyttsp4_wakeup(pdata, dev, ignore_irq);

	return cyttsp4_sleep(pdata, dev, ignore_irq);
}

static int cyttsp4_irq_stat(struct cyttsp4_core_platform_data *pdata,
		struct device *dev)
{

	return gpio_get_value(pdata->irq_gpio);
}

static struct regulator *keyled_regulator;
static int touchkey_regulator_cnt;

static int cyttsp4_led_power_onoff(int on)
{
	int ret;

	if (keyled_regulator == NULL) {
		printk(KERN_INFO" %s, %d\n", __func__, __LINE__);
		keyled_regulator = regulator_get(NULL, "key_led");
		if (IS_ERR(keyled_regulator)) {
			printk(KERN_DEBUG"can not get KEY_LED_3.3V\n");
			return -1;
		}
	ret = regulator_set_voltage(keyled_regulator, 3300000, 3300000);
	printk(KERN_INFO"regulator_set_voltage ret = %d\n", ret);

	}

	if (on) {
		printk(KERN_INFO"Touchkey On\n");
		ret = regulator_enable(keyled_regulator);
		if (ret) {
			pr_err("can not enable KEY_LED_3.3V, ret=%d\n", ret);
		} else {
			touchkey_regulator_cnt++;
			printk(KERN_DEBUG"regulator_enable ret = %d, cnt=%d\n",
				ret, touchkey_regulator_cnt);
		}

	} else {
		printk(KERN_INFO"Touchkey Off\n");
		ret = regulator_disable(keyled_regulator);
		if (ret) {
			pr_err("can not disabled KEY_LED_3.3V ret=%d\n", ret);
		} else {
			touchkey_regulator_cnt--;
			printk(KERN_DEBUG"regulator_disable ret= %d, cnt=%d\n",
				ret, touchkey_regulator_cnt);
		}

	}
	return 0;
}

#ifdef BUTTON_PALM_REJECTION
static u16 cyttsp4_btn_keys[] = {
	/* use this table to map buttons to keycodes (see input.h) */
	KEY_MENU,		/* 139 */
	KEY_MENU,		/* 139 */
	KEY_BACK,		/* 158 */
	KEY_BACK,		/* 158 */
};
#else
/* Button to keycode conversion */
static u16 cyttsp4_btn_keys[] = {
	/* use this table to map buttons to keycodes (see input.h) */
	KEY_MENU,		/* 139 */
	KEY_BACK,		/* 158 */
};
#endif	// BUTTON_PALM_REJECTION

static struct touch_settings cyttsp4_sett_btn_keys = {
	.data = (uint8_t *)&cyttsp4_btn_keys[0],
	.size = ARRAY_SIZE(cyttsp4_btn_keys),
	.tag = 0,
};

static struct cyttsp4_core_platform_data _cyttsp4_core_platform_data = {
	.irq_gpio = CYTTSP4_I2C_IRQ_GPIO,
//	.rst_gpio = CYTTSP4_I2C_RST_GPIO,
	.xres = cyttsp4_xres,
	.init = cyttsp4_init,
	.power = cyttsp4_power,
	.irq_stat = cyttsp4_irq_stat,
	.led_power = cyttsp4_led_power_onoff,
	.sett = {
		NULL,	/* Reserved */
		NULL,	/* Command Registers */
		NULL,	/* Touch Report */
		NULL,	/* Cypress Data Record */
		NULL,	/* Test Record */
		NULL,	/* Panel Configuration Record */
		NULL, /* &cyttsp4_sett_param_regs, */
		NULL, /* &cyttsp4_sett_param_size, */
		NULL,	/* Reserved */
		NULL,	/* Reserved */
		NULL,	/* Operational Configuration Record */
		NULL, /* &cyttsp4_sett_ddata, *//* Design Data Record */
		NULL, /* &cyttsp4_sett_mdata, *//* Manufacturing Data Record */
		NULL,	/* Config and Test Registers */
		&cyttsp4_sett_btn_keys,	/* button-to-keycode table */
	},
	.fw = cyttsp4_firmware,
};

/*
static struct cyttsp4_core cyttsp4_core_device = {
	.name = CYTTSP4_CORE_NAME,
	.id = "main_ttsp_core",
	.adap_id = CYTTSP4_I2C_NAME,
	.dev = {
		.platform_data = &_cyttsp4_core_platform_data,
	},
};
*/

static struct cyttsp4_core_info cyttsp4_core_info __initdata= {
	.name = CYTTSP4_CORE_NAME,
	.id = "main_ttsp_core",
	.adap_id = CYTTSP4_I2C_NAME,
       .platform_data = &_cyttsp4_core_platform_data,
};




static const uint16_t cyttsp4_abs[] = {
	ABS_MT_POSITION_X, CY_ABS_MIN_X, CY_ABS_MAX_X, 0, 0,
	ABS_MT_POSITION_Y, CY_ABS_MIN_Y, CY_ABS_MAX_Y, 0, 0,
	ABS_MT_PRESSURE, CY_ABS_MIN_P, CY_ABS_MAX_P, 0, 0,
	CY_IGNORE_VALUE, CY_ABS_MIN_W, CY_ABS_MAX_W, 0, 0,
	ABS_MT_TRACKING_ID, CY_ABS_MIN_T, CY_ABS_MAX_T, 0, 0,
	ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0,
	ABS_MT_TOUCH_MINOR, 0, 255, 0, 0,
	ABS_MT_ORIENTATION, -128, 127, 0, 0,
};

struct touch_framework cyttsp4_framework = {
	.abs = (uint16_t *)&cyttsp4_abs[0],
	.size = ARRAY_SIZE(cyttsp4_abs),
	.enable_vkeys = 0,
};

static struct cyttsp4_mt_platform_data _cyttsp4_mt_platform_data = {
	.frmwrk = &cyttsp4_framework,
	.flags = 0x00,
	.inp_dev_name = CYTTSP4_MT_NAME,
};

/*
struct cyttsp4_device cyttsp4_mt_device = {
	.name = CYTTSP4_MT_NAME,
	.core_id = "main_ttsp_core",
	.dev = {
		.platform_data = &_cyttsp4_mt_platform_data,
	}
};
*/

struct cyttsp4_device_info cyttsp4_mt_info __initdata = {
	.name = CYTTSP4_MT_NAME,
	.core_id = "main_ttsp_core",
      .platform_data = &_cyttsp4_mt_platform_data,
};

static struct cyttsp4_btn_platform_data _cyttsp4_btn_platform_data = {
	.inp_dev_name = CYTTSP4_BTN_NAME,
};

/*
struct cyttsp4_device cyttsp4_btn_device = {
	.name = CYTTSP4_BTN_NAME,
	.core_id = "main_ttsp_core",
	.dev = {
		.platform_data = &_cyttsp4_btn_platform_data,
	}
};
*/

struct cyttsp4_device_info cyttsp4_btn_info __initdata = {
	.name = CYTTSP4_BTN_NAME,
	.core_id = "main_ttsp_core",
      .platform_data = &_cyttsp4_btn_platform_data,
};

#ifdef CYTTSP4_VIRTUAL_KEYS
static ssize_t cyttps4_virtualkeys_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf,
		__stringify(EV_KEY) ":"
		__stringify(KEY_BACK) ":1360:90:160:180"
		":" __stringify(EV_KEY) ":"
		__stringify(KEY_MENU) ":1360:270:160:180"
		":" __stringify(EV_KEY) ":"
		__stringify(KEY_HOME) ":1360:450:160:180"
		":" __stringify(EV_KEY) ":"
		__stringify(KEY_SEARCH) ":1360:630:160:180"
		"\n");
}

static struct kobj_attribute cyttsp4_virtualkeys_attr = {
	.attr = {
		.name = "virtualkeys.cyttsp4_mt",
		.mode = S_IRUGO,
	},
	.show = &cyttps4_virtualkeys_show,
};

static struct attribute *cyttsp4_properties_attrs[] = {
	&cyttsp4_virtualkeys_attr.attr,
	NULL
};

static struct attribute_group cyttsp4_properties_attr_group = {
	.attrs = cyttsp4_properties_attrs,
};
#endif

/* I2C 2 */
static struct i2c_board_info touch_i2c_devices[] = {
	{
		I2C_BOARD_INFO(CYTTSP4_I2C_NAME, CYTTSP4_I2C_TCH_ADR),
	        .irq = irq_pin(CYTTSP4_I2C_IRQ_GPIO ),
	},
};

void __init board_tsp_init(void)
{
	//int ret = 0;

	printk("[TSP] board_tsp_init + \n");

	gpio_request(CYTTSP4_I2C_IRQ_GPIO, NULL);
	gpio_direction_input(CYTTSP4_I2C_IRQ_GPIO);
	//gpio_pull_up_port(CYTTSP4_I2C_IRQ_GPIO);
#if defined(CONFIG_MACH_HEATLTE) || defined(CONFIG_MACH_VIVALTOLTE)
    #if defined(CONFIG_MACH_HEATLTE)
	if(system_rev>=3){
	msleep(100);
	gpio_request(CYTTSP4_ENABLE_GPIO, NULL);
	gpio_direction_output(CYTTSP4_ENABLE_GPIO, 1);
	gpio_pull_up_port(CYTTSP4_ENABLE_GPIO);
	}
	else{
	gpio_request(CYTTSP4_ENABLE_GPIO_REV00, NULL);
	gpio_direction_output(CYTTSP4_ENABLE_GPIO_REV00, 1);
	gpio_pull_up_port(CYTTSP4_ENABLE_GPIO_REV00);		
	}
    #else	
	gpio_request(CYTTSP4_ENABLE_GPIO_REV00, NULL);
	gpio_direction_output(CYTTSP4_ENABLE_GPIO_REV00, 1);
	gpio_pull_up_port(CYTTSP4_ENABLE_GPIO_REV00);
    #endif
#endif
	i2c_register_board_info(4, touch_i2c_devices,
                ARRAY_SIZE(touch_i2c_devices));

	cyttsp4_register_core_device(&cyttsp4_core_info);
	cyttsp4_register_device(&cyttsp4_mt_info);
#if 1	//defined(CONFIG_TOUCHSCREEN_CYPRESS_TMA46X_SUPPORT_BUTTON)
	cyttsp4_register_device(&cyttsp4_btn_info);
#endif
	printk("[TSP] board_tsp_init - \n");
}
