/*
 * proc-status.c - Shared /proc/<pid>/status helpers
 * Copyright (c) 2026 Red Hat Inc.
 * All Rights Reserved.
 *
 * This software may be freely redistributed and/or modified under the
 * terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2, or (at your option) any
 * later version.
 */

#include "config.h"
#include <stdio.h>
#include <stdio_ext.h>
#include <string.h>
#include "proc-status.h"

/*
 * proc_read_status - read selected fields from /proc/<pid>/status.
 * @pid: process ID whose status file should be read.
 * @status: destination structure cleared and populated on success.
 *
 * Returns 0 when the status file was opened and parsed, -1 when it could not
 * be opened.
 */
int proc_read_status(pid_t pid, struct proc_status *status)
{
	char path[64], line[256];
	FILE *f;

	memset(status, 0, sizeof(*status));
	status->uid = (uid_t)-1;
	status->euid = (uid_t)-1;

	snprintf(path, sizeof(path), "/proc/%d/status", pid);
	f = fopen(path, "rte");
	if (!f)
		return -1;

	__fsetlocking(f, FSETLOCKING_BYCALLER);
	while (fgets(line, sizeof(line), f)) {
		uid_t uid, euid;
		int fields;

		if (sscanf(line, "Name: %63s", status->name) == 1) {
			status->seen_name = 1;
			continue;
		}
		fields = sscanf(line, "Uid: %u %u", &uid, &euid);
		if (fields >= 1) {
			status->uid = uid;
			status->seen_uid = 1;
			if (fields == 2) {
				status->euid = euid;
				status->seen_euid = 1;
			}
			continue;
		}
		if (sscanf(line, "NoNewPrivs: %lu",
			   &status->no_new_privs) == 1) {
			status->seen_no_new_privs = 1;
			continue;
		}
		if (sscanf(line, "Seccomp: %lu", &status->seccomp) == 1) {
			status->seen_seccomp = 1;
			continue;
		}
	}
	fclose(f);
	return 0;
}
