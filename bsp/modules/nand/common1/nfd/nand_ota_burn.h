/*
 * nand_ota_burn.h for  SUNXI NAND .
 *
 * Copyright (C) 2016 Allwinner.
 *
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */
#ifndef _NAND_OTA_BURN_H_
#define _NAND_OTA_BURN_H_

#include <asm/cacheflush.h>
#include <linux/blkdev.h>
#include <linux/blkpg.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/hdreg.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <sunxi-boot.h>
#include <linux/timer.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/wait.h>
//#include "nand_lib.h"

extern int nand_print(const char *fmt, ...);
extern int nand_physic_lock(void);
extern int nand_physic_unlock(void);

extern int nand_get_param(void *nand_param);
extern int nand_get_param_for_uboottail(void *nand_param);

extern int nand_read_nboot_data(unsigned char *buf, unsigned int len);
extern int nand_read_uboot_data(unsigned char *buf, unsigned int len);
extern int nand_write_nboot_data(unsigned char *buf, unsigned int len);
extern int nand_write_uboot_data(unsigned char *buf, unsigned int len);
extern int nand_dragonborad_test_one(unsigned char *buf, unsigned char *oob,
				     unsigned int blk_num);
extern int nand_is_secure_sys(void);
extern int nand_check_uboot(unsigned char *buf, unsigned int len);
extern int nand_get_uboot_total_len(void);

int nand_check_boot(void);
__s32 nand_dragon_board_test(void);

#endif
