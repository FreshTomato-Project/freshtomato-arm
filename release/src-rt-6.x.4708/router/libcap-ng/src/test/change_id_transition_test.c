/* SPDX-License-Identifier: LGPL-2.1-or-later */
/* change_id_transition_test.c -- unprivileged credential transition tests
 * Copyright 2026 Red Hat Inc.
 * All Rights Reserved.
 */

#include "config.h"
#include "../cap-ng.h"
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>

static struct __user_cap_data_struct current_caps[2];
static unsigned int group_calls;
static unsigned int uid_calls;
static int keepcaps;
static gid_t expected_gid = 7;
static struct {
	unsigned int calls;
	unsigned int retries;
	size_t previous_size;
	int error;
	int missing;
} lookup;

/*
 * check - require a transition invariant.
 * @condition: nonzero when the invariant holds.
 * @message: diagnostic on failure.
 *
 * Returns no value; terminates the test on failure.
 */
static void check(int condition, const char *message)
{
	if (!condition) {
		fprintf(stderr, "%s\n", message);
		exit(EXIT_FAILURE);
	}
}

/*
 * capget - provide a fixed capability ABI and simulated process state.
 * @header: requested ABI, updated during version discovery.
 * @data: destination capability words, or NULL for discovery.
 *
 * Returns 0 on a read, -1 with EINVAL for version discovery.
 */
int capget(cap_user_header_t header, const cap_user_data_t data)
{
	header->version = _LINUX_CAPABILITY_VERSION_3;
	if (!data) {
		errno = EINVAL;
		return -1;
	}
	memcpy(data, current_caps, sizeof(current_caps));
	return 0;
}

/*
 * capset - enforce the capability subset rules relevant to this test.
 * @header: capability ABI selected by the library.
 * @data: requested process capabilities.
 *
 * Returns 0 on success, -1 if effective exceeds permitted or permitted grows.
 * No kernel credentials are changed by this test executable.
 */
int capset(cap_user_header_t header, cap_user_data_t data)
{
	size_t i;

	check(header->version == _LINUX_CAPABILITY_VERSION_3,
	      "Unexpected capability ABI");
	for (i = 0; i < 2; i++) {
		if ((data[i].effective & ~data[i].permitted) ||
		    (data[i].permitted & ~current_caps[i].permitted)) {
			errno = EPERM;
			return -1;
		}
	}
	memcpy(current_caps, data, sizeof(current_caps));
	return 0;
}

/*
 * prctl - allow keepcaps without touching the running test's credentials.
 * @option: requested operation; optional arguments are unused.
 *
 * Returns 0 for keepcaps, -1 with EINVAL for unrelated feature probes.
 */
int prctl(int option, ...)
{
	if (option == PR_SET_KEEPCAPS) {
		va_list ap;

		va_start(ap, option);
		keepcaps = va_arg(ap, int);
		va_end(ap);
		return 0;
	}
	errno = EINVAL;
	return -1;
}

/*
 * setgroups - check transition-time SETGID without changing real groups.
 * @count: number of requested groups.
 * @groups: requested group IDs.
 *
 * Returns 0 with effective SETGID, -1 with EPERM otherwise.
 */
int setgroups(size_t count, const gid_t *groups)
{
	check(count > 0 && groups != NULL, "Missing supplementary groups");
	group_calls++;
	if (current_caps[0].effective & (1U << CAP_SETGID))
		return 0;
	errno = EPERM;
	return -1;
}

/*
 * getpwuid - poison shared passwd storage to reject non-reentrant lookups.
 * @uid: target user ID.
 *
 * Returns another account's fields, as if an intervening lookup overwrote
 * the shared result. Only caller-owned getpwuid_r storage is stable.
 */
struct passwd *getpwuid(uid_t uid)
{
	static struct passwd account = {
		.pw_name = "other-account", .pw_uid = 42, .pw_gid = 0,
	};

	check(uid == account.pw_uid, "Unexpected user lookup");
	return &account;
}

/*
 * getpwuid_r - resolve a synthetic account into caller-owned storage.
 * @uid: target user ID.
 * @pw: destination account record.
 * @buf: destination for account strings.
 * @size: buffer capacity.
 * @result: resolved record, or NULL when absent.
 *
 * Returns a configured error or account, exercising buffer growth and
 * missing users without relying on local NSS data. The five parameters
 * are required by the libc interface being interposed.
 */
int getpwuid_r(uid_t uid, struct passwd *pw, char *buf, size_t size,
	      struct passwd **result)
{
	check(uid == 42, "Unexpected reentrant user lookup");
	check(size > lookup.previous_size, "Lookup buffer did not grow");
	lookup.previous_size = size;
	lookup.calls++;
	*result = NULL;
	if (lookup.calls <= lookup.retries)
		return ERANGE;
	if (lookup.error || lookup.missing)
		return lookup.error;
	check(size >= sizeof("transition-test"), "Lookup buffer too small");
	memset(pw, 0, sizeof(*pw));
	strcpy(buf, "transition-test");
	pw->pw_name = buf;
	pw->pw_uid = uid;
	pw->pw_gid = 7;
	*result = pw;
	return 0;
}

/*
 * getgrouplist - supply one natural group for the synthetic account.
 * @user: account name.
 * @gid: primary group to include.
 * @groups: output group array.
 * @count: capacity on input, group count on output.
 *
 * Returns 1; the library supplies space for at least one group.
 */
int getgrouplist(const char *user, gid_t gid, gid_t *groups, int *count)
{
	check(!strcmp(user, "transition-test") && gid == expected_gid &&
	      *count >= 1,
	      "Unexpected group lookup");
	groups[0] = gid;
	*count = 1;
	return 1;
}

/*
 * initgroups - exercise the same SETGID check as staged group application.
 * @user: account name.
 * @gid: natural primary group.
 *
 * Returns the simulated setgroups result.
 */
int initgroups(const char *user, gid_t gid)
{
	check(!strcmp(user, "transition-test") && gid == expected_gid,
	      "Unexpected initgroups account");
	return setgroups(1, &gid);
}

/*
 * setresgid - accept the explicit primary group without changing real IDs.
 * @real: requested real GID.
 * @effective: requested effective GID.
 * @saved: requested saved GID.
 *
 * Returns 0 after checking the expected primary group override.
 */
int setresgid(gid_t real, gid_t effective, gid_t saved)
{
	check(real == expected_gid && effective == real && saved == real,
	      "Unexpected group transition");
	return 0;
}

/*
 * setresuid - verify SETUID is available without changing real user IDs.
 * @real: requested real UID.
 * @effective: requested effective UID.
 * @saved: requested saved UID.
 *
 * Returns 0; unexpected IDs or missing SETUID fail the test.
 */
int setresuid(uid_t real, uid_t effective, uid_t saved)
{
	uid_calls++;
	check(real == 42 && effective == 42 && saved == 42,
	      "Unexpected user transition");
	check(current_caps[0].effective & (1U << CAP_SETUID),
	      "Missing temporary SETUID");
	return 0;
}

/*
 * prepare_transition - reset the simulated credentials and lookup state.
 *
 * Returns no value; stages an empty final capability set.
 */
static void prepare_transition(void)
{
	memset(current_caps, 0, sizeof(current_caps));
	current_caps[0].effective = (1U << CAP_SETGID) | (1U << CAP_SETUID);
	current_caps[0].permitted = current_caps[0].effective;
	memset(&lookup, 0, sizeof(lookup));
	group_calls = uid_calls = 0;
	capng_clear(CAPNG_SELECT_ALL);
}

/*
 * test_account_lookup - exercise private lookup storage and failure cleanup.
 *
 * Returns no value; checks both natural-group modes with a default or explicit
 * primary group, repeated ERANGE, missing accounts, and lookup errors.
 */
static void test_account_lookup(void)
{
	int merged, explicit_gid, outcome;
	gid_t group = 8;

	for (merged = 0; merged <= 1; merged++) {
		for (explicit_gid = 0; explicit_gid <= 1; explicit_gid++) {
			for (outcome = 0; outcome < 4; outcome++) {
				capng_flags_t flags = CAPNG_INIT_SUPP_GRP;
				int rc;

				prepare_transition();
				expected_gid = explicit_gid ? 9 : 7;
				lookup.retries = outcome == 1 ? 2 : 0;
				lookup.missing = outcome == 2;
				lookup.error = outcome == 3 ? EIO : 0;
				if (merged) {
					flags |= CAPNG_APPLY_STAGED_GROUPS;
					check(capng_stage_additional_groups(&group, 1)
					      == 0, "Failed to stage lookup groups");
				}
				rc = capng_change_id(42, explicit_gid ? 9 : -1,
						     flags);
				check(rc == (outcome >= 2 ? -10 : 0),
				      "Unexpected account lookup result");
				check(lookup.calls == lookup.retries + 1,
				      "Unexpected account lookup count");
				check(group_calls == (outcome < 2) &&
				      uid_calls == (outcome < 2) && !keepcaps,
				      "Incorrect lookup failure cleanup");
				check(capng_change_id(-1, -1,
						CAPNG_APPLY_STAGED_GROUPS) == -13,
				      "Lookup retained staged groups");
			}
		}
	}
}

/*
 * main - verify group-only transitions and exact final SETGID preservation.
 *
 * Returns success only when each mode works without real process privileges.
 */
int main(void)
{
	const capng_flags_t modes[] = {
		CAPNG_APPLY_STAGED_GROUPS,
		CAPNG_INIT_SUPP_GRP,
		CAPNG_INIT_SUPP_GRP | CAPNG_APPLY_STAGED_GROUPS,
	};
	gid_t group = 8;
	size_t i;
	int retain;

	for (i = 0; i < sizeof(modes) / sizeof(modes[0]); i++) {
		for (retain = 0; retain <= 1; retain++) {
			prepare_transition();
			if (retain)
				check(capng_update(CAPNG_ADD, CAPNG_PERMITTED,
						   CAP_SETGID) == 0,
				      "Failed to stage permitted-only SETGID");
			if (modes[i] & CAPNG_APPLY_STAGED_GROUPS)
				check(capng_stage_additional_groups(&group, 1) == 0,
				      "Failed to stage groups");
			check(capng_change_id(i == 0 ? -1 : 42, -1, modes[i]) == 0,
			      "Group-only transition failed");
			check(group_calls == 1, "Groups were not applied once");
			check(current_caps[0].effective == 0 &&
			      current_caps[0].permitted ==
				(retain ? 1U << CAP_SETGID : 0),
			      "Temporary capabilities changed the final request");
			check(capng_change_id(-1, -1,
					     CAPNG_APPLY_STAGED_GROUPS) == -13,
			      "Staged groups were not consumed");
		}
	}
	test_account_lookup();
	return EXIT_SUCCESS;
}
