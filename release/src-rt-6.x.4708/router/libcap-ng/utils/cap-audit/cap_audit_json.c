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

/* JSON output serialization: machine-readable capability summaries for
 * automated tooling and regression comparison.
 */

#include "cap_audit.h"

#include <stdio.h>
#include <stdlib.h>

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
	printf("\n  ]");

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

		if (cap_total_denied(check) > 0 && cap_total_granted(check) == 0) {
			name_json = json_escape(capng_capability_to_name(i));
			if (!first_denied)
				printf(",\n");
			printf("    {\n");
			printf("      \"number\": %d,\n", i);
			printf("      \"name\": \"%s\",\n",
			       name_json ? name_json : "");
			printf("      \"attempts\": %lu\n",
			       cap_total_denied(check));
			printf("    }");
			first_denied = 0;
			free(name_json);
		}
	}
	printf("\n  ]\n");
	printf("}\n");
}
