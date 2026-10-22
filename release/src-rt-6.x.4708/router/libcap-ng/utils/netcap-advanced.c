/*
 * netcap-advanced.c - Advanced capability analysis
 * Copyright (c) 2026 Steve Grubb
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
#include <arpa/inet.h>
#if defined(HAVE_BLUETOOTH_BLUETOOTH_H) && defined(HAVE_BLUETOOTH_HCI_H)
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#define HAVE_NETCAP_BLUETOOTH 1
#endif
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <linux/capability.h>
#include <linux/inet_diag.h>
#include <linux/netlink.h>
#include <linux/sock_diag.h>
#ifdef HAVE_NETCAP_VSOCK
#include <sys/socket.h>
#include <linux/vm_sockets.h>
#ifdef HAVE_LINUX_VM_SOCKETS_DIAG_H
#include <linux/vm_sockets_diag.h>
#endif
#endif
#include <limits.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include "cap-ng.h"
#include "netcap-advanced.h"
#include "proc-output.h"
#include "proc-sanitize.h"
#include "proc-status.h"

/*
 * Overview:
 * netcap --advanced builds a process/socket ownership model from procfs and
 * sock_diag, then renders it as a tree or JSON without changing system state.
 *
 * The flow is: discover interface addresses, map socket inode->process
 * ownership from /proc/<pid>/fd, parse protocol-specific listener tables,
 * and project each endpoint onto interface/plane groupings for reporting.
 *
 * For internet sockets, wildcard binds are expanded onto concrete interface
 * addresses so the rendered tree/JSON can be consumed as an exposure map.
 * Bluetooth listeners are collected from /proc/net/rfcomm and /proc/net/hci,
 * with adapter names discovered via /sys/class/bluetooth and controller
 * addresses resolved through HCI ioctls before ownership is stitched back
 * through socket inodes gathered from /proc/<pid>/fd.
 * VSOCK listeners are collected via sock_diag when available and fall back to
 * /proc parsing when not, with ownership stitched back through socket inodes.
 * Tree output supports colorized capability/flag severity, with --no-color
 * forcing plain text. SO_REUSEPORT is detected per socket via pidfd_open and
 * pidfd_getfd while scanning /proc/<pid>/fd so the flag can be propagated into
 * endpoint rendering.
 *
 * Process metadata includes ambient capability enumeration with per-capability
 * detail and cgroup-unit extraction limited to system.slice services to keep
 * ownership context focused on service units. Line wrapping for tree output is
 * ANSI-escape-aware so colorized text wraps at display width without breaking
 * SGR sequences.
 *
 * Results depend on the current network namespace and procfs visibility;
 * restricted privileges can hide processes/sockets and yield partial output.
 */

#ifdef NETCAP_DIAG_DEBUG
#define diag_dbg(fmt, ...) \
	fprintf(stderr, "netcap-diag: " fmt "\n", ##__VA_ARGS__)
#else
#define diag_dbg(fmt, ...) do { } while (0)
#endif

enum plane_kind {
	PLANE_INET_EXTERNAL,
	PLANE_INET_LOOPBACK,
	PLANE_PACKET,
	PLANE_BLUETOOTH,
	PLANE_VSOCK,
	PLANE_COUNT,
};

#define PLANE_PACKET_NAME	"LINK-LAYER"
#define PLANE_BLUETOOTH_NAME	"BLUETOOTH"
/* Keep user-facing key name centralized to avoid legacy regressions. */
#define DEFENSES_RUNS_AS_KEY	"runs_as_nonroot"
#define BT_ADAPTER_HEURISTIC	"hci?"
#define BT_ADDR_UNKNOWN		"*"

enum endpoint_flags {
	FLAG_WILDCARD_BIND = 1U << 0,
	FLAG_PRIVILEGED_CAPS = 1U << 2,
	FLAG_HYPERVISOR_PLANE = 1U << 4,
	FLAG_SSH_VSOCK_22 = 1U << 5,
	FLAG_REUSEPORT = 1U << 6,
	FLAG_PROXIMITY_PLANE = 1U << 7,
};

struct strset {
	const char **slots;
	size_t slots_cap;
	size_t used;
};

struct iface_addr {
	int af;
	char *addr;
};

struct iface_info {
	char *name;
	struct iface_addr *addrs;
	size_t addrs_n;
	size_t addrs_cap;
};

struct defense_info {
	char *runs_as_nonroot;
	char *no_new_privs;
	char *seccomp;
	char *lsm_label;
};

struct process_info {
	int pid;
	uid_t uid;
	char *comm;
	char *exe;
	char *unit;
	char *caps;
	char *ambient_caps;
	int ambient_present;
	int open_ended_bounding;
	int has_privileged_caps;
	struct defense_info defenses;
};

struct inode_proc {
	unsigned long inode;
	struct process_info **procs;
	size_t n;
	size_t cap;
	int reuseport;
};

struct endpoint {
	char *proto;
	char *bind;
	char *label;
	unsigned int port;
	unsigned int vsock_cid;
	enum plane_kind plane;
	char *ifname;
	char *ifaddr;
	struct process_info **procs;
	size_t procs_n;
	size_t procs_cap;
	int wildcard_bind;
	int reuseport;
};

struct model {
	struct iface_info *ifaces;
	size_t ifaces_n;
	size_t ifaces_cap;
	struct process_info **procs;
	size_t procs_n;
	size_t procs_cap;
	struct inode_proc *inode_map;
	size_t inode_n;
	size_t inode_cap;
	size_t *inode_slots;
	size_t inode_slots_cap;
	struct endpoint *eps;
	size_t eps_n;
	size_t eps_cap;
};

#define INODE_SLOT_EMPTY	SIZE_MAX
#define PIDSET_EMPTY	INT_MIN

struct pidset {
	int *slots;
	size_t cap;
	size_t used;
};

struct endpoint_attrs {
	int wildcard;
	int reuseport;
};

#ifdef HAVE_NETCAP_BLUETOOTH
#define BT_ADAPTER_CACHE_MAX	8

struct bt_adapter_info {
	char name[IF_NAMESIZE];
	char addr[18];
};

static struct bt_adapter_info bt_adapters[BT_ADAPTER_CACHE_MAX];
static size_t bt_adapters_n;
static int bt_adapters_loaded;
#endif

static void free_process(struct process_info *p)
	__attr_access ((__read_write__, 1));
static void free_endpoint(struct endpoint *e)
	__attr_access ((__read_write__, 1));
static int str_is_bdaddr(const char *s)
	__attr_access ((__read_only__, 1))
	__attribute_pure__;
static char *xstrdup(const char *s)
	__attribute_malloc__
	__attr_dealloc_free
	__attr_access ((__read_only__, 1))
	__wur;
static int vec_grow(void **v, size_t *cap, size_t item)
	__attr_access ((__read_write__, 1))
	__attr_access ((__read_write__, 2))
	__wur;
static char *read_first_line(const char *path)
	__attribute_malloc__
	__attr_dealloc_free
	__attr_access ((__read_only__, 1))
	__wur;
static char *extract_unit_from_cgroup(int pid)
	__attribute_malloc__
	__attr_dealloc_free
	__wur;
static char *caps_summary_for_pid(int pid, int *privileged, int *has_amb,
				  int *has_bnd, char **amb_list)
	__attribute_malloc__
	__attr_dealloc_free
	__attr_access ((__write_only__, 2))
	__attr_access ((__write_only__, 3))
	__attr_access ((__write_only__, 4))
	__attr_access ((__write_only__, 5))
	__wur;

static int use_color;

#define COLOR_ORANGE	"\033[38;5;208m"
#define COLOR_YELLOW	"\033[38;5;226m"
#define COLOR_GREEN	"\033[38;5;82m"
#define COLOR_RESET	"\033[0m"

enum cap_severity {
	CAP_SEV_NEUTRAL,
	CAP_SEV_YELLOW,
	CAP_SEV_ORANGE,
};

static const char *orange_caps[] = {
	"sys_ptrace", "sys_module", "sys_rawio", "setuid", "setgid",
	"setpcap", "audit_control",
};

static const char *yellow_caps[] = {
	"sys_admin", "dac_override", "dac_read_search", "net_admin",
	"net_raw", "chown", "fowner", "mknod", "sys_chroot",
};

/*
 * cap_name_severity - classify one capability name into severity tiers.
 * @name: capability token without "cap_" prefix.
 *
 * Returns severity bucket used for tree color selection.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static enum cap_severity cap_name_severity(const char *name)
{
	size_t i;

	for (i = 0; i < sizeof(orange_caps) / sizeof(orange_caps[0]); i++) {
		if (strcmp(name, orange_caps[i]) == 0)
			return CAP_SEV_ORANGE;
	}
	for (i = 0; i < sizeof(yellow_caps) / sizeof(yellow_caps[0]); i++) {
		if (strcmp(name, yellow_caps[i]) == 0)
			return CAP_SEV_YELLOW;
	}
	return CAP_SEV_NEUTRAL;
}

/*
 * caps_contains_token - test whether @token appears as a capability list item.
 * @caps: comma/space separated capability summary text.
 * @token: capability token to locate.
 *
 * Returns non-zero when @token is present as a whole list element, else 0.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static int caps_contains_token(const char *caps, const char *token)
{
	size_t len;
	const char *p;

	if (!caps || !token)
		return 0;
	len = strlen(token);
	for (p = caps; (p = strstr(p, token)) != NULL; p++) {
		int left_ok = (p == caps) ||
			(p > caps + 1 && p[-1] == ' ' && p[-2] == ',');
		char right = p[len];
		int right_ok = right == 0 || right == ',' || right == ' ' || right == '[';

		if (left_ok && right_ok)
			return 1;
	}
	return 0;
}

/*
 * sev_color - map severity level to ANSI color sequence.
 * @sev: severity class to map.
 *
 * Returns static color code pointer, or NULL when uncolored.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static const char *sev_color(enum cap_severity sev)
{
	if (sev == CAP_SEV_ORANGE)
		return COLOR_ORANGE;
	if (sev == CAP_SEV_YELLOW)
		return COLOR_YELLOW;
	return NULL;
}

/*
 * caps_worst_severity - find highest severity capability in @caps text.
 * @caps: capability list text from caps_summary_for_pid().
 *
 * Returns highest matched severity, or CAP_SEV_NEUTRAL.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static enum cap_severity caps_worst_severity(const char *caps)
{
	size_t i;

	if (!caps)
		return CAP_SEV_NEUTRAL;

	if (strcmp(caps, "(full)") == 0)
		return CAP_SEV_ORANGE;

	for (i = 0; i < sizeof(orange_caps) / sizeof(orange_caps[0]); i++) {
		if (caps_contains_token(caps, orange_caps[i]))
			return CAP_SEV_ORANGE;
	}
	for (i = 0; i < sizeof(yellow_caps) / sizeof(yellow_caps[0]); i++) {
		if (caps_contains_token(caps, yellow_caps[i]))
			return CAP_SEV_YELLOW;
	}
	return CAP_SEV_NEUTRAL;
}

static void free_model(struct model *m);
static void json_escape(const char *s);
static int bind_sort_cmp(const char *a, const char *b);
static struct inode_proc *lookup_inode(struct model *m, unsigned long inode);
static void render_interfaces_text(struct model *m);
static void render_interfaces_json(struct model *m);

/*
 * iface_addr_cmp - order interface addresses by family then text.
 * @av/@bv: qsort pointers to struct iface_addr elements.
 *
 * Returns strcmp-style ordering suitable for qsort().
 */
static int iface_addr_cmp(const void *av, const void *bv)
{
	const struct iface_addr *a = av;
	const struct iface_addr *b = bv;

	if (a->af != b->af)
		return a->af - b->af;
	return strcmp(a->addr, b->addr);
}

/*
 * iface_info_cmp - order interfaces by name for stable output.
 * @av/@bv: qsort pointers to struct iface_info elements.
 *
 * Returns strcmp-style ordering suitable for qsort().
 */
static int iface_info_cmp(const void *av, const void *bv)
{
	const struct iface_info *a = av;
	const struct iface_info *b = bv;

	return strcmp(a->name, b->name);
}

/*
 * sort_interfaces - sort interfaces and per-interface addresses in place.
 * @m: model containing interface inventory from getifaddrs().
 *
 * Returns no value.
 */
static void sort_interfaces(struct model *m)
{
	size_t i;

	if (m->ifaces_n > 1)
		qsort(m->ifaces, m->ifaces_n, sizeof(struct iface_info),
			iface_info_cmp);
	for (i = 0; i < m->ifaces_n; i++) {
		if (m->ifaces[i].addrs_n > 1)
			qsort(m->ifaces[i].addrs, m->ifaces[i].addrs_n,
				sizeof(struct iface_addr), iface_addr_cmp);
	}
}

/*
 * str_hash - hash a string key for open-addressed set placement.
 * @s: NUL-terminated key string.
 * @slots_cap: destination hash table capacity.
 *
 * Returns index in [0, @slots_cap).
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static size_t str_hash(const char *s, size_t slots_cap)
{
	uint64_t x = 1469598103934665603ULL;

	for (; *s; s++) {
		x ^= (unsigned char)*s;
		x *= 1099511628211ULL;
	}
	return (size_t)(x % slots_cap);
}

/*
 * strset_rebuild - resize and rehash string set storage.
 * @set: set object whose slots array is replaced.
 * @new_cap: requested slot capacity before minimum clamping.
 *
 * Returns 0 on success, -1 on allocation failure.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static int strset_rebuild(struct strset *set, size_t new_cap)
{
	const char **slots;
	size_t i;

	if (new_cap < 16)
		new_cap = 16;
	slots = calloc(new_cap, sizeof(*slots));
	if (!slots) {
		fprintf(stderr, "Out of memory\n");
		return -1;
	}
	for (i = 0; i < set->slots_cap; i++) {
		size_t pos;

		if (!set->slots[i])
			continue;
		pos = str_hash(set->slots[i], new_cap);
		while (slots[pos])
			pos = (pos + 1) % new_cap;
		slots[pos] = set->slots[i];
	}
	free(set->slots);
	set->slots = slots;
	set->slots_cap = new_cap;
	return 0;
}

/*
 * strset_add - insert @s into the string set if absent.
 * @set: destination hash set.
 * @s: caller-owned string pointer stored by reference.
 *
 * Returns 1 when inserted, 0 when already present, -1 on failure.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static int strset_add(struct strset *set, const char *s)
{
	size_t pos;

	if (set->slots_cap == 0) {
		if (strset_rebuild(set, 16) != 0)
			return -1;
	}
	if ((set->used + 1) * 4 >= set->slots_cap * 3) {
		size_t new_cap = set->slots_cap * 2;

		if (new_cap < set->slots_cap)
			return -1;
		if (strset_rebuild(set, new_cap) != 0)
			return -1;
	}

	pos = str_hash(s, set->slots_cap);
	while (set->slots[pos]) {
		if (strcmp(set->slots[pos], s) == 0)
			return 0;
		pos = (pos + 1) % set->slots_cap;
	}
	set->slots[pos] = s;
	set->used++;
	return 1;
}

/*
 * strset_free - release dynamic storage owned by @set.
 * @set: hash set to reset.
 *
 * Returns no value.
 * Side effects/assumptions: Frees heap memory referenced by @set.
 */
static void strset_free(struct strset *set)
{
	free(set->slots);
	set->slots = NULL;
	set->slots_cap = 0;
	set->used = 0;
}

/*
 * inode_hash - hash inode key for inode ownership map slot selection.
 * @inode: socket inode value.
 * @slots_cap: destination hash table capacity.
 *
 * Returns index in [0, @slots_cap).
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static size_t inode_hash(unsigned long inode, size_t slots_cap)
{
	uint64_t x = inode;

	x ^= x >> 33;
	x *= 0xff51afd7ed558ccdULL;
	x ^= x >> 33;
	x *= 0xc4ceb9fe1a85ec53ULL;
	x ^= x >> 33;
	return (size_t)(x % slots_cap);
}

/*
 * inode_hash_rebuild - resize and repopulate inode hash slots.
 * @m: model containing inode_map entries and slot metadata.
 * @new_cap: requested new slot capacity.
 *
 * Returns 0 on success, -1 on allocation failure.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static int inode_hash_rebuild(struct model *m, size_t new_cap)
{
	size_t i;
	size_t *slots;

	if (new_cap < 16)
		new_cap = 16;
	slots = malloc(new_cap * sizeof(*slots));
	if (!slots) {
		fprintf(stderr, "Out of memory\n");
		return -1;
	}
	for (i = 0; i < new_cap; i++)
		slots[i] = INODE_SLOT_EMPTY;

	for (i = 0; i < m->inode_n; i++) {
		size_t pos = inode_hash(m->inode_map[i].inode, new_cap);

		while (slots[pos] != INODE_SLOT_EMPTY)
			pos = (pos + 1) % new_cap;
		slots[pos] = i;
	}

	free(m->inode_slots);
	m->inode_slots = slots;
	m->inode_slots_cap = new_cap;
	return 0;
}

/*
 * inode_hash_ensure_capacity - grow inode hash table when load is high.
 * @m: model whose inode slot table may be resized.
 *
 * Returns 0 when capacity is sufficient or grown, -1 on failure.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static int inode_hash_ensure_capacity(struct model *m)
{
	size_t new_cap;

	if (m->inode_slots_cap == 0)
		return inode_hash_rebuild(m, 16);
	if ((m->inode_n + 1) * 4 < m->inode_slots_cap * 3)
		return 0;

	new_cap = m->inode_slots_cap * 2;
	if (new_cap < m->inode_slots_cap)
		return -1;
	return inode_hash_rebuild(m, new_cap);
}

/*
 * inode_hash_find - find inode_map index for @inode.
 * @m: model containing inode hash slots.
 * @inode: inode key to search.
 *
 * Returns non-negative inode_map index on hit, -1 on miss.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static ssize_t inode_hash_find(struct model *m, unsigned long inode)
{
	size_t pos;
	size_t start;

	if (m->inode_slots_cap == 0)
		return -1;

	pos = inode_hash(inode, m->inode_slots_cap);
	start = pos;
	while (m->inode_slots[pos] != INODE_SLOT_EMPTY) {
		size_t idx = m->inode_slots[pos];

		if (m->inode_map[idx].inode == inode)
			return idx;
		pos = (pos + 1) % m->inode_slots_cap;
		if (pos == start)
			break;
	}
	return -1;
}

/*
 * inode_hash_insert - place one inode_map index into hash slots.
 * @m: model containing destination hash slots.
 * @idx: inode_map entry index to insert.
 *
 * Returns no value.
 * Side effects/assumptions: Mutates @m->inode_slots insertion state.
 */
static void inode_hash_insert(struct model *m, size_t idx)
{
	size_t pos = inode_hash(m->inode_map[idx].inode, m->inode_slots_cap);

	while (m->inode_slots[pos] != INODE_SLOT_EMPTY)
		pos = (pos + 1) % m->inode_slots_cap;
	m->inode_slots[pos] = idx;
}

/*
 * xstrdup - duplicate @s into a newly allocated string.
 * @s: source string, or NULL.
 *
 * Returns caller-owned copy, or NULL for NULL input or allocation failure.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static char *xstrdup(const char *s)
{
	char *p;

	if (!s)
		return NULL;
	p = strdup(s);
	if (!p)
		fprintf(stderr, "Out of memory\n");
	return p;
}

/*
 * vec_grow - grow vector storage for dynamic arrays.
 * @v: pointer to heap buffer pointer updated on success.
 * @cap: current/new capacity element count.
 * @item: size in bytes of each element.
 *
 * Returns 0 on success, -1 on allocation failure.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static int vec_grow(void **v, size_t *cap, size_t item)
{
	void *p;
	size_t ncap;

	if (*cap)
		ncap = *cap * 2;
	else
		ncap = 8;
	p = realloc(*v, ncap * item);
	if (!p) {
		fprintf(stderr, "Out of memory\n");
		return -1;
	}
	*v = p;
	*cap = ncap;
	return 0;
}

/*
 * pid_hash - hash process ID for pidset probing.
 * @pid: process id key.
 * @cap: pidset slot capacity.
 *
 * Returns slot index in [0, @cap).
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static size_t pid_hash(int pid, size_t cap)
{
	uint32_t v = (uint32_t)pid;

	v ^= v >> 16;
	v *= 0x7feb352dU;
	v ^= v >> 15;
	v *= 0x846ca68bU;
	v ^= v >> 16;
	return v % cap;
}

/*
 * pidset_rehash - resize/reinsert pidset contents.
 * @ps: pidset to grow.
 * @new_cap: requested slot capacity.
 *
 * Returns 0 on success, -1 on allocation failure.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static int pidset_rehash(struct pidset *ps, size_t new_cap)
{
	int *new_slots;
	size_t i;

	new_slots = malloc(new_cap * sizeof(*new_slots));
	if (!new_slots)
		return -1;
	for (i = 0; i < new_cap; i++)
		new_slots[i] = PIDSET_EMPTY;
	for (i = 0; i < ps->cap; i++) {
		size_t pos;

		if (ps->slots[i] == PIDSET_EMPTY)
			continue;
		pos = pid_hash(ps->slots[i], new_cap);
		while (new_slots[pos] != PIDSET_EMPTY)
			pos = (pos + 1) % new_cap;
		new_slots[pos] = ps->slots[i];
	}
	free(ps->slots);
	ps->slots = new_slots;
	ps->cap = new_cap;
	return 0;
}

/*
 * pidset_init - initialize pidset with empty hash table state.
 * @ps: pidset object to initialize.
 *
 * Returns 0 on success, -1 on allocation failure.
 * Side effects/assumptions: Allocates heap storage for @ps slots.
 */
static int pidset_init(struct pidset *ps)
{
	ps->cap = 16;
	ps->used = 0;
	ps->slots = malloc(ps->cap * sizeof(*ps->slots));
	if (!ps->slots)
		return -1;
	for (size_t i = 0; i < ps->cap; i++)
		ps->slots[i] = PIDSET_EMPTY;
	return 0;
}

/*
 * pidset_free - release pidset storage and reset fields.
 * @ps: pidset object to clear.
 *
 * Returns no value.
 * Side effects/assumptions: Frees heap memory referenced by @ps.
 */
static void pidset_free(struct pidset *ps)
{
	free(ps->slots);
	ps->slots = NULL;
	ps->cap = 0;
	ps->used = 0;
}

/*
 * pidset_test_and_add - query/insert PID in dedup set.
 * @ps: pidset tracking seen process IDs.
 * @pid: process id to test and insert.
 *
 * Returns 1 if already present, 0 if newly inserted, -1 on failure.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static int pidset_test_and_add(struct pidset *ps, int pid)
{
	size_t pos;

	if ((ps->used + 1) * 10 >= ps->cap * 7) {
		if (pidset_rehash(ps, ps->cap * 2))
			return -1;
	}
	pos = pid_hash(pid, ps->cap);
	while (ps->slots[pos] != PIDSET_EMPTY) {
		if (ps->slots[pos] == pid)
			return 1;
		pos = (pos + 1) % ps->cap;
	}
	ps->slots[pos] = pid;
	ps->used++;
	return 0;
}

static const struct {
	unsigned int bit;
	const char *name;
	const char *color;
} flag_names[] = {
	{ FLAG_PROXIMITY_PLANE, "proximity-plane", COLOR_YELLOW },
	{ FLAG_HYPERVISOR_PLANE, "hypervisor-plane", COLOR_YELLOW },
	{ FLAG_SSH_VSOCK_22, "ssh-on-vsock-port-22", NULL },
	{ FLAG_WILDCARD_BIND, "wildcard-bind", COLOR_YELLOW },
	{ FLAG_REUSEPORT, "reuseport", COLOR_YELLOW },
	{ FLAG_PRIVILEGED_CAPS, "privileged-caps", NULL },
};

/* Keep tree and JSON flag selection identical, including VSOCK exclusions. */
static unsigned int process_flags(const struct process_info *p,
				  const struct endpoint *e)
{
	unsigned int flags = p->has_privileged_caps ? FLAG_PRIVILEGED_CAPS : 0;

	if (e->plane == PLANE_VSOCK) {
		flags |= FLAG_HYPERVISOR_PLANE;
		if (e->port == 22)
			flags |= FLAG_SSH_VSOCK_22;
	} else {
		if (e->plane == PLANE_BLUETOOTH)
			flags |= FLAG_PROXIMITY_PLANE;
		if (e->wildcard_bind)
			flags |= FLAG_WILDCARD_BIND;
		if (e->reuseport)
			flags |= FLAG_REUSEPORT;
	}
	return flags;
}

/*
 * print_flag_nodes - render endpoint/process flag leaves under "flags" node.
 * @pfx_flags: tree prefix for flag child lines.
 * @width: wrap width for tree nodes.
 * @flags: bitmask of endpoint/process flags to print.
 * @priv_sev: privileged capability severity for colorizing that flag.
 *
 * Returns no value.
 * Side effects/assumptions: Writes formatted output to stdout.
 */
static void print_flag_nodes(const char *pfx_flags, int width,
	unsigned int flags, enum cap_severity priv_sev)
{
	size_t i;
	size_t n = 0;
	size_t printed = 0;
	char node[256];

	for (i = 0; i < sizeof(flag_names) / sizeof(flag_names[0]); i++)
		if (flags & flag_names[i].bit)
			n++;
	if (!n) {
		proc_tree_print_node(pfx_flags, 1, "(none)", width);
		return;
	}
	for (i = 0; i < sizeof(flag_names) / sizeof(flag_names[0]); i++) {
		const char *color = flag_names[i].color;

		if (!(flags & flag_names[i].bit))
			continue;
		if (flag_names[i].bit == FLAG_PRIVILEGED_CAPS)
			color = sev_color(priv_sev);
		if (use_color && color)
			snprintf(node, sizeof(node), "%s%s%s", color, flag_names[i].name,
				COLOR_RESET);
		else
			snprintf(node, sizeof(node), "%s", flag_names[i].name);
		printed++;
		proc_tree_print_node(pfx_flags, printed == n, node, width);
	}
}

/*
 * str_is_loopback - check whether textual @addr is loopback for @af.
 * @af: address family used to interpret @addr.
 * @addr: textual IPv4/IPv6 address to classify.
 *
 * Returns non-zero for loopback addresses, else 0.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static int str_is_loopback(int af, const char *addr)
{
	struct in_addr a4;
	struct in6_addr a6;

	if (af == AF_INET) {
		if (inet_pton(AF_INET, addr, &a4) != 1)
			return 0;
		return (ntohl(a4.s_addr) & 0xff000000U) == 0x7f000000U;
	}
	if (af == AF_INET6) {
		if (inet_pton(AF_INET6, addr, &a6) != 1)
			return 0;
		return IN6_IS_ADDR_LOOPBACK(&a6);
	}
	return 0;
}

/*
 * str_is_wildcard - check whether textual @addr is wildcard-any for @af.
 * @af: address family used to interpret @addr.
 * @addr: textual IPv4/IPv6 bind address to classify.
 *
 * Returns non-zero for 0.0.0.0/:: wildcard binds, else 0.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static int str_is_wildcard(int af, const char *addr)
{
	if (af == AF_INET)
		return strcmp(addr, "0.0.0.0") == 0;
	if (af == AF_INET6)
		return strcmp(addr, "::") == 0;
	return 0;
}

/*
 * str_is_multicast - check whether textual @addr is multicast for @af.
 * @af: address family used to interpret @addr.
 * @addr: textual IPv4/IPv6 bind address to classify.
 *
 * Returns non-zero for multicast addresses, else 0.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static int str_is_multicast(int af, const char *addr)
{
	struct in_addr a4;
	struct in6_addr a6;

	if (af == AF_INET) {
		if (inet_pton(AF_INET, addr, &a4) != 1)
			return 0;
		return IN_MULTICAST(ntohl(a4.s_addr));
	}
	if (af == AF_INET6) {
		if (inet_pton(AF_INET6, addr, &a6) != 1)
			return 0;
		return IN6_IS_ADDR_MULTICAST(&a6);
	}
	return 0;
}

/*
 * find_iface - locate interface record by name in @m.
 * @m: model containing interface inventory.
 * @name: interface name key (borrowed, not owned).
 *
 * Returns a mutable iface pointer on match, or NULL if not found.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static struct iface_info *find_iface(struct model *m, const char *name)
{
	size_t i;

	for (i = 0; i < m->ifaces_n; i++)
		if (strcmp(m->ifaces[i].name, name) == 0)
			return &m->ifaces[i];
	return NULL;
}

/*
 * add_iface_addr - append one interface address if not already present.
 * @ifc: interface record to update (takes ownership of duplicated @addr).
 * @af: address family for @addr.
 * @addr: textual address to copy into @ifc.
 *
 * Returns 0 on success, -1 on allocation failure.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static int add_iface_addr(struct iface_info *ifc, int af, const char *addr)
{
	size_t i;

	for (i = 0; i < ifc->addrs_n; i++) {
		if (ifc->addrs[i].af == af && strcmp(ifc->addrs[i].addr, addr) == 0)
			return 0;
	}
	if (ifc->addrs_n == ifc->addrs_cap && vec_grow((void **)&ifc->addrs,
		&ifc->addrs_cap, sizeof(struct iface_addr)))
		return -1;
	ifc->addrs[ifc->addrs_n].af = af;
	ifc->addrs[ifc->addrs_n].addr = xstrdup(addr);
	if (!ifc->addrs[ifc->addrs_n].addr)
		return -1;
	ifc->addrs_n++;
	return 0;
}

/*
 * collect_interfaces - snapshot AF_INET/AF_INET6 iface addresses in @m.
 * @m: model populated from getifaddrs() in the current network namespace.
 *
 * Returns 0 on success, -1 on getifaddrs/allocation failure.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static int collect_interfaces(struct model *m)
{
	struct ifaddrs *ifa, *cur;
	char buf[INET6_ADDRSTRLEN];
	struct iface_info *ifc;

	if (getifaddrs(&ifa) != 0)
		return -1;
	for (cur = ifa; cur; cur = cur->ifa_next) {
		if (!cur->ifa_name || !cur->ifa_addr)
			continue;
		if (cur->ifa_addr->sa_family != AF_INET &&
		    cur->ifa_addr->sa_family != AF_INET6)
			continue;
		ifc = find_iface(m, cur->ifa_name);
		if (!ifc) {
			if (m->ifaces_n == m->ifaces_cap &&
			    vec_grow((void **)&m->ifaces, &m->ifaces_cap,
			    sizeof(struct iface_info)))
				goto fail;
			ifc = &m->ifaces[m->ifaces_n++];
			memset(ifc, 0, sizeof(*ifc));
			ifc->name = xstrdup(cur->ifa_name);
			if (!ifc->name)
				goto fail;
		}
		if (cur->ifa_addr->sa_family == AF_INET) {
			if (!inet_ntop(AF_INET,
				&((struct sockaddr_in *)cur->ifa_addr)->sin_addr,
				buf, sizeof(buf)))
				continue;
		} else {
			if (!inet_ntop(AF_INET6,
				&((struct sockaddr_in6 *)cur->ifa_addr)->sin6_addr,
				buf, sizeof(buf)))
				continue;
		}
		if (add_iface_addr(ifc, cur->ifa_addr->sa_family, buf))
			goto fail;
	}
	freeifaddrs(ifa);
	return 0;
fail:
	freeifaddrs(ifa);
	return -1;
}

/*
 * read_first_line - read and trim the first line from @path.
 * @path: procfs/sysfs-style file path to read.
 *
 * Returns a caller-owned string, or NULL on open/read/allocation failure.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static char *read_first_line(const char *path)
{
	int fd;
	char *buf;
	ssize_t len;

	fd = open(path, O_RDONLY | O_CLOEXEC);
	if (fd < 0)
		return NULL;
	buf = malloc(1024);
	if (!buf) {
		close(fd);
		return NULL;
	}
	len = read(fd, buf, 1023);
	close(fd);
	if (len <= 0) {
		free(buf);
		return NULL;
	}
	buf[len] = 0;
	while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
		buf[--len] = 0;
	return buf;
}

/*
 * extract_unit_from_cgroup - best-effort unit name lookup for @pid.
 * @pid: process ID whose /proc/<pid>/cgroup is inspected.
 *
 * Returns a caller-owned unit/scope name, or NULL if not found/readable.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static char *extract_unit_from_cgroup(int pid)
{
	char path[64], line[512];
	FILE *f;

	snprintf(path, sizeof(path), "/proc/%d/cgroup", pid);
	f = fopen(path, "rte");
	if (!f)
		return NULL;
	__fsetlocking(f, FSETLOCKING_BYCALLER);
	while (fgets(line, sizeof(line), f)) {
		char *s;
		s = strstr(line, ".service");
		if (!s)
			s = strstr(line, ".scope");
		if (s && !strstr(line, "system.slice"))
			continue;
		if (s) {
			while (s > line && *s != '/')
				s--;
			if (*s == '/')
				s++;
			char *e = s;
			while (*e && *e != '\n' && *e != '/')
				e++;
			*e = 0;
			fclose(f);
			return xstrdup(s);
		}
	}
	fclose(f);
	return NULL;
}

/*
 * caps_summary_for_pid - format capability summary for one process.
 * @pid: process ID inspected through libcap-ng APIs.
 * @privileged: out flag set when notable privileged effective caps exist.
 * @has_amb: out flag set when ambient capabilities are present.
 * @has_bnd: out flag set when bounding set entries are present.
 *
 * Returns caller-owned summary text; errors degrade to "(none)" style text.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static char *caps_summary_for_pid(int pid, int *privileged, int *has_amb,
	int *has_bnd, char **amb_list)
{
	char out[4096];
	char amb_out[1024];
	char *amb_dst = amb_out;
	char *dst = out;
	size_t amb_left = sizeof(amb_out);
	size_t left = sizeof(out);
	int i, first = 1, amb_first = 1;
	capng_results_t c;

	*privileged = 0;
	*has_amb = 0;
	*has_bnd = 0;
	*amb_list = NULL;

	capng_clear(CAPNG_SELECT_ALL);
	capng_setpid(pid);
	if (capng_get_caps_process())
		return xstrdup("(none)");

	if (capng_have_capability(CAPNG_EFFECTIVE, CAP_SYS_ADMIN) ||
	    capng_have_capability(CAPNG_EFFECTIVE, CAP_SYS_PTRACE) ||
	    capng_have_capability(CAPNG_EFFECTIVE, CAP_DAC_READ_SEARCH) ||
	    capng_have_capability(CAPNG_EFFECTIVE, CAP_NET_ADMIN) ||
	    capng_have_capability(CAPNG_EFFECTIVE, CAP_NET_RAW))
		*privileged = 1;

	c = capng_have_capabilities(CAPNG_SELECT_CAPS);
	if (c == CAPNG_FULL) {
		strncpy(out, "(full)", sizeof(out));
		out[sizeof(out) - 1] = 0;
	} else if (c <= CAPNG_NONE) {
		strncpy(out, "(none)", sizeof(out));
		out[sizeof(out) - 1] = 0;
	} else {
		*dst = 0;
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			size_t n;
			size_t sep = first ? 0 : 2;

			if (!capng_have_capability(CAPNG_PERMITTED, i))
				continue;
			const char *name = capng_capability_to_name(i);
			if (!name)
				continue;
			if (strncmp(name, "cap_", 4) == 0)
				name += 4;

			n = strlen(name);
			if (left <= sep + n)
				break;
			if (!first) {
				*dst++ = ',';
				*dst++ = ' ';
				left -= 2;
			}
			memcpy(dst, name, n);
			dst += n;
			left -= n;
			*dst = 0;
			first = 0;
		}
		if (out[0] == 0) {
			strncpy(out, "(none)", sizeof(out));
			out[sizeof(out) - 1] = 0;
		}
	}
	if (capng_have_capabilities(CAPNG_SELECT_AMBIENT) > CAPNG_NONE)
		*has_amb = 1;
	if (*has_amb) {
		*amb_dst = 0;
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			size_t n;
			size_t sep = amb_first ? 0 : 2;
			const char *name;

			if (!capng_have_capability(CAPNG_AMBIENT, i))
				continue;
			name = capng_capability_to_name(i);
			if (!name)
				continue;
			if (strncmp(name, "cap_", 4) == 0)
				name += 4;

			n = strlen(name);
			if (amb_left <= sep + n)
				break;
			if (!amb_first) {
				*amb_dst++ = ',';
				*amb_dst++ = ' ';
				amb_left -= 2;
			}
			memcpy(amb_dst, name, n);
			amb_dst += n;
			amb_left -= n;
			*amb_dst = 0;
			amb_first = 0;
		}
		if (amb_out[0])
			*amb_list = xstrdup(amb_out);
	}
	if (capng_have_capabilities(CAPNG_SELECT_BOUNDS) > CAPNG_NONE)
		*has_bnd = 1;
	return xstrdup(out);
}

/*
 * parse_status_defenses - read process hardening metadata into @d.
 * @pid: process ID whose procfs status/attr files are parsed.
 * @uid: process real UID, used for root/non-root interpretation.
 * @d: destination struct receiving caller-freed string fields.
 * @status: parsed /proc/<pid>/status fields consumed for hardening decode.
 *
 * Missing fields are tolerated; function leaves best-effort defaults.
 * Returns no value.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static void parse_status_defenses(int pid, uid_t uid, struct defense_info *d,
	const struct proc_status *status)
{
	char path[64];

	d->runs_as_nonroot = xstrdup(uid != 0 ? "yes" : "no");
	d->no_new_privs = xstrdup("unknown");
	d->seccomp = xstrdup("disabled");
	d->lsm_label = NULL;

	if (status->seen_no_new_privs) {
		free(d->no_new_privs);
		d->no_new_privs = xstrdup(status->no_new_privs ? "yes" : "no");
	}
	if (status->seen_seccomp) {
		free(d->seccomp);
		if (status->seccomp == 0)
			d->seccomp = xstrdup("disabled");
		else if (status->seccomp == 1)
			d->seccomp = xstrdup("strict");
		else
			d->seccomp = xstrdup("filter");
	}

	snprintf(path, sizeof(path), "/proc/%d/attr/current", pid);
	d->lsm_label = read_first_line(path);
	if (sanitize_untrusted_owned(&d->lsm_label) < 0) {
		free(d->lsm_label);
		d->lsm_label = NULL;
	}

}

/*
 * add_process - collect process metadata and append it to @m.
 * @m: model taking ownership of the created process_info on success.
 * @pid: numeric process ID to read from /proc.
 *
 * Returns stored process pointer on success, or NULL on parse/allocation error.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static struct process_info *add_process(struct model *m, int pid)
{
	char path[64];
	char exepath[PATH_MAX];
	ssize_t exelen;
	int has_amb = 0, has_bnd = 0;
	char *amb_list = NULL;
	struct proc_status status;
	struct process_info *p;

	if (proc_read_status(pid, &status) < 0 || !status.seen_uid)
		return NULL;

	p = calloc(1, sizeof(*p));
	if (!p) {
		fprintf(stderr, "Out of memory\n");
		return NULL;
	}
	p->pid = pid;
	p->uid = status.uid;
	p->comm = xstrdup(status.seen_name ? status.name : "?");
	if (sanitize_untrusted_owned(&p->comm) < 0)
		goto fail;
	snprintf(path, sizeof(path), "/proc/%d/exe", pid);
	exelen = readlink(path, exepath, sizeof(exepath) - 1);
	if (exelen >= 0) {
		size_t deleted_len = strlen(" (deleted)");

		exepath[exelen] = '\0';
		if ((size_t)exelen > deleted_len &&
		    strcmp(exepath + exelen - deleted_len,
			" (deleted)") == 0)
			exepath[exelen - deleted_len] = '\0';
		p->exe = xstrdup(exepath);
		if (sanitize_untrusted_owned(&p->exe) < 0)
			goto fail;
	}
	p->unit = extract_unit_from_cgroup(pid);
	if (sanitize_untrusted_owned(&p->unit) < 0)
		goto fail;
	p->caps = caps_summary_for_pid(pid, &p->has_privileged_caps,
		&has_amb, &has_bnd, &amb_list);
	p->ambient_caps = amb_list;
	p->ambient_present = has_amb;
	p->open_ended_bounding = has_bnd;
	parse_status_defenses(pid, status.uid, &p->defenses, &status);
	if (!p->comm || (exelen >= 0 && !p->exe) || !p->caps ||
	    (has_amb && !p->ambient_caps) ||
	    !p->defenses.runs_as_nonroot ||
	    !p->defenses.no_new_privs || !p->defenses.seccomp)
		goto fail;

	if (m->procs_n == m->procs_cap && vec_grow((void **)&m->procs,
	    &m->procs_cap, sizeof(struct process_info *)))
		goto fail;
	m->procs[m->procs_n++] = p;
	return p;

fail:
	free_process(p);
	return NULL;
}

/*
 * add_inode_proc - add inode->process ownership mapping into @m.
 * @m: model containing inode map storage.
 * @inode: socket inode key from procfs/netlink tables.
 * @p: process entry pointer that must remain valid for @m lifetime.
 *
 * Returns 0 on success, -1 on allocation failure.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static int add_inode_proc(struct model *m, unsigned long inode,
	struct process_info *p)
{
	size_t j;
	ssize_t idx;
	struct inode_proc *ip = NULL;

	idx = inode_hash_find(m, inode);
	if (idx >= 0)
		ip = &m->inode_map[idx];
	if (!ip) {
		if (inode_hash_ensure_capacity(m) != 0)
			return -1;
		if (m->inode_n == m->inode_cap && vec_grow((void **)&m->inode_map,
		    &m->inode_cap, sizeof(struct inode_proc)))
			return -1;
		ip = &m->inode_map[m->inode_n++];
		memset(ip, 0, sizeof(*ip));
		ip->inode = inode;
		inode_hash_insert(m, m->inode_n - 1);
	}
	for (j = 0; j < ip->n; j++)
		if (ip->procs[j]->pid == p->pid)
			return 0;
	if (ip->n == ip->cap && vec_grow((void **)&ip->procs, &ip->cap,
	    sizeof(struct process_info *)))
		return -1;
	ip->procs[ip->n++] = p;
	return 0;
}

/*
 * dup_target_fd - duplicate one target process fd into this process.
 * @pid: process id owning @fdnum.
 * @fdnum: fd number inside @pid to duplicate.
 *
 * Uses pidfd_open + pidfd_getfd when supported by the running kernel.
 * Returns duplicated fd on success, or -1 when unsupported/inaccessible.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static int dup_target_fd(int pid, int fdnum)
{
#if defined(__NR_pidfd_open) && defined(__NR_pidfd_getfd)
	static int warned_unavail;
	int pidfd;
	int dupfd;

	pidfd = syscall(__NR_pidfd_open, pid, 0);
	if (pidfd < 0) {
		if (errno == ENOSYS && !warned_unavail) {
			diag_dbg("pidfd_getfd unavailable; fd inspection disabled");
			warned_unavail = 1;
		}
		return -1;
	}

	dupfd = syscall(__NR_pidfd_getfd, pidfd, fdnum, 0);
	if (dupfd < 0) {
		if (errno == ENOSYS && !warned_unavail) {
			diag_dbg("pidfd_getfd unavailable; fd inspection disabled");
			warned_unavail = 1;
		}
		close(pidfd);
		return -1;
	}

	close(pidfd);
	return dupfd;
#else
	(void)pid;
	(void)fdnum;
	return -1;
#endif
}

/*
 * probe_reuseport_fd - probe SO_REUSEPORT on one duplicated socket fd.
 * @fd: socket fd already duplicated into this process.
 *
 * Returns 1/0 on successful getsockopt, or -1 when unsupported/inaccessible.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static int probe_reuseport_fd(int fd)
{
	int val = 0;
	socklen_t len = sizeof(val);
	int rc;

	rc = getsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &val, &len);
	if (rc < 0)
		return -1;

	return val ? 1 : 0;
}

/*
 * collect_proc_inodes - build inode ownership map from /proc/<pid>/fd links.
 * @m: model receiving process entries and inode->process associations.
 *
 * This is best-effort and skips tasks/fds hidden by permissions or races.
 * Returns no value.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static void collect_proc_inodes(struct model *m)
{
	DIR *d;
	struct dirent *ent;

	d = opendir("/proc");
	if (!d)
		return;
	while ((ent = readdir(d))) {
		int pid;
		DIR *fds;
		struct dirent *fdent;
		char fdpath[64];
		struct process_info *p;

		if (ent->d_name[0] < '0' || ent->d_name[0] > '9')
			continue;
		pid = atoi(ent->d_name);
		if (pid <= 0)
			continue;
		p = add_process(m, pid);
		if (!p)
			continue;
		snprintf(fdpath, sizeof(fdpath), "/proc/%d/fd", pid);
		fds = opendir(fdpath);
		if (!fds)
			continue;
		while ((fdent = readdir(fds))) {
			char link[256], *s;
			ssize_t l;
			unsigned long inode;
			int fdnum;
			int dupfd;
			int reuseport;
			struct inode_proc *ip;

			if (fdent->d_name[0] == '.')
				continue;
			/* Resolve links in the directory we are enumerating. */
			l = readlinkat(dirfd(fds), fdent->d_name, link,
				       sizeof(link) - 1);
			if (l < 0)
				continue;
			link[l] = 0;
			/*
			 * procfs may expose socket links in kernel-dependent formats.
			 * Handle both common "socket:[inode]" and "[0000]:inode" forms.
			 */
			if (strncmp(link, "socket:[", 8) == 0) {
				s = link + 8;
			} else if (strncmp(link, "[0000]:", 7) == 0) {
				s = link + 7;
			} else {
				continue;
			}
			inode = strtoul(s, NULL, 10);
			if (!inode)
				continue;
			add_inode_proc(m, inode, p);

			fdnum = atoi(fdent->d_name);
			if (fdnum < 0)
				continue;
			dupfd = dup_target_fd(pid, fdnum);
			if (dupfd < 0)
				continue;
			/*
			 * Probe SO_REUSEPORT here because this is the only stage with both
			 * a live target socket and the owning process context; store the
			 * result on inode_proc so endpoint projection can reuse it later.
			 */
			reuseport = probe_reuseport_fd(dupfd);
			close(dupfd);
			if (reuseport != 1)
				continue;
			ip = lookup_inode(m, inode);
			if (ip)
				ip->reuseport |= 1;
		}
		closedir(fds);
	}
	closedir(d);
}

/*
 * lookup_inode - locate inode ownership entry in @m.
 * @m: model containing inode ownership map.
 * @inode: socket inode key to resolve.
 *
 * Returns mutable map entry pointer, or NULL if inode is unknown.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static struct inode_proc *lookup_inode(struct model *m, unsigned long inode)
{
	ssize_t idx;

	idx = inode_hash_find(m, inode);
	if (idx < 0)
		return NULL;
	return &m->inode_map[idx];
}

/* Socket and VSOCK endpoints keep one pointer per PID, in discovery order. */
static int add_endpoint_processes(struct endpoint *e, const struct inode_proc *ip)
{
	size_t i, j;

	for (i = 0; i < ip->n; i++) {
		for (j = 0; j < e->procs_n; j++)
			if (e->procs[j]->pid == ip->procs[i]->pid)
				break;
		if (j < e->procs_n)
			continue;
		if (e->procs_n == e->procs_cap &&
		    vec_grow((void **)&e->procs, &e->procs_cap, sizeof(*e->procs)))
			return -1;
		e->procs[e->procs_n++] = ip->procs[i];
	}
	return 0;
}

/*
 * add_endpoint - add/merge one inet or packet endpoint in @m.
 * @m: model receiving endpoint data.
 * @proto/@bind/@ifname/@ifaddr: copied strings for endpoint identity.
 * @port/@plane: endpoint attributes for rendering/grouping.
 * @attrs: wildcard/reuseport flags carried from parser/projection stages.
 * @ip: inode-owner mapping whose process pointers are attached to endpoint.
 *
 * Returns 0 on success, -1 on allocation failure.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static int add_endpoint(struct model *m, const char *proto, const char *bind,
	unsigned int port, enum plane_kind plane, const char *ifname,
	const char *ifaddr, const struct endpoint_attrs *attrs,
	struct inode_proc *ip)
{
	size_t i;
	struct endpoint *e;
	char label[256];

	if (strchr(bind, ':') && !str_is_bdaddr(bind))
		snprintf(label, sizeof(label), "%s:[%s]:%u", proto, bind, port);
	else
		snprintf(label, sizeof(label), "%s:%s:%u", proto, bind, port);

	for (i = 0; i < m->eps_n; i++) {
		e = &m->eps[i];
		if (strcmp(e->label, label) == 0 && strcmp(e->ifname, ifname) == 0 &&
		    strcmp(e->ifaddr, ifaddr) == 0) {
			e->reuseport |= attrs->reuseport;
			goto add_procs;
		}
	}
	if (m->eps_n == m->eps_cap && vec_grow((void **)&m->eps, &m->eps_cap,
	    sizeof(struct endpoint)))
		return -1;
	e = &m->eps[m->eps_n];
	memset(e, 0, sizeof(*e));
	e->proto = xstrdup(proto);
	e->bind = xstrdup(bind);
	e->label = xstrdup(label);
	e->port = port;
	e->vsock_cid = 0;
	e->plane = plane;
	e->ifname = xstrdup(ifname);
	e->ifaddr = xstrdup(ifaddr);
	e->wildcard_bind = attrs->wildcard;
	e->reuseport = attrs->reuseport;
	if (!e->proto || !e->bind || !e->label || !e->ifname || !e->ifaddr) {
		free_endpoint(e);
		memset(e, 0, sizeof(*e));
		return -1;
	}
	m->eps_n++;
add_procs:
	return add_endpoint_processes(e, ip);
}

#ifdef HAVE_NETCAP_VSOCK
/*
 * add_vsock_endpoint - add/merge one VSOCK endpoint in @m.
 * @m: model receiving endpoint data.
 * @type: socket type label (stream/dgram/seqpacket) copied into model.
 * @cid/@port: source CID/port; @cid may be VMADDR_CID_ANY.
 * @ip: inode-owner mapping whose process pointers are attached to endpoint.
 *
 * Returns 0 on success, -1 on allocation failure.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static int add_vsock_endpoint(struct model *m, const char *type,
	unsigned int cid, unsigned int port, struct inode_proc *ip)
{
	size_t i;
	struct endpoint *e;
	char label[128];
	char cidbuf[32];

	if (cid == VMADDR_CID_ANY)
		strcpy(cidbuf, "ANY");
	else
		snprintf(cidbuf, sizeof(cidbuf), "%u", cid);
	snprintf(label, sizeof(label), "%s:cid=%s:%u", type, cidbuf, port);

	for (i = 0; i < m->eps_n; i++) {
		e = &m->eps[i];
		if (e->plane == PLANE_VSOCK && strcmp(e->label, label) == 0)
			goto add_procs;
	}
	if (m->eps_n == m->eps_cap && vec_grow((void **)&m->eps, &m->eps_cap,
	    sizeof(struct endpoint)))
		return -1;
	e = &m->eps[m->eps_n];
	memset(e, 0, sizeof(*e));
	e->proto = xstrdup(type);
	e->bind = xstrdup(cidbuf);
	e->label = xstrdup(label);
	e->port = port;
	e->vsock_cid = cid;
	e->plane = PLANE_VSOCK;
	e->ifname = xstrdup("");
	e->ifaddr = xstrdup("");
	e->reuseport = 0;
	if (!e->proto || !e->bind || !e->label || !e->ifname || !e->ifaddr) {
		free_endpoint(e);
		memset(e, 0, sizeof(*e));
		return -1;
	}
	m->eps_n++;

add_procs:
	return add_endpoint_processes(e, ip);
}
#endif

/*
 * endpoint_to_ifaces - project one inet bind onto iface/address groupings.
 * @m: model containing iface inventory and endpoint lists.
 * @proto/@af/@bind/@port: socket identity from procfs/sock_diag.
 * @ip: inode-owner mapping used to attach owning processes.
 *
 * Wildcard binds expand across non-loopback ifaces in current netns.
 * Returns no value.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static void endpoint_to_ifaces(struct model *m, const char *proto, int af,
	const char *bind, unsigned int port, int reuseport,
	struct inode_proc *ip)
{
	size_t i, j;
	int wildcard = str_is_wildcard(af, bind);
	int multicast = str_is_multicast(af, bind);
	int matched = 0;
	struct endpoint_attrs attrs;

	attrs.wildcard = wildcard;
	attrs.reuseport = reuseport;

	for (i = 0; i < m->ifaces_n; i++) {
		struct iface_info *ifc = &m->ifaces[i];
		for (j = 0; j < ifc->addrs_n; j++) {
			if (ifc->addrs[j].af != af)
				continue;
			if (wildcard) {
				/*
				 * 0.0.0.0/:: listeners are treated as externally reachable; keep
				 * loopback out of this expansion to avoid duplicate exposure rows.
				 */
				if (strcmp(ifc->name, "lo") == 0)
					continue;
				add_endpoint(m, proto, bind, port, PLANE_INET_EXTERNAL,
					ifc->name, ifc->addrs[j].addr, &attrs, ip);
				matched = 1;
			} else if (strcmp(ifc->addrs[j].addr, bind) == 0) {
				enum plane_kind plane = str_is_loopback(af, bind) ?
					PLANE_INET_LOOPBACK : PLANE_INET_EXTERNAL;
				add_endpoint(m, proto, bind, port, plane, ifc->name,
					ifc->addrs[j].addr, &attrs, ip);
				matched = 1;
			}
		}
	}
	if (!matched)
		add_endpoint(m, proto, bind, port,
			str_is_loopback(af, bind) ?
			PLANE_INET_LOOPBACK : PLANE_INET_EXTERNAL,
			str_is_loopback(af, bind) ? "lo" :
			(multicast ? "multicast/group" : "unknown"),
			bind, &attrs, ip);
}

/*
 * parse_inet_file - parse one procfs inet socket table into endpoints.
 * @m: model receiving endpoint mappings.
 * @path: procfs table path (tcp/udp/raw variants).
 * @proto: protocol label used in rendered endpoint names.
 * @af: address family used to decode local bind addresses.
 *
 * Non-listeners/unowned sockets are skipped; output is best-effort.
 * Returns no value.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static void parse_inet_file(struct model *m, const char *path,
	const char *proto, int af)
{
	FILE *f;
	char line[512];
	int row = 0;

	f = fopen(path, "rte");
	if (!f)
		return;
	__fsetlocking(f, FSETLOCKING_BYCALLER);
	while (fgets(line, sizeof(line), f)) {
		unsigned int lport, rport, state;
		unsigned long txq, rxq, tmr, when, retr, inode;
		int d, uid, timeout;
		char laddrh[96], raddrh[96], more[128];
		char addr[INET6_ADDRSTRLEN];
		struct inode_proc *ip;
		more[0] = 0;
		if (!row++)
			continue;
		if (sscanf(line, "%d: %95[0-9A-Fa-f]:%X %95[0-9A-Fa-f]:%X %X "
			"%lX:%lX %lX:%lX %lX %d %d %lu %127s",
			&d, laddrh, &lport, raddrh, &rport, &state, &txq, &rxq,
			&tmr, &when, &retr, &uid, &timeout, &inode, more) < 14)
			continue;
		if ((strcmp(proto, "tcp") == 0 ||
		     strcmp(proto, "tcp6") == 0) && state != 0x0A)
			continue;
		if ((strcmp(proto, "udp") == 0 ||
		     strcmp(proto, "udp6") == 0 ||
		     strcmp(proto, "udplite") == 0 ||
		     strcmp(proto, "udplite6") == 0) && lport == 0)
			continue;
		ip = lookup_inode(m, inode);
		if (!ip)
			continue;
		if (af == AF_INET) {
			struct in_addr v4;
			unsigned int host;

			if (sscanf(laddrh, "%8x", &host) != 1)
				continue;
			/*
			 * procfs inet tables print IPv4 addresses as host-
			 * order hex. Assigning directly to s_addr keeps the
			 * bytes correct on both little- and big-endian systems.
			 */
			v4.s_addr = host;
			if (!inet_ntop(AF_INET, &v4, addr, sizeof(addr)))
				continue;
		} else {
			unsigned char bytes[16] = { 0 };
			int i;
			int ok = 1;
			if (strlen(laddrh) != 32)
				continue;
			for (i = 0; i < 4; i++) {
				uint32_t host;
				uint32_t net;

				if (sscanf(laddrh + (i * 8), "%8x", &host) != 1) {
					ok = 0;
					break;
				}
				net = htonl(host);
				memcpy(bytes + (i * 4), &net, sizeof(net));
			}
			if (!ok)
				continue;
			if (!inet_ntop(AF_INET6, bytes, addr, sizeof(addr)))
				continue;
		}
		endpoint_to_ifaces(m, proto, af, addr, lport, ip->reuseport, ip);
	}
	fclose(f);
}

/*
 * parse_packet_file - parse /proc/net/packet and add packet endpoints.
 * @m: model receiving packet-plane endpoint/process mappings.
 *
 * Visibility depends on current netns and procfs access permissions.
 * Returns no value.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static void parse_packet_file(struct model *m)
{
	FILE *f;
	char line[512], ifn[IF_NAMESIZE];
	int row = 0;

	f = fopen("/proc/net/packet", "rte");
	if (!f)
		return;
	__fsetlocking(f, FSETLOCKING_BYCALLER);
	while (fgets(line, sizeof(line), f)) {
		unsigned long sk, inode;
		unsigned int ref, type, proto, iface, r, rmem, uid;
		struct inode_proc *ip;
		struct endpoint_attrs attrs;
		char bind[64], addr[64], name[64];

		if (!row++)
			continue;
		if (sscanf(line, "%lX %u %u %X %u %u %u %u %lu",
			&sk, &ref, &type, &proto, &iface, &r, &rmem, &uid,
			&inode) < 9)
			continue;
		ip = lookup_inode(m, inode);
		if (!ip)
			continue;
		if (!if_indextoname(iface, ifn))
			strcpy(ifn, "unknown");
		snprintf(bind, sizeof(bind), "::");
		snprintf(addr, sizeof(addr), "ifindex:%u", iface);
		snprintf(name, sizeof(name), "packet");
		attrs.wildcard = 0;
		attrs.reuseport = 0;
		add_endpoint(m, name, bind, proto, PLANE_PACKET, ifn, addr,
			&attrs, ip);
	}
	fclose(f);
}

#ifdef HAVE_NETCAP_VSOCK
/*
 * parse_u32_hex_or_dec - parse @s as decimal or hexadecimal u32.
 * @s: numeric token from procfs/netlink text fields.
 * @out: destination value on successful parse.
 *
 * Returns 0 on success, -1 if @s is not a valid integer token.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
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
	/*
	 * The caller stores CIDs, ports, and similar procfs fields in u32s.
	 * Reject oversized text values so they cannot wrap into a false match.
	 */
	if (v > UINT_MAX)
		return -1;
	*out = (unsigned int)v;
	return 0;
}
#endif

/*
 * str_is_bdaddr - validate one string as a Bluetooth device address.
 * @s: text to classify.
 *
 * Returns non-zero for XX:XX:XX:XX:XX:XX tokens, else zero.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static int str_is_bdaddr(const char *s)
{
	size_t i;

	if (!s || strlen(s) != 17)
		return 0;
	for (i = 0; i < 17; i++) {
		char c = s[i];

		if ((i + 1) % 3 == 0) {
			if (c != ':')
				return 0;
			continue;
		}
		if (!((c >= '0' && c <= '9') ||
		      (c >= 'a' && c <= 'f') ||
		      (c >= 'A' && c <= 'F')))
			return 0;
	}
	return 1;
}

#ifdef HAVE_NETCAP_BLUETOOTH
/*
 * bt_format_addr - render one controller address into XX:XX:XX:XX:XX:XX.
 * @bdaddr: kernel Bluetooth address to stringify.
 * @addr: destination buffer.
 * @addr_sz: size of @addr in bytes.
 *
 * Returns no value.
 * Side effects/assumptions: Writes a formatted controller address into @addr.
 */
static void bt_format_addr(const bdaddr_t *bdaddr, char *addr, size_t addr_sz)
{
	snprintf(addr, addr_sz, "%02X:%02X:%02X:%02X:%02X:%02X",
		bdaddr->b[5], bdaddr->b[4], bdaddr->b[3],
		bdaddr->b[2], bdaddr->b[1], bdaddr->b[0]);
}

/*
 * bt_parse_adapter_dev_id - convert one sysfs adapter name into a device id.
 * @name: adapter name such as hci0.
 * @dev_id: destination device index.
 *
 * Returns 0 on success, -1 when @name is not an HCI adapter name.
 * Side effects/assumptions: Operates on in-memory data and does not change
 * system state.
 */
static int bt_parse_adapter_dev_id(const char *name, int *dev_id)
{
	unsigned int id;
	char tail;

	if (sscanf(name, "hci%u%c", &id, &tail) != 1 || id > UINT16_MAX)
		return -1;
	*dev_id = (int)id;
	return 0;
}

/*
 * bt_load_adapters - populate the adapter cache from sysfs and HCI ioctls.
 *
 * Adapter names come from /sys/class/bluetooth. Each name is resolved to its
 * BD_ADDR with HCIGETDEVINFO so later procfs parsing can map a single visible
 * adapter onto HCI and RFCOMM sockets. If the HCI socket or any ioctl fails,
 * the cache remains empty and lookups fall back to "unknown".
 * Returns no value.
 * Side effects/assumptions: Reads sysfs and issues HCI ioctls; it does not
 * change kernel configuration.
 */
static void bt_load_adapters(void)
{
	DIR *d;
	struct dirent *ent;
	struct bt_adapter_info tmp[BT_ADAPTER_CACHE_MAX];
	size_t tmp_n = 0;
	int fd;

	if (bt_adapters_loaded)
		return;
	bt_adapters_loaded = 1;

	d = opendir("/sys/class/bluetooth");
	if (!d)
		return;

	fd = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
	if (fd < 0)
		goto out;

	while ((ent = readdir(d))) {
		struct hci_dev_info di;
		size_t name_len;
		int dev_id;

		if (ent->d_name[0] == '.')
			continue;
		if (tmp_n == BT_ADAPTER_CACHE_MAX)
			break;
		/* Keep complete adapter names so cached identities stay exact. */
		name_len = strlen(ent->d_name);
		if (name_len >= sizeof(tmp[tmp_n].name))
			continue;
		if (bt_parse_adapter_dev_id(ent->d_name, &dev_id))
			continue;
		memset(&di, 0, sizeof(di));
		di.dev_id = (uint16_t)dev_id;
		if (ioctl(fd, HCIGETDEVINFO, &di) < 0)
			goto fail;
		memcpy(tmp[tmp_n].name, ent->d_name, name_len + 1);
		bt_format_addr(&di.bdaddr, tmp[tmp_n].addr,
			sizeof(tmp[tmp_n].addr));
		tmp_n++;
	}

	if (tmp_n) {
		memcpy(bt_adapters, tmp, tmp_n * sizeof(tmp[0]));
		bt_adapters_n = tmp_n;
	}
	close(fd);
out:
	closedir(d);
	return;
fail:
	close(fd);
	bt_adapters_n = 0;
	closedir(d);
}


/*
 * bt_resolve_single_adapter - pick the only adapter when the cache is unique.
 * @name: destination buffer for adapter name.
 * @name_sz: size of @name in bytes.
 * @addr: destination buffer for adapter BD_ADDR, or "*".
 * @addr_sz: size of @addr in bytes.
 *
 * Current /proc/net/hci and /proc/net/rfcomm output does not identify the
 * bound adapter. When exactly one controller is available, use it; otherwise
 * report the adapter as hci? and the bind address as *.
 * Returns no value.
 * Side effects/assumptions: Reads the in-memory adapter cache and does not
 * change system state.
 */
static void bt_resolve_single_adapter(char *name, size_t name_sz,
	char *addr, size_t addr_sz)
{
	bt_load_adapters();
	if (bt_adapters_n == 1) {
		snprintf(name, name_sz, "%s", bt_adapters[0].name);
		snprintf(addr, addr_sz, "%s", bt_adapters[0].addr);
		return;
	}

	snprintf(name, name_sz, "%s", BT_ADAPTER_HEURISTIC);
	snprintf(addr, addr_sz, "%s", BT_ADDR_UNKNOWN);
}

/*
 * HCI and RFCOMM expose the same generic socket columns:
 * sk RefCnt Rmem Wmem User Inode Parent. Neither supplies adapter identity,
 * channel, or listener state. Keep reporting owned sockets at port 0 and
 * use the existing single-adapter heuristic; do not guess from socket data.
 */
static void parse_bluetooth_file(struct model *m, const char *path,
				 const char *proto)
{
	FILE *f;
	char line[512];
	struct endpoint_attrs attrs = { 0, 0 };

	f = fopen(path, "rte");
	if (!f)
		return;
	__fsetlocking(f, FSETLOCKING_BYCALLER);
	while (fgets(line, sizeof(line), f)) {
		unsigned long sk, inode, parent;
		unsigned int refcnt, rmem, wmem, uid;
		char ifname[IF_NAMESIZE], ifaddr[32];
		struct inode_proc *ip;

		if (sscanf(line, "%lx %u %u %u %u %lu %lu",
			   &sk, &refcnt, &rmem, &wmem, &uid, &inode, &parent) != 7)
			continue;
		ip = lookup_inode(m, inode);
		if (!ip)
			continue;
		bt_resolve_single_adapter(ifname, sizeof(ifname),
					 ifaddr, sizeof(ifaddr));
		add_endpoint(m, proto, ifaddr, 0, PLANE_BLUETOOTH, ifname,
			     ifaddr, &attrs, ip);
	}
	fclose(f);
}
#endif

#ifdef HAVE_NETCAP_VSOCK
/*
 * parse_vsock_file - fallback VSOCK parser using /proc/net/vsock.
 * @m: model receiving parsed VSOCK endpoint/process mappings.
 *
 * Used when sock_diag support is unavailable or denied.
 * Returns no value.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static void parse_vsock_file(struct model *m)
{
	FILE *f;
	char line[512];

	f = fopen("/proc/net/vsock", "rte");
	if (!f)
		return;
	__fsetlocking(f, FSETLOCKING_BYCALLER);
	while (fgets(line, sizeof(line), f)) {
		char work[512];
		char *tok[24];
		char *save = NULL;
		char *local, *s, *sep;
		int tcnt = 0;
		unsigned long inode;
		unsigned int st, type, cid, port;
		struct inode_proc *ip;
		const char *kind;

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
		for (int i = 0; i < tcnt; i++) {
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

		if (type == SOCK_STREAM) {
			if (st != 0x0A)
				continue;
			kind = "stream";
		} else if (type == SOCK_SEQPACKET) {
			if (st != 0x0A)
				continue;
			kind = "seqpacket";
		} else if (type == SOCK_DGRAM) {
			if (port == 0)
				continue;
			kind = "dgram";
		} else {
			continue;
		}

		ip = lookup_inode(m, inode);
		if (!ip)
			continue;
		add_vsock_endpoint(m, kind, cid, port, ip);
	}
	fclose(f);
}

/*
 * vsock_type_to_name - map VSOCK socket type to display label.
 * @type: SOCK_* type value from kernel socket metadata.
 *
 * Returns static string label, or NULL for unknown/unsupported type.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static const char *vsock_type_to_name(unsigned int type)
{
	if (type == SOCK_STREAM)
		return "stream";
	if (type == SOCK_DGRAM)
		return "dgram";
	if (type == SOCK_SEQPACKET)
		return "seqpacket";
	return NULL;
}

#ifdef HAVE_LINUX_VM_SOCKETS_DIAG_H
/*
 * parse_vsock_diag_messages - consume VSOCK sock_diag dump replies.
 * @m: model receiving VSOCK endpoint/process mappings.
 * @fd: open NETLINK_SOCK_DIAG socket with pending VSOCK responses.
 *
 * Returns 0 on NLMSG_DONE, or -1 on malformed/error netlink messages.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static int parse_vsock_diag_messages(struct model *m, int fd)
{
	char buf[8192];
	ssize_t len;

	while (1) {
		len = recv(fd, buf, sizeof(buf), 0);
		if (len < 0) {
			if (errno == EINTR)
				continue;
			return -1;
		}
		if (len == 0)
			return -1;

		struct nlmsghdr *nlh;
		unsigned int rem;

		if (len > UINT_MAX)
			return -1;
		rem = (unsigned int)len;

		for (nlh = (struct nlmsghdr *)buf;
		     NLMSG_OK(nlh, rem);
		     nlh = NLMSG_NEXT(nlh, rem)) {
			struct vsock_diag_msg *r;
			struct inode_proc *ip;
			const char *kind;

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
			kind = vsock_type_to_name(r->vdiag_type);
			if (!kind)
				continue;

			if ((r->vdiag_type == SOCK_STREAM ||
			     r->vdiag_type == SOCK_SEQPACKET) &&
			    r->vdiag_state != TCP_LISTEN)
				continue;
			if (r->vdiag_type == SOCK_DGRAM &&
			    r->vdiag_src_port == 0)
				continue;

			ip = lookup_inode(m, r->vdiag_ino);
			if (!ip)
				continue;

			diag_dbg("vsock type=%u state=%u src=%u:%u dst=%u:%u ino=%u",
				r->vdiag_type, r->vdiag_state, r->vdiag_src_cid,
				r->vdiag_src_port, r->vdiag_dst_cid, r->vdiag_dst_port,
				r->vdiag_ino);
			add_vsock_endpoint(m, kind, r->vdiag_src_cid,
				r->vdiag_src_port, ip);
		}
	}
}

/*
 * parse_vsock_diag - request VSOCK listener dump via sock_diag netlink.
 * @m: model receiving parsed VSOCK endpoint/process mappings.
 *
 * Returns 0 on success, -1 with errno on permission/support/socket errors.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static int parse_vsock_diag(struct model *m)
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
	rc = parse_vsock_diag_messages(m, fd);
out:
	close(fd);
	return rc;
}
#else
/*
 * parse_vsock_diag - unsupported-build stub for VSOCK sock_diag path.
 * @m: unused model pointer.
 *
 * Returns -1 and sets errno=EOPNOTSUPP.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static int parse_vsock_diag(struct model *m)
{
	(void)m;
	errno = EOPNOTSUPP;
	return -1;
}
#endif
#endif

/*
 * parse_diag_messages - consume inet sock_diag replies for @proto/@af.
 * @m: model receiving parsed endpoint mappings.
 * @fd: open NETLINK_SOCK_DIAG socket with pending responses.
 * @proto: requested protocol (SCTP/DCCP).
 * @af: requested address family (AF_INET/AF_INET6).
 *
 * Returns 0 on NLMSG_DONE, or -1 on malformed/error netlink messages.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static int parse_diag_messages(struct model *m, int fd, int proto, int af)
{
	char buf[8192];
	ssize_t len;

	while (1) {
		len = recv(fd, buf, sizeof(buf), 0);
		if (len < 0) {
			if (errno == EINTR)
				continue;
			return -1;
		}
		if (len == 0)
			return -1;

		diag_dbg("recv proto=%d af=%d len=%zd", proto, af, len);

		struct nlmsghdr *nlh;
		unsigned int rem;

		if (len > UINT_MAX)
			return -1;
		rem = (unsigned int)len;

		for (nlh = (struct nlmsghdr *)buf;
		     NLMSG_OK(nlh, rem);
		     nlh = NLMSG_NEXT(nlh, rem)) {
			struct inet_diag_msg *r;
			char addr[INET6_ADDRSTRLEN];
			unsigned int port;
			struct inode_proc *ip;

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
				diag_dbg("error proto=%d af=%d err=%d", proto, af,
					-e->error);
				return -1;
			}
			if (nlh->nlmsg_len < NLMSG_LENGTH(sizeof(*r)))
				continue;

			r = NLMSG_DATA(nlh);
			ip = lookup_inode(m, r->idiag_inode);
			if (!ip)
				continue;
			port = ntohs(r->id.idiag_sport);
			if (!port)
				continue;

			if (af == AF_INET) {
				if (!inet_ntop(AF_INET, r->id.idiag_src, addr,
				    sizeof(addr)))
					continue;
			} else {
				if (!inet_ntop(AF_INET6, r->id.idiag_src, addr,
				    sizeof(addr)))
					continue;
			}
			endpoint_to_ifaces(m,
				proto == IPPROTO_SCTP ? "sctp" : "dccp",
				af, addr, port, ip->reuseport, ip);
		}
	}
}

/*
 * parse_diag_for_proto_af - issue one inet sock_diag listener dump request.
 * @m: model receiving parsed endpoint mappings.
 * @proto: protocol selector for the request.
 * @af: address family selector for the request.
 *
 * Best-effort helper; failures are tolerated by callers.
 * Returns no value.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static void parse_diag_for_proto_af(struct model *m, int proto, int af)
{
	struct {
		struct nlmsghdr nlh;
		struct inet_diag_req_v2 req;
	} req;
	struct sockaddr_nl sa;
	int fd;

	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_SOCK_DIAG);
	if (fd < 0)
		return;

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

	diag_dbg("send proto=%d af=%d len=%u", proto, af,
		req.nlh.nlmsg_len);
	if (send(fd, &req, req.nlh.nlmsg_len, 0) < 0)
		goto out;
	parse_diag_messages(m, fd, proto, af);
out:
	close(fd);
}

/*
 * parse_diag_listeners - collect SCTP/DCCP listeners via sock_diag.
 * @m: model receiving discovered listener endpoints.
 *
 * Returns no value.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static void parse_diag_listeners(struct model *m)
{
	parse_diag_for_proto_af(m, IPPROTO_SCTP, AF_INET);
	parse_diag_for_proto_af(m, IPPROTO_SCTP, AF_INET6);
	parse_diag_for_proto_af(m, IPPROTO_DCCP, AF_INET);
	parse_diag_for_proto_af(m, IPPROTO_DCCP, AF_INET6);
}

/*
 * collect_endpoints - gather all endpoint classes into @m.
 * @m: model receiving inet, diag, packet, Bluetooth, and vsock mappings.
 *
 * Data source is current netns procfs/netlink visibility.
 * Returns no value.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static void collect_endpoints(struct model *m)
{
	parse_inet_file(m, "/proc/net/tcp", "tcp", AF_INET);
	parse_inet_file(m, "/proc/net/tcp6", "tcp6", AF_INET6);
	parse_inet_file(m, "/proc/net/udp", "udp", AF_INET);
	parse_inet_file(m, "/proc/net/udp6", "udp6", AF_INET6);
	parse_inet_file(m, "/proc/net/udplite", "udplite", AF_INET);
	parse_inet_file(m, "/proc/net/udplite6", "udplite6", AF_INET6);
	parse_inet_file(m, "/proc/net/raw", "raw", AF_INET);
	parse_inet_file(m, "/proc/net/raw6", "raw6", AF_INET6);
	parse_diag_listeners(m);
	parse_packet_file(m);
#ifdef HAVE_NETCAP_VSOCK
	if (parse_vsock_diag(m) < 0) {
		diag_dbg("vsock diag unavailable (%s), falling back to /proc",
			strerror(errno));
		parse_vsock_file(m);
	}
#endif
#ifdef HAVE_NETCAP_BLUETOOTH
	parse_bluetooth_file(m, "/proc/net/rfcomm", "rfcomm");
	parse_bluetooth_file(m, "/proc/net/hci", "hci");
#endif
}

/*
 * endpoint_cmp - qsort comparator for stable endpoint grouping.
 * @a: pointer to first endpoint element.
 * @b: pointer to second endpoint element.
 *
 * Sort order is plane, interface name, interface address, then label.
 * Returns negative/zero/positive qsort ordering result.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static int endpoint_cmp(const void *a, const void *b)
{
	const struct endpoint *ea = a, *eb = b;
	/*
	 * Sort by plane for single-pass render grouping, then interface/protocol,
	 * then bind (wildcards first), then port, then ifaddr/label for stability.
	 */
	if (ea->plane != eb->plane)
		return ea->plane - eb->plane;
	if (strcmp(ea->ifname, eb->ifname) != 0)
		return strcmp(ea->ifname, eb->ifname);
	if (strcmp(ea->proto, eb->proto) != 0)
		return strcmp(ea->proto, eb->proto);
	if (bind_sort_cmp(ea->bind, eb->bind) != 0)
		return bind_sort_cmp(ea->bind, eb->bind);
	if (ea->port != eb->port)
		return ea->port < eb->port ? -1 : 1;
	if (strcmp(ea->ifaddr, eb->ifaddr) != 0)
		return strcmp(ea->ifaddr, eb->ifaddr);
	return strcmp(ea->label, eb->label);
}


/*
 * format_bind_node - normalize bind text for tree display.
 * @dst: output buffer receiving display text.
 * @dst_sz: size of @dst in bytes.
 * @bind: endpoint bind address to format.
 *
 * Returns no value.
 * Side effects/assumptions: Writes formatted bind label into @dst.
 */
static void format_bind_node(char *dst, size_t dst_sz, const char *bind)
{
	if (strcmp(bind, "0.0.0.0") == 0 || strcmp(bind, "::") == 0)
		snprintf(dst, dst_sz, "*");
	else if (strchr(bind, ':') && !str_is_bdaddr(bind))
		snprintf(dst, dst_sz, "[%s]", bind);
	else
		snprintf(dst, dst_sz, "%s", bind);
}

/*
 * bind_sort_cmp - compare bind addresses with wildcard-first ordering.
 * @a: first bind address.
 * @b: second bind address.
 *
 * Returns negative/zero/positive ordering suitable for qsort tie-breaks.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static int bind_sort_cmp(const char *a, const char *b)
{
	int a_star = strcmp(a, "0.0.0.0") == 0 || strcmp(a, "::") == 0 ||
		strcmp(a, "*") == 0;
	int b_star = strcmp(b, "0.0.0.0") == 0 || strcmp(b, "::") == 0 ||
		strcmp(b, "*") == 0;

	if (a_star != b_star)
		return a_star ? -1 : 1;
	return strcmp(a, b);
}

/* Unknown defense states stay uncolored rather than implying protection. */
static const char *defense_color(const char *value)
{
	if (!strcmp(value, "yes") || !strcmp(value, "filter") ||
	    !strcmp(value, "strict"))
		return COLOR_GREEN;
	if (!strcmp(value, "no") || !strcmp(value, "disabled"))
		return COLOR_YELLOW;
	return "";
}

/*
 * render_tree_process_details - emit one process subtree under an endpoint.
 * @prefix: parent tree prefix for process node.
 * @is_last: non-zero when process node is final sibling.
 * @p: process metadata to render.
 * @e: endpoint context supplying endpoint-derived flags.
 * @width: display width used for wrapping rendered nodes.
 *
 * Returns no value.
 * Side effects/assumptions: Writes formatted tree output to stdout.
 */
static void render_tree_process_details(const char *prefix,
						int is_last,
					struct process_info *p,
					const struct endpoint *e,
					int width)
{
	char pfx_child[256], pfx_def[256], pfx_flags[256];
	char line[4096];
	const struct defense_info *d = &p->defenses;
	const struct {
		const char *name, *value, *color;
	} defenses[] = {
		{ DEFENSES_RUNS_AS_KEY, d->runs_as_nonroot,
		  defense_color(d->runs_as_nonroot) },
		{ "no_new_privs", d->no_new_privs, defense_color(d->no_new_privs) },
		{ "seccomp", d->seccomp, defense_color(d->seccomp) },
		{ "lsm", d->lsm_label, !d->lsm_label || !d->lsm_label[0] ? "" :
		  strstr(d->lsm_label, "unconfined_t") ? COLOR_ORANGE : COLOR_GREEN },
	};
	size_t def_n = d->lsm_label ? 4 : 3;
	size_t ai;
	unsigned int flags = process_flags(p, e);
	enum cap_severity priv_sev = caps_worst_severity(p->caps);

	snprintf(line, sizeof(line), "%s (pid=%d uid=%u%s%s%s%s)",
		p->comm, p->pid, (unsigned int)p->uid,
		p->exe ? " exe=" : "",
		p->exe ? p->exe : "",
		p->unit ? " unit=" : "",
		p->unit ? p->unit : "");
	proc_tree_print_node(prefix, is_last, line, width);
	proc_tree_build_child_prefix(pfx_child, sizeof(pfx_child), prefix, is_last);

	if (strcmp(p->caps, "(full)") == 0 || strcmp(p->caps, "(none)") == 0) {
		const char *c = strcmp(p->caps, "(full)") == 0 ? COLOR_ORANGE :
			COLOR_GREEN;
		if (use_color)
			snprintf(line, sizeof(line), "caps: %s%s%s", c, p->caps,
				COLOR_RESET);
		else
			snprintf(line, sizeof(line), "caps: %s", p->caps);
	} else {
		char capsbuf[3072] = "";
		char *tmp = xstrdup(p->caps);
		char *save = NULL;
		char *tok;
		int first = 1;

		if (!tmp) {
			snprintf(line, sizeof(line), "caps: %s", p->caps);
		} else {
			for (tok = strtok_r(tmp, ",", &save); tok;
			     tok = strtok_r(NULL, ",", &save)) {
				char part[256];
				char *t = tok;
				enum cap_severity sev;
				while (*t == ' ')
					t++;
				sev = cap_name_severity(t);
				if (use_color && sev_color(sev))
					snprintf(part, sizeof(part), "%s%s%s",
						sev_color(sev), t, COLOR_RESET);
				else
					snprintf(part, sizeof(part), "%s", t);
				if (!first)
					strncat(capsbuf, ", ", sizeof(capsbuf)-strlen(capsbuf)-1);
				strncat(capsbuf, part, sizeof(capsbuf)-strlen(capsbuf)-1);
				first = 0;
			}
			free(tmp);
			if (p->ambient_present) {
				if (use_color)
					strncat(capsbuf, " [\033[38;5;208mambient-present\033[0m]",
						sizeof(capsbuf)-strlen(capsbuf)-1);
				else
					strncat(capsbuf, " [ambient-present]",
						sizeof(capsbuf)-strlen(capsbuf)-1);
			}
			if (p->open_ended_bounding) {
				if (use_color && strcmp(p->caps, "(full)") != 0 &&
				    caps_contains_token(p->caps, "setpcap"))
					strncat(capsbuf,
						" [\033[38;5;208mopen-ended-bounding\033[0m]",
						sizeof(capsbuf) - strlen(capsbuf) - 1);
				else
					strncat(capsbuf, " [open-ended-bounding]",
						sizeof(capsbuf) - strlen(capsbuf) - 1);
			}
			snprintf(line, sizeof(line), "caps: %s", capsbuf);
		}
	}
	proc_tree_print_node(pfx_child, 0, line, width);

	if (p->ambient_caps) {
		if (use_color)
			snprintf(line, sizeof(line), "ambient: %s%s%s", COLOR_ORANGE,
				p->ambient_caps, COLOR_RESET);
		else
			snprintf(line, sizeof(line), "ambient: %s", p->ambient_caps);
		proc_tree_print_node(pfx_child, 0, line, width);
	}

	proc_tree_print_node(pfx_child, 0, "defenses", width);
	proc_tree_build_child_prefix(pfx_def, sizeof(pfx_def), pfx_child, 0);
	for (ai = 0; ai < def_n; ai++) {
		char defense[512];
		const char *color = use_color ? defenses[ai].color : "";

		snprintf(defense, sizeof(defense), "%s: %s%s%s",
			 defenses[ai].name, color, defenses[ai].value,
			 color[0] ? COLOR_RESET : "");
		proc_tree_print_node(pfx_def, ai + 1 == def_n, defense, width);
	}

	proc_tree_print_node(pfx_child, 1, "flags", width);
	proc_tree_build_child_prefix(pfx_flags, sizeof(pfx_flags), pfx_child, 1);
	print_flag_nodes(pfx_flags, width, flags, priv_sev);
}


/*
 * render_json_process - emit one process object inside endpoint JSON arrays.
 * @p: process metadata record to serialize.
 * @ep: endpoint context contributing endpoint-related flags.
 * @indent: indentation prefix already prepared by caller.
 *
 * Returns no value.
 * Side effects/assumptions: Writes JSON fragments to stdout.
 */
static void render_json_process(struct process_info *p,
				const struct endpoint *ep,
				const char *indent)
{
	unsigned int flags = process_flags(p, ep);
	int firstf = 1;
	size_t i;

	printf("%s{\"comm\": ", indent);
	json_escape(p->comm);
	if (p->exe) {
		printf(", \"exe\": ");
		json_escape(p->exe);
	}
	printf(", \"pid\": %d, \"uid\": %u", p->pid,
		(unsigned int)p->uid);
	if (p->unit) {
		printf(", \"unit\": ");
		json_escape(p->unit);
	}
	printf(", \"caps\": ");
	json_escape(p->caps);
	printf(", \"ambient_present\": %s",
		p->ambient_present ? "true" : "false");
	if (p->ambient_caps) {
		printf(", \"ambient_caps\": ");
		json_escape(p->ambient_caps);
	}
	printf(", \"open_ended_bounding\": %s",
		p->open_ended_bounding ? "true" : "false");
	printf(", \"defenses\": {\"" DEFENSES_RUNS_AS_KEY "\": ");
	json_escape(p->defenses.runs_as_nonroot);
	printf(", \"no_new_privs\": ");
	json_escape(p->defenses.no_new_privs);
	printf(", \"seccomp\": ");
	json_escape(p->defenses.seccomp);
	if (p->defenses.lsm_label) {
		printf(", \"lsm\": ");
		json_escape(p->defenses.lsm_label);
	}
	printf("}, \"flags\": [");

	for (i = 0; i < sizeof(flag_names) / sizeof(flag_names[0]); i++) {
		if (!(flags & flag_names[i].bit))
			continue;
		if (!firstf)
			printf(", ");
		json_escape(flag_names[i].name);
		firstf = 0;
	}
	printf("]}");
}


/* Endpoints in a displayed port group can repeat owners across ifaddrs. */
static void render_tree_processes(const char *prefix, int width,
				 const struct endpoint *eps, size_t count)
{
	struct pidset seen;
	struct process_info *pending = NULL;
	const struct endpoint *pending_ep = NULL;
	size_t i, j;

	if (pidset_init(&seen))
		return;
	for (i = 0; i < count; i++) {
		const struct endpoint *e = &eps[i];

		for (j = 0; j < e->procs_n; j++) {
			struct process_info *p = e->procs[j];

			if (pidset_test_and_add(&seen, p->pid))
				continue;
			/*
			 * A later endpoint may contain only duplicate owners.
			 * Defer one row so "last" means last visible process,
			 * not last entry in the underlying endpoint arrays.
			 */
			if (pending)
				render_tree_process_details(prefix, 0, pending,
							    pending_ep, width);
			pending = p;
			pending_ep = e;
		}
	}
	if (pending)
		render_tree_process_details(prefix, 1, pending, pending_ep, width);
	pidset_free(&seen);
}

/*
 * endpoint_cmp keeps each plane/interface/protocol/bind/port contiguous.
 * Walk those ranges directly; interface addresses remain separate in the
 * model for JSON, but share one owner list per displayed tree port.
 */
static void render_tree(struct model *m)
{
	size_t i, plane_end;
	int width = proc_output_width();

	if (m->eps_n > 1)
		qsort(m->eps, m->eps_n, sizeof(struct endpoint), endpoint_cmp);
	for (i = 0; i < m->eps_n; i = plane_end) {
		enum plane_kind plane = m->eps[i].plane;
		const char *plane_name = plane == PLANE_INET_EXTERNAL ?
			"INET (external)" :
			plane == PLANE_INET_LOOPBACK ? "INET (loopback)" :
			plane == PLANE_PACKET ? PLANE_PACKET_NAME :
			plane == PLANE_BLUETOOTH ? PLANE_BLUETOOTH_NAME : "VSOCK";
		char pfx_iface[256];
		size_t j, iface_end;

		for (plane_end = i + 1; plane_end < m->eps_n &&
		     m->eps[plane_end].plane == plane; plane_end++)
			;
		proc_tree_print_node("", plane_end == m->eps_n, plane_name, width);
		proc_tree_build_child_prefix(pfx_iface, sizeof(pfx_iface), "",
					    plane_end == m->eps_n);
		if (plane == PLANE_VSOCK) {
			/* VSOCK has no interface/bind hierarchy. */
			for (j = i; j < plane_end; j++) {
				char pfx_proc[256];
				int last = j + 1 == plane_end;

				proc_tree_print_node(pfx_iface, last, m->eps[j].label,
						     width);
				proc_tree_build_child_prefix(pfx_proc, sizeof(pfx_proc),
							    pfx_iface, last);
				render_tree_processes(pfx_proc, width, &m->eps[j], 1);
			}
			continue;
		}
		for (j = i; j < plane_end; j = iface_end) {
			char pfx_proto[256];
			size_t k, proto_end;

			for (iface_end = j + 1; iface_end < plane_end &&
			     !strcmp(m->eps[iface_end].ifname, m->eps[j].ifname);
			     iface_end++)
				;
			proc_tree_print_node(pfx_iface, iface_end == plane_end,
					     m->eps[j].ifname, width);
			proc_tree_build_child_prefix(pfx_proto, sizeof(pfx_proto),
						    pfx_iface, iface_end == plane_end);
			for (k = j; k < iface_end; k = proto_end) {
				const char *proto = m->eps[k].proto;
				char pfx_bind[256], proto_line[64];
				size_t b, bind_end;
				const char *color = use_color &&
					(!strcmp(proto, "raw") || !strcmp(proto, "raw6") ||
					 !strcmp(proto, "packet") || !strcmp(proto, "hci")) ?
					COLOR_YELLOW : "";

				for (proto_end = k + 1; proto_end < iface_end &&
				     !strcmp(m->eps[proto_end].proto, proto); proto_end++)
					;
				snprintf(proto_line, sizeof(proto_line), "%s%s%s",
					 color, proto, color[0] ? COLOR_RESET : "");
				proc_tree_print_node(pfx_proto, proto_end == iface_end,
						     proto_line, width);
				proc_tree_build_child_prefix(pfx_bind, sizeof(pfx_bind),
							    pfx_proto, proto_end == iface_end);
				for (b = k; b < proto_end; b = bind_end) {
					char pfx_port[256], bind_line[128];
					size_t p, port_end;

					for (bind_end = b + 1; bind_end < proto_end &&
					     !strcmp(m->eps[bind_end].bind, m->eps[b].bind);
					     bind_end++)
						;
					format_bind_node(bind_line, sizeof(bind_line),
							 m->eps[b].bind);
					proc_tree_print_node(pfx_bind, bind_end == proto_end,
							     bind_line, width);
					proc_tree_build_child_prefix(pfx_port, sizeof(pfx_port),
							pfx_bind, bind_end == proto_end);
					for (p = b; p < bind_end; p = port_end) {
						char pfx_proc[256], port_line[64];

						for (port_end = p + 1; port_end < bind_end &&
						     m->eps[port_end].port == m->eps[p].port;
						     port_end++)
							;
						snprintf(port_line, sizeof(port_line), "%u",
							 m->eps[p].port);
						proc_tree_print_node(pfx_port, port_end == bind_end,
								     port_line, width);
						proc_tree_build_child_prefix(pfx_proc, sizeof(pfx_proc),
							pfx_port, port_end == bind_end);
						render_tree_processes(pfx_proc, width, &m->eps[p],
								      port_end - p);
					}
				}
			}
		}
	}
}

/*
 * render_interfaces_text - print interface inventory in human-readable form.
 * @m: model whose interface inventory is rendered.
 *
 * Returns no value.
 */
static void render_interfaces_text(struct model *m)
{
	size_t i;

	sort_interfaces(m);
	for (i = 0; i < m->ifaces_n; i++)
		printf("%s\n", m->ifaces[i].name);
}

/*
 * json_escape - write @s as a quoted JSON string to stdout.
 * @s: UTF-8/text string to emit as one JSON string literal.
 *
 * Control characters and quotes are escaped; caller handles separators.
 * Returns no value.
 * Side effects/assumptions: Writes to stdout and may read procfs/netns
 * state indirectly via caller-supplied model-derived strings.
 */
static void json_escape(const char *s)
{
	const unsigned char *p = (const unsigned char *)s;
	putchar('"');
	for (; *p; p++) {
		if (*p == '"')
			fputs("\\\"", stdout);
		else if (*p == '\\')
			fputs("\\\\", stdout);
		else if (*p < 0x20)
			printf("\\u%04x", *p);
		else
			putchar(*p);
	}
	putchar('"');
}

/*
 * render_json - print machine-readable advanced report JSON to stdout.
 * @m: model to render; endpoint array is sorted in place before printing.
 *
 * Returns no value.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static void render_json(struct model *m)
{
	size_t i, j, k, l;
	int first_plane = 1;

	if (m->eps_n > 1)
		qsort(m->eps, m->eps_n, sizeof(struct endpoint), endpoint_cmp);
	puts("{");
	puts("  \"schema_version\": 1,");
	puts("  \"planes\": [");
	for (i = 0; i < PLANE_COUNT; i++) {
		const char *pname = i == PLANE_INET_EXTERNAL ? "INET" :
			i == PLANE_INET_LOOPBACK ? "INET" :
			i == PLANE_PACKET ? PLANE_PACKET_NAME :
			i == PLANE_BLUETOOTH ? PLANE_BLUETOOTH_NAME : "VSOCK";
		const char *scope = i == PLANE_INET_EXTERNAL ? "external" :
			i == PLANE_INET_LOOPBACK ? "loopback" : NULL;
		struct strset seen_ifaces = { 0 };
		int first_if = 1;
#ifdef HAVE_NETCAP_VSOCK
		int first_vsock = 1;
#endif

#ifndef HAVE_NETCAP_VSOCK
		if (i == PLANE_VSOCK)
			continue;
#endif
		if (!first_plane)
			puts(",");
		first_plane = 0;
		printf("    {\"name\": ");
		json_escape(pname);
		if (scope) {
			printf(", \"scope\": ");
			json_escape(scope);
		}
#ifdef HAVE_NETCAP_VSOCK
		if (i == PLANE_VSOCK)
			puts(", \"endpoints\": [");
		else
#endif
			puts(", \"ifaces\": [");

#ifdef HAVE_NETCAP_VSOCK
		if (i == PLANE_VSOCK) {
			for (j = 0; j < m->eps_n; j++) {
				struct endpoint *ep = &m->eps[j];

				if (ep->plane != (enum plane_kind)i)
					continue;
				if (!first_vsock)
					puts(",");
				first_vsock = 0;
				printf("      {\"label\": ");
				json_escape(ep->label);
				printf(", \"vsock_type\": ");
				json_escape(ep->proto);
				printf(", \"cid\": ");
				if (ep->vsock_cid == VMADDR_CID_ANY)
					json_escape("ANY");
				else
					printf("%u", ep->vsock_cid);
				printf(", \"port\": %u", ep->port);
				puts(", \"processes\": [");
				for (size_t pi = 0; pi < ep->procs_n; pi++) {
					struct process_info *p = ep->procs[pi];

					render_json_process(p, ep, "        ");
					if (pi + 1 != ep->procs_n)
						puts(",");
					else
						putchar('\n');
				}
				puts("      ]}");
			}
			puts("    ]}");
			strset_free(&seen_ifaces);
			continue;
		}
#endif

		for (j = 0; j < m->eps_n; j++) {
			struct strset seen_addrs = { 0 };
			const char *ifn = m->eps[j].ifname;
			int first_addr = 1;
			int seen;

			if (m->eps[j].plane != (enum plane_kind)i)
				continue;
			seen = strset_add(&seen_ifaces, ifn);
			if (seen < 0) {
				strset_free(&seen_addrs);
				continue;
			}
			if (seen == 0)
				continue;
			if (!first_if)
				puts(",");
			first_if = 0;
			printf("      {\"name\": ");
			json_escape(ifn);
			puts(", \"addrs\": [");

			for (k = 0; k < m->eps_n; k++) {
				const char *ifa = m->eps[k].ifaddr;
				int first_ep = 1;
				int addr_seen;
				if (m->eps[k].plane != (enum plane_kind)i ||
				    strcmp(m->eps[k].ifname, ifn) != 0)
					continue;
				addr_seen = strset_add(&seen_addrs, ifa);
				if (addr_seen < 0)
					break;
				if (addr_seen == 0)
					continue;
				if (!first_addr)
					puts(",");
				first_addr = 0;
				printf("        {\"addr\": ");
				json_escape(ifa);
				puts(", \"endpoints\": [");
				for (l = 0; l < m->eps_n; l++) {
					struct endpoint *ep = &m->eps[l];
					if (ep->plane != (enum plane_kind)i ||
					    strcmp(ep->ifname, ifn) != 0 ||
					    strcmp(ep->ifaddr, ifa) != 0)
						continue;
					if (!first_ep)
						puts(",");
					first_ep = 0;
					printf("          {\"label\": ");
					json_escape(ep->label);
					printf(", \"proto\": ");
					json_escape(ep->proto);
					printf(", \"bind\": ");
					json_escape(ep->bind);
					printf(", \"port\": %u", ep->port);
					puts(", \"processes\": [");
					for (size_t pi = 0; pi < ep->procs_n; pi++) {
						struct process_info *p = ep->procs[pi];

						render_json_process(p, ep, "            ");
						if (pi + 1 != ep->procs_n)
							puts(",");
						else
							putchar('\n');
					}
					puts("          ]}");
				}
				puts("        ]}");
			}
			puts("      ]}");
			strset_free(&seen_addrs);
		}
		puts("    ]}");
		strset_free(&seen_ifaces);
	}
	puts("  ]");
	puts("}");
}

/*
 * render_interfaces_json - print interface inventory as JSON.
 * @m: model whose interface inventory is rendered.
 *
 * Returns no value.
 */
static void render_interfaces_json(struct model *m)
{
	size_t i;

	sort_interfaces(m);
	puts("{");
	puts("  \"schema_version\": 1,");
	puts("  \"interfaces\": [");
	for (i = 0; i < m->ifaces_n; i++) {
		printf("    ");
		json_escape(m->ifaces[i].name);
		printf("%s\n", i + 1 == m->ifaces_n ? "" : ",");
	}
	puts("  ]");
	puts("}");
}

/*
 * free_endpoint - free one endpoint and all owned dynamic fields.
 * @e: endpoint entry pointer, or NULL.
 *
 * Returns no value.
 */
static void free_endpoint(struct endpoint *e)
{
	if (!e)
		return;
	free(e->proto);
	free(e->bind);
	free(e->label);
	free(e->ifname);
	free(e->ifaddr);
	free(e->procs);
}

/*
 * filter_model_interface - keep only one named interface in @m.
 * @m: model container to prune in place.
 * @ifname: interface name to retain.
 *
 * Returns no value.
 */
static void filter_model_interface(struct model *m, const char *ifname)
{
	size_t i, out;

	if (!m || !ifname)
		return;

	for (i = 0, out = 0; i < m->ifaces_n; i++) {
		if (strcmp(m->ifaces[i].name, ifname) == 0) {
			if (out != i)
				m->ifaces[out] = m->ifaces[i];
			out++;
			continue;
		}
		free(m->ifaces[i].name);
		for (size_t j = 0; j < m->ifaces[i].addrs_n; j++)
			free(m->ifaces[i].addrs[j].addr);
		free(m->ifaces[i].addrs);
	}
	m->ifaces_n = out;

	for (i = 0, out = 0; i < m->eps_n; i++) {
		if (strcmp(m->eps[i].ifname, ifname) == 0) {
			if (out != i)
				m->eps[out] = m->eps[i];
			out++;
			continue;
		}
		free_endpoint(&m->eps[i]);
	}
	m->eps_n = out;
}

/*
 * free_process - free one process_info and all owned dynamic fields.
 * @p: process entry pointer, or NULL.
 *
 * Returns no value.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static void free_process(struct process_info *p)
{
	if (!p)
		return;
	free(p->comm);
	free(p->exe);
	free(p->unit);
	free(p->caps);
	free(p->ambient_caps);
	free(p->defenses.runs_as_nonroot);
	free(p->defenses.no_new_privs);
	free(p->defenses.seccomp);
	free(p->defenses.lsm_label);
	free(p);
}

/*
 * free_model - free all heap allocations referenced by @m.
 * @m: model container whose internal arrays/strings are released.
 *
 * Returns no value.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
static void free_model(struct model *m)
{
	size_t i, j;
	for (i = 0; i < m->ifaces_n; i++) {
		free(m->ifaces[i].name);
		for (j = 0; j < m->ifaces[i].addrs_n; j++)
			free(m->ifaces[i].addrs[j].addr);
		free(m->ifaces[i].addrs);
	}
	free(m->ifaces);
	for (i = 0; i < m->procs_n; i++)
		free_process(m->procs[i]);
	free(m->procs);
	for (i = 0; i < m->inode_n; i++)
		free(m->inode_map[i].procs);
	free(m->inode_map);
	free(m->inode_slots);
	for (i = 0; i < m->eps_n; i++)
		free_endpoint(&m->eps[i]);
	free(m->eps);
}

/*
 * netcap_advanced_main - entry point for "netcap --advanced" mode.
 * @opts: parsed options; must be non-NULL and have @advanced set.
 *
 * Returns 0 after rendering advanced output, or 1 when advanced mode
 * is not requested.
 * Side effects/assumptions: Reads procfs/netlink in the current network
 * namespace, prints to stdout/stderr, and root is typically needed for a
 * fuller process-to-socket ownership mapping.
 */
int netcap_advanced_main(const struct netcap_opts *opts)
{
	struct model m;

	if (!opts || !opts->advanced)
		return 1;
	memset(&m, 0, sizeof(m));
	if (collect_interfaces(&m) != 0) {
		fprintf(stderr, "warning: failed to enumerate interfaces\n");
	}
	if (opts->interface)
		filter_model_interface(&m, opts->interface);
	if (opts->list_interfaces) {
		if (opts->json)
			render_interfaces_json(&m);
		else
			render_interfaces_text(&m);
		free_model(&m);
		return 0;
	}
	collect_proc_inodes(&m);
	collect_endpoints(&m);
	if (opts->interface)
		filter_model_interface(&m, opts->interface);
	use_color = !opts->json && !opts->no_color && isatty(STDOUT_FILENO);
	if (opts->json)
		render_json(&m);
	else
		render_tree(&m);
	free_model(&m);
	return 0;
}
