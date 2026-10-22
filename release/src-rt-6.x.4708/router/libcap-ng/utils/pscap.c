/*
 * pscap.c - A program that lists running processes with capabilities
 * Copyright (c) 2009,2012,2019-20,2023-24,2026 Red Hat Inc.
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
#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "cap-ng.h"
#include "proc-account.h"
#include "proc-output.h"
#include "proc-sanitize.h"
#include "proc-status.h"

#define CMD_LEN 16
#define ACCOUNT_LEN 32
#define USERNS_MARK_LEN 3	// two characters plus '\0'.

static void usage(void)
{
	fprintf(stderr, "usage: pscap [-a] [-p pid] [--tree]\n");
	exit(1);
}

struct proc_info {
	pid_t pid;
	pid_t ppid;
	char *cmd;
	char account[ACCOUNT_LEN];
	char *caps_text;
};

static int append_marker(char **text, const char *marker)
	__attr_access ((__read_write__, 1))
	__attr_access ((__read_only__, 2))
	__wur;
static char *format_caps(int caps, bool ambient, bool bounds)
	__attribute_malloc__
	__attr_dealloc_free
	__wur;

/*
 * read_euid - read one process effective UID from procfs.
 * @pid: process ID whose status file should be inspected.
 *
 * Returns the effective UID on success, or (uid_t)-1 when it cannot be read.
 */
static uid_t read_euid(int pid)
{
	struct proc_status status;

	if (proc_read_status(pid, &status) < 0 || !status.seen_euid)
		return (uid_t)-1;
	return status.euid;
}

/*
 * get_account_name - format the account name for a process effective UID.
 * @pid: process ID whose effective UID should be read.
 * @account: destination buffer receiving the account label.
 * @account_len: size of @account in bytes.
 *
 * Returns no value.
 */
static void get_account_name(int pid, char *account, size_t account_len)
{
	uid_t euid = read_euid(pid);

	proc_format_account_name_from_euid(euid, account, account_len);
}

/*
 * compare_pid - order processes by pid for sorting/bsearch
 * @a: pointer to left struct proc_info
 * @b: pointer to right struct proc_info
 *
 * Returns -1, 0, or 1 for ordering.
 */
static int compare_pid(const void *a, const void *b)
{
	const struct proc_info *left = a;
	const struct proc_info *right = b;

	if (left->pid < right->pid)
		return -1;
	if (left->pid > right->pid)
		return 1;
	return 0;
}

/*
 * find_proc - locate a process record by pid
 * @procs: process array sorted by pid
 * @count: number of entries in @procs
 * @pid: process id to locate
 *
 * Returns pointer to the matching entry or NULL if not found.
 */
static void *find_proc(struct proc_info *procs, size_t count, pid_t pid)
{
	struct proc_info key;

	key.pid = pid;
	return bsearch(&key, procs, count, sizeof(*procs), compare_pid);
}

/*
 * append_marker - append a marker string to the capability text
 * @text: capability text buffer pointer to append to
 * @marker: marker string to append (e.g. " @" or " +")
 *
 * Returns 0 on success, -1 on allocation failure.
 */
static int append_marker(char **text, const char *marker)
{
	size_t len = strlen(*text);
	size_t marker_len = strlen(marker);
	char *tmp = realloc(*text, len + marker_len + 1);

	if (!tmp)
		return -1;
	memcpy(tmp + len, marker, marker_len + 1);
	*text = tmp;
	return 0;
}

/*
 * format_caps - format capability text with optional markers
 * @caps: capability summary state from capng_have_capabilities()
 * @ambient: true if ambient capabilities are present
 * @bounds: true if bounding set differs from full
 *
 * Returns allocated capability string or NULL on allocation failure.
 */
static char *format_caps(int caps, bool ambient, bool bounds)
{
	char *text;

	if (caps == CAPNG_PARTIAL)
		text = capng_print_caps_text(CAPNG_PRINT_BUFFER,
					     CAPNG_PERMITTED);
	else if (caps == CAPNG_FULL)
		text = strdup("full");
	else
		text = strdup("none");

	if (!text)
		return NULL;
	if (ambient && append_marker(&text, " @") < 0) {
		free(text);
		return NULL;
	}
	if (bounds && append_marker(&text, " +") < 0) {
		free(text);
		return NULL;
	}
	return text;
}

struct proc_tree {
	struct proc_info *procs;
	size_t count;
	int width;
};

/* A NULL prefix identifies a root; descendants share netcap's renderer. */
static void print_tree_node(const struct proc_tree *tree,
			    const struct proc_info *proc, const char *prefix,
			    bool is_last)
{
	size_t child_total = 0;
	size_t child_seen = 0;
	size_t i;
	char *text;
	char *cont_prefix;
	size_t prefix_len = (prefix ? strlen(prefix) : 0) + sizeof("│  ");

	cont_prefix = malloc(prefix_len);
	if (!cont_prefix)
		return;
	proc_tree_build_child_prefix(cont_prefix, prefix_len, prefix, is_last);

	/* Escaping a process name can expand it beyond a fixed-size label. */
	if (asprintf(&text, "%s(%d:%s) [%s]", proc->cmd, proc->pid,
		     proc->account, proc->caps_text) < 0) {
		free(cont_prefix);
		return;
	}
	proc_tree_print_node(prefix, is_last, text, tree->width);
	free(text);

	for (i = 0; i < tree->count; i++) {
		if (tree->procs[i].ppid == proc->pid)
			child_total++;
	}

	for (i = 0; i < tree->count; i++) {
		if (tree->procs[i].ppid != proc->pid)
			continue;
		child_seen++;
		print_tree_node(tree, &tree->procs[i], cont_prefix,
				child_seen == child_total);
	}

	free(cont_prefix);
}

/*
 * print_tree - render all process trees in pid order
 * @procs: process array
 * @count: number of entries in @procs
 *
 * Returns nothing. Each tree starts at a pid whose parent isn't present.
 */
static void print_tree(struct proc_info *procs, size_t count)
{
	size_t i;
	struct proc_tree tree = { procs, count, proc_output_width() };

	if (count > 1)
		qsort(procs, count, sizeof(*procs), compare_pid);
	for (i = 0; i < count; i++) {
		if (!find_proc(procs, count, procs[i].ppid))
			print_tree_node(&tree, &procs[i], NULL, true);
	}
}

/*
 * Precise recursive checks for parent-child relation between namespaces 
 * using ioctl() were avoided, because there didn't seem to be any case when
 * we may dereference the namespace symlink in /proc/PID/ns for processes in
 * user namespaces other than the current or child ones. Thus, the check just
 * tries to dereference the link and checks that it does not point to the
 * current NS.
 */
static bool in_child_userns(int pid)
{
	char ns_file_path[32];
	struct stat statbuf;
	ino_t own_ns_inode;
	dev_t own_ns_dev;

	if (stat("/proc/self/ns/user", &statbuf) < 0)
		return false;

	own_ns_inode = statbuf.st_ino;
	own_ns_dev = statbuf.st_dev;

	snprintf(ns_file_path, sizeof(ns_file_path), "/proc/%d/ns/user", pid);
	if (stat(ns_file_path, &statbuf) < 0)
		return false;

	return statbuf.st_ino != own_ns_inode || statbuf.st_dev != own_ns_dev;
}

int main(int argc, char *argv[])
{
	char *endptr = NULL;
	DIR *d;
	struct dirent *ent;
	int header = 0, show_all = 0, caps;
	pid_t our_pid = getpid();
	pid_t target_pid = 0;
	uid_t last_uid = (uid_t)-1;
	const char *name = NULL;
	int tree_mode = 0;
	struct proc_info *procs = NULL;
	size_t proc_count = 0;
	size_t proc_capacity = 0;
	size_t i;

	for (i = 1; i < (size_t)argc; i++) {
		if (strcmp(argv[i], "-a") == 0) {
			show_all = 1;
			continue;
		}
		if (strcmp(argv[i], "--tree") == 0) {
			tree_mode = 1;
			continue;
		}
		if (strcmp(argv[i], "-p") == 0) {
			long parsed_pid;

			if (i + 1 >= (size_t)argc)
				usage();
			errno = 0;
			parsed_pid = strtol(argv[++i], &endptr, 10);
			if (errno) {
				fprintf(stderr, "Can't read pid: %s\n",
					argv[i]);
				return 1;
			}
			if ((endptr == argv[i]) || (*endptr != '\0')
			    || parsed_pid <= 0 || parsed_pid > INT_MAX) {
				fprintf(stderr, "Invalid pid argument: %s\n",
					argv[i]);
				return 1;
			}
			target_pid = (pid_t)parsed_pid;
			if (target_pid == 1)
				show_all = 1;
			continue;
		}
		usage();
	}

	d = opendir("/proc");
	if (d == NULL) {
		fprintf(stderr, "Can't open /proc: %s\n", strerror(errno));
		return 1;
	}
	while (( ent = readdir(d) )) {
		int pid, ppid;
		char buf[100];
		char *safe_cmd = NULL;
		char *tmp, cmd[CMD_LEN + USERNS_MARK_LEN], state;
		int fd, len;

		// Skip non-process dir entries
		if(*ent->d_name<'0' || *ent->d_name>'9')
			continue;
		errno = 0;
		pid = strtol(ent->d_name, NULL, 10);
		if (errno)
			continue;

		if (target_pid && (pid != target_pid))
			continue;

		/* Skip our pid so we aren't listed */
		if (pid == our_pid)
			continue;

		// Parse up the stat file for the proc
		snprintf(buf, sizeof(buf), "/proc/%d/stat", pid);
		fd = open(buf, O_RDONLY|O_CLOEXEC, 0);
		if (fd < 0)
			continue;
		len = read(fd, buf, sizeof(buf) - 1);
		close(fd);
		if (len < 40)
			continue;
		buf[len] = 0;
		tmp = strrchr(buf, ')');
		if (tmp)
			*tmp = 0;
		else
			continue;
		memset(cmd, 0, sizeof(cmd));
		sscanf(buf, "%d (%15c", &ppid, cmd); // ppid is throwaway
		sscanf(tmp+2, "%c %d", &state, &ppid);

		// Skip kthreads
		if (pid == 2 || ppid == 2)
			continue;

		// now get the capabilities
		capng_clear(CAPNG_SELECT_ALL);
		capng_setpid(pid);
		if (capng_get_caps_process())
			continue;

		// And print out anything with capabilities
		caps = capng_have_capabilities(CAPNG_SELECT_CAPS);
		safe_cmd = sanitize_untrusted_field(cmd);
		if (!safe_cmd)
			continue;
		if (in_child_userns(pid)) {
			char *marked = malloc(strlen(safe_cmd) + 3);

			if (!marked) {
				free(safe_cmd);
				continue;
			}
			snprintf(marked, strlen(safe_cmd) + 3, "%s *", safe_cmd);
			free(safe_cmd);
			safe_cmd = marked;
		}
		if (tree_mode) {
			char *caps_text;
			bool has_ambient;
			bool has_bounds;

			if (!show_all && caps <= CAPNG_NONE) {
				free(safe_cmd);
				continue;
			}

			has_ambient = capng_have_capabilities(
					CAPNG_SELECT_AMBIENT) > CAPNG_NONE;
			has_bounds = capng_have_capabilities(
					CAPNG_SELECT_BOUNDS) > CAPNG_NONE;

			caps_text = format_caps(caps, has_ambient, has_bounds);
			if (!caps_text) {
				free(safe_cmd);
				continue;
			}

			if (proc_count == proc_capacity) {
				size_t new_capacity = proc_capacity ?
					proc_capacity * 2 : 256;
				struct proc_info *pi_tmp;

				pi_tmp = realloc(procs, new_capacity *
					      sizeof(*procs));
				if (!pi_tmp) {
					free(caps_text);
					free(safe_cmd);
					continue;
				}
				procs = pi_tmp;
				proc_capacity = new_capacity;
			}

			procs[proc_count].pid = pid;
			procs[proc_count].ppid = ppid;
			procs[proc_count].cmd = safe_cmd;
			get_account_name(pid, procs[proc_count].account,
					 sizeof(procs[proc_count].account));
			procs[proc_count].caps_text = caps_text;
			proc_count++;
			safe_cmd = NULL;
		} else if (caps > CAPNG_NONE) {
			char *caps_text;
			bool has_ambient;
			bool has_bounds;
			uid_t euid = read_euid(pid);

			if (header == 0) {
				printf("%-7s %-7s %-16s %-15s %s\n",
				    "ppid", "pid", "uid", "command",
				    "capabilities");
				header = 1;
			}
			proc_update_account_cache(euid, &last_uid, &name);
			has_ambient = capng_have_capabilities(
					CAPNG_SELECT_AMBIENT) > CAPNG_NONE;
			has_bounds = capng_have_capabilities(
					CAPNG_SELECT_BOUNDS) > CAPNG_NONE;
			caps_text = format_caps(caps, has_ambient, has_bounds);
			if (!caps_text) {
				free(safe_cmd);
				continue;
			}
			if (name) {
				printf("%-7d %-7d %-16s %-15s ", ppid, pid,
					name, safe_cmd);
			} else
				printf("%-7d %-7d %-16u %-15s ", ppid, pid,
					(unsigned int)last_uid, safe_cmd);
			printf("%s\n", caps_text);
			free(caps_text);
			free(safe_cmd);
			safe_cmd = NULL;
		}
		if (safe_cmd)
			free(safe_cmd);
	}
	closedir(d);
	if (tree_mode) {
		print_tree(procs, proc_count);
		for (i = 0; i < proc_count; i++) {
			free(procs[i].cmd);
			free(procs[i].caps_text);
		}
		free(procs);
	}
	return 0;
}
