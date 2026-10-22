/* file_caps_test.c -- filesystem capability loading tests
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
#include <byteswap.h>
#include <endian.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#ifdef HAVE_SYS_XATTR_H
# include <sys/xattr.h>
#elif defined(HAVE_ATTR_XATTR_H)
# include <attr/xattr.h>
#endif

#if defined(HAVE_SYS_XATTR_H) || defined(HAVE_ATTR_XATTR_H)

enum mock_revision {
	MOCK_REVISION_1,
	MOCK_REVISION_2,
#ifdef VFS_CAP_REVISION_3
	MOCK_REVISION_3,
#endif
};

static enum mock_revision mock_revision;
#ifdef VFS_CAP_REVISION_3
static struct vfs_ns_cap_data written_data;
#else
static struct vfs_cap_data written_data;
#endif
static size_t written_size;
static unsigned int write_count;

static __le32 cpu_to_le32(uint32_t value)
{
#if __BYTE_ORDER == __BIG_ENDIAN
	return bswap_32(value);
#else
	return value;
#endif
}

static uint32_t le32_to_cpu(__le32 value)
{
#if __BYTE_ORDER == __BIG_ENDIAN
	return bswap_32(value);
#else
	return value;
#endif
}

/*
 * Return only the selected ABI prefix. Poisoning the remainder makes any
 * access beyond the returned length deterministic, like a dirty stack slot.
 */
ssize_t fgetxattr(int fd, const char *name, void *value, size_t size)
{
#ifdef VFS_CAP_REVISION_3
	struct vfs_ns_cap_data data = { 0 };
#else
	struct vfs_cap_data data = { 0 };
#endif
	size_t xattr_size;

	(void)fd;
	if (strcmp(name, "security.capability") != 0) {
		errno = ENODATA;
		return -1;
	}

#ifdef VFS_CAP_REVISION_3
	if (mock_revision == MOCK_REVISION_3) {
		data.magic_etc = cpu_to_le32(VFS_CAP_REVISION_3 |
						VFS_CAP_FLAGS_EFFECTIVE);
		data.data[0].permitted = cpu_to_le32(1U << CAP_CHOWN);
		data.data[1].permitted = cpu_to_le32(1U);
		data.data[1].inheritable = cpu_to_le32(2U);
		data.rootid = cpu_to_le32(4242U);
		xattr_size = XATTR_CAPS_SZ_3;
	} else
#endif
	if (mock_revision == MOCK_REVISION_2) {
		data.magic_etc = cpu_to_le32(VFS_CAP_REVISION_2);
		data.data[0].permitted = cpu_to_le32(1U << CAP_CHOWN);
		data.data[0].inheritable = cpu_to_le32(1U << CAP_KILL);
		data.data[1].permitted = cpu_to_le32(4U);
		data.data[1].inheritable = cpu_to_le32(8U);
		xattr_size = XATTR_CAPS_SZ_2;
	} else {
		data.magic_etc = cpu_to_le32(VFS_CAP_REVISION_1 |
						VFS_CAP_FLAGS_EFFECTIVE);
		data.data[0].permitted = cpu_to_le32(1U << CAP_CHOWN);
		data.data[0].inheritable = cpu_to_le32(1U << CAP_KILL);
		xattr_size = XATTR_CAPS_SZ_1;
	}

	if (size < xattr_size) {
		errno = ERANGE;
		return -1;
	}
	memset(value, 0xFF, size);
	memcpy(value, &data, xattr_size);
	return xattr_size;
}

int fsetxattr(int fd, const char *name, const void *value, size_t size,
		int flags)
{
	(void)fd;
	(void)flags;
	if (strcmp(name, "security.capability") != 0 ||
						size > sizeof(written_data)) {
		errno = EINVAL;
		return -1;
	}

	memset(&written_data, 0, sizeof(written_data));
	memcpy(&written_data, value, size);
	written_size = size;
	write_count++;
	return 0;
}

static void check_numeric_caps(const char *expected)
{
	char *actual;

	actual = capng_print_caps_numeric(CAPNG_PRINT_BUFFER,
					CAPNG_SELECT_CAPS);
	if (actual == NULL || strcmp(actual, expected) != 0) {
		fprintf(stderr, "Unexpected capability state:\n%s",
			actual == NULL ? "(null)\n" : actual);
		free(actual);
		abort();
	}
	free(actual);
}

static void check_written_caps(int fd, uid_t rootid, uint32_t revision,
			       size_t expected_size)
{
	uint32_t magic;

	capng_clear(CAPNG_SELECT_CAPS);
	if (capng_update(CAPNG_ADD, CAPNG_EFFECTIVE | CAPNG_PERMITTED,
							CAP_CHOWN) != 0)
		abort();
	if (capng_set_rootid(rootid) != 0)
		abort();
	if (capng_get_rootid() != rootid)
		abort();

	written_size = 0;
	write_count = 0;
	if (capng_apply_caps_fd(fd) != 0)
		abort();
	if (write_count != 1 || written_size != expected_size)
		abort();

	magic = le32_to_cpu(written_data.magic_etc);
	if (magic != (revision | VFS_CAP_FLAGS_EFFECTIVE))
		abort();
	if (le32_to_cpu(written_data.data[0].permitted) !=
							(1U << CAP_CHOWN))
		abort();
	if (le32_to_cpu(written_data.data[0].inheritable) != 0)
		abort();
#ifdef VFS_CAP_REVISION_3
	if (revision == VFS_CAP_REVISION_3 &&
			le32_to_cpu(written_data.rootid) != (uint32_t)rootid)
		abort();
#endif
}

int main(void)
{
	FILE *file;
	int fd;

#ifdef VFS_CAP_REVISION_3
	mock_revision = MOCK_REVISION_3;
	if (capng_get_caps_fd(-1) != 0) {
		puts("Failed loading revision 3 file capabilities");
		abort();
	}
	check_numeric_caps("Effective:   00000003, 00000001\n"
			   "Permitted:   00000001, 00000001\n"
			   "Inheritable: 00000002, 00000000\n");
	if (capng_get_rootid() != 4242) {
		puts("Failed loading revision 3 rootid");
		abort();
	}
#endif

	mock_revision = MOCK_REVISION_1;
	if (capng_get_caps_fd(-1) != 0) {
		puts("Failed loading revision 1 file capabilities");
		abort();
	}
	check_numeric_caps("Effective:   00000000, 00000021\n"
			   "Permitted:   00000000, 00000001\n"
			   "Inheritable: 00000000, 00000020\n");
	if (capng_get_rootid() != CAPNG_UNSET_ROOTID) {
		puts("Revision 1 load retained a rootid");
		abort();
	}

	mock_revision = MOCK_REVISION_2;
	if (capng_get_caps_fd(-1) != 0) {
		puts("Failed loading revision 2 file capabilities");
		abort();
	}
	check_numeric_caps("Effective:   00000000, 00000000\n"
			   "Permitted:   00000004, 00000001\n"
			   "Inheritable: 00000008, 00000020\n");
	if (capng_get_rootid() != CAPNG_UNSET_ROOTID) {
		puts("Revision 2 load retained a rootid");
		abort();
	}

	file = tmpfile();
	if (file == NULL)
		abort();
	fd = fileno(file);
	check_written_caps(fd, CAPNG_UNSET_ROOTID, VFS_CAP_REVISION_2,
			 XATTR_CAPS_SZ_2);
#ifdef VFS_CAP_REVISION_3
	check_written_caps(fd, 0, VFS_CAP_REVISION_3, XATTR_CAPS_SZ_3);
	check_written_caps(fd, 4242, VFS_CAP_REVISION_3, XATTR_CAPS_SZ_3);
	check_written_caps(fd, (uid_t)UINT32_C(0x80000000),
			 VFS_CAP_REVISION_3, XATTR_CAPS_SZ_3);
	check_written_caps(fd, CAPNG_UNSET_ROOTID, VFS_CAP_REVISION_2,
			 XATTR_CAPS_SZ_2);
#endif
	fclose(file);

	return EXIT_SUCCESS;
}
#else
int main(void)
{
	return 77;
}
#endif
