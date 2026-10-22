/*
 * proc-account.h - Shared account name helpers for proc-based utilities
 * Copyright (c) 2026 Red Hat Inc.
 * All Rights Reserved.
 *
 * This software may be freely redistributed and/or modified under the
 * terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2, or (at your option) any
 * later version.
 */

#ifndef PROC_ACCOUNT_H
#define PROC_ACCOUNT_H

#include <stddef.h>
#include <sys/types.h>
#include "gcc-attributes.h"

void proc_format_account_name_from_euid(uid_t euid, char *account,
					size_t account_len)
	__attr_access ((__write_only__, 2, 3));
void proc_update_account_cache(uid_t uid, uid_t *last_uid,
			       const char **cached_name)
	__attr_access ((__read_write__, 2))
	__attr_access ((__write_only__, 3));

#endif
