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

#include "cap_audit.h"

#include <bpf/bpf.h>
#include <errno.h>
#include <libaudit.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

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
 * When the tool observes a capset syscall from the initial PID, it splits
 * capability accounting into initialization and operational phases. The
 * initialization phase covers all capability checks from process start to
 * the first capset. The operational phase covers everything after. This
 * separation allows the tool to distinguish capabilities needed for one-time
 * setup (binding privileged ports, chroot, loading restricted configuration)
 * from capabilities needed for ongoing operation. Recommendations for
 * programmatic capability dropping use only the operational set, while
 * deployment recommendations (file capabilities, systemd, containers) use
 * the union since the process must start with sufficient capabilities for
 * initialization. Programs that never call capset produce an undifferentiated
 * report identical to previous versions.
 *
 * Core design problem:
 * when tracing as root, many capability checks come from kernel-internal
 * work under the same PID rather than from app logic. The tool uses a
 * two-layer noise filter pipeline, split between BPF and userspace, to
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
 * Layer 2 - userspace always-noise filter:
 * handle_cap_event() unconditionally drops two classes of capability
 * checks that never represent real application requirements. First,
 * execve-triggered SYS_ADMIN and SETPCAP checks are kernel-internal
 * credential-transition noise. Second, cap-audit identifies the known
 * advisory memory overcommit probe by combining syscall, capability, and
 * CAP_OPT_NOAUDIT: cap_vm_enough_memory checks CAP_SYS_ADMIN on
 * brk/mmap/mprotect/mremap and the operation succeeds regardless of the
 * result. CAP_OPT_NOAUDIT means "do not audit", not "advisory", so it
 * is only a confirming signal here. Other enforcement checks also use
 * CAP_OPT_NOAUDIT to avoid audit spam, and must still be reported. The
 * syscall + capability match identifies the specific advisory call site,
 * while CAP_OPT_NOAUDIT confirms that the event followed the advisory path
 * rather than a security-gating path on the same syscall.
 *
 * Layer 3 - final drain shutdown backstop:
 * after waitpid() reports that the initial PID has exited, cap-audit does a
 * short final ring-buffer drain before analyzing results. That drain should
 * stay conservative because queued late SYS_ADMIN/SETPCAP checks from the
 * exiting interpreter/runtime are more likely to be teardown chatter than a
 * meaningful application requirement. The shutdown backstop is intentionally
 * limited to the initial PID and only applies during that final drain.
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

struct audit_state state;
int audit_machine = -1;

static void sig_handler(int sig __attribute__((unused)))
{
	state.stop = 1;
}

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

static int init_capng(void)
{
	capng_clear(CAPNG_SELECT_BOTH);

	if (capng_get_caps_process() != 0) {
		fprintf(stderr, "Error: Failed to get process capabilities\n");
		return -1;
	}

	return 0;
}

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

static void usage(FILE *out, const char *prog)
{
	fprintf(out, "Usage: %s [options] -- command [args...]\n", prog);
	fprintf(out, "Options:\n");
	fprintf(out, "  -h, --help       Show this help message\n");
	fprintf(out, "  -v, --verbose    Verbose output\n");
	fprintf(out, "  -j, --json       JSON output\n");
	fprintf(out, "  -y, --yaml       YAML output\n");
}

int main(int argc, char **argv)
{
	int err;
	int arg_idx;
	pid_t child;
	pid_t ret_pid;
	int wstatus;

	if (argc < 2) {
		usage(stderr, argv[0]);
		return 1;
	}

	arg_idx = 1;
	while (arg_idx < argc && argv[arg_idx][0] == '-') {
		if (!strcmp(argv[arg_idx], "-h") ||
		    !strcmp(argv[arg_idx], "--help")) {
			usage(stdout, argv[0]);
			return 0;
		} else if (!strcmp(argv[arg_idx], "-v") ||
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
		} else {
			fprintf(stderr, "Error: Unknown option '%s'\n",
				argv[arg_idx]);
			usage(stderr, argv[0]);
			return 1;
		}
		arg_idx++;
	}

	if (arg_idx >= argc) {
		fprintf(stderr, "Error: No command specified\n");
		usage(stderr, argv[0]);
		return 1;
	}

	state.target_argv = &argv[arg_idx];

	if (init_capng() != 0)
		return 1;

	if (check_auditor_caps() != 0)
		return 1;

	if (set_memlock_rlimit() != 0)
		return 1;

	state.app.exe = strdup(state.target_argv[0]);
	state.app.prog_type = UNSUPPORTED;
	if (audit_machine < 0)
		audit_machine = audit_detect_machine();
	if (audit_machine < 0) {
		fprintf(stderr,
			"Error: unable to determine hardware architecture for syscall lookup. Exiting.\n");
		return 1;
	}
	state.app.execve_nr = audit_name_to_syscall("execve", audit_machine);
	state.app.mmap_nr = audit_name_to_syscall("mmap", audit_machine);
	state.app.brk_nr = audit_name_to_syscall("brk", audit_machine);
	state.app.mprotect_nr = audit_name_to_syscall("mprotect",
						      audit_machine);
	state.app.mremap_nr = audit_name_to_syscall("mremap", audit_machine);
	state.app.capset_nr = audit_name_to_syscall("capset", audit_machine);

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
		err = ring_buffer__poll(state.rb, 100);
		if (err < 0 && err != -EINTR) {
			fprintf(stderr, "Error polling ring buffer: %s\n",
				strerror(-err));
			break;
		}

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
		if (state.app.checks[i].op_reason)
			free(state.app.checks[i].op_reason);
		if (state.app.checks[i].denied_syscalls)
			free(state.app.checks[i].denied_syscalls);
	}

	return 0;
}
