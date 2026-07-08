/* Type information for ada/gcc-interface/utils.c.
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
gt_ggc_mx_gnat_binding_level (void *x_p)
{
  struct gnat_binding_level * x = (struct gnat_binding_level *)x_p;
  struct gnat_binding_level * xlimit = x;
  while (ggc_test_and_set_mark (xlimit))
   xlimit = ((*xlimit).chain);
  while (x != xlimit)
    {
      gt_ggc_m_18gnat_binding_level ((*x).chain);
      gt_ggc_m_9tree_node ((*x).block);
      gt_ggc_m_9tree_node ((*x).jmpbuf_decl);
      x = ((*x).chain);
    }
}

void
gt_ggc_mx_pad_type_hash (void *x_p)
{
  struct pad_type_hash * const x = (struct pad_type_hash *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_m_9tree_node ((*x).type);
    }
}

void
gt_ggc_mx (struct pad_type_hash& x_r ATTRIBUTE_UNUSED)
{
  struct pad_type_hash * ATTRIBUTE_UNUSED x = &x_r;
  gt_ggc_m_9tree_node ((*x).type);
}

void
gt_ggc_mx (struct pad_type_hash *& x)
{
  if (x)
    gt_ggc_mx_pad_type_hash ((void *) x);
}

void
gt_ggc_mx_hash_table_pad_type_hasher_ (void *x_p)
{
  hash_table<pad_type_hasher> * const x = (hash_table<pad_type_hasher> *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_mx (x);
    }
}

void
gt_ggc_mx (struct pad_type_hasher& x_r ATTRIBUTE_UNUSED)
{
  struct pad_type_hasher * ATTRIBUTE_UNUSED x = &x_r;
}

void
gt_pch_nx_gnat_binding_level (void *x_p)
{
  struct gnat_binding_level * x = (struct gnat_binding_level *)x_p;
  struct gnat_binding_level * xlimit = x;
  while (gt_pch_note_object (xlimit, xlimit, gt_pch_p_18gnat_binding_level))
   xlimit = ((*xlimit).chain);
  while (x != xlimit)
    {
      gt_pch_n_18gnat_binding_level ((*x).chain);
      gt_pch_n_9tree_node ((*x).block);
      gt_pch_n_9tree_node ((*x).jmpbuf_decl);
      x = ((*x).chain);
    }
}

void
gt_pch_nx_pad_type_hash (void *x_p)
{
  struct pad_type_hash * const x = (struct pad_type_hash *)x_p;
  if (gt_pch_note_object (x, x, gt_pch_p_13pad_type_hash))
    {
      gt_pch_n_9tree_node ((*x).type);
    }
}

void
gt_pch_nx (struct pad_type_hash& x_r ATTRIBUTE_UNUSED)
{
  struct pad_type_hash * ATTRIBUTE_UNUSED x = &x_r;
  gt_pch_n_9tree_node ((*x).type);
}

void
gt_pch_nx (struct pad_type_hash *& x)
{
  if (x)
    gt_pch_nx_pad_type_hash ((void *) x);
}

void
gt_pch_nx_hash_table_pad_type_hasher_ (void *x_p)
{
  hash_table<pad_type_hasher> * const x = (hash_table<pad_type_hasher> *)x_p;
  if (gt_pch_note_object (x, x, gt_pch_p_27hash_table_pad_type_hasher_))
    {
      gt_pch_nx (x);
    }
}

void
gt_pch_nx (struct pad_type_hasher& x_r ATTRIBUTE_UNUSED)
{
  struct pad_type_hasher * ATTRIBUTE_UNUSED x = &x_r;
}

void
gt_pch_p_18gnat_binding_level (ATTRIBUTE_UNUSED void *this_obj,
	void *x_p,
	ATTRIBUTE_UNUSED gt_pointer_operator op,
	ATTRIBUTE_UNUSED void *cookie)
{
  struct gnat_binding_level * x ATTRIBUTE_UNUSED = (struct gnat_binding_level *)x_p;
  if ((void *)(x) == this_obj)
    op (&((*x).chain), cookie);
  if ((void *)(x) == this_obj)
    op (&((*x).block), cookie);
  if ((void *)(x) == this_obj)
    op (&((*x).jmpbuf_decl), cookie);
}

void
gt_pch_p_13pad_type_hash (ATTRIBUTE_UNUSED void *this_obj,
	void *x_p,
	ATTRIBUTE_UNUSED gt_pointer_operator op,
	ATTRIBUTE_UNUSED void *cookie)
{
  struct pad_type_hash * x ATTRIBUTE_UNUSED = (struct pad_type_hash *)x_p;
  if ((void *)(x) == this_obj)
    op (&((*x).type), cookie);
}

void
gt_pch_nx (struct pad_type_hash* x ATTRIBUTE_UNUSED,
	ATTRIBUTE_UNUSED gt_pointer_operator op,
	ATTRIBUTE_UNUSED void *cookie)
{
    op (&((*x).type), cookie);
}

void
gt_pch_p_27hash_table_pad_type_hasher_ (ATTRIBUTE_UNUSED void *this_obj,
	void *x_p,
	ATTRIBUTE_UNUSED gt_pointer_operator op,
	ATTRIBUTE_UNUSED void *cookie)
{
  struct hash_table<pad_type_hasher> * x ATTRIBUTE_UNUSED = (struct hash_table<pad_type_hasher> *)x_p;
  if ((void *)(x) == this_obj)
    gt_pch_nx (&((*x)), op, cookie);
}

void
gt_pch_nx (struct pad_type_hasher* x ATTRIBUTE_UNUSED,
	ATTRIBUTE_UNUSED gt_pointer_operator op,
	ATTRIBUTE_UNUSED void *cookie)
{
}

/* GC roots.  */

static void gt_ggc_ma_dummy_node_table (void *);
static void
gt_ggc_ma_dummy_node_table (ATTRIBUTE_UNUSED void *x_p)
{
  if (dummy_node_table != NULL) {
    size_t i0;
    for (i0 = 0; i0 != (size_t)(max_gnat_nodes); i0++) {
      gt_ggc_m_9tree_node (dummy_node_table[i0]);
    }
    ggc_mark (dummy_node_table);
  }
}

static void gt_pch_pa_dummy_node_table
    (void *, void *, gt_pointer_operator, void *);
static void gt_pch_pa_dummy_node_table (ATTRIBUTE_UNUSED void *this_obj,
      ATTRIBUTE_UNUSED void *x_p,
      ATTRIBUTE_UNUSED gt_pointer_operator op,
      ATTRIBUTE_UNUSED void * cookie)
{
  if (dummy_node_table != NULL) {
    size_t i0;
    for (i0 = 0; i0 != (size_t)(max_gnat_nodes) && ((void *)dummy_node_table == this_obj); i0++) {
      if ((void *)(dummy_node_table) == this_obj)
        op (&(dummy_node_table[i0]), cookie);
    }
    if ((void *)(&dummy_node_table) == this_obj)
      op (&(dummy_node_table), cookie);
  }
}

static void gt_pch_na_dummy_node_table (void *);
static void
gt_pch_na_dummy_node_table (ATTRIBUTE_UNUSED void *x_p)
{
  if (dummy_node_table != NULL) {
    size_t i0;
    for (i0 = 0; i0 != (size_t)(max_gnat_nodes); i0++) {
      gt_pch_n_9tree_node (dummy_node_table[i0]);
    }
    gt_pch_note_object (dummy_node_table, &dummy_node_table, gt_pch_pa_dummy_node_table);
  }
}

static void gt_ggc_ma_associate_gnat_to_gnu (void *);
static void
gt_ggc_ma_associate_gnat_to_gnu (ATTRIBUTE_UNUSED void *x_p)
{
  if (associate_gnat_to_gnu != NULL) {
    size_t i0;
    for (i0 = 0; i0 != (size_t)(max_gnat_nodes); i0++) {
      gt_ggc_m_9tree_node (associate_gnat_to_gnu[i0]);
    }
    ggc_mark (associate_gnat_to_gnu);
  }
}

static void gt_pch_pa_associate_gnat_to_gnu
    (void *, void *, gt_pointer_operator, void *);
static void gt_pch_pa_associate_gnat_to_gnu (ATTRIBUTE_UNUSED void *this_obj,
      ATTRIBUTE_UNUSED void *x_p,
      ATTRIBUTE_UNUSED gt_pointer_operator op,
      ATTRIBUTE_UNUSED void * cookie)
{
  if (associate_gnat_to_gnu != NULL) {
    size_t i0;
    for (i0 = 0; i0 != (size_t)(max_gnat_nodes) && ((void *)associate_gnat_to_gnu == this_obj); i0++) {
      if ((void *)(associate_gnat_to_gnu) == this_obj)
        op (&(associate_gnat_to_gnu[i0]), cookie);
    }
    if ((void *)(&associate_gnat_to_gnu) == this_obj)
      op (&(associate_gnat_to_gnu), cookie);
  }
}

static void gt_pch_na_associate_gnat_to_gnu (void *);
static void
gt_pch_na_associate_gnat_to_gnu (ATTRIBUTE_UNUSED void *x_p)
{
  if (associate_gnat_to_gnu != NULL) {
    size_t i0;
    for (i0 = 0; i0 != (size_t)(max_gnat_nodes); i0++) {
      gt_pch_n_9tree_node (associate_gnat_to_gnu[i0]);
    }
    gt_pch_note_object (associate_gnat_to_gnu, &associate_gnat_to_gnu, gt_pch_pa_associate_gnat_to_gnu);
  }
}

EXPORTED_CONST struct ggc_root_tab gt_ggc_r_gt_ada_utils_h[] = {
  {
    &built_in_attributes[0],
    1 * ((int) ATTR_LAST),
    sizeof (built_in_attributes[0]),
    &gt_ggc_mx_tree_node,
    &gt_pch_nx_tree_node
  },
  {
    &builtin_types[0],
    1 * ((int) BT_LAST + 1),
    sizeof (builtin_types[0]),
    &gt_ggc_mx_tree_node,
    &gt_pch_nx_tree_node
  },
  {
    &dummy_global,
    1,
    sizeof (dummy_global),
    &gt_ggc_mx_tree_node,
    &gt_pch_nx_tree_node
  },
  {
    &pad_type_hash_table,
    1,
    sizeof (pad_type_hash_table),
    &gt_ggc_mx_hash_table_pad_type_hasher_,
    &gt_pch_nx_hash_table_pad_type_hasher_
  },
  {
    &builtin_decls,
    1,
    sizeof (builtin_decls),
    &gt_ggc_mx_vec_tree_va_gc_,
    &gt_pch_nx_vec_tree_va_gc_
  },
  {
    &global_decls,
    1,
    sizeof (global_decls),
    &gt_ggc_mx_vec_tree_va_gc_,
    &gt_pch_nx_vec_tree_va_gc_
  },
  {
    &global_context,
    1,
    sizeof (global_context),
    &gt_ggc_mx_tree_node,
    &gt_pch_nx_tree_node
  },
  {
    &current_binding_level,
    1,
    sizeof (current_binding_level),
    &gt_ggc_mx_gnat_binding_level,
    &gt_pch_nx_gnat_binding_level
  },
  {
    &float_types[0],
    1 * (NUM_MACHINE_MODES),
    sizeof (float_types[0]),
    &gt_ggc_mx_tree_node,
    &gt_pch_nx_tree_node
  },
  {
    &signed_and_unsigned_types[0][0],
    1 * (2 * MAX_BITS_PER_WORD + 1) * (2),
    sizeof (signed_and_unsigned_types[0][0]),
    &gt_ggc_mx_tree_node,
    &gt_pch_nx_tree_node
  },
  {
    &dummy_node_table,
    1,
    sizeof (dummy_node_table),
    &gt_ggc_ma_dummy_node_table,
    &gt_pch_na_dummy_node_table
  },
  {
    &associate_gnat_to_gnu,
    1,
    sizeof (associate_gnat_to_gnu),
    &gt_ggc_ma_associate_gnat_to_gnu,
    &gt_pch_na_associate_gnat_to_gnu
  },
  LAST_GGC_ROOT_TAB
};

EXPORTED_CONST struct ggc_root_tab gt_ggc_rd_gt_ada_utils_h[] = {
  { &free_block_chain, 1, sizeof (free_block_chain), NULL, NULL },
  { &free_binding_level, 1, sizeof (free_binding_level), NULL, NULL },
  LAST_GGC_ROOT_TAB
};

void
gt_clear_caches_gt_ada_utils_h ()
{
  gt_cleare_cache (pad_type_hash_table);
}

