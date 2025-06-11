#pragma once

/* a minimal version of libosmocore utils.h */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*! \defgroup utils General-purpose utility functions
 *  @{
 * \file utils.h */

/*! Determine number of elements in an array of static size */
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

#define OSMO_ASSERT(exp)    \
do { \
    if (OSMO_UNLIKELY(!(exp))) { \
        fprintf(stderr, "Assert failed %s %s:%d\n", #exp, __FILE__, __LINE__); \
		exit(2342); \
    } \
} while (0)

/*! Return the maximum of two specified values */
#define OSMO_MAX(a, b) ((a) >= (b) ? (a) : (b))
/*! Return the minimum of two specified values */
#define OSMO_MIN(a, b) ((a) >= (b) ? (b) : (a))
/*! Return a typical cmp result for comparable entities a and b. */
#define OSMO_CMP(a, b) ((a) < (b)? -1 : ((a) > (b)? 1 : 0))

/*! Stringify the name of a macro x, e.g. an FSM event name.
 * Note: if nested within another preprocessor macro, this will
 * stringify the value of x instead of its name. */
#define OSMO_STRINGIFY(x) #x
/*! Stringify the value of a macro x, e.g. a port number. */
#define OSMO_STRINGIFY_VAL(x) OSMO_STRINGIFY(x)
/*! Make a value_string entry from an enum value name */
#define OSMO_VALUE_STRING(x) { x, #x }

/*! Branch prediction optimizations */
#if defined(__GNUC__)
#define OSMO_LIKELY(exp) __builtin_expect(!!(exp), 1)
#define OSMO_UNLIKELY(exp) __builtin_expect(!!(exp), 0)
#else
#define OSMO_LIKELY(exp) exp
#define OSMO_UNLIKELY(exp) exp
#endif


/*! A mapping between human-readable string and numeric value */
struct value_string {
	uint32_t value;		/*!< numeric value */
	const char *str;	/*!< human-readable string */
};

const char *get_value_string(const struct value_string *vs, uint32_t val);
const char *get_value_string_or_null(const struct value_string *vs,
				     uint32_t val);

int get_string_value(const struct value_string *vs, const char *str);

bool osmo_identifier_valid(const char *str);
bool osmo_separated_identifiers_valid(const char *str, const char *sep_chars);
void osmo_identifier_sanitize_buf(char *str, const char *sep_chars, char replace_with);

char osmo_bcd2char(uint8_t bcd);
/* only works for numbers in ASCII */
uint8_t osmo_char2bcd(char c);

int osmo_bcd2str(char *dst, size_t dst_size, const uint8_t *bcd, int start_nibble, int end_nibble, bool allow_hex);
int osmo_str2bcd(uint8_t *dst, size_t dst_size, const char *digits, int start_nibble, int end_nibble, bool allow_hex);

/*! @} */
