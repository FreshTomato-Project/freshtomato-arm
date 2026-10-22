// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * cap-audit - Aggregate syscall outcomes for denied capability checks.
 * Copyright 2026 Red Hat Inc.
 * All Rights Reserved.
 */

#include "cap_audit.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

/* Internal restart results can be visible at the raw syscall tracepoint. */
#define KERNEL_ERESTARTSYS		512
#define KERNEL_ERESTARTNOINTR		513
#define KERNEL_ERESTARTNOHAND		514
#define KERNEL_ERESTART_RESTARTBLOCK	516

static int is_kernel_restart(int error)
{
	return error == KERNEL_ERESTARTSYS ||
	       error == KERNEL_ERESTARTNOINTR ||
	       error == KERNEL_ERESTARTNOHAND ||
	       error == KERNEL_ERESTART_RESTARTBLOCK;
}

enum syscall_outcome_class classify_syscall_outcome(__s64 result)
{
	int error;

	if (result >= 0)
		return SYSCALL_OUTCOME_SUCCESS;
	if (result < -4095)
		return SYSCALL_OUTCOME_OTHER;

	error = (int)-result;
	if (error == EPERM || error == EACCES)
		return SYSCALL_OUTCOME_PERMISSION;
	if (error == EINTR || is_kernel_restart(error))
		return SYSCALL_OUTCOME_INTERRUPTED;

	return SYSCALL_OUTCOME_OTHER;
}

const char *syscall_outcome_class_name(enum syscall_outcome_class class)
{
	switch (class) {
	case SYSCALL_OUTCOME_SUCCESS:
		return "success";
	case SYSCALL_OUTCOME_PERMISSION:
		return "permission_failure";
	case SYSCALL_OUTCOME_OTHER:
		return "other_failure";
	case SYSCALL_OUTCOME_INTERRUPTED:
		return "interrupted";
	}
	return "other_failure";
}

int add_cap_syscall_outcome(struct cap_check *check, int syscall_nr,
			    __s64 result)
{
	struct syscall_outcome *tmp;
	size_t new_capacity;
	size_t i;

	/* Successful return values are equivalent for capability analysis. */
	if (result >= 0)
		result = 0;

	for (i = 0; i < check->outcome_count; i++) {
		struct syscall_outcome *outcome = &check->outcomes[i];

		if (outcome->syscall_nr == syscall_nr &&
		    outcome->result == result) {
			outcome->count++;
			return 0;
		}
	}

	if (check->outcome_count == check->outcome_capacity) {
		new_capacity = check->outcome_capacity ?
			       check->outcome_capacity * 2 : 4;
		tmp = realloc(check->outcomes,
		      new_capacity * sizeof(*check->outcomes));
		if (!tmp)
			return -1;
		check->outcomes = tmp;
		check->outcome_capacity = new_capacity;
	}

	check->outcomes[check->outcome_count].syscall_nr = syscall_nr;
	check->outcomes[check->outcome_count].result = result;
	check->outcomes[check->outcome_count].count = 1;
	check->outcome_count++;
	return 0;
}

void summarize_cap_outcomes(const struct cap_check *check,
			    struct cap_outcome_summary *summary)
{
	size_t i;

	memset(summary, 0, sizeof(*summary));
	for (i = 0; i < check->outcome_count; i++) {
		const struct syscall_outcome *outcome = &check->outcomes[i];

		switch (classify_syscall_outcome(outcome->result)) {
		case SYSCALL_OUTCOME_SUCCESS:
			summary->succeeded += outcome->count;
			break;
		case SYSCALL_OUTCOME_PERMISSION:
			summary->permission_failed += outcome->count;
			break;
		case SYSCALL_OUTCOME_OTHER:
			summary->other_failed += outcome->count;
			break;
		case SYSCALL_OUTCOME_INTERRUPTED:
			summary->interrupted += outcome->count;
			break;
		}
	}
}

static int has_unmatched_denied_syscall(const struct cap_check *check)
{
	size_t i;
	size_t j;

	for (i = 0; i < check->denied_syscall_count; i++) {
		int found = 0;

		for (j = 0; j < check->outcome_count; j++) {
			if (check->denied_syscalls[i] ==
			    check->outcomes[j].syscall_nr) {
				found = 1;
				break;
			}
		}
		if (!found)
			return 1;
	}
	return 0;
}

enum denial_assessment assess_cap_denials(const struct cap_check *check)
{
	struct cap_outcome_summary summary;

	if (cap_total_denied(check) == 0)
		return DENIAL_ASSESSMENT_NONE;

	summarize_cap_outcomes(check, &summary);
	if (summary.permission_failed > 0)
		return DENIAL_ASSESSMENT_PERMISSION;
	if (summary.succeeded > 0 && summary.interrupted > 0)
		return DENIAL_ASSESSMENT_MIXED_INTERRUPTED;
	if (has_unmatched_denied_syscall(check))
		return DENIAL_ASSESSMENT_INCONCLUSIVE;
	/* A success must not hide correlated failures for other reasons. */
	if (summary.succeeded > 0 && summary.other_failed > 0)
		return DENIAL_ASSESSMENT_MIXED_OTHER;
	if (summary.succeeded > 0)
		return DENIAL_ASSESSMENT_SUCCEEDED;
	if (summary.interrupted > 0 || check->outcome_count == 0)
		return DENIAL_ASSESSMENT_INCONCLUSIVE;
	return DENIAL_ASSESSMENT_OTHER;
}

const char *denial_assessment_name(enum denial_assessment assessment)
{
	switch (assessment) {
	case DENIAL_ASSESSMENT_PERMISSION:
		return "permission_failure";
	case DENIAL_ASSESSMENT_MIXED_INTERRUPTED:
		return "mixed_success_interruption";
	case DENIAL_ASSESSMENT_MIXED_OTHER:
		return "mixed_success_other_failure";
	case DENIAL_ASSESSMENT_INCONCLUSIVE:
		return "inconclusive";
	case DENIAL_ASSESSMENT_SUCCEEDED:
		return "succeeded_despite_denial";
	case DENIAL_ASSESSMENT_OTHER:
		return "denial_not_established";
	case DENIAL_ASSESSMENT_NONE:
		return "none";
	}
	return "none";
}

int syscall_result_errno(__s64 result)
{
	int error;

	if (result >= 0 || result < -4095)
		return 0;
	error = (int)-result;
	if (is_kernel_restart(error))
		return 0;
	return error;
}

const char *syscall_result_name(__s64 result)
{
	int error;

	if (result >= 0 || result < -4095)
		return NULL;
	error = (int)-result;

	switch (error) {
	case EPERM:
		return "EPERM";
	case EACCES:
		return "EACCES";
	case ENOENT:
		return "ENOENT";
	case EINTR:
		return "EINTR";
	case EBADF:
		return "EBADF";
#ifdef EBADFD
	case EBADFD:
		return "EBADFD";
#endif
	case KERNEL_ERESTARTSYS:
		return "ERESTARTSYS";
	case KERNEL_ERESTARTNOINTR:
		return "ERESTARTNOINTR";
	case KERNEL_ERESTARTNOHAND:
		return "ERESTARTNOHAND";
	case KERNEL_ERESTART_RESTARTBLOCK:
		return "ERESTART_RESTARTBLOCK";
	default:
		return NULL;
	}
}
