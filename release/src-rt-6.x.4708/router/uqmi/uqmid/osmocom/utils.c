/*
 * a minimal version of utils.c from libosmocore
 *
 * (C) 2011 by Harald Welte <laforge@gnumonks.org>
 * (C) 2011 by Sylvain Munaut <tnt@246tNt.com>
 * (C) 2014 by Nils O. Selåsdal <noselasd@fiane.dyndns.org>
 *
 * All Rights Reserved
 *
 * SPDX-License-Identifier: GPL-2.0+
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
 */


#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <stdio.h>
#include <inttypes.h>
#include <limits.h>
#include <ctype.h>

#include "utils.h"

static __thread char namebuf[255];
static const char osmo_identifier_illegal_chars[] = "., {}[]()<>|~\\^`'\"?=;/+*&%$#!";

/*! Determine if a given identifier is valid, i.e. doesn't contain illegal chars
 *  \param[in] str String to validate
 *  \param[in] sep_chars Permitted separation characters between identifiers.
 *  \returns true in case \a str contains only valid identifiers and sep_chars, false otherwise
 */
bool osmo_separated_identifiers_valid(const char *str, const char *sep_chars)
{
	/* characters that are illegal in names */
	unsigned int i;
	size_t len;

	/* an empty string is not a valid identifier */
	if (!str || (len = strlen(str)) == 0)
		return false;

	for (i = 0; i < len; i++) {
		if (sep_chars && strchr(sep_chars, str[i]))
			continue;
		/* check for 7-bit ASCII */
		if (str[i] & 0x80)
			return false;
		if (!isprint((int)str[i]))
			return false;
		/* check for some explicit reserved control characters */
		if (strchr(osmo_identifier_illegal_chars, str[i]))
			return false;
	}

	return true;
}

/*! Determine if a given identifier is valid, i.e. doesn't contain illegal chars
 *  \param[in] str String to validate
 *  \returns true in case \a str contains valid identifier, false otherwise
 */
bool osmo_identifier_valid(const char *str)
{
	return osmo_separated_identifiers_valid(str, NULL);
}

/*! Replace characters in the given string buffer so that it is guaranteed to pass osmo_separated_identifiers_valid().
 * To guarantee passing osmo_separated_identifiers_valid(), replace_with must not itself be an illegal character. If in
 * doubt, use '-'.
 * \param[inout] str  Identifier to sanitize, must be nul terminated and in a writable buffer.
 * \param[in] sep_chars  Additional characters that are to be replaced besides osmo_identifier_illegal_chars.
 * \param[in] replace_with  Replace any illegal characters with this character.
 */
void osmo_identifier_sanitize_buf(char *str, const char *sep_chars, char replace_with)
{
	char *pos;
	if (!str)
		return;
	for (pos = str; *pos; pos++) {
		if (strchr(osmo_identifier_illegal_chars, *pos)
		    || (sep_chars && strchr(sep_chars, *pos)))
			*pos = replace_with;
	}
}

/*! get human-readable string for given value
 *  \param[in] vs Array of value_string tuples
 *  \param[in] val Value to be converted
 *  \returns pointer to human-readable string
 *
 * If val is found in vs, the array's string entry is returned. Otherwise, an
 * "unknown" string containing the actual value is composed in a static buffer
 * that is reused across invocations.
 */
const char *get_value_string(const struct value_string *vs, uint32_t val)
{
	const char *str = get_value_string_or_null(vs, val);
	if (str)
		return str;

	snprintf(namebuf, sizeof(namebuf), "unknown 0x%"PRIx32, val);
	namebuf[sizeof(namebuf) - 1] = '\0';
	return namebuf;
}

/*! get human-readable string or NULL for given value
 *  \param[in] vs Array of value_string tuples
 *  \param[in] val Value to be converted
 *  \returns pointer to human-readable string or NULL if val is not found
 */
const char *get_value_string_or_null(const struct value_string *vs,
				     uint32_t val)
{
	int i;

	if (!vs)
		return NULL;

	for (i = 0;; i++) {
		if (vs[i].value == 0 && vs[i].str == NULL)
			break;
		if (vs[i].value == val)
			return vs[i].str;
	}

	return NULL;
}

/*! get numeric value for given human-readable string
 *  \param[in] vs Array of value_string tuples
 *  \param[in] str human-readable string
 *  \returns numeric value (>0) or negative numer in case of error
 */
int get_string_value(const struct value_string *vs, const char *str)
{
	int i;

	for (i = 0;; i++) {
		if (vs[i].value == 0 && vs[i].str == NULL)
			break;
		if (!strcasecmp(vs[i].str, str))
			return vs[i].value;
	}
	return -EINVAL;
}

/*! Convert BCD-encoded digit into printable character
 *  \param[in] bcd A single BCD-encoded digit
 *  \returns single printable character
 */
char osmo_bcd2char(uint8_t bcd)
{
	if (bcd < 0xa)
		return '0' + bcd;
	else
		return 'A' + (bcd - 0xa);
}

/*! Convert number in ASCII to BCD value
 *  \param[in] c ASCII character
 *  \returns BCD encoded value of character
 */
uint8_t osmo_char2bcd(char c)
{
	if (c >= '0' && c <= '9')
		return c - 0x30;
	else if (c >= 'A' && c <= 'F')
		return 0xa + (c - 'A');
	else if (c >= 'a' && c <= 'f')
		return 0xa + (c - 'a');
	else
		return 0;
}

/*! Convert BCD to string.
 * The given nibble offsets are interpreted in BCD order, i.e. nibble 0 is bcd[0] & 0xf, nibble 1 is bcd[0] >> 4, nibble
 * 3 is bcd[1] & 0xf, etc..
 *  \param[out] dst  Output string buffer, is always nul terminated when dst_size > 0.
 *  \param[in] dst_size  sizeof() the output string buffer.
 *  \param[in] bcd  Binary coded data buffer.
 *  \param[in] start_nibble  Offset to start from, in nibbles, typically 1 to skip the first nibble.
 *  \param[in] end_nibble  Offset to stop before, in nibbles, e.g. sizeof(bcd)*2 - (bcd[0] & GSM_MI_ODD? 0:1).
 *  \param[in] allow_hex  If false, return error if there are digits other than 0-9. If true, return those as [A-F].
 *  \returns The strlen that would be written if the output buffer is large enough, excluding nul byte (like
 *           snprintf()), or -EINVAL if allow_hex is false and a digit > 9 is encountered. On -EINVAL, the conversion is
 *           still completed as if allow_hex were passed as true. Return -ENOMEM if dst is NULL or dst_size is zero.
 *           If end_nibble <= start_nibble, write an empty string to dst and return 0.
 */
int osmo_bcd2str(char *dst, size_t dst_size, const uint8_t *bcd, int start_nibble, int end_nibble, bool allow_hex)
{
	char *dst_end = dst + dst_size - 1;
	int nibble_i;
	int rc = 0;

	if (!dst || dst_size < 1 || start_nibble < 0)
		return -ENOMEM;

	for (nibble_i = start_nibble; nibble_i < end_nibble && dst < dst_end; nibble_i++, dst++) {
		uint8_t nibble = bcd[nibble_i >> 1];
		if ((nibble_i & 1))
			nibble >>= 4;
		nibble &= 0xf;

		if (!allow_hex && nibble > 9)
			rc = -EINVAL;

		*dst = osmo_bcd2char(nibble);
	}
	*dst = '\0';

	if (rc < 0)
		return rc;
	return OSMO_MAX(0, end_nibble - start_nibble);
}

/*! Convert string to BCD.
 * The given nibble offsets are interpreted in BCD order, i.e. nibble 0 is bcd[0] & 0x0f, nibble 1 is bcd[0] & 0xf0, nibble
 * 3 is bcd[1] & 0x0f, etc..
 *  \param[out] dst  Output BCD buffer.
 *  \param[in] dst_size  sizeof() the output string buffer.
 *  \param[in] digits  String containing decimal or hexadecimal digits in upper or lower case.
 *  \param[in] start_nibble  Offset to start from, in nibbles, typically 1 to skip the first (MI type) nibble.
 *  \param[in] end_nibble  Negative to write all digits found in str, followed by 0xf nibbles to fill any started octet.
 *                         If >= 0, stop before this offset in nibbles, e.g. to get default behavior, pass
 *                         start_nibble + strlen(str) + ((start_nibble + strlen(str)) & 1? 1 : 0) + 1.
 *  \param[in] allow_hex  If false, return error if there are hexadecimal digits (A-F). If true, write those to
 *                        BCD.
 *  \returns The buffer size in octets that is used to place all bcd digits (including the skipped nibbles
 *           from 'start_nibble' and rounded up to full octets); -EINVAL on invalid digits;
 *           -ENOMEM if dst is NULL, if dst_size is too small to contain all nibbles, or if start_nibble is negative.
 */
int osmo_str2bcd(uint8_t *dst, size_t dst_size, const char *digits, int start_nibble, int end_nibble, bool allow_hex)
{
	const char *digit = digits;
	int nibble_i;

	if (!dst || !dst_size || start_nibble < 0)
		return -ENOMEM;

	if (end_nibble < 0) {
		end_nibble = start_nibble + strlen(digits);
		/* If the last octet is not complete, add another filler nibble */
		if (end_nibble & 1)
			end_nibble++;
	}
	if ((unsigned int) (end_nibble / 2) > dst_size)
		return -ENOMEM;

	for (nibble_i = start_nibble; nibble_i < end_nibble; nibble_i++) {
		uint8_t nibble = 0xf;
		int octet = nibble_i >> 1;
		if (*digit) {
			char c = *digit;
			digit++;
			if (c >= '0' && c <= '9')
				nibble = c - '0';
			else if (allow_hex && c >= 'A' && c <= 'F')
				nibble = 0xa + (c - 'A');
			else if (allow_hex && c >= 'a' && c <= 'f')
				nibble = 0xa + (c - 'a');
			else
				return -EINVAL;
		}
		nibble &= 0xf;
		if ((nibble_i & 1))
			dst[octet] = (nibble << 4) | (dst[octet] & 0x0f);
		else
			dst[octet] = (dst[octet] & 0xf0) | nibble;
	}

	/* floor(float(end_nibble) / 2) */
	return end_nibble / 2;
}
