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

#include "config.h"
#include <bpf/bpf.h>
#include <bpf/libbpf.h>
#include <errno.h>
#include <ctype.h>
#include <libaudit.h>
#include <linux/capability.h>
#include <limits.h>
#include <signal.h>
#include <sys/resource.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include "cap-ng.h"
#include "cap_audit.skel.h"

/*
 * Overview:
 * cap-audit launches a target application, traces that process tree using
 * eBPF hooks, and reports which Linux capabilities were actually exercised.
 * The userspace side performs three major jobs:
 *
 * (1) prepare the runtime environment by checking our own capabilities and
 * raising rlimits;
 * (2) coordinate with the eBPF program by registering the target PID before
 * exec() and consuming capability check events from the ring buffer; and
 * (3) analyze collected data to present required, conditional, and denied
 * capabilities in human and machine-readable formats.
 *
 * Core design problem:
 * when tracing as root, many capability checks come from kernel-internal
 * work under the same PID rather than from app logic. The tool uses a
 * three-layer noise filter pipeline, split between BPF and userspace, to
 * separate real requirements from incidental checks.
 *
 * Layer 1 - pre-exec noise (BPF phase gate):
 * after fork() but before execve() completes, the child is still this
 * auditor image. PATH lookup, directory traversal, and exec machinery can
 * trigger checks like DAC_READ_SEARCH/SYS_ADMIN/SETPCAP that are unrelated
 * to the target app. The BPF side tracks PID phases and suppresses all
 * capability events until sched_process_exec transitions the PID from
 * phase 1 (pre-exec) to phase 2 (post-exec).
 *
 * Layer 2 - post-exec startup noise (userspace startup gate):
 * immediately after exec and before main(), runtime linker activity
 * (mmap/brk/mprotect) can trigger repeated SYS_ADMIN checks via
 * security_mmap_addr(). Exec credential transitions also emit SYS_ADMIN and
 * SETPCAP from execve itself. handle_cap_event() applies two startup filters:
 * is_always_noise() unconditionally drops execve SYS_ADMIN/SETPCAP, and
 * is_startup_noise() drops mmap/brk/mprotect SYS_ADMIN until a genuine
 * capability event arrives and sets recording_ready.
 *
 * Layer 3 - shutdown noise (userspace drain gate):
 * when tracing stops, interpreter/runtime cleanup can recreate the same
 * mmap/brk SYS_ADMIN pattern. During final ring-buffer drain, shutting_down
 * reuses is_startup_noise() to suppress that shutdown-only chatter.
 *
 * The is_always_noise()/is_startup_noise() split is deliberate: always-noise
 * events are dropped without opening recording_ready. That keeps the startup
 * gate closed across shebang re-exec chains (script -> /usr/bin/env ->
 * interpreter), where multiple exec transitions would otherwise open the gate
 * early and let interpreter linker noise leak through.
 *
 * PID filtering remains central: parent registers the child immediately after
 * fork(), BPF follows forks/exits to keep the target set precise, and each
 * event carries capability, syscall context, namespace info, and result for
 * per-capability aggregation.
 *
 * Parent/child startup is synchronized with a pipe. The child blocks in
 * read(sync_pipe) until the parent has inserted its PID in target_pids and
 * written a go byte. That ordering prevents missed events and prevents events
 * from arriving before the PID is registered.
 */

typedef enum { UNSUPPORTED, ELF, PYTHON } type_t;
#define ELFMAGIC "\177ELF"

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
};

struct cap_check {
	int capability;
	unsigned long count;
	unsigned long granted;
	unsigned long denied;
	int needed;
	char *reason;
};

// Program global variables
struct app_caps {
	pid_t pid;
	char *exe;
	int execve_nr;
	int mmap_nr;
	int brk_nr;
	int mprotect_nr;
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

// Global program state
struct audit_state {
	struct cap_audit_bpf *skel;
	struct ring_buffer *rb;
	struct app_caps app;
	int recording_ready;
	int verbose;
	int json_output;
	int yaml_output;
	int sync_pipe[2];
	char **target_argv;
	volatile sig_atomic_t stop;
	int shutting_down;
};

static struct audit_state state;
static int audit_machine = -1;	// Hardware architecture (syscall lookup)

static int include_cap_in_recommendations(int cap)
{
	/*
	 * SETPCAP from capset(2) can be a legitimate app event (for example,
	 * libcap-ng adjusting process capabilities). But if the executable has
	 * file capabilities and CAP_SETPCAP is absent from that xattr, observed
	 * SETPCAP usually reflects internal capability-set bookkeeping rather
	 * than a deployment-time requirement to grant CAP_SETPCAP.
	 */
	if (cap == CAP_SETPCAP && state.app.file_caps &&
	    !state.app.file_setpcap)
		return 0;

	return 1;
}

/*
 * is_always_noise - capability checks that are never real application needs.
 *
 * SYS_ADMIN and SETPCAP from execve are always kernel-internal credential
 * transitions (install_exec_creds/commit_creds). No application needs these
 * capabilities for exec to succeed. Dropping these without setting
 * recording_ready keeps the startup gate closed across shebang re-exec
 * chains, so startup noise filtering still applies in the final interpreter.
 */
static int is_always_noise(const struct cap_event *e)
{
	if (e->syscall_nr == state.app.execve_nr &&
	   (e->capability == CAP_SYS_ADMIN ||
	    e->capability == CAP_SETPCAP))
		return 1;

	return 0;
}

/*
 * is_startup_noise - runtime linker noise filtered during startup only.
 *
 * SYS_ADMIN from mmap/brk/mprotect happens when root maps memory via
 * security_mmap_addr(). The runtime linker does this while loading shared
 * libraries. After startup (recording_ready=1), these are not filtered
 * because they could represent legitimate app usage (e.g., MAP_HUGETLB).
 */
static int is_startup_noise(const struct cap_event *e)
{
	if (e->capability == CAP_SYS_ADMIN &&
	    (e->syscall_nr == state.app.mmap_nr ||
	     e->syscall_nr == state.app.brk_nr ||
	     e->syscall_nr == state.app.mprotect_nr))
		return 1;

	return 0;
}

/*
 * resolve_target_exe - read resolved executable path for target process.
 * @pid: pid of the traced process.
 * @exepath: buffer for resolved path.
 * @exepath_len: size of buffer.
 *
 * Waits for /proc/<pid>/exe to update after exec by ignoring pointers to
 * the auditor binary. Returns 0 on success, -1 on error.
 */
static int resolve_target_exe(pid_t pid, char *exepath, size_t exepath_len)
{
	char linkpath[64];
	char selfpath[PATH_MAX];
	ssize_t len;
	ssize_t self_len;
	int tries = 50;

	if (snprintf(linkpath, sizeof(linkpath), "/proc/%d/exe", pid) < 0)
		return -1;

	self_len = readlink("/proc/self/exe", selfpath, sizeof(selfpath) - 1);
	if (self_len >= 0)
		selfpath[self_len] = '\0';

	while (tries--) {
		len = readlink(linkpath, exepath, exepath_len - 1);
		if (len < 0) {
			fprintf(stderr, "Warning: readlink(%s) failed: %s\n",
				linkpath, strerror(errno));
			return -1;
		}
		exepath[len] = '\0';

		if (self_len < 0 || strcmp(exepath, selfpath) != 0)
			break;

		if (tries == 0)
			fprintf(stderr,
				"Warning: %s still points to auditor binary (%s)\n",
				linkpath, exepath);
		usleep(10000);
	}

	return 0;
}

/*
 * inspect_target_file_caps - read file capability xattr of target program.
 * @pid: pid of the traced process.
 *
 * Uses /proc/<pid>/exe to query file capabilities with capng_get_caps_fd.
 * Sets flags describing whether file capabilities are present and whether
 * CAP_SETPCAP appears in that xattr. Returns 0 on success, -1 on error.
 */
static int inspect_target_file_caps(pid_t pid)
{
	char exepath[PATH_MAX];
	int fd;
	struct stat st;
	capng_results_t caps;

	state.app.file_caps = 0;
	state.app.file_setpcap = 0;

	if (resolve_target_exe(pid, exepath, sizeof(exepath)) < 0)
		return -1;

	fd = open(exepath, O_RDONLY | O_CLOEXEC);
	if (fd < 0) {
		fprintf(stderr, "Warning: open(%s) failed: %s\n",
			exepath, strerror(errno));
		return -1;
	}

	if (fstat(fd, &st) < 0) {
		fprintf(stderr, "Warning: fstat(%s) failed: %s\n",
			exepath, strerror(errno));
		close(fd);
		return -1;
	}
	if (!S_ISREG(st.st_mode)) {
		fprintf(stderr, "Warning: %s is not a regular file\n",
			exepath);
		close(fd);
		return -1;
	}

	capng_clear(CAPNG_SELECT_BOTH);
	if (capng_get_caps_fd(fd)) {
		if (errno != ENODATA)
			fprintf(stderr,
				"Warning: capng_get_caps_fd(%s) failed: %s\n",
				exepath, strerror(errno));
		close(fd);
		if (capng_get_caps_process())
			fprintf(stderr,
				"Warning: failed to restore process capabilities\n");
		return -1;
	}
	close(fd);

	caps = capng_have_capabilities(CAPNG_SELECT_CAPS);
	if (caps == CAPNG_NONE)
		caps = capng_have_permitted_capabilities();
	if (caps > CAPNG_NONE)
		state.app.file_caps = 1;

	if (capng_have_capability(CAPNG_PERMITTED, CAP_SETPCAP) ||
	    capng_have_capability(CAPNG_INHERITABLE, CAP_SETPCAP))
		state.app.file_setpcap = 1;

	if (state.verbose)
		printf("[*] File caps source: %s (has_caps=%d setpcap=%d)\n",
		       exepath, state.app.file_caps, state.app.file_setpcap);

	if (capng_get_caps_process())
		fprintf(stderr, "Warning: failed to restore process capabilities\n");

	return 0;
}

static const char *cap_name_safe(int cap)
{
	const char *name = capng_capability_to_name(cap);
	return name ? name : "unknown";
}

static void print_cap_name_upper(int cap)
{
	const char *name = cap_name_safe(cap);
	int i;

	for (i = 0; name[i]; i++)
		printf("%c", toupper((unsigned char)name[i]));
}

/*
 * sig_handler - handle termination signals.
 * @sig: signal number (unused).
 *
 * Sets the global stop flag so the main loop can exit cleanly. Returns
 * nothing.
 */
static void sig_handler(int sig __attribute__((unused)))
{
	// Signal just toggles stop flag; main loop polls this.
	state.stop = 1;
}

/*
 * set_memlock_rlimit - raise RLIMIT_MEMLOCK for BPF object loading.
 *
 * Returns 0 on success or -1 if the limit cannot be raised. Errors are
 * reported to stderr with a hint about missing privileges.
 */
static int set_memlock_rlimit(void)
{
	struct rlimit rlim_new = {
		.rlim_cur = RLIM_INFINITY,
		.rlim_max = RLIM_INFINITY,
	};

	if (setrlimit(RLIMIT_MEMLOCK, &rlim_new) < 0) {
		fprintf(stderr, "Error: Failed to raise memlock rlimit: %s\n",
			errno == EPERM ?
			"insufficient privileges" : strerror(errno));
		return -1;
	}

	return 0;
}

/*
 * init_capng - initialize libcap-ng state for the auditor.
 *
 * Clears cached capability information and refreshes it from the current
 * process. Returns 0 on success, -1 on failure.
 */
static int init_capng(void)
{
	capng_clear(CAPNG_SELECT_BOTH);

	if (capng_get_caps_process() != 0) {
		fprintf(stderr, "Error: Failed to get process capabilities\n");
		return -1;
	}

	return 0;
}

/*
 * check_auditor_caps - verify the auditor has the capabilities it needs.
 *
 * Ensures CAP_BPF/CAP_SYS_ADMIN and CAP_PERFMON/CAP_SYS_ADMIN are available
 * for loading and running the eBPF program. Warns if CAP_SYS_PTRACE is
 * absent. Returns 0 if requirements are satisfied, -1 otherwise.
 */
static int check_auditor_caps(void)
{
	if (!capng_have_capability(CAPNG_EFFECTIVE, CAP_BPF) &&
	    !capng_have_capability(CAPNG_EFFECTIVE, CAP_SYS_ADMIN)) {
		fprintf(
		    stderr,
		    "Error: Need CAP_BPF or CAP_SYS_ADMIN to run auditor\n");
		return -1;
	}

	if (!capng_have_capability(CAPNG_EFFECTIVE, CAP_PERFMON) &&
	    !capng_have_capability(CAPNG_EFFECTIVE, CAP_SYS_ADMIN)) {
		fprintf(stderr, "Error: Need CAP_PERFMON or CAP_SYS_ADMIN for "
			"perf events\n");
		return -1;
	}

	if (!capng_have_capability(CAPNG_EFFECTIVE, CAP_SYS_PTRACE))
		fprintf(stderr, "Warning: CAP_SYS_PTRACE not available, stack "
			"traces may be limited\n");

	return 0;
}

/*
 * set_target_pid - register a PID in the BPF target map for tracing.
 * @pid: process ID to watch.
 *
 * Looks up the target_pids map file descriptor, inserts the PID with a value
 * of 1 (phase 1: pre-exec), and optionally logs registration when verbose.
 * Returns 0 on success or -1 on error.
 */
static int set_target_pid(pid_t pid)
{
	int map_fd;
	__u8 val = 1;

	map_fd = bpf_map__fd(state.skel->maps.target_pids);
	if (map_fd < 0) {
		fprintf(stderr, "Error: Failed to get target_pids map fd\n");
		return -1;
	}

	if (bpf_map_update_elem(map_fd, &pid, &val, BPF_ANY) != 0) {
		fprintf(stderr, "Error: Failed to register target PID %d: %s\n",
			pid, strerror(errno));
		return -1;
	}

	if (state.verbose)
		printf("[*] Registered PID %d for tracing\n", pid);

	return 0;
}

/*
 * read_sysctl - a helper function to read a given sysctl value
 * @path: the path to the sysctl to read
 * @value: a pointer where the value is stored
 *
 * No return value
 */
static void read_sysctl(const char *path, int *value)
{
	FILE *f;

	f = fopen(path, "r");
	if (f) {
		if (fscanf(f, "%d", value) != 1)
			*value = -1;
		fclose(f);
	} else {
		*value = -1;
	}
}

/*
 * read_system_state - snapshot kernel tunables relevant to capabilities.
 * @app: application tracking structure to populate.
 *
 * Reads a handful of /proc/sys values that influence capability behavior
 * (ptrace scope, perf_event paranoid, BPF toggles, kernel version). Missing
 * files are recorded as -1 to indicate unknown. No return value.
 */
static void read_system_state(struct app_caps *app)
{
	FILE *f;

	// Each read is best-effort; -1 indicates the kernel entry was missing.
	read_sysctl("/proc/sys/kernel/yama/ptrace_scope",
		    &app->yama_ptrace_scope);
	read_sysctl("/proc/sys/kernel/kptr_restrict", &app->kptr_restrict);
	read_sysctl("/proc/sys/kernel/dmesg_restrict", &app->dmesg_restrict);
	read_sysctl("/proc/sys/kernel/modules_disabled",
		    &app->modules_disabled);
	read_sysctl("/proc/sys/kernel/perf_event_paranoid",
		    &app->perf_event_paranoid);
	read_sysctl("/proc/sys/kernel/unprivileged_bpf_disabled",
		    &app->unprivileged_bpf_disabled);
	read_sysctl("/proc/sys/net/core/bpf_jit_enable", &app->bpf_jit_enable);
	read_sysctl("/proc/sys/net/core/bpf_jit_harden", &app->bpf_jit_harden);
	read_sysctl("/proc/sys/net/core/bpf_jit_kallsyms",
		    &app->bpf_jit_kallsyms);
	read_sysctl("/proc/sys/vm/mmap_min_addr", &app->mmap_min_addr);
	read_sysctl("/proc/sys/fs/protected_hardlinks",
		    &app->protected_hardlinks);
	read_sysctl("/proc/sys/fs/protected_symlinks",
		    &app->protected_symlinks);
	read_sysctl("/proc/sys/fs/suid_dumpable", &app->suid_dumpable);

	f = fopen("/proc/sys/kernel/osrelease", "r");
	if (f) {
		if (!fgets(app->kernel_version, sizeof(app->kernel_version), f))
			app->kernel_version[0] = 0;
		app->kernel_version[strcspn(app->kernel_version, "\n")] = 0;
		fclose(f);
	}
}

/*
 * syscall_name_from_nr - resolve a syscall number to a name using libaudit.
 * @nr: syscall number.
 *
 * Detects the machine architecture once, then asks libaudit to translate the
 * syscall number. Returns the syscall name or NULL if not known.
 */
static const char *syscall_name_from_nr(int nr)
{
	if (audit_machine < 0)
		return NULL;

	return audit_syscall_to_name(nr, audit_machine);
}

/*
 * update_reason - create or refresh the human-readable reason for a cap.
 * @check: capability tracking entry to update.
 * @syscall_nr: syscall that triggered the capability check.
 *
 * Frees any existing reason, then builds a new string that ties the
 * capability to the triggering syscall. For unknown syscalls, uses a generic
 * message. On allocation failure, leaves reason NULL.
 */
static void update_reason(struct cap_check *check, int syscall_nr)
{
	const char *syscall_name;

	if (check->reason)
		free(check->reason);

	if (syscall_nr < 0) {
		if (asprintf(&check->reason,
			     "Used during capability check (syscall unknown)") < 0)
			check->reason = NULL;
		return;
	}

	syscall_name = syscall_name_from_nr(syscall_nr);
	if (asprintf(&check->reason, "Used by %s (syscall %d)",
		     syscall_name ? syscall_name : "unknown", syscall_nr) < 0)
		check->reason = NULL;
}

/*
 * json_escape - escape a string for JSON output.
 * @input: string to escape.
 *
 * Returns a newly allocated escaped string or NULL on allocation failure.
 */
static char *json_escape(const char *input)
{
	size_t i;
	size_t needed = 0;
	char *out;
	char *pos;

	if (!input)
		return strdup("");

	for (i = 0; input[i]; i++) {
		unsigned char c = input[i];

		switch (c) {
		case '\"':
		case '\\':
		case '\b':
		case '\f':
		case '\n':
		case '\r':
		case '\t':
			needed += 2;
			break;
		default:
			if (c < 0x20)
				needed += 6;
			else
				needed++;
			break;
		}
	}

	out = malloc(needed + 1);
	if (!out)
		return NULL;

	pos = out;
	for (i = 0; input[i]; i++) {
		unsigned char c = input[i];

		switch (c) {
		case '\"':
			*pos++ = '\\';
			*pos++ = '\"';
			break;
		case '\\':
			*pos++ = '\\';
			*pos++ = '\\';
			break;
		case '\b':
			*pos++ = '\\';
			*pos++ = 'b';
			break;
		case '\f':
			*pos++ = '\\';
			*pos++ = 'f';
			break;
		case '\n':
			*pos++ = '\\';
			*pos++ = 'n';
			break;
		case '\r':
			*pos++ = '\\';
			*pos++ = 'r';
			break;
		case '\t':
			*pos++ = '\\';
			*pos++ = 't';
			break;
		default:
			if (c < 0x20) {
				snprintf(pos, 7, "\\u%04x", c);
				pos += 6;
			} else {
				*pos++ = c;
			}
			break;
		}
	}
	*pos = '\0';

	return out;
}

/*
 * handle_cap_event - process one capability event from the ring buffer.
 * @ctx: unused callback context.
 * @data: pointer to struct cap_event from BPF.
 * @data_sz: size of the event (unused, validated by libbpf).
 *
 * Optionally prints verbose details, updates per-capability counters, and
 * marks capabilities as definitely needed when the kernel granted them.
 * Returns 0 to keep polling.
 */
static int handle_cap_event(void *ctx __attribute__((unused)), void *data,
		     size_t data_sz __attribute__((unused)))
{
	const struct cap_event *e = data;

	/*
	 * Filter block 1 (always-noise): drop exec credential-transition
	 * checks unconditionally. This must run first so these events do not
	 * set recording_ready and prematurely open the startup gate.
	 */
	if (is_always_noise(e)) {
		if (state.verbose)
			printf("[CAP] Filtered exec noise: cap=%s syscall=%s\n",
			       cap_name_safe(e->capability),
			       syscall_name_from_nr(e->syscall_nr) ?:
			       "unknown");
		return 0;
	}

	/*
	 * Filter block 2 (startup-noise gate): while recording_ready is still
	 * closed, drop runtime-linker mmap/brk/mprotect SYS_ADMIN checks.
	 * First non-noise event opens the gate for normal recording.
	 */
	if (!state.recording_ready) {
		if (is_startup_noise(e)) {
			if (state.verbose)
				printf("[CAP] Filtered startup noise: "
				       "cap=%s syscall=%s\n",
				       cap_name_safe(e->capability),
				       syscall_name_from_nr(e->syscall_nr) ?:
				       "unknown");
			return 0;
		}
		state.recording_ready = 1;
		if (state.verbose)
			printf("[CAP] Recording started\n");
	}

	/*
	 * Filter block 3 (shutdown-noise drain): after stop is decided,
	 * reapply startup-noise filtering during final drain because runtime
	 * teardown can produce the same mmap/brk SYS_ADMIN pattern.
	 */
	if (state.shutting_down && is_startup_noise(e)) {
		if (state.verbose)
			printf("[CAP] Filtered shutdown noise: "
			       "cap=%s syscall=%s\n",
				cap_name_safe(e->capability),
				syscall_name_from_nr(e->syscall_nr) ?:
				"unknown");
		return 0;
	}

	if (state.verbose) {
		printf("[CAP] pid=%d cap=%d (%s) result=%s syscall=%d (%s) "
		       "comm=%s\n",
		       e->pid, e->capability,
		       cap_name_safe(e->capability),
		       e->result ? "GRANTED" : "DENIED", e->syscall_nr,
		       syscall_name_from_nr(e->syscall_nr) ?: "unknown",
		       e->comm);
	}

	if (e->capability >= 0 && e->capability <= CAP_LAST_CAP) {
		struct cap_check *check;

		check = &state.app.checks[e->capability];
		check->capability = e->capability;
		check->count++;

		// Track kernel decision outcome for this capability.
		if (e->result > 0)
			check->granted++;
		else if (e->result == 0)
			check->denied++;

		// First access grant marks the capability as required.
		if (e->result > 0 && check->needed != 1) {
			check->needed = 1;
			update_reason(check, e->syscall_nr);
		}
	}

	return 0;
}

/*
 * analyze_capabilities - print human-readable analysis of observations.
 *
 * Walks the aggregated per-capability statistics and system context to
 * highlight required, conditional, and denied capabilities. Also emits
 * configuration snippets for common deployment targets. No return value.
 */
static void analyze_capabilities(void)
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
	printf("==============================================================="
	       "=======\n");
	printf("CAPABILITY ANALYSIS FOR: %s (PID %d)\n", state.app.exe,
	       state.app.pid);
	printf("==============================================================="
	       "=======\n\n");

	printf("SYSTEM CONTEXT:\n");
	printf("---------------------------------------------------------------"
	       "-------\n");
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
	printf("---------------------------------------------------------------"
	       "-------\n");
	for (i = 0; i <= CAP_LAST_CAP; i++) {
		struct cap_check *check;

		check = &state.app.checks[i];
		if (check->granted > 0) {
			has_required = 1;
			// Summarize how many times the kernel permitted usage.
			printf("  %s (#%d)\n", cap_name_safe(i), i);
			printf("    Checks: %lu granted, %lu denied\n",
			       check->granted, check->denied);
			if (check->reason)
				printf("    Reason: %s\n", check->reason);
			if (!include_cap_in_recommendations(i))
				printf("    Note: Internal to capability setup; "
				       "excluded from recommendations.\n");
			printf("\n");
		}
	}
	if (!has_required)
		printf("  None - Application does not require elevated "
		       "capabilities!\n\n");

	printf("CONDITIONAL CAPABILITIES:\n");
	printf("---------------------------------------------------------------"
	       "-------\n");

	if (state.app.yama_ptrace_scope > 0) {
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			if (state.app.checks[i].count > 0 &&
			    state.app.checks[i].granted == 0 &&
			    i == CAP_SYS_PTRACE) {
				has_conditional = 1;
				conditional_count++;
				printf("  CAP_SYS_PTRACE\n");
				printf("    Needed when "
				       "kernel.yama.ptrace_scope > 0\n");
				printf("    Current value: %d (capability "
				       "needed)\n",
				       state.app.yama_ptrace_scope);
				printf("\n");
			}
		}
	}

	if (state.app.perf_event_paranoid >= 2) {
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			if (state.app.checks[i].count > 0 && i == CAP_PERFMON) {
				has_conditional = 1;
				conditional_count++;
				// CAP_SYS_ADMIN may substitute on older kernels.
				printf("  CAP_PERFMON\n");
				printf("    Needed when "
				       "kernel.perf_event_paranoid >= 2\n");
				printf("    Current value: %d (capability "
				       "needed)\n",
				       state.app.perf_event_paranoid);
				printf("    Note: CAP_SYS_ADMIN can substitute "
				       "on kernels < 5.8\n");
				printf("\n");
			}
		}
	}

	if (state.app.unprivileged_bpf_disabled == 1) {
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			if (state.app.checks[i].count > 0 && i == CAP_BPF) {
				has_conditional = 1;
				conditional_count++;
				printf("  CAP_BPF\n");
				printf("    Needed when "
				       "kernel.unprivileged_bpf_disabled = 1\n");
				printf("    Current value: %d (capability "
				       "needed)\n",
				       state.app.unprivileged_bpf_disabled);
				printf("    Note: CAP_SYS_ADMIN can substitute "
				       "on kernels < 5.8\n");
				printf("\n");
			}
		}
	}

	if (state.app.kptr_restrict >= 1) {
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			if (state.app.checks[i].count > 0 &&
			    i == CAP_SYSLOG) {
				has_conditional = 1;
				conditional_count++;
				printf("  CAP_SYSLOG\n");
				printf("    Needed when "
				       "kernel.kptr_restrict >= 1\n");
				printf("    Current value: %d (capability "
				       "needed)\n",
				       state.app.kptr_restrict);
				printf("\n");
			}
		}
	}

	if (state.app.dmesg_restrict >= 1) {
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			if (state.app.checks[i].count > 0 &&
			    i == CAP_SYSLOG) {
				has_conditional = 1;
				conditional_count++;
				printf("  CAP_SYSLOG\n");
				printf("    Needed when "
				       "kernel.dmesg_restrict >= 1\n");
				printf("    Current value: %d (capability "
				       "needed)\n",
				       state.app.dmesg_restrict);
				printf("\n");
			}
		}
	}

	if (state.app.modules_disabled == 1) {
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			if (state.app.checks[i].count > 0 &&
			    i == CAP_SYS_MODULE) {
				has_conditional = 1;
				conditional_count++;
				printf("  NOTE: kernel.modules_disabled = 1\n");
				printf("    CAP_SYS_MODULE is ineffective!\n");
				printf("    Module loading is permanently "
				       "disabled.\n");
				printf("\n");
			}
		}
	}

	if (state.app.mmap_min_addr > 0) {
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			if (state.app.checks[i].count > 0 &&
			    i == CAP_SYS_RAWIO) {
				has_conditional = 1;
				conditional_count++;
				printf("  CAP_SYS_RAWIO\n");
				printf("    Needed when vm.mmap_min_addr > 0 "
				       "to map low addresses\n");
				printf("    Current value: %d (capability "
				       "needed)\n",
				       state.app.mmap_min_addr);
				printf("\n");
			}
		}
	}

	if (state.app.protected_hardlinks == 1) {
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			if (state.app.checks[i].count > 0 && i == CAP_FOWNER) {
				has_conditional = 1;
				conditional_count++;
				printf("  CAP_FOWNER\n");
				printf("    Needed when fs.protected_hardlinks "
				       "= 1 to link files not owned by the caller\n");
				printf("    Current value: %d (capability needed)\n",
				       state.app.protected_hardlinks);
				printf("\n");
			}
		}
	}

	if (state.app.protected_symlinks == 1) {
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			if (state.app.checks[i].count > 0 &&
			    i == CAP_DAC_OVERRIDE) {
				has_conditional = 1;
				conditional_count++;
				printf("  CAP_DAC_OVERRIDE\n");
				printf("    Needed when fs.protected_symlinks = 1 for "
				       "symlinks in world-writable directories\n");
				printf("    Current value: %d (capability needed)\n",
				       state.app.protected_symlinks);
				printf("\n");
			}
		}
	}

	if (state.app.suid_dumpable == 2) {
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			if (state.app.checks[i].count > 0 &&
			    i == CAP_SYS_PTRACE) {
				has_conditional = 1;
				conditional_count++;
				printf("  CAP_SYS_PTRACE\n");
				printf("    Needed when fs.suid_dumpable = 2 for core "
				       "dumps and ptrace of setuid programs\n");
				printf("    Current value: %d (capability needed)\n",
				       state.app.suid_dumpable);
				printf("\n");
			}
		}
	}

	if (!has_conditional)
		printf("  None\n\n");

	printf("ATTEMPTED BUT DENIED:\n");
	printf("---------------------------------------------------------------"
	       "-------\n");
	for (i = 0; i <= CAP_LAST_CAP; i++) {
		struct cap_check *check;

		check = &state.app.checks[i];
		if (check->denied > 0 && check->granted == 0) {
			has_denied = 1;
			printf("  %s (#%d)\n", cap_name_safe(i), i);
			printf("    Attempts: %lu (all denied)\n",
			       check->denied);
			printf("    Impact: Application may have reduced "
			       "functionality\n");
			printf("\n");
		}
	}
	if (!has_denied)
		printf("  None\n\n");

	for (i = 0; i <= CAP_LAST_CAP; i++) {
		total_checks += state.app.checks[i].count;
		if (state.app.checks[i].granted > 0)
			required_count++;
		if (state.app.checks[i].denied > 0 &&
		    state.app.checks[i].granted == 0)
			denied_count++;
	}

	printf("SUMMARY:\n");
	printf("---------------------------------------------------------------"
	       "-------\n");
	printf("  Total capability checks: %lu\n", total_checks);
	printf("  Required capabilities: %d\n", required_count);
	printf("  Conditional capabilities: %d\n", conditional_count);
	printf("  Denied operations: %d\n", denied_count);
	printf("\n");

	if (required_count > 0) {
		printf("RECOMMENDATIONS:\n");
		printf("-------------------------------------------------------"
		       "---------------\n");
		if (state.app.prog_type != UNSUPPORTED) {
			printf("  Programmatic solution (%s):\n",
			       state.app.prog_type == ELF ?
			       "C with libcap-ng" :
			       "Python with python3-libcap-ng");

			if (state.app.prog_type == ELF) {
				printf("    #include <cap-ng.h>\n");
				printf("    ...\n");
				printf("    capng_clear(CAPNG_SELECT_BOTH);\n");
				printf("    capng_updatev(CAPNG_ADD, "
				       "CAPNG_EFFECTIVE|CAPNG_PERMITTED");
				for (i = 0; i <= CAP_LAST_CAP; i++) {
					if (state.app.checks[i].granted > 0 &&
					    include_cap_in_recommendations(i)) {
						printf(", ");
						print_cap_name_upper(i);
					}
				}
				printf(", -1);\n");
				printf("    if (capng_change_id(uid, gid, "
				       "CAPNG_DROP_SUPP_GRP | "
				       "CAPNG_CLEAR_BOUNDING))\n");
				printf("\tperror(\"capng_change_id\");\n\n");
			} else if (state.app.prog_type == PYTHON) {
				printf("    import sys\n");
				printf("    import _capng as capng\n");
				printf("    ...\n");
				printf("    capng.capng_clear(capng.CAPNG_SELECT_BOTH)\n");
				printf("    capng.capng_updatev(capng.CAPNG_ADD, "
				       "capng.CAPNG_EFFECTIVE|capng.CAPNG_PERMITTED");
				for (i = 0; i <= CAP_LAST_CAP; i++) {
					if (state.app.checks[i].granted > 0 &&
					    include_cap_in_recommendations(i)) {
						printf(", capng.");
						print_cap_name_upper(i);
					}
				}
				printf(", -1)\n");
				printf("    e = capng.capng_change_id(uid, gid, "
				       "capng.CAPNG_DROP_SUPP_GRP | "
				       "capng.CAPNG_CLEAR_BOUNDING)\n");
				printf("    if e < 0:\n");
				printf("\tprint(f\"Error: {e}\")\n");
				printf("\tsys.exit(1)\n\n");
			}
		}

		printf("  For systemd service:\n");
		printf("    [Service]\n");
		printf("    User=<non-root-user>\n");
		printf("    Group=<non-root-group>\n");
		printf("    AmbientCapabilities=");
		first = 1;
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			if (state.app.checks[i].granted > 0 &&
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
			if (state.app.checks[i].granted > 0 &&
			    include_cap_in_recommendations(i)) {
				if (!first)
					printf(" ");
				printf("%s", cap_name_safe(i));
				first = 0;
			}
		}
		printf("\n\n");

		printf("  For file capabilities (via filecap):\n");
		printf("    filecap /path/to/binary");
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			if (state.app.checks[i].granted > 0 &&
			    include_cap_in_recommendations(i))
				printf(" %s", cap_name_safe(i));
		}
		printf("\n\n");

		printf("  For Docker/Podman:\n");
		printf("    docker run --user $(id -u):$(id -g) \\\n");
		printf("      --cap-drop=ALL \\\n");
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			if (state.app.checks[i].granted > 0 &&
			    include_cap_in_recommendations(i))
				printf("      --cap-add=%s \\\n",
				       cap_name_safe(i));
		}
		printf("      your-image:tag\n\n");

		printf("  For Kubernetes:\n");
		printf("    securityContext:\n");
		printf("      runAsUser: 1000\n");
		printf("      runAsGroup: 1000\n");
		printf("      capabilities:\n");
		printf("        drop:\n");
		printf("          - ALL\n");
		printf("        add:\n");
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			if (state.app.checks[i].granted > 0 &&
			    include_cap_in_recommendations(i))
				printf("          - %s\n", cap_name_safe(i));
		}
		printf("\n");
	} else {
		printf("RECOMMENDATIONS:\n");
		printf("-------------------------------------------------------"
		       "---------------\n");
		printf("  This application does not require any elevated "
		       "capabilities!\n");
		printf("  Run as an unprivileged user with no special "
		       "capabilities.\n\n");
	}
}

/*
 * output_json - emit collected results in JSON format.
 *
 * Serializes application info, system context, required capabilities, and
 * denied-only attempts to stdout. No return value.
 */
static void output_json(void)
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

	printf("  \"required_capabilities\": [\n");
	first_cap = 1;
	for (i = 0; i <= CAP_LAST_CAP; i++) {
		struct cap_check *check = &state.app.checks[i];
		char *name_json;
		char *reason_json;

		if (check->granted > 0) {
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
	}
	printf("\n  ],\n");

	printf("  \"denied_capabilities\": [\n");
	first_denied = 1;
	for (i = 0; i <= CAP_LAST_CAP; i++) {
		struct cap_check *check = &state.app.checks[i];
		char *name_json;

		if (check->denied > 0 && check->granted == 0) {
			name_json = json_escape(capng_capability_to_name(i));
			if (!first_denied)
				printf(",\n");
			printf("    {\n");
			printf("      \"number\": %d,\n", i);
			printf("      \"name\": \"%s\",\n",
			       name_json ? name_json : "");
			printf("      \"attempts\": %lu\n", check->denied);
			printf("    }");
			first_denied = 0;
			free(name_json);
		}
	}
	printf("\n  ]\n");
	printf("}\n");
}

/*
 * output_yaml - emit collected results in YAML format.
 *
 * Provides a YAML representation mirroring the JSON layout so consumers can
 * parse the auditor output more easily. No return value.
 */
static void output_yaml(void) {
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

	printf("required_capabilities:\n");
	for (i = 0; i <= CAP_LAST_CAP; i++) {
		struct cap_check *check = &state.app.checks[i];

		if (check->granted > 0) {
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
	}

	printf("denied_capabilities:\n");
	for (i = 0; i <= CAP_LAST_CAP; i++) {
		struct cap_check *check = &state.app.checks[i];

		if (check->denied > 0 && check->granted == 0) {
			printf("  - number: %d\n", i);
			printf("    name: %s\n", cap_name_safe(i));
			printf("    attempts: %lu\n", check->denied);
		}
	}
}

/*
 * classify_app - determine if the target is an ELF binary or Python script.
 * @exe: path to the executable.
 *
 * Reads the first line of the file to spot a shebang with "python" or the
 * ELF magic. Returns PYTHON, ELF, or UNSUPPORTED accordingly.
 */
static type_t classify_app(const char *exe)
{
	int fd;
	ssize_t rc;
	char buf[257];

	fd = open(exe, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Cannot open %s - %s\n", exe, strerror(errno));
		exit(1);
	}

	// classify the app
	rc = read(fd, buf, 256);
	close(fd);
	if (rc > 0) {
		// terminate buffer
		buf[rc] = 0;
		// check for shebang
		if (buf[0] == '#' && buf[1] == '!') {
			// limit search to first line
			char *ptr = strchr(buf, '\n');
			if (ptr)
				*ptr = 0;
			// see if python is anywhere on first line
			if (strstr(buf, "python"))
				return PYTHON;
			// next check if elf binary
		} else if (strncmp(buf, ELFMAGIC, 4) == 0)
			return ELF;
	}

	// If neither, then libcap-ng doesn't suport it
	return UNSUPPORTED;
}

/*
 * Parses options, validates the auditor's own capabilities, loads and
 * attaches the BPF program, forks the target, registers its PID for tracing,
 * and drives the ring buffer loop until the target exits or the user stops
 * tracing. On completion, prints the requested report format. Returns 0 on
 * success or non-zero on failure.
 */
int main(int argc, char **argv)
{
	int err;
	int arg_idx;
	pid_t child;
	pid_t ret_pid;
	int wstatus;


	if (argc < 2) {
		fprintf(stderr, "Usage: %s [options] -- command [args...]\n",
			argv[0]);
		fprintf(stderr, "Options:\n");
		fprintf(stderr, "  -v, --verbose    Verbose output\n");
		fprintf(stderr, "  -j, --json       JSON output\n");
		fprintf(stderr, "  -y, --yaml       YAML output\n");
		return 1;
	}

	arg_idx = 1;
	while (arg_idx < argc && argv[arg_idx][0] == '-') {
		if (!strcmp(argv[arg_idx], "-v") ||
		    !strcmp(argv[arg_idx], "--verbose"))
			state.verbose = 1;
		else if (!strcmp(argv[arg_idx], "-j") ||
			 !strcmp(argv[arg_idx], "--json"))
			state.json_output = 1;
		else if (!strcmp(argv[arg_idx], "-y") ||
			 !strcmp(argv[arg_idx], "--yaml"))
			state.yaml_output = 1;
		else if (!strcmp(argv[arg_idx], "--")) {
			arg_idx++;
			break;
		}
		arg_idx++;
	}

	if (arg_idx >= argc) {
		fprintf(stderr, "Error: No command specified\n");
		return 1;
	}

	state.target_argv = &argv[arg_idx];

	// Confirm the auditor itself holds the privileges needed for tracing.
	if (init_capng() != 0)
		return 1;

	if (check_auditor_caps() != 0)
		return 1;

	// Allow libbpf to pin maps by removing memlock limits early.
	if (set_memlock_rlimit() != 0)
		return 1;

	state.app.exe = strdup(state.target_argv[0]);
	state.app.prog_type = UNSUPPORTED;
	if (audit_machine < 0)
		audit_machine = audit_detect_machine();
	if (audit_machine < 0) {
		fprintf(stderr,
			"Warning: unable to determine hardware architecture for syscall lookup\n");
	}
	state.app.execve_nr = audit_name_to_syscall("execve", audit_machine);
	state.app.mmap_nr = audit_name_to_syscall("mmap", audit_machine);
	state.app.brk_nr = audit_name_to_syscall("brk", audit_machine);
	state.app.mprotect_nr = audit_name_to_syscall("mprotect",
						      audit_machine);

	// Load and attach BPF program before forking so probes are ready.
	state.skel = cap_audit_bpf__open_and_load();
	if (!state.skel) {
		fprintf(stderr, "Error: Failed to load BPF program: %s\n",
			strerror(errno));
		return 1;
	}

	err = cap_audit_bpf__attach(state.skel);
	if (err) {
		fprintf(stderr, "Error: Failed to attach BPF programs: %s\n",
			strerror(-err));
		cap_audit_bpf__destroy(state.skel);
		return 1;
	}

	state.rb = ring_buffer__new(bpf_map__fd(state.skel->maps.cap_events),
				    handle_cap_event, NULL, NULL);
	if (!state.rb) {
		fprintf(stderr, "Error: Failed to create ring buffer: %s\n",
			strerror(errno));
		cap_audit_bpf__destroy(state.skel);
		return 1;
	}

	printf("[*] Capability auditor started\n");

	if (pipe(state.sync_pipe) != 0) {
		fprintf(stderr, "Error: pipe failed: %s\n", strerror(errno));
		ring_buffer__free(state.rb);
		cap_audit_bpf__destroy(state.skel);
		return 1;
	}

	child = fork();
	if (child == 0) {
		char sync_byte;
		ssize_t bytes;

		/*
		 * Child waits until parent registers this PID in target_pids.
		 * This prevents events from an unregistered PID and avoids
		 * missing earliest post-exec events.
		 */
		close(state.sync_pipe[1]);
		bytes = read(state.sync_pipe[0], &sync_byte, 1);
		if (bytes != 1) {
			if (bytes < 0)
				perror("read");
			else
				fprintf(stderr,
					"Error: failed to sync with parent\n");
			close(state.sync_pipe[0]);
			exit(1);
		}
		close(state.sync_pipe[0]);
		execvp(state.target_argv[0], state.target_argv);
		perror("execvp");
		exit(1);
	} else if (child < 0) {
		fprintf(stderr, "Error: fork failed: %s\n", strerror(errno));
		close(state.sync_pipe[0]);
		close(state.sync_pipe[1]);
		ring_buffer__free(state.rb);
		cap_audit_bpf__destroy(state.skel);
		return 1;
	}

	state.app.pid = child;

	close(state.sync_pipe[0]);
	if (set_target_pid(child) != 0) {
		close(state.sync_pipe[1]);
		kill(child, SIGKILL);
		waitpid(child, NULL, 0);
		ring_buffer__free(state.rb);
		cap_audit_bpf__destroy(state.skel);
		free(state.app.exe);
		return 1;
	}

	if (write(state.sync_pipe[1], "1", 1) != 1) {
		fprintf(stderr, "Error: write failed: %s\n", strerror(errno));
		close(state.sync_pipe[1]);
		kill(child, SIGKILL);
		waitpid(child, NULL, 0);
		ring_buffer__free(state.rb);
		cap_audit_bpf__destroy(state.skel);
		free(state.app.exe);
		return 1;
	}
	close(state.sync_pipe[1]);

	{
		char resolved[PATH_MAX];

		if (resolve_target_exe(child, resolved, sizeof(resolved)) == 0) {
			char *resolved_dup = strdup(resolved);

			if (resolved_dup) {
				free(state.app.exe);
				state.app.exe = resolved_dup;
			}
			state.app.prog_type = classify_app(state.app.exe);
		} else {
			fprintf(stderr,
				"Warning: unable to resolve target path\n");
		}
	}

	read_system_state(&state.app);
	inspect_target_file_caps(child);
	if (state.app.prog_type == PYTHON && state.verbose)
		printf("[*] Script interpreter: %s\n", state.app.exe);
	printf("[*] Tracing application: %s (PID %d)\n", state.app.exe, child);
	printf("[*] Press Ctrl-C to stop\n\n");

	signal(SIGINT, sig_handler);
	signal(SIGTERM, sig_handler);

	while (!state.stop) {
		// Poll ring buffer to drain events;
		// timeout keeps signals timely.
		err = ring_buffer__poll(state.rb, 100);
		if (err < 0 && err != -EINTR) {
			fprintf(stderr, "Error polling ring buffer: %s\n",
				strerror(-err));
			break;
		}

		// Detect when the target process has exited.
		ret_pid = waitpid(child, &wstatus, WNOHANG);
		if (ret_pid == child) {
			if (WIFEXITED(wstatus))
				printf(
				   "\n[*] Application exited with status %d\n",
				   WEXITSTATUS(wstatus));
			else if (WIFSIGNALED(wstatus))
				printf(
				  "\n[*] Application terminated by signal %d\n",
				  WTERMSIG(wstatus));
			break;
		}
	}

	printf("[*] Analyzing results...\n");

	state.shutting_down = 1;
	usleep(100000);
	ring_buffer__poll(state.rb, 0);

	if (state.json_output)
		output_json();
	else if (state.yaml_output)
		output_yaml();
	else
		analyze_capabilities();

	ring_buffer__free(state.rb);
	cap_audit_bpf__destroy(state.skel);
	free(state.app.exe);

	for (int i = 0; i <= CAP_LAST_CAP; i++) {
		if (state.app.checks[i].reason)
			free(state.app.checks[i].reason);
	}

	return 0;
}
