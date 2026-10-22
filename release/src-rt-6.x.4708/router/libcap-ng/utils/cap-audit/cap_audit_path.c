// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * cap-audit - Target command path resolution.
 * Copyright (c) 2026 Red Hat Inc.
 * All Rights Reserved.
 */

#include "cap_audit_path.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static char *make_path_candidate(const char *dir, size_t dir_len,
				 const char *target, size_t target_len)
{
	char candidate[PATH_MAX];

	if (!dir_len) {
		/* Empty PATH components refer to the current directory. */
		if (target_len + 3 > sizeof(candidate))
			return NULL;
		candidate[0] = '.';
		candidate[1] = '/';
		memcpy(candidate + 2, target, target_len + 1);
		return strdup(candidate);
	}

	if (dir_len + target_len + 2 > sizeof(candidate))
		return NULL;

	memcpy(candidate, dir, dir_len);
	candidate[dir_len] = '/';
	memcpy(candidate + dir_len + 1, target, target_len + 1);

	return strdup(candidate);
}

static int is_executable_file(const char *path)
{
	struct stat st;

	/* This preflight is advisory; execvp() makes the final decision. */
	return stat(path, &st) == 0 && S_ISREG(st.st_mode) &&
	       access(path, X_OK) == 0;
}

char *resolve_target_command(const char *target)
{
	const char *path;
	const char *dir;
	size_t target_len;

	if (!target || !target[0])
		return NULL;

	if (strchr(target, '/'))
		return is_executable_file(target) ? strdup(target) : NULL;

	target_len = strlen(target);
	path = getenv("PATH");
	if (!path || !path[0])
		return NULL;

	for (dir = path; dir;) {
		char *candidate;
		const char *next = strchr(dir, ':');
		size_t dir_len = next ? (size_t)(next - dir) : strlen(dir);

		candidate = make_path_candidate(dir, dir_len, target,
						target_len);
		if (candidate) {
			if (is_executable_file(candidate))
				return candidate;
			free(candidate);
		}

		dir = next ? next + 1 : NULL;
	}

	return NULL;
}
