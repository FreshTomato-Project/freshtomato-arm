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

/* JSON output serialization: machine-readable capability summaries for
 * automated tooling and regression comparison.
 */

#include "cap_audit.h"

#include <stdio.h>
#include <stdlib.h>

static void print_json_denied_syscalls(const struct cap_check *check)
{
	size_t i;

	printf("      \"denied_syscalls\": [\n");
	for (i = 0; i < check->denied_syscall_count; i++) {
		const char *name;
		char *name_json;

		name = syscall_name_from_nr(check->denied_syscalls[i]);
		name_json = json_escape(name ? name : "unknown");
		if (i > 0)
			printf(",\n");
		printf("        {\"number\": %d, \"name\": \"%s\"}",
		       check->denied_syscalls[i], name_json ? name_json : "");
		free(name_json);
	}
	printf("\n      ],\n");
}

static void print_json_outcomes(const struct cap_check *check)
{
	size_t i;

	printf("      \"assessment\": \"%s\",\n",
	       denial_assessment_name(assess_cap_denials(check)));
	printf("      \"syscall_outcomes\": [\n");
	for (i = 0; i < check->outcome_count; i++) {
		const struct syscall_outcome *outcome = &check->outcomes[i];
		enum syscall_outcome_class class;
		const char *result_name;
		const char *syscall_name;
		char *syscall_json;
		int error;

		class = classify_syscall_outcome(outcome->result);
		result_name = syscall_result_name(outcome->result);
		error = syscall_result_errno(outcome->result);
		syscall_name = syscall_name_from_nr(outcome->syscall_nr);
		syscall_json = json_escape(syscall_name ? syscall_name : "unknown");
		if (i > 0)
			printf(",\n");
		printf("        {\n");
		printf("          \"syscall_number\": %d,\n",
		       outcome->syscall_nr);
		printf("          \"syscall\": \"%s\",\n",
		       syscall_json ? syscall_json : "");
		printf("          \"outcome_class\": \"%s\",\n",
		       syscall_outcome_class_name(class));
		printf("          \"raw_return\": %lld,\n",
		       (long long)outcome->result);
		if (result_name)
			printf("          \"return_name\": \"%s\",\n",
			       result_name);
		else
			printf("          \"return_name\": null,\n");
		if (error > 0)
			printf("          \"errno\": %d,\n", error);
		else
			printf("          \"errno\": null,\n");
		printf("          \"count\": %lu\n", outcome->count);
		printf("        }");
		free(syscall_json);
	}
	printf("\n      ]\n");
}

static void print_json_capset_only(void)
{
	int cap;
	int first_cap = 1;

	printf("  \"capset_only_capabilities\": [\n");
	for (cap = 0; cap <= CAP_LAST_CAP; cap++) {
		char *name_json;
		__u64 mask;
		int first_set = 1;

		if (!cap_is_capset_only(cap))
			continue;
		name_json = json_escape(capng_capability_to_name(cap));
		if (!first_cap)
			printf(",\n");
		printf("    {\n");
		printf("      \"number\": %d,\n", cap);
		printf("      \"name\": \"%s\",\n",
		       name_json ? name_json : "");
		printf("      \"requested_sets\": [");
		mask = 1ULL << cap;
		if (state.app.capset.effective & mask) {
			printf("\"effective\"");
			first_set = 0;
		}
		if (state.app.capset.permitted & mask) {
			printf("%s\"permitted\"", first_set ? "" : ", ");
			first_set = 0;
		}
		if (state.app.capset.inheritable & mask)
			printf("%s\"inheritable\"", first_set ? "" : ", ");
		printf("]\n");
		printf("    }");
		first_cap = 0;
		free(name_json);
	}
	printf("\n  ]");
}

void output_json(void)
{
	int i;
	int first_cap;
	int first_denied;
	char *exe_json;
	char *kernel_json;

	exe_json = json_escape(state.app.exe);
	kernel_json = json_escape(state.app.kernel_version);

	printf("{\n");
	printf("  \"application\": {\n");
	printf("    \"pid\": %d,\n", state.app.pid);
	printf("    \"comm\": \"%s\"\n", exe_json ? exe_json : "");
	printf("  },\n");

	printf("  \"system_context\": {\n");
	printf("    \"kernel_version\": \"%s\",\n",
	       kernel_json ? kernel_json : "");
	printf("    \"yama_ptrace_scope\": %d,\n", state.app.yama_ptrace_scope);
	printf("    \"kptr_restrict\": %d,\n", state.app.kptr_restrict);
	printf("    \"dmesg_restrict\": %d,\n", state.app.dmesg_restrict);
	printf("    \"modules_disabled\": %d,\n", state.app.modules_disabled);
	printf("    \"perf_event_paranoid\": %d,\n",
	       state.app.perf_event_paranoid);
	printf("    \"unprivileged_bpf_disabled\": %d,\n",
	       state.app.unprivileged_bpf_disabled);
	printf("    \"bpf_jit_enable\": %d,\n", state.app.bpf_jit_enable);
	printf("    \"bpf_jit_harden\": %d,\n", state.app.bpf_jit_harden);
	printf("    \"bpf_jit_kallsyms\": %d,\n",
	       state.app.bpf_jit_kallsyms);
	printf("    \"vm_mmap_min_addr\": %d,\n", state.app.mmap_min_addr);
	printf("    \"fs_protected_hardlinks\": %d,\n",
	       state.app.protected_hardlinks);
	printf("    \"fs_protected_symlinks\": %d,\n",
	       state.app.protected_symlinks);
	printf("    \"fs_suid_dumpable\": %d\n", state.app.suid_dumpable);
	printf("  },\n");

	free(exe_json);
	free(kernel_json);

	printf("  \"capability_drop_observed\": %s,\n",
	       state.capset_observed ? "true" : "false");
	printf("  \"keepcaps_transition_incomplete\": %s,\n",
	       state.keepcaps_incomplete ? "true" : "false");
	printf("  \"successful_capset_calls\": %lu,\n",
	       state.app.capset.successful_calls);

	printf("  \"required_capabilities\": [\n");
	first_cap = 1;
	for (i = 0; i <= CAP_LAST_CAP; i++) {
		struct cap_check *check = &state.app.checks[i];
		char *name_json;
		char *reason_json;

		if (cap_required_union(check)) {
			name_json = json_escape(capng_capability_to_name(i));
			reason_json = cap_union_reason(check) ?
				json_escape(cap_union_reason(check)) : NULL;
			if (!first_cap)
				printf(",\n");
			printf("    {\n");
			printf("      \"number\": %d,\n", i);
			printf("      \"name\": \"%s\",\n",
			       name_json ? name_json : "");
			printf("      \"checks\": {\n");
			printf("        \"total\": %lu,\n",
			       cap_total_checks(check));
			printf("        \"granted\": %lu,\n",
			       cap_total_granted(check));
			printf("        \"denied\": %lu\n",
			       cap_total_denied(check));
			printf("      }");
			if (cap_union_reason(check))
				printf(",\n      \"reason\": \"%s\"\n",
				       reason_json ? reason_json : "");
			else
				printf("\n");
			printf("    }");
			first_cap = 0;
			free(name_json);
			free(reason_json);
		}
	}
	printf("\n  ],\n");
	print_json_capset_only();

	if (state.capset_observed) {
		printf(",\n  \"initialization_capabilities\": [\n");
		first_cap = 1;
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			struct cap_check *check = &state.app.checks[i];
			char *name_json;
			char *reason_json;

			if (check->granted == 0)
				continue;

			name_json = json_escape(capng_capability_to_name(i));
			reason_json = check->reason ?
				json_escape(check->reason) : NULL;
			if (!first_cap)
				printf(",\n");
			printf("    {\n");
			printf("      \"number\": %d,\n", i);
			printf("      \"name\": \"%s\",\n",
			       name_json ? name_json : "");
			printf("      \"checks\": {\n");
			printf("        \"total\": %lu,\n", check->count);
			printf("        \"granted\": %lu,\n", check->granted);
			printf("        \"denied\": %lu\n", check->denied);
			printf("      }");
			if (check->reason)
				printf(",\n      \"reason\": \"%s\"\n",
				       reason_json ? reason_json : "");
			else
				printf("\n");
			printf("    }");
			first_cap = 0;
			free(name_json);
			free(reason_json);
		}
		printf("\n  ],\n");

		printf("  \"operational_capabilities\": [\n");
		first_cap = 1;
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			struct cap_check *check = &state.app.checks[i];
			char *name_json;
			char *reason_json;

			if (check->op_granted == 0)
				continue;

			name_json = json_escape(capng_capability_to_name(i));
			reason_json = check->op_reason ?
				json_escape(check->op_reason) : NULL;
			if (!first_cap)
				printf(",\n");
			printf("    {\n");
			printf("      \"number\": %d,\n", i);
			printf("      \"name\": \"%s\",\n",
			       name_json ? name_json : "");
			printf("      \"checks\": {\n");
			printf("        \"total\": %lu,\n", check->op_count);
			printf("        \"granted\": %lu,\n",
			       check->op_granted);
			printf("        \"denied\": %lu\n",
			       check->op_denied);
			printf("      }");
			if (check->op_reason)
				printf(",\n      \"reason\": \"%s\"\n",
				       reason_json ? reason_json : "");
			else
				printf("\n");
			printf("    }");
			first_cap = 0;
			free(name_json);
			free(reason_json);
		}
		printf("\n  ]");
	}

	printf(",\n");

	printf("  \"denied_capabilities\": [\n");
	first_denied = 1;
	for (i = 0; i <= CAP_LAST_CAP; i++) {
		struct cap_check *check = &state.app.checks[i];
		char *name_json;

		if (cap_total_denied(check) > 0 &&
		    cap_total_granted(check) == 0) {
			name_json = json_escape(capng_capability_to_name(i));
			if (!first_denied)
				printf(",\n");
			printf("    {\n");
			printf("      \"number\": %d,\n", i);
			printf("      \"name\": \"%s\",\n",
			       name_json ? name_json : "");
			printf("      \"not_granted_checks\": %lu,\n",
			       cap_total_denied(check));
			print_json_denied_syscalls(check);
			print_json_outcomes(check);
			printf("    }");
			first_denied = 0;
			free(name_json);
		}
	}
	printf("\n  ]\n");
	printf("}\n");
}
