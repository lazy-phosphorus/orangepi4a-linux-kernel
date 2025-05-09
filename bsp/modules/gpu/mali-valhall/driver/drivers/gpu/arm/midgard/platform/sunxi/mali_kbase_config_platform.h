/*
 *
 * (C) COPYRIGHT 2014-2017 ARM Limited. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * SPDX-License-Identifier: GPL-2.0
 *
 */

/**
 * Power management configuration
 *
 * Attached value: pointer to @ref kbase_pm_callback_conf
 * Default value: See @ref kbase_pm_callback_conf
 */
extern struct kbase_pm_callback_conf sunxi_pm_callbacks;
#define POWER_MANAGEMENT_CALLBACKS (&sunxi_pm_callbacks)

/**
 * Platform specific configuration functions
 *
 * Attached value: pointer to @ref kbase_platform_funcs_conf
 * Default value: See @ref kbase_platform_funcs_conf
 */
#define PLATFORM_FUNCS (&sunxi_platform_conf)

extern struct kbase_platform_funcs_conf sunxi_platform_conf;

#define PLATFORM_PROTECTED_CALLBACKS (&sunxi_protected_ops)

extern struct protected_mode_ops sunxi_protected_ops;
/**
 * AUTO_SUSPEND_DELAY - Autosuspend delay
 *
 * The delay time (in milliseconds) to be used for autosuspend
 */
#define AUTO_SUSPEND_DELAY (100)
