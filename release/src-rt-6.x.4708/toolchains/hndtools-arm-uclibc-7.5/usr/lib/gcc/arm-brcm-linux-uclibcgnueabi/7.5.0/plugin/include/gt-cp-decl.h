/* Type information for cp/decl.c.
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
gt_ggc_mx_named_label_entry (void *x_p)
{
  struct named_label_entry * const x = (struct named_label_entry *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_m_9tree_node ((*x).label_decl);
      gt_ggc_m_16cp_binding_level ((*x).binding_level);
      gt_ggc_m_9tree_node ((*x).names_in_scope);
      gt_ggc_m_15vec_tree_va_gc_ ((*x).bad_decls);
      gt_ggc_m_21named_label_use_entry ((*x).uses);
    }
}

void
gt_ggc_mx (struct named_label_entry& x_r ATTRIBUTE_UNUSED)
{
  struct named_label_entry * ATTRIBUTE_UNUSED x = &x_r;
  gt_ggc_m_9tree_node ((*x).label_decl);
  gt_ggc_m_16cp_binding_level ((*x).binding_level);
  gt_ggc_m_9tree_node ((*x).names_in_scope);
  gt_ggc_m_15vec_tree_va_gc_ ((*x).bad_decls);
  gt_ggc_m_21named_label_use_entry ((*x).uses);
}

void
gt_ggc_mx (struct named_label_entry *& x)
{
  if (x)
    gt_ggc_mx_named_label_entry ((void *) x);
}

void
gt_ggc_mx_named_label_use_entry (void *x_p)
{
  struct named_label_use_entry * x = (struct named_label_use_entry *)x_p;
  struct named_label_use_entry * xlimit = x;
  while (ggc_test_and_set_mark (xlimit))
   xlimit = ((*xlimit).next);
  while (x != xlimit)
    {
      gt_ggc_m_21named_label_use_entry ((*x).next);
      gt_ggc_m_16cp_binding_level ((*x).binding_level);
      gt_ggc_m_9tree_node ((*x).names_in_scope);
      x = ((*x).next);
    }
}

void
gt_ggc_mx_vec_incomplete_var_va_gc_ (void *x_p)
{
  vec<incomplete_var,va_gc> * const x = (vec<incomplete_var,va_gc> *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_mx (x);
    }
}

void
gt_ggc_mx (struct incomplete_var& x_r ATTRIBUTE_UNUSED)
{
  struct incomplete_var * ATTRIBUTE_UNUSED x = &x_r;
  gt_ggc_m_9tree_node ((*x).decl);
  gt_ggc_m_9tree_node ((*x).incomplete_type);
}

void
gt_ggc_mx_hash_table_typename_hasher_ (void *x_p)
{
  hash_table<typename_hasher> * const x = (hash_table<typename_hasher> *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_mx (x);
    }
}

void
gt_ggc_mx (struct typename_hasher& x_r ATTRIBUTE_UNUSED)
{
  struct typename_hasher * ATTRIBUTE_UNUSED x = &x_r;
}

void
gt_pch_nx_named_label_entry (void *x_p)
{
  struct named_label_entry * const x = (struct named_label_entry *)x_p;
  if (gt_pch_note_object (x, x, gt_pch_p_17named_label_entry))
    {
      gt_pch_n_9tree_node ((*x).label_decl);
      gt_pch_n_16cp_binding_level ((*x).binding_level);
      gt_pch_n_9tree_node ((*x).names_in_scope);
      gt_pch_n_15vec_tree_va_gc_ ((*x).bad_decls);
      gt_pch_n_21named_label_use_entry ((*x).uses);
    }
}

void
gt_pch_nx (struct named_label_entry& x_r ATTRIBUTE_UNUSED)
{
  struct named_label_entry * ATTRIBUTE_UNUSED x = &x_r;
  gt_pch_n_9tree_node ((*x).label_decl);
  gt_pch_n_16cp_binding_level ((*x).binding_level);
  gt_pch_n_9tree_node ((*x).names_in_scope);
  gt_pch_n_15vec_tree_va_gc_ ((*x).bad_decls);
  gt_pch_n_21named_label_use_entry ((*x).uses);
}

void
gt_pch_nx (struct named_label_entry *& x)
{
  if (x)
    gt_pch_nx_named_label_entry ((void *) x);
}

void
gt_pch_nx_named_label_use_entry (void *x_p)
{
  struct named_label_use_entry * x = (struct named_label_use_entry *)x_p;
  struct named_label_use_entry * xlimit = x;
  while (gt_pch_note_object (xlimit, xlimit, gt_pch_p_21named_label_use_entry))
   xlimit = ((*xlimit).next);
  while (x != xlimit)
    {
      gt_pch_n_21named_label_use_entry ((*x).next);
      gt_pch_n_16cp_binding_level ((*x).binding_level);
      gt_pch_n_9tree_node ((*x).names_in_scope);
      x = ((*x).next);
    }
}

void
gt_pch_nx_vec_incomplete_var_va_gc_ (void *x_p)
{
  vec<incomplete_var,va_gc> * const x = (vec<incomplete_var,va_gc> *)x_p;
  if (gt_pch_note_object (x, x, gt_pch_p_25vec_incomplete_var_va_gc_))
    {
      gt_pch_nx (x);
    }
}

void
gt_pch_nx (struct incomplete_var& x_r ATTRIBUTE_UNUSED)
{
  struct incomplete_var * ATTRIBUTE_UNUSED x = &x_r;
  gt_pch_n_9tree_node ((*x).decl);
  gt_pch_n_9tree_node ((*x).incomplete_type);
}

void
gt_pch_nx_hash_table_typename_hasher_ (void *x_p)
{
  hash_table<typename_hasher> * const x = (hash_table<typename_hasher> *)x_p;
  if (gt_pch_note_object (x, x, gt_pch_p_27hash_table_typename_hasher_))
    {
      gt_pch_nx (x);
    }
}

void
gt_pch_nx (struct typename_hasher& x_r ATTRIBUTE_UNUSED)
{
  struct typename_hasher * ATTRIBUTE_UNUSED x = &x_r;
}

void
gt_pch_p_17named_label_entry (ATTRIBUTE_UNUSED void *this_obj,
	void *x_p,
	ATTRIBUTE_UNUSED gt_pointer_operator op,
	ATTRIBUTE_UNUSED void *cookie)
{
  struct named_label_entry * x ATTRIBUTE_UNUSED = (struct named_label_entry *)x_p;
  if ((void *)(x) == this_obj)
    op (&((*x).label_decl), cookie);
  if ((void *)(x) == this_obj)
    op (&((*x).binding_level), cookie);
  if ((void *)(x) == this_obj)
    op (&((*x).names_in_scope), cookie);
  if ((void *)(x) == this_obj)
    op (&((*x).bad_decls), cookie);
  if ((void *)(x) == this_obj)
    op (&((*x).uses), cookie);
}

void
gt_pch_nx (struct named_label_entry* x ATTRIBUTE_UNUSED,
	ATTRIBUTE_UNUSED gt_pointer_operator op,
	ATTRIBUTE_UNUSED void *cookie)
{
    op (&((*x).label_decl), cookie);
    op (&((*x).binding_level), cookie);
    op (&((*x).names_in_scope), cookie);
    op (&((*x).bad_decls), cookie);
    op (&((*x).uses), cookie);
}

void
gt_pch_p_21named_label_use_entry (ATTRIBUTE_UNUSED void *this_obj,
	void *x_p,
	ATTRIBUTE_UNUSED gt_pointer_operator op,
	ATTRIBUTE_UNUSED void *cookie)
{
  struct named_label_use_entry * x ATTRIBUTE_UNUSED = (struct named_label_use_entry *)x_p;
  if ((void *)(x) == this_obj)
    op (&((*x).next), cookie);
  if ((void *)(x) == this_obj)
    op (&((*x).binding_level), cookie);
  if ((void *)(x) == this_obj)
    op (&((*x).names_in_scope), cookie);
}

void
gt_pch_p_25vec_incomplete_var_va_gc_ (ATTRIBUTE_UNUSED void *this_obj,
	void *x_p,
	ATTRIBUTE_UNUSED gt_pointer_operator op,
	ATTRIBUTE_UNUSED void *cookie)
{
  struct vec<incomplete_var,va_gc> * x ATTRIBUTE_UNUSED = (struct vec<incomplete_var,va_gc> *)x_p;
  if ((void *)(x) == this_obj)
    gt_pch_nx (&((*x)), op, cookie);
}

void
gt_pch_nx (struct incomplete_var* x ATTRIBUTE_UNUSED,
	ATTRIBUTE_UNUSED gt_pointer_operator op,
	ATTRIBUTE_UNUSED void *cookie)
{
    op (&((*x).decl), cookie);
    op (&((*x).incomplete_type), cookie);
}

void
gt_pch_p_27hash_table_typename_hasher_ (ATTRIBUTE_UNUSED void *this_obj,
	void *x_p,
	ATTRIBUTE_UNUSED gt_pointer_operator op,
	ATTRIBUTE_UNUSED void *cookie)
{
  struct hash_table<typename_hasher> * x ATTRIBUTE_UNUSED = (struct hash_table<typename_hasher> *)x_p;
  if ((void *)(x) == this_obj)
    gt_pch_nx (&((*x)), op, cookie);
}

void
gt_pch_nx (struct typename_hasher* x ATTRIBUTE_UNUSED,
	ATTRIBUTE_UNUSED gt_pointer_operator op,
	ATTRIBUTE_UNUSED void *cookie)
{
}

/* GC roots.  */

EXPORTED_CONST struct ggc_root_tab gt_ggc_r_gt_cp_decl_h[] = {
  {
    &decomp_type_table,
    1,
    sizeof (decomp_type_table),
    &gt_ggc_mx_hash_map_tree_tree_,
    &gt_pch_nx_hash_map_tree_tree_
  },
  {
    &typename_htab,
    1,
    sizeof (typename_htab),
    &gt_ggc_mx_hash_table_typename_hasher_,
    &gt_pch_nx_hash_table_typename_hasher_
  },
  {
    &incomplete_vars,
    1,
    sizeof (incomplete_vars),
    &gt_ggc_mx_vec_incomplete_var_va_gc_,
    &gt_pch_nx_vec_incomplete_var_va_gc_
  },
  LAST_GGC_ROOT_TAB
};

EXPORTED_CONST struct ggc_root_tab gt_pch_rs_gt_cp_decl_h[] = {
  { &start_cleanup_cnt, 1, sizeof (start_cleanup_cnt), NULL, NULL },
  LAST_GGC_ROOT_TAB
};

