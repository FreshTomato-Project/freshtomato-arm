// SPDX-License-Identifier: GPL-2.0-or-later
/* cap_audit_namespace_test.c -- capability recommendation scope tests
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
	return cap == CAP_SYS_ADMIN ? "sys_admin" : "unknown";
}

const char *syscall_name_from_nr(int nr)
{
	(void)nr;
	return "mount";
}

/*
 * update_reason_to - omit diagnostic allocation in namespace-only tests.
 * @target: unused diagnostic destination.
 * @syscall_nr: unused syscall number.
 *
 * Returns no value; namespace assertions do not depend on reason text.
 */
void update_reason_to(char **target, int syscall_nr)
{
	(void)target;
	(void)syscall_nr;
}

int cap_required_union(const struct cap_check *check)
{
	return check->needed || check->op_needed;
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
	return check->op_reason ? check->op_reason : check->reason;
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

static char *analyze_event(__u32 target_ns, int result)
{
	service_config_t service = {
		.user_raw = "validation-user",
		.user_uid = 1000,
		.user_primary_gid = 1000,
		.user_is_set = true,
		.user_resolved = true,
		.exec_start = "/usr/bin/validation-target",
	};
	struct cap_event event = {
		.pid = 1234,
		.capability = CAP_SYS_ADMIN,
		.result = result,
		.syscall_nr = 165,
		.targ_ns_inum = target_ns,
	};

	memset(&state, 0, sizeof(state));
	state.app.exe = "/usr/bin/validation-target";
	state.app.pid = 1234;
	state.app.prog_type = ELF;
	strcpy(state.app.kernel_version, "validation");
	state.service_file = "/tmp/validation.service";
	state.service_cfg = &service;
	state.baseline_user_ns_inum = 100;

	if (handle_cap_event(NULL, &event, sizeof(event)) != 0)
		fail("Event handler rejected test event");
	if ((target_ns != 0 && target_ns != state.baseline_user_ns_inum) !=
	    state.foreign_target_ns_observed)
		fail("Event handler misclassified target namespace");
	return capture_analysis();
}

static void expect_suppressed(char *output)
{
	if (!strstr(output, "Warning:"))
		fail("Namespace warning was not emitted");
	if (strstr(output, "AmbientCapabilities=") ||
	    strstr(output, "CapabilityBoundingSet="))
		fail("Foreign namespace produced service capability directives");
	if (!strstr(output, "sys_admin"))
		fail("Foreign namespace observation was hidden from diagnostics");
	free(output);
}

int main(void)
{
	char *output;

	output = analyze_event(100, 1);
	if (!strstr(output, "AmbientCapabilities=sys_admin") ||
	    !strstr(output, "CapabilityBoundingSet=sys_admin"))
		fail("Baseline namespace recommendation was suppressed");
	free(output);

	expect_suppressed(analyze_event(200, 1));
	expect_suppressed(analyze_event(200, 0));

	puts("cap-audit namespace recommendation tests passed");
	return 0;
}
