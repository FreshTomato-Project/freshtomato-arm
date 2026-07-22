/* Type information for ada/gcc-interface/decl.c.
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
gt_ggc_mx_hash_table_value_annotation_hasher_ (void *x_p)
{
  hash_table<value_annotation_hasher> * const x = (hash_table<value_annotation_hasher> *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_mx (x);
    }
}

void
gt_ggc_mx (struct value_annotation_hasher& x_r ATTRIBUTE_UNUSED)
{
  struct value_annotation_hasher * ATTRIBUTE_UNUSED x = &x_r;
}

void
gt_ggc_mx_vec_Entity_Id_va_gc_atomic_ (void *x_p)
{
  vec<Entity_Id,va_gc_atomic> * const x = (vec<Entity_Id,va_gc_atomic> *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_mx (x);
    }
}

void
gt_ggc_mx_tree_entity_vec_map (void *x_p)
{
  struct tree_entity_vec_map * const x = (struct tree_entity_vec_map *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_m_9tree_node ((*x).base.from);
      gt_ggc_m_27vec_Entity_Id_va_gc_atomic_ ((*x).to);
    }
}

void
gt_ggc_mx (struct tree_entity_vec_map& x_r ATTRIBUTE_UNUSED)
{
  struct tree_entity_vec_map * ATTRIBUTE_UNUSED x = &x_r;
  gt_ggc_m_9tree_node ((*x).base.from);
  gt_ggc_m_27vec_Entity_Id_va_gc_atomic_ ((*x).to);
}

void
gt_ggc_mx (struct tree_entity_vec_map *& x)
{
  if (x)
    gt_ggc_mx_tree_entity_vec_map ((void *) x);
}

void
gt_ggc_mx_hash_table_dummy_type_hasher_ (void *x_p)
{
  hash_table<dummy_type_hasher> * const x = (hash_table<dummy_type_hasher> *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_mx (x);
    }
}

void
gt_ggc_mx (struct dummy_type_hasher& x_r ATTRIBUTE_UNUSED)
{
  struct dummy_type_hasher * ATTRIBUTE_UNUSED x = &x_r;
}

void
gt_pch_nx_hash_table_value_annotation_hasher_ (void *x_p)
{
  hash_table<value_annotation_hasher> * const x = (hash_table<value_annotation_hasher> *)x_p;
  if (gt_pch_note_object (x, x, gt_pch_p_35hash_table_value_annotation_hasher_))
    {
      gt_pch_nx (x);
    }
}

void
gt_pch_nx (struct value_annotation_hasher& x_r ATTRIBUTE_UNUSED)
{
  struct value_annotation_hasher * ATTRIBUTE_UNUSED x = &x_r;
}

void
gt_pch_nx_vec_Entity_Id_va_gc_atomic_ (void *x_p)
{
  vec<Entity_Id,va_gc_atomic> * const x = (vec<Entity_Id,va_gc_atomic> *)x_p;
  if (gt_pch_note_object (x, x, gt_pch_p_27vec_Entity_Id_va_gc_atomic_))
    {
      gt_pch_nx (x);
    }
}

void
gt_pch_nx_tree_entity_vec_map (void *x_p)
{
  struct tree_entity_vec_map * const x = (struct tree_entity_vec_map *)x_p;
  if (gt_pch_note_object (x, x, gt_pch_p_19tree_entity_vec_map))
    {
      gt_pch_n_9tree_node ((*x).base.from);
      gt_pch_n_27vec_Entity_Id_va_gc_atomic_ ((*x).to);
    }
}

void
gt_pch_nx (struct tree_entity_vec_map& x_r ATTRIBUTE_UNUSED)
{
  struct tree_entity_vec_map * ATTRIBUTE_UNUSED x = &x_r;
  gt_pch_n_9tree_node ((*x).base.from);
  gt_pch_n_27vec_Entity_Id_va_gc_atomic_ ((*x).to);
}

void
gt_pch_nx (struct tree_entity_vec_map *& x)
{
  if (x)
    gt_pch_nx_tree_entity_vec_map ((void *) x);
}

void
gt_pch_nx_hash_table_dummy_type_hasher_ (void *x_p)
{
  hash_table<dummy_type_hasher> * const x = (hash_table<dummy_type_hasher> *)x_p;
  if (gt_pch_note_object (x, x, gt_pch_p_29hash_table_dummy_type_hasher_))
    {
      gt_pch_nx (x);
    }
}

void
gt_pch_nx (struct dummy_type_hasher& x_r ATTRIBUTE_UNUSED)
{
  struct dummy_type_hasher * ATTRIBUTE_UNUSED x = &x_r;
}

void
gt_pch_p_35hash_table_value_annotation_hasher_ (ATTRIBUTE_UNUSED void *this_obj,
	void *x_p,
	ATTRIBUTE_UNUSED gt_pointer_operator op,
	ATTRIBUTE_UNUSED void *cookie)
{
  struct hash_table<value_annotation_hasher> * x ATTRIBUTE_UNUSED = (struct hash_table<value_annotation_hasher> *)x_p;
  if ((void *)(x) == this_obj)
    gt_pch_nx (&((*x)), op, cookie);
}

void
gt_pch_nx (struct value_annotation_hasher* x ATTRIBUTE_UNUSED,
	ATTRIBUTE_UNUSED gt_pointer_operator op,
	ATTRIBUTE_UNUSED void *cookie)
{
}

void
gt_pch_p_27vec_Entity_Id_va_gc_atomic_ (ATTRIBUTE_UNUSED void *this_obj,
	void *x_p,
	ATTRIBUTE_UNUSED gt_pointer_operator op,
	ATTRIBUTE_UNUSED void *cookie)
{
  struct vec<Entity_Id,va_gc_atomic> * x ATTRIBUTE_UNUSED = (struct vec<Entity_Id,va_gc_atomic> *)x_p;
  if ((void *)(x) == this_obj)
    gt_pch_nx (&((*x)), op, cookie);
}

void
gt_pch_p_19tree_entity_vec_map (ATTRIBUTE_UNUSED void *this_obj,
	void *x_p,
	ATTRIBUTE_UNUSED gt_pointer_operator op,
	ATTRIBUTE_UNUSED void *cookie)
{
  struct tree_entity_vec_map * x ATTRIBUTE_UNUSED = (struct tree_entity_vec_map *)x_p;
  if ((void *)(x) == this_obj)
    op (&((*x).base.from), cookie);
  if ((void *)(x) == this_obj)
    op (&((*x).to), cookie);
}

void
gt_pch_nx (struct tree_entity_vec_map* x ATTRIBUTE_UNUSED,
	ATTRIBUTE_UNUSED gt_pointer_operator op,
	ATTRIBUTE_UNUSED void *cookie)
{
    op (&((*x).base.from), cookie);
    op (&((*x).to), cookie);
}

void
gt_pch_p_29hash_table_dummy_type_hasher_ (ATTRIBUTE_UNUSED void *this_obj,
	void *x_p,
	ATTRIBUTE_UNUSED gt_pointer_operator op,
	ATTRIBUTE_UNUSED void *cookie)
{
  struct hash_table<dummy_type_hasher> * x ATTRIBUTE_UNUSED = (struct hash_table<dummy_type_hasher> *)x_p;
  if ((void *)(x) == this_obj)
    gt_pch_nx (&((*x)), op, cookie);
}

void
gt_pch_nx (struct dummy_type_hasher* x ATTRIBUTE_UNUSED,
	ATTRIBUTE_UNUSED gt_pointer_operator op,
	ATTRIBUTE_UNUSED void *cookie)
{
}

/* GC roots.  */

EXPORTED_CONST struct ggc_root_tab gt_ggc_r_gt_ada_decl_h[] = {
  {
    &dummy_to_subprog_map,
    1,
    sizeof (dummy_to_subprog_map),
    &gt_ggc_mx_hash_table_dummy_type_hasher_,
    &gt_pch_nx_hash_table_dummy_type_hasher_
  },
  {
    &annotate_value_cache,
    1,
    sizeof (annotate_value_cache),
    &gt_ggc_mx_hash_table_value_annotation_hasher_,
    &gt_pch_nx_hash_table_value_annotation_hasher_
  },
  LAST_GGC_ROOT_TAB
};

void
gt_clear_caches_gt_ada_decl_h ()
{
  gt_cleare_cache (dummy_to_subprog_map);
  gt_cleare_cache (annotate_value_cache);
}

