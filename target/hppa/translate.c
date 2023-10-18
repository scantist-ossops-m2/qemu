/*
 * HPPA emulation cpu translation for qemu.
 *
 * Copyright (c) 2016 Richard Henderson <rth@twiddle.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "qemu/osdep.h"
#include "cpu.h"
#include "disas/disas.h"
#include "qemu/host-utils.h"
#include "exec/exec-all.h"
#include "tcg/tcg-op.h"
#include "exec/helper-proto.h"
#include "exec/helper-gen.h"
#include "exec/translator.h"
#include "exec/log.h"

#define HELPER_H "helper.h"
#include "exec/helper-info.c.inc"
#undef  HELPER_H


/* Since we have a distinction between register size and address size,
   we need to redefine all of these.  */

#undef TCGv
#undef tcg_temp_new
#undef tcg_global_mem_new

#define TCGv_tl              TCGv_i64
#define tcg_temp_new_tl      tcg_temp_new_i64
#define tcg_gen_extu_reg_tl  tcg_gen_mov_i64

#define TCGv_reg             TCGv_i64

#define tcg_temp_new         tcg_temp_new_i64
#define tcg_global_mem_new   tcg_global_mem_new_i64

#define tcg_gen_movi_reg     tcg_gen_movi_i64
#define tcg_gen_mov_reg      tcg_gen_mov_i64
#define tcg_gen_ld8u_reg     tcg_gen_ld8u_i64
#define tcg_gen_ld8s_reg     tcg_gen_ld8s_i64
#define tcg_gen_ld16u_reg    tcg_gen_ld16u_i64
#define tcg_gen_ld16s_reg    tcg_gen_ld16s_i64
#define tcg_gen_ld32u_reg    tcg_gen_ld32u_i64
#define tcg_gen_ld32s_reg    tcg_gen_ld32s_i64
#define tcg_gen_ld_reg       tcg_gen_ld_i64
#define tcg_gen_st8_reg      tcg_gen_st8_i64
#define tcg_gen_st16_reg     tcg_gen_st16_i64
#define tcg_gen_st32_reg     tcg_gen_st32_i64
#define tcg_gen_st_reg       tcg_gen_st_i64
#define tcg_gen_add_reg      tcg_gen_add_i64
#define tcg_gen_addi_reg     tcg_gen_addi_i64
#define tcg_gen_sub_reg      tcg_gen_sub_i64
#define tcg_gen_neg_reg      tcg_gen_neg_i64
#define tcg_gen_subfi_reg    tcg_gen_subfi_i64
#define tcg_gen_subi_reg     tcg_gen_subi_i64
#define tcg_gen_and_reg      tcg_gen_and_i64
#define tcg_gen_andi_reg     tcg_gen_andi_i64
#define tcg_gen_or_reg       tcg_gen_or_i64
#define tcg_gen_ori_reg      tcg_gen_ori_i64
#define tcg_gen_xor_reg      tcg_gen_xor_i64
#define tcg_gen_xori_reg     tcg_gen_xori_i64
#define tcg_gen_not_reg      tcg_gen_not_i64
#define tcg_gen_shl_reg      tcg_gen_shl_i64
#define tcg_gen_shli_reg     tcg_gen_shli_i64
#define tcg_gen_shr_reg      tcg_gen_shr_i64
#define tcg_gen_shri_reg     tcg_gen_shri_i64
#define tcg_gen_sar_reg      tcg_gen_sar_i64
#define tcg_gen_sari_reg     tcg_gen_sari_i64
#define tcg_gen_brcond_reg   tcg_gen_brcond_i64
#define tcg_gen_brcondi_reg  tcg_gen_brcondi_i64
#define tcg_gen_setcond_reg  tcg_gen_setcond_i64
#define tcg_gen_setcondi_reg tcg_gen_setcondi_i64
#define tcg_gen_mul_reg      tcg_gen_mul_i64
#define tcg_gen_muli_reg     tcg_gen_muli_i64
#define tcg_gen_div_reg      tcg_gen_div_i64
#define tcg_gen_rem_reg      tcg_gen_rem_i64
#define tcg_gen_divu_reg     tcg_gen_divu_i64
#define tcg_gen_remu_reg     tcg_gen_remu_i64
#define tcg_gen_discard_reg  tcg_gen_discard_i64
#define tcg_gen_trunc_reg_i32 tcg_gen_extrl_i64_i32
#define tcg_gen_trunc_i64_reg tcg_gen_mov_i64
#define tcg_gen_extu_i32_reg tcg_gen_extu_i32_i64
#define tcg_gen_ext_i32_reg  tcg_gen_ext_i32_i64
#define tcg_gen_extu_reg_i64 tcg_gen_mov_i64
#define tcg_gen_ext_reg_i64  tcg_gen_mov_i64
#define tcg_gen_ext8u_reg    tcg_gen_ext8u_i64
#define tcg_gen_ext8s_reg    tcg_gen_ext8s_i64
#define tcg_gen_ext16u_reg   tcg_gen_ext16u_i64
#define tcg_gen_ext16s_reg   tcg_gen_ext16s_i64
#define tcg_gen_ext32u_reg   tcg_gen_ext32u_i64
#define tcg_gen_ext32s_reg   tcg_gen_ext32s_i64
#define tcg_gen_bswap16_reg  tcg_gen_bswap16_i64
#define tcg_gen_bswap32_reg  tcg_gen_bswap32_i64
#define tcg_gen_bswap64_reg  tcg_gen_bswap64_i64
#define tcg_gen_concat_reg_i64 tcg_gen_concat32_i64
#define tcg_gen_andc_reg     tcg_gen_andc_i64
#define tcg_gen_eqv_reg      tcg_gen_eqv_i64
#define tcg_gen_nand_reg     tcg_gen_nand_i64
#define tcg_gen_nor_reg      tcg_gen_nor_i64
#define tcg_gen_orc_reg      tcg_gen_orc_i64
#define tcg_gen_clz_reg      tcg_gen_clz_i64
#define tcg_gen_ctz_reg      tcg_gen_ctz_i64
#define tcg_gen_clzi_reg     tcg_gen_clzi_i64
#define tcg_gen_ctzi_reg     tcg_gen_ctzi_i64
#define tcg_gen_clrsb_reg    tcg_gen_clrsb_i64
#define tcg_gen_ctpop_reg    tcg_gen_ctpop_i64
#define tcg_gen_rotl_reg     tcg_gen_rotl_i64
#define tcg_gen_rotli_reg    tcg_gen_rotli_i64
#define tcg_gen_rotr_reg     tcg_gen_rotr_i64
#define tcg_gen_rotri_reg    tcg_gen_rotri_i64
#define tcg_gen_deposit_reg  tcg_gen_deposit_i64
#define tcg_gen_deposit_z_reg tcg_gen_deposit_z_i64
#define tcg_gen_extract_reg  tcg_gen_extract_i64
#define tcg_gen_sextract_reg tcg_gen_sextract_i64
#define tcg_gen_extract2_reg tcg_gen_extract2_i64
#define tcg_constant_reg     tcg_constant_i64
#define tcg_gen_movcond_reg  tcg_gen_movcond_i64
#define tcg_gen_add2_reg     tcg_gen_add2_i64
#define tcg_gen_sub2_reg     tcg_gen_sub2_i64
#define tcg_gen_qemu_ld_reg  tcg_gen_qemu_ld_i64
#define tcg_gen_qemu_st_reg  tcg_gen_qemu_st_i64
#define tcg_gen_atomic_xchg_reg tcg_gen_atomic_xchg_i64
#define tcg_gen_trunc_reg_ptr   tcg_gen_trunc_i64_ptr

typedef struct DisasCond {
    TCGCond c;
    TCGv_reg a0, a1;
} DisasCond;

typedef struct DisasContext {
    DisasContextBase base;
    CPUState *cs;

    uint64_t iaoq_f;
    uint64_t iaoq_b;
    uint64_t iaoq_n;
    TCGv_reg iaoq_n_var;

    DisasCond null_cond;
    TCGLabel *null_lab;

    uint32_t insn;
    uint32_t tb_flags;
    int mmu_idx;
    int privilege;
    bool psw_n_nonzero;
    bool is_pa20;

#ifdef CONFIG_USER_ONLY
    MemOp unalign;
#endif
} DisasContext;

#ifdef CONFIG_USER_ONLY
#define UNALIGN(C)  (C)->unalign
#else
#define UNALIGN(C)  MO_ALIGN
#endif

/* Note that ssm/rsm instructions number PSW_W and PSW_E differently.  */
static int expand_sm_imm(DisasContext *ctx, int val)
{
    if (val & PSW_SM_E) {
        val = (val & ~PSW_SM_E) | PSW_E;
    }
    if (val & PSW_SM_W) {
        val = (val & ~PSW_SM_W) | PSW_W;
    }
    return val;
}

/* Inverted space register indicates 0 means sr0 not inferred from base.  */
static int expand_sr3x(DisasContext *ctx, int val)
{
    return ~val;
}

/* Convert the M:A bits within a memory insn to the tri-state value
   we use for the final M.  */
static int ma_to_m(DisasContext *ctx, int val)
{
    return val & 2 ? (val & 1 ? -1 : 1) : 0;
}

/* Convert the sign of the displacement to a pre or post-modify.  */
static int pos_to_m(DisasContext *ctx, int val)
{
    return val ? 1 : -1;
}

static int neg_to_m(DisasContext *ctx, int val)
{
    return val ? -1 : 1;
}

/* Used for branch targets and fp memory ops.  */
static int expand_shl2(DisasContext *ctx, int val)
{
    return val << 2;
}

/* Used for fp memory ops.  */
static int expand_shl3(DisasContext *ctx, int val)
{
    return val << 3;
}

/* Used for assemble_21.  */
static int expand_shl11(DisasContext *ctx, int val)
{
    return val << 11;
}

static int assemble_6(DisasContext *ctx, int val)
{
    /*
     * Officially, 32 * x + 32 - y.
     * Here, x is already in bit 5, and y is [4:0].
     * Since -y = ~y + 1, in 5 bits 32 - y => y ^ 31 + 1,
     * with the overflow from bit 4 summing with x.
     */
    return (val ^ 31) + 1;
}

/* Translate CMPI doubleword conditions to standard. */
static int cmpbid_c(DisasContext *ctx, int val)
{
    return val ? val : 4; /* 0 == "*<<" */
}


/* Include the auto-generated decoder.  */
#include "decode-insns.c.inc"

/* We are not using a goto_tb (for whatever reason), but have updated
   the iaq (for whatever reason), so don't do it again on exit.  */
#define DISAS_IAQ_N_UPDATED  DISAS_TARGET_0

/* We are exiting the TB, but have neither emitted a goto_tb, nor
   updated the iaq for the next instruction to be executed.  */
#define DISAS_IAQ_N_STALE    DISAS_TARGET_1

/* Similarly, but we want to return to the main loop immediately
   to recognize unmasked interrupts.  */
#define DISAS_IAQ_N_STALE_EXIT      DISAS_TARGET_2
#define DISAS_EXIT                  DISAS_TARGET_3

/* global register indexes */
static TCGv_reg cpu_gr[32];
static TCGv_i64 cpu_sr[4];
static TCGv_i64 cpu_srH;
static TCGv_reg cpu_iaoq_f;
static TCGv_reg cpu_iaoq_b;
static TCGv_i64 cpu_iasq_f;
static TCGv_i64 cpu_iasq_b;
static TCGv_reg cpu_sar;
static TCGv_reg cpu_psw_n;
static TCGv_reg cpu_psw_v;
static TCGv_reg cpu_psw_cb;
static TCGv_reg cpu_psw_cb_msb;

void hppa_translate_init(void)
{
#define DEF_VAR(V)  { &cpu_##V, #V, offsetof(CPUHPPAState, V) }

    typedef struct { TCGv_reg *var; const char *name; int ofs; } GlobalVar;
    static const GlobalVar vars[] = {
        { &cpu_sar, "sar", offsetof(CPUHPPAState, cr[CR_SAR]) },
        DEF_VAR(psw_n),
        DEF_VAR(psw_v),
        DEF_VAR(psw_cb),
        DEF_VAR(psw_cb_msb),
        DEF_VAR(iaoq_f),
        DEF_VAR(iaoq_b),
    };

#undef DEF_VAR

    /* Use the symbolic register names that match the disassembler.  */
    static const char gr_names[32][4] = {
        "r0",  "r1",  "r2",  "r3",  "r4",  "r5",  "r6",  "r7",
        "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15",
        "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23",
        "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31"
    };
    /* SR[4-7] are not global registers so that we can index them.  */
    static const char sr_names[5][4] = {
        "sr0", "sr1", "sr2", "sr3", "srH"
    };

    int i;

    cpu_gr[0] = NULL;
    for (i = 1; i < 32; i++) {
        cpu_gr[i] = tcg_global_mem_new(tcg_env,
                                       offsetof(CPUHPPAState, gr[i]),
                                       gr_names[i]);
    }
    for (i = 0; i < 4; i++) {
        cpu_sr[i] = tcg_global_mem_new_i64(tcg_env,
                                           offsetof(CPUHPPAState, sr[i]),
                                           sr_names[i]);
    }
    cpu_srH = tcg_global_mem_new_i64(tcg_env,
                                     offsetof(CPUHPPAState, sr[4]),
                                     sr_names[4]);

    for (i = 0; i < ARRAY_SIZE(vars); ++i) {
        const GlobalVar *v = &vars[i];
        *v->var = tcg_global_mem_new(tcg_env, v->ofs, v->name);
    }

    cpu_iasq_f = tcg_global_mem_new_i64(tcg_env,
                                        offsetof(CPUHPPAState, iasq_f),
                                        "iasq_f");
    cpu_iasq_b = tcg_global_mem_new_i64(tcg_env,
                                        offsetof(CPUHPPAState, iasq_b),
                                        "iasq_b");
}

static DisasCond cond_make_f(void)
{
    return (DisasCond){
        .c = TCG_COND_NEVER,
        .a0 = NULL,
        .a1 = NULL,
    };
}

static DisasCond cond_make_t(void)
{
    return (DisasCond){
        .c = TCG_COND_ALWAYS,
        .a0 = NULL,
        .a1 = NULL,
    };
}

static DisasCond cond_make_n(void)
{
    return (DisasCond){
        .c = TCG_COND_NE,
        .a0 = cpu_psw_n,
        .a1 = tcg_constant_reg(0)
    };
}

static DisasCond cond_make_tmp(TCGCond c, TCGv_reg a0, TCGv_reg a1)
{
    assert (c != TCG_COND_NEVER && c != TCG_COND_ALWAYS);
    return (DisasCond){ .c = c, .a0 = a0, .a1 = a1 };
}

static DisasCond cond_make_0_tmp(TCGCond c, TCGv_reg a0)
{
    return cond_make_tmp(c, a0, tcg_constant_reg(0));
}

static DisasCond cond_make_0(TCGCond c, TCGv_reg a0)
{
    TCGv_reg tmp = tcg_temp_new();
    tcg_gen_mov_reg(tmp, a0);
    return cond_make_0_tmp(c, tmp);
}

static DisasCond cond_make(TCGCond c, TCGv_reg a0, TCGv_reg a1)
{
    TCGv_reg t0 = tcg_temp_new();
    TCGv_reg t1 = tcg_temp_new();

    tcg_gen_mov_reg(t0, a0);
    tcg_gen_mov_reg(t1, a1);
    return cond_make_tmp(c, t0, t1);
}

static void cond_free(DisasCond *cond)
{
    switch (cond->c) {
    default:
        cond->a0 = NULL;
        cond->a1 = NULL;
        /* fallthru */
    case TCG_COND_ALWAYS:
        cond->c = TCG_COND_NEVER;
        break;
    case TCG_COND_NEVER:
        break;
    }
}

static TCGv_reg load_gpr(DisasContext *ctx, unsigned reg)
{
    if (reg == 0) {
        TCGv_reg t = tcg_temp_new();
        tcg_gen_movi_reg(t, 0);
        return t;
    } else {
        return cpu_gr[reg];
    }
}

static TCGv_reg dest_gpr(DisasContext *ctx, unsigned reg)
{
    if (reg == 0 || ctx->null_cond.c != TCG_COND_NEVER) {
        return tcg_temp_new();
    } else {
        return cpu_gr[reg];
    }
}

static void save_or_nullify(DisasContext *ctx, TCGv_reg dest, TCGv_reg t)
{
    if (ctx->null_cond.c != TCG_COND_NEVER) {
        tcg_gen_movcond_reg(ctx->null_cond.c, dest, ctx->null_cond.a0,
                            ctx->null_cond.a1, dest, t);
    } else {
        tcg_gen_mov_reg(dest, t);
    }
}

static void save_gpr(DisasContext *ctx, unsigned reg, TCGv_reg t)
{
    if (reg != 0) {
        save_or_nullify(ctx, cpu_gr[reg], t);
    }
}

#if HOST_BIG_ENDIAN
# define HI_OFS  0
# define LO_OFS  4
#else
# define HI_OFS  4
# define LO_OFS  0
#endif

static TCGv_i32 load_frw_i32(unsigned rt)
{
    TCGv_i32 ret = tcg_temp_new_i32();
    tcg_gen_ld_i32(ret, tcg_env,
                   offsetof(CPUHPPAState, fr[rt & 31])
                   + (rt & 32 ? LO_OFS : HI_OFS));
    return ret;
}

static TCGv_i32 load_frw0_i32(unsigned rt)
{
    if (rt == 0) {
        TCGv_i32 ret = tcg_temp_new_i32();
        tcg_gen_movi_i32(ret, 0);
        return ret;
    } else {
        return load_frw_i32(rt);
    }
}

static TCGv_i64 load_frw0_i64(unsigned rt)
{
    TCGv_i64 ret = tcg_temp_new_i64();
    if (rt == 0) {
        tcg_gen_movi_i64(ret, 0);
    } else {
        tcg_gen_ld32u_i64(ret, tcg_env,
                          offsetof(CPUHPPAState, fr[rt & 31])
                          + (rt & 32 ? LO_OFS : HI_OFS));
    }
    return ret;
}

static void save_frw_i32(unsigned rt, TCGv_i32 val)
{
    tcg_gen_st_i32(val, tcg_env,
                   offsetof(CPUHPPAState, fr[rt & 31])
                   + (rt & 32 ? LO_OFS : HI_OFS));
}

#undef HI_OFS
#undef LO_OFS

static TCGv_i64 load_frd(unsigned rt)
{
    TCGv_i64 ret = tcg_temp_new_i64();
    tcg_gen_ld_i64(ret, tcg_env, offsetof(CPUHPPAState, fr[rt]));
    return ret;
}

static TCGv_i64 load_frd0(unsigned rt)
{
    if (rt == 0) {
        TCGv_i64 ret = tcg_temp_new_i64();
        tcg_gen_movi_i64(ret, 0);
        return ret;
    } else {
        return load_frd(rt);
    }
}

static void save_frd(unsigned rt, TCGv_i64 val)
{
    tcg_gen_st_i64(val, tcg_env, offsetof(CPUHPPAState, fr[rt]));
}

static void load_spr(DisasContext *ctx, TCGv_i64 dest, unsigned reg)
{
#ifdef CONFIG_USER_ONLY
    tcg_gen_movi_i64(dest, 0);
#else
    if (reg < 4) {
        tcg_gen_mov_i64(dest, cpu_sr[reg]);
    } else if (ctx->tb_flags & TB_FLAG_SR_SAME) {
        tcg_gen_mov_i64(dest, cpu_srH);
    } else {
        tcg_gen_ld_i64(dest, tcg_env, offsetof(CPUHPPAState, sr[reg]));
    }
#endif
}

/* Skip over the implementation of an insn that has been nullified.
   Use this when the insn is too complex for a conditional move.  */
static void nullify_over(DisasContext *ctx)
{
    if (ctx->null_cond.c != TCG_COND_NEVER) {
        /* The always condition should have been handled in the main loop.  */
        assert(ctx->null_cond.c != TCG_COND_ALWAYS);

        ctx->null_lab = gen_new_label();

        /* If we're using PSW[N], copy it to a temp because... */
        if (ctx->null_cond.a0 == cpu_psw_n) {
            ctx->null_cond.a0 = tcg_temp_new();
            tcg_gen_mov_reg(ctx->null_cond.a0, cpu_psw_n);
        }
        /* ... we clear it before branching over the implementation,
           so that (1) it's clear after nullifying this insn and
           (2) if this insn nullifies the next, PSW[N] is valid.  */
        if (ctx->psw_n_nonzero) {
            ctx->psw_n_nonzero = false;
            tcg_gen_movi_reg(cpu_psw_n, 0);
        }

        tcg_gen_brcond_reg(ctx->null_cond.c, ctx->null_cond.a0,
                           ctx->null_cond.a1, ctx->null_lab);
        cond_free(&ctx->null_cond);
    }
}

/* Save the current nullification state to PSW[N].  */
static void nullify_save(DisasContext *ctx)
{
    if (ctx->null_cond.c == TCG_COND_NEVER) {
        if (ctx->psw_n_nonzero) {
            tcg_gen_movi_reg(cpu_psw_n, 0);
        }
        return;
    }
    if (ctx->null_cond.a0 != cpu_psw_n) {
        tcg_gen_setcond_reg(ctx->null_cond.c, cpu_psw_n,
                            ctx->null_cond.a0, ctx->null_cond.a1);
        ctx->psw_n_nonzero = true;
    }
    cond_free(&ctx->null_cond);
}

/* Set a PSW[N] to X.  The intention is that this is used immediately
   before a goto_tb/exit_tb, so that there is no fallthru path to other
   code within the TB.  Therefore we do not update psw_n_nonzero.  */
static void nullify_set(DisasContext *ctx, bool x)
{
    if (ctx->psw_n_nonzero || x) {
        tcg_gen_movi_reg(cpu_psw_n, x);
    }
}

/* Mark the end of an instruction that may have been nullified.
   This is the pair to nullify_over.  Always returns true so that
   it may be tail-called from a translate function.  */
static bool nullify_end(DisasContext *ctx)
{
    TCGLabel *null_lab = ctx->null_lab;
    DisasJumpType status = ctx->base.is_jmp;

    /* For NEXT, NORETURN, STALE, we can easily continue (or exit).
       For UPDATED, we cannot update on the nullified path.  */
    assert(status != DISAS_IAQ_N_UPDATED);

    if (likely(null_lab == NULL)) {
        /* The current insn wasn't conditional or handled the condition
           applied to it without a branch, so the (new) setting of
           NULL_COND can be applied directly to the next insn.  */
        return true;
    }
    ctx->null_lab = NULL;

    if (likely(ctx->null_cond.c == TCG_COND_NEVER)) {
        /* The next instruction will be unconditional,
           and NULL_COND already reflects that.  */
        gen_set_label(null_lab);
    } else {
        /* The insn that we just executed is itself nullifying the next
           instruction.  Store the condition in the PSW[N] global.
           We asserted PSW[N] = 0 in nullify_over, so that after the
           label we have the proper value in place.  */
        nullify_save(ctx);
        gen_set_label(null_lab);
        ctx->null_cond = cond_make_n();
    }
    if (status == DISAS_NORETURN) {
        ctx->base.is_jmp = DISAS_NEXT;
    }
    return true;
}

static uint64_t gva_offset_mask(DisasContext *ctx)
{
    return (ctx->tb_flags & PSW_W
            ? MAKE_64BIT_MASK(0, 62)
            : MAKE_64BIT_MASK(0, 32));
}

static void copy_iaoq_entry(DisasContext *ctx, TCGv_reg dest,
                            uint64_t ival, TCGv_reg vval)
{
    uint64_t mask = gva_offset_mask(ctx);

    if (ival != -1) {
        tcg_gen_movi_reg(dest, ival & mask);
        return;
    }
    tcg_debug_assert(vval != NULL);

    /*
     * We know that the IAOQ is already properly masked.
     * This optimization is primarily for "iaoq_f = iaoq_b".
     */
    if (vval == cpu_iaoq_f || vval == cpu_iaoq_b) {
        tcg_gen_mov_reg(dest, vval);
    } else {
        tcg_gen_andi_reg(dest, vval, mask);
    }
}

static inline uint64_t iaoq_dest(DisasContext *ctx, int64_t disp)
{
    return ctx->iaoq_f + disp + 8;
}

static void gen_excp_1(int exception)
{
    gen_helper_excp(tcg_env, tcg_constant_i32(exception));
}

static void gen_excp(DisasContext *ctx, int exception)
{
    copy_iaoq_entry(ctx, cpu_iaoq_f, ctx->iaoq_f, cpu_iaoq_f);
    copy_iaoq_entry(ctx, cpu_iaoq_b, ctx->iaoq_b, cpu_iaoq_b);
    nullify_save(ctx);
    gen_excp_1(exception);
    ctx->base.is_jmp = DISAS_NORETURN;
}

static bool gen_excp_iir(DisasContext *ctx, int exc)
{
    nullify_over(ctx);
    tcg_gen_st_reg(tcg_constant_reg(ctx->insn),
                   tcg_env, offsetof(CPUHPPAState, cr[CR_IIR]));
    gen_excp(ctx, exc);
    return nullify_end(ctx);
}

static bool gen_illegal(DisasContext *ctx)
{
    return gen_excp_iir(ctx, EXCP_ILL);
}

#ifdef CONFIG_USER_ONLY
#define CHECK_MOST_PRIVILEGED(EXCP) \
    return gen_excp_iir(ctx, EXCP)
#else
#define CHECK_MOST_PRIVILEGED(EXCP) \
    do {                                     \
        if (ctx->privilege != 0) {           \
            return gen_excp_iir(ctx, EXCP);  \
        }                                    \
    } while (0)
#endif

static bool use_goto_tb(DisasContext *ctx, uint64_t dest)
{
    return translator_use_goto_tb(&ctx->base, dest);
}

/* If the next insn is to be nullified, and it's on the same page,
   and we're not attempting to set a breakpoint on it, then we can
   totally skip the nullified insn.  This avoids creating and
   executing a TB that merely branches to the next TB.  */
static bool use_nullify_skip(DisasContext *ctx)
{
    return (((ctx->iaoq_b ^ ctx->iaoq_f) & TARGET_PAGE_MASK) == 0
            && !cpu_breakpoint_test(ctx->cs, ctx->iaoq_b, BP_ANY));
}

static void gen_goto_tb(DisasContext *ctx, int which,
                        uint64_t f, uint64_t b)
{
    if (f != -1 && b != -1 && use_goto_tb(ctx, f)) {
        tcg_gen_goto_tb(which);
        copy_iaoq_entry(ctx, cpu_iaoq_f, f, NULL);
        copy_iaoq_entry(ctx, cpu_iaoq_b, b, NULL);
        tcg_gen_exit_tb(ctx->base.tb, which);
    } else {
        copy_iaoq_entry(ctx, cpu_iaoq_f, f, cpu_iaoq_b);
        copy_iaoq_entry(ctx, cpu_iaoq_b, b, ctx->iaoq_n_var);
        tcg_gen_lookup_and_goto_ptr();
    }
}

static bool cond_need_sv(int c)
{
    return c == 2 || c == 3 || c == 6;
}

static bool cond_need_cb(int c)
{
    return c == 4 || c == 5;
}

/* Need extensions from TCGv_i32 to TCGv_reg. */
static bool cond_need_ext(DisasContext *ctx, bool d)
{
    return !(ctx->is_pa20 && d);
}

/*
 * Compute conditional for arithmetic.  See Page 5-3, Table 5-1, of
 * the Parisc 1.1 Architecture Reference Manual for details.
 */

static DisasCond do_cond(DisasContext *ctx, unsigned cf, bool d,
                         TCGv_reg res, TCGv_reg cb_msb, TCGv_reg sv)
{
    DisasCond cond;
    TCGv_reg tmp;

    switch (cf >> 1) {
    case 0: /* Never / TR    (0 / 1) */
        cond = cond_make_f();
        break;
    case 1: /* = / <>        (Z / !Z) */
        if (cond_need_ext(ctx, d)) {
            tmp = tcg_temp_new();
            tcg_gen_ext32u_reg(tmp, res);
            res = tmp;
        }
        cond = cond_make_0(TCG_COND_EQ, res);
        break;
    case 2: /* < / >=        (N ^ V / !(N ^ V) */
        tmp = tcg_temp_new();
        tcg_gen_xor_reg(tmp, res, sv);
        if (cond_need_ext(ctx, d)) {
            tcg_gen_ext32s_reg(tmp, tmp);
        }
        cond = cond_make_0_tmp(TCG_COND_LT, tmp);
        break;
    case 3: /* <= / >        (N ^ V) | Z / !((N ^ V) | Z) */
        /*
         * Simplify:
         *   (N ^ V) | Z
         *   ((res < 0) ^ (sv < 0)) | !res
         *   ((res ^ sv) < 0) | !res
         *   (~(res ^ sv) >= 0) | !res
         *   !(~(res ^ sv) >> 31) | !res
         *   !(~(res ^ sv) >> 31 & res)
         */
        tmp = tcg_temp_new();
        tcg_gen_eqv_reg(tmp, res, sv);
        if (cond_need_ext(ctx, d)) {
            tcg_gen_sextract_reg(tmp, tmp, 31, 1);
            tcg_gen_and_reg(tmp, tmp, res);
            tcg_gen_ext32u_reg(tmp, tmp);
        } else {
            tcg_gen_sari_reg(tmp, tmp, 63);
            tcg_gen_and_reg(tmp, tmp, res);
        }
        cond = cond_make_0_tmp(TCG_COND_EQ, tmp);
        break;
    case 4: /* NUV / UV      (!C / C) */
        /* Only bit 0 of cb_msb is ever set. */
        cond = cond_make_0(TCG_COND_EQ, cb_msb);
        break;
    case 5: /* ZNV / VNZ     (!C | Z / C & !Z) */
        tmp = tcg_temp_new();
        tcg_gen_neg_reg(tmp, cb_msb);
        tcg_gen_and_reg(tmp, tmp, res);
        if (cond_need_ext(ctx, d)) {
            tcg_gen_ext32u_reg(tmp, tmp);
        }
        cond = cond_make_0_tmp(TCG_COND_EQ, tmp);
        break;
    case 6: /* SV / NSV      (V / !V) */
        if (cond_need_ext(ctx, d)) {
            tmp = tcg_temp_new();
            tcg_gen_ext32s_reg(tmp, sv);
            sv = tmp;
        }
        cond = cond_make_0(TCG_COND_LT, sv);
        break;
    case 7: /* OD / EV */
        tmp = tcg_temp_new();
        tcg_gen_andi_reg(tmp, res, 1);
        cond = cond_make_0_tmp(TCG_COND_NE, tmp);
        break;
    default:
        g_assert_not_reached();
    }
    if (cf & 1) {
        cond.c = tcg_invert_cond(cond.c);
    }

    return cond;
}

/* Similar, but for the special case of subtraction without borrow, we
   can use the inputs directly.  This can allow other computation to be
   deleted as unused.  */

static DisasCond do_sub_cond(DisasContext *ctx, unsigned cf, bool d,
                             TCGv_reg res, TCGv_reg in1,
                             TCGv_reg in2, TCGv_reg sv)
{
    TCGCond tc;
    bool ext_uns;

    switch (cf >> 1) {
    case 1: /* = / <> */
        tc = TCG_COND_EQ;
        ext_uns = true;
        break;
    case 2: /* < / >= */
        tc = TCG_COND_LT;
        ext_uns = false;
        break;
    case 3: /* <= / > */
        tc = TCG_COND_LE;
        ext_uns = false;
        break;
    case 4: /* << / >>= */
        tc = TCG_COND_LTU;
        ext_uns = true;
        break;
    case 5: /* <<= / >> */
        tc = TCG_COND_LEU;
        ext_uns = true;
        break;
    default:
        return do_cond(ctx, cf, d, res, NULL, sv);
    }

    if (cf & 1) {
        tc = tcg_invert_cond(tc);
    }
    if (cond_need_ext(ctx, d)) {
        TCGv_reg t1 = tcg_temp_new();
        TCGv_reg t2 = tcg_temp_new();

        if (ext_uns) {
            tcg_gen_ext32u_reg(t1, in1);
            tcg_gen_ext32u_reg(t2, in2);
        } else {
            tcg_gen_ext32s_reg(t1, in1);
            tcg_gen_ext32s_reg(t2, in2);
        }
        return cond_make_tmp(tc, t1, t2);
    }
    return cond_make(tc, in1, in2);
}

/*
 * Similar, but for logicals, where the carry and overflow bits are not
 * computed, and use of them is undefined.
 *
 * Undefined or not, hardware does not trap.  It seems reasonable to
 * assume hardware treats cases c={4,5,6} as if C=0 & V=0, since that's
 * how cases c={2,3} are treated.
 */

static DisasCond do_log_cond(DisasContext *ctx, unsigned cf, bool d,
                             TCGv_reg res)
{
    TCGCond tc;
    bool ext_uns;

    switch (cf) {
    case 0:  /* never */
    case 9:  /* undef, C */
    case 11: /* undef, C & !Z */
    case 12: /* undef, V */
        return cond_make_f();

    case 1:  /* true */
    case 8:  /* undef, !C */
    case 10: /* undef, !C | Z */
    case 13: /* undef, !V */
        return cond_make_t();

    case 2:  /* == */
        tc = TCG_COND_EQ;
        ext_uns = true;
        break;
    case 3:  /* <> */
        tc = TCG_COND_NE;
        ext_uns = true;
        break;
    case 4:  /* < */
        tc = TCG_COND_LT;
        ext_uns = false;
        break;
    case 5:  /* >= */
        tc = TCG_COND_GE;
        ext_uns = false;
        break;
    case 6:  /* <= */
        tc = TCG_COND_LE;
        ext_uns = false;
        break;
    case 7:  /* > */
        tc = TCG_COND_GT;
        ext_uns = false;
        break;

    case 14: /* OD */
    case 15: /* EV */
        return do_cond(ctx, cf, d, res, NULL, NULL);

    default:
        g_assert_not_reached();
    }

    if (cond_need_ext(ctx, d)) {
        TCGv_reg tmp = tcg_temp_new();

        if (ext_uns) {
            tcg_gen_ext32u_reg(tmp, res);
        } else {
            tcg_gen_ext32s_reg(tmp, res);
        }
        return cond_make_0_tmp(tc, tmp);
    }
    return cond_make_0(tc, res);
}

/* Similar, but for shift/extract/deposit conditions.  */

static DisasCond do_sed_cond(DisasContext *ctx, unsigned orig, bool d,
                             TCGv_reg res)
{
    unsigned c, f;

    /* Convert the compressed condition codes to standard.
       0-2 are the same as logicals (nv,<,<=), while 3 is OD.
       4-7 are the reverse of 0-3.  */
    c = orig & 3;
    if (c == 3) {
        c = 7;
    }
    f = (orig & 4) / 4;

    return do_log_cond(ctx, c * 2 + f, d, res);
}

/* Similar, but for unit conditions.  */

static DisasCond do_unit_cond(unsigned cf, bool d, TCGv_reg res,
                              TCGv_reg in1, TCGv_reg in2)
{
    DisasCond cond;
    TCGv_reg tmp, cb = NULL;
    uint64_t d_repl = d ? 0x0000000100000001ull : 1;

    if (cf & 8) {
        /* Since we want to test lots of carry-out bits all at once, do not
         * do our normal thing and compute carry-in of bit B+1 since that
         * leaves us with carry bits spread across two words.
         */
        cb = tcg_temp_new();
        tmp = tcg_temp_new();
        tcg_gen_or_reg(cb, in1, in2);
        tcg_gen_and_reg(tmp, in1, in2);
        tcg_gen_andc_reg(cb, cb, res);
        tcg_gen_or_reg(cb, cb, tmp);
    }

    switch (cf >> 1) {
    case 0: /* never / TR */
    case 1: /* undefined */
    case 5: /* undefined */
        cond = cond_make_f();
        break;

    case 2: /* SBZ / NBZ */
        /* See hasless(v,1) from
         * https://graphics.stanford.edu/~seander/bithacks.html#ZeroInWord
         */
        tmp = tcg_temp_new();
        tcg_gen_subi_reg(tmp, res, d_repl * 0x01010101u);
        tcg_gen_andc_reg(tmp, tmp, res);
        tcg_gen_andi_reg(tmp, tmp, d_repl * 0x80808080u);
        cond = cond_make_0(TCG_COND_NE, tmp);
        break;

    case 3: /* SHZ / NHZ */
        tmp = tcg_temp_new();
        tcg_gen_subi_reg(tmp, res, d_repl * 0x00010001u);
        tcg_gen_andc_reg(tmp, tmp, res);
        tcg_gen_andi_reg(tmp, tmp, d_repl * 0x80008000u);
        cond = cond_make_0(TCG_COND_NE, tmp);
        break;

    case 4: /* SDC / NDC */
        tcg_gen_andi_reg(cb, cb, d_repl * 0x88888888u);
        cond = cond_make_0(TCG_COND_NE, cb);
        break;

    case 6: /* SBC / NBC */
        tcg_gen_andi_reg(cb, cb, d_repl * 0x80808080u);
        cond = cond_make_0(TCG_COND_NE, cb);
        break;

    case 7: /* SHC / NHC */
        tcg_gen_andi_reg(cb, cb, d_repl * 0x80008000u);
        cond = cond_make_0(TCG_COND_NE, cb);
        break;

    default:
        g_assert_not_reached();
    }
    if (cf & 1) {
        cond.c = tcg_invert_cond(cond.c);
    }

    return cond;
}

static TCGv_reg get_carry(DisasContext *ctx, bool d,
                          TCGv_reg cb, TCGv_reg cb_msb)
{
    if (cond_need_ext(ctx, d)) {
        TCGv_reg t = tcg_temp_new();
        tcg_gen_extract_reg(t, cb, 32, 1);
        return t;
    }
    return cb_msb;
}

static TCGv_reg get_psw_carry(DisasContext *ctx, bool d)
{
    return get_carry(ctx, d, cpu_psw_cb, cpu_psw_cb_msb);
}

/* Compute signed overflow for addition.  */
static TCGv_reg do_add_sv(DisasContext *ctx, TCGv_reg res,
                          TCGv_reg in1, TCGv_reg in2)
{
    TCGv_reg sv = tcg_temp_new();
    TCGv_reg tmp = tcg_temp_new();

    tcg_gen_xor_reg(sv, res, in1);
    tcg_gen_xor_reg(tmp, in1, in2);
    tcg_gen_andc_reg(sv, sv, tmp);

    return sv;
}

/* Compute signed overflow for subtraction.  */
static TCGv_reg do_sub_sv(DisasContext *ctx, TCGv_reg res,
                          TCGv_reg in1, TCGv_reg in2)
{
    TCGv_reg sv = tcg_temp_new();
    TCGv_reg tmp = tcg_temp_new();

    tcg_gen_xor_reg(sv, res, in1);
    tcg_gen_xor_reg(tmp, in1, in2);
    tcg_gen_and_reg(sv, sv, tmp);

    return sv;
}

static void do_add(DisasContext *ctx, unsigned rt, TCGv_reg in1,
                   TCGv_reg in2, unsigned shift, bool is_l,
                   bool is_tsv, bool is_tc, bool is_c, unsigned cf, bool d)
{
    TCGv_reg dest, cb, cb_msb, cb_cond, sv, tmp;
    unsigned c = cf >> 1;
    DisasCond cond;

    dest = tcg_temp_new();
    cb = NULL;
    cb_msb = NULL;
    cb_cond = NULL;

    if (shift) {
        tmp = tcg_temp_new();
        tcg_gen_shli_reg(tmp, in1, shift);
        in1 = tmp;
    }

    if (!is_l || cond_need_cb(c)) {
        TCGv_reg zero = tcg_constant_reg(0);
        cb_msb = tcg_temp_new();
        cb = tcg_temp_new();

        tcg_gen_add2_reg(dest, cb_msb, in1, zero, in2, zero);
        if (is_c) {
            tcg_gen_add2_reg(dest, cb_msb, dest, cb_msb,
                             get_psw_carry(ctx, d), zero);
        }
        tcg_gen_xor_reg(cb, in1, in2);
        tcg_gen_xor_reg(cb, cb, dest);
        if (cond_need_cb(c)) {
            cb_cond = get_carry(ctx, d, cb, cb_msb);
        }
    } else {
        tcg_gen_add_reg(dest, in1, in2);
        if (is_c) {
            tcg_gen_add_reg(dest, dest, get_psw_carry(ctx, d));
        }
    }

    /* Compute signed overflow if required.  */
    sv = NULL;
    if (is_tsv || cond_need_sv(c)) {
        sv = do_add_sv(ctx, dest, in1, in2);
        if (is_tsv) {
            /* ??? Need to include overflow from shift.  */
            gen_helper_tsv(tcg_env, sv);
        }
    }

    /* Emit any conditional trap before any writeback.  */
    cond = do_cond(ctx, cf, d, dest, cb_cond, sv);
    if (is_tc) {
        tmp = tcg_temp_new();
        tcg_gen_setcond_reg(cond.c, tmp, cond.a0, cond.a1);
        gen_helper_tcond(tcg_env, tmp);
    }

    /* Write back the result.  */
    if (!is_l) {
        save_or_nullify(ctx, cpu_psw_cb, cb);
        save_or_nullify(ctx, cpu_psw_cb_msb, cb_msb);
    }
    save_gpr(ctx, rt, dest);

    /* Install the new nullification.  */
    cond_free(&ctx->null_cond);
    ctx->null_cond = cond;
}

static bool do_add_reg(DisasContext *ctx, arg_rrr_cf_d_sh *a,
                       bool is_l, bool is_tsv, bool is_tc, bool is_c)
{
    TCGv_reg tcg_r1, tcg_r2;

    if (a->cf) {
        nullify_over(ctx);
    }
    tcg_r1 = load_gpr(ctx, a->r1);
    tcg_r2 = load_gpr(ctx, a->r2);
    do_add(ctx, a->t, tcg_r1, tcg_r2, a->sh, is_l,
           is_tsv, is_tc, is_c, a->cf, a->d);
    return nullify_end(ctx);
}

static bool do_add_imm(DisasContext *ctx, arg_rri_cf *a,
                       bool is_tsv, bool is_tc)
{
    TCGv_reg tcg_im, tcg_r2;

    if (a->cf) {
        nullify_over(ctx);
    }
    tcg_im = tcg_constant_reg(a->i);
    tcg_r2 = load_gpr(ctx, a->r);
    /* All ADDI conditions are 32-bit. */
    do_add(ctx, a->t, tcg_im, tcg_r2, 0, 0, is_tsv, is_tc, 0, a->cf, false);
    return nullify_end(ctx);
}

static void do_sub(DisasContext *ctx, unsigned rt, TCGv_reg in1,
                   TCGv_reg in2, bool is_tsv, bool is_b,
                   bool is_tc, unsigned cf, bool d)
{
    TCGv_reg dest, sv, cb, cb_msb, zero, tmp;
    unsigned c = cf >> 1;
    DisasCond cond;

    dest = tcg_temp_new();
    cb = tcg_temp_new();
    cb_msb = tcg_temp_new();

    zero = tcg_constant_reg(0);
    if (is_b) {
        /* DEST,C = IN1 + ~IN2 + C.  */
        tcg_gen_not_reg(cb, in2);
        tcg_gen_add2_reg(dest, cb_msb, in1, zero, get_psw_carry(ctx, d), zero);
        tcg_gen_add2_reg(dest, cb_msb, dest, cb_msb, cb, zero);
        tcg_gen_xor_reg(cb, cb, in1);
        tcg_gen_xor_reg(cb, cb, dest);
    } else {
        /*
         * DEST,C = IN1 + ~IN2 + 1.  We can produce the same result in fewer
         * operations by seeding the high word with 1 and subtracting.
         */
        TCGv_reg one = tcg_constant_reg(1);
        tcg_gen_sub2_reg(dest, cb_msb, in1, one, in2, zero);
        tcg_gen_eqv_reg(cb, in1, in2);
        tcg_gen_xor_reg(cb, cb, dest);
    }

    /* Compute signed overflow if required.  */
    sv = NULL;
    if (is_tsv || cond_need_sv(c)) {
        sv = do_sub_sv(ctx, dest, in1, in2);
        if (is_tsv) {
            gen_helper_tsv(tcg_env, sv);
        }
    }

    /* Compute the condition.  We cannot use the special case for borrow.  */
    if (!is_b) {
        cond = do_sub_cond(ctx, cf, d, dest, in1, in2, sv);
    } else {
        cond = do_cond(ctx, cf, d, dest, get_carry(ctx, d, cb, cb_msb), sv);
    }

    /* Emit any conditional trap before any writeback.  */
    if (is_tc) {
        tmp = tcg_temp_new();
        tcg_gen_setcond_reg(cond.c, tmp, cond.a0, cond.a1);
        gen_helper_tcond(tcg_env, tmp);
    }

    /* Write back the result.  */
    save_or_nullify(ctx, cpu_psw_cb, cb);
    save_or_nullify(ctx, cpu_psw_cb_msb, cb_msb);
    save_gpr(ctx, rt, dest);

    /* Install the new nullification.  */
    cond_free(&ctx->null_cond);
    ctx->null_cond = cond;
}

static bool do_sub_reg(DisasContext *ctx, arg_rrr_cf_d *a,
                       bool is_tsv, bool is_b, bool is_tc)
{
    TCGv_reg tcg_r1, tcg_r2;

    if (a->cf) {
        nullify_over(ctx);
    }
    tcg_r1 = load_gpr(ctx, a->r1);
    tcg_r2 = load_gpr(ctx, a->r2);
    do_sub(ctx, a->t, tcg_r1, tcg_r2, is_tsv, is_b, is_tc, a->cf, a->d);
    return nullify_end(ctx);
}

static bool do_sub_imm(DisasContext *ctx, arg_rri_cf *a, bool is_tsv)
{
    TCGv_reg tcg_im, tcg_r2;

    if (a->cf) {
        nullify_over(ctx);
    }
    tcg_im = tcg_constant_reg(a->i);
    tcg_r2 = load_gpr(ctx, a->r);
    /* All SUBI conditions are 32-bit. */
    do_sub(ctx, a->t, tcg_im, tcg_r2, is_tsv, 0, 0, a->cf, false);
    return nullify_end(ctx);
}

static void do_cmpclr(DisasContext *ctx, unsigned rt, TCGv_reg in1,
                      TCGv_reg in2, unsigned cf, bool d)
{
    TCGv_reg dest, sv;
    DisasCond cond;

    dest = tcg_temp_new();
    tcg_gen_sub_reg(dest, in1, in2);

    /* Compute signed overflow if required.  */
    sv = NULL;
    if (cond_need_sv(cf >> 1)) {
        sv = do_sub_sv(ctx, dest, in1, in2);
    }

    /* Form the condition for the compare.  */
    cond = do_sub_cond(ctx, cf, d, dest, in1, in2, sv);

    /* Clear.  */
    tcg_gen_movi_reg(dest, 0);
    save_gpr(ctx, rt, dest);

    /* Install the new nullification.  */
    cond_free(&ctx->null_cond);
    ctx->null_cond = cond;
}

static void do_log(DisasContext *ctx, unsigned rt, TCGv_reg in1,
                   TCGv_reg in2, unsigned cf, bool d,
                   void (*fn)(TCGv_reg, TCGv_reg, TCGv_reg))
{
    TCGv_reg dest = dest_gpr(ctx, rt);

    /* Perform the operation, and writeback.  */
    fn(dest, in1, in2);
    save_gpr(ctx, rt, dest);

    /* Install the new nullification.  */
    cond_free(&ctx->null_cond);
    if (cf) {
        ctx->null_cond = do_log_cond(ctx, cf, d, dest);
    }
}

static bool do_log_reg(DisasContext *ctx, arg_rrr_cf_d *a,
                       void (*fn)(TCGv_reg, TCGv_reg, TCGv_reg))
{
    TCGv_reg tcg_r1, tcg_r2;

    if (a->cf) {
        nullify_over(ctx);
    }
    tcg_r1 = load_gpr(ctx, a->r1);
    tcg_r2 = load_gpr(ctx, a->r2);
    do_log(ctx, a->t, tcg_r1, tcg_r2, a->cf, a->d, fn);
    return nullify_end(ctx);
}

static void do_unit(DisasContext *ctx, unsigned rt, TCGv_reg in1,
                    TCGv_reg in2, unsigned cf, bool d, bool is_tc,
                    void (*fn)(TCGv_reg, TCGv_reg, TCGv_reg))
{
    TCGv_reg dest;
    DisasCond cond;

    if (cf == 0) {
        dest = dest_gpr(ctx, rt);
        fn(dest, in1, in2);
        save_gpr(ctx, rt, dest);
        cond_free(&ctx->null_cond);
    } else {
        dest = tcg_temp_new();
        fn(dest, in1, in2);

        cond = do_unit_cond(cf, d, dest, in1, in2);

        if (is_tc) {
            TCGv_reg tmp = tcg_temp_new();
            tcg_gen_setcond_reg(cond.c, tmp, cond.a0, cond.a1);
            gen_helper_tcond(tcg_env, tmp);
        }
        save_gpr(ctx, rt, dest);

        cond_free(&ctx->null_cond);
        ctx->null_cond = cond;
    }
}

#ifndef CONFIG_USER_ONLY
/* The "normal" usage is SP >= 0, wherein SP == 0 selects the space
   from the top 2 bits of the base register.  There are a few system
   instructions that have a 3-bit space specifier, for which SR0 is
   not special.  To handle this, pass ~SP.  */
static TCGv_i64 space_select(DisasContext *ctx, int sp, TCGv_reg base)
{
    TCGv_ptr ptr;
    TCGv_reg tmp;
    TCGv_i64 spc;

    if (sp != 0) {
        if (sp < 0) {
            sp = ~sp;
        }
        spc = tcg_temp_new_tl();
        load_spr(ctx, spc, sp);
        return spc;
    }
    if (ctx->tb_flags & TB_FLAG_SR_SAME) {
        return cpu_srH;
    }

    ptr = tcg_temp_new_ptr();
    tmp = tcg_temp_new();
    spc = tcg_temp_new_tl();

    /* Extract top 2 bits of the address, shift left 3 for uint64_t index. */
    tcg_gen_shri_reg(tmp, base, (ctx->tb_flags & PSW_W ? 64 : 32) - 5);
    tcg_gen_andi_reg(tmp, tmp, 030);
    tcg_gen_trunc_reg_ptr(ptr, tmp);

    tcg_gen_add_ptr(ptr, ptr, tcg_env);
    tcg_gen_ld_i64(spc, ptr, offsetof(CPUHPPAState, sr[4]));

    return spc;
}
#endif

static void form_gva(DisasContext *ctx, TCGv_tl *pgva, TCGv_reg *pofs,
                     unsigned rb, unsigned rx, int scale, int64_t disp,
                     unsigned sp, int modify, bool is_phys)
{
    TCGv_reg base = load_gpr(ctx, rb);
    TCGv_reg ofs;
    TCGv_tl addr;

    /* Note that RX is mutually exclusive with DISP.  */
    if (rx) {
        ofs = tcg_temp_new();
        tcg_gen_shli_reg(ofs, cpu_gr[rx], scale);
        tcg_gen_add_reg(ofs, ofs, base);
    } else if (disp || modify) {
        ofs = tcg_temp_new();
        tcg_gen_addi_reg(ofs, base, disp);
    } else {
        ofs = base;
    }

    *pofs = ofs;
    *pgva = addr = tcg_temp_new_tl();
    tcg_gen_extu_reg_tl(addr, modify <= 0 ? ofs : base);
    tcg_gen_andi_tl(addr, addr, gva_offset_mask(ctx));
#ifndef CONFIG_USER_ONLY
    if (!is_phys) {
        tcg_gen_or_tl(addr, addr, space_select(ctx, sp, base));
    }
#endif
}

/* Emit a memory load.  The modify parameter should be
 * < 0 for pre-modify,
 * > 0 for post-modify,
 * = 0 for no base register update.
 */
static void do_load_32(DisasContext *ctx, TCGv_i32 dest, unsigned rb,
                       unsigned rx, int scale, int64_t disp,
                       unsigned sp, int modify, MemOp mop)
{
    TCGv_reg ofs;
    TCGv_tl addr;

    /* Caller uses nullify_over/nullify_end.  */
    assert(ctx->null_cond.c == TCG_COND_NEVER);

    form_gva(ctx, &addr, &ofs, rb, rx, scale, disp, sp, modify,
             ctx->mmu_idx == MMU_PHYS_IDX);
    tcg_gen_qemu_ld_i32(dest, addr, ctx->mmu_idx, mop | UNALIGN(ctx));
    if (modify) {
        save_gpr(ctx, rb, ofs);
    }
}

static void do_load_64(DisasContext *ctx, TCGv_i64 dest, unsigned rb,
                       unsigned rx, int scale, int64_t disp,
                       unsigned sp, int modify, MemOp mop)
{
    TCGv_reg ofs;
    TCGv_tl addr;

    /* Caller uses nullify_over/nullify_end.  */
    assert(ctx->null_cond.c == TCG_COND_NEVER);

    form_gva(ctx, &addr, &ofs, rb, rx, scale, disp, sp, modify,
             ctx->mmu_idx == MMU_PHYS_IDX);
    tcg_gen_qemu_ld_i64(dest, addr, ctx->mmu_idx, mop | UNALIGN(ctx));
    if (modify) {
        save_gpr(ctx, rb, ofs);
    }
}

static void do_store_32(DisasContext *ctx, TCGv_i32 src, unsigned rb,
                        unsigned rx, int scale, int64_t disp,
                        unsigned sp, int modify, MemOp mop)
{
    TCGv_reg ofs;
    TCGv_tl addr;

    /* Caller uses nullify_over/nullify_end.  */
    assert(ctx->null_cond.c == TCG_COND_NEVER);

    form_gva(ctx, &addr, &ofs, rb, rx, scale, disp, sp, modify,
             ctx->mmu_idx == MMU_PHYS_IDX);
    tcg_gen_qemu_st_i32(src, addr, ctx->mmu_idx, mop | UNALIGN(ctx));
    if (modify) {
        save_gpr(ctx, rb, ofs);
    }
}

static void do_store_64(DisasContext *ctx, TCGv_i64 src, unsigned rb,
                        unsigned rx, int scale, int64_t disp,
                        unsigned sp, int modify, MemOp mop)
{
    TCGv_reg ofs;
    TCGv_tl addr;

    /* Caller uses nullify_over/nullify_end.  */
    assert(ctx->null_cond.c == TCG_COND_NEVER);

    form_gva(ctx, &addr, &ofs, rb, rx, scale, disp, sp, modify,
             ctx->mmu_idx == MMU_PHYS_IDX);
    tcg_gen_qemu_st_i64(src, addr, ctx->mmu_idx, mop | UNALIGN(ctx));
    if (modify) {
        save_gpr(ctx, rb, ofs);
    }
}

#define do_load_reg   do_load_64
#define do_store_reg  do_store_64

static bool do_load(DisasContext *ctx, unsigned rt, unsigned rb,
                    unsigned rx, int scale, int64_t disp,
                    unsigned sp, int modify, MemOp mop)
{
    TCGv_reg dest;

    nullify_over(ctx);

    if (modify == 0) {
        /* No base register update.  */
        dest = dest_gpr(ctx, rt);
    } else {
        /* Make sure if RT == RB, we see the result of the load.  */
        dest = tcg_temp_new();
    }
    do_load_reg(ctx, dest, rb, rx, scale, disp, sp, modify, mop);
    save_gpr(ctx, rt, dest);

    return nullify_end(ctx);
}

static bool do_floadw(DisasContext *ctx, unsigned rt, unsigned rb,
                      unsigned rx, int scale, int64_t disp,
                      unsigned sp, int modify)
{
    TCGv_i32 tmp;

    nullify_over(ctx);

    tmp = tcg_temp_new_i32();
    do_load_32(ctx, tmp, rb, rx, scale, disp, sp, modify, MO_TEUL);
    save_frw_i32(rt, tmp);

    if (rt == 0) {
        gen_helper_loaded_fr0(tcg_env);
    }

    return nullify_end(ctx);
}

static bool trans_fldw(DisasContext *ctx, arg_ldst *a)
{
    return do_floadw(ctx, a->t, a->b, a->x, a->scale ? 2 : 0,
                     a->disp, a->sp, a->m);
}

static bool do_floadd(DisasContext *ctx, unsigned rt, unsigned rb,
                      unsigned rx, int scale, int64_t disp,
                      unsigned sp, int modify)
{
    TCGv_i64 tmp;

    nullify_over(ctx);

    tmp = tcg_temp_new_i64();
    do_load_64(ctx, tmp, rb, rx, scale, disp, sp, modify, MO_TEUQ);
    save_frd(rt, tmp);

    if (rt == 0) {
        gen_helper_loaded_fr0(tcg_env);
    }

    return nullify_end(ctx);
}

static bool trans_fldd(DisasContext *ctx, arg_ldst *a)
{
    return do_floadd(ctx, a->t, a->b, a->x, a->scale ? 3 : 0,
                     a->disp, a->sp, a->m);
}

static bool do_store(DisasContext *ctx, unsigned rt, unsigned rb,
                     int64_t disp, unsigned sp,
                     int modify, MemOp mop)
{
    nullify_over(ctx);
    do_store_reg(ctx, load_gpr(ctx, rt), rb, 0, 0, disp, sp, modify, mop);
    return nullify_end(ctx);
}

static bool do_fstorew(DisasContext *ctx, unsigned rt, unsigned rb,
                       unsigned rx, int scale, int64_t disp,
                       unsigned sp, int modify)
{
    TCGv_i32 tmp;

    nullify_over(ctx);

    tmp = load_frw_i32(rt);
    do_store_32(ctx, tmp, rb, rx, scale, disp, sp, modify, MO_TEUL);

    return nullify_end(ctx);
}

static bool trans_fstw(DisasContext *ctx, arg_ldst *a)
{
    return do_fstorew(ctx, a->t, a->b, a->x, a->scale ? 2 : 0,
                      a->disp, a->sp, a->m);
}

static bool do_fstored(DisasContext *ctx, unsigned rt, unsigned rb,
                       unsigned rx, int scale, int64_t disp,
                       unsigned sp, int modify)
{
    TCGv_i64 tmp;

    nullify_over(ctx);

    tmp = load_frd(rt);
    do_store_64(ctx, tmp, rb, rx, scale, disp, sp, modify, MO_TEUQ);

    return nullify_end(ctx);
}

static bool trans_fstd(DisasContext *ctx, arg_ldst *a)
{
    return do_fstored(ctx, a->t, a->b, a->x, a->scale ? 3 : 0,
                      a->disp, a->sp, a->m);
}

static bool do_fop_wew(DisasContext *ctx, unsigned rt, unsigned ra,
                       void (*func)(TCGv_i32, TCGv_env, TCGv_i32))
{
    TCGv_i32 tmp;

    nullify_over(ctx);
    tmp = load_frw0_i32(ra);

    func(tmp, tcg_env, tmp);

    save_frw_i32(rt, tmp);
    return nullify_end(ctx);
}

static bool do_fop_wed(DisasContext *ctx, unsigned rt, unsigned ra,
                       void (*func)(TCGv_i32, TCGv_env, TCGv_i64))
{
    TCGv_i32 dst;
    TCGv_i64 src;

    nullify_over(ctx);
    src = load_frd(ra);
    dst = tcg_temp_new_i32();

    func(dst, tcg_env, src);

    save_frw_i32(rt, dst);
    return nullify_end(ctx);
}

static bool do_fop_ded(DisasContext *ctx, unsigned rt, unsigned ra,
                       void (*func)(TCGv_i64, TCGv_env, TCGv_i64))
{
    TCGv_i64 tmp;

    nullify_over(ctx);
    tmp = load_frd0(ra);

    func(tmp, tcg_env, tmp);

    save_frd(rt, tmp);
    return nullify_end(ctx);
}

static bool do_fop_dew(DisasContext *ctx, unsigned rt, unsigned ra,
                       void (*func)(TCGv_i64, TCGv_env, TCGv_i32))
{
    TCGv_i32 src;
    TCGv_i64 dst;

    nullify_over(ctx);
    src = load_frw0_i32(ra);
    dst = tcg_temp_new_i64();

    func(dst, tcg_env, src);

    save_frd(rt, dst);
    return nullify_end(ctx);
}

static bool do_fop_weww(DisasContext *ctx, unsigned rt,
                        unsigned ra, unsigned rb,
                        void (*func)(TCGv_i32, TCGv_env, TCGv_i32, TCGv_i32))
{
    TCGv_i32 a, b;

    nullify_over(ctx);
    a = load_frw0_i32(ra);
    b = load_frw0_i32(rb);

    func(a, tcg_env, a, b);

    save_frw_i32(rt, a);
    return nullify_end(ctx);
}

static bool do_fop_dedd(DisasContext *ctx, unsigned rt,
                        unsigned ra, unsigned rb,
                        void (*func)(TCGv_i64, TCGv_env, TCGv_i64, TCGv_i64))
{
    TCGv_i64 a, b;

    nullify_over(ctx);
    a = load_frd0(ra);
    b = load_frd0(rb);

    func(a, tcg_env, a, b);

    save_frd(rt, a);
    return nullify_end(ctx);
}

/* Emit an unconditional branch to a direct target, which may or may not
   have already had nullification handled.  */
static bool do_dbranch(DisasContext *ctx, uint64_t dest,
                       unsigned link, bool is_n)
{
    if (ctx->null_cond.c == TCG_COND_NEVER && ctx->null_lab == NULL) {
        if (link != 0) {
            copy_iaoq_entry(ctx, cpu_gr[link], ctx->iaoq_n, ctx->iaoq_n_var);
        }
        ctx->iaoq_n = dest;
        if (is_n) {
            ctx->null_cond.c = TCG_COND_ALWAYS;
        }
    } else {
        nullify_over(ctx);

        if (link != 0) {
            copy_iaoq_entry(ctx, cpu_gr[link], ctx->iaoq_n, ctx->iaoq_n_var);
        }

        if (is_n && use_nullify_skip(ctx)) {
            nullify_set(ctx, 0);
            gen_goto_tb(ctx, 0, dest, dest + 4);
        } else {
            nullify_set(ctx, is_n);
            gen_goto_tb(ctx, 0, ctx->iaoq_b, dest);
        }

        nullify_end(ctx);

        nullify_set(ctx, 0);
        gen_goto_tb(ctx, 1, ctx->iaoq_b, ctx->iaoq_n);
        ctx->base.is_jmp = DISAS_NORETURN;
    }
    return true;
}

/* Emit a conditional branch to a direct target.  If the branch itself
   is nullified, we should have already used nullify_over.  */
static bool do_cbranch(DisasContext *ctx, int64_t disp, bool is_n,
                       DisasCond *cond)
{
    uint64_t dest = iaoq_dest(ctx, disp);
    TCGLabel *taken = NULL;
    TCGCond c = cond->c;
    bool n;

    assert(ctx->null_cond.c == TCG_COND_NEVER);

    /* Handle TRUE and NEVER as direct branches.  */
    if (c == TCG_COND_ALWAYS) {
        return do_dbranch(ctx, dest, 0, is_n && disp >= 0);
    }
    if (c == TCG_COND_NEVER) {
        return do_dbranch(ctx, ctx->iaoq_n, 0, is_n && disp < 0);
    }

    taken = gen_new_label();
    tcg_gen_brcond_reg(c, cond->a0, cond->a1, taken);
    cond_free(cond);

    /* Not taken: Condition not satisfied; nullify on backward branches. */
    n = is_n && disp < 0;
    if (n && use_nullify_skip(ctx)) {
        nullify_set(ctx, 0);
        gen_goto_tb(ctx, 0, ctx->iaoq_n, ctx->iaoq_n + 4);
    } else {
        if (!n && ctx->null_lab) {
            gen_set_label(ctx->null_lab);
            ctx->null_lab = NULL;
        }
        nullify_set(ctx, n);
        if (ctx->iaoq_n == -1) {
            /* The temporary iaoq_n_var died at the branch above.
               Regenerate it here instead of saving it.  */
            tcg_gen_addi_reg(ctx->iaoq_n_var, cpu_iaoq_b, 4);
        }
        gen_goto_tb(ctx, 0, ctx->iaoq_b, ctx->iaoq_n);
    }

    gen_set_label(taken);

    /* Taken: Condition satisfied; nullify on forward branches.  */
    n = is_n && disp >= 0;
    if (n && use_nullify_skip(ctx)) {
        nullify_set(ctx, 0);
        gen_goto_tb(ctx, 1, dest, dest + 4);
    } else {
        nullify_set(ctx, n);
        gen_goto_tb(ctx, 1, ctx->iaoq_b, dest);
    }

    /* Not taken: the branch itself was nullified.  */
    if (ctx->null_lab) {
        gen_set_label(ctx->null_lab);
        ctx->null_lab = NULL;
        ctx->base.is_jmp = DISAS_IAQ_N_STALE;
    } else {
        ctx->base.is_jmp = DISAS_NORETURN;
    }
    return true;
}

/* Emit an unconditional branch to an indirect target.  This handles
   nullification of the branch itself.  */
static bool do_ibranch(DisasContext *ctx, TCGv_reg dest,
                       unsigned link, bool is_n)
{
    TCGv_reg a0, a1, next, tmp;
    TCGCond c;

    assert(ctx->null_lab == NULL);

    if (ctx->null_cond.c == TCG_COND_NEVER) {
        if (link != 0) {
            copy_iaoq_entry(ctx, cpu_gr[link], ctx->iaoq_n, ctx->iaoq_n_var);
        }
        next = tcg_temp_new();
        tcg_gen_mov_reg(next, dest);
        if (is_n) {
            if (use_nullify_skip(ctx)) {
                copy_iaoq_entry(ctx, cpu_iaoq_f, -1, next);
                tcg_gen_addi_reg(next, next, 4);
                copy_iaoq_entry(ctx, cpu_iaoq_b, -1, next);
                nullify_set(ctx, 0);
                ctx->base.is_jmp = DISAS_IAQ_N_UPDATED;
                return true;
            }
            ctx->null_cond.c = TCG_COND_ALWAYS;
        }
        ctx->iaoq_n = -1;
        ctx->iaoq_n_var = next;
    } else if (is_n && use_nullify_skip(ctx)) {
        /* The (conditional) branch, B, nullifies the next insn, N,
           and we're allowed to skip execution N (no single-step or
           tracepoint in effect).  Since the goto_ptr that we must use
           for the indirect branch consumes no special resources, we
           can (conditionally) skip B and continue execution.  */
        /* The use_nullify_skip test implies we have a known control path.  */
        tcg_debug_assert(ctx->iaoq_b != -1);
        tcg_debug_assert(ctx->iaoq_n != -1);

        /* We do have to handle the non-local temporary, DEST, before
           branching.  Since IOAQ_F is not really live at this point, we
           can simply store DEST optimistically.  Similarly with IAOQ_B.  */
        copy_iaoq_entry(ctx, cpu_iaoq_f, -1, dest);
        next = tcg_temp_new();
        tcg_gen_addi_reg(next, dest, 4);
        copy_iaoq_entry(ctx, cpu_iaoq_b, -1, next);

        nullify_over(ctx);
        if (link != 0) {
            copy_iaoq_entry(ctx, cpu_gr[link], ctx->iaoq_n, ctx->iaoq_n_var);
        }
        tcg_gen_lookup_and_goto_ptr();
        return nullify_end(ctx);
    } else {
        c = ctx->null_cond.c;
        a0 = ctx->null_cond.a0;
        a1 = ctx->null_cond.a1;

        tmp = tcg_temp_new();
        next = tcg_temp_new();

        copy_iaoq_entry(ctx, tmp, ctx->iaoq_n, ctx->iaoq_n_var);
        tcg_gen_movcond_reg(c, next, a0, a1, tmp, dest);
        ctx->iaoq_n = -1;
        ctx->iaoq_n_var = next;

        if (link != 0) {
            tcg_gen_movcond_reg(c, cpu_gr[link], a0, a1, cpu_gr[link], tmp);
        }

        if (is_n) {
            /* The branch nullifies the next insn, which means the state of N
               after the branch is the inverse of the state of N that applied
               to the branch.  */
            tcg_gen_setcond_reg(tcg_invert_cond(c), cpu_psw_n, a0, a1);
            cond_free(&ctx->null_cond);
            ctx->null_cond = cond_make_n();
            ctx->psw_n_nonzero = true;
        } else {
            cond_free(&ctx->null_cond);
        }
    }
    return true;
}

/* Implement
 *    if (IAOQ_Front{30..31} < GR[b]{30..31})
 *      IAOQ_Next{30..31} ← GR[b]{30..31};
 *    else
 *      IAOQ_Next{30..31} ← IAOQ_Front{30..31};
 * which keeps the privilege level from being increased.
 */
static TCGv_reg do_ibranch_priv(DisasContext *ctx, TCGv_reg offset)
{
    TCGv_reg dest;
    switch (ctx->privilege) {
    case 0:
        /* Privilege 0 is maximum and is allowed to decrease.  */
        return offset;
    case 3:
        /* Privilege 3 is minimum and is never allowed to increase.  */
        dest = tcg_temp_new();
        tcg_gen_ori_reg(dest, offset, 3);
        break;
    default:
        dest = tcg_temp_new();
        tcg_gen_andi_reg(dest, offset, -4);
        tcg_gen_ori_reg(dest, dest, ctx->privilege);
        tcg_gen_movcond_reg(TCG_COND_GTU, dest, dest, offset, dest, offset);
        break;
    }
    return dest;
}

#ifdef CONFIG_USER_ONLY
/* On Linux, page zero is normally marked execute only + gateway.
   Therefore normal read or write is supposed to fail, but specific
   offsets have kernel code mapped to raise permissions to implement
   system calls.  Handling this via an explicit check here, rather
   in than the "be disp(sr2,r0)" instruction that probably sent us
   here, is the easiest way to handle the branch delay slot on the
   aforementioned BE.  */
static void do_page_zero(DisasContext *ctx)
{
    TCGv_reg tmp;

    /* If by some means we get here with PSW[N]=1, that implies that
       the B,GATE instruction would be skipped, and we'd fault on the
       next insn within the privileged page.  */
    switch (ctx->null_cond.c) {
    case TCG_COND_NEVER:
        break;
    case TCG_COND_ALWAYS:
        tcg_gen_movi_reg(cpu_psw_n, 0);
        goto do_sigill;
    default:
        /* Since this is always the first (and only) insn within the
           TB, we should know the state of PSW[N] from TB->FLAGS.  */
        g_assert_not_reached();
    }

    /* Check that we didn't arrive here via some means that allowed
       non-sequential instruction execution.  Normally the PSW[B] bit
       detects this by disallowing the B,GATE instruction to execute
       under such conditions.  */
    if (ctx->iaoq_b != ctx->iaoq_f + 4) {
        goto do_sigill;
    }

    switch (ctx->iaoq_f & -4) {
    case 0x00: /* Null pointer call */
        gen_excp_1(EXCP_IMP);
        ctx->base.is_jmp = DISAS_NORETURN;
        break;

    case 0xb0: /* LWS */
        gen_excp_1(EXCP_SYSCALL_LWS);
        ctx->base.is_jmp = DISAS_NORETURN;
        break;

    case 0xe0: /* SET_THREAD_POINTER */
        tcg_gen_st_reg(cpu_gr[26], tcg_env, offsetof(CPUHPPAState, cr[27]));
        tmp = tcg_temp_new();
        tcg_gen_ori_reg(tmp, cpu_gr[31], 3);
        copy_iaoq_entry(ctx, cpu_iaoq_f, -1, tmp);
        tcg_gen_addi_reg(tmp, tmp, 4);
        copy_iaoq_entry(ctx, cpu_iaoq_b, -1, tmp);
        ctx->base.is_jmp = DISAS_IAQ_N_UPDATED;
        break;

    case 0x100: /* SYSCALL */
        gen_excp_1(EXCP_SYSCALL);
        ctx->base.is_jmp = DISAS_NORETURN;
        break;

    default:
    do_sigill:
        gen_excp_1(EXCP_ILL);
        ctx->base.is_jmp = DISAS_NORETURN;
        break;
    }
}
#endif

static bool trans_nop(DisasContext *ctx, arg_nop *a)
{
    cond_free(&ctx->null_cond);
    return true;
}

static bool trans_break(DisasContext *ctx, arg_break *a)
{
    return gen_excp_iir(ctx, EXCP_BREAK);
}

static bool trans_sync(DisasContext *ctx, arg_sync *a)
{
    /* No point in nullifying the memory barrier.  */
    tcg_gen_mb(TCG_BAR_SC | TCG_MO_ALL);

    cond_free(&ctx->null_cond);
    return true;
}

static bool trans_mfia(DisasContext *ctx, arg_mfia *a)
{
    unsigned rt = a->t;
    TCGv_reg tmp = dest_gpr(ctx, rt);
    tcg_gen_movi_reg(tmp, ctx->iaoq_f);
    save_gpr(ctx, rt, tmp);

    cond_free(&ctx->null_cond);
    return true;
}

static bool trans_mfsp(DisasContext *ctx, arg_mfsp *a)
{
    unsigned rt = a->t;
    unsigned rs = a->sp;
    TCGv_i64 t0 = tcg_temp_new_i64();
    TCGv_reg t1 = tcg_temp_new();

    load_spr(ctx, t0, rs);
    tcg_gen_shri_i64(t0, t0, 32);
    tcg_gen_trunc_i64_reg(t1, t0);

    save_gpr(ctx, rt, t1);

    cond_free(&ctx->null_cond);
    return true;
}

static bool trans_mfctl(DisasContext *ctx, arg_mfctl *a)
{
    unsigned rt = a->t;
    unsigned ctl = a->r;
    TCGv_reg tmp;

    switch (ctl) {
    case CR_SAR:
        if (a->e == 0) {
            /* MFSAR without ,W masks low 5 bits.  */
            tmp = dest_gpr(ctx, rt);
            tcg_gen_andi_reg(tmp, cpu_sar, 31);
            save_gpr(ctx, rt, tmp);
            goto done;
        }
        save_gpr(ctx, rt, cpu_sar);
        goto done;
    case CR_IT: /* Interval Timer */
        /* FIXME: Respect PSW_S bit.  */
        nullify_over(ctx);
        tmp = dest_gpr(ctx, rt);
        if (translator_io_start(&ctx->base)) {
            gen_helper_read_interval_timer(tmp);
            ctx->base.is_jmp = DISAS_IAQ_N_STALE;
        } else {
            gen_helper_read_interval_timer(tmp);
        }
        save_gpr(ctx, rt, tmp);
        return nullify_end(ctx);
    case 26:
    case 27:
        break;
    default:
        /* All other control registers are privileged.  */
        CHECK_MOST_PRIVILEGED(EXCP_PRIV_REG);
        break;
    }

    tmp = tcg_temp_new();
    tcg_gen_ld_reg(tmp, tcg_env, offsetof(CPUHPPAState, cr[ctl]));
    save_gpr(ctx, rt, tmp);

 done:
    cond_free(&ctx->null_cond);
    return true;
}

static bool trans_mtsp(DisasContext *ctx, arg_mtsp *a)
{
    unsigned rr = a->r;
    unsigned rs = a->sp;
    TCGv_i64 t64;

    if (rs >= 5) {
        CHECK_MOST_PRIVILEGED(EXCP_PRIV_REG);
    }
    nullify_over(ctx);

    t64 = tcg_temp_new_i64();
    tcg_gen_extu_reg_i64(t64, load_gpr(ctx, rr));
    tcg_gen_shli_i64(t64, t64, 32);

    if (rs >= 4) {
        tcg_gen_st_i64(t64, tcg_env, offsetof(CPUHPPAState, sr[rs]));
        ctx->tb_flags &= ~TB_FLAG_SR_SAME;
    } else {
        tcg_gen_mov_i64(cpu_sr[rs], t64);
    }

    return nullify_end(ctx);
}

static bool trans_mtctl(DisasContext *ctx, arg_mtctl *a)
{
    unsigned ctl = a->t;
    TCGv_reg reg;
    TCGv_reg tmp;

    if (ctl == CR_SAR) {
        reg = load_gpr(ctx, a->r);
        tmp = tcg_temp_new();
        tcg_gen_andi_reg(tmp, reg, ctx->is_pa20 ? 63 : 31);
        save_or_nullify(ctx, cpu_sar, tmp);

        cond_free(&ctx->null_cond);
        return true;
    }

    /* All other control registers are privileged or read-only.  */
    CHECK_MOST_PRIVILEGED(EXCP_PRIV_REG);

#ifndef CONFIG_USER_ONLY
    nullify_over(ctx);
    reg = load_gpr(ctx, a->r);

    switch (ctl) {
    case CR_IT:
        gen_helper_write_interval_timer(tcg_env, reg);
        break;
    case CR_EIRR:
        gen_helper_write_eirr(tcg_env, reg);
        break;
    case CR_EIEM:
        gen_helper_write_eiem(tcg_env, reg);
        ctx->base.is_jmp = DISAS_IAQ_N_STALE_EXIT;
        break;

    case CR_IIASQ:
    case CR_IIAOQ:
        /* FIXME: Respect PSW_Q bit */
        /* The write advances the queue and stores to the back element.  */
        tmp = tcg_temp_new();
        tcg_gen_ld_reg(tmp, tcg_env,
                       offsetof(CPUHPPAState, cr_back[ctl - CR_IIASQ]));
        tcg_gen_st_reg(tmp, tcg_env, offsetof(CPUHPPAState, cr[ctl]));
        tcg_gen_st_reg(reg, tcg_env,
                       offsetof(CPUHPPAState, cr_back[ctl - CR_IIASQ]));
        break;

    case CR_PID1:
    case CR_PID2:
    case CR_PID3:
    case CR_PID4:
        tcg_gen_st_reg(reg, tcg_env, offsetof(CPUHPPAState, cr[ctl]));
#ifndef CONFIG_USER_ONLY
        gen_helper_change_prot_id(tcg_env);
#endif
        break;

    default:
        tcg_gen_st_reg(reg, tcg_env, offsetof(CPUHPPAState, cr[ctl]));
        break;
    }
    return nullify_end(ctx);
#endif
}

static bool trans_mtsarcm(DisasContext *ctx, arg_mtsarcm *a)
{
    TCGv_reg tmp = tcg_temp_new();

    tcg_gen_not_reg(tmp, load_gpr(ctx, a->r));
    tcg_gen_andi_reg(tmp, tmp, ctx->is_pa20 ? 63 : 31);
    save_or_nullify(ctx, cpu_sar, tmp);

    cond_free(&ctx->null_cond);
    return true;
}

static bool trans_ldsid(DisasContext *ctx, arg_ldsid *a)
{
    TCGv_reg dest = dest_gpr(ctx, a->t);

#ifdef CONFIG_USER_ONLY
    /* We don't implement space registers in user mode. */
    tcg_gen_movi_reg(dest, 0);
#else
    TCGv_i64 t0 = tcg_temp_new_i64();

    tcg_gen_mov_i64(t0, space_select(ctx, a->sp, load_gpr(ctx, a->b)));
    tcg_gen_shri_i64(t0, t0, 32);
    tcg_gen_trunc_i64_reg(dest, t0);
#endif
    save_gpr(ctx, a->t, dest);

    cond_free(&ctx->null_cond);
    return true;
}

static bool trans_rsm(DisasContext *ctx, arg_rsm *a)
{
    CHECK_MOST_PRIVILEGED(EXCP_PRIV_OPR);
#ifndef CONFIG_USER_ONLY
    TCGv_reg tmp;

    nullify_over(ctx);

    tmp = tcg_temp_new();
    tcg_gen_ld_reg(tmp, tcg_env, offsetof(CPUHPPAState, psw));
    tcg_gen_andi_reg(tmp, tmp, ~a->i);
    gen_helper_swap_system_mask(tmp, tcg_env, tmp);
    save_gpr(ctx, a->t, tmp);

    /* Exit the TB to recognize new interrupts, e.g. PSW_M.  */
    ctx->base.is_jmp = DISAS_IAQ_N_STALE_EXIT;
    return nullify_end(ctx);
#endif
}

static bool trans_ssm(DisasContext *ctx, arg_ssm *a)
{
    CHECK_MOST_PRIVILEGED(EXCP_PRIV_OPR);
#ifndef CONFIG_USER_ONLY
    TCGv_reg tmp;

    nullify_over(ctx);

    tmp = tcg_temp_new();
    tcg_gen_ld_reg(tmp, tcg_env, offsetof(CPUHPPAState, psw));
    tcg_gen_ori_reg(tmp, tmp, a->i);
    gen_helper_swap_system_mask(tmp, tcg_env, tmp);
    save_gpr(ctx, a->t, tmp);

    /* Exit the TB to recognize new interrupts, e.g. PSW_I.  */
    ctx->base.is_jmp = DISAS_IAQ_N_STALE_EXIT;
    return nullify_end(ctx);
#endif
}

static bool trans_mtsm(DisasContext *ctx, arg_mtsm *a)
{
    CHECK_MOST_PRIVILEGED(EXCP_PRIV_OPR);
#ifndef CONFIG_USER_ONLY
    TCGv_reg tmp, reg;
    nullify_over(ctx);

    reg = load_gpr(ctx, a->r);
    tmp = tcg_temp_new();
    gen_helper_swap_system_mask(tmp, tcg_env, reg);

    /* Exit the TB to recognize new interrupts.  */
    ctx->base.is_jmp = DISAS_IAQ_N_STALE_EXIT;
    return nullify_end(ctx);
#endif
}

static bool do_rfi(DisasContext *ctx, bool rfi_r)
{
    CHECK_MOST_PRIVILEGED(EXCP_PRIV_OPR);
#ifndef CONFIG_USER_ONLY
    nullify_over(ctx);

    if (rfi_r) {
        gen_helper_rfi_r(tcg_env);
    } else {
        gen_helper_rfi(tcg_env);
    }
    /* Exit the TB to recognize new interrupts.  */
    tcg_gen_exit_tb(NULL, 0);
    ctx->base.is_jmp = DISAS_NORETURN;

    return nullify_end(ctx);
#endif
}

static bool trans_rfi(DisasContext *ctx, arg_rfi *a)
{
    return do_rfi(ctx, false);
}

static bool trans_rfi_r(DisasContext *ctx, arg_rfi_r *a)
{
    return do_rfi(ctx, true);
}

static bool trans_halt(DisasContext *ctx, arg_halt *a)
{
    CHECK_MOST_PRIVILEGED(EXCP_PRIV_OPR);
#ifndef CONFIG_USER_ONLY
    nullify_over(ctx);
    gen_helper_halt(tcg_env);
    ctx->base.is_jmp = DISAS_NORETURN;
    return nullify_end(ctx);
#endif
}

static bool trans_reset(DisasContext *ctx, arg_reset *a)
{
    CHECK_MOST_PRIVILEGED(EXCP_PRIV_OPR);
#ifndef CONFIG_USER_ONLY
    nullify_over(ctx);
    gen_helper_reset(tcg_env);
    ctx->base.is_jmp = DISAS_NORETURN;
    return nullify_end(ctx);
#endif
}

static bool trans_getshadowregs(DisasContext *ctx, arg_getshadowregs *a)
{
    CHECK_MOST_PRIVILEGED(EXCP_PRIV_OPR);
#ifndef CONFIG_USER_ONLY
    nullify_over(ctx);
    gen_helper_getshadowregs(tcg_env);
    return nullify_end(ctx);
#endif
}

static bool trans_nop_addrx(DisasContext *ctx, arg_ldst *a)
{
    if (a->m) {
        TCGv_reg dest = dest_gpr(ctx, a->b);
        TCGv_reg src1 = load_gpr(ctx, a->b);
        TCGv_reg src2 = load_gpr(ctx, a->x);

        /* The only thing we need to do is the base register modification.  */
        tcg_gen_add_reg(dest, src1, src2);
        save_gpr(ctx, a->b, dest);
    }
    cond_free(&ctx->null_cond);
    return true;
}

static bool trans_probe(DisasContext *ctx, arg_probe *a)
{
    TCGv_reg dest, ofs;
    TCGv_i32 level, want;
    TCGv_tl addr;

    nullify_over(ctx);

    dest = dest_gpr(ctx, a->t);
    form_gva(ctx, &addr, &ofs, a->b, 0, 0, 0, a->sp, 0, false);

    if (a->imm) {
        level = tcg_constant_i32(a->ri);
    } else {
        level = tcg_temp_new_i32();
        tcg_gen_trunc_reg_i32(level, load_gpr(ctx, a->ri));
        tcg_gen_andi_i32(level, level, 3);
    }
    want = tcg_constant_i32(a->write ? PAGE_WRITE : PAGE_READ);

    gen_helper_probe(dest, tcg_env, addr, level, want);

    save_gpr(ctx, a->t, dest);
    return nullify_end(ctx);
}

static bool trans_ixtlbx(DisasContext *ctx, arg_ixtlbx *a)
{
    if (ctx->is_pa20) {
        return false;
    }
    CHECK_MOST_PRIVILEGED(EXCP_PRIV_OPR);
#ifndef CONFIG_USER_ONLY
    TCGv_tl addr;
    TCGv_reg ofs, reg;

    nullify_over(ctx);

    form_gva(ctx, &addr, &ofs, a->b, 0, 0, 0, a->sp, 0, false);
    reg = load_gpr(ctx, a->r);
    if (a->addr) {
        gen_helper_itlba_pa11(tcg_env, addr, reg);
    } else {
        gen_helper_itlbp_pa11(tcg_env, addr, reg);
    }

    /* Exit TB for TLB change if mmu is enabled.  */
    if (ctx->tb_flags & PSW_C) {
        ctx->base.is_jmp = DISAS_IAQ_N_STALE;
    }
    return nullify_end(ctx);
#endif
}

static bool trans_pxtlbx(DisasContext *ctx, arg_pxtlbx *a)
{
    CHECK_MOST_PRIVILEGED(EXCP_PRIV_OPR);
#ifndef CONFIG_USER_ONLY
    TCGv_tl addr;
    TCGv_reg ofs;

    nullify_over(ctx);

    form_gva(ctx, &addr, &ofs, a->b, a->x, 0, 0, a->sp, a->m, false);
    if (a->m) {
        save_gpr(ctx, a->b, ofs);
    }
    if (a->local) {
        gen_helper_ptlbe(tcg_env);
    } else {
        gen_helper_ptlb(tcg_env, addr);
    }

    /* Exit TB for TLB change if mmu is enabled.  */
    if (ctx->tb_flags & PSW_C) {
        ctx->base.is_jmp = DISAS_IAQ_N_STALE;
    }
    return nullify_end(ctx);
#endif
}

/*
 * Implement the pcxl and pcxl2 Fast TLB Insert instructions.
 * See
 *     https://parisc.wiki.kernel.org/images-parisc/a/a9/Pcxl2_ers.pdf
 *     page 13-9 (195/206)
 */
static bool trans_ixtlbxf(DisasContext *ctx, arg_ixtlbxf *a)
{
    if (ctx->is_pa20) {
        return false;
    }
    CHECK_MOST_PRIVILEGED(EXCP_PRIV_OPR);
#ifndef CONFIG_USER_ONLY
    TCGv_tl addr, atl, stl;
    TCGv_reg reg;

    nullify_over(ctx);

    /*
     * FIXME:
     *  if (not (pcxl or pcxl2))
     *    return gen_illegal(ctx);
     */

    atl = tcg_temp_new_tl();
    stl = tcg_temp_new_tl();
    addr = tcg_temp_new_tl();

    tcg_gen_ld32u_i64(stl, tcg_env,
                      a->data ? offsetof(CPUHPPAState, cr[CR_ISR])
                      : offsetof(CPUHPPAState, cr[CR_IIASQ]));
    tcg_gen_ld32u_i64(atl, tcg_env,
                      a->data ? offsetof(CPUHPPAState, cr[CR_IOR])
                      : offsetof(CPUHPPAState, cr[CR_IIAOQ]));
    tcg_gen_shli_i64(stl, stl, 32);
    tcg_gen_or_tl(addr, atl, stl);

    reg = load_gpr(ctx, a->r);
    if (a->addr) {
        gen_helper_itlba_pa11(tcg_env, addr, reg);
    } else {
        gen_helper_itlbp_pa11(tcg_env, addr, reg);
    }

    /* Exit TB for TLB change if mmu is enabled.  */
    if (ctx->tb_flags & PSW_C) {
        ctx->base.is_jmp = DISAS_IAQ_N_STALE;
    }
    return nullify_end(ctx);
#endif
}

static bool trans_ixtlbt(DisasContext *ctx, arg_ixtlbt *a)
{
    if (!ctx->is_pa20) {
        return false;
    }
    CHECK_MOST_PRIVILEGED(EXCP_PRIV_OPR);
#ifndef CONFIG_USER_ONLY
    nullify_over(ctx);
    {
        TCGv_i64 src1 = load_gpr(ctx, a->r1);
        TCGv_i64 src2 = load_gpr(ctx, a->r2);

        if (a->data) {
            gen_helper_idtlbt_pa20(tcg_env, src1, src2);
        } else {
            gen_helper_iitlbt_pa20(tcg_env, src1, src2);
        }
    }
    /* Exit TB for TLB change if mmu is enabled.  */
    if (ctx->tb_flags & PSW_C) {
        ctx->base.is_jmp = DISAS_IAQ_N_STALE;
    }
    return nullify_end(ctx);
#endif
}

static bool trans_lpa(DisasContext *ctx, arg_ldst *a)
{
    CHECK_MOST_PRIVILEGED(EXCP_PRIV_OPR);
#ifndef CONFIG_USER_ONLY
    TCGv_tl vaddr;
    TCGv_reg ofs, paddr;

    nullify_over(ctx);

    form_gva(ctx, &vaddr, &ofs, a->b, a->x, 0, 0, a->sp, a->m, false);

    paddr = tcg_temp_new();
    gen_helper_lpa(paddr, tcg_env, vaddr);

    /* Note that physical address result overrides base modification.  */
    if (a->m) {
        save_gpr(ctx, a->b, ofs);
    }
    save_gpr(ctx, a->t, paddr);

    return nullify_end(ctx);
#endif
}

static bool trans_lci(DisasContext *ctx, arg_lci *a)
{
    CHECK_MOST_PRIVILEGED(EXCP_PRIV_OPR);

    /* The Coherence Index is an implementation-defined function of the
       physical address.  Two addresses with the same CI have a coherent
       view of the cache.  Our implementation is to return 0 for all,
       since the entire address space is coherent.  */
    save_gpr(ctx, a->t, tcg_constant_reg(0));

    cond_free(&ctx->null_cond);
    return true;
}

static bool trans_add(DisasContext *ctx, arg_rrr_cf_d_sh *a)
{
    return do_add_reg(ctx, a, false, false, false, false);
}

static bool trans_add_l(DisasContext *ctx, arg_rrr_cf_d_sh *a)
{
    return do_add_reg(ctx, a, true, false, false, false);
}

static bool trans_add_tsv(DisasContext *ctx, arg_rrr_cf_d_sh *a)
{
    return do_add_reg(ctx, a, false, true, false, false);
}

static bool trans_add_c(DisasContext *ctx, arg_rrr_cf_d_sh *a)
{
    return do_add_reg(ctx, a, false, false, false, true);
}

static bool trans_add_c_tsv(DisasContext *ctx, arg_rrr_cf_d_sh *a)
{
    return do_add_reg(ctx, a, false, true, false, true);
}

static bool trans_sub(DisasContext *ctx, arg_rrr_cf_d *a)
{
    return do_sub_reg(ctx, a, false, false, false);
}

static bool trans_sub_tsv(DisasContext *ctx, arg_rrr_cf_d *a)
{
    return do_sub_reg(ctx, a, true, false, false);
}

static bool trans_sub_tc(DisasContext *ctx, arg_rrr_cf_d *a)
{
    return do_sub_reg(ctx, a, false, false, true);
}

static bool trans_sub_tsv_tc(DisasContext *ctx, arg_rrr_cf_d *a)
{
    return do_sub_reg(ctx, a, true, false, true);
}

static bool trans_sub_b(DisasContext *ctx, arg_rrr_cf_d *a)
{
    return do_sub_reg(ctx, a, false, true, false);
}

static bool trans_sub_b_tsv(DisasContext *ctx, arg_rrr_cf_d *a)
{
    return do_sub_reg(ctx, a, true, true, false);
}

static bool trans_andcm(DisasContext *ctx, arg_rrr_cf_d *a)
{
    return do_log_reg(ctx, a, tcg_gen_andc_reg);
}

static bool trans_and(DisasContext *ctx, arg_rrr_cf_d *a)
{
    return do_log_reg(ctx, a, tcg_gen_and_reg);
}

static bool trans_or(DisasContext *ctx, arg_rrr_cf_d *a)
{
    if (a->cf == 0) {
        unsigned r2 = a->r2;
        unsigned r1 = a->r1;
        unsigned rt = a->t;

        if (rt == 0) { /* NOP */
            cond_free(&ctx->null_cond);
            return true;
        }
        if (r2 == 0) { /* COPY */
            if (r1 == 0) {
                TCGv_reg dest = dest_gpr(ctx, rt);
                tcg_gen_movi_reg(dest, 0);
                save_gpr(ctx, rt, dest);
            } else {
                save_gpr(ctx, rt, cpu_gr[r1]);
            }
            cond_free(&ctx->null_cond);
            return true;
        }
#ifndef CONFIG_USER_ONLY
        /* These are QEMU extensions and are nops in the real architecture:
         *
         * or %r10,%r10,%r10 -- idle loop; wait for interrupt
         * or %r31,%r31,%r31 -- death loop; offline cpu
         *                      currently implemented as idle.
         */
        if ((rt == 10 || rt == 31) && r1 == rt && r2 == rt) { /* PAUSE */
            /* No need to check for supervisor, as userland can only pause
               until the next timer interrupt.  */
            nullify_over(ctx);

            /* Advance the instruction queue.  */
            copy_iaoq_entry(ctx, cpu_iaoq_f, ctx->iaoq_b, cpu_iaoq_b);
            copy_iaoq_entry(ctx, cpu_iaoq_b, ctx->iaoq_n, ctx->iaoq_n_var);
            nullify_set(ctx, 0);

            /* Tell the qemu main loop to halt until this cpu has work.  */
            tcg_gen_st_i32(tcg_constant_i32(1), tcg_env,
                           offsetof(CPUState, halted) - offsetof(HPPACPU, env));
            gen_excp_1(EXCP_HALTED);
            ctx->base.is_jmp = DISAS_NORETURN;

            return nullify_end(ctx);
        }
#endif
    }
    return do_log_reg(ctx, a, tcg_gen_or_reg);
}

static bool trans_xor(DisasContext *ctx, arg_rrr_cf_d *a)
{
    return do_log_reg(ctx, a, tcg_gen_xor_reg);
}

static bool trans_cmpclr(DisasContext *ctx, arg_rrr_cf_d *a)
{
    TCGv_reg tcg_r1, tcg_r2;

    if (a->cf) {
        nullify_over(ctx);
    }
    tcg_r1 = load_gpr(ctx, a->r1);
    tcg_r2 = load_gpr(ctx, a->r2);
    do_cmpclr(ctx, a->t, tcg_r1, tcg_r2, a->cf, a->d);
    return nullify_end(ctx);
}

static bool trans_uxor(DisasContext *ctx, arg_rrr_cf_d *a)
{
    TCGv_reg tcg_r1, tcg_r2;

    if (a->cf) {
        nullify_over(ctx);
    }
    tcg_r1 = load_gpr(ctx, a->r1);
    tcg_r2 = load_gpr(ctx, a->r2);
    do_unit(ctx, a->t, tcg_r1, tcg_r2, a->cf, a->d, false, tcg_gen_xor_reg);
    return nullify_end(ctx);
}

static bool do_uaddcm(DisasContext *ctx, arg_rrr_cf_d *a, bool is_tc)
{
    TCGv_reg tcg_r1, tcg_r2, tmp;

    if (a->cf) {
        nullify_over(ctx);
    }
    tcg_r1 = load_gpr(ctx, a->r1);
    tcg_r2 = load_gpr(ctx, a->r2);
    tmp = tcg_temp_new();
    tcg_gen_not_reg(tmp, tcg_r2);
    do_unit(ctx, a->t, tcg_r1, tmp, a->cf, a->d, is_tc, tcg_gen_add_reg);
    return nullify_end(ctx);
}

static bool trans_uaddcm(DisasContext *ctx, arg_rrr_cf_d *a)
{
    return do_uaddcm(ctx, a, false);
}

static bool trans_uaddcm_tc(DisasContext *ctx, arg_rrr_cf_d *a)
{
    return do_uaddcm(ctx, a, true);
}

static bool do_dcor(DisasContext *ctx, arg_rr_cf_d *a, bool is_i)
{
    TCGv_reg tmp;

    nullify_over(ctx);

    tmp = tcg_temp_new();
    tcg_gen_shri_reg(tmp, cpu_psw_cb, 3);
    if (!is_i) {
        tcg_gen_not_reg(tmp, tmp);
    }
    tcg_gen_andi_reg(tmp, tmp, (uint64_t)0x1111111111111111ull);
    tcg_gen_muli_reg(tmp, tmp, 6);
    do_unit(ctx, a->t, load_gpr(ctx, a->r), tmp, a->cf, a->d, false,
            is_i ? tcg_gen_add_reg : tcg_gen_sub_reg);
    return nullify_end(ctx);
}

static bool trans_dcor(DisasContext *ctx, arg_rr_cf_d *a)
{
    return do_dcor(ctx, a, false);
}

static bool trans_dcor_i(DisasContext *ctx, arg_rr_cf_d *a)
{
    return do_dcor(ctx, a, true);
}

static bool trans_ds(DisasContext *ctx, arg_rrr_cf *a)
{
    TCGv_reg dest, add1, add2, addc, zero, in1, in2;
    TCGv_reg cout;

    nullify_over(ctx);

    in1 = load_gpr(ctx, a->r1);
    in2 = load_gpr(ctx, a->r2);

    add1 = tcg_temp_new();
    add2 = tcg_temp_new();
    addc = tcg_temp_new();
    dest = tcg_temp_new();
    zero = tcg_constant_reg(0);

    /* Form R1 << 1 | PSW[CB]{8}.  */
    tcg_gen_add_reg(add1, in1, in1);
    tcg_gen_add_reg(add1, add1, get_psw_carry(ctx, false));

    /*
     * Add or subtract R2, depending on PSW[V].  Proper computation of
     * carry requires that we subtract via + ~R2 + 1, as described in
     * the manual.  By extracting and masking V, we can produce the
     * proper inputs to the addition without movcond.
     */
    tcg_gen_sextract_reg(addc, cpu_psw_v, 31, 1);
    tcg_gen_xor_reg(add2, in2, addc);
    tcg_gen_andi_reg(addc, addc, 1);

    tcg_gen_add2_reg(dest, cpu_psw_cb_msb, add1, zero, add2, zero);
    tcg_gen_add2_reg(dest, cpu_psw_cb_msb, dest, cpu_psw_cb_msb, addc, zero);

    /* Write back the result register.  */
    save_gpr(ctx, a->t, dest);

    /* Write back PSW[CB].  */
    tcg_gen_xor_reg(cpu_psw_cb, add1, add2);
    tcg_gen_xor_reg(cpu_psw_cb, cpu_psw_cb, dest);

    /* Write back PSW[V] for the division step.  */
    cout = get_psw_carry(ctx, false);
    tcg_gen_neg_reg(cpu_psw_v, cout);
    tcg_gen_xor_reg(cpu_psw_v, cpu_psw_v, in2);

    /* Install the new nullification.  */
    if (a->cf) {
        TCGv_reg sv = NULL;
        if (cond_need_sv(a->cf >> 1)) {
            /* ??? The lshift is supposed to contribute to overflow.  */
            sv = do_add_sv(ctx, dest, add1, add2);
        }
        ctx->null_cond = do_cond(ctx, a->cf, false, dest, cout, sv);
    }

    return nullify_end(ctx);
}

static bool trans_addi(DisasContext *ctx, arg_rri_cf *a)
{
    return do_add_imm(ctx, a, false, false);
}

static bool trans_addi_tsv(DisasContext *ctx, arg_rri_cf *a)
{
    return do_add_imm(ctx, a, true, false);
}

static bool trans_addi_tc(DisasContext *ctx, arg_rri_cf *a)
{
    return do_add_imm(ctx, a, false, true);
}

static bool trans_addi_tc_tsv(DisasContext *ctx, arg_rri_cf *a)
{
    return do_add_imm(ctx, a, true, true);
}

static bool trans_subi(DisasContext *ctx, arg_rri_cf *a)
{
    return do_sub_imm(ctx, a, false);
}

static bool trans_subi_tsv(DisasContext *ctx, arg_rri_cf *a)
{
    return do_sub_imm(ctx, a, true);
}

static bool trans_cmpiclr(DisasContext *ctx, arg_rri_cf_d *a)
{
    TCGv_reg tcg_im, tcg_r2;

    if (a->cf) {
        nullify_over(ctx);
    }

    tcg_im = tcg_constant_reg(a->i);
    tcg_r2 = load_gpr(ctx, a->r);
    do_cmpclr(ctx, a->t, tcg_im, tcg_r2, a->cf, a->d);

    return nullify_end(ctx);
}

static bool trans_ld(DisasContext *ctx, arg_ldst *a)
{
    if (!ctx->is_pa20 && a->size > MO_32) {
        return gen_illegal(ctx);
    }
    return do_load(ctx, a->t, a->b, a->x, a->scale ? a->size : 0,
                   a->disp, a->sp, a->m, a->size | MO_TE);
}

static bool trans_st(DisasContext *ctx, arg_ldst *a)
{
    assert(a->x == 0 && a->scale == 0);
    if (!ctx->is_pa20 && a->size > MO_32) {
        return gen_illegal(ctx);
    }
    return do_store(ctx, a->t, a->b, a->disp, a->sp, a->m, a->size | MO_TE);
}

static bool trans_ldc(DisasContext *ctx, arg_ldst *a)
{
    MemOp mop = MO_TE | MO_ALIGN | a->size;
    TCGv_reg zero, dest, ofs;
    TCGv_tl addr;

    if (!ctx->is_pa20 && a->size > MO_32) {
        return gen_illegal(ctx);
    }

    nullify_over(ctx);

    if (a->m) {
        /* Base register modification.  Make sure if RT == RB,
           we see the result of the load.  */
        dest = tcg_temp_new();
    } else {
        dest = dest_gpr(ctx, a->t);
    }

    form_gva(ctx, &addr, &ofs, a->b, a->x, a->scale ? a->size : 0,
             a->disp, a->sp, a->m, ctx->mmu_idx == MMU_PHYS_IDX);

    /*
     * For hppa1.1, LDCW is undefined unless aligned mod 16.
     * However actual hardware succeeds with aligned mod 4.
     * Detect this case and log a GUEST_ERROR.
     *
     * TODO: HPPA64 relaxes the over-alignment requirement
     * with the ,co completer.
     */
    gen_helper_ldc_check(addr);

    zero = tcg_constant_reg(0);
    tcg_gen_atomic_xchg_reg(dest, addr, zero, ctx->mmu_idx, mop);

    if (a->m) {
        save_gpr(ctx, a->b, ofs);
    }
    save_gpr(ctx, a->t, dest);

    return nullify_end(ctx);
}

static bool trans_stby(DisasContext *ctx, arg_stby *a)
{
    TCGv_reg ofs, val;
    TCGv_tl addr;

    nullify_over(ctx);

    form_gva(ctx, &addr, &ofs, a->b, 0, 0, a->disp, a->sp, a->m,
             ctx->mmu_idx == MMU_PHYS_IDX);
    val = load_gpr(ctx, a->r);
    if (a->a) {
        if (tb_cflags(ctx->base.tb) & CF_PARALLEL) {
            gen_helper_stby_e_parallel(tcg_env, addr, val);
        } else {
            gen_helper_stby_e(tcg_env, addr, val);
        }
    } else {
        if (tb_cflags(ctx->base.tb) & CF_PARALLEL) {
            gen_helper_stby_b_parallel(tcg_env, addr, val);
        } else {
            gen_helper_stby_b(tcg_env, addr, val);
        }
    }
    if (a->m) {
        tcg_gen_andi_reg(ofs, ofs, ~3);
        save_gpr(ctx, a->b, ofs);
    }

    return nullify_end(ctx);
}

static bool trans_stdby(DisasContext *ctx, arg_stby *a)
{
    TCGv_reg ofs, val;
    TCGv_tl addr;

    if (!ctx->is_pa20) {
        return false;
    }
    nullify_over(ctx);

    form_gva(ctx, &addr, &ofs, a->b, 0, 0, a->disp, a->sp, a->m,
             ctx->mmu_idx == MMU_PHYS_IDX);
    val = load_gpr(ctx, a->r);
    if (a->a) {
        if (tb_cflags(ctx->base.tb) & CF_PARALLEL) {
            gen_helper_stdby_e_parallel(tcg_env, addr, val);
        } else {
            gen_helper_stdby_e(tcg_env, addr, val);
        }
    } else {
        if (tb_cflags(ctx->base.tb) & CF_PARALLEL) {
            gen_helper_stdby_b_parallel(tcg_env, addr, val);
        } else {
            gen_helper_stdby_b(tcg_env, addr, val);
        }
    }
    if (a->m) {
        tcg_gen_andi_reg(ofs, ofs, ~7);
        save_gpr(ctx, a->b, ofs);
    }

    return nullify_end(ctx);
}

static bool trans_lda(DisasContext *ctx, arg_ldst *a)
{
    int hold_mmu_idx = ctx->mmu_idx;

    CHECK_MOST_PRIVILEGED(EXCP_PRIV_OPR);
    ctx->mmu_idx = MMU_PHYS_IDX;
    trans_ld(ctx, a);
    ctx->mmu_idx = hold_mmu_idx;
    return true;
}

static bool trans_sta(DisasContext *ctx, arg_ldst *a)
{
    int hold_mmu_idx = ctx->mmu_idx;

    CHECK_MOST_PRIVILEGED(EXCP_PRIV_OPR);
    ctx->mmu_idx = MMU_PHYS_IDX;
    trans_st(ctx, a);
    ctx->mmu_idx = hold_mmu_idx;
    return true;
}

static bool trans_ldil(DisasContext *ctx, arg_ldil *a)
{
    TCGv_reg tcg_rt = dest_gpr(ctx, a->t);

    tcg_gen_movi_reg(tcg_rt, a->i);
    save_gpr(ctx, a->t, tcg_rt);
    cond_free(&ctx->null_cond);
    return true;
}

static bool trans_addil(DisasContext *ctx, arg_addil *a)
{
    TCGv_reg tcg_rt = load_gpr(ctx, a->r);
    TCGv_reg tcg_r1 = dest_gpr(ctx, 1);

    tcg_gen_addi_reg(tcg_r1, tcg_rt, a->i);
    save_gpr(ctx, 1, tcg_r1);
    cond_free(&ctx->null_cond);
    return true;
}

static bool trans_ldo(DisasContext *ctx, arg_ldo *a)
{
    TCGv_reg tcg_rt = dest_gpr(ctx, a->t);

    /* Special case rb == 0, for the LDI pseudo-op.
       The COPY pseudo-op is handled for free within tcg_gen_addi_tl.  */
    if (a->b == 0) {
        tcg_gen_movi_reg(tcg_rt, a->i);
    } else {
        tcg_gen_addi_reg(tcg_rt, cpu_gr[a->b], a->i);
    }
    save_gpr(ctx, a->t, tcg_rt);
    cond_free(&ctx->null_cond);
    return true;
}

static bool do_cmpb(DisasContext *ctx, unsigned r, TCGv_reg in1,
                    unsigned c, unsigned f, bool d, unsigned n, int disp)
{
    TCGv_reg dest, in2, sv;
    DisasCond cond;

    in2 = load_gpr(ctx, r);
    dest = tcg_temp_new();

    tcg_gen_sub_reg(dest, in1, in2);

    sv = NULL;
    if (cond_need_sv(c)) {
        sv = do_sub_sv(ctx, dest, in1, in2);
    }

    cond = do_sub_cond(ctx, c * 2 + f, d, dest, in1, in2, sv);
    return do_cbranch(ctx, disp, n, &cond);
}

static bool trans_cmpb(DisasContext *ctx, arg_cmpb *a)
{
    if (!ctx->is_pa20 && a->d) {
        return false;
    }
    nullify_over(ctx);
    return do_cmpb(ctx, a->r2, load_gpr(ctx, a->r1),
                   a->c, a->f, a->d, a->n, a->disp);
}

static bool trans_cmpbi(DisasContext *ctx, arg_cmpbi *a)
{
    if (!ctx->is_pa20 && a->d) {
        return false;
    }
    nullify_over(ctx);
    return do_cmpb(ctx, a->r, tcg_constant_reg(a->i),
                   a->c, a->f, a->d, a->n, a->disp);
}

static bool do_addb(DisasContext *ctx, unsigned r, TCGv_reg in1,
                    unsigned c, unsigned f, unsigned n, int disp)
{
    TCGv_reg dest, in2, sv, cb_cond;
    DisasCond cond;
    bool d = false;

    /*
     * For hppa64, the ADDB conditions change with PSW.W,
     * dropping ZNV, SV, OD in favor of double-word EQ, LT, LE.
     */
    if (ctx->tb_flags & PSW_W) {
        d = c >= 5;
        if (d) {
            c &= 3;
        }
    }

    in2 = load_gpr(ctx, r);
    dest = tcg_temp_new();
    sv = NULL;
    cb_cond = NULL;

    if (cond_need_cb(c)) {
        TCGv_reg cb = tcg_temp_new();
        TCGv_reg cb_msb = tcg_temp_new();

        tcg_gen_movi_reg(cb_msb, 0);
        tcg_gen_add2_reg(dest, cb_msb, in1, cb_msb, in2, cb_msb);
        tcg_gen_xor_reg(cb, in1, in2);
        tcg_gen_xor_reg(cb, cb, dest);
        cb_cond = get_carry(ctx, d, cb, cb_msb);
    } else {
        tcg_gen_add_reg(dest, in1, in2);
    }
    if (cond_need_sv(c)) {
        sv = do_add_sv(ctx, dest, in1, in2);
    }

    cond = do_cond(ctx, c * 2 + f, d, dest, cb_cond, sv);
    save_gpr(ctx, r, dest);
    return do_cbranch(ctx, disp, n, &cond);
}

static bool trans_addb(DisasContext *ctx, arg_addb *a)
{
    nullify_over(ctx);
    return do_addb(ctx, a->r2, load_gpr(ctx, a->r1), a->c, a->f, a->n, a->disp);
}

static bool trans_addbi(DisasContext *ctx, arg_addbi *a)
{
    nullify_over(ctx);
    return do_addb(ctx, a->r, tcg_constant_reg(a->i), a->c, a->f, a->n, a->disp);
}

static bool trans_bb_sar(DisasContext *ctx, arg_bb_sar *a)
{
    TCGv_reg tmp, tcg_r;
    DisasCond cond;

    nullify_over(ctx);

    tmp = tcg_temp_new();
    tcg_r = load_gpr(ctx, a->r);
    if (cond_need_ext(ctx, a->d)) {
        /* Force shift into [32,63] */
        tcg_gen_ori_reg(tmp, cpu_sar, 32);
        tcg_gen_shl_reg(tmp, tcg_r, tmp);
    } else {
        tcg_gen_shl_reg(tmp, tcg_r, cpu_sar);
    }

    cond = cond_make_0_tmp(a->c ? TCG_COND_GE : TCG_COND_LT, tmp);
    return do_cbranch(ctx, a->disp, a->n, &cond);
}

static bool trans_bb_imm(DisasContext *ctx, arg_bb_imm *a)
{
    TCGv_reg tmp, tcg_r;
    DisasCond cond;
    int p;

    nullify_over(ctx);

    tmp = tcg_temp_new();
    tcg_r = load_gpr(ctx, a->r);
    p = a->p | (cond_need_ext(ctx, a->d) ? 32 : 0);
    tcg_gen_shli_reg(tmp, tcg_r, p);

    cond = cond_make_0(a->c ? TCG_COND_GE : TCG_COND_LT, tmp);
    return do_cbranch(ctx, a->disp, a->n, &cond);
}

static bool trans_movb(DisasContext *ctx, arg_movb *a)
{
    TCGv_reg dest;
    DisasCond cond;

    nullify_over(ctx);

    dest = dest_gpr(ctx, a->r2);
    if (a->r1 == 0) {
        tcg_gen_movi_reg(dest, 0);
    } else {
        tcg_gen_mov_reg(dest, cpu_gr[a->r1]);
    }

    /* All MOVB conditions are 32-bit. */
    cond = do_sed_cond(ctx, a->c, false, dest);
    return do_cbranch(ctx, a->disp, a->n, &cond);
}

static bool trans_movbi(DisasContext *ctx, arg_movbi *a)
{
    TCGv_reg dest;
    DisasCond cond;

    nullify_over(ctx);

    dest = dest_gpr(ctx, a->r);
    tcg_gen_movi_reg(dest, a->i);

    /* All MOVBI conditions are 32-bit. */
    cond = do_sed_cond(ctx, a->c, false, dest);
    return do_cbranch(ctx, a->disp, a->n, &cond);
}

static bool trans_shrp_sar(DisasContext *ctx, arg_shrp_sar *a)
{
    TCGv_reg dest, src2;

    if (!ctx->is_pa20 && a->d) {
        return false;
    }
    if (a->c) {
        nullify_over(ctx);
    }

    dest = dest_gpr(ctx, a->t);
    src2 = load_gpr(ctx, a->r2);
    if (a->r1 == 0) {
        if (a->d) {
            tcg_gen_shr_reg(dest, src2, cpu_sar);
        } else {
            TCGv_reg tmp = tcg_temp_new();

            tcg_gen_ext32u_reg(dest, src2);
            tcg_gen_andi_reg(tmp, cpu_sar, 31);
            tcg_gen_shr_reg(dest, dest, tmp);
        }
    } else if (a->r1 == a->r2) {
        if (a->d) {
            tcg_gen_rotr_reg(dest, src2, cpu_sar);
        } else {
            TCGv_i32 t32 = tcg_temp_new_i32();
            TCGv_i32 s32 = tcg_temp_new_i32();

            tcg_gen_trunc_reg_i32(t32, src2);
            tcg_gen_trunc_reg_i32(s32, cpu_sar);
            tcg_gen_andi_i32(s32, s32, 31);
            tcg_gen_rotr_i32(t32, t32, s32);
            tcg_gen_extu_i32_reg(dest, t32);
        }
    } else {
        TCGv_reg src1 = load_gpr(ctx, a->r1);

        if (a->d) {
            TCGv_reg t = tcg_temp_new();
            TCGv_reg n = tcg_temp_new();

            tcg_gen_xori_reg(n, cpu_sar, 63);
            tcg_gen_shl_reg(t, src2, n);
            tcg_gen_shli_reg(t, t, 1);
            tcg_gen_shr_reg(dest, src1, cpu_sar);
            tcg_gen_or_reg(dest, dest, t);
        } else {
            TCGv_i64 t = tcg_temp_new_i64();
            TCGv_i64 s = tcg_temp_new_i64();

            tcg_gen_concat_reg_i64(t, src2, src1);
            tcg_gen_extu_reg_i64(s, cpu_sar);
            tcg_gen_andi_i64(s, s, 31);
            tcg_gen_shr_i64(t, t, s);
            tcg_gen_trunc_i64_reg(dest, t);
        }
    }
    save_gpr(ctx, a->t, dest);

    /* Install the new nullification.  */
    cond_free(&ctx->null_cond);
    if (a->c) {
        ctx->null_cond = do_sed_cond(ctx, a->c, false, dest);
    }
    return nullify_end(ctx);
}

static bool trans_shrp_imm(DisasContext *ctx, arg_shrp_imm *a)
{
    unsigned width, sa;
    TCGv_reg dest, t2;

    if (!ctx->is_pa20 && a->d) {
        return false;
    }
    if (a->c) {
        nullify_over(ctx);
    }

    width = a->d ? 64 : 32;
    sa = width - 1 - a->cpos;

    dest = dest_gpr(ctx, a->t);
    t2 = load_gpr(ctx, a->r2);
    if (a->r1 == 0) {
        tcg_gen_extract_reg(dest, t2, sa, width - sa);
    } else if (width == TARGET_LONG_BITS) {
        tcg_gen_extract2_reg(dest, t2, cpu_gr[a->r1], sa);
    } else {
        assert(!a->d);
        if (a->r1 == a->r2) {
            TCGv_i32 t32 = tcg_temp_new_i32();
            tcg_gen_trunc_reg_i32(t32, t2);
            tcg_gen_rotri_i32(t32, t32, sa);
            tcg_gen_extu_i32_reg(dest, t32);
        } else {
            TCGv_i64 t64 = tcg_temp_new_i64();
            tcg_gen_concat_reg_i64(t64, t2, cpu_gr[a->r1]);
            tcg_gen_shri_i64(t64, t64, sa);
            tcg_gen_trunc_i64_reg(dest, t64);
        }
    }
    save_gpr(ctx, a->t, dest);

    /* Install the new nullification.  */
    cond_free(&ctx->null_cond);
    if (a->c) {
        ctx->null_cond = do_sed_cond(ctx, a->c, false, dest);
    }
    return nullify_end(ctx);
}

static bool trans_extr_sar(DisasContext *ctx, arg_extr_sar *a)
{
    unsigned widthm1 = a->d ? 63 : 31;
    TCGv_reg dest, src, tmp;

    if (!ctx->is_pa20 && a->d) {
        return false;
    }
    if (a->c) {
        nullify_over(ctx);
    }

    dest = dest_gpr(ctx, a->t);
    src = load_gpr(ctx, a->r);
    tmp = tcg_temp_new();

    /* Recall that SAR is using big-endian bit numbering.  */
    tcg_gen_andi_reg(tmp, cpu_sar, widthm1);
    tcg_gen_xori_reg(tmp, tmp, widthm1);

    if (a->se) {
        if (!a->d) {
            tcg_gen_ext32s_reg(dest, src);
            src = dest;
        }
        tcg_gen_sar_reg(dest, src, tmp);
        tcg_gen_sextract_reg(dest, dest, 0, a->len);
    } else {
        if (!a->d) {
            tcg_gen_ext32u_reg(dest, src);
            src = dest;
        }
        tcg_gen_shr_reg(dest, src, tmp);
        tcg_gen_extract_reg(dest, dest, 0, a->len);
    }
    save_gpr(ctx, a->t, dest);

    /* Install the new nullification.  */
    cond_free(&ctx->null_cond);
    if (a->c) {
        ctx->null_cond = do_sed_cond(ctx, a->c, a->d, dest);
    }
    return nullify_end(ctx);
}

static bool trans_extr_imm(DisasContext *ctx, arg_extr_imm *a)
{
    unsigned len, cpos, width;
    TCGv_reg dest, src;

    if (!ctx->is_pa20 && a->d) {
        return false;
    }
    if (a->c) {
        nullify_over(ctx);
    }

    len = a->len;
    width = a->d ? 64 : 32;
    cpos = width - 1 - a->pos;
    if (cpos + len > width) {
        len = width - cpos;
    }

    dest = dest_gpr(ctx, a->t);
    src = load_gpr(ctx, a->r);
    if (a->se) {
        tcg_gen_sextract_reg(dest, src, cpos, len);
    } else {
        tcg_gen_extract_reg(dest, src, cpos, len);
    }
    save_gpr(ctx, a->t, dest);

    /* Install the new nullification.  */
    cond_free(&ctx->null_cond);
    if (a->c) {
        ctx->null_cond = do_sed_cond(ctx, a->c, a->d, dest);
    }
    return nullify_end(ctx);
}

static bool trans_depi_imm(DisasContext *ctx, arg_depi_imm *a)
{
    unsigned len, width;
    uint64_t mask0, mask1;
    TCGv_reg dest;

    if (!ctx->is_pa20 && a->d) {
        return false;
    }
    if (a->c) {
        nullify_over(ctx);
    }

    len = a->len;
    width = a->d ? 64 : 32;
    if (a->cpos + len > width) {
        len = width - a->cpos;
    }

    dest = dest_gpr(ctx, a->t);
    mask0 = deposit64(0, a->cpos, len, a->i);
    mask1 = deposit64(-1, a->cpos, len, a->i);

    if (a->nz) {
        TCGv_reg src = load_gpr(ctx, a->t);
        tcg_gen_andi_reg(dest, src, mask1);
        tcg_gen_ori_reg(dest, dest, mask0);
    } else {
        tcg_gen_movi_reg(dest, mask0);
    }
    save_gpr(ctx, a->t, dest);

    /* Install the new nullification.  */
    cond_free(&ctx->null_cond);
    if (a->c) {
        ctx->null_cond = do_sed_cond(ctx, a->c, a->d, dest);
    }
    return nullify_end(ctx);
}

static bool trans_dep_imm(DisasContext *ctx, arg_dep_imm *a)
{
    unsigned rs = a->nz ? a->t : 0;
    unsigned len, width;
    TCGv_reg dest, val;

    if (!ctx->is_pa20 && a->d) {
        return false;
    }
    if (a->c) {
        nullify_over(ctx);
    }

    len = a->len;
    width = a->d ? 64 : 32;
    if (a->cpos + len > width) {
        len = width - a->cpos;
    }

    dest = dest_gpr(ctx, a->t);
    val = load_gpr(ctx, a->r);
    if (rs == 0) {
        tcg_gen_deposit_z_reg(dest, val, a->cpos, len);
    } else {
        tcg_gen_deposit_reg(dest, cpu_gr[rs], val, a->cpos, len);
    }
    save_gpr(ctx, a->t, dest);

    /* Install the new nullification.  */
    cond_free(&ctx->null_cond);
    if (a->c) {
        ctx->null_cond = do_sed_cond(ctx, a->c, a->d, dest);
    }
    return nullify_end(ctx);
}

static bool do_dep_sar(DisasContext *ctx, unsigned rt, unsigned c,
                       bool d, bool nz, unsigned len, TCGv_reg val)
{
    unsigned rs = nz ? rt : 0;
    unsigned widthm1 = d ? 63 : 31;
    TCGv_reg mask, tmp, shift, dest;
    uint64_t msb = 1ULL << (len - 1);

    dest = dest_gpr(ctx, rt);
    shift = tcg_temp_new();
    tmp = tcg_temp_new();

    /* Convert big-endian bit numbering in SAR to left-shift.  */
    tcg_gen_andi_reg(shift, cpu_sar, widthm1);
    tcg_gen_xori_reg(shift, shift, widthm1);

    mask = tcg_temp_new();
    tcg_gen_movi_reg(mask, msb + (msb - 1));
    tcg_gen_and_reg(tmp, val, mask);
    if (rs) {
        tcg_gen_shl_reg(mask, mask, shift);
        tcg_gen_shl_reg(tmp, tmp, shift);
        tcg_gen_andc_reg(dest, cpu_gr[rs], mask);
        tcg_gen_or_reg(dest, dest, tmp);
    } else {
        tcg_gen_shl_reg(dest, tmp, shift);
    }
    save_gpr(ctx, rt, dest);

    /* Install the new nullification.  */
    cond_free(&ctx->null_cond);
    if (c) {
        ctx->null_cond = do_sed_cond(ctx, c, d, dest);
    }
    return nullify_end(ctx);
}

static bool trans_dep_sar(DisasContext *ctx, arg_dep_sar *a)
{
    if (!ctx->is_pa20 && a->d) {
        return false;
    }
    if (a->c) {
        nullify_over(ctx);
    }
    return do_dep_sar(ctx, a->t, a->c, a->d, a->nz, a->len,
                      load_gpr(ctx, a->r));
}

static bool trans_depi_sar(DisasContext *ctx, arg_depi_sar *a)
{
    if (!ctx->is_pa20 && a->d) {
        return false;
    }
    if (a->c) {
        nullify_over(ctx);
    }
    return do_dep_sar(ctx, a->t, a->c, a->d, a->nz, a->len,
                      tcg_constant_reg(a->i));
}

static bool trans_be(DisasContext *ctx, arg_be *a)
{
    TCGv_reg tmp;

#ifdef CONFIG_USER_ONLY
    /* ??? It seems like there should be a good way of using
       "be disp(sr2, r0)", the canonical gateway entry mechanism
       to our advantage.  But that appears to be inconvenient to
       manage along side branch delay slots.  Therefore we handle
       entry into the gateway page via absolute address.  */
    /* Since we don't implement spaces, just branch.  Do notice the special
       case of "be disp(*,r0)" using a direct branch to disp, so that we can
       goto_tb to the TB containing the syscall.  */
    if (a->b == 0) {
        return do_dbranch(ctx, a->disp, a->l, a->n);
    }
#else
    nullify_over(ctx);
#endif

    tmp = tcg_temp_new();
    tcg_gen_addi_reg(tmp, load_gpr(ctx, a->b), a->disp);
    tmp = do_ibranch_priv(ctx, tmp);

#ifdef CONFIG_USER_ONLY
    return do_ibranch(ctx, tmp, a->l, a->n);
#else
    TCGv_i64 new_spc = tcg_temp_new_i64();

    load_spr(ctx, new_spc, a->sp);
    if (a->l) {
        copy_iaoq_entry(ctx, cpu_gr[31], ctx->iaoq_n, ctx->iaoq_n_var);
        tcg_gen_mov_i64(cpu_sr[0], cpu_iasq_f);
    }
    if (a->n && use_nullify_skip(ctx)) {
        copy_iaoq_entry(ctx, cpu_iaoq_f, -1, tmp);
        tcg_gen_addi_reg(tmp, tmp, 4);
        copy_iaoq_entry(ctx, cpu_iaoq_b, -1, tmp);
        tcg_gen_mov_i64(cpu_iasq_f, new_spc);
        tcg_gen_mov_i64(cpu_iasq_b, cpu_iasq_f);
    } else {
        copy_iaoq_entry(ctx, cpu_iaoq_f, ctx->iaoq_b, cpu_iaoq_b);
        if (ctx->iaoq_b == -1) {
            tcg_gen_mov_i64(cpu_iasq_f, cpu_iasq_b);
        }
        copy_iaoq_entry(ctx, cpu_iaoq_b, -1, tmp);
        tcg_gen_mov_i64(cpu_iasq_b, new_spc);
        nullify_set(ctx, a->n);
    }
    tcg_gen_lookup_and_goto_ptr();
    ctx->base.is_jmp = DISAS_NORETURN;
    return nullify_end(ctx);
#endif
}

static bool trans_bl(DisasContext *ctx, arg_bl *a)
{
    return do_dbranch(ctx, iaoq_dest(ctx, a->disp), a->l, a->n);
}

static bool trans_b_gate(DisasContext *ctx, arg_b_gate *a)
{
    uint64_t dest = iaoq_dest(ctx, a->disp);

    nullify_over(ctx);

    /* Make sure the caller hasn't done something weird with the queue.
     * ??? This is not quite the same as the PSW[B] bit, which would be
     * expensive to track.  Real hardware will trap for
     *    b  gateway
     *    b  gateway+4  (in delay slot of first branch)
     * However, checking for a non-sequential instruction queue *will*
     * diagnose the security hole
     *    b  gateway
     *    b  evil
     * in which instructions at evil would run with increased privs.
     */
    if (ctx->iaoq_b == -1 || ctx->iaoq_b != ctx->iaoq_f + 4) {
        return gen_illegal(ctx);
    }

#ifndef CONFIG_USER_ONLY
    if (ctx->tb_flags & PSW_C) {
        CPUHPPAState *env = cpu_env(ctx->cs);
        int type = hppa_artype_for_page(env, ctx->base.pc_next);
        /* If we could not find a TLB entry, then we need to generate an
           ITLB miss exception so the kernel will provide it.
           The resulting TLB fill operation will invalidate this TB and
           we will re-translate, at which point we *will* be able to find
           the TLB entry and determine if this is in fact a gateway page.  */
        if (type < 0) {
            gen_excp(ctx, EXCP_ITLB_MISS);
            return true;
        }
        /* No change for non-gateway pages or for priv decrease.  */
        if (type >= 4 && type - 4 < ctx->privilege) {
            dest = deposit32(dest, 0, 2, type - 4);
        }
    } else {
        dest &= -4;  /* priv = 0 */
    }
#endif

    if (a->l) {
        TCGv_reg tmp = dest_gpr(ctx, a->l);
        if (ctx->privilege < 3) {
            tcg_gen_andi_reg(tmp, tmp, -4);
        }
        tcg_gen_ori_reg(tmp, tmp, ctx->privilege);
        save_gpr(ctx, a->l, tmp);
    }

    return do_dbranch(ctx, dest, 0, a->n);
}

static bool trans_blr(DisasContext *ctx, arg_blr *a)
{
    if (a->x) {
        TCGv_reg tmp = tcg_temp_new();
        tcg_gen_shli_reg(tmp, load_gpr(ctx, a->x), 3);
        tcg_gen_addi_reg(tmp, tmp, ctx->iaoq_f + 8);
        /* The computation here never changes privilege level.  */
        return do_ibranch(ctx, tmp, a->l, a->n);
    } else {
        /* BLR R0,RX is a good way to load PC+8 into RX.  */
        return do_dbranch(ctx, ctx->iaoq_f + 8, a->l, a->n);
    }
}

static bool trans_bv(DisasContext *ctx, arg_bv *a)
{
    TCGv_reg dest;

    if (a->x == 0) {
        dest = load_gpr(ctx, a->b);
    } else {
        dest = tcg_temp_new();
        tcg_gen_shli_reg(dest, load_gpr(ctx, a->x), 3);
        tcg_gen_add_reg(dest, dest, load_gpr(ctx, a->b));
    }
    dest = do_ibranch_priv(ctx, dest);
    return do_ibranch(ctx, dest, 0, a->n);
}

static bool trans_bve(DisasContext *ctx, arg_bve *a)
{
    TCGv_reg dest;

#ifdef CONFIG_USER_ONLY
    dest = do_ibranch_priv(ctx, load_gpr(ctx, a->b));
    return do_ibranch(ctx, dest, a->l, a->n);
#else
    nullify_over(ctx);
    dest = do_ibranch_priv(ctx, load_gpr(ctx, a->b));

    copy_iaoq_entry(ctx, cpu_iaoq_f, ctx->iaoq_b, cpu_iaoq_b);
    if (ctx->iaoq_b == -1) {
        tcg_gen_mov_i64(cpu_iasq_f, cpu_iasq_b);
    }
    copy_iaoq_entry(ctx, cpu_iaoq_b, -1, dest);
    tcg_gen_mov_i64(cpu_iasq_b, space_select(ctx, 0, dest));
    if (a->l) {
        copy_iaoq_entry(ctx, cpu_gr[a->l], ctx->iaoq_n, ctx->iaoq_n_var);
    }
    nullify_set(ctx, a->n);
    tcg_gen_lookup_and_goto_ptr();
    ctx->base.is_jmp = DISAS_NORETURN;
    return nullify_end(ctx);
#endif
}

static bool trans_nopbts(DisasContext *ctx, arg_nopbts *a)
{
    /* All branch target stack instructions implement as nop. */
    return ctx->is_pa20;
}

/*
 * Float class 0
 */

static void gen_fcpy_f(TCGv_i32 dst, TCGv_env unused, TCGv_i32 src)
{
    tcg_gen_mov_i32(dst, src);
}

static bool trans_fid_f(DisasContext *ctx, arg_fid_f *a)
{
    uint64_t ret;

    if (ctx->is_pa20) {
        ret = 0x13080000000000ULL; /* PA8700 (PCX-W2) */
    } else {
        ret = 0x0f080000000000ULL; /* PA7300LC (PCX-L2) */
    }

    nullify_over(ctx);
    save_frd(0, tcg_constant_i64(ret));
    return nullify_end(ctx);
}

static bool trans_fcpy_f(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_wew(ctx, a->t, a->r, gen_fcpy_f);
}

static void gen_fcpy_d(TCGv_i64 dst, TCGv_env unused, TCGv_i64 src)
{
    tcg_gen_mov_i64(dst, src);
}

static bool trans_fcpy_d(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_ded(ctx, a->t, a->r, gen_fcpy_d);
}

static void gen_fabs_f(TCGv_i32 dst, TCGv_env unused, TCGv_i32 src)
{
    tcg_gen_andi_i32(dst, src, INT32_MAX);
}

static bool trans_fabs_f(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_wew(ctx, a->t, a->r, gen_fabs_f);
}

static void gen_fabs_d(TCGv_i64 dst, TCGv_env unused, TCGv_i64 src)
{
    tcg_gen_andi_i64(dst, src, INT64_MAX);
}

static bool trans_fabs_d(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_ded(ctx, a->t, a->r, gen_fabs_d);
}

static bool trans_fsqrt_f(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_wew(ctx, a->t, a->r, gen_helper_fsqrt_s);
}

static bool trans_fsqrt_d(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_ded(ctx, a->t, a->r, gen_helper_fsqrt_d);
}

static bool trans_frnd_f(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_wew(ctx, a->t, a->r, gen_helper_frnd_s);
}

static bool trans_frnd_d(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_ded(ctx, a->t, a->r, gen_helper_frnd_d);
}

static void gen_fneg_f(TCGv_i32 dst, TCGv_env unused, TCGv_i32 src)
{
    tcg_gen_xori_i32(dst, src, INT32_MIN);
}

static bool trans_fneg_f(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_wew(ctx, a->t, a->r, gen_fneg_f);
}

static void gen_fneg_d(TCGv_i64 dst, TCGv_env unused, TCGv_i64 src)
{
    tcg_gen_xori_i64(dst, src, INT64_MIN);
}

static bool trans_fneg_d(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_ded(ctx, a->t, a->r, gen_fneg_d);
}

static void gen_fnegabs_f(TCGv_i32 dst, TCGv_env unused, TCGv_i32 src)
{
    tcg_gen_ori_i32(dst, src, INT32_MIN);
}

static bool trans_fnegabs_f(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_wew(ctx, a->t, a->r, gen_fnegabs_f);
}

static void gen_fnegabs_d(TCGv_i64 dst, TCGv_env unused, TCGv_i64 src)
{
    tcg_gen_ori_i64(dst, src, INT64_MIN);
}

static bool trans_fnegabs_d(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_ded(ctx, a->t, a->r, gen_fnegabs_d);
}

/*
 * Float class 1
 */

static bool trans_fcnv_d_f(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_wed(ctx, a->t, a->r, gen_helper_fcnv_d_s);
}

static bool trans_fcnv_f_d(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_dew(ctx, a->t, a->r, gen_helper_fcnv_s_d);
}

static bool trans_fcnv_w_f(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_wew(ctx, a->t, a->r, gen_helper_fcnv_w_s);
}

static bool trans_fcnv_q_f(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_wed(ctx, a->t, a->r, gen_helper_fcnv_dw_s);
}

static bool trans_fcnv_w_d(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_dew(ctx, a->t, a->r, gen_helper_fcnv_w_d);
}

static bool trans_fcnv_q_d(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_ded(ctx, a->t, a->r, gen_helper_fcnv_dw_d);
}

static bool trans_fcnv_f_w(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_wew(ctx, a->t, a->r, gen_helper_fcnv_s_w);
}

static bool trans_fcnv_d_w(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_wed(ctx, a->t, a->r, gen_helper_fcnv_d_w);
}

static bool trans_fcnv_f_q(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_dew(ctx, a->t, a->r, gen_helper_fcnv_s_dw);
}

static bool trans_fcnv_d_q(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_ded(ctx, a->t, a->r, gen_helper_fcnv_d_dw);
}

static bool trans_fcnv_t_f_w(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_wew(ctx, a->t, a->r, gen_helper_fcnv_t_s_w);
}

static bool trans_fcnv_t_d_w(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_wed(ctx, a->t, a->r, gen_helper_fcnv_t_d_w);
}

static bool trans_fcnv_t_f_q(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_dew(ctx, a->t, a->r, gen_helper_fcnv_t_s_dw);
}

static bool trans_fcnv_t_d_q(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_ded(ctx, a->t, a->r, gen_helper_fcnv_t_d_dw);
}

static bool trans_fcnv_uw_f(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_wew(ctx, a->t, a->r, gen_helper_fcnv_uw_s);
}

static bool trans_fcnv_uq_f(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_wed(ctx, a->t, a->r, gen_helper_fcnv_udw_s);
}

static bool trans_fcnv_uw_d(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_dew(ctx, a->t, a->r, gen_helper_fcnv_uw_d);
}

static bool trans_fcnv_uq_d(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_ded(ctx, a->t, a->r, gen_helper_fcnv_udw_d);
}

static bool trans_fcnv_f_uw(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_wew(ctx, a->t, a->r, gen_helper_fcnv_s_uw);
}

static bool trans_fcnv_d_uw(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_wed(ctx, a->t, a->r, gen_helper_fcnv_d_uw);
}

static bool trans_fcnv_f_uq(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_dew(ctx, a->t, a->r, gen_helper_fcnv_s_udw);
}

static bool trans_fcnv_d_uq(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_ded(ctx, a->t, a->r, gen_helper_fcnv_d_udw);
}

static bool trans_fcnv_t_f_uw(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_wew(ctx, a->t, a->r, gen_helper_fcnv_t_s_uw);
}

static bool trans_fcnv_t_d_uw(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_wed(ctx, a->t, a->r, gen_helper_fcnv_t_d_uw);
}

static bool trans_fcnv_t_f_uq(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_dew(ctx, a->t, a->r, gen_helper_fcnv_t_s_udw);
}

static bool trans_fcnv_t_d_uq(DisasContext *ctx, arg_fclass01 *a)
{
    return do_fop_ded(ctx, a->t, a->r, gen_helper_fcnv_t_d_udw);
}

/*
 * Float class 2
 */

static bool trans_fcmp_f(DisasContext *ctx, arg_fclass2 *a)
{
    TCGv_i32 ta, tb, tc, ty;

    nullify_over(ctx);

    ta = load_frw0_i32(a->r1);
    tb = load_frw0_i32(a->r2);
    ty = tcg_constant_i32(a->y);
    tc = tcg_constant_i32(a->c);

    gen_helper_fcmp_s(tcg_env, ta, tb, ty, tc);

    return nullify_end(ctx);
}

static bool trans_fcmp_d(DisasContext *ctx, arg_fclass2 *a)
{
    TCGv_i64 ta, tb;
    TCGv_i32 tc, ty;

    nullify_over(ctx);

    ta = load_frd0(a->r1);
    tb = load_frd0(a->r2);
    ty = tcg_constant_i32(a->y);
    tc = tcg_constant_i32(a->c);

    gen_helper_fcmp_d(tcg_env, ta, tb, ty, tc);

    return nullify_end(ctx);
}

static bool trans_ftest(DisasContext *ctx, arg_ftest *a)
{
    TCGv_reg t;

    nullify_over(ctx);

    t = tcg_temp_new();
    tcg_gen_ld32u_reg(t, tcg_env, offsetof(CPUHPPAState, fr0_shadow));

    if (a->y == 1) {
        int mask;
        bool inv = false;

        switch (a->c) {
        case 0: /* simple */
            tcg_gen_andi_reg(t, t, 0x4000000);
            ctx->null_cond = cond_make_0(TCG_COND_NE, t);
            goto done;
        case 2: /* rej */
            inv = true;
            /* fallthru */
        case 1: /* acc */
            mask = 0x43ff800;
            break;
        case 6: /* rej8 */
            inv = true;
            /* fallthru */
        case 5: /* acc8 */
            mask = 0x43f8000;
            break;
        case 9: /* acc6 */
            mask = 0x43e0000;
            break;
        case 13: /* acc4 */
            mask = 0x4380000;
            break;
        case 17: /* acc2 */
            mask = 0x4200000;
            break;
        default:
            gen_illegal(ctx);
            return true;
        }
        if (inv) {
            TCGv_reg c = tcg_constant_reg(mask);
            tcg_gen_or_reg(t, t, c);
            ctx->null_cond = cond_make(TCG_COND_EQ, t, c);
        } else {
            tcg_gen_andi_reg(t, t, mask);
            ctx->null_cond = cond_make_0(TCG_COND_EQ, t);
        }
    } else {
        unsigned cbit = (a->y ^ 1) - 1;

        tcg_gen_extract_reg(t, t, 21 - cbit, 1);
        ctx->null_cond = cond_make_0(TCG_COND_NE, t);
    }

 done:
    return nullify_end(ctx);
}

/*
 * Float class 2
 */

static bool trans_fadd_f(DisasContext *ctx, arg_fclass3 *a)
{
    return do_fop_weww(ctx, a->t, a->r1, a->r2, gen_helper_fadd_s);
}

static bool trans_fadd_d(DisasContext *ctx, arg_fclass3 *a)
{
    return do_fop_dedd(ctx, a->t, a->r1, a->r2, gen_helper_fadd_d);
}

static bool trans_fsub_f(DisasContext *ctx, arg_fclass3 *a)
{
    return do_fop_weww(ctx, a->t, a->r1, a->r2, gen_helper_fsub_s);
}

static bool trans_fsub_d(DisasContext *ctx, arg_fclass3 *a)
{
    return do_fop_dedd(ctx, a->t, a->r1, a->r2, gen_helper_fsub_d);
}

static bool trans_fmpy_f(DisasContext *ctx, arg_fclass3 *a)
{
    return do_fop_weww(ctx, a->t, a->r1, a->r2, gen_helper_fmpy_s);
}

static bool trans_fmpy_d(DisasContext *ctx, arg_fclass3 *a)
{
    return do_fop_dedd(ctx, a->t, a->r1, a->r2, gen_helper_fmpy_d);
}

static bool trans_fdiv_f(DisasContext *ctx, arg_fclass3 *a)
{
    return do_fop_weww(ctx, a->t, a->r1, a->r2, gen_helper_fdiv_s);
}

static bool trans_fdiv_d(DisasContext *ctx, arg_fclass3 *a)
{
    return do_fop_dedd(ctx, a->t, a->r1, a->r2, gen_helper_fdiv_d);
}

static bool trans_xmpyu(DisasContext *ctx, arg_xmpyu *a)
{
    TCGv_i64 x, y;

    nullify_over(ctx);

    x = load_frw0_i64(a->r1);
    y = load_frw0_i64(a->r2);
    tcg_gen_mul_i64(x, x, y);
    save_frd(a->t, x);

    return nullify_end(ctx);
}

/* Convert the fmpyadd single-precision register encodings to standard.  */
static inline int fmpyadd_s_reg(unsigned r)
{
    return (r & 16) * 2 + 16 + (r & 15);
}

static bool do_fmpyadd_s(DisasContext *ctx, arg_mpyadd *a, bool is_sub)
{
    int tm = fmpyadd_s_reg(a->tm);
    int ra = fmpyadd_s_reg(a->ra);
    int ta = fmpyadd_s_reg(a->ta);
    int rm2 = fmpyadd_s_reg(a->rm2);
    int rm1 = fmpyadd_s_reg(a->rm1);

    nullify_over(ctx);

    do_fop_weww(ctx, tm, rm1, rm2, gen_helper_fmpy_s);
    do_fop_weww(ctx, ta, ta, ra,
                is_sub ? gen_helper_fsub_s : gen_helper_fadd_s);

    return nullify_end(ctx);
}

static bool trans_fmpyadd_f(DisasContext *ctx, arg_mpyadd *a)
{
    return do_fmpyadd_s(ctx, a, false);
}

static bool trans_fmpysub_f(DisasContext *ctx, arg_mpyadd *a)
{
    return do_fmpyadd_s(ctx, a, true);
}

static bool do_fmpyadd_d(DisasContext *ctx, arg_mpyadd *a, bool is_sub)
{
    nullify_over(ctx);

    do_fop_dedd(ctx, a->tm, a->rm1, a->rm2, gen_helper_fmpy_d);
    do_fop_dedd(ctx, a->ta, a->ta, a->ra,
                is_sub ? gen_helper_fsub_d : gen_helper_fadd_d);

    return nullify_end(ctx);
}

static bool trans_fmpyadd_d(DisasContext *ctx, arg_mpyadd *a)
{
    return do_fmpyadd_d(ctx, a, false);
}

static bool trans_fmpysub_d(DisasContext *ctx, arg_mpyadd *a)
{
    return do_fmpyadd_d(ctx, a, true);
}

static bool trans_fmpyfadd_f(DisasContext *ctx, arg_fmpyfadd_f *a)
{
    TCGv_i32 x, y, z;

    nullify_over(ctx);
    x = load_frw0_i32(a->rm1);
    y = load_frw0_i32(a->rm2);
    z = load_frw0_i32(a->ra3);

    if (a->neg) {
        gen_helper_fmpynfadd_s(x, tcg_env, x, y, z);
    } else {
        gen_helper_fmpyfadd_s(x, tcg_env, x, y, z);
    }

    save_frw_i32(a->t, x);
    return nullify_end(ctx);
}

static bool trans_fmpyfadd_d(DisasContext *ctx, arg_fmpyfadd_d *a)
{
    TCGv_i64 x, y, z;

    nullify_over(ctx);
    x = load_frd0(a->rm1);
    y = load_frd0(a->rm2);
    z = load_frd0(a->ra3);

    if (a->neg) {
        gen_helper_fmpynfadd_d(x, tcg_env, x, y, z);
    } else {
        gen_helper_fmpyfadd_d(x, tcg_env, x, y, z);
    }

    save_frd(a->t, x);
    return nullify_end(ctx);
}

static bool trans_diag(DisasContext *ctx, arg_diag *a)
{
    CHECK_MOST_PRIVILEGED(EXCP_PRIV_OPR);
#ifndef CONFIG_USER_ONLY
    if (a->i == 0x100) {
        /* emulate PDC BTLB, called by SeaBIOS-hppa */
        nullify_over(ctx);
        gen_helper_diag_btlb(tcg_env);
        return nullify_end(ctx);
    }
#endif
    qemu_log_mask(LOG_UNIMP, "DIAG opcode 0x%04x ignored\n", a->i);
    return true;
}

static void hppa_tr_init_disas_context(DisasContextBase *dcbase, CPUState *cs)
{
    DisasContext *ctx = container_of(dcbase, DisasContext, base);
    int bound;

    ctx->cs = cs;
    ctx->tb_flags = ctx->base.tb->flags;
    ctx->is_pa20 = hppa_is_pa20(cpu_env(cs));

#ifdef CONFIG_USER_ONLY
    ctx->privilege = MMU_IDX_TO_PRIV(MMU_USER_IDX);
    ctx->mmu_idx = MMU_USER_IDX;
    ctx->iaoq_f = ctx->base.pc_first | ctx->privilege;
    ctx->iaoq_b = ctx->base.tb->cs_base | ctx->privilege;
    ctx->unalign = (ctx->tb_flags & TB_FLAG_UNALIGN ? MO_UNALN : MO_ALIGN);
#else
    ctx->privilege = (ctx->tb_flags >> TB_FLAG_PRIV_SHIFT) & 3;
    ctx->mmu_idx = (ctx->tb_flags & PSW_D
                    ? PRIV_P_TO_MMU_IDX(ctx->privilege, ctx->tb_flags & PSW_P)
                    : MMU_PHYS_IDX);

    /* Recover the IAOQ values from the GVA + PRIV.  */
    uint64_t cs_base = ctx->base.tb->cs_base;
    uint64_t iasq_f = cs_base & ~0xffffffffull;
    int32_t diff = cs_base;

    ctx->iaoq_f = (ctx->base.pc_first & ~iasq_f) + ctx->privilege;
    ctx->iaoq_b = (diff ? ctx->iaoq_f + diff : -1);
#endif
    ctx->iaoq_n = -1;
    ctx->iaoq_n_var = NULL;

    /* Bound the number of instructions by those left on the page.  */
    bound = -(ctx->base.pc_first | TARGET_PAGE_MASK) / 4;
    ctx->base.max_insns = MIN(ctx->base.max_insns, bound);
}

static void hppa_tr_tb_start(DisasContextBase *dcbase, CPUState *cs)
{
    DisasContext *ctx = container_of(dcbase, DisasContext, base);

    /* Seed the nullification status from PSW[N], as saved in TB->FLAGS.  */
    ctx->null_cond = cond_make_f();
    ctx->psw_n_nonzero = false;
    if (ctx->tb_flags & PSW_N) {
        ctx->null_cond.c = TCG_COND_ALWAYS;
        ctx->psw_n_nonzero = true;
    }
    ctx->null_lab = NULL;
}

static void hppa_tr_insn_start(DisasContextBase *dcbase, CPUState *cs)
{
    DisasContext *ctx = container_of(dcbase, DisasContext, base);

    tcg_gen_insn_start(ctx->iaoq_f, ctx->iaoq_b);
}

static void hppa_tr_translate_insn(DisasContextBase *dcbase, CPUState *cs)
{
    DisasContext *ctx = container_of(dcbase, DisasContext, base);
    CPUHPPAState *env = cpu_env(cs);
    DisasJumpType ret;

    /* Execute one insn.  */
#ifdef CONFIG_USER_ONLY
    if (ctx->base.pc_next < TARGET_PAGE_SIZE) {
        do_page_zero(ctx);
        ret = ctx->base.is_jmp;
        assert(ret != DISAS_NEXT);
    } else
#endif
    {
        /* Always fetch the insn, even if nullified, so that we check
           the page permissions for execute.  */
        uint32_t insn = translator_ldl(env, &ctx->base, ctx->base.pc_next);

        /* Set up the IA queue for the next insn.
           This will be overwritten by a branch.  */
        if (ctx->iaoq_b == -1) {
            ctx->iaoq_n = -1;
            ctx->iaoq_n_var = tcg_temp_new();
            tcg_gen_addi_reg(ctx->iaoq_n_var, cpu_iaoq_b, 4);
        } else {
            ctx->iaoq_n = ctx->iaoq_b + 4;
            ctx->iaoq_n_var = NULL;
        }

        if (unlikely(ctx->null_cond.c == TCG_COND_ALWAYS)) {
            ctx->null_cond.c = TCG_COND_NEVER;
            ret = DISAS_NEXT;
        } else {
            ctx->insn = insn;
            if (!decode(ctx, insn)) {
                gen_illegal(ctx);
            }
            ret = ctx->base.is_jmp;
            assert(ctx->null_lab == NULL);
        }
    }

    /* Advance the insn queue.  Note that this check also detects
       a priority change within the instruction queue.  */
    if (ret == DISAS_NEXT && ctx->iaoq_b != ctx->iaoq_f + 4) {
        if (ctx->iaoq_b != -1 && ctx->iaoq_n != -1
            && use_goto_tb(ctx, ctx->iaoq_b)
            && (ctx->null_cond.c == TCG_COND_NEVER
                || ctx->null_cond.c == TCG_COND_ALWAYS)) {
            nullify_set(ctx, ctx->null_cond.c == TCG_COND_ALWAYS);
            gen_goto_tb(ctx, 0, ctx->iaoq_b, ctx->iaoq_n);
            ctx->base.is_jmp = ret = DISAS_NORETURN;
        } else {
            ctx->base.is_jmp = ret = DISAS_IAQ_N_STALE;
        }
    }
    ctx->iaoq_f = ctx->iaoq_b;
    ctx->iaoq_b = ctx->iaoq_n;
    ctx->base.pc_next += 4;

    switch (ret) {
    case DISAS_NORETURN:
    case DISAS_IAQ_N_UPDATED:
        break;

    case DISAS_NEXT:
    case DISAS_IAQ_N_STALE:
    case DISAS_IAQ_N_STALE_EXIT:
        if (ctx->iaoq_f == -1) {
            copy_iaoq_entry(ctx, cpu_iaoq_f, -1, cpu_iaoq_b);
            copy_iaoq_entry(ctx, cpu_iaoq_b, ctx->iaoq_n, ctx->iaoq_n_var);
#ifndef CONFIG_USER_ONLY
            tcg_gen_mov_i64(cpu_iasq_f, cpu_iasq_b);
#endif
            nullify_save(ctx);
            ctx->base.is_jmp = (ret == DISAS_IAQ_N_STALE_EXIT
                                ? DISAS_EXIT
                                : DISAS_IAQ_N_UPDATED);
        } else if (ctx->iaoq_b == -1) {
            copy_iaoq_entry(ctx, cpu_iaoq_b, -1, ctx->iaoq_n_var);
        }
        break;

    default:
        g_assert_not_reached();
    }
}

static void hppa_tr_tb_stop(DisasContextBase *dcbase, CPUState *cs)
{
    DisasContext *ctx = container_of(dcbase, DisasContext, base);
    DisasJumpType is_jmp = ctx->base.is_jmp;

    switch (is_jmp) {
    case DISAS_NORETURN:
        break;
    case DISAS_TOO_MANY:
    case DISAS_IAQ_N_STALE:
    case DISAS_IAQ_N_STALE_EXIT:
        copy_iaoq_entry(ctx, cpu_iaoq_f, ctx->iaoq_f, cpu_iaoq_f);
        copy_iaoq_entry(ctx, cpu_iaoq_b, ctx->iaoq_b, cpu_iaoq_b);
        nullify_save(ctx);
        /* FALLTHRU */
    case DISAS_IAQ_N_UPDATED:
        if (is_jmp != DISAS_IAQ_N_STALE_EXIT) {
            tcg_gen_lookup_and_goto_ptr();
            break;
        }
        /* FALLTHRU */
    case DISAS_EXIT:
        tcg_gen_exit_tb(NULL, 0);
        break;
    default:
        g_assert_not_reached();
    }
}

static void hppa_tr_disas_log(const DisasContextBase *dcbase,
                              CPUState *cs, FILE *logfile)
{
    target_ulong pc = dcbase->pc_first;

#ifdef CONFIG_USER_ONLY
    switch (pc) {
    case 0x00:
        fprintf(logfile, "IN:\n0x00000000:  (null)\n");
        return;
    case 0xb0:
        fprintf(logfile, "IN:\n0x000000b0:  light-weight-syscall\n");
        return;
    case 0xe0:
        fprintf(logfile, "IN:\n0x000000e0:  set-thread-pointer-syscall\n");
        return;
    case 0x100:
        fprintf(logfile, "IN:\n0x00000100:  syscall\n");
        return;
    }
#endif

    fprintf(logfile, "IN: %s\n", lookup_symbol(pc));
    target_disas(logfile, cs, pc, dcbase->tb->size);
}

static const TranslatorOps hppa_tr_ops = {
    .init_disas_context = hppa_tr_init_disas_context,
    .tb_start           = hppa_tr_tb_start,
    .insn_start         = hppa_tr_insn_start,
    .translate_insn     = hppa_tr_translate_insn,
    .tb_stop            = hppa_tr_tb_stop,
    .disas_log          = hppa_tr_disas_log,
};

void gen_intermediate_code(CPUState *cs, TranslationBlock *tb, int *max_insns,
                           target_ulong pc, void *host_pc)
{
    DisasContext ctx;
    translator_loop(cs, tb, max_insns, pc, host_pc, &hppa_tr_ops, &ctx.base);
}
