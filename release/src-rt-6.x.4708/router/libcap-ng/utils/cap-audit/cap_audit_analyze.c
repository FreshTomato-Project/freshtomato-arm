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

/* Human-readable analysis output: capability summaries, deployment
 * recommendations, and wrapped terminal formatting helpers.
 */

#include "cap_audit.h"

#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

int include_cap_in_recommendations(int cap)
{
	if (cap == CAP_SETPCAP && state.app.file_caps &&
	    !state.app.file_setpcap)
		return 0;

	return 1;
}

static void cap_name_upper_buf(int cap, char *buf, size_t buf_len)
{
	const char *name = cap_name_safe(cap);
	size_t i;

	if (buf_len == 0)
		return;

	for (i = 0; name[i] && i + 1 < buf_len; i++)
		buf[i] = toupper((unsigned char)name[i]);
	buf[i] = '\0';
}

static int get_output_width(void)
{
	struct winsize ws;
	const char *columns;
	long env_width;

	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0)
		return ws.ws_col;

	columns = getenv("COLUMNS");
	if (columns) {
		env_width = strtol(columns, NULL, 10);
		if (env_width >= 40 && env_width <= 400)
			return (int)env_width;
	}

	return 80;
}

static void print_rule(char ch)
{
	int i;
	int width = get_output_width();

	if (width < 40)
		width = 40;

	for (i = 0; i < width; i++)
		putchar(ch);
	putchar('\n');
}

static void print_wrapped_text(const char *indent, const char *text)
{
	size_t indent_len;
	char *cont_indent;
	int width;
	int content_width;
	const char *p;
	int line_len = 0;
	int first_line = 1;

	if (!text) {
		printf("%s\n", indent);
		return;
	}

	indent_len = strlen(indent);
	cont_indent = malloc(indent_len + 1);
	if (!cont_indent) {
		printf("%s%s\n", indent, text);
		return;
	}
	memset(cont_indent, ' ', indent_len);
	cont_indent[indent_len] = '\0';

	width = get_output_width();
	if (width < 40)
		width = 40;
	content_width = width - (int)indent_len;
	if (content_width < 16)
		content_width = 16;

	printf("%s", indent);
	p = text;
	while (*p) {
		size_t word_len;
		int need_space = line_len > 0;

		while (*p == ' ')
			p++;

		if (*p == '\n') {
			putchar('\n');
			printf("%s", first_line ? cont_indent : cont_indent);
			line_len = 0;
			first_line = 0;
			p++;
			continue;
		}
		if (*p == '\0')
			break;

		word_len = strcspn(p, " \n");
		if (need_space &&
		    line_len + 1 + (int)word_len > content_width) {
			putchar('\n');
			printf("%s", cont_indent);
			line_len = 0;
			need_space = 0;
			first_line = 0;
		}
		if (need_space) {
			putchar(' ');
			line_len++;
		}
		fwrite(p, 1, word_len, stdout);
		line_len += word_len;
		p += word_len;
	}
	putchar('\n');
	free(cont_indent);
}

static void print_wrappedf(const char *indent, const char *fmt, ...)
{
	va_list ap;
	char *buf;

	va_start(ap, fmt);
	if (vasprintf(&buf, fmt, ap) < 0)
		buf = NULL;
	va_end(ap);

	if (buf) {
		print_wrapped_text(indent, buf);
		free(buf);
	} else {
		print_wrapped_text(indent, "(formatting error)");
	}
}

static int cap_in_programmatic_set(int cap)
{
	if (!include_cap_in_recommendations(cap))
		return 0;

	if (state.capset_observed)
		return state.app.checks[cap].op_granted > 0;

	return state.app.checks[cap].granted > 0;
}

static void print_updatev_wrapped(const char *prefix, const char *cap_prefix,
				  const char *suffix)
{
	int width = get_output_width();
	size_t prefix_len = strlen(prefix);
	size_t suffix_len = strlen(suffix);
	int cur_len = prefix_len;
	int cont_indent = 8;
	int i;

	if (width < 40)
		width = 40;

	for (i = 0; prefix[i] == ' '; i++)
		;
	if (i > 0)
		cont_indent = i + 4;

	printf("%s", prefix);
	for (i = 0; i <= CAP_LAST_CAP; i++) {
		char cap_name[64];
		char item[96];
		size_t item_len;

		if (!cap_in_programmatic_set(i))
			continue;

		cap_name_upper_buf(i, cap_name, sizeof(cap_name));
		snprintf(item, sizeof(item), "%s%s", cap_prefix, cap_name);
		item_len = strlen(item);

		if (cur_len > (int)prefix_len &&
		    cur_len + 2 + (int)item_len + (int)suffix_len > width) {
			printf(",\n%*s%s", cont_indent, "", item);
			cur_len = cont_indent + item_len;
		} else {
			printf(", %s", item);
			cur_len += 2 + item_len;
		}
	}

	if (cur_len + 2 + (int)suffix_len > width && cur_len > cont_indent) {
		printf(",\n%*s%s", cont_indent, "", suffix);
	} else {
		printf(", %s", suffix);
	}
}

void analyze_capabilities(void)
{
	int has_required = 0;
	int has_conditional = 0;
	int has_denied = 0;
	int required_count = 0;
	int conditional_count = 0;
	int denied_count = 0;
	unsigned long total_checks = 0;
	int i;
	int first;

	printf("\n");
	print_rule('=');
	print_wrappedf("", "CAPABILITY ANALYSIS FOR: %s (PID %d)",
		       state.app.exe, state.app.pid);
	print_rule('=');
	printf("\n");

	printf("SYSTEM CONTEXT:\n");
	print_rule('-');
	printf("  Kernel version: %s\n", state.app.kernel_version);
	printf("  kernel.yama.ptrace_scope: %d\n", state.app.yama_ptrace_scope);
	printf("  kernel.kptr_restrict: %d\n", state.app.kptr_restrict);
	printf("  kernel.dmesg_restrict: %d\n", state.app.dmesg_restrict);
	printf("  kernel.modules_disabled: %d\n", state.app.modules_disabled);
	printf("  kernel.perf_event_paranoid: %d\n",
	       state.app.perf_event_paranoid);
	printf("  kernel.unprivileged_bpf_disabled: %d\n",
	       state.app.unprivileged_bpf_disabled);
	printf("  net.core.bpf_jit_enable: %d\n", state.app.bpf_jit_enable);
	printf("  net.core.bpf_jit_harden: %d\n", state.app.bpf_jit_harden);
	printf("  net.core.bpf_jit_kallsyms: %d\n",
	       state.app.bpf_jit_kallsyms);
	printf("  vm.mmap_min_addr: %d\n", state.app.mmap_min_addr);
	printf("  fs.protected_hardlinks: %d\n", state.app.protected_hardlinks);
	printf("  fs.protected_symlinks: %d\n", state.app.protected_symlinks);
	printf("  fs.suid_dumpable: %d\n", state.app.suid_dumpable);
	printf("\n");

	printf("REQUIRED CAPABILITIES:\n");
	print_rule('-');
	if (!state.capset_observed) {
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			struct cap_check *check;

			check = &state.app.checks[i];
			if (check->granted > 0) {
				has_required = 1;
				printf("  %s (#%d)\n", cap_name_safe(i), i);
				printf("    Checks: %lu granted, %lu denied\n",
				       check->granted, check->denied);
				if (check->reason)
					print_wrappedf("    Reason: ",
						       "%s", check->reason);
				if (!include_cap_in_recommendations(i))
					print_wrapped_text("    Note: ",
							   "Internal to capability setup; excluded from recommendations.");
				printf("\n");
			}
		}
		if (!has_required)
			print_wrapped_text("  ",
					   "None - Application does not require elevated capabilities!\n");
	} else {
		print_wrapped_text("",
				   "INITIALIZATION CAPABILITIES (before capability drop):");
		print_rule('-');
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			struct cap_check *check = &state.app.checks[i];

			if (check->granted > 0) {
				has_required = 1;
				printf("  %s (#%d)\n", cap_name_safe(i), i);
				printf("    Checks: %lu granted, %lu denied\n",
				       check->granted, check->denied);
				if (check->reason)
					print_wrappedf("    Reason: ",
						       "%s", check->reason);
				if (!include_cap_in_recommendations(i))
					print_wrapped_text("    Note: ",
							   "Internal to capability setup; excluded from recommendations.");
				printf("\n");
			}
		}
		if (!has_required)
			printf("  None\n\n");

		print_wrapped_text("",
				   "OPERATIONAL CAPABILITIES (after capability drop):");
		print_rule('-');
		has_required = 0;
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			struct cap_check *check = &state.app.checks[i];

			if (check->op_granted > 0) {
				has_required = 1;
				printf("  %s (#%d)\n", cap_name_safe(i), i);
				printf("    Checks: %lu granted, %lu denied\n",
				       check->op_granted, check->op_denied);
				if (check->op_reason)
					print_wrappedf("    Reason: ",
						       "%s", check->op_reason);
				if (!include_cap_in_recommendations(i))
					print_wrapped_text("    Note: ",
							   "Internal to capability setup; excluded from recommendations.");
				printf("\n");
			}
		}
		if (!has_required)
			printf("  None\n\n");
	}

	printf("CONDITIONAL CAPABILITIES:\n");
	print_rule('-');

	if (state.app.yama_ptrace_scope > 0) {
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			if (cap_total_checks(&state.app.checks[i]) > 0 &&
			    state.app.checks[i].granted == 0 &&
			    i == CAP_SYS_PTRACE) {
				has_conditional = 1;
				conditional_count++;
				printf("  CAP_SYS_PTRACE\n");
				print_wrapped_text("    ",
						   "Needed when kernel.yama.ptrace_scope > 0");
				print_wrappedf("    ",
					       "Current value: %d (capability needed)",
					       state.app.yama_ptrace_scope);
				printf("\n");
			}
		}
	}

	if (state.app.perf_event_paranoid >= 2) {
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			if (cap_total_checks(&state.app.checks[i]) > 0 &&
			    i == CAP_PERFMON) {
				has_conditional = 1;
				conditional_count++;
				printf("  CAP_PERFMON\n");
				print_wrapped_text("    ",
						   "Needed when kernel.perf_event_paranoid >= 2");
				print_wrappedf("    ",
					       "Current value: %d (capability needed)",
					       state.app.perf_event_paranoid);
				print_wrapped_text("    Note: ",
						   "CAP_SYS_ADMIN can substitute on kernels < 5.8");
				printf("\n");
			}
		}
	}

	if (state.app.unprivileged_bpf_disabled == 1) {
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			if (cap_total_checks(&state.app.checks[i]) > 0 &&
			    i == CAP_BPF) {
				has_conditional = 1;
				conditional_count++;
				printf("  CAP_BPF\n");
				print_wrapped_text("    ",
						   "Needed when kernel.unprivileged_bpf_disabled = 1");
				print_wrappedf("    ",
					       "Current value: %d (capability needed)",
					       state.app.unprivileged_bpf_disabled);
				print_wrapped_text("    Note: ",
						   "CAP_SYS_ADMIN can substitute on kernels < 5.8");
				printf("\n");
			}
		}
	}

	if (state.app.kptr_restrict >= 1) {
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			if (cap_total_checks(&state.app.checks[i]) > 0 &&
			    i == CAP_SYSLOG) {
				has_conditional = 1;
				conditional_count++;
				printf("  CAP_SYSLOG\n");
				print_wrapped_text("    ",
						   "Needed when kernel.kptr_restrict >= 1");
				print_wrappedf("    ",
					       "Current value: %d (capability needed)",
					       state.app.kptr_restrict);
				printf("\n");
			}
		}
	}

	if (state.app.dmesg_restrict >= 1) {
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			if (cap_total_checks(&state.app.checks[i]) > 0 &&
			    i == CAP_SYSLOG) {
				has_conditional = 1;
				conditional_count++;
				printf("  CAP_SYSLOG\n");
				print_wrapped_text("    ",
						   "Needed when kernel.dmesg_restrict >= 1");
				print_wrappedf("    ",
					       "Current value: %d (capability needed)",
					       state.app.dmesg_restrict);
				printf("\n");
			}
		}
	}

	if (state.app.modules_disabled == 1) {
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			if (cap_total_checks(&state.app.checks[i]) > 0 &&
			    i == CAP_SYS_MODULE) {
				has_conditional = 1;
				conditional_count++;
				printf("  NOTE: kernel.modules_disabled = 1\n");
				print_wrapped_text("    ",
						   "CAP_SYS_MODULE is ineffective!");
				print_wrapped_text("    ",
						   "Module loading is permanently disabled.");
				printf("\n");
			}
		}
	}

	if (state.app.mmap_min_addr > 0) {
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			if (cap_total_checks(&state.app.checks[i]) > 0 &&
			    i == CAP_SYS_RAWIO) {
				has_conditional = 1;
				conditional_count++;
				printf("  CAP_SYS_RAWIO\n");
				print_wrapped_text("    ",
						   "Needed when vm.mmap_min_addr > 0 to map low addresses");
				print_wrappedf("    ",
					       "Current value: %d (capability needed)",
					       state.app.mmap_min_addr);
				printf("\n");
			}
		}
	}

	if (state.app.protected_hardlinks == 1) {
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			if (cap_total_checks(&state.app.checks[i]) > 0 &&
			    i == CAP_FOWNER) {
				has_conditional = 1;
				conditional_count++;
				printf("  CAP_FOWNER\n");
				print_wrapped_text("    ",
						   "Needed when fs.protected_hardlinks = 1 to link files not owned by the caller");
				print_wrappedf("    ",
					       "Current value: %d (capability needed)",
					       state.app.protected_hardlinks);
				printf("\n");
			}
		}
	}

	if (state.app.protected_symlinks == 1) {
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			if (cap_total_checks(&state.app.checks[i]) > 0 &&
			    i == CAP_DAC_OVERRIDE) {
				has_conditional = 1;
				conditional_count++;
				printf("  CAP_DAC_OVERRIDE\n");
				print_wrapped_text("    ",
						   "Needed when fs.protected_symlinks = 1 for symlinks in world-writable directories");
				print_wrappedf("    ",
					       "Current value: %d (capability needed)",
					       state.app.protected_symlinks);
				printf("\n");
			}
		}
	}

	if (state.app.suid_dumpable == 2) {
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			if (cap_total_checks(&state.app.checks[i]) > 0 &&
			    i == CAP_SYS_PTRACE) {
				has_conditional = 1;
				conditional_count++;
				printf("  CAP_SYS_PTRACE\n");
				print_wrapped_text("    ",
						   "Needed when fs.suid_dumpable = 2 for core dumps and ptrace of setuid programs");
				print_wrappedf("    ",
					       "Current value: %d (capability needed)",
					       state.app.suid_dumpable);
				printf("\n");
			}
		}
	}

	if (!has_conditional)
		printf("  None\n\n");

	printf("ATTEMPTED BUT DENIED:\n");
	print_rule('-');
	for (i = 0; i <= CAP_LAST_CAP; i++) {
		struct cap_check *check;

		check = &state.app.checks[i];
		if (cap_total_denied(check) > 0 && cap_total_granted(check) == 0) {
			size_t j;

			has_denied = 1;
			printf("  %s (#%d)\n", cap_name_safe(i), i);
			printf("    Attempts: %lu (all denied)\n",
			       cap_total_denied(check));
			printf("    Syscalls: ");
			if (check->denied_syscall_count == 0)
				printf("unknown\n");
			for (j = 0; j < check->denied_syscall_count; j++) {
				const char *syscall_name;
				int syscall_nr;

				syscall_nr = check->denied_syscalls[j];
				syscall_name = syscall_name_from_nr(syscall_nr);
				if (j > 0)
					printf(", ");
				if (syscall_name)
					printf("%s", syscall_name);
				else
					printf("unknown(#%d)", syscall_nr);
			}
			if (check->denied_syscall_count > 0)
				printf("\n");
			print_wrapped_text("    Impact: ",
					   "Application may have reduced functionality");
			printf("\n");
		}
	}
	if (!has_denied)
		printf("  None\n\n");

	for (i = 0; i <= CAP_LAST_CAP; i++) {
		total_checks += cap_total_checks(&state.app.checks[i]);
		if (cap_required_union(&state.app.checks[i]))
			required_count++;
		if (cap_total_denied(&state.app.checks[i]) > 0 &&
		    cap_total_granted(&state.app.checks[i]) == 0)
			denied_count++;
	}

	printf("SUMMARY:\n");
	print_rule('-');
	printf("  Total capability checks: %lu\n", total_checks);
	printf("  Required capabilities: %d\n", required_count);
	printf("  Conditional capabilities: %d\n", conditional_count);
	printf("  Denied operations: %d\n", denied_count);
	printf("\n");

	if (required_count > 0) {
		printf("RECOMMENDATIONS:\n");
		print_rule('-');
		if (state.app.prog_type != UNSUPPORTED) {
			printf("  Programmatic solution (%s):\n",
			       state.app.prog_type == ELF ?
			       "C with libcap-ng" :
			       "Python with python3-libcap-ng");
			if (state.capset_observed)
				print_wrapped_text("    Note: ",
						   "The application drops capabilities after initialization. The programmatic snippet reflects the operational set only.");

			if (state.app.prog_type == ELF) {
				printf("    #include <cap-ng.h>\n");
				printf("    ...\n");
				printf("    capng_clear(CAPNG_SELECT_BOTH);\n");
				print_updatev_wrapped("    capng_updatev(CAPNG_ADD, "
						      "CAPNG_EFFECTIVE|CAPNG_PERMITTED",
						      "", "-1);\n");
				printf("    if (capng_change_id(uid, gid, "
				       "CAPNG_DROP_SUPP_GRP | "
				       "CAPNG_CLEAR_BOUNDING))\n");
				printf("\tperror(\"capng_change_id\");\n\n");
			} else if (state.app.prog_type == PYTHON) {
				printf("    import sys\n");
				printf("    import _capng as capng\n");
				printf("    ...\n");
				printf("    capng.capng_clear(capng.CAPNG_SELECT_BOTH)\n");
				print_updatev_wrapped("    capng.capng_updatev("
						      "capng.CAPNG_ADD, "
						      "capng.CAPNG_EFFECTIVE|"
						      "capng.CAPNG_PERMITTED",
						      "capng.", "-1)\n");
				printf("    e = capng.capng_change_id(uid, gid, "
				       "capng.CAPNG_DROP_SUPP_GRP | "
				       "capng.CAPNG_CLEAR_BOUNDING)\n");
				printf("    if e < 0:\n");
				printf("\tprint(f\"Error: {e}\")\n");
				printf("\tsys.exit(1)\n\n");
			}
		}

		printf("  For systemd service:\n");
		if (state.capset_observed)
			print_wrapped_text("    Note: ",
					   "Ambient capabilities must include initialization requirements. The application drops to the operational set internally via capset.");
		printf("    [Service]\n");
		printf("    User=<non-root-user>\n");
		printf("    Group=<non-root-group>\n");
		printf("    AmbientCapabilities=");
		first = 1;
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			if (cap_required_union(&state.app.checks[i]) &&
			    include_cap_in_recommendations(i)) {
				if (!first)
					printf(" ");
				printf("%s", cap_name_safe(i));
				first = 0;
			}
		}
		printf("\n");
		printf("    CapabilityBoundingSet=");
		first = 1;
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			if (cap_required_union(&state.app.checks[i]) &&
			    include_cap_in_recommendations(i)) {
				if (!first)
					printf(" ");
				printf("%s", cap_name_safe(i));
				first = 0;
			}
		}
		printf("\n\n");

		printf("  For file capabilities (via filecap):\n");
		if (state.capset_observed)
			print_wrapped_text("    Note: ",
					   "File capabilities must include initialization requirements. The application drops to the operational set internally via capset.");
		printf("    filecap /path/to/binary");
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			if (cap_required_union(&state.app.checks[i]) &&
			    include_cap_in_recommendations(i))
				printf(" %s", cap_name_safe(i));
		}
		printf("\n\n");

		printf("  For Docker/Podman:\n");
		if (state.capset_observed)
			print_wrapped_text("    Note: ",
					   "Container capabilities must include initialization requirements. The application drops to the operational set internally via capset.");
		printf("    docker run --user $(id -u):$(id -g) \\\n");
		printf("      --cap-drop=ALL \\\n");
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			if (cap_required_union(&state.app.checks[i]) &&
			    include_cap_in_recommendations(i))
				printf("      --cap-add=%s \\\n",
				       cap_name_safe(i));
		}
		printf("      your-image:tag\n\n");

		printf("  For Kubernetes:\n");
		if (state.capset_observed)
			print_wrapped_text("    Note: ",
					   "Container capabilities must include initialization requirements. The application drops to the operational set internally via capset.");
		printf("    securityContext:\n");
		printf("      runAsUser: 1000\n");
		printf("      runAsGroup: 1000\n");
		printf("      capabilities:\n");
		printf("        drop:\n");
		printf("          - ALL\n");
		printf("        add:\n");
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			if (cap_required_union(&state.app.checks[i]) &&
			    include_cap_in_recommendations(i))
				printf("          - %s\n", cap_name_safe(i));
		}
		printf("\n");
	} else {
		printf("RECOMMENDATIONS:\n");
		print_rule('-');
		print_wrapped_text("  ",
				   "This application does not require any elevated capabilities!");
		print_wrapped_text("  ",
				   "Run as an unprivileged user with no special capabilities.");
		printf("\n");
	}

	print_wrapped_text("EXPERIMENTAL NOTICE: ",
			   "cap-audit output is experimental, but very close.");
}
