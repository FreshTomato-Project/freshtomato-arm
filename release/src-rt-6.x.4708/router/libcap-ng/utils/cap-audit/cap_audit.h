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

#ifndef CAP_AUDIT_H
#define CAP_AUDIT_H

#include "config.h"

#include <bpf/libbpf.h>
#include <linux/capability.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>

#include "cap-ng.h"
#include "cap_audit.skel.h"

#ifndef CAP_OPT_NOAUDIT
#define CAP_OPT_NOAUDIT 0x2
#endif

#define ELFMAGIC "\177ELF"

typedef enum { UNSUPPORTED, ELF, PYTHON } type_t;

struct cap_event {
	__u64 timestamp_ns;
	__u32 pid;
	__u32 tid;
	__u32 uid;
	__u32 gid;
	int capability;
	int result;
	int syscall_nr;
	char comm[16];
	__u64 stack_id;
	__u32 targ_ns_inum;
	__u32 cap_opts;
};

struct cap_check {
	int capability;
	unsigned long count;
	unsigned long granted;
	unsigned long denied;
	int needed;
	char *reason;
	unsigned long op_count;
	unsigned long op_granted;
	unsigned long op_denied;
	int op_needed;
	char *op_reason;
	int *denied_syscalls;
	size_t denied_syscall_count;
	size_t denied_syscall_capacity;
};

struct app_caps {
	pid_t pid;
	char *exe;
	int execve_nr;
	int mmap_nr;
	int brk_nr;
	int mprotect_nr;
	int mremap_nr;
	int capset_nr;
	type_t prog_type;
	struct cap_check checks[CAP_LAST_CAP + 1];
	int yama_ptrace_scope;
	int kptr_restrict;
	int dmesg_restrict;
	int modules_disabled;
	int perf_event_paranoid;
	int unprivileged_bpf_disabled;
	int bpf_jit_enable;
	int bpf_jit_harden;
	int bpf_jit_kallsyms;
	int mmap_min_addr;
	int protected_hardlinks;
	int protected_symlinks;
	int suid_dumpable;
	char kernel_version[64];
	int file_caps;
	int file_setpcap;
};

struct audit_state {
	struct cap_audit_bpf *skel;
	struct ring_buffer *rb;
	struct app_caps app;
	int verbose;
	int json_output;
	int yaml_output;
	int sync_pipe[2];
	char **target_argv;
	int capset_observed;
	volatile sig_atomic_t stop;
	int shutting_down;
};

extern struct audit_state state;
extern int audit_machine;

int handle_cap_event(void *ctx, void *data, size_t data_sz);
void analyze_capabilities(void);
void output_json(void);
void output_yaml(void);
int include_cap_in_recommendations(int cap);
const char *cap_name_safe(int cap);
const char *syscall_name_from_nr(int nr);
void read_sysctl(const char *path, int *value);
void read_system_state(struct app_caps *app);
int resolve_target_exe(pid_t pid, char *exepath, size_t exepath_len);
int inspect_target_file_caps(pid_t pid);
char *json_escape(const char *input);
void update_reason_to(char **target, int syscall_nr);
void update_reason(struct cap_check *check, int syscall_nr);
void update_reason_op(struct cap_check *check, int syscall_nr);
int cap_required_union(const struct cap_check *check);
unsigned long cap_total_checks(const struct cap_check *check);
unsigned long cap_total_granted(const struct cap_check *check);
unsigned long cap_total_denied(const struct cap_check *check);
const char *cap_union_reason(const struct cap_check *check);
type_t classify_app(const char *exe);

#endif
