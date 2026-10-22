// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (c) 2026 Red Hat Inc.

#ifndef CAP_AUDIT_PATH_H
#define CAP_AUDIT_PATH_H

#include "gcc-attributes.h"

char *resolve_target_command(const char *target)
	__attribute_malloc__
	__attr_dealloc_free
	__attr_access ((__read_only__, 1))
	__wur;

#endif
