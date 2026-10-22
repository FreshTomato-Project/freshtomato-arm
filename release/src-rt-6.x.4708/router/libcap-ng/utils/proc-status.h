/*
 * proc-status.h - Shared /proc/<pid>/status helpers
 * Copyright (c) 2026 Red Hat Inc.
 * All Rights Reserved.
 *
 * This software may be freely redistributed and/or modified under the
 * terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2, or (at your option) any
 * later version.
 */

#ifndef PROC_STATUS_H
#define PROC_STATUS_H

#include <sys/types.h>
#include "gcc-attributes.h"

#define PROC_STATUS_NAME_LEN 64

struct proc_status {
	char name[PROC_STATUS_NAME_LEN];
	uid_t uid;
	uid_t euid;
	unsigned long no_new_privs;
	unsigned long seccomp;
	int seen_name;
	int seen_uid;
	int seen_euid;
	int seen_no_new_privs;
	int seen_seccomp;
};

int proc_read_status(pid_t pid, struct proc_status *status)
	__attr_access ((__write_only__, 2))
	__wur;

#endif
