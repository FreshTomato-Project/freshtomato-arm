/* Definitions for POSIX spawn interface.
   Copyright (C) 2000,2003,2004,2009,2011,2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef	_SPAWN_H
#define	_SPAWN_H	1

#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <features.h>
#include <sched.h>
#define __need_sigset_t
#include <signal.h>
#include <sys/types.h>


/* Data structure to contain attributes for thread creation.  */
typedef struct {
	short int __flags;
	pid_t __pgrp;
	sigset_t __sd;
	sigset_t __ss;
	struct sched_param __sp;
	int __policy;
	int __pad[16];
} posix_spawnattr_t;


/* Data structure to contain information about the actions to be
   performed in the new process with respect to file descriptors.  */
typedef struct {
	int __allocated;
	int __used;
	struct __spawn_action *__actions;
	int __pad[16];
} posix_spawn_file_actions_t;


/* Flags to be set in the `posix_spawnattr_t'.  */
#define POSIX_SPAWN_RESETIDS		0x01
#define POSIX_SPAWN_SETPGROUP		0x02
#define POSIX_SPAWN_SETSIGDEF		0x04
#define POSIX_SPAWN_SETSIGMASK		0x08
#define POSIX_SPAWN_SETSCHEDPARAM	0x10
#define POSIX_SPAWN_SETSCHEDULER	0x20
#define POSIX_SPAWN_USEVFORK		0x40 /* GNU extension */


#define __POSIX_SPAWN_MASK (POSIX_SPAWN_RESETIDS		\
			    | POSIX_SPAWN_SETPGROUP		\
			    | POSIX_SPAWN_SETSIGDEF		\
			    | POSIX_SPAWN_SETSIGMASK		\
			    | POSIX_SPAWN_SETSCHEDPARAM		\
			    | POSIX_SPAWN_SETSCHEDULER		\
			    | POSIX_SPAWN_USEVFORK)

__BEGIN_DECLS

/* Spawn a new process executing PATH with the attributes describes in *ATTRP.
   Before running the process perform the actions described in FILE-ACTIONS.

   This function is a possible cancellation point and therefore not
   marked with __THROW. */
int posix_spawn(pid_t * restrict pid, const char * restrict path,
		const posix_spawn_file_actions_t * restrict file_actions,
		const posix_spawnattr_t * restrict attrp,
		char * const argv[restrict],
		char * const envp[restrict]);

/* Similar to `posix_spawn' but search for FILE in the PATH.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
int posix_spawnp(pid_t *pid, const char *file,
		 const posix_spawn_file_actions_t *file_actions,
		 const posix_spawnattr_t *attrp,
		 char * const argv[], char * const envp[]);

/* Initialize data structure with attributes for `spawn' to default values. */
inline static int
posix_spawnattr_init(posix_spawnattr_t *attr)
{
	memset(attr, 0, sizeof(*attr));
	return 0;
}

/* Free resources associated with ATTR.  */
inline static int
posix_spawnattr_destroy(posix_spawnattr_t *attr)
{
	return 0;
}

/* Store signal mask for signals with default handling from ATTR in
   SIGDEFAULT.  */
inline static int
posix_spawnattr_getsigdefault(const posix_spawnattr_t *attr,
			      sigset_t *sigdefault)
{
	memcpy(sigdefault, &attr->__sd, sizeof(sigset_t));
	return 0;
}


/* Set signal mask for signals with default handling in ATTR to SIGDEFAULT.  */
inline static int
posix_spawnattr_setsigdefault(posix_spawnattr_t *attr,
			      const sigset_t *sigdefault)
{
	memcpy(&attr->__sd, sigdefault, sizeof(sigset_t));
	return 0;
}

/* Store signal mask for the new process from ATTR in SIGMASK.  */
inline static int
posix_spawnattr_getsigmask(const posix_spawnattr_t *attr,
			   sigset_t *sigmask)
{
	memcpy(sigmask, &attr->__ss, sizeof(sigset_t));
	return 0;
}


/* Set signal mask for the new process in ATTR to SIGMASK.  */
inline static int
posix_spawnattr_setsigmask(posix_spawnattr_t *attr,
			   const sigset_t *sigmask)
{
	memcpy(&attr->__ss, sigmask, sizeof(sigset_t));
	return 0;
}

/* Get flag word from the attribute structure.  */
inline static int
posix_spawnattr_getflags(const posix_spawnattr_t *attr, short int *flags)
{
	*flags = attr->__flags;
	return 0;
}

/* Store flags in the attribute structure.  */
inline static int
posix_spawnattr_setflags(posix_spawnattr_t *attr, short int flags)
{
	/* Check no invalid bits are set.  */
	if (flags & ~__POSIX_SPAWN_MASK)
		return EINVAL;

	attr->__flags = flags;
	return 0;
}

/* Get process group ID from the attribute structure.  */
inline static int
posix_spawnattr_getpgroup(const posix_spawnattr_t *attr, pid_t *pgroup)
{
	*pgroup = attr->__pgrp;
	return 0;
}

/* Store process group ID in the attribute structure.  */
inline static int
posix_spawnattr_setpgroup(posix_spawnattr_t *attr, pid_t pgroup)
{
	attr->__pgrp = pgroup;
	return 0;
}

/* Get scheduling policy from the attribute structure.  */
inline static int
posix_spawnattr_getschedpolicy(const posix_spawnattr_t *attr,
			       int *schedpolicy)
{
	*schedpolicy = attr->__policy;
	return 0;
}

/* Store scheduling policy in the attribute structure.  */
inline static int
posix_spawnattr_setschedpolicy(posix_spawnattr_t *attr, int schedpolicy)
{
	switch (schedpolicy) {
	case SCHED_OTHER:
	case SCHED_FIFO:
	case SCHED_RR:
		break;
	default:
		return EINVAL;
	}

	attr->__policy = schedpolicy;
	return 0;
}

/* Get scheduling parameters from the attribute structure.  */
static inline int
posix_spawnattr_getschedparam(const posix_spawnattr_t *attr,
			      struct sched_param *schedparam)
{
	memcpy(schedparam, &attr->__sp, sizeof(attr->__sp));
	return 0;
}

/* Store scheduling parameters in the attribute structure.  */
static inline int
posix_spawnattr_setschedparam(posix_spawnattr_t *attr,
			      const struct sched_param *schedparam)
{
	attr->__sp = *schedparam;
	return 0;
}

/* Initialize data structure for file attribute for `spawn' call.  */
inline static int
posix_spawn_file_actions_init(posix_spawn_file_actions_t *file_actions)
{
	memset(file_actions, 0, sizeof(*file_actions));
	return 0;
}

/* Free resources associated with FILE-ACTIONS.  */
inline static int
posix_spawn_file_actions_destroy(posix_spawn_file_actions_t *file_actions)
{
	free(file_actions->__actions);
	return 0;
}

/* Add an action to FILE-ACTIONS which tells the implementation to call
   `open' for the given file during the `spawn' call.  */
int posix_spawn_file_actions_addopen(posix_spawn_file_actions_t * restrict
				     file_actions, int fd,
				     const char * restrict path,
				     int oflag, mode_t mode)
     __THROW;

/* Add an action to FILE-ACTIONS which tells the implementation to call
   `close' for the given file descriptor during the `spawn' call.  */
int posix_spawn_file_actions_addclose(posix_spawn_file_actions_t *file_actions,
				      int fd)
     __THROW;

/* Add an action to FILE-ACTIONS which tells the implementation to call
   `dup2' for the given file descriptors during the `spawn' call.  */
int posix_spawn_file_actions_adddup2(posix_spawn_file_actions_t *file_actions,
				     int fd, int newfd) __THROW;

__END_DECLS

#endif /* spawn.h */
