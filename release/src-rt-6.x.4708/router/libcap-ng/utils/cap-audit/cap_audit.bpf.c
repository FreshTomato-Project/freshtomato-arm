// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * cap_audit.bpf.c - Capture capability checks for a target application
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

#include "vmlinux.h"
#include <bpf/bpf_core_read.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>

/*
 * BPF overview:
 * The BPF side attaches to capability helpers (cap_capable, ns_capable,
 * capable) and syscall tracepoints to capture capability checks only for a
 * target process tree. A PID hash map gates all work; if the PID is not in
 * target_pids the probes exit immediately. For traced tasks the program builds
 * cap_event records with task identity, syscall context, namespace inode, and
 * stack id, tracks per-capability statistics, and streams finalized events to
 * userspace through a ring buffer. Fork/exit tracepoints keep the PID filter
 * in sync so children are traced and exits are pruned.
 */

#ifndef PERF_MAX_STACK_DEPTH
#define PERF_MAX_STACK_DEPTH 127
#endif

#if !defined(__TARGET_ARCH_x86) && !defined(__TARGET_ARCH_arm64) && \
	!defined(__TARGET_ARCH_arm) && !defined(__TARGET_ARCH_powerpc) && \
	!defined(__TARGET_ARCH_s390) && !defined(__TARGET_ARCH_riscv) && \
	!defined(__TARGET_ARCH_mips) && !defined(__TARGET_ARCH_loongarch)
#if defined(__x86_64__) || defined(__i386__)
#define __TARGET_ARCH_x86
#elif defined(__aarch64__)
#define __TARGET_ARCH_arm64
#elif defined(__arm__)
#define __TARGET_ARCH_arm
#elif defined(__powerpc__)
#define __TARGET_ARCH_powerpc
#elif defined(__s390x__) || defined(__s390__)
#define __TARGET_ARCH_s390
#elif defined(__riscv)
#define __TARGET_ARCH_riscv
#elif defined(__mips__)
#define __TARGET_ARCH_mips
#elif defined(__loongarch64)
#define __TARGET_ARCH_loongarch
#else
#define __TARGET_ARCH_x86
#endif
#endif

char LICENSE[] SEC("license") = "GPL";

struct cap_event {
	__u64 timestamp_ns;
	__u32 pid;
	__u32 tid;
	__u32 uid;
	__u32 gid;
	int capability;
	int result;
	int syscall_nr;
	char comm[TASK_COMM_LEN];
	__u64 stack_id;
	__u32 targ_ns_inum;
};

struct cap_stats {
	__u64 checks;
	__u64 granted;
	__u64 denied;
};

// This sets the limit for how many child processes can be traced.
// Because of this limit, the tracer may not be suitable for shell
// scripts or long running process that fork child handlers that
// terminate soon after launching. When this fills up, no more
// children will be traced. This is the breaking point for long
// running apps. The other limits aren't as likely to be broken.
// This is approx 16 bytes per entry. (Default uses 128K of memory)
struct {
	__uint(type, BPF_MAP_TYPE_HASH);
	__type(key, __u32);
	__type(value, __u8);
	__uint(max_entries, 8192);
} target_pids SEC(".maps");

// This declares the size of the ring buf that holds events for
// userspace to access.
struct {
	__uint(type, BPF_MAP_TYPE_RINGBUF);
	__uint(max_entries, 256 * 1024);
} cap_events SEC(".maps");

// This declares how many unique stack traces to hold. This is used
// to determine which syscall a capability was requested from. If this
// fills up, no more stack traces will be collected. This is about
// 1K per entry. (Default uses 20 MB of memory)
struct {
	__uint(type, BPF_MAP_TYPE_STACK_TRACE);
	__uint(key_size, sizeof(__u32));
	__uint(value_size, PERF_MAX_STACK_DEPTH * sizeof(__u64));
	__uint(max_entries, 20000);
} stack_traces SEC(".maps");

// This declares how many capabilities can be watched. As of the 6.18
// kernel, it only uses 40. So, 64 is future proof as none have been added
// in a while.
struct {
	__uint(type, BPF_MAP_TYPE_ARRAY);
	__type(key, __u32);
	__type(value, struct cap_stats);
	__uint(max_entries, 64);
} capability_stats SEC(".maps");

// The cap_events_inflight map uses pid_tgid as the key. There is a race
// scenario when deep syscall chains that check multiple capabilities
// or nested function calls where each checks a capability. In these cases
// because it uses the same pid_tgid, it can overwrite a previous event.
// example:
//
// In kernel, during mount():
// sys_mount() {
//  First check
//  if (!capable(CAP_SYS_ADMIN))  // ← kprobe #1 fires
//      return -EPERM;
//
//  Path resolution might trigger
//  if (!capable(CAP_DAC_OVERRIDE))  // ← kprobe #2 fires BEFORE kretprobe #1!
//      return -EACCES;
//  ... more work ...
//
//    return 0;  // ← kretprobe #1 and #2 fire
//}
//
// Statistics are SAFE: The capability_stats map is updated immediately in
// the kprobe using atomic operations, so counts are always accurate. The
// probability is higher for complex syscalls like mount, setuid, or network
// operations.
//
// Possible solutions
// Option 1: Per-CPU Map - change to BPF_MAP_TYPE_PERCPU_HASH. Drawback is
// it uses a map for each CPU so if 64 cores, map is 64KB of memory.
//
// Option 2: Include Stack Pointer in Key -
// key[0] = bpf_get_current_pid_tgid();
// key[1] = PT_REGS_SP(ctx);  // Stack pointer makes it unique
// bpf_map_update_elem(&cap_events_inflight, &key, ev, BPF_ANY);
//
// Option 3: Accept the Race (Current Approach)
// Rationale:
// * The capability_stats map is always correct (atomic updates)
// * Individual event details might be wrong, but aggregate data is right
// * For the tool's purpose (determining required capabilities), statistics
//   are what matter
// * Individual events are mainly for debugging/verbose mode

struct {
	__uint(type, BPF_MAP_TYPE_HASH);
	__type(key, __u64);
	__type(value, struct cap_event);
	__uint(max_entries, 1024);
} cap_events_inflight SEC(".maps");

// In theory, if sys_exit is not called, a syscall can leak. This can
// happen due to SIGKILL or a core dump. This might matter if this is
// tracing a long running with many threads some of which get SIGKILL.
// Might be more likely if its a child process rather than a thread.
// Because each run of the tracer is a new instance, the only concern
// is long tracing sessions. If this really was a concern, we could
// change to BPF_MAP_TYPE_PERCPU_HASH so that a leak on CPU0 doesn't
// affect tracing on CPU1. This is just mentioned here because it is
// an esoteric problem and not likely to show up. But this documents
// it and a possible solution. The drawback is that it uses more memory.
struct {
	__uint(type, BPF_MAP_TYPE_HASH);
	__type(key, __u64);
	__type(value, int);
	__uint(max_entries, 4096);
} current_syscalls SEC(".maps");

/*
 * should_trace_pid - check if the current PID is in the target set.
 * @pid: process ID of the current task.
 *
 * Looks up the PID in target_pids and returns 1 when tracing is enabled for
 * it, otherwise 0.
 */
static __always_inline int should_trace_pid(__u32 pid)
{
	__u8 *trace;

	trace = bpf_map_lookup_elem(&target_pids, &pid);
	return trace ? 1 : 0;
}

/*
 * record_stats - increment capability check count.
 * @cap: capability number.
 *
 * Increments the "checks" counter for the capability in capability_stats.
 * Out-of-range capability numbers are ignored. Returns nothing.
 */
static __always_inline void record_stats(int cap)
{
	__u32 key;
	struct cap_stats *stats;

	if (cap < 0 || cap >= 64)
		return;

	key = (__u32)cap;
	stats = bpf_map_lookup_elem(&capability_stats, &key);
	if (stats)
		__sync_fetch_and_add(&stats->checks, 1);
	else {
		struct cap_stats new_stats = { 0 };

		new_stats.checks = 1;
		bpf_map_update_elem(&capability_stats, &key, &new_stats,
				    BPF_ANY);
	}
}

/*
 * update_result_stats - record whether a capability check succeeded.
 * @cap: capability number from the in-flight event.
 * @ret: return value from the capability helper (0 = granted).
 *
 * Updates the granted/denied counters for the capability when a matching
 * entry already exists. Out-of-range capability numbers are ignored.
 */
static __always_inline void update_result_stats(int cap, int ret)
{
	__u32 key;
	struct cap_stats *stats;

	if (cap < 0 || cap >= 64)
		return;

	key = (__u32)cap;
	stats = bpf_map_lookup_elem(&capability_stats, &key);
	if (!stats)
		return;

	if (!ret)
		__sync_fetch_and_add(&stats->granted, 1);
	else
		__sync_fetch_and_add(&stats->denied, 1);
}

/*
 * read_syscall - fetch the syscall number for the current task.
 * @ctx: pt_regs provided by the kprobe.
 *
 * Uses a per-thread map populated by sys_enter tracepoints when available,
 * and falls back to architecture-specific pt_regs fields. Returns the syscall
 * number or -1 when it cannot be determined.
 */
static __always_inline int read_syscall(struct pt_regs *ctx)
{
	__u64 pid_tgid;
	int *nr;

	pid_tgid = bpf_get_current_pid_tgid();
	nr = bpf_map_lookup_elem(&current_syscalls, &pid_tgid);
	if (nr)
		return *nr;

#ifdef __TARGET_ARCH_x86
	return BPF_CORE_READ(ctx, orig_ax);
#elif defined(__TARGET_ARCH_arm64)
	return BPF_CORE_READ(ctx, syscallno);
#elif defined(__TARGET_ARCH_powerpc)
	return BPF_CORE_READ(ctx, gpr[0]);
#elif defined(__TARGET_ARCH_s390)
	return BPF_CORE_READ(ctx, gprs[2]);
#else
	return -1;
#endif
}

/*
 * fill_event_common - populate the static fields of a cap_event.
 * @ev: event structure to fill.
 * @ctx: pt_regs from the capability hook.
 * @cap: capability number being checked.
 *
 * Captures PID/TID, timestamp, UID/GID, command name, syscall number, and
 * user stack id for the current task.
 */
static __always_inline void fill_event_common(struct cap_event *ev,
					      struct pt_regs *ctx, int cap)
{
	__u64 pid_tgid;
	__u64 uid_gid;

	pid_tgid = bpf_get_current_pid_tgid();
	ev->pid = pid_tgid >> 32;
	ev->tid = (__u32)pid_tgid;
	ev->capability = cap;
	ev->timestamp_ns = bpf_ktime_get_ns();

	uid_gid = bpf_get_current_uid_gid();
	ev->uid = uid_gid >> 32;
	ev->gid = (__u32)uid_gid;
	bpf_get_current_comm(&ev->comm, sizeof(ev->comm));

	ev->syscall_nr = read_syscall(ctx);
	ev->stack_id = bpf_get_stackid(ctx, &stack_traces,
				       BPF_F_USER_STACK);
}

/*
 * stash_event - store a partially filled event until the kretprobe fires.
 * @ev: event to stash.
 *
 * Keeps the event keyed by pid_tgid in cap_events_inflight so the return
 * probe can finalize result status before emitting to userspace.
 */
static __always_inline void stash_event(struct cap_event *ev)
{
	__u64 pid_tgid;

	pid_tgid = bpf_get_current_pid_tgid();
	bpf_map_update_elem(&cap_events_inflight, &pid_tgid, ev,
			    BPF_ANY);
}

/*
 * submit_event - finalize and emit a stashed event to the ring buffer.
 * @ret: return code from the capability helper (0 = granted).
 *
 * Looks up the in-flight event, copies it to the ring buffer, sets the result
 * flag (1 = granted, 0 = denied), and removes the temporary entry. Returns 0
 * whether or not an event was emitted.
 */
static __always_inline int submit_event(int ret)
{
	__u64 pid_tgid;
	struct cap_event *stored;
	struct cap_event *out;

	pid_tgid = bpf_get_current_pid_tgid();
	stored = bpf_map_lookup_elem(&cap_events_inflight, &pid_tgid);
	if (!stored)
		return 0;

	out = bpf_ringbuf_reserve(&cap_events, sizeof(*out), 0);
	if (!out)
		goto cleanup;

	__builtin_memcpy(out, stored, sizeof(*out));
	out->result = ret ? 0 : 1;

	bpf_ringbuf_submit(out, 0);

cleanup:
	bpf_map_delete_elem(&cap_events_inflight, &pid_tgid);
	return 0;
}

/*
 * handle_capable - common logic for capability helper entry probes.
 * @ctx: pt_regs for the probed function.
 * @cap: capability number under evaluation.
 * @targ_ns: optional target namespace pointer (may be NULL).
 *
 * Filters by PID first; for traced tasks it records a stats increment,
 * populates a cap_event with contextual information, and stashes it so the
 * return probe can attach the result. Returns 0 to indicate the kprobe should
 * allow normal execution to continue.
 */
static __always_inline int handle_capable(struct pt_regs *ctx, int cap,
					  struct user_namespace *targ_ns)
{
	struct cap_event ev = { 0 };
	__u32 pid;

	pid = bpf_get_current_pid_tgid() >> 32;
	if (!should_trace_pid(pid))
		return 0;

	/* Track how many times this capability was inspected. */
	record_stats(cap);
	/* Collect task identity, syscall, and stack trace information. */
	fill_event_common(&ev, ctx, cap);

	if (targ_ns) {
		struct ns_common *ns;

		ns = (struct ns_common *)targ_ns;
		ev.targ_ns_inum = BPF_CORE_READ(ns, inum);
	}

	/* Save event so the kretprobe can add the success/failure result. */
	stash_event(&ev);
	return 0;
}

/*
 * trace_cap_capable - entry probe for cap_capable().
 *
 * Delegates to handle_capable(). Arguments mirror the kernel helper but are
 * unused here beyond the capability number and namespace pointer. Returns 0.
 */
SEC("kprobe/cap_capable")
int BPF_KPROBE(trace_cap_capable, const struct cred *cred,
	       struct user_namespace *targ_ns, int cap, unsigned int opts)
{
	return handle_capable(ctx, cap, targ_ns);
}

/*
 * trace_cap_capable_ret - return probe for cap_capable().
 * @ret: kernel return value (0 = granted).
 *
 * Updates result statistics for the capability tied to this pid_tgid and
 * emits the finalized event to userspace. Always returns 0.
 */
SEC("kretprobe/cap_capable")
int BPF_KRETPROBE(trace_cap_capable_ret, int ret)
{
	__u64 pid_tgid;
	struct cap_event *stored;

	pid_tgid = bpf_get_current_pid_tgid();
	stored = bpf_map_lookup_elem(&cap_events_inflight, &pid_tgid);
	if (stored)
		update_result_stats(stored->capability, ret);

	return submit_event(ret);
}

/*
 * trace_ns_capable - entry probe for ns_capable().
 *
 * Uses handle_capable() to capture namespace-aware capability checks. Returns
 * 0.
 */
SEC("kprobe/ns_capable")
int BPF_KPROBE(trace_ns_capable, struct user_namespace *ns, int cap)
{
	return handle_capable(ctx, cap, ns);
}

/*
 * trace_ns_capable_ret - return probe for ns_capable().
 * @ret: kernel return value.
 *
 * Emits the stored event with the grant/deny result. Returns 0.
 */
SEC("kretprobe/ns_capable")
int BPF_KRETPROBE(trace_ns_capable_ret, int ret)
{
	return submit_event(ret);
}

/*
 * trace_ns_capable_noaudit - entry probe for ns_capable_noaudit().
 *
 * Captures capability checks that bypass kernel audit logging but should be
 * observed by the auditor. Returns 0.
 */
SEC("kprobe/ns_capable_noaudit")
int BPF_KPROBE(trace_ns_capable_noaudit, struct user_namespace *ns, int cap)
{
	return handle_capable(ctx, cap, ns);
}

/*
 * trace_ns_capable_noaudit_ret - return probe for ns_capable_noaudit().
 * @ret: kernel return value.
 *
 * Finalizes and emits the stored event. Returns 0.
 */
SEC("kretprobe/ns_capable_noaudit")
int BPF_KRETPROBE(trace_ns_capable_noaudit_ret, int ret)
{
	return submit_event(ret);
}

/*
 * trace_capable - entry probe for capable().
 *
 * Handles capability checks that do not involve namespaces. Returns 0.
 */
SEC("kprobe/capable")
int BPF_KPROBE(trace_capable, int cap)
{
	return handle_capable(ctx, cap, NULL);
}

/*
 * trace_capable_ret - return probe for capable().
 * @ret: kernel return value.
 *
 * Emits the stored capability event. Returns 0.
 */
SEC("kretprobe/capable")
int BPF_KRETPROBE(trace_capable_ret, int ret)
{
	return submit_event(ret);
}

/*
 * trace_sys_enter - remember syscall numbers on entry.
 * @ctx: raw_syscalls/sys_enter tracepoint context.
 *
 * Stores the syscall number in a per-thread map for later lookup by the
 * capability probes. No-op for non-traced PIDs. Returns 0.
 */
SEC("tracepoint/raw_syscalls/sys_enter")
int trace_sys_enter(struct trace_event_raw_sys_enter *ctx)
{
	__u64 pid_tgid;
	__u32 pid;
	__u32 id;

	pid_tgid = bpf_get_current_pid_tgid();
	pid = pid_tgid >> 32;
	if (!should_trace_pid(pid))
		return 0;

	id = ctx->id;
	bpf_map_update_elem(&current_syscalls, &pid_tgid, &id, BPF_ANY);
	return 0;
}

/*
 * trace_sys_exit - clear syscall tracking on exit.
 * @ctx: raw_syscalls/sys_exit tracepoint context.
 *
 * Removes the stored syscall number for the thread when tracing. Returns 0.
 */
SEC("tracepoint/raw_syscalls/sys_exit")
int trace_sys_exit(struct trace_event_raw_sys_exit *ctx)
{
	__u64 pid_tgid;
	__u32 pid;

	pid_tgid = bpf_get_current_pid_tgid();
	pid = pid_tgid >> 32;
	if (!should_trace_pid(pid))
		return 0;

	bpf_map_delete_elem(&current_syscalls, &pid_tgid);
	return 0;
}

/*
 * trace_sched_process_fork - follow new child processes.
 * @ctx: sched_process_fork tracepoint data.
 *
 * When a traced parent forks, automatically add the child PID to the filter
 * map so subsequent capability checks are captured. Returns 0.
 */
SEC("tracepoint/sched/sched_process_fork")
int trace_sched_process_fork(struct trace_event_raw_sched_process_fork *ctx)
{
	__u8 val = 1;
	__u32 parent_pid;
	__u32 child_pid;

	parent_pid = ctx->parent_pid;
	child_pid = ctx->child_pid;

	if (should_trace_pid(parent_pid))
		bpf_map_update_elem(&target_pids, &child_pid, &val, BPF_ANY);

	return 0;
}

/*
 * trace_sched_process_exit - prune exited processes from the target set.
 * @ctx: sched_process_exit tracepoint data.
 *
 * Removes the exiting PID from the target_pids map to prevent stale entries.
 * Returns 0.
 */
SEC("tracepoint/sched/sched_process_exit")
int trace_sched_process_exit(struct trace_event_raw_sched_process_template *ctx)
{
	__u32 pid;

	pid = ctx->pid;
	bpf_map_delete_elem(&target_pids, &pid);

	return 0;
}
