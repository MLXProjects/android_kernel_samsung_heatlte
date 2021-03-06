/*
 * Driver for Samsung S5K4ECGX  Camera
 *
 * Copyright (C) 2012-2013 Renesas Mobile Corp.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/* for debug */
/* #undef DEBUG */
#define DEBUG 1
/* #define DEBUG */

#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/v4l2-mediabus.h>
#include <linux/module.h>
#include <linux/videodev2.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/d2153/core.h>

#include <media/soc_camera.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-chip-ident.h>
#include <media/v4l2-ctrls.h>
#include <media/sh_mobile_csi2.h>
#include <media/sh_mobile_rcu.h>

#include <mach/r8a7373.h>
int flash_check;
/* CAM0 Power function */
int S5K4ECGX_power(struct device *dev, int power_on)
{
	struct clk *vclk1_clk, *vclk2_clk;
	int iRet;
	struct regulator *regulator;
	dev_dbg(dev, "%s(): power_on=%d\n", __func__, power_on);

	vclk1_clk = clk_get(NULL, "vclk1_clk");
	if (IS_ERR(vclk1_clk)) {
		dev_err(dev, "clk_get(vclk1_clk) failed\n");
		return -1;
	}

	vclk2_clk = clk_get(NULL, "vclk2_clk");
	if (IS_ERR(vclk2_clk)) {
		dev_err(dev, "clk_get(vclk2_clk) failed\n");
		return -1;
	}

	if (power_on) {
		printk(KERN_ALERT "%s PowerON\n", __func__);
		sh_csi2_power(dev, power_on);
		gpio_set_value(GPIO_PORT3, 0); /* CAM_PWR_EN Low */
		gpio_set_value(GPIO_PORT16, 0); /* CAM1_RST_N */
		gpio_set_value(GPIO_PORT91, 0); /* CAM1_STBY */
		gpio_set_value(GPIO_PORT20, 0); /* CAM0_RST_N */
		gpio_set_value(GPIO_PORT45, 0); /* CAM0_STBY */

		/* CAM_AVDD_2V8  On */
		regulator = regulator_get(NULL, "cam_sensor_a");
		if (IS_ERR(regulator))
			return -1;
		if (regulator_enable(regulator))
			dev_warn(dev, "Could not enable regulator\n");

		regulator_put(regulator);

		mdelay(2);

		/* CAM_VDDIO_1V8 On */
		regulator = regulator_get(NULL, "cam_sensor_io");
		if (IS_ERR(regulator))
			return -1;
		if (regulator_enable(regulator))
			dev_warn(dev, "Could not enable regulator\n");
		regulator_put(regulator);

		mdelay(2);

		gpio_set_value(GPIO_PORT91, 1); /* CAM1_STBY */
		mdelay(2);

		iRet = clk_set_rate(vclk1_clk,
			clk_round_rate(vclk1_clk, 24000000));
		if (0 != iRet) {
			dev_err(dev,
				"clk_set_rate(vclk1_clk) failed (ret=%d)\n",
				iRet);
		}

		iRet = clk_enable(vclk1_clk);
		if (0 != iRet) {
			dev_err(dev, "clk_enable(vclk1_clk) failed (ret=%d)\n",
				iRet);
		}
		mdelay(3);

		gpio_set_value(GPIO_PORT16, 1); /* CAM1_RST_N */
		mdelay(2);

		gpio_set_value(GPIO_PORT91, 0); /* CAM1_STBY */
		mdelay(2);

		/* CAM_CORE_1V2  On */
		gpio_set_value(GPIO_PORT3, 1);
		mdelay(1);

		gpio_set_value(GPIO_PORT45, 1); /* CAM0_STBY */
		mdelay(1);

		gpio_set_value(GPIO_PORT20, 1); /* CAM0_RST_N Hi */
		udelay(70);
		/* 1ms */

		/* 5M_AF_2V8 On */
		regulator = regulator_get(NULL, "cam_af");
		if (IS_ERR(regulator))
			return -1;
		if (regulator_enable(regulator))
			dev_warn(dev, "Could not enable regulator\n");
		regulator_put(regulator);

		printk(KERN_ALERT "%s PowerON fin\n", __func__);
	} else {
		printk(KERN_ALERT "%s PowerOFF\n", __func__);

		gpio_set_value(GPIO_PORT20, 0); /* CAM0_RST_N */
		mdelay(1);

		clk_disable(vclk1_clk);

		gpio_set_value(GPIO_PORT45, 0); /* CAM0_STBY */
		mdelay(1);

		gpio_set_value(GPIO_PORT16, 0); /* CAM1_RST_N */
		mdelay(1);

		/* CAM_CORE_1V2  Off */
		gpio_set_value(GPIO_PORT3, 0);
		mdelay(1);

		/* CAM_VDDIO_1V8 Off */
		regulator = regulator_get(NULL, "cam_sensor_io");
		if (IS_ERR(regulator))
			return -1;
		regulator_disable(regulator);
		regulator_put(regulator);
		mdelay(1);

		/* CAM_AVDD_2V8  Off */
		regulator = regulator_get(NULL, "cam_sensor_a");
		if (IS_ERR(regulator))
			return -1;
		regulator_disable(regulator);
		regulator_put(regulator);
		mdelay(1);

		/* 5M_AF_2V8 Off */
		regulator = regulator_get(NULL, "cam_af");
		if (IS_ERR(regulator))
			return -1;
		regulator_disable(regulator);
		regulator_put(regulator);
		sh_csi2_power(dev, power_on);
		printk(KERN_ALERT "%s PowerOFF fin\n", __func__);
	}

	clk_put(vclk1_clk);
	clk_put(vclk2_clk);

	return 0;
}

static int s5k4ecgx_s_power(struct v4l2_subdev *sd, int on)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);
	int ret;
	if (on) {
		ret = soc_camera_power_on(&client->dev, ssdd);
		if (ret < 0)
			return ret;
	} else{
		ret = soc_camera_power_off(&client->dev, ssdd);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static ssize_t maincamtype_S5K4ECGX_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	char *sensorname = "S5K4ECGX";
	return sprintf(buf, "%s\n", sensorname);
}

static ssize_t maincamfw_S5K4ECGX_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	char *sensorfw = "S5K4ECGX N";
	return sprintf(buf, "%s\n", sensorfw);
}

static ssize_t maincamflash_S5K4ECGX_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	if ((0 >= count) || !buf)
		return 0;
	if (buf[0] == '0') {
		sh_mobile_rcu_flash(0);
		flash_check = 0;
	} else {
		sh_mobile_rcu_flash(1);
		flash_check = 1;
	}
	return count;
}

static ssize_t mainvendorid_S5K4ECGX_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	char *vendorid = "0x0302";
	return sprintf(buf, "%s\n", vendorid);
}

static DEVICE_ATTR(rear_camtype, S_IRUGO, maincamtype_S5K4ECGX_show, NULL);
static DEVICE_ATTR(rear_camfw, S_IRUGO, maincamfw_S5K4ECGX_show, NULL);
static DEVICE_ATTR(rear_flash, 0640, NULL, maincamflash_S5K4ECGX_store);
static DEVICE_ATTR(rear_vendorid, S_IRUGO, mainvendorid_S5K4ECGX_show, NULL);

struct S5K4ECGX_datafmt {
	enum v4l2_mbus_pixelcode	code;
	enum v4l2_colorspace		colorspace;
};

struct S5K4ECGX {
	struct v4l2_subdev		subdev;
	struct v4l2_ctrl_handler hdl;
	const struct S5K4ECGX_datafmt	*fmt;
	unsigned int			width;
	unsigned int			height;
};

static const struct S5K4ECGX_datafmt S5K4ECGX_colour_fmts[] = {
	{V4L2_MBUS_FMT_SBGGR10_1X10,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_SGBRG10_1X10,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_SGRBG10_1X10,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_SRGGB10_1X10,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_UYVY8_2X8,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_VYUY8_2X8,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_YUYV8_2X8,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_YVYU8_2X8,	V4L2_COLORSPACE_SRGB},
};

static struct S5K4ECGX *to_S5K4ECGX(const struct i2c_client *client)
{
	return container_of(i2c_get_clientdata(client),
					struct S5K4ECGX, subdev);
}

/* Find a data format by a pixel code in an array */
static const struct S5K4ECGX_datafmt *S5K4ECGX_find_datafmt(
					enum v4l2_mbus_pixelcode code)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(S5K4ECGX_colour_fmts); i++)
		if (S5K4ECGX_colour_fmts[i].code == code)
			return S5K4ECGX_colour_fmts + i;

	return NULL;
}

/* select nearest higher resolution for capture */
static void S5K4ECGX_res_roundup(u32 *width, u32 *height)
{
	int i;
	int res_x[] = { 640, 1280, 1280, 2560 };
	int res_y[] = { 480, 720, 960, 1920 };

	for (i = 0; i < ARRAY_SIZE(res_x); i++) {
		if (res_x[i] >= *width && res_y[i] >= *height) {
			*width = res_x[i];
			*height = res_y[i];
			return;
		}
	}

	*width = res_x[3];
	*height = res_y[3];
}

static int S5K4ECGX_try_fmt(struct v4l2_subdev *sd,
	       struct v4l2_mbus_framefmt *mf)
{
	const struct S5K4ECGX_datafmt *fmt = S5K4ECGX_find_datafmt(mf->code);

	dev_dbg(sd->v4l2_dev->dev, "%s(%u)\n", __func__, mf->code);

	if (!fmt) {
		mf->code	= S5K4ECGX_colour_fmts[0].code;
		mf->colorspace	= S5K4ECGX_colour_fmts[0].colorspace;
	}

	dev_dbg(sd->v4l2_dev->dev, "in: mf->width = %d, height = %d\n",
		mf->width, mf->height);
	S5K4ECGX_res_roundup(&mf->width, &mf->height);
	dev_dbg(sd->v4l2_dev->dev, "out: mf->width = %d, height = %d\n",
		mf->width, mf->height);
	mf->field	= V4L2_FIELD_NONE;

	return 0;
}

static int S5K4ECGX_s_fmt(struct v4l2_subdev *sd,
	     struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct S5K4ECGX *priv = to_S5K4ECGX(client);

	dev_dbg(sd->v4l2_dev->dev, "%s(%u)\n", __func__, mf->code);

	/* MIPI CSI could have changed the format, double-check */
	if (!S5K4ECGX_find_datafmt(mf->code)) {
		dev_err(sd->v4l2_dev->dev, "%s -EINVAL\n", __func__);
		return -EINVAL;
	}

	S5K4ECGX_try_fmt(sd, mf);

	priv->fmt	= S5K4ECGX_find_datafmt(mf->code);
	priv->width	= mf->width;
	priv->height	= mf->height;

	return 0;
}

static int S5K4ECGX_g_fmt(struct v4l2_subdev *sd,
	     struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct S5K4ECGX *priv = to_S5K4ECGX(client);

	const struct S5K4ECGX_datafmt *fmt = priv->fmt;

	mf->code	= fmt->code;
	mf->colorspace	= fmt->colorspace;
	mf->width	= priv->width;
	mf->height	= priv->height;
	mf->field	= V4L2_FIELD_NONE;

	return 0;
}

static int S5K4ECGX_g_crop(struct v4l2_subdev *sd, struct v4l2_crop *a)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct S5K4ECGX *priv = to_S5K4ECGX(client);
	struct v4l2_rect *rect = &a->c;

	a->type		= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	rect->top	= 0;
	rect->left	= 0;
	rect->width	= priv->width;
	rect->height	= priv->height;

	return 0;
}

static int S5K4ECGX_cropcap(struct v4l2_subdev *sd, struct v4l2_cropcap *a)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct S5K4ECGX *priv = to_S5K4ECGX(client);

	a->bounds.left			= 0;
	a->bounds.top			= 0;
	a->bounds.width			= priv->width;
	a->bounds.height		= priv->height;
	dev_dbg(&client->dev, "crop: width = %d, height = %d\n",
		a->bounds.width, a->bounds.height);
	a->defrect			= a->bounds;
	a->type				= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	a->pixelaspect.numerator	= 1;
	a->pixelaspect.denominator	= 1;

	return 0;
}

static int S5K4ECGX_enum_fmt(struct v4l2_subdev *sd, unsigned int index,
		enum v4l2_mbus_pixelcode *code)
{
	if ((unsigned int)index >= ARRAY_SIZE(S5K4ECGX_colour_fmts))
		return -EINVAL;

	*code = S5K4ECGX_colour_fmts[index].code;
	return 0;
}

static int S5K4ECGX_g_chip_ident(struct v4l2_subdev *sd,
		    struct v4l2_dbg_chip_ident *id)
{
	/* check i2c device */
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct i2c_msg msg[2];
	unsigned char send_buf[2];
	unsigned char rcv_buf[2];
	int loop = 0;
	int ret = 0;

	msg[0].addr = client->addr;
	msg[0].flags = client->flags & I2C_M_TEN;
	msg[0].len = 2;
	msg[0].buf = (char *) send_buf;
	/* FW Sensor ID Support */
	send_buf[0] = 0x01;
	send_buf[1] = 0x5A;

	msg[1].addr = client->addr;
	msg[1].flags = client->flags & I2C_M_TEN;
	msg[1].flags |= I2C_M_RD;
	msg[1].len = 2;
	msg[1].buf = rcv_buf;

	for (loop = 0; loop < 5; loop++) {
		ret = i2c_transfer(client->adapter, msg, 2);
		if (0 <= ret)
			break;
	}
	if (0 > ret) {
		dev_err(&client->dev, "%s :Read Error(%d)\n", __func__, ret);
		id->ident = V4L2_IDENT_NONE;
	} else {
		dev_dbg(&client->dev, "%s :Read OK\n", __func__);
		id->ident = V4L2_IDENT_S5K4ECGX;
	}
	id->revision = 0;

	return 0;
}

/* Request bus settings on camera side */
static int S5K4ECGX_g_mbus_config(struct v4l2_subdev *sd,
				struct v4l2_mbus_config *cfg)
{
	cfg->type = V4L2_MBUS_CSI2;
	cfg->flags = V4L2_MBUS_CSI2_2_LANE |
		V4L2_MBUS_CSI2_CHANNEL_0 |
		V4L2_MBUS_CSI2_CONTINUOUS_CLOCK;

	return 0;
}

/* Alter bus settings on camera side */
static int S5K4ECGX_s_mbus_config(struct v4l2_subdev *sd,
				const struct v4l2_mbus_config *cfg)
{
	return 0;
}

static int S5K4ECGX_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	switch (ctrl->id) {
	case V4L2_CID_GET_TUNING:
#ifdef CONFIG_SOC_CAMERA_S5K4ECGX_TUNING
		ctrl->value = 1;
#else
		ctrl->value = 0;
#endif
		/* no break */
	default:
		return 0;
	}
	return -ENOIOCTLCMD;
}

static struct v4l2_subdev_video_ops S5K4ECGX_subdev_video_ops = {
	.s_mbus_fmt	= S5K4ECGX_s_fmt,
	.g_mbus_fmt	= S5K4ECGX_g_fmt,
	.try_mbus_fmt	= S5K4ECGX_try_fmt,
	.enum_mbus_fmt	= S5K4ECGX_enum_fmt,
	.g_crop		= S5K4ECGX_g_crop,
	.cropcap	= S5K4ECGX_cropcap,
	.g_mbus_config	= S5K4ECGX_g_mbus_config,
	.s_mbus_config	= S5K4ECGX_s_mbus_config,
};

static struct v4l2_subdev_core_ops S5K4ECGX_subdev_core_ops = {
	.s_power	= s5k4ecgx_s_power,
	.g_chip_ident	= S5K4ECGX_g_chip_ident,
	.g_ctrl		= S5K4ECGX_g_ctrl,
};

static struct v4l2_subdev_ops S5K4ECGX_subdev_ops = {
	.core	= &S5K4ECGX_subdev_core_ops,
	.video	= &S5K4ECGX_subdev_video_ops,
};

static int S5K4ECGX_s_ctrl(struct v4l2_ctrl *ctrl)
{
	return 0;
}

static struct v4l2_ctrl_ops S5K4ECGX_ctrl_ops = {
	.s_ctrl = S5K4ECGX_s_ctrl,
};

static int S5K4ECGX_probe(struct i2c_client *client,
			const struct i2c_device_id *did)
{
	struct S5K4ECGX *priv;
	struct soc_camera_subdev_desc *sdesc = soc_camera_i2c_to_desc(client);
	int ret = 0;

	dev_dbg(&client->dev, "%s():\n", __func__);

	if (!sdesc) {
		dev_err(&client->dev, "S5K4ECGX: missing platform data!\n");
		return -EINVAL;
	}

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		dev_err(&client->dev,
			"S5K4ECGX: Failed to allocate memory for private data!\n");
		return -ENOMEM;
	}

	v4l2_i2c_subdev_init(&priv->subdev, client, &S5K4ECGX_subdev_ops);
	v4l2_ctrl_handler_init(&priv->hdl, 4);
	v4l2_ctrl_new_std(&priv->hdl, &S5K4ECGX_ctrl_ops,
			V4L2_CID_VFLIP, 0, 1, 1, 0);
	v4l2_ctrl_new_std(&priv->hdl, &S5K4ECGX_ctrl_ops,
			V4L2_CID_HFLIP, 0, 1, 1, 0);
	v4l2_ctrl_new_std(&priv->hdl, &S5K4ECGX_ctrl_ops,
			V4L2_CID_GAIN, 0, 127, 1, 66);
	v4l2_ctrl_new_std(&priv->hdl, &S5K4ECGX_ctrl_ops,
			V4L2_CID_AUTO_WHITE_BALANCE, 0, 1, 1, 1);
	priv->subdev.ctrl_handler = &priv->hdl;
	if (priv->hdl.error) {
		int err = priv->hdl.error;

		kfree(priv);
		return err;
	}

	priv->width	= 640;
	priv->height	= 480;
	priv->fmt	= &S5K4ECGX_colour_fmts[0];
	ret = v4l2_ctrl_handler_setup(&priv->hdl);
	if (0 > ret) {
		dev_err(&client->dev,
			"S5K4ECGX: v4l2_ctrl_handler_setup Error(%d)\n", ret);
		kfree(priv);
		return ret;
	}

	if (cam_class_init == false) {
		dev_dbg(&client->dev,
			"Start create class for factory test mode !\n");
		camera_class = class_create(THIS_MODULE, "camera");
		cam_class_init = true;
	}

	if (camera_class) {
		dev_dbg(&client->dev, "Create main camera device !\n");

		sec_main_cam_dev = device_create(camera_class,
						NULL, 0, NULL, "rear");
		if (IS_ERR(sec_main_cam_dev)) {
			dev_err(&client->dev,
				"Failed to create device"
				"(sec_main_cam_dev)!\n");
		}

		if (device_create_file(sec_main_cam_dev,
					&dev_attr_rear_camtype) < 0) {
			dev_err(&client->dev,
				"failed to create main camera "
				"device file, %s\n",
				dev_attr_rear_camtype.attr.name);
		}
		if (device_create_file(sec_main_cam_dev,
					&dev_attr_rear_camfw) < 0) {
			dev_err(&client->dev,
				"failed to create main camera "
				"device file, %s\n",
				dev_attr_rear_camfw.attr.name);
		}
		if (device_create_file(sec_main_cam_dev,
					&dev_attr_rear_flash) < 0) {
			dev_err(&client->dev,
				"failed to create main camera "
				"device file, %s\n",
				dev_attr_rear_flash.attr.name);
		}
		if (device_create_file(sec_main_cam_dev,
					&dev_attr_rear_vendorid) < 0) {
			dev_err(&client->dev,
				"failed to create main camera "
				"device file, %s\n",
				dev_attr_rear_vendorid.attr.name);
		}
	}

	return ret;
}

static int S5K4ECGX_remove(struct i2c_client *client)
{
	struct S5K4ECGX *priv = to_S5K4ECGX(client);
	struct soc_camera_subdev_desc *sdesc = soc_camera_i2c_to_desc(client);

	v4l2_device_unregister_subdev(&priv->subdev);
	if (sdesc->free_bus)
		sdesc->free_bus(sdesc);
	v4l2_ctrl_handler_free(&priv->hdl);
	kfree(priv);

	return 0;
}

static const struct i2c_device_id S5K4ECGX_id[] = {
	{ "S5K4ECGX", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, S5K4ECGX_id);

static struct i2c_driver S5K4ECGX_i2c_driver = {
	.driver = {
		.name = "S5K4ECGX",
	},
	.probe		= S5K4ECGX_probe,
	.remove		= S5K4ECGX_remove,
	.id_table	= S5K4ECGX_id,
};

module_i2c_driver(S5K4ECGX_i2c_driver);

MODULE_DESCRIPTION("Samsung S5K4ECGX Camera driver");
MODULE_AUTHOR("Renesas Mobile Corp.");
MODULE_LICENSE("GPL v2");
