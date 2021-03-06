/******************************************************************************
* Copyright (C) 2013 Broadcom Corporation
*
* @file   /kernel/drivers/misc/bcm-bt-lpm.c
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation version 2.
*
* This program is distributed "as is" WITHOUT ANY WARRANTY of any
* kind, whether express or implied; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
******************************************************************************/
/*
 * Broadcom bluetooth low power mode driver.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/broadcom/bcm-bt-lpm.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/version.h>

#include <linux/interrupt.h>
#include <asm/irq.h>
#include <linux/uaccess.h>
#include <linux/serial_core.h>
#include <linux/tty.h>
#include <linux/delay.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
	#include <linux/pm_wakeup.h>
#else
	#include <linux/wakelock.h>
#endif

#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>

#ifndef BCM_BT_LPM_BT_WAKE_ASSERT
#define BCM_BT_LPM_BT_WAKE_ASSERT 0
#endif

#ifndef BCM_BT_LPM_BT_WAKE_DEASSERT
#define BCM_BT_LPM_BT_WAKE_DEASSERT (!(BCM_BT_LPM_BT_WAKE_ASSERT))
#endif

#ifndef BCM_BT_LPM_HOST_WAKE_ASSERT
#define BCM_BT_LPM_HOST_WAKE_ASSERT 0
#endif
#ifndef BCM_BT_LPM_HOST_WAKE_DEASSERT
#define BCM_BT_LPM_HOST_WAKE_DEASSERT (!(BCM_BT_LPM_HOST_WAKE_ASSERT))
#endif

#define BCM_SHARED_UART_MAGIC	0x80
#define TIO_ASSERT_BT_WAKE	_IO(BCM_SHARED_UART_MAGIC, 3)
#define TIO_DEASSERT_BT_WAKE	_IO(BCM_SHARED_UART_MAGIC, 4)
#define TIO_GET_BT_WAKE_STATE	_IO(BCM_SHARED_UART_MAGIC, 5)


struct bcm_bt_lpm_struct {
	spinlock_t bcm_bt_lpm_lock;
	struct uart_port *uport;
	int host_irq;
};

struct bcm_bt_lpm_entry_struct {
	struct bcm_bt_lpm_platform_data *pdata;
	struct bcm_bt_lpm_struct *plpm;
	struct platform_device *pdev;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
	struct wakeup_source *bt_wake_ws;
	struct wakeup_source *host_wake_ws;
#else
	struct wake_lock bt_wake_lock;
	struct wake_lock host_wake_lock;
#endif
};

struct bcm_bt_lpm_entry_struct *priv_g;

struct bcm_bt_lpm_ldisc_data {
	int	(*open)(struct tty_struct *);
	void	(*close)(struct tty_struct *);
};

static struct tty_ldisc_ops bcm_bt_lpm_ldisc_ops;
static struct bcm_bt_lpm_ldisc_data bcm_bt_lpm_ldisc_saved;

int bcm_bt_lpm_assert_bt_wake(void)
{
	pr_debug("%s BLUETOOTH: Enter ASSERT BT_WAKE\n", __func__);
	if (priv_g == NULL) {
		pr_err(
		"%s BLUETOOTH:data corrupted: cannot assert bt_wake\n",
				__func__);
		return -EFAULT;
	}
	if (unlikely((priv_g->pdata->bt_wake_gpio == -1)))
		return -EFAULT;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
	__pm_stay_awake(priv_g->bt_wake_ws);
#else
	if (!wake_lock_active(&priv_g->bt_wake_lock))
		wake_lock(&priv_g->bt_wake_lock);

#endif
	gpio_set_value(priv_g->pdata->bt_wake_gpio,
					BCM_BT_LPM_BT_WAKE_ASSERT);
	pr_debug("%s BLUETOOTH: Exit ASSERT BT_WAKE\n", __func__);
	return 0;
}

int bcm_bt_lpm_deassert_bt_wake(void)
{
	if (priv_g == NULL) {
		pr_err(
		"%s BLUETOOTH:data corrupted:cannot de-assert bt_wake\n",
						__func__);
		return -EFAULT;
	}
	if (unlikely((priv_g->pdata->bt_wake_gpio == -1)))
		return -EFAULT;
	gpio_set_value(priv_g->pdata->bt_wake_gpio,
					BCM_BT_LPM_BT_WAKE_DEASSERT);
	pr_debug("%s: BLUETOOTH: BT_WAKE de-asserted.\n", __func__);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
	__pm_relax(priv_g->bt_wake_ws);
#else
	if (wake_lock_active(&priv_g->bt_wake_lock))
		wake_unlock(&priv_g->bt_wake_lock);
#endif
	return 0;
}

static int bcm_bt_lpm_init_bt_wake(
				struct bcm_bt_lpm_entry_struct *priv)
{
	int rc = 0;

	pr_debug("%s BLUETOOTH:Enter(init bt wake).\n", __func__);

	if (priv->pdata->bt_wake_gpio < 0) {
		pr_err("%s: Exiting => invalid bt-wake-gpio=%d\n",
		__func__, priv->pdata->bt_wake_gpio);
		return -EFAULT;
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
	rc = devm_gpio_request_one(&priv->pdev->dev,
			priv->pdata->bt_wake_gpio,
			GPIOF_OUT_INIT_LOW,
			"bt_wake_gpio");
#else
	rc = gpio_request(priv->pdata->bt_wake_gpio, "bt_wake_gpio");
#endif

	if (rc) {
		pr_err
	("%s: failed to init bt_wake: bt_wake_gpio request err%d\n",
		__func__, rc);
		return rc;
	}

	pr_debug("%s: BT_WAKE GPIO: %d\n", __func__,
                        priv->pdata->bt_wake_gpio);

	rc = gpio_direction_output(
			priv->pdata->bt_wake_gpio,
			BCM_BT_LPM_BT_WAKE_DEASSERT);

	pr_debug("%s BLUETOOTH:Exit(init bt wake).\n", __func__);
	return 0;
}

static void bcm_bt_lpm_tty_cleanup(void)
{
	int err;

	pr_debug("%s BLUETOOTH: Entering(tty cleanup).\n", __func__);
	err = tty_unregister_ldisc(N_BRCM_HCI);
	if (err)
		pr_err(
		"can't unregister N_BRCM_HCI line discipline\n");
	else
		pr_info("N_BRCM_HCI line discipline removed\n");
	pr_debug("%s BLUETOOTH: Exiting(tty cleanup).\n", __func__);
}

static void bcm_bt_lpm_clean_bt_wake(
			struct bcm_bt_lpm_entry_struct *priv,
			bool b_tty)
{
	if (priv->pdata->bt_wake_gpio < 0)
		return;

	gpio_free((unsigned)priv->pdata->bt_wake_gpio);

	if (b_tty)
		bcm_bt_lpm_tty_cleanup();

	pr_debug("%s BLUETOOTH:Exiting (Clean BT Wake).\n", __func__);
}

static irqreturn_t bcm_bt_lpm_host_wake_isr(int irq, void *dev)
{
	unsigned int host_wake;
	unsigned long flags;
	struct bcm_bt_lpm_entry_struct *priv;

	priv = (struct bcm_bt_lpm_entry_struct *)dev;
	if (priv == NULL) {
		pr_err(
		"%s BLUETOOTH: Error data pointer is null\n",
							__func__);
		return IRQ_HANDLED;
	}
	spin_lock_irqsave(&priv->plpm->bcm_bt_lpm_lock, flags);

	host_wake = gpio_get_value(
			priv->pdata->host_wake_gpio);
	if (BCM_BT_LPM_HOST_WAKE_ASSERT == host_wake) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
		__pm_stay_awake(priv->host_wake_ws);
#else
	if (!wake_lock_active(&priv->host_wake_lock))
		wake_lock(&priv->host_wake_lock);
#endif
	} else {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
		__pm_relax(priv->host_wake_ws);
#else
	if (wake_lock_active(&priv->host_wake_lock))
		wake_unlock(&priv->host_wake_lock);
#endif
	}

	spin_unlock_irqrestore(&priv->plpm->bcm_bt_lpm_lock, flags);
	pr_debug("%s BLUETOOTH:Exiting (host_wake_isr).\n", __func__);
	return IRQ_HANDLED;
}

static int bcm_bt_lpm_init_hostwake(struct bcm_bt_lpm_entry_struct *priv)
{
	int rc = -1;

	pr_debug("%s BLUETOOTH:Entering.\n", __func__);
	if ((priv == NULL) ||
		(priv->pdata->host_wake_gpio < 0)) {
		pr_err("%s BLUETOOTH: invalid host-wake-gpio.\n",
			__func__);
		return -EFAULT;
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
	rc = devm_gpio_request_one(&priv->pdev->dev,
				priv->pdata->host_wake_gpio,
				GPIOF_OUT_INIT_LOW,
				"host_wake_gpio");
#else
	rc = gpio_request(priv->pdata->host_wake_gpio, "host_wake_gpio");
#endif

	pr_debug("%s: HOST_WAKE GPIO: %d\n", __func__,
		priv->pdata->host_wake_gpio);

	if (rc) {
		pr_err
	    ("%s: failed to configure host-wake-gpio err=%d\n",
		     __func__, rc);
		return rc;
	}
	gpio_direction_input(priv->pdata->host_wake_gpio);

	pr_debug("%s BLUETOOTH:Exiting (init_host_wake).\n", __func__);
	return rc;
}

static void bcm_bt_lpm_clean_host_wake(
				struct bcm_bt_lpm_entry_struct *priv)
{
	pr_debug("%s BLUETOOTH:Entering(clean host wake).\n", __func__);
	if ((priv == NULL) ||
		 (priv->pdata->host_wake_gpio < 0)) {
		pr_err("%s BLUETOOTH: NULL ptr or invalid hw gpio.\n",
			__func__);
		return;
	}

	gpio_free((unsigned)priv->pdata->host_wake_gpio);

	free_irq(priv->plpm->host_irq, priv_g);
	pr_debug("%s BLUETOOTH:Exiting (clean_host_wake).\n", __func__);
}


void bcm_bt_lpm_start(void)
{
	int rc = -1;

	pr_debug("%s BLUETOOTH: Entering (lpm start).\n", __func__);
	if (priv_g != NULL) {
		pr_debug(
		"%s: BLUETOOTH: data pointer is valid\n", __func__);

		rc = gpio_to_irq(
			priv_g->pdata->host_wake_gpio);
		if (rc < 0) {
			pr_err
		("%s: failed to configure BT Host Mgmt err=%d\n",
				__func__, rc);
			return;
		}
		priv_g->plpm->host_irq = rc;
		pr_debug("%s: BLUETOOTH: request host_wake_irq=%d\n",
			__func__, priv_g->plpm->host_irq);

		rc = request_irq(
			priv_g->plpm->host_irq,
			bcm_bt_lpm_host_wake_isr,
			(IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING |
			IRQF_NO_SUSPEND), "bt_host_wake", priv_g);
		if (rc) {
			pr_err("%s: request irq failed: err=%d\n",
						 __func__, rc);
		}

		rc = irq_set_irq_wake(priv_g->plpm->host_irq, 1);

		if (rc) {
			pr_err("%s: failed to configure BT as wakeup source\n",
					__func__);
			return;
		}
		else
			pr_info("%s: configure BT as wakeup source\n", __func__);
	} else {
		pr_err("%s: BLUETOOTH:data ptr null, Uninitialized\n",
						__func__);
		return;
	}
	pr_debug("%s BLUETOOTH:Exiting (lpm_start).\n", __func__);
	return;
}


static int bcm_bt_get_bt_wake_state(unsigned long arg)
{
	void __user *uarg = (void __user *)arg;
	unsigned long tmp;

	pr_debug("%s BLUETOOTH: Entering (bt wake state).\n", __func__);
	if ((priv_g == NULL) ||
		 (priv_g->pdata->bt_wake_gpio < 0)) {
		pr_err("%s BLUETOOTH: NULL ptr or invalid bw gpio.\n",
			__func__);
		return -EFAULT;
	}

	tmp = gpio_get_value(priv_g->pdata->bt_wake_gpio);

	pr_info(
		"%s: (bt_wake:%d), gpio_get_value(tmp:%d)\n",
		__func__, priv_g->pdata->bt_wake_gpio,
						(int) tmp);

	if (copy_to_user(uarg, &tmp, sizeof(*uarg)))
		return -EFAULT;
	pr_debug("%s BLUETOOTH:Exiting (get_bt_wake_state).\n", __func__);
	return 0;
}

static int bcm_bt_lpm_tty_ioctl(struct tty_struct *tty,
			struct file *file,
			unsigned int cmd, unsigned long arg)
{
	int rc = -1;

	pr_debug("%s:BLUETOOTH:Enter bcm_bt_lpm_tty_ioctl,cmd=%d\n",
						__func__, cmd);
	switch (cmd) {
	case TIO_ASSERT_BT_WAKE:
		rc = bcm_bt_lpm_assert_bt_wake();
		pr_debug("%s: BLUETOOTH: TIO_ASSERT_BT_WAKE\n",
							__func__);
		break;

	case TIO_DEASSERT_BT_WAKE:
		rc = bcm_bt_lpm_deassert_bt_wake();
		pr_debug("%s: BLUETOOTH: TIO_DEASSERT_BT_WAKE\n",
							__func__);
		break;

	case TIO_GET_BT_WAKE_STATE:
		rc = bcm_bt_get_bt_wake_state(arg);
		pr_debug("%s: BLUETOOTH: TIO_GET_BT_WAKE_STATE\n",
							__func__);
		break;

	default:
		pr_err("%s: BLUETOOTH: switch default. cmd=%d\n",
						__func__, cmd);
		return n_tty_ioctl_helper(tty, file, cmd, arg);

	}
	pr_debug("%s:BLUETOOTH:Exit bcm_bt_lpm_tty_ioctl,cmd=%d\n",
						__func__, cmd);
	return rc;
}

static int bcm_bt_lpm_tty_open(struct tty_struct *tty)
{
	struct uart_state *state;

	pr_debug("%s BLUETOOTH: Entering(tty open).\n", __func__);
	state = tty->driver_data;
	if (priv_g == NULL) {
		pr_err("%s BLUETOOTH: NULL ptr or invalid bw gpio.\n",
			__func__);
		return -EFAULT;
	}
	priv_g->plpm->uport = state->uart_port;

	bcm_bt_lpm_init_bt_wake(priv_g);
	bcm_bt_lpm_init_hostwake(priv_g);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
	priv_g->bt_wake_ws = wakeup_source_register(
					"bcm-bt-lpm-btwake-ws");
	if (priv_g->bt_wake_ws == NULL) {
		pr_err(
		"%s: Failed to register bt-wake wakeup source\n",
							__func__);
		return -ENODEV;
	}

	priv_g->host_wake_ws = wakeup_source_register(
					"bcm-bt-lpm-hostwake-ws");
	if (priv_g->host_wake_ws == NULL) {
		pr_err(
		"%s: failed to register host-wake wakeup source\n",
							__func__);
		return -ENODEV;
	}
#else
	wake_lock_init(&priv_g->host_wake_lock,
					WAKE_LOCK_SUSPEND, "BT_HOSTWAKE");
	wake_lock_init(&priv_g->bt_wake_lock,
					WAKE_LOCK_SUSPEND, "BT_BTWAKE");
#endif

	bcm_bt_lpm_start();
	pr_debug("bcm_bt_lpm_tty_open()::open(): x%p",
				bcm_bt_lpm_ldisc_saved.open);

	if (bcm_bt_lpm_ldisc_saved.open)
		return bcm_bt_lpm_ldisc_saved.open(tty);
	pr_debug("%s BLUETOOTH:Exiting (tty_open).\n", __func__);
	return 0;
}

static void bcm_bt_lpm_tty_close(struct tty_struct *tty)
{
	struct uart_state *state;

	pr_debug("%s BLUETOOTH: Entering.\n", __func__);
	if (!priv_g || !priv_g->plpm) {
		pr_err("%s:data freed?priv_g:0x%p,p_lpm: 0x%p\n",
				__func__, priv_g, priv_g->plpm);
		return;
	}
	priv_g->plpm->uport = 0;
	state = tty->driver_data;

	bcm_bt_lpm_clean_bt_wake(priv_g, false);
	bcm_bt_lpm_clean_host_wake(priv_g);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
	wakeup_source_unregister(priv_g->bt_wake_ws);
	wakeup_source_unregister(priv_g->host_wake_ws);
#else
	wake_lock_destroy(&priv_g->bt_wake_lock);
	wake_lock_destroy(&priv_g->host_wake_lock);
#endif
	pr_debug("bcm_bt_lpm_tty_close(line: %d)::close(): x%p",
	state->uart_port->line, bcm_bt_lpm_ldisc_saved.close);

	if (bcm_bt_lpm_ldisc_saved.close)
		bcm_bt_lpm_ldisc_saved.close(tty);
	pr_debug("%s BLUETOOTH:Exiting (tty_close).\n", __func__);
	return;
}

static int bcm_bt_lpm_tty_init(void)
{
	int err;
	pr_debug("%s: BLUETOOTH (tty init):\n", __func__);

	/* Inherit the N_TTY's ops */
	n_tty_inherit_ops(&bcm_bt_lpm_ldisc_ops);

	bcm_bt_lpm_ldisc_ops.owner = THIS_MODULE;
	bcm_bt_lpm_ldisc_ops.name = "bcm_bt_lpm_tty";
	bcm_bt_lpm_ldisc_ops.ioctl = bcm_bt_lpm_tty_ioctl;
	bcm_bt_lpm_ldisc_saved.open = bcm_bt_lpm_ldisc_ops.open;
	bcm_bt_lpm_ldisc_saved.close = bcm_bt_lpm_ldisc_ops.close;
	bcm_bt_lpm_ldisc_ops.open = bcm_bt_lpm_tty_open;
	bcm_bt_lpm_ldisc_ops.close = bcm_bt_lpm_tty_close;

	err = tty_register_ldisc(N_BRCM_HCI, &bcm_bt_lpm_ldisc_ops);
	if (err)
		pr_err("can't register N_BRCM_HCI line discipline\n");
	else
		pr_info("N_BRCM_HCI line discipline registered\n");

	pr_debug("%s BLUETOOTH:Exiting(tty_init).\n", __func__);
	return err;
}

static const struct of_device_id bcm_bt_lpm_of_match[] = {
		{ .compatible = "bcm,bcm-bt-lpm",},
		{ /* Sentinel */ },
	};

MODULE_DEVICE_TABLE(of, bcm_bt_lpm_of_match);

static int bcm_bt_lpm_probe(struct platform_device *pdev)
{
	int rc = -EINVAL;
	struct bcm_bt_lpm_platform_data *pdata = NULL;

	const struct of_device_id *match = NULL;
	match = of_match_device(bcm_bt_lpm_of_match, &pdev->dev);
	if (!match) {
		pr_err("%s: **ERROR** No matcing device found\n",
							__func__);
	}

	priv_g = devm_kzalloc(&pdev->dev,
			sizeof(*priv_g),
			GFP_KERNEL);

	if (priv_g == NULL)
		return -ENOMEM;

	priv_g->pdata = devm_kzalloc(&pdev->dev,
			sizeof(
			struct bcm_bt_lpm_platform_data
			),
			GFP_KERNEL);

	if (priv_g->pdata == NULL)
		return -ENOMEM;

	priv_g->plpm  = devm_kzalloc(&pdev->dev,
			sizeof(struct bcm_bt_lpm_struct),
			GFP_KERNEL);

	if (priv_g->plpm == NULL)
		return -ENOMEM;

	spin_lock_init(&priv_g->plpm->bcm_bt_lpm_lock);

	if (!match && pdev->dev.platform_data) {
		pdata = pdev->dev.platform_data;
		priv_g->pdata->bt_wake_gpio   = pdata->bt_wake_gpio;
		priv_g->pdata->host_wake_gpio = pdata->host_wake_gpio;
	} else if (match && pdev->dev.of_node) {
		priv_g->pdata->bt_wake_gpio =
					of_get_named_gpio(
					pdev->dev.of_node,
					"bt-wake-gpio",
					0);
		if (!gpio_is_valid(
			priv_g->pdata->bt_wake_gpio)) {
			pr_err(
			"%s: ERROR Invalid bt-wake-gpio\n",
							__func__);
			rc = priv_g->pdata->bt_wake_gpio;
			goto out;
		}

		priv_g->pdata->host_wake_gpio =
				of_get_named_gpio(pdev->dev.of_node,
				"host-wake-gpio", 0);
		if (!gpio_is_valid(
			priv_g->pdata->host_wake_gpio)) {
			pr_err(
			"%s:ERROR Invalid host-wake-gpio\n",
							__func__);
			rc = priv_g->pdata->host_wake_gpio;
			goto out;
		}
	} else {
		pr_err("%s: **ERROR** NO platform data/device tree available\n",
							__func__);
		rc = -ENODEV;
		goto out;
	}

	priv_g->pdev = pdev;

	pr_info("%s: bt_wake_gpio=%d, host_wake_gpio=%d\n",
		__func__, priv_g->pdata->bt_wake_gpio,
		priv_g->pdata->host_wake_gpio);

	/* register line discipline driver */
	rc = bcm_bt_lpm_tty_init();

	if (rc) {
		pr_err("%s: bcm_bt_lpm_tty_init failed\n",
							__func__);
		rc = -1;
	}

	pr_debug("%s BLUETOOTH:Exiting (probe_good).\n", __func__);
	return rc;

out:
	pr_debug("%s BLUETOOTH:Exiting after cleaning.\n",
							__func__);
	return rc;
}

static int bcm_bt_lpm_remove(struct platform_device *pdev)
{
	if (priv_g == NULL)
		return 0;

	if (priv_g->pdata) {
		bcm_bt_lpm_clean_bt_wake(priv_g, true);
		bcm_bt_lpm_clean_host_wake(priv_g);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
		wakeup_source_unregister(priv_g->bt_wake_ws);
		wakeup_source_unregister(priv_g->host_wake_ws);
#endif
	}
	if (priv_g != NULL) {
		if ((pdev->dev.of_node != NULL) &&
				(priv_g->pdata != NULL)) {
			kfree(priv_g->pdata);
			kfree(priv_g->plpm);
		}
		kfree(priv_g);
		priv_g = NULL;
	}
	pr_debug("%s BLUETOOTH:Exiting (lpm_remove).\n", __func__);
	return 0;
}

static struct platform_driver bcm_bt_lpm_platform_driver = {
	.probe = bcm_bt_lpm_probe,
	.remove = bcm_bt_lpm_remove,
	.driver = {
		   .name = "bcm-bt-lpm",
		   .owner = THIS_MODULE,
		   .of_match_table = bcm_bt_lpm_of_match,
		   },
};

static int __init bcm_bt_lpm_init(void)
{
	int rc = -1;

	pr_debug("%s BLUETOOTH: Entering.\n", __func__);
	rc = platform_driver_register(&bcm_bt_lpm_platform_driver);
	if (rc)
		pr_err(
		"BLUETOOTH: driver register failed err=%d, Exiting %s.\n",
							rc, __func__);
	else
		pr_debug(
		"BLUETOOTH: Init success. Exiting %s.\n", __func__);
	return rc;
}

static void __exit bcm_bt_lpm_exit(void)
{
	platform_driver_unregister(&bcm_bt_lpm_platform_driver);
	pr_debug("%s BLUETOOTH:Exiting (lpm_exit).\n", __func__);
}

module_init(bcm_bt_lpm_init);
module_exit(bcm_bt_lpm_exit);

MODULE_DESCRIPTION("bcm-bt-lpm");
MODULE_AUTHOR("Broadcom");
MODULE_LICENSE("GPL");
MODULE_ALIAS_LDISC(N_BRCM_HCI);

