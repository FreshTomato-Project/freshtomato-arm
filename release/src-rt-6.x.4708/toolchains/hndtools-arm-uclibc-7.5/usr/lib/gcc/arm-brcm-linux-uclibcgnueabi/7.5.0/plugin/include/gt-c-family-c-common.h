/* Type information for c-family/c-common.h.
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
gt_ggc_mx_sorted_fields_type (void *x_p)
{
  struct sorted_fields_type * const x = (struct sorted_fields_type *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      {
        size_t l0 = (size_t)(((*x)).len);
        {
          size_t i0;
          for (i0 = 0; i0 != l0; i0++) {
            gt_ggc_m_9tree_node ((*x).elts[i0]);
          }
        }
      }
    }
}

void
gt_ggc_mx_hash_table_c_type_hasher_ (void *x_p)
{
  hash_table<c_type_hasher> * const x = (hash_table<c_type_hasher> *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_mx (x);
    }
}

void
gt_ggc_mx (struct c_type_hasher& x_r ATTRIBUTE_UNUSED)
{
  struct c_type_hasher * ATTRIBUTE_UNUSED x = &x_r;
}

void
gt_ggc_mx_vec_const_char_p_va_gc_ (void *x_p)
{
  vec<const_char_p,va_gc> * const x = (vec<const_char_p,va_gc> *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_mx (x);
    }
}

void
gt_ggc_mx_vec_tree_gc_vec_va_gc_ (void *x_p)
{
  vec<tree_gc_vec,va_gc> * const x = (vec<tree_gc_vec,va_gc> *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_mx (x);
    }
}

void
gt_ggc_mx (vec<tree,va_gc> *& x)
{
  if (x)
    gt_ggc_mx_vec_tree_va_gc_ ((void *) x);
}

void
gt_pch_nx_sorted_fields_type (void *x_p)
{
  struct sorted_fields_type * const x = (struct sorted_fields_type *)x_p;
  if (gt_pch_note_object (x, x, gt_pch_p_18sorted_fields_type))
    {
      {
        size_t l0 = (size_t)(((*x)).len);
        {
          size_t i0;
          for (i0 = 0; i0 != l0; i0++) {
            gt_pch_n_9tree_node ((*x).elts[i0]);
          }
        }
      }
    }
}

void
gt_pch_nx_hash_table_c_type_hasher_ (void *x_p)
{
  hash_table<c_type_hasher> * const x = (hash_table<c_type_hasher> *)x_p;
  if (gt_pch_note_object (x, x, gt_pch_p_25hash_table_c_type_hasher_))
    {
      gt_pch_nx (x);
    }
}

void
gt_pch_nx (struct c_type_hasher& x_r ATTRIBUTE_UNUSED)
{
  struct c_type_hasher * ATTRIBUTE_UNUSED x = &x_r;
}

void
gt_pch_nx_vec_const_char_p_va_gc_ (void *x_p)
{
  vec<const_char_p,va_gc> * const x = (vec<const_char_p,va_gc> *)x_p;
  if (gt_pch_note_object (x, x, gt_pch_p_23vec_const_char_p_va_gc_))
    {
      gt_pch_nx (x);
    }
}

void
gt_pch_nx_vec_tree_gc_vec_va_gc_ (void *x_p)
{
  vec<tree_gc_vec,va_gc> * const x = (vec<tree_gc_vec,va_gc> *)x_p;
  if (gt_pch_note_object (x, x, gt_pch_p_22vec_tree_gc_vec_va_gc_))
    {
      gt_pch_nx (x);
    }
}

void
gt_pch_nx (vec<tree,va_gc> *& x)
{
  if (x)
    gt_pch_nx_vec_tree_va_gc_ ((void *) x);
}

void
gt_pch_p_18sorted_fields_type (ATTRIBUTE_UNUSED void *this_obj,
	void *x_p,
	ATTRIBUTE_UNUSED gt_pointer_operator op,
	ATTRIBUTE_UNUSED void *cookie)
{
  struct sorted_fields_type * x ATTRIBUTE_UNUSED = (struct sorted_fields_type *)x_p;
  {
    size_t l0 = (size_t)(((*x)).len);
    {
      size_t i0;
      for (i0 = 0; i0 != l0; i0++) {
        if ((void *)(x) == this_obj)
          op (&((*x).elts[i0]), cookie);
      }
    }
  }
}

void
gt_pch_p_25hash_table_c_type_hasher_ (ATTRIBUTE_UNUSED void *this_obj,
	void *x_p,
	ATTRIBUTE_UNUSED gt_pointer_operator op,
	ATTRIBUTE_UNUSED void *cookie)
{
  struct hash_table<c_type_hasher> * x ATTRIBUTE_UNUSED = (struct hash_table<c_type_hasher> *)x_p;
  if ((void *)(x) == this_obj)
    gt_pch_nx (&((*x)), op, cookie);
}

void
gt_pch_nx (struct c_type_hasher* x ATTRIBUTE_UNUSED,
	ATTRIBUTE_UNUSED gt_pointer_operator op,
	ATTRIBUTE_UNUSED void *cookie)
{
}

void
gt_pch_p_23vec_const_char_p_va_gc_ (ATTRIBUTE_UNUSED void *this_obj,
	void *x_p,
	ATTRIBUTE_UNUSED gt_pointer_operator op,
	ATTRIBUTE_UNUSED void *cookie)
{
  struct vec<const_char_p,va_gc> * x ATTRIBUTE_UNUSED = (struct vec<const_char_p,va_gc> *)x_p;
  if ((void *)(x) == this_obj)
    gt_pch_nx (&((*x)), op, cookie);
}

void
gt_pch_p_22vec_tree_gc_vec_va_gc_ (ATTRIBUTE_UNUSED void *this_obj,
	void *x_p,
	ATTRIBUTE_UNUSED gt_pointer_operator op,
	ATTRIBUTE_UNUSED void *cookie)
{
  struct vec<tree_gc_vec,va_gc> * x ATTRIBUTE_UNUSED = (struct vec<tree_gc_vec,va_gc> *)x_p;
  if ((void *)(x) == this_obj)
    gt_pch_nx (&((*x)), op, cookie);
}

/* GC roots.  */

static void gt_ggc_ma_ridpointers (void *);
static void
gt_ggc_ma_ridpointers (ATTRIBUTE_UNUSED void *x_p)
{
  if (ridpointers != NULL) {
    size_t i0;
    for (i0 = 0; i0 != (size_t)((int) RID_MAX); i0++) {
      gt_ggc_m_9tree_node (ridpointers[i0]);
    }
    ggc_mark (ridpointers);
  }
}

static void gt_pch_pa_ridpointers
    (void *, void *, gt_pointer_operator, void *);
static void gt_pch_pa_ridpointers (ATTRIBUTE_UNUSED void *this_obj,
      ATTRIBUTE_UNUSED void *x_p,
      ATTRIBUTE_UNUSED gt_pointer_operator op,
      ATTRIBUTE_UNUSED void * cookie)
{
  if (ridpointers != NULL) {
    size_t i0;
    for (i0 = 0; i0 != (size_t)((int) RID_MAX) && ((void *)ridpointers == this_obj); i0++) {
      if ((void *)(ridpointers) == this_obj)
        op (&(ridpointers[i0]), cookie);
    }
    if ((void *)(&ridpointers) == this_obj)
      op (&(ridpointers), cookie);
  }
}

static void gt_pch_na_ridpointers (void *);
static void
gt_pch_na_ridpointers (ATTRIBUTE_UNUSED void *x_p)
{
  if (ridpointers != NULL) {
    size_t i0;
    for (i0 = 0; i0 != (size_t)((int) RID_MAX); i0++) {
      gt_pch_n_9tree_node (ridpointers[i0]);
    }
    gt_pch_note_object (ridpointers, &ridpointers, gt_pch_pa_ridpointers);
  }
}

EXPORTED_CONST struct ggc_root_tab gt_ggc_r_gt_c_family_c_common_h[] = {
  {
    &g_string_concat_db,
    1,
    sizeof (g_string_concat_db),
    &gt_ggc_mx_string_concat_db,
    &gt_pch_nx_string_concat_db
  },
  {
    &registered_builtin_types,
    1,
    sizeof (registered_builtin_types),
    &gt_ggc_mx_tree_node,
    &gt_pch_nx_tree_node
  },
  {
    &c_global_trees[0],
    1 * (CTI_MAX),
    sizeof (c_global_trees[0]),
    &gt_ggc_mx_tree_node,
    &gt_pch_nx_tree_node
  },
  {
    &ridpointers,
    1,
    sizeof (ridpointers),
    &gt_ggc_ma_ridpointers,
    &gt_pch_na_ridpointers
  },
  {
    &optimize_args,
    1,
    sizeof (optimize_args),
    &gt_ggc_mx_vec_const_char_p_va_gc_,
    &gt_pch_nx_vec_const_char_p_va_gc_
  },
  {
    &built_in_attributes[0],
    1 * ((int) ATTR_LAST),
    sizeof (built_in_attributes[0]),
    &gt_ggc_mx_tree_node,
    &gt_pch_nx_tree_node
  },
  {
    &type_hash_table,
    1,
    sizeof (type_hash_table),
    &gt_ggc_mx_hash_table_c_type_hasher_,
    &gt_pch_nx_hash_table_c_type_hasher_
  },
  LAST_GGC_ROOT_TAB
};

EXPORTED_CONST struct ggc_root_tab gt_ggc_rd_gt_c_family_c_common_h[] = {
  { &tree_vector_cache, 1, sizeof (tree_vector_cache), NULL, NULL },
  LAST_GGC_ROOT_TAB
};

EXPORTED_CONST struct ggc_root_tab gt_pch_rs_gt_c_family_c_common_h[] = {
  { &pending_lang_change, 1, sizeof (pending_lang_change), NULL, NULL },
  { &compound_literal_number, 1, sizeof (compound_literal_number), NULL, NULL },
  LAST_GGC_ROOT_TAB
};

