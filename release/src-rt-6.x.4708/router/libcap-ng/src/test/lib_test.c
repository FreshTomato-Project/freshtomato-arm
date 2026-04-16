/* lib_test.c -- simple libcap-ng test suite
 * Copyright 2009,2012-13 Red Hat Inc.
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
 * along with this program; see the file COPYING.LIB. If not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor
 * Boston, MA 02110-1335, USA.
 *
 * Authors:
 *      Steve Grubb <sgrubb@redhat.com>
 */

#include "config.h"
#include "../cap-ng.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdint.h>
#include <unistd.h>

struct proc_caps {
	uint64_t effective;
	uint64_t permitted;
	uint64_t inheritable;
};

static unsigned int get_last_cap(void)
{
	int fd;

	fd = open("/proc/sys/kernel/cap_last_cap", O_RDONLY);
	if (fd == -1) {
		return CAP_LAST_CAP;
	} else {
		char buf[8];
		int num = read(fd, buf, sizeof(buf));
		if (num > 0) {
			errno = 0;
			unsigned int val = strtoul(buf, NULL, 10);
			if (errno == 0)
				return val;
		}
		close(fd);
	}
	return CAP_LAST_CAP;
}

static int read_process_caps(struct proc_caps *caps)
{
	FILE *f;
	char buf[128];
	unsigned long long val;
	int found = 0;

	memset(caps, 0, sizeof(*caps));
	f = fopen("/proc/self/status", "re");
	if (f == NULL)
		return -1;

	while (fgets(buf, sizeof(buf), f)) {
		if (sscanf(buf, "CapEff:\t%llx", &val) == 1) {
			caps->effective = val;
			found |= 1;
		} else if (sscanf(buf, "CapPrm:\t%llx", &val) == 1) {
			caps->permitted = val;
			found |= 2;
		} else if (sscanf(buf, "CapInh:\t%llx", &val) == 1) {
			caps->inheritable = val;
			found |= 4;
		}
	}
	fclose(f);

	return found == 7 ? 0 : -1;
}

static int expected_cap(uint64_t set, unsigned int capability)
{
	return (set & (1ULL << capability)) ? 1 : 0;
}

static capng_results_t expected_caps_result(uint64_t set, unsigned int last)
{
	unsigned int i;
	int found = 0, missing = 0;

	for (i = 0; i <= last; i++) {
		if (expected_cap(set, i))
			found = 1;
		else
			missing = 1;
	}
	if (!found)
		return CAPNG_NONE;
	if (!missing)
		return CAPNG_FULL;
	return CAPNG_PARTIAL;
}

int main(void)
{
	int rc;
	unsigned int i, len, last = get_last_cap();
	struct proc_caps caps;
	char expected[128];
	char *text;
	const char *name;
	void *saved;

	puts("Doing process capability tests...");
	if (read_process_caps(&caps)) {
		puts("Failed reading process capabilities from procfs");
		abort();
	}
	if (capng_get_caps_process()) {
		puts("Failed getting process capabilities");
		abort();
	}

	for (i = 0; i <= last; i++) {
		if (capng_have_capability(CAPNG_PERMITTED, i) !=
				expected_cap(caps.permitted, i)) {
			puts("Failed process permitted capabilities test");
			abort();
		}
		if (capng_have_capability(CAPNG_EFFECTIVE, i) !=
				expected_cap(caps.effective, i)) {
			puts("Failed process effective capabilities test");
			abort();
		}
		if (capng_have_capability(CAPNG_INHERITABLE, i) !=
				expected_cap(caps.inheritable, i)) {
			puts("Failed process inheritable capabilities test");
			abort();
		}
	}
	if (capng_have_permitted_capabilities() !=
			expected_caps_result(caps.permitted, last)) {
		puts("Failed process permitted capabilities aggregate test");
		abort();
	}
	if (capng_have_capabilities(CAPNG_SELECT_CAPS) !=
			expected_caps_result(caps.effective, last)) {
		puts("Failed process effective capabilities aggregate test");
		abort();
	}
	text = capng_print_caps_numeric(CAPNG_PRINT_BUFFER, CAPNG_SELECT_CAPS);
	if (text == NULL) {
		puts("Failed getting process numeric capabilities");
		abort();
	}
	snprintf(expected, sizeof(expected),
		"Effective:   %08X, %08X\n"
		"Permitted:   %08X, %08X\n"
		"Inheritable: %08X, %08X\n",
		(unsigned int)(caps.effective >> 32),
		(unsigned int)(caps.effective & 0xFFFFFFFFU),
		(unsigned int)(caps.permitted >> 32),
		(unsigned int)(caps.permitted & 0xFFFFFFFFU),
		(unsigned int)(caps.inheritable >> 32),
		(unsigned int)(caps.inheritable & 0xFFFFFFFFU));
	if (strcmp(text, expected)) {
		snprintf(expected, sizeof(expected),
			"Effective:   %08X\n"
			"Permitted:   %08X\n"
			"Inheritable: %08X\n",
			(unsigned int)caps.effective,
			(unsigned int)caps.permitted,
			(unsigned int)caps.inheritable);
		if (strcmp(text, expected)) {
			puts("Failed process numeric capabilities test");
			free(text);
			abort();
		}
	}
	free(text);

	puts("Doing basic bit tests...");
	capng_clear(CAPNG_SELECT_BOTH);
	errno = 0;
	rc = capng_apply(0);
	if (rc != -1 || errno != EINVAL) {
		puts("Failed apply empty selection test");
		abort();
	}
	if (capng_have_permitted_capabilities() != CAPNG_NONE) {
		puts("Failed permitted capabilities none test");
		abort();
	}
	saved = capng_save_state();
	capng_fill(CAPNG_SELECT_BOTH);
	if (capng_have_permitted_capabilities() != CAPNG_FULL) {
		puts("Failed permitted capabilities full test");
		abort();
	}
	capng_restore_state(&saved);
	capng_clear(CAPNG_SELECT_BOTH);
	rc = capng_update(CAPNG_ADD, CAPNG_PERMITTED, CAP_CHOWN);
	if (rc) {
		puts("Failed update permitted test");
		abort();
	}
	if (capng_have_permitted_capabilities() != CAPNG_PARTIAL) {
		puts("Failed permitted capabilities partial test");
		abort();
	}
	if (capng_have_capabilities(CAPNG_SELECT_BOTH) != CAPNG_NONE) {
		puts("Failed clearing capabilities");
		abort();
	}
	saved = capng_save_state();
	capng_fill(CAPNG_SELECT_BOTH);
	if (capng_have_capabilities(CAPNG_SELECT_BOTH) != CAPNG_FULL) {
		puts("Failed filling capabilities");
		abort();
	}
	// Need to detect if version 1 or 2 capabilities
	text = capng_print_caps_numeric(CAPNG_PRINT_BUFFER, CAPNG_SELECT_CAPS);
	len = strlen(text);
	free(text);
	if (len < 80 && last > 30)	// The kernel & headers are mismatched
		last = 30;
	// Now test that restore still works
	capng_restore_state(&saved);
	if (capng_have_capabilities(CAPNG_SELECT_BOTH) != CAPNG_NONE) {
		puts("Failed restoring capabilities");
		abort();
	}
	printf("Doing advanced bit tests for %d capabilities...\n", last);
	for (i=0; i<=last; i++) {
		capng_clear(CAPNG_SELECT_BOTH);
		rc = capng_update(CAPNG_ADD, CAPNG_EFFECTIVE, i);
		if (rc) {
			puts("Failed update test 1");
			abort();
		}
		rc = capng_have_capability(CAPNG_EFFECTIVE, i);
		if (rc == 0) {
			puts("Failed have capability test 1");
			capng_print_caps_numeric(CAPNG_PRINT_STDOUT,
					CAPNG_SELECT_CAPS);
			abort();
		}
		if(capng_have_capabilities(CAPNG_SELECT_CAPS)!=CAPNG_PARTIAL){
			puts("Failed have capabilities test 1");
			capng_print_caps_numeric(CAPNG_PRINT_STDOUT,
					CAPNG_SELECT_CAPS);
			abort();
		}
#if CAP_LAST_CAP > 31
		rc = capng_update(CAPNG_ADD, CAPNG_BOUNDING_SET, i);
		if (rc) {
			puts("Failed bset update test 2");
			abort();
		}
		rc = capng_have_capability(CAPNG_BOUNDING_SET, i);
		if (rc == 0) {
			puts("Failed bset have capability test 2");
			capng_print_caps_numeric(CAPNG_PRINT_STDOUT,
					CAPNG_SELECT_BOTH);
			abort();
		}
		if(capng_have_capabilities(CAPNG_SELECT_BOUNDS)!=CAPNG_PARTIAL){
			puts("Failed bset have capabilities test 2");
			capng_print_caps_numeric(CAPNG_PRINT_STDOUT,
					CAPNG_SELECT_BOTH);
			abort();
		}
#endif
		text=capng_print_caps_text(CAPNG_PRINT_BUFFER, CAPNG_EFFECTIVE);
		if (text == NULL) {
			puts("Failed getting print text to buffer");
			abort();
		}
		name = capng_capability_to_name(i);
		if (name == NULL) {
			printf("Failed converting capability %d to name\n", i);
			abort();
		}
		if (strcmp(text, name)) {
			puts("Failed print text comparison");
			printf("%s != %s\n", text, name);
			abort();
		}
		free(text);
		// Now make sure the mask part is working
		capng_fill(CAPNG_SELECT_BOTH);
		rc = capng_update(CAPNG_DROP, CAPNG_EFFECTIVE, i);
		if (rc) {
			puts("Failed update test 3");
			abort();
		}
		// Should be partial
		if(capng_have_capabilities(CAPNG_SELECT_CAPS)!=CAPNG_PARTIAL){
			puts("Failed have capabilities test 3");
			capng_print_caps_numeric(CAPNG_PRINT_STDOUT,
					CAPNG_SELECT_CAPS);
			abort();
		}
		// Add back the bit and should be full capabilities
		rc = capng_update(CAPNG_ADD, CAPNG_EFFECTIVE, i);
		if (rc) {
			puts("Failed update test 4");
			abort();
		}
		if (capng_have_capabilities(CAPNG_SELECT_CAPS) != CAPNG_FULL){
			puts("Failed have capabilities test 4");
			capng_print_caps_numeric(CAPNG_PRINT_STDOUT,
					CAPNG_SELECT_CAPS);
			abort();
		}
	}
	// Verify text formatting when many capabilities are present
	capng_clear(CAPNG_SELECT_BOTH);
	for (i=0; i<=last; i++) {
		rc = capng_update(CAPNG_ADD, CAPNG_EFFECTIVE, i);
		if (rc) {
			puts("Failed setup test for print buffer length");
			abort();
		}
	}
	text = capng_print_caps_text(CAPNG_PRINT_BUFFER, CAPNG_EFFECTIVE);
	if (text == NULL) {
		puts("Failed getting full print text to buffer");
		abort();
	}
	len = 0;
	for (i=0; i<=last; i++) {
		if (capng_have_capability(CAPNG_EFFECTIVE, i)) {
			name = capng_capability_to_name(i);
			if (name == NULL)
				name = "unknown";
			if (len)
				len += 2;
			len += strlen(name);
		}
	}
	if (strlen(text) != len) {
		puts("Failed print text length comparison");
		printf("%zu != %u\n", strlen(text), len);
		abort();
	}
	free(text);

	// Now test the updatev function
	capng_clear(CAPNG_SELECT_BOTH);
	rc = capng_updatev(CAPNG_ADD, CAPNG_EFFECTIVE,
			CAP_CHOWN, CAP_FOWNER, CAP_KILL, -1);
	if (rc) {
		puts("Failed updatev test");
		abort();
	}
	rc = capng_have_capability(CAPNG_EFFECTIVE, CAP_CHOWN) &&
		capng_have_capability(CAPNG_EFFECTIVE, CAP_FOWNER) &&
		capng_have_capability(CAPNG_EFFECTIVE, CAP_KILL);
	if (rc == 0) {
		puts("Failed have updatev capability test");
		capng_print_caps_numeric(CAPNG_PRINT_STDOUT,
				CAPNG_SELECT_CAPS);
		abort();
	}

	return EXIT_SUCCESS;
}
