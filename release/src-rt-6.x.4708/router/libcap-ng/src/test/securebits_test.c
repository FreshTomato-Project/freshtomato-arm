/* securebits_test.c -- capng_lock securebits tests
 * Copyright 2026 Red Hat Inc.
 * All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335, USA.
 */

#include "config.h"
#include "../cap-ng.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>

#ifdef HAVE_LINUX_SECUREBITS_H
#include <linux/securebits.h>
#endif

#ifndef SECURE_NOROOT
#define SECURE_NOROOT 0
#endif
#ifndef SECURE_NOROOT_LOCKED
#define SECURE_NOROOT_LOCKED 1
#endif
#ifndef SECURE_NO_SETUID_FIXUP
#define SECURE_NO_SETUID_FIXUP 2
#endif
#ifndef SECURE_NO_SETUID_FIXUP_LOCKED
#define SECURE_NO_SETUID_FIXUP_LOCKED 3
#endif

#if defined(PR_GET_SECUREBITS) && defined(PR_SET_SECUREBITS)

#define FUTURE_SECUREBIT (1U << 20)
#define LOCK_SECUREBITS (1U << SECURE_NOROOT | \
			 1U << SECURE_NOROOT_LOCKED | \
			 1U << SECURE_NO_SETUID_FIXUP | \
			 1U << SECURE_NO_SETUID_FIXUP_LOCKED)

static unsigned int current_securebits;
static unsigned long written_securebits;
static unsigned int get_count;
static unsigned int set_count;
static unsigned int no_new_privs_count;
static int fail_get;
static int fail_set;
static int fail_no_new_privs;

int prctl(int option, ...)
{
	va_list args;
	unsigned long arg2;
	int rc = 0;

	va_start(args, option);
	arg2 = va_arg(args, unsigned long);
	va_end(args);

	if (option == PR_GET_SECUREBITS) {
		get_count++;
		if (fail_get) {
			errno = EIO;
			return -1;
		}
		return (int)current_securebits;
	}
	if (option == PR_SET_SECUREBITS) {
		set_count++;
		written_securebits = arg2;
		if (fail_set) {
			errno = EPERM;
			return -1;
		}
		current_securebits = (unsigned int)arg2;
#ifdef PR_SET_NO_NEW_PRIVS
	} else if (option == PR_SET_NO_NEW_PRIVS) {
		no_new_privs_count++;
		if (fail_no_new_privs) {
			errno = EPERM;
			return -1;
		}
#endif
	} else {
		errno = EINVAL;
		rc = -1;
	}

	return rc;
}

static void reset_mock(unsigned int securebits)
{
	current_securebits = securebits;
	written_securebits = 0;
	get_count = 0;
	set_count = 0;
	no_new_privs_count = 0;
	fail_get = 0;
	fail_set = 0;
	fail_no_new_privs = 0;
}

static void check(int condition, const char *message)
{
	if (condition == 0) {
		fprintf(stderr, "%s\n", message);
		abort();
	}
}

int main(void)
{
	unsigned int expected = FUTURE_SECUREBIT | LOCK_SECUREBITS;

	reset_mock(FUTURE_SECUREBIT);
	check(capng_lock() == 0, "Failed locking securebits");
	check(get_count == 1, "Securebits were not read");
	check(set_count == 1, "Securebits were not written");
	check(written_securebits == expected,
	      "Existing securebits were not preserved");
#ifdef PR_SET_NO_NEW_PRIVS
	check(no_new_privs_count == 1, "No-new-privileges was not set");
#endif

	reset_mock(expected);
	check(capng_lock() == 0, "Idempotent lock failed");
	check(get_count == 1, "Idempotent lock did not read securebits");
	check(set_count == 0, "Idempotent lock rewrote securebits");

	reset_mock(0);
	fail_get = 1;
	check(capng_lock() == -1, "Securebits read failure was not reported");
	check(set_count == 0, "Securebits were written after a read failure");

	reset_mock(0);
	fail_set = 1;
	check(capng_lock() == -1, "Securebits write failure was not reported");

#ifdef PR_SET_NO_NEW_PRIVS
	reset_mock(expected);
	fail_no_new_privs = 1;
	check(capng_lock() == -2,
	      "No-new-privileges failure was not reported");

	reset_mock(0);
	fail_get = 1;
	fail_no_new_privs = 1;
	check(capng_lock() == -3, "Combined failures were not reported");
#endif

	return EXIT_SUCCESS;
}

#else

int main(void)
{
	return 77;
}

#endif
