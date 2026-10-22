// SPDX-License-Identifier: GPL-2.0-or-later
/* cap_audit_service_test.c -- service credential regression tests
 * Copyright 2026 Red Hat Inc.
 * All Rights Reserved.
 */

#include "config.h"

#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <unistd.h>

#include "cap_audit.h"

struct audit_state state;
int audit_machine;

static int change_id_calls;
static int changed_uid;
static int changed_gid;
static capng_flags_t changed_flags;
static int seed_service_caps;

int __real_capng_get_caps_process(void);

/*
 * __wrap_capng_get_caps_process - seed deterministic launch capabilities.
 *
 * Returns the real read result outside capability tests. In those tests,
 * supplies CHOWN/KILL privileges and unrelated inherited/ambient KILL state
 * without changing kernel credentials.
 */
int __wrap_capng_get_caps_process(void)
{
	if (!seed_service_caps)
		return __real_capng_get_caps_process();
	capng_clear(CAPNG_SELECT_ALL);
	if (capng_update(CAPNG_ADD, CAPNG_EFFECTIVE | CAPNG_PERMITTED,
			 CAP_CHOWN) != 0)
		return -1;
	return capng_update(CAPNG_ADD, CAPNG_EFFECTIVE | CAPNG_PERMITTED |
			    CAPNG_INHERITABLE | CAPNG_AMBIENT, CAP_KILL);
}

static void fail(const char *msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}

const char *cap_name_safe(int cap)
{
	(void)cap;
	return "unknown";
}

int __wrap_capng_change_id(int uid, int gid, capng_flags_t flags)
{
	change_id_calls++;
	changed_uid = uid;
	changed_gid = gid;
	changed_flags = flags;
	return 0;
}

static char *write_service(const char *dir, const char *name,
			   const char *contents)
{
	char *path;
	size_t len = strlen(contents);
	ssize_t written;
	int fd;

	if (asprintf(&path, "%s/%s", dir, name) < 0)
		fail("Failed to allocate service path");
	fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if (fd < 0)
		fail("Failed to open temporary service");
	written = write(fd, contents, len);
	close(fd);
	if (written < 0 || (size_t)written != len)
		fail("Failed to write temporary service");

	return path;
}

static int parse_unit(const char *dir, const char *name, const char *contents,
		      service_config_t *cfg)
{
	char *path;
	int rc;

	path = write_service(dir, name, contents);
	rc = parse_service_file(path, cfg);
	unlink(path);
	free(path);
	return rc;
}

static void expect_parse_failure(const char *dir, const char *name,
				 const char *contents)
{
	service_config_t cfg;

	if (parse_unit(dir, name, contents, &cfg) == 0) {
		free_config(&cfg);
		fail(name);
	}
}

static void test_dynamic_user(const char *dir)
{
	service_config_t cfg;
	const char unit[] =
		"[Service]\n"
		"DynamicUser=yes\n"
		"ExecStart=/usr/bin/true\n";

	if (parse_unit(dir, "dynamic.service", unit, &cfg) != 0)
		fail("DynamicUser service should parse");
	if (!cfg.user_resolved || !cfg.user_raw ||
	    strcmp(cfg.user_raw, "nobody") != 0)
		fail("DynamicUser should resolve to nobody for simulation");
	if (cfg.user_uid == 0 || cfg.user_uid > INT_MAX ||
	    cfg.user_primary_gid == 0 || cfg.user_primary_gid > INT_MAX)
		fail("DynamicUser simulation must use representable non-root IDs");

	change_id_calls = 0;
	if (apply_service_config(&cfg) != 0)
		fail("DynamicUser credentials should apply");
	if (change_id_calls != 1 || changed_uid != (int)cfg.user_uid ||
	    changed_gid != (int)cfg.user_primary_gid)
		fail("DynamicUser credentials did not reach capng_change_id");
	if (!(changed_flags & CAPNG_INIT_SUPP_GRP))
		fail("DynamicUser should initialize supplementary groups");

	free_config(&cfg);
}

static void test_unresolved_dynamic_user(const char *dir)
{
	service_config_t cfg;
	const char unit[] =
		"[Service]\n"
		"DynamicUser=yes\n"
		"User=cap-audit-no-such-user\n"
		"ExecStart=/usr/bin/true\n";

	if (parse_unit(dir, "dynamic-name.service", unit, &cfg) != 0)
		fail("Unresolved DynamicUser name should use the simulation user");
	if (!cfg.user_resolved || !cfg.user_raw ||
	    strcmp(cfg.user_raw, "nobody") != 0)
		fail("Unresolved DynamicUser name should map to nobody");
	free_config(&cfg);
}

static void test_valid_credentials(const char *dir)
{
	service_config_t cfg;
	const char unit[] =
		"[Service]\n"
		"User=0\n"
		"Group=0\n"
		"SupplementaryGroups=1 2\n"
		/* A later false assignment must undo an earlier true value. */
		"DynamicUser=yes\nDynamicUser=no\n"
		"NoNewPrivileges=yes\nNoNewPrivileges=no\n"
		"ExecStart=/usr/bin/true\n";

	if (parse_unit(dir, "valid.service", unit, &cfg) != 0)
		fail("Valid numeric credentials should parse");
	if (!cfg.user_resolved || cfg.user_uid != 0 ||
	    !cfg.group_is_set || cfg.group_gid != 0 ||
	    cfg.sup_groups.count != 2)
		fail("Valid numeric credentials were not preserved");
	if (cfg.dynamic_user || cfg.no_new_privs)
		fail("Boolean directives did not preserve the last assignment");
	free_config(&cfg);
}

static void test_invalid_credentials(const char *dir)
{
	static const struct {
		const char *name;
		const char *unit;
	} tests[] = {
		{ "negative-user.service",
		  "[Service]\nUser=-1\nExecStart=/usr/bin/true\n" },
		{ "positive-user.service",
		  "[Service]\nUser=+1\nExecStart=/usr/bin/true\n" },
		{ "sentinel-user.service",
		  "[Service]\nUser=4294967295\nExecStart=/usr/bin/true\n" },
		{ "wrapped-user.service",
		  "[Service]\nUser=4294967296\nExecStart=/usr/bin/true\n" },
		{ "int-user.service",
		  "[Service]\nUser=2147483648\nExecStart=/usr/bin/true\n" },
		{ "legacy-user.service",
		  "[Service]\nUser=65535\nExecStart=/usr/bin/true\n" },
		{ "negative-group.service",
		  "[Service]\nGroup=-1\nExecStart=/usr/bin/true\n" },
		{ "wrapped-group.service",
		  "[Service]\nGroup=4294967296\nExecStart=/usr/bin/true\n" },
		{ "negative-supplementary.service",
		  "[Service]\nSupplementaryGroups=-1\nExecStart=/usr/bin/true\n" },
		{ "wrapped-supplementary.service",
		  "[Service]\nSupplementaryGroups=4294967296\nExecStart=/usr/bin/true\n" },
		{ "dynamic-negative-user.service",
		  "[Service]\nDynamicUser=yes\nUser=-1\n"
		  "ExecStart=/usr/bin/true\n" },
		{ "invalid-dynamic-user.service",
		  "[Service]\nDynamicUser=invalid\nExecStart=/usr/bin/true\n" },
		{ "invalid-no-new-privileges.service",
		  "[Service]\nNoNewPrivileges=invalid\nExecStart=/usr/bin/true\n" },
	};
	size_t i;

	for (i = 0; i < sizeof(tests) / sizeof(tests[0]); i++)
		expect_parse_failure(dir, tests[i].name, tests[i].unit);
}

static void test_sink_validation(void)
{
	service_config_t cfg = {
		.user_uid = (uid_t)UINT32_MAX,
		.user_primary_gid = (gid_t)UINT32_MAX,
		.user_is_set = true,
		.user_resolved = true,
	};

	change_id_calls = 0;
	if (apply_service_config(&cfg) == 0)
		fail("Unrepresentable credentials should fail before application");
	if (change_id_calls != 0)
		fail("Invalid credentials reached capng_change_id");
}

static void test_exec_start_privilege_prefixes(const char *dir)
{
	service_config_t cfg;
	const char bang_unit[] =
		"[Service]\n"
		"User=1234\n"
		"Group=1234\n"
		"SupplementaryGroups=1 2\n"
		"CapabilityBoundingSet=CAP_CHOWN\n"
		"NoNewPrivileges=yes\n"
		"ExecStart=-!/usr/bin/true\n";
	const char plus_unit[] =
		"[Service]\n"
		"ExecStart=-+/usr/bin/true\n";
	const char combined_plus_unit[] =
		"[Service]\n"
		"ExecStart=!-+/usr/bin/true\n";
	const char double_bang_unit[] =
		"[Service]\n"
		"ExecStart=!!/usr/bin/true\n";

	if (parse_unit(dir, "bang.service", bang_unit, &cfg) != 0)
		fail("ExecStart=! service should parse");
	if (!cfg.exec_start_no_setuid)
		fail("ExecStart=! credential semantics were not retained");
	if (!cfg.exec_argv || strcmp(cfg.exec_argv[0], "/usr/bin/true"))
		fail("ExecStart=! prefixes were not removed from executable");

	change_id_calls = 0;
	changed_uid = 0;
	changed_gid = 0;
	changed_flags = CAPNG_NO_FLAG;
	if (apply_service_config(&cfg) != 0)
		fail("ExecStart=! configuration should apply");
	if (change_id_calls != 1 || changed_uid != -1 || changed_gid != -1)
		fail("ExecStart=! must leave credential changes to the command");
	if (changed_flags & (CAPNG_INIT_SUPP_GRP |
			     CAPNG_APPLY_STAGED_GROUPS))
		fail("ExecStart=! must not apply supplementary groups");
	if (!(changed_flags & CAPNG_APPLY_BOUNDING))
		fail("ExecStart=! must still apply the capability bounding set");
	if (prctl(PR_GET_NO_NEW_PRIVS, 0, 0, 0, 0) != 1)
		fail("ExecStart=! must still apply NoNewPrivileges");
	free_config(&cfg);

	expect_parse_failure(dir, "plus.service", plus_unit);
	expect_parse_failure(dir, "combined-plus.service", combined_plus_unit);
	expect_parse_failure(dir, "double-bang.service", double_bang_unit);
	expect_parse_failure(dir, "argv0.service",
		"[Service]\nExecStart=@/usr/bin/echo alias payload\n");
	expect_parse_failure(dir, "combined-argv0.service",
		"[Service]\nExecStart=-!:@/usr/bin/echo alias payload\n");
}

static void test_exec_start_environment_words(const char *dir)
{
	service_config_t cfg;
	const char unit[] =
		"[Service]\n"
		"ExecStart=/usr/bin/firewalld --nofork --nopid "
		"$FIREWALLD_ARGS --label=$KEEP\n";
	const char literal_unit[] =
		"[Service]\n"
		"ExecStart=:/usr/bin/true $LITERAL_ARG\n";

	if (parse_unit(dir, "environment.service", unit, &cfg) != 0)
		fail("ExecStart environment argument should parse");
	if (cfg.exec_argc != 4 ||
	    strcmp(cfg.exec_argv[0], "/usr/bin/firewalld") ||
	    strcmp(cfg.exec_argv[1], "--nofork") ||
	    strcmp(cfg.exec_argv[2], "--nopid") ||
	    strcmp(cfg.exec_argv[3], "--label=$KEEP"))
		fail("Standalone ExecStart environment argument was not omitted");
	free_config(&cfg);

	if (parse_unit(dir, "literal-environment.service", literal_unit,
		       &cfg) != 0)
		fail("ExecStart=: environment argument should parse");
	if (cfg.exec_argc != 2 ||
	    strcmp(cfg.exec_argv[0], "/usr/bin/true") ||
	    strcmp(cfg.exec_argv[1], "$LITERAL_ARG"))
		fail("ExecStart=: must disable environment argument omission");
	free_config(&cfg);
}

/*
 * test_service_ambient - apply ambient settings for root, non-root and !.
 * @dir: temporary directory for unit fixtures.
 *
 * Returns no value; fails on a missing ambient grant, leaked ambient state,
 * or an unintended effective/permitted change. Credential application is
 * wrapped, so no real privileges are required or changed.
 */
static void test_service_ambient(const char *dir)
{
	static const char *units[] = {
		"[Service]\nAmbientCapabilities=CAP_CHOWN\n"
		"ExecStart=/usr/bin/true\n",
		"[Service]\nUser=0\nAmbientCapabilities=CAP_CHOWN\n"
		"ExecStart=/usr/bin/true\n",
		"[Service]\nUser=1234\nAmbientCapabilities=CAP_CHOWN\n"
		"ExecStart=/usr/bin/true\n",
		"[Service]\nUser=1234\nAmbientCapabilities=CAP_CHOWN\n"
		"ExecStart=!/usr/bin/true\n",
	};
	service_config_t cfg;
	size_t i;

	seed_service_caps = 1;
	for (i = 0; i < sizeof(units) / sizeof(units[0]); i++) {
		if (parse_unit(dir, "ambient.service", units[i], &cfg) != 0)
			fail("Ambient service should parse");
		change_id_calls = 0;
		if (apply_service_config(&cfg) != 0 || change_id_calls != 1)
			fail("Ambient service should apply");
		if (capng_have_capability(CAPNG_AMBIENT, CAP_CHOWN) != 1 ||
		    capng_have_capability(CAPNG_INHERITABLE, CAP_CHOWN) != 1 ||
		    capng_have_capability(CAPNG_AMBIENT, CAP_KILL) != 0 ||
		    (changed_flags & CAPNG_CLEAR_AMBIENT))
			fail("Configured ambient capabilities were not staged exactly");
		if (capng_have_capability(CAPNG_EFFECTIVE, CAP_CHOWN) != 1 ||
		    capng_have_capability(CAPNG_PERMITTED, CAP_CHOWN) != 1 ||
		    capng_have_capability(CAPNG_EFFECTIVE, CAP_KILL) != (i != 2) ||
		    capng_have_capability(CAPNG_PERMITTED, CAP_KILL) != (i != 2))
			fail("Ambient staging changed the wrong process capabilities");
		free_config(&cfg);
	}
	seed_service_caps = 0;
}

/*
 * test_capability_lists - exercise identical ambient and bounding list rules.
 * @dir: temporary directory for unit fixtures.
 *
 * Returns no value; fails if union, subtraction, or a reset produces an
 * unexpected set. All checks operate on parsed data without applying caps.
 */
static void test_capability_lists(const char *dir)
{
	static const char *keys[] = {
		"CapabilityBoundingSet", "AmbientCapabilities",
	};
	static const struct {
		const char *first, *second;
		int chown, kill, other;
	} cases[] = {
		{ "CAP_CHOWN", "CAP_KILL", 1, 1, 0 },
		{ "CAP_CHOWN CAP_KILL", "~CAP_CHOWN", 0, 1, 0 },
		{ "CAP_CHOWN", "", 0, 0, 0 },
		{ "CAP_CHOWN", "~", 1, 1, 1 },
		{ "~CAP_CHOWN", "~CAP_KILL", 0, 0, 1 },
		{ "", "~CAP_CHOWN", 0, 0, 0 },
		{ "~", "", 0, 0, 0 },
		{ "", "CAP_KILL", 0, 1, 0 },
	};
	size_t key, i;

	for (key = 0; key < sizeof(keys) / sizeof(keys[0]); key++) {
		for (i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
			service_config_t cfg;
			const cap_set_t *set;
			char *unit;
			int cap;

			if (asprintf(&unit, "[Service]\n%s=%s\n%s=%s\n",
				     keys[key], cases[i].first,
				     keys[key], cases[i].second) < 0)
				fail("Failed to allocate capability unit");
			if (parse_unit(dir, "caps.service", unit, &cfg) != 0)
				fail("Capability list should parse");
			free(unit);
			set = key == 0 ? &cfg.bounding : &cfg.ambient;
			if (!set->seen)
				fail("Capability assignment was not recorded");
			for (cap = 0; cap <= CAP_LAST_CAP; cap++) {
				int expected = cap == CAP_CHOWN ? cases[i].chown :
					cap == CAP_KILL ? cases[i].kill : cases[i].other;

				if (set->caps[cap] != expected)
					fail("Incorrect capability list merge or reset");
			}
			free_config(&cfg);
		}
	}
}

/*
 * test_exec_start_reset - reject command sequences but allow explicit resets.
 * @dir: temporary directory for unit fixtures.
 *
 * Returns no value; fails if a command is silently discarded or a reset
 * retains an earlier command's arguments or credential override.
 */
static void test_exec_start_reset(const char *dir)
{
	service_config_t cfg;
	const char unit[] =
		"[Service]\nExecStart=!/usr/bin/false discarded\n"
		"ExecStart=\nExecStart=/usr/bin/true kept\n";

	expect_parse_failure(dir, "oneshot-sequence.service",
		"[Service]\nType=oneshot\nExecStart=/usr/bin/echo first\n"
		"ExecStart=/usr/bin/echo second\n");
	expect_parse_failure(dir, "simple-sequence.service",
		"[Service]\nExecStart=/usr/bin/echo first\n"
		"ExecStart=/usr/bin/echo second\n");
	if (parse_unit(dir, "reset.service", unit, &cfg) != 0)
		fail("An explicit ExecStart reset should parse");
	if (cfg.exec_argc != 2 || cfg.exec_start_no_setuid ||
	    strcmp(cfg.exec_argv[0], "/usr/bin/true") ||
	    strcmp(cfg.exec_argv[1], "kept"))
		fail("ExecStart reset retained earlier command state");
	free_config(&cfg);
}

/*
 * test_unit_continuations - reject logical lines the parser cannot assemble.
 * @dir: temporary directory for unit fixtures.
 *
 * Returns no value; fails on accepted continuations or rejected comments.
 */
static void test_unit_continuations(const char *dir)
{
	service_config_t cfg;
	const char comments[] =
		"[Unit]\n# ignored \\\nDescription=ignored\n"
		"[Service]\n; ignored \\\nExecStart=/usr/bin/true\n";

	expect_parse_failure(dir, "continued-command.service",
		"[Service]\nExecStart=/usr/bin/echo first \\\n second\n");
	expect_parse_failure(dir, "continued-comment.service",
		"[Service]\nExecStart=/usr/bin/echo first \\\n"
		"# comment\n second\n");
	expect_parse_failure(dir, "continued-section.service",
		"[Unit]\nDescription=continued \\\n"
		"[Service]\nExecStart=/usr/bin/true\n");
	if (parse_unit(dir, "comments.service", comments, &cfg) != 0)
		fail("Backslashes in comment lines should be ignored");
	if (cfg.exec_argc != 1 || strcmp(cfg.exec_argv[0], "/usr/bin/true"))
		fail("Comments changed the parsed command");
	free_config(&cfg);
}

/*
 * test_exec_start_escapes - keep literal escapes and reject unhandled C escapes.
 * @dir: temporary directory for unit fixtures.
 *
 * Returns no value; fails if unsupported syntax changes argv silently or
 * supported quoting loses empty arguments, backslashes, or quote characters.
 */
static void test_exec_start_escapes(const char *dir)
{
	static const char *bad_units[] = {
		"[Service]\nExecStart=/usr/bin/echo \"\\x41\"\n",
		"[Service]\nExecStart=/usr/bin/echo '\\x41'\n",
		"[Service]\nExecStart=/usr/bin/echo \\x41\n",
		"[Service]\nExecStart=/usr/bin/echo \"\\s\"\n",
		"[Service]\nExecStart=/usr/bin/echo \"\\q\"\n",
	};
	const char unit[] =
		"[Service]\nExecStart=/usr/bin/echo \"two words\" '' "
		"\"a\\\\b\" \"a\\\"b\" 'a\\'b'\n";
	static const char *expected[] = {
		"/usr/bin/echo", "two words", "", "a\\b", "a\"b", "a'b",
	};
	service_config_t cfg;
	size_t i;

	for (i = 0; i < sizeof(bad_units) / sizeof(bad_units[0]); i++)
		expect_parse_failure(dir, "escape.service", bad_units[i]);
	if (parse_unit(dir, "literal-escapes.service", unit, &cfg) != 0)
		fail("Literal escapes and quoted words should parse");
	if (cfg.exec_argc != sizeof(expected) / sizeof(expected[0]))
		fail("Quoting changed the argument count");
	for (i = 0; i < cfg.exec_argc; i++) {
		if (strcmp(cfg.exec_argv[i], expected[i]))
			fail("Literal escape or quoted argument changed");
	}
	free_config(&cfg);
}

int main(void)
{
	char dir[] = "/tmp/libcap-ng-service-XXXXXX";

	if (mkdtemp(dir) == NULL)
		fail("Failed to create temporary directory");

	test_dynamic_user(dir);
	test_unresolved_dynamic_user(dir);
	test_valid_credentials(dir);
	test_invalid_credentials(dir);
	test_sink_validation();
	test_exec_start_privilege_prefixes(dir);
	test_exec_start_environment_words(dir);
	test_service_ambient(dir);
	test_capability_lists(dir);
	test_exec_start_reset(dir);
	test_unit_continuations(dir);
	test_exec_start_escapes(dir);

	rmdir(dir);
	puts("cap-audit service credential tests passed");
	return 0;
}
