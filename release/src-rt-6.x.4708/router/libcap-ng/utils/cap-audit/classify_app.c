// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (c) 2026 Red Hat Inc.
/*
 * classify_app.c - File type detection helper for cap-audit
 *
 * This helper was split out of cap_audit_util.c so the short-read corner
 * cases can be exercised by make check. The important rule is that file
 * classification may only inspect bytes that were actually returned by
 * read(); tiny files must not be classified from uninitialized stack data.
 */

#include "config.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "classify_app.h"

#define ELFMAGIC "\177ELF"

type_t classify_app(const char *exe)
{
	int fd;
	ssize_t rc;
	char buf[257];

	fd = open(exe, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Cannot open %s - %s\n", exe, strerror(errno));
		exit(1);
	}

	rc = read(fd, buf, 256);
	close(fd);
	if (rc > 0) {
		buf[rc] = 0;
		/*
		 * Only inspect signatures that fully fit in the bytes we read.
		 * Short reads happen on tiny files and should not consult
		 * uninitialized stack data past rc.
		 */
		if (rc >= 2 && buf[0] == '#' && buf[1] == '!') {
			char *ptr = strchr(buf, '\n');

			if (ptr)
				*ptr = 0;
			if (strstr(buf, "python"))
				return PYTHON;
		} else if (rc >= 4 && strncmp(buf, ELFMAGIC, 4) == 0)
			return ELF;
	}

	return UNSUPPORTED;
}
