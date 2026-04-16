// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * cap-audit - Trace a target process to discover required capabilities.
 * Copyright (c) 2026 Red Hat Inc.
 * All Rights Reserved.
 *
 * This software may be freely redistributed and/or modified under the
 * terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Authors:
 *   Steve Grubb <sgrubb@redhat.com>
 *   Portions of this code were made with codex 5.2
 */

/* Ring buffer event processing: noise filtering, shutdown backstop,
 * and per-capability accounting for observed checks.
 */

#include "cap_audit.h"

#include <stdio.h>
#include <stdlib.h>

static int is_always_noise(const struct cap_event *e)
{
	if (e->syscall_nr == state.app.execve_nr &&
	   (e->capability == CAP_SYS_ADMIN ||
	    e->capability == CAP_SETPCAP))
		return 1;

	if ((e->cap_opts & CAP_OPT_NOAUDIT) &&
	    e->capability == CAP_SYS_ADMIN &&
	    (e->syscall_nr == state.app.brk_nr ||
	     e->syscall_nr == state.app.mmap_nr ||
	     e->syscall_nr == state.app.mprotect_nr ||
	     e->syscall_nr == state.app.mremap_nr))
		return 1;

	return 0;
}

static int is_shutdown_noise(const struct cap_event *e)
{
	if (!state.shutting_down)
		return 0;

	if (e->pid != (__u32)state.app.pid)
		return 0;

	if (e->capability == CAP_SYS_ADMIN ||
	    e->capability == CAP_SETPCAP)
		return 1;

	return 0;
}

static void add_denied_syscall(struct cap_check *check, int syscall_nr)
{
	size_t i;
	int *tmp;
	size_t new_cap;

	for (i = 0; i < check->denied_syscall_count; i++) {
		if (check->denied_syscalls[i] == syscall_nr)
			return;
	}

	if (check->denied_syscall_count == check->denied_syscall_capacity) {
		new_cap = check->denied_syscall_capacity ?
			  check->denied_syscall_capacity * 2 : 4;
		tmp = realloc(check->denied_syscalls, new_cap * sizeof(int));
		if (!tmp)
			return;
		check->denied_syscalls = tmp;
		check->denied_syscall_capacity = new_cap;
	}

	check->denied_syscalls[check->denied_syscall_count++] = syscall_nr;
}

int handle_cap_event(void *ctx __attribute__((unused)), void *data,
		     size_t data_sz __attribute__((unused)))
{
	const struct cap_event *e = data;
	int op_phase = state.capset_observed;

	if (is_always_noise(e)) {
		if (state.verbose) {
			if (e->syscall_nr == state.app.execve_nr)
				printf("[CAP] Filtered exec noise: "
				       "cap=%s syscall=%s\n",
				       cap_name_safe(e->capability),
				       syscall_name_from_nr(e->syscall_nr) ?:
				       "unknown");
			else
				printf("[CAP] Filtered advisory check: "
				       "cap=%s syscall=%s (CAP_OPT_NOAUDIT)\n",
				       cap_name_safe(e->capability),
				       syscall_name_from_nr(e->syscall_nr) ?:
				       "unknown");
		}
		return 0;
	}

	if (is_shutdown_noise(e)) {
		if (state.verbose)
			printf("[CAP] Filtered shutdown noise: "
			       "cap=%s syscall=%s\n",
			       cap_name_safe(e->capability),
			       syscall_name_from_nr(e->syscall_nr) ?:
			       "unknown");
		return 0;
	}

	if (state.verbose) {
		printf("[CAP] pid=%d cap=%s result=%s syscall=%s "
		       "comm=%s\n",
		       e->pid, cap_name_safe(e->capability),
		       e->result ? "GRANTED" : "DENIED",
		       syscall_name_from_nr(e->syscall_nr) ?: "unknown",
		       e->comm);
	}

	if (!state.capset_observed &&
	    e->syscall_nr == state.app.capset_nr &&
	    e->pid == (__u32)state.app.pid) {
		state.capset_observed = 1;
		if (state.verbose)
			printf("[CAP] Capability drop detected (capset from "
			       "initial PID); switching to operational "
			       "phase\n");
	}

	if (e->capability >= 0 && e->capability <= CAP_LAST_CAP) {
		struct cap_check *check;

		check = &state.app.checks[e->capability];
		check->capability = e->capability;

		if (op_phase) {
			check->op_count++;
			if (e->result > 0)
				check->op_granted++;
			else if (e->result == 0) {
				check->op_denied++;
				add_denied_syscall(check, e->syscall_nr);
			}
			if (e->result > 0 && check->op_needed != 1) {
				check->op_needed = 1;
				update_reason_op(check, e->syscall_nr);
			}
		} else {
			check->count++;
			if (e->result > 0)
				check->granted++;
			else if (e->result == 0) {
				check->denied++;
				add_denied_syscall(check, e->syscall_nr);
			}

			if (e->result > 0 && check->needed != 1) {
				check->needed = 1;
				update_reason(check, e->syscall_nr);
			}
		}
	}

	return 0;
}
