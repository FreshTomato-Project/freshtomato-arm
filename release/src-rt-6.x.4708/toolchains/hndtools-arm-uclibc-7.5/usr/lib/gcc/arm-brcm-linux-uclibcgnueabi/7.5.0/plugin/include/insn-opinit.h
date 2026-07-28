/* Generated automatically by the program `genopinit'
   from the machine description file `md'.  */

#ifndef GCC_INSN_OPINIT_H
#define GCC_INSN_OPINIT_H 1
enum optab_tag {
  unknown_optab,
  sext_optab,
  trunc_optab,
  zext_optab,
  sfix_optab,
  ufix_optab,
  sfloat_optab,
  ufloat_optab,
  lrint_optab,
  lround_optab,
  lfloor_optab,
  lceil_optab,
  fract_optab,
  fractuns_optab,
  satfract_optab,
  satfractuns_optab,
  sfixtrunc_optab,
  ufixtrunc_optab,
  smul_widen_optab,
  umul_widen_optab,
  usmul_widen_optab,
  smadd_widen_optab,
  umadd_widen_optab,
  ssmadd_widen_optab,
  usmadd_widen_optab,
  smsub_widen_optab,
  umsub_widen_optab,
  ssmsub_widen_optab,
  usmsub_widen_optab,
  vec_load_lanes_optab,
  vec_store_lanes_optab,
  vcond_optab,
  vcondu_optab,
  vcondeq_optab,
  vcond_mask_optab,
  vec_cmp_optab,
  vec_cmpu_optab,
  vec_cmpeq_optab,
  maskload_optab,
  maskstore_optab,
  add_optab,
  addv_optab,
  ssadd_optab,
  usadd_optab,
  sub_optab,
  subv_optab,
  sssub_optab,
  ussub_optab,
  smul_optab,
  smulv_optab,
  ssmul_optab,
  usmul_optab,
  sdiv_optab,
  sdivv_optab,
  ssdiv_optab,
  udiv_optab,
  usdiv_optab,
  sdivmod_optab,
  udivmod_optab,
  smod_optab,
  umod_optab,
  ftrunc_optab,
  and_optab,
  ior_optab,
  xor_optab,
  ashl_optab,
  ssashl_optab,
  usashl_optab,
  ashr_optab,
  lshr_optab,
  rotl_optab,
  rotr_optab,
  vashl_optab,
  vashr_optab,
  vlshr_optab,
  vrotl_optab,
  vrotr_optab,
  smin_optab,
  smax_optab,
  umin_optab,
  umax_optab,
  neg_optab,
  negv_optab,
  ssneg_optab,
  usneg_optab,
  abs_optab,
  absv_optab,
  one_cmpl_optab,
  bswap_optab,
  ffs_optab,
  clz_optab,
  ctz_optab,
  clrsb_optab,
  popcount_optab,
  parity_optab,
  cmp_optab,
  ucmp_optab,
  eq_optab,
  ne_optab,
  gt_optab,
  ge_optab,
  lt_optab,
  le_optab,
  unord_optab,
  powi_optab,
  sqrt_optab,
  sync_old_add_optab,
  sync_old_sub_optab,
  sync_old_ior_optab,
  sync_old_and_optab,
  sync_old_xor_optab,
  sync_old_nand_optab,
  sync_new_add_optab,
  sync_new_sub_optab,
  sync_new_ior_optab,
  sync_new_and_optab,
  sync_new_xor_optab,
  sync_new_nand_optab,
  sync_compare_and_swap_optab,
  sync_lock_test_and_set_optab,
  mov_optab,
  movstrict_optab,
  movmisalign_optab,
  storent_optab,
  insv_optab,
  extv_optab,
  extzv_optab,
  insvmisalign_optab,
  extvmisalign_optab,
  extzvmisalign_optab,
  push_optab,
  reload_in_optab,
  reload_out_optab,
  cbranch_optab,
  addcc_optab,
  negcc_optab,
  notcc_optab,
  movcc_optab,
  cmov_optab,
  cstore_optab,
  ctrap_optab,
  addv4_optab,
  subv4_optab,
  mulv4_optab,
  uaddv4_optab,
  usubv4_optab,
  umulv4_optab,
  negv3_optab,
  addptr3_optab,
  smul_highpart_optab,
  umul_highpart_optab,
  cmpmem_optab,
  cmpstr_optab,
  cmpstrn_optab,
  movmem_optab,
  setmem_optab,
  strlen_optab,
  fma_optab,
  fms_optab,
  fnma_optab,
  fnms_optab,
  rint_optab,
  round_optab,
  floor_optab,
  ceil_optab,
  btrunc_optab,
  nearbyint_optab,
  acos_optab,
  asin_optab,
  atan2_optab,
  atan_optab,
  copysign_optab,
  cos_optab,
  exp10_optab,
  exp2_optab,
  exp_optab,
  expm1_optab,
  fmod_optab,
  ilogb_optab,
  isinf_optab,
  ldexp_optab,
  log10_optab,
  log1p_optab,
  log2_optab,
  log_optab,
  logb_optab,
  pow_optab,
  remainder_optab,
  rsqrt_optab,
  scalb_optab,
  signbit_optab,
  significand_optab,
  sin_optab,
  sincos_optab,
  tan_optab,
  fmax_optab,
  fmin_optab,
  reduc_smax_scal_optab,
  reduc_smin_scal_optab,
  reduc_plus_scal_optab,
  reduc_umax_scal_optab,
  reduc_umin_scal_optab,
  sdot_prod_optab,
  ssum_widen_optab,
  udot_prod_optab,
  usum_widen_optab,
  usad_optab,
  ssad_optab,
  vec_extract_optab,
  vec_init_optab,
  vec_pack_sfix_trunc_optab,
  vec_pack_ssat_optab,
  vec_pack_trunc_optab,
  vec_pack_ufix_trunc_optab,
  vec_pack_usat_optab,
  vec_perm_const_optab,
  vec_perm_optab,
  vec_realign_load_optab,
  vec_set_optab,
  vec_shr_optab,
  vec_unpacks_float_hi_optab,
  vec_unpacks_float_lo_optab,
  vec_unpacks_hi_optab,
  vec_unpacks_lo_optab,
  vec_unpacku_float_hi_optab,
  vec_unpacku_float_lo_optab,
  vec_unpacku_hi_optab,
  vec_unpacku_lo_optab,
  vec_widen_smult_even_optab,
  vec_widen_smult_hi_optab,
  vec_widen_smult_lo_optab,
  vec_widen_smult_odd_optab,
  vec_widen_sshiftl_hi_optab,
  vec_widen_sshiftl_lo_optab,
  vec_widen_umult_even_optab,
  vec_widen_umult_hi_optab,
  vec_widen_umult_lo_optab,
  vec_widen_umult_odd_optab,
  vec_widen_ushiftl_hi_optab,
  vec_widen_ushiftl_lo_optab,
  sync_add_optab,
  sync_and_optab,
  sync_ior_optab,
  sync_lock_release_optab,
  sync_nand_optab,
  sync_sub_optab,
  sync_xor_optab,
  atomic_add_fetch_optab,
  atomic_add_optab,
  atomic_and_fetch_optab,
  atomic_and_optab,
  atomic_bit_test_and_set_optab,
  atomic_bit_test_and_complement_optab,
  atomic_bit_test_and_reset_optab,
  atomic_compare_and_swap_optab,
  atomic_exchange_optab,
  atomic_fetch_add_optab,
  atomic_fetch_and_optab,
  atomic_fetch_nand_optab,
  atomic_fetch_or_optab,
  atomic_fetch_sub_optab,
  atomic_fetch_xor_optab,
  atomic_load_optab,
  atomic_nand_fetch_optab,
  atomic_nand_optab,
  atomic_or_fetch_optab,
  atomic_or_optab,
  atomic_store_optab,
  atomic_sub_fetch_optab,
  atomic_sub_optab,
  atomic_xor_fetch_optab,
  atomic_xor_optab,
  get_thread_pointer_optab,
  set_thread_pointer_optab,
  FIRST_CONV_OPTAB = sext_optab,
  LAST_CONVLIB_OPTAB = satfractuns_optab,
  LAST_CONV_OPTAB = maskstore_optab,
  FIRST_NORM_OPTAB = add_optab,
  LAST_NORMLIB_OPTAB = sync_lock_test_and_set_optab,
  LAST_NORM_OPTAB = set_thread_pointer_optab
};

#define NUM_OPTABS          274
#define NUM_CONVLIB_OPTABS  15
#define NUM_NORMLIB_OPTABS  80
#define NUM_OPTAB_PATTERNS  781
typedef enum optab_tag optab;
typedef enum optab_tag convert_optab;
typedef enum optab_tag direct_optab;

struct optab_libcall_d
{
  char libcall_suffix;
  const char *libcall_basename;
  void (*libcall_gen) (optab, const char *name,
		       char suffix, machine_mode);
};

struct convert_optab_libcall_d
{
  const char *libcall_basename;
  void (*libcall_gen) (convert_optab, const char *name,
		       machine_mode, machine_mode);
};

/* Given an enum insn_code, access the function to construct
   the body of that kind of insn.  */
#define GEN_FCN(CODE) (insn_data[CODE].genfun)

#ifdef NUM_RTX_CODE
/* Contains the optab used for each rtx code, and vice-versa.  */
extern const optab code_to_optab_[NUM_RTX_CODE];
extern const enum rtx_code optab_to_code_[NUM_OPTABS];

static inline optab
code_to_optab (enum rtx_code code)
{
  return code_to_optab_[code];
}

static inline enum rtx_code
optab_to_code (optab op)
{
  return optab_to_code_[op];
}
#endif

extern const struct convert_optab_libcall_d convlib_def[NUM_CONVLIB_OPTABS];
extern const struct optab_libcall_d normlib_def[NUM_NORMLIB_OPTABS];

/* Returns the active icode for the given (encoded) optab.  */
extern enum insn_code raw_optab_handler (unsigned);
extern bool swap_optab_enable (optab, machine_mode, bool);

/* Target-dependent globals.  */
struct target_optabs {
  /* Patterns that are used by optabs that are enabled for this target.  */
  bool pat_enable[NUM_OPTAB_PATTERNS];
};
extern void init_all_optabs (struct target_optabs *);

extern struct target_optabs default_target_optabs;
extern struct target_optabs *this_fn_optabs;
#if SWITCHABLE_TARGET
extern struct target_optabs *this_target_optabs;
#else
#define this_target_optabs (&default_target_optabs)
#endif
#endif
