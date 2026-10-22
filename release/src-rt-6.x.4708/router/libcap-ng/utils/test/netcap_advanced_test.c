// SPDX-License-Identifier: GPL-2.0-or-later
/* Test the real collector and renderer without depending on host sockets. */
#include "../netcap-advanced.c"

static void fail(const char *message)
{
	fprintf(stderr, "%s\n", message);
	exit(EXIT_FAILURE);
}

static FILE *capture_output(int *saved)
{
	FILE *file = tmpfile();

	*saved = dup(STDOUT_FILENO);
	if (!file || *saved < 0 || fflush(stdout) ||
	    dup2(fileno(file), STDOUT_FILENO) < 0)
		fail("Cannot capture report");
	return file;
}

static char *finish_output(FILE *file, int saved)
{
	long len;
	char *text;

	if (fflush(stdout) || dup2(saved, STDOUT_FILENO) < 0)
		fail("Cannot restore stdout");
	close(saved);
	len = ftell(file);
	if (len < 0 || !(text = calloc((size_t)len + 1, 1)))
		fail("Cannot allocate captured report");
	rewind(file);
	if (fread(text, 1, (size_t)len, file) != (size_t)len)
		fail("Cannot read captured report");
	fclose(file);
	return text;
}

static void test_process_rendering(void)
{
	struct process_info p = {
		.pid = 1234, .comm = "owner", .caps = "(full)",
		.has_privileged_caps = 1,
		.defenses = { "no", "yes", "filter", "unconfined_t" },
	};
	struct endpoint e = { .wildcard_bind = 1, .reuseport = 1 };
	const struct {
		enum plane_kind plane;
		unsigned int port, flags;
		const char *json;
	} cases[] = {
		{ PLANE_INET_EXTERNAL, 80,
		  FLAG_WILDCARD_BIND | FLAG_REUSEPORT | FLAG_PRIVILEGED_CAPS,
		  "[\"wildcard-bind\", \"reuseport\", \"privileged-caps\"]" },
		{ PLANE_BLUETOOTH, 0, FLAG_PROXIMITY_PLANE | FLAG_WILDCARD_BIND |
		  FLAG_REUSEPORT | FLAG_PRIVILEGED_CAPS,
		  "[\"proximity-plane\", \"wildcard-bind\", \"reuseport\", \"privileged-caps\"]" },
		{ PLANE_VSOCK, 22, FLAG_HYPERVISOR_PLANE | FLAG_SSH_VSOCK_22 |
		  FLAG_PRIVILEGED_CAPS,
		  "[\"hypervisor-plane\", \"ssh-on-vsock-port-22\", \"privileged-caps\"]" },
		{ PLANE_VSOCK, 80, FLAG_HYPERVISOR_PLANE | FLAG_PRIVILEGED_CAPS,
		  "[\"hypervisor-plane\", \"privileged-caps\"]" },
	};
	size_t i;
	int saved;
	FILE *file;
	char *text;

	for (i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
		e.plane = cases[i].plane;
		e.port = cases[i].port;
		if (process_flags(&p, &e) != cases[i].flags)
			fail("Process flag selection changed");
		file = capture_output(&saved);
		render_json_process(&p, &e, "");
		text = finish_output(file, saved);
		if (!strstr(text, cases[i].json) || strchr(text, '\033'))
			fail("JSON flag names/order changed");
		free(text);
	}
	use_color = 1;
	file = capture_output(&saved);
	render_tree_process_details("", 1, &p, &e, 80);
	text = finish_output(file, saved);
	if (!strstr(text, "runs_as_nonroot: " COLOR_YELLOW "no" COLOR_RESET) ||
	    !strstr(text, "no_new_privs: " COLOR_GREEN "yes" COLOR_RESET) ||
	    !strstr(text, "seccomp: " COLOR_GREEN "filter" COLOR_RESET) ||
	    !strstr(text, "lsm: " COLOR_ORANGE "unconfined_t" COLOR_RESET))
		fail("Defense colors changed");
	free(text);
	if (strcmp(defense_color("unknown"), "") ||
	    strcmp(defense_color("strict"), COLOR_GREEN) ||
	    strcmp(defense_color("disabled"), COLOR_YELLOW))
		fail("Defense state classification changed");
	use_color = 0;
}

static void test_tree_groups(void)
{
	struct model m = { 0 };
	struct process_info p = {
		.pid = 1234, .comm = "owner", .caps = "(none)",
		.defenses = { "yes", "unknown", "disabled", NULL },
	};
	struct process_info q = p;
	struct process_info *owners[] = { &p, &q };
	struct inode_proc ip = { .procs = owners, .n = 1 };
	struct endpoint_attrs attrs = { 1, 0 };
	FILE *file;
	char *text, *first, *second;
	int saved;

	if (add_endpoint(&m, "tcp", "0.0.0.0", 80, PLANE_INET_EXTERNAL,
			 "eth0", "192.0.2.1", &attrs, &ip) ||
	    add_endpoint(&m, "tcp", "0.0.0.0", 80, PLANE_INET_EXTERNAL,
			 "eth0", "192.0.2.2", &attrs, &ip) ||
	    add_endpoint(&m, "udp6", "::1", 53, PLANE_INET_LOOPBACK,
			 "lo", "::1", &attrs, &ip))
		fail("Cannot build endpoint fixture");
	file = capture_output(&saved);
	render_tree(&m);
	text = finish_output(file, saved);
	first = strstr(text, "owner (pid=1234");
	second = first ? strstr(first + 1, "owner (pid=1234") : NULL;
	if (!first || !second || strstr(second + 1, "owner (pid=1234") ||
	    strstr(text, "├─ owner") ||
	    !strstr(text, "├─ INET (external)") ||
	    !strstr(text, "└─ INET (loopback)") ||
	    !strstr(text, "└─ [::1]") || !strstr(text, "└─ *"))
		fail("Tree grouping, bind formatting, or owner dedup changed");
	free(text);

	/* The physical last endpoint repeats p, but q is the last unique row. */
	q.pid = 5678;
	q.comm = "other";
	ip.n = 2;
	if (add_endpoint(&m, "tcp", "0.0.0.0", 80, PLANE_INET_EXTERNAL,
			 "eth0", "192.0.2.1", &attrs, &ip))
		fail("Cannot add second owner");
	file = capture_output(&saved);
	render_tree(&m);
	text = finish_output(file, saved);
	if (!strstr(text, "├─ owner") || !strstr(text, "└─ other") ||
	    strstr(text, "├─ other"))
		fail("Last unique owner must close the process branch");
	free(text);
	free_model(&m);
}

#ifdef HAVE_NETCAP_VSOCK
static void test_vsock_owners(void)
{
	struct model m = { 0 };
	struct process_info first = { .pid = 100 }, second = { .pid = 200 };
	struct process_info *owners[] = { &first, &second, &first };
	struct inode_proc ip = { .procs = owners, .n = 3 };
	struct endpoint *e;

	if (add_vsock_endpoint(&m, "stream", VMADDR_CID_ANY, 22, &ip) ||
	    add_vsock_endpoint(&m, "stream", VMADDR_CID_ANY, 22, &ip))
		fail("Cannot build VSOCK fixture");
	e = &m.eps[0];
	if (m.eps_n != 1 || e->procs_n != 2 || e->procs[0] != &first ||
	    e->procs[1] != &second || e->plane != PLANE_VSOCK ||
	    e->vsock_cid != VMADDR_CID_ANY || e->port != 22 ||
	    strcmp(e->label, "stream:cid=ANY:22"))
		fail("VSOCK identity or owner merging changed");
	free_model(&m);
}
#endif

#ifdef HAVE_NETCAP_BLUETOOTH
static void test_bluetooth_tables(void)
{
	char path[] = "/tmp/netcap-bluetooth-XXXXXX";
	int fd = mkstemp(path);
	FILE *file;
	struct process_info owner = { .pid = 1234 };
	size_t count, i;

	if (fd < 0 || !(file = fdopen(fd, "w")))
		fail("Cannot create Bluetooth table fixture");
	fputs("sk RefCnt Rmem Wmem User Inode Parent\n"
	      "malformed\n"
	      "0 1 0 0 1000 12 0\n"
	      "0 1 0 0 1000 13 0\n", file);
	if (fclose(file))
		fail("Cannot write Bluetooth table fixture");

	/* Keep this independent of sysfs, Bluetooth hardware, and privileges. */
	bt_adapters_loaded = 1;
	strcpy(bt_adapters[0].name, "hci7");
	strcpy(bt_adapters[0].addr, "AA:BB:CC:DD:EE:FF");
	for (count = 0; count <= 2; count++) {
		struct model m = { 0 };
		const char *name = count == 1 ? "hci7" : "hci?";
		const char *addr = count == 1 ? "AA:BB:CC:DD:EE:FF" : "*";

		bt_adapters_n = count;
		if (add_inode_proc(&m, 12, &owner))
			fail("Cannot create socket ownership fixture");
		parse_bluetooth_file(&m, path, "rfcomm");
		parse_bluetooth_file(&m, path, "hci");
		parse_bluetooth_file(&m, path, "hci");
		if (m.eps_n != 2 || strcmp(m.eps[0].proto, "rfcomm") ||
		    strcmp(m.eps[1].proto, "hci"))
			fail("Bluetooth protocols were lost or duplicated");
		for (i = 0; i < m.eps_n; i++) {
			struct endpoint *e = &m.eps[i];

			if (e->plane != PLANE_BLUETOOTH || e->port != 0 ||
			    strcmp(e->ifname, name) || strcmp(e->bind, addr) ||
			    strcmp(e->ifaddr, addr) || e->procs_n != 1 ||
			    e->procs[0] != &owner)
				fail("Bluetooth ownership or adapter fallback changed");
		}
		free_model(&m);
	}
	unlink(path);
}
#endif

int main(void)
{
	test_process_rendering();
	test_tree_groups();
#ifdef HAVE_NETCAP_VSOCK
	test_vsock_owners();
#endif
#ifdef HAVE_NETCAP_BLUETOOTH
	test_bluetooth_tables();
#endif
	puts("Advanced netcap tests passed");
	return 0;
}
