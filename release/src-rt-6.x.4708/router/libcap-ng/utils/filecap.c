/*
 * filecap.c - A program that lists running processes with capabilities
 * Copyright (c) 2009-10,2012,2017-20,2026 Red Hat Inc.
 * All Rights Reserved.
 *
 * This software may be freely redistributed and/or modified under the
 * terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor
 * Boston, MA 02110-1335, USA.
 *
 * Authors:
 *   Steve Grubb <sgrubb@redhat.com>
 */

#include "config.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "cap-ng.h"
#include "filecap-path.h"
#include <fcntl.h>
#include <ftw.h>

#ifndef FTW_CONTINUE
#define FTW_CONTINUE 0
#endif


static int show_all = 0, header = 0, capabilities = 0, cremove = 0;
static int single_file = 0;

static int check_file(const char *fpath, const struct stat *sb,
		      int typeflag_unused, struct FTW *s_unused)
	__attr_access ((__read_only__, 1))
	__attr_access ((__read_only__, 2));
static int scan_path_entry(const char *entry, void *data)
	__attr_access ((__read_only__, 1))
	__attr_access ((__read_write__, 2));
static int scan_path_env(const char *path_env, int nftw_flags)
	__attr_access ((__read_only__, 1))
	__wur;

static void usage(void)
{
	fprintf(stderr,
	    "usage: filecap [-a | -d | /dir | /dir/file [cap1 cap2 ...] ]\n");
	exit(1);
}

// Returns 1 on error and 0 otherwise
static int check_file(const char *fpath,
		const struct stat *sb,
		int typeflag_unused __attribute__ ((unused)),
		struct FTW *s_unused __attribute__ ((unused)))
{
	int ret = FTW_CONTINUE;

	if (S_ISREG(sb->st_mode) == 0)
		return ret;

	int fd = open(fpath, O_RDONLY|O_CLOEXEC);
	if (fd >= 0) {
		capng_results_t rc;
		int permitted = 0;

		capng_clear(CAPNG_SELECT_BOTH);
		if (capng_get_caps_fd(fd) < 0 && errno != ENODATA) {
			fprintf(stderr,
				"Unable to get capabilities of %s: %s\n",
				fpath, strerror(errno));
			if (single_file)
				ret = 1;
		}
		rc = capng_have_capabilities(CAPNG_SELECT_CAPS);
		if (rc == CAPNG_NONE) {
			permitted = 1;
			rc = capng_have_permitted_capabilities();
		}
		if (rc > CAPNG_NONE) {
			if (header == 0) {
				header = 1;
				printf("%-9s %-22s capabilities  rootid\n",
				       "set", "file");
			}

			uid_t rootid = capng_get_rootid();
			printf("%s %-22s ",
			       permitted ? "permitted" : "effective",  fpath);

			if (rc == CAPNG_FULL)
				printf("full");
			else
				capng_print_caps_text(CAPNG_PRINT_STDOUT,
						CAPNG_PERMITTED);

			if (rootid != CAPNG_UNSET_ROOTID)
				printf(" %ju", (uintmax_t)rootid);
			printf("\n");
		}
		close(fd);
	} // Ignore open failure since printing a warning clutters output
	  // There's a lot of non-root unreadble files in /usr
	return ret;
}

/*
 * scan_path_env - walk each PATH element while preserving empty entries.
 *
 * Empty PATH components mean the current working directory. strtok() drops
 * them, which makes the default filecap scan disagree with normal PATH
 * lookup semantics.
 */
struct path_scan_ctx {
	int nftw_flags;
};

static int scan_path_entry(const char *entry, void *data)
{
	struct path_scan_ctx *ctx = data;

	nftw(entry, check_file, 1024, ctx->nftw_flags);
	return 0;
}

static int scan_path_env(const char *path_env, int nftw_flags)
{
	struct path_scan_ctx ctx;

	ctx.nftw_flags = nftw_flags;
	return filecap_foreach_path(path_env, scan_path_entry, &ctx);
}


// Use cases:
//  filecap
//  filecap -a
//  filecap /path/dir
//  filecap /path/file
//  filecap /path/file capability1 capability2 capability 3 ...
//
int main(int argc, char *argv[])
{
#if CAP_LAST_CAP < 31 || !defined (VFS_CAP_U32) || \
	(!defined (HAVE_ATTR_XATTR_H) && !defined(HAVE_SYS_XATTR_H))
	fprintf(stderr, "File based capabilities are not supported\n");
#else
	char *path_env, *path = NULL, *dir = NULL;
	struct stat sbuf;
	struct stat path_sbuf;
	int nftw_flags = FTW_PHYS;
	int i, rc = 0;
	int have_path_stat = 0;

	if (argc >1) {
		for (i=1; i<argc; i++) {
			if (strcmp(argv[i], "-a") == 0) {
				show_all = 1;
				if (argc != 2)
					usage();
			} else if (strcmp(argv[i], "-d") == 0) {
				int j;
				for (j=0; j<=CAP_LAST_CAP; j++) {
					const char *n =
						capng_capability_to_name(j);
					if (n == NULL)
						n = "unknown";
					printf("%s\n", n);
				}
				return 0;
			} else if (argv[i][0] == '/') {
				if (lstat(argv[i], &sbuf) != 0) {
					fprintf(stderr,
						"Error checking path %s (%s)\n",
						argv[i], strerror(errno));
					exit(1);
				}
				// Clear all capabilities in case cap strings
				// follow. If we get a second file we err out
				// so this is safe
				if (S_ISREG(sbuf.st_mode) && path == NULL &&
								 dir == NULL) {
					path = argv[i];
					path_sbuf = sbuf;
					have_path_stat = 1;
					capng_clear(CAPNG_SELECT_BOTH);
				} else if (S_ISDIR(sbuf.st_mode) && path == NULL
								&& dir == NULL)
					dir = argv[i];
				else {
					fprintf(stderr,
						"Must be one regular file or "
						"directory\n");
					exit(1);
				}
			} else {
				int cap = capng_name_to_capability(argv[i]);
				if (cap >= 0) {
					if (path == NULL)
						usage();
					capng_update(CAPNG_ADD,
						CAPNG_PERMITTED|CAPNG_EFFECTIVE,
						cap);
					capabilities = 1;
				} else if (strcmp("none", argv[i]) == 0) {
					capng_clear(CAPNG_SELECT_BOTH);
					capabilities = 1;
					cremove = 1;
				} else {
					fprintf(stderr,
						"Unrecognized capability.\n");
					usage();
				}
			}
		}
	}
	if (path == NULL && dir == NULL && show_all == 0) {
		path_env = getenv("PATH");
		if (path_env != NULL) {
			if (scan_path_env(path_env, nftw_flags) < 0)
				return 1;
		}
	} else if (path == NULL && dir == NULL && show_all == 1) {
		// Find files
		nftw("/", check_file, 1024, nftw_flags);
	} else if (dir) {
		// Print out the dir
		nftw(dir, check_file, 1024, nftw_flags);
	}else if (path && capabilities == 0) {
		// Print out specific file
		single_file = 1;
		rc = check_file(path, &sbuf, 0, NULL);
	} else if (path && capabilities == 1) {
		// Write capabilities to file
		struct stat fbuf;
		int fd = open(path, O_WRONLY|O_NOFOLLOW|O_CLOEXEC);
		if (fd < 0) {
			fprintf(stderr,
				"Could not open %s for writing (%s)\n", path,
				strerror(errno));
			return 1;
		}
		if ((fstat(fd, &fbuf))) {
			fprintf(stderr, "Could not fstat %s (%s)\n",
					path, strerror(errno));
			close(fd);
			return 1;
		}
		if (have_path_stat &&
		    (fbuf.st_dev != path_sbuf.st_dev ||
		     fbuf.st_ino != path_sbuf.st_ino)) {
			fprintf(stderr,
				"File changed during operation, refusing "
				"to continue\n");
			close(fd);
			return 1;
		}
		if (capng_apply_caps_fd(fd) < 0) {
			fprintf(stderr,
				"Could not set capabilities on %s: %s\n",
				path, strerror(errno));
			rc = 1;
		}
		close(fd);
	}
#endif
	return rc;
}
