/*
 * timer.c a shim layer around osmo timer to use libubox/uloop timers
 *
 * Copyright (C) 2023 Alexander Couzens <lynxis@fe80.eu>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */


#include <limits.h>

#include "timer.h"

static void osmo_timer_call_callback(struct uloop_timeout *timeout)
{
	struct pipe_timer *timer = container_of(timeout, struct pipe_timer, timeout);

	if (timer->cb)
		timer->cb(timer->data);
}

void osmo_timer_setup(struct pipe_timer *timer, void (*cb)(void *data), void *data)
{
	timer->timeout.cb = osmo_timer_call_callback;
	timer->cb = cb;
	timer->data = data;
}

void osmo_timer_add(struct pipe_timer *timer)
{
	uloop_timeout_add(&timer->timeout);
}

void osmo_timer_schedule(struct pipe_timer *timer, int seconds, int microseconds)
{
	int msecs = seconds * 1000 + microseconds / 1000;
	if (msecs == 0 && microseconds > 0)
		msecs = 1;

	uloop_timeout_set(&timer->timeout, msecs);
}

void osmo_timer_del(struct pipe_timer *timer)
{
	uloop_timeout_cancel(&timer->timeout);
}

int osmo_timer_pending(const struct pipe_timer *timer)
{
	return timer->timeout.pending;
}

int osmo_timer_remaining(struct pipe_timer *timer,
             const struct timeval *now,
             struct timeval *remaining)
{
	int64_t remain = uloop_timeout_remaining64(&timer->timeout);
	if (remain > INT_MAX)
		return INT_MAX;
	
	return remain;
}
