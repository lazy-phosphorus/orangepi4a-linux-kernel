/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * (C) Copyright 2010-2015
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * javen, 2011-4-14, create this file
 *
 * usb detect module.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/clk.h>

#include <linux/debugfs.h>
#include <linux/seq_file.h>

#include <asm/byteorder.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/unaligned.h>
#include <linux/gpio.h>
#include  "../include/sunxi_usb_config.h"
#include  "usb_manager.h"
#include  "usb_hw_scan.h"
#include  "usb_msg_center.h"

#include <linux/power_supply.h>

static struct usb_scan_info g_usb_scan_info;

static struct power_supply __maybe_unused *psy;

int device_insmod_delay;
static void (*__usb_hw_scan)(struct usb_scan_info *);

#ifndef SUNXI_USB_FPGA
static __u32 get_pin_data(struct usb_gpio *usb_gpio)
{
	return gpio_get_value(usb_gpio->gpio_set.gpio);
}

#if IS_ENABLED(CONFIG_USB_G_ANDROID) || IS_ENABLED(CONFIG_USB_MASS_STORAGE)
static int get_usb_gadget_functions(void)
{
	struct file *filep;
	loff_t pos;
	char buf[32] = {0};

	filep = filp_open("/sys/class/android_usb/android0/functions",
			O_RDONLY, 0);
	if (IS_ERR(filep))
		return 0;

	pos = 0;
	vfs_read(filep, (char __user *)buf, 32, &pos);
	filp_close(filep, NULL);

	if (strlen(buf) == 0)
		return 0;
	else
		return 1;
}
#endif

/**
 * filter the PIO burr
 * @usb_gpio: .
 * @value:    store the value to be read
 *
 * try 10 times, if value is the same, then consider no shake. return any one.
 * if not the same, then the current read is invalid
 *
 * return: 0 - the valude is the same, valid, 1 - the value change, invalid.
 */
static __u32 PIODataIn_debounce(struct usb_gpio *usb_gpio, __u32 *value)
{
	__u32 retry  = 0;
	__u32 time   = 10;
	__u32 temp1  = 0;
	__u32 cnt    = 0;
	__u32 change = 0;	/* if have shake */

	/**
	 * try 10 times, if value is the same,
	 * then current read is valid; otherwise invalid.
	 */
	if (usb_gpio->valid) {
		retry = time;
		while (retry--) {
			temp1 = get_pin_data(usb_gpio);
			if (temp1)
				cnt++;
		}

		/* 10 times, the value is all 0 or 1 */
		if ((cnt == time) || (cnt == 0))
			change = 0;
		else
			change = 1;
	} else {
		change = 1;
	}

	if (!change)
		*value = temp1;

	DMSG_DBG_MANAGER("usb_gpio->valid = %x, cnt = %x, change= %d, temp1 = %x\n",
		usb_gpio->valid, cnt, change, temp1);

	return change;
}

static __u32 PMUDataIn_debounce(struct power_supply *pmu_psy, union power_supply_propval *value)
{
	__u32 retry  = 0;
	__u32 time   = 10;
	union power_supply_propval temp;
	__u32 cnt    = 0;
	__u32 change = 0;	/* if have shake */

	/**
	 * try 10 times, if value is the same,
	 * then current read is valid; otherwise invalid.
	 */
	if (pmu_psy) {
		retry = time;
		while (retry--) {
			power_supply_get_property(pmu_psy,
						POWER_SUPPLY_PROP_SCOPE, &temp);

			temp.intval &= 0x3;
			temp.intval -= 1;

			if (temp.intval)
				cnt++;
		}

		/* 10 times, the value is all 0 or 1 */
		if ((cnt == time) || (cnt == 0))
			change = 0;
		else
			change = 1;
	}

	if (!change)
		*value = temp;

	DMSG_DBG_MANAGER("usb_gpio->valid = %x, cnt = %x, change= %d, temp1 = %x\n",
		usb_gpio->valid, cnt, change, temp1);

	return change;
}


static __u32 get_pmu_state(struct usb_scan_info *info)
{
	enum usb_id_state id_state = USB_DEVICE_MODE;
	union power_supply_propval pin_data;

	if (!PMUDataIn_debounce(info->cfg->port.pmu_psy, &pin_data)) {
		if (pin_data.intval)
			id_state = USB_DEVICE_MODE;
		else
			id_state = USB_HOST_MODE;

		info->id_old_state = id_state;
	} else {
		id_state = info->id_old_state;
	}

	return id_state;
}

static u32 get_id_state(struct usb_scan_info *info)
{
	enum usb_id_state id_state = USB_DEVICE_MODE;
	__u32 pin_data = 0;

	if (info->cfg->port.id_type == USB_ID_TYPE_GPIO) {
		if (info->cfg->port.id.valid) {
			if (!PIODataIn_debounce(&info->cfg->port.id, &pin_data)) {
				if (pin_data)
					id_state = USB_DEVICE_MODE;
				else
					id_state = USB_HOST_MODE;

				info->id_old_state = id_state;
			} else {
				id_state = info->id_old_state;
			}
		}
	} else if (info->cfg->port.id_type == USB_ID_TYPE_AXP) {
#if IS_ENABLED(CONFIG_TYPE_C)
		if (axp_usb_cc_status())
			id_state = USB_DEVICE_MODE;
		else
			id_state = USB_HOST_MODE;
#endif
		} else {
			id_state = info->id_old_state;
	}

	return id_state;
}

static u32 get_detect_vbus_state(struct usb_scan_info *info)
{
	enum usb_det_vbus_state det_vbus_state = USB_DET_VBUS_INVALID;
	__u32 pin_data = 0;
	union power_supply_propval temp;

	if (info->cfg->port.detect_mode == USB_DETECT_MODE_THREAD) {
		if (info->cfg->port.det_vbus_type == USB_DET_VBUS_TYPE_GPIO) {
			if (info->cfg->port.det_vbus.valid) {
				if (!PIODataIn_debounce(&info->cfg->port.det_vbus, &pin_data)) {
					if (pin_data)
						det_vbus_state = USB_DET_VBUS_VALID;
					else
						det_vbus_state = USB_DET_VBUS_INVALID;

					info->det_vbus_old_state = det_vbus_state;
				} else {
					det_vbus_state = info->det_vbus_old_state;
				}
			}
		} else if (info->cfg->port.det_vbus_type
					== USB_DET_VBUS_TYPE_AXP) {
				/* det_vbus is axp, get data only once */
				if (!psy) {
					if (of_find_property(info->cfg->pdev->dev.of_node,
								"det_vbus_supply", NULL))
						psy = devm_power_supply_get_by_phandle(&info->cfg->pdev->dev,
								"det_vbus_supply");

					if (!psy || IS_ERR(psy)) {
						DMSG_INFO("%s()%d WARN: "
								"get power supply failed\n",
								__func__, __LINE__);
					} else {
						goto get_property;
					}

				} else {
get_property:
					power_supply_get_property(psy,
							POWER_SUPPLY_PROP_ONLINE, &temp);

					if (temp.intval)
						det_vbus_state = USB_DET_VBUS_VALID;
					else
						det_vbus_state = USB_DET_VBUS_INVALID;
				}

		} else {
			det_vbus_state = info->det_vbus_old_state;
		}
	} else if (info->cfg->port.detect_mode == USB_DETECT_MODE_INTR) {
		det_vbus_state = USB_DET_VBUS_VALID;
	} else if (info->cfg->port.detect_mode == USB_DETECT_MODE_NOTIFY) {
		if (info->cfg->port.det_vbus_type == USB_DET_VBUS_TYPE_AXP) {
			/* det_vbus is axp, get data only once */
			if (!psy) {
				if (of_find_property(info->cfg->pdev->dev.of_node,
							"det_vbus_supply", NULL))
					psy = devm_power_supply_get_by_phandle(&info->cfg->pdev->dev,
							"det_vbus_supply");

				if (!psy || IS_ERR(psy)) {
					DMSG_INFO("%s()%d WARN: "
							"get power supply failed\n",
							__func__, __LINE__);
				} else {
					goto get_property_notify;
				}

			} else {
get_property_notify:
				power_supply_get_property(psy,
						POWER_SUPPLY_PROP_ONLINE, &temp);

				if (temp.intval)
					det_vbus_state = USB_DET_VBUS_VALID;
				else
					det_vbus_state = USB_DET_VBUS_INVALID;
			}
		} else {
			det_vbus_state = info->det_vbus_old_state;
		}
	} else {
		DMSG_ERR("ERR: get_detect_vbus_state, ");
		DMSG_ERR("usb det mode isn't supported\n");
	}

	return det_vbus_state;
}

static u32 get_dp_dm_status(struct usb_scan_info *info)
{
	u32 ret  = 0;
	u32 ret0 = 0;
	u32 ret1 = 0;
	u32 ret2 = 0;

	ret0 = get_dp_dm_status_normal();
	ret1 = get_dp_dm_status_normal();
	ret2 = get_dp_dm_status_normal();

	/* continuous 3 times, to avoid the voltage sudden changes */
	if ((ret0 == ret1) && (ret0 == ret2)) {
		ret = ret0;
	} else if (ret2 == 0x11) {
		if (get_usb_role() == SW_USB_ROLE_DEVICE) {
			ret = 0x11;
			DMSG_ERR("ERR: dp/dm status is continuous(0x11)\n");
		}
	} else {
		ret = ret2;
	}

	return ret;
}
#endif

static void do_vbus0_id0(struct usb_scan_info *info)
{
	enum sw_usb_role role = SW_USB_ROLE_NULL;

	role = get_usb_role();
	device_insmod_delay = 0;

	switch (role) {
	case SW_USB_ROLE_NULL:
		if (info->cfg->port.detect_mode == USB_DETECT_MODE_THREAD) {
			/* delay for vbus is stably */
			if (atomic_read(&thread_suspend_flag))
				break;
			if (info->host_insmod_delay
					< USB_SCAN_INSMOD_HOST_DRIVER_DELAY) {
				info->host_insmod_delay++;
				break;
			}
			info->host_insmod_delay = 0;

			/* insmod usb host */
			hw_insmod_usb_host();
		} else if (info->cfg->port.detect_mode
					== USB_DETECT_MODE_INTR) {
			hw_insmod_usb_host();
		} else if (info->cfg->port.detect_mode
					== USB_DETECT_MODE_NOTIFY) {
			if (atomic_read(&notify_suspend_flag))
				break;
			hw_insmod_usb_host();
		} else {
			DMSG_ERR("ERR: do_vbus0_id0, ");
			DMSG_ERR("usb det mode isn't supported, role=%d\n",
				role);
		}
		break;

	case SW_USB_ROLE_HOST:
		/* nothing to do */
		break;

	case SW_USB_ROLE_DEVICE:
		/* rmmod usb device */
		if (atomic_read(&thread_suspend_flag))
			break;
		if (atomic_read(&notify_suspend_flag))
			break;
		hw_rmmod_usb_device();
		break;

	default:
		DMSG_ERR("ERR: unknown usb role(%d)\n", role);
	}
}

static void do_vbus0_id1(struct usb_scan_info *info)
{
	enum sw_usb_role role = SW_USB_ROLE_NULL;

	role = get_usb_role();
	device_insmod_delay = 0;
	info->host_insmod_delay   = 0;

	switch (role) {
	case SW_USB_ROLE_NULL:
		/* nothing to do */
		break;
	case SW_USB_ROLE_HOST:
		if (atomic_read(&thread_suspend_flag))
			break;
		if (atomic_read(&notify_suspend_flag))
			break;
		hw_rmmod_usb_host();
		break;
	case SW_USB_ROLE_DEVICE:
		if (atomic_read(&thread_suspend_flag))
			break;
		if (atomic_read(&notify_suspend_flag))
			break;
		hw_rmmod_usb_device();
		break;
	default:
		DMSG_ERR("ERR: unknown usb role(%d)\n", role);
	}
}

static void do_vbus1_id0(struct usb_scan_info *info)
{
	enum sw_usb_role role = SW_USB_ROLE_NULL;

	role = get_usb_role();
	device_insmod_delay = 0;

	switch (role) {
	case SW_USB_ROLE_NULL:
		if (info->cfg->port.detect_mode == USB_DETECT_MODE_THREAD) {
			if (atomic_read(&thread_suspend_flag))
				break;
			/* delay for vbus is stably */
			if (info->host_insmod_delay
					< USB_SCAN_INSMOD_HOST_DRIVER_DELAY) {
				info->host_insmod_delay++;
				break;
			}
			info->host_insmod_delay = 0;

			hw_insmod_usb_host();
		} else if (info->cfg->port.detect_mode
					== USB_DETECT_MODE_INTR) {
			hw_insmod_usb_host();
		} else if (info->cfg->port.detect_mode
					== USB_DETECT_MODE_NOTIFY) {
			if (atomic_read(&notify_suspend_flag))
				break;
			hw_insmod_usb_host();
		} else {
			DMSG_ERR("ERR: do_vbus1_id0, ");
			DMSG_ERR("usb det mode isn't supported, role=%d\n",
				role);
		}
		break;
	case SW_USB_ROLE_HOST:
		/* nothing to do */
		break;
	case SW_USB_ROLE_DEVICE:
		if (info->cfg->port.detect_mode == USB_DETECT_MODE_THREAD) {
			if (atomic_read(&thread_suspend_flag))
				break;
			hw_rmmod_usb_device();
		} else if (info->cfg->port.detect_mode
					== USB_DETECT_MODE_INTR) {
			hw_rmmod_usb_device();
		} else if (info->cfg->port.detect_mode
					== USB_DETECT_MODE_NOTIFY) {
			if (atomic_read(&notify_suspend_flag))
				break;
			hw_rmmod_usb_device();
		} else {
			DMSG_ERR("ERR: do_vbus1_id0, ");
			DMSG_ERR("usb det mode isn't supported, role=%d\n",
				role);
		}
		break;
	default:
		DMSG_ERR("ERR: unknown usb role(%d)\n", role);
	}
}

static void do_vbus1_id1(struct usb_scan_info *info)
{
	enum sw_usb_role role = SW_USB_ROLE_NULL;

	role = get_usb_role();
	info->host_insmod_delay = 0;

	switch (role) {
	case SW_USB_ROLE_NULL:
#ifndef SUNXI_USB_FPGA
		if (info->cfg->port.detect_mode == USB_DETECT_MODE_THREAD) {
			if (get_dp_dm_status(info) == 0x00) {
				if (atomic_read(&thread_suspend_flag))
					break;
				/* delay for vbus is stably */
				if (device_insmod_delay < USB_SCAN_INSMOD_DEVICE_DRIVER_DELAY) {
					device_insmod_delay++;
					break;
				}

				device_insmod_delay = 0;
#if IS_ENABLED(CONFIG_USB_G_ANDROID) || IS_ENABLED(CONFIG_USB_MASS_STORAGE)
				if (get_usb_gadget_functions())
#endif
					hw_insmod_usb_device();
			}
		} else if (info->cfg->port.detect_mode
					== USB_DETECT_MODE_INTR) {
			hw_insmod_usb_device();
		} else if (info->cfg->port.detect_mode
					== USB_DETECT_MODE_NOTIFY) {
			if (atomic_read(&notify_suspend_flag))
				break;
			hw_insmod_usb_device();
		} else {
			DMSG_ERR("ERR: do_vbus1_id1, ");
			DMSG_ERR("usb det mode isn't supported, role=%d\n",
				role);
		}
#else
		hw_insmod_usb_device();
#endif
		break;
	case SW_USB_ROLE_HOST:
		if (info->cfg->port.detect_mode == USB_DETECT_MODE_THREAD) {
			if (atomic_read(&thread_suspend_flag))
				break;
			hw_rmmod_usb_host();
		} else if (info->cfg->port.detect_mode
					== USB_DETECT_MODE_INTR) {
			hw_rmmod_usb_host();
		} else if (info->cfg->port.detect_mode
					== USB_DETECT_MODE_NOTIFY) {
			if (atomic_read(&notify_suspend_flag))
				break;
			hw_rmmod_usb_host();
		} else {
			DMSG_ERR("ERR: do_vbus1_id1, ");
			DMSG_ERR("usb det mode isn't supported, role=%d\n",
				role);
		}
		break;
	case SW_USB_ROLE_DEVICE:
		/* nothing to do */
		break;
	default:
		DMSG_ERR("ERR: unknown usb role(%d)\n", role);
	}
}

#ifdef SUNXI_USB_FPGA
static u32 usb_vbus_id_state = 1;
__u32 set_vbus_id_state(u32 state)
{
	usb_vbus_id_state = state;
	return 0;
}
static __u32 get_vbus_id_state(struct usb_scan_info *info)
{
	return usb_vbus_id_state;
}
static __u32 get_vbus_pmu_state(struct usb_scan_info *info)
{
	return usb_vbus_id_state;
}

#else
static __u32 get_vbus_id_state(struct usb_scan_info *info)
{
	u32 state = 0;

	if (get_id_state(info) == USB_DEVICE_MODE)
		x_set_bit(state, 0);

	if (get_detect_vbus_state(info) == USB_DET_VBUS_VALID)
		x_set_bit(state, 1);

	return state;
}

static __u32 get_vbus_pmu_state(struct usb_scan_info *info)
{
	u32 state = 0;

	if (get_pmu_state(info) == USB_DEVICE_MODE)
		x_set_bit(state, 0);

	if (get_detect_vbus_state(info) == USB_DET_VBUS_VALID)
		x_set_bit(state, 1);

	return state;
}

#endif

static void vbus_id_hw_scan(struct usb_scan_info *info)
{
	__u32 vbus_id_state = 0;

	vbus_id_state = get_vbus_id_state(info);

	if (usb_hw_scan_debug)
		DMSG_INFO("Id=%d,role=%d\n", vbus_id_state, get_usb_role());

	switch (vbus_id_state) {
	case  0x00:
		do_vbus0_id0(info);
		break;
	case  0x01:
		do_vbus0_id1(info);
		break;
	case  0x02:
		do_vbus1_id0(info);
		break;
	case  0x03:
		do_vbus1_id1(info);
		break;
	default:
		DMSG_ERR("ERR: vbus_id_hw_scan: ");
		DMSG_ERR("unknown vbus_id_state(0x%x)\n",
			vbus_id_state);
	}
}

static void vbus_pmu_hw_scan(struct usb_scan_info *info)
{
	__u32 vbus_id_state = 0;

	vbus_id_state = get_vbus_pmu_state(info);

	if (usb_hw_scan_debug)
		DMSG_INFO("Id=%d,role=%d\n", vbus_id_state, get_usb_role());

	switch (vbus_id_state) {
	case  0x00:
		do_vbus0_id0(info);
		break;
	case  0x01:
		do_vbus0_id1(info);
		break;
	case  0x02:
		do_vbus1_id0(info);
		break;
	case  0x03:
		do_vbus1_id1(info);
		break;
	default:
		DMSG_ERR("ERR: vbus_pmu_hw_scan: ");
		DMSG_ERR("unknown vbus_id_state(0x%x)\n",
			vbus_id_state);
	}
}

static void null_hw_scan(struct usb_scan_info *info)
{
	DMSG_DBG_MANAGER("null_hw_scan\n");
}

void usb_hw_scan(struct usb_cfg *cfg)
{
	__usb_hw_scan(&g_usb_scan_info);
}

__s32 usb_hw_scan_init(struct usb_cfg *cfg)
{
	struct usb_scan_info *scan_info = &g_usb_scan_info;
	struct usb_port_info *port_info = NULL;
	__s32 ret = 0;

	memset(scan_info, 0, sizeof(struct usb_scan_info));
	device_insmod_delay = 0;
	scan_info->cfg			= cfg;
	scan_info->id_old_state		= USB_DEVICE_MODE;
	scan_info->det_vbus_old_state	= USB_DET_VBUS_INVALID;

	port_info = &(cfg->port);
	switch (port_info->port_type) {
	case USB_PORT_TYPE_DEVICE:
		__usb_hw_scan = null_hw_scan;
		break;
	case USB_PORT_TYPE_HOST:
		__usb_hw_scan = null_hw_scan;
		break;
	case USB_PORT_TYPE_OTG:
#ifdef SUNXI_USB_FPGA
	{
		if (port_info->detect_type == USB_DETECT_TYPE_VBUS_ID)
			__usb_hw_scan = vbus_id_hw_scan;
		else if (port_info->detect_type == USB_DETECT_TYPE_VBUS_PMU)
			__usb_hw_scan = vbus_pmu_hw_scan;
	}
#else
	{
		if (port_info->detect_mode == USB_DETECT_MODE_THREAD) {
			__u32 need_det_vbus_req_gpio = 1;

			if (port_info->det_vbus_type == USB_DET_VBUS_TYPE_GPIO
				&& port_info->id_type == USB_ID_TYPE_GPIO) {
				if ((port_info->id.valid == 0) || (port_info->det_vbus.valid == 0)) {
					DMSG_ERR("ERR: usb detect tpye is vbus/id, but id(%d)/vbus(%d) is invalid\n",
						port_info->id.valid,
						port_info->det_vbus.valid);
					ret = -1;
					port_info->id.valid = 0;
					port_info->det_vbus.valid = 0;
					goto failed;
				}

				/**
				 * if id and vbus use the same pin,
				 * then need not to pull pio.
				 */
				if (port_info->id.gpio_set.gpio == port_info->det_vbus.gpio_set.gpio) {
					/**
					 * when id and det_vbus reuse,
					 * the det bus need not request
					 * gpio again.
					 */
					need_det_vbus_req_gpio = 0;
				}
			}

			/* request id gpio */
			if (port_info->id.valid) {
				ret = gpio_request(
						port_info->id.gpio_set.gpio,
						"otg_id");
				if (ret != 0) {
					DMSG_ERR("ERR: id gpio_request failed\n");
					ret = -1;
					port_info->id.valid = 0;
					port_info->det_vbus.valid = 0;
					goto failed;
				}

				/* set config, input */
				gpio_direction_input(
						port_info->id.gpio_set.gpio);
				gpio_set_value(
						port_info->id.gpio_set.gpio,
						1);
			}

			/* request det_vbus gpio */
			if (port_info->det_vbus.valid
					&& need_det_vbus_req_gpio) {
				ret = gpio_request(port_info->det_vbus.gpio_set.gpio, "otg_det");
				if (ret != 0) {
					DMSG_ERR("ERR: det_vbus gpio_request failed\n");
					ret = -1;
					port_info->det_vbus.valid = 0;
					goto failed;
				}

				/* set config, input */
				gpio_direction_input(port_info->det_vbus.gpio_set.gpio);
			}
		} else if (port_info->detect_mode == USB_DETECT_MODE_INTR) {
			if (port_info->id.valid) {
				char pin_name[SUNXI_PIN_NAME_MAX_LEN];

				ret = gpio_request(
						port_info->id.gpio_set.gpio,
						"otg_id");
				if (ret != 0) {
					DMSG_ERR("ERR: id gpio_request failed\n");
					ret = -1;
					port_info->id.valid = 0;
					port_info->det_vbus.valid = 0;
					goto failed;
				}

				sunxi_gpio_to_name(
						port_info->id.gpio_set.gpio,
						pin_name);

				/* set id gpio pull up */
			}
		}

		if (port_info->detect_type == USB_DETECT_TYPE_VBUS_ID)
			__usb_hw_scan = vbus_id_hw_scan;
		else if (port_info->detect_type == USB_DETECT_TYPE_VBUS_PMU)
			__usb_hw_scan = vbus_pmu_hw_scan;
	}
#endif
		break;
	default:
		DMSG_ERR("ERR: unknown port_type(%d)\n", cfg->port.port_type);
		ret = -1;
		goto failed;
	}

	return 0;

failed:
#ifndef SUNXI_USB_FPGA
	if (port_info->id.valid)
		gpio_free(port_info->id.gpio_set.gpio);

	if (port_info->det_vbus.valid)
		gpio_free(port_info->det_vbus.gpio_set.gpio);
#endif
	__usb_hw_scan = null_hw_scan;
	return ret;
}

#ifdef SUNXI_USB_FPGA
__s32 usb_hw_scan_exit(struct usb_cfg *cfg)
{
	return 0;
}
#else
__s32 usb_hw_scan_exit(struct usb_cfg *cfg)
{
	if (cfg->port.id.valid)
		gpio_free(cfg->port.id.gpio_set.gpio);

	if (cfg->port.det_vbus.valid)
		gpio_free(cfg->port.det_vbus.gpio_set.gpio);

	return 0;
}
#endif
