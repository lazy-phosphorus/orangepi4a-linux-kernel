/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/* drv_edp_common.h
 *
 * Copyright (c) 2007-2017 Allwinnertech Co., Ltd.
 * Author: zhengxiaobin <zhengxiaobin@allwinnertech.com>
 *
 * edp driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#ifndef _DRV_EDP_COMMON_H
#define _DRV_EDP_COMMON_H

#define EDP_DEBUG_LEVEL 0
#define EDP_BYPASS_CR_TRANNING 0
#define EDP_BYPASS_EQ_TRANNING 0

#define __LINUX_PLAT__

#if defined(__LINUX_PLAT__)
#include <linux/module.h>
#include <linux/uaccess.h>
#include <asm/memory.h>
#include <asm/unistd.h>
#include "asm-generic/int-ll64.h"
#include "linux/kernel.h"
#include "linux/mm.h"
#include "linux/semaphore.h"
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/dma-mapping.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/of_iommu.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/compat.h>
#include <uapi/video/sunxi_display2.h>


#define edp_wrn(fmt, ...)                                                     \
	pr_warn("[EDP] %s:%d " fmt "", __func__, __LINE__, ##__VA_ARGS__)

#if EDP_DEBUG_LEVEL == 2
#define edp_here pr_warn("[EDP] %s:%d\n", __func__, __LINE__)
#else
#define edp_here
#endif /* endif EDP_DEBUG */

#if EDP_DEBUG_LEVEL >= 1
#define edp_dbg(fmt, ...)                                                     \
	pr_warn("[EDP] %s:%d " fmt "", __func__, __LINE__, ##__VA_ARGS__)
#else
#define edp_dbg(fmt, ...)
#endif /* endif EDP_DEBUG */


#else
#include <common.h>
#include <malloc.h>
#include <sunxi_display2.h>
#include <sys_config.h>
#include <asm/arch/intc.h>
#include <pwm.h>
#include <asm/arch/timer.h>
#include <asm/arch/platform.h>
#include <linux/list.h>
#include <asm/memory.h>
#include <div64.h>
#include <fdt_support.h>
#include <power/sunxi/pmu.h>
#include "asm/io.h"
#include "../disp_sys_intf.h"

#define edp_wrn(fmt, ...)                                                     \
	printf("[EDP] %s:%d " fmt "", __func__, __LINE__, ##__VA_ARGS__)

#if EDP_DEBUG_LEVEL == 2
#define edp_here printf("[EDP] %s:%d\n", __func__, __LINE__)
#else
#define edp_here
#endif /* endif EDP_DEBUG */

#if EDP_DEBUG_LEVEL >= 1
#define edp_dbg(fmt, ...)                                                     \
	printf("[EDP] %s:%d " fmt "", __func__, __LINE__, ##__VA_ARGS__)
#else
#define edp_dbg(fmt, ...)
#endif /* endif EDP_DEBUG */
#endif

s32 edp_delay_ms(u32 ms);
s32 edp_delay_us(u32 us);


#endif /* End of file */
