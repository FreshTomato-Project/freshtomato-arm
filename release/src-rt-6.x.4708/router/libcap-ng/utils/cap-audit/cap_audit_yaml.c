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

/* YAML output serialization: alternate machine-readable output mirroring
 * the JSON structure for downstream consumers.
 */

#include "cap_audit.h"

#include <stdio.h>
#include <stdlib.h>

static void print_yaml_denied_syscalls(const struct cap_check *check)
{
	size_t i;

	if (check->denied_syscall_count == 0) {
		printf("    denied_syscalls: []\n");
		return;
	}

	printf("    denied_syscalls:\n");
	for (i = 0; i < check->denied_syscall_count; i++) {
		const char *name;

		name = syscall_name_from_nr(check->denied_syscalls[i]);
		printf("      - number: %d\n", check->denied_syscalls[i]);
		printf("        name: %s\n", name ? name : "unknown");
	}
}

static void print_yaml_outcomes(const struct cap_check *check)
{
	size_t i;

	printf("    assessment: %s\n",
	       denial_assessment_name(assess_cap_denials(check)));
	if (check->outcome_count == 0) {
		printf("    syscall_outcomes: []\n");
		return;
	}

	printf("    syscall_outcomes:\n");
	for (i = 0; i < check->outcome_count; i++) {
		const struct syscall_outcome *outcome = &check->outcomes[i];
		enum syscall_outcome_class class;
		const char *result_name;
		const char *syscall_name;
		int error;

		class = classify_syscall_outcome(outcome->result);
		result_name = syscall_result_name(outcome->result);
		error = syscall_result_errno(outcome->result);
		syscall_name = syscall_name_from_nr(outcome->syscall_nr);
		printf("      - syscall_number: %d\n", outcome->syscall_nr);
		printf("        syscall: %s\n",
		       syscall_name ? syscall_name : "unknown");
		printf("        outcome_class: %s\n",
		       syscall_outcome_class_name(class));
		printf("        raw_return: %lld\n",
		       (long long)outcome->result);
		printf("        return_name: %s\n",
		       result_name ? result_name : "null");
		if (error > 0)
			printf("        errno: %d\n", error);
		else
			printf("        errno: null\n");
		printf("        count: %lu\n", outcome->count);
	}
}

static void print_yaml_capset_only(void)
{
	int cap;
	int found = 0;

	for (cap = 0; cap <= CAP_LAST_CAP; cap++) {
		__u64 mask;

		if (!cap_is_capset_only(cap))
			continue;
		if (!found)
			printf("capset_only_capabilities:\n");
		printf("  - number: %d\n", cap);
		printf("    name: %s\n", cap_name_safe(cap));
		printf("    requested_sets:\n");
		mask = 1ULL << cap;
		if (state.app.capset.effective & mask)
			printf("      - effective\n");
		if (state.app.capset.permitted & mask)
			printf("      - permitted\n");
		if (state.app.capset.inheritable & mask)
			printf("      - inheritable\n");
		found = 1;
	}
	if (!found)
		printf("capset_only_capabilities: []\n");
}

void output_yaml(void)
{
	int i;
	char *exe_yaml = json_escape(state.app.exe);
	char *kernel_yaml = json_escape(state.app.kernel_version);

	printf("application:\n");
	printf("  pid: %d\n", state.app.pid);
	printf("  comm: \"%s\"\n", exe_yaml ? exe_yaml : "");

	printf("system_context:\n");
	printf("  kernel_version: \"%s\"\n", kernel_yaml ? kernel_yaml : "");
	printf("  yama_ptrace_scope: %d\n", state.app.yama_ptrace_scope);
	printf("  kptr_restrict: %d\n", state.app.kptr_restrict);
	printf("  dmesg_restrict: %d\n", state.app.dmesg_restrict);
	printf("  modules_disabled: %d\n", state.app.modules_disabled);
	printf("  perf_event_paranoid: %d\n",
	       state.app.perf_event_paranoid);
	printf("  unprivileged_bpf_disabled: %d\n",
	       state.app.unprivileged_bpf_disabled);
	printf("  bpf_jit_enable: %d\n", state.app.bpf_jit_enable);
	printf("  bpf_jit_harden: %d\n", state.app.bpf_jit_harden);
	printf("  bpf_jit_kallsyms: %d\n", state.app.bpf_jit_kallsyms);
	printf("  vm_mmap_min_addr: %d\n", state.app.mmap_min_addr);
	printf("  fs_protected_hardlinks: %d\n",
	       state.app.protected_hardlinks);
	printf("  fs_protected_symlinks: %d\n",
	       state.app.protected_symlinks);
	printf("  fs_suid_dumpable: %d\n", state.app.suid_dumpable);
	free(exe_yaml);
	free(kernel_yaml);

	printf("capability_drop_observed: %s\n",
	       state.capset_observed ? "true" : "false");
	printf("keepcaps_transition_incomplete: %s\n",
	       state.keepcaps_incomplete ? "true" : "false");
	printf("successful_capset_calls: %lu\n",
	       state.app.capset.successful_calls);

	printf("required_capabilities:\n");
	for (i = 0; i <= CAP_LAST_CAP; i++) {
		struct cap_check *check = &state.app.checks[i];

		if (cap_required_union(check)) {
			printf("  - number: %d\n", i);
			printf("    name: %s\n", cap_name_safe(i));
			printf("    checks:\n");
			printf("      total: %lu\n", cap_total_checks(check));
			printf("      granted: %lu\n", cap_total_granted(check));
			printf("      denied: %lu\n", cap_total_denied(check));
			if (cap_union_reason(check)) {
				char *reason_yaml =
					json_escape(cap_union_reason(check));
				printf("    reason: \"%s\"\n",
				       reason_yaml ? reason_yaml : "");
				free(reason_yaml);
			}
		}
	}

	print_yaml_capset_only();

	if (state.capset_observed) {
		printf("initialization_capabilities:\n");
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			struct cap_check *check = &state.app.checks[i];

			if (check->granted == 0)
				continue;

			printf("  - number: %d\n", i);
			printf("    name: %s\n", cap_name_safe(i));
			printf("    checks:\n");
			printf("      total: %lu\n", check->count);
			printf("      granted: %lu\n", check->granted);
			printf("      denied: %lu\n", check->denied);
			if (check->reason) {
				char *reason_yaml = json_escape(check->reason);
				printf("    reason: \"%s\"\n",
				       reason_yaml ? reason_yaml : "");
				free(reason_yaml);
			}
		}

		printf("operational_capabilities:\n");
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			struct cap_check *check = &state.app.checks[i];

			if (check->op_granted == 0)
				continue;

			printf("  - number: %d\n", i);
			printf("    name: %s\n", cap_name_safe(i));
			printf("    checks:\n");
			printf("      total: %lu\n", check->op_count);
			printf("      granted: %lu\n", check->op_granted);
			printf("      denied: %lu\n", check->op_denied);
			if (check->op_reason) {
				char *reason_yaml = json_escape(check->op_reason);
				printf("    reason: \"%s\"\n",
				       reason_yaml ? reason_yaml : "");
				free(reason_yaml);
			}
		}
	}

	printf("denied_capabilities:\n");
	for (i = 0; i <= CAP_LAST_CAP; i++) {
		struct cap_check *check = &state.app.checks[i];

		if (cap_total_denied(check) > 0 &&
		    cap_total_granted(check) == 0) {
			printf("  - number: %d\n", i);
			printf("    name: %s\n", cap_name_safe(i));
			printf("    not_granted_checks: %lu\n",
			       cap_total_denied(check));
			print_yaml_denied_syscalls(check);
			print_yaml_outcomes(check);
		}
	}
}
