/*
 * filecap-path.c - PATH parsing helpers for filecap
 * Copyright (c) 2026 Red Hat Inc.
 * All Rights Reserved.
 *
 * This software may be freely redistributed and/or modified under the
 * terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * filecap scans PATH when invoked without an explicit file or directory.
 * Empty PATH components are significant because the shell interprets them
 * as the current working directory. This helper exists so filecap keeps
 * those entries instead of dropping them the way strtok() would, and so
 * the edge cases can be tested under make check without walking a real
 * filesystem tree.
 */

#include "config.h"
#include <stdlib.h>
#include <string.h>
#include "filecap-path.h"

int filecap_foreach_path(const char *path_env, filecap_path_cb cb, void *data)
{
	const char *start;

	if (path_env == NULL)
		return 0;
	if (cb == NULL)
		return -1;

	start = path_env;
	while (start) {
		const char *end = strchr(start, ':');
		size_t len = end ? (size_t)(end - start) : strlen(start);
		char *entry = NULL;
		const char *path = ".";
		int rc;

		if (len != 0) {
			entry = strndup(start, len);
			if (entry == NULL)
				return -1;
			path = entry;
		}
		rc = cb(path, data);
		free(entry);
		if (rc)
			return rc;

		if (!end)
			break;
		start = end + 1;
	}

	return 0;
}
