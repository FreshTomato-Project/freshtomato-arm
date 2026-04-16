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

/* YAML output serialization: alternate machine-readable output mirroring
 * the JSON structure for downstream consumers.
 */

#include "cap_audit.h"

#include <stdio.h>
#include <stdlib.h>

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

		if (cap_total_denied(check) > 0 && cap_total_granted(check) == 0) {
			printf("  - number: %d\n", i);
			printf("    name: %s\n", cap_name_safe(i));
			printf("    attempts: %lu\n", cap_total_denied(check));
		}
	}
}
