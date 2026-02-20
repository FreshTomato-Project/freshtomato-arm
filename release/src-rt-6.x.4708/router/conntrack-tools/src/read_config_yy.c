/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "read_config_yy.y"

/*
 * (C) 2006-2009 by Pablo Neira Ayuso <pablo@netfilter.org>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Description: configuration file abstract grammar
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <stdarg.h>
#include <limits.h>
#include "conntrackd.h"
#include "bitops.h"
#include "cidr.h"
#include "helper.h"
#include "stack.h"
#include "log.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sched.h>
#include <dlfcn.h>

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack_tcp.h>

extern char *yytext;
extern int   yylineno;

int yylex (void);
int yyerror (const char *msg);
void yyrestart (FILE *input_file);

struct ct_conf conf;

static void __kernel_filter_start(void);
static void __kernel_filter_add_state(int value);

static struct channel_conf *conf_get_channel(void)
{
	if (conf.channel_num >= MULTICHANNEL_MAX) {
		dlog(LOG_ERR, "too many dedicated links in the configuration "
		     "file (Maximum: %d)", MULTICHANNEL_MAX);
		exit(EXIT_FAILURE);
	}

	return &conf.channel[conf.channel_num];
}

struct stack symbol_stack;

enum {
	SYMBOL_HELPER_QUEUE_NUM,
	SYMBOL_HELPER_QUEUE_LEN,
	SYMBOL_HELPER_POLICY_EXPECT_ROOT,
	SYMBOL_HELPER_EXPECT_POLICY_LEAF,
};


#line 150 "read_config_yy.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
#ifndef YY_YY_READ_CONFIG_YY_H_INCLUDED
# define YY_YY_READ_CONFIG_YY_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    T_IPV4_ADDR = 258,             /* T_IPV4_ADDR  */
    T_IPV4_IFACE = 259,            /* T_IPV4_IFACE  */
    T_PORT = 260,                  /* T_PORT  */
    T_HASHSIZE = 261,              /* T_HASHSIZE  */
    T_HASHLIMIT = 262,             /* T_HASHLIMIT  */
    T_MULTICAST = 263,             /* T_MULTICAST  */
    T_PATH = 264,                  /* T_PATH  */
    T_UNIX = 265,                  /* T_UNIX  */
    T_REFRESH = 266,               /* T_REFRESH  */
    T_IPV6_ADDR = 267,             /* T_IPV6_ADDR  */
    T_IPV6_IFACE = 268,            /* T_IPV6_IFACE  */
    T_BACKLOG = 269,               /* T_BACKLOG  */
    T_GROUP = 270,                 /* T_GROUP  */
    T_IGNORE = 271,                /* T_IGNORE  */
    T_SETUP = 272,                 /* T_SETUP  */
    T_LOG = 273,                   /* T_LOG  */
    T_UDP = 274,                   /* T_UDP  */
    T_ICMP = 275,                  /* T_ICMP  */
    T_IGMP = 276,                  /* T_IGMP  */
    T_VRRP = 277,                  /* T_VRRP  */
    T_TCP = 278,                   /* T_TCP  */
    T_LOCK = 279,                  /* T_LOCK  */
    T_BUFFER_SIZE_MAX_GROWN = 280, /* T_BUFFER_SIZE_MAX_GROWN  */
    T_EXPIRE = 281,                /* T_EXPIRE  */
    T_TIMEOUT = 282,               /* T_TIMEOUT  */
    T_GENERAL = 283,               /* T_GENERAL  */
    T_SYNC = 284,                  /* T_SYNC  */
    T_STATS = 285,                 /* T_STATS  */
    T_BUFFER_SIZE = 286,           /* T_BUFFER_SIZE  */
    T_SYNC_MODE = 287,             /* T_SYNC_MODE  */
    T_ALARM = 288,                 /* T_ALARM  */
    T_FTFW = 289,                  /* T_FTFW  */
    T_CHECKSUM = 290,              /* T_CHECKSUM  */
    T_WINDOWSIZE = 291,            /* T_WINDOWSIZE  */
    T_ON = 292,                    /* T_ON  */
    T_OFF = 293,                   /* T_OFF  */
    T_FOR = 294,                   /* T_FOR  */
    T_IFACE = 295,                 /* T_IFACE  */
    T_PURGE = 296,                 /* T_PURGE  */
    T_RESEND_QUEUE_SIZE = 297,     /* T_RESEND_QUEUE_SIZE  */
    T_ESTABLISHED = 298,           /* T_ESTABLISHED  */
    T_SYN_SENT = 299,              /* T_SYN_SENT  */
    T_SYN_RECV = 300,              /* T_SYN_RECV  */
    T_FIN_WAIT = 301,              /* T_FIN_WAIT  */
    T_CLOSE_WAIT = 302,            /* T_CLOSE_WAIT  */
    T_LAST_ACK = 303,              /* T_LAST_ACK  */
    T_TIME_WAIT = 304,             /* T_TIME_WAIT  */
    T_CLOSE = 305,                 /* T_CLOSE  */
    T_LISTEN = 306,                /* T_LISTEN  */
    T_SYSLOG = 307,                /* T_SYSLOG  */
    T_RCVBUFF = 308,               /* T_RCVBUFF  */
    T_SNDBUFF = 309,               /* T_SNDBUFF  */
    T_NOTRACK = 310,               /* T_NOTRACK  */
    T_POLL_SECS = 311,             /* T_POLL_SECS  */
    T_FILTER = 312,                /* T_FILTER  */
    T_ADDRESS = 313,               /* T_ADDRESS  */
    T_PROTOCOL = 314,              /* T_PROTOCOL  */
    T_STATE = 315,                 /* T_STATE  */
    T_ACCEPT = 316,                /* T_ACCEPT  */
    T_FROM = 317,                  /* T_FROM  */
    T_USERSPACE = 318,             /* T_USERSPACE  */
    T_KERNELSPACE = 319,           /* T_KERNELSPACE  */
    T_EVENT_ITER_LIMIT = 320,      /* T_EVENT_ITER_LIMIT  */
    T_DEFAULT = 321,               /* T_DEFAULT  */
    T_NETLINK_OVERRUN_RESYNC = 322, /* T_NETLINK_OVERRUN_RESYNC  */
    T_NICE = 323,                  /* T_NICE  */
    T_IPV4_DEST_ADDR = 324,        /* T_IPV4_DEST_ADDR  */
    T_IPV6_DEST_ADDR = 325,        /* T_IPV6_DEST_ADDR  */
    T_SCHEDULER = 326,             /* T_SCHEDULER  */
    T_TYPE = 327,                  /* T_TYPE  */
    T_PRIO = 328,                  /* T_PRIO  */
    T_NETLINK_EVENTS_RELIABLE = 329, /* T_NETLINK_EVENTS_RELIABLE  */
    T_DISABLE_INTERNAL_CACHE = 330, /* T_DISABLE_INTERNAL_CACHE  */
    T_DISABLE_EXTERNAL_CACHE = 331, /* T_DISABLE_EXTERNAL_CACHE  */
    T_ERROR_QUEUE_LENGTH = 332,    /* T_ERROR_QUEUE_LENGTH  */
    T_OPTIONS = 333,               /* T_OPTIONS  */
    T_TCP_WINDOW_TRACKING = 334,   /* T_TCP_WINDOW_TRACKING  */
    T_EXPECT_SYNC = 335,           /* T_EXPECT_SYNC  */
    T_HELPER = 336,                /* T_HELPER  */
    T_HELPER_QUEUE_NUM = 337,      /* T_HELPER_QUEUE_NUM  */
    T_HELPER_QUEUE_LEN = 338,      /* T_HELPER_QUEUE_LEN  */
    T_HELPER_POLICY = 339,         /* T_HELPER_POLICY  */
    T_HELPER_EXPECT_TIMEOUT = 340, /* T_HELPER_EXPECT_TIMEOUT  */
    T_HELPER_EXPECT_MAX = 341,     /* T_HELPER_EXPECT_MAX  */
    T_SYSTEMD = 342,               /* T_SYSTEMD  */
    T_STARTUP_RESYNC = 343,        /* T_STARTUP_RESYNC  */
    T_IP = 344,                    /* T_IP  */
    T_PATH_VAL = 345,              /* T_PATH_VAL  */
    T_NUMBER = 346,                /* T_NUMBER  */
    T_SIGNED_NUMBER = 347,         /* T_SIGNED_NUMBER  */
    T_STRING = 348                 /* T_STRING  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define T_IPV4_ADDR 258
#define T_IPV4_IFACE 259
#define T_PORT 260
#define T_HASHSIZE 261
#define T_HASHLIMIT 262
#define T_MULTICAST 263
#define T_PATH 264
#define T_UNIX 265
#define T_REFRESH 266
#define T_IPV6_ADDR 267
#define T_IPV6_IFACE 268
#define T_BACKLOG 269
#define T_GROUP 270
#define T_IGNORE 271
#define T_SETUP 272
#define T_LOG 273
#define T_UDP 274
#define T_ICMP 275
#define T_IGMP 276
#define T_VRRP 277
#define T_TCP 278
#define T_LOCK 279
#define T_BUFFER_SIZE_MAX_GROWN 280
#define T_EXPIRE 281
#define T_TIMEOUT 282
#define T_GENERAL 283
#define T_SYNC 284
#define T_STATS 285
#define T_BUFFER_SIZE 286
#define T_SYNC_MODE 287
#define T_ALARM 288
#define T_FTFW 289
#define T_CHECKSUM 290
#define T_WINDOWSIZE 291
#define T_ON 292
#define T_OFF 293
#define T_FOR 294
#define T_IFACE 295
#define T_PURGE 296
#define T_RESEND_QUEUE_SIZE 297
#define T_ESTABLISHED 298
#define T_SYN_SENT 299
#define T_SYN_RECV 300
#define T_FIN_WAIT 301
#define T_CLOSE_WAIT 302
#define T_LAST_ACK 303
#define T_TIME_WAIT 304
#define T_CLOSE 305
#define T_LISTEN 306
#define T_SYSLOG 307
#define T_RCVBUFF 308
#define T_SNDBUFF 309
#define T_NOTRACK 310
#define T_POLL_SECS 311
#define T_FILTER 312
#define T_ADDRESS 313
#define T_PROTOCOL 314
#define T_STATE 315
#define T_ACCEPT 316
#define T_FROM 317
#define T_USERSPACE 318
#define T_KERNELSPACE 319
#define T_EVENT_ITER_LIMIT 320
#define T_DEFAULT 321
#define T_NETLINK_OVERRUN_RESYNC 322
#define T_NICE 323
#define T_IPV4_DEST_ADDR 324
#define T_IPV6_DEST_ADDR 325
#define T_SCHEDULER 326
#define T_TYPE 327
#define T_PRIO 328
#define T_NETLINK_EVENTS_RELIABLE 329
#define T_DISABLE_INTERNAL_CACHE 330
#define T_DISABLE_EXTERNAL_CACHE 331
#define T_ERROR_QUEUE_LENGTH 332
#define T_OPTIONS 333
#define T_TCP_WINDOW_TRACKING 334
#define T_EXPECT_SYNC 335
#define T_HELPER 336
#define T_HELPER_QUEUE_NUM 337
#define T_HELPER_QUEUE_LEN 338
#define T_HELPER_POLICY 339
#define T_HELPER_EXPECT_TIMEOUT 340
#define T_HELPER_EXPECT_MAX 341
#define T_SYSTEMD 342
#define T_STARTUP_RESYNC 343
#define T_IP 344
#define T_PATH_VAL 345
#define T_NUMBER 346
#define T_SIGNED_NUMBER 347
#define T_STRING 348

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 80 "read_config_yy.y"

	int		val;
	char		*string;

#line 394 "read_config_yy.c"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_READ_CONFIG_YY_H_INCLUDED  */
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_T_IPV4_ADDR = 3,                /* T_IPV4_ADDR  */
  YYSYMBOL_T_IPV4_IFACE = 4,               /* T_IPV4_IFACE  */
  YYSYMBOL_T_PORT = 5,                     /* T_PORT  */
  YYSYMBOL_T_HASHSIZE = 6,                 /* T_HASHSIZE  */
  YYSYMBOL_T_HASHLIMIT = 7,                /* T_HASHLIMIT  */
  YYSYMBOL_T_MULTICAST = 8,                /* T_MULTICAST  */
  YYSYMBOL_T_PATH = 9,                     /* T_PATH  */
  YYSYMBOL_T_UNIX = 10,                    /* T_UNIX  */
  YYSYMBOL_T_REFRESH = 11,                 /* T_REFRESH  */
  YYSYMBOL_T_IPV6_ADDR = 12,               /* T_IPV6_ADDR  */
  YYSYMBOL_T_IPV6_IFACE = 13,              /* T_IPV6_IFACE  */
  YYSYMBOL_T_BACKLOG = 14,                 /* T_BACKLOG  */
  YYSYMBOL_T_GROUP = 15,                   /* T_GROUP  */
  YYSYMBOL_T_IGNORE = 16,                  /* T_IGNORE  */
  YYSYMBOL_T_SETUP = 17,                   /* T_SETUP  */
  YYSYMBOL_T_LOG = 18,                     /* T_LOG  */
  YYSYMBOL_T_UDP = 19,                     /* T_UDP  */
  YYSYMBOL_T_ICMP = 20,                    /* T_ICMP  */
  YYSYMBOL_T_IGMP = 21,                    /* T_IGMP  */
  YYSYMBOL_T_VRRP = 22,                    /* T_VRRP  */
  YYSYMBOL_T_TCP = 23,                     /* T_TCP  */
  YYSYMBOL_T_LOCK = 24,                    /* T_LOCK  */
  YYSYMBOL_T_BUFFER_SIZE_MAX_GROWN = 25,   /* T_BUFFER_SIZE_MAX_GROWN  */
  YYSYMBOL_T_EXPIRE = 26,                  /* T_EXPIRE  */
  YYSYMBOL_T_TIMEOUT = 27,                 /* T_TIMEOUT  */
  YYSYMBOL_T_GENERAL = 28,                 /* T_GENERAL  */
  YYSYMBOL_T_SYNC = 29,                    /* T_SYNC  */
  YYSYMBOL_T_STATS = 30,                   /* T_STATS  */
  YYSYMBOL_T_BUFFER_SIZE = 31,             /* T_BUFFER_SIZE  */
  YYSYMBOL_T_SYNC_MODE = 32,               /* T_SYNC_MODE  */
  YYSYMBOL_T_ALARM = 33,                   /* T_ALARM  */
  YYSYMBOL_T_FTFW = 34,                    /* T_FTFW  */
  YYSYMBOL_T_CHECKSUM = 35,                /* T_CHECKSUM  */
  YYSYMBOL_T_WINDOWSIZE = 36,              /* T_WINDOWSIZE  */
  YYSYMBOL_T_ON = 37,                      /* T_ON  */
  YYSYMBOL_T_OFF = 38,                     /* T_OFF  */
  YYSYMBOL_T_FOR = 39,                     /* T_FOR  */
  YYSYMBOL_T_IFACE = 40,                   /* T_IFACE  */
  YYSYMBOL_T_PURGE = 41,                   /* T_PURGE  */
  YYSYMBOL_T_RESEND_QUEUE_SIZE = 42,       /* T_RESEND_QUEUE_SIZE  */
  YYSYMBOL_T_ESTABLISHED = 43,             /* T_ESTABLISHED  */
  YYSYMBOL_T_SYN_SENT = 44,                /* T_SYN_SENT  */
  YYSYMBOL_T_SYN_RECV = 45,                /* T_SYN_RECV  */
  YYSYMBOL_T_FIN_WAIT = 46,                /* T_FIN_WAIT  */
  YYSYMBOL_T_CLOSE_WAIT = 47,              /* T_CLOSE_WAIT  */
  YYSYMBOL_T_LAST_ACK = 48,                /* T_LAST_ACK  */
  YYSYMBOL_T_TIME_WAIT = 49,               /* T_TIME_WAIT  */
  YYSYMBOL_T_CLOSE = 50,                   /* T_CLOSE  */
  YYSYMBOL_T_LISTEN = 51,                  /* T_LISTEN  */
  YYSYMBOL_T_SYSLOG = 52,                  /* T_SYSLOG  */
  YYSYMBOL_T_RCVBUFF = 53,                 /* T_RCVBUFF  */
  YYSYMBOL_T_SNDBUFF = 54,                 /* T_SNDBUFF  */
  YYSYMBOL_T_NOTRACK = 55,                 /* T_NOTRACK  */
  YYSYMBOL_T_POLL_SECS = 56,               /* T_POLL_SECS  */
  YYSYMBOL_T_FILTER = 57,                  /* T_FILTER  */
  YYSYMBOL_T_ADDRESS = 58,                 /* T_ADDRESS  */
  YYSYMBOL_T_PROTOCOL = 59,                /* T_PROTOCOL  */
  YYSYMBOL_T_STATE = 60,                   /* T_STATE  */
  YYSYMBOL_T_ACCEPT = 61,                  /* T_ACCEPT  */
  YYSYMBOL_T_FROM = 62,                    /* T_FROM  */
  YYSYMBOL_T_USERSPACE = 63,               /* T_USERSPACE  */
  YYSYMBOL_T_KERNELSPACE = 64,             /* T_KERNELSPACE  */
  YYSYMBOL_T_EVENT_ITER_LIMIT = 65,        /* T_EVENT_ITER_LIMIT  */
  YYSYMBOL_T_DEFAULT = 66,                 /* T_DEFAULT  */
  YYSYMBOL_T_NETLINK_OVERRUN_RESYNC = 67,  /* T_NETLINK_OVERRUN_RESYNC  */
  YYSYMBOL_T_NICE = 68,                    /* T_NICE  */
  YYSYMBOL_T_IPV4_DEST_ADDR = 69,          /* T_IPV4_DEST_ADDR  */
  YYSYMBOL_T_IPV6_DEST_ADDR = 70,          /* T_IPV6_DEST_ADDR  */
  YYSYMBOL_T_SCHEDULER = 71,               /* T_SCHEDULER  */
  YYSYMBOL_T_TYPE = 72,                    /* T_TYPE  */
  YYSYMBOL_T_PRIO = 73,                    /* T_PRIO  */
  YYSYMBOL_T_NETLINK_EVENTS_RELIABLE = 74, /* T_NETLINK_EVENTS_RELIABLE  */
  YYSYMBOL_T_DISABLE_INTERNAL_CACHE = 75,  /* T_DISABLE_INTERNAL_CACHE  */
  YYSYMBOL_T_DISABLE_EXTERNAL_CACHE = 76,  /* T_DISABLE_EXTERNAL_CACHE  */
  YYSYMBOL_T_ERROR_QUEUE_LENGTH = 77,      /* T_ERROR_QUEUE_LENGTH  */
  YYSYMBOL_T_OPTIONS = 78,                 /* T_OPTIONS  */
  YYSYMBOL_T_TCP_WINDOW_TRACKING = 79,     /* T_TCP_WINDOW_TRACKING  */
  YYSYMBOL_T_EXPECT_SYNC = 80,             /* T_EXPECT_SYNC  */
  YYSYMBOL_T_HELPER = 81,                  /* T_HELPER  */
  YYSYMBOL_T_HELPER_QUEUE_NUM = 82,        /* T_HELPER_QUEUE_NUM  */
  YYSYMBOL_T_HELPER_QUEUE_LEN = 83,        /* T_HELPER_QUEUE_LEN  */
  YYSYMBOL_T_HELPER_POLICY = 84,           /* T_HELPER_POLICY  */
  YYSYMBOL_T_HELPER_EXPECT_TIMEOUT = 85,   /* T_HELPER_EXPECT_TIMEOUT  */
  YYSYMBOL_T_HELPER_EXPECT_MAX = 86,       /* T_HELPER_EXPECT_MAX  */
  YYSYMBOL_T_SYSTEMD = 87,                 /* T_SYSTEMD  */
  YYSYMBOL_T_STARTUP_RESYNC = 88,          /* T_STARTUP_RESYNC  */
  YYSYMBOL_T_IP = 89,                      /* T_IP  */
  YYSYMBOL_T_PATH_VAL = 90,                /* T_PATH_VAL  */
  YYSYMBOL_T_NUMBER = 91,                  /* T_NUMBER  */
  YYSYMBOL_T_SIGNED_NUMBER = 92,           /* T_SIGNED_NUMBER  */
  YYSYMBOL_T_STRING = 93,                  /* T_STRING  */
  YYSYMBOL_94_ = 94,                       /* '{'  */
  YYSYMBOL_95_ = 95,                       /* '}'  */
  YYSYMBOL_YYACCEPT = 96,                  /* $accept  */
  YYSYMBOL_configfile = 97,                /* configfile  */
  YYSYMBOL_lines = 98,                     /* lines  */
  YYSYMBOL_line = 99,                      /* line  */
  YYSYMBOL_logfile_bool = 100,             /* logfile_bool  */
  YYSYMBOL_logfile_path = 101,             /* logfile_path  */
  YYSYMBOL_syslog_bool = 102,              /* syslog_bool  */
  YYSYMBOL_syslog_facility = 103,          /* syslog_facility  */
  YYSYMBOL_lock = 104,                     /* lock  */
  YYSYMBOL_refreshtime = 105,              /* refreshtime  */
  YYSYMBOL_expiretime = 106,               /* expiretime  */
  YYSYMBOL_timeout = 107,                  /* timeout  */
  YYSYMBOL_purge = 108,                    /* purge  */
  YYSYMBOL_multicast_line = 109,           /* multicast_line  */
  YYSYMBOL_multicast_options = 110,        /* multicast_options  */
  YYSYMBOL_multicast_option = 111,         /* multicast_option  */
  YYSYMBOL_udp_line = 112,                 /* udp_line  */
  YYSYMBOL_udp_options = 113,              /* udp_options  */
  YYSYMBOL_udp_option = 114,               /* udp_option  */
  YYSYMBOL_tcp_line = 115,                 /* tcp_line  */
  YYSYMBOL_tcp_options = 116,              /* tcp_options  */
  YYSYMBOL_tcp_option = 117,               /* tcp_option  */
  YYSYMBOL_hashsize = 118,                 /* hashsize  */
  YYSYMBOL_hashlimit = 119,                /* hashlimit  */
  YYSYMBOL_unix_line = 120,                /* unix_line  */
  YYSYMBOL_unix_options = 121,             /* unix_options  */
  YYSYMBOL_unix_option = 122,              /* unix_option  */
  YYSYMBOL_sync = 123,                     /* sync  */
  YYSYMBOL_sync_list = 124,                /* sync_list  */
  YYSYMBOL_sync_line = 125,                /* sync_line  */
  YYSYMBOL_option_line = 126,              /* option_line  */
  YYSYMBOL_options = 127,                  /* options  */
  YYSYMBOL_option = 128,                   /* option  */
  YYSYMBOL_expect_list = 129,              /* expect_list  */
  YYSYMBOL_expect_item = 130,              /* expect_item  */
  YYSYMBOL_sync_mode_alarm = 131,          /* sync_mode_alarm  */
  YYSYMBOL_sync_mode_ftfw = 132,           /* sync_mode_ftfw  */
  YYSYMBOL_sync_mode_notrack = 133,        /* sync_mode_notrack  */
  YYSYMBOL_sync_mode_alarm_list = 134,     /* sync_mode_alarm_list  */
  YYSYMBOL_sync_mode_alarm_line = 135,     /* sync_mode_alarm_line  */
  YYSYMBOL_sync_mode_ftfw_list = 136,      /* sync_mode_ftfw_list  */
  YYSYMBOL_sync_mode_ftfw_line = 137,      /* sync_mode_ftfw_line  */
  YYSYMBOL_sync_mode_notrack_list = 138,   /* sync_mode_notrack_list  */
  YYSYMBOL_sync_mode_notrack_line = 139,   /* sync_mode_notrack_line  */
  YYSYMBOL_disable_internal_cache = 140,   /* disable_internal_cache  */
  YYSYMBOL_disable_external_cache = 141,   /* disable_external_cache  */
  YYSYMBOL_resend_queue_size = 142,        /* resend_queue_size  */
  YYSYMBOL_startup_resync = 143,           /* startup_resync  */
  YYSYMBOL_window_size = 144,              /* window_size  */
  YYSYMBOL_tcp_states = 145,               /* tcp_states  */
  YYSYMBOL_tcp_state = 146,                /* tcp_state  */
  YYSYMBOL_general = 147,                  /* general  */
  YYSYMBOL_general_list = 148,             /* general_list  */
  YYSYMBOL_general_line = 149,             /* general_line  */
  YYSYMBOL_systemd = 150,                  /* systemd  */
  YYSYMBOL_netlink_buffer_size = 151,      /* netlink_buffer_size  */
  YYSYMBOL_netlink_buffer_size_max_grown = 152, /* netlink_buffer_size_max_grown  */
  YYSYMBOL_netlink_overrun_resync = 153,   /* netlink_overrun_resync  */
  YYSYMBOL_netlink_events_reliable = 154,  /* netlink_events_reliable  */
  YYSYMBOL_nice = 155,                     /* nice  */
  YYSYMBOL_scheduler = 156,                /* scheduler  */
  YYSYMBOL_scheduler_options = 157,        /* scheduler_options  */
  YYSYMBOL_scheduler_line = 158,           /* scheduler_line  */
  YYSYMBOL_event_iterations_limit = 159,   /* event_iterations_limit  */
  YYSYMBOL_poll_secs = 160,                /* poll_secs  */
  YYSYMBOL_filter = 161,                   /* filter  */
  YYSYMBOL_filter_list = 162,              /* filter_list  */
  YYSYMBOL_filter_item = 163,              /* filter_item  */
  YYSYMBOL_filter_protocol_list = 164,     /* filter_protocol_list  */
  YYSYMBOL_filter_protocol_item = 165,     /* filter_protocol_item  */
  YYSYMBOL_filter_address_list = 166,      /* filter_address_list  */
  YYSYMBOL_filter_address_item = 167,      /* filter_address_item  */
  YYSYMBOL_filter_state_list = 168,        /* filter_state_list  */
  YYSYMBOL_filter_state_item = 169,        /* filter_state_item  */
  YYSYMBOL_stats = 170,                    /* stats  */
  YYSYMBOL_stats_list = 171,               /* stats_list  */
  YYSYMBOL_stat_line = 172,                /* stat_line  */
  YYSYMBOL_stat_logfile_bool = 173,        /* stat_logfile_bool  */
  YYSYMBOL_stat_logfile_path = 174,        /* stat_logfile_path  */
  YYSYMBOL_stat_syslog_bool = 175,         /* stat_syslog_bool  */
  YYSYMBOL_stat_syslog_facility = 176,     /* stat_syslog_facility  */
  YYSYMBOL_helper = 177,                   /* helper  */
  YYSYMBOL_helper_list = 178,              /* helper_list  */
  YYSYMBOL_helper_line = 179,              /* helper_line  */
  YYSYMBOL_helper_type = 180,              /* helper_type  */
  YYSYMBOL_helper_setup = 181,             /* helper_setup  */
  YYSYMBOL_helper_type_list = 182,         /* helper_type_list  */
  YYSYMBOL_helper_type_line = 183,         /* helper_type_line  */
  YYSYMBOL_helper_policy_list = 184,       /* helper_policy_list  */
  YYSYMBOL_helper_policy_line = 185,       /* helper_policy_line  */
  YYSYMBOL_helper_policy_expect_max = 186, /* helper_policy_expect_max  */
  YYSYMBOL_helper_policy_expect_timeout = 187 /* helper_policy_expect_timeout  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  16
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   334

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  96
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  92
/* YYNRULES -- Number of rules.  */
#define YYNRULES  232
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  371

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   348


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    94,     2,    95,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   115,   115,   116,   119,   120,   123,   124,   125,   126,
     129,   134,   138,   149,   154,   159,   193,   204,   209,   214,
     219,   224,   240,   257,   258,   260,   281,   323,   344,   350,
     372,   379,   386,   393,   400,   407,   423,   440,   441,   443,
     456,   475,   488,   507,   525,   532,   539,   546,   553,   560,
     578,   598,   599,   601,   614,   633,   646,   665,   683,   690,
     697,   704,   711,   718,   723,   728,   733,   735,   736,   739,
     750,   755,   765,   766,   768,   769,   770,   771,   772,   773,
     774,   775,   776,   777,   778,   781,   783,   784,   787,   792,
     797,   809,   817,   829,   830,   832,   838,   843,   848,   853,
     854,   856,   857,   858,   859,   862,   863,   865,   866,   867,
     868,   869,   870,   873,   874,   876,   877,   878,   879,   880,
     883,   888,   893,   898,   903,   908,   913,   918,   923,   924,
     926,   934,   942,   950,   958,   966,   974,   982,   990,   999,
    1001,  1002,  1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,
    1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,
    1025,  1026,  1028,  1033,  1038,  1043,  1048,  1053,  1058,  1063,
    1069,  1071,  1072,  1075,  1090,  1099,  1104,  1114,  1119,  1124,
    1129,  1130,  1132,  1141,  1154,  1155,  1157,  1178,  1197,  1216,
    1225,  1247,  1248,  1250,  1311,  1374,  1383,  1397,  1398,  1400,
    1402,  1412,  1413,  1416,  1417,  1418,  1419,  1422,  1427,  1431,
    1442,  1447,  1452,  1486,  1491,  1492,  1495,  1496,  1499,  1605,
    1610,  1615,  1616,  1619,  1622,  1633,  1644,  1671,  1672,  1675,
    1676,  1679,  1694
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "T_IPV4_ADDR",
  "T_IPV4_IFACE", "T_PORT", "T_HASHSIZE", "T_HASHLIMIT", "T_MULTICAST",
  "T_PATH", "T_UNIX", "T_REFRESH", "T_IPV6_ADDR", "T_IPV6_IFACE",
  "T_BACKLOG", "T_GROUP", "T_IGNORE", "T_SETUP", "T_LOG", "T_UDP",
  "T_ICMP", "T_IGMP", "T_VRRP", "T_TCP", "T_LOCK",
  "T_BUFFER_SIZE_MAX_GROWN", "T_EXPIRE", "T_TIMEOUT", "T_GENERAL",
  "T_SYNC", "T_STATS", "T_BUFFER_SIZE", "T_SYNC_MODE", "T_ALARM", "T_FTFW",
  "T_CHECKSUM", "T_WINDOWSIZE", "T_ON", "T_OFF", "T_FOR", "T_IFACE",
  "T_PURGE", "T_RESEND_QUEUE_SIZE", "T_ESTABLISHED", "T_SYN_SENT",
  "T_SYN_RECV", "T_FIN_WAIT", "T_CLOSE_WAIT", "T_LAST_ACK", "T_TIME_WAIT",
  "T_CLOSE", "T_LISTEN", "T_SYSLOG", "T_RCVBUFF", "T_SNDBUFF", "T_NOTRACK",
  "T_POLL_SECS", "T_FILTER", "T_ADDRESS", "T_PROTOCOL", "T_STATE",
  "T_ACCEPT", "T_FROM", "T_USERSPACE", "T_KERNELSPACE",
  "T_EVENT_ITER_LIMIT", "T_DEFAULT", "T_NETLINK_OVERRUN_RESYNC", "T_NICE",
  "T_IPV4_DEST_ADDR", "T_IPV6_DEST_ADDR", "T_SCHEDULER", "T_TYPE",
  "T_PRIO", "T_NETLINK_EVENTS_RELIABLE", "T_DISABLE_INTERNAL_CACHE",
  "T_DISABLE_EXTERNAL_CACHE", "T_ERROR_QUEUE_LENGTH", "T_OPTIONS",
  "T_TCP_WINDOW_TRACKING", "T_EXPECT_SYNC", "T_HELPER",
  "T_HELPER_QUEUE_NUM", "T_HELPER_QUEUE_LEN", "T_HELPER_POLICY",
  "T_HELPER_EXPECT_TIMEOUT", "T_HELPER_EXPECT_MAX", "T_SYSTEMD",
  "T_STARTUP_RESYNC", "T_IP", "T_PATH_VAL", "T_NUMBER", "T_SIGNED_NUMBER",
  "T_STRING", "'{'", "'}'", "$accept", "configfile", "lines", "line",
  "logfile_bool", "logfile_path", "syslog_bool", "syslog_facility", "lock",
  "refreshtime", "expiretime", "timeout", "purge", "multicast_line",
  "multicast_options", "multicast_option", "udp_line", "udp_options",
  "udp_option", "tcp_line", "tcp_options", "tcp_option", "hashsize",
  "hashlimit", "unix_line", "unix_options", "unix_option", "sync",
  "sync_list", "sync_line", "option_line", "options", "option",
  "expect_list", "expect_item", "sync_mode_alarm", "sync_mode_ftfw",
  "sync_mode_notrack", "sync_mode_alarm_list", "sync_mode_alarm_line",
  "sync_mode_ftfw_list", "sync_mode_ftfw_line", "sync_mode_notrack_list",
  "sync_mode_notrack_line", "disable_internal_cache",
  "disable_external_cache", "resend_queue_size", "startup_resync",
  "window_size", "tcp_states", "tcp_state", "general", "general_list",
  "general_line", "systemd", "netlink_buffer_size",
  "netlink_buffer_size_max_grown", "netlink_overrun_resync",
  "netlink_events_reliable", "nice", "scheduler", "scheduler_options",
  "scheduler_line", "event_iterations_limit", "poll_secs", "filter",
  "filter_list", "filter_item", "filter_protocol_list",
  "filter_protocol_item", "filter_address_list", "filter_address_item",
  "filter_state_list", "filter_state_item", "stats", "stats_list",
  "stat_line", "stat_logfile_bool", "stat_logfile_path",
  "stat_syslog_bool", "stat_syslog_facility", "helper", "helper_list",
  "helper_line", "helper_type", "helper_setup", "helper_type_list",
  "helper_type_line", "helper_policy_list", "helper_policy_line",
  "helper_policy_expect_max", "helper_policy_expect_timeout", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-81)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     112,   -80,   -60,   -42,   -39,    73,   112,   -81,   -81,   -81,
     -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   101,   154,
       6,    51,    -6,    13,    49,    84,    24,    33,    39,    -7,
      65,   -29,    68,   100,    -4,    69,   141,   182,   -81,   -81,
     -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,
     -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   -45,    92,
      61,    88,   103,   108,   180,   140,   170,   -81,   -81,   -81,
     -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,
     110,    46,   -81,   -81,   -81,   -81,   -81,   -81,   206,    74,
     142,   174,   152,   -81,   -81,   -81,   -81,   -81,   -81,   -81,
     -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,
      30,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,
     -81,   -81,   172,   -81,   -81,   173,   -81,   175,   -81,   -81,
     -81,   176,   177,   178,   -81,   -81,   -81,   -81,   -81,   -81,
     -81,   -81,   -81,   -81,   181,   -81,   -81,   183,    14,   184,
     185,     4,   139,   -81,     3,   -81,    36,   -81,    -3,   -81,
     -81,   -81,    81,   187,   -81,   186,   190,   -81,   -81,   -81,
     -81,   -15,    11,    20,   -81,   -81,   189,   192,   -81,   -81,
       7,     2,     8,   195,   196,   197,   210,   193,   198,   199,
     -81,   -81,    75,   202,   201,   204,   213,   194,   203,   205,
     208,   209,   -81,   -81,     0,   211,   212,   216,   215,   214,
     218,   220,   224,   225,   226,   -81,   -81,    18,    76,   109,
     217,   -12,   -81,   -81,   207,   151,   -81,   -81,   143,   147,
     222,   227,   228,   229,   230,   231,   -81,   -81,   -81,   -81,
     -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,
     -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,
     -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,
     -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   235,   236,
     219,   221,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,
     223,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,
     -81,   -81,   -81,   -81,   237,   238,   -81,   -81,   -81,   -81,
     -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,
     -81,   -81,   -81,   -81,   -81,   -81,    94,   126,   -81,   -81,
       1,     5,    56,    97,   200,   239,   -81,   -81,   -81,   -81,
     -81,   -81,   241,   242,   -81,   -81,   -81,   -81,   -81,   -81,
     -81,   -81,   -81,   -81,   179,   -81,   -81,   -81,   -81,   245,
     -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,
     -81
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     0,     0,     0,     0,     3,     4,     7,     6,
       8,     9,   140,    72,   201,   214,     1,     5,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   139,   144,
     145,   147,   146,   148,   142,   143,   149,   141,   159,   150,
     151,   155,   156,   157,   158,   152,   153,   154,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    71,    74,    75,
      76,    77,    78,    79,    80,    73,    84,    81,    82,    83,
       0,     0,   200,   202,   203,   204,   205,   206,     0,     0,
       0,     0,     0,   213,   215,   216,   217,    64,    65,    67,
      10,    11,    12,    16,   163,   162,    13,    14,    15,   176,
       0,   180,   175,   164,   165,   166,   169,   171,   167,   168,
     160,   161,     0,    23,    17,     0,    37,     0,    51,    18,
      19,     0,     0,     0,    20,    86,   207,   208,   209,   210,
     211,   212,   219,   220,     0,   224,   225,     0,     0,     0,
       0,     0,     0,    23,     0,    37,     0,    51,     0,    99,
     105,   113,     0,     0,   227,     0,     0,    66,    68,   180,
     180,     0,     0,     0,   177,   181,     0,     0,   170,   172,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      21,    24,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    35,    38,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    49,    52,     0,     0,     0,
       0,     0,    85,    87,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   173,   174,    22,    25,
      27,    26,    28,    30,    33,    34,    29,    32,    31,    36,
      39,    44,    40,    47,    48,    43,    46,    45,    41,    42,
      50,    53,    58,    54,    61,    62,    57,    60,    59,    55,
      56,    63,    96,   101,   102,   103,   104,   100,     0,     0,
       0,     0,    97,   108,   109,   106,   111,   107,   112,   110,
       0,    98,   115,   116,   114,   117,   118,   119,    88,    89,
      90,    91,    93,   221,     0,     0,   226,   228,   229,   230,
     178,   179,   191,   191,   184,   184,   197,   197,   127,   124,
     122,   123,   125,   126,   120,   121,     0,     0,   232,   231,
       0,     0,     0,     0,   128,   128,    95,    92,    94,   218,
     223,   222,     0,     0,   190,   192,   189,   188,   187,   186,
     183,   185,   182,   196,     0,   198,   195,   193,   194,     0,
     132,   130,   131,   133,   134,   135,   136,   137,   138,   129,
     199
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -81,   -81,   -81,   267,   -81,   -81,   -81,   -81,   -81,    58,
      82,    -2,    22,   -81,   149,   -81,   -81,   153,   -81,   -81,
     155,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,
     -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,
     -81,   -81,   -81,   -81,   -81,    85,   -81,    87,   -81,   -81,
     -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,
     -81,   -81,   -81,   -81,   -81,   -81,    93,   -81,    -5,   -81,
      19,   -81,    16,   -81,   -81,   -81,   -81,   -81,   -81,   -81,
     -81,   -81,   -81,   -81,    -9,   -81,   -81,   -81,   -81,   -81,
     -81,   -81
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     5,     6,     7,    39,    40,    41,    42,    43,    68,
      69,    70,    71,    72,   154,   191,    73,   156,   203,    74,
     158,   216,    44,    45,    46,   148,   168,     8,    19,    75,
      76,   162,   223,   326,   338,    77,    78,    79,   217,   277,
     218,   285,   219,   294,   295,   286,   287,   288,   289,   354,
     369,     9,    18,    47,    48,    49,    50,    51,    52,    53,
      54,   152,   179,    55,    56,    57,   151,   175,   332,   351,
     330,   345,   334,   355,    10,    20,    83,    84,    85,    86,
      87,    11,    21,    94,    95,    96,   327,   341,   225,   307,
     308,   309
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     205,   230,   206,   205,   342,   206,   181,   182,   342,   207,
     181,   182,   207,   343,    12,   183,   184,   343,   185,   183,
     184,   122,   185,   165,    80,   300,   301,   232,   166,    59,
     106,   107,   208,   110,    13,   208,   234,   209,   186,   193,
     209,   194,   186,   187,    62,    63,   231,   187,   195,   123,
     210,   211,    14,   210,   211,    15,   188,   189,    81,    65,
     188,   189,   171,   172,   173,   111,   212,   213,    88,   212,
     213,   196,   233,    16,   214,   347,   197,   214,   193,   348,
     194,   235,   302,   139,   140,    97,   108,   195,   116,   198,
     199,   239,   215,   149,   150,   260,   344,   240,   190,   174,
     346,    82,   238,    63,    98,   200,   201,    22,    23,   167,
     196,    24,   278,   272,   103,   197,   347,    65,   279,    25,
     348,   100,   101,    89,   104,    26,    27,   125,   198,   199,
     105,   202,    28,    90,    91,    92,    63,   113,   114,   141,
       1,     2,     3,    99,   200,   201,    93,   136,   137,   349,
      65,   350,   280,    29,   127,   126,   109,    30,    31,   112,
     220,   221,    58,   117,   281,    59,    32,   144,    33,    34,
     249,   282,    35,    60,   102,    36,   222,    61,   118,   119,
      62,    63,   128,   124,   290,   280,    64,   336,    37,   337,
     349,   115,   352,     4,   129,    65,    38,   281,    89,   130,
     138,   171,   172,   173,   291,   171,   172,   173,    90,    91,
      92,   176,   177,   131,   132,   275,   283,   292,   359,   120,
     121,   339,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   134,    66,   145,   178,   133,   304,   305,   310,   276,
     284,   293,   311,   142,   143,   147,   306,   244,   245,    67,
     253,   254,   264,   265,   298,   299,   320,   321,   322,   323,
     324,   325,   228,   229,   135,   146,   153,   155,   370,   157,
     159,   160,   161,    17,   163,   273,   226,   164,   169,   170,
     224,   227,   236,   237,   241,   242,   246,   255,   243,   247,
     248,   250,   251,   252,   256,   353,   257,   258,   259,   274,
     261,   303,   180,   262,   296,   263,   297,   266,   192,   267,
     333,   268,   204,   269,   270,     0,   312,   271,   340,     0,
       0,   313,   314,   315,   316,   317,   318,   319,   328,   329,
     357,   358,   331,   335,   356
};

static const yytype_int16 yycheck[] =
{
       3,    16,     5,     3,     3,     5,     3,     4,     3,    12,
       3,     4,    12,    12,    94,    12,    13,    12,    15,    12,
      13,    66,    15,     9,    18,    37,    38,    16,    14,    11,
      37,    38,    35,    62,    94,    35,    16,    40,    35,     3,
      40,     5,    35,    40,    26,    27,    61,    40,    12,    94,
      53,    54,    94,    53,    54,    94,    53,    54,    52,    41,
      53,    54,    58,    59,    60,    94,    69,    70,    17,    69,
      70,    35,    61,     0,    77,    19,    40,    77,     3,    23,
       5,    61,    94,    37,    38,    91,    93,    12,    92,    53,
      54,    89,    95,    63,    64,    95,    95,    89,    95,    95,
      95,    95,    95,    27,    91,    69,    70,     6,     7,    95,
      35,    10,    36,    95,    90,    40,    19,    41,    42,    18,
      23,    37,    38,    72,    91,    24,    25,    66,    53,    54,
      91,    95,    31,    82,    83,    84,    27,    37,    38,    93,
      28,    29,    30,    94,    69,    70,    95,    37,    38,    93,
      41,    95,    76,    52,    66,    94,    91,    56,    57,    91,
      79,    80,     8,    94,    88,    11,    65,    93,    67,    68,
      95,    95,    71,    19,    90,    74,    95,    23,    37,    38,
      26,    27,    94,    91,    75,    76,    32,    93,    87,    95,
      93,    91,    95,    81,    91,    41,    95,    88,    72,    91,
      90,    58,    59,    60,    95,    58,    59,    60,    82,    83,
      84,    72,    73,    33,    34,   217,   218,   219,    39,    37,
      38,    95,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    91,    78,    91,    95,    55,    85,    86,    95,   217,
     218,   219,    95,    37,    38,    93,    95,    37,    38,    95,
      37,    38,    37,    38,    37,    38,    37,    38,    37,    38,
      37,    38,   169,   170,    94,    91,    94,    94,    23,    94,
      94,    94,    94,     6,    93,   217,    90,    94,    94,    94,
      93,    91,    93,    91,    89,    89,    93,    93,    91,    91,
      91,    89,    91,    89,    91,    95,    91,    89,    89,   217,
      89,    94,   153,    91,   219,    89,   219,    93,   155,    91,
     315,    91,   157,    89,    89,    -1,    94,    91,   327,    -1,
      -1,    94,    94,    94,    94,    94,    91,    91,    91,    91,
      89,    89,   313,   317,    95
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    28,    29,    30,    81,    97,    98,    99,   123,   147,
     170,   177,    94,    94,    94,    94,     0,    99,   148,   124,
     171,   178,     6,     7,    10,    18,    24,    25,    31,    52,
      56,    57,    65,    67,    68,    71,    74,    87,    95,   100,
     101,   102,   103,   104,   118,   119,   120,   149,   150,   151,
     152,   153,   154,   155,   156,   159,   160,   161,     8,    11,
      19,    23,    26,    27,    32,    41,    78,    95,   105,   106,
     107,   108,   109,   112,   115,   125,   126,   131,   132,   133,
      18,    52,    95,   172,   173,   174,   175,   176,    17,    72,
      82,    83,    84,    95,   179,   180,   181,    91,    91,    94,
      37,    38,    90,    90,    91,    91,    37,    38,    93,    91,
      62,    94,    91,    37,    38,    91,    92,    94,    37,    38,
      37,    38,    66,    94,    91,    66,    94,    66,    94,    91,
      91,    33,    34,    55,    91,    94,    37,    38,    90,    37,
      38,    93,    37,    38,    93,    91,    91,    93,   121,    63,
      64,   162,   157,    94,   110,    94,   113,    94,   116,    94,
      94,    94,   127,    93,    94,     9,    14,    95,   122,    94,
      94,    58,    59,    60,    95,   163,    72,    73,    95,   158,
     110,     3,     4,    12,    13,    15,    35,    40,    53,    54,
      95,   111,   113,     3,     5,    12,    35,    40,    53,    54,
      69,    70,    95,   114,   116,     3,     5,    12,    35,    40,
      53,    54,    69,    70,    77,    95,   117,   134,   136,   138,
      79,    80,    95,   128,    93,   184,    90,    91,   162,   162,
      16,    61,    16,    61,    16,    61,    93,    91,    95,    89,
      89,    89,    89,    91,    37,    38,    93,    91,    91,    95,
      89,    91,    89,    37,    38,    93,    91,    91,    89,    89,
      95,    89,    91,    89,    37,    38,    93,    91,    91,    89,
      89,    91,    95,   105,   106,   107,   108,   135,    36,    42,
      76,    88,    95,   107,   108,   137,   141,   142,   143,   144,
      75,    95,   107,   108,   139,   140,   141,   143,    37,    38,
      37,    38,    94,    94,    85,    86,    95,   185,   186,   187,
      95,    95,    94,    94,    94,    94,    94,    94,    91,    91,
      37,    38,    37,    38,    37,    38,   129,   182,    91,    91,
     166,   166,   164,   164,   168,   168,    93,    95,   130,    95,
     180,   183,     3,    12,    95,   167,    95,    19,    23,    93,
      95,   165,    95,    95,   145,   169,    95,    89,    89,    39,
      43,    44,    45,    46,    47,    48,    49,    50,    51,   146,
      23
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    96,    97,    97,    98,    98,    99,    99,    99,    99,
     100,   100,   101,   102,   102,   103,   104,   105,   106,   107,
     108,   109,   109,   110,   110,   111,   111,   111,   111,   111,
     111,   111,   111,   111,   111,   112,   112,   113,   113,   114,
     114,   114,   114,   114,   114,   114,   114,   114,   114,   115,
     115,   116,   116,   117,   117,   117,   117,   117,   117,   117,
     117,   117,   117,   117,   118,   119,   120,   121,   121,   122,
     122,   123,   124,   124,   125,   125,   125,   125,   125,   125,
     125,   125,   125,   125,   125,   126,   127,   127,   128,   128,
     128,   128,   128,   129,   129,   130,   131,   132,   133,   134,
     134,   135,   135,   135,   135,   136,   136,   137,   137,   137,
     137,   137,   137,   138,   138,   139,   139,   139,   139,   139,
     140,   140,   141,   141,   142,   143,   143,   144,   145,   145,
     146,   146,   146,   146,   146,   146,   146,   146,   146,   147,
     148,   148,   149,   149,   149,   149,   149,   149,   149,   149,
     149,   149,   149,   149,   149,   149,   149,   149,   149,   149,
     150,   150,   151,   152,   153,   153,   153,   154,   154,   155,
     156,   157,   157,   158,   158,   159,   160,   161,   161,   161,
     162,   162,   163,   163,   164,   164,   165,   165,   165,   163,
     163,   166,   166,   167,   167,   163,   163,   168,   168,   169,
     170,   171,   171,   172,   172,   172,   172,   173,   173,   174,
     175,   175,   176,   177,   178,   178,   179,   179,   180,   181,
     181,   182,   182,   183,   180,   180,   180,   184,   184,   185,
     185,   186,   187
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     1,     1,     2,     1,     1,     1,     1,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     4,     5,     0,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     4,     5,     0,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     4,
       5,     0,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     4,     0,     2,     2,
       2,     4,     0,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     4,     0,     2,     2,     2,
       2,     2,     4,     0,     2,     1,     5,     5,     5,     0,
       2,     1,     1,     1,     1,     0,     2,     1,     1,     1,
       1,     1,     1,     0,     2,     1,     1,     1,     1,     1,
       2,     2,     2,     2,     2,     2,     2,     2,     0,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     4,
       0,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       4,     0,     2,     2,     2,     2,     2,     4,     6,     6,
       0,     2,     5,     5,     0,     2,     1,     1,     1,     5,
       5,     0,     2,     2,     2,     5,     5,     0,     2,     3,
       4,     0,     2,     1,     1,     1,     1,     2,     2,     2,
       2,     2,     2,     4,     0,     2,     1,     1,     7,     2,
       2,     0,     2,     1,     2,     2,     5,     0,     2,     1,
       1,     2,     2
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 10: /* logfile_bool: T_LOG T_ON  */
#line 130 "read_config_yy.y"
{
	strncpy(conf.logfile, DEFAULT_LOGFILE, FILENAME_MAXLEN);
}
#line 1865 "read_config_yy.c"
    break;

  case 11: /* logfile_bool: T_LOG T_OFF  */
#line 135 "read_config_yy.y"
{
}
#line 1872 "read_config_yy.c"
    break;

  case 12: /* logfile_path: T_LOG T_PATH_VAL  */
#line 139 "read_config_yy.y"
{
	if (strlen((yyvsp[0].string)) > FILENAME_MAXLEN) {
		dlog(LOG_ERR, "LogFile path is longer than %u characters",
		     FILENAME_MAXLEN);
		exit(EXIT_FAILURE);
	}
	snprintf(conf.logfile, sizeof(conf.logfile), "%s", (yyvsp[0].string));
	free((yyvsp[0].string));
}
#line 1886 "read_config_yy.c"
    break;

  case 13: /* syslog_bool: T_SYSLOG T_ON  */
#line 150 "read_config_yy.y"
{
	conf.syslog_facility = DEFAULT_SYSLOG_FACILITY;
}
#line 1894 "read_config_yy.c"
    break;

  case 14: /* syslog_bool: T_SYSLOG T_OFF  */
#line 155 "read_config_yy.y"
{
	conf.syslog_facility = -1;
}
#line 1902 "read_config_yy.c"
    break;

  case 15: /* syslog_facility: T_SYSLOG T_STRING  */
#line 160 "read_config_yy.y"
{
	if (!strcmp((yyvsp[0].string), "daemon"))
		conf.syslog_facility = LOG_DAEMON;
	else if (!strcmp((yyvsp[0].string), "local0"))
		conf.syslog_facility = LOG_LOCAL0;
	else if (!strcmp((yyvsp[0].string), "local1"))
		conf.syslog_facility = LOG_LOCAL1;
	else if (!strcmp((yyvsp[0].string), "local2"))
		conf.syslog_facility = LOG_LOCAL2;
	else if (!strcmp((yyvsp[0].string), "local3"))
		conf.syslog_facility = LOG_LOCAL3;
	else if (!strcmp((yyvsp[0].string), "local4"))
		conf.syslog_facility = LOG_LOCAL4;
	else if (!strcmp((yyvsp[0].string), "local5"))
		conf.syslog_facility = LOG_LOCAL5;
	else if (!strcmp((yyvsp[0].string), "local6"))
		conf.syslog_facility = LOG_LOCAL6;
	else if (!strcmp((yyvsp[0].string), "local7"))
		conf.syslog_facility = LOG_LOCAL7;
	else {
		dlog(LOG_WARNING, "'%s' is not a known syslog facility, "
		     "ignoring", (yyvsp[0].string));
		free((yyvsp[0].string));
		break;
	}
	free((yyvsp[0].string));

	if (conf.stats.syslog_facility != -1 &&
	    conf.syslog_facility != conf.stats.syslog_facility)
		dlog(LOG_WARNING, "conflicting Syslog facility "
		     "values, defaulting to General");
}
#line 1939 "read_config_yy.c"
    break;

  case 16: /* lock: T_LOCK T_PATH_VAL  */
#line 194 "read_config_yy.y"
{
	if (strlen((yyvsp[0].string)) > FILENAME_MAXLEN) {
		dlog(LOG_ERR, "LockFile path is longer than %u characters",
		     FILENAME_MAXLEN);
		exit(EXIT_FAILURE);
	}
	snprintf(conf.lockfile, sizeof(conf.lockfile), "%s", (yyvsp[0].string));
	free((yyvsp[0].string));
}
#line 1953 "read_config_yy.c"
    break;

  case 17: /* refreshtime: T_REFRESH T_NUMBER  */
#line 205 "read_config_yy.y"
{
	conf.refresh = (yyvsp[0].val);
}
#line 1961 "read_config_yy.c"
    break;

  case 18: /* expiretime: T_EXPIRE T_NUMBER  */
#line 210 "read_config_yy.y"
{
	conf.cache_timeout = (yyvsp[0].val);
}
#line 1969 "read_config_yy.c"
    break;

  case 19: /* timeout: T_TIMEOUT T_NUMBER  */
#line 215 "read_config_yy.y"
{
	conf.commit_timeout = (yyvsp[0].val);
}
#line 1977 "read_config_yy.c"
    break;

  case 20: /* purge: T_PURGE T_NUMBER  */
#line 220 "read_config_yy.y"
{
	conf.purge_timeout = (yyvsp[0].val);
}
#line 1985 "read_config_yy.c"
    break;

  case 21: /* multicast_line: T_MULTICAST '{' multicast_options '}'  */
#line 225 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();

	if (conf.channel_type_global != CHANNEL_NONE &&
	    conf.channel_type_global != CHANNEL_MCAST) {
		dlog(LOG_ERR, "cannot use `Multicast' with other "
		     "dedicated link protocols!");
		exit(EXIT_FAILURE);
	}
	conf.channel_type_global = CHANNEL_MCAST;
	channel_conf->channel_type = CHANNEL_MCAST;
	channel_conf->channel_flags = CHANNEL_F_BUFFERED;
	conf.channel_num++;
}
#line 2004 "read_config_yy.c"
    break;

  case 22: /* multicast_line: T_MULTICAST T_DEFAULT '{' multicast_options '}'  */
#line 241 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();

	if (conf.channel_type_global != CHANNEL_NONE &&
	    conf.channel_type_global != CHANNEL_MCAST) {
		dlog(LOG_ERR, "cannot use `Multicast' with other "
		     "dedicated link protocols!");
		exit(EXIT_FAILURE);
	}
	conf.channel_type_global = CHANNEL_MCAST;
	channel_conf->channel_type = CHANNEL_MCAST;
	channel_conf->channel_flags = CHANNEL_F_DEFAULT | CHANNEL_F_BUFFERED;
	conf.channel_default = conf.channel_num;
	conf.channel_num++;
}
#line 2024 "read_config_yy.c"
    break;

  case 25: /* multicast_option: T_IPV4_ADDR T_IP  */
#line 261 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();

	if (!inet_aton((yyvsp[0].string), &channel_conf->u.mcast.in.inet_addr)) {
		dlog(LOG_WARNING, "%s is not a valid IPv4 address", (yyvsp[0].string));
		free((yyvsp[0].string));
		break;
	}

        if (channel_conf->u.mcast.ipproto == AF_INET6) {
		dlog(LOG_WARNING, "your multicast address is IPv4 but "
		     "is binded to an IPv6 interface? "
		     "Surely, this is not what you want");
		break;
	}

	free((yyvsp[0].string));
	channel_conf->u.mcast.ipproto = AF_INET;
}
#line 2048 "read_config_yy.c"
    break;

  case 26: /* multicast_option: T_IPV6_ADDR T_IP  */
#line 282 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();
	int err;

	err = inet_pton(AF_INET6, (yyvsp[0].string), &channel_conf->u.mcast.in);
	if (err == 0) {
		dlog(LOG_WARNING, "%s is not a valid IPv6 address", (yyvsp[0].string));
		free((yyvsp[0].string));
		break;
	} else if (err < 0) {
		dlog(LOG_ERR, "inet_pton(): IPv6 unsupported!");
		exit(EXIT_FAILURE);
	}

	if (channel_conf->u.mcast.ipproto == AF_INET) {
		dlog(LOG_WARNING, "your multicast address is IPv6 but "
		     "is binded to an IPv4 interface? "
		     "Surely this is not what you want");
		free((yyvsp[0].string));
		break;
	}

	channel_conf->u.mcast.ipproto = AF_INET6;

	if (channel_conf->channel_ifname[0] &&
	    !channel_conf->u.mcast.ifa.interface_index6) {
		unsigned int idx;

		idx = if_nametoindex((yyvsp[0].string));
		if (!idx) {
			dlog(LOG_WARNING, "%s is an invalid interface", (yyvsp[0].string));
			free((yyvsp[0].string));
			break;
		}

		channel_conf->u.mcast.ifa.interface_index6 = idx;
		channel_conf->u.mcast.ipproto = AF_INET6;
	}
	free((yyvsp[0].string));
}
#line 2093 "read_config_yy.c"
    break;

  case 27: /* multicast_option: T_IPV4_IFACE T_IP  */
#line 324 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();

	if (!inet_aton((yyvsp[0].string), &channel_conf->u.mcast.ifa.interface_addr)) {
		dlog(LOG_WARNING, "%s is not a valid IPv4 address", (yyvsp[0].string));
		free((yyvsp[0].string));
		break;
	}
	free((yyvsp[0].string));

        if (channel_conf->u.mcast.ipproto == AF_INET6) {
		dlog(LOG_WARNING, "your multicast interface is IPv4 but "
		     "is binded to an IPv6 interface? "
		     "Surely, this is not what you want");
		break;
	}

	channel_conf->u.mcast.ipproto = AF_INET;
}
#line 2117 "read_config_yy.c"
    break;

  case 28: /* multicast_option: T_IPV6_IFACE T_IP  */
#line 345 "read_config_yy.y"
{
	dlog(LOG_WARNING, "`IPv6_interface' not required, ignoring");
	free((yyvsp[0].string));
}
#line 2126 "read_config_yy.c"
    break;

  case 29: /* multicast_option: T_IFACE T_STRING  */
#line 351 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();
	unsigned int idx;

	strncpy(channel_conf->channel_ifname, (yyvsp[0].string), IFNAMSIZ);

	idx = if_nametoindex((yyvsp[0].string));
	if (!idx) {
		dlog(LOG_WARNING, "%s is an invalid interface", (yyvsp[0].string));
		free((yyvsp[0].string));
		break;
	}

	if (channel_conf->u.mcast.ipproto == AF_INET6) {
		channel_conf->u.mcast.ifa.interface_index6 = idx;
		channel_conf->u.mcast.ipproto = AF_INET6;
	}

	free((yyvsp[0].string));
}
#line 2151 "read_config_yy.c"
    break;

  case 30: /* multicast_option: T_GROUP T_NUMBER  */
#line 373 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();

	channel_conf->u.mcast.port = (yyvsp[0].val);
}
#line 2161 "read_config_yy.c"
    break;

  case 31: /* multicast_option: T_SNDBUFF T_NUMBER  */
#line 380 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();

	channel_conf->u.mcast.sndbuf = (yyvsp[0].val);
}
#line 2171 "read_config_yy.c"
    break;

  case 32: /* multicast_option: T_RCVBUFF T_NUMBER  */
#line 387 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();

	channel_conf->u.mcast.rcvbuf = (yyvsp[0].val);
}
#line 2181 "read_config_yy.c"
    break;

  case 33: /* multicast_option: T_CHECKSUM T_ON  */
#line 394 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();

	channel_conf->u.mcast.checksum = 0;
}
#line 2191 "read_config_yy.c"
    break;

  case 34: /* multicast_option: T_CHECKSUM T_OFF  */
#line 401 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();

	channel_conf->u.mcast.checksum = 1;
}
#line 2201 "read_config_yy.c"
    break;

  case 35: /* udp_line: T_UDP '{' udp_options '}'  */
#line 408 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();

	if (conf.channel_type_global != CHANNEL_NONE &&
	    conf.channel_type_global != CHANNEL_UDP) {
		dlog(LOG_ERR, "cannot use `UDP' with other "
		     "dedicated link protocols!");
		exit(EXIT_FAILURE);
	}
	conf.channel_type_global = CHANNEL_UDP;
	channel_conf->channel_type = CHANNEL_UDP;
	channel_conf->channel_flags = CHANNEL_F_BUFFERED;
	conf.channel_num++;
}
#line 2220 "read_config_yy.c"
    break;

  case 36: /* udp_line: T_UDP T_DEFAULT '{' udp_options '}'  */
#line 424 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();

	if (conf.channel_type_global != CHANNEL_NONE &&
	    conf.channel_type_global != CHANNEL_UDP) {
		dlog(LOG_ERR, "cannot use `UDP' with other "
		     "dedicated link protocols!");
		exit(EXIT_FAILURE);
	}
	conf.channel_type_global = CHANNEL_UDP;
	channel_conf->channel_type = CHANNEL_UDP;
	channel_conf->channel_flags = CHANNEL_F_DEFAULT | CHANNEL_F_BUFFERED;
	conf.channel_default = conf.channel_num;
	conf.channel_num++;
}
#line 2240 "read_config_yy.c"
    break;

  case 39: /* udp_option: T_IPV4_ADDR T_IP  */
#line 444 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();

	if (!inet_aton((yyvsp[0].string), &channel_conf->u.udp.server.ipv4.inet_addr)) {
		dlog(LOG_WARNING, "%s is not a valid IPv4 address", (yyvsp[0].string));
		free((yyvsp[0].string));
		break;
	}
	free((yyvsp[0].string));
	channel_conf->u.udp.ipproto = AF_INET;
}
#line 2256 "read_config_yy.c"
    break;

  case 40: /* udp_option: T_IPV6_ADDR T_IP  */
#line 457 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();
	int err;

	err = inet_pton(AF_INET6, (yyvsp[0].string), &channel_conf->u.udp.server.ipv6);
	if (err == 0) {
		dlog(LOG_WARNING, "%s is not a valid IPv6 address", (yyvsp[0].string));
		free((yyvsp[0].string));
		break;
	} else if (err < 0) {
		dlog(LOG_ERR, "inet_pton(): IPv6 unsupported!");
		exit(EXIT_FAILURE);
	}

	free((yyvsp[0].string));
	channel_conf->u.udp.ipproto = AF_INET6;
}
#line 2278 "read_config_yy.c"
    break;

  case 41: /* udp_option: T_IPV4_DEST_ADDR T_IP  */
#line 476 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();

	if (!inet_aton((yyvsp[0].string), &channel_conf->u.udp.client.inet_addr)) {
		dlog(LOG_WARNING, "%s is not a valid IPv4 address", (yyvsp[0].string));
		free((yyvsp[0].string));
		break;
	}
	free((yyvsp[0].string));
	channel_conf->u.udp.ipproto = AF_INET;
}
#line 2294 "read_config_yy.c"
    break;

  case 42: /* udp_option: T_IPV6_DEST_ADDR T_IP  */
#line 489 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();
	int err;

	err = inet_pton(AF_INET6, (yyvsp[0].string), &channel_conf->u.udp.client);
	if (err == 0) {
		dlog(LOG_WARNING, "%s is not a valid IPv6 address", (yyvsp[0].string));
		free((yyvsp[0].string));
		break;
	} else if (err < 0) {
		dlog(LOG_ERR, "inet_pton(): IPv6 unsupported!");
		exit(EXIT_FAILURE);
	}

	free((yyvsp[0].string));
	channel_conf->u.udp.ipproto = AF_INET6;
}
#line 2316 "read_config_yy.c"
    break;

  case 43: /* udp_option: T_IFACE T_STRING  */
#line 508 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();
	int idx;

	strncpy(channel_conf->channel_ifname, (yyvsp[0].string), IFNAMSIZ);

	idx = if_nametoindex((yyvsp[0].string));
	if (!idx) {
		dlog(LOG_WARNING, "%s is an invalid interface", (yyvsp[0].string));
		free((yyvsp[0].string));
		break;
	}
	channel_conf->u.udp.server.ipv6.scope_id = idx;

	free((yyvsp[0].string));
}
#line 2337 "read_config_yy.c"
    break;

  case 44: /* udp_option: T_PORT T_NUMBER  */
#line 526 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();

	channel_conf->u.udp.port = (yyvsp[0].val);
}
#line 2347 "read_config_yy.c"
    break;

  case 45: /* udp_option: T_SNDBUFF T_NUMBER  */
#line 533 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();

	channel_conf->u.udp.sndbuf = (yyvsp[0].val);
}
#line 2357 "read_config_yy.c"
    break;

  case 46: /* udp_option: T_RCVBUFF T_NUMBER  */
#line 540 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();

	channel_conf->u.udp.rcvbuf = (yyvsp[0].val);
}
#line 2367 "read_config_yy.c"
    break;

  case 47: /* udp_option: T_CHECKSUM T_ON  */
#line 547 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();

	channel_conf->u.udp.checksum = 0;
}
#line 2377 "read_config_yy.c"
    break;

  case 48: /* udp_option: T_CHECKSUM T_OFF  */
#line 554 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();

	channel_conf->u.udp.checksum = 1;
}
#line 2387 "read_config_yy.c"
    break;

  case 49: /* tcp_line: T_TCP '{' tcp_options '}'  */
#line 561 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();

	if (conf.channel_type_global != CHANNEL_NONE &&
	    conf.channel_type_global != CHANNEL_TCP) {
		dlog(LOG_ERR, "cannot use `TCP' with other "
		     "dedicated link protocols!");
		exit(EXIT_FAILURE);
	}
	conf.channel_type_global = CHANNEL_TCP;
	channel_conf->channel_type = CHANNEL_TCP;
	channel_conf->channel_flags = CHANNEL_F_BUFFERED |
				      CHANNEL_F_STREAM |
				      CHANNEL_F_ERRORS;
	conf.channel_num++;
}
#line 2408 "read_config_yy.c"
    break;

  case 50: /* tcp_line: T_TCP T_DEFAULT '{' tcp_options '}'  */
#line 579 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();

	if (conf.channel_type_global != CHANNEL_NONE &&
	    conf.channel_type_global != CHANNEL_TCP) {
		dlog(LOG_ERR, "cannot use `TCP' with other "
		     "dedicated link protocols!");
		exit(EXIT_FAILURE);
	}
	conf.channel_type_global = CHANNEL_TCP;
	channel_conf->channel_type = CHANNEL_TCP;
	channel_conf->channel_flags = CHANNEL_F_DEFAULT |
				      CHANNEL_F_BUFFERED |
				      CHANNEL_F_STREAM |
				      CHANNEL_F_ERRORS;
	conf.channel_default = conf.channel_num;
	conf.channel_num++;
}
#line 2431 "read_config_yy.c"
    break;

  case 53: /* tcp_option: T_IPV4_ADDR T_IP  */
#line 602 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();

	if (!inet_aton((yyvsp[0].string), &channel_conf->u.tcp.server.ipv4.inet_addr)) {
		dlog(LOG_WARNING, "%s is not a valid IPv4 address", (yyvsp[0].string));
		free((yyvsp[0].string));
		break;
	}
	free((yyvsp[0].string));
	channel_conf->u.tcp.ipproto = AF_INET;
}
#line 2447 "read_config_yy.c"
    break;

  case 54: /* tcp_option: T_IPV6_ADDR T_IP  */
#line 615 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();
	int err;

	err = inet_pton(AF_INET6, (yyvsp[0].string), &channel_conf->u.tcp.server.ipv6);
	if (err == 0) {
		dlog(LOG_WARNING, "%s is not a valid IPv6 address", (yyvsp[0].string));
		free((yyvsp[0].string));
		break;
	} else if (err < 0) {
		dlog(LOG_ERR, "inet_pton(): IPv6 unsupported!");
		exit(EXIT_FAILURE);
	}

	free((yyvsp[0].string));
	channel_conf->u.tcp.ipproto = AF_INET6;
}
#line 2469 "read_config_yy.c"
    break;

  case 55: /* tcp_option: T_IPV4_DEST_ADDR T_IP  */
#line 634 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();

	if (!inet_aton((yyvsp[0].string), &channel_conf->u.tcp.client.inet_addr)) {
		dlog(LOG_WARNING, "%s is not a valid IPv4 address", (yyvsp[0].string));
		free((yyvsp[0].string));
		break;
	}
	free((yyvsp[0].string));
	channel_conf->u.tcp.ipproto = AF_INET;
}
#line 2485 "read_config_yy.c"
    break;

  case 56: /* tcp_option: T_IPV6_DEST_ADDR T_IP  */
#line 647 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();
	int err;

	err = inet_pton(AF_INET6, (yyvsp[0].string), &channel_conf->u.tcp.client);
	if (err == 0) {
		dlog(LOG_WARNING, "%s is not a valid IPv6 address", (yyvsp[0].string));
		free((yyvsp[0].string));
		break;
	} else if (err < 0) {
		dlog(LOG_ERR, "inet_pton(): IPv6 unsupported!");
		exit(EXIT_FAILURE);
	}

	free((yyvsp[0].string));
	channel_conf->u.tcp.ipproto = AF_INET6;
}
#line 2507 "read_config_yy.c"
    break;

  case 57: /* tcp_option: T_IFACE T_STRING  */
#line 666 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();
	int idx;

	strncpy(channel_conf->channel_ifname, (yyvsp[0].string), IFNAMSIZ);

	idx = if_nametoindex((yyvsp[0].string));
	if (!idx) {
		dlog(LOG_WARNING, "%s is an invalid interface", (yyvsp[0].string));
		free((yyvsp[0].string));
		break;
	}
	channel_conf->u.tcp.server.ipv6.scope_id = idx;

	free((yyvsp[0].string));
}
#line 2528 "read_config_yy.c"
    break;

  case 58: /* tcp_option: T_PORT T_NUMBER  */
#line 684 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();

	channel_conf->u.tcp.port = (yyvsp[0].val);
}
#line 2538 "read_config_yy.c"
    break;

  case 59: /* tcp_option: T_SNDBUFF T_NUMBER  */
#line 691 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();

	channel_conf->u.tcp.sndbuf = (yyvsp[0].val);
}
#line 2548 "read_config_yy.c"
    break;

  case 60: /* tcp_option: T_RCVBUFF T_NUMBER  */
#line 698 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();

	channel_conf->u.tcp.rcvbuf = (yyvsp[0].val);
}
#line 2558 "read_config_yy.c"
    break;

  case 61: /* tcp_option: T_CHECKSUM T_ON  */
#line 705 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();

	channel_conf->u.tcp.checksum = 0;
}
#line 2568 "read_config_yy.c"
    break;

  case 62: /* tcp_option: T_CHECKSUM T_OFF  */
#line 712 "read_config_yy.y"
{
	struct channel_conf *channel_conf = conf_get_channel();

	channel_conf->u.tcp.checksum = 1;
}
#line 2578 "read_config_yy.c"
    break;

  case 63: /* tcp_option: T_ERROR_QUEUE_LENGTH T_NUMBER  */
#line 719 "read_config_yy.y"
{
	CONFIG(channelc).error_queue_length = (yyvsp[0].val);
}
#line 2586 "read_config_yy.c"
    break;

  case 64: /* hashsize: T_HASHSIZE T_NUMBER  */
#line 724 "read_config_yy.y"
{
	conf.hashsize = (yyvsp[0].val);
}
#line 2594 "read_config_yy.c"
    break;

  case 65: /* hashlimit: T_HASHLIMIT T_NUMBER  */
#line 729 "read_config_yy.y"
{
	conf.limit = (yyvsp[0].val);
}
#line 2602 "read_config_yy.c"
    break;

  case 69: /* unix_option: T_PATH T_PATH_VAL  */
#line 740 "read_config_yy.y"
{
	if (strlen((yyvsp[0].string)) >= UNIX_PATH_MAX) {
		dlog(LOG_ERR, "Path is longer than %u characters",
		     UNIX_PATH_MAX - 1);
		exit(EXIT_FAILURE);
	}
	strcpy(conf.local.path, (yyvsp[0].string));
	free((yyvsp[0].string));
}
#line 2616 "read_config_yy.c"
    break;

  case 70: /* unix_option: T_BACKLOG T_NUMBER  */
#line 751 "read_config_yy.y"
{
	dlog(LOG_WARNING, "deprecated unix backlog configuration, ignoring.");
}
#line 2624 "read_config_yy.c"
    break;

  case 71: /* sync: T_SYNC '{' sync_list '}'  */
#line 756 "read_config_yy.y"
{
	if (conf.flags & CTD_STATS_MODE) {
		dlog(LOG_ERR, "cannot use both `Stats' and `Sync' "
		     "clauses in conntrackd.conf");
		exit(EXIT_FAILURE);
	}
	conf.flags |= CTD_SYNC_MODE;
}
#line 2637 "read_config_yy.c"
    break;

  case 88: /* option: T_TCP_WINDOW_TRACKING T_ON  */
#line 788 "read_config_yy.y"
{
	CONFIG(sync).tcp_window_tracking = 1;
}
#line 2645 "read_config_yy.c"
    break;

  case 89: /* option: T_TCP_WINDOW_TRACKING T_OFF  */
#line 793 "read_config_yy.y"
{
	CONFIG(sync).tcp_window_tracking = 0;
}
#line 2653 "read_config_yy.c"
    break;

  case 90: /* option: T_EXPECT_SYNC T_ON  */
#line 798 "read_config_yy.y"
{
	CONFIG(flags) |= CTD_EXPECT;
	CONFIG(netlink).subsys_id = NFNL_SUBSYS_NONE;
	CONFIG(netlink).groups = NF_NETLINK_CONNTRACK_NEW |
				 NF_NETLINK_CONNTRACK_UPDATE |
				 NF_NETLINK_CONNTRACK_DESTROY |
				 NF_NETLINK_CONNTRACK_EXP_NEW |
				 NF_NETLINK_CONNTRACK_EXP_UPDATE |
				 NF_NETLINK_CONNTRACK_EXP_DESTROY;
}
#line 2668 "read_config_yy.c"
    break;

  case 91: /* option: T_EXPECT_SYNC T_OFF  */
#line 810 "read_config_yy.y"
{
	CONFIG(netlink).subsys_id = NFNL_SUBSYS_CTNETLINK;
	CONFIG(netlink).groups = NF_NETLINK_CONNTRACK_NEW |
				 NF_NETLINK_CONNTRACK_UPDATE |
				 NF_NETLINK_CONNTRACK_DESTROY;
}
#line 2679 "read_config_yy.c"
    break;

  case 92: /* option: T_EXPECT_SYNC '{' expect_list '}'  */
#line 818 "read_config_yy.y"
{
	CONFIG(flags) |= CTD_EXPECT;
	CONFIG(netlink).subsys_id = NFNL_SUBSYS_NONE;
	CONFIG(netlink).groups = NF_NETLINK_CONNTRACK_NEW |
				 NF_NETLINK_CONNTRACK_UPDATE |
				 NF_NETLINK_CONNTRACK_DESTROY |
				 NF_NETLINK_CONNTRACK_EXP_NEW |
				 NF_NETLINK_CONNTRACK_EXP_UPDATE |
				 NF_NETLINK_CONNTRACK_EXP_DESTROY;
}
#line 2694 "read_config_yy.c"
    break;

  case 95: /* expect_item: T_STRING  */
#line 833 "read_config_yy.y"
{
	exp_filter_add(STATE(exp_filter), (yyvsp[0].string));
	free((yyvsp[0].string));
}
#line 2703 "read_config_yy.c"
    break;

  case 96: /* sync_mode_alarm: T_SYNC_MODE T_ALARM '{' sync_mode_alarm_list '}'  */
#line 839 "read_config_yy.y"
{
	conf.flags |= CTD_SYNC_ALARM;
}
#line 2711 "read_config_yy.c"
    break;

  case 97: /* sync_mode_ftfw: T_SYNC_MODE T_FTFW '{' sync_mode_ftfw_list '}'  */
#line 844 "read_config_yy.y"
{
	conf.flags |= CTD_SYNC_FTFW;
}
#line 2719 "read_config_yy.c"
    break;

  case 98: /* sync_mode_notrack: T_SYNC_MODE T_NOTRACK '{' sync_mode_notrack_list '}'  */
#line 849 "read_config_yy.y"
{
	conf.flags |= CTD_SYNC_NOTRACK;
}
#line 2727 "read_config_yy.c"
    break;

  case 120: /* disable_internal_cache: T_DISABLE_INTERNAL_CACHE T_ON  */
#line 884 "read_config_yy.y"
{
	conf.sync.internal_cache_disable = 1;
}
#line 2735 "read_config_yy.c"
    break;

  case 121: /* disable_internal_cache: T_DISABLE_INTERNAL_CACHE T_OFF  */
#line 889 "read_config_yy.y"
{
	conf.sync.internal_cache_disable = 0;
}
#line 2743 "read_config_yy.c"
    break;

  case 122: /* disable_external_cache: T_DISABLE_EXTERNAL_CACHE T_ON  */
#line 894 "read_config_yy.y"
{
	conf.sync.external_cache_disable = 1;
}
#line 2751 "read_config_yy.c"
    break;

  case 123: /* disable_external_cache: T_DISABLE_EXTERNAL_CACHE T_OFF  */
#line 899 "read_config_yy.y"
{
	conf.sync.external_cache_disable = 0;
}
#line 2759 "read_config_yy.c"
    break;

  case 124: /* resend_queue_size: T_RESEND_QUEUE_SIZE T_NUMBER  */
#line 904 "read_config_yy.y"
{
	conf.resend_queue_size = (yyvsp[0].val);
}
#line 2767 "read_config_yy.c"
    break;

  case 125: /* startup_resync: T_STARTUP_RESYNC T_ON  */
#line 909 "read_config_yy.y"
{
	conf.startup_resync = 1;
}
#line 2775 "read_config_yy.c"
    break;

  case 126: /* startup_resync: T_STARTUP_RESYNC T_OFF  */
#line 914 "read_config_yy.y"
{
	conf.startup_resync = 0;
}
#line 2783 "read_config_yy.c"
    break;

  case 127: /* window_size: T_WINDOWSIZE T_NUMBER  */
#line 919 "read_config_yy.y"
{
	conf.window_size = (yyvsp[0].val);
}
#line 2791 "read_config_yy.c"
    break;

  case 130: /* tcp_state: T_SYN_SENT  */
#line 927 "read_config_yy.y"
{
	ct_filter_add_state(STATE(us_filter),
			    IPPROTO_TCP,
			    TCP_CONNTRACK_SYN_SENT);

	__kernel_filter_add_state(TCP_CONNTRACK_SYN_SENT);
}
#line 2803 "read_config_yy.c"
    break;

  case 131: /* tcp_state: T_SYN_RECV  */
#line 935 "read_config_yy.y"
{
	ct_filter_add_state(STATE(us_filter),
			    IPPROTO_TCP,
			    TCP_CONNTRACK_SYN_RECV);

	__kernel_filter_add_state(TCP_CONNTRACK_SYN_RECV);
}
#line 2815 "read_config_yy.c"
    break;

  case 132: /* tcp_state: T_ESTABLISHED  */
#line 943 "read_config_yy.y"
{
	ct_filter_add_state(STATE(us_filter),
			    IPPROTO_TCP,
			    TCP_CONNTRACK_ESTABLISHED);

	__kernel_filter_add_state(TCP_CONNTRACK_ESTABLISHED);
}
#line 2827 "read_config_yy.c"
    break;

  case 133: /* tcp_state: T_FIN_WAIT  */
#line 951 "read_config_yy.y"
{
	ct_filter_add_state(STATE(us_filter),
			    IPPROTO_TCP,
			    TCP_CONNTRACK_FIN_WAIT);

	__kernel_filter_add_state(TCP_CONNTRACK_FIN_WAIT);
}
#line 2839 "read_config_yy.c"
    break;

  case 134: /* tcp_state: T_CLOSE_WAIT  */
#line 959 "read_config_yy.y"
{
	ct_filter_add_state(STATE(us_filter),
			    IPPROTO_TCP,
			    TCP_CONNTRACK_CLOSE_WAIT);

	__kernel_filter_add_state(TCP_CONNTRACK_CLOSE_WAIT);
}
#line 2851 "read_config_yy.c"
    break;

  case 135: /* tcp_state: T_LAST_ACK  */
#line 967 "read_config_yy.y"
{
	ct_filter_add_state(STATE(us_filter),
			    IPPROTO_TCP,
			    TCP_CONNTRACK_LAST_ACK);

	__kernel_filter_add_state(TCP_CONNTRACK_LAST_ACK);
}
#line 2863 "read_config_yy.c"
    break;

  case 136: /* tcp_state: T_TIME_WAIT  */
#line 975 "read_config_yy.y"
{
	ct_filter_add_state(STATE(us_filter),
			    IPPROTO_TCP,
			    TCP_CONNTRACK_TIME_WAIT);

	__kernel_filter_add_state(TCP_CONNTRACK_TIME_WAIT);
}
#line 2875 "read_config_yy.c"
    break;

  case 137: /* tcp_state: T_CLOSE  */
#line 983 "read_config_yy.y"
{
	ct_filter_add_state(STATE(us_filter),
			    IPPROTO_TCP,
			    TCP_CONNTRACK_CLOSE);

	__kernel_filter_add_state(TCP_CONNTRACK_CLOSE);
}
#line 2887 "read_config_yy.c"
    break;

  case 138: /* tcp_state: T_LISTEN  */
#line 991 "read_config_yy.y"
{
	ct_filter_add_state(STATE(us_filter),
			    IPPROTO_TCP,
			    TCP_CONNTRACK_LISTEN);

	__kernel_filter_add_state(TCP_CONNTRACK_LISTEN);
}
#line 2899 "read_config_yy.c"
    break;

  case 160: /* systemd: T_SYSTEMD T_ON  */
#line 1025 "read_config_yy.y"
                                { conf.systemd = 1; }
#line 2905 "read_config_yy.c"
    break;

  case 161: /* systemd: T_SYSTEMD T_OFF  */
#line 1026 "read_config_yy.y"
                                { conf.systemd = 0; }
#line 2911 "read_config_yy.c"
    break;

  case 162: /* netlink_buffer_size: T_BUFFER_SIZE T_NUMBER  */
#line 1029 "read_config_yy.y"
{
	conf.netlink_buffer_size = (yyvsp[0].val);
}
#line 2919 "read_config_yy.c"
    break;

  case 163: /* netlink_buffer_size_max_grown: T_BUFFER_SIZE_MAX_GROWN T_NUMBER  */
#line 1034 "read_config_yy.y"
{
	conf.netlink_buffer_size_max_grown = (yyvsp[0].val);
}
#line 2927 "read_config_yy.c"
    break;

  case 164: /* netlink_overrun_resync: T_NETLINK_OVERRUN_RESYNC T_ON  */
#line 1039 "read_config_yy.y"
{
	conf.nl_overrun_resync = 30;
}
#line 2935 "read_config_yy.c"
    break;

  case 165: /* netlink_overrun_resync: T_NETLINK_OVERRUN_RESYNC T_OFF  */
#line 1044 "read_config_yy.y"
{
	conf.nl_overrun_resync = -1;
}
#line 2943 "read_config_yy.c"
    break;

  case 166: /* netlink_overrun_resync: T_NETLINK_OVERRUN_RESYNC T_NUMBER  */
#line 1049 "read_config_yy.y"
{
	conf.nl_overrun_resync = (yyvsp[0].val);
}
#line 2951 "read_config_yy.c"
    break;

  case 167: /* netlink_events_reliable: T_NETLINK_EVENTS_RELIABLE T_ON  */
#line 1054 "read_config_yy.y"
{
	conf.netlink.events_reliable = 1;
}
#line 2959 "read_config_yy.c"
    break;

  case 168: /* netlink_events_reliable: T_NETLINK_EVENTS_RELIABLE T_OFF  */
#line 1059 "read_config_yy.y"
{
	conf.netlink.events_reliable = 0;
}
#line 2967 "read_config_yy.c"
    break;

  case 169: /* nice: T_NICE T_SIGNED_NUMBER  */
#line 1064 "read_config_yy.y"
{
	dlog(LOG_WARNING, "deprecated nice configuration, ignoring. The "
	     "nice value can be set externally with nice(1) and renice(1).");
}
#line 2976 "read_config_yy.c"
    break;

  case 173: /* scheduler_line: T_TYPE T_STRING  */
#line 1076 "read_config_yy.y"
{
	if (strcasecmp((yyvsp[0].string), "rr") == 0) {
		conf.sched.type = SCHED_RR;
	} else if (strcasecmp((yyvsp[0].string), "fifo") == 0) {
		conf.sched.type = SCHED_FIFO;
	} else {
		dlog(LOG_ERR, "unknown scheduler `%s'", (yyvsp[0].string));
		free((yyvsp[0].string));
		exit(EXIT_FAILURE);
	}

	free((yyvsp[0].string));
}
#line 2994 "read_config_yy.c"
    break;

  case 174: /* scheduler_line: T_PRIO T_NUMBER  */
#line 1091 "read_config_yy.y"
{
	conf.sched.prio = (yyvsp[0].val);
	if (conf.sched.prio < 0 || conf.sched.prio > 99) {
		dlog(LOG_ERR, "`Priority' must be [0, 99]\n");
		exit(EXIT_FAILURE);
	}
}
#line 3006 "read_config_yy.c"
    break;

  case 175: /* event_iterations_limit: T_EVENT_ITER_LIMIT T_NUMBER  */
#line 1100 "read_config_yy.y"
{
	CONFIG(event_iterations_limit) = (yyvsp[0].val);
}
#line 3014 "read_config_yy.c"
    break;

  case 176: /* poll_secs: T_POLL_SECS T_NUMBER  */
#line 1105 "read_config_yy.y"
{
	conf.flags |= CTD_POLL;
	conf.poll_kernel_secs = (yyvsp[0].val);
	if (conf.poll_kernel_secs == 0) {
		dlog(LOG_ERR, "`PollSecs' clause must be > 0");
		exit(EXIT_FAILURE);
	}
}
#line 3027 "read_config_yy.c"
    break;

  case 177: /* filter: T_FILTER '{' filter_list '}'  */
#line 1115 "read_config_yy.y"
{
	CONFIG(filter_from_kernelspace) = 0;
}
#line 3035 "read_config_yy.c"
    break;

  case 178: /* filter: T_FILTER T_FROM T_USERSPACE '{' filter_list '}'  */
#line 1120 "read_config_yy.y"
{
	CONFIG(filter_from_kernelspace) = 0;
}
#line 3043 "read_config_yy.c"
    break;

  case 179: /* filter: T_FILTER T_FROM T_KERNELSPACE '{' filter_list '}'  */
#line 1125 "read_config_yy.y"
{
	CONFIG(filter_from_kernelspace) = 1;
}
#line 3051 "read_config_yy.c"
    break;

  case 182: /* filter_item: T_PROTOCOL T_ACCEPT '{' filter_protocol_list '}'  */
#line 1133 "read_config_yy.y"
{
	ct_filter_set_logic(STATE(us_filter),
			    CT_FILTER_L4PROTO,
			    CT_FILTER_POSITIVE);

	__kernel_filter_start();
}
#line 3063 "read_config_yy.c"
    break;

  case 183: /* filter_item: T_PROTOCOL T_IGNORE '{' filter_protocol_list '}'  */
#line 1142 "read_config_yy.y"
{
	ct_filter_set_logic(STATE(us_filter),
			    CT_FILTER_L4PROTO,
			    CT_FILTER_NEGATIVE);

	__kernel_filter_start();

	nfct_filter_set_logic(STATE(filter),
			      NFCT_FILTER_L4PROTO,
			      NFCT_FILTER_LOGIC_NEGATIVE);
}
#line 3079 "read_config_yy.c"
    break;

  case 186: /* filter_protocol_item: T_STRING  */
#line 1158 "read_config_yy.y"
{
	struct protoent *pent;

	pent = getprotobyname((yyvsp[0].string));
	if (pent == NULL) {
		dlog(LOG_WARNING, "getprotobyname() cannot find "
		     "protocol `%s' in /etc/protocols", (yyvsp[0].string));
		free((yyvsp[0].string));
		break;
	}
	free((yyvsp[0].string));
	ct_filter_add_proto(STATE(us_filter), pent->p_proto);

	__kernel_filter_start();

	nfct_filter_add_attr_u32(STATE(filter),
				 NFCT_FILTER_L4PROTO,
				 pent->p_proto);
}
#line 3103 "read_config_yy.c"
    break;

  case 187: /* filter_protocol_item: T_TCP  */
#line 1179 "read_config_yy.y"
{
	struct protoent *pent;

	pent = getprotobyname("tcp");
	if (pent == NULL) {
		dlog(LOG_WARNING, "getprotobyname() cannot find "
		     "protocol `tcp' in /etc/protocols");
		break;
	}
	ct_filter_add_proto(STATE(us_filter), pent->p_proto);

	__kernel_filter_start();

	nfct_filter_add_attr_u32(STATE(filter),
				 NFCT_FILTER_L4PROTO,
				 pent->p_proto);
}
#line 3125 "read_config_yy.c"
    break;

  case 188: /* filter_protocol_item: T_UDP  */
#line 1198 "read_config_yy.y"
{
	struct protoent *pent;

	pent = getprotobyname("udp");
	if (pent == NULL) {
		dlog(LOG_WARNING, "getprotobyname() cannot find "
					"protocol `udp' in /etc/protocols");
		break;
	}
	ct_filter_add_proto(STATE(us_filter), pent->p_proto);

	__kernel_filter_start();

	nfct_filter_add_attr_u32(STATE(filter),
				 NFCT_FILTER_L4PROTO,
				 pent->p_proto);
}
#line 3147 "read_config_yy.c"
    break;

  case 189: /* filter_item: T_ADDRESS T_ACCEPT '{' filter_address_list '}'  */
#line 1217 "read_config_yy.y"
{
	ct_filter_set_logic(STATE(us_filter),
			    CT_FILTER_ADDRESS,
			    CT_FILTER_POSITIVE);

	__kernel_filter_start();
}
#line 3159 "read_config_yy.c"
    break;

  case 190: /* filter_item: T_ADDRESS T_IGNORE '{' filter_address_list '}'  */
#line 1226 "read_config_yy.y"
{
	ct_filter_set_logic(STATE(us_filter),
			    CT_FILTER_ADDRESS,
			    CT_FILTER_NEGATIVE);

	__kernel_filter_start();

	nfct_filter_set_logic(STATE(filter),
			      NFCT_FILTER_SRC_IPV4,
			      NFCT_FILTER_LOGIC_NEGATIVE);
	nfct_filter_set_logic(STATE(filter),
			      NFCT_FILTER_DST_IPV4,
			      NFCT_FILTER_LOGIC_NEGATIVE);
	nfct_filter_set_logic(STATE(filter),
			      NFCT_FILTER_SRC_IPV6,
			      NFCT_FILTER_LOGIC_NEGATIVE);
	nfct_filter_set_logic(STATE(filter),
			      NFCT_FILTER_DST_IPV6,
			      NFCT_FILTER_LOGIC_NEGATIVE);
}
#line 3184 "read_config_yy.c"
    break;

  case 193: /* filter_address_item: T_IPV4_ADDR T_IP  */
#line 1251 "read_config_yy.y"
{
	union inet_address ip;
	char *slash;
	unsigned int cidr = 32;

	memset(&ip, 0, sizeof(union inet_address));

	slash = strchr((yyvsp[0].string), '/');
	if (slash) {
		*slash = '\0';
		cidr = atoi(slash+1);
		if (cidr > 32) {
			dlog(LOG_WARNING, "%s/%d is not a valid network, "
			     "ignoring", (yyvsp[0].string), cidr);
			free((yyvsp[0].string));
			break;
		}
	}

	if (!inet_aton((yyvsp[0].string), (struct in_addr *) &ip.ipv4)) {
		dlog(LOG_WARNING, "%s is not a valid IPv4, ignoring", (yyvsp[0].string));
		free((yyvsp[0].string));
		break;
	}

	if (slash && cidr < 32) {
		/* network byte order */
		struct ct_filter_netmask_ipv4 tmp = {
			.ip = ip.ipv4,
			.mask = ipv4_cidr2mask_net(cidr)
		};

		if (!ct_filter_add_netmask(STATE(us_filter), &tmp, AF_INET)) {
			if (errno == EEXIST)
				dlog(LOG_WARNING, "netmask %s is "
				     "repeated in the ignore pool", (yyvsp[0].string));
		}
	} else {
		if (!ct_filter_add_ip(STATE(us_filter), &ip, AF_INET)) {
			if (errno == EEXIST)
				dlog(LOG_WARNING, "IP %s is repeated in "
				     "the ignore pool", (yyvsp[0].string));
			if (errno == ENOSPC)
				dlog(LOG_WARNING, "too many IP in the "
				     "ignore pool!");
		}
	}
	free((yyvsp[0].string));
	__kernel_filter_start();

	/* host byte order */
	struct nfct_filter_ipv4 filter_ipv4 = {
		.addr = ntohl(ip.ipv4),
		.mask = ipv4_cidr2mask_host(cidr),
	};

	nfct_filter_add_attr(STATE(filter), NFCT_FILTER_SRC_IPV4, &filter_ipv4);
	nfct_filter_add_attr(STATE(filter), NFCT_FILTER_DST_IPV4, &filter_ipv4);
}
#line 3248 "read_config_yy.c"
    break;

  case 194: /* filter_address_item: T_IPV6_ADDR T_IP  */
#line 1312 "read_config_yy.y"
{
	union inet_address ip;
	char *slash;
	int cidr = 128;
	struct nfct_filter_ipv6 filter_ipv6;
	int err;

	memset(&ip, 0, sizeof(union inet_address));

	slash = strchr((yyvsp[0].string), '/');
	if (slash) {
		*slash = '\0';
		cidr = atoi(slash+1);
		if (cidr > 128) {
			dlog(LOG_WARNING, "%s/%d is not a valid network, "
			     "ignoring", (yyvsp[0].string), cidr);
			free((yyvsp[0].string));
			break;
		}
	}

	err = inet_pton(AF_INET6, (yyvsp[0].string), &ip.ipv6);
	if (err == 0) {
		dlog(LOG_WARNING, "%s is not a valid IPv6, ignoring", (yyvsp[0].string));
		free((yyvsp[0].string));
		break;
	} else if (err < 0) {
		dlog(LOG_ERR, "inet_pton(): IPv6 unsupported!");
		exit(EXIT_FAILURE);
	}

	if (slash && cidr < 128) {
		struct ct_filter_netmask_ipv6 tmp;

		memcpy(tmp.ip, ip.ipv6, sizeof(uint32_t)*4);
		ipv6_cidr2mask_net(cidr, tmp.mask);
		if (!ct_filter_add_netmask(STATE(us_filter), &tmp, AF_INET6)) {
			if (errno == EEXIST)
				dlog(LOG_WARNING, "netmask %s is "
				     "repeated in the ignore pool", (yyvsp[0].string));
		}
	} else {
		if (!ct_filter_add_ip(STATE(us_filter), &ip, AF_INET6)) {
			if (errno == EEXIST)
				dlog(LOG_WARNING, "IP %s is repeated in "
				     "the ignore pool", (yyvsp[0].string));
			if (errno == ENOSPC)
				dlog(LOG_WARNING, "too many IP in the "
				     "ignore pool!");
		}
	}
	free((yyvsp[0].string));
	__kernel_filter_start();

	/* host byte order */
	ipv6_addr2addr_host(ip.ipv6, filter_ipv6.addr);
	ipv6_cidr2mask_host(cidr, filter_ipv6.mask);

	nfct_filter_add_attr(STATE(filter), NFCT_FILTER_SRC_IPV6, &filter_ipv6);
	nfct_filter_add_attr(STATE(filter), NFCT_FILTER_DST_IPV6, &filter_ipv6);
}
#line 3314 "read_config_yy.c"
    break;

  case 195: /* filter_item: T_STATE T_ACCEPT '{' filter_state_list '}'  */
#line 1375 "read_config_yy.y"
{
	ct_filter_set_logic(STATE(us_filter),
			    CT_FILTER_STATE,
			    CT_FILTER_POSITIVE);

	__kernel_filter_start();
}
#line 3326 "read_config_yy.c"
    break;

  case 196: /* filter_item: T_STATE T_IGNORE '{' filter_state_list '}'  */
#line 1384 "read_config_yy.y"
{
	ct_filter_set_logic(STATE(us_filter),
			    CT_FILTER_STATE,
			    CT_FILTER_NEGATIVE);


	__kernel_filter_start();

	nfct_filter_set_logic(STATE(filter),
			      NFCT_FILTER_L4PROTO_STATE,
			      NFCT_FILTER_LOGIC_NEGATIVE);
}
#line 3343 "read_config_yy.c"
    break;

  case 200: /* stats: T_STATS '{' stats_list '}'  */
#line 1403 "read_config_yy.y"
{
	if (conf.flags & CTD_SYNC_MODE) {
		dlog(LOG_ERR, "cannot use both `Stats' and `Sync' "
		     "clauses in conntrackd.conf");
		exit(EXIT_FAILURE);
	}
	conf.flags |= CTD_STATS_MODE;
}
#line 3356 "read_config_yy.c"
    break;

  case 207: /* stat_logfile_bool: T_LOG T_ON  */
#line 1423 "read_config_yy.y"
{
	strncpy(conf.stats.logfile, DEFAULT_STATS_LOGFILE, FILENAME_MAXLEN);
}
#line 3364 "read_config_yy.c"
    break;

  case 208: /* stat_logfile_bool: T_LOG T_OFF  */
#line 1428 "read_config_yy.y"
{
}
#line 3371 "read_config_yy.c"
    break;

  case 209: /* stat_logfile_path: T_LOG T_PATH_VAL  */
#line 1432 "read_config_yy.y"
{
	if (strlen((yyvsp[0].string)) > FILENAME_MAXLEN) {
		dlog(LOG_ERR, "stats LogFile path is longer than %u characters",
		     FILENAME_MAXLEN);
		exit(EXIT_FAILURE);
	}
	snprintf(conf.stats.logfile, sizeof(conf.stats.logfile), "%s", (yyvsp[0].string));
	free((yyvsp[0].string));
}
#line 3385 "read_config_yy.c"
    break;

  case 210: /* stat_syslog_bool: T_SYSLOG T_ON  */
#line 1443 "read_config_yy.y"
{
	conf.stats.syslog_facility = DEFAULT_SYSLOG_FACILITY;
}
#line 3393 "read_config_yy.c"
    break;

  case 211: /* stat_syslog_bool: T_SYSLOG T_OFF  */
#line 1448 "read_config_yy.y"
{
	conf.stats.syslog_facility = -1;
}
#line 3401 "read_config_yy.c"
    break;

  case 212: /* stat_syslog_facility: T_SYSLOG T_STRING  */
#line 1453 "read_config_yy.y"
{
	if (!strcmp((yyvsp[0].string), "daemon"))
		conf.stats.syslog_facility = LOG_DAEMON;
	else if (!strcmp((yyvsp[0].string), "local0"))
		conf.stats.syslog_facility = LOG_LOCAL0;
	else if (!strcmp((yyvsp[0].string), "local1"))
		conf.stats.syslog_facility = LOG_LOCAL1;
	else if (!strcmp((yyvsp[0].string), "local2"))
		conf.stats.syslog_facility = LOG_LOCAL2;
	else if (!strcmp((yyvsp[0].string), "local3"))
		conf.stats.syslog_facility = LOG_LOCAL3;
	else if (!strcmp((yyvsp[0].string), "local4"))
		conf.stats.syslog_facility = LOG_LOCAL4;
	else if (!strcmp((yyvsp[0].string), "local5"))
		conf.stats.syslog_facility = LOG_LOCAL5;
	else if (!strcmp((yyvsp[0].string), "local6"))
		conf.stats.syslog_facility = LOG_LOCAL6;
	else if (!strcmp((yyvsp[0].string), "local7"))
		conf.stats.syslog_facility = LOG_LOCAL7;
	else {
		dlog(LOG_WARNING, "'%s' is not a known syslog facility, "
		     "ignoring.", (yyvsp[0].string));
		free((yyvsp[0].string));
		break;
	}
	free((yyvsp[0].string));

	if (conf.syslog_facility != -1 &&
	    conf.stats.syslog_facility != conf.syslog_facility)
		dlog(LOG_WARNING, "conflicting Syslog facility "
		     "values, defaulting to General");
}
#line 3438 "read_config_yy.c"
    break;

  case 213: /* helper: T_HELPER '{' helper_list '}'  */
#line 1487 "read_config_yy.y"
{
	conf.flags |= CTD_HELPER;
}
#line 3446 "read_config_yy.c"
    break;

  case 218: /* helper_type: T_TYPE T_STRING T_STRING T_STRING '{' helper_type_list '}'  */
#line 1500 "read_config_yy.y"
{
	struct ctd_helper_instance *helper_inst;
	struct ctd_helper *helper;
	struct stack_item *e;
	uint16_t l3proto;
	uint8_t l4proto;

	if (strcmp((yyvsp[-4].string), "inet") == 0)
		l3proto = AF_INET;
	else if (strcmp((yyvsp[-4].string), "inet6") == 0)
		l3proto = AF_INET6;
	else {
		dlog(LOG_ERR, "unknown layer 3 protocol");
		free((yyvsp[-4].string));
		exit(EXIT_FAILURE);
	}
	free((yyvsp[-4].string));

	if (strcmp((yyvsp[-3].string), "tcp") == 0)
		l4proto = IPPROTO_TCP;
	else if (strcmp((yyvsp[-3].string), "udp") == 0)
		l4proto = IPPROTO_UDP;
	else {
		dlog(LOG_ERR, "unknown layer 4 protocol");
		free((yyvsp[-3].string));
		exit(EXIT_FAILURE);
	}
	free((yyvsp[-3].string));

#ifdef BUILD_CTHELPER
	helper = helper_find(CONNTRACKD_LIB_DIR, (yyvsp[-5].string), l4proto, RTLD_NOW);
	if (helper == NULL) {
		dlog(LOG_ERR, "Unknown `%s' helper", (yyvsp[-5].string));
		free((yyvsp[-5].string));
		exit(EXIT_FAILURE);
	}
#else
	dlog(LOG_ERR, "Helper support is disabled, recompile conntrackd");
	exit(EXIT_FAILURE);
#endif
	free((yyvsp[-5].string));

	helper_inst = calloc(1, sizeof(struct ctd_helper_instance));
	if (helper_inst == NULL)
		break;

	helper_inst->l3proto = l3proto;
	helper_inst->l4proto = l4proto;
	helper_inst->helper = helper;

	while ((e = stack_item_pop(&symbol_stack, -1)) != NULL) {

		switch(e->type) {
		case SYMBOL_HELPER_QUEUE_NUM: {
			int *qnum = (int *) &e->data;

			helper_inst->queue_num = *qnum;
			stack_item_free(e);
			break;
		}
		case SYMBOL_HELPER_QUEUE_LEN: {
			int *qlen = (int *) &e->data;

			helper_inst->queue_len = *qlen;
			stack_item_free(e);
			break;
		}
		case SYMBOL_HELPER_POLICY_EXPECT_ROOT: {
			struct ctd_helper_policy *pol =
				(struct ctd_helper_policy *) &e->data;
			struct ctd_helper_policy *matching = NULL;
			int i;

			for (i=0; i<CTD_HELPER_POLICY_MAX; i++) {
				if (strcmp(helper->policy[i].name,
					   pol->name) != 0)
					continue;

				matching = pol;
				break;
			}
			if (matching == NULL) {
				dlog(LOG_ERR, "Unknown policy `%s' in helper "
				     "configuration", pol->name);
				exit(EXIT_FAILURE);
			}
			/* FIXME: First set default policy, then change only
			 * tuned fields, not everything.
			 */
			memcpy(&helper->policy[i], pol,
				sizeof(struct ctd_helper_policy));

			stack_item_free(e);
			break;
		}
		default:
			dlog(LOG_ERR, "Unexpected symbol parsing helper "
			     "policy");
			exit(EXIT_FAILURE);
			break;
		}
	}
	list_add(&helper_inst->head, &CONFIG(cthelper).list);
}
#line 3555 "read_config_yy.c"
    break;

  case 219: /* helper_setup: T_SETUP T_ON  */
#line 1606 "read_config_yy.y"
{
	CONFIG(cthelper).setup = true;
}
#line 3563 "read_config_yy.c"
    break;

  case 220: /* helper_setup: T_SETUP T_OFF  */
#line 1611 "read_config_yy.y"
{
	CONFIG(cthelper).setup = false;
}
#line 3571 "read_config_yy.c"
    break;

  case 224: /* helper_type: T_HELPER_QUEUE_NUM T_NUMBER  */
#line 1623 "read_config_yy.y"
{
	int *qnum;
	struct stack_item *e;

	e = stack_item_alloc(SYMBOL_HELPER_QUEUE_NUM, sizeof(int));
	qnum = (int *) e->data;
	*qnum = (yyvsp[0].val);
	stack_item_push(&symbol_stack, e);
}
#line 3585 "read_config_yy.c"
    break;

  case 225: /* helper_type: T_HELPER_QUEUE_LEN T_NUMBER  */
#line 1634 "read_config_yy.y"
{
	int *qlen;
	struct stack_item *e;

	e = stack_item_alloc(SYMBOL_HELPER_QUEUE_LEN, sizeof(int));
	qlen = (int *) e->data;
	*qlen = (yyvsp[0].val);
	stack_item_push(&symbol_stack, e);
}
#line 3599 "read_config_yy.c"
    break;

  case 226: /* helper_type: T_HELPER_POLICY T_STRING '{' helper_policy_list '}'  */
#line 1645 "read_config_yy.y"
{
	struct stack_item *e;
	struct ctd_helper_policy *policy;

	e = stack_item_pop(&symbol_stack, SYMBOL_HELPER_EXPECT_POLICY_LEAF);
	if (e == NULL) {
		dlog(LOG_ERR, "Helper policy configuration empty, fix your "
		     "configuration file, please");
		free((yyvsp[-3].string));
		exit(EXIT_FAILURE);
		break;
	}
	if (strlen((yyvsp[-3].string)) > CTD_HELPER_NAME_LEN) {
		dlog(LOG_ERR, "Helper Policy is longer than %u characters",
		     CTD_HELPER_NAME_LEN);
		exit(EXIT_FAILURE);
	}

	policy = (struct ctd_helper_policy *) &e->data;
	snprintf(policy->name, sizeof(policy->name), "%s", (yyvsp[-3].string));
	free((yyvsp[-3].string));
	/* Now object is complete. */
	e->type = SYMBOL_HELPER_POLICY_EXPECT_ROOT;
	stack_item_push(&symbol_stack, e);
}
#line 3629 "read_config_yy.c"
    break;

  case 231: /* helper_policy_expect_max: T_HELPER_EXPECT_MAX T_NUMBER  */
#line 1680 "read_config_yy.y"
{
	struct stack_item *e;
	struct ctd_helper_policy *policy;

	e = stack_item_pop(&symbol_stack, SYMBOL_HELPER_EXPECT_POLICY_LEAF);
	if (e == NULL) {
		e = stack_item_alloc(SYMBOL_HELPER_EXPECT_POLICY_LEAF,
				     sizeof(struct ctd_helper_policy));
	}
	policy = (struct ctd_helper_policy *) &e->data;
	policy->expect_max = (yyvsp[0].val);
	stack_item_push(&symbol_stack, e);
}
#line 3647 "read_config_yy.c"
    break;

  case 232: /* helper_policy_expect_timeout: T_HELPER_EXPECT_TIMEOUT T_NUMBER  */
#line 1695 "read_config_yy.y"
{
	struct stack_item *e;
	struct ctd_helper_policy *policy;

	e = stack_item_pop(&symbol_stack, SYMBOL_HELPER_EXPECT_POLICY_LEAF);
	if (e == NULL) {
		e = stack_item_alloc(SYMBOL_HELPER_EXPECT_POLICY_LEAF,
				     sizeof(struct ctd_helper_policy));
	}
	policy = (struct ctd_helper_policy *) &e->data;
	policy->expect_timeout = (yyvsp[0].val);
	stack_item_push(&symbol_stack, e);
}
#line 3665 "read_config_yy.c"
    break;


#line 3669 "read_config_yy.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 1709 "read_config_yy.y"


int __attribute__((noreturn))
yyerror(const char *msg)
{
	dlog(LOG_ERR, "parsing config file in line (%d), symbol '%s': %s",
	     yylineno, yytext, msg);
	exit(EXIT_FAILURE);
}

static void __kernel_filter_start(void)
{
	if (!STATE(filter)) {
		STATE(filter) = nfct_filter_create();
		if (!STATE(filter)) {
			dlog(LOG_ERR, "cannot create ignore pool!");
			exit(EXIT_FAILURE);
		}
	}
}

static void __kernel_filter_add_state(int value)
{
	__kernel_filter_start();

	struct nfct_filter_proto filter_proto = {
		.proto = IPPROTO_TCP,
		.state = value
	};
	nfct_filter_add_attr(STATE(filter),
			     NFCT_FILTER_L4PROTO_STATE,
			     &filter_proto);
}

int
init_config(char *filename)
{
	FILE *fp;

	fp = fopen(filename, "r");
	if (!fp)
		return -1;

	/* Zero may be a valid facility */
	CONFIG(syslog_facility) = -1;
	CONFIG(stats).syslog_facility = -1;
	CONFIG(netlink).subsys_id = -1;

#ifdef BUILD_SYSTEMD
        CONFIG(systemd) = 1;
#endif /* BUILD_SYSTEMD */

	/* Initialize list of user-space helpers */
	INIT_LIST_HEAD(&CONFIG(cthelper).list);

	stack_init(&symbol_stack);

	yyrestart(fp);
	yyparse();
	fclose(fp);

#ifndef BUILD_SYSTEMD
	if (CONFIG(systemd) == 1) {
		dlog(LOG_WARNING, "systemd runtime support activated but "
		     "conntrackd was built without support "
		     "for it. Recompile conntrackd");
	}
#endif /* BUILD_SYSTEMD */

	/* set to default is not specified */
	if (strcmp(CONFIG(lockfile), "") == 0)
		strncpy(CONFIG(lockfile), DEFAULT_LOCKFILE, FILENAME_MAXLEN);

	/* default to 180 seconds of expiration time: cache entries */
	if (CONFIG(cache_timeout) == 0)
		CONFIG(cache_timeout) = 180;

	/* default to 60 seconds: purge kernel entries */
	if (CONFIG(purge_timeout) == 0)
		CONFIG(purge_timeout) = 60;

	/* default to 60 seconds of refresh time */
	if (CONFIG(refresh) == 0)
		CONFIG(refresh) = 60;

	if (CONFIG(resend_queue_size) == 0)
		CONFIG(resend_queue_size) = 131072;

	/* default to a window size of 300 packets */
	if (CONFIG(window_size) == 0)
		CONFIG(window_size) = 300;

	if (CONFIG(event_iterations_limit) == 0)
		CONFIG(event_iterations_limit) = 100;

	/* default number of bucket of the hashtable that are committed in
	   one run loop. XXX: no option available to tune this value yet. */
	if (CONFIG(general).commit_steps == 0)
		CONFIG(general).commit_steps = 8192;

	/* if overrun, automatically resync with kernel after 30 seconds */
	if (CONFIG(nl_overrun_resync) == 0)
		CONFIG(nl_overrun_resync) = 30;

	/* default to 128 elements in the channel error queue */
	if (CONFIG(channelc).error_queue_length == 0)
		CONFIG(channelc).error_queue_length = 128;

	if (CONFIG(netlink).subsys_id == -1) {
		CONFIG(netlink).subsys_id = NFNL_SUBSYS_CTNETLINK;
		CONFIG(netlink).groups = NF_NETLINK_CONNTRACK_NEW |
					 NF_NETLINK_CONNTRACK_UPDATE |
					 NF_NETLINK_CONNTRACK_DESTROY;
	}

	/* default hashtable buckets and maximum number of entries */
	if (!CONFIG(hashsize))
		CONFIG(hashsize) = 65536;
	if (!CONFIG(limit))
		CONFIG(limit) = 262144;

	return 0;
}
