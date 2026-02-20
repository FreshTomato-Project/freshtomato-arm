/*
 * (C) 2005-2012 by Pablo Neira Ayuso <pablo@netfilter.org>
 * (C) 2012 by Intra2net AG <http://www.intra2net.com>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Note:
 *	Yes, portions of this code has been stolen from iptables ;)
 *	Special thanks to the the Netfilter Core Team.
 *	Thanks to Javier de Miguel Rodriguez <jmiguel at talika.eii.us.es>
 *	for introducing me to advanced firewalling stuff.
 *
 *						--pablo 13/04/2005
 *
 * 2005-04-16 Harald Welte <laforge@netfilter.org>: 
 * 	Add support for conntrack accounting and conntrack mark
 * 2005-06-23 Harald Welte <laforge@netfilter.org>:
 * 	Add support for expect creation
 * 2005-09-24 Harald Welte <laforge@netfilter.org>:
 * 	Remove remaints of "-A"
 * 2007-04-22 Pablo Neira Ayuso <pablo@netfilter.org>:
 * 	Ported to the new libnetfilter_conntrack API
 * 2008-04-13 Pablo Neira Ayuso <pablo@netfilter.org>:
 *	Way more flexible update and delete operations
 *
 * Part of this code has been funded by Sophos Astaro <http://www.sophos.com>
 */

#include "conntrack.h"

#include <stdio.h>
#include <assert.h>
#include <getopt.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <inttypes.h>
#include <dirent.h>
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#include <signal.h>
#include <string.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libmnl/libmnl.h>
#include <linux/netfilter/nf_conntrack_common.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

struct nfct_mnl_socket {
	struct mnl_socket	*mnl;
	uint32_t		portid;
};

static struct nfct_mnl_socket _sock;
static struct nfct_mnl_socket _modifier_sock;
static struct nfct_mnl_socket _event_sock;

struct u32_mask {
	uint32_t value;
	uint32_t mask;
};

/* These are the template objects that are used to send commands. */
struct ct_tmpl {
	struct nf_conntrack *ct;
	struct nf_expect *exp;
	/* Expectations require the expectation tuple and the mask. */
	struct nf_conntrack *exptuple, *mask;

	/* Allows filtering/setting specific bits in the ctmark */
	struct u32_mask mark;

	/* Allow to filter by mark from kernel-space. */
	struct nfct_filter_dump_mark filter_mark_kernel;
	bool filter_mark_kernel_set;

	/* Allow to filter by status from kernel-space. */
	struct nfct_filter_dump_mark filter_status_kernel;
	bool filter_status_kernel_set;

	/* Allows filtering by ctlabels */
	struct nfct_bitmask *label;

	/* Allows setting/removing specific ctlabels */
	struct nfct_bitmask *label_modify;
};

static struct ct_tmpl *cur_tmpl;

struct ct_cmd {
	struct list_head list;
	unsigned int	command;
	unsigned int	cmd;
	unsigned int	type;
	unsigned int	event_mask;
	int		options;
	int		family;
	int		protonum;
	size_t		socketbuffersize;
	struct ct_tmpl	tmpl;
};

struct ct_label {
	struct list_head list;
	char *name;
	bool is_modify;
};

static int alloc_tmpl_objects(struct ct_tmpl *tmpl)
{
	tmpl->ct = nfct_new();
	tmpl->exptuple = nfct_new();
	tmpl->mask = nfct_new();
	tmpl->exp = nfexp_new();

	memset(&tmpl->mark, 0, sizeof(tmpl->mark));

	cur_tmpl = tmpl;

	return tmpl->ct != NULL && tmpl->exptuple != NULL &&
	       tmpl->mask != NULL && tmpl->exp != NULL;
}

static void free_tmpl_objects(struct ct_tmpl *tmpl)
{
	if (!tmpl)
		return;
	if (tmpl->ct)
		nfct_destroy(tmpl->ct);
	if (tmpl->exptuple)
		nfct_destroy(tmpl->exptuple);
	if (tmpl->mask)
		nfct_destroy(tmpl->mask);
	if (tmpl->exp)
		nfexp_destroy(tmpl->exp);
	if (tmpl->label)
		nfct_bitmask_destroy(tmpl->label);
	if (tmpl->label_modify)
		nfct_bitmask_destroy(tmpl->label_modify);
}

enum ct_options {
	CT_OPT_ORIG_SRC_BIT	= 0,
	CT_OPT_ORIG_SRC 	= (1 << CT_OPT_ORIG_SRC_BIT),

	CT_OPT_ORIG_DST_BIT	= 1,
	CT_OPT_ORIG_DST		= (1 << CT_OPT_ORIG_DST_BIT),

	CT_OPT_ORIG		= (CT_OPT_ORIG_SRC | CT_OPT_ORIG_DST),

	CT_OPT_REPL_SRC_BIT	= 2,
	CT_OPT_REPL_SRC		= (1 << CT_OPT_REPL_SRC_BIT),

	CT_OPT_REPL_DST_BIT	= 3,
	CT_OPT_REPL_DST		= (1 << CT_OPT_REPL_DST_BIT),

	CT_OPT_REPL		= (CT_OPT_REPL_SRC | CT_OPT_REPL_DST),

	CT_OPT_PROTO_BIT	= 4,
	CT_OPT_PROTO		= (1 << CT_OPT_PROTO_BIT),

	CT_OPT_TUPLE_ORIG	= (CT_OPT_ORIG | CT_OPT_PROTO),
	CT_OPT_TUPLE_REPL	= (CT_OPT_REPL | CT_OPT_PROTO),

	CT_OPT_TIMEOUT_BIT	= 5,
	CT_OPT_TIMEOUT		= (1 << CT_OPT_TIMEOUT_BIT),

	CT_OPT_STATUS_BIT	= 6,
	CT_OPT_STATUS		= (1 << CT_OPT_STATUS_BIT),

	CT_OPT_ZERO_BIT		= 7,
	CT_OPT_ZERO		= (1 << CT_OPT_ZERO_BIT),

	CT_OPT_EVENT_MASK_BIT	= 8,
	CT_OPT_EVENT_MASK	= (1 << CT_OPT_EVENT_MASK_BIT),

	CT_OPT_EXP_SRC_BIT	= 9,
	CT_OPT_EXP_SRC		= (1 << CT_OPT_EXP_SRC_BIT),

	CT_OPT_EXP_DST_BIT	= 10,
	CT_OPT_EXP_DST		= (1 << CT_OPT_EXP_DST_BIT),

	CT_OPT_MASK_SRC_BIT	= 11,
	CT_OPT_MASK_SRC		= (1 << CT_OPT_MASK_SRC_BIT),

	CT_OPT_MASK_DST_BIT	= 12,
	CT_OPT_MASK_DST		= (1 << CT_OPT_MASK_DST_BIT),

	CT_OPT_NATRANGE_BIT	= 13,
	CT_OPT_NATRANGE		= (1 << CT_OPT_NATRANGE_BIT),

	CT_OPT_MARK_BIT		= 14,
	CT_OPT_MARK		= (1 << CT_OPT_MARK_BIT),

	CT_OPT_ID_BIT		= 15,
	CT_OPT_ID		= (1 << CT_OPT_ID_BIT),

	CT_OPT_FAMILY_BIT	= 16,
	CT_OPT_FAMILY		= (1 << CT_OPT_FAMILY_BIT),

	CT_OPT_SRC_NAT_BIT	= 17,
	CT_OPT_SRC_NAT		= (1 << CT_OPT_SRC_NAT_BIT),

	CT_OPT_DST_NAT_BIT	= 18,
	CT_OPT_DST_NAT		= (1 << CT_OPT_DST_NAT_BIT),

	CT_OPT_OUTPUT_BIT	= 19,
	CT_OPT_OUTPUT		= (1 << CT_OPT_OUTPUT_BIT),

	CT_OPT_SECMARK_BIT	= 20,
	CT_OPT_SECMARK		= (1 << CT_OPT_SECMARK_BIT),

	CT_OPT_BUFFERSIZE_BIT	= 21,
	CT_OPT_BUFFERSIZE	= (1 << CT_OPT_BUFFERSIZE_BIT),

	CT_OPT_ANY_NAT_BIT	= 22,
	CT_OPT_ANY_NAT		= (1 << CT_OPT_ANY_NAT_BIT),

	CT_OPT_ZONE_BIT		= 23,
	CT_OPT_ZONE		= (1 << CT_OPT_ZONE_BIT),

	CT_OPT_LABEL_BIT	= 24,
	CT_OPT_LABEL		= (1 << CT_OPT_LABEL_BIT),

	CT_OPT_ADD_LABEL_BIT	= 25,
	CT_OPT_ADD_LABEL	= (1 << CT_OPT_ADD_LABEL_BIT),

	CT_OPT_DEL_LABEL_BIT	= 26,
	CT_OPT_DEL_LABEL	= (1 << CT_OPT_DEL_LABEL_BIT),

	CT_OPT_ORIG_ZONE_BIT	= 27,
	CT_OPT_ORIG_ZONE	= (1 << CT_OPT_ORIG_ZONE_BIT),

	CT_OPT_REPL_ZONE_BIT	= 28,
	CT_OPT_REPL_ZONE	= (1 << CT_OPT_REPL_ZONE_BIT),

	CT_OPT_LABELMAP_BIT	= 29,
	CT_OPT_LABELMAP		= (1 << CT_OPT_LABELMAP_BIT),
};
/* If you add a new option, you have to update NUMBER_OF_OPT in conntrack.h */

/* Update this mask to allow to filter based on new options. */
#define CT_COMPARISON (CT_OPT_PROTO | CT_OPT_ORIG | CT_OPT_REPL | \
		       CT_OPT_MARK | CT_OPT_SECMARK |  CT_OPT_STATUS | \
		       CT_OPT_ID | CT_OPT_ZONE | CT_OPT_LABEL | \
		       CT_OPT_ORIG_ZONE | CT_OPT_REPL_ZONE)

static const char *optflags[NUMBER_OF_OPT] = {
	[CT_OPT_ORIG_SRC_BIT] 	= "src",
	[CT_OPT_ORIG_DST_BIT]	= "dst",
	[CT_OPT_REPL_SRC_BIT]	= "reply-src",
	[CT_OPT_REPL_DST_BIT]	= "reply-dst",
	[CT_OPT_PROTO_BIT]	= "protonum",
	[CT_OPT_TIMEOUT_BIT]	= "timeout",
	[CT_OPT_STATUS_BIT]	= "status",
	[CT_OPT_ZERO_BIT]	= "zero",
	[CT_OPT_EVENT_MASK_BIT]	= "event-mask",
	[CT_OPT_EXP_SRC_BIT]	= "tuple-src",
	[CT_OPT_EXP_DST_BIT]	= "tuple-dst",
	[CT_OPT_MASK_SRC_BIT]	= "mask-src",
	[CT_OPT_MASK_DST_BIT]	= "mask-dst",
	[CT_OPT_NATRANGE_BIT]	= "nat-range",
	[CT_OPT_MARK_BIT]	= "mark",
	[CT_OPT_ID_BIT]		= "id",
	[CT_OPT_FAMILY_BIT]	= "family",
	[CT_OPT_SRC_NAT_BIT]	= "src-nat",
	[CT_OPT_DST_NAT_BIT]	= "dst-nat",
	[CT_OPT_OUTPUT_BIT]	= "output",
	[CT_OPT_SECMARK_BIT]	= "secmark",
	[CT_OPT_BUFFERSIZE_BIT]	= "buffer-size",
	[CT_OPT_ANY_NAT_BIT]	= "any-nat",
	[CT_OPT_ZONE_BIT]	= "zone",
	[CT_OPT_LABEL_BIT]	= "label",
	[CT_OPT_ADD_LABEL_BIT]	= "label-add",
	[CT_OPT_DEL_LABEL_BIT]	= "label-del",
	[CT_OPT_ORIG_ZONE_BIT]	= "orig-zone",
	[CT_OPT_REPL_ZONE_BIT]	= "reply-zone",
	[CT_OPT_LABELMAP_BIT]	= "labelmap",
};

static struct option original_opts[] = {
	{"dump", 2, 0, 'L'},
	{"create", 2, 0, 'I'},
	{"add", 2, 0, 'A'},
	{"delete", 2, 0, 'D'},
	{"update", 2, 0, 'U'},
	{"get", 2, 0, 'G'},
	{"flush", 2, 0, 'F'},
	{"event", 2, 0, 'E'},
	{"counter", 2, 0, 'C'},
	{"stats", 0, 0, 'S'},
	{"version", 0, 0, 'V'},
	{"help", 0, 0, 'h'},
	{"orig-src", 1, 0, 's'},
	{"src", 1, 0, 's'},
	{"orig-dst", 1, 0, 'd'},
	{"dst", 1, 0, 'd'},
	{"reply-src", 1, 0, 'r'},
	{"reply-dst", 1, 0, 'q'},
	{"protonum", 1, 0, 'p'},
	{"timeout", 1, 0, 't'},
	{"status", 1, 0, 'u'},
	{"zero", 0, 0, 'z'},
	{"event-mask", 1, 0, 'e'},
	{"tuple-src", 1, 0, '['},
	{"tuple-dst", 1, 0, ']'},
	{"mask-src", 1, 0, '{'},
	{"mask-dst", 1, 0, '}'},
	{"nat-range", 1, 0, 'a'},	/* deprecated */
	{"mark", 1, 0, 'm'},
	{"secmark", 1, 0, 'c'},
	{"id", 1, 0, 'i'},		/* deprecated */
	{"family", 1, 0, 'f'},
	{"src-nat", 2, 0, 'n'},
	{"dst-nat", 2, 0, 'g'},
	{"output", 1, 0, 'o'},
	{"buffer-size", 1, 0, 'b'},
	{"any-nat", 2, 0, 'j'},
	{"zone", 1, 0, 'w'},
	{"label", 1, 0, 'l'},
	{"labelmap", 1, 0, 'M'},
	{"label-add", 1, 0, '<'},
	{"label-del", 2, 0, '>'},
	{"orig-zone", 1, 0, '('},
	{"reply-zone", 1, 0, ')'},
	{0, 0, 0, 0}
};

static const char *getopt_str = ":LIUDGEFAhVs:d:r:q:"
				"p:t:u:e:a:z[:]:{:}:m:i:f:o:n::"
				"g::c:b:C::Sj::w:l:<:>::(:):M:";

/* Table of legal combinations of commands and options.  If any of the
 * given commands make an option legal, that option is legal (applies to
 * CMD_LIST and CMD_ZERO only).
 * Key:
 *  0  illegal
 *  1  compulsory
 *  2  optional
 *  3  undecided, see flag combination checkings in generic_opt_check()
 */

static char commands_v_options[NUMBER_OF_CMD][NUMBER_OF_OPT] =
/* Well, it's better than "Re: Linux vs FreeBSD" */
{
			/* s d r q p t u z e [ ] { } a m i f n g o c b j w l < > ( ) M */
	[CT_LIST_BIT]	= {2,2,2,2,2,0,2,2,0,0,0,2,2,0,2,0,2,2,2,2,2,0,2,2,2,0,0,2,2,2},
	[CT_CREATE_BIT]	= {3,3,3,3,1,1,2,0,0,0,0,0,0,2,2,0,0,2,2,0,0,0,0,2,0,2,0,2,2,0},
	[CT_UPDATE_BIT]	= {2,2,2,2,2,2,2,0,0,0,0,2,2,0,2,2,2,2,2,2,0,0,0,0,2,2,2,0,0,2},
	[CT_DELETE_BIT]	= {2,2,2,2,2,2,2,0,0,0,0,2,2,0,2,2,2,2,2,2,0,0,0,2,2,0,0,2,2,2},
	[CT_GET_BIT]	= {3,3,3,3,1,0,0,0,0,0,0,0,0,0,0,2,0,0,0,2,0,0,0,0,2,0,0,0,0,0},
	[CT_FLUSH_BIT]	= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0},
	[CT_EVENT_BIT]	= {2,2,2,2,2,0,0,0,2,0,0,2,2,0,2,0,2,2,2,2,2,2,2,2,2,0,0,2,2,2},
	[CT_VERSION_BIT]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	[CT_HELP_BIT]	= {0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	[EXP_LIST_BIT]	= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,2,0,0,0,0,0,0,0,0,0,0},
	[EXP_CREATE_BIT]= {1,1,2,2,1,1,2,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	[EXP_DELETE_BIT]= {1,1,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	[EXP_GET_BIT]	= {1,1,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	[EXP_FLUSH_BIT]	= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	[EXP_EVENT_BIT]	= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0},
	[CT_COUNT_BIT]	= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	[EXP_COUNT_BIT]	= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	[CT_STATS_BIT]	= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	[EXP_STATS_BIT]	= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	[CT_ADD_BIT]	= {3,3,3,3,1,1,2,0,0,0,0,0,0,2,2,0,0,2,2,0,0,0,0,2,0,2,0,2,2,0},
};

static const int cmd2type[][2] = {
	['L']	= { CT_LIST, 	EXP_LIST },
	['I']	= { CT_CREATE,	EXP_CREATE },
	['D']	= { CT_DELETE,	EXP_DELETE },
	['G']	= { CT_GET,	EXP_GET },
	['F']	= { CT_FLUSH,	EXP_FLUSH },
	['E']	= { CT_EVENT,	EXP_EVENT },
	['V']	= { CT_VERSION,	CT_VERSION },
	['h']	= { CT_HELP,	CT_HELP },
	['C']	= { CT_COUNT,	EXP_COUNT },
	['S']	= { CT_STATS,	EXP_STATS },
	['U']	= { CT_UPDATE,	0 },
	['A']	= { CT_ADD,	0 },
};

static const int opt2type[] = {
	['s']	= CT_OPT_ORIG_SRC,
	['d']	= CT_OPT_ORIG_DST,
	['r']	= CT_OPT_REPL_SRC,
	['q']	= CT_OPT_REPL_DST,
	['{']	= CT_OPT_MASK_SRC,
	['}']	= CT_OPT_MASK_DST,
	['[']	= CT_OPT_EXP_SRC,
	[']']	= CT_OPT_EXP_DST,
	['n']	= CT_OPT_SRC_NAT,
	['g']	= CT_OPT_DST_NAT,
	['m']	= CT_OPT_MARK,
	['c']	= CT_OPT_SECMARK,
	['i']	= CT_OPT_ID,
	['j']	= CT_OPT_ANY_NAT,
	['w']	= CT_OPT_ZONE,
	['l']	= CT_OPT_LABEL,
	['<']	= CT_OPT_ADD_LABEL,
	['>']	= CT_OPT_DEL_LABEL,
	['(']	= CT_OPT_ORIG_ZONE,
	[')']	= CT_OPT_REPL_ZONE,
	['M']	= CT_OPT_LABELMAP,
};

static const int opt2maskopt[] = {
	['s']	= '{',
	['d']	= '}',
	['g']   = 0,
	['j']   = 0,
	['n']   = 0,
	['r']	= 0, /* no netmask */
	['q']	= 0, /* support yet */
	['{']	= 0,
	['}']	= 0,
	['[']	= '{',
	[']']	= '}',
};

static const int opt2family_attr[][2] = {
	['s']	= { ATTR_ORIG_IPV4_SRC,	ATTR_ORIG_IPV6_SRC },
	['d']	= { ATTR_ORIG_IPV4_DST,	ATTR_ORIG_IPV6_DST },
	['g']   = { ATTR_DNAT_IPV4, ATTR_DNAT_IPV6 },
	['n']   = { ATTR_SNAT_IPV4, ATTR_SNAT_IPV6 },
	['r']	= { ATTR_REPL_IPV4_SRC, ATTR_REPL_IPV6_SRC },
	['q']	= { ATTR_REPL_IPV4_DST, ATTR_REPL_IPV6_DST },
	['{']	= { ATTR_ORIG_IPV4_SRC,	ATTR_ORIG_IPV6_SRC },
	['}']	= { ATTR_ORIG_IPV4_DST,	ATTR_ORIG_IPV6_DST },
	['[']	= { ATTR_ORIG_IPV4_SRC, ATTR_ORIG_IPV6_SRC },
	[']']	= { ATTR_ORIG_IPV4_DST, ATTR_ORIG_IPV6_DST },
};

static const int opt2attr[] = {
	['s']	= ATTR_ORIG_L3PROTO,
	['d']	= ATTR_ORIG_L3PROTO,
	['g']	= ATTR_ORIG_L3PROTO,
	['n']	= ATTR_ORIG_L3PROTO,
	['r']	= ATTR_REPL_L3PROTO,
	['q']	= ATTR_REPL_L3PROTO,
	['{']	= ATTR_ORIG_L3PROTO,
	['}']	= ATTR_ORIG_L3PROTO,
	['[']	= ATTR_ORIG_L3PROTO,
	[']']	= ATTR_ORIG_L3PROTO,
	['m']	= ATTR_MARK,
	['c']	= ATTR_SECMARK,
	['i']	= ATTR_ID,
	['w']	= ATTR_ZONE,
	['l']	= ATTR_CONNLABELS,
	['<']	= ATTR_CONNLABELS,
	['>']	= ATTR_CONNLABELS,
	['(']	= ATTR_ORIG_ZONE,
	[')']	= ATTR_REPL_ZONE,
};

enum ct_direction {
	DIR_SRC = 0,
	DIR_DST = 1,
};

union ct_address {
	uint32_t v4;
	uint32_t v6[4];
};

static struct ct_network {
	union ct_address netmask;
	union ct_address network;
} dir2network[2];

static const int famdir2attr[2][2] = {
	{ ATTR_ORIG_IPV4_SRC, ATTR_ORIG_IPV4_DST },
	{ ATTR_ORIG_IPV6_SRC, ATTR_ORIG_IPV6_DST }
};

static char exit_msg[NUMBER_OF_CMD][64] = {
	[CT_LIST_BIT] 		= "%d flow entries have been shown.\n",
	[CT_CREATE_BIT]		= "%d flow entries have been created.\n",
	[CT_UPDATE_BIT]		= "%d flow entries have been updated.\n",
	[CT_DELETE_BIT]		= "%d flow entries have been deleted.\n",
	[CT_GET_BIT] 		= "%d flow entries have been shown.\n",
	[CT_EVENT_BIT]		= "%d flow events have been shown.\n",
	[EXP_LIST_BIT]		= "%d expectations have been shown.\n",
	[EXP_DELETE_BIT]	= "%d expectations have been shown.\n",
	[CT_ADD_BIT]		= "%d flow entries have been added.\n",
};

static const char usage_commands[] =
	"Commands:\n"
	"  -L [table] [options]\t\tList conntrack or expectation table\n"
	"  -G [table] parameters\t\tGet conntrack or expectation\n"
	"  -D [table] parameters\t\tDelete conntrack or expectation\n"
	"  -I [table] parameters\t\tCreate a conntrack or expectation\n"
	"  -U [table] parameters\t\tUpdate a conntrack\n"
	"  -E [table] [options]\t\tShow events\n"
	"  -F [table]\t\t\tFlush table\n"
	"  -C [table]\t\t\tShow counter\n"
	"  -S\t\t\t\tShow statistics\n";

static const char usage_tables[] =
	"Tables: conntrack, expect, dying, unconfirmed\n";

static const char usage_conntrack_parameters[] =
	"Conntrack parameters and options:\n"
	"  -n, --src-nat ip\t\t\tsource NAT ip\n"
	"  -g, --dst-nat ip\t\t\tdestination NAT ip\n"
	"  -j, --any-nat ip\t\t\tsource or destination NAT ip\n"
	"  -m, --mark mark\t\t\tSet mark\n"
	"  -c, --secmark secmark\t\t\tSet selinux secmark\n"
	"  -e, --event-mask eventmask\t\tEvent mask, eg. NEW,DESTROY\n"
	"  -z, --zero \t\t\t\tZero counters while listing\n"
	"  -o, --output type[,...]\t\tOutput format, eg. xml\n"
	"  -l, --label label[,...]\t\tconntrack labels\n"
	"  --labelmap path\t\t\tconnlabel file to use instead of default\n";

static const char usage_expectation_parameters[] =
	"Expectation parameters and options:\n"
	"  --tuple-src ip\tSource address in expect tuple\n"
	"  --tuple-dst ip\tDestination address in expect tuple\n"
	;

static const char usage_update_parameters[] =
	"Updating parameters and options:\n"
	"  --label-add label\tAdd label\n"
	"  --label-del label\tDelete label\n";

static const char usage_parameters[] =
	"Common parameters and options:\n"
	"  -s, --src, --orig-src ip\t\tSource address from original direction\n"
	"  -d, --dst, --orig-dst ip\t\tDestination address from original direction\n"
	"  -r, --reply-src ip\t\tSource address from reply direction\n"
	"  -q, --reply-dst ip\t\tDestination address from reply direction\n"
	"  -p, --protonum proto\t\tLayer 4 Protocol, eg. 'tcp'\n"
	"  -f, --family proto\t\tLayer 3 Protocol, eg. 'ipv6'\n"
	"  -t, --timeout timeout\t\tSet timeout\n"
	"  -u, --status status\t\tSet status, eg. ASSURED\n"
	"  -w, --zone value\t\tSet conntrack zone\n"
	"  --orig-zone value\t\tSet zone for original direction\n"
	"  --reply-zone value\t\tSet zone for reply direction\n"
	"  -b, --buffer-size\t\tNetlink socket buffer size\n"
	"  --mask-src ip\t\t\tSource mask address\n"
	"  --mask-dst ip\t\t\tDestination mask address\n"
	;

#define OPTION_OFFSET 256

static struct nfct_handle *cth;
static struct option *opts = original_opts;
static unsigned int global_option_offset = 0;

#define ADDR_VALID_FLAGS_MAX   2
static unsigned int addr_valid_flags[ADDR_VALID_FLAGS_MAX] = {
	CT_OPT_ORIG_SRC | CT_OPT_ORIG_DST,
	CT_OPT_REPL_SRC | CT_OPT_REPL_DST,
};

static LIST_HEAD(proto_list);

static char *labelmap_path;
static struct nfct_labelmap *labelmap;
static int filter_family;

void register_proto(struct ctproto_handler *h)
{
	if (strcmp(h->version, VERSION) != 0) {
		fprintf(stderr, "plugin `%s': version %s (I'm %s)\n",
			h->name, h->version, VERSION);
		exit(1);
	}
	list_add(&h->head, &proto_list);
}

#define BUFFER_SIZE(ret, size, len, offset) do {			\
	if ((int)ret > (int)len)					\
		ret = len;						\
	size += ret;							\
	offset += ret;							\
	len -= ret;							\
} while(0)

static int ct_snprintf_u32_bitmap(char *buf, size_t size,
				  const struct nf_conntrack *ct,
				  const struct ct_print_opts *attr)
{
	unsigned int offset = 0, ret, len = size;
	bool found = false;
	uint32_t val;
	int i;

	val = nfct_get_attr_u32(ct, attr->type);

	for (i = 0; i < attr->val_mapping_count; i++) {
		if (!(val & (1 << i)))
			continue;
		if (!attr->val_mapping[i])
			continue;

		ret = snprintf(buf + offset, len, "%s,", attr->val_mapping[i]);
		BUFFER_SIZE(ret, size, len, offset);
		found = true;
	}

	if (found) {
		offset--;
		ret = snprintf(buf + offset, len, " ");
		BUFFER_SIZE(ret, size, len, offset);
	}

	return offset;
}

static int ct_snprintf_attr(char *buf, size_t size,
			    const struct nf_conntrack *ct,
			    const struct ct_print_opts *attr)
{
	char ipstr[INET6_ADDRSTRLEN] = {};
	unsigned int offset = 0;
	int type = attr->type;
	int len = size, ret;

	ret = snprintf(buf, len, "%s ", attr->name);
	BUFFER_SIZE(ret, size, len, offset);

	switch (attr->datatype) {
	case CT_ATTR_TYPE_U8:
		if (attr->val_mapping)
			ret = snprintf(buf + offset, len, "%s ",
				       attr->val_mapping[nfct_get_attr_u8(ct, type)]);
		else
			ret = snprintf(buf + offset, len, "%u ",
				       nfct_get_attr_u8(ct, type));
		break;
	case CT_ATTR_TYPE_BE16:
		ret = snprintf(buf + offset, len, "%u ",
			       ntohs(nfct_get_attr_u16(ct, type)));
		break;
	case CT_ATTR_TYPE_U16:
		ret = snprintf(buf + offset, len, "%u ",
			       nfct_get_attr_u16(ct, type));
		break;
	case CT_ATTR_TYPE_BE32:
		ret = snprintf(buf + offset, len, "%u ",
			       ntohl(nfct_get_attr_u32(ct, type)));
		break;
	case CT_ATTR_TYPE_U32:
		ret = snprintf(buf + offset, len, "%u ",
			       nfct_get_attr_u32(ct, type));
		break;
	case CT_ATTR_TYPE_U64:
		ret = snprintf(buf + offset, len, "%lu ",
			       nfct_get_attr_u64(ct, type));
		break;
	case CT_ATTR_TYPE_IPV4:
		inet_ntop(AF_INET, nfct_get_attr(ct, type),
			  ipstr, sizeof(ipstr));
		ret = snprintf(buf + offset, len, "%s ", ipstr);
		break;
	case CT_ATTR_TYPE_IPV6:
		inet_ntop(AF_INET6, nfct_get_attr(ct, type),
			  ipstr, sizeof(ipstr));
		ret = snprintf(buf + offset, len, "%s ", ipstr);
		break;
	case CT_ATTR_TYPE_U32_BITMAP:
		ret = ct_snprintf_u32_bitmap(buf + offset, len, ct, attr);
		if (ret == 0 && type == ATTR_STATUS)
			ret = snprintf(buf + offset, len, "UNSET ");
		break;
	default:
		/* Unsupported datatype, should not ever happen */
		ret = 0;
		break;
	}
	BUFFER_SIZE(ret, size, len, offset);

	return offset;
}

int ct_snprintf_opts(char *buf, unsigned int len, const struct nf_conntrack *ct,
		     const struct ct_print_opts *attrs)
{
	unsigned int size = 0, offset = 0, ret;
	int i;

	for (i = 0; attrs[i].name; ++i) {
		if (!nfct_attr_is_set(ct, attrs[i].type))
			continue;

		ret = ct_snprintf_attr(buf + offset, len, ct, &attrs[i]);
		BUFFER_SIZE(ret, size, len, offset);
	}

	return offset;
}

static const struct ct_print_opts attrs_ip_map[] = {
	[ATTR_ORIG_IPV4_SRC] = { "-s", ATTR_ORIG_IPV4_SRC, CT_ATTR_TYPE_IPV4, 0, 0 },
	[ATTR_ORIG_IPV4_DST] = { "-d", ATTR_ORIG_IPV4_DST, CT_ATTR_TYPE_IPV4, 0, 0 },
	[ATTR_DNAT_IPV4]     = { "-g", ATTR_REPL_IPV4_SRC, CT_ATTR_TYPE_IPV4, 0, 0 },
	[ATTR_SNAT_IPV4]     = { "-n", ATTR_REPL_IPV4_DST, CT_ATTR_TYPE_IPV4, 0, 0 },
	[ATTR_REPL_IPV4_SRC] = { "-r", ATTR_REPL_IPV4_SRC, CT_ATTR_TYPE_IPV4, 0, 0 },
	[ATTR_REPL_IPV4_DST] = { "-q", ATTR_REPL_IPV4_DST, CT_ATTR_TYPE_IPV4, 0, 0 },
	[ATTR_ORIG_IPV6_SRC] = { "-s", ATTR_ORIG_IPV6_SRC, CT_ATTR_TYPE_IPV6, 0, 0 },
	[ATTR_ORIG_IPV6_DST] = { "-d", ATTR_ORIG_IPV6_DST, CT_ATTR_TYPE_IPV6, 0, 0 },
	[ATTR_DNAT_IPV6]     = { "-g", ATTR_REPL_IPV6_SRC, CT_ATTR_TYPE_IPV6, 0, 0 },
	[ATTR_SNAT_IPV6]     = { "-n", ATTR_REPL_IPV6_DST, CT_ATTR_TYPE_IPV6, 0, 0 },
	[ATTR_REPL_IPV6_SRC] = { "-r", ATTR_REPL_IPV6_SRC, CT_ATTR_TYPE_IPV6, 0, 0 },
	[ATTR_REPL_IPV6_DST] = { "-q", ATTR_REPL_IPV6_DST, CT_ATTR_TYPE_IPV6, 0, 0 },
};

static const char *conntrack_status_map[] = {
	[IPS_SEEN_REPLY_BIT] = "SEEN_REPLY",
	[IPS_ASSURED_BIT] = "ASSURED",
	[IPS_FIXED_TIMEOUT_BIT] = "FIXED_TIMEOUT",
	[IPS_EXPECTED_BIT] = "EXPECTED"
};

static struct ct_print_opts attrs_generic[] = {
	{ "-t", ATTR_TIMEOUT, CT_ATTR_TYPE_U32, 0, 0 },
	{ "-u", ATTR_STATUS, CT_ATTR_TYPE_U32_BITMAP,
		sizeof(conntrack_status_map)/sizeof(conntrack_status_map[0]),
		conntrack_status_map },
	{ "-c", ATTR_SECMARK, CT_ATTR_TYPE_U32, 0, 0 },
	{ "-w", ATTR_ZONE, CT_ATTR_TYPE_U16, 0, 0 },
	{ "--orig-zone", ATTR_ORIG_ZONE, CT_ATTR_TYPE_U16, 0, 0 },
	{ "--reply-zone", ATTR_REPL_ZONE, CT_ATTR_TYPE_U16, 0, 0 },
	{},
};

static int ct_save_snprintf(char *buf, size_t len,
			    const struct nf_conntrack *ct,
			    struct nfct_labelmap *map,
			    enum nf_conntrack_msg_type type)
{
	struct ct_print_opts tuple_attr_print[5] = {};
	unsigned int size = 0, offset = 0;
	struct ctproto_handler *cur;
	uint8_t l3proto, l4proto;
	int tuple_attrs[4] = {};
	bool l4proto_set;
	unsigned i;
	int ret;

	switch (type) {
	case NFCT_T_NEW:
		ret = snprintf(buf + offset, len, "-A ");
		BUFFER_SIZE(ret, size, len, offset);
		break;
	case NFCT_T_UPDATE:
		ret = snprintf(buf + offset, len, "-U ");
		BUFFER_SIZE(ret, size, len, offset);
		break;
	case NFCT_T_DESTROY:
		ret = snprintf(buf + offset, len, "-D ");
		BUFFER_SIZE(ret, size, len, offset);
		break;
	default:
		break;
	}

	ret = ct_snprintf_opts(buf + offset, len, ct, attrs_generic);
	BUFFER_SIZE(ret, size, len, offset);

	l3proto = nfct_get_attr_u8(ct, ATTR_ORIG_L3PROTO);
	if (!l3proto)
		l3proto = nfct_get_attr_u8(ct, ATTR_REPL_L3PROTO);
	switch (l3proto) {
	case AF_INET:
		tuple_attrs[0] = ATTR_ORIG_IPV4_SRC;
		tuple_attrs[1] = ATTR_ORIG_IPV4_DST;
		tuple_attrs[2] = nfct_getobjopt(ct, NFCT_GOPT_IS_DNAT) ?
					ATTR_DNAT_IPV4 : ATTR_REPL_IPV4_SRC;
		tuple_attrs[3] = nfct_getobjopt(ct, NFCT_GOPT_IS_SNAT) ?
					ATTR_SNAT_IPV4 : ATTR_REPL_IPV4_DST;
		break;
	case AF_INET6:
		tuple_attrs[0] = ATTR_ORIG_IPV6_SRC;
		tuple_attrs[1] = ATTR_ORIG_IPV6_DST;
		tuple_attrs[2] = nfct_getobjopt(ct, NFCT_GOPT_IS_DNAT) ?
					ATTR_DNAT_IPV6 : ATTR_REPL_IPV6_SRC;
		tuple_attrs[3] = nfct_getobjopt(ct, NFCT_GOPT_IS_SNAT) ?
					ATTR_SNAT_IPV6 : ATTR_REPL_IPV6_DST;
		break;
	default:
		fprintf(stderr, "WARNING: unsupported l3proto %d, skipping.\n",
			l3proto);
		return 0;
	}

	for (i = 0; i < sizeof(tuple_attrs) / sizeof(tuple_attrs[0]); i++) {
		memcpy(&tuple_attr_print[i], &attrs_ip_map[tuple_attrs[i]],
		       sizeof(tuple_attr_print[0]));
	}

	ret = ct_snprintf_opts(buf + offset, len, ct, tuple_attr_print);
	BUFFER_SIZE(ret, size, len, offset);

	l4proto = nfct_get_attr_u8(ct, ATTR_L4PROTO);

	l4proto_set = false;
	/* is it in the list of supported protocol? */
	list_for_each_entry(cur, &proto_list, head) {
		if (cur->protonum != l4proto)
			continue;

		ret = snprintf(buf + offset, len, "-p %s ", cur->name);
		BUFFER_SIZE(ret, size, len, offset);

		ret = ct_snprintf_opts(buf + offset, len, ct, cur->print_opts);
		BUFFER_SIZE(ret, size, len, offset);

		l4proto_set = true;
		break;
	}

	if (!l4proto_set) {
		ret = snprintf(buf + offset, len, "-p %d ", l4proto);
		BUFFER_SIZE(ret, size, len, offset);
	}

	/* skip trailing space, if any */
	for (; size && buf[size-1] == ' '; --size)
		buf[size-1] = '\0';

	return size;
}

extern struct ctproto_handler ct_proto_unknown;

static int parse_proto_num(const char *str)
{
	unsigned long val;
	char *endptr;

	val = strtoul(str, &endptr, 0);
	if (val > IPPROTO_RAW ||
	    endptr == str ||
	    *endptr != '\0')
		return -1;

	return val;
}

static struct ctproto_handler *findproto(char *name, int *pnum)
{
	struct ctproto_handler *cur;
	struct protoent *pent;
	int protonum;

	/* is it in the list of supported protocol? */
	list_for_each_entry(cur, &proto_list, head) {
		if (strcasecmp(cur->name, name) == 0) {
			*pnum = cur->protonum;
			return cur;
		}
	}
	/* using the protocol name for an unsupported protocol? */
	if ((pent = getprotobyname(name))) {
		*pnum = pent->p_proto;
		return &ct_proto_unknown;
	}
	/* using a protocol number? */
	protonum = parse_proto_num(name);
	if (protonum >= 0) {
		/* try lookup by number, perhaps this protocol is supported */
		list_for_each_entry(cur, &proto_list, head) {
			if (cur->protonum == protonum) {
				*pnum = protonum;
				return cur;
			}
		}
		*pnum = protonum;
		return &ct_proto_unknown;
	}

	return NULL;
}

static void
extension_help(struct ctproto_handler *h, int protonum)
{
	const char *name;

	if (h == &ct_proto_unknown) {
		struct protoent *pent;

		pent = getprotobynumber(protonum);
		if (!pent)
			name = h->name;
		else
			name = pent->p_name;
	} else {
		name = h->name;
	}

	fprintf(stdout, "Proto `%s' help:\n", name);
	h->help();
}

static void __attribute__((noreturn))
exit_tryhelp(int status)
{
	fprintf(stderr, "Try `%s -h' or '%s --help' for more information.\n",
			PROGNAME, PROGNAME);
	exit(status);
}

static void free_options(void)
{
	if (opts != original_opts) {
		free(opts);
		opts = original_opts;
		global_option_offset = 0;
	}
}

void __attribute__((noreturn))
exit_error(enum exittype status, const char *msg, ...)
{
	va_list args;

	free_options();
	va_start(args, msg);
	fprintf(stderr,"%s v%s (conntrack-tools): ", PROGNAME, VERSION);
	vfprintf(stderr, msg, args);
	fprintf(stderr, "\n");
	va_end(args);
	if (status == PARAMETER_PROBLEM)
		exit_tryhelp(status);
	/* release template objects that were allocated in the setup stage. */
	free_tmpl_objects(cur_tmpl);
	exit(status);
}

static int bit2cmd(int command)
{
	int i;

	for (i = 0; i < NUMBER_OF_CMD; i++)
		if (command & (1<<i))
			break;

	return i;
}

static const char *get_long_opt(int opt)
{
	struct option o;
	int i;

	for (i = 0;; i++) {
		o = opts[i];
		if (o.name == NULL)
			break;
		if (o.val == opt)
			return o.name;
	}
	return "unknown";
}

int generic_opt_check(int local_options, int num_opts,
		      char *optset, const char *optflg[],
		      unsigned int *coupled_flags, int coupled_flags_size,
		      int *partial)
{
	int i, matching = -1, special_case = 0;

	for (i = 0; i < num_opts; i++) {
		if (!(local_options & (1<<i))) {
			if (optset[i] == 1)
				exit_error(PARAMETER_PROBLEM, 
					   "You need to supply the "
					   "`--%s' option for this "
					   "command", optflg[i]);
		} else {
			if (optset[i] == 0)
				exit_error(PARAMETER_PROBLEM, "Illegal "
					   "option `--%s' with this "
					   "command", optflg[i]);
		}
		if (optset[i] == 3)
			special_case = 1;
	}

	/* no weird flags combinations, leave */
	if (!special_case || coupled_flags == NULL)
		return 1;

	*partial = -1;
	for (i=0; i<coupled_flags_size; i++) {
		/* we look for an exact matching to ensure this is correct */
		if ((local_options & coupled_flags[i]) == coupled_flags[i]) {
			matching = i;
			break;
		}
		/* ... otherwise look for the first partial matching */
		if ((local_options & coupled_flags[i]) && *partial < 0) {
			*partial = i;
		}
	}

	/* we found an exact matching, game over */
	if (matching >= 0)
		return 1;

	/* report a partial matching to suggest something */
	return 0;
}

static struct option *
merge_options(struct option *oldopts, const struct option *newopts,
	      unsigned int *option_offset)
{
	unsigned int num_old, num_new, i;
	struct option *merge;

	for (num_old = 0; oldopts[num_old].name; num_old++);
	for (num_new = 0; newopts[num_new].name; num_new++);

	global_option_offset += OPTION_OFFSET;
	*option_offset = global_option_offset;

	merge = malloc(sizeof(struct option) * (num_new + num_old + 1));
	if (merge == NULL)
		return NULL;

	memcpy(merge, oldopts, num_old * sizeof(struct option));
	for (i = 0; i < num_new; i++) {
		merge[num_old + i] = newopts[i];
		merge[num_old + i].val += *option_offset;
	}
	memset(merge + num_old + num_new, 0, sizeof(struct option));

	return merge;
}

/* From linux/errno.h */
#define ENOTSUPP        524     /* Operation is not supported */

/* Translates errno numbers into more human-readable form than strerror. */
static const char *
err2str(int err, enum ct_command command)
{
	unsigned int i;
	struct table_struct {
		enum ct_command act;
		int err;
		const char *message;
	} table [] =
	  { { CT_LIST, ENOTSUPP, "function not implemented" },
	    { 0xFFFF, EINVAL, "invalid parameters" },
	    { CT_CREATE, EEXIST, "Such conntrack exists, try -U to update" },
	    { CT_CREATE|CT_GET|CT_DELETE|CT_ADD, ENOENT,
		    "such conntrack doesn't exist" },
	    { CT_CREATE|CT_GET|CT_ADD, ENOMEM, "not enough memory" },
	    { CT_GET, EAFNOSUPPORT, "protocol not supported" },
	    { CT_CREATE|CT_ADD, ETIME, "conntrack has expired" },
	    { EXP_CREATE, ENOENT, "master conntrack not found" },
	    { EXP_CREATE, EINVAL, "invalid parameters" },
	    { ~0U, EPERM, "sorry, you must be root or get "
		    	   "CAP_NET_ADMIN capability to do this"}
	  };

	for (i = 0; i < sizeof(table)/sizeof(struct table_struct); i++) {
		if ((table[i].act & command) && table[i].err == err)
			return table[i].message;
	}

	return strerror(err);
}

static int mark_cmp(const struct u32_mask *m, const struct nf_conntrack *ct)
{
	return nfct_attr_is_set(ct, ATTR_MARK) &&
		(nfct_get_attr_u32(ct, ATTR_MARK) & m->mask) == m->value;
}

#define PARSE_STATUS 0
#define PARSE_EVENT 1
#define PARSE_OUTPUT 2
#define PARSE_MAX 3

enum {
	_O_XML	= (1 << 0),
	_O_EXT	= (1 << 1),
	_O_TMS	= (1 << 2),
	_O_ID	= (1 << 3),
	_O_KTMS	= (1 << 4),
	_O_CL	= (1 << 5),
	_O_SAVE	= (1 << 6),
};

enum {
	CT_EVENT_F_NEW	= (1 << 0),
	CT_EVENT_F_UPD	= (1 << 1),
	CT_EVENT_F_DEL 	= (1 << 2),
	CT_EVENT_F_ALL	= CT_EVENT_F_NEW | CT_EVENT_F_UPD | CT_EVENT_F_DEL,
};

static struct parse_parameter {
	const char	*parameter[8];
	size_t  size;
	unsigned int value[8];
} parse_array[PARSE_MAX] = {
	{ {"ASSURED", "SEEN_REPLY", "UNSET", "FIXED_TIMEOUT", "EXPECTED", "OFFLOAD", "HW_OFFLOAD"}, 7,
	  { IPS_ASSURED, IPS_SEEN_REPLY, 0, IPS_FIXED_TIMEOUT, IPS_EXPECTED, IPS_OFFLOAD, IPS_HW_OFFLOAD} },
	{ {"ALL", "NEW", "UPDATES", "DESTROY"}, 4,
	  { CT_EVENT_F_ALL, CT_EVENT_F_NEW, CT_EVENT_F_UPD, CT_EVENT_F_DEL } },
	{ {"xml", "extended", "timestamp", "id", "ktimestamp", "labels", "userspace", "save"}, 8,
	  { _O_XML, _O_EXT, _O_TMS, _O_ID, _O_KTMS, _O_CL, 0, _O_SAVE },
	},
};

static int
do_parse_parameter(const char *str, size_t str_length, unsigned int *value,
		   int parse_type)
{
	size_t i;
	int ret = 0;
	struct parse_parameter *p = &parse_array[parse_type];

	if (strncasecmp(str, "SRC_NAT", str_length) == 0) {
		fprintf(stderr, "WARNING: ignoring SRC_NAT, "
				"use --src-nat instead\n");
		return 1;
	}

	if (strncasecmp(str, "DST_NAT", str_length) == 0) {
		fprintf(stderr, "WARNING: ignoring DST_NAT, "
				"use --dst-nat instead\n");
		return 1;
	}

	for (i = 0; i < p->size; i++)
		if (strncasecmp(str, p->parameter[i], str_length) == 0) {
			*value |= p->value[i];
			ret = 1;
			break;
		}

	return ret;
}

static void
parse_parameter(const char *arg, unsigned int *status, int parse_type)
{
	const char *comma;

	while ((comma = strchr(arg, ',')) != NULL) {
		if (comma == arg 
		    || !do_parse_parameter(arg, comma-arg, status, parse_type))
			exit_error(PARAMETER_PROBLEM,"Bad parameter `%s'", arg);
		arg = comma+1;
	}

	if (strlen(arg) == 0
	    || !do_parse_parameter(arg, strlen(arg), status, parse_type))
		exit_error(PARAMETER_PROBLEM, "Bad parameter `%s'", arg);
}

static void
parse_parameter_mask(const char *arg, unsigned int *status, unsigned int *mask, int parse_type)
{
	static const char unreplied[] = "UNREPLIED";
	unsigned int *value;
	const char *comma;
	bool negated;

	while ((comma = strchr(arg, ',')) != NULL) {
		if (comma == arg)
			exit_error(PARAMETER_PROBLEM,"Bad parameter `%s'", arg);

		negated = *arg == '!';
		if (negated)
			arg++;
		if (comma == arg)
			exit_error(PARAMETER_PROBLEM,"Bad parameter `%s'", arg);

		value = negated ? mask : status;

		if (!negated && strncmp(arg, unreplied, strlen(unreplied)) == 0) {
			*mask |= IPS_SEEN_REPLY;
			arg = comma+1;
			continue;
		}

		if (!do_parse_parameter(arg, comma-arg, value, parse_type))
			exit_error(PARAMETER_PROBLEM,"Bad parameter `%s'", arg);
		arg = comma+1;
	}

	negated = *arg == '!';
	if (negated)
		arg++;
	value = negated ? mask : status;

	if (!negated && strncmp(arg, unreplied, strlen(unreplied)) == 0) {
		*mask |= IPS_SEEN_REPLY;
		return;
	}

	if (strlen(arg) == 0
	    || !do_parse_parameter(arg, strlen(arg),
		    value, parse_type))
		exit_error(PARAMETER_PROBLEM, "Bad parameter `%s'", arg);
}

static int parse_value(const char *str, uint32_t *ret, uint64_t max)
{
	char *endptr;
	uint64_t val;

	assert(max <= UINT32_MAX);

	errno = 0;
	val = strtoul(str, &endptr, 0);
	if (endptr == str ||
	    *endptr != '\0' ||
	    (val == ULONG_MAX && errno == ERANGE) ||
	    val > max)
		return -1;

	*ret = val;

	return 0;
}

static int
parse_u32_mask(const char *arg, struct u32_mask *m)
{
	uint64_t val, mask;
	char *endptr;

	val = strtoul(arg, &endptr, 0);
	if (endptr == arg ||
	    (*endptr != '\0' && *endptr != '/') ||
	    (val == ULONG_MAX && errno == ERANGE) ||
	    val > UINT32_MAX)
		return -1;

	m->value = val;

	if (*endptr == '/') {
		mask = strtoul(endptr + 1, &endptr, 0);
		if (endptr == arg ||
		    *endptr != '\0' ||
		    (val == ULONG_MAX && errno == ERANGE) ||
		    val > UINT32_MAX)
			return -1;

		m->mask = mask;
	} else {
		m->mask = ~0;
	}

	return 0;
}

static int
get_label(char *name)
{
	int bit = nfct_labelmap_get_bit(labelmap, name);
	if (bit < 0)
		exit_error(PARAMETER_PROBLEM, "unknown label '%s'", name);
	return bit;
}

static void
set_label(struct nfct_bitmask *b, char *name)
{
	int bit = get_label(name);
	nfct_bitmask_set_bit(b, bit);
}

static unsigned int
set_max_label(char *name, unsigned int current_max)
{
	int bit = get_label(name);
	if ((unsigned int) bit > current_max)
		return (unsigned int) bit;
	return current_max;
}

static unsigned int
parse_label_get_max(char *arg)
{
	unsigned int max = 0;
	char *parse;

	while ((parse = strchr(arg, ',')) != NULL) {
		parse[0] = '\0';
		max = set_max_label(arg, max);
		arg = &parse[1];
	}

	max = set_max_label(arg, max);
	return max;
}

static void
parse_label(struct nfct_bitmask *b, char *arg)
{
	char * parse;
	while ((parse = strchr(arg, ',')) != NULL) {
		parse[0] = '\0';
		set_label(b, arg);
		arg = &parse[1];
	}
	set_label(b, arg);
}

static void
add_command(unsigned int *cmd, const int newcmd)
{
	if (*cmd)
		exit_error(PARAMETER_PROBLEM, "Invalid commands combination");
	*cmd |= newcmd;
}

static char *get_optional_arg(int argc, char *argv[])
{
	char *arg = NULL;

	/* Nasty bug or feature in getopt_long ?
	 * It seems that it behaves badly with optional arguments.
	 * Fortunately, I just stole the fix from iptables ;) */
	if (optarg)
		return arg;
	else if (optind < argc && argv[optind][0] != '-' &&
		 argv[optind][0] != '!')
		arg = argv[optind++];

	return arg;
}

enum {
	CT_TABLE_CONNTRACK,
	CT_TABLE_EXPECT,
	CT_TABLE_DYING,
	CT_TABLE_UNCONFIRMED,
};

static unsigned int check_type(int argc, char *argv[])
{
	const char *table = get_optional_arg(argc, argv);

	/* default to conntrack subsystem if nothing has been specified. */
	if (table == NULL)
		return CT_TABLE_CONNTRACK;

	if (strncmp("expect", table, strlen(table)) == 0)
		return CT_TABLE_EXPECT;
	else if (strncmp("conntrack", table, strlen(table)) == 0)
		return CT_TABLE_CONNTRACK;
	else if (strncmp("dying", table, strlen(table)) == 0)
		return CT_TABLE_DYING;
	else if (strncmp("unconfirmed", table, strlen(table)) == 0)
		return CT_TABLE_UNCONFIRMED;
	else
		exit_error(PARAMETER_PROBLEM, "unknown type `%s'", table);

	return 0;
}

static void set_family(int *family, int new)
{
	if (*family == AF_UNSPEC)
		*family = new;
	else if (*family != new)
		exit_error(PARAMETER_PROBLEM, "mismatched address family");
}

struct addr_parse {
	struct in_addr addr;
	struct in6_addr addr6;
	unsigned int family;
};

static int
parse_inetaddr(const char *cp, struct addr_parse *parse)
{
	if (inet_aton(cp, &parse->addr))
		return AF_INET;
	else if (inet_pton(AF_INET6, cp, &parse->addr6) > 0)
		return AF_INET6;
	return AF_UNSPEC;
}

static int
parse_addr(const char *cp, union ct_address *address, int *mask)
{
	char buf[INET6_ADDRSTRLEN];
	struct addr_parse parse;
	char *slash, *end;
	int family;

	strncpy((char *) &buf, cp, INET6_ADDRSTRLEN);
	buf[INET6_ADDRSTRLEN - 1] = '\0';

	if (mask != NULL) {
		slash = strchr(buf, '/');
		if (slash != NULL) {
			*mask = strtol(slash + 1, &end, 10);
			if (*mask < 0 || end != slash + strlen(slash))
				*mask = -2; /* invalid netmask */
			slash[0] = '\0';
		} else {
			*mask = -1; /* no netmask */
		}
	}

	family = parse_inetaddr(buf, &parse);
	switch (family) {
	case AF_INET:
		address->v4 = parse.addr.s_addr;
		if (mask != NULL && *mask > 32)
			*mask = -2; /* invalid netmask */
		break;
	case AF_INET6:
		memcpy(address->v6, &parse.addr6, sizeof(parse.addr6));
		if (mask != NULL && *mask > 128)
			*mask = -2; /* invalid netmask */
		break;
	}

	return family;
}

static bool
valid_port(char *cursor)
{
	const char *str = cursor;
	/* Missing port number */
	if (!*str)
		return false;

	/* Must be entirely digits - no spaces or +/- */
	while (*cursor) {
		if (!isdigit(*cursor))
			return false;
		else
			++cursor;
	}

	/* Must be in range */
	errno = 0;
	long port = strtol(str, NULL, 10);

	if ((errno == ERANGE && (port == LONG_MAX || port == LONG_MIN))
		|| (errno != 0 && port == 0) || (port > USHRT_MAX))
		return false;

	return true;
}

static void
split_address_and_port(const char *arg, char **address, char **port_str)
{
	char *cursor = strchr(arg, '[');

	if (cursor) {
		/* IPv6 address with port*/
		char *start = cursor + 1;

		cursor = strchr(start, ']');
		if (start == cursor) {
			exit_error(PARAMETER_PROBLEM,
				   "No IPv6 address specified");
		} else if (!cursor) {
			exit_error(PARAMETER_PROBLEM,
				   "No closing ']' around IPv6 address");
		}
		size_t len = cursor - start;

		cursor = strchr(cursor, ':');
		if (cursor) {
			/* Copy address only if there is a port */
			*address = strndup(start, len);
		}
	} else {
		cursor = strchr(arg, ':');
		if (cursor && !strchr(cursor + 1, ':')) {
			/* IPv4 address with port */
			*address = strndup(arg, cursor - arg);
		} else {
			/* v6 address */
			cursor = NULL;
		}
	}
	if (cursor) {
		/* Parse port entry */
		cursor++;
		if (strlen(cursor) == 0) {
			exit_error(PARAMETER_PROBLEM,
				   "No port specified after `:'");
		}
		if (!valid_port(cursor)) {
			exit_error(PARAMETER_PROBLEM,
				   "Invalid port `%s'", cursor);
		}
		*port_str = strdup(cursor);
	} else {
		/* No port colon or more than one colon (ipv6)
		 * assume arg is straight IP address and no port
		 */
		*address = strdup(arg);
	}
}

static void usage(const char *prog)
{
	fprintf(stdout, "Command line interface for the connection "
			"tracking system. Version %s\n", VERSION);
	fprintf(stdout, "Usage: %s [commands] [options]\n", prog);

	fprintf(stdout, "\n%s", usage_commands);
	fprintf(stdout, "\n%s", usage_tables);
	fprintf(stdout, "\n%s", usage_conntrack_parameters);
	fprintf(stdout, "\n%s", usage_expectation_parameters);
	fprintf(stdout, "\n%s", usage_update_parameters);
	fprintf(stdout, "\n%s\n", usage_parameters);
}

static unsigned int output_mask;

static int
filter_label(const struct nf_conntrack *ct, const struct ct_tmpl *tmpl)
{
	if (tmpl->label == NULL)
		return 0;

	const struct nfct_bitmask *ctb = nfct_get_attr(ct, ATTR_CONNLABELS);
	if (ctb == NULL)
		return 1;

	for (unsigned int i = 0; i <= nfct_bitmask_maxbit(tmpl->label); i++) {
		if (nfct_bitmask_test_bit(tmpl->label, i) &&
		    !nfct_bitmask_test_bit(ctb, i))
				return 1;
	}

	return 0;
}

static int filter_mark(const struct ct_cmd *cmd, const struct nf_conntrack *ct)
{
	const struct ct_tmpl *tmpl = &cmd->tmpl;

	if ((cmd->options & CT_OPT_MARK) &&
	     !mark_cmp(&tmpl->mark, ct))
		return 1;
	return 0;
}

static int filter_nat(const struct ct_cmd *cmd, const struct nf_conntrack *ct)
{
	int check_srcnat = cmd->options & CT_OPT_SRC_NAT ? 1 : 0;
	int check_dstnat = cmd->options & CT_OPT_DST_NAT ? 1 : 0;
	struct nf_conntrack *obj = cmd->tmpl.ct;
	int has_srcnat = 0, has_dstnat = 0;
	uint32_t ip;
	uint16_t port;

	if (cmd->options & CT_OPT_ANY_NAT)
		check_srcnat = check_dstnat = 1;

	if (check_srcnat) {
		int check_address = 0, check_port = 0;

		if (nfct_attr_is_set(obj, ATTR_SNAT_IPV4)) {
			check_address = 1;
			ip = nfct_get_attr_u32(obj, ATTR_SNAT_IPV4);
			if (nfct_getobjopt(ct, NFCT_GOPT_IS_SNAT) &&
			    ip == nfct_get_attr_u32(ct, ATTR_REPL_IPV4_DST))
				has_srcnat = 1;
		}
		if (nfct_attr_is_set(obj, ATTR_SNAT_PORT)) {
			int ret = 0;

			check_port = 1;
			port = nfct_get_attr_u16(obj, ATTR_SNAT_PORT);
			if (nfct_getobjopt(ct, NFCT_GOPT_IS_SPAT) &&
			    port == nfct_get_attr_u16(ct, ATTR_REPL_PORT_DST))
				ret = 1;

			/* the address matches but the port does not. */
			if (check_address && has_srcnat && !ret)
				has_srcnat = 0;
			if (!check_address && ret)
				has_srcnat = 1;
		}
		if (!check_address && !check_port &&
		    (nfct_getobjopt(ct, NFCT_GOPT_IS_SNAT) ||
		     nfct_getobjopt(ct, NFCT_GOPT_IS_SPAT)))
		  	has_srcnat = 1;
	}
	if (check_dstnat) {
		int check_address = 0, check_port = 0;

		if (nfct_attr_is_set(obj, ATTR_DNAT_IPV4)) {
			check_address = 1;
			ip = nfct_get_attr_u32(obj, ATTR_DNAT_IPV4);
			if (nfct_getobjopt(ct, NFCT_GOPT_IS_DNAT) &&
			    ip == nfct_get_attr_u32(ct, ATTR_REPL_IPV4_SRC))
				has_dstnat = 1;
		}
		if (nfct_attr_is_set(obj, ATTR_DNAT_PORT)) {
			int ret = 0;

			check_port = 1;
			port = nfct_get_attr_u16(obj, ATTR_DNAT_PORT);
			if (nfct_getobjopt(ct, NFCT_GOPT_IS_DPAT) &&
			    port == nfct_get_attr_u16(ct, ATTR_REPL_PORT_SRC))
				ret = 1;

			/* the address matches but the port does not. */
			if (check_address && has_dstnat && !ret)
				has_dstnat = 0;
			if (!check_address && ret)
				has_dstnat = 1;
		}
		if (!check_address && !check_port &&
		    (nfct_getobjopt(ct, NFCT_GOPT_IS_DNAT) ||
		     nfct_getobjopt(ct, NFCT_GOPT_IS_DPAT)))
			has_dstnat = 1;
	}
	if (cmd->options & CT_OPT_ANY_NAT)
		return !(has_srcnat || has_dstnat);
	else if ((cmd->options & CT_OPT_SRC_NAT) &&
		 (cmd->options & CT_OPT_DST_NAT))
		return !(has_srcnat && has_dstnat);
	else if (cmd->options & CT_OPT_SRC_NAT)
		return !has_srcnat;
	else if (cmd->options & CT_OPT_DST_NAT)
		return !has_dstnat;

	return 0;
}

static int
nfct_ip6_net_cmp(const union ct_address *addr, const struct ct_network *net)
{
	int i;
	for (i=0;i<4;i++)
		if ((addr->v6[i] & net->netmask.v6[i]) != net->network.v6[i])
			return 1;
	return 0;
}

static int
nfct_ip_net_cmp(int family, const union ct_address *addr,
		const struct ct_network *net)
{
	switch(family) {
	case AF_INET:
		return (addr->v4 & net->netmask.v4) != net->network.v4;
	case AF_INET6:
		return nfct_ip6_net_cmp(addr, net);
	default:
		return 0;
	}
}

static int
nfct_filter_network_direction(const struct nf_conntrack *ct, enum ct_direction dir)
{
	const int family = filter_family;
	const union ct_address *address;
	enum nf_conntrack_attr attr;
	struct ct_network *net = &dir2network[dir];

	if (nfct_get_attr_u8(ct, ATTR_ORIG_L3PROTO) != family)
		return 1;

	attr = famdir2attr[family == AF_INET6][dir];
	address = nfct_get_attr(ct, attr);

	return nfct_ip_net_cmp(family, address, net);
}

static int
filter_network(const struct ct_cmd *cmd, const struct nf_conntrack *ct)
{
	if (cmd->options & CT_OPT_MASK_SRC) {
		if (nfct_filter_network_direction(ct, DIR_SRC))
			return 1;
	}

	if (cmd->options & CT_OPT_MASK_DST) {
		if (nfct_filter_network_direction(ct, DIR_DST))
			return 1;
	}
	return 0;
}

static int
nfct_filter(struct ct_cmd *cmd, struct nf_conntrack *ct,
	    const struct ct_tmpl *tmpl)
{
	struct nf_conntrack *obj = cmd->tmpl.ct;

	if (filter_nat(cmd, ct) ||
	    filter_mark(cmd, ct) ||
	    filter_label(ct, tmpl) ||
	    filter_network(cmd, ct))
		return 1;

	if (cmd->options & CT_COMPARISON &&
	    !nfct_cmp(obj, ct, NFCT_CMP_ALL | NFCT_CMP_MASK))
		return 1;

	return 0;
}

static int counter;
static int dump_xml_header_done = 1;

static void __attribute__((noreturn))
event_sighandler(int s)
{
	if (dump_xml_header_done == 0) {
		printf("</conntrack>\n");
		fflush(stdout);
	}

	fprintf(stderr, "%s v%s (conntrack-tools): ", PROGNAME, VERSION);
	fprintf(stderr, "%d flow events have been shown.\n", counter);
	mnl_socket_close(_sock.mnl);
	exit(0);
}

static void __attribute__((noreturn))
exp_event_sighandler(int s)
{
	if (dump_xml_header_done == 0) {
		printf("</expect>\n");
		fflush(stdout);
	}

	fprintf(stderr, "%s v%s (conntrack-tools): ", PROGNAME, VERSION);
	fprintf(stderr, "%d expectation events have been shown.\n", counter);
	nfct_close(cth);
	exit(0);
}

static char *pid2name(pid_t pid)
{
	char procname[256], *prog;
	FILE *fp;
	int ret;

	ret = snprintf(procname, sizeof(procname), "/proc/%lu/stat", (unsigned long)pid);
	if (ret < 0 || ret > (int)sizeof(procname))
		return NULL;

	fp = fopen(procname, "r");
	if (!fp)
		return NULL;

	ret = fscanf(fp, "%*u (%m[^)]", &prog);

	fclose(fp);

	if (ret == 1)
		return prog;

	return NULL;
}

static char *portid2name(pid_t pid, uint32_t portid, unsigned long inode)
{
	const struct dirent *ent;
	char procname[256];
	DIR *dir;
	int ret;

	ret = snprintf(procname, sizeof(procname), "/proc/%lu/fd/", (unsigned long)pid);
	if (ret < 0 || ret >= (int)sizeof(procname))
		return NULL;

	dir = opendir(procname);
	if (!dir)
		return NULL;

	for (;;) {
		unsigned long ino;
		char tmp[128];
		ssize_t rl;

		ent = readdir(dir);
		if (!ent)
			break;

		if (ent->d_type != DT_LNK)
			continue;

		ret = snprintf(procname, sizeof(procname), "/proc/%d/fd/%s",
			       pid, ent->d_name);
		if (ret < 0 || ret >= (int)sizeof(procname))
			continue;

		rl = readlink(procname, tmp, sizeof(tmp));
		if (rl <= 0 || rl >= (ssize_t)sizeof(tmp))
			continue;

		tmp[rl] = 0;

		ret = sscanf(tmp, "socket:[%lu]", &ino);
		if (ret == 1 && ino == inode) {
			closedir(dir);
			return pid2name(pid);
		}
	}

	closedir(dir);
	return NULL;
}

static char *name_by_portid(uint32_t portid, unsigned long inode)
{
	const struct dirent *ent;
	char *prog;
	DIR *dir;

	/* Many netlink users use their process ID to allocate the first port id. */
	prog = portid2name(portid, portid, inode);
	if (prog)
		return prog;

	/* no luck, search harder. */
	dir = opendir("/proc");
	if (!dir)
		return NULL;

	for (;;) {
		unsigned long pid;
		char *end;

		ent = readdir(dir);
		if (!ent)
			break;

		if (ent->d_type != DT_DIR)
			continue;

		pid = strtoul(ent->d_name, &end, 10);
		if (pid <= 1 || *end)
			continue;

		if (pid == portid) /* already tried */
			continue;

		prog = portid2name(pid, portid, inode);
		if (prog)
			break;
	}

	closedir(dir);
	return prog;
}

static char *get_progname(uint32_t portid)
{
	FILE *fp = fopen("/proc/net/netlink", "r");
	uint32_t portid_check;
	unsigned long inode;
	int ret, prot;

	if (!fp)
		return NULL;

	for (;;) {
		char line[256];

		if (!fgets(line, sizeof(line), fp))
			break;

		ret = sscanf(line, "%*x %d %u %*x %*d %*d %*x %*d %*u %lu\n",
			     &prot, &portid_check, &inode);

		if (ret == EOF)
			break;

		if (ret == 3 && portid_check == portid && prot == NETLINK_NETFILTER) {
			static uint32_t last_portid;
			static uint32_t last_inode;
			static char *last_program;
			char *prog;

			fclose(fp);

			if (last_portid == portid && last_inode == inode)
				return last_program;

			prog = name_by_portid(portid, inode);

			free(last_program);
			last_program = prog;
			last_portid = portid;
			last_inode = inode;
			return prog;
		}
	}

	fclose(fp);
	return NULL;
}

static void event_print_timestamp(const struct nf_conntrack *ct)
{
	struct timeval tv;

	/* nfct_attr_is_set returns -1 if attr is unknown */
	if (nfct_attr_is_set(ct, ATTR_TIMESTAMP_EVENT) > 0) {
		uint64_t event_ts = nfct_get_attr_u64(ct, ATTR_TIMESTAMP_EVENT);
		static const uint64_t ns_per_s = 1000000000ul;
		static const uint64_t ns_to_usec = 1000ul;

		tv.tv_sec = event_ts / ns_per_s;
		tv.tv_usec = (event_ts % ns_per_s) / ns_to_usec;
	} else {
		gettimeofday(&tv, NULL);
	}

	printf("[%-.8ld.%-.6ld]\t", tv.tv_sec, tv.tv_usec);
}

static int event_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nfgenmsg *nfh = mnl_nlmsg_get_payload(nlh);
	unsigned int op_type = NFCT_O_DEFAULT;
	enum nf_conntrack_msg_type type;
	struct ct_cmd *cmd = data;
	unsigned int op_flags = 0;
	struct nf_conntrack *ct;
	char buf[1024];

	switch(nlh->nlmsg_type & 0xff) {
	case IPCTNL_MSG_CT_NEW:
		if (nlh->nlmsg_flags & NLM_F_CREATE)
			type = NFCT_T_NEW;
		else
			type = NFCT_T_UPDATE;
		break;
	case IPCTNL_MSG_CT_DELETE:
		type = NFCT_T_DESTROY;
		break;
	default:
		/* Unknown event type. */
		type = 0;
		break;
	}

	ct = nfct_new();
	if (!ct)
		goto out;

	if (nfct_nlmsg_parse(nlh, ct) < 0)
		goto out;

	if ((filter_family != AF_UNSPEC &&
	     filter_family != nfh->nfgen_family) ||
	    nfct_filter(cmd, ct, cur_tmpl))
		goto out;

	if (output_mask & _O_SAVE) {
		ct_save_snprintf(buf, sizeof(buf), ct, labelmap, type);
		goto done;
	}

	if (output_mask & _O_XML) {
		op_type = NFCT_O_XML;
		if (dump_xml_header_done) {
			dump_xml_header_done = 0;
			printf("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
			       "<conntrack>\n");
		}
	}
	if (output_mask & _O_EXT)
		op_flags = NFCT_OF_SHOW_LAYER3;
	if (output_mask & _O_TMS) {
		if (!(output_mask & _O_XML)) {
			event_print_timestamp(ct);
		} else
			op_flags |= NFCT_OF_TIME;
	}
	if (output_mask & _O_KTMS)
		op_flags |= NFCT_OF_TIMESTAMP;
	if (output_mask & _O_ID)
		op_flags |= NFCT_OF_ID;

	nfct_snprintf_labels(buf, sizeof(buf), ct, type, op_type, op_flags, labelmap);
done:
	if (nlh->nlmsg_pid && !(output_mask & _O_XML)) {
		char *prog = get_progname(nlh->nlmsg_pid);

		if (prog)
			printf("%s [USERSPACE] portid=%u progname=%s\n", buf, nlh->nlmsg_pid, prog);
		else
			printf("%s [USERSPACE] portid=%u\n", buf, nlh->nlmsg_pid);
	} else {
		puts(buf);
	}
	fflush(stdout);

	counter++;
out:
	nfct_destroy(ct);
	return MNL_CB_OK;
}

static int nfct_mnl_request(struct nfct_mnl_socket *sock, uint16_t subsys,
			    int family, uint16_t type, uint16_t flags,
			    mnl_cb_t cb, const struct nf_conntrack *ct,
			    const struct ct_cmd *cmd);

static int mnl_nfct_delete_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nfct_mnl_socket *modifier_sock = &_modifier_sock;
	unsigned int op_type = NFCT_O_DEFAULT;
	unsigned int op_flags = 0;
	struct ct_cmd *cmd = data;
	struct nf_conntrack *ct;
	char buf[1024];
	int res;

	ct = nfct_new();
	if (ct == NULL)
		return MNL_CB_OK;

	nfct_nlmsg_parse(nlh, ct);

	if (nfct_filter(cmd, ct, cur_tmpl))
		goto destroy_ok;

	res = nfct_mnl_request(modifier_sock, NFNL_SUBSYS_CTNETLINK,
			       nfct_get_attr_u8(ct, ATTR_ORIG_L3PROTO),
			       IPCTNL_MSG_CT_DELETE, NLM_F_ACK, NULL, ct, NULL);
	if (res < 0) {
		/* the entry has vanish in middle of the delete */
		if (errno == ENOENT)
			goto destroy_ok;
		exit_error(OTHER_PROBLEM,
			   "Operation failed: %s",
			   err2str(errno, CT_DELETE));
	}

	if (output_mask & _O_SAVE) {
		ct_save_snprintf(buf, sizeof(buf), ct, labelmap, NFCT_T_DESTROY);
		goto done;
	}

	if (output_mask & _O_XML)
		op_type = NFCT_O_XML;
	if (output_mask & _O_EXT)
		op_flags = NFCT_OF_SHOW_LAYER3;
	if (output_mask & _O_ID)
		op_flags |= NFCT_OF_ID;

	nfct_snprintf(buf, sizeof(buf), ct, NFCT_T_UNKNOWN, op_type, op_flags);
done:
	printf("%s\n", buf);

	counter++;

destroy_ok:
	nfct_destroy(ct);
	return MNL_CB_OK;
}

static int mnl_nfct_print_cb(const struct nlmsghdr *nlh, void *data)
{
	unsigned int op_type = NFCT_O_DEFAULT;
	unsigned int op_flags = 0;
	struct nf_conntrack *ct;
	char buf[1024];

	ct = nfct_new();
	if (ct == NULL)
		return MNL_CB_OK;

	nfct_nlmsg_parse(nlh, ct);

	if (output_mask & _O_SAVE) {
		ct_save_snprintf(buf, sizeof(buf), ct, labelmap, NFCT_T_NEW);
		goto done;
	}

	if (output_mask & _O_XML)
		op_type = NFCT_O_XML;
	if (output_mask & _O_EXT)
		op_flags = NFCT_OF_SHOW_LAYER3;
	if (output_mask & _O_ID)
		op_flags |= NFCT_OF_ID;

	nfct_snprintf_labels(buf, sizeof(buf), ct, NFCT_T_UNKNOWN, op_type, op_flags, labelmap);
done:
	printf("%s\n", buf);

	nfct_destroy(ct);

	return MNL_CB_OK;
}

static void copy_mark(const struct ct_cmd *cmd, struct nf_conntrack *tmp,
		      const struct nf_conntrack *ct,
		      const struct u32_mask *m)
{
	if (cmd->options & CT_OPT_MARK) {
		uint32_t mark = nfct_get_attr_u32(ct, ATTR_MARK);
		mark = (mark & ~m->mask) ^ m->value;
		nfct_set_attr_u32(tmp, ATTR_MARK, mark);
	}
}

static void copy_status(const struct ct_cmd *cmd, struct nf_conntrack *tmp,
			const struct nf_conntrack *ct)
{
	if (cmd->options & CT_OPT_STATUS) {
		/* copy existing flags, we only allow setting them. */
		uint32_t status = nfct_get_attr_u32(ct, ATTR_STATUS);
		status |= nfct_get_attr_u32(tmp, ATTR_STATUS);
		nfct_set_attr_u32(tmp, ATTR_STATUS, status);
	}
}

static struct nfct_bitmask *xnfct_bitmask_clone(const struct nfct_bitmask *a)
{
	struct nfct_bitmask *b = nfct_bitmask_clone(a);
	if (!b)
		exit_error(OTHER_PROBLEM, "out of memory");
	return b;
}

static void copy_label(const struct ct_cmd *cmd, struct nf_conntrack *tmp,
		       const struct nf_conntrack *ct,
		       const struct ct_tmpl *tmpl)
{
	struct nfct_bitmask *ctb, *newmask;
	unsigned int i;

	if ((cmd->options & (CT_OPT_ADD_LABEL|CT_OPT_DEL_LABEL)) == 0)
		return;

	nfct_copy_attr(tmp, ct, ATTR_CONNLABELS);
	ctb = (void *) nfct_get_attr(tmp, ATTR_CONNLABELS);

	if (cmd->options & CT_OPT_ADD_LABEL) {
		if (ctb == NULL) {
			nfct_set_attr(tmp, ATTR_CONNLABELS,
					xnfct_bitmask_clone(tmpl->label_modify));
			return;
		}
		/* If we send a bitmask shorter than the kernel sent to us, the bits we
		 * omit will be cleared (as "padding").  So we always have to send the
		 * same sized bitmask as we received.
		 *
		 * Mask has to have the same size as the labels, otherwise it will not
		 * be encoded by libnetfilter_conntrack, as different sizes are not
		 * accepted by the kernel.
		 */
		newmask = nfct_bitmask_new(nfct_bitmask_maxbit(ctb));

		for (i = 0; i <= nfct_bitmask_maxbit(ctb); i++) {
			if (nfct_bitmask_test_bit(tmpl->label_modify, i)) {
				nfct_bitmask_set_bit(ctb, i);
				nfct_bitmask_set_bit(newmask, i);
			} else if (nfct_bitmask_test_bit(ctb, i)) {
				/* Kernel only retains old bit values that are sent as
				 * zeroes in BOTH labels and mask.
				 */
				nfct_bitmask_unset_bit(ctb, i);
			}
		}
		nfct_set_attr(tmp, ATTR_CONNLABELS_MASK, newmask);
	} else if (ctb != NULL) {
		/* CT_OPT_DEL_LABEL */
		if (tmpl->label_modify == NULL) {
			newmask = nfct_bitmask_new(0);
			if (newmask)
				nfct_set_attr(tmp, ATTR_CONNLABELS, newmask);
			return;
		}

		for (i = 0; i <= nfct_bitmask_maxbit(ctb); i++) {
			if (nfct_bitmask_test_bit(tmpl->label_modify, i))
				nfct_bitmask_unset_bit(ctb, i);
		}

		newmask = xnfct_bitmask_clone(tmpl->label_modify);
		nfct_set_attr(tmp, ATTR_CONNLABELS_MASK, newmask);
	}
}

static int mnl_nfct_update_cb(const struct nlmsghdr *nlh, void *data)
{
	struct ct_cmd *cmd = data;
	struct nfct_mnl_socket *modifier_sock = &_modifier_sock;
	struct nf_conntrack *ct, *obj = cmd->tmpl.ct, *tmp = NULL;
	int res;

	ct = nfct_new();
	if (ct == NULL)
		return MNL_CB_OK;

	nfct_nlmsg_parse(nlh, ct);

	if (filter_nat(cmd, ct) ||
	    filter_label(ct, cur_tmpl) ||
	    filter_network(cmd, ct))
		goto destroy_ok;

	if (nfct_attr_is_set(obj, ATTR_ID) && nfct_attr_is_set(ct, ATTR_ID) &&
	    nfct_get_attr_u32(obj, ATTR_ID) != nfct_get_attr_u32(ct, ATTR_ID))
		goto destroy_ok;

	if (cmd->options & CT_OPT_TUPLE_ORIG &&
	    !nfct_cmp(obj, ct, NFCT_CMP_ORIG))
		goto destroy_ok;
	if (cmd->options & CT_OPT_TUPLE_REPL &&
	    !nfct_cmp(obj, ct, NFCT_CMP_REPL))
		goto destroy_ok;

	tmp = nfct_new();
	if (tmp == NULL)
		exit_error(OTHER_PROBLEM, "out of memory");

	nfct_copy(tmp, ct, NFCT_CP_ORIG);
	nfct_copy(tmp, obj, NFCT_CP_META);
	copy_mark(cmd, tmp, ct, &cur_tmpl->mark);
	copy_status(cmd, tmp, ct);
	copy_label(cmd, tmp, ct, cur_tmpl);

	/* do not send NFCT_Q_UPDATE if ct appears unchanged */
	if (nfct_cmp(tmp, ct, NFCT_CMP_ALL | NFCT_CMP_MASK))
		goto destroy_ok;

	res = nfct_mnl_request(modifier_sock, NFNL_SUBSYS_CTNETLINK,
			       nfct_get_attr_u8(ct, ATTR_ORIG_L3PROTO),
			       IPCTNL_MSG_CT_NEW, NLM_F_ACK, NULL, tmp, NULL);
	if (res < 0) {
		/* the entry has vanish in middle of the update */
		if (errno == ENOENT)
			goto destroy_ok;
		else if (cmd->options & (CT_OPT_ADD_LABEL | CT_OPT_DEL_LABEL) &&
			 !nfct_attr_is_set(ct, ATTR_CONNLABELS) &&
			 errno == ENOSPC)
			goto destroy_ok;

		exit_error(OTHER_PROBLEM,
			   "Operation failed: %s",
			   err2str(errno, CT_UPDATE));
	}

	res = nfct_mnl_request(modifier_sock, NFNL_SUBSYS_CTNETLINK,
			       nfct_get_attr_u8(ct, ATTR_ORIG_L3PROTO),
			       IPCTNL_MSG_CT_GET, 0,
			       mnl_nfct_print_cb, tmp, NULL);
	if (res < 0) {
		/* the entry has vanish in middle of the update */
		if (errno == ENOENT)
			goto destroy_ok;
		exit_error(OTHER_PROBLEM,
			   "Operation failed: %s",
			   err2str(errno, CT_UPDATE));
	}

	counter++;

destroy_ok:
	if (tmp)
		nfct_destroy(tmp);
	nfct_destroy(ct);

	return MNL_CB_OK;
}

static int dump_exp_cb(enum nf_conntrack_msg_type type,
		      struct nf_expect *exp,
		      void *data)
{
	char buf[1024];
	unsigned int op_type = NFCT_O_DEFAULT;
	unsigned int op_flags = 0;

	if (output_mask & _O_XML) {
		op_type = NFCT_O_XML;
		if (dump_xml_header_done) {
			dump_xml_header_done = 0;
			printf("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
			       "<expect>\n");
		}
	}
	if (output_mask & _O_TMS) {
		if (!(output_mask & _O_XML)) {
			struct timeval tv;
			gettimeofday(&tv, NULL);
			printf("[%-8ld.%-6ld]\t", tv.tv_sec, tv.tv_usec);
		} else
			op_flags |= NFCT_OF_TIME;
	}

	nfexp_snprintf(buf,sizeof(buf), exp, NFCT_T_UNKNOWN, op_type, op_flags);
	printf("%s\n", buf);
	counter++;

	return NFCT_CB_CONTINUE;
}

static int event_exp_cb(enum nf_conntrack_msg_type type,
			struct nf_expect *exp, void *data)
{
	char buf[1024];
	unsigned int op_type = NFCT_O_DEFAULT;
	unsigned int op_flags = 0;

	if (output_mask & _O_XML) {
		op_type = NFCT_O_XML;
		if (dump_xml_header_done) {
			dump_xml_header_done = 0;
			printf("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
			       "<expect>\n");
		}
	}
	if (output_mask & _O_TMS) {
		if (!(output_mask & _O_XML)) {
			struct timeval tv;
			gettimeofday(&tv, NULL);
			printf("[%-8ld.%-6ld]\t", tv.tv_sec, tv.tv_usec);
		} else
			op_flags |= NFCT_OF_TIME;
	}

	nfexp_snprintf(buf,sizeof(buf), exp, type, op_type, op_flags);
	printf("%s\n", buf);
	fflush(stdout);
	counter++;

	return NFCT_CB_CONTINUE;
}

static int count_exp_cb(enum nf_conntrack_msg_type type,
			struct nf_expect *exp,
			void *data)
{
	counter++;
	return NFCT_CB_CONTINUE;
}

#ifndef CT_STATS_PROC
#define CT_STATS_PROC "/proc/net/stat/nf_conntrack"
#endif

/* As of 2.6.29, we have 16 entries, this is enough */
#ifndef CT_STATS_ENTRIES_MAX
#define CT_STATS_ENTRIES_MAX 64
#endif

/* maximum string length currently is 13 characters */
#ifndef CT_STATS_STRING_MAX
#define CT_STATS_STRING_MAX 64
#endif

static int display_proc_conntrack_stats(void)
{
	int ret = 0;
	FILE *fd;
	char buf[4096], *token, *nl;
	char output[CT_STATS_ENTRIES_MAX][CT_STATS_STRING_MAX];
	unsigned int value[CT_STATS_ENTRIES_MAX], i, max;
	int cpu;

	fd = fopen(CT_STATS_PROC, "r");
	if (fd == NULL)
		return -1;

	if (fgets(buf, sizeof(buf), fd) == NULL) {
		ret = -1;
		goto out_err;
	}

	/* trim off trailing \n */
	nl = strchr(buf, '\n');
	if (nl != NULL)
		*nl = '\0';

	token = strtok(buf, " ");
	for (i=0; token != NULL && i<CT_STATS_ENTRIES_MAX; i++) {
		strncpy(output[i], token, CT_STATS_STRING_MAX);
		output[i][CT_STATS_STRING_MAX-1]='\0';
		token = strtok(NULL, " ");
	}
	max = i;

	for (cpu = 0; fgets(buf, sizeof(buf), fd) != NULL; cpu++) {
		nl = strchr(buf, '\n');
		while (nl != NULL) {
			*nl = '\0';
			nl = strchr(buf, '\n');
		}
		token = strtok(buf, " ");
		for (i = 0; token != NULL && i < CT_STATS_ENTRIES_MAX; i++) {
			value[i] = (unsigned int) strtol(token, (char**) NULL, 16);
			token = strtok(NULL, " ");
		}

		printf("cpu=%-4u\t", cpu);
		for (i = 0; i < max; i++)
			printf("%s=%u ", output[i], value[i]);
		printf("\n");
	}
	if (cpu == 0)
		ret = -1;

out_err:
	fclose(fd);
	return ret;
}

static int nfct_mnl_socket_open(struct nfct_mnl_socket *socket,
				unsigned int events)
{
	socket->mnl = mnl_socket_open(NETLINK_NETFILTER);
	if (socket->mnl == NULL) {
		perror("mnl_socket_open");
		return -1;
	}
	if (mnl_socket_bind(socket->mnl, events, MNL_SOCKET_AUTOPID) < 0) {
		perror("mnl_socket_bind");
		return -1;
	}
	socket->portid = mnl_socket_get_portid(socket->mnl);

	return 0;
}

static struct nlmsghdr *
nfct_mnl_nlmsghdr_put(char *buf, uint16_t subsys, uint16_t type,
		      uint16_t flags, uint8_t family)
{
	struct nlmsghdr *nlh;
	struct nfgenmsg *nfh;

	nlh = mnl_nlmsg_put_header(buf);
	nlh->nlmsg_type = (subsys << 8) | type;
	nlh->nlmsg_flags = NLM_F_REQUEST | flags;
	nlh->nlmsg_seq = time(NULL);

	nfh = mnl_nlmsg_put_extra_header(nlh, sizeof(struct nfgenmsg));
	nfh->nfgen_family = family;
	nfh->version = NFNETLINK_V0;
	nfh->res_id = 0;

	return nlh;
}

static void nfct_mnl_socket_close(const struct nfct_mnl_socket *sock)
{
	mnl_socket_close(sock->mnl);
}

static int nfct_mnl_socket_check_open(struct nfct_mnl_socket *socket,
				       unsigned int events)
{
	if (socket->mnl != NULL)
		return 0;

	return nfct_mnl_socket_open(socket, events);
}

static void nfct_mnl_socket_check_close(struct nfct_mnl_socket *sock)
{
	if (sock->mnl) {
		nfct_mnl_socket_close(sock);
		memset(sock, 0, sizeof(*sock));
	}
}

static int __nfct_mnl_dump(struct nfct_mnl_socket *sock,
			   const struct nlmsghdr *nlh, mnl_cb_t cb, void *data)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	int res;

	res = mnl_socket_sendto(sock->mnl, nlh, nlh->nlmsg_len);
	if (res < 0)
		return res;

	res = mnl_socket_recvfrom(sock->mnl, buf, sizeof(buf));
	while (res > 0) {
		res = mnl_cb_run(buf, res, nlh->nlmsg_seq, sock->portid,
				 cb, data);
		if (res <= MNL_CB_STOP)
			break;

		res = mnl_socket_recvfrom(sock->mnl, buf, sizeof(buf));
	}

	return res;
}

static int
nfct_mnl_dump(struct nfct_mnl_socket *sock, uint16_t subsys, uint16_t type,
	      mnl_cb_t cb, struct ct_cmd *cmd,
	      const struct nfct_filter_dump *filter_dump)
{
	uint8_t family = cmd ? cmd->family : AF_UNSPEC;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;

	nlh = nfct_mnl_nlmsghdr_put(buf, subsys, type, NLM_F_DUMP, family);

	if (filter_dump)
		nfct_nlmsg_build_filter(nlh, filter_dump);

	return __nfct_mnl_dump(sock, nlh, cb, cmd);
}

static int nfct_mnl_talk(struct nfct_mnl_socket *sock,
			 const struct nlmsghdr *nlh, mnl_cb_t cb,
			 const struct ct_cmd *cmd)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	int ret;

	ret = mnl_socket_sendto(sock->mnl, nlh, nlh->nlmsg_len);
	if (ret < 0)
		return ret;

	ret = mnl_socket_recvfrom(sock->mnl, buf, sizeof(buf));
	if (ret < 0)
		return ret;

	return mnl_cb_run(buf, ret, nlh->nlmsg_seq, sock->portid, cb, (void *)cmd);
}

static int nfct_mnl_request(struct nfct_mnl_socket *sock, uint16_t subsys,
			    int family, uint16_t type, uint16_t flags,
			    mnl_cb_t cb, const struct nf_conntrack *ct,
			    const struct ct_cmd *cmd)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	int err;

	nlh = nfct_mnl_nlmsghdr_put(buf, subsys, type, flags, family);

	if (ct) {
		err = nfct_nlmsg_build(nlh, ct);
		if (err < 0)
			return err;
	}

	return nfct_mnl_talk(sock, nlh, cb, cmd);
}

#define UNKNOWN_STATS_NUM 4

static int nfct_stats_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, CTA_STATS_MAX + UNKNOWN_STATS_NUM) < 0)
		return MNL_CB_OK;

	if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0) {
		perror("mnl_attr_validate");
		return MNL_CB_ERROR;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static int nfct_stats_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nlattr *tb[CTA_STATS_MAX + UNKNOWN_STATS_NUM + 1] = {};
	struct nfgenmsg *nfg = mnl_nlmsg_get_payload(nlh);
	const char *attr2name[CTA_STATS_MAX + UNKNOWN_STATS_NUM + 1] = {
		[CTA_STATS_SEARCHED]	= "searched",
		[CTA_STATS_FOUND]	= "found",
		[CTA_STATS_NEW]		= "new",
		[CTA_STATS_INVALID]	= "invalid",
		[CTA_STATS_IGNORE]	= "ignore",
		[CTA_STATS_DELETE]	= "delete",
		[CTA_STATS_DELETE_LIST]	= "delete_list",
		[CTA_STATS_INSERT]	= "insert",
		[CTA_STATS_INSERT_FAILED] = "insert_failed",
		[CTA_STATS_DROP]	= "drop",
		[CTA_STATS_EARLY_DROP]	= "early_drop",
		[CTA_STATS_ERROR]	= "error",
		[CTA_STATS_SEARCH_RESTART] = "search_restart",
		[CTA_STATS_CLASH_RESOLVE] = "clash_resolve",
		[CTA_STATS_CHAIN_TOOLONG] = "chaintoolong",

		/* leave at end.  Allows to show counters supported
		 * by newer kernel with older conntrack-tools release.
		 */
		[CTA_STATS_MAX + 1] = "unknown1",
		[CTA_STATS_MAX + 2] = "unknown2",
		[CTA_STATS_MAX + 3] = "unknown3",
		[CTA_STATS_MAX + 4] = "unknown4",
	};
	int i;

	mnl_attr_parse(nlh, sizeof(*nfg), nfct_stats_attr_cb, tb);

	printf("cpu=%-4u\t", ntohs(nfg->res_id));

	for (i=0; i <= CTA_STATS_MAX + UNKNOWN_STATS_NUM; i++) {
		if (tb[i]) {
			printf("%s=%u ",
				attr2name[i], ntohl(mnl_attr_get_u32(tb[i])));
		}
	}
	printf("\n");
	return MNL_CB_OK;
}

static int nfexp_stats_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, CTA_STATS_EXP_MAX) < 0)
		return MNL_CB_OK;

	if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0) {
		perror("mnl_attr_validate");
		return MNL_CB_ERROR;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static int nfexp_stats_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nlattr *tb[CTA_STATS_EXP_MAX+1] = {};
	struct nfgenmsg *nfg = mnl_nlmsg_get_payload(nlh);
	const char *attr2name[CTA_STATS_EXP_MAX+1] = {
		[CTA_STATS_EXP_NEW] = "expect_new",
		[CTA_STATS_EXP_CREATE] = "expect_create",
		[CTA_STATS_EXP_DELETE] = "expect_delete",
	};
	int i;

	mnl_attr_parse(nlh, sizeof(*nfg), nfexp_stats_attr_cb, tb);

	printf("cpu=%-4u\t", ntohs(nfg->res_id));

	for (i=0; i<CTA_STATS_EXP_MAX+1; i++) {
		if (tb[i]) {
			printf("%s=%u ",
				attr2name[i], ntohl(mnl_attr_get_u32(tb[i])));
		}
	}
	printf("\n");
	return MNL_CB_OK;
}

static int nfct_stats_global_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, CTA_STATS_GLOBAL_MAX) < 0)
		return MNL_CB_OK;

	if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0) {
		perror("mnl_attr_validate");
		return MNL_CB_ERROR;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static int nfct_global_stats_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nlattr *tb[CTA_STATS_GLOBAL_MAX+1] = {};
	struct nfgenmsg *nfg = mnl_nlmsg_get_payload(nlh);

	mnl_attr_parse(nlh, sizeof(*nfg), nfct_stats_global_attr_cb, tb);

	if (tb[CTA_STATS_GLOBAL_ENTRIES]) {
		printf("%d\n",
			ntohl(mnl_attr_get_u32(tb[CTA_STATS_GLOBAL_ENTRIES])));
	}
	return MNL_CB_OK;
}

static int mnl_nfct_dump_cb(const struct nlmsghdr *nlh, void *data)
{
	unsigned int op_type = NFCT_O_DEFAULT;
	unsigned int op_flags = 0;
	struct ct_cmd *cmd = data;
	struct nf_conntrack *ct;
	char buf[4096];

	ct = nfct_new();
	if (ct == NULL)
		return MNL_CB_OK;

	nfct_nlmsg_parse(nlh, ct);

	if (nfct_filter(cmd, ct, cur_tmpl)) {
		nfct_destroy(ct);
		return MNL_CB_OK;
	}

	if (output_mask & _O_SAVE) {
		ct_save_snprintf(buf, sizeof(buf), ct, labelmap, NFCT_T_NEW);
		goto done;
	}

	if (output_mask & _O_XML) {
		op_type = NFCT_O_XML;
		if (dump_xml_header_done) {
			dump_xml_header_done = 0;
			printf("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
			       "<conntrack>\n");
		}
	}
	if (output_mask & _O_EXT)
		op_flags = NFCT_OF_SHOW_LAYER3;
	if (output_mask & _O_KTMS)
		op_flags |= NFCT_OF_TIMESTAMP;
	if (output_mask & _O_ID)
		op_flags |= NFCT_OF_ID;

	nfct_snprintf_labels(buf, sizeof(buf), ct, NFCT_T_UNKNOWN, op_type,
			     op_flags, labelmap);
done:
	printf("%s\n", buf);

	nfct_destroy(ct);

	counter++;

	return MNL_CB_OK;
}

static struct ctproto_handler *h;

static void labelmap_init(void)
{
	if (labelmap)
		return;
	labelmap = nfct_labelmap_new(labelmap_path);
	if (!labelmap)
		perror("nfct_labelmap_new");
}

static void
nfct_network_attr_prepare(const int family, enum ct_direction dir,
			  const struct ct_tmpl *tmpl)
{
	const union ct_address *address, *netmask;
	enum nf_conntrack_attr attr;
	int i;
	struct ct_network *net = &dir2network[dir];

	attr = famdir2attr[family == AF_INET6][dir];

	address = nfct_get_attr(tmpl->ct, attr);
	netmask = nfct_get_attr(tmpl->mask, attr);

	switch(family) {
	case AF_INET:
		net->network.v4 = address->v4 & netmask->v4;
		break;
	case AF_INET6:
		for (i=0;i<4;i++)
			net->network.v6[i] = address->v6[i] & netmask->v6[i];
		break;
	}

	memcpy(&net->netmask, netmask, sizeof(union ct_address));

	/* avoid exact source matching */
	nfct_attr_unset(tmpl->ct, attr);
}

static void nfct_filter_init(const struct ct_cmd *cmd)
{
	const struct ct_tmpl *tmpl = &cmd->tmpl;
	int family = cmd->family;

	filter_family = family;
	if (cmd->options & CT_OPT_MASK_SRC) {
		assert(family != AF_UNSPEC);
		if (!(cmd->options & CT_OPT_ORIG_SRC))
			exit_error(PARAMETER_PROBLEM,
			           "Can't use --mask-src without --src");
		nfct_network_attr_prepare(family, DIR_SRC, tmpl);
	}

	if (cmd->options & CT_OPT_MASK_DST) {
		assert(family != AF_UNSPEC);
		if (!(cmd->options & CT_OPT_ORIG_DST))
			exit_error(PARAMETER_PROBLEM,
			           "Can't use --mask-dst without --dst");
		nfct_network_attr_prepare(family, DIR_DST, tmpl);
	}
}

static void merge_bitmasks(struct nfct_bitmask **current,
			  struct nfct_bitmask *src)
{
	unsigned int i;

	if (*current == NULL) {
		*current = src;
		return;
	}

	/* "current" must be the larger bitmask object */
	if (nfct_bitmask_maxbit(src) > nfct_bitmask_maxbit(*current)) {
		struct nfct_bitmask *tmp = *current;
		*current = src;
		src = tmp;
	}

	for (i = 0; i <= nfct_bitmask_maxbit(src); i++) {
		if (nfct_bitmask_test_bit(src, i))
			nfct_bitmask_set_bit(*current, i);
	}

	nfct_bitmask_destroy(src);
}


static void
nfct_build_netmask(uint32_t *dst, int b, int n)
{
	int i;

	for (i = 0; i < n; i++) {
		if (b >= 32) {
			dst[i] = 0xffffffff;
			b -= 32;
		} else if (b > 0) {
			dst[i] = htonl(~0u << (32 - b));
			b = 0;
		} else {
			dst[i] = 0;
		}
	}
}

static void
nfct_set_addr_only(const int opt, struct nf_conntrack *ct, union ct_address *ad,
		   const int l3protonum)
{
	switch (l3protonum) {
	case AF_INET:
		nfct_set_attr_u32(ct,
		                  opt2family_attr[opt][0],
		                  ad->v4);
		break;
	case AF_INET6:
		nfct_set_attr(ct,
		              opt2family_attr[opt][1],
		              &ad->v6);
		break;
	}
}

static void
nfct_set_addr_opt(const int opt, struct nf_conntrack *ct, union ct_address *ad,
		  const int l3protonum, unsigned int *options)
{
	*options |= opt2type[opt];
	nfct_set_addr_only(opt, ct, ad, l3protonum);
	nfct_set_attr_u8(ct, opt2attr[opt], l3protonum);
}

static void
nfct_parse_addr_from_opt(const int opt, const char *arg,
			 struct nf_conntrack *ct,
			 struct nf_conntrack *ctmask,
			 union ct_address *ad, int *family,
			 unsigned int *options)
{
	int mask, maskopt;

	const int l3protonum = parse_addr(arg, ad, &mask);
	if (l3protonum == AF_UNSPEC) {
		exit_error(PARAMETER_PROBLEM,
			   "Invalid IP address `%s'", arg);
	}
	set_family(family, l3protonum);
	maskopt = opt2maskopt[opt];
	if (mask != -1 && !maskopt) {
		exit_error(PARAMETER_PROBLEM,
		           "CIDR notation unavailable"
		           " for `--%s'", get_long_opt(opt));
	} else if (mask == -2) {
		exit_error(PARAMETER_PROBLEM,
		           "Invalid netmask");
	}

	nfct_set_addr_opt(opt, ct, ad, l3protonum, options);

	/* bail if we don't have a netmask to set*/
	if (mask == -1 || !maskopt || ctmask == NULL)
		return;

	switch(l3protonum) {
	case AF_INET:
		if (mask == 32)
			return;
		nfct_build_netmask(&ad->v4, mask, 1);
		break;
	case AF_INET6:
		if (mask == 128)
			return;
		nfct_build_netmask((uint32_t *) &ad->v6, mask, 4);
		break;
	}

	nfct_set_addr_opt(maskopt, ctmask, ad, l3protonum, options);
}

static void
nfct_set_nat_details(const int opt, struct nf_conntrack *ct,
		     union ct_address *ad, const char *port_str,
		     const int family)
{
	const int type = opt2type[opt];

	nfct_set_addr_only(opt, ct, ad, family);
	if (port_str && type == CT_OPT_SRC_NAT) {
		nfct_set_attr_u16(ct, ATTR_SNAT_PORT,
				  ntohs((uint16_t)atoi(port_str)));
	} else if (port_str && type == CT_OPT_DST_NAT) {
		nfct_set_attr_u16(ct, ATTR_DNAT_PORT,
				  ntohs((uint16_t)atoi(port_str)));
	}

}

static int print_stats(const struct ct_cmd *cmd)
{
	if (cmd->command && exit_msg[cmd->cmd][0]) {
		fprintf(stderr, "%s v%s (conntrack-tools): ",PROGNAME,VERSION);
		fprintf(stderr, exit_msg[cmd->cmd], counter);
		if (counter == 0 &&
		    !(cmd->command & (CT_LIST | EXP_LIST)))
			return -1;
	}

	return 0;
}

static void parse_and_merge_labels(struct list_head *label_list, struct ct_tmpl *tmpl)
{
	struct ct_label *label, *next;
	struct nfct_bitmask *bitmask;
	unsigned int max;

	list_for_each_entry_safe(label, next, label_list, list) {
		max = parse_label_get_max(label->name);
		bitmask = nfct_bitmask_new(max);
		if (!bitmask)
			exit_error(OTHER_PROBLEM, "out of memory");

		parse_label(bitmask, label->name);

		/* join "-l foo -l bar" into single bitmask object */
		if (label->is_modify)
			merge_bitmasks(&tmpl->label_modify, bitmask);
		else
			merge_bitmasks(&tmpl->label, bitmask);

		list_del(&label->list);
		free(label->name);
		free(label);
	}
}

static void do_parse(struct ct_cmd *ct_cmd, int argc, char *argv[])
{
	unsigned int type = 0, event_mask = 0, l4flags = 0, status = 0;
	int protonum = 0, family = AF_UNSPEC;
	size_t socketbuffersize = 0;
	unsigned int command = 0;
	unsigned int options = 0;
	struct ct_tmpl *tmpl;
	int res = 0, partial;
	union ct_address ad;
	LIST_HEAD(labels);
	uint32_t value;
	int c, cmd;

	/* we release these objects in the exit_error() path. */
	if (!alloc_tmpl_objects(&ct_cmd->tmpl))
		exit_error(OTHER_PROBLEM, "out of memory");

	tmpl = &ct_cmd->tmpl;

	/* disable explicit missing arguments error output from getopt_long */
	opterr = 0;
	/* reset optind, for the case do_parse is called multiple times */
	optind = 0;

	while ((c = getopt_long(argc, argv, getopt_str, opts, NULL)) != -1) {
	switch(c) {
		/* commands */
		case 'L':
			type = check_type(argc, argv);
			/* Special case: dumping dying and unconfirmed list
			 * are handled like normal conntrack dumps.
			 */
			if (type == CT_TABLE_DYING ||
			    type == CT_TABLE_UNCONFIRMED)
				add_command(&command, cmd2type[c][0]);
			else
				add_command(&command, cmd2type[c][type]);
			break;
		case 'I':
		case 'D':
		case 'G':
		case 'F':
		case 'E':
		case 'V':
		case 'h':
		case 'C':
		case 'S':
		case 'U':
		case 'A':
			type = check_type(argc, argv);
			if (type == CT_TABLE_DYING ||
			    type == CT_TABLE_UNCONFIRMED) {
				exit_error(PARAMETER_PROBLEM,
					   "Can't do that command with "
					   "tables `dying' and `unconfirmed'");
			}
			if (cmd2type[c][type])
				add_command(&command, cmd2type[c][type]);
			else {
				exit_error(PARAMETER_PROBLEM,
					   "Can't do --%s on %s",
					   get_long_opt(c),
					   type == CT_TABLE_CONNTRACK ?
					           "conntrack" : "expectations");
			}
			break;
		/* options */
		case 's':
		case 'd':
		case 'r':
		case 'q':
			nfct_parse_addr_from_opt(c, optarg, tmpl->ct,
						 tmpl->mask, &ad, &family,
						 &options);
			break;
		case '[':
		case ']':
			nfct_parse_addr_from_opt(c, optarg, tmpl->exptuple,
						 tmpl->mask, &ad, &family,
						 &options);
			break;
		case '{':
		case '}':
			nfct_parse_addr_from_opt(c, optarg, tmpl->mask,
						 NULL, &ad, &family, &options);
			break;
		case 'p':
			options |= CT_OPT_PROTO;
			h = findproto(optarg, &protonum);
			if (!h)
				exit_error(PARAMETER_PROBLEM,
					   "`%s' unsupported protocol",
					   optarg);

			opts = merge_options(opts, h->opts, &h->option_offset);
			if (opts == NULL)
				exit_error(OTHER_PROBLEM, "out of memory");

			nfct_set_attr_u8(tmpl->ct, ATTR_L4PROTO, protonum);
			break;
		case 't':
			options |= CT_OPT_TIMEOUT;
			nfct_set_attr_u32(tmpl->ct, ATTR_TIMEOUT, atol(optarg));
			nfexp_set_attr_u32(tmpl->exp,
					   ATTR_EXP_TIMEOUT, atol(optarg));
			break;
		case 'u':
			options |= CT_OPT_STATUS;
			parse_parameter_mask(optarg, &status,
					    &tmpl->filter_status_kernel.mask,
					    PARSE_STATUS);
			nfct_set_attr_u32(tmpl->ct, ATTR_STATUS, status);
			if (tmpl->filter_status_kernel.mask == 0)
				tmpl->filter_status_kernel.mask = status;

			tmpl->filter_status_kernel.val = status;
			tmpl->filter_status_kernel_set = true;
			break;
		case 'e':
			options |= CT_OPT_EVENT_MASK;
			parse_parameter(optarg, &event_mask, PARSE_EVENT);
			break;
		case 'o':
			options |= CT_OPT_OUTPUT;
			parse_parameter(optarg, &output_mask, PARSE_OUTPUT);
			if ((output_mask & _O_SAVE) &&
			    (output_mask & (_O_EXT |_O_TMS |_O_ID | _O_KTMS | _O_CL | _O_XML)))
				exit_error(OTHER_PROBLEM,
					   "cannot combine save output with any other output type, use -o save only");
			break;
		case 'z':
			options |= CT_OPT_ZERO;
			break;
		case 'n':
		case 'g':
		case 'j':
			options |= opt2type[c];
			char *optional_arg = get_optional_arg(argc, argv);

			if (optional_arg) {
				char *port_str = NULL;
				char *nat_address = NULL;

				split_address_and_port(optional_arg,
						       &nat_address,
						       &port_str);
				nfct_parse_addr_from_opt(c, nat_address,
							 tmpl->ct, NULL,
							 &ad, &family,
							 &options);
				if (c == 'j') {
					/* Set details on both src and dst
					 * with any-nat
					 */
					nfct_set_nat_details('g', tmpl->ct, &ad,
							     port_str, family);
					nfct_set_nat_details('n', tmpl->ct, &ad,
							     port_str, family);
				} else {
					nfct_set_nat_details(c, tmpl->ct, &ad,
							     port_str, family);
				}
				free(port_str);
				free(nat_address);
			}
			break;
		case 'w':
		case '(':
		case ')':
			if (parse_value(optarg, &value, UINT16_MAX) < 0)
				exit_error(OTHER_PROBLEM, "unexpected value '%s' with -%c option", optarg, c);

			options |= opt2type[c];
			nfct_set_attr_u16(tmpl->ct, opt2attr[c], value);
			break;
		case 'i':
		case 'c':
			if (parse_value(optarg, &value, UINT32_MAX) < 0)
				exit_error(OTHER_PROBLEM, "unexpected value '%s' with -%c option", optarg, c);

			options |= opt2type[c];
			nfct_set_attr_u32(tmpl->ct, opt2attr[c], value);
			break;
		case 'm':
			options |= opt2type[c];
			if (parse_u32_mask(optarg, &tmpl->mark) < 0)
				exit_error(OTHER_PROBLEM, "unexpected value '%s' with -%c option", optarg, c);

			tmpl->filter_mark_kernel.val = tmpl->mark.value;
			tmpl->filter_mark_kernel.mask = tmpl->mark.mask;
			tmpl->filter_mark_kernel_set = true;
			break;
		case 'l':
		case '<':
		case '>':
			options |= opt2type[c];

			if ((options & (CT_OPT_DEL_LABEL|CT_OPT_ADD_LABEL)) ==
			    (CT_OPT_DEL_LABEL|CT_OPT_ADD_LABEL))
				exit_error(OTHER_PROBLEM, "cannot use --label-add and "
							"--label-del at the same time");

			if (c == '>') { /* DELETE */
				char *tmp = get_optional_arg(argc, argv);
				if (tmp == NULL) /* delete all labels */
					break;
				optarg = tmp;
			}

			struct ct_label *l = calloc(1, sizeof(*l));
			if (!l)
				exit_error(OTHER_PROBLEM, "out of memory");

			l->name = strdup(optarg);
			l->is_modify = c == '<' || c == '>';
			list_add_tail(&l->list, &labels);
			break;
		case 'a':
			fprintf(stderr, "WARNING: ignoring -%c, "
					"deprecated option.\n", c);
			break;
		case 'f':
			options |= CT_OPT_FAMILY;
			if (strncmp(optarg, "ipv4", strlen("ipv4")) == 0)
				set_family(&family, AF_INET);
			else if (strncmp(optarg, "ipv6", strlen("ipv6")) == 0)
				set_family(&family, AF_INET6);
			else
				exit_error(PARAMETER_PROBLEM,
					   "`%s' unsupported protocol",
					   optarg);
			break;
		case 'b':
			socketbuffersize = atol(optarg);
			options |= CT_OPT_BUFFERSIZE;
			break;
		case 'M':
			if (labelmap_path)
				exit_error(PARAMETER_PROBLEM, "option `--labelmap' can only be specified once");

			labelmap_path = strdup(optarg);
			options |= CT_OPT_LABELMAP;
			break;
		case ':':
			exit_error(PARAMETER_PROBLEM,
				   "option `%s' requires an "
				   "argument", argv[optind-1]);
		case '?':
			exit_error(PARAMETER_PROBLEM,
				   "unknown option `%s'", argv[optind-1]);
			break;
		default:
			if (h && h->parse_opts &&
			    !h->parse_opts(c - h->option_offset, tmpl->ct,
					   tmpl->exptuple, tmpl->mask,
					   &l4flags))
				exit_error(PARAMETER_PROBLEM, "parse error");
			break;
		}
	}

	/* default family only for the following commands */
	if (family == AF_UNSPEC) {
		switch (command) {
		case CT_LIST:
		case CT_UPDATE:
		case CT_DELETE:
		case CT_GET:
		case CT_FLUSH:
		case CT_EVENT:
			break;
		default:
			family = AF_INET;
			break;
		}
	}

	/* any of these options (might) use labels */
	if ((options & (CT_OPT_LABEL | CT_OPT_ADD_LABEL | CT_OPT_DEL_LABEL)) ||
	    ((options & CT_OPT_OUTPUT) && (output_mask & _O_CL))) {
		labelmap_init();
		parse_and_merge_labels(&labels, tmpl);
	}

	/* we cannot check this combination with generic_opt_check. */
	if (options & CT_OPT_ANY_NAT &&
	   ((options & CT_OPT_SRC_NAT) || (options & CT_OPT_DST_NAT))) {
		exit_error(PARAMETER_PROBLEM, "cannot specify `--src-nat' or "
					      "`--dst-nat' with `--any-nat'");
	}
	cmd = bit2cmd(command);
	res = generic_opt_check(options, NUMBER_OF_OPT,
				commands_v_options[cmd], optflags,
				addr_valid_flags, ADDR_VALID_FLAGS_MAX,
				&partial);
	if (!res) {
		switch(partial) {
		case -1:
		case 0:
			exit_error(PARAMETER_PROBLEM, "you have to specify "
						      "`--src' and `--dst'");
			break;
		case 1:
			exit_error(PARAMETER_PROBLEM, "you have to specify "
						      "`--reply-src' and "
						      "`--reply-dst'");
			break;
		}
	}
	if (!(command & CT_HELP) && h && h->final_check)
		h->final_check(l4flags, cmd, tmpl->ct);

	free_options();

	ct_cmd->command = command;
	ct_cmd->cmd = cmd;
	ct_cmd->options = options;
	ct_cmd->family = family;
	ct_cmd->type = type;
	ct_cmd->protonum = protonum;
	ct_cmd->event_mask = event_mask;
	ct_cmd->socketbuffersize = socketbuffersize;
}

static int do_command_ct(const char *progname, struct ct_cmd *cmd,
			 struct nfct_mnl_socket *sock)
{
	struct nfct_mnl_socket *modifier_sock = &_modifier_sock;
	struct nfct_mnl_socket *event_sock = &_event_sock;
	struct nfct_filter_dump *filter_dump;
	uint16_t nl_flags = 0;
	int res = 0;

	switch(cmd->command) {
	case CT_LIST:
		if (cmd->type == CT_TABLE_DYING) {
			res = nfct_mnl_dump(sock, NFNL_SUBSYS_CTNETLINK,
					    IPCTNL_MSG_CT_GET_DYING,
					    mnl_nfct_dump_cb, cmd, NULL);
			break;
		} else if (cmd->type == CT_TABLE_UNCONFIRMED) {
			res = nfct_mnl_dump(sock, NFNL_SUBSYS_CTNETLINK,
					    IPCTNL_MSG_CT_GET_UNCONFIRMED,
					    mnl_nfct_dump_cb, cmd, NULL);
			break;
		}

		if (cmd->options & CT_COMPARISON &&
		    cmd->options & CT_OPT_ZERO)
			exit_error(PARAMETER_PROBLEM, "Can't use -z with "
						      "filtering parameters");

		nfct_filter_init(cmd);

		filter_dump = nfct_filter_dump_create();
		if (filter_dump == NULL)
			exit_error(OTHER_PROBLEM, "OOM");

		if (cmd->tmpl.filter_mark_kernel_set) {
			nfct_filter_dump_set_attr(filter_dump,
						  NFCT_FILTER_DUMP_MARK,
						  &cmd->tmpl.filter_mark_kernel);
		}
		nfct_filter_dump_set_attr_u8(filter_dump,
					     NFCT_FILTER_DUMP_L3NUM,
					     cmd->family);
		if (cmd->tmpl.filter_status_kernel_set) {
			nfct_filter_dump_set_attr(filter_dump,
						  NFCT_FILTER_DUMP_STATUS,
						  &cmd->tmpl.filter_status_kernel);
		}
		if (cmd->options & CT_OPT_ZERO) {
			res = nfct_mnl_dump(sock, NFNL_SUBSYS_CTNETLINK,
					    IPCTNL_MSG_CT_GET_CTRZERO,
					    mnl_nfct_dump_cb, cmd, filter_dump);
		} else {
			res = nfct_mnl_dump(sock, NFNL_SUBSYS_CTNETLINK,
					    IPCTNL_MSG_CT_GET,
					    mnl_nfct_dump_cb, cmd, filter_dump);
		}

		nfct_filter_dump_destroy(filter_dump);

		if (dump_xml_header_done == 0) {
			printf("</conntrack>\n");
			fflush(stdout);
		}
		break;
	case EXP_LIST:
		cth = nfct_open(EXPECT, 0);
		if (!cth)
			exit_error(OTHER_PROBLEM, "Can't open handler");

		nfexp_callback_register(cth, NFCT_T_ALL, dump_exp_cb, NULL);
		res = nfexp_query(cth, NFCT_Q_DUMP, &cmd->family);
		nfct_close(cth);

		if (dump_xml_header_done == 0) {
			printf("</expect>\n");
			fflush(stdout);
		}
		break;

	case CT_CREATE:
	case CT_ADD:
		if ((cmd->options & CT_OPT_ORIG) && !(cmd->options & CT_OPT_REPL))
			nfct_setobjopt(cmd->tmpl.ct, NFCT_SOPT_SETUP_REPLY);
		else if (!(cmd->options & CT_OPT_ORIG) && (cmd->options & CT_OPT_REPL))
			nfct_setobjopt(cmd->tmpl.ct, NFCT_SOPT_SETUP_ORIGINAL);

		if (cmd->options & CT_OPT_MARK)
			nfct_set_attr_u32(cmd->tmpl.ct, ATTR_MARK, cmd->tmpl.mark.value);

		if (cmd->options & CT_OPT_ADD_LABEL)
			nfct_set_attr(cmd->tmpl.ct, ATTR_CONNLABELS,
					xnfct_bitmask_clone(cmd->tmpl.label_modify));

		if (cmd->command == CT_CREATE)
			nl_flags = NLM_F_EXCL;

		res = nfct_mnl_request(sock, NFNL_SUBSYS_CTNETLINK, cmd->family,
				       IPCTNL_MSG_CT_NEW,
				       NLM_F_CREATE | NLM_F_ACK | nl_flags,
				       NULL, cmd->tmpl.ct, NULL);
		if (res >= 0)
			counter++;
		break;

	case EXP_CREATE:
		nfexp_set_attr(cmd->tmpl.exp, ATTR_EXP_MASTER, cmd->tmpl.ct);
		nfexp_set_attr(cmd->tmpl.exp, ATTR_EXP_EXPECTED, cmd->tmpl.exptuple);
		nfexp_set_attr(cmd->tmpl.exp, ATTR_EXP_MASK, cmd->tmpl.mask);

		cth = nfct_open(EXPECT, 0);
		if (!cth)
			exit_error(OTHER_PROBLEM, "Can't open handler");

		res = nfexp_query(cth, NFCT_Q_CREATE, cmd->tmpl.exp);
		nfct_close(cth);
		break;

	case CT_UPDATE:
		if (nfct_mnl_socket_check_open(modifier_sock, 0) < 0)
			exit_error(OTHER_PROBLEM, "Can't open handler");

		nfct_filter_init(cmd);
		res = nfct_mnl_dump(sock, NFNL_SUBSYS_CTNETLINK,
				    IPCTNL_MSG_CT_GET, mnl_nfct_update_cb,
				    cmd, NULL);
		break;

	case CT_DELETE:
		if (nfct_mnl_socket_check_open(modifier_sock, 0) < 0)
			exit_error(OTHER_PROBLEM, "Can't open handler");

		nfct_filter_init(cmd);

		filter_dump = nfct_filter_dump_create();
		if (filter_dump == NULL)
			exit_error(OTHER_PROBLEM, "OOM");

		if (cmd->tmpl.filter_mark_kernel_set) {
			nfct_filter_dump_set_attr(filter_dump,
						  NFCT_FILTER_DUMP_MARK,
						  &cmd->tmpl.filter_mark_kernel);
		}
		nfct_filter_dump_set_attr_u8(filter_dump,
					     NFCT_FILTER_DUMP_L3NUM,
					     cmd->family);

		res = nfct_mnl_dump(sock, NFNL_SUBSYS_CTNETLINK,
				    IPCTNL_MSG_CT_GET, mnl_nfct_delete_cb,
				    cmd, filter_dump);

		nfct_filter_dump_destroy(filter_dump);
		break;

	case EXP_DELETE:
		nfexp_set_attr(cmd->tmpl.exp, ATTR_EXP_EXPECTED, cmd->tmpl.ct);

		cth = nfct_open(EXPECT, 0);
		if (!cth)
			exit_error(OTHER_PROBLEM, "Can't open handler");

		res = nfexp_query(cth, NFCT_Q_DESTROY, cmd->tmpl.exp);
		nfct_close(cth);
		break;

	case CT_GET:
		res = nfct_mnl_request(sock, NFNL_SUBSYS_CTNETLINK, cmd->family,
				       IPCTNL_MSG_CT_GET, 0,
				       mnl_nfct_dump_cb, cmd->tmpl.ct, cmd);
		break;

	case EXP_GET:
		nfexp_set_attr(cmd->tmpl.exp, ATTR_EXP_MASTER, cmd->tmpl.ct);

		cth = nfct_open(EXPECT, 0);
		if (!cth)
			exit_error(OTHER_PROBLEM, "Can't open handler");

		nfexp_callback_register(cth, NFCT_T_ALL, dump_exp_cb, NULL);
		res = nfexp_query(cth, NFCT_Q_GET, cmd->tmpl.exp);
		nfct_close(cth);
		break;

	case CT_FLUSH:
		res = nfct_mnl_request(sock, NFNL_SUBSYS_CTNETLINK, cmd->family,
				       IPCTNL_MSG_CT_DELETE, NLM_F_ACK, NULL, NULL, NULL);

		fprintf(stderr, "%s v%s (conntrack-tools): ",PROGNAME,VERSION);
		fprintf(stderr,"connection tracking table has been emptied.\n");
		break;

	case EXP_FLUSH:
		cth = nfct_open(EXPECT, 0);
		if (!cth)
			exit_error(OTHER_PROBLEM, "Can't open handler");
		res = nfexp_query(cth, NFCT_Q_FLUSH, &cmd->family);
		nfct_close(cth);
		fprintf(stderr, "%s v%s (conntrack-tools): ",PROGNAME,VERSION);
		fprintf(stderr,"expectation table has been emptied.\n");
		break;

	case CT_EVENT:
		if (cmd->options & CT_OPT_EVENT_MASK) {
			unsigned int nl_events = 0;

			if (cmd->event_mask & CT_EVENT_F_NEW)
				nl_events |= NF_NETLINK_CONNTRACK_NEW;
			if (cmd->event_mask & CT_EVENT_F_UPD)
				nl_events |= NF_NETLINK_CONNTRACK_UPDATE;
			if (cmd->event_mask & CT_EVENT_F_DEL)
				nl_events |= NF_NETLINK_CONNTRACK_DESTROY;

			res = nfct_mnl_socket_open(event_sock, nl_events);
		} else {
			res = nfct_mnl_socket_open(event_sock,
						   NF_NETLINK_CONNTRACK_NEW |
						   NF_NETLINK_CONNTRACK_UPDATE |
						   NF_NETLINK_CONNTRACK_DESTROY);
		}

		if (res < 0)
			exit_error(OTHER_PROBLEM, "Can't open netlink socket");

		if (cmd->options & CT_OPT_BUFFERSIZE) {
			size_t socketbuffersize = cmd->socketbuffersize;

			socklen_t socklen = sizeof(socketbuffersize);

			res = setsockopt(mnl_socket_get_fd(sock->mnl),
					 SOL_SOCKET, SO_RCVBUFFORCE,
					 &socketbuffersize,
					 sizeof(socketbuffersize));
			if (res < 0) {
				setsockopt(mnl_socket_get_fd(sock->mnl),
					   SOL_SOCKET, SO_RCVBUF,
					   &socketbuffersize,
					   sizeof(socketbuffersize));
			}
			getsockopt(mnl_socket_get_fd(sock->mnl), SOL_SOCKET,
				   SO_RCVBUF, &socketbuffersize, &socklen);
			fprintf(stderr, "NOTICE: Netlink socket buffer size "
					"has been set to %zu bytes.\n",
					socketbuffersize);
		}

		nfct_filter_init(cmd);

		signal(SIGINT, event_sighandler);
		signal(SIGTERM, event_sighandler);

		while (1) {
			char buf[MNL_SOCKET_BUFFER_SIZE];

			res = mnl_socket_recvfrom(event_sock->mnl, buf, sizeof(buf));
			if (res < 0) {
				if (errno == ENOBUFS) {
					fprintf(stderr,
						"WARNING: We have hit ENOBUFS! We "
						"are losing events.\nThis message "
						"means that the current netlink "
						"socket buffer size is too small.\n"
						"Please, check --buffer-size in "
						"conntrack(8) manpage.\n");
					continue;
				}
				exit_error(OTHER_PROBLEM,
					   "failed to received netlink event: %s",
					   strerror(errno));
				break;
			}
			mnl_cb_run(buf, res, 0, 0, event_cb, cmd);
		}
		mnl_socket_close(event_sock->mnl);
		break;

	case EXP_EVENT:
		if (cmd->options & CT_OPT_EVENT_MASK) {
			unsigned int nl_events = 0;

			if (cmd->event_mask & CT_EVENT_F_NEW)
				nl_events |= NF_NETLINK_CONNTRACK_EXP_NEW;
			if (cmd->event_mask & CT_EVENT_F_UPD)
				nl_events |= NF_NETLINK_CONNTRACK_EXP_UPDATE;
			if (cmd->event_mask & CT_EVENT_F_DEL)
				nl_events |= NF_NETLINK_CONNTRACK_EXP_DESTROY;

			cth = nfct_open(CONNTRACK, nl_events);
		} else {
			cth = nfct_open(EXPECT,
					NF_NETLINK_CONNTRACK_EXP_NEW |
					NF_NETLINK_CONNTRACK_EXP_UPDATE |
					NF_NETLINK_CONNTRACK_EXP_DESTROY);
		}

		if (!cth)
			exit_error(OTHER_PROBLEM, "Can't open handler");
		signal(SIGINT, exp_event_sighandler);
		signal(SIGTERM, exp_event_sighandler);
		nfexp_callback_register(cth, NFCT_T_ALL, event_exp_cb, NULL);
		res = nfexp_catch(cth);
		nfct_close(cth);
		break;
	case CT_COUNT:
		/* If we fail with netlink, fall back to /proc to ensure
		 * backward compatibility.
		 */
		res = nfct_mnl_request(sock, NFNL_SUBSYS_CTNETLINK, AF_UNSPEC,
				       IPCTNL_MSG_CT_GET_STATS, 0,
				       nfct_global_stats_cb, NULL, NULL);

		/* don't look at /proc, we got the information via ctnetlink */
		if (res >= 0)
			break;

		{
#define NF_CONNTRACK_COUNT_PROC "/proc/sys/net/netfilter/nf_conntrack_count"
		FILE *fd;
		int count;
		fd = fopen(NF_CONNTRACK_COUNT_PROC, "r");
		if (fd == NULL) {
			exit_error(OTHER_PROBLEM, "Can't open %s",
				   NF_CONNTRACK_COUNT_PROC);
		}
		if (fscanf(fd, "%d", &count) != 1) {
			exit_error(OTHER_PROBLEM, "Can't read %s",
				   NF_CONNTRACK_COUNT_PROC);
		}
		fclose(fd);
		printf("%d\n", count);
		break;
	}
	case EXP_COUNT:
		cth = nfct_open(EXPECT, 0);
		if (!cth)
			exit_error(OTHER_PROBLEM, "Can't open handler");

		nfexp_callback_register(cth, NFCT_T_ALL, count_exp_cb, NULL);
		res = nfexp_query(cth, NFCT_Q_DUMP, &cmd->family);
		nfct_close(cth);
		printf("%d\n", counter);
		break;
	case CT_STATS:
		/* If we fail with netlink, fall back to /proc to ensure
		 * backward compatibility.
		 */
		res = nfct_mnl_dump(sock, NFNL_SUBSYS_CTNETLINK,
				    IPCTNL_MSG_CT_GET_STATS_CPU,
				    nfct_stats_cb, NULL, NULL);

		/* don't look at /proc, we got the information via ctnetlink */
		if (res >= 0)
			break;

		goto try_proc;

	case EXP_STATS:
		/* If we fail with netlink, fall back to /proc to ensure
		 * backward compatibility.
		 */
		res = nfct_mnl_dump(sock, NFNL_SUBSYS_CTNETLINK_EXP,
				    IPCTNL_MSG_EXP_GET_STATS_CPU,
				    nfexp_stats_cb, NULL, NULL);

		/* don't look at /proc, we got the information via ctnetlink */
		if (res >= 0)
			break;
try_proc:
		if (display_proc_conntrack_stats() < 0)
			exit_error(OTHER_PROBLEM, "Can't open /proc interface");
		break;
	case CT_VERSION:
		printf("%s v%s (conntrack-tools)\n", PROGNAME, VERSION);
		break;
	case CT_HELP:
		usage(progname);
		if (cmd->options & CT_OPT_PROTO)
			extension_help(h, cmd->protonum);
		break;
	default:
		usage(progname);
		break;
	}

	if (res < 0)
		exit_error(OTHER_PROBLEM, "Operation failed: %s",
			   err2str(errno, cmd->command));

	free_tmpl_objects(&cmd->tmpl);
	if (labelmap)
		nfct_labelmap_destroy(labelmap);
	free(labelmap_path);

	return EXIT_SUCCESS;
}

/* Taken from iptables/xshared.c:
 * - add_argv()
 * - add_param_to_argv()
 * - free_argv()
 */
#define MAX_ARGC	255
struct argv_store {
	int argc;
	char *argv[MAX_ARGC];
	int argvattr[MAX_ARGC];
};

/* function adding one argument to store, updating argc
 * returns if argument added, does not return otherwise */
static void add_argv(struct argv_store *store, const char *what, int quoted)
{
	if (store->argc + 1 >= MAX_ARGC)
		exit_error(PARAMETER_PROBLEM, "too many arguments");
	if (!what)
		exit_error(PARAMETER_PROBLEM, "invalid NULL argument");

	store->argv[store->argc] = strdup(what);
	store->argvattr[store->argc] = quoted;
	store->argv[++store->argc] = NULL;
}

static void free_argv(struct argv_store *store)
{
	while (store->argc) {
		store->argc--;
		free(store->argv[store->argc]);
		store->argvattr[store->argc] = 0;
	}
}

struct ct_param_buf {
	char	buffer[1024];
	int 	len;
};

static void add_param(struct ct_param_buf *param, const char *curchar)
{
	param->buffer[param->len++] = *curchar;
	if (param->len >= (int)sizeof(param->buffer))
		exit_error(PARAMETER_PROBLEM, "Parameter too long!");
}

static void add_param_to_argv(struct argv_store *store, char *parsestart)
{
	int quote_open = 0, escaped = 0, quoted = 0;
	struct ct_param_buf param = {};
	char *curchar;

	/* After fighting with strtok enough, here's now
	 * a 'real' parser. According to Rusty I'm now no
	 * longer a real hacker, but I can live with that */

	for (curchar = parsestart; *curchar; curchar++) {
		if (quote_open) {
			if (escaped) {
				add_param(&param, curchar);
				escaped = 0;
				continue;
			} else if (*curchar == '\\') {
				escaped = 1;
				continue;
			} else if (*curchar == '"') {
				quote_open = 0;
			} else {
				add_param(&param, curchar);
				continue;
			}
		} else {
			if (*curchar == '"') {
				quote_open = 1;
				quoted = 1;
				continue;
			}
		}

		switch (*curchar) {
		case '"':
			break;
		case ' ':
		case '\t':
		case '\n':
			if (!param.len) {
				/* two spaces? */
				continue;
			}
			break;
		default:
			/* regular character, copy to buffer */
			add_param(&param, curchar);
			continue;
		}

		param.buffer[param.len] = '\0';
		add_argv(store, param.buffer, quoted);
		param.len = 0;
		quoted = 0;
	}
	if (param.len) {
		param.buffer[param.len] = '\0';
		add_argv(store, param.buffer, 0);
	}
}

static void ct_file_parse_line(struct list_head *cmd_list,
			       const char *progname, char *buffer)
{
	struct argv_store store = {};
	struct ct_cmd *ct_cmd;

	/* skip prepended tabs and spaces */
	for (; *buffer == ' ' || *buffer == '\t'; buffer++);

	if (buffer[0] == '\n' ||
	    buffer[0] == '#')
		return;

	add_argv(&store, progname, false);
	add_param_to_argv(&store, buffer);

	ct_cmd = calloc(1, sizeof(*ct_cmd));
	if (!ct_cmd)
		exit_error(OTHER_PROBLEM, "OOM");

	do_parse(ct_cmd, store.argc, store.argv);
	free_argv(&store);

	list_add_tail(&ct_cmd->list, cmd_list);
}

static void ct_parse_file(struct list_head *cmd_list, const char *progname,
			  const char *file_name)
{
	char buffer[10240] = {};
	FILE *file;

	if (!strcmp(file_name, "-"))
		file_name = "/dev/stdin";

	file = fopen(file_name, "r");
	if (!file)
		exit_error(PARAMETER_PROBLEM,
			   "Failed to open file %s for reading", file_name);

	while (fgets(buffer, sizeof(buffer), file))
		ct_file_parse_line(cmd_list, progname, buffer);

	fclose(file);
}

static struct {
	uint32_t	command;
	const char	*text;
} ct_unsupp_cmd_parse_file[] = {
	{ CT_LIST,	"-L"	},
	{ CT_GET,	"-G"	},
	{ CT_EVENT,	"-E"	},
	{ CT_VERSION,	"-V"	},
	{ CT_HELP,	"-h"	},
	{ EXP_LIST,	"-L expect" },
	{ EXP_CREATE,	"-C expect" },
	{ EXP_DELETE,	"-D expect" },
	{ EXP_GET,	"-G expect" },
	{ EXP_FLUSH,	"-F expect" },
	{ EXP_EVENT,	"-E expect" },
	{ CT_COUNT,	"-C"	},
	{ EXP_COUNT,	"-C expect" },
	{ CT_STATS,	"-S"	},
	{ EXP_STATS,	"-S expect" },
	{ 0, NULL },
};

static const char *ct_unsupp_cmd_file(const struct ct_cmd *cmd)
{
	int i;

	for (i = 0; ct_unsupp_cmd_parse_file[i].text; i++) {
		if (cmd->command == ct_unsupp_cmd_parse_file[i].command)
			return ct_unsupp_cmd_parse_file[i].text;
	}

	return "unknown";
}

int main(int argc, char *argv[])
{
	struct nfct_mnl_socket *modifier_sock = &_modifier_sock;
	struct nfct_mnl_socket *sock = &_sock;
	struct ct_cmd *cmd, *next;
	LIST_HEAD(cmd_list);
	int res = 0;

	register_tcp();
	register_udp();
	register_udplite();
	register_sctp();
	register_dccp();
	register_icmp();
	register_icmpv6();
	register_gre();
	register_unknown();

	if (nfct_mnl_socket_open(sock, 0) < 0)
		exit_error(OTHER_PROBLEM, "Can't open handler");

	if (argc > 2 &&
	    (!strcmp(argv[1], "-R") || !strcmp(argv[1], "--load-file"))) {
		ct_parse_file(&cmd_list, argv[0], argv[2]);

		list_for_each_entry(cmd, &cmd_list, list) {
			if (!(cmd->command &
			      (CT_CREATE | CT_ADD | CT_UPDATE | CT_DELETE | CT_FLUSH)))
				exit_error(PARAMETER_PROBLEM,
					   "Cannot use command `%s' with --load-file",
					   ct_unsupp_cmd_file(cmd));
		}
		list_for_each_entry_safe(cmd, next, &cmd_list, list) {
			res |= do_command_ct(argv[0], cmd, sock);
			list_del(&cmd->list);
			free(cmd);
		}
	} else {
		cmd = calloc(1, sizeof(*cmd));
		if (!cmd)
			exit_error(OTHER_PROBLEM, "OOM");

		do_parse(cmd, argc, argv);
		res = do_command_ct(argv[0], cmd, sock);
		res |= print_stats(cmd);
		free(cmd);
	}
	nfct_mnl_socket_close(sock);
	nfct_mnl_socket_check_close(modifier_sock);

	return res < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
