/*
 * netcap.c - A program that lists network apps with capabilities
 * Copyright (c) 2009-10,2012,2020 Red Hat Inc.
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
 *
 * The /proc/net/tcp|udp|raw parsing code was borrowed from netstat.c
 */

#include "config.h"
#include <arpa/inet.h>
#ifdef HAVE_NETCAP_ADVANCED
#include <linux/inet_diag.h>
#include <linux/netlink.h>
#include <linux/sock_diag.h>
#include <linux/vm_sockets.h>
#ifdef HAVE_LINUX_VM_SOCKETS_DIAG_H
#include <linux/vm_sockets_diag.h>
#endif
#endif
#include <limits.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <pwd.h>
#include "cap-ng.h"
#include "proc-llist.h"
#include "netcap-advanced.h"
#include "proc-sanitize.h"

static llist l;
static int perm_warn = 0, header = 0, last_uid = -1;
static char *tacct = NULL;

static void usage(void)
{
	fprintf(stderr, "usage: netcap [--advanced "
		"[--interface IFACE] [--list-interfaces] [--json] "
		"[--no-color]]\n");
	exit(1);
}

static int collect_process_info(void)
{
	DIR *d, *f;
	struct dirent *ent;
	d = opendir("/proc");
	if (d == NULL) {
		fprintf(stderr, "Can't open /proc: %s\n", strerror(errno));
		return 1;
	}
	while (( ent = readdir(d) )) {
		FILE *sf;
		int pid, ppid;
		capng_results_t caps;
		char buf[100];
		char *safe_cmd;
		char *tmp, cmd[16], state;
		char *text = NULL, *bounds = NULL, *ambient = NULL;
		int fd, len, euid = -1;

		// Skip non-process dir entries
		if(*ent->d_name<'0' || *ent->d_name>'9')
			continue;
		errno = 0;
		pid = strtol(ent->d_name, NULL, 10);
		if (errno)
			continue;

		// Parse up the stat file for the proc
		snprintf(buf, sizeof(buf), "/proc/%d/stat", pid);
		fd = open(buf, O_RDONLY|O_CLOEXEC, 0);
		if (fd < 0)
			continue;
		len = read(fd, buf, sizeof buf - 1);
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
		if (sscanf(buf, "%d (%15c", &ppid, cmd) != 2)
			continue;
		if (sscanf(tmp+2, "%c %d", &state, &ppid) != 2)
			continue;

		// Skip kthreads
		if (pid == 2 || ppid == 2)
			continue;

		// now get the capabilities
		capng_clear(CAPNG_SELECT_ALL);
		capng_setpid(pid);
		if (capng_get_caps_process())
			continue;
		caps = capng_have_capabilities(CAPNG_SELECT_CAPS);
		if (caps <= CAPNG_NONE)
			continue;
		if (caps == CAPNG_FULL) {
			text = strdup("full");
			if (!text) {
				fprintf(stderr, "Out of memory\n");
				continue;
			}
		} else {
			text = capng_print_caps_text(CAPNG_PRINT_BUFFER,
					CAPNG_PERMITTED);
			if (text == NULL) {
				fprintf(stderr, "Out of memory doing pid %d\n",
					pid);
				continue;
			}
		}

		// Get the effective uid
		snprintf(buf, sizeof(buf), "/proc/%d/status", pid);
		sf = fopen(buf, "rte");
		if (sf == NULL)
			euid = 0;
		else {
			int line = 0;
			__fsetlocking(sf, FSETLOCKING_BYCALLER);
			while (fgets(buf, sizeof(buf), sf)) {
				if (line == 0) {
					line++;
					continue;
				}
				if (memcmp(buf, "Uid:", 4) == 0) {
					int id;
					if (sscanf(buf, "Uid: %d %d",
						&id, &euid) == 2)
						break;
				}
			}
			fclose(sf);
			if (euid == -1)
				euid = 0;
		}

		caps = capng_have_capabilities(CAPNG_SELECT_AMBIENT);
		if (caps > CAPNG_NONE)
			ambient = strdup("@");
		else
			ambient = strdup("");
		if (!ambient) {
			fprintf(stderr, "Out of memory\n");
			free(text);
			continue;
		}

		// Now record the bounding set information
		caps = capng_have_capabilities(CAPNG_SELECT_BOUNDS);
		if (caps > CAPNG_NONE)
			bounds = strdup("+");
		else
			bounds = strdup("");
		if (!bounds) {
			fprintf(stderr, "Out of memory\n");
			free(text);
			free(ambient);
			continue;
		}

		// Now lets get the inodes each process has open
		snprintf(buf, sizeof(buf), "/proc/%d/fd", pid);
		f = opendir(buf);
		if (f == NULL) {
			if (errno == EACCES) {
				if (perm_warn == 0) {
					fprintf(stderr,
						"You may need to be root to "
						"get a full report\n");
					perm_warn = 1;
				}
			} else
				fprintf(stderr, "Can't open %s: %s\n", buf,
					strerror(errno));
			free(text);
			free(bounds);
			free(ambient);
			continue;
		}
		// For each file in the fd dir...
		struct dirent *fd_ent;
		while (( fd_ent = readdir(f) )) {
			char line[256], ln[256], *s, *e;
			unsigned long inode;
			lnode node;
			int llen;

			if (fd_ent->d_name[0] == '.')
				continue;
			snprintf(ln, 256, "%s/%s", buf, fd_ent->d_name);
			if ((llen = readlink(ln, line, sizeof(line)-1)) < 0)
				continue;
			line[llen] = 0;

			// Only look at the socket entries
			if (memcmp(line, "socket:", 7) == 0) {
				// Type 1 sockets
				s = strchr(line+7, '[');
				if (s == NULL)
					continue;
				s++;
				e = strchr(s, ']');
				if (e == NULL)
					continue;
				*e = 0;
			} else if (memcmp(line, "[0000]:", 7) == 0) {
				// Type 2 sockets
				s = line + 8;
			} else
				continue;
			errno = 0;
			inode = strtoul(s, NULL, 10);
			if (errno)
				continue;
			safe_cmd = sanitize_untrusted_field(cmd);
			if (!safe_cmd)
				continue;
			node.ppid = ppid;
			node.pid = pid;
			node.uid = euid;
			node.cmd = safe_cmd;
			node.inode = inode;
			node.capabilities = strdup(text);
			node.bounds = strdup(bounds);
			node.ambient = strdup(ambient);
			if (node.cmd && node.capabilities && node.bounds &&
			    node.ambient)
				// We make one entry for each socket inode
				list_append(&l, &node);
			else {
				free(node.cmd);
				free(node.capabilities);
				free(node.bounds);
				free(node.ambient);
			}
		}
		closedir(f);
		free(text);
		free(bounds);
		free(ambient);
	}
	closedir(d);
	return 0;
}

static void report_finding(unsigned int port, const char *type, const char *ifc)
{
	struct passwd *p;
	lnode *n = list_get_cur(&l);

	// And print out anything with capabilities
	if (header == 0) {
		printf("%-7s %-7s %-16s %-15s %-8s %-15s %s\n",
			"ppid", "pid", "acct", "command", "type", "port",
			"capabilities");
		header = 1;
	}
	if (n->uid == 0) {
		// Take short cut for this one
		tacct = "root";
		last_uid = 0;
	} else if (last_uid != (int)n->uid) {
		// Only look up if name changed
		p = getpwuid(n->uid);
		last_uid = n->uid;
		if (p)
			tacct = p->pw_name;
		// If not taking this branch, use last val
	}
	if (tacct) {
		printf("%-7d %-7d %-16s", n->ppid, n->pid, tacct);
	} else
		printf("%-7d %-7d %-16d", n->ppid, n->pid, last_uid);
	printf(" %-15s %-8s", n->cmd, type);
	if (ifc)
		printf(" %-15s", ifc);
	else
		printf(" %-15u", port);
	printf(" %s %s%s\n", n->capabilities, n->ambient, n->bounds);
}

static void read_net(const char *proc, const char *type, int use_local_port)
{
	int line = 0;
	FILE *f;
	char buf[256];
	unsigned long rxq, txq, time_len, retr, inode;
	unsigned int local_port, rem_port, state, timer_run;
	int d, uid, timeout;
	char rem_addr[128], local_addr[128], more[512];

	f = fopen(proc, "rte");
	if (f == NULL) {
		if (errno != ENOENT)
			fprintf(stderr, "Can't open %s: %s\n",
				proc, strerror(errno));
		return;
	}
	__fsetlocking(f, FSETLOCKING_BYCALLER);
	while (fgets(buf, sizeof(buf), f)) {
		if (line == 0) {
			line++;
			continue;
		}
		more[0] = 0;
		if (sscanf(buf, "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X "
			"%lX:%lX %X:%lX %lX %d %d %lu %511s\n",
			&d, local_addr, &local_port, rem_addr, &rem_port,
			&state, &txq, &rxq, &timer_run, &time_len, &retr,
			&uid, &timeout, &inode, more) < 14)
			continue;
		if (list_find_inode(&l, inode))
			report_finding(use_local_port ? local_port : 0,
					type, NULL);
	}
	fclose(f);
}

// Caller must have buffer >= 65 bytes
static void get_interface(unsigned int iface, char *ifc)
{
	unsigned int line = 0;
	FILE *f;
	char buf[256], more[256];

	// Terminate the interface in case of error
	*ifc = 0;

	// Offset the interface number since header is 2 lines long
	iface += 2;

	f = fopen("/proc/net/dev", "rte");
	if (f == NULL) {
		if (errno != ENOENT)
			fprintf(stderr, "Can't open /proc/net/dev: %s\n",
				strerror(errno));
		return;
	}
	__fsetlocking(f, FSETLOCKING_BYCALLER);
	while (fgets(buf, sizeof(buf), f)) {
		if (line == iface) {
			char *c;
			char *safe_ifc;

			sscanf(buf, "%16s: %255s\n", ifc, more);
			c = strchr(ifc, ':');
			if (c)
				*c = 0;
			safe_ifc = sanitize_untrusted_field(ifc);
			if (safe_ifc) {
				strncpy(ifc, safe_ifc, 64);
				ifc[64] = '\0';
				free(safe_ifc);
			} else
				*ifc = 0;
			fclose(f);
			return;
		}
		line++;
	}
	fclose(f);
}

static void read_packet(void)
{
	int line = 0;
	FILE *f;
	char buf[256];
	unsigned long sk, inode;
	unsigned int ref_cnt, type, proto, iface, r, rmem, uid;
	char more[256], ifc[65];

	f = fopen("/proc/net/packet", "rte");
	if (f == NULL) {
		if (errno != ENOENT)
			fprintf(stderr, "Can't open /proc/net/packet: %s\n",
				strerror(errno));
		return;
	}
	__fsetlocking(f, FSETLOCKING_BYCALLER);
	while (fgets(buf, sizeof(buf), f)) {
		if (line == 0) {
			line++;
			continue;
		}
		more[0] = 0;
		if (sscanf(buf, "%lX %u %u %X %u %u %u %u %lu %255s\n",
			&sk, &ref_cnt, &type, &proto, &iface,
			&r, &rmem, &uid, &inode, more) < 9)
			continue;
		get_interface(iface, ifc);
		if (list_find_inode(&l, inode))
			report_finding(0, "pkt", ifc);
	}
	fclose(f);
}

#ifdef HAVE_NETCAP_ADVANCED
static int parse_u32_hex_or_dec(const char *s, unsigned int *out)
{
	char *end;
	unsigned long v;
	int base = 10;
	const char *p;

	for (p = s; *p; p++) {
		if ((*p >= 'a' && *p <= 'f') || (*p >= 'A' && *p <= 'F')) {
			base = 16;
			break;
		}
	}
	if (strncmp(s, "0x", 2) == 0 || strncmp(s, "0X", 2) == 0)
		base = 16;
	if (base == 10 && strlen(s) > 3 && s[0] == '0')
		base = 16;
	v = strtoul(s, &end, base);
	if (end == s || *end)
		return -1;
	*out = (unsigned int)v;
	return 0;
}

static int read_diag_messages(int fd, int proto, const char *type)
{
	char buf[8192];
	ssize_t len;
	struct sockaddr_nl nladdr;
	socklen_t nladdr_len;

	while (1) {
		nladdr_len = sizeof(nladdr);
		memset(&nladdr, 0, sizeof(nladdr));
		len = recvfrom(fd, buf, sizeof(buf), 0,
				(struct sockaddr *)&nladdr, &nladdr_len);
		if (len < 0) {
			if (errno == EINTR)
				continue;
			return -1;
		}
		if (len == 0)
			return -1;
		if (nladdr_len < sizeof(nladdr) || nladdr.nl_pid != 0)
			continue;

		struct nlmsghdr *nlh;
		unsigned int rem;

		if (len > UINT_MAX)
			return -1;
		rem = (unsigned int)len;

		for (nlh = (struct nlmsghdr *)buf;
		     NLMSG_OK(nlh, rem);
		     nlh = NLMSG_NEXT(nlh, rem)) {
			struct inet_diag_msg *r;
			unsigned int port;

			if (nlh->nlmsg_type == NLMSG_DONE)
				return 0;
			if (nlh->nlmsg_type == NLMSG_ERROR) {
				struct nlmsgerr *e;

				if (nlh->nlmsg_len < NLMSG_LENGTH(sizeof(*e)))
					return -1;
				e = NLMSG_DATA(nlh);
				if (e->error == 0)
					continue;
				errno = -e->error;
				return -1;
			}
			if (nlh->nlmsg_len < NLMSG_LENGTH(sizeof(*r)))
				continue;

			r = NLMSG_DATA(nlh);
			if (!list_find_inode(&l, r->idiag_inode))
				continue;
			port = ntohs(r->id.idiag_sport);
			if (!port)
				continue;

			if (proto == IPPROTO_SCTP || proto == IPPROTO_DCCP)
				report_finding(port, type, NULL);
		}
	}
}

static int read_diag_for_proto_af(int proto, int af, const char *type)
{
	struct {
		struct nlmsghdr nlh;
		struct inet_diag_req_v2 req;
	} req;
	struct sockaddr_nl sa;
	int fd;
	int rc = -1;

	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_SOCK_DIAG);
	if (fd < 0)
		return -1;

	memset(&req, 0, sizeof(req));
	req.nlh.nlmsg_len = NLMSG_LENGTH(sizeof(req.req));
	req.nlh.nlmsg_type = SOCK_DIAG_BY_FAMILY;
	req.nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	req.req.sdiag_family = af;
	req.req.sdiag_protocol = proto;
	req.req.idiag_states = 1U << TCP_LISTEN;

	memset(&sa, 0, sizeof(sa));
	sa.nl_family = AF_NETLINK;
	sa.nl_pid = 0;
	if (connect(fd, (struct sockaddr *)&sa, sizeof(sa)) < 0)
		goto out;

	if (send(fd, &req, req.nlh.nlmsg_len, 0) < 0)
		goto out;

	rc = read_diag_messages(fd, proto, type);
out:
	close(fd);
	return rc;
}

static void read_diag_listeners(void)
{
	int sctp_ok = 0;
	int dccp_ok = 0;

	if (read_diag_for_proto_af(IPPROTO_SCTP, AF_INET, "sctp") == 0)
		sctp_ok = 1;
	if (read_diag_for_proto_af(IPPROTO_SCTP, AF_INET6, "sctp") == 0)
		sctp_ok = 1;
	if (read_diag_for_proto_af(IPPROTO_DCCP, AF_INET, "dccp") == 0)
		dccp_ok = 1;
	if (read_diag_for_proto_af(IPPROTO_DCCP, AF_INET6, "dccp") == 0)
		dccp_ok = 1;

	if (!dccp_ok) {
		read_net("/proc/net/dccp", "dccp", 1);
		read_net("/proc/net/dccp6", "dccp", 1);
	}
	(void)sctp_ok;
}

#ifdef HAVE_LINUX_VM_SOCKETS_DIAG_H
static int read_vsock_diag_messages(int fd)
{
	char buf[8192];
	ssize_t len;
	struct sockaddr_nl nladdr;
	socklen_t nladdr_len;

	while (1) {
		nladdr_len = sizeof(nladdr);
		memset(&nladdr, 0, sizeof(nladdr));
		len = recvfrom(fd, buf, sizeof(buf), 0,
				(struct sockaddr *)&nladdr, &nladdr_len);
		if (len < 0) {
			if (errno == EINTR)
				continue;
			return -1;
		}
		if (len == 0)
			return -1;
		if (nladdr_len < sizeof(nladdr) || nladdr.nl_pid != 0)
			continue;

		struct nlmsghdr *nlh;
		unsigned int rem;

		if (len > UINT_MAX)
			return -1;
		rem = (unsigned int)len;

		for (nlh = (struct nlmsghdr *)buf;
		     NLMSG_OK(nlh, rem);
		     nlh = NLMSG_NEXT(nlh, rem)) {
			struct vsock_diag_msg *r;

			if (nlh->nlmsg_type == NLMSG_DONE)
				return 0;
			if (nlh->nlmsg_type == NLMSG_ERROR) {
				struct nlmsgerr *e;

				if (nlh->nlmsg_len < NLMSG_LENGTH(sizeof(*e)))
					return -1;
				e = NLMSG_DATA(nlh);
				if (e->error == 0)
					continue;
				errno = -e->error;
				return -1;
			}
			if (nlh->nlmsg_len < NLMSG_LENGTH(sizeof(*r)))
				continue;

			r = NLMSG_DATA(nlh);
			if (r->vdiag_family != AF_VSOCK)
				continue;
			if (r->vdiag_type != SOCK_STREAM ||
			    r->vdiag_state != TCP_LISTEN)
				continue;
			if (!list_find_inode(&l, r->vdiag_ino))
				continue;
			if (r->vdiag_src_port == 0)
				continue;

			report_finding(r->vdiag_src_port, "vsock", NULL);
		}
	}
}

static int read_vsock_diag(void)
{
	struct {
		struct nlmsghdr nlh;
		struct vsock_diag_req req;
	} req;
	struct sockaddr_nl sa;
	int fd;
	int rc = -1;

	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_SOCK_DIAG);
	if (fd < 0)
		return -1;

	memset(&req, 0, sizeof(req));
	req.nlh.nlmsg_len = NLMSG_LENGTH(sizeof(req.req));
	req.nlh.nlmsg_type = SOCK_DIAG_BY_FAMILY;
	req.nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	req.req.sdiag_family = AF_VSOCK;
	req.req.sdiag_protocol = 0;
	req.req.vdiag_states = ~0U;

	memset(&sa, 0, sizeof(sa));
	sa.nl_family = AF_NETLINK;
	sa.nl_pid = 0;

	if (sendto(fd, &req, req.nlh.nlmsg_len, 0,
		   (struct sockaddr *)&sa, sizeof(sa)) < 0)
		goto out;

	rc = read_vsock_diag_messages(fd);
out:
	close(fd);
	return rc;
}
#else
static int read_vsock_diag(void)
{
	errno = EOPNOTSUPP;
	return -1;
}
#endif

static void read_vsock_proc(void)
{
	FILE *f;
	char line[512];

	f = fopen("/proc/net/vsock", "rte");
	if (f == NULL) {
		if (errno != ENOENT)
			fprintf(stderr, "Can't open /proc/net/vsock: %s\n",
				strerror(errno));
		return;
	}
	__fsetlocking(f, FSETLOCKING_BYCALLER);
	while (fgets(line, sizeof(line), f)) {
		char work[512];
		char *tok[24];
		char *save = NULL;
		char *local, *sep, *s;
		int tcnt = 0;
		unsigned long inode;
		unsigned int st, type, cid, port;

		if (strstr(line, "Local") || strstr(line, "local") ||
		    strstr(line, "Num"))
			continue;
		snprintf(work, sizeof(work), "%s", line);
		s = strtok_r(work, " \t\n", &save);
		while (s && tcnt < (int)(sizeof(tok) / sizeof(tok[0]))) {
			tok[tcnt++] = s;
			s = strtok_r(NULL, " \t\n", &save);
		}
		if (tcnt < 5)
			continue;

		local = NULL;
		int i;

		for (i = 0; i < tcnt; i++) {
			if (strchr(tok[i], ':')) {
				local = tok[i];
				break;
			}
		}
		if (!local)
			continue;
		sep = strchr(local, ':');
		if (!sep)
			continue;
		*sep = '\0';
		if (parse_u32_hex_or_dec(local, &cid) ||
		    parse_u32_hex_or_dec(sep + 1, &port))
			continue;

		if (parse_u32_hex_or_dec(tok[tcnt - 2], &st))
			continue;
		if (parse_u32_hex_or_dec(tok[tcnt - 3], &type))
			continue;
		inode = strtoul(tok[tcnt - 1], NULL, 10);
		if (!inode)
			continue;

		if (type != SOCK_STREAM || st != 0x0A || port == 0)
			continue;
		if (!list_find_inode(&l, inode))
			continue;

		(void)cid;
		report_finding(port, "vsock", NULL);
	}
	fclose(f);
}

static void read_vsock(void)
{
	if (read_vsock_diag() < 0)
		read_vsock_proc();
}
#endif

int main(int argc, char **argv)
{
	struct netcap_opts opts = { 0, 0, 0, 0, NULL };
	int i;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--advanced") == 0)
			opts.advanced = 1;
		else if (strcmp(argv[i], "--interface") == 0) {
			if (i + 1 >= argc) {
				fputs("--interface requires an argument\n",
					stderr);
				usage();
			}
			opts.interface = argv[++i];
		} else if (strncmp(argv[i], "--interface=", 12) == 0) {
			if (argv[i][12] == 0) {
				fputs("--interface requires an argument\n",
					stderr);
				usage();
			}
			opts.interface = argv[i] + 12;
		}
		else if (strcmp(argv[i], "--list-interfaces") == 0)
			opts.list_interfaces = 1;
		else if (strcmp(argv[i], "--json") == 0)
			opts.json = 1;
		else if (strcmp(argv[i], "--no-color") == 0)
			opts.no_color = 1;
		else {
			fprintf(stderr, "Unknown option: %s\n", argv[i]);
			usage();
		}
	}

	if (opts.json && !opts.advanced) {
		fputs("--json is only valid with --advanced\n", stderr);
		usage();
	}
	if (opts.list_interfaces && !opts.advanced) {
		fputs("--list-interfaces is only valid with --advanced\n",
			stderr);
		usage();
	}
	if (opts.interface && !opts.advanced) {
		fputs("--interface is only valid with --advanced\n", stderr);
		usage();
	}

	if (opts.advanced) {
#ifdef HAVE_NETCAP_ADVANCED
		return netcap_advanced_main(&opts);
#else
		fputs("netcap --advanced was disabled at configure time\n",
			stderr);
		fputs("because required kernel headers were not available\n",
			stderr);
		return 1;
#endif
	}

	if (argc > 1) {
		fputs("Too many arguments\n", stderr);
		usage();
	}

	list_create(&l);
	collect_process_info();

	// Now we check the tcp socket list...
	read_net("/proc/net/tcp", "tcp", 1);
	read_net("/proc/net/tcp6", "tcp6", 1);

	// Next udp sockets...
	read_net("/proc/net/udp", "udp", 1);
	read_net("/proc/net/udp6", "udp6", 1);
	read_net("/proc/net/udplite", "udplite", 1);
	read_net("/proc/net/udplite6", "udplite6", 1);

	// Next, raw sockets...
	read_net("/proc/net/raw", "raw", 0);
	read_net("/proc/net/raw6", "raw6", 0);

	// And last, read packet sockets
	read_packet();

	// Add listeners from protocols supported in advanced mode
#ifdef HAVE_NETCAP_ADVANCED
	read_diag_listeners();
	read_vsock();
#endif

	// Could also do icmp,netlink,unix

	list_clear(&l);
	return 0;
}
