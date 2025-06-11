/*
 * uqmi -- tiny QMI support implementation
 *
 * Copyright (C) 2023 Alexander Couzens <lynxis@fe80.eu>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#ifndef __UQMID_GSMTAP_UTIL_H
#define __UQMID_GSMTAP_UTIL_H

#include <stddef.h>

struct modem;

int gsmtap_enable(const char *gsmtap_addr);
void gsmtap_disable(void);
void gsmtap_send(struct modem *modem, void *data, size_t length);

#endif /* __UQMID_GSMTAP_UTIL_H */
