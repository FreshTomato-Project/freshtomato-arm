// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * cap-audit - Track capabilities requested by successful capset calls.
 * Copyright 2026 Red Hat Inc.
 * All Rights Reserved.
 */

#include "cap_audit.h"

/* Merge one successful capset payload into the application-wide union. */
void record_successful_capset(const struct cap_event *event)
{
	if (event->syscall_ret != 0)
		return;

	state.app.capset.effective |= event->capset_effective;
	state.app.capset.permitted |= event->capset_permitted;
	state.app.capset.inheritable |= event->capset_inheritable;
	state.app.capset.successful_calls++;
}

/* Return whether any successful capset requested the capability. */
int cap_requested_by_capset(int cap)
{
	__u64 requested;

	if (cap < 0 || cap >= 64)
		return 0;

	requested = state.app.capset.effective |
		    state.app.capset.permitted |
		    state.app.capset.inheritable;
	return (requested & (1ULL << cap)) != 0;
}

/* Identify capset requests that have no confirmed granted use. */
int cap_is_capset_only(int cap)
{
	if (cap < 0 || cap > CAP_LAST_CAP)
		return 0;

	return cap_requested_by_capset(cap) &&
	       !cap_required_union(&state.app.checks[cap]);
}

/* Build the safe current-binary union used by deployment recommendations. */
int cap_is_compat_requirement(int cap)
{
	if (cap < 0 || cap > CAP_LAST_CAP)
		return 0;

	return cap_required_union(&state.app.checks[cap]) ||
	       cap_requested_by_capset(cap);
}
