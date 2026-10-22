// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (c) 2026 Red Hat Inc.

#ifndef CAP_AUDIT_CLASSIFY_APP_H
#define CAP_AUDIT_CLASSIFY_APP_H

#include "gcc-attributes.h"

typedef enum { UNSUPPORTED, ELF, PYTHON } type_t;

type_t classify_app(const char *exe)
	__attr_access ((__read_only__, 1))
	__wur;

#endif
