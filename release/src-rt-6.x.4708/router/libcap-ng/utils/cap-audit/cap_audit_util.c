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

/* Shared utility helpers: target inspection, syscall/cap name lookup,
 * output escaping, and common capability aggregation functions.
 */

#include "cap_audit.h"

#include <errno.h>
#include <fcntl.h>
#include <libaudit.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int resolve_target_exe(pid_t pid, char *exepath, size_t exepath_len)
{
	char linkpath[64];
	char selfpath[PATH_MAX];
	ssize_t len;
	ssize_t self_len;
	int tries = 50;

	if (snprintf(linkpath, sizeof(linkpath), "/proc/%d/exe", pid) < 0)
		return -1;

	self_len = readlink("/proc/self/exe", selfpath, sizeof(selfpath) - 1);
	if (self_len >= 0)
		selfpath[self_len] = '\0';

	while (tries--) {
		len = readlink(linkpath, exepath, exepath_len - 1);
		if (len < 0) {
			fprintf(stderr, "Warning: readlink(%s) failed: %s\n",
				linkpath, strerror(errno));
			return -1;
		}
		exepath[len] = '\0';

		if (self_len < 0 || strcmp(exepath, selfpath) != 0)
			break;

		if (tries == 0)
			fprintf(stderr,
				"Warning: %s still points to auditor binary (%s)\n",
				linkpath, exepath);
		usleep(10000);
	}

	return 0;
}

int inspect_target_file_caps(pid_t pid)
{
	char exepath[PATH_MAX];
	int fd;
	struct stat st;
	capng_results_t caps;

	state.app.file_caps = 0;
	state.app.file_setpcap = 0;

	if (resolve_target_exe(pid, exepath, sizeof(exepath)) < 0)
		return -1;

	fd = open(exepath, O_RDONLY | O_CLOEXEC);
	if (fd < 0) {
		fprintf(stderr, "Warning: open(%s) failed: %s\n",
			exepath, strerror(errno));
		return -1;
	}

	if (fstat(fd, &st) < 0) {
		fprintf(stderr, "Warning: fstat(%s) failed: %s\n",
			exepath, strerror(errno));
		close(fd);
		return -1;
	}
	if (!S_ISREG(st.st_mode)) {
		fprintf(stderr, "Warning: %s is not a regular file\n",
			exepath);
		close(fd);
		return -1;
	}

	capng_clear(CAPNG_SELECT_BOTH);
	if (capng_get_caps_fd(fd)) {
		if (errno != ENODATA)
			fprintf(stderr,
				"Warning: capng_get_caps_fd(%s) failed: %s\n",
				exepath, strerror(errno));
		close(fd);
		if (capng_get_caps_process())
			fprintf(stderr,
				"Warning: failed to restore process capabilities\n");
		return -1;
	}
	close(fd);

	caps = capng_have_capabilities(CAPNG_SELECT_CAPS);
	if (caps == CAPNG_NONE)
		caps = capng_have_permitted_capabilities();
	if (caps > CAPNG_NONE)
		state.app.file_caps = 1;

	if (capng_have_capability(CAPNG_PERMITTED, CAP_SETPCAP) ||
	    capng_have_capability(CAPNG_INHERITABLE, CAP_SETPCAP))
		state.app.file_setpcap = 1;

	if (state.verbose)
		printf("[*] File caps source: %s (has_caps=%d setpcap=%d)\n",
		       exepath, state.app.file_caps, state.app.file_setpcap);

	if (capng_get_caps_process())
		fprintf(stderr, "Warning: failed to restore process capabilities\n");

	return 0;
}

const char *cap_name_safe(int cap)
{
	const char *name = capng_capability_to_name(cap);

	return name ? name : "unknown";
}

void read_sysctl(const char *path, int *value)
{
	FILE *f;

	f = fopen(path, "r");
	if (f) {
		if (fscanf(f, "%d", value) != 1)
			*value = -1;
		fclose(f);
	} else {
		*value = -1;
	}
}

void read_system_state(struct app_caps *app)
{
	FILE *f;

	read_sysctl("/proc/sys/kernel/yama/ptrace_scope",
		    &app->yama_ptrace_scope);
	read_sysctl("/proc/sys/kernel/kptr_restrict", &app->kptr_restrict);
	read_sysctl("/proc/sys/kernel/dmesg_restrict", &app->dmesg_restrict);
	read_sysctl("/proc/sys/kernel/modules_disabled",
		    &app->modules_disabled);
	read_sysctl("/proc/sys/kernel/perf_event_paranoid",
		    &app->perf_event_paranoid);
	read_sysctl("/proc/sys/kernel/unprivileged_bpf_disabled",
		    &app->unprivileged_bpf_disabled);
	read_sysctl("/proc/sys/net/core/bpf_jit_enable", &app->bpf_jit_enable);
	read_sysctl("/proc/sys/net/core/bpf_jit_harden", &app->bpf_jit_harden);
	read_sysctl("/proc/sys/net/core/bpf_jit_kallsyms",
		    &app->bpf_jit_kallsyms);
	read_sysctl("/proc/sys/vm/mmap_min_addr", &app->mmap_min_addr);
	read_sysctl("/proc/sys/fs/protected_hardlinks",
		    &app->protected_hardlinks);
	read_sysctl("/proc/sys/fs/protected_symlinks",
		    &app->protected_symlinks);
	read_sysctl("/proc/sys/fs/suid_dumpable", &app->suid_dumpable);

	f = fopen("/proc/sys/kernel/osrelease", "r");
	if (f) {
		if (!fgets(app->kernel_version, sizeof(app->kernel_version), f))
			app->kernel_version[0] = 0;
		app->kernel_version[strcspn(app->kernel_version, "\n")] = 0;
		fclose(f);
	}
}

const char *syscall_name_from_nr(int nr)
{
	if (audit_machine < 0)
		return NULL;

	return audit_syscall_to_name(nr, audit_machine);
}

char *json_escape(const char *input)
{
	size_t i;
	size_t needed = 0;
	char *out;
	char *pos;

	if (!input)
		return strdup("");

	for (i = 0; input[i]; i++) {
		unsigned char c = input[i];

		switch (c) {
		case '\"':
		case '\\':
		case '\b':
		case '\f':
		case '\n':
		case '\r':
		case '\t':
			needed += 2;
			break;
		default:
			if (c < 0x20)
				needed += 6;
			else
				needed++;
			break;
		}
	}

	out = malloc(needed + 1);
	if (!out)
		return NULL;

	pos = out;
	for (i = 0; input[i]; i++) {
		unsigned char c = input[i];

		switch (c) {
		case '\"':
			*pos++ = '\\';
			*pos++ = '\"';
			break;
		case '\\':
			*pos++ = '\\';
			*pos++ = '\\';
			break;
		case '\b':
			*pos++ = '\\';
			*pos++ = 'b';
			break;
		case '\f':
			*pos++ = '\\';
			*pos++ = 'f';
			break;
		case '\n':
			*pos++ = '\\';
			*pos++ = 'n';
			break;
		case '\r':
			*pos++ = '\\';
			*pos++ = 'r';
			break;
		case '\t':
			*pos++ = '\\';
			*pos++ = 't';
			break;
		default:
			if (c < 0x20) {
				snprintf(pos, 7, "\\u%04x", c);
				pos += 6;
			} else {
				*pos++ = c;
			}
			break;
		}
	}
	*pos = '\0';

	return out;
}

void update_reason_to(char **target, int syscall_nr)
{
	const char *syscall_name;

	if (*target)
		free(*target);

	if (syscall_nr < 0) {
		if (asprintf(target,
			     "Used during capability check (syscall unknown)") < 0)
			*target = NULL;
		return;
	}

	syscall_name = syscall_name_from_nr(syscall_nr);
	if (asprintf(target, "Used by %s",
		     syscall_name ? syscall_name : "unknown") < 0)
		*target = NULL;
}

void update_reason(struct cap_check *check, int syscall_nr)
{
	update_reason_to(&check->reason, syscall_nr);
}

void update_reason_op(struct cap_check *check, int syscall_nr)
{
	update_reason_to(&check->op_reason, syscall_nr);
}

int cap_required_union(const struct cap_check *check)
{
	return check->granted > 0 || check->op_granted > 0;
}

unsigned long cap_total_checks(const struct cap_check *check)
{
	return check->count + check->op_count;
}

unsigned long cap_total_granted(const struct cap_check *check)
{
	return check->granted + check->op_granted;
}

unsigned long cap_total_denied(const struct cap_check *check)
{
	return check->denied + check->op_denied;
}

const char *cap_union_reason(const struct cap_check *check)
{
	if (check->reason)
		return check->reason;
	return check->op_reason;
}

type_t classify_app(const char *exe)
{
	int fd;
	ssize_t rc;
	char buf[257];

	fd = open(exe, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Cannot open %s - %s\n", exe, strerror(errno));
		exit(1);
	}

	rc = read(fd, buf, 256);
	close(fd);
	if (rc > 0) {
		buf[rc] = 0;
		if (buf[0] == '#' && buf[1] == '!') {
			char *ptr = strchr(buf, '\n');

			if (ptr)
				*ptr = 0;
			if (strstr(buf, "python"))
				return PYTHON;
		} else if (strncmp(buf, ELFMAGIC, 4) == 0)
			return ELF;
	}

	return UNSUPPORTED;
}
