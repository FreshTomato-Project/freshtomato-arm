/*
 * timer.h a shim layer around osmo timer to use libubox/uloop timers
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

#pragma once

#include <libubox/uloop.h>

/* add a simple layer struct to allow to support the *data pointer in libubox */
struct pipe_timer {
	struct uloop_timeout timeout;
	void (*cb)(void*);
	void *data;
};

/* struct osmo_timer_list -> struct uloop_timeout */
#define osmo_timer_list pipe_timer

void osmo_timer_setup(struct pipe_timer *timer, void (*cb)(void *data), void *data);
void osmo_timer_add(struct pipe_timer *timer);
void osmo_timer_schedule(struct pipe_timer *timer, int seconds, int microseconds);
void osmo_timer_del(struct pipe_timer *timer);
int osmo_timer_pending(const struct pipe_timer *timer);
int osmo_timer_remaining(struct pipe_timer *timer,
             const struct timeval *now,
             struct timeval *remaining);
