/*
 * pscap.c - A program that lists running processes with capabilities
 * Copyright (c) 2009,2012,2020 Red Hat Inc.
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
#include <stdio.h>
#include <stdio_ext.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include "cap-ng.h"

#define CMD_LEN 16
#define USERNS_MARK_LEN 3	// two characters plus '\0'.

static void usage(void)
{
	fprintf(stderr, "usage: pscap [-a] [-p pid] [--tree]\n");
	exit(1);
}

struct proc_info {
	pid_t pid;
	pid_t ppid;
	char cmd[CMD_LEN + USERNS_MARK_LEN];
	char *caps_text;
};

static int get_width(void)
{
	struct winsize ws;
	char *e;
	long c;

	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0)
		return ws.ws_col;

	e = getenv("COLUMNS");
	if (e) {
		char *endptr;

		errno = 0;
		c = strtol(e, &endptr, 10);
		if (errno == 0 && endptr != e && *endptr == '\0' && c > 0)
			return (int)c;
	}

	return 80;
}

static size_t wrap_to(const char *text, size_t max)
{
	size_t len = strlen(text);
	size_t i;

	if (len <= max)
		return len;

	for (i = max; i > 0; i--) {
		if (text[i - 1] == ',') {
			if (i < len && text[i] == ' ')
				return i + 1;
			return i;
		}
		if (text[i - 1] == ' ')
			return i;
	}

	return max;
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
	if (ambient)
		append_marker(&text, " @");
	if (bounds)
		append_marker(&text, " +");
	return text;
}

/*
 * print_tree_node - render a node and its children in tree mode
 * @procs: process array
 * @count: number of entries in @procs
 * @index: index of current node in @procs
 * @prefix: current line prefix
 * @is_last: true if this node is the last child of its parent
 * @is_root: true if this node is a tree root
 *
 * Returns nothing. Recurses through children to emit full subtree.
 */
static void print_tree_node(struct proc_info *procs, size_t count,
			    size_t index, const char *prefix, bool is_last,
			    bool is_root, int width)
{
	struct proc_info *proc = &procs[index];
	size_t child_total = 0;
	size_t child_seen = 0;
	size_t i;
	size_t prefix_len;
	size_t cont_len;
	size_t avail;
	size_t n;
	const char *caps;
	char head[64];
	const char *branch = "";
	char *line_prefix;
	char *cont_prefix;
	char *child_prefix;

	if (!is_root)
		branch = is_last ? "   " : "│  ";

	line_prefix = malloc(strlen(prefix) + strlen(is_root ? "" :
					      (is_last ? "└─ " : "├─ ")) + 1);
	if (!line_prefix)
		return;
	strcpy(line_prefix, prefix);
	if (!is_root)
		strcat(line_prefix, is_last ? "└─ " : "├─ ");

	cont_prefix = malloc(strlen(prefix) + strlen(branch) + 1);
	if (!cont_prefix) {
		free(line_prefix);
		return;
	}
	strcpy(cont_prefix, prefix);
	strcat(cont_prefix, branch);

	snprintf(head, sizeof(head), "%s(%d) [", proc->cmd, proc->pid);
	prefix_len = strlen(line_prefix);
	cont_len = strlen(cont_prefix);
	caps = proc->caps_text;

	if ((int)(prefix_len + strlen(head) + strlen(caps) + 1) <= width) {
		printf("%s%s%s]\n", line_prefix, head, caps);
		goto children;
	}

	avail = width > (int)(prefix_len + strlen(head)) ?
		(size_t)(width - (int)(prefix_len + strlen(head))) : 10;
	if (avail < 10)
		avail = 10;
	n = wrap_to(caps, avail);
	printf("%s%s%.*s\n", line_prefix, head, (int)n, caps);
	caps += n;

	while (*caps) {
		avail = width > (int)cont_len ? (size_t)(width - (int)cont_len) : 10;
		if (avail < 10)
			avail = 10;
		if (strlen(caps) + 1 <= avail) {
			printf("%s%s]\n", cont_prefix, caps);
			break;
		}
		n = wrap_to(caps, avail);
		printf("%s%.*s\n", cont_prefix, (int)n, caps);
		caps += n;
	}

children:
	free(line_prefix);
	free(cont_prefix);

	for (i = 0; i < count; i++) {
		if (procs[i].ppid == proc->pid)
			child_total++;
	}

	if (child_total == 0)
		return;

	child_prefix = malloc(strlen(prefix) + strlen(branch) + 1);
	if (!child_prefix)
		return;
	strcpy(child_prefix, prefix);
	strcat(child_prefix, branch);

	for (i = 0; i < count; i++) {
		if (procs[i].ppid != proc->pid)
			continue;
		child_seen++;
		print_tree_node(procs, count, i, child_prefix,
				child_seen == child_total, false, width);
	}

	free(child_prefix);
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
	int width = get_width();

	qsort(procs, count, sizeof(*procs), compare_pid);
	for (i = 0; i < count; i++) {
		if (!find_proc(procs, count, procs[i].ppid))
			print_tree_node(procs, count, i, "", true, true, width);
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
	int uid = -1;
	char *name = NULL;
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
			if (i + 1 >= (size_t)argc)
				usage();
			errno = 0;
			target_pid = strtol(argv[++i], &endptr, 10);
			if (errno) {
				fprintf(stderr, "Can't read pid: %s\n",
					argv[i]);
				return 1;
			}
			if ((endptr == argv[i]) || (*endptr != '\0')
			    || !target_pid) {
				fprintf(stderr, "Invalid pid argument: %s\n",
					argv[i]);
				return 1;
			}
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
		int pid, ppid, euid = -1;
		char buf[100];
		char *tmp, cmd[CMD_LEN + USERNS_MARK_LEN], state;
		int fd, len;
		struct passwd *p;

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
		if (in_child_userns(pid))
			strcat(cmd, " *");
		if (tree_mode) {
			char *caps_text;
			bool has_ambient;
			bool has_bounds;

			if (!show_all && caps <= CAPNG_NONE)
				continue;

			has_ambient = capng_have_capabilities(
					CAPNG_SELECT_AMBIENT) > CAPNG_NONE;
			has_bounds = capng_have_capabilities(
					CAPNG_SELECT_BOUNDS) > CAPNG_NONE;

			caps_text = format_caps(caps, has_ambient, has_bounds);
			if (!caps_text)
				continue;

			if (proc_count == proc_capacity) {
				size_t new_capacity = proc_capacity ?
					proc_capacity * 2 : 256;
				struct proc_info *pi_tmp;

				pi_tmp = realloc(procs, new_capacity *
					      sizeof(*procs));
				if (!pi_tmp) {
					free(caps_text);
					continue;
				}
				procs = pi_tmp;
				proc_capacity = new_capacity;
			}

			procs[proc_count].pid = pid;
			procs[proc_count].ppid = ppid;
			strncpy(procs[proc_count].cmd, cmd,
				sizeof(procs[proc_count].cmd) - 1);
			procs[proc_count].cmd[
				sizeof(procs[proc_count].cmd) - 1] = '\0';
			procs[proc_count].caps_text = caps_text;
			proc_count++;
		} else if (caps > CAPNG_NONE) {
			// Get the effective uid
			FILE *f;
			int line;
			snprintf(buf, sizeof(buf), "/proc/%d/status", pid);
			f = fopen(buf, "rte");
			if (f == NULL)
				euid = 0;
			else {
				line = 0;
				__fsetlocking(f, FSETLOCKING_BYCALLER);
				while (fgets(buf, sizeof(buf), f)) {
					if (line == 0) {
						line++;
						continue;
					}
					if (memcmp(buf, "Uid:", 4) == 0) {
						int id;
						sscanf(buf, "Uid: %d %d",
							&id, &euid);
						break;
					}
				}
				fclose(f);
			}

			if (header == 0) {
				printf("%-7s %-7s %-16s %-15s %s\n",
				    "ppid", "pid", "uid", "command",
				    "capabilities");
				header = 1;
			}
			if (euid == 0) {
				// Take short cut for this one
				name = "root";
				uid = 0;
			} else if (euid != uid) {
				// Only look up if name changed
				p = getpwuid(euid);
				uid = euid;
				if (p)
					name = p->pw_name;
				// If not taking this branch, use last val
			}

			if (name) {
				printf("%-7d %-7d %-16s %-15s ", ppid, pid,
					name, cmd);
			} else
				printf("%-7d %-7d %-16d %-15s ", ppid, pid,
					uid, cmd);
			if (caps == CAPNG_PARTIAL) {
				capng_print_caps_text(CAPNG_PRINT_STDOUT,
							CAPNG_PERMITTED);
				if (capng_have_capabilities(
					    CAPNG_SELECT_AMBIENT) > CAPNG_NONE)
					printf(" @");
				if (capng_have_capabilities(CAPNG_SELECT_BOUNDS)
							 > CAPNG_NONE)
					printf(" +");
				printf("\n");
			} else {
				printf("full");
				if (capng_have_capabilities(
					    CAPNG_SELECT_AMBIENT) > CAPNG_NONE)
					printf(" @");
				if (capng_have_capabilities(CAPNG_SELECT_BOUNDS)
							 > CAPNG_NONE)
					printf(" +");
				printf("\n");
			}
		}
	}
	closedir(d);
	if (tree_mode) {
		print_tree(procs, proc_count);
		for (i = 0; i < proc_count; i++)
			free(procs[i].caps_text);
		free(procs);
	}
	return 0;
}
