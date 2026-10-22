// SPDX-License-Identifier: GPL-2.0-or-later
/* cap_audit_recommendation_test.c -- service recommendation tests
 * Copyright 2026 Red Hat Inc.
 * All Rights Reserved.
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cap_audit.h"

struct audit_state state;
int audit_machine;

static void fail(const char *msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}

const char *cap_name_safe(int cap)
{
	switch (cap) {
	case CAP_CHOWN:
		return "chown";
	case CAP_DAC_OVERRIDE:
		return "dac_override";
	case CAP_FOWNER:
		return "fowner";
	case CAP_SETPCAP:
		return "setpcap";
	case CAP_NET_BIND_SERVICE:
		return "net_bind_service";
	case CAP_SYS_CHROOT:
		return "sys_chroot";
	case CAP_SYS_PTRACE:
		return "sys_ptrace";
	case CAP_SYS_ADMIN:
		return "sys_admin";
	case CAP_SYS_RESOURCE:
		return "sys_resource";
	default:
		return "unknown";
	}
}

const char *syscall_name_from_nr(int nr)
{
	(void)nr;
	return "openat";
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

static char *capture_analysis(void)
{
	FILE *capture;
	char *output;
	long len;
	int saved_stdout;

	capture = tmpfile();
	if (!capture)
		fail("Failed to create output file");
	saved_stdout = dup(STDOUT_FILENO);
	if (saved_stdout < 0)
		fail("Failed to save stdout");
	if (fflush(stdout) || dup2(fileno(capture), STDOUT_FILENO) < 0)
		fail("Failed to capture stdout");

	analyze_capabilities();

	if (fflush(stdout) || dup2(saved_stdout, STDOUT_FILENO) < 0)
		fail("Failed to restore stdout");
	close(saved_stdout);
	if (fseek(capture, 0, SEEK_END) != 0)
		fail("Failed to seek captured output");
	len = ftell(capture);
	if (len < 0 || fseek(capture, 0, SEEK_SET) != 0)
		fail("Failed to measure captured output");
	output = malloc((size_t)len + 1);
	if (!output)
		fail("Failed to allocate captured output");
	if (fread(output, 1, (size_t)len, capture) != (size_t)len)
		fail("Failed to read captured output");
	output[len] = '\0';
	fclose(capture);

	return output;
}

static void expect_line(const char *output, const char *line)
{
	const char *found = strstr(output, line);
	size_t len = strlen(line);

	if (!found || found[len] != '\n')
		fail(line);
}

static int contains_tokens(const char *output, const char *const *tokens,
			   size_t count)
{
	size_t i;

	for (i = 0; i < count; i++) {
		output = strstr(output, tokens[i]);
		if (!output)
			return 0;
		output += strlen(tokens[i]);
	}

	return 1;
}

static size_t count_text(const char *output, const char *expected)
{
	size_t count = 0;
	size_t len = strlen(expected);

	while ((output = strstr(output, expected)) != NULL) {
		count++;
		output += len;
	}

	return count;
}

static void setup_analysis(service_config_t *service)
{
	static int denied_syscall = 257;
	struct cap_check *check;

	memset(&state, 0, sizeof(state));
	memset(service, 0, sizeof(*service));
	state.app.exe = "/usr/bin/recommendation-target";
	state.app.pid = 1234;
	state.app.prog_type = ELF;
	strcpy(state.app.kernel_version, "validation");
	state.service_file = "/tmp/recommendation.service";
	state.service_cfg = service;
	state.capset_observed = 1;
	state.app.capset.effective = 1ULL << CAP_SYS_RESOURCE;
	state.app.capset.permitted = 1ULL << CAP_SYS_RESOURCE;
	state.app.capset.successful_calls = 1;

	service->user_raw = "validation-user";
	service->user_uid = 1000;
	service->user_primary_gid = 1000;
	service->user_is_set = true;
	service->user_resolved = true;
	service->exec_start = "/usr/bin/recommendation-target";
	service->ambient.seen = true;
	service->ambient.caps[CAP_SYS_ADMIN] = true;
	service->bounding.seen = true;
	service->bounding.caps[CAP_SYS_PTRACE] = true;
	service->bounding.caps[CAP_SYS_ADMIN] = true;
	service->bounding.caps[CAP_SYS_RESOURCE] = true;

	check = &state.app.checks[CAP_CHOWN];
	check->count = 1;
	check->denied = 1;
	check->denied_syscalls = &denied_syscall;
	check->denied_syscall_count = 1;

	check = &state.app.checks[CAP_DAC_OVERRIDE];
	check->count = 1;
	check->granted = 1;

	check = &state.app.checks[CAP_FOWNER];
	check->count = 2;
	check->granted = 1;
	check->denied = 1;

	check = &state.app.checks[CAP_NET_BIND_SERVICE];
	check->count = 1;
	check->granted = 1;

	check = &state.app.checks[CAP_SYS_CHROOT];
	check->op_count = 1;
	check->op_granted = 1;
}

static void setup_empty_analysis(service_config_t *service)
{
	memset(&state, 0, sizeof(state));
	memset(service, 0, sizeof(*service));
	state.app.exe = "/usr/bin/empty-target";
	state.app.pid = 5678;
	strcpy(state.app.kernel_version, "validation");
	state.app.yama_ptrace_scope = 1;
	state.app.protected_symlinks = 1;
	state.app.suid_dumpable = 2;
	state.service_file = "/tmp/empty.service";
	state.service_cfg = service;

	service->exec_start = "/usr/bin/empty-target";
	service->bounding.seen = true;

	state.app.checks[CAP_DAC_OVERRIDE].count = 1;
	state.app.checks[CAP_DAC_OVERRIDE].denied = 1;
	state.app.checks[CAP_SYS_PTRACE].count = 1;
	state.app.checks[CAP_SYS_PTRACE].denied = 1;
}

static void test_setpcap_recommendations(void)
{
	service_config_t service;
	char *output;
	int source;

	/* Granted use, capset requests and unit intent each justify retention. */
	for (source = 0; source < 3; source++) {
		setup_empty_analysis(&service);
		if (source == 0) {
			state.app.checks[CAP_SETPCAP].count = 1;
			state.app.checks[CAP_SETPCAP].granted = 1;
		} else if (source == 1) {
			state.app.capset.permitted = 1ULL << CAP_SETPCAP;
			state.app.capset.successful_calls = 1;
		} else {
			service.bounding.caps[CAP_SETPCAP] = true;
		}
		output = capture_analysis();
		expect_line(output, "    CapabilityBoundingSet=setpcap");
		free(output);
	}
}

int main(void)
{
	static const char *const denied_guidance[] = {
		"Detailed evidence appears in",
		"CAPABILITIES WITH ONLY NOT-GRANTED CHECKS",
		"not-granted check",
		"does not add",
		"capability",
		"automatically",
	};
	static const char *const capset_guidance[] = {
		"CAPSET-ONLY CAPABILITIES",
		"sys_resource",
		"application's",
		"capset",
		"setup",
		"before",
		"removing",
		"deployment",
		"boundary",
	};
	service_config_t service;
	char *output;

	test_setpcap_recommendations();
	setup_analysis(&service);
	output = capture_analysis();

	expect_line(output, "    AmbientCapabilities: sys_admin");
	if (!contains_tokens(output,
		(const char *const[]) {
			"Recommended current-service-compatible [Service]",
			"AmbientCapabilities=dac_override fowner "
				"net_bind_service sys_chroot sys_admin sys_resource",
			"CapabilityBoundingSet=dac_override fowner "
				"net_bind_service sys_chroot sys_ptrace sys_admin "
				"sys_resource",
		}, 3))
		fail("Compatible configuration did not retain unit capabilities");
	if (strstr(output, "observed-workload-minimized") ||
	    count_text(output, "  Recommended ") != 1)
		fail("A second configuration removed unobserved capabilities");
	expect_line(output,
		    "    Initialization capabilities: dac_override fowner "
		    "net_bind_service");
	expect_line(output,
		    "    Operational capabilities: sys_chroot");

	if (!strstr(output, "chown: Additional evidence required") ||
	    !strstr(output, "Capability is absent from the configured"))
		fail("Denied-only capability was hidden from diagnostics");
	if (!contains_tokens(output, denied_guidance,
			     sizeof(denied_guidance) /
			     sizeof(denied_guidance[0])))
		fail("Denied-only capability lacked investigation guidance");
	if (count_text(output,
		       "    Configured but not observed in this run:") != 1)
		fail("Unused bounding capability was not reported as removable");
	expect_line(output,
		    "      AmbientCapabilities and CapabilityBoundingSet: "
		    "sys_admin");
	expect_line(output,
		    "      CapabilityBoundingSet only: sys_ptrace");
	if (!contains_tokens(output,
		(const char *const[]) {
			"Relevant functionality may not have been",
			"Retained in the",
			"current-service-compatible configuration",
			"manual review",
			"targeted testing",
		}, 5))
		fail("Unobserved configured capability lacked safe guidance");
	if (!strstr(output, "CAPSET-ONLY CAPABILITIES:") ||
	    !strstr(output, "sys_resource (#24)") ||
	    !strstr(output, "Capset-only compatibility capabilities: ") ||
	    !strstr(output,
		    "current-binary-compatible configuration retains these"))
		fail("Capset-only capability lacked compatibility diagnostics");
	if (!strstr(output, "  Current service context:\n") ||
	    !strstr(output, "    AmbientCapabilities: sys_admin\n") ||
	    !strstr(output,
		    "    CapabilityBoundingSet: sys_ptrace sys_admin "
		    "sys_resource\n"))
		fail("Current service context was not labeled");
	if (!contains_tokens(output, capset_guidance,
			     sizeof(capset_guidance) /
			     sizeof(capset_guidance[0])))
		fail("Capset-only capability lacked coordinated removal guidance");
	if (strstr(output,
		   "A listed capability is retained separately because a"))
		fail("Unrelated capset caveat was shown for denied capabilities");
	expect_line(output, "    NoNewPrivileges=no");
	if (!contains_tokens(output,
		(const char *const[]) {
			"The service was audited",
			"Enabling it was not validated",
			"configuration retains",
			"NoNewPrivileges=no",
			"functionality that starts child",
			"processes or helpers",
		}, 6))
		fail("Disabled NoNewPrivileges lacked compatibility guidance");

	free(output);
	service.no_new_privs = true;
	output = capture_analysis();
	expect_line(output, "    NoNewPrivileges=yes");
	if (strstr(output,
		   "The service was audited with this setting disabled"))
		fail("Enabled NoNewPrivileges received disabled-setting guidance");
	service.no_new_privs = false;

	free(output);
	state.app.checks[CAP_SYS_RESOURCE].count = 1;
	state.app.checks[CAP_SYS_RESOURCE].denied = 1;
	output = capture_analysis();
	if (!strstr(output,
		    "A listed capability is retained separately because a"))
		fail("Capset caveat was omitted for an overlapping capability");

	free(output);
	setup_empty_analysis(&service);
	output = capture_analysis();
	if (!strstr(output,
		    "None - No confirmed granted capability use was observed."))
		fail("No-granted-use summary was not evidence based");
	expect_line(output, "    CapabilityBoundingSet: (none)");
	expect_line(output, "    AmbientCapabilities: not configured (none)");
	expect_line(output, "    CapabilityBoundingSet=");
	if (strstr(output, "    CapabilityBoundingSet=(none)"))
		fail("Empty recommendation used invalid systemd syntax");
	if (!strstr(output, "CAPABILITY-RELATED SYSTEM CONTEXT:") ||
	    !strstr(output, "kernel.yama.ptrace_scope = 1") ||
	    !strstr(output, "fs.suid_dumpable = 2") ||
	    !strstr(output,
		    "Capabilities with related system context: 1"))
		fail("Related system context was not grouped by capability");
	if (count_text(output, "  CAP_SYS_PTRACE\n") != 1)
		fail("CAP_SYS_PTRACE system context was duplicated");
	if (strstr(output, "  CAP_DAC_OVERRIDE\n") ||
	    strstr(output, "capability needed") ||
	    strstr(output, "CONDITIONAL CAPABILITIES"))
		fail("System context was presented as capability evidence");

	free(output);
	setup_analysis(&service);
	state.service_file = NULL;
	state.service_cfg = NULL;
	state.capset_observed = 0;
	memset(&state.app.capset, 0, sizeof(state.app.capset));
	output = capture_analysis();
	if (!contains_tokens(output,
		(const char *const[]) {
			"capng_updatev(CAPNG_ADD",
			"CAP_DAC_OVERRIDE",
			"CAP_FOWNER",
			"CAP_NET_BIND_SERVICE",
			"-1);",
		}, 5) || strstr(output, ", DAC_OVERRIDE"))
		fail("C recommendation did not use CAP_ constants");

	free(output);
	puts("cap-audit service recommendation tests passed");
	return 0;
}
