/*
 * Copyright (C) 2010-2014, 2017 ARM Limited. All rights reserved.
 * 
 * This program is free software and is provided to you under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation, and any use by you of this program is subject to the terms of such GNU licence.
 * 
 * A copy of the licence is included with the program, and can also be obtained from Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/**
 * @file mali_osk_timers.c
 * Implementation of the OS abstraction layer for the kernel device driver
 */

#include <linux/timer.h>
#include <linux/slab.h>
#include "mali_osk.h"
#include "mali_kernel_common.h"

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)

#define TIMER_DATA_TYPE		unsigned long
#define TIMER_FUNC_TYPE		void (*)(TIMER_DATA_TYPE)

static inline void timer_setup(struct timer_list *timer,
		void (*callback)(struct timer_list *),
		unsigned int flags)
{
		__setup_timer(timer, (TIMER_FUNC_TYPE)callback,
				(TIMER_DATA_TYPE)timer, flags);
}
#endif

typedef void (*timer_timeout_function_t)(unsigned long);

_mali_osk_timer_t *_mali_osk_timer_init(void)
{
	_mali_osk_timer_t *t = (_mali_osk_timer_t *)kmalloc(sizeof(_mali_osk_timer_t), GFP_KERNEL);
	return t;
}

void _mali_osk_timer_add(_mali_osk_timer_t *tim, unsigned long ticks_to_expire)
{
	MALI_DEBUG_ASSERT_POINTER(tim);
	tim->timer.expires = jiffies + ticks_to_expire;
	add_timer(&(tim->timer));
}

void _mali_osk_timer_mod(_mali_osk_timer_t *tim, unsigned long ticks_to_expire)
{
	MALI_DEBUG_ASSERT_POINTER(tim);
	mod_timer(&(tim->timer), jiffies + ticks_to_expire);
}

void _mali_osk_timer_del(_mali_osk_timer_t *tim)
{
	MALI_DEBUG_ASSERT_POINTER(tim);
	del_timer_sync(&(tim->timer));
}

void _mali_osk_timer_del_async(_mali_osk_timer_t *tim)
{
	MALI_DEBUG_ASSERT_POINTER(tim);
	del_timer(&(tim->timer));
}

mali_bool _mali_osk_timer_pending(_mali_osk_timer_t *tim)
{
	MALI_DEBUG_ASSERT_POINTER(tim);
	return 1 == timer_pending(&(tim->timer));
}

void _mali_osk_timer_setcallback(_mali_osk_timer_t *tim, _mali_osk_timer_callback_t callback, void *data)
{
	MALI_DEBUG_ASSERT_POINTER(tim);
	timer_setup(&tim->timer, callback, 0);
}

void _mali_osk_timer_term(_mali_osk_timer_t *tim)
{
	MALI_DEBUG_ASSERT_POINTER(tim);
	kfree(tim);
}
