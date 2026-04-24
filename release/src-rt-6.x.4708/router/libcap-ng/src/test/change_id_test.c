/* change_id_test.c -- capng_change_id additional group tests
 * Copyright 2026 Red Hat Inc.
 * All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; see the file COPYING.LIB. If not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor
 * Boston, MA 02110-1335, USA.
 */

#include "config.h"
#include "../cap-ng.h"
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static void fail(const char *msg)
{
	fprintf(stderr, "%s\n", msg);
	_exit(EXIT_FAILURE);
}

static int gid_in_list(const gid_t *gids, size_t count, gid_t gid)
{
	size_t i;

	for (i = 0; i < count; i++) {
		if (gids[i] == gid)
			return 1;
	}
	return 0;
}

static int rc_in_list(int rc, const int *allowed, size_t count)
{
	size_t i;

	for (i = 0; i < count; i++) {
		if (rc == allowed[i])
			return 1;
	}
	return 0;
}

static int get_current_groups(gid_t **gids, size_t *count)
{
	int ngroups;

	*gids = NULL;
	*count = 0;

	ngroups = getgroups(0, NULL);
	if (ngroups < 0)
		return -1;
	if (ngroups == 0)
		return 0;

	*gids = malloc(sizeof(gid_t) * ngroups);
	if (*gids == NULL)
		return -1;
	ngroups = getgroups(ngroups, *gids);
	if (ngroups < 0) {
		free(*gids);
		*gids = NULL;
		return -1;
	}
	*count = ngroups;
	return 0;
}

static int get_natural_groups(uid_t uid, gid_t gid, gid_t **gids, size_t *count)
{
	struct passwd *pw;
	gid_t *list;
	int ngroups = 1;
	int rc;

	*gids = NULL;
	*count = 0;

	pw = getpwuid(uid);
	if (pw == NULL)
		return -1;

	list = malloc(sizeof(gid_t));
	if (list == NULL)
		return -1;

	rc = getgrouplist(pw->pw_name, gid, list, &ngroups);
	if (rc == -1) {
		gid_t *tmp;

		tmp = realloc(list, sizeof(gid_t) * ngroups);
		if (tmp == NULL) {
			free(list);
			return -1;
		}
		list = tmp;
		rc = getgrouplist(pw->pw_name, gid, list, &ngroups);
	}
	if (rc == -1) {
		free(list);
		return -1;
	}

	*gids = list;
	*count = ngroups;
	return 0;
}

static int merge_groups(const gid_t *base, size_t base_cnt,
		const gid_t *extra, size_t extra_cnt, gid_t **merged,
		size_t *merged_cnt)
{
	gid_t *list;
	size_t i, count = 0, total = base_cnt + extra_cnt;

	*merged = NULL;
	*merged_cnt = 0;
	if (total == 0)
		return 0;

	list = malloc(sizeof(gid_t) * total);
	if (list == NULL)
		return -1;

	for (i = 0; i < base_cnt; i++) {
		if (gid_in_list(list, count, base[i]) == 0)
			list[count++] = base[i];
	}
	for (i = 0; i < extra_cnt; i++) {
		if (gid_in_list(list, count, extra[i]) == 0)
			list[count++] = extra[i];
	}

	*merged = list;
	*merged_cnt = count;
	return 0;
}

static void check_groups(const gid_t *expected, size_t expected_cnt)
{
	gid_t *actual;
	size_t actual_cnt;
	size_t i;

	if (get_current_groups(&actual, &actual_cnt))
		fail("Failed to get current additional groups");
	if (actual_cnt != expected_cnt)
		fail("Unexpected additional group count");
	for (i = 0; i < expected_cnt; i++) {
		if (actual[i] != expected[i])
			fail("Unexpected additional group value");
	}
	free(actual);
}

static int choose_extra_gid(const gid_t *skip, size_t skip_cnt, gid_t *gid)
{
	struct group *gr;

	setgrent();
	while ((gr = getgrent()) != NULL) {
		if (gid_in_list(skip, skip_cnt, gr->gr_gid) == 0) {
			*gid = gr->gr_gid;
			endgrent();
			return 0;
		}
	}
	endgrent();
	return -1;
}

static int read_bounding_cap(unsigned int cap)
{
#ifdef PR_CAPBSET_READ
	return prctl(PR_CAPBSET_READ, cap, 0, 0, 0);
#else
	(void)cap;
	return -1;
#endif
}

static int find_drop_test_bounding_cap(unsigned int *cap)
{
	unsigned int i;

	for (i = 0; cap_valid(i); i++) {
		int rc;

		if (i == CAP_SETPCAP || i == CAP_SETUID || i == CAP_SETGID)
			continue;
		rc = read_bounding_cap(i);
		if (rc < 0)
			return -1;
		if (rc == 1) {
			*cap = i;
			return 0;
		}
	}
	for (i = 0; cap_valid(i); i++) {
		int rc;

		rc = read_bounding_cap(i);
		if (rc < 0)
			return -1;
		if (rc == 1) {
			*cap = i;
			return 0;
		}
	}
	return -1;
}

static void test_staged_only(void)
{
	static const int allowed[] = { -2, -3, -14 };
	gid_t staged[2];
	gid_t *natural = NULL;
	size_t natural_cnt = 0;
	size_t staged_cnt = 1;
	int rc;

	if (capng_get_caps_process())
		fail("Failed to initialize libcap-ng state");
	if (get_natural_groups(getuid(), getgid(), &natural, &natural_cnt))
		fail("Failed to resolve natural groups");

	staged[0] = getgid();
	if (choose_extra_gid(natural, natural_cnt, &staged[1]) == 0)
		staged_cnt = 2;
	free(natural);

	rc = capng_stage_additional_groups(staged, staged_cnt);
	if (rc)
		fail("Failed to stage additional groups");
	rc = capng_change_id(-1, -1, CAPNG_APPLY_STAGED_GROUPS);
	if (rc == 0)
		check_groups(staged, staged_cnt);
	else if (rc_in_list(rc, allowed, sizeof(allowed) / sizeof(int)) == 0)
		fail("Unexpected staged-only return code");
}

static void test_init_only(void)
{
	static const int allowed[] = { -2, -3, -5 };
	gid_t *natural = NULL;
	size_t natural_cnt = 0;
	int rc;

	if (capng_get_caps_process())
		fail("Failed to initialize libcap-ng state");
	if (get_natural_groups(getuid(), getgid(), &natural, &natural_cnt))
		fail("Failed to resolve natural groups");

	rc = capng_change_id(getuid(), -1, CAPNG_INIT_SUPP_GRP);
	if (rc == 0)
		check_groups(natural, natural_cnt);
	else if (rc_in_list(rc, allowed, sizeof(allowed) / sizeof(int)) == 0)
		fail("Unexpected init-only return code");
	free(natural);
}

static void test_init_and_staged(void)
{
	static const int allowed[] = { -2, -3, -14 };
	gid_t staged[2];
	gid_t *natural = NULL, *merged = NULL;
	size_t natural_cnt = 0, merged_cnt = 0;
	size_t staged_cnt = 1;
	int rc;

	if (capng_get_caps_process())
		fail("Failed to initialize libcap-ng state");
	if (get_natural_groups(getuid(), getgid(), &natural, &natural_cnt))
		fail("Failed to resolve natural groups");

	staged[0] = getgid();
	if (choose_extra_gid(natural, natural_cnt, &staged[1]) == 0)
		staged_cnt = 2;
	if (merge_groups(natural, natural_cnt, staged, staged_cnt,
				&merged, &merged_cnt))
		fail("Failed to merge expected additional groups");

	rc = capng_stage_additional_groups(staged, staged_cnt);
	if (rc)
		fail("Failed to stage additional groups");
	rc = capng_change_id(getuid(), -1,
			CAPNG_INIT_SUPP_GRP |
			CAPNG_APPLY_STAGED_GROUPS);
	if (rc == 0)
		check_groups(merged, merged_cnt);
	else if (rc_in_list(rc, allowed, sizeof(allowed) / sizeof(int)) == 0)
		fail("Unexpected init+staged return code");

	free(natural);
	free(merged);
}

static void test_invalid_drop_and_staged(void)
{
	gid_t gid = getgid();
	int rc;

	if (capng_get_caps_process())
		fail("Failed to initialize libcap-ng state");
	rc = capng_stage_additional_groups(&gid, 1);
	if (rc)
		fail("Failed to stage additional groups");
	rc = capng_change_id(-1, -1,
			CAPNG_DROP_SUPP_GRP |
			CAPNG_APPLY_STAGED_GROUPS);
	if (rc != -12)
		fail("Unexpected drop+staged return code");
}

static void test_staged_ignored_without_flag(void)
{
	static const int allowed[] = { -2, -3 };
	gid_t gid = getgid();
	int rc;

	if (capng_get_caps_process())
		fail("Failed to initialize libcap-ng state");
	rc = capng_stage_additional_groups(&gid, 1);
	if (rc)
		fail("Failed to stage additional groups");
	rc = capng_change_id(-1, -1, CAPNG_NO_FLAG);
	if (rc && rc_in_list(rc, allowed, sizeof(allowed) / sizeof(int)) == 0)
		fail("Unexpected return code when staged groups are ignored");
	rc = capng_change_id(-1, -1, CAPNG_APPLY_STAGED_GROUPS);
	if (rc != -13)
		fail("Staged groups were not cleared when ignored");
}

static void test_staged_cleared_after_use(void)
{
	static const int allowed[] = { -2, -3, -14 };
	gid_t gid = getgid();
	int rc;

	if (capng_get_caps_process())
		fail("Failed to initialize libcap-ng state");
	rc = capng_stage_additional_groups(&gid, 1);
	if (rc)
		fail("Failed to stage additional groups");
	rc = capng_change_id(-1, -1, CAPNG_APPLY_STAGED_GROUPS);
	if (rc && rc_in_list(rc, allowed, sizeof(allowed) / sizeof(int)) == 0)
		fail("Unexpected return code when consuming staged groups");
	rc = capng_change_id(-1, -1, CAPNG_APPLY_STAGED_GROUPS);
	if (rc != -13)
		fail("Staged groups were not cleared after use");
}

static void test_apply_bounding_during_change_id(void)
{
	static const int allowed[] = { -2, -3, -8, -9 };
	unsigned int cap;
	int rc;

	if (capng_get_caps_process())
		fail("Failed to initialize libcap-ng state");
	if (find_drop_test_bounding_cap(&cap))
		return;
	if (capng_update(CAPNG_DROP, CAPNG_BOUNDING_SET, cap))
		fail("Failed to prepare bounding set change");

	rc = capng_change_id(-1, -1, CAPNG_APPLY_BOUNDING);
	if (rc == 0) {
		if (read_bounding_cap(cap) != 0)
			fail("Prepared bounding set was not applied");
	} else if (rc_in_list(rc, allowed, sizeof(allowed) / sizeof(int)) == 0)
		fail("Unexpected apply-bounding return code");
}

static void test_apply_bounding_noop_without_state(void)
{
	static const int allowed[] = { -2, -3 };
	unsigned int cap;
	int before, rc;

	if (capng_get_caps_process())
		fail("Failed to initialize libcap-ng state");
	if (find_drop_test_bounding_cap(&cap))
		return;
	before = read_bounding_cap(cap);
	if (before != 1)
		fail("Expected selected bounding capability to be present");

	rc = capng_change_id(-1, -1, CAPNG_APPLY_BOUNDING);
	if (rc == 0) {
		if (read_bounding_cap(cap) != before)
			fail("Unprepared bounding set should be a no-op");
	} else if (rc_in_list(rc, allowed, sizeof(allowed) / sizeof(int)) == 0)
		fail("Unexpected no-op apply-bounding return code");
}

static void test_invalid_apply_and_clear_bounding(void)
{
	if (capng_get_caps_process())
		fail("Failed to initialize libcap-ng state");
	if (capng_change_id(-1, -1,
			CAPNG_APPLY_BOUNDING | CAPNG_CLEAR_BOUNDING) != -17)
		fail("Unexpected apply+clear bounding return code");
}

static void test_apply_bounding_preserves_requested_helper_cap(void)
{
	static const int allowed[] = { -2, -3, -8, -9 };
	unsigned int cap;
	int rc;

	if (capng_get_caps_process())
		fail("Failed to initialize libcap-ng state");
	if (find_drop_test_bounding_cap(&cap))
		return;
	if (capng_have_capability(CAPNG_PERMITTED, CAP_SETPCAP) == 0)
		return;
	if (capng_update(CAPNG_ADD, CAPNG_EFFECTIVE|CAPNG_PERMITTED,
				CAP_SETPCAP))
		fail("Failed to request CAP_SETPCAP");
	if (capng_update(CAPNG_DROP, CAPNG_BOUNDING_SET, cap))
		fail("Failed to prepare bounding set change");

	rc = capng_change_id(-1, -1, CAPNG_APPLY_BOUNDING);
	if (rc == 0) {
		if (capng_have_capability(CAPNG_EFFECTIVE, CAP_SETPCAP) == 0)
			fail("Requested CAP_SETPCAP was removed from effective");
		if (capng_have_capability(CAPNG_PERMITTED, CAP_SETPCAP) == 0)
			fail("Requested CAP_SETPCAP was removed from permitted");
	} else if (rc_in_list(rc, allowed, sizeof(allowed) / sizeof(int)) == 0)
		fail("Unexpected helper capability cleanup return code");
}

static void test_apply_bounding_preserves_permitted_only_helper_cap(void)
{
	static const int allowed[] = { -2, -3, -8, -9 };
	unsigned int cap;
	int rc;

	if (capng_get_caps_process())
		fail("Failed to initialize libcap-ng state");
	if (find_drop_test_bounding_cap(&cap))
		return;
	if (capng_have_capability(CAPNG_PERMITTED, CAP_SETPCAP) == 0)
		return;
	if (capng_update(CAPNG_DROP, CAPNG_EFFECTIVE, CAP_SETPCAP))
		fail("Failed to clear effective CAP_SETPCAP request");
	if (capng_update(CAPNG_ADD, CAPNG_PERMITTED, CAP_SETPCAP))
		fail("Failed to request permitted CAP_SETPCAP");
	if (capng_update(CAPNG_DROP, CAPNG_BOUNDING_SET, cap))
		fail("Failed to prepare bounding set change");

	rc = capng_change_id(-1, -1, CAPNG_APPLY_BOUNDING);
	if (rc == 0) {
		if (capng_have_capability(CAPNG_EFFECTIVE, CAP_SETPCAP))
			fail("Permitted-only CAP_SETPCAP became effective");
		if (capng_have_capability(CAPNG_PERMITTED, CAP_SETPCAP) == 0)
			fail("Permitted-only CAP_SETPCAP was removed");
	} else if (rc_in_list(rc, allowed, sizeof(allowed) / sizeof(int)) == 0)
		fail("Unexpected permitted-only helper cleanup return code");
}

static void test_bounding_state_ignored_without_flag(void)
{
	static const int allowed[] = { -2, -3 };
	unsigned int cap;
	int rc;

	if (capng_get_caps_process())
		fail("Failed to initialize libcap-ng state");
	if (find_drop_test_bounding_cap(&cap))
		return;
	if (capng_update(CAPNG_DROP, CAPNG_BOUNDING_SET, cap))
		fail("Failed to prepare bounding set change");

	rc = capng_change_id(-1, -1, CAPNG_NO_FLAG);
	if (rc == 0) {
		if (read_bounding_cap(cap) != 1)
			fail("Bounding set changed without CAPNG_APPLY_BOUNDING");
	} else if (rc_in_list(rc, allowed, sizeof(allowed) / sizeof(int)) == 0)
		fail("Unexpected backward compatibility return code");
}

static void run_test(const char *name, void (*test)(void))
{
	pid_t pid;
	int status;

	fflush(NULL);
	pid = fork();
	if (pid < 0) {
		perror("fork");
		abort();
	}
	if (pid == 0) {
		test();
		_exit(EXIT_SUCCESS);
	}
	if (waitpid(pid, &status, 0) < 0) {
		perror("waitpid");
		abort();
	}
	if (WIFEXITED(status) == 0 || WEXITSTATUS(status) != EXIT_SUCCESS) {
		printf("Failed %s test\n", name);
		abort();
	}
}

int main(void)
{
	puts("Doing capng_change_id additional group tests...");
	run_test("staged-only additional groups", test_staged_only);
	run_test("init-only additional groups", test_init_only);
	run_test("init+staged additional groups", test_init_and_staged);
	run_test("invalid drop+staged combination",
			test_invalid_drop_and_staged);
	run_test("staged groups ignored without flag",
			test_staged_ignored_without_flag);
	run_test("staged state cleared after use",
			test_staged_cleared_after_use);
	run_test("apply prepared bounding set",
			test_apply_bounding_during_change_id);
	run_test("apply bounding is no-op without prepared state",
			test_apply_bounding_noop_without_state);
	run_test("invalid apply+clear bounding combination",
			test_invalid_apply_and_clear_bounding);
	run_test("helper cleanup keeps requested capability",
			test_apply_bounding_preserves_requested_helper_cap);
	run_test("helper cleanup keeps permitted-only capability",
			test_apply_bounding_preserves_permitted_only_helper_cap);
	run_test("prepared bounding state ignored without flag",
			test_bounding_state_ignored_without_flag);
	return EXIT_SUCCESS;
}
