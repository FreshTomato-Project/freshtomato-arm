/* Generated automatically by the program `gentarget-def'.  */
#ifndef GCC_INSN_TARGET_DEF_H
#define GCC_INSN_TARGET_DEF_H

static inline rtx_insn *
insnify (rtx x)
{
  if (!x)
    return NULL;
  if (rtx_insn *insn = dyn_cast <rtx_insn *> (x))
    return insn;
  start_sequence ();
  emit (x, false);
  rtx_insn *res = get_insns ();
  end_sequence ();
  return res;
}

static rtx_insn *
invalid_rtx_rtx (rtx, rtx)
{
  gcc_unreachable ();
}

#undef TARGET_HAVE_ALLOCATE_STACK
#define TARGET_HAVE_ALLOCATE_STACK hook_bool_void_false
#undef TARGET_GEN_ALLOCATE_STACK
#define TARGET_GEN_ALLOCATE_STACK invalid_rtx_rtx
#undef TARGET_CODE_FOR_ALLOCATE_STACK
#define TARGET_CODE_FOR_ALLOCATE_STACK CODE_FOR_nothing

static rtx_insn *
invalid_rtx_rtx_rtx (rtx, rtx, rtx)
{
  gcc_unreachable ();
}

#undef TARGET_HAVE_ATOMIC_TEST_AND_SET
#define TARGET_HAVE_ATOMIC_TEST_AND_SET hook_bool_void_false
#undef TARGET_GEN_ATOMIC_TEST_AND_SET
#define TARGET_GEN_ATOMIC_TEST_AND_SET invalid_rtx_rtx_rtx
#undef TARGET_CODE_FOR_ATOMIC_TEST_AND_SET
#define TARGET_CODE_FOR_ATOMIC_TEST_AND_SET CODE_FOR_nothing

static rtx_insn *
invalid_rtx (rtx)
{
  gcc_unreachable ();
}

#undef TARGET_HAVE_BUILTIN_LONGJMP
#define TARGET_HAVE_BUILTIN_LONGJMP hook_bool_void_false
#undef TARGET_GEN_BUILTIN_LONGJMP
#define TARGET_GEN_BUILTIN_LONGJMP invalid_rtx
#undef TARGET_CODE_FOR_BUILTIN_LONGJMP
#define TARGET_CODE_FOR_BUILTIN_LONGJMP CODE_FOR_nothing

static bool
target_have_builtin_setjmp_receiver (void)
{
  return 
#line 6323 "../../gcc/config/arm/arm.md"
(flag_pic);
}

static rtx_insn *
target_gen_builtin_setjmp_receiver (rtx x0)
{
  gcc_checking_assert (targetm.have_builtin_setjmp_receiver ());
  return insnify (gen_builtin_setjmp_receiver (x0));
}

#undef TARGET_HAVE_BUILTIN_SETJMP_RECEIVER
#define TARGET_HAVE_BUILTIN_SETJMP_RECEIVER target_have_builtin_setjmp_receiver
#undef TARGET_GEN_BUILTIN_SETJMP_RECEIVER
#define TARGET_GEN_BUILTIN_SETJMP_RECEIVER target_gen_builtin_setjmp_receiver
#undef TARGET_CODE_FOR_BUILTIN_SETJMP_RECEIVER
#define TARGET_CODE_FOR_BUILTIN_SETJMP_RECEIVER CODE_FOR_builtin_setjmp_receiver

#undef TARGET_HAVE_BUILTIN_SETJMP_SETUP
#define TARGET_HAVE_BUILTIN_SETJMP_SETUP hook_bool_void_false
#undef TARGET_GEN_BUILTIN_SETJMP_SETUP
#define TARGET_GEN_BUILTIN_SETJMP_SETUP invalid_rtx
#undef TARGET_CODE_FOR_BUILTIN_SETJMP_SETUP
#define TARGET_CODE_FOR_BUILTIN_SETJMP_SETUP CODE_FOR_nothing

#undef TARGET_HAVE_CANONICALIZE_FUNCPTR_FOR_COMPARE
#define TARGET_HAVE_CANONICALIZE_FUNCPTR_FOR_COMPARE hook_bool_void_false
#undef TARGET_GEN_CANONICALIZE_FUNCPTR_FOR_COMPARE
#define TARGET_GEN_CANONICALIZE_FUNCPTR_FOR_COMPARE invalid_rtx_rtx
#undef TARGET_CODE_FOR_CANONICALIZE_FUNCPTR_FOR_COMPARE
#define TARGET_CODE_FOR_CANONICALIZE_FUNCPTR_FOR_COMPARE CODE_FOR_nothing

static rtx_insn *
target_gen_call (rtx x0, rtx opt1, rtx opt2, rtx)
{
  return insnify (gen_call (x0, opt1, opt2));
}

#undef TARGET_HAVE_CALL
#define TARGET_HAVE_CALL hook_bool_void_true
#undef TARGET_GEN_CALL
#define TARGET_GEN_CALL target_gen_call
#undef TARGET_CODE_FOR_CALL
#define TARGET_CODE_FOR_CALL CODE_FOR_call

static rtx_insn *
invalid_rtx_rtx_rtx_rtx (rtx, rtx, rtx, rtx)
{
  gcc_unreachable ();
}

#undef TARGET_HAVE_CALL_POP
#define TARGET_HAVE_CALL_POP hook_bool_void_false
#undef TARGET_GEN_CALL_POP
#define TARGET_GEN_CALL_POP invalid_rtx_rtx_rtx_rtx
#undef TARGET_CODE_FOR_CALL_POP
#define TARGET_CODE_FOR_CALL_POP CODE_FOR_nothing

static rtx_insn *
target_gen_call_value (rtx x0, rtx x1, rtx opt2, rtx opt3, rtx)
{
  return insnify (gen_call_value (x0, x1, opt2, opt3));
}

#undef TARGET_HAVE_CALL_VALUE
#define TARGET_HAVE_CALL_VALUE hook_bool_void_true
#undef TARGET_GEN_CALL_VALUE
#define TARGET_GEN_CALL_VALUE target_gen_call_value
#undef TARGET_CODE_FOR_CALL_VALUE
#define TARGET_CODE_FOR_CALL_VALUE CODE_FOR_call_value

static rtx_insn *
invalid_rtx_rtx_rtx_rtx_rtx (rtx, rtx, rtx, rtx, rtx)
{
  gcc_unreachable ();
}

#undef TARGET_HAVE_CALL_VALUE_POP
#define TARGET_HAVE_CALL_VALUE_POP hook_bool_void_false
#undef TARGET_GEN_CALL_VALUE_POP
#define TARGET_GEN_CALL_VALUE_POP invalid_rtx_rtx_rtx_rtx_rtx
#undef TARGET_CODE_FOR_CALL_VALUE_POP
#define TARGET_CODE_FOR_CALL_VALUE_POP CODE_FOR_nothing

static bool
target_have_casesi (void)
{
  return 
#line 8661 "../../gcc/config/arm/arm.md"
((TARGET_32BIT || optimize_size || flag_pic) && !target_pure_code);
}

static rtx_insn *
target_gen_casesi (rtx x0, rtx x1, rtx x2, rtx x3, rtx x4)
{
  gcc_checking_assert (targetm.have_casesi ());
  return insnify (gen_casesi (x0, x1, x2, x3, x4));
}

#undef TARGET_HAVE_CASESI
#define TARGET_HAVE_CASESI target_have_casesi
#undef TARGET_GEN_CASESI
#define TARGET_GEN_CASESI target_gen_casesi
#undef TARGET_CODE_FOR_CASESI
#define TARGET_CODE_FOR_CASESI CODE_FOR_casesi

#undef TARGET_HAVE_CHECK_STACK
#define TARGET_HAVE_CHECK_STACK hook_bool_void_false
#undef TARGET_GEN_CHECK_STACK
#define TARGET_GEN_CHECK_STACK invalid_rtx
#undef TARGET_CODE_FOR_CHECK_STACK
#define TARGET_CODE_FOR_CHECK_STACK CODE_FOR_nothing

#undef TARGET_HAVE_CLEAR_CACHE
#define TARGET_HAVE_CLEAR_CACHE hook_bool_void_false
#undef TARGET_GEN_CLEAR_CACHE
#define TARGET_GEN_CLEAR_CACHE invalid_rtx_rtx
#undef TARGET_CODE_FOR_CLEAR_CACHE
#define TARGET_CODE_FOR_CLEAR_CACHE CODE_FOR_nothing

#undef TARGET_HAVE_DOLOOP_BEGIN
#define TARGET_HAVE_DOLOOP_BEGIN hook_bool_void_false
#undef TARGET_GEN_DOLOOP_BEGIN
#define TARGET_GEN_DOLOOP_BEGIN invalid_rtx_rtx
#undef TARGET_CODE_FOR_DOLOOP_BEGIN
#define TARGET_CODE_FOR_DOLOOP_BEGIN CODE_FOR_nothing

static bool
target_have_doloop_end (void)
{
  return 
#line 1620 "../../gcc/config/arm/thumb2.md"
(TARGET_32BIT);
}

static rtx_insn *
target_gen_doloop_end (rtx x0, rtx x1)
{
  gcc_checking_assert (targetm.have_doloop_end ());
  return insnify (gen_doloop_end (x0, x1));
}

#undef TARGET_HAVE_DOLOOP_END
#define TARGET_HAVE_DOLOOP_END target_have_doloop_end
#undef TARGET_GEN_DOLOOP_END
#define TARGET_GEN_DOLOOP_END target_gen_doloop_end
#undef TARGET_CODE_FOR_DOLOOP_END
#define TARGET_CODE_FOR_DOLOOP_END CODE_FOR_doloop_end

static rtx_insn *
target_gen_eh_return (rtx x0)
{
  return insnify (gen_eh_return (x0));
}

#undef TARGET_HAVE_EH_RETURN
#define TARGET_HAVE_EH_RETURN hook_bool_void_true
#undef TARGET_GEN_EH_RETURN
#define TARGET_GEN_EH_RETURN target_gen_eh_return
#undef TARGET_CODE_FOR_EH_RETURN
#define TARGET_CODE_FOR_EH_RETURN CODE_FOR_eh_return

static rtx_insn *
target_gen_epilogue (void)
{
  return insnify (gen_epilogue ());
}

#undef TARGET_HAVE_EPILOGUE
#define TARGET_HAVE_EPILOGUE hook_bool_void_true
#undef TARGET_GEN_EPILOGUE
#define TARGET_GEN_EPILOGUE target_gen_epilogue
#undef TARGET_CODE_FOR_EPILOGUE
#define TARGET_CODE_FOR_EPILOGUE CODE_FOR_epilogue

static rtx_insn *
invalid_void (void)
{
  gcc_unreachable ();
}

#undef TARGET_HAVE_EXCEPTION_RECEIVER
#define TARGET_HAVE_EXCEPTION_RECEIVER hook_bool_void_false
#undef TARGET_GEN_EXCEPTION_RECEIVER
#define TARGET_GEN_EXCEPTION_RECEIVER invalid_void
#undef TARGET_CODE_FOR_EXCEPTION_RECEIVER
#define TARGET_CODE_FOR_EXCEPTION_RECEIVER CODE_FOR_nothing

static bool
target_have_extv (void)
{
  return 
#line 4421 "../../gcc/config/arm/arm.md"
(arm_arch_thumb2);
}

static rtx_insn *
target_gen_extv (rtx x0, rtx x1, rtx x2, rtx x3)
{
  gcc_checking_assert (targetm.have_extv ());
  return insnify (gen_extv (x0, x1, x2, x3));
}

#undef TARGET_HAVE_EXTV
#define TARGET_HAVE_EXTV target_have_extv
#undef TARGET_GEN_EXTV
#define TARGET_GEN_EXTV target_gen_extv
#undef TARGET_CODE_FOR_EXTV
#define TARGET_CODE_FOR_EXTV CODE_FOR_extv

static bool
target_have_extzv (void)
{
  return 
#line 4330 "../../gcc/config/arm/arm.md"
(TARGET_THUMB1 || arm_arch_thumb2);
}

static rtx_insn *
target_gen_extzv (rtx x0, rtx x1, rtx x2, rtx x3)
{
  gcc_checking_assert (targetm.have_extzv ());
  return insnify (gen_extzv (x0, x1, x2, x3));
}

#undef TARGET_HAVE_EXTZV
#define TARGET_HAVE_EXTZV target_have_extzv
#undef TARGET_GEN_EXTZV
#define TARGET_GEN_EXTZV target_gen_extzv
#undef TARGET_CODE_FOR_EXTZV
#define TARGET_CODE_FOR_EXTZV CODE_FOR_extzv

static rtx_insn *
target_gen_indirect_jump (rtx x0)
{
  return insnify (gen_indirect_jump (x0));
}

#undef TARGET_HAVE_INDIRECT_JUMP
#define TARGET_HAVE_INDIRECT_JUMP hook_bool_void_true
#undef TARGET_GEN_INDIRECT_JUMP
#define TARGET_GEN_INDIRECT_JUMP target_gen_indirect_jump
#undef TARGET_CODE_FOR_INDIRECT_JUMP
#define TARGET_CODE_FOR_INDIRECT_JUMP CODE_FOR_indirect_jump

static bool
target_have_insv (void)
{
  return 
#line 2699 "../../gcc/config/arm/arm.md"
(TARGET_ARM || arm_arch_thumb2);
}

static rtx_insn *
target_gen_insv (rtx x0, rtx x1, rtx x2, rtx x3)
{
  gcc_checking_assert (targetm.have_insv ());
  return insnify (gen_insv (x0, x1, x2, x3));
}

#undef TARGET_HAVE_INSV
#define TARGET_HAVE_INSV target_have_insv
#undef TARGET_GEN_INSV
#define TARGET_GEN_INSV target_gen_insv
#undef TARGET_CODE_FOR_INSV
#define TARGET_CODE_FOR_INSV CODE_FOR_insv

static rtx_insn *
target_gen_jump (rtx x0)
{
  return insnify (gen_jump (x0));
}

#undef TARGET_HAVE_JUMP
#define TARGET_HAVE_JUMP hook_bool_void_true
#undef TARGET_GEN_JUMP
#define TARGET_GEN_JUMP target_gen_jump
#undef TARGET_CODE_FOR_JUMP
#define TARGET_CODE_FOR_JUMP CODE_FOR_jump

static rtx_insn *
target_gen_load_multiple (rtx x0, rtx x1, rtx x2)
{
  gcc_checking_assert (targetm.have_load_multiple ());
  return insnify (gen_load_multiple (x0, x1, x2));
}

#undef TARGET_HAVE_LOAD_MULTIPLE
#define TARGET_HAVE_LOAD_MULTIPLE target_have_doloop_end
#undef TARGET_GEN_LOAD_MULTIPLE
#define TARGET_GEN_LOAD_MULTIPLE target_gen_load_multiple
#undef TARGET_CODE_FOR_LOAD_MULTIPLE
#define TARGET_CODE_FOR_LOAD_MULTIPLE CODE_FOR_load_multiple

#undef TARGET_HAVE_MEM_SIGNAL_FENCE
#define TARGET_HAVE_MEM_SIGNAL_FENCE hook_bool_void_false
#undef TARGET_GEN_MEM_SIGNAL_FENCE
#define TARGET_GEN_MEM_SIGNAL_FENCE invalid_rtx
#undef TARGET_CODE_FOR_MEM_SIGNAL_FENCE
#define TARGET_CODE_FOR_MEM_SIGNAL_FENCE CODE_FOR_nothing

#undef TARGET_HAVE_MEM_THREAD_FENCE
#define TARGET_HAVE_MEM_THREAD_FENCE hook_bool_void_false
#undef TARGET_GEN_MEM_THREAD_FENCE
#define TARGET_GEN_MEM_THREAD_FENCE invalid_rtx
#undef TARGET_CODE_FOR_MEM_THREAD_FENCE
#define TARGET_CODE_FOR_MEM_THREAD_FENCE CODE_FOR_nothing

static bool
target_have_memory_barrier (void)
{
  return 
#line 40 "../../gcc/config/arm/sync.md"
(TARGET_HAVE_MEMORY_BARRIER);
}

static rtx_insn *
target_gen_memory_barrier (void)
{
  gcc_checking_assert (targetm.have_memory_barrier ());
  return insnify (gen_memory_barrier ());
}

#undef TARGET_HAVE_MEMORY_BARRIER
#define TARGET_HAVE_MEMORY_BARRIER target_have_memory_barrier
#undef TARGET_GEN_MEMORY_BARRIER
#define TARGET_GEN_MEMORY_BARRIER target_gen_memory_barrier
#undef TARGET_CODE_FOR_MEMORY_BARRIER
#define TARGET_CODE_FOR_MEMORY_BARRIER CODE_FOR_memory_barrier

#undef TARGET_HAVE_MOVSTR
#define TARGET_HAVE_MOVSTR hook_bool_void_false
#undef TARGET_GEN_MOVSTR
#define TARGET_GEN_MOVSTR invalid_rtx_rtx_rtx
#undef TARGET_CODE_FOR_MOVSTR
#define TARGET_CODE_FOR_MOVSTR CODE_FOR_nothing

#undef TARGET_HAVE_NONLOCAL_GOTO
#define TARGET_HAVE_NONLOCAL_GOTO hook_bool_void_false
#undef TARGET_GEN_NONLOCAL_GOTO
#define TARGET_GEN_NONLOCAL_GOTO invalid_rtx_rtx_rtx_rtx
#undef TARGET_CODE_FOR_NONLOCAL_GOTO
#define TARGET_CODE_FOR_NONLOCAL_GOTO CODE_FOR_nothing

#undef TARGET_HAVE_NONLOCAL_GOTO_RECEIVER
#define TARGET_HAVE_NONLOCAL_GOTO_RECEIVER hook_bool_void_false
#undef TARGET_GEN_NONLOCAL_GOTO_RECEIVER
#define TARGET_GEN_NONLOCAL_GOTO_RECEIVER invalid_void
#undef TARGET_CODE_FOR_NONLOCAL_GOTO_RECEIVER
#define TARGET_CODE_FOR_NONLOCAL_GOTO_RECEIVER CODE_FOR_nothing

#undef TARGET_HAVE_OACC_DIM_POS
#define TARGET_HAVE_OACC_DIM_POS hook_bool_void_false
#undef TARGET_GEN_OACC_DIM_POS
#define TARGET_GEN_OACC_DIM_POS invalid_rtx_rtx
#undef TARGET_CODE_FOR_OACC_DIM_POS
#define TARGET_CODE_FOR_OACC_DIM_POS CODE_FOR_nothing

#undef TARGET_HAVE_OACC_DIM_SIZE
#define TARGET_HAVE_OACC_DIM_SIZE hook_bool_void_false
#undef TARGET_GEN_OACC_DIM_SIZE
#define TARGET_GEN_OACC_DIM_SIZE invalid_rtx_rtx
#undef TARGET_CODE_FOR_OACC_DIM_SIZE
#define TARGET_CODE_FOR_OACC_DIM_SIZE CODE_FOR_nothing

#undef TARGET_HAVE_OACC_FORK
#define TARGET_HAVE_OACC_FORK hook_bool_void_false
#undef TARGET_GEN_OACC_FORK
#define TARGET_GEN_OACC_FORK invalid_rtx_rtx_rtx
#undef TARGET_CODE_FOR_OACC_FORK
#define TARGET_CODE_FOR_OACC_FORK CODE_FOR_nothing

#undef TARGET_HAVE_OACC_JOIN
#define TARGET_HAVE_OACC_JOIN hook_bool_void_false
#undef TARGET_GEN_OACC_JOIN
#define TARGET_GEN_OACC_JOIN invalid_rtx_rtx_rtx
#undef TARGET_CODE_FOR_OACC_JOIN
#define TARGET_CODE_FOR_OACC_JOIN CODE_FOR_nothing

#undef TARGET_HAVE_OMP_SIMT_ENTER
#define TARGET_HAVE_OMP_SIMT_ENTER hook_bool_void_false
#undef TARGET_GEN_OMP_SIMT_ENTER
#define TARGET_GEN_OMP_SIMT_ENTER invalid_rtx_rtx_rtx
#undef TARGET_CODE_FOR_OMP_SIMT_ENTER
#define TARGET_CODE_FOR_OMP_SIMT_ENTER CODE_FOR_nothing

#undef TARGET_HAVE_OMP_SIMT_EXIT
#define TARGET_HAVE_OMP_SIMT_EXIT hook_bool_void_false
#undef TARGET_GEN_OMP_SIMT_EXIT
#define TARGET_GEN_OMP_SIMT_EXIT invalid_rtx
#undef TARGET_CODE_FOR_OMP_SIMT_EXIT
#define TARGET_CODE_FOR_OMP_SIMT_EXIT CODE_FOR_nothing

#undef TARGET_HAVE_OMP_SIMT_LANE
#define TARGET_HAVE_OMP_SIMT_LANE hook_bool_void_false
#undef TARGET_GEN_OMP_SIMT_LANE
#define TARGET_GEN_OMP_SIMT_LANE invalid_rtx
#undef TARGET_CODE_FOR_OMP_SIMT_LANE
#define TARGET_CODE_FOR_OMP_SIMT_LANE CODE_FOR_nothing

#undef TARGET_HAVE_OMP_SIMT_LAST_LANE
#define TARGET_HAVE_OMP_SIMT_LAST_LANE hook_bool_void_false
#undef TARGET_GEN_OMP_SIMT_LAST_LANE
#define TARGET_GEN_OMP_SIMT_LAST_LANE invalid_rtx_rtx
#undef TARGET_CODE_FOR_OMP_SIMT_LAST_LANE
#define TARGET_CODE_FOR_OMP_SIMT_LAST_LANE CODE_FOR_nothing

#undef TARGET_HAVE_OMP_SIMT_ORDERED
#define TARGET_HAVE_OMP_SIMT_ORDERED hook_bool_void_false
#undef TARGET_GEN_OMP_SIMT_ORDERED
#define TARGET_GEN_OMP_SIMT_ORDERED invalid_rtx_rtx
#undef TARGET_CODE_FOR_OMP_SIMT_ORDERED
#define TARGET_CODE_FOR_OMP_SIMT_ORDERED CODE_FOR_nothing

#undef TARGET_HAVE_OMP_SIMT_VOTE_ANY
#define TARGET_HAVE_OMP_SIMT_VOTE_ANY hook_bool_void_false
#undef TARGET_GEN_OMP_SIMT_VOTE_ANY
#define TARGET_GEN_OMP_SIMT_VOTE_ANY invalid_rtx_rtx
#undef TARGET_CODE_FOR_OMP_SIMT_VOTE_ANY
#define TARGET_CODE_FOR_OMP_SIMT_VOTE_ANY CODE_FOR_nothing

#undef TARGET_HAVE_OMP_SIMT_XCHG_BFLY
#define TARGET_HAVE_OMP_SIMT_XCHG_BFLY hook_bool_void_false
#undef TARGET_GEN_OMP_SIMT_XCHG_BFLY
#define TARGET_GEN_OMP_SIMT_XCHG_BFLY invalid_rtx_rtx_rtx
#undef TARGET_CODE_FOR_OMP_SIMT_XCHG_BFLY
#define TARGET_CODE_FOR_OMP_SIMT_XCHG_BFLY CODE_FOR_nothing

#undef TARGET_HAVE_OMP_SIMT_XCHG_IDX
#define TARGET_HAVE_OMP_SIMT_XCHG_IDX hook_bool_void_false
#undef TARGET_GEN_OMP_SIMT_XCHG_IDX
#define TARGET_GEN_OMP_SIMT_XCHG_IDX invalid_rtx_rtx_rtx
#undef TARGET_CODE_FOR_OMP_SIMT_XCHG_IDX
#define TARGET_CODE_FOR_OMP_SIMT_XCHG_IDX CODE_FOR_nothing

static bool
target_have_prefetch (void)
{
  return 
#line 11336 "../../gcc/config/arm/arm.md"
(TARGET_32BIT && arm_arch5e);
}

static rtx_insn *
target_gen_prefetch (rtx x0, rtx x1, rtx x2)
{
  gcc_checking_assert (targetm.have_prefetch ());
  return insnify (gen_prefetch (x0, x1, x2));
}

#undef TARGET_HAVE_PREFETCH
#define TARGET_HAVE_PREFETCH target_have_prefetch
#undef TARGET_GEN_PREFETCH
#define TARGET_GEN_PREFETCH target_gen_prefetch
#undef TARGET_CODE_FOR_PREFETCH
#define TARGET_CODE_FOR_PREFETCH CODE_FOR_prefetch

static rtx_insn *
target_gen_probe_stack (rtx x0)
{
  gcc_checking_assert (targetm.have_probe_stack ());
  return insnify (gen_probe_stack (x0));
}

#undef TARGET_HAVE_PROBE_STACK
#define TARGET_HAVE_PROBE_STACK target_have_doloop_end
#undef TARGET_GEN_PROBE_STACK
#define TARGET_GEN_PROBE_STACK target_gen_probe_stack
#undef TARGET_CODE_FOR_PROBE_STACK
#define TARGET_CODE_FOR_PROBE_STACK CODE_FOR_probe_stack

#undef TARGET_HAVE_PROBE_STACK_ADDRESS
#define TARGET_HAVE_PROBE_STACK_ADDRESS hook_bool_void_false
#undef TARGET_GEN_PROBE_STACK_ADDRESS
#define TARGET_GEN_PROBE_STACK_ADDRESS invalid_rtx
#undef TARGET_CODE_FOR_PROBE_STACK_ADDRESS
#define TARGET_CODE_FOR_PROBE_STACK_ADDRESS CODE_FOR_nothing

static rtx_insn *
target_gen_prologue (void)
{
  return insnify (gen_prologue ());
}

#undef TARGET_HAVE_PROLOGUE
#define TARGET_HAVE_PROLOGUE hook_bool_void_true
#undef TARGET_GEN_PROLOGUE
#define TARGET_GEN_PROLOGUE target_gen_prologue
#undef TARGET_CODE_FOR_PROLOGUE
#define TARGET_CODE_FOR_PROLOGUE CODE_FOR_prologue

#undef TARGET_HAVE_PTR_EXTEND
#define TARGET_HAVE_PTR_EXTEND hook_bool_void_false
#undef TARGET_GEN_PTR_EXTEND
#define TARGET_GEN_PTR_EXTEND invalid_rtx_rtx
#undef TARGET_CODE_FOR_PTR_EXTEND
#define TARGET_CODE_FOR_PTR_EXTEND CODE_FOR_nothing

#undef TARGET_HAVE_RELOAD_LOAD_ADDRESS
#define TARGET_HAVE_RELOAD_LOAD_ADDRESS hook_bool_void_false
#undef TARGET_GEN_RELOAD_LOAD_ADDRESS
#define TARGET_GEN_RELOAD_LOAD_ADDRESS invalid_rtx_rtx
#undef TARGET_CODE_FOR_RELOAD_LOAD_ADDRESS
#define TARGET_CODE_FOR_RELOAD_LOAD_ADDRESS CODE_FOR_nothing

#undef TARGET_HAVE_RESTORE_STACK_BLOCK
#define TARGET_HAVE_RESTORE_STACK_BLOCK hook_bool_void_false
#undef TARGET_GEN_RESTORE_STACK_BLOCK
#define TARGET_GEN_RESTORE_STACK_BLOCK invalid_rtx_rtx
#undef TARGET_CODE_FOR_RESTORE_STACK_BLOCK
#define TARGET_CODE_FOR_RESTORE_STACK_BLOCK CODE_FOR_nothing

#undef TARGET_HAVE_RESTORE_STACK_FUNCTION
#define TARGET_HAVE_RESTORE_STACK_FUNCTION hook_bool_void_false
#undef TARGET_GEN_RESTORE_STACK_FUNCTION
#define TARGET_GEN_RESTORE_STACK_FUNCTION invalid_rtx_rtx
#undef TARGET_CODE_FOR_RESTORE_STACK_FUNCTION
#define TARGET_CODE_FOR_RESTORE_STACK_FUNCTION CODE_FOR_nothing

#undef TARGET_HAVE_RESTORE_STACK_NONLOCAL
#define TARGET_HAVE_RESTORE_STACK_NONLOCAL hook_bool_void_false
#undef TARGET_GEN_RESTORE_STACK_NONLOCAL
#define TARGET_GEN_RESTORE_STACK_NONLOCAL invalid_rtx_rtx
#undef TARGET_CODE_FOR_RESTORE_STACK_NONLOCAL
#define TARGET_CODE_FOR_RESTORE_STACK_NONLOCAL CODE_FOR_nothing

static bool
target_have_return (void)
{
  return 
#line 8381 "../../gcc/config/arm/arm.md"
((TARGET_ARM || (TARGET_THUMB2
                   && ARM_FUNC_TYPE (arm_current_func_type ()) == ARM_FT_NORMAL
                   && !IS_STACKALIGN (arm_current_func_type ())))
     && USE_RETURN_INSN (FALSE));
}

static rtx_insn *
target_gen_return (void)
{
  gcc_checking_assert (targetm.have_return ());
  return insnify (gen_return ());
}

#undef TARGET_HAVE_RETURN
#define TARGET_HAVE_RETURN target_have_return
#undef TARGET_GEN_RETURN
#define TARGET_GEN_RETURN target_gen_return
#undef TARGET_CODE_FOR_RETURN
#define TARGET_CODE_FOR_RETURN CODE_FOR_return

#undef TARGET_HAVE_SAVE_STACK_BLOCK
#define TARGET_HAVE_SAVE_STACK_BLOCK hook_bool_void_false
#undef TARGET_GEN_SAVE_STACK_BLOCK
#define TARGET_GEN_SAVE_STACK_BLOCK invalid_rtx_rtx
#undef TARGET_CODE_FOR_SAVE_STACK_BLOCK
#define TARGET_CODE_FOR_SAVE_STACK_BLOCK CODE_FOR_nothing

#undef TARGET_HAVE_SAVE_STACK_FUNCTION
#define TARGET_HAVE_SAVE_STACK_FUNCTION hook_bool_void_false
#undef TARGET_GEN_SAVE_STACK_FUNCTION
#define TARGET_GEN_SAVE_STACK_FUNCTION invalid_rtx_rtx
#undef TARGET_CODE_FOR_SAVE_STACK_FUNCTION
#define TARGET_CODE_FOR_SAVE_STACK_FUNCTION CODE_FOR_nothing

#undef TARGET_HAVE_SAVE_STACK_NONLOCAL
#define TARGET_HAVE_SAVE_STACK_NONLOCAL hook_bool_void_false
#undef TARGET_GEN_SAVE_STACK_NONLOCAL
#define TARGET_GEN_SAVE_STACK_NONLOCAL invalid_rtx_rtx
#undef TARGET_CODE_FOR_SAVE_STACK_NONLOCAL
#define TARGET_CODE_FOR_SAVE_STACK_NONLOCAL CODE_FOR_nothing

static rtx_insn *
target_gen_sibcall (rtx x0, rtx opt1, rtx opt2, rtx)
{
  gcc_checking_assert (targetm.have_sibcall ());
  return insnify (gen_sibcall (x0, opt1, opt2));
}

#undef TARGET_HAVE_SIBCALL
#define TARGET_HAVE_SIBCALL target_have_doloop_end
#undef TARGET_GEN_SIBCALL
#define TARGET_GEN_SIBCALL target_gen_sibcall
#undef TARGET_CODE_FOR_SIBCALL
#define TARGET_CODE_FOR_SIBCALL CODE_FOR_sibcall

static rtx_insn *
target_gen_sibcall_epilogue (void)
{
  gcc_checking_assert (targetm.have_sibcall_epilogue ());
  return insnify (gen_sibcall_epilogue ());
}

#undef TARGET_HAVE_SIBCALL_EPILOGUE
#define TARGET_HAVE_SIBCALL_EPILOGUE target_have_doloop_end
#undef TARGET_GEN_SIBCALL_EPILOGUE
#define TARGET_GEN_SIBCALL_EPILOGUE target_gen_sibcall_epilogue
#undef TARGET_CODE_FOR_SIBCALL_EPILOGUE
#define TARGET_CODE_FOR_SIBCALL_EPILOGUE CODE_FOR_sibcall_epilogue

static rtx_insn *
target_gen_sibcall_value (rtx x0, rtx x1, rtx opt2, rtx opt3, rtx)
{
  gcc_checking_assert (targetm.have_sibcall_value ());
  return insnify (gen_sibcall_value (x0, x1, opt2, opt3));
}

#undef TARGET_HAVE_SIBCALL_VALUE
#define TARGET_HAVE_SIBCALL_VALUE target_have_doloop_end
#undef TARGET_GEN_SIBCALL_VALUE
#define TARGET_GEN_SIBCALL_VALUE target_gen_sibcall_value
#undef TARGET_CODE_FOR_SIBCALL_VALUE
#define TARGET_CODE_FOR_SIBCALL_VALUE CODE_FOR_sibcall_value

static bool
target_have_simple_return (void)
{
  return 
#line 8381 "../../gcc/config/arm/arm.md"
((TARGET_ARM || (TARGET_THUMB2
                   && ARM_FUNC_TYPE (arm_current_func_type ()) == ARM_FT_NORMAL
                   && !IS_STACKALIGN (arm_current_func_type ())))
     && use_simple_return_p ());
}

static rtx_insn *
target_gen_simple_return (void)
{
  gcc_checking_assert (targetm.have_simple_return ());
  return insnify (gen_simple_return ());
}

#undef TARGET_HAVE_SIMPLE_RETURN
#define TARGET_HAVE_SIMPLE_RETURN target_have_simple_return
#undef TARGET_GEN_SIMPLE_RETURN
#define TARGET_GEN_SIMPLE_RETURN target_gen_simple_return
#undef TARGET_CODE_FOR_SIMPLE_RETURN
#define TARGET_CODE_FOR_SIMPLE_RETURN CODE_FOR_simple_return

#undef TARGET_HAVE_SPLIT_STACK_PROLOGUE
#define TARGET_HAVE_SPLIT_STACK_PROLOGUE hook_bool_void_false
#undef TARGET_GEN_SPLIT_STACK_PROLOGUE
#define TARGET_GEN_SPLIT_STACK_PROLOGUE invalid_void
#undef TARGET_CODE_FOR_SPLIT_STACK_PROLOGUE
#define TARGET_CODE_FOR_SPLIT_STACK_PROLOGUE CODE_FOR_nothing

#undef TARGET_HAVE_SPLIT_STACK_SPACE_CHECK
#define TARGET_HAVE_SPLIT_STACK_SPACE_CHECK hook_bool_void_false
#undef TARGET_GEN_SPLIT_STACK_SPACE_CHECK
#define TARGET_GEN_SPLIT_STACK_SPACE_CHECK invalid_rtx_rtx
#undef TARGET_CODE_FOR_SPLIT_STACK_SPACE_CHECK
#define TARGET_CODE_FOR_SPLIT_STACK_SPACE_CHECK CODE_FOR_nothing

#undef TARGET_HAVE_STACK_PROTECT_SET
#define TARGET_HAVE_STACK_PROTECT_SET hook_bool_void_false
#undef TARGET_GEN_STACK_PROTECT_SET
#define TARGET_GEN_STACK_PROTECT_SET invalid_rtx_rtx
#undef TARGET_CODE_FOR_STACK_PROTECT_SET
#define TARGET_CODE_FOR_STACK_PROTECT_SET CODE_FOR_nothing

#undef TARGET_HAVE_STACK_PROTECT_TEST
#define TARGET_HAVE_STACK_PROTECT_TEST hook_bool_void_false
#undef TARGET_GEN_STACK_PROTECT_TEST
#define TARGET_GEN_STACK_PROTECT_TEST invalid_rtx_rtx_rtx
#undef TARGET_CODE_FOR_STACK_PROTECT_TEST
#define TARGET_CODE_FOR_STACK_PROTECT_TEST CODE_FOR_nothing

static rtx_insn *
target_gen_store_multiple (rtx x0, rtx x1, rtx x2)
{
  gcc_checking_assert (targetm.have_store_multiple ());
  return insnify (gen_store_multiple (x0, x1, x2));
}

#undef TARGET_HAVE_STORE_MULTIPLE
#define TARGET_HAVE_STORE_MULTIPLE target_have_doloop_end
#undef TARGET_GEN_STORE_MULTIPLE
#define TARGET_GEN_STORE_MULTIPLE target_gen_store_multiple
#undef TARGET_CODE_FOR_STORE_MULTIPLE
#define TARGET_CODE_FOR_STORE_MULTIPLE CODE_FOR_store_multiple

static bool
target_have_tablejump (void)
{
  return 
#line 1918 "../../gcc/config/arm/thumb1.md"
(TARGET_THUMB1);
}

static rtx_insn *
target_gen_tablejump (rtx x0, rtx x1)
{
  gcc_checking_assert (targetm.have_tablejump ());
  return insnify (gen_tablejump (x0, x1));
}

#undef TARGET_HAVE_TABLEJUMP
#define TARGET_HAVE_TABLEJUMP target_have_tablejump
#undef TARGET_GEN_TABLEJUMP
#define TARGET_GEN_TABLEJUMP target_gen_tablejump
#undef TARGET_CODE_FOR_TABLEJUMP
#define TARGET_CODE_FOR_TABLEJUMP CODE_FOR_tablejump

static rtx_insn *
target_gen_trap (void)
{
  return insnify (gen_trap ());
}

#undef TARGET_HAVE_TRAP
#define TARGET_HAVE_TRAP hook_bool_void_true
#undef TARGET_GEN_TRAP
#define TARGET_GEN_TRAP target_gen_trap
#undef TARGET_CODE_FOR_TRAP
#define TARGET_CODE_FOR_TRAP CODE_FOR_trap

#undef TARGET_HAVE_UNIQUE
#define TARGET_HAVE_UNIQUE hook_bool_void_false
#undef TARGET_GEN_UNIQUE
#define TARGET_GEN_UNIQUE invalid_void
#undef TARGET_CODE_FOR_UNIQUE
#define TARGET_CODE_FOR_UNIQUE CODE_FOR_nothing

static rtx_insn *
target_gen_untyped_call (rtx x0, rtx x1, rtx x2)
{
  return insnify (gen_untyped_call (x0, x1, x2));
}

#undef TARGET_HAVE_UNTYPED_CALL
#define TARGET_HAVE_UNTYPED_CALL hook_bool_void_true
#undef TARGET_GEN_UNTYPED_CALL
#define TARGET_GEN_UNTYPED_CALL target_gen_untyped_call
#undef TARGET_CODE_FOR_UNTYPED_CALL
#define TARGET_CODE_FOR_UNTYPED_CALL CODE_FOR_untyped_call

static rtx_insn *
target_gen_untyped_return (rtx x0, rtx x1)
{
  return insnify (gen_untyped_return (x0, x1));
}

#undef TARGET_HAVE_UNTYPED_RETURN
#define TARGET_HAVE_UNTYPED_RETURN hook_bool_void_true
#undef TARGET_GEN_UNTYPED_RETURN
#define TARGET_GEN_UNTYPED_RETURN target_gen_untyped_return
#undef TARGET_CODE_FOR_UNTYPED_RETURN
#define TARGET_CODE_FOR_UNTYPED_RETURN CODE_FOR_untyped_return

#endif /* GCC_INSN_TARGET_DEF_H */
