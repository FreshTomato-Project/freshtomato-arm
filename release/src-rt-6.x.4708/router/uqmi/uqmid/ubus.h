/*
 * uqmid
 * Copyright (C) 2023 Alexander Couzens <lynxis@fe80.eu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef __UQMID_UBUS_H
#define __UQMID_UBUS_H

struct modem;
struct ubus_context;

extern struct ubus_context *ubus_ctx;
int uqmid_ubus_init(const char *path);

int uqmid_ubus_modem_add(struct modem *modem);
void uqmid_ubus_modem_destroy(struct modem *modem);
void uqmid_ubus_modem_notify_change(struct modem *modem, int event);

#endif /* __UQMID_UBUS_H */
