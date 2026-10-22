/*
 * proc-output.h - Shared terminal output helpers for proc utilities
 * Copyright (c) 2026 Red Hat Inc.
 * All Rights Reserved.
 *
 * This software may be freely redistributed and/or modified under the
 * terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2, or (at your option) any
 * later version.
 */

#ifndef PROC_OUTPUT_H
#define PROC_OUTPUT_H

#include <stddef.h>
#include "gcc-attributes.h"

int proc_output_width(void);
void proc_print_wrapped(const char *head, const char *cont, const char *text,
			int width)
	__attr_access ((__read_only__, 1))
	__attr_access ((__read_only__, 2))
	__attr_access ((__read_only__, 3));
void proc_tree_print_node(const char *prefix, int is_last, const char *txt,
			  int width)
	__attr_access ((__read_only__, 1))
	__attr_access ((__read_only__, 3));
void proc_tree_build_child_prefix(char *dst, size_t dst_sz,
				  const char *prefix, int parent_is_last)
	__attr_access ((__write_only__, 1, 2))
	__attr_access ((__read_only__, 3));

#endif
