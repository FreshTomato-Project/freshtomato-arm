/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

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

#line 258 "read_config_yy.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_READ_CONFIG_YY_H_INCLUDED  */
