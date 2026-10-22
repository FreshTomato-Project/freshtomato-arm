// SPDX-License-Identifier: GPL-2.0-or-later
/* cap_audit_util_test.c -- cap-audit helper regression tests
 * Copyright 2026 Red Hat Inc.
 * All Rights Reserved.
 *
 * These checks pin down classify_app() behavior for very small files.
 * The historical failure mode was reading a short file and then comparing
 * shebang or ELF signatures past the bytes returned by read().
 */

#include "config.h"
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include "classify_app.h"

static void fail(const char *msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}

static void write_file(const char *path, const char *data, size_t len)
{
	int fd;
	ssize_t rc;

	fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0700);
	if (fd < 0)
		fail("Failed to open temporary file");
	rc = write(fd, data, len);
	close(fd);
	if (rc < 0 || (size_t)rc != len)
		fail("Failed to write temporary file");
}

int main(void)
{
	char dir[] = "/tmp/libcap-ng-cap-audit-XXXXXX";
	char path[PATH_MAX];

	if (mkdtemp(dir) == NULL)
		fail("Failed to create temporary directory");

	snprintf(path, sizeof(path), "%s/one-byte", dir);
	write_file(path, "#", 1);
	if (classify_app(path) != UNSUPPORTED)
		fail("One-byte file should be unsupported");

	snprintf(path, sizeof(path), "%s/short-shebang", dir);
	write_file(path, "#!", 2);
	if (classify_app(path) != UNSUPPORTED)
		fail("Short shebang should be unsupported");

	snprintf(path, sizeof(path), "%s/short-elf", dir);
	write_file(path, "\177EL", 3);
	if (classify_app(path) != UNSUPPORTED)
		fail("Short ELF magic should be unsupported");

	snprintf(path, sizeof(path), "%s/python", dir);
	write_file(path, "#!/usr/bin/python\n", 18);
	if (classify_app(path) != PYTHON)
		fail("Python shebang should be detected");

	snprintf(path, sizeof(path), "%s/elf", dir);
	write_file(path, "\177ELF", 4);
	if (classify_app(path) != ELF)
		fail("ELF magic should be detected");

	puts("cap-audit classify_app tests passed");
	return 0;
}
