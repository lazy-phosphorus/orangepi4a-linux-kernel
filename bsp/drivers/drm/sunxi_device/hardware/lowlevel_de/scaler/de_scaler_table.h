/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Allwinner SoCs display driver.
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

/**
 *	All Winner Tech, All Right Reserved. 2014-2015 Copyright (c)
 *
 *	File name   :       de_scaler_table.h
 *
 *	Description :       display engine 2.0 vsu/gsu common fir table
 *	                    information
 *
 *	History     :       2014/03/26  vito cheng  v0.1  Initial version
 *
 */

#ifndef _DE_SCALER_TABLE_H_
#define _DE_SCALER_TABLE_H_

#include "linux/types.h"

#define VSU_ZOOM0_SIZE	1
#define VSU_ZOOM1_SIZE	8
#define VSU_ZOOM2_SIZE	4
#define VSU_ZOOM3_SIZE	1
#define VSU_ZOOM4_SIZE	1
#define VSU_ZOOM5_SIZE	1

#define VSU_PHASE_NUM 32
#define GSU_PHASE_NUM 16

extern u32 lan2coefftab16[256];
extern u32 lan3coefftab32_left[512];
extern u32 lan3coefftab32_right[512];
extern u32 lan2coefftab32[512];
extern u32 bicubic8coefftab32_left[512];
extern u32 bicubic8coefftab32_right[512];
extern u32 bicubic4coefftab32[512];
extern u32 linearcoefftab32_4tab[32];

#endif /* #ifndef _DE_SCALER_TABLE_H_ */
