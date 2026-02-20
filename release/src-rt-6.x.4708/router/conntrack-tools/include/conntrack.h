#ifndef _CONNTRACK_H
#define _CONNTRACK_H

#include "linux_list.h"
#include <stdint.h>

#define PROGNAME "conntrack"

#include <netinet/in.h>

#include <linux/netfilter/nf_conntrack_common.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

enum ct_command {
	CT_NONE		= 0,

	CT_LIST_BIT 	= 0,
	CT_LIST 	= (1 << CT_LIST_BIT),

	CT_CREATE_BIT	= 1,
	CT_CREATE	= (1 << CT_CREATE_BIT),

	CT_UPDATE_BIT	= 2,
	CT_UPDATE	= (1 << CT_UPDATE_BIT),

	CT_DELETE_BIT	= 3,
	CT_DELETE	= (1 << CT_DELETE_BIT),

	CT_GET_BIT	= 4,
	CT_GET		= (1 << CT_GET_BIT),

	CT_FLUSH_BIT	= 5,
	CT_FLUSH	= (1 << CT_FLUSH_BIT),

	CT_EVENT_BIT	= 6,
	CT_EVENT	= (1 << CT_EVENT_BIT),

	CT_VERSION_BIT	= 7,
	CT_VERSION	= (1 << CT_VERSION_BIT),

	CT_HELP_BIT	= 8,
	CT_HELP		= (1 << CT_HELP_BIT),

	EXP_LIST_BIT 	= 9,
	EXP_LIST 	= (1 << EXP_LIST_BIT),

	EXP_CREATE_BIT	= 10,
	EXP_CREATE	= (1 << EXP_CREATE_BIT),

	EXP_DELETE_BIT	= 11,
	EXP_DELETE	= (1 << EXP_DELETE_BIT),

	EXP_GET_BIT	= 12,
	EXP_GET		= (1 << EXP_GET_BIT),

	EXP_FLUSH_BIT	= 13,
	EXP_FLUSH	= (1 << EXP_FLUSH_BIT),

	EXP_EVENT_BIT	= 14,
	EXP_EVENT	= (1 << EXP_EVENT_BIT),

	CT_COUNT_BIT	= 15,
	CT_COUNT	= (1 << CT_COUNT_BIT),

	EXP_COUNT_BIT	= 16,
	EXP_COUNT	= (1 << EXP_COUNT_BIT),

	CT_STATS_BIT	= 17,
	CT_STATS	= (1 << CT_STATS_BIT),

	EXP_STATS_BIT	= 18,
	EXP_STATS	= (1 << EXP_STATS_BIT),

	CT_ADD_BIT	= 19,
	CT_ADD		= (1 << CT_ADD_BIT),

	_CT_BIT_MAX	= 20,
};

#define NUMBER_OF_CMD   _CT_BIT_MAX
#define NUMBER_OF_OPT   30

struct nf_conntrack;

struct ctproto_handler {
	struct list_head 	head;

	const char		*name;
	uint16_t 		protonum;
	const char		*version;

	uint32_t		protoinfo_attr;

	int (*parse_opts)(char c,
			  struct nf_conntrack *ct,
			  struct nf_conntrack *exptuple,
			  struct nf_conntrack *mask,
			  unsigned int *flags);

	void (*final_check)(unsigned int flags,
			    unsigned int command,
			    struct nf_conntrack *ct);

	const struct ct_print_opts *print_opts;

	void (*help)(void);

	struct option 		*opts;

	unsigned int		option_offset;
};

enum exittype {
	OTHER_PROBLEM = 1,
	PARAMETER_PROBLEM,
	VERSION_PROBLEM
};

int generic_opt_check(int options, int nops,
		      char *optset, const char *optflg[],
		      unsigned int *coupled_flags, int coupled_flags_size,
		      int *partial);
void exit_error(enum exittype status, const char *msg, ...);

extern void register_proto(struct ctproto_handler *h);

enum ct_attr_type {
	CT_ATTR_TYPE_NONE = 0,
	CT_ATTR_TYPE_U8,
	CT_ATTR_TYPE_BE16,
	CT_ATTR_TYPE_U16,
	CT_ATTR_TYPE_BE32,
	CT_ATTR_TYPE_U32,
	CT_ATTR_TYPE_U64,
	CT_ATTR_TYPE_U32_BITMAP,
	CT_ATTR_TYPE_IPV4,
	CT_ATTR_TYPE_IPV6,
};

struct ct_print_opts {
	const char		*name;
	enum nf_conntrack_attr	type;
	enum ct_attr_type	datatype;
	short			val_mapping_count;
	const char		**val_mapping;
};

extern int ct_snprintf_opts(char *buf, unsigned int len,
			    const struct nf_conntrack *ct,
			    const struct ct_print_opts *attrs);

extern void register_tcp(void);
extern void register_udp(void);
extern void register_udplite(void);
extern void register_sctp(void);
extern void register_dccp(void);
extern void register_icmp(void);
extern void register_icmpv6(void);
extern void register_gre(void);
extern void register_unknown(void);

#endif
