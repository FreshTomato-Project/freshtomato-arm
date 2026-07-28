/* Type information for cp/cp-objcp-common.c.
   Copyright (C) 2004-2017 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

/* This file is machine generated.  Do not edit.  */

void
gt_ggc_mx_hash_table_debug_type_hasher_ (void *x_p)
{
  hash_table<debug_type_hasher> * const x = (hash_table<debug_type_hasher> *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_mx (x);
    }
}

void
gt_ggc_mx (struct debug_type_hasher& x_r ATTRIBUTE_UNUSED)
{
  struct debug_type_hasher * ATTRIBUTE_UNUSED x = &x_r;
}

void
gt_pch_nx_hash_table_debug_type_hasher_ (void *x_p)
{
  hash_table<debug_type_hasher> * const x = (hash_table<debug_type_hasher> *)x_p;
  if (gt_pch_note_object (x, x, gt_pch_p_29hash_table_debug_type_hasher_))
    {
      gt_pch_nx (x);
    }
}

void
gt_pch_nx (struct debug_type_hasher& x_r ATTRIBUTE_UNUSED)
{
  struct debug_type_hasher * ATTRIBUTE_UNUSED x = &x_r;
}

void
gt_pch_p_29hash_table_debug_type_hasher_ (ATTRIBUTE_UNUSED void *this_obj,
	void *x_p,
	ATTRIBUTE_UNUSED gt_pointer_operator op,
	ATTRIBUTE_UNUSED void *cookie)
{
  struct hash_table<debug_type_hasher> * x ATTRIBUTE_UNUSED = (struct hash_table<debug_type_hasher> *)x_p;
  if ((void *)(x) == this_obj)
    gt_pch_nx (&((*x)), op, cookie);
}

void
gt_pch_nx (struct debug_type_hasher* x ATTRIBUTE_UNUSED,
	ATTRIBUTE_UNUSED gt_pointer_operator op,
	ATTRIBUTE_UNUSED void *cookie)
{
}

/* GC roots.  */

EXPORTED_CONST struct ggc_root_tab gt_ggc_r_gt_cp_cp_objcp_common_h[] = {
  {
    &shadowed_var_for_decl,
    1,
    sizeof (shadowed_var_for_decl),
    &gt_ggc_mx_hash_table_tree_decl_map_cache_hasher_,
    &gt_pch_nx_hash_table_tree_decl_map_cache_hasher_
  },
  {
    &debug_type_hash,
    1,
    sizeof (debug_type_hash),
    &gt_ggc_mx_hash_table_debug_type_hasher_,
    &gt_pch_nx_hash_table_debug_type_hasher_
  },
  LAST_GGC_ROOT_TAB
};

void
gt_clear_caches_gt_cp_cp_objcp_common_h ()
{
  gt_cleare_cache (shadowed_var_for_decl);
  gt_cleare_cache (debug_type_hash);
}

