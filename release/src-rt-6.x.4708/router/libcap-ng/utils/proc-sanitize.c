/*
 * proc-sanitize.c - Shared terminal sanitization helpers for proc text
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "proc-sanitize.h"

/*
 * sanitize_untrusted_field - escape terminal control bytes in untrusted text.
 * @src: source text gathered from procfs/cgroup metadata.
 *
 * Returns caller-owned sanitized text, or NULL on allocation failure.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
char *sanitize_untrusted_field(const char *src)
{
	size_t in_len;
	char *dst;
	char *out;
	size_t i;

	if (!src)
		return NULL;
	in_len = strlen(src);
	dst = malloc(in_len * 4 + 1);
	if (!dst)
		return NULL;
	out = dst;
	for (i = 0; i < in_len; i++) {
		unsigned char c = (unsigned char)src[i];

		if (c < 0x20 || c == 0x7f) {
			snprintf(out, 5, "\\x%02X", c);
			out += 4;
		} else {
			*out++ = (char)c;
		}
	}
	*out = '\0';
	return dst;
}

/*
 * sanitize_untrusted_owned - replace owned string with sanitized version.
 * @s: pointer to owned string pointer that will be replaced in place.
 *
 * Returns 0 on success, -1 on allocation failure.
 * Side effects/assumptions: Operates on in-memory data and may read
 * procfs/netns state; it does not change kernel configuration.
 */
int sanitize_untrusted_owned(char **s)
{
	char *safe;

	if (!s || !*s)
		return 0;
	safe = sanitize_untrusted_field(*s);
	if (!safe)
		return -1;
	free(*s);
	*s = safe;
	return 0;
}
