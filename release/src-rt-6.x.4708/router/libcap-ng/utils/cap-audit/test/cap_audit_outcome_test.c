// SPDX-License-Identifier: GPL-2.0-or-later
/* cap_audit_outcome_test.c -- syscall outcome correlation tests
 * Copyright 2026 Red Hat Inc.
 * All Rights Reserved.
 */

#include "config.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>

#include "cap_audit.h"

#define TEST_MOUNT_NR		1001
#define TEST_IOCTL_NR		1002
#define TEST_PROCESS_VM_NR	1003
#define TEST_OPENAT_NR		1004
#define TEST_BIND_NR		1005
#define TEST_LINK_NR		1006
#define TEST_KILL_NR		1007
#define TEST_ACCESS_NR		1008
#define TEST_CAPSET_NR		1009
#define TEST_ID_NR		1010
#define TEST_ERESTARTSYS	512

static const char *const id_calls[] = {
	"setuid", "setuid32", "setreuid", "setreuid32",
	"setresuid", "setresuid32", "setfsuid", "setfsuid32",
	"setgid", "setgid32", "setregid", "setregid32",
	"setresgid", "setresgid32", "setfsgid", "setfsgid32",
	"setgroups", "setgroups32",
};

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
	case CAP_DAC_READ_SEARCH:
		return "dac_read_search";
	case CAP_FOWNER:
		return "fowner";
	case CAP_KILL:
		return "kill";
	case CAP_SETGID:
		return "setgid";
	case CAP_SETUID:
		return "setuid";
	case CAP_SETPCAP:
		return "setpcap";
	case CAP_NET_BIND_SERVICE:
		return "net_bind_service";
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
	if (nr >= TEST_ID_NR &&
	    nr - TEST_ID_NR < (int)(sizeof(id_calls) / sizeof(id_calls[0])))
		return id_calls[nr - TEST_ID_NR];
	switch (nr) {
	case TEST_MOUNT_NR:
		return "mount";
	case TEST_IOCTL_NR:
		return "ioctl";
	case TEST_PROCESS_VM_NR:
		return "process_vm_readv";
	case TEST_OPENAT_NR:
		return "openat";
	case TEST_BIND_NR:
		return "bind";
	case TEST_LINK_NR:
		return "link";
	case TEST_KILL_NR:
		return "kill";
	case TEST_ACCESS_NR:
		return "access";
	case TEST_CAPSET_NR:
		return "capset";
	default:
		return NULL;
	}
}

char *json_escape(const char *input)
{
	return input ? strdup(input) : strdup("");
}

/*
 * update_reason_to - provide deterministic reasons for synthetic syscalls.
 * @target: caller-owned startup or operational reason to replace.
 * @syscall_nr: synthetic syscall number.
 *
 * Returns no value; replaces the reason with allocated text or NULL.
 */
void update_reason_to(char **target, int syscall_nr)
{
	const char *name = syscall_name_from_nr(syscall_nr);

	free(*target);
	if (asprintf(target, "Used by %s", name ? name : "unknown") < 0)
		*target = NULL;
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

static void emit_check(int cap, int syscall_nr, int result)
{
	struct cap_event event = {
		.pid = 1234,
		.tid = 1234,
		.capability = cap,
		.result = result,
		.syscall_nr = syscall_nr,
		.event_type = CAP_EVENT_CHECK,
	};

	if (handle_cap_event(NULL, &event, sizeof(event)) != 0)
		fail("Capability event was rejected");
}

static void emit_outcome(int cap, int syscall_nr, __s64 result)
{
	struct cap_event event = {
		.pid = 1234,
		.syscall_nr = syscall_nr,
		.syscall_ret = result,
		.denied_caps = 1ULL << cap,
		.event_type = CAP_EVENT_SYSCALL_RESULT,
	};

	if (handle_cap_event(NULL, &event, sizeof(event)) != 0)
		fail("Syscall outcome event was rejected");
}

static void emit_capset(__s64 result, __u64 effective, __u64 permitted,
			__u64 inheritable)
{
	struct cap_event event = {
		.pid = 1235,
		.syscall_ret = result,
		.capset_effective = effective,
		.capset_permitted = permitted,
		.capset_inheritable = inheritable,
		.event_type = CAP_EVENT_CAPSET,
	};

	if (handle_cap_event(NULL, &event, sizeof(event)) != 0)
		fail("capset event was rejected");
}

static void setup_events(void)
{
	int i;

	memset(&state, 0, sizeof(state));
	state.app.exe = "/usr/bin/outcome-target";
	state.app.pid = 1234;
	state.app.prog_type = ELF;
	strcpy(state.app.kernel_version, "validation");

	emit_check(CAP_NET_BIND_SERVICE, TEST_BIND_NR, 1);

	emit_check(CAP_SYS_ADMIN, TEST_MOUNT_NR, 0);
	emit_outcome(CAP_SYS_ADMIN, TEST_MOUNT_NR, -EPERM);
	emit_check(CAP_SYS_ADMIN, TEST_MOUNT_NR, 0);
	emit_outcome(CAP_SYS_ADMIN, TEST_MOUNT_NR, 0);
	emit_check(CAP_SYS_ADMIN, TEST_MOUNT_NR, 0);
	emit_outcome(CAP_SYS_ADMIN, TEST_MOUNT_NR, -ENOENT);

	emit_check(CAP_DAC_OVERRIDE, TEST_IOCTL_NR, 0);
#ifdef EBADFD
	emit_outcome(CAP_DAC_OVERRIDE, TEST_IOCTL_NR, -EBADFD);
#else
	emit_outcome(CAP_DAC_OVERRIDE, TEST_IOCTL_NR, -EBADF);
#endif

	for (i = 0; i < 3; i++) {
		emit_check(CAP_FOWNER, TEST_LINK_NR, 0);
		emit_outcome(CAP_FOWNER, TEST_LINK_NR, 0);
	}
	for (i = 0; i < 2; i++) {
		emit_check(CAP_DAC_READ_SEARCH, TEST_ACCESS_NR, 0);
		emit_outcome(CAP_DAC_READ_SEARCH, TEST_ACCESS_NR, 0);
	}
	emit_check(CAP_DAC_READ_SEARCH, TEST_ACCESS_NR, 0);
	emit_outcome(CAP_DAC_READ_SEARCH, TEST_ACCESS_NR, -ENOENT);

	emit_check(CAP_KILL, TEST_KILL_NR, 0);
	emit_outcome(CAP_KILL, TEST_KILL_NR, -EINTR);
	emit_check(CAP_KILL, TEST_KILL_NR, 0);
	emit_outcome(CAP_KILL, TEST_KILL_NR, -TEST_ERESTARTSYS);

	for (i = 0; i < 20; i++) {
		emit_check(CAP_SYS_PTRACE, TEST_PROCESS_VM_NR, 0);
		emit_outcome(CAP_SYS_PTRACE, TEST_PROCESS_VM_NR, 0);
	}
	emit_check(CAP_SYS_PTRACE, TEST_PROCESS_VM_NR, 0);
	emit_outcome(CAP_SYS_PTRACE, TEST_PROCESS_VM_NR, -EINTR);

	/* A success correlated to another capability must not count here. */
	emit_check(CAP_NET_BIND_SERVICE, TEST_OPENAT_NR, 0);
	emit_outcome(CAP_NET_BIND_SERVICE, TEST_OPENAT_NR, 0);
	emit_check(CAP_CHOWN, TEST_OPENAT_NR, 0);

	/* Failed capset payloads are diagnostic noise, not compatibility input. */
	emit_capset(-EPERM, 1ULL << CAP_SYS_CHROOT,
		    1ULL << CAP_SYS_CHROOT, 0);
	emit_capset(0, (1ULL << CAP_NET_BIND_SERVICE) |
		       (1ULL << CAP_SYS_RESOURCE),
		    (1ULL << CAP_NET_BIND_SERVICE) |
		       (1ULL << CAP_SYS_RESOURCE), 0);
}

typedef void (*output_fn)(void);

static char *capture_output(output_fn output, int fd)
{
	FILE *capture;
	char *text;
	long len;
	int saved_fd;

	capture = tmpfile();
	if (!capture)
		fail("Failed to create output file");
	saved_fd = dup(fd);
	if (saved_fd < 0)
		fail("Failed to save output descriptor");
	if (fflush(NULL) || dup2(fileno(capture), fd) < 0)
		fail("Failed to capture output");

	output();

	if (fflush(NULL) || dup2(saved_fd, fd) < 0)
		fail("Failed to restore output descriptor");
	close(saved_fd);
	if (fseek(capture, 0, SEEK_END) != 0)
		fail("Failed to seek captured output");
	len = ftell(capture);
	if (len < 0 || fseek(capture, 0, SEEK_SET) != 0)
		fail("Failed to measure captured output");
	text = malloc((size_t)len + 1);
	if (!text)
		fail("Failed to allocate captured output");
	if (fread(text, 1, (size_t)len, capture) != (size_t)len)
		fail("Failed to read captured output");
	text[len] = '\0';
	fclose(capture);

	return text;
}

static void expect_text(const char *output, const char *expected)
{
	if (!strstr(output, expected))
		fail(expected);
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

static void test_human_output(void)
{
	service_config_t service = {
		.user_raw = "validation-user",
		.user_uid = 1000,
		.user_primary_gid = 1000,
		.user_is_set = true,
		.user_resolved = true,
		.exec_start = "/usr/bin/outcome-target",
		.bounding = {
			.seen = true,
		},
	};
	char *output;

	output = capture_output(analyze_capabilities, STDOUT_FILENO);
	expect_text(output, "mount: 1 failed with -EPERM");
	expect_text(output, "mount: 1 failed with -ENOENT");
#ifdef EBADFD
	expect_text(output, "ioctl: 1 failed with -EBADFD");
#else
	expect_text(output, "ioctl: 1 failed with -EBADF");
#endif
	expect_text(output, "process_vm_readv: 20 succeeded");
	expect_text(output, "process_vm_readv: 1 interrupted with -EINTR");
	expect_text(output, "link: 3 succeeded");
	expect_text(output, "access: 2 succeeded");
	expect_text(output, "access: 1 failed with -ENOENT");
	expect_text(output, "kill: 1 interrupted with -EINTR");
	expect_text(output, "kill: 1 interrupted with -ERESTARTSYS");
	expect_text(output, "CAPSET-ONLY CAPABILITIES:");
	expect_text(output, "sys_resource (#24)");
	expect_text(output,
		    "Requested by successful capset in: effective, permitted");
	expect_text(output, "compatibility constraint");
	expect_text(output, "confirmed");
	expect_text(output, "functional use");
	expect_text(output, "Successful capset calls: 1");
	expect_text(output, "Capset-only capabilities: 1");
	expect_text(output, "CAPABILITIES WITH ONLY NOT-GRANTED CHECKS:");
	expect_text(output, "Capability checks returning not granted: 1");
	expect_text(output,
		    "Outcomes of syscall invocations containing such a check:");
	expect_text(output, "Associated syscall invocations:");
	expect_text(output, "Failure categories:");
	expect_text(output, "sys_admin: Manual investigation required");
	expect_text(output, "sys_ptrace: Manual investigation required");
	expect_text(output, "chown: Additional evidence required");
	expect_text(output, "openat: outcome unavailable");
	expect_text(output, "kill: Additional evidence required");
	expect_text(output, "fowner: Omitted; associated syscalls succeeded");
	expect_text(output,
		    "dac_read_search: Omitted; successes and non-permission "
		    "failures observed");
	expect_text(output,
		    "Associated syscall invocations: 2 succeeded and 1 failed");
	expect_text(output, "non-permission reasons");
	expect_text(output,
		    "not-granted capability checks are not established");
	expect_text(output, "cause of those failures");
	expect_text(output, "Manual investigation required: 2");
	expect_text(output, "Additional evidence required: 2");
	expect_text(output, "Omitted after associated syscall success: 1");
	expect_text(output,
		    "Omitted after mixed success/non-permission failure: 1");
	expect_text(output, "Capability check not established as cause: 1");
	expect_text(output,
		    "CapabilityBoundingSet=net_bind_service sys_resource");
	free(output);

	service.bounding.caps[CAP_NET_BIND_SERVICE] = true;
	state.service_file = "/tmp/outcome.service";
	state.service_cfg = &service;
	output = capture_output(analyze_capabilities, STDOUT_FILENO);
	expect_text(output, "sys_admin: Manual investigation required");
	expect_text(output, "Capability is absent from the configured");
	expect_text(output, "Mixed capability-check results: net_bind_service");
	expect_text(output,
		    "Capset-only compatibility capabilities: sys_resource");
	expect_text(output, "sys_resource is absent from the configured");
	expect_text(output,
		    "Detailed evidence appears in CAPABILITIES WITH ONLY");
	if (count_text(output, "mount: 1 failed with -EPERM") != 1 ||
	    count_text(output, "mount: 1 failed with -ENOENT") != 1)
		fail("Service recommendations duplicated syscall outcomes");
	expect_text(output,
		    "CapabilityBoundingSet=net_bind_service sys_resource");
	free(output);
	state.service_cfg = NULL;
}

static void test_structured_output(void)
{
	char *output;

	output = capture_output(output_json, STDOUT_FILENO);
	expect_text(output, "\"assessment\": \"permission_failure\"");
	expect_text(output, "\"assessment\": \"mixed_success_interruption\"");
	expect_text(output,
		    "\"assessment\": \"mixed_success_other_failure\"");
	expect_text(output, "\"assessment\": \"denial_not_established\"");
	expect_text(output, "\"assessment\": \"inconclusive\"");
	expect_text(output, "\"assessment\": \"succeeded_despite_denial\"");
	expect_text(output, "\"return_name\": \"EPERM\"");
	expect_text(output, "\"return_name\": \"ENOENT\"");
	expect_text(output, "\"not_granted_checks\": 1");
	expect_text(output, "\"successful_capset_calls\": 1");
	expect_text(output, "\"capset_only_capabilities\": [");
	expect_text(output, "\"name\": \"sys_resource\"");
	expect_text(output,
		    "\"requested_sets\": [\"effective\", \"permitted\"]");
	expect_text(output, "\"denied_syscalls\": [");
	expect_text(output, "\"return_name\": \"ERESTARTSYS\",\n"
		    "          \"errno\": null");
#ifdef EBADFD
	expect_text(output, "\"return_name\": \"EBADFD\"");
#endif
	expect_text(output, "\"count\": 20");
	free(output);

	output = capture_output(output_yaml, STDOUT_FILENO);
	expect_text(output, "assessment: permission_failure");
	expect_text(output, "assessment: mixed_success_interruption");
	expect_text(output, "assessment: mixed_success_other_failure");
	expect_text(output, "assessment: denial_not_established");
	expect_text(output, "assessment: inconclusive");
	expect_text(output, "assessment: succeeded_despite_denial");
	expect_text(output, "return_name: EPERM");
	expect_text(output, "return_name: ENOENT");
	expect_text(output, "not_granted_checks: 1");
	expect_text(output, "successful_capset_calls: 1");
	expect_text(output, "capset_only_capabilities:");
	expect_text(output, "name: sys_resource");
	expect_text(output, "denied_syscalls:");
	expect_text(output, "return_name: ERESTARTSYS\n"
		    "        errno: null");
	expect_text(output, "count: 20");
	free(output);
}

static void test_capset_without_setpcap(void)
{
	pid_t child = fork();
	int status;

	if (child < 0)
		fail("Failed to fork capset test");
	if (child == 0) {
		struct __user_cap_header_struct header = {
			.version = _LINUX_CAPABILITY_VERSION_3,
		};
		struct __user_cap_data_struct data[2] = { 0 };

		/*
		 * Drop every capability in this child, then repeat the call.
		 * The second capset succeeds despite its SETPCAP check failing.
		 * This is the kernel behavior behind the optional-check filter.
		 */
		if (syscall(SYS_capset, &header, data) != 0 ||
		    syscall(SYS_capget, &header, data) != 0)
			_exit(1);
		if (data[0].effective || data[0].permitted ||
		    data[0].inheritable || data[1].effective ||
		    data[1].permitted || data[1].inheritable)
			_exit(2);
		if (syscall(SYS_capset, &header, data) != 0)
			_exit(3);
		/* Adding outside old I | P must still fail without SETPCAP. */
		data[0].inheritable = 1U << CAP_CHOWN;
		if (syscall(SYS_capset, &header, data) != -1 || errno != EPERM)
			_exit(4);
		_exit(0);
	}
	if (waitpid(child, &status, 0) != child ||
	    !WIFEXITED(status) || WEXITSTATUS(status) != 0)
		fail("Kernel capset behavior did not match optional-check rules");
}

static void test_optional_setpcap(void)
{
	struct cap_event event = {
		.pid = 1234,
		.capability = CAP_SETPCAP,
		.result = 1,
		.syscall_nr = TEST_CAPSET_NR,
		.event_type = CAP_EVENT_CHECK,
		.capset_inh_optional = 1,
	};
	struct cap_check *check;
	char *output;

	memset(&state, 0, sizeof(state));
	state.app.exe = "/usr/bin/capset-target";
	state.app.pid = event.pid;
	state.app.capset_nr = TEST_CAPSET_NR;
	state.baseline_user_ns_inum = 100;
	check = &state.app.checks[CAP_SETPCAP];

	handle_cap_event(NULL, &event, sizeof(event));
	if (!state.capset_observed || cap_total_checks(check) != 0)
		fail("Optional SETPCAP check polluted accounting or lost phase");

	/* The same optional probe from a child is harmless, even if denied. */
	event.pid++;
	event.result = 0;
	event.targ_ns_inum = 200;
	handle_cap_event(NULL, &event, sizeof(event));
	if (cap_total_checks(check) != 0 || !state.foreign_target_ns_observed)
		fail("Optional child check lost namespace scope or was counted");

	/* A required/unknown capset check and a use outside capset survive. */
	event.capset_inh_optional = 0;
	event.result = 1;
	handle_cap_event(NULL, &event, sizeof(event));
	event.syscall_nr = TEST_IOCTL_NR;
	handle_cap_event(NULL, &event, sizeof(event));
	if (cap_total_granted(check) != 2)
		fail("Non-optional SETPCAP use was suppressed");

	/* Optional use must not erase an explicit successful capset request. */
	free(check->reason);
	free(check->op_reason);
	memset(check, 0, sizeof(*check));
	state.foreign_target_ns_observed = false;
	emit_capset(0, 1ULL << CAP_SETPCAP, 1ULL << CAP_SETPCAP, 0);
	if (!cap_is_capset_only(CAP_SETPCAP))
		fail("Explicit SETPCAP capset request was suppressed");
	output = capture_output(analyze_capabilities, STDOUT_FILENO);
	expect_text(output, "CapabilityBoundingSet=setpcap");
	free(output);
}

/*
 * test_event_reasons - keep startup and operational reasons independent.
 *
 * Returns no value; fails if either event branch selects the wrong field
 * when calling the shared reason helper.
 */
static void test_event_reasons(void)
{
	struct cap_check *check = &state.app.checks[CAP_CHOWN];

	memset(&state, 0, sizeof(state));
	state.app.pid = 1234;
	emit_check(CAP_CHOWN, TEST_OPENAT_NR, 1);
	state.capset_observed = true;
	emit_check(CAP_CHOWN, TEST_IOCTL_NR, 1);
	if (!check->reason || strcmp(check->reason, "Used by openat") ||
	    !check->op_reason || strcmp(check->op_reason, "Used by ioctl"))
		fail("Startup and operational reasons were not kept separate");
	free(check->reason);
	free(check->op_reason);
}

/* Emit the completion of a keepcaps prctl on a selected target thread. */
static void emit_keepcaps(__u32 tid, int enabled, __s64 ret)
{
	struct cap_event event = {
		.pid = 1234,
		.tid = tid,
		.event_type = CAP_EVENT_KEEPCAPS,
		.keepcaps = enabled,
		.syscall_ret = ret,
	};

	handle_cap_event(NULL, &event, sizeof(event));
}

/* Free diagnostic storage so repeated synthetic runs also test ownership. */
static void reset_keepcaps_test(void)
{
	int cap;

	if (state.keepcaps_windows)
		fail("Test leaked a keepcaps window");
	for (cap = 0; cap <= CAP_LAST_CAP; cap++) {
		free(state.app.checks[cap].reason);
		free(state.app.checks[cap].op_reason);
		free(state.app.checks[cap].denied_syscalls);
		free(state.app.checks[cap].outcomes);
	}
	memset(&state, 0, sizeof(state));
	state.app.pid = 1234;
	state.app.exe = "/usr/bin/keepcaps-target";
	state.app.prog_type = ELF;
	state.app.capset_nr = TEST_CAPSET_NR;
}

/* Model capng_change_id's temporary set, ID changes, then final capset. */
static void test_keepcaps_transition(void)
{
	struct cap_event capset = {
		.pid = 1234,
		.syscall_nr = TEST_CAPSET_NR,
		.event_type = CAP_EVENT_CAPSET,
		.capset_permitted = (1ULL << CAP_SETGID) |
			(1ULL << CAP_SETUID) | (1ULL << CAP_NET_BIND_SERVICE),
	};
	service_config_t service = { 0 };
	char *output, *operational;

	reset_keepcaps_test();
	emit_keepcaps(1234, 1, 0);
	handle_cap_event(NULL, &capset, sizeof(capset));
	emit_check(CAP_SETGID, TEST_ID_NR + 12, 1); /* setresgid */
	emit_check(CAP_SETGID, TEST_ID_NR + 16, 1); /* setgroups */
	emit_check(CAP_SETUID, TEST_ID_NR + 4, 1); /* setresuid */
	emit_keepcaps(1234, 0, 0);
	capset.capset_permitted = 1ULL << CAP_NET_BIND_SERVICE;
	handle_cap_event(NULL, &capset, sizeof(capset));
	emit_check(CAP_NET_BIND_SERVICE, TEST_BIND_NR, 1);
	finish_cap_events();
	if (state.app.checks[CAP_SETGID].granted != 2 ||
	    state.app.checks[CAP_SETUID].granted != 1 ||
	    state.app.checks[CAP_SETGID].op_count ||
	    state.app.checks[CAP_SETUID].op_count ||
	    state.app.checks[CAP_NET_BIND_SERVICE].op_granted != 1 ||
	    state.keepcaps_incomplete ||
	    !cap_is_compat_requirement(CAP_SETUID) ||
	    !cap_is_compat_requirement(CAP_SETGID))
		fail("Credential-transition checks were not initialization");
	if (strcmp(state.app.checks[CAP_SETGID].reason, "Used by setresgid") ||
	    strcmp(state.app.checks[CAP_SETUID].reason, "Used by setresuid"))
		fail("Credential-transition reasons were lost");
	output = capture_output(analyze_capabilities, STDOUT_FILENO);
	expect_text(output, "capng_updatev(CAPNG_ADD");
	if (strstr(output, "CAP_SETUID") || strstr(output, "CAP_SETGID"))
		fail("Programmatic snippet retained identity capabilities");
	free(output);

	output = capture_output(output_json, STDOUT_FILENO);
	expect_text(output, "\"keepcaps_transition_incomplete\": false");
	operational = strstr(output, "\"operational_capabilities\"");
	if (!operational || strstr(operational, "\"name\": \"setuid\"") ||
	    strstr(operational, "\"name\": \"setgid\""))
		fail("JSON retained transition capabilities as operational");
	free(output);
	output = capture_output(output_yaml, STDOUT_FILENO);
	expect_text(output, "keepcaps_transition_incomplete: false");
	operational = strstr(output, "operational_capabilities:");
	if (!operational || strstr(operational, "name: setuid") ||
	    strstr(operational, "name: setgid"))
		fail("YAML retained transition capabilities as operational");
	free(output);

	/* Both application requests and explicit unit requests get guidance. */
	state.service_cfg = &service;
	state.service_file = "/tmp/keepcaps.service";
	output = capture_output(analyze_capabilities, STDOUT_FILENO);
	expect_text(output, "Credential transition: setgid is requested");
	expect_text(output, "Credential transition: setuid is requested");
	expect_text(output, "CapabilityBoundingSet=setgid setuid net_bind_service");
	free(output);
	state.keepcaps_incomplete = true;
	output = capture_output(analyze_capabilities, STDOUT_FILENO);
	if (strstr(output, "Credential transition:"))
		fail("Incomplete tracing still claimed capabilities unneeded");
	free(output);
	state.keepcaps_incomplete = false;
	state.app.capset.permitted = 0;
	service.bounding.seen = true;
	service.bounding.caps[CAP_SETUID] = true;
	output = capture_output(analyze_capabilities, STDOUT_FILENO);
	expect_text(output, "Credential transition: setuid is requested");
	if (strstr(output, "Credential transition: setgid is requested"))
		fail("Unrequested capability labeled requested");
	free(output);
	emit_check(CAP_SETUID, TEST_ID_NR, 1);
	emit_check(CAP_SETGID, TEST_ID_NR + 8, 0);
	service.bounding.caps[CAP_SETGID] = true;
	output = capture_output(analyze_capabilities, STDOUT_FILENO);
	if (strstr(output, "Credential transition:"))
		fail("Operational UID/GID checks did not block unneeded guidance");
	free(output);
	reset_keepcaps_test();
	/* Checks before the first capset are already initialization work. */
	emit_keepcaps(1234, 1, 0);
	emit_check(CAP_SETUID, TEST_ID_NR, 1);
	emit_keepcaps(1234, 0, 0);
	if (state.app.checks[CAP_SETUID].count != 1 ||
	    state.app.checks[CAP_SETUID].granted != 1 ||
	    state.app.checks[CAP_SETUID].op_count || state.capset_observed)
		fail("Pre-capset credential checks were lost or double counted");
	reset_keepcaps_test();
}

/* Exercise thread isolation, repeat enables, denials, and every ID variant. */
static void test_keepcaps_scope(void)
{
	struct cap_event other = {
		.pid = 1234,
		.tid = 1235,
		.capability = CAP_SETUID,
		.syscall_nr = TEST_ID_NR,
		.result = 1,
		.event_type = CAP_EVENT_CHECK,
	};
	size_t i;

	reset_keepcaps_test();
	/* The ordinary capset-only case must keep genuine operational use. */
	emit_check(CAP_SETUID, TEST_ID_NR, 1);
	emit_check(CAP_SETPCAP, TEST_CAPSET_NR, 1);
	emit_check(CAP_SETUID, TEST_ID_NR, 1);
	if (state.app.checks[CAP_SETUID].granted != 1 ||
	    state.app.checks[CAP_SETUID].op_granted != 1)
		fail("Capset-only phase tracking changed");
	reset_keepcaps_test();
	state.capset_observed = 1;
	emit_keepcaps(1234, 1, 0);
	for (i = 0; i < sizeof(id_calls) / sizeof(id_calls[0]); i++) {
		emit_check(i < 8 ? CAP_SETUID : CAP_SETGID, TEST_ID_NR + i, 1);
		emit_keepcaps(1234, 1, 0); /* Setting a flag is not nesting. */
	}
	emit_check(CAP_SETUID, TEST_ID_NR, 0);
	emit_outcome(CAP_SETUID, TEST_ID_NR, -EPERM);
	emit_check(CAP_SETUID, TEST_IOCTL_NR, 1); /* Not an ID change. */
	emit_check(CAP_SETGID, -1, 1); /* Unknown syscall: do not guess. */
	handle_cap_event(NULL, &other, sizeof(other));
	emit_keepcaps(1235, 0, 0); /* Another thread cannot close the pair. */
	if (!state.keepcaps_windows || state.app.checks[CAP_SETUID].granted)
		fail("Another thread closed the keepcaps window");
	emit_keepcaps(1235, 1, 0);
	handle_cap_event(NULL, &other, sizeof(other));
	emit_keepcaps(1234, 0, 0);
	emit_keepcaps(1235, 0, 0);
	if (state.app.checks[CAP_SETUID].granted != 9 ||
	    state.app.checks[CAP_SETUID].denied != 1 ||
	    state.app.checks[CAP_SETGID].granted != 10 ||
	    state.app.checks[CAP_SETUID].op_granted != 2 ||
	    state.app.checks[CAP_SETGID].op_granted != 1 ||
	    state.app.checks[CAP_SETUID].outcome_count != 1 ||
	    state.app.checks[CAP_SETUID].outcomes[0].result != -EPERM ||
	    strcmp(state.app.checks[CAP_SETUID].op_reason, "Used by ioctl"))
		fail("Keepcaps scope, reasons, or denial evidence changed");
	reset_keepcaps_test();
}

/* Incomplete/failed pairs warn and must not erase operational requirements. */
static void test_keepcaps_incomplete(void)
{
	struct cap_event end = {
		.pid = 1234,
		.tid = 1234,
		.event_type = CAP_EVENT_TASK_END,
	};
	char *output;

	reset_keepcaps_test();
	state.capset_observed = 1;
	emit_keepcaps(1234, 1, -EPERM);
	emit_check(CAP_SETUID, TEST_ID_NR, 1);
	emit_keepcaps(1234, 0, 0);
	if (state.app.checks[CAP_SETUID].op_granted != 1 ||
	    state.keepcaps_windows || state.keepcaps_init_caps)
		fail("Failed keepcaps enable changed phase accounting");
	emit_keepcaps(1234, 1, 0);
	emit_check(CAP_SETUID, TEST_ID_NR, 1);
	emit_check(CAP_SETGID, TEST_ID_NR + 8, 0);
	emit_outcome(CAP_SETGID, TEST_ID_NR + 8, -EPERM);
	emit_keepcaps(1234, 0, -EPERM);
	output = capture_output(finish_cap_events, STDERR_FILENO);
	expect_text(output, "Warning: PID 1234 TID 1234");
	expect_text(output, "no successful disable was observed");
	free(output);
	if (!state.keepcaps_incomplete || state.keepcaps_windows ||
	    state.app.checks[CAP_SETUID].op_granted != 2 ||
	    state.app.checks[CAP_SETGID].op_denied != 1 ||
	    state.app.checks[CAP_SETGID].outcome_count != 1 ||
	    state.keepcaps_init_caps)
		fail("Incomplete keepcaps pair lost original accounting");
	output = capture_output(output_json, STDOUT_FILENO);
	expect_text(output, "\"keepcaps_transition_incomplete\": true");
	free(output);
	output = capture_output(output_yaml, STDOUT_FILENO);
	expect_text(output, "keepcaps_transition_incomplete: true");
	free(output);
	reset_keepcaps_test();
	state.capset_observed = 1;
	emit_keepcaps(1234, 1, 0);
	emit_check(CAP_SETUID, TEST_ID_NR, 1);
	handle_cap_event(NULL, &end, sizeof(end));
	emit_keepcaps(1234, 0, 0);
	if (!state.keepcaps_incomplete || state.keepcaps_windows ||
	    state.app.checks[CAP_SETUID].op_granted != 1)
		fail("Keepcaps calls paired across exec/exit");
	reset_keepcaps_test();
}

int main(void)
{
	test_keepcaps_transition();
	test_keepcaps_scope();
	test_keepcaps_incomplete();
	test_capset_without_setpcap();
	test_event_reasons();
	test_optional_setpcap();
	if (classify_syscall_outcome(0) != SYSCALL_OUTCOME_SUCCESS ||
	    classify_syscall_outcome(-EPERM) != SYSCALL_OUTCOME_PERMISSION ||
	    classify_syscall_outcome(-EACCES) != SYSCALL_OUTCOME_PERMISSION ||
	    classify_syscall_outcome(-EINTR) != SYSCALL_OUTCOME_INTERRUPTED ||
	    classify_syscall_outcome(-EBADF) != SYSCALL_OUTCOME_OTHER ||
	    classify_syscall_outcome(-TEST_ERESTARTSYS) !=
					SYSCALL_OUTCOME_INTERRUPTED ||
	    syscall_result_errno(-EPERM) != EPERM ||
	    syscall_result_errno(-TEST_ERESTARTSYS) != 0 ||
	    !syscall_result_name(-ENOENT) ||
	    strcmp(syscall_result_name(-ENOENT), "ENOENT") != 0 ||
	    !syscall_result_name(-TEST_ERESTARTSYS) ||
	    strcmp(syscall_result_name(-TEST_ERESTARTSYS),
		   "ERESTARTSYS") != 0)
		fail("Raw syscall outcomes were misclassified");

	setup_events();
	if (state.app.checks[CAP_CHOWN].outcome_count != 0 ||
	    state.app.checks[CAP_NET_BIND_SERVICE].outcome_count != 1)
		fail("Syscall outcome was correlated to the wrong capability");
	if (state.app.capset.successful_calls != 1 ||
	    !cap_is_capset_only(CAP_SYS_RESOURCE) ||
	    cap_is_capset_only(CAP_NET_BIND_SERVICE) ||
	    cap_requested_by_capset(CAP_SYS_CHROOT))
		fail("capset payloads were not filtered or classified correctly");
	test_human_output();
	test_structured_output();

	puts("cap-audit syscall outcome tests passed");
	return 0;
}
