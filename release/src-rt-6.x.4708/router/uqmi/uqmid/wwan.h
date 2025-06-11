/*
 * uqmid -- implement a daemon based on the uqmi idea
 *
 * Copyright (C) 2023-2024 Alexander Couzens <lynxis@fe80.eu>
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

#include <stdbool.h>
#include <stddef.h>

struct modem;
struct wwan_conf;

int wwan_refresh_device(struct modem *modem);
int wwan_read_configuration(const char *sysfs_path, struct wwan_conf *config);
int wwan_set_configuration(const char *sysfs_path, const struct wwan_conf *config);
int wwan_ifupdown(const char *netif, bool up);
int wwan_set_mtu(const char *netif, unsigned int mtu);
