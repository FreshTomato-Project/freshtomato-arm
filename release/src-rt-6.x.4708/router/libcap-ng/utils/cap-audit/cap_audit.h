// SPDX-License-Identifier: GPL-2.0-or-later
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
#include <stdbool.h>
#include <linux/capability.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>

#include "cap-ng.h"
#include "classify_app.h"
#include "cap_audit.skel.h"
#include "gcc-attributes.h"

#ifndef CAP_OPT_NOAUDIT
#define CAP_OPT_NOAUDIT 0x2
#endif

enum cap_event_type {
	CAP_EVENT_CHECK,
	CAP_EVENT_SYSCALL_RESULT,
	CAP_EVENT_CAPSET,
	CAP_EVENT_KEEPCAPS,
	CAP_EVENT_TASK_END,
};

struct cap_event {
	__u32 pid;
	int capability;
	int result;
	int syscall_nr;
	char comm[16];
	__u32 targ_ns_inum;
	__u32 cap_opts;
	__s64 syscall_ret;
	__u64 denied_caps;
	__u64 capset_effective;
	__u64 capset_permitted;
	__u64 capset_inheritable;
	__u32 event_type;
	/* Kernel capset arguments proved this SETPCAP check optional. */
	__u32 capset_inh_optional;
	__u32 tid;
	__u32 keepcaps;
};

enum syscall_outcome_class {
	SYSCALL_OUTCOME_SUCCESS,
	SYSCALL_OUTCOME_PERMISSION,
	SYSCALL_OUTCOME_OTHER,
	SYSCALL_OUTCOME_INTERRUPTED,
};

enum denial_assessment {
	DENIAL_ASSESSMENT_NONE,
	DENIAL_ASSESSMENT_PERMISSION,
	DENIAL_ASSESSMENT_MIXED_INTERRUPTED,
	DENIAL_ASSESSMENT_MIXED_OTHER,
	DENIAL_ASSESSMENT_INCONCLUSIVE,
	DENIAL_ASSESSMENT_SUCCEEDED,
	DENIAL_ASSESSMENT_OTHER,
};

struct syscall_outcome {
	int syscall_nr;
	__s64 result;
	unsigned long count;
};

struct cap_outcome_summary {
	unsigned long succeeded;
	unsigned long permission_failed;
	unsigned long other_failed;
	unsigned long interrupted;
};

struct capset_usage {
	__u64 effective;
	__u64 permitted;
	__u64 inheritable;
	unsigned long successful_calls;
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
	struct syscall_outcome *outcomes;
	size_t outcome_count;
	size_t outcome_capacity;
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
	struct capset_usage capset;
};

typedef struct {
	bool seen;
	bool caps[CAP_LAST_CAP + 1];
} cap_set_t;

typedef struct {
	gid_t *gids;
	size_t count;
	bool is_set;
} gid_list_t;

typedef struct {
	char *user_raw;
	uid_t user_uid;
	gid_t user_primary_gid;
	bool user_is_set;
	bool user_resolved;
	gid_t group_gid;
	bool group_is_set;
	gid_list_t sup_groups;
	cap_set_t bounding;
	cap_set_t ambient;
	bool dynamic_user;
	bool no_new_privs;
	char *service_type;
	char *exec_start;
	char **exec_argv;
	size_t exec_argc;
	/* ExecStart=! leaves configured credential changes to the command. */
	bool exec_start_no_setuid;
} service_config_t;

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
	/* Pending per-thread credential transitions, owned by event handling. */
	struct keepcaps_window *keepcaps_windows;
	__u64 keepcaps_init_caps;
	bool keepcaps_incomplete;
	__u32 baseline_user_ns_inum;
	bool foreign_target_ns_observed;
	volatile sig_atomic_t stop;
	char *service_file;
	service_config_t *service_cfg;
};

extern struct audit_state state;
extern int audit_machine;

int handle_cap_event(void *ctx, void *data, size_t data_sz)
	__attr_access ((__read_only__, 2));
void finish_cap_events(void);
void analyze_capabilities(void);
void output_json(void);
void output_yaml(void);
const char *cap_name_safe(int cap) __returns_nonnull;
const char *syscall_name_from_nr(int nr);
void read_sysctl(const char *path, int *value)
	__attr_access ((__read_only__, 1))
	__attr_access ((__write_only__, 2));
void read_system_state(struct app_caps *app)
	__attr_access ((__read_write__, 1));
int resolve_target_exe(pid_t pid, char *exepath, size_t exepath_len)
	__attr_access ((__write_only__, 2, 3))
	__wur;
char *json_escape(const char *input)
	__attribute_malloc__
	__attr_dealloc_free
	__attr_access ((__read_only__, 1))
	__wur;
void update_reason_to(char **target, int syscall_nr)
	__attr_access ((__read_write__, 1));
int cap_required_union(const struct cap_check *check)
	__attr_access ((__read_only__, 1))
	__attribute_pure__;
unsigned long cap_total_checks(const struct cap_check *check)
	__attr_access ((__read_only__, 1))
	__attribute_pure__;
unsigned long cap_total_granted(const struct cap_check *check)
	__attr_access ((__read_only__, 1))
	__attribute_pure__;
unsigned long cap_total_denied(const struct cap_check *check)
	__attr_access ((__read_only__, 1))
	__attribute_pure__;
const char *cap_union_reason(const struct cap_check *check)
	__attr_access ((__read_only__, 1))
	__attribute_pure__;
void record_successful_capset(const struct cap_event *event)
	__attr_access ((__read_only__, 1));
int cap_requested_by_capset(int cap) __attribute_pure__;
int cap_is_capset_only(int cap) __attribute_pure__;
int cap_is_compat_requirement(int cap) __attribute_pure__;
int add_cap_syscall_outcome(struct cap_check *check, int syscall_nr,
			    __s64 result)
	__attr_access ((__read_write__, 1));
enum syscall_outcome_class classify_syscall_outcome(__s64 result)
	__attribute_const__;
const char *syscall_outcome_class_name(enum syscall_outcome_class class)
	__attribute_const__;
void summarize_cap_outcomes(const struct cap_check *check,
			    struct cap_outcome_summary *summary)
	__attr_access ((__read_only__, 1))
	__attr_access ((__write_only__, 2));
enum denial_assessment assess_cap_denials(const struct cap_check *check)
	__attr_access ((__read_only__, 1))
	__attribute_pure__;
const char *denial_assessment_name(enum denial_assessment assessment)
	__attribute_const__;
int syscall_result_errno(__s64 result)
	__attribute_const__;
const char *syscall_result_name(__s64 result)
	__attribute_const__;
type_t classify_app(const char *exe)
	__attr_access ((__read_only__, 1))
	__wur;
int parse_service_file(const char *path, service_config_t *cfg)
	__attr_access ((__read_only__, 1))
	__attr_access ((__read_write__, 2))
	__wur;
int apply_service_config(const service_config_t *cfg)
	__attr_access ((__read_only__, 1))
	__wur;
void free_config(service_config_t *cfg)
	__attr_access ((__read_write__, 1));
void print_service_config(const service_config_t *cfg)
	__attr_access ((__read_only__, 1));

#endif
