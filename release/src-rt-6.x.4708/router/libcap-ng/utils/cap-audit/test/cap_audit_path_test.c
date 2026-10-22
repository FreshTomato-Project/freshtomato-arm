// SPDX-License-Identifier: GPL-2.0-or-later
/* cap_audit_path_test.c -- target PATH resolution regression tests
 * Copyright 2026 Red Hat Inc.
 * All Rights Reserved.
 */

#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "cap_audit_path.h"

static void fail(const char *msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}

static void make_executable(const char *path)
{
	int fd;
	const char script[] = "#!/bin/sh\nexit 0\n";
	ssize_t rc;

	fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0700);
	if (fd < 0)
		fail("Failed to create executable target");
	rc = write(fd, script, sizeof(script) - 1);
	close(fd);
	if (rc != (ssize_t)(sizeof(script) - 1))
		fail("Failed to write executable target");
}

int main(void)
{
	char root[] = "/tmp/libcap-ng-cap-audit-path-XXXXXX";
	char early[PATH_MAX];
	char late[PATH_MAX];
	char early_target[PATH_MAX];
	char late_target[PATH_MAX];
	char link_target[PATH_MAX];
	char path[PATH_MAX * 2 + 2];
	char *resolved;

	if (!mkdtemp(root))
		fail("Failed to create temporary directory");
	if (snprintf(early, sizeof(early), "%s/early", root) < 0 ||
	    snprintf(late, sizeof(late), "%s/late", root) < 0 ||
	    snprintf(early_target, sizeof(early_target), "%s/probe",
		     early) < 0 ||
	    snprintf(late_target, sizeof(late_target), "%s/probe", late) < 0 ||
	    snprintf(link_target, sizeof(link_target), "%s/link", root) < 0)
		fail("Failed to format target paths");
	if (mkdir(early, 0700) || mkdir(late, 0700) ||
	    mkdir(early_target, 0700))
		fail("Failed to create PATH directories");
	make_executable(late_target);

	if (snprintf(path, sizeof(path), "%s:%s", early, late) < 0 ||
	    setenv("PATH", path, 1))
		fail("Failed to set test PATH");
	resolved = resolve_target_command("probe");
	if (!resolved || strcmp(resolved, late_target))
		fail("Resolver did not skip a directory PATH candidate");
	free(resolved);

	resolved = resolve_target_command(early_target);
	if (resolved)
		fail("Resolver accepted a directory target");

	resolved = resolve_target_command(late_target);
	if (!resolved || strcmp(resolved, late_target))
		fail("Resolver rejected a regular executable");
	free(resolved);

	if (symlink(late_target, link_target))
		fail("Failed to create executable symlink");
	resolved = resolve_target_command(link_target);
	if (!resolved || strcmp(resolved, link_target))
		fail("Resolver rejected a symlink to an executable");
	free(resolved);
	if (unlink(link_target) || unlink(late_target) ||
	    rmdir(early_target) || rmdir(early) || rmdir(late) || rmdir(root))
		fail("Failed to remove temporary paths");

	puts("cap-audit target PATH tests passed");
	return 0;
}
