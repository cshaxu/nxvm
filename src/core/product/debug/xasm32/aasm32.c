/* This file is a part of NXVM project. */

#include "core/product/utils.h"

#include "core/product/debug/xasm32/aasm32.h"

#define NTVDM64_TYPE_TRACE_CONTEXT   trace
#define NTVDM64_TYPE_TRACE_ERROR flagError

#if NTVDM64_TYPE_TRACE_ENABLED == 1
#define _chrf(n) \
    do { \
        (n); \
        if (NTVDM64_TYPE_TRACE_ERROR) { \
            (NTVDM64_TYPE_TRACE_CONTEXT).flagError = 1; \
            ntvdm64_type_trace_finalize(&(NTVDM64_TYPE_TRACE_CONTEXT)); \
            return info; \
        } \
    } while (0)
#else
#define _chrf(n) \
    do { \
        (n); \
        if (NTVDM64_TYPE_TRACE_ERROR) { \
            return info; \
        } \
    } while (0)
#endif

/* set error and return */
#define _ser_  do {flagError = 1; return;} while (0)
#define _sert_ do {flagError = 1; return token;} while (0)
#define _serf_ do {flagError = 1; return info;} while (0)

/* operand size */
#define _SetOperandSize(n) (prefix_oprsize = (n) ? ((defsize ? 4 : 2) != (n)) : 0)
/* address size of the source operand */
#define _SetAddressSize(n) (prefix_addrsize = (n) ? ((defsize ? 4 : 2) != (n)) : 0)

typedef enum {
    TYPE_NONE,
    TYPE_SREG, TYPE_CREG,
    TYPE_DREG, TYPE_TREG,
    TYPE_I16_16, TYPE_I16_32,
    TYPE_R8, TYPE_R16, TYPE_R32,
    TYPE_I8, TYPE_I16, TYPE_I32,
    TYPE_M, TYPE_M8, TYPE_M16, TYPE_M32
} t_aasm_oprtype;
typedef enum {
    MOD_M,
    MOD_M_DISP8,
    MOD_M_DISP16,
    MOD_M_DISP32,
    MOD_R
} t_aasm_oprmod;
typedef enum {
    MEM_BX_SI, MEM_BX_DI,
    MEM_BP_SI, MEM_BP_DI,
    MEM_SI,    MEM_DI,
    MEM_BP,    MEM_BX,
    MEM_EAX, MEM_ECX,
    MEM_EDX, MEM_EBX,
    MEM_SIB, MEM_EBP,
    MEM_ESI, MEM_EDI,
    MEM_BX_AL, MEM_EBX_AL
} t_aasm_oprmem;
typedef enum {
    R8_AL, R8_CL, R8_DL, R8_BL,
    R8_AH, R8_CH, R8_DH, R8_BH
} t_aasm_oprreg8;
typedef enum {
    R16_AX, R16_CX, R16_DX, R16_BX,
    R16_SP, R16_BP, R16_SI, R16_DI
} t_aasm_oprreg16;
typedef enum {
    R32_EAX, R32_ECX, R32_EDX, R32_EBX,
    R32_ESP, R32_EBP, R32_ESI, R32_EDI
} t_aasm_oprreg32;
typedef enum {
    SREG_ES, SREG_CS,
    SREG_SS, SREG_DS,
    SREG_FS, SREG_GS
} t_aasm_oprsreg;
typedef enum {
    CREG_CR0, CREG_CR1, CREG_CR2, CREG_CR3,
    CREG_CR4, CREG_CR5, CREG_CR6, CREG_CR7
} t_aasm_oprcreg;
typedef enum {
    DREG_DR0, DREG_DR1, DREG_DR2, DREG_DR3,
    DREG_DR4, DREG_DR5, DREG_DR6, DREG_DR7
} t_aasm_oprdreg;
typedef enum {
    TREG_TR0, TREG_TR1, TREG_TR2, TREG_TR3,
    TREG_TR4, TREG_TR5, TREG_TR6, TREG_TR7
} t_aasm_oprtreg;
typedef enum {
    PTR_NONE,
    PTR_SHORT,
    PTR_NEAR,
    PTR_FAR
} t_aasm_oprptr;
typedef struct {
    t_aasm_oprreg32 base;
    t_aasm_oprreg32 index;
    uint8_t scale;
} t_aasm_oprsib;
typedef struct {
    t_aasm_oprtype  type;
    t_aasm_oprmod   mod;   /* active when type = 1, 2, 3, 6, 7, 8
     * 0 = mem; 1 = mem+disp8; 2 = mem+disp16; 3 = reg */
    t_aasm_oprmem   mem;   /* active when mod = 0, 1, 2
     * 0 = [BX+SI], 1 = [BX+DI], 2 = [BP+SI], 3 = [BP+DI],
     * 4 = [SI], 5 = [DI], 6 = [BP], 7 = [BX] */
    t_aasm_oprsib   sib;   /* active when mem = MEM_SIB */
    t_aasm_oprreg8  reg8;  /* active when type = 1, mod = 3
     * 0 = AL, 1 = CL, 2 = DL, 3 = BL,
     * 4 = AH, 5 = CH, 6 = DH, 7 = BH */
    t_aasm_oprreg16 reg16; /* active when type = 2, mod = 3
     * 0 = AX, 1 = CX, 2 = DX, 3 = BX,
     * 4 = SP, 5 = BP, 6 = SI, 7 = DI */
    t_aasm_oprreg32 reg32; /* active when type = 3, mod = 3
     * 0 = EAX, 1 = ECX, 2 = EDX, 3 = EBX,
     * 4 = ESP, 5 = EBP, 6 = ESI, 7 = EDI */
    t_aasm_oprsreg  sreg;  /* active when type = 3
     * 0 = ES, 1 = CS, 2 = SS, 3 = DS, 4 = FS, 5 = GS */
    t_aasm_oprcreg  creg;
    t_aasm_oprdreg  dreg;
    t_aasm_oprtreg  treg;
    uint8_t   imms;  /* if imm is signed */
    uint8_t   immn;  /* if imm is negative */
    uint8_t   imm8;
    uint16_t  imm16;
    uint32_t    imm32;
    char            disp8;
    uint16_t  disp16; /* use as imm when type = 6; use by modrm as disp when mod = 0,1,2; */
    uint32_t    disp32; /* use as imm when type = 7; use by modrm as disp when mod = 0,1,2; */
    t_aasm_oprptr   ptr; /* 0 = near; 1 = far */
    uint16_t   rcs;
    uint32_t       reip;
    char            label[0x100];
    uint8_t flages, flagcs, flagss, flagds, flagfs, flaggs;
} t_aasm_oprinfo;
/* global variables */

typedef uint8_t t_aasm_prefix;

typedef struct aasm32_context {
    ntvdm64_type_trace trace;
    uint8_t defsize;
    t_aasm_prefix prefix_oprsizeg, prefix_addrsizeg;
    t_aasm_prefix prefix_oprsize, prefix_addrsize;
    t_aasm_prefix prefix_lock, prefix_repz, prefix_repnz;
    uint8_t acode[15];
    uint8_t iop;
    char *rop, *ropr1, *ropr2, *ropr3;
    uint16_t avcs, avip;
    char *aop, *aopr1, *aopr2;
    uint8_t flagError;
    t_aasm_oprinfo aoprig, aopri1, aopri2, aopri3;
    t_aasm_oprinfo *rinfo;
    uint8_t tokimm8;
    uint16_t tokimm16;
    uint32_t tokimm32;
    char tokchar;
    char tokstring[0x100], toklabel[0x100];
    char *tokptr;
    char *rstart;
} aasm32_context;

static _Thread_local aasm32_context *aasmContext;

#define trace (aasmContext->trace)
#define defsize (aasmContext->defsize)
#define prefix_oprsizeg (aasmContext->prefix_oprsizeg)
#define prefix_addrsizeg (aasmContext->prefix_addrsizeg)
#define prefix_oprsize (aasmContext->prefix_oprsize)
#define prefix_addrsize (aasmContext->prefix_addrsize)
#define prefix_lock (aasmContext->prefix_lock)
#define prefix_repz (aasmContext->prefix_repz)
#define prefix_repnz (aasmContext->prefix_repnz)
#define acode (aasmContext->acode)
#define iop (aasmContext->iop)
#define rop (aasmContext->rop)
#define ropr1 (aasmContext->ropr1)
#define ropr2 (aasmContext->ropr2)
#define ropr3 (aasmContext->ropr3)
#define avcs (aasmContext->avcs)
#define avip (aasmContext->avip)
#define aop (aasmContext->aop)
#define aopr1 (aasmContext->aopr1)
#define aopr2 (aasmContext->aopr2)
#define flagError (aasmContext->flagError)
#define aoprig (aasmContext->aoprig)
#define aopri1 (aasmContext->aopri1)
#define aopri2 (aasmContext->aopri2)
#define aopri3 (aasmContext->aopri3)
#define rinfo (aasmContext->rinfo)
#define tokimm8 (aasmContext->tokimm8)
#define tokimm16 (aasmContext->tokimm16)
#define tokimm32 (aasmContext->tokimm32)
#define tokchar (aasmContext->tokchar)
#define tokstring (aasmContext->tokstring)
#define toklabel (aasmContext->toklabel)
#define tokptr (aasmContext->tokptr)
#define rstart (aasmContext->rstart)
/* arg flag level 0 */
#define isNONE(oprinf)  ((oprinf).type  == TYPE_NONE)
#define isR8(oprinf)    ((oprinf).type  == TYPE_R8 && (oprinf).mod == MOD_R)
#define isR16(oprinf)   ((oprinf).type  == TYPE_R16 && (oprinf).mod == MOD_R)
#define isR32(oprinf)   ((oprinf).type  == TYPE_R32 && (oprinf).mod == MOD_R)
#define isSREG(oprinf)  ((oprinf).type  == TYPE_SREG && (oprinf).mod == MOD_R)
#define isCREG(oprinf)  ((oprinf).type  == TYPE_CREG)
#define isDREG(oprinf)  ((oprinf).type  == TYPE_DREG)
#define isTREG(oprinf)  ((oprinf).type  == TYPE_TREG)
#define isI8(oprinf)    ((oprinf).type  == TYPE_I8)
#define isI8u(oprinf)   (isI8(oprinf)   && !(oprinf).imms)
#define isI8s(oprinf)   (isI8(oprinf)   && (oprinf).imms)
#define isI16(oprinf)   ((oprinf).type  == TYPE_I16)
#define isI16u(oprinf)  (isI16(oprinf)  && !(oprinf).imms)
#define isI16s(oprinf)  (isI16(oprinf)  && (oprinf).imms)
#define isI32(oprinf)   ((oprinf).type  == TYPE_I32)
#define isI32u(oprinf)  (isI32(oprinf)  && !(oprinf).imms)
#define isI32s(oprinf)  (isI32(oprinf)  && (oprinf).imms)
#define isI16p(oprinf)  ((oprinf).type  == TYPE_I16_16)
#define isI32p(oprinf)  ((oprinf).type  == TYPE_I16_32)
#define isM(oprinf)     (((oprinf).type == TYPE_M   || (oprinf).type == TYPE_M8   || \
                          (oprinf).type == TYPE_M16 || (oprinf).type == TYPE_M32) && (oprinf).mod != MOD_R)
#define isM8(oprinf)    (((oprinf).type == TYPE_M   || (oprinf).type == TYPE_M8 ) && (oprinf).mod != MOD_R)
#define isM16(oprinf)   (((oprinf).type == TYPE_M   || (oprinf).type == TYPE_M16) && (oprinf).mod != MOD_R)
#define isM32(oprinf)   (((oprinf).type == TYPE_M   || (oprinf).type == TYPE_M32) && (oprinf).mod != MOD_R)
#define isMs(oprinf)    ((oprinf).type  == TYPE_M   && (oprinf).mod != MOD_R)
#define isM8s(oprinf)   ((oprinf).type  == TYPE_M8  && (oprinf).mod != MOD_R)
#define isM16s(oprinf)  ((oprinf).type  == TYPE_M16 && (oprinf).mod != MOD_R)
#define isM32s(oprinf)  ((oprinf).type  == TYPE_M32 && (oprinf).mod != MOD_R)
#define isPNONE(oprinf) ((oprinf).ptr == PTR_NONE)
#define isNEAR(oprinf)  ((oprinf).ptr == PTR_NEAR)
#define isSHORT(oprinf) ((oprinf).ptr == PTR_SHORT)
#define isFAR(oprinf)   ((oprinf).ptr == PTR_FAR)
/* arg flag level 1 */
#define isRM8s(oprinf)  (isR8 (oprinf) || isM8s(oprinf))
#define isRM16s(oprinf) (isR16(oprinf) || isM16s(oprinf))
#define isRM32s(oprinf) (isR32(oprinf) || isM32s(oprinf))
#define isRM8(oprinf)   (isR8 (oprinf) || isM8 (oprinf))
#define isRM16(oprinf)  (isR16(oprinf) || isM16(oprinf))
#define isRM32(oprinf)  (isR32(oprinf) || isM32(oprinf))
#define isRM(oprinf)    (isRM8(oprinf) || isRM16(oprinf))
#define isCR0(oprinf)   (isCREG(oprinf) && (oprinf).creg == CREG_CR0)
#define isCR2(oprinf)   (isCREG(oprinf) && (oprinf).creg == CREG_CR2)
#define isCR3(oprinf)   (isCREG(oprinf) && (oprinf).creg == CREG_CR3)
#define isDR0(oprinf)   (isDREG(oprinf) && (oprinf).creg == DREG_DR0)
#define isDR1(oprinf)   (isDREG(oprinf) && (oprinf).creg == DREG_DR1)
#define isDR2(oprinf)   (isDREG(oprinf) && (oprinf).creg == DREG_DR2)
#define isDR3(oprinf)   (isDREG(oprinf) && (oprinf).creg == DREG_DR3)
#define isDR6(oprinf)   (isDREG(oprinf) && (oprinf).creg == DREG_DR6)
#define isDR7(oprinf)   (isDREG(oprinf) && (oprinf).creg == DREG_DR7)
#define isTR6(oprinf)   (isTREG(oprinf) && (oprinf).creg == TREG_TR6)
#define isTR7(oprinf)   (isTREG(oprinf) && (oprinf).creg == TREG_TR7)
#define isAL(oprinf)    (isR8 (oprinf) && (oprinf).reg8  == R8_AL)
#define isCL(oprinf)    (isR8 (oprinf) && (oprinf).reg8  == R8_CL)
#define isDL(oprinf)    (isR8 (oprinf) && (oprinf).reg8  == R8_DL)
#define isBL(oprinf)    (isR8 (oprinf) && (oprinf).reg8  == R8_BL)
#define isAH(oprinf)    (isR8 (oprinf) && (oprinf).reg8  == R8_AH)
#define isCH(oprinf)    (isR8 (oprinf) && (oprinf).reg8  == R8_CH)
#define isDH(oprinf)    (isR8 (oprinf) && (oprinf).reg8  == R8_DH)
#define isBH(oprinf)    (isR8 (oprinf) && (oprinf).reg8  == R8_BH)
#define isAX(oprinf)    (isR16(oprinf) && (oprinf).reg16 == R16_AX)
#define isCX(oprinf)    (isR16(oprinf) && (oprinf).reg16 == R16_CX)
#define isDX(oprinf)    (isR16(oprinf) && (oprinf).reg16 == R16_DX)
#define isBX(oprinf)    (isR16(oprinf) && (oprinf).reg16 == R16_BX)
#define isSP(oprinf)    (isR16(oprinf) && (oprinf).reg16 == R16_SP)
#define isBP(oprinf)    (isR16(oprinf) && (oprinf).reg16 == R16_BP)
#define isSI(oprinf)    (isR16(oprinf) && (oprinf).reg16 == R16_SI)
#define isDI(oprinf)    (isR16(oprinf) && (oprinf).reg16 == R16_DI)
#define isEAX(oprinf)   (isR32(oprinf) && (oprinf).reg32 == R32_EAX)
#define isECX(oprinf)   (isR32(oprinf) && (oprinf).reg32 == R32_ECX)
#define isEDX(oprinf)   (isR32(oprinf) && (oprinf).reg32 == R32_EDX)
#define isEBX(oprinf)   (isR32(oprinf) && (oprinf).reg32 == R32_EBX)
#define isESP(oprinf)   (isR32(oprinf) && (oprinf).reg32 == R32_ESP)
#define isEBP(oprinf)   (isR32(oprinf) && (oprinf).reg32 == R32_EBP)
#define isESI(oprinf)   (isR32(oprinf) && (oprinf).reg32 == R32_ESI)
#define isEDI(oprinf)   (isR32(oprinf) && (oprinf).reg32 == R32_EDI)
#define isES(oprinf)    (isSREG(oprinf) && (oprinf).sreg   == SREG_ES)
#define isCS(oprinf)    (isSREG(oprinf) && (oprinf).sreg   == SREG_CS)
#define isSS(oprinf)    (isSREG(oprinf) && (oprinf).sreg   == SREG_SS)
#define isDS(oprinf)    (isSREG(oprinf) && (oprinf).sreg   == SREG_DS)
#define isFS(oprinf)    (isSREG(oprinf) && (oprinf).sreg   == SREG_FS)
#define isGS(oprinf)    (isSREG(oprinf) && (oprinf).sreg   == SREG_GS)
#define isESDI8(oprinf)    (isM8 (oprinf) && (oprinf).flages && (oprinf).mem == MEM_DI  && (oprinf).mod == MOD_M)
#define isESDI16(oprinf)   (isM16(oprinf) && (oprinf).flages && (oprinf).mem == MEM_DI  && (oprinf).mod == MOD_M)
#define isESDI32(oprinf)   (isM32(oprinf) && (oprinf).flages && (oprinf).mem == MEM_DI  && (oprinf).mod == MOD_M)
#define isESEDI8(oprinf)   (isM8 (oprinf) && (oprinf).flages && (oprinf).mem == MEM_EDI && (oprinf).mod == MOD_M)
#define isESEDI16(oprinf)  (isM16(oprinf) && (oprinf).flages && (oprinf).mem == MEM_EDI && (oprinf).mod == MOD_M)
#define isESEDI32(oprinf)  (isM32(oprinf) && (oprinf).flages && (oprinf).mem == MEM_EDI && (oprinf).mod == MOD_M)
#define isDSSI8(oprinf)    (isM8 (oprinf) && (oprinf).mem == MEM_SI  && (oprinf).mod == MOD_M)
#define isDSSI16(oprinf)   (isM16(oprinf) && (oprinf).mem == MEM_SI  && (oprinf).mod == MOD_M)
#define isDSSI32(oprinf)   (isM32(oprinf) && (oprinf).mem == MEM_SI  && (oprinf).mod == MOD_M)
#define isDSESI8(oprinf)   (isM8 (oprinf) && (oprinf).mem == MEM_ESI && (oprinf).mod == MOD_M)
#define isDSESI16(oprinf)  (isM16(oprinf) && (oprinf).mem == MEM_ESI && (oprinf).mod == MOD_M)
#define isDSESI32(oprinf)  (isM32(oprinf) && (oprinf).mem == MEM_ESI && (oprinf).mod == MOD_M)
#define isDSBXAL8(oprinf)  (isM8 (oprinf) && (oprinf).mem == MEM_BX_AL)
#define isDSEBXAL8(oprinf) (isM8 (oprinf) && (oprinf).mem == MEM_EBX_AL)
#define isESDI8s(oprinf)    (isM8s (oprinf) && (oprinf).flages && (oprinf).mem == MEM_DI  && (oprinf).mod == MOD_M)
#define isESDI16s(oprinf)   (isM16s(oprinf) && (oprinf).flages && (oprinf).mem == MEM_DI  && (oprinf).mod == MOD_M)
#define isESDI32s(oprinf)   (isM32s(oprinf) && (oprinf).flages && (oprinf).mem == MEM_DI  && (oprinf).mod == MOD_M)
#define isESEDI8s(oprinf)   (isM8s (oprinf) && (oprinf).flages && (oprinf).mem == MEM_EDI && (oprinf).mod == MOD_M)
#define isESEDI16s(oprinf)  (isM16s(oprinf) && (oprinf).flages && (oprinf).mem == MEM_EDI && (oprinf).mod == MOD_M)
#define isESEDI32s(oprinf)  (isM32s(oprinf) && (oprinf).flages && (oprinf).mem == MEM_EDI && (oprinf).mod == MOD_M)
#define isDSSI8s(oprinf)    (isM8s (oprinf) && (oprinf).mem == MEM_SI  && (oprinf).mod == MOD_M)
#define isDSSI16s(oprinf)   (isM16s(oprinf) && (oprinf).mem == MEM_SI  && (oprinf).mod == MOD_M)
#define isDSSI32s(oprinf)   (isM32s(oprinf) && (oprinf).mem == MEM_SI  && (oprinf).mod == MOD_M)
#define isDSESI8s(oprinf)   (isM8s (oprinf) && (oprinf).mem == MEM_ESI && (oprinf).mod == MOD_M)
#define isDSESI16s(oprinf)  (isM16s(oprinf) && (oprinf).mem == MEM_ESI && (oprinf).mod == MOD_M)
#define isDSESI32s(oprinf)  (isM32s(oprinf) && (oprinf).mem == MEM_ESI && (oprinf).mod == MOD_M)
#define isDSBXAL8s(oprinf)  (isM8s (oprinf) && (oprinf).mem == MEM_BX_AL)
#define isDSEBXAL8s(oprinf) (isM8s (oprinf) && (oprinf).mem == MEM_EBX_AL)

/* arg flag level 2 */
#define ARG_NONE        (isNONE (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_I8          (isI8   (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_I16         (isI16  (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_I32         (isI32  (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_I8s         (isI8s  (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_I16s        (isI16s (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_I32s        (isI32s (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_R32         (isR32  (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_M32         (isM32  (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_M16s        (isM16s (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_M32s        (isM32s (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_RM8         (isRM8  (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_RM8s        (isRM8s (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_RM16s       (isRM16s(aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_RM32s       (isRM32s(aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_RM8_R8      (isRM8  (aopri1) && isR8  (aopri2)  && isNONE(aopri3))
#define ARG_RM16_R16    (isRM16 (aopri1) && isR16 (aopri2)  && isNONE(aopri3))
#define ARG_RM32_R32    (isRM32 (aopri1) && isR32 (aopri2)  && isNONE(aopri3))
#define ARG_R8_RM8      (isR8   (aopri1) && isRM8 (aopri2)  && isNONE(aopri3))
#define ARG_R16_RM16    (isR16  (aopri1) && isRM16(aopri2)  && isNONE(aopri3))
#define ARG_R32_RM32    (isR32  (aopri1) && isRM32(aopri2)  && isNONE(aopri3))
#define ARG_R16_RM8s    (isR16  (aopri1) && isRM8s(aopri2)  && isNONE(aopri3))
#define ARG_R32_RM8s    (isR32  (aopri1) && isRM8s(aopri2)  && isNONE(aopri3))
#define ARG_R32_RM16s   (isR32  (aopri1) && isRM16s(aopri2) && isNONE(aopri3))
#define ARG_CR0_R32     (isCR0  (aopri1) && isR32(aopri2)   && isNONE(aopri3))
#define ARG_CR2_R32     (isCR2  (aopri1) && isR32(aopri2)   && isNONE(aopri3))
#define ARG_CR3_R32     (isCR3  (aopri1) && isR32(aopri2)   && isNONE(aopri3))
#define ARG_R32_CR0     (isR32  (aopri1) && isCR0(aopri2)   && isNONE(aopri3))
#define ARG_R32_CR2     (isR32  (aopri1) && isCR2(aopri2)   && isNONE(aopri3))
#define ARG_R32_CR3     (isR32  (aopri1) && isCR3(aopri2)   && isNONE(aopri3))
#define ARG_DR0_R32     (isDR0  (aopri1) && isR32(aopri2)   && isNONE(aopri3))
#define ARG_DR1_R32     (isDR1  (aopri1) && isR32(aopri2)   && isNONE(aopri3))
#define ARG_DR2_R32     (isDR2  (aopri1) && isR32(aopri2)   && isNONE(aopri3))
#define ARG_DR3_R32     (isDR3  (aopri1) && isR32(aopri2)   && isNONE(aopri3))
#define ARG_DR6_R32     (isDR6  (aopri1) && isR32(aopri2)   && isNONE(aopri3))
#define ARG_DR7_R32     (isDR7  (aopri1) && isR32(aopri2)   && isNONE(aopri3))
#define ARG_R32_DR0     (isR32  (aopri1) && isDR0(aopri2)   && isNONE(aopri3))
#define ARG_R32_DR1     (isR32  (aopri1) && isDR1(aopri2)   && isNONE(aopri3))
#define ARG_R32_DR2     (isR32  (aopri1) && isDR2(aopri2)   && isNONE(aopri3))
#define ARG_R32_DR3     (isR32  (aopri1) && isDR3(aopri2)   && isNONE(aopri3))
#define ARG_R32_DR6     (isR32  (aopri1) && isDR6(aopri2)   && isNONE(aopri3))
#define ARG_R32_DR7     (isR32  (aopri1) && isDR7(aopri2)   && isNONE(aopri3))
#define ARG_TR6_R32     (isTR6  (aopri1) && isR32(aopri2)   && isNONE(aopri3))
#define ARG_TR7_R32     (isTR7  (aopri1) && isR32(aopri2)   && isNONE(aopri3))
#define ARG_R32_TR6     (isR32  (aopri1) && isTR6(aopri2)   && isNONE(aopri3))
#define ARG_R32_TR7     (isR32  (aopri1) && isTR7(aopri2)   && isNONE(aopri3))
#define ARG_ES          (isES   (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_CS          (isCS   (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_SS          (isSS   (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_DS          (isDS   (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_FS          (isFS   (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_GS          (isGS   (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_AX          (isAX   (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_CX          (isCX   (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_DX          (isDX   (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_BX          (isBX   (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_SP          (isSP   (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_BP          (isBP   (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_SI          (isSI   (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_DI          (isDI   (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_EAX         (isEAX  (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_ECX         (isECX  (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_EDX         (isEDX  (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_EBX         (isEBX  (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_ESP         (isESP  (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_EBP         (isEBP  (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_ESI         (isESI  (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_EDI         (isEDI  (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_AL_I8u      (isAL   (aopri1) && isI8u (aopri2)  && isNONE(aopri3))
#define ARG_AX_I8u      (isAX   (aopri1) && isI8u (aopri2)  && isNONE(aopri3))
#define ARG_EAX_I8u     (isEAX  (aopri1) && isI8u (aopri2)  && isNONE(aopri3))
#define ARG_I8u_AL      (isI8u  (aopri1) && isAL  (aopri2)  && isNONE(aopri3))
#define ARG_I8u_AX      (isI8u  (aopri1) && isAX  (aopri2)  && isNONE(aopri3))
#define ARG_I8u_EAX      (isI8u (aopri1) && isEAX (aopri2)  && isNONE(aopri3))
#define ARG_AL_I8       (isAL   (aopri1) && isI8  (aopri2)  && isNONE(aopri3))
#define ARG_CL_I8       (isCL   (aopri1) && isI8  (aopri2)  && isNONE(aopri3))
#define ARG_DL_I8       (isDL   (aopri1) && isI8  (aopri2)  && isNONE(aopri3))
#define ARG_BL_I8       (isBL   (aopri1) && isI8  (aopri2)  && isNONE(aopri3))
#define ARG_AH_I8       (isAH   (aopri1) && isI8  (aopri2)  && isNONE(aopri3))
#define ARG_CH_I8       (isCH   (aopri1) && isI8  (aopri2)  && isNONE(aopri3))
#define ARG_DH_I8       (isDH   (aopri1) && isI8  (aopri2)  && isNONE(aopri3))
#define ARG_BH_I8       (isBH   (aopri1) && isI8  (aopri2)  && isNONE(aopri3))
#define ARG_AX_I16      (isAX   (aopri1) && isI16 (aopri2)  && isNONE(aopri3))
#define ARG_CX_I16      (isCX   (aopri1) && isI16 (aopri2)  && isNONE(aopri3))
#define ARG_DX_I16      (isDX   (aopri1) && isI16 (aopri2)  && isNONE(aopri3))
#define ARG_BX_I16      (isBX   (aopri1) && isI16 (aopri2)  && isNONE(aopri3))
#define ARG_SP_I16      (isSP   (aopri1) && isI16 (aopri2)  && isNONE(aopri3))
#define ARG_BP_I16      (isBP   (aopri1) && isI16 (aopri2)  && isNONE(aopri3))
#define ARG_SI_I16      (isSI   (aopri1) && isI16 (aopri2)  && isNONE(aopri3))
#define ARG_DI_I16      (isDI   (aopri1) && isI16 (aopri2)  && isNONE(aopri3))
#define ARG_EAX_I32     (isEAX  (aopri1) && isI32 (aopri2)  && isNONE(aopri3))
#define ARG_ECX_I32     (isECX  (aopri1) && isI32 (aopri2)  && isNONE(aopri3))
#define ARG_EDX_I32     (isEDX  (aopri1) && isI32 (aopri2)  && isNONE(aopri3))
#define ARG_EBX_I32     (isEBX  (aopri1) && isI32 (aopri2)  && isNONE(aopri3))
#define ARG_ESP_I32     (isESP  (aopri1) && isI32 (aopri2)  && isNONE(aopri3))
#define ARG_EBP_I32     (isEBP  (aopri1) && isI32 (aopri2)  && isNONE(aopri3))
#define ARG_ESI_I32     (isESI  (aopri1) && isI32 (aopri2)  && isNONE(aopri3))
#define ARG_EDI_I32     (isEDI  (aopri1) && isI32 (aopri2)  && isNONE(aopri3))
#define ARG_RM8_I8      (isRM8s (aopri1) && isI8  (aopri2)  && isNONE(aopri3))
#define ARG_RM16_I16    (isRM16s(aopri1) && isI16 (aopri2)  && isNONE(aopri3))
#define ARG_RM32_I32    (isRM32s(aopri1) && isI32 (aopri2)  && isNONE(aopri3))
#define ARG_RM16_I8     (isRM16s(aopri1) && isI8  (aopri2)  && isNONE(aopri3))
#define ARG_RM32_I8     (isRM32s(aopri1) && isI8  (aopri2)  && isNONE(aopri3))
#define ARG_RM16_I8     (isRM16s(aopri1) && isI8  (aopri2)  && isNONE(aopri3))
#define ARG_I16_I8      (isI16  (aopri1) && isI8  (aopri2)  && isNONE(aopri3))
#define ARG_RM16_SREG   (isRM16 (aopri1) && isSREG (aopri2) && isNONE(aopri3))
#define ARG_RM32_SREG   (isRM32 (aopri1) && isSREG (aopri2) && isNONE(aopri3))
#define ARG_SREG_RM16   (isSREG  (aopri1) && isRM16(aopri2) && isNONE(aopri3))
#define ARG_SREG_RM32   (isSREG  (aopri1) && isRM32(aopri2) && isNONE(aopri3))
#define ARG_RM16        (isRM16 (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_RM32        (isRM32 (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_AX_AX       (isAX   (aopri1) && isAX  (aopri2)  && isNONE(aopri3))
#define ARG_CX_AX       (isCX   (aopri1) && isAX  (aopri2)  && isNONE(aopri3))
#define ARG_DX_AX       (isDX   (aopri1) && isAX  (aopri2)  && isNONE(aopri3))
#define ARG_BX_AX       (isBX   (aopri1) && isAX  (aopri2)  && isNONE(aopri3))
#define ARG_SP_AX       (isSP   (aopri1) && isAX  (aopri2)  && isNONE(aopri3))
#define ARG_BP_AX       (isBP   (aopri1) && isAX  (aopri2)  && isNONE(aopri3))
#define ARG_SI_AX       (isSI   (aopri1) && isAX  (aopri2)  && isNONE(aopri3))
#define ARG_DI_AX       (isDI   (aopri1) && isAX  (aopri2)  && isNONE(aopri3))
#define ARG_EAX_EAX     (isEAX  (aopri1) && isEAX (aopri2)  && isNONE(aopri3))
#define ARG_ECX_EAX     (isECX  (aopri1) && isEAX (aopri2)  && isNONE(aopri3))
#define ARG_EDX_EAX     (isEDX  (aopri1) && isEAX (aopri2)  && isNONE(aopri3))
#define ARG_EBX_EAX     (isEBX  (aopri1) && isEAX (aopri2)  && isNONE(aopri3))
#define ARG_ESP_EAX     (isESP  (aopri1) && isEAX (aopri2)  && isNONE(aopri3))
#define ARG_EBP_EAX     (isEBP  (aopri1) && isEAX (aopri2)  && isNONE(aopri3))
#define ARG_ESI_EAX     (isESI  (aopri1) && isEAX (aopri2)  && isNONE(aopri3))
#define ARG_EDI_EAX     (isEDI  (aopri1) && isEAX (aopri2)  && isNONE(aopri3))
#define ARG_AL_MOFFS8   (isAL   (aopri1) && isM8  (aopri2)  && (aopri2.mod == MOD_M && (aopri2.mem == MEM_BP || aopri2.mem == MEM_EBP)) && isNONE(aopri3))
#define ARG_MOFFS8_AL   (isM8   (aopri1) && isAL  (aopri2)  && (aopri1.mod == MOD_M && (aopri1.mem == MEM_BP || aopri2.mem == MEM_EBP)) && isNONE(aopri3))
#define ARG_AX_MOFFS16  (isAX   (aopri1) && isM16 (aopri2)  && (aopri2.mod == MOD_M && (aopri2.mem == MEM_BP || aopri2.mem == MEM_EBP)) && isNONE(aopri3))
#define ARG_MOFFS16_AX  (isM16  (aopri1) && isAX  (aopri2)  && (aopri1.mod == MOD_M && (aopri1.mem == MEM_BP || aopri1.mem == MEM_EBP)) && isNONE(aopri3))
#define ARG_EAX_MOFFS32 (isEAX  (aopri1) && isM32 (aopri2)  && (aopri2.mod == MOD_M && (aopri2.mem == MEM_BP || aopri2.mem == MEM_EBP)) && isNONE(aopri3))
#define ARG_MOFFS32_EAX (isM32  (aopri1) && isEAX (aopri2)  && (aopri1.mod == MOD_M && (aopri1.mem == MEM_BP || aopri1.mem == MEM_EBP)) && isNONE(aopri3))
#define ARG_R16_M16     (isR16  (aopri1) && isM16 (aopri2)  && isNONE(aopri3))
#define ARG_R32_M32     (isR32  (aopri1) && isM32 (aopri2)  && isNONE(aopri3))
#define ARG_I16u        (isI16u (aopri1) && isNONE(aopri2)  && isNONE(aopri3))
#define ARG_PNONE_I8s   (isPNONE(aopri1) && isI8s (aopri1)  && isNONE(aopri2) && isNONE(aopri3))
#define ARG_PNONE_I16s  (isPNONE(aopri1) && isI16s(aopri1)  && isNONE(aopri2) && isNONE(aopri3))
#define ARG_PNONE_I32s  (isPNONE(aopri1) && isI32s(aopri1)  && isNONE(aopri2) && isNONE(aopri3))
#define ARG_SHORT_I8s   (isSHORT(aopri1) && isI8s (aopri1)  && isNONE(aopri2) && isNONE(aopri3))
#define ARG_NEAR_I16s   (isNEAR (aopri1) && isI16s(aopri1)  && isNONE(aopri2) && isNONE(aopri3))
#define ARG_NEAR_I32s   (isNEAR (aopri1) && isI32s(aopri1)  && isNONE(aopri2) && isNONE(aopri3))
#define ARG_FAR_I16_16  (isFAR  (aopri1) && isI16p(aopri1)  && isNONE(aopri2) && isNONE(aopri3))
#define ARG_FAR_I16_32  (isFAR  (aopri1) && isI32p(aopri1)  && isNONE(aopri2) && isNONE(aopri3))
#define ARG_PNONE_RM16s (isPNONE(aopri1) && isRM16s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_NEAR_RM16s  (isNEAR (aopri1) && isRM16s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_PNONE_RM32s (isPNONE(aopri1) && isRM32s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_NEAR_RM32s  (isNEAR (aopri1) && isRM32s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_FAR_M16_16  (isFAR  (aopri1) && isM16s (aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_FAR_M16_32  (isFAR  (aopri1) && isM32s (aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_RM8_CL      (isRM8s (aopri1) && isCL   (aopri2) && isNONE(aopri3))
#define ARG_RM16_CL     (isRM16s(aopri1) && isCL   (aopri2) && isNONE(aopri3))
#define ARG_RM32_CL     (isRM32s(aopri1) && isCL   (aopri2) && isNONE(aopri3))
#define ARG_AL_DX       (isAL   (aopri1) && isDX   (aopri2) && isNONE(aopri3))
#define ARG_DX_AL       (isDX   (aopri1) && isAL   (aopri2) && isNONE(aopri3))
#define ARG_AX_DX       (isAX   (aopri1) && isDX   (aopri2) && isNONE(aopri3))
#define ARG_EAX_DX      (isEAX  (aopri1) && isDX   (aopri2) && isNONE(aopri3))
#define ARG_DX_EAX      (isDX   (aopri1) && isEAX  (aopri2) && isNONE(aopri3))
#define ARG_ESDI8_DSSI8   (isESDI8 (aopri1) && isDSSI8 (aopri2) && isNONE(aopri3))
#define ARG_ESDI16_DSSI16 (isESDI16(aopri1) && isDSSI16(aopri2) && isNONE(aopri3))
#define ARG_ESDI32_DSSI32 (isESDI32(aopri1) && isDSSI32(aopri2) && isNONE(aopri3))
#define ARG_DSSI8_ESDI8   (isDSSI8 (aopri1) && isESDI8 (aopri2) && isNONE(aopri3))
#define ARG_DSSI16_ESDI16 (isDSSI16(aopri1) && isESDI16(aopri2) && isNONE(aopri3))
#define ARG_DSSI32_ESDI32 (isDSSI32(aopri1) && isESDI32(aopri2) && isNONE(aopri3))
#define ARG_ESDI8_DX      (isESDI8 (aopri1) && isDX    (aopri2) && isNONE(aopri3))
#define ARG_ESDI16_DX     (isESDI16(aopri1) && isDX    (aopri2) && isNONE(aopri3))
#define ARG_ESDI32_DX     (isESDI32(aopri1) && isDX    (aopri2) && isNONE(aopri3))
#define ARG_DX_DSSI8      (isDX(aopri1)     && isDSSI8 (aopri2) && isNONE(aopri3))
#define ARG_DX_DSSI16     (isDX(aopri1)     && isDSSI16(aopri2) && isNONE(aopri3))
#define ARG_DX_DSSI32     (isDX(aopri1)     && isDSSI32(aopri2) && isNONE(aopri3))
#define ARG_DSSI8         (isDSSI8 (aopri1) && isNONE  (aopri2) && isNONE(aopri3))
#define ARG_DSSI16        (isDSSI16(aopri1) && isNONE  (aopri2) && isNONE(aopri3))
#define ARG_DSSI32        (isDSSI32(aopri1) && isNONE  (aopri2) && isNONE(aopri3))
#define ARG_ESDI8         (isESDI8 (aopri1) && isNONE  (aopri2) && isNONE(aopri3))
#define ARG_ESDI16        (isESDI16(aopri1) && isNONE  (aopri2) && isNONE(aopri3))
#define ARG_ESDI32        (isESDI32(aopri1) && isNONE  (aopri2) && isNONE(aopri3))
#define ARG_ESEDI8_DSESI8   (isESEDI8 (aopri1) && isDSESI8 (aopri2) && isNONE(aopri3))
#define ARG_ESEDI16_DSESI16 (isESEDI16(aopri1) && isDSESI16(aopri2) && isNONE(aopri3))
#define ARG_ESEDI32_DSESI32 (isESEDI32(aopri1) && isDSESI32(aopri2) && isNONE(aopri3))
#define ARG_DSESI8_ESEDI8   (isDSESI8 (aopri1) && isESEDI8 (aopri2) && isNONE(aopri3))
#define ARG_DSESI16_ESEDI16 (isDSESI16(aopri1) && isESEDI16(aopri2) && isNONE(aopri3))
#define ARG_DSESI32_ESEDI32 (isDSESI32(aopri1) && isESEDI32(aopri2) && isNONE(aopri3))
#define ARG_ESEDI8_DX       (isESEDI8 (aopri1) && isDX    (aopri2)  && isNONE(aopri3))
#define ARG_ESEDI16_DX      (isESEDI16(aopri1) && isDX    (aopri2)  && isNONE(aopri3))
#define ARG_ESEDI32_DX      (isESEDI32(aopri1) && isDX    (aopri2)  && isNONE(aopri3))
#define ARG_DX_DSESI8       (isDX(aopri1)      && isDSESI8 (aopri2) && isNONE(aopri3))
#define ARG_DX_DSESI16      (isDX(aopri1)      && isDSESI16(aopri2) && isNONE(aopri3))
#define ARG_DX_DSESI32      (isDX(aopri1)      && isDSESI32(aopri2) && isNONE(aopri3))
#define ARG_DSESI8          (isDSESI8 (aopri1) && isNONE  (aopri2)  && isNONE(aopri3))
#define ARG_DSESI16         (isDSESI16(aopri1) && isNONE  (aopri2)  && isNONE(aopri3))
#define ARG_DSESI32         (isDSESI32(aopri1) && isNONE  (aopri2)  && isNONE(aopri3))
#define ARG_ESEDI8          (isESEDI8 (aopri1) && isNONE  (aopri2)  && isNONE(aopri3))
#define ARG_ESEDI16         (isESEDI16(aopri1) && isNONE  (aopri2)  && isNONE(aopri3))
#define ARG_ESEDI32         (isESEDI32(aopri1) && isNONE  (aopri2)  && isNONE(aopri3))
#define ARG_DSBXAL8         (isDSBXAL8(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_DSEBXAL8        (isDSEBXAL8(aopri1)&& isNONE(aopri2) && isNONE(aopri3))
#define ARG_ESDI8_DSSI8   (isESDI8 (aopri1) && isDSSI8 (aopri2) && isNONE(aopri3))
#define ARG_ESDI16_DSSI16 (isESDI16(aopri1) && isDSSI16(aopri2) && isNONE(aopri3))
#define ARG_ESDI32_DSSI32 (isESDI32(aopri1) && isDSSI32(aopri2) && isNONE(aopri3))
#define ARG_DSSI8_ESDI8   (isDSSI8 (aopri1) && isESDI8 (aopri2) && isNONE(aopri3))
#define ARG_DSSI16_ESDI16 (isDSSI16(aopri1) && isESDI16(aopri2) && isNONE(aopri3))
#define ARG_DSSI32_ESDI32 (isDSSI32(aopri1) && isESDI32(aopri2) && isNONE(aopri3))
#define ARG_ESDI8s_DSSI8s   (isESDI8s (aopri1) && isDSSI8s (aopri2) && isNONE(aopri3))
#define ARG_ESDI16s_DSSI16s (isESDI16s(aopri1) && isDSSI16s(aopri2) && isNONE(aopri3))
#define ARG_ESDI32s_DSSI32s (isESDI32s(aopri1) && isDSSI32s(aopri2) && isNONE(aopri3))
#define ARG_DSSI8s_ESDI8s   (isDSSI8s (aopri1) && isESDI8s (aopri2) && isNONE(aopri3))
#define ARG_DSSI16s_ESDI16s (isDSSI16s(aopri1) && isESDI16s(aopri2) && isNONE(aopri3))
#define ARG_DSSI32s_ESDI32s (isDSSI32s(aopri1) && isESDI32s(aopri2) && isNONE(aopri3))
#define ARG_ESDI8s_DX         (isESDI8s (aopri1)  && isDX(aopri2) && isNONE(aopri3))
#define ARG_ESDI16s_DX        (isESDI16s(aopri1)  && isDX(aopri2) && isNONE(aopri3))
#define ARG_ESDI32s_DX        (isESDI32s(aopri1)  && isDX(aopri2) && isNONE(aopri3))
#define ARG_DX_DSSI8s         (isDX(aopri1)  && isDSSI8s (aopri2) && isNONE(aopri3))
#define ARG_DX_DSSI16s        (isDX(aopri1)  && isDSSI16s(aopri2) && isNONE(aopri3))
#define ARG_DX_DSSI32s        (isDX(aopri1)  && isDSSI32s(aopri2) && isNONE(aopri3))
#define ARG_DSSI8s            (isDSSI8s (aopri1)  && isNONE(aopri2) && isNONE(aopri3))
#define ARG_DSSI16s           (isDSSI16s(aopri1)  && isNONE(aopri2) && isNONE(aopri3))
#define ARG_DSSI32s           (isDSSI32s(aopri1)  && isNONE(aopri2) && isNONE(aopri3))
#define ARG_ESDI8s            (isESDI8s (aopri1)  && isNONE(aopri2) && isNONE(aopri3))
#define ARG_ESDI16s           (isESDI16s(aopri1)  && isNONE(aopri2) && isNONE(aopri3))
#define ARG_ESDI32s           (isESDI32s(aopri1)  && isNONE(aopri2) && isNONE(aopri3))
#define ARG_ESEDI8s_DSESI8s   (isESEDI8s (aopri1) && isDSESI8s (aopri2) && isNONE(aopri3))
#define ARG_ESEDI16s_DSESI16s (isESEDI16s(aopri1) && isDSESI16s(aopri2) && isNONE(aopri3))
#define ARG_ESEDI32s_DSESI32s (isESEDI32s(aopri1) && isDSESI32s(aopri2) && isNONE(aopri3))
#define ARG_DSESI8s_ESEDI8s   (isDSESI8s (aopri1) && isESEDI8s (aopri2) && isNONE(aopri3))
#define ARG_DSESI16s_ESEDI16s (isDSESI16s(aopri1) && isESEDI16s(aopri2) && isNONE(aopri3))
#define ARG_DSESI32s_ESEDI32s (isDSESI32s(aopri1) && isESEDI32s(aopri2) && isNONE(aopri3))
#define ARG_ESEDI8s_DX        (isESEDI8s (aopri1) && isDX(aopri2) && isNONE(aopri3))
#define ARG_ESEDI16s_DX       (isESEDI16s(aopri1) && isDX(aopri2) && isNONE(aopri3))
#define ARG_ESEDI32s_DX       (isESEDI32s(aopri1) && isDX(aopri2) && isNONE(aopri3))
#define ARG_DX_DSESI8s        (isDX(aopri1) && isDSESI8s (aopri2) && isNONE(aopri3))
#define ARG_DX_DSESI16s       (isDX(aopri1) && isDSESI16s(aopri2) && isNONE(aopri3))
#define ARG_DX_DSESI32s       (isDX(aopri1) && isDSESI32s(aopri2) && isNONE(aopri3))
#define ARG_DSESI8s           (isDSESI8s (aopri1)  && isNONE(aopri2) && isNONE(aopri3))
#define ARG_DSESI16s          (isDSESI16s(aopri1)  && isNONE(aopri2) && isNONE(aopri3))
#define ARG_DSESI32s          (isDSESI32s(aopri1)  && isNONE(aopri2) && isNONE(aopri3))
#define ARG_ESEDI8s           (isESEDI8s (aopri1)  && isNONE(aopri2) && isNONE(aopri3))
#define ARG_ESEDI16s          (isESEDI16s(aopri1)  && isNONE(aopri2) && isNONE(aopri3))
#define ARG_ESEDI32s          (isESEDI32s(aopri1)  && isNONE(aopri2) && isNONE(aopri3))
#define ARG_DSBXAL8s          (isDSBXAL8s(aopri1)  && isNONE(aopri2) && isNONE(aopri3))
#define ARG_DSEBXAL8s         (isDSEBXAL8s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_R16_RM16_I8       (isR16(aopri1)  && isRM16  (aopri2)  && isI8(aopri3))
#define ARG_R32_RM32_I8       (isR32(aopri1)  && isRM32  (aopri2)  && isI8(aopri3))
#define ARG_R16_RM16_I16      (isR16(aopri1)  && isRM16  (aopri2)  && isI16(aopri3))
#define ARG_R32_RM32_I32      (isR32(aopri1)  && isRM32  (aopri2)  && isI32(aopri3))
#define ARG_RM16_R16_I8       (isRM16(aopri1) && isR16   (aopri2)  && isI8(aopri3))
#define ARG_RM32_R32_I8       (isRM32(aopri1) && isR32   (aopri2)  && isI8(aopri3))
#define ARG_RM16_R16_CL       (isRM16(aopri1) && isR16   (aopri2)  && isCL(aopri3))
#define ARG_RM32_R32_CL       (isRM32(aopri1) && isR32   (aopri2)  && isCL(aopri3))
/* assembly compiler: lexical scanner */
typedef enum {
    STATE_START,
    STATE_BY,STATE_BYT, /* BYTE */
    STATE_W,STATE_WO,STATE_WOR, /* WORD */
    STATE_DW,STATE_DWO, /* DWORD */
    STATE_DWOR,
    STATE_P,STATE_PT,           /* PTR */
    STATE_N,STATE_NE,STATE_NEA, /* NEAR */
    STATE_EA,           /* EAX */
    STATE_EC,           /* ECX */
    STATE_ED,           /* EDX, EDI */
    STATE_EB,           /* EBX, EBP */
    STATE_ES,           /* ESP, ESI */
    STATE_FA,           /* FAR */
    STATE_SH,STATE_SHO, STATE_SHOR, /* SHORT */
    STATE_CR,           /* CRn */
    STATE_DR,           /* DRn */
    STATE_T,STATE_TR,           /* TRn */
    STATE_A,                    /* AX, AH, AL, NUM */
    STATE_B,                    /* BX, BH, BL, BP, NUM */
    STATE_C,                    /* CX, CH, CL, CS, NUM */
    STATE_D,                    /* DX, DH, DL, DS, DI, NUM, DWORD */
    STATE_E,                    /* ES, NUM */
    STATE_F,                    /* FS, NUM, FAR */
    STATE_G,                    /* GS, NUM, FAR */
    STATE_S,                    /* SS, SP, SI, SHORT */
    STATE_NUM1,                 /* NUM */
    STATE_NUM2,
    STATE_NUM3,
    STATE_NUM4,
    STATE_NUM5,
    STATE_NUM6,
    STATE_NUM7,
    STATE_NUM8
} t_aasm_scan_state;
typedef enum {
    TOKEN_NULL,TOKEN_END,
    TOKEN_LSPAREN,TOKEN_RSPAREN,
    TOKEN_COLON,TOKEN_PLUS,TOKEN_MINUS,TOKEN_TIMES,
    TOKEN_BYTE,TOKEN_WORD,TOKEN_DWORD,
    TOKEN_SHORT,TOKEN_NEAR,TOKEN_FAR,TOKEN_PTR,
    TOKEN_IMM8,TOKEN_IMM16,TOKEN_IMM32,
    TOKEN_AH,TOKEN_BH,TOKEN_CH,TOKEN_DH,
    TOKEN_AL,TOKEN_BL,TOKEN_CL,TOKEN_DL,
    TOKEN_AX,TOKEN_BX,TOKEN_CX,TOKEN_DX,
    TOKEN_SP,TOKEN_BP,TOKEN_SI,TOKEN_DI,
    TOKEN_EAX,TOKEN_EBX,TOKEN_ECX,TOKEN_EDX,
    TOKEN_ESP,TOKEN_EBP,TOKEN_ESI,TOKEN_EDI,
    TOKEN_ES,TOKEN_CS,TOKEN_SS,TOKEN_DS,TOKEN_FS,TOKEN_GS,
    TOKEN_CR0,TOKEN_CR2,TOKEN_CR3,
    TOKEN_DR0,TOKEN_DR1,TOKEN_DR2,TOKEN_DR3,TOKEN_DR6,TOKEN_DR7,
    TOKEN_TR6,TOKEN_TR7,TOKEN_DOLLAR
} t_aasm_token;
/* token variables */
#define tokch  (*tokptr)
#define take(n) (flagend = 1, token = (n))
static t_aasm_token gettoken(char *str) {
    uint8_t toklen = 0;
    uint32_t tokimm = 0;
    uint8_t flagend = 0;
    t_aasm_token token = TOKEN_NULL;
    t_aasm_scan_state state = STATE_START;
    char *tokptrbak;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("gettoken");
    tokimm8 = 0x00;
    tokimm16 = 0x0000;
    tokimm32 = 0x00000000;
    if (str) tokptr = str;
    if (!tokptr) {
        NTVDM64_TYPE_TRACE_CALL_END;
        return token;
    }
    tokptrbak = tokptr;
    do {
        switch (state) {
        case STATE_START:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_START)");
            switch (tokch) {
            case '[':
                take(TOKEN_LSPAREN);
                break;
            case ']':
                take(TOKEN_RSPAREN);
                break;
            case ':':
                take(TOKEN_COLON);
                break;
            case '+':
                take(TOKEN_PLUS);
                break;
            case '-':
                take(TOKEN_MINUS);
                break;
            case '*':
                take(TOKEN_TIMES);
                break;
            case '$':
                take(TOKEN_DOLLAR);
                break;
            case '0':
                tokimm = 0x0;
                toklen = 1;
                state = STATE_NUM1;
                break;
            case '1':
                tokimm = 0x1;
                toklen = 1;
                state = STATE_NUM1;
                break;
            case '2':
                tokimm = 0x2;
                toklen = 1;
                state = STATE_NUM1;
                break;
            case '3':
                tokimm = 0x3;
                toklen = 1;
                state = STATE_NUM1;
                break;
            case '4':
                tokimm = 0x4;
                toklen = 1;
                state = STATE_NUM1;
                break;
            case '5':
                tokimm = 0x5;
                toklen = 1;
                state = STATE_NUM1;
                break;
            case '6':
                tokimm = 0x6;
                toklen = 1;
                state = STATE_NUM1;
                break;
            case '7':
                tokimm = 0x7;
                toklen = 1;
                state = STATE_NUM1;
                break;
            case '8':
                tokimm = 0x8;
                toklen = 1;
                state = STATE_NUM1;
                break;
            case '9':
                tokimm = 0x9;
                toklen = 1;
                state = STATE_NUM1;
                break;
            case 'a':
                tokimm = 0xa;
                toklen = 1;
                state = STATE_A;
                break;
            case 'b':
                tokimm = 0xb;
                toklen = 1;
                state = STATE_B;
                break;
            case 'c':
                tokimm = 0xc;
                toklen = 1;
                state = STATE_C;
                break;
            case 'd':
                tokimm = 0xd;
                toklen = 1;
                state = STATE_D;
                break;
            case 'e':
                tokimm = 0xe;
                toklen = 1;
                state = STATE_E;
                break;
            case 'f':
                tokimm = 0xf;
                toklen = 1;
                state = STATE_F;
                break;
            case 'g':
                tokimm = 0xf;
                toklen = 1;
                state = STATE_G;
                break;
            case 'n':
                state = STATE_N;
                break;
            case 'p':
                state = STATE_P;
                break;
            case 's':
                state = STATE_S;
                break;
            case 't':
                state = STATE_T;
                break;
            case 'w':
                state = STATE_W;
                break;
            case ' ':
            case '\t':
                break;
            case '\0':
                tokptr--;
                take(TOKEN_END);
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_NUM1:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_NUM1)");
            switch (tokch) {
            case '0':
                tokimm = (tokimm << 4) | 0x0;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '1':
                tokimm = (tokimm << 4) | 0x1;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '2':
                tokimm = (tokimm << 4) | 0x2;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '3':
                tokimm = (tokimm << 4) | 0x3;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '4':
                tokimm = (tokimm << 4) | 0x4;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '5':
                tokimm = (tokimm << 4) | 0x5;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '6':
                tokimm = (tokimm << 4) | 0x6;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '7':
                tokimm = (tokimm << 4) | 0x7;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '8':
                tokimm = (tokimm << 4) | 0x8;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '9':
                tokimm = (tokimm << 4) | 0x9;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'a':
                tokimm = (tokimm << 4) | 0xa;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'b':
                tokimm = (tokimm << 4) | 0xb;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'c':
                tokimm = (tokimm << 4) | 0xc;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'd':
                tokimm = (tokimm << 4) | 0xd;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'e':
                tokimm = (tokimm << 4) | 0xe;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'f':
                tokimm = (tokimm << 4) | 0xf;
                toklen = 2;
                state = STATE_NUM2;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_NUM2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_NUM2)");
            switch (tokch) {
            case '0':
                tokimm = (tokimm << 4) | 0x0;
                toklen = 3;
                state = STATE_NUM3;
                break;
            case '1':
                tokimm = (tokimm << 4) | 0x1;
                toklen = 3;
                state = STATE_NUM3;
                break;
            case '2':
                tokimm = (tokimm << 4) | 0x2;
                toklen = 3;
                state = STATE_NUM3;
                break;
            case '3':
                tokimm = (tokimm << 4) | 0x3;
                toklen = 3;
                state = STATE_NUM3;
                break;
            case '4':
                tokimm = (tokimm << 4) | 0x4;
                toklen = 3;
                state = STATE_NUM3;
                break;
            case '5':
                tokimm = (tokimm << 4) | 0x5;
                toklen = 3;
                state = STATE_NUM3;
                break;
            case '6':
                tokimm = (tokimm << 4) | 0x6;
                toklen = 3;
                state = STATE_NUM3;
                break;
            case '7':
                tokimm = (tokimm << 4) | 0x7;
                toklen = 3;
                state = STATE_NUM3;
                break;
            case '8':
                tokimm = (tokimm << 4) | 0x8;
                toklen = 3;
                state = STATE_NUM3;
                break;
            case '9':
                tokimm = (tokimm << 4) | 0x9;
                toklen = 3;
                state = STATE_NUM3;
                break;
            case 'a':
                tokimm = (tokimm << 4) | 0xa;
                toklen = 3;
                state = STATE_NUM3;
                break;
            case 'b':
                tokimm = (tokimm << 4) | 0xb;
                toklen = 3;
                state = STATE_NUM3;
                break;
            case 'c':
                tokimm = (tokimm << 4) | 0xc;
                toklen = 3;
                state = STATE_NUM3;
                break;
            case 'd':
                tokimm = (tokimm << 4) | 0xd;
                toklen = 3;
                state = STATE_NUM3;
                break;
            case 'e':
                tokimm = (tokimm << 4) | 0xe;
                toklen = 3;
                state = STATE_NUM3;
                break;
            case 'f':
                tokimm = (tokimm << 4) | 0xf;
                toklen = 3;
                state = STATE_NUM3;
                break;
            default:
                tokptr--;
                tokimm8 = (uint8_t) tokimm;
                take(TOKEN_IMM8);
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_NUM3:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_NUM3)");
            switch (tokch) {
            case '0':
                tokimm = (tokimm << 4) | 0x0;
                toklen = 4;
                state = STATE_NUM4;
                break;
            case '1':
                tokimm = (tokimm << 4) | 0x1;
                toklen = 4;
                state = STATE_NUM4;
                break;
            case '2':
                tokimm = (tokimm << 4) | 0x2;
                toklen = 4;
                state = STATE_NUM4;
                break;
            case '3':
                tokimm = (tokimm << 4) | 0x3;
                toklen = 4;
                state = STATE_NUM4;
                break;
            case '4':
                tokimm = (tokimm << 4) | 0x4;
                toklen = 4;
                state = STATE_NUM4;
                break;
            case '5':
                tokimm = (tokimm << 4) | 0x5;
                toklen = 4;
                state = STATE_NUM4;
                break;
            case '6':
                tokimm = (tokimm << 4) | 0x6;
                toklen = 4;
                state = STATE_NUM4;
                break;
            case '7':
                tokimm = (tokimm << 4) | 0x7;
                toklen = 4;
                state = STATE_NUM4;
                break;
            case '8':
                tokimm = (tokimm << 4) | 0x8;
                toklen = 4;
                state = STATE_NUM4;
                break;
            case '9':
                tokimm = (tokimm << 4) | 0x9;
                toklen = 4;
                state = STATE_NUM4;
                break;
            case 'a':
                tokimm = (tokimm << 4) | 0xa;
                toklen = 4;
                state = STATE_NUM4;
                break;
            case 'b':
                tokimm = (tokimm << 4) | 0xb;
                toklen = 4;
                state = STATE_NUM4;
                break;
            case 'c':
                tokimm = (tokimm << 4) | 0xc;
                toklen = 4;
                state = STATE_NUM4;
                break;
            case 'd':
                tokimm = (tokimm << 4) | 0xd;
                toklen = 4;
                state = STATE_NUM4;
                break;
            case 'e':
                tokimm = (tokimm << 4) | 0xe;
                toklen = 4;
                state = STATE_NUM4;
                break;
            case 'f':
                tokimm = (tokimm << 4) | 0xf;
                toklen = 4;
                state = STATE_NUM4;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_NUM4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_NUM4)");
            switch (tokch) {
            case '0':
                tokimm = (tokimm << 4) | 0x0;
                toklen = 5;
                state = STATE_NUM5;
                break;
            case '1':
                tokimm = (tokimm << 4) | 0x1;
                toklen = 5;
                state = STATE_NUM5;
                break;
            case '2':
                tokimm = (tokimm << 4) | 0x2;
                toklen = 5;
                state = STATE_NUM5;
                break;
            case '3':
                tokimm = (tokimm << 4) | 0x3;
                toklen = 5;
                state = STATE_NUM5;
                break;
            case '4':
                tokimm = (tokimm << 4) | 0x4;
                toklen = 5;
                state = STATE_NUM5;
                break;
            case '5':
                tokimm = (tokimm << 4) | 0x5;
                toklen = 5;
                state = STATE_NUM5;
                break;
            case '6':
                tokimm = (tokimm << 4) | 0x6;
                toklen = 5;
                state = STATE_NUM5;
                break;
            case '7':
                tokimm = (tokimm << 4) | 0x7;
                toklen = 5;
                state = STATE_NUM5;
                break;
            case '8':
                tokimm = (tokimm << 4) | 0x8;
                toklen = 5;
                state = STATE_NUM5;
                break;
            case '9':
                tokimm = (tokimm << 4) | 0x9;
                toklen = 5;
                state = STATE_NUM5;
                break;
            case 'a':
                tokimm = (tokimm << 4) | 0xa;
                toklen = 5;
                state = STATE_NUM5;
                break;
            case 'b':
                tokimm = (tokimm << 4) | 0xb;
                toklen = 5;
                state = STATE_NUM5;
                break;
            case 'c':
                tokimm = (tokimm << 4) | 0xc;
                toklen = 5;
                state = STATE_NUM5;
                break;
            case 'd':
                tokimm = (tokimm << 4) | 0xd;
                toklen = 5;
                state = STATE_NUM5;
                break;
            case 'e':
                tokimm = (tokimm << 4) | 0xe;
                toklen = 5;
                state = STATE_NUM5;
                break;
            case 'f':
                tokimm = (tokimm << 4) | 0xf;
                toklen = 5;
                state = STATE_NUM5;
                break;
            default:
                tokptr--;
                tokimm16 = (uint16_t) tokimm;
                take(TOKEN_IMM16);
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_NUM5:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_NUM5)");
            switch (tokch) {
            case '0':
                tokimm = (tokimm << 4) | 0x0;
                toklen = 6;
                state = STATE_NUM6;
                break;
            case '1':
                tokimm = (tokimm << 4) | 0x1;
                toklen = 6;
                state = STATE_NUM6;
                break;
            case '2':
                tokimm = (tokimm << 4) | 0x2;
                toklen = 6;
                state = STATE_NUM6;
                break;
            case '3':
                tokimm = (tokimm << 4) | 0x3;
                toklen = 6;
                state = STATE_NUM6;
                break;
            case '4':
                tokimm = (tokimm << 4) | 0x4;
                toklen = 6;
                state = STATE_NUM6;
                break;
            case '5':
                tokimm = (tokimm << 4) | 0x5;
                toklen = 6;
                state = STATE_NUM6;
                break;
            case '6':
                tokimm = (tokimm << 4) | 0x6;
                toklen = 6;
                state = STATE_NUM6;
                break;
            case '7':
                tokimm = (tokimm << 4) | 0x7;
                toklen = 6;
                state = STATE_NUM6;
                break;
            case '8':
                tokimm = (tokimm << 4) | 0x8;
                toklen = 6;
                state = STATE_NUM6;
                break;
            case '9':
                tokimm = (tokimm << 4) | 0x9;
                toklen = 6;
                state = STATE_NUM6;
                break;
            case 'a':
                tokimm = (tokimm << 4) | 0xa;
                toklen = 6;
                state = STATE_NUM6;
                break;
            case 'b':
                tokimm = (tokimm << 4) | 0xb;
                toklen = 6;
                state = STATE_NUM6;
                break;
            case 'c':
                tokimm = (tokimm << 4) | 0xc;
                toklen = 6;
                state = STATE_NUM6;
                break;
            case 'd':
                tokimm = (tokimm << 4) | 0xd;
                toklen = 6;
                state = STATE_NUM6;
                break;
            case 'e':
                tokimm = (tokimm << 4) | 0xe;
                toklen = 6;
                state = STATE_NUM6;
                break;
            case 'f':
                tokimm = (tokimm << 4) | 0xf;
                toklen = 6;
                state = STATE_NUM6;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_NUM6:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_NUM6)");
            switch (tokch) {
            case '0':
                tokimm = (tokimm << 4) | 0x0;
                toklen = 7;
                state = STATE_NUM7;
                break;
            case '1':
                tokimm = (tokimm << 4) | 0x1;
                toklen = 7;
                state = STATE_NUM7;
                break;
            case '2':
                tokimm = (tokimm << 4) | 0x2;
                toklen = 7;
                state = STATE_NUM7;
                break;
            case '3':
                tokimm = (tokimm << 4) | 0x3;
                toklen = 7;
                state = STATE_NUM7;
                break;
            case '4':
                tokimm = (tokimm << 4) | 0x4;
                toklen = 7;
                state = STATE_NUM7;
                break;
            case '5':
                tokimm = (tokimm << 4) | 0x5;
                toklen = 7;
                state = STATE_NUM7;
                break;
            case '6':
                tokimm = (tokimm << 4) | 0x6;
                toklen = 7;
                state = STATE_NUM7;
                break;
            case '7':
                tokimm = (tokimm << 4) | 0x7;
                toklen = 7;
                state = STATE_NUM7;
                break;
            case '8':
                tokimm = (tokimm << 4) | 0x8;
                toklen = 7;
                state = STATE_NUM7;
                break;
            case '9':
                tokimm = (tokimm << 4) | 0x9;
                toklen = 7;
                state = STATE_NUM7;
                break;
            case 'a':
                tokimm = (tokimm << 4) | 0xa;
                toklen = 7;
                state = STATE_NUM7;
                break;
            case 'b':
                tokimm = (tokimm << 4) | 0xb;
                toklen = 7;
                state = STATE_NUM7;
                break;
            case 'c':
                tokimm = (tokimm << 4) | 0xc;
                toklen = 7;
                state = STATE_NUM7;
                break;
            case 'd':
                tokimm = (tokimm << 4) | 0xd;
                toklen = 7;
                state = STATE_NUM7;
                break;
            case 'e':
                tokimm = (tokimm << 4) | 0xe;
                toklen = 7;
                state = STATE_NUM7;
                break;
            case 'f':
                tokimm = (tokimm << 4) | 0xf;
                toklen = 7;
                state = STATE_NUM7;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_NUM7:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_NUM7)");
            switch (tokch) {
            case '0':
                tokimm = (tokimm << 4) | 0x0;
                toklen = 8;
                state = STATE_NUM8;
                break;
            case '1':
                tokimm = (tokimm << 4) | 0x1;
                toklen = 8;
                state = STATE_NUM8;
                break;
            case '2':
                tokimm = (tokimm << 4) | 0x2;
                toklen = 8;
                state = STATE_NUM8;
                break;
            case '3':
                tokimm = (tokimm << 4) | 0x3;
                toklen = 8;
                state = STATE_NUM8;
                break;
            case '4':
                tokimm = (tokimm << 4) | 0x4;
                toklen = 8;
                state = STATE_NUM8;
                break;
            case '5':
                tokimm = (tokimm << 4) | 0x5;
                toklen = 8;
                state = STATE_NUM8;
                break;
            case '6':
                tokimm = (tokimm << 4) | 0x6;
                toklen = 8;
                state = STATE_NUM8;
                break;
            case '7':
                tokimm = (tokimm << 4) | 0x7;
                toklen = 8;
                state = STATE_NUM8;
                break;
            case '8':
                tokimm = (tokimm << 4) | 0x8;
                toklen = 8;
                state = STATE_NUM8;
                break;
            case '9':
                tokimm = (tokimm << 4) | 0x9;
                toklen = 8;
                state = STATE_NUM8;
                break;
            case 'a':
                tokimm = (tokimm << 4) | 0xa;
                toklen = 8;
                state = STATE_NUM8;
                break;
            case 'b':
                tokimm = (tokimm << 4) | 0xb;
                toklen = 8;
                state = STATE_NUM8;
                break;
            case 'c':
                tokimm = (tokimm << 4) | 0xc;
                toklen = 8;
                state = STATE_NUM8;
                break;
            case 'd':
                tokimm = (tokimm << 4) | 0xd;
                toklen = 8;
                state = STATE_NUM8;
                break;
            case 'e':
                tokimm = (tokimm << 4) | 0xe;
                toklen = 8;
                state = STATE_NUM8;
                break;
            case 'f':
                tokimm = (tokimm << 4) | 0xf;
                toklen = 8;
                state = STATE_NUM8;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_NUM8:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_NUM8)");
            switch (tokch) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
                tokptr--;
                _sert_;
                break;
                break;
            default:
                tokptr--;
                tokimm32 = (uint32_t) tokimm;
                take(TOKEN_IMM32);
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_A:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_A)");
            switch (tokch) {
            case '0':
                tokimm = (tokimm << 4) | 0x0;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '1':
                tokimm = (tokimm << 4) | 0x1;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '2':
                tokimm = (tokimm << 4) | 0x2;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '3':
                tokimm = (tokimm << 4) | 0x3;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '4':
                tokimm = (tokimm << 4) | 0x4;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '5':
                tokimm = (tokimm << 4) | 0x5;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '6':
                tokimm = (tokimm << 4) | 0x6;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '7':
                tokimm = (tokimm << 4) | 0x7;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '8':
                tokimm = (tokimm << 4) | 0x8;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '9':
                tokimm = (tokimm << 4) | 0x9;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'a':
                tokimm = (tokimm << 4) | 0xa;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'b':
                tokimm = (tokimm << 4) | 0xb;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'c':
                tokimm = (tokimm << 4) | 0xc;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'd':
                tokimm = (tokimm << 4) | 0xd;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'e':
                tokimm = (tokimm << 4) | 0xe;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'f':
                tokimm = (tokimm << 4) | 0xf;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'x':
                take(TOKEN_AX);
                break;
            case 'h':
                take(TOKEN_AH);
                break;
            case 'l':
                take(TOKEN_AL);
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_B:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_B)");
            switch (tokch) {
            case '0':
                tokimm = (tokimm << 4) | 0x0;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '1':
                tokimm = (tokimm << 4) | 0x1;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '2':
                tokimm = (tokimm << 4) | 0x2;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '3':
                tokimm = (tokimm << 4) | 0x3;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '4':
                tokimm = (tokimm << 4) | 0x4;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '5':
                tokimm = (tokimm << 4) | 0x5;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '6':
                tokimm = (tokimm << 4) | 0x6;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '7':
                tokimm = (tokimm << 4) | 0x7;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '8':
                tokimm = (tokimm << 4) | 0x8;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '9':
                tokimm = (tokimm << 4) | 0x9;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'a':
                tokimm = (tokimm << 4) | 0xa;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'b':
                tokimm = (tokimm << 4) | 0xb;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'c':
                tokimm = (tokimm << 4) | 0xc;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'd':
                tokimm = (tokimm << 4) | 0xd;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'e':
                tokimm = (tokimm << 4) | 0xe;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'f':
                tokimm = (tokimm << 4) | 0xf;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'x':
                take(TOKEN_BX);
                break;
            case 'h':
                take(TOKEN_BH);
                break;
            case 'l':
                take(TOKEN_BL);
                break;
            case 'p':
                take(TOKEN_BP);
                break;
            case 'y':
                state = STATE_BY;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_C:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_C)");
            switch (tokch) {
            case '0':
                tokimm = (tokimm << 4) | 0x0;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '1':
                tokimm = (tokimm << 4) | 0x1;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '2':
                tokimm = (tokimm << 4) | 0x2;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '3':
                tokimm = (tokimm << 4) | 0x3;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '4':
                tokimm = (tokimm << 4) | 0x4;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '5':
                tokimm = (tokimm << 4) | 0x5;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '6':
                tokimm = (tokimm << 4) | 0x6;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '7':
                tokimm = (tokimm << 4) | 0x7;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '8':
                tokimm = (tokimm << 4) | 0x8;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '9':
                tokimm = (tokimm << 4) | 0x9;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'a':
                tokimm = (tokimm << 4) | 0xa;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'b':
                tokimm = (tokimm << 4) | 0xb;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'c':
                tokimm = (tokimm << 4) | 0xc;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'd':
                tokimm = (tokimm << 4) | 0xd;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'e':
                tokimm = (tokimm << 4) | 0xe;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'f':
                tokimm = (tokimm << 4) | 0xf;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'x':
                take(TOKEN_CX);
                break;
            case 'h':
                take(TOKEN_CH);
                break;
            case 'l':
                take(TOKEN_CL);
                break;
            case 'r':
                state = STATE_CR;
                break;
            case 's':
                take(TOKEN_CS);
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_D:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_D)");
            switch (tokch) {
            case '0':
                tokimm = (tokimm << 4) | 0x0;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '1':
                tokimm = (tokimm << 4) | 0x1;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '2':
                tokimm = (tokimm << 4) | 0x2;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '3':
                tokimm = (tokimm << 4) | 0x3;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '4':
                tokimm = (tokimm << 4) | 0x4;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '5':
                tokimm = (tokimm << 4) | 0x5;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '6':
                tokimm = (tokimm << 4) | 0x6;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '7':
                tokimm = (tokimm << 4) | 0x7;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '8':
                tokimm = (tokimm << 4) | 0x8;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '9':
                tokimm = (tokimm << 4) | 0x9;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'a':
                tokimm = (tokimm << 4) | 0xa;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'b':
                tokimm = (tokimm << 4) | 0xb;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'c':
                tokimm = (tokimm << 4) | 0xc;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'd':
                tokimm = (tokimm << 4) | 0xd;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'e':
                tokimm = (tokimm << 4) | 0xe;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'f':
                tokimm = (tokimm << 4) | 0xf;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'x':
                take(TOKEN_DX);
                break;
            case 'h':
                take(TOKEN_DH);
                break;
            case 'l':
                take(TOKEN_DL);
                break;
            case 'r':
                state = STATE_DR;
                break;
            case 's':
                take(TOKEN_DS);
                break;
            case 'i':
                take(TOKEN_DI);
                break;
            case 'w':
                state = STATE_DW;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_E:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_E)");
            switch (tokch) {
            case '0':
                tokimm = (tokimm << 4) | 0x0;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '1':
                tokimm = (tokimm << 4) | 0x1;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '2':
                tokimm = (tokimm << 4) | 0x2;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '3':
                tokimm = (tokimm << 4) | 0x3;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '4':
                tokimm = (tokimm << 4) | 0x4;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '5':
                tokimm = (tokimm << 4) | 0x5;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '6':
                tokimm = (tokimm << 4) | 0x6;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '7':
                tokimm = (tokimm << 4) | 0x7;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '8':
                tokimm = (tokimm << 4) | 0x8;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '9':
                tokimm = (tokimm << 4) | 0x9;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'a':
                tokimm = (tokimm << 4) | 0xa;
                toklen = 2;
                state = STATE_EA;
                break;
            case 'b':
                tokimm = (tokimm << 4) | 0xb;
                toklen = 2;
                state = STATE_EB;
                break;
            case 'c':
                tokimm = (tokimm << 4) | 0xc;
                toklen = 2;
                state = STATE_EC;
                break;
            case 'd':
                tokimm = (tokimm << 4) | 0xd;
                toklen = 2;
                state = STATE_ED;
                break;
            case 'e':
                tokimm = (tokimm << 4) | 0xe;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'f':
                tokimm = (tokimm << 4) | 0xf;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 's':
                state = STATE_ES;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_F:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_F)");
            switch (tokch) {
            case '0':
                tokimm = (tokimm << 4) | 0x0;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '1':
                tokimm = (tokimm << 4) | 0x1;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '2':
                tokimm = (tokimm << 4) | 0x2;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '3':
                tokimm = (tokimm << 4) | 0x3;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '4':
                tokimm = (tokimm << 4) | 0x4;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '5':
                tokimm = (tokimm << 4) | 0x5;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '6':
                tokimm = (tokimm << 4) | 0x6;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '7':
                tokimm = (tokimm << 4) | 0x7;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '8':
                tokimm = (tokimm << 4) | 0x8;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case '9':
                tokimm = (tokimm << 4) | 0x9;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'a':
                tokimm = (tokimm << 4) | 0xa;
                toklen = 2;
                state = STATE_FA;
                break;
            case 'b':
                tokimm = (tokimm << 4) | 0xb;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'c':
                tokimm = (tokimm << 4) | 0xc;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'd':
                tokimm = (tokimm << 4) | 0xd;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'e':
                tokimm = (tokimm << 4) | 0xe;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 'f':
                tokimm = (tokimm << 4) | 0xf;
                toklen = 2;
                state = STATE_NUM2;
                break;
            case 's':
                take(TOKEN_FS);
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_G:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_G)");
            switch (tokch) {
            case 's':
                take(TOKEN_GS);
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_N:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_N)");
            switch (tokch) {
            case 'e':
                state = STATE_NE;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_P:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_P)");
            switch (tokch) {
            case 't':
                state = STATE_PT;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_S:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_S)");
            switch (tokch) {
            case 'i':
                take(TOKEN_SI);
                break;
            case 'p':
                take(TOKEN_SP);
                break;
            case 's':
                take(TOKEN_SS);
                break;
            case 'h':
                state = STATE_SH;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_T:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_T)");
            switch (tokch) {
            case 'r':
                state = STATE_TR;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_W:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_W)");
            switch (tokch) {
            case 'o':
                state = STATE_WO;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_BY:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_BY)");
            switch (tokch) {
            case 't':
                state = STATE_BYT;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_CR:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_CR)");
            switch (tokch) {
            case '0':
                take(TOKEN_CR0);
                break;
            case '2':
                take(TOKEN_CR2);
                break;
            case '3':
                take(TOKEN_CR3);
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_DR:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_DR)");
            switch (tokch) {
            case '0':
                take(TOKEN_DR0);
                break;
            case '1':
                take(TOKEN_DR1);
                break;
            case '2':
                take(TOKEN_DR2);
                break;
            case '3':
                take(TOKEN_DR3);
                break;
            case '6':
                take(TOKEN_DR6);
                break;
            case '7':
                take(TOKEN_DR7);
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_DW:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_DW)");
            switch (tokch) {
            case 'o':
                state = STATE_DWO;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_EA:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_EA)");
            switch (tokch) {
            case 'x':
                take(TOKEN_EAX);
                break;
            default:
                tokptr--;
                tokimm8 = (uint8_t) tokimm;
                take(TOKEN_IMM8);
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_EB:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_EB)");
            switch (tokch) {
            case 'p':
                take(TOKEN_EBP);
                break;
            case 'x':
                take(TOKEN_EBX);
                break;
            default:
                tokptr--;
                tokimm8 = (uint8_t) tokimm;
                take(TOKEN_IMM8);
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_EC:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_EC)");
            switch (tokch) {
            case 'x':
                take(TOKEN_ECX);
                break;
            default:
                tokptr--;
                tokimm8 = (uint8_t) tokimm;
                take(TOKEN_IMM8);
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_ED:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_ED)");
            switch (tokch) {
            case 'i':
                take(TOKEN_EDI);
                break;
            case 'x':
                take(TOKEN_EDX);
                break;
            default:
                tokptr--;
                tokimm8 = (uint8_t) tokimm;
                take(TOKEN_IMM8);
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_ES:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_ES)");
            switch (tokch) {
            case 'i':
                take(TOKEN_ESI);
                break;
            case 'p':
                take(TOKEN_ESP);
                break;
            default:
                tokptr--;
                take(TOKEN_ES);
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_FA:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_FA)");
            switch (tokch) {
            case '0':
                tokimm = (tokimm << 4) | 0x0;
                toklen = 3;
                state = STATE_NUM3;
                break;
            case '1':
                tokimm = (tokimm << 4) | 0x1;
                toklen = 3;
                state = STATE_NUM3;
                break;
            case '2':
                tokimm = (tokimm << 4) | 0x2;
                toklen = 3;
                state = STATE_NUM3;
                break;
            case '3':
                tokimm = (tokimm << 4) | 0x3;
                toklen = 3;
                state = STATE_NUM3;
                break;
            case '4':
                tokimm = (tokimm << 4) | 0x4;
                toklen = 3;
                state = STATE_NUM3;
                break;
            case '5':
                tokimm = (tokimm << 4) | 0x5;
                toklen = 3;
                state = STATE_NUM3;
                break;
            case '6':
                tokimm = (tokimm << 4) | 0x6;
                toklen = 3;
                state = STATE_NUM3;
                break;
            case '7':
                tokimm = (tokimm << 4) | 0x7;
                toklen = 3;
                state = STATE_NUM3;
                break;
            case '8':
                tokimm = (tokimm << 4) | 0x8;
                toklen = 3;
                state = STATE_NUM3;
                break;
            case '9':
                tokimm = (tokimm << 4) | 0x9;
                toklen = 3;
                state = STATE_NUM3;
                break;
            case 'a':
                tokimm = (tokimm << 4) | 0xa;
                toklen = 3;
                state = STATE_NUM3;
                break;
            case 'b':
                tokimm = (tokimm << 4) | 0xb;
                toklen = 3;
                state = STATE_NUM3;
                break;
            case 'c':
                tokimm = (tokimm << 4) | 0xc;
                toklen = 3;
                state = STATE_NUM3;
                break;
            case 'd':
                tokimm = (tokimm << 4) | 0xd;
                toklen = 3;
                state = STATE_NUM3;
                break;
            case 'e':
                tokimm = (tokimm << 4) | 0xe;
                toklen = 3;
                state = STATE_NUM3;
                break;
            case 'f':
                tokimm = (tokimm << 4) | 0xf;
                toklen = 3;
                state = STATE_NUM3;
                break;
            case 'r':
                take(TOKEN_FAR);
                break;
            default:
                tokptr--;
                tokimm8 = (uint8_t) tokimm;
                take(TOKEN_IMM8);
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_NE:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_NE)");
            switch (tokch) {
            case 'a':
                state = STATE_NEA;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_PT:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_PT)");
            switch (tokch) {
            case 'r':
                take(TOKEN_PTR);
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_SH:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_SH)");
            switch (tokch) {
            case 'o':
                state = STATE_SHO;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_TR:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_TR)");
            switch (tokch) {
            case '6':
                take(TOKEN_TR6);
                break;
            case '7':
                take(TOKEN_TR7);
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_WO:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_WO)");
            switch (tokch) {
            case 'r':
                state = STATE_WOR;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_BYT:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_BYT)");
            switch (tokch) {
            case 'e':
                take(TOKEN_BYTE);
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_DWO:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_DWO)");
            switch (tokch) {
            case 'r':
                state = STATE_DWOR;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_NEA:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_NEA)");
            switch (tokch) {
            case 'r':
                take(TOKEN_NEAR);
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_SHO:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_SHO)");
            switch (tokch) {
            case 'r':
                state = STATE_SHOR;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_WOR:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_WOR)");
            switch (tokch) {
            case 'd':
                take(TOKEN_WORD);
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_DWOR:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_DWOR)");
            switch (tokch) {
            case 'd':
                take(TOKEN_DWORD);
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case STATE_SHOR:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(STATE_SHOR)");
            switch (tokch) {
            case 't':
                take(TOKEN_SHORT);
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("state(default)");
            tokptr--;
            _sert_;
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        }
        tokptr++;
    } while (!flagend);
    NTVDM64_TYPE_TRACE_CALL_END;
    return token;
}
static void printtoken(t_aasm_token token) {
    switch (token) {
    case TOKEN_NULL:
        STD_PRINTF(" NULL ");
        break;
    case TOKEN_END:
        STD_PRINTF(" END ");
        break;
    case TOKEN_LSPAREN:
        STD_PRINTF(" [[ ");
        break;
    case TOKEN_RSPAREN:
        STD_PRINTF(" ]] ");
        break;
    case TOKEN_COLON:
        STD_PRINTF(" :: ");
        break;
    case TOKEN_PLUS:
        STD_PRINTF(" ++ ");
        break;
    case TOKEN_MINUS:
        STD_PRINTF(" -- ");
        break;
    case TOKEN_TIMES:
        STD_PRINTF(" ** ");
        break;
    case TOKEN_BYTE:
        STD_PRINTF(" BYTE ");
        break;
    case TOKEN_WORD:
        STD_PRINTF(" WORD ");
        break;
    case TOKEN_DWORD:
        STD_PRINTF(" DWORD ");
        break;
    case TOKEN_PTR:
        STD_PRINTF(" PTR ");
        break;
    case TOKEN_NEAR:
        STD_PRINTF(" NEAR ");
        break;
    case TOKEN_FAR:
        STD_PRINTF(" FAR ");
        break;
    case TOKEN_SHORT:
        STD_PRINTF(" SHORT ");
        break;
    case TOKEN_IMM8:
        STD_PRINTF(" I8(%02X) ",  tokimm8);
        break;
    case TOKEN_IMM16:
        STD_PRINTF(" I16(%04X) ", tokimm16);
        break;
    case TOKEN_IMM32:
        STD_PRINTF(" I32(%08X) ", tokimm32);
        break;
    case TOKEN_AH:
        STD_PRINTF(" AH ");
        break;
    case TOKEN_BH:
        STD_PRINTF(" BH ");
        break;
    case TOKEN_CH:
        STD_PRINTF(" CH ");
        break;
    case TOKEN_DH:
        STD_PRINTF(" DH ");
        break;
    case TOKEN_AL:
        STD_PRINTF(" AL ");
        break;
    case TOKEN_BL:
        STD_PRINTF(" BL ");
        break;
    case TOKEN_CL:
        STD_PRINTF(" CL ");
        break;
    case TOKEN_DL:
        STD_PRINTF(" DL ");
        break;
    case TOKEN_AX:
        STD_PRINTF(" AX ");
        break;
    case TOKEN_BX:
        STD_PRINTF(" BX ");
        break;
    case TOKEN_CX:
        STD_PRINTF(" CX ");
        break;
    case TOKEN_DX:
        STD_PRINTF(" DX ");
        break;
    case TOKEN_SP:
        STD_PRINTF(" SP ");
        break;
    case TOKEN_BP:
        STD_PRINTF(" BP ");
        break;
    case TOKEN_SI:
        STD_PRINTF(" SI ");
        break;
    case TOKEN_DI:
        STD_PRINTF(" DI ");
        break;
    case TOKEN_ES:
        STD_PRINTF(" ES ");
        break;
    case TOKEN_CS:
        STD_PRINTF(" CS ");
        break;
    case TOKEN_SS:
        STD_PRINTF(" SS ");
        break;
    case TOKEN_DS:
        STD_PRINTF(" DS ");
        break;
    case TOKEN_FS:
        STD_PRINTF(" FS ");
        break;
    case TOKEN_GS:
        STD_PRINTF(" GS ");
        break;
    case TOKEN_EAX:
        STD_PRINTF(" EAX ");
        break;
    case TOKEN_EBX:
        STD_PRINTF(" EBX ");
        break;
    case TOKEN_ECX:
        STD_PRINTF(" ECX ");
        break;
    case TOKEN_EDX:
        STD_PRINTF(" EDX ");
        break;
    case TOKEN_ESP:
        STD_PRINTF(" ESP ");
        break;
    case TOKEN_EBP:
        STD_PRINTF(" EBP ");
        break;
    case TOKEN_ESI:
        STD_PRINTF(" ESI ");
        break;
    case TOKEN_EDI:
        STD_PRINTF(" EDI ");
        break;
    case TOKEN_CR0:
        STD_PRINTF(" CR0 ");
        break;
    case TOKEN_CR2:
        STD_PRINTF(" CR2 ");
        break;
    case TOKEN_CR3:
        STD_PRINTF(" CR3 ");
        break;
    case TOKEN_DR0:
        STD_PRINTF(" DR0 ");
        break;
    case TOKEN_DR1:
        STD_PRINTF(" DR1 ");
        break;
    case TOKEN_DR2:
        STD_PRINTF(" DR2 ");
        break;
    case TOKEN_DR3:
        STD_PRINTF(" DR3 ");
        break;
    case TOKEN_DR6:
        STD_PRINTF(" DR6 ");
        break;
    case TOKEN_DR7:
        STD_PRINTF(" DR7 ");
        break;
    case TOKEN_TR6:
        STD_PRINTF(" TR6 ");
        break;
    case TOKEN_TR7:
        STD_PRINTF(" TR7 ");
        break;
    default:
        STD_PRINTF(" <ERROR> ");
        break;
        break;
    }
}
static void matchtoken(t_aasm_token token) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("matchtoken");
    if (gettoken(NULL) != token) _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}

/* assembly compiler: parser / grammar */
static t_aasm_oprinfo parsearg_mem(t_aasm_token token) {
    t_aasm_oprinfo info;
    uint8_t oldtoken;
    uint8_t bx,bp,si,di,neg,al;
    uint8_t eax,ecx,edx,ebx,esp,ebp,esi,edi;
    uint8_t ieax,iecx,iedx,iebx,iebp,iesi,iedi;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("parsearg_mem");
    STD_MEMSET((void *)(&info), 0x00, sizeof(t_aasm_oprinfo));
    bx = bp = si = di = neg = al = 0;
    eax = ecx = edx = ebx = esp = ebp = esi = edi = 0;
    ieax = iecx = iedx = iebx = iebp = iesi = iedi = 0;
    info.type = TYPE_M;
    info.mod = MOD_M;
    info.sib.base = R32_EBP;  /* EBP for NULL Base */
    info.sib.index = R32_ESP; /* ESP for NULL Index */
    info.sib.scale = 0;
    oldtoken = token;
    _chrf(token = gettoken(NULL));
    if (token == TOKEN_COLON) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_COLON");
        switch (oldtoken) {
        case TOKEN_ES:
            info.flages = 1;
            break;
        case TOKEN_CS:
            info.flagcs = 1;
            break;
        case TOKEN_SS:
            info.flagss = 1;
            break;
        case TOKEN_DS:
            info.flagds = 1;
            break;
        case TOKEN_FS:
            info.flagfs = 1;
            break;
        case TOKEN_GS:
            info.flaggs = 1;
            break;
        default:
            _serf_;
            break;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else if (token == TOKEN_NULL || token == TOKEN_END) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_NULL/TOKEN_END)");
        switch (oldtoken) {
        case TOKEN_ES:
            info.type = TYPE_SREG;
            info.mod =  MOD_R;
            info.sreg =  SREG_ES;
            break;
        case TOKEN_CS:
            info.type = TYPE_SREG;
            info.mod =  MOD_R;
            info.sreg =  SREG_CS;
            break;
        case TOKEN_SS:
            info.type = TYPE_SREG;
            info.mod =  MOD_R;
            info.sreg =  SREG_SS;
            break;
        case TOKEN_DS:
            info.type = TYPE_SREG;
            info.mod =  MOD_R;
            info.sreg =  SREG_DS;
            break;
        case TOKEN_FS:
            info.type = TYPE_SREG;
            info.mod =  MOD_R;
            info.sreg =  SREG_FS;
            break;
        case TOKEN_GS:
            info.type = TYPE_SREG;
            info.mod =  MOD_R;
            info.sreg =  SREG_GS;
            break;
        default:
            _serf_;
            break;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        NTVDM64_TYPE_TRACE_CALL_END;
        return info;
    } else _serf_;
    _chrf(matchtoken(TOKEN_LSPAREN));
    _chrf(token = gettoken(NULL));
    while (token != TOKEN_RSPAREN) {
        switch (token) {
        case TOKEN_PLUS:
            break;
        case TOKEN_MINUS:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_MINUS)");
            _chrf(token = gettoken(NULL));
            neg = 1;
            switch (token) {
            case TOKEN_IMM8:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_IMM8)");
                if (info.mod != MOD_M) _serf_;
                if (tokimm8 > 0x80) _serf_;
                else {
                    tokimm8 = (~tokimm8) + 1;
                    info.disp8 = tokimm8;
                    info.mod = MOD_M_DISP8;
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            case TOKEN_IMM16:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_IMM16)");
                if (info.mod != MOD_M) _serf_;
                if (tokimm16 > 0xff80) _serf_;
                else {
                    tokimm16 = (~tokimm16) + 1;
                    info.disp16 = tokimm16;
                    info.mod = MOD_M_DISP16;
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            case TOKEN_IMM32:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_IMM32)");
                if (info.mod != MOD_M) _serf_;
                if (tokimm32 > 0xffffff80) _serf_;
                else {
                    tokimm32 = (~tokimm32) + 1;
                    info.disp32 = tokimm32;
                    info.mod = MOD_M_DISP32;
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            default:
                _serf_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case TOKEN_BX:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_BX)");
            if (bx) _serf_;
            else bx = 1;
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case TOKEN_SI:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_SI)");
            if (si) _serf_;
            else si = 1;
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case TOKEN_BP:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_BP)");
            if (bp) _serf_;
            else bp = 1;
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case TOKEN_DI:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_DI)");
            if (di) _serf_;
            else di = 1;
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case TOKEN_AL:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_AL)");
            if (al) _serf_;
            else al = 1;
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case TOKEN_IMM8:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_IMM8)");
            if (info.mod != MOD_M) _serf_;
            info.mod = MOD_M_DISP8;
            info.disp8 = tokimm8;
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case TOKEN_IMM16:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_IMM16)");
            if (info.mod != MOD_M) _serf_;
            info.mod = MOD_M_DISP16;
            info.disp16 = tokimm16;
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case TOKEN_IMM32:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_IMM32)");
            if (info.mod != MOD_M) _serf_;
            info.mod = MOD_M_DISP32;
            info.disp32 = tokimm32;
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case TOKEN_EAX:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_EAX)");
            _chrf(token = gettoken(NULL));
            if (token == TOKEN_TIMES) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_TIMES)");
                if (ieax) _serf_;
                else {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!ieax");
                    ieax = 1;
                    _chrf(token = gettoken(NULL));
                    if (token != TOKEN_IMM8) _serf_;
                    else {
                        info.sib.scale = tokimm8;
                        info.sib.index = R32_EAX;
                        _chrf(token = gettoken(NULL));
                    }
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
            } else {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_!TIMES)");
                if (eax) _serf_;
                else {
                    eax = 1;
                    info.sib.base = R32_EAX;
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            continue;
            break;
        case TOKEN_ECX:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_ECX)");
            _chrf(token = gettoken(NULL));
            if (token == TOKEN_TIMES) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_TIMES)");
                if (iecx) _serf_;
                else {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!iecx");
                    iecx = 1;
                    _chrf(token = gettoken(NULL));
                    if (token != TOKEN_IMM8) _serf_;
                    else {
                        info.sib.scale = tokimm8;
                        info.sib.index = R32_ECX;
                        _chrf(token = gettoken(NULL));
                    }
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
            } else {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(!TOKEN_TIMES)");
                if (ecx) _serf_;
                else {
                    ecx = 1;
                    info.sib.base = R32_ECX;
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            continue;
            break;
        case TOKEN_EDX:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_EDX)");
            _chrf(token = gettoken(NULL));
            if (token == TOKEN_TIMES) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_TIMES)");
                if (iedx) _serf_;
                else {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!iedx");
                    iedx = 1;
                    _chrf(token = gettoken(NULL));
                    if (token != TOKEN_IMM8) _serf_;
                    else {
                        info.sib.scale = tokimm8;
                        info.sib.index = R32_EDX;
                        _chrf(token = gettoken(NULL));
                    }
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
            } else {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(!TOKEN_TIMES)");
                if (edx) _serf_;
                else {
                    edx = 1;
                    info.sib.base = R32_EDX;
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            continue;
            break;
        case TOKEN_EBX:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_EBX)");
            _chrf(token = gettoken(NULL));
            if (token == TOKEN_TIMES) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_TIMES)");
                if (iebx) _serf_;
                else {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!iebx");
                    iebx = 1;
                    _chrf(token = gettoken(NULL));
                    if (token != TOKEN_IMM8) _serf_;
                    else {
                        info.sib.scale = tokimm8;
                        info.sib.index = R32_EBX;
                        _chrf(token = gettoken(NULL));
                    }
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
            } else {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(!TOKEN_TIMES)");
                if (ebx) _serf_;
                else {
                    ebx = 1;
                    info.sib.base = R32_EBX;
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            continue;
            break;
        case TOKEN_ESP:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_ESP)");
            _chrf(token = gettoken(NULL));
            if (token == TOKEN_TIMES) _serf_;
            else {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(!TOKEN_TIMES)");
                if (esp) _serf_;
                else {
                    esp = 1;
                    info.sib.base = R32_ESP;
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            continue;
            break;
        case TOKEN_EBP:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_EBP)");
            _chrf(token = gettoken(NULL));
            if (token == TOKEN_TIMES) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_TIMES)");
                if (iebp) _serf_;
                else {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!iebp");
                    iebx = 1;
                    _chrf(token = gettoken(NULL));
                    if (token != TOKEN_IMM8) _serf_;
                    else {
                        info.sib.scale = tokimm8;
                        info.sib.index = R32_EBP;
                        _chrf(token = gettoken(NULL));
                    }
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
            } else {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(!TOKEN_TIMES)");
                if (ebp) _serf_;
                else {
                    ebp = 1;
                    info.sib.base = R32_EBP;
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            continue;
            break;
        case TOKEN_ESI:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_ESI)");
            _chrf(token = gettoken(NULL));
            if (token == TOKEN_TIMES) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_TIMES)");
                if (iesi) _serf_;
                else {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!iesi");
                    iesi = 1;
                    _chrf(token = gettoken(NULL));
                    if (token != TOKEN_IMM8) _serf_;
                    else {
                        info.sib.scale = tokimm8;
                        info.sib.index = R32_ESI;
                        _chrf(token = gettoken(NULL));
                    }
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
            } else {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(!TOKEN_TIMES)");
                if (esi) _serf_;
                else {
                    esi = 1;
                    info.sib.base = R32_ESI;
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            continue;
            break;
        case TOKEN_EDI:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_EDI)");
            _chrf(token = gettoken(NULL));
            if (token == TOKEN_TIMES) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_TIMES)");
                if (iedi) _serf_;
                else {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!iedi");
                    iedi = 1;
                    _chrf(token = gettoken(NULL));
                    if (token != TOKEN_IMM8) _serf_;
                    else {
                        info.sib.scale = tokimm8;
                        info.sib.index = R32_EDI;
                        _chrf(token = gettoken(NULL));
                    }
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
            } else {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(!TOKEN_TIMES)");
                if (edi) _serf_;
                else {
                    edi = 1;
                    info.sib.base = R32_EDI;
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            continue;
            break;
        default:
            _serf_;
            break;
        }
        _chrf(token = gettoken(NULL));
    }
    _chrf(token = gettoken(NULL));
    if (token != TOKEN_END) _serf_;

    if (al) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("al");
        if (bp || si || di || eax || ecx || edx || esp || ebp || esi || edi ||
                ieax || iecx || iedx || iebx || iebp || iesi || iedi || info.mod != MOD_M)
            _serf_;
        if (bx && !ebx) {
            info.mem = MEM_BX_AL;
        } else if (!bx && ebx) {
            info.mem = MEM_EBX_AL;
        } else _serf_;
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else {
        if (bx || bp || si || di || info.mod == MOD_M_DISP16) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("16-bit Addressing");
            if (!bx && !si && !bp && !di) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("[DISP16]");
                info.mem = MEM_BP;
                if (info.mod == MOD_M_DISP16)
                    info.mod = MOD_M;
                else _serf_;
                NTVDM64_TYPE_TRACE_BLOCK_END;
            } else {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bx/bp/si/di");
                if ( bx &&  si && !bp && !di) info.mem = MEM_BX_SI;
                else if ( bx && !si && !bp &&  di) info.mem = MEM_BX_DI;
                else if (!bx &&  si &&  bp && !di) info.mem = MEM_BP_SI;
                else if (!bx && !si &&  bp &&  di) info.mem = MEM_BP_DI;
                else if ( bx && !si && !bp && !di) info.mem = MEM_BX;
                else if (!bx &&  si && !bp && !di) info.mem = MEM_SI;
                else if (!bx && !si &&  bp && !di) {
                    info.mem = MEM_BP;
                    if (info.mod == MOD_M) {
                        info.mod = MOD_M_DISP8;
                        info.disp8 = 0x00;
                    }
                } else if (!bx && !si && !bp && di) info.mem = MEM_DI;
                else _serf_;
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else if (eax || ecx || edx || ebx || esp || ebp || esi || edi ||
                   ieax || iecx || iedx || iebx || iebp || iesi || iedi || info.mod == MOD_M_DISP32) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("32-bit Addressing");
            if (!eax && !ecx && !edx && !ebx && !esp && !ebp && !esi && !edi) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!base");
                if (info.mod == MOD_M_DISP32) {
                    info.mod = MOD_M;
                    if ( ieax || iecx || iedx || iebx || iebp || iesi || iedi)
                        info.mem = MEM_SIB;
                    else
                        info.mem = MEM_EBP;
                } else _serf_;
                NTVDM64_TYPE_TRACE_BLOCK_END;
            } else {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("base");
                if (esp || ieax || iecx || iedx || iebx || iebp || iesi || iedi) {
                    info.mem = MEM_SIB;
                } else if (eax) info.mem = MEM_EAX;
                else if (ecx) info.mem = MEM_ECX;
                else if (edx) info.mem = MEM_EDX;
                else if (ebx) info.mem = MEM_EBX;
                else if (ebp) info.mem = MEM_EBP;
                else if (esi) info.mem = MEM_ESI;
                else if (edi) info.mem = MEM_EDI;
                else _serf_;
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else _serf_;
    }
    switch (info.mem) {
    case MEM_BX_SI:
    case MEM_BX_DI:
    case MEM_BX:
    case MEM_SI:
    case MEM_DI:
        if (info.flagds) info.flagds = 0;
        break;
    case MEM_BP_SI:
    case MEM_BP_DI:
        if (info.flagss) info.flagss = 0;
        break;
    case MEM_BP:
        if (!bp && info.flagds) info.flagds = 0;
        else if (bp && info.flagss) info.flagss = 0;
        break;
    case MEM_EAX:
    case MEM_ECX:
    case MEM_EDX:
    case MEM_EBX:
    case MEM_ESI:
    case MEM_EDI:
        if (info.flagds) info.flagds = 0;
        break;
    case MEM_EBP:
        if (info.flagss) info.flagss = 0;
        break;
    case MEM_SIB:
        if (info.sib.base == R32_ESP ||
                (info.sib.base == R32_EBP && info.mod != MOD_M)) {
            if (info.flagss) info.flagss = 0;
        } else if (info.flagds) info.flagds = 0;
        break;
    case MEM_BX_AL:
    case MEM_EBX_AL:
        break;
    default:
        _serf_;
        break;
    }
    info.type = TYPE_M;
    NTVDM64_TYPE_TRACE_CALL_END;
    return info;
}
static t_aasm_oprinfo parsearg_imm(t_aasm_token token) {
    t_aasm_oprinfo info;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("parsearg_imm");

    STD_MEMSET((void *)(&info), 0x00, sizeof(t_aasm_oprinfo));

    if (token == TOKEN_PLUS) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_PLUS)");
        info.imms = 1;
        info.immn = 0;
        _chrf(token = gettoken(NULL));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else if (token == TOKEN_MINUS) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_MINUS)");
        info.imms = 1;
        info.immn = 1;
        _chrf(token = gettoken(NULL));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }

    if (token == TOKEN_IMM8) {
        info.type = TYPE_I8;
        if (!info.immn) info.imm8 = tokimm8;
        else info.imm8 = (~tokimm8) + 1;
    } else if (token == TOKEN_IMM16) {
        info.type = TYPE_I16;
        if (!info.immn) info.imm16 = tokimm16;
        else info.imm16 = (~tokimm16) + 1;
    } else if (token == TOKEN_IMM32) {
        info.type = TYPE_I32;
        if (!info.immn) info.imm32 = tokimm32;
        else info.imm32 = (~tokimm32) + 1;
    } else _serf_;

    _chrf(token = gettoken(NULL));
    if (token == TOKEN_COLON) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(!TOKEN_END)");
        if (info.imms) _serf_;
        if (info.type == TYPE_I16) info.rcs = info.imm16;
        else {
            info.type = TYPE_NONE;
            _serf_;
        }
        info.type = TYPE_NONE;
        _chrf(token = gettoken(NULL));
        if (token == TOKEN_IMM16) {
            info.reip = tokimm16;
            info.type = TYPE_I16_16;
        } else if (token == TOKEN_IMM32) {
            info.reip = tokimm32;
            info.type = TYPE_I16_32;
        } else _serf_;
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else if (token != TOKEN_END) _serf_;
    NTVDM64_TYPE_TRACE_CALL_END;
    return info;
}
static t_aasm_oprinfo parsearg(char *arg) {
    t_aasm_token token;
    t_aasm_oprinfo info;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("parsearg");
    STD_MEMSET((void *)(&info), 0x00 ,sizeof(t_aasm_oprinfo));
    if (!arg || !arg[0]) {
        info.type = TYPE_NONE;
        NTVDM64_TYPE_TRACE_CALL_END;
        return info;
    }
    _chrf(token = gettoken(arg));
    switch (token) {
    case TOKEN_NULL:
    case TOKEN_END:
        info.type = TYPE_NONE;
        break;
    case TOKEN_BYTE:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_BYTE)");
        _chrf(token = gettoken(NULL));
        if (token == TOKEN_PTR) _chrf(token = gettoken(NULL));
        _chrf(info = parsearg_mem(token));
        info.type = TYPE_M8;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case TOKEN_WORD:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_WORD)");
        _chrf(token = gettoken(NULL));
        if (token == TOKEN_PTR) _chrf(token = gettoken(NULL));
        _chrf(info = parsearg_mem(token));
        info.type = TYPE_M16;
        info.ptr = PTR_NEAR;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case TOKEN_DWORD:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_DWORD)");
        _chrf(token = gettoken(NULL));
        if (token == TOKEN_PTR) _chrf(token = gettoken(NULL));
        _chrf(info = parsearg_mem(token));
        info.type = TYPE_M32;
        info.ptr = PTR_FAR;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case TOKEN_AL:
        info.type = TYPE_R8;
        info.mod = MOD_R;
        info.reg8 = R8_AL;
        break;
    case TOKEN_CL:
        info.type = TYPE_R8;
        info.mod = MOD_R;
        info.reg8 = R8_CL;
        break;
    case TOKEN_DL:
        info.type = TYPE_R8;
        info.mod = MOD_R;
        info.reg8 = R8_DL;
        break;
    case TOKEN_BL:
        info.type = TYPE_R8;
        info.mod = MOD_R;
        info.reg8 = R8_BL;
        break;
    case TOKEN_AH:
        info.type = TYPE_R8;
        info.mod = MOD_R;
        info.reg8 = R8_AH;
        break;
    case TOKEN_CH:
        info.type = TYPE_R8;
        info.mod = MOD_R;
        info.reg8 = R8_CH;
        break;
    case TOKEN_DH:
        info.type = TYPE_R8;
        info.mod = MOD_R;
        info.reg8 = R8_DH;
        break;
    case TOKEN_BH:
        info.type = TYPE_R8;
        info.mod = MOD_R;
        info.reg8 = R8_BH;
        break;
    case TOKEN_AX:
        info.type = TYPE_R16;
        info.mod = MOD_R;
        info.reg16 = R16_AX;
        info.ptr = PTR_NEAR;
        break;
    case TOKEN_CX:
        info.type = TYPE_R16;
        info.mod = MOD_R;
        info.reg16 = R16_CX;
        info.ptr = PTR_NEAR;
        break;
    case TOKEN_DX:
        info.type = TYPE_R16;
        info.mod = MOD_R;
        info.reg16 = R16_DX;
        info.ptr = PTR_NEAR;
        break;
    case TOKEN_BX:
        info.type = TYPE_R16;
        info.mod = MOD_R;
        info.reg16 = R16_BX;
        info.ptr = PTR_NEAR;
        break;
    case TOKEN_SP:
        info.type = TYPE_R16;
        info.mod = MOD_R;
        info.reg16 = R16_SP;
        info.ptr = PTR_NEAR;
        break;
    case TOKEN_BP:
        info.type = TYPE_R16;
        info.mod = MOD_R;
        info.reg16 = R16_BP;
        info.ptr = PTR_NEAR;
        break;
    case TOKEN_SI:
        info.type = TYPE_R16;
        info.mod = MOD_R;
        info.reg16 = R16_SI;
        info.ptr = PTR_NEAR;
        break;
    case TOKEN_DI:
        info.type = TYPE_R16;
        info.mod = MOD_R;
        info.reg16 = R16_DI;
        info.ptr = PTR_NEAR;
        break;
    case TOKEN_EAX:
        info.type = TYPE_R32;
        info.mod = MOD_R;
        info.reg32 = R32_EAX;
        info.ptr = PTR_NEAR;
        break;
    case TOKEN_ECX:
        info.type = TYPE_R32;
        info.mod = MOD_R;
        info.reg32 = R32_ECX;
        info.ptr = PTR_NEAR;
        break;
    case TOKEN_EDX:
        info.type = TYPE_R32;
        info.mod = MOD_R;
        info.reg32 = R32_EDX;
        info.ptr = PTR_NEAR;
        break;
    case TOKEN_EBX:
        info.type = TYPE_R32;
        info.mod = MOD_R;
        info.reg32 = R32_EBX;
        info.ptr = PTR_NEAR;
        break;
    case TOKEN_ESP:
        info.type = TYPE_R32;
        info.mod = MOD_R;
        info.reg32 = R32_ESP;
        info.ptr = PTR_NEAR;
        break;
    case TOKEN_EBP:
        info.type = TYPE_R32;
        info.mod = MOD_R;
        info.reg32 = R32_EBP;
        info.ptr = PTR_NEAR;
        break;
    case TOKEN_ESI:
        info.type = TYPE_R32;
        info.mod = MOD_R;
        info.reg32 = R32_ESI;
        info.ptr = PTR_NEAR;
        break;
    case TOKEN_EDI:
        info.type = TYPE_R32;
        info.mod = MOD_R;
        info.reg32 = R32_EDI;
        info.ptr = PTR_NEAR;
        break;
    case TOKEN_ES:
    case TOKEN_CS:
    case TOKEN_SS:
    case TOKEN_DS:
    case TOKEN_FS:
    case TOKEN_GS:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_SREGs)");
        _chrf(info = parsearg_mem(token));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case TOKEN_PLUS:
    case TOKEN_MINUS:
    case TOKEN_IMM8:
    case TOKEN_IMM16:
    case TOKEN_IMM32:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_IMMs)");
        _chrf(info = parsearg_imm(token));
        if (info.type == TYPE_I16_16)
            info.ptr = PTR_FAR;
        else if (info.type == TYPE_I16_32)
            info.ptr = PTR_FAR;
        else
            info.ptr = PTR_NONE;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case TOKEN_SHORT:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_SHORT)");
        _chrf(token = gettoken(NULL));
        if (token == TOKEN_PTR) _chrf(token = gettoken(NULL));
        if (token == TOKEN_PLUS || token == TOKEN_MINUS) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_SIGNs)");
            _chrf(info = parsearg_imm(token));
            if (info.type != TYPE_I8) _serf_;
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else _serf_;
        info.ptr = PTR_SHORT;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case TOKEN_NEAR:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_NEAR)");
        _chrf(token = gettoken(NULL));
        if (token == TOKEN_PTR) _chrf(token = gettoken(NULL));
        switch (token) {
        case TOKEN_WORD:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_WORD)");
            _chrf(token = gettoken(NULL));
            if (token == TOKEN_PTR) _chrf(token = gettoken(NULL));
            _chrf(info = parsearg_mem(token));
            info.type = TYPE_M16;
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case TOKEN_DWORD:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_DWORD)");
            _chrf(token = gettoken(NULL));
            if (token == TOKEN_PTR) _chrf(token = gettoken(NULL));
            _chrf(info = parsearg_mem(token));
            info.type = TYPE_M32;
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case TOKEN_PLUS:
        case TOKEN_MINUS:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_PLUS/TOKEN_MINUS)");
            _chrf(info = parsearg_imm(token));
            if (info.type != TYPE_I16 && info.type != TYPE_I32) _serf_;
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case TOKEN_ES:
        case TOKEN_CS:
        case TOKEN_SS:
        case TOKEN_DS:
        case TOKEN_FS:
        case TOKEN_GS:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_SREGs)");
            _chrf(info = parsearg_mem(token));
            if (info.type != TYPE_M) _serf_;
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            _serf_;
            break;
        }
        info.ptr = PTR_NEAR;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case TOKEN_FAR:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_FAR)");
        _chrf(token = gettoken(NULL));
        if (token == TOKEN_PTR) _chrf(token = gettoken(NULL));
        switch (token) {
        case TOKEN_WORD:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_WORD)");
            _chrf(token = gettoken(NULL));
            if (token == TOKEN_PTR) _chrf(token = gettoken(NULL));
            _chrf(info = parsearg_mem(token));
            info.type = TYPE_M16;
            info.ptr = PTR_FAR;
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case TOKEN_DWORD:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_DWORD)");
            _chrf(token = gettoken(NULL));
            if (token == TOKEN_PTR) _chrf(token = gettoken(NULL));
            _chrf(info = parsearg_mem(token));
            info.type = TYPE_M32;
            info.ptr = PTR_FAR;
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case TOKEN_IMM16:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_IMM16)");
            _chrf(info = parsearg_imm(token));
            if (info.type != TYPE_I16_16 && info.type != TYPE_I16_32) _serf_;
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case TOKEN_ES:
        case TOKEN_CS:
        case TOKEN_SS:
        case TOKEN_DS:
        case TOKEN_FS:
        case TOKEN_GS:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("token(TOKEN_SREGs)");
            _chrf(info = parsearg_mem(token));
            if (info.type != TYPE_M) _serf_;
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            _serf_;
            break;
        }
        info.ptr = PTR_FAR;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case TOKEN_CR0:
        info.type = TYPE_CREG;
        info.creg = CREG_CR0;
        break;
    case TOKEN_CR2:
        info.type = TYPE_CREG;
        info.creg = CREG_CR2;
        break;
    case TOKEN_CR3:
        info.type = TYPE_CREG;
        info.creg = CREG_CR3;
        break;
    case TOKEN_DR0:
        info.type = TYPE_DREG;
        info.dreg = DREG_DR0;
        break;
    case TOKEN_DR1:
        info.type = TYPE_DREG;
        info.dreg = DREG_DR1;
        break;
    case TOKEN_DR2:
        info.type = TYPE_DREG;
        info.dreg = DREG_DR2;
        break;
    case TOKEN_DR3:
        info.type = TYPE_DREG;
        info.dreg = DREG_DR3;
        break;
    case TOKEN_DR6:
        info.type = TYPE_DREG;
        info.dreg = DREG_DR6;
        break;
    case TOKEN_DR7:
        info.type = TYPE_DREG;
        info.dreg = DREG_DR7;
        break;
    case TOKEN_TR6:
        info.type = TYPE_TREG;
        info.treg = TREG_TR6;
        break;
    case TOKEN_TR7:
        info.type = TYPE_TREG;
        info.treg = TREG_TR7;
        break;
    default:
        _serf_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
    return info;
}
/* assembly compiler: code generator */
static void _c_setbyte(uint8_t byte) {
    (*(uint8_t *)(acode + iop)) = byte;
    iop += 1;
}
static void _c_setword(uint16_t word) {
    (*(uint16_t *)(acode + iop)) = word;
    iop += 2;
}
static void _c_setdword(uint32_t dword) {
    (*(uint32_t *)(acode + iop)) = dword;
    iop += 4;
}
static void _c_imm8(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_c_imm8");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_setbyte(byte));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void _c_imm16(uint16_t word) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_c_imm16");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_setword(word));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void _c_imm32(uint32_t dword) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_c_imm32");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_setdword(dword));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void _c_modrm(t_aasm_oprinfo rminfo, uint8_t reg) {
    uint8_t sibval;
    uint8_t modrmval = (reg << 3);

    NTVDM64_TYPE_TRACE_CALL_BEGIN("_c_rminfo");

    switch (rminfo.mem) {
    case MEM_BX_SI:
    case MEM_BX_DI:
    case MEM_BP_SI:
    case MEM_BP_DI:
    case MEM_SI:
    case MEM_DI:
    case MEM_BP:
    case MEM_BX:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("16-bit Addressing");
        _SetAddressSize(2);
        switch (rminfo.mod) {
        case MOD_M:
            modrmval |= (0 << 6);
            modrmval |= (uint8_t)rminfo.mem;
            _c_setbyte(modrmval);
            switch (rminfo.mem) {
            case MEM_BP:
                _c_setword(rminfo.disp16);
                break;
            default:
                break;
            }
            break;
        case MOD_M_DISP8:
            modrmval |= (1 << 6);
            modrmval |= (uint8_t)rminfo.mem;
            _c_setbyte(modrmval);
            _c_setbyte(rminfo.disp8);
            break;
        case MOD_M_DISP16:
            modrmval |= (2 << 6);
            modrmval |= (uint8_t)rminfo.mem;
            _c_setbyte(modrmval);
            _c_setword(rminfo.disp16);
            break;
        case MOD_R:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("mod(MOD_R)");
            modrmval |= (3 << 6);
            switch (rminfo.type) {
            case TYPE_R8:
                modrmval |= (uint8_t)rminfo.reg8;
                _c_setbyte(modrmval);
                break;
            case TYPE_R16:
                modrmval |= (uint8_t)rminfo.reg16;
                _c_setbyte(modrmval);
                break;
            case TYPE_R32:
                modrmval |= (uint8_t)rminfo.reg32;
                _c_setbyte(modrmval);
                break;
            default:
                _ser_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            _ser_;
            break;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case MEM_EAX:
    case MEM_ECX:
    case MEM_EDX:
    case MEM_EBX:
    case MEM_SIB:
    case MEM_EBP:
    case MEM_ESI:
    case MEM_EDI:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("32-bit Addressing");
        _SetAddressSize(4);
        switch (rminfo.mod) {
        case MOD_M:
            modrmval |= (0 << 6);
            modrmval |= (uint8_t)rminfo.mem & 0x07;
            _c_setbyte(modrmval);
            switch (rminfo.mem) {
            case MEM_SIB:
                sibval = (uint8_t)rminfo.sib.base;
                sibval |= ((uint8_t)rminfo.sib.index << 3);
                switch (rminfo.sib.scale) {
                case 0:
                    rminfo.sib.scale = 0;
                    break;
                case 1:
                    rminfo.sib.scale = 0;
                    break;
                case 2:
                    rminfo.sib.scale = 1;
                    break;
                case 4:
                    rminfo.sib.scale = 2;
                    break;
                case 8:
                    rminfo.sib.scale = 3;
                    break;
                default:
                    _ser_;
                    break;
                }
                sibval |= (rminfo.sib.scale << 6);
                _c_setbyte(sibval);
                switch (rminfo.sib.base) {
                case R32_EBP:
                    _c_setdword(rminfo.disp32);
                    break;
                default:
                    break;
                }
                break;
            case MEM_EBP:
                _c_setdword(rminfo.disp32);
                break;
            default:
                break;
            }
            break;
        case MOD_M_DISP8:
            modrmval |= (1 << 6);
            modrmval |= (uint8_t)rminfo.mem & 0x07;
            _c_setbyte(modrmval);
            switch (rminfo.mem) {
            case MEM_SIB:
                sibval = (uint8_t)rminfo.sib.base;
                sibval |= ((uint8_t)rminfo.sib.index << 3);
                switch (rminfo.sib.scale) {
                case 0:
                    rminfo.sib.scale = 0;
                    break;
                case 1:
                    rminfo.sib.scale = 0;
                    break;
                case 2:
                    rminfo.sib.scale = 1;
                    break;
                case 4:
                    rminfo.sib.scale = 2;
                    break;
                case 8:
                    rminfo.sib.scale = 3;
                    break;
                default:
                    _ser_;
                    break;
                }
                sibval |= (rminfo.sib.scale << 6);
                _c_setbyte(sibval);
                break;
            default:
                break;
            }
            _c_setbyte(rminfo.disp8);
            break;
        case MOD_M_DISP32:
            modrmval |= (2 << 6);
            modrmval |= (uint8_t)rminfo.mem & 0x07;
            _c_setbyte(modrmval);
            switch (rminfo.mem) {
            case MEM_SIB:
                sibval = (uint8_t)rminfo.sib.base;
                sibval |= ((uint8_t)rminfo.sib.index << 3);
                switch (rminfo.sib.scale) {
                case 0:
                    rminfo.sib.scale = 0;
                    break;
                case 1:
                    rminfo.sib.scale = 0;
                    break;
                case 2:
                    rminfo.sib.scale = 1;
                    break;
                case 4:
                    rminfo.sib.scale = 2;
                    break;
                case 8:
                    rminfo.sib.scale = 3;
                    break;
                default:
                    _ser_;
                    break;
                }
                sibval |= (rminfo.sib.scale << 6);
                _c_setbyte(sibval);
                break;
            default:
                break;
            }
            _c_setdword(rminfo.disp32);
            break;
        case MOD_R:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("mod(MOD_R)");
            modrmval |= (3 << 6);
            switch (rminfo.type) {
            case TYPE_R8:
                modrmval |= (uint8_t)rminfo.reg8;
                _c_setbyte(modrmval);
                break;
            case TYPE_R16:
                modrmval |= (uint8_t)rminfo.reg16;
                _c_setbyte(modrmval);
                break;
            case TYPE_R32:
                modrmval |= (uint8_t)rminfo.reg32;
                _c_setbyte(modrmval);
                break;
            default:
                _ser_;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            _ser_;
            break;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}

/* concrete instructions */
static void ADD_RM8_R8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADD_RM8_R8");
    _c_setbyte(0x00);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void ADD_RM32_R32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADD_RM32_R32");
    _SetOperandSize(byte);
    _c_setbyte(0x01);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void ADD_R8_RM8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADD_R8_RM8");
    _c_setbyte(0x02);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void ADD_R32_RM32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADD_R32_RM32");
    _SetOperandSize(byte);
    _c_setbyte(0x03);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void ADD_AL_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADD_AL_I8");
    _c_setbyte(0x04);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(aopri2.imm8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void ADD_EAX_I32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADD_EAX_I32");
    _SetOperandSize(byte);
    _c_setbyte(0x05);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri2.imm16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm32(aopri2.imm32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PUSH_ES() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_ES");
    _c_setbyte(0x06);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void POP_ES() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_ES");
    _c_setbyte(0x07);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void OR_RM8_R8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OR_RM8_R8");
    _c_setbyte(0x08);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void OR_RM32_R32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OR_RM32_R32");
    _SetOperandSize(byte);
    _c_setbyte(0x09);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void OR_R8_RM8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OR_R8_RM8");
    _c_setbyte(0x0a);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void OR_R32_RM32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OR_R32_RM32");
    _SetOperandSize(byte);
    _c_setbyte(0x0b);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void OR_AL_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OR_AL_I8");
    _c_setbyte(0x0c);
    _c_imm8(aopri2.imm8);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void OR_EAX_I32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OR_EAX_I32");
    _SetOperandSize(byte);
    _c_setbyte(0x0d);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri2.imm16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm32(aopri2.imm32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PUSH_CS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_CS");
    _c_setbyte(0x0e);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void POP_CS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_CS");
    _c_setbyte(0x0f);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_0F() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_0F");
    _c_setbyte(0x0f);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void ADC_RM8_R8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADC_RM8_R8");
    _c_setbyte(0x10);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void ADC_RM32_R32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADC_RM32_R32");
    _SetOperandSize(byte);
    _c_setbyte(0x11);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void ADC_R8_RM8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADC_R8_RM8");
    _c_setbyte(0x12);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void ADC_R32_RM32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADC_R32_RM32");
    _SetOperandSize(byte);
    _c_setbyte(0x13);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void ADC_AL_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADC_AL_I8");
    _c_setbyte(0x14);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(aopri2.imm8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void ADC_EAX_I32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADC_EAX_I32");
    _SetOperandSize(byte);
    _c_setbyte(0x15);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri2.imm16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm32(aopri2.imm32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PUSH_SS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_SS");
    _c_setbyte(0x16);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void POP_SS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_SS");
    _c_setbyte(0x17);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SBB_RM8_R8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SBB_RM8_R8");
    _c_setbyte(0x18);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SBB_RM32_R32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SBB_RM32_R32");
    _SetOperandSize(byte);
    _c_setbyte(0x19);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SBB_R8_RM8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SBB_R8_RM8");
    _c_setbyte(0x1a);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SBB_R32_RM32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SBB_R32_RM32");
    _SetOperandSize(byte);
    _c_setbyte(0x1b);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SBB_AL_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SBB_AL_I8");
    _c_setbyte(0x1c);
    _c_imm8(aopri2.imm8);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SBB_EAX_I32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SBB_EAX_I32");
    _SetOperandSize(byte);
    _c_setbyte(0x1d);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri2.imm16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm32(aopri2.imm32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PUSH_DS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_DS");
    _c_setbyte(0x1e);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void POP_DS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_DS");
    _c_setbyte(0x1f);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void AND_RM8_R8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("AND_RM8_R8");
    _c_setbyte(0x20);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void AND_RM32_R32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("AND_RM32_R32");
    _SetOperandSize(byte);
    _c_setbyte(0x21);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void AND_R8_RM8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("AND_R8_RM8");
    _c_setbyte(0x22);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void AND_R32_RM32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("AND_R32_RM32");
    _SetOperandSize(byte);
    _c_setbyte(0x23);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void AND_AL_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("AND_AL_I8");
    _c_setbyte(0x24);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(aopri2.imm8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void AND_EAX_I32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("AND_EAX_I32");
    _SetOperandSize(byte);
    _c_setbyte(0x25);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri2.imm16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm32(aopri2.imm32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PREFIX_ES() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PREFIX_ES");
    if (ARG_NONE) aoprig.flages = 1;
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void DAA() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("DAA");
    if (ARG_NONE) _c_setbyte(0x27);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SUB_RM8_R8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SUB_RM8_R8");
    _c_setbyte(0x28);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SUB_RM32_R32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SUB_RM32_R32");
    _SetOperandSize(byte);
    _c_setbyte(0x29);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SUB_R8_RM8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SUB_R8_RM8");
    _c_setbyte(0x2a);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SUB_R32_RM32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SUB_R32_RM32");
    _SetOperandSize(byte);
    _c_setbyte(0x2b);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SUB_AL_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SUB_AL_I8");
    _c_setbyte(0x2c);
    _c_imm8(aopri2.imm8);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SUB_EAX_I32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SUB_EAX_I32");
    _SetOperandSize(byte);
    _c_setbyte(0x2d);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri2.imm16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm32(aopri2.imm32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PREFIX_CS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PREFIX_CS");
    if (ARG_NONE) aoprig.flagcs = 1;
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void DAS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("DAS");
    if (ARG_NONE) _c_setbyte(0x2f);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XOR_RM8_R8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XOR_RM8_R8");
    _c_setbyte(0x30);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XOR_RM32_R32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XOR_RM32_R32");
    _SetOperandSize(byte);
    _c_setbyte(0x31);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XOR_R8_RM8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XOR_R8_RM8");
    _c_setbyte(0x32);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XOR_R32_RM32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XOR_R32_RM32");
    _SetOperandSize(byte);
    _c_setbyte(0x33);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XOR_AL_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XOR_AL_I8");
    _c_setbyte(0x34);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(aopri2.imm8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XOR_EAX_I32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XOR_EAX_I32");
    _SetOperandSize(byte);
    _c_setbyte(0x35);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri2.imm16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm32(aopri2.imm32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PREFIX_SS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PREFIX_SS");
    if (ARG_NONE) aoprig.flagss = 1;
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void AAA() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("AAA");
    if (ARG_NONE) _c_setbyte(0x37);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CMP_RM8_R8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CMP_RM8_R8");
    _c_setbyte(0x38);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CMP_RM32_R32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CMP_RM32_R32");
    _SetOperandSize(byte);
    _c_setbyte(0x39);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CMP_R8_RM8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CMP_R8_RM8");
    _c_setbyte(0x3a);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CMP_R32_RM32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CMP_R32_RM32");
    _SetOperandSize(byte);
    _c_setbyte(0x3b);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CMP_AL_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CMP_AL_I8");
    _c_setbyte(0x3c);
    _c_imm8(aopri2.imm8);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CMP_EAX_I32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CMP_EAX_I32");
    _SetOperandSize(byte);
    _c_setbyte(0x3d);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri2.imm16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm32(aopri2.imm32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PREFIX_DS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PREFIX_DS");
    if (ARG_NONE) aoprig.flagds = 1;
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void AAS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("AAS");
    if (ARG_NONE) _c_setbyte(0x3f);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INC_EAX(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INC_EAX");
    _SetOperandSize(byte);
    _c_setbyte(0x40);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INC_ECX(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INC_ECX");
    _SetOperandSize(byte);
    _c_setbyte(0x41);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INC_EDX(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INC_EDX");
    _SetOperandSize(byte);
    _c_setbyte(0x42);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INC_EBX(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INC_EBX");
    _SetOperandSize(byte);
    _c_setbyte(0x43);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INC_ESP(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INC_ESP");
    _SetOperandSize(byte);
    _c_setbyte(0x44);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INC_EBP(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INC_EBP");
    _SetOperandSize(byte);
    _c_setbyte(0x45);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INC_ESI(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INC_ESI");
    _SetOperandSize(byte);
    _c_setbyte(0x46);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INC_EDI(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INC_EDI");
    _SetOperandSize(byte);
    _c_setbyte(0x47);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void DEC_EAX(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("DEC_EAX");
    _SetOperandSize(byte);
    _c_setbyte(0x48);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void DEC_ECX(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("DEC_ECX");
    _SetOperandSize(byte);
    _c_setbyte(0x49);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void DEC_EDX(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("DEC_EDX");
    _SetOperandSize(byte);
    _c_setbyte(0x4a);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void DEC_EBX(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("DEC_EBX");
    _SetOperandSize(byte);
    _c_setbyte(0x4b);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void DEC_ESP(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("DEC_ESP");
    _SetOperandSize(byte);
    _c_setbyte(0x4c);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void DEC_EBP(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("DEC_EBP");
    _SetOperandSize(byte);
    _c_setbyte(0x4d);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void DEC_ESI(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("DEC_ESI");
    _SetOperandSize(byte);
    _c_setbyte(0x4e);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void DEC_EDI(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("DEC_EDI");
    _SetOperandSize(byte);
    _c_setbyte(0x4f);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PUSH_EAX(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_EAX");
    _SetOperandSize(byte);
    _c_setbyte(0x50);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PUSH_ECX(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_ECX");
    _SetOperandSize(byte);
    _c_setbyte(0x51);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PUSH_EDX(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_EDX");
    _SetOperandSize(byte);
    _c_setbyte(0x52);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PUSH_EBX(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_EBX");
    _SetOperandSize(byte);
    _c_setbyte(0x53);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PUSH_ESP(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_ESP");
    _SetOperandSize(byte);
    _c_setbyte(0x54);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PUSH_EBP(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_EBP");
    _SetOperandSize(byte);
    _c_setbyte(0x55);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PUSH_ESI(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_ESI");
    _SetOperandSize(byte);
    _c_setbyte(0x56);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PUSH_EDI(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_EDI");
    _SetOperandSize(byte);
    _c_setbyte(0x57);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void POP_EAX(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_EAX");
    _SetOperandSize(byte);
    _c_setbyte(0x58);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void POP_ECX(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_ECX");
    _SetOperandSize(byte);
    _c_setbyte(0x59);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void POP_EDX(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_EDX");
    _SetOperandSize(byte);
    _c_setbyte(0x5a);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void POP_EBX(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_EBX");
    _SetOperandSize(byte);
    _c_setbyte(0x5b);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void POP_ESP(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_ESP");
    _SetOperandSize(byte);
    _c_setbyte(0x5c);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void POP_EBP(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_EBP");
    _SetOperandSize(byte);
    _c_setbyte(0x5d);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void POP_ESI(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_ESI");
    _SetOperandSize(byte);
    _c_setbyte(0x5e);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void POP_EDI(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_EDI");
    _SetOperandSize(byte);
    _c_setbyte(0x5f);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PUSHA(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSHA");
    _SetOperandSize(byte);
    _c_setbyte(0x60);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void POPA(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POPA");
    _SetOperandSize(byte);
    _c_setbyte(0x61);
    NTVDM64_TYPE_TRACE_CALL_END;
}

static void BOUND_R32_M32_32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("BOUND_R32_M32_32");
    _SetOperandSize(byte);
    _c_setbyte(0x62);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void ARPL_RM16_R16() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ARPL_RM16_R16");
    if (ARG_RM16_R16) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ARG_RM16_R16");
        _c_setbyte(0x63);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PREFIX_FS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PREFIX_FS");
    if (ARG_NONE) aoprig.flagfs = 1;
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PREFIX_GS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PREFIX_GS");
    if (ARG_NONE) aoprig.flaggs = 1;
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PREFIX_OprSize() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PREFIX_OprSize");
    if (ARG_NONE) prefix_oprsizeg = 1;
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PREFIX_AddrSize() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PREFIX_AddrSize");
    if (ARG_NONE) prefix_addrsizeg = 1;
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PUSH_I32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_I32");
    _SetOperandSize(byte);
    _c_setbyte(0x68);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri1.imm16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm32(aopri1.imm32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void IMUL_R32_RM32_I32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("IMUL_R32_RM32_I32");
    _SetOperandSize(byte);
    _c_setbyte(0x69);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg16));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri3.imm16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg32));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm32(aopri3.imm32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;;
}
static void PUSH_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_I8");
    _c_setbyte(0x6a);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(aopri1.imm8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void IMUL_R32_RM32_I8(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("IMUL_R32_RM32_I32");
    _SetOperandSize(byte);
    _c_setbyte(0x6b);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg16));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(aopri3.imm8));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg32));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(aopri3.imm8));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;;
}
static void INSB() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INSB");
    _c_setbyte(0x6c);
    rinfo = NULL;
    if (ARG_NONE) ;
    else if (ARG_ESDI8_DX) _SetAddressSize(2);
    else if (ARG_ESEDI8_DX) _SetAddressSize(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INSW(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INSW");
    _SetOperandSize(byte);
    _c_setbyte(0x6d);
    rinfo = NULL;
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        if (ARG_NONE) ;
        else if (ARG_ESDI16_DX) _SetAddressSize(2);
        else if (ARG_ESEDI16_DX) _SetAddressSize(4);
        else _ser_;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        if (ARG_NONE) ;
        else if (ARG_ESDI32_DX) _SetAddressSize(2);
        else if (ARG_ESEDI32_DX) _SetAddressSize(4);
        else _ser_;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void OUTSB() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OUTSB");
    _c_setbyte(0x6e);
    rinfo = &aopri1;
    if (rinfo->flagds) rinfo->flagds = 0;
    if (ARG_NONE) rinfo = NULL;
    else if (ARG_DX_DSSI8) _SetAddressSize(2);
    else if (ARG_DSESI8) _SetAddressSize(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void OUTSW(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OUTSW");
    _SetOperandSize(byte);
    _c_setbyte(0x6f);
    rinfo = &aopri1;
    if (rinfo->flagds) rinfo->flagds = 0;
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        if (ARG_NONE) rinfo = NULL;
        else if (ARG_DX_DSSI16) _SetAddressSize(2);
        else if (ARG_DX_DSESI16) _SetAddressSize(4);
        else _ser_;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        if (ARG_NONE) rinfo = NULL;
        else if (ARG_DX_DSSI32) _SetAddressSize(2);
        else if (ARG_DX_DSESI32) _SetAddressSize(4);
        else _ser_;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_80(uint8_t rid) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_80");
    _c_setbyte(0x80);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, rid));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(aopri2.imm8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_81(uint8_t rid, uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_81");
    _SetOperandSize(byte);
    _c_setbyte(0x81);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, rid));
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri2.imm16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm32(aopri2.imm32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_83(uint8_t rid, uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_83");
    _SetOperandSize(byte);
    _c_setbyte(0x83);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, rid));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(aopri2.imm8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void TEST_RM8_R8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("TEST_RM8_R8");
    _c_setbyte(0x84);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void TEST_RM32_R32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("TEST_RM32_R32");
    _SetOperandSize(byte);
    _c_setbyte(0x85);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XCHG_RM8_R8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XCHG_RM8_R8");
    _c_setbyte(0x86);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XCHG_RM32_R32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XCHG_RM32_R32");
    _SetOperandSize(byte);
    _c_setbyte(0x87);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_RM8_R8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_RM8_R8");
    _c_setbyte(0x88);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_RM32_R32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_RM32_R32");
    _SetOperandSize(byte);
    _c_setbyte(0x89);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_R8_RM8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_RM8_R8");
    _c_setbyte(0x8a);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_R32_RM32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_R32_RM32");
    _SetOperandSize(byte);
    _c_setbyte(0x8b);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_RM16_SREG(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_RM16_SREG");
    _SetOperandSize(byte);
    _c_setbyte(0x8c);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.sreg));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LEA_R32_M32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LEA_R32_M32");
    _SetOperandSize(byte);
    _c_setbyte(0x8d);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_SREG_RM16(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_SREG_RM16");
    _SetOperandSize(byte);
    _c_setbyte(0x8e);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.sreg));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_8F(uint8_t rid, uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_8F");
    _SetOperandSize(byte);
    _c_setbyte(0x8f);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, rid));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void NOP() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("NOP");
    if (ARG_NONE) _c_setbyte(0x90);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XCHG_EAX_EAX(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XCHG_EAX_EAX");
    _SetOperandSize(byte);
    _c_setbyte(0x90);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XCHG_ECX_EAX(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XCHG_ECX_EAX");
    _SetOperandSize(byte);
    _c_setbyte(0x91);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XCHG_EDX_EAX(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XCHG_EDX_EAX");
    _SetOperandSize(byte);
    _c_setbyte(0x92);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XCHG_EBX_EAX(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XCHG_EBX_EAX");
    _SetOperandSize(byte);
    _c_setbyte(0x93);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XCHG_ESP_EAX(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XCHG_ESP_EAX");
    _SetOperandSize(byte);
    _c_setbyte(0x94);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XCHG_EBP_EAX(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XCHG_EBP_EAX");
    _SetOperandSize(byte);
    _c_setbyte(0x95);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XCHG_ESI_EAX(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XCHG_ESI_EAX");
    _SetOperandSize(byte);
    _c_setbyte(0x96);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XCHG_EDI_EAX(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XCHG_EDI_EAX");
    _SetOperandSize(byte);
    _c_setbyte(0x97);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CBW(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CBW");
    _SetOperandSize(byte);
    if (ARG_NONE) _c_setbyte(0x98);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CWD(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CWD");
    _SetOperandSize(byte);
    if (ARG_NONE) _c_setbyte(0x99);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CALL_PTR16_32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CALL_PTR16_32");
    _SetOperandSize(byte);
    _c_setbyte(0x9a);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16((uint16_t) aopri1.reip));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm32((uint32_t) aopri1.reip));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri1.rcs));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void WAIT() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("WAIT");
    _c_setbyte(0x9b);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PUSHF(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSHF");
    _SetOperandSize(byte);
    _c_setbyte(0x9c);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void POPF(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POPF");
    _SetOperandSize(byte);
    _c_setbyte(0x9d);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SAHF() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SAHF");
    _c_setbyte(0x9e);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LAHF() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LAHF");
    _c_setbyte(0x9f);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_AL_MOFFS8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_AL_MOFFS8");
    _c_setbyte(0xa0);
    if (aopri2.mem == MEM_BP) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("16-bit Addressing");
        _SetAddressSize(2);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri2.disp16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else if (aopri2.mem == MEM_EBP) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("32-bit Addressing");
        _SetAddressSize(4);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri2.disp32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_EAX_MOFFS32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_EAX_MOFFS32");
    _SetOperandSize(byte);
    _c_setbyte(0xa1);
    if (aopri2.mem == MEM_BP) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("16-bit Addressing");
        _SetAddressSize(2);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri2.disp16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else if (aopri2.mem == MEM_EBP) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("32-bit Addressing");
        _SetAddressSize(4);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm32(aopri2.disp32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_MOFFS8_AL() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_MOFFS8_AL");
    _c_setbyte(0xa2);
    if (aopri1.mem == MEM_BP) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("16-bit Addressing");
        _SetAddressSize(2);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri1.disp16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else if (aopri1.mem == MEM_EBP) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("32-bit Addressing");
        _SetAddressSize(4);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri1.disp32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_MOFFS32_EAX(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_MOFFS32_EAX");
    _SetOperandSize(byte);
    _c_setbyte(0xa3);
    if (aopri1.mem == MEM_BP) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("16-bit Addressing");
        _SetAddressSize(2);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri1.disp16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else if (aopri1.mem == MEM_EBP) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("32-bit Addressing");
        _SetAddressSize(4);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm32(aopri1.disp32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOVSB() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOVSB");
    _c_setbyte(0xa4);
    rinfo = &aopri2;
    if (rinfo->flagds) rinfo->flagds = 0;
    if (ARG_NONE) rinfo = NULL;
    else if (ARG_ESDI8_DSSI8) _SetAddressSize(2);
    else if (ARG_ESEDI8_DSESI8) _SetAddressSize(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOVSW(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOVSW");
    _SetOperandSize(byte);
    _c_setbyte(0xa5);
    rinfo = &aopri2;
    if (rinfo->flagds) rinfo->flagds = 0;
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        if (ARG_NONE) rinfo = NULL;
        else if (ARG_ESDI16_DSSI16) _SetAddressSize(2);
        else if (ARG_ESEDI16_DSESI16) _SetAddressSize(4);
        else _ser_;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        if (ARG_NONE) rinfo = NULL;
        else if (ARG_ESDI32_DSSI32) _SetAddressSize(2);
        else if (ARG_ESEDI32_DSESI32) _SetAddressSize(4);
        else _ser_;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CMPSB() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CMPSB");
    _c_setbyte(0xa6);
    rinfo = &aopri1;
    if (rinfo->flagds) rinfo->flagds = 0;
    if (ARG_NONE) rinfo = NULL;
    else if (ARG_DSSI8_ESDI8) _SetAddressSize(2);
    else if (ARG_DSESI8_ESEDI8) _SetAddressSize(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CMPSW(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CMPSW");
    _SetOperandSize(byte);
    _c_setbyte(0xa7);
    rinfo = &aopri1;
    if (rinfo->flagds) rinfo->flagds = 0;
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        if (ARG_NONE) rinfo = NULL;
        else if (ARG_DSSI16_ESDI16) _SetAddressSize(2);
        else if (ARG_DSESI16_ESEDI16) _SetAddressSize(4);
        else _ser_;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        if (ARG_NONE) rinfo = NULL;
        else if (ARG_DSSI32_ESDI32) _SetAddressSize(2);
        else if (ARG_DSESI32_ESEDI32) _SetAddressSize(4);
        else _ser_;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void TEST_AL_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("TEST_AL_I8");
    _c_setbyte(0xa8);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(aopri2.imm8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void TEST_EAX_I32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("TEST_EAX_I32");
    _SetOperandSize(byte);
    _c_setbyte(0xa9);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri2.imm16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm32(aopri2.imm32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void STOSB() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("STOSB");
    _c_setbyte(0xaa);
    rinfo = NULL;
    if (ARG_NONE) ;
    else if (ARG_ESDI8) _SetAddressSize(2);
    else if (ARG_ESEDI8) _SetAddressSize(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void STOSW(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("STOSW");
    _SetOperandSize(byte);
    _c_setbyte(0xab);
    rinfo = NULL;
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        if (ARG_NONE) ;
        else if (ARG_ESDI16) _SetAddressSize(2);
        else if (ARG_ESEDI16) _SetAddressSize(4);
        else _ser_;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        if (ARG_NONE) ;
        else if (ARG_ESDI32) _SetAddressSize(2);
        else if (ARG_ESEDI32) _SetAddressSize(4);
        else _ser_;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LODSB() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LODSB");
    _c_setbyte(0xac);
    rinfo = &aopri1;
    if (rinfo->flagds) rinfo->flagds = 0;
    if (ARG_NONE) rinfo = NULL;
    else if (ARG_DSSI8) _SetAddressSize(2);
    else if (ARG_DSESI8) _SetAddressSize(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LODSW(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LODSW");
    _SetOperandSize(byte);
    _c_setbyte(0xad);
    rinfo = &aopri1;
    if (rinfo->flagds) rinfo->flagds = 0;
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        if (ARG_NONE) rinfo = NULL;
        else if (ARG_DSSI16) _SetAddressSize(2);
        else if (ARG_DSESI16) _SetAddressSize(4);
        else _ser_;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        if (ARG_NONE) rinfo = NULL;
        else if (ARG_DSSI32) _SetAddressSize(2);
        else if (ARG_DSESI32) _SetAddressSize(4);
        else _ser_;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SCASB() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SCASB");
    _c_setbyte(0xae);
    rinfo = NULL;
    if (ARG_NONE) ;
    else if (ARG_ESDI8) _SetAddressSize(2);
    else if (ARG_ESEDI8) _SetAddressSize(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SCASW(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SCASW");
    _SetOperandSize(byte);
    _c_setbyte(0xaf);
    rinfo = NULL;
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        if (ARG_NONE) ;
        else if (ARG_ESDI16) _SetAddressSize(2);
        else if (ARG_ESEDI16) _SetAddressSize(4);
        else _ser_;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        if (ARG_NONE) ;
        else if (ARG_ESDI32) _SetAddressSize(2);
        else if (ARG_ESEDI32) _SetAddressSize(4);
        else _ser_;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_AL_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_AL_I8");
    _c_setbyte(0xb0);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(aopri2.imm8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_CL_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_CL_I8");
    _c_setbyte(0xb1);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(aopri2.imm8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_DL_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_DL_I8");
    _c_setbyte(0xb2);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(aopri2.imm8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_BL_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_BL_I8");
    _c_setbyte(0xb3);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(aopri2.imm8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_AH_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_AH_I8");
    _c_setbyte(0xb4);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(aopri2.imm8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_CH_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_CH_I8");
    _c_setbyte(0xb5);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(aopri2.imm8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_DH_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_DH_I8");
    _c_setbyte(0xb6);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(aopri2.imm8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_BH_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_BH_I8");
    _c_setbyte(0xb7);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(aopri2.imm8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_EAX_I32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_EAX_I32");
    _SetOperandSize(byte);
    _c_setbyte(0xb8);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri2.imm16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm32(aopri2.imm32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_ECX_I32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_ECX_I32");
    _SetOperandSize(byte);
    _c_setbyte(0xb9);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri2.imm16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm32(aopri2.imm32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_EDX_I32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_EDX_I32");
    _SetOperandSize(byte);
    _c_setbyte(0xba);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri2.imm16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm32(aopri2.imm32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_EBX_I32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_EBX_I32");
    _SetOperandSize(byte);
    _c_setbyte(0xbb);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri2.imm16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm32(aopri2.imm32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_ESP_I32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_ESP_I32");
    _SetOperandSize(byte);
    _c_setbyte(0xbc);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri2.imm16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm32(aopri2.imm32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_EBP_I32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_EBP_I32");
    _SetOperandSize(byte);
    _c_setbyte(0xbd);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri2.imm16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm32(aopri2.imm32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_ESI_I32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_ESI_I32");
    _SetOperandSize(byte);
    _c_setbyte(0xbe);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri2.imm16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm32(aopri2.imm32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_EDI_I32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_EDI_I32");
    _SetOperandSize(byte);
    _c_setbyte(0xbf);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri2.imm16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm32(aopri2.imm32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_C0(uint8_t rid) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_C0");
    _c_setbyte(0xc0);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, rid));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(aopri2.imm8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_C1(uint8_t rid, uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_C1");
    _SetOperandSize(byte);
    _c_setbyte(0xc1);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, rid));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(aopri2.imm8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void RET_I16() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("RET_I16");
    _c_setbyte(0xc2);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri1.imm16));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void RET_() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("RET");
    _c_setbyte(0xc3);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LES_R32_M16_32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LES_R32_M16_32");
    _SetOperandSize(byte);
    _c_setbyte(0xc4);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LDS_R32_M16_32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LDS_R32_M16_32");
    _c_setbyte(0xc5);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_C6(uint8_t rid) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_C6");
    _c_setbyte(0xc6);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, rid));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(aopri2.imm8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_C7(uint8_t rid, uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_C7");
    _SetOperandSize(byte);
    _c_setbyte(0xc7);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, rid));
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri2.imm16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm32(aopri2.imm32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void ENTER() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ENTER");
    if (ARG_I16_I8) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ARG_I16_I8");
        _c_setbyte(0xc8);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri1.imm16));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(aopri2.imm8));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LEAVE() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LEAVE");
    if (ARG_NONE) _c_setbyte(0xc9);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void RETF_I16() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("RETF_I16");
    _c_setbyte(0xca);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri1.imm16));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void RETF_() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("RETF_");
    _c_setbyte(0xcb);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INT3() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INT3");
    _c_setbyte(0xcc);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INT_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INT_I8");
    _c_setbyte(0xcd);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(aopri1.imm8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INTO() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INTO");
    _c_setbyte(0xcd);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void IRET(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("IRET");
    _SetOperandSize(byte);
    _c_setbyte(0xcf);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_D0(uint8_t rid) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_DO");
    _c_setbyte(0xd0);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, rid));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_D1(uint8_t rid, uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_D1");
    _SetOperandSize(byte);
    _c_setbyte(0xd1);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, rid));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_D2(uint8_t rid) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_D2");
    _c_setbyte(0xd2);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, rid));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_D3(uint8_t rid, uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_D3");
    _SetOperandSize(byte);
    _c_setbyte(0xd3);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, rid));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void AAM() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("AAM");
    if (ARG_NONE) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ARG_NONE");
        _c_setbyte(0xd4);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(0x0a));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else if (ARG_I8) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ARG_I8");
        _c_setbyte(0xd4);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(aopri1.imm8));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void AAD() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("AAD");
    if (ARG_NONE) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ARG_NONE");
        _c_setbyte(0xd5);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(0x0a));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else if (ARG_I8) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ARG_I8");
        _c_setbyte(0xd5);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(aopri1.imm8));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XLATB() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XLATB");
    rinfo = &aopri1;
    if (rinfo->flagds) rinfo->flagds = 0;
    _c_setbyte(0xd7);
    if (ARG_DSBXAL8) {
        _SetAddressSize(2);
    } else if (ARG_DSEBXAL8) {
        _SetAddressSize(4);
    } else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void IN_AL_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("IN_AL_I8");
    _c_setbyte(0xe4);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(aopri2.imm8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void IN_EAX_I8(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("IN_AL_I8");
    _SetOperandSize(byte);
    _c_setbyte(0xe5);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(aopri2.imm8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void OUT_I8_AL() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OUT_I8_AL");
    _c_setbyte(0xe6);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(aopri1.imm8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void OUT_I8_EAX(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OUT_I8_EAX");
    _c_setbyte(0xe7);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(aopri1.imm8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CALL_REL32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CALL_REL32");
    _SetOperandSize(byte);
    _c_setbyte(0xe8);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri1.imm16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm32(aopri1.imm32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JMP_REL32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JMP_REL32");
    _SetOperandSize(byte);
    _c_setbyte(0xe9);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri1.imm16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm32(aopri1.imm32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JMP_PTR16_32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JMP_PTR16_32");
    _SetOperandSize(byte);
    _c_setbyte(0xea);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16((uint16_t) aopri1.reip));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm32((uint32_t) aopri1.reip));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri1.rcs));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void IN_AL_DX() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("IN_AL_DX");
    _c_setbyte(0xec);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void IN_EAX_DX(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("IN_EAX_DX");
    _SetOperandSize(byte);
    _c_setbyte(0xed);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void OUT_DX_AL() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OUT_DX_AL");
    _c_setbyte(0xee);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void OUT_DX_EAX(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OUT_DX_EAX");
    _SetOperandSize(byte);
    _c_setbyte(0xef);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PREFIX_LOCK() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PREFIX_LOCK");
    if (ARG_NONE) prefix_lock = 1;
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void QDX() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PREFIX_LOCK");
    if (ARG_I8) {
        _c_setbyte(0xf1);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(aopri1.imm8));
    } else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PREFIX_REPNZ() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PREFIX_REPNZ");
    if (ARG_NONE) prefix_repnz = 1;
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PREFIX_REPZ() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PREFIX_REPZ");
    if (ARG_NONE) prefix_repz = 1;
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void HLT() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("HLT");
    if (ARG_NONE) _c_setbyte(0xf4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CMC() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CMC");
    if (ARG_NONE) _c_setbyte(0xf5);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_F6(uint8_t rid) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_F6");
    _c_setbyte(0xf6);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, rid));
    if (!rid) NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(aopri2.imm8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_F7(uint8_t rid, uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_F7");
    _SetOperandSize(byte);
    _c_setbyte(0xf7);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, rid));
    if (!rid) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!rid");
        switch (byte) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm16(aopri2.imm16));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm32(aopri2.imm32));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            _ser_;
            break;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CLC() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CLC");
    if (ARG_NONE) _c_setbyte(0xf8);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void STC() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("STC");
    if (ARG_NONE) _c_setbyte(0xf9);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CLI() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CLI");
    if (ARG_NONE) _c_setbyte(0xfa);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void STI() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("STI");
    if (ARG_NONE) _c_setbyte(0xfb);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CLD() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CLD");
    if (ARG_NONE) _c_setbyte(0xfc);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void STD() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("STD");
    if (ARG_NONE) _c_setbyte(0xfd);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_FE(uint8_t rid) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_FE");
    _c_setbyte(0xfe);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, rid));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_FF(uint8_t rid, uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_FF");
    _SetOperandSize(byte);
    _c_setbyte(0xff);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, rid));
    NTVDM64_TYPE_TRACE_CALL_END;
}
/* concrete extended instructions */
static void INS_0F_00(uint8_t rid, uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_0F_00");
    _SetOperandSize(byte);
    INS_0F();
    _c_setbyte(0x00);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, rid));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_0F_01(uint8_t rid, uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_0F_01");
    _SetOperandSize(byte);
    INS_0F();
    _c_setbyte(0x01);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, rid));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LAR_R32_RM32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LAR_R32_RM32");
    _SetOperandSize(byte);
    INS_0F();
    _c_setbyte(0x02);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LSL_R32_RM32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LSL_R32_RM32");
    _SetOperandSize(byte);
    INS_0F();
    _c_setbyte(0x03);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CLTS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CLTS");
    INS_0F();
    _c_setbyte(0x06);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_R32_CR(uint8_t crid) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_R32_CR");
    INS_0F();
    _c_setbyte(0x20);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, crid));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_R32_DR(uint8_t drid) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_R32_DR");
    INS_0F();
    _c_setbyte(0x21);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, drid));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_CR_R32(uint8_t crid) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_CR_R32");
    INS_0F();
    _c_setbyte(0x22);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, crid));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_DR_R32(uint8_t drid) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_DR_R32");
    INS_0F();
    _c_setbyte(0x23);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, drid));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_R32_TR(uint8_t trid) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_R32_TR");
    INS_0F();
    _c_setbyte(0x24);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, trid));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_TR_R32(uint8_t trid) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_TR_R32");
    INS_0F();
    _c_setbyte(0x26);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, trid));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SETCC_RM8(uint8_t opcode) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SETCC_RM8");
    if (ARG_RM8) {
        INS_0F();
        _c_setbyte(opcode);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, 0));
    } else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PUSH_FS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_FS");
    INS_0F();
    _c_setbyte(0xa0);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void POP_FS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_FS");
    INS_0F();
    _c_setbyte(0xa1);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void BT_RM32_R32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("BT_RM32_R32");
    _SetOperandSize(byte);
    INS_0F();
    _c_setbyte(0xa3);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg16));
        break;
    case 4:
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg32));
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SHLD_RM32_R32_I8(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SHLD_RM32_R32_I8");
    _SetOperandSize(byte);
    INS_0F();
    _c_setbyte(0xa4);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg16));
        break;
    case 4:
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg32));
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(aopri3.imm8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SHLD_RM32_R32_CL(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SHLD_RM32_R32_CL");
    _SetOperandSize(byte);
    INS_0F();
    _c_setbyte(0xa5);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg16));
        break;
    case 4:
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg32));
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PUSH_GS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_GS");
    INS_0F();
    _c_setbyte(0xa8);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void POP_GS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_GS");
    INS_0F();
    _c_setbyte(0xa9);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void BTS_RM32_R32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("BTS_RM32_R32");
    _SetOperandSize(byte);
    INS_0F();
    _c_setbyte(0xab);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg16));
        break;
    case 4:
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg32));
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SHRD_RM32_R32_I8(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SHRD_RM32_R32_I8");
    _SetOperandSize(byte);
    INS_0F();
    _c_setbyte(0xac);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg16));
        break;
    case 4:
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg32));
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(aopri3.imm8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SHRD_RM32_R32_CL(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SHRD_RM32_R32_CL");
    _SetOperandSize(byte);
    INS_0F();
    _c_setbyte(0xad);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg16));
        break;
    case 4:
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg32));
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void IMUL_R32_RM32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("IMUL_R32_RM32");
    _SetOperandSize(byte);
    INS_0F();
    _c_setbyte(0xab);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg16));
        break;
    case 4:
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg32));
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LSS_R32_M16_32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LSS_R32_M16_32");
    _SetOperandSize(byte);
    INS_0F();
    _c_setbyte(0xb2);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void BTR_RM32_R32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("BTR_RM32_R32");
    _SetOperandSize(byte);
    INS_0F();
    _c_setbyte(0xb3);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg16));
        break;
    case 4:
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg32));
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LFS_R32_M16_32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LFS_R32_M16_32");
    _SetOperandSize(byte);
    INS_0F();
    _c_setbyte(0xb4);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LGS_R32_M16_32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LGS_R32_M16_32");
    _SetOperandSize(byte);
    INS_0F();
    _c_setbyte(0xb5);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOVZX_R32_RM8(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOVZX_R32_RM8");
    _SetOperandSize(byte);
    INS_0F();
    _c_setbyte(0xb6);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOVZX_R32_RM16() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOVZX_R32_RM16");
    INS_0F();
    _c_setbyte(0xb7);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg32));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_0F_BA(uint8_t rid, uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_0F_BA");
    _SetOperandSize(byte);
    INS_0F();
    _c_setbyte(0xba);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, rid));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_imm8(aopri2.imm8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void BTC_RM32_R32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("BTC_RM32_R32");
    _SetOperandSize(byte);
    INS_0F();
    _c_setbyte(0xbb);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg16));
        break;
    case 4:
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri1, aopri2.reg32));
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void BSF_R32_RM32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("BSF_R32_RM32");
    _SetOperandSize(byte);
    INS_0F();
    _c_setbyte(0xbc);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void BSR_R32_RM32(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("BSR_R32_RM32");
    _SetOperandSize(byte);
    INS_0F();
    _c_setbyte(0xbd);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOVSX_R32_RM8(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOVSX_R32_RM8");
    _SetOperandSize(byte);
    INS_0F();
    _c_setbyte(0xbe);
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg16));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg32));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOVSX_R32_RM16() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOVSX_R32_RM16");
    INS_0F();
    _c_setbyte(0xbf);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_c_modrm(aopri2, aopri1.reg32));
    NTVDM64_TYPE_TRACE_CALL_END;
}

/* abstract instructions */
static void PUSH() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH");
    if      (ARG_ES) PUSH_ES();
    else if (ARG_CS) PUSH_CS();
    else if (ARG_SS) PUSH_SS();
    else if (ARG_DS) PUSH_DS();
    else if (ARG_FS) PUSH_FS();
    else if (ARG_GS) PUSH_GS();
    else if (ARG_AX) PUSH_EAX(2);
    else if (ARG_CX) PUSH_ECX(2);
    else if (ARG_DX) PUSH_EDX(2);
    else if (ARG_BX) PUSH_EBX(2);
    else if (ARG_SP) PUSH_ESP(2);
    else if (ARG_BP) PUSH_EBP(2);
    else if (ARG_SI) PUSH_ESI(2);
    else if (ARG_DI) PUSH_EDI(2);
    else if (ARG_EAX) PUSH_EAX(4);
    else if (ARG_ECX) PUSH_ECX(4);
    else if (ARG_EDX) PUSH_EDX(4);
    else if (ARG_EBX) PUSH_EBX(4);
    else if (ARG_ESP) PUSH_ESP(4);
    else if (ARG_EBP) PUSH_EBP(4);
    else if (ARG_ESI) PUSH_ESI(4);
    else if (ARG_EDI) PUSH_EDI(4);
    else if (ARG_RM16) INS_FF(0x06, 2);
    else if (ARG_RM32) INS_FF(0x06, 4);
    else if (ARG_I8)  PUSH_I8();
    else if (ARG_I16) PUSH_I32(2);
    else if (ARG_I32) PUSH_I32(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void POP() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP");
    if      (ARG_ES) POP_ES();
    else if (ARG_CS) POP_CS();
    else if (ARG_SS) POP_SS();
    else if (ARG_DS) POP_DS();
    else if (ARG_FS) POP_FS();
    else if (ARG_GS) POP_GS();
    else if (ARG_AX) POP_EAX(2);
    else if (ARG_CX) POP_ECX(2);
    else if (ARG_DX) POP_EDX(2);
    else if (ARG_BX) POP_EBX(2);
    else if (ARG_SP) POP_ESP(2);
    else if (ARG_BP) POP_EBP(2);
    else if (ARG_SI) POP_ESI(2);
    else if (ARG_DI) POP_EDI(2);
    else if (ARG_EAX) POP_EAX(4);
    else if (ARG_ECX) POP_ECX(4);
    else if (ARG_EDX) POP_EDX(4);
    else if (ARG_EBX) POP_EBX(4);
    else if (ARG_ESP) POP_ESP(4);
    else if (ARG_EBP) POP_EBP(4);
    else if (ARG_ESI) POP_ESI(4);
    else if (ARG_EDI) POP_EDI(4);
    else if (ARG_RM16) INS_8F(0x00, 2);
    else if (ARG_RM32) INS_8F(0x00, 4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void ADD() {
    uint8_t rid = 0x00;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADD");
    if      (ARG_AL_I8)    ADD_AL_I8();
    else if (ARG_AX_I16)   ADD_EAX_I32(2);
    else if (ARG_EAX_I32)  ADD_EAX_I32(4);
    else if (ARG_R8_RM8)   ADD_R8_RM8();
    else if (ARG_R16_RM16) ADD_R32_RM32(2);
    else if (ARG_R32_RM32) ADD_R32_RM32(4);
    else if (ARG_RM8_I8)   INS_80(rid);
    else if (ARG_RM16_I8) {
        if (!aopri2.immn)
            if (aopri2.imm8 > 0x7f) {
                aopri2.type = TYPE_I16;
                aopri2.imm16 = aopri2.imm8;
                INS_81(rid, 2);
            } else INS_83(rid, 2);
        else if (aopri2.imm8 > 0x7f)
            INS_83(rid, 2);
        else {
            aopri2.type = TYPE_I16;
            aopri2.imm16 = 0xff00 | aopri2.imm8;
            INS_81(rid, 2);
        }
    } else if (ARG_RM32_I8)  {
        if (!aopri2.immn)
            if (aopri2.imm8 > 0x7f) {
                aopri2.type = TYPE_I32;
                aopri2.imm32 = aopri2.imm8;
                INS_81(rid, 4);
            } else INS_83(rid, 4);
        else if (aopri2.imm8 > 0x7f)
            INS_83(rid, 4);
        else {
            aopri2.type = TYPE_I32;
            aopri2.imm32 = 0xffffff00 | aopri2.imm8;
            INS_81(rid, 4);
        }
    } else if (ARG_RM16_I16) INS_81(rid, 2);
    else if (ARG_RM32_I32) INS_81(rid, 4);
    else if (ARG_RM8_R8)   ADD_RM8_R8();
    else if (ARG_RM16_R16) ADD_RM32_R32(2);
    else if (ARG_RM32_R32) ADD_RM32_R32(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void OR() {
    uint8_t rid = 0x01;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OR");
    if      (ARG_AL_I8)    OR_AL_I8();
    else if (ARG_AX_I16)   OR_EAX_I32(2);
    else if (ARG_EAX_I32)  OR_EAX_I32(4);
    else if (ARG_R8_RM8)   OR_R8_RM8();
    else if (ARG_R16_RM16) OR_R32_RM32(2);
    else if (ARG_R32_RM32) OR_R32_RM32(4);
    else if (ARG_RM8_I8)   INS_80(rid);
    else if (ARG_RM16_I8) {
        if (!aopri2.immn)
            if (aopri2.imm8 > 0x7f) {
                aopri2.type = TYPE_I16;
                aopri2.imm16 = aopri2.imm8;
                INS_81(rid, 2);
            } else INS_83(rid, 2);
        else if (aopri2.imm8 > 0x7f)
            INS_83(rid, 2);
        else {
            aopri2.type = TYPE_I16;
            aopri2.imm16 = 0xff00 | aopri2.imm8;
            INS_81(rid, 2);
        }
    } else if (ARG_RM32_I8)  {
        if (!aopri2.immn)
            if (aopri2.imm8 > 0x7f) {
                aopri2.type = TYPE_I32;
                aopri2.imm32 = aopri2.imm8;
                INS_81(rid, 4);
            } else INS_83(rid, 4);
        else if (aopri2.imm8 > 0x7f)
            INS_83(rid, 4);
        else {
            aopri2.type = TYPE_I32;
            aopri2.imm32 = 0xffffff00 | aopri2.imm8;
            INS_81(rid, 4);
        }
    } else if (ARG_RM16_I16) INS_81(rid, 2);
    else if (ARG_RM32_I32) INS_81(rid, 4);
    else if (ARG_RM8_R8)   OR_RM8_R8();
    else if (ARG_RM16_R16) OR_RM32_R32(2);
    else if (ARG_RM32_R32) OR_RM32_R32(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void ADC() {
    uint8_t rid = 0x02;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADC");
    if      (ARG_AL_I8)    ADC_AL_I8();
    else if (ARG_AX_I16)   ADC_EAX_I32(2);
    else if (ARG_EAX_I32)  ADC_EAX_I32(4);
    else if (ARG_R8_RM8)   ADC_R8_RM8();
    else if (ARG_R16_RM16) ADC_R32_RM32(2);
    else if (ARG_R32_RM32) ADC_R32_RM32(4);
    else if (ARG_RM8_I8)   INS_80(rid);
    else if (ARG_RM16_I8) {
        if (!aopri2.immn)
            if (aopri2.imm8 > 0x7f) {
                aopri2.type = TYPE_I16;
                aopri2.imm16 = aopri2.imm8;
                INS_81(rid, 2);
            } else INS_83(rid, 2);
        else if (aopri2.imm8 > 0x7f)
            INS_83(rid, 2);
        else {
            aopri2.type = TYPE_I16;
            aopri2.imm16 = 0xff00 | aopri2.imm8;
            INS_81(rid, 2);
        }
    } else if (ARG_RM32_I8)  {
        if (!aopri2.immn)
            if (aopri2.imm8 > 0x7f) {
                aopri2.type = TYPE_I32;
                aopri2.imm32 = aopri2.imm8;
                INS_81(rid, 4);
            } else INS_83(rid, 4);
        else if (aopri2.imm8 > 0x7f)
            INS_83(rid, 4);
        else {
            aopri2.type = TYPE_I32;
            aopri2.imm32 = 0xffffff00 | aopri2.imm8;
            INS_81(rid, 4);
        }
    } else if (ARG_RM16_I16) INS_81(rid, 2);
    else if (ARG_RM32_I32) INS_81(rid, 4);
    else if (ARG_RM8_R8)   ADC_RM8_R8();
    else if (ARG_RM16_R16) ADC_RM32_R32(2);
    else if (ARG_RM32_R32) ADC_RM32_R32(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SBB() {
    uint8_t rid = 0x03;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SBB");
    if      (ARG_AL_I8)    SBB_AL_I8();
    else if (ARG_AX_I16)   SBB_EAX_I32(2);
    else if (ARG_EAX_I32)  SBB_EAX_I32(4);
    else if (ARG_R8_RM8)   SBB_R8_RM8();
    else if (ARG_R16_RM16) SBB_R32_RM32(2);
    else if (ARG_R32_RM32) SBB_R32_RM32(4);
    else if (ARG_RM8_I8)   INS_80(rid);
    else if (ARG_RM16_I8) {
        if (!aopri2.immn)
            if (aopri2.imm8 > 0x7f) {
                aopri2.type = TYPE_I16;
                aopri2.imm16 = aopri2.imm8;
                INS_81(rid, 2);
            } else INS_83(rid, 2);
        else if (aopri2.imm8 > 0x7f)
            INS_83(rid, 2);
        else {
            aopri2.type = TYPE_I16;
            aopri2.imm16 = 0xff00 | aopri2.imm8;
            INS_81(rid, 2);
        }
    } else if (ARG_RM32_I8)  {
        if (!aopri2.immn)
            if (aopri2.imm8 > 0x7f) {
                aopri2.type = TYPE_I32;
                aopri2.imm32 = aopri2.imm8;
                INS_81(rid, 4);
            } else INS_83(rid, 4);
        else if (aopri2.imm8 > 0x7f)
            INS_83(rid, 4);
        else {
            aopri2.type = TYPE_I32;
            aopri2.imm32 = 0xffffff00 | aopri2.imm8;
            INS_81(rid, 4);
        }
    } else if (ARG_RM16_I16) INS_81(rid, 2);
    else if (ARG_RM32_I32) INS_81(rid, 4);
    else if (ARG_RM8_R8)   SBB_RM8_R8();
    else if (ARG_RM16_R16) SBB_RM32_R32(2);
    else if (ARG_RM32_R32) SBB_RM32_R32(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void AND() {
    uint8_t rid = 0x04;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("AND");
    if      (ARG_AL_I8)    AND_AL_I8();
    else if (ARG_AX_I16)   AND_EAX_I32(2);
    else if (ARG_EAX_I32)  AND_EAX_I32(4);
    else if (ARG_R8_RM8)   AND_R8_RM8();
    else if (ARG_R16_RM16) AND_R32_RM32(2);
    else if (ARG_R32_RM32) AND_R32_RM32(4);
    else if (ARG_RM8_I8)   INS_80(rid);
    else if (ARG_RM16_I8) {
        if (!aopri2.immn)
            if (aopri2.imm8 > 0x7f) {
                aopri2.type = TYPE_I16;
                aopri2.imm16 = aopri2.imm8;
                INS_81(rid, 2);
            } else INS_83(rid, 2);
        else if (aopri2.imm8 > 0x7f)
            INS_83(rid, 2);
        else {
            aopri2.type = TYPE_I16;
            aopri2.imm16 = 0xff00 | aopri2.imm8;
            INS_81(rid, 2);
        }
    } else if (ARG_RM32_I8)  {
        if (!aopri2.immn)
            if (aopri2.imm8 > 0x7f) {
                aopri2.type = TYPE_I32;
                aopri2.imm32 = aopri2.imm8;
                INS_81(rid, 4);
            } else INS_83(rid, 4);
        else if (aopri2.imm8 > 0x7f)
            INS_83(rid, 4);
        else {
            aopri2.type = TYPE_I32;
            aopri2.imm32 = 0xffffff00 | aopri2.imm8;
            INS_81(rid, 4);
        }
    } else if (ARG_RM16_I16) INS_81(rid, 2);
    else if (ARG_RM32_I32) INS_81(rid, 4);
    else if (ARG_RM8_R8)   AND_RM8_R8();
    else if (ARG_RM16_R16) AND_RM32_R32(2);
    else if (ARG_RM32_R32) AND_RM32_R32(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SUB() {
    uint8_t rid = 0x05;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SUB");
    if      (ARG_AL_I8)    SUB_AL_I8();
    else if (ARG_AX_I16)   SUB_EAX_I32(2);
    else if (ARG_EAX_I32)  SUB_EAX_I32(4);
    else if (ARG_R8_RM8)   SUB_R8_RM8();
    else if (ARG_R16_RM16) SUB_R32_RM32(2);
    else if (ARG_R32_RM32) SUB_R32_RM32(4);
    else if (ARG_RM8_I8)   INS_80(rid);
    else if (ARG_RM16_I8) {
        if (!aopri2.immn)
            if (aopri2.imm8 > 0x7f) {
                aopri2.type = TYPE_I16;
                aopri2.imm16 = aopri2.imm8;
                INS_81(rid, 2);
            } else INS_83(rid, 2);
        else if (aopri2.imm8 > 0x7f)
            INS_83(rid, 2);
        else {
            aopri2.type = TYPE_I16;
            aopri2.imm16 = 0xff00 | aopri2.imm8;
            INS_81(rid, 2);
        }
    } else if (ARG_RM32_I8)  {
        if (!aopri2.immn)
            if (aopri2.imm8 > 0x7f) {
                aopri2.type = TYPE_I32;
                aopri2.imm32 = aopri2.imm8;
                INS_81(rid, 4);
            } else INS_83(rid, 4);
        else if (aopri2.imm8 > 0x7f)
            INS_83(rid, 4);
        else {
            aopri2.type = TYPE_I32;
            aopri2.imm32 = 0xffffff00 | aopri2.imm8;
            INS_81(rid, 4);
        }
    } else if (ARG_RM16_I16) INS_81(rid, 2);
    else if (ARG_RM32_I32) INS_81(rid, 4);
    else if (ARG_RM8_R8)   SUB_RM8_R8();
    else if (ARG_RM16_R16) SUB_RM32_R32(2);
    else if (ARG_RM32_R32) SUB_RM32_R32(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XOR() {
    uint8_t rid = 0x06;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XOR");
    if      (ARG_AL_I8)    XOR_AL_I8();
    else if (ARG_AX_I16)   XOR_EAX_I32(2);
    else if (ARG_EAX_I32)  XOR_EAX_I32(4);
    else if (ARG_R8_RM8)   XOR_R8_RM8();
    else if (ARG_R16_RM16) XOR_R32_RM32(2);
    else if (ARG_R32_RM32) XOR_R32_RM32(4);
    else if (ARG_RM8_I8)   INS_80(rid);
    else if (ARG_RM16_I8) {
        if (!aopri2.immn)
            if (aopri2.imm8 > 0x7f) {
                aopri2.type = TYPE_I16;
                aopri2.imm16 = aopri2.imm8;
                INS_81(rid, 2);
            } else INS_83(rid, 2);
        else if (aopri2.imm8 > 0x7f)
            INS_83(rid, 2);
        else {
            aopri2.type = TYPE_I16;
            aopri2.imm16 = 0xff00 | aopri2.imm8;
            INS_81(rid, 2);
        }
    } else if (ARG_RM32_I8)  {
        if (!aopri2.immn)
            if (aopri2.imm8 > 0x7f) {
                aopri2.type = TYPE_I32;
                aopri2.imm32 = aopri2.imm8;
                INS_81(rid, 4);
            } else INS_83(rid, 4);
        else if (aopri2.imm8 > 0x7f)
            INS_83(rid, 4);
        else {
            aopri2.type = TYPE_I32;
            aopri2.imm32 = 0xffffff00 | aopri2.imm8;
            INS_81(rid, 4);
        }
    } else if (ARG_RM16_I16) INS_81(rid, 2);
    else if (ARG_RM32_I32) INS_81(rid, 4);
    else if (ARG_RM8_R8)   XOR_RM8_R8();
    else if (ARG_RM16_R16) XOR_RM32_R32(2);
    else if (ARG_RM32_R32) XOR_RM32_R32(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CMP() {
    uint8_t rid = 0x07;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CMP");
    if      (ARG_AL_I8)    CMP_AL_I8();
    else if (ARG_AX_I16)   CMP_EAX_I32(2);
    else if (ARG_EAX_I32)  CMP_EAX_I32(4);
    else if (ARG_R8_RM8)   CMP_R8_RM8();
    else if (ARG_R16_RM16) CMP_R32_RM32(2);
    else if (ARG_R32_RM32) CMP_R32_RM32(4);
    else if (ARG_RM8_I8)   INS_80(rid);
    else if (ARG_RM16_I8) {
        if (!aopri2.immn)
            if (aopri2.imm8 > 0x7f) {
                aopri2.type = TYPE_I16;
                aopri2.imm16 = aopri2.imm8;
                INS_81(rid, 2);
            } else INS_83(rid, 2);
        else if (aopri2.imm8 > 0x7f)
            INS_83(rid, 2);
        else {
            aopri2.type = TYPE_I16;
            aopri2.imm16 = 0xff00 | aopri2.imm8;
            INS_81(rid, 2);
        }
    } else if (ARG_RM32_I8)  {
        if (!aopri2.immn)
            if (aopri2.imm8 > 0x7f) {
                aopri2.type = TYPE_I32;
                aopri2.imm32 = aopri2.imm8;
                INS_81(rid, 4);
            } else INS_83(rid, 4);
        else if (aopri2.imm8 > 0x7f)
            INS_83(rid, 4);
        else {
            aopri2.type = TYPE_I32;
            aopri2.imm32 = 0xffffff00 | aopri2.imm8;
            INS_81(rid, 4);
        }
    } else if (ARG_RM16_I16) INS_81(rid, 2);
    else if (ARG_RM32_I32) INS_81(rid, 4);
    else if (ARG_RM8_R8)   CMP_RM8_R8();
    else if (ARG_RM16_R16) CMP_RM32_R32(2);
    else if (ARG_RM32_R32) CMP_RM32_R32(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INC() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INC");
    if      (ARG_AX) INC_EAX(2);
    else if (ARG_CX) INC_ECX(2);
    else if (ARG_DX) INC_EDX(2);
    else if (ARG_BX) INC_EBX(2);
    else if (ARG_SP) INC_ESP(2);
    else if (ARG_BP) INC_EBP(2);
    else if (ARG_SI) INC_ESI(2);
    else if (ARG_DI) INC_EDI(2);
    else if (ARG_EAX) INC_EAX(4);
    else if (ARG_ECX) INC_ECX(4);
    else if (ARG_EDX) INC_EDX(4);
    else if (ARG_EBX) INC_EBX(4);
    else if (ARG_ESP) INC_ESP(4);
    else if (ARG_EBP) INC_EBP(4);
    else if (ARG_ESI) INC_ESI(4);
    else if (ARG_EDI) INC_EDI(4);
    else if (ARG_RM8s) INS_FE(0x00);
    else if (ARG_RM16s) INS_FF(0x00, 2);
    else if (ARG_RM32s) INS_FF(0x00, 4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void DEC() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("DEC");
    if      (ARG_AX) DEC_EAX(2);
    else if (ARG_CX) DEC_ECX(2);
    else if (ARG_DX) DEC_EDX(2);
    else if (ARG_BX) DEC_EBX(2);
    else if (ARG_SP) DEC_ESP(2);
    else if (ARG_BP) DEC_EBP(2);
    else if (ARG_SI) DEC_ESI(2);
    else if (ARG_DI) DEC_EDI(2);
    else if (ARG_EAX) DEC_EAX(4);
    else if (ARG_ECX) DEC_ECX(4);
    else if (ARG_EDX) DEC_EDX(4);
    else if (ARG_EBX) DEC_EBX(4);
    else if (ARG_ESP) DEC_ESP(4);
    else if (ARG_EBP) DEC_EBP(4);
    else if (ARG_ESI) DEC_ESI(4);
    else if (ARG_EDI) DEC_EDI(4);
    else if (ARG_RM8s) INS_FE(0x01);
    else if (ARG_RM16s) INS_FF(0x01, 2);
    else if (ARG_RM32s) INS_FF(0x01, 4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void BOUND() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("BOUND");
    if (ARG_R16_M16) BOUND_R32_M32_32(2);
    else if (ARG_R32_M32) BOUND_R32_M32_32(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void IMUL() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("IMUL");
    if      (ARG_RM8s) INS_F6(0x05);
    else if (ARG_RM16s) INS_F7(0x05, 2);
    else if (ARG_RM32s) INS_F7(0x05, 4);
    else if (ARG_R16_RM16) IMUL_R32_RM32(2);
    else if (ARG_R32_RM32) IMUL_R32_RM32(4);
    else if (ARG_R16_RM16_I8)  IMUL_R32_RM32_I8(2);
    else if (ARG_R32_RM32_I8)  IMUL_R32_RM32_I8(4);
    else if (ARG_R16_RM16_I16) IMUL_R32_RM32_I32(2);
    else if (ARG_R32_RM32_I32) IMUL_R32_RM32_I32(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS");
    if (ARG_ESDI8s_DX || ARG_ESEDI8s_DX) INSB();
    else if (ARG_ESDI16s_DX || ARG_ESEDI16s_DX) INSW(2);
    else if (ARG_ESDI32s_DX || ARG_ESEDI32s_DX) INSW(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void OUTS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OUTS");
    if (ARG_DX_DSSI8s || ARG_DX_DSESI8s) OUTSB();
    else if (ARG_DX_DSSI16s || ARG_DX_DSESI16s) OUTSW(2);
    else if (ARG_DX_DSSI32s || ARG_DX_DSESI32s) OUTSW(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JCC_REL(uint8_t opcode) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JCC_REL");
    if (ARG_PNONE_I8s || ARG_SHORT_I8s) {
        _c_setbyte(opcode);
        _c_imm8(aopri1.imm8);
    } else if (ARG_PNONE_I16s || ARG_NEAR_I16s) {
        _SetOperandSize(2);
        INS_0F();
        _c_setbyte(opcode + 0x10);
        _c_imm16(aopri1.imm16);
    } else if (ARG_PNONE_I32s || ARG_NEAR_I32s) {
        _SetOperandSize(4);
        INS_0F();
        _c_setbyte(opcode + 0x10);
        _c_imm16(aopri1.imm32);
    } else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void TEST() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("TEST");
    if      (ARG_AL_I8) TEST_AL_I8();
    else if (ARG_AX_I16) TEST_EAX_I32(2);
    else if (ARG_EAX_I32) TEST_EAX_I32(4);
    else if (ARG_RM8_R8) TEST_RM8_R8();
    else if (ARG_RM16_R16) TEST_RM32_R32(2);
    else if (ARG_RM32_R32) TEST_RM32_R32(4);
    else if (ARG_RM8_I8) INS_F6(0x00);
    else if (ARG_RM16_I16) INS_F7(0x00, 2);
    else if (ARG_RM32_I32) INS_F7(0x00, 4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XCHG() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XCHG");
    if      (ARG_AX_AX) XCHG_EAX_EAX(2);
    else if (ARG_CX_AX) XCHG_ECX_EAX(2);
    else if (ARG_DX_AX) XCHG_EDX_EAX(2);
    else if (ARG_BX_AX) XCHG_EBX_EAX(2);
    else if (ARG_SP_AX) XCHG_ESP_EAX(2);
    else if (ARG_BP_AX) XCHG_EBP_EAX(2);
    else if (ARG_SI_AX) XCHG_ESI_EAX(2);
    else if (ARG_DI_AX) XCHG_EDI_EAX(2);
    else if (ARG_EAX_EAX) XCHG_EAX_EAX(4);
    else if (ARG_ECX_EAX) XCHG_ECX_EAX(4);
    else if (ARG_EDX_EAX) XCHG_EDX_EAX(4);
    else if (ARG_EBX_EAX) XCHG_EBX_EAX(4);
    else if (ARG_ESP_EAX) XCHG_ESP_EAX(4);
    else if (ARG_EBP_EAX) XCHG_EBP_EAX(4);
    else if (ARG_ESI_EAX) XCHG_ESI_EAX(4);
    else if (ARG_EDI_EAX) XCHG_EDI_EAX(4);
    else if (ARG_RM8_R8) XCHG_RM8_R8();
    else if (ARG_RM16_R16) XCHG_RM32_R32(2);
    else if (ARG_RM32_R32) XCHG_RM32_R32(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV");
    if      (ARG_AL_I8)  MOV_AL_I8();
    else if (ARG_CL_I8)  MOV_CL_I8();
    else if (ARG_DL_I8)  MOV_DL_I8();
    else if (ARG_BL_I8)  MOV_BL_I8();
    else if (ARG_AH_I8)  MOV_AH_I8();
    else if (ARG_CH_I8)  MOV_CH_I8();
    else if (ARG_DH_I8)  MOV_DH_I8();
    else if (ARG_BH_I8)  MOV_BH_I8();
    else if (ARG_AX_I16) MOV_EAX_I32(2);
    else if (ARG_CX_I16) MOV_ECX_I32(2);
    else if (ARG_DX_I16) MOV_EDX_I32(2);
    else if (ARG_BX_I16) MOV_EBX_I32(2);
    else if (ARG_SP_I16) MOV_ESP_I32(2);
    else if (ARG_BP_I16) MOV_EBP_I32(2);
    else if (ARG_SI_I16) MOV_ESI_I32(2);
    else if (ARG_DI_I16) MOV_EDI_I32(2);
    else if (ARG_EAX_I32) MOV_EAX_I32(4);
    else if (ARG_ECX_I32) MOV_ECX_I32(4);
    else if (ARG_EDX_I32) MOV_EDX_I32(4);
    else if (ARG_EBX_I32) MOV_EBX_I32(4);
    else if (ARG_ESP_I32) MOV_ESP_I32(4);
    else if (ARG_EBP_I32) MOV_EBP_I32(4);
    else if (ARG_ESI_I32) MOV_ESI_I32(4);
    else if (ARG_EDI_I32) MOV_EDI_I32(4);
    else if (ARG_AL_MOFFS8)   MOV_AL_MOFFS8();
    else if (ARG_MOFFS8_AL)   MOV_MOFFS8_AL();
    else if (ARG_AX_MOFFS16)  MOV_EAX_MOFFS32(2);
    else if (ARG_MOFFS16_AX)  MOV_MOFFS32_EAX(2);
    else if (ARG_EAX_MOFFS32) MOV_EAX_MOFFS32(4);
    else if (ARG_MOFFS32_EAX) MOV_MOFFS32_EAX(4);
    else if (ARG_R8_RM8)   MOV_R8_RM8();
    else if (ARG_R16_RM16) MOV_R32_RM32(2);
    else if (ARG_R32_RM32) MOV_R32_RM32(4);
    else if (ARG_RM8_R8)   MOV_RM8_R8();
    else if (ARG_RM16_R16) MOV_RM32_R32(2);
    else if (ARG_RM32_R32) MOV_RM32_R32(4);
    else if (ARG_RM16_SREG) MOV_RM16_SREG(2);
    else if (ARG_RM32_SREG) MOV_RM16_SREG(4);
    else if (ARG_SREG_RM16) MOV_SREG_RM16(2);
    else if (ARG_SREG_RM32) MOV_SREG_RM16(4);
    else if (ARG_RM8_I8)   INS_C6(0x00);
    else if (ARG_RM16_I16) INS_C7(0x00, 2);
    else if (ARG_RM32_I32) INS_C7(0x00, 4);
    else if (ARG_R32_CR0)  MOV_R32_CR(0);
    else if (ARG_R32_CR2)  MOV_R32_CR(2);
    else if (ARG_R32_CR3)  MOV_R32_CR(3);
    else if (ARG_CR0_R32)  MOV_CR_R32(0);
    else if (ARG_CR2_R32)  MOV_CR_R32(2);
    else if (ARG_CR3_R32)  MOV_CR_R32(3);
    else if (ARG_R32_DR0)  MOV_R32_DR(0);
    else if (ARG_R32_DR1)  MOV_R32_DR(1);
    else if (ARG_R32_DR2)  MOV_R32_DR(2);
    else if (ARG_R32_DR3)  MOV_R32_DR(3);
    else if (ARG_R32_DR6)  MOV_R32_DR(6);
    else if (ARG_R32_DR7)  MOV_R32_DR(7);
    else if (ARG_DR0_R32)  MOV_DR_R32(0);
    else if (ARG_DR1_R32)  MOV_DR_R32(1);
    else if (ARG_DR2_R32)  MOV_DR_R32(2);
    else if (ARG_DR3_R32)  MOV_DR_R32(3);
    else if (ARG_DR6_R32)  MOV_DR_R32(6);
    else if (ARG_DR7_R32)  MOV_DR_R32(7);
    else if (ARG_R32_DR6)  MOV_R32_TR(6);
    else if (ARG_R32_DR7)  MOV_R32_TR(7);
    else if (ARG_TR6_R32)  MOV_TR_R32(6);
    else if (ARG_TR7_R32)  MOV_TR_R32(7);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LEA() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LEA");
    if (ARG_R16_M16) LEA_R32_M32(2);
    else if (ARG_R32_M32) LEA_R32_M32(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CALL() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CALL");
    if      (ARG_FAR_I16_16) CALL_PTR16_32(2);
    else if (ARG_FAR_I16_32) CALL_PTR16_32(4);
    else if (ARG_NEAR_I16s  || ARG_PNONE_I16s) CALL_REL32(2);
    else if (ARG_NEAR_I32s  || ARG_PNONE_I32s) CALL_REL32(4);
    else if (ARG_NEAR_RM16s || ARG_PNONE_RM16s) INS_FF(0x02, 2);
    else if (ARG_NEAR_RM32s || ARG_PNONE_RM32s) INS_FF(0x02, 4);
    else if (ARG_FAR_M16_16) INS_FF(0x03, 2);
    else if (ARG_FAR_M16_32) INS_FF(0x03, 4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOVS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOVS");
    if (ARG_ESDI8s_DSSI8s || ARG_ESEDI8s_DSESI8s) MOVSB();
    else if (ARG_ESDI16s_DSSI16s || ARG_ESEDI16s_DSESI16s) MOVSW(2);
    else if (ARG_ESDI32s_DSSI32s || ARG_ESEDI32s_DSESI32s) MOVSW(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CMPS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CMPS");
    if (ARG_DSSI8s_ESDI8s || ARG_DSESI8s_ESEDI8s) CMPSB();
    else if (ARG_DSSI16s_ESDI16s || ARG_DSESI16s_ESEDI16s) CMPSW(2);
    else if (ARG_DSSI32s_ESDI32s || ARG_DSESI32s_ESEDI32s) CMPSW(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void STOS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("STOS");
    if (ARG_ESDI8s || ARG_ESEDI8s) STOSB();
    else if (ARG_ESDI16s || ARG_ESEDI16s) STOSW(2);
    else if (ARG_ESDI32s || ARG_ESEDI32s) STOSW(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LODS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LODS");
    if (ARG_DSSI8s || ARG_DSESI8s) LODSB();
    else if (ARG_DSSI16s || ARG_DSESI16s) LODSW(2);
    else if (ARG_DSSI32s || ARG_DSESI32s) LODSW(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SCAS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SCAS");
    if (ARG_ESDI8s || ARG_ESEDI8s) SCASB();
    else if (ARG_ESDI16s || ARG_ESEDI16s) SCASW(2);
    else if (ARG_ESDI32s || ARG_ESEDI32s) SCASW(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void RET() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("RET");
    if (ARG_I16u) RET_I16();
    else if (ARG_NONE) RET_();
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LES() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LES");
    if (ARG_R16_M16) LES_R32_M16_32(2);
    else if (ARG_R32_M32) LES_R32_M16_32(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LDS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LDS");
    if (ARG_R16_M16) LDS_R32_M16_32(2);
    else if (ARG_R32_M32) LDS_R32_M16_32(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void RETF() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("RETF");
    if      (ARG_I16u) RETF_I16();
    else if (ARG_NONE) RETF_();
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INT() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INT");
    if (ARG_I8) INT_I8();
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XLAT() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XLAT");
    if (ARG_DSBXAL8 || ARG_DSEBXAL8) XLATB();
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void ROL() {
    uint8_t rid = 0x00;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ROL");
    if (ARG_RM8_I8 && aopri2.imm8 == 1) INS_D0(rid);
    else if (ARG_RM16_I8 && aopri2.imm8 == 1) INS_D1(rid, 2);
    else if (ARG_RM32_I8 && aopri2.imm8 == 1) INS_D1(rid, 4);
    else if (ARG_RM8_CL)  INS_D2(rid);
    else if (ARG_RM16_CL) INS_D3(rid, 2);
    else if (ARG_RM32_CL) INS_D3(rid, 4);
    else if (ARG_RM8_I8)  INS_C0(rid);
    else if (ARG_RM16_I8) INS_C1(rid, 2);
    else if (ARG_RM32_I8) INS_C1(rid, 4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void ROR() {
    uint8_t rid = 0x01;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ROR");
    if (ARG_RM8_I8 && aopri2.imm8 == 1) INS_D0(rid);
    else if (ARG_RM16_I8 && aopri2.imm8 == 1) INS_D1(rid, 2);
    else if (ARG_RM32_I8 && aopri2.imm8 == 1) INS_D1(rid, 4);
    else if (ARG_RM8_CL)  INS_D2(rid);
    else if (ARG_RM16_CL) INS_D3(rid, 2);
    else if (ARG_RM32_CL) INS_D3(rid, 4);
    else if (ARG_RM8_I8)  INS_C0(rid);
    else if (ARG_RM16_I8) INS_C1(rid, 2);
    else if (ARG_RM32_I8) INS_C1(rid, 4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void RCL() {
    uint8_t rid = 0x02;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("RCL");
    if (ARG_RM8_I8 && aopri2.imm8 == 1) INS_D0(rid);
    else if (ARG_RM16_I8 && aopri2.imm8 == 1) INS_D1(rid, 2);
    else if (ARG_RM32_I8 && aopri2.imm8 == 1) INS_D1(rid, 4);
    else if (ARG_RM8_CL)  INS_D2(rid);
    else if (ARG_RM16_CL) INS_D3(rid, 2);
    else if (ARG_RM32_CL) INS_D3(rid, 4);
    else if (ARG_RM8_I8)  INS_C0(rid);
    else if (ARG_RM16_I8) INS_C1(rid, 2);
    else if (ARG_RM32_I8) INS_C1(rid, 4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void RCR() {
    uint8_t rid = 0x03;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("IN");
    if (ARG_RM8_I8 && aopri2.imm8 == 1) INS_D0(rid);
    else if (ARG_RM16_I8 && aopri2.imm8 == 1) INS_D1(rid, 2);
    else if (ARG_RM32_I8 && aopri2.imm8 == 1) INS_D1(rid, 4);
    else if (ARG_RM8_CL)  INS_D2(rid);
    else if (ARG_RM16_CL) INS_D3(rid, 2);
    else if (ARG_RM32_CL) INS_D3(rid, 4);
    else if (ARG_RM8_I8)  INS_C0(rid);
    else if (ARG_RM16_I8) INS_C1(rid, 2);
    else if (ARG_RM32_I8) INS_C1(rid, 4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SHL() {
    uint8_t rid = 0x04;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SHL");
    if (ARG_RM8_I8 && aopri2.imm8 == 1) INS_D0(rid);
    else if (ARG_RM16_I8 && aopri2.imm8 == 1) INS_D1(rid, 2);
    else if (ARG_RM32_I8 && aopri2.imm8 == 1) INS_D1(rid, 4);
    else if (ARG_RM8_CL)  INS_D2(rid);
    else if (ARG_RM16_CL) INS_D3(rid, 2);
    else if (ARG_RM32_CL) INS_D3(rid, 4);
    else if (ARG_RM8_I8)  INS_C0(rid);
    else if (ARG_RM16_I8) INS_C1(rid, 2);
    else if (ARG_RM32_I8) INS_C1(rid, 4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SHR() {
    uint8_t rid = 0x05;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SHR");
    if (ARG_RM8_I8 && aopri2.imm8 == 1) INS_D0(rid);
    else if (ARG_RM16_I8 && aopri2.imm8 == 1) INS_D1(rid, 2);
    else if (ARG_RM32_I8 && aopri2.imm8 == 1) INS_D1(rid, 4);
    else if (ARG_RM8_CL)  INS_D2(rid);
    else if (ARG_RM16_CL) INS_D3(rid, 2);
    else if (ARG_RM32_CL) INS_D3(rid, 4);
    else if (ARG_RM8_I8)  INS_C0(rid);
    else if (ARG_RM16_I8) INS_C1(rid, 2);
    else if (ARG_RM32_I8) INS_C1(rid, 4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SAL() {
    uint8_t rid = 0x04;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SAL");
    if (ARG_RM8_I8 && aopri2.imm8 == 1) INS_D0(rid);
    else if (ARG_RM16_I8 && aopri2.imm8 == 1) INS_D1(rid, 2);
    else if (ARG_RM32_I8 && aopri2.imm8 == 1) INS_D1(rid, 4);
    else if (ARG_RM8_CL)  INS_D2(rid);
    else if (ARG_RM16_CL) INS_D3(rid, 2);
    else if (ARG_RM32_CL) INS_D3(rid, 4);
    else if (ARG_RM8_I8)  INS_C0(rid);
    else if (ARG_RM16_I8) INS_C1(rid, 2);
    else if (ARG_RM32_I8) INS_C1(rid, 4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SAR() {
    uint8_t rid = 0x07;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SAR");
    if (ARG_RM8_I8 && aopri2.imm8 == 1) INS_D0(rid);
    else if (ARG_RM16_I8 && aopri2.imm8 == 1) INS_D1(rid, 2);
    else if (ARG_RM32_I8 && aopri2.imm8 == 1) INS_D1(rid, 4);
    else if (ARG_RM8_CL)  INS_D2(rid);
    else if (ARG_RM16_CL) INS_D3(rid, 2);
    else if (ARG_RM32_CL) INS_D3(rid, 4);
    else if (ARG_RM8_I8)  INS_C0(rid);
    else if (ARG_RM16_I8) INS_C1(rid, 2);
    else if (ARG_RM32_I8) INS_C1(rid, 4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void IN() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("IN");
    if      (ARG_AL_I8u)  IN_AL_I8();
    else if (ARG_AX_I8u)  IN_EAX_I8(2);
    else if (ARG_EAX_I8u) IN_EAX_I8(4);
    else if (ARG_AL_DX)   IN_AL_DX();
    else if (ARG_AX_DX)   IN_EAX_DX(2);
    else if (ARG_EAX_DX)  IN_EAX_DX(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void OUT() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OUT");
    if      (ARG_I8u_AL)  OUT_I8_AL();
    else if (ARG_I8u_AX)  OUT_I8_EAX(2);
    else if (ARG_I8u_EAX) OUT_I8_EAX(4);
    else if (ARG_DX_AL)   OUT_DX_AL();
    else if (ARG_DX_AX)   OUT_DX_EAX(2);
    else if (ARG_DX_EAX)  OUT_DX_EAX(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void NOT() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("NOT");
    if      (ARG_RM8s) INS_F6(0x02);
    else if (ARG_RM16s) INS_F7(0x02, 2);
    else if (ARG_RM32s) INS_F7(0x02, 4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void NEG() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("NEG");
    if      (ARG_RM8s) INS_F6(0x03);
    else if (ARG_RM16s) INS_F7(0x03, 2);
    else if (ARG_RM32s) INS_F7(0x03, 4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MUL() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MUL");
    if      (ARG_RM8s) INS_F6(0x04);
    else if (ARG_RM16s) INS_F7(0x04, 2);
    else if (ARG_RM32s) INS_F7(0x04, 4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void DIV() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("DIV");
    if      (ARG_RM8s) INS_F6(0x06);
    else if (ARG_RM16s) INS_F7(0x06, 2);
    else if (ARG_RM32s) INS_F7(0x06, 4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void IDIV() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("IDIV");
    if      (ARG_RM8s) INS_F6(0x07);
    else if (ARG_RM16s) INS_F7(0x07, 2);
    else if (ARG_RM32s) INS_F7(0x07, 4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JMP() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JMP");
    if      (ARG_FAR_I16_16) JMP_PTR16_32(2);
    else if (ARG_FAR_I16_32) JMP_PTR16_32(4);
    else if (ARG_SHORT_I8s  || ARG_PNONE_I8s)  JCC_REL(0xeb);
    else if (ARG_NEAR_I16s  || ARG_PNONE_I16s) JMP_REL32(2);
    else if (ARG_NEAR_I32s  || ARG_PNONE_I32s) JMP_REL32(4);
    else if (ARG_NEAR_RM16s || ARG_PNONE_RM16s) INS_FF(0x04, 2);
    else if (ARG_NEAR_RM32s || ARG_PNONE_RM32s) INS_FF(0x04, 4);
    else if (ARG_FAR_M16_16) INS_FF(0x05, 2);
    else if (ARG_FAR_M16_32) INS_FF(0x05, 4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
/* abstract extended instructions */
static void SLDT() {
    uint8_t rid = 0x00;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SLDT");
    if (ARG_RM16) INS_0F_00(rid, 2);
    else if (ARG_R32) INS_0F_00(rid, 4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void STR() {
    uint8_t rid = 0x01;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("STR");
    if (ARG_RM16) INS_0F_00(rid, 2);
    else if (ARG_R32) INS_0F_00(rid, 4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LLDT() {
    uint8_t rid = 0x02;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LLDT");
    if (ARG_RM16) INS_0F_00(rid, 0);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LTR() {
    uint8_t rid = 0x03;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LTR");
    if (ARG_RM16) INS_0F_00(rid, 0);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void VERR() {
    uint8_t rid = 0x04;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("VERR");
    if (ARG_RM16) INS_0F_00(rid, 0);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void VERW() {
    uint8_t rid = 0x05;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("VERW");
    if (ARG_RM16) INS_0F_00(rid, 0);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SGDT() {
    uint8_t rid = 0x00;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SGDT");
    if (ARG_M16s) INS_0F_01(rid, 2);
    else if (ARG_M32s) INS_0F_01(rid, 4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SIDT() {
    uint8_t rid = 0x01;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SIDT");
    if (ARG_M16s) INS_0F_01(rid, 2);
    else if (ARG_M32s) INS_0F_01(rid, 4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LGDT() {
    uint8_t rid = 0x02;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SIDT");
    if (ARG_M16s) INS_0F_01(rid, 2);
    else if (ARG_M32) INS_0F_01(rid, 4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LIDT() {
    uint8_t rid = 0x03;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LIDT");
    if (ARG_M16s) INS_0F_01(rid, 2);
    else if (ARG_M32) INS_0F_01(rid, 4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SMSW() {
    uint8_t rid = 0x04;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SMSW");
    if (ARG_RM16) INS_0F_01(rid, 2);
    else if (ARG_R32) INS_0F_01(rid, 4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LMSW() {
    uint8_t rid = 0x06;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LMSW");
    if (ARG_RM16) INS_0F_01(rid, 0);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LAR() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LAR");
    if (ARG_R16_RM16) LAR_R32_RM32(2);
    else if (ARG_R32_RM32) LAR_R32_RM32(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LSL() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LSL");
    if (ARG_R16_RM16) LSL_R32_RM32(2);
    else if (ARG_R32_RM32) LSL_R32_RM32(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void BT() {
    uint8_t rid = 0x04;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("BT");
    if (ARG_RM16_R16) BT_RM32_R32(2);
    else if (ARG_RM32_R32) BT_RM32_R32(4);
    else if (ARG_RM16_I8) INS_0F_BA(rid, 2);
    else if (ARG_RM32_I8) INS_0F_BA(rid, 4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SHLD() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SHLD");
    if (ARG_RM16_R16_I8) SHLD_RM32_R32_I8(2);
    if (ARG_RM16_R16_CL) SHLD_RM32_R32_CL(2);
    else if (ARG_RM32_R32_I8) SHLD_RM32_R32_I8(4);
    else if (ARG_RM32_R32_CL) SHLD_RM32_R32_CL(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void BTS() {
    uint8_t rid = 0x05;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("BTS");
    if (ARG_RM16_R16) BTS_RM32_R32(2);
    else if (ARG_RM32_R32) BTS_RM32_R32(4);
    else if (ARG_RM16_I8) INS_0F_BA(rid, 2);
    else if (ARG_RM32_I8) INS_0F_BA(rid, 4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SHRD() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SHRD");
    if (ARG_RM16_R16_I8) SHRD_RM32_R32_I8(2);
    if (ARG_RM16_R16_CL) SHRD_RM32_R32_CL(2);
    else if (ARG_RM32_R32_I8) SHRD_RM32_R32_I8(4);
    else if (ARG_RM32_R32_CL) SHRD_RM32_R32_CL(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LSS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LSS");
    if (ARG_R16_M16) LSS_R32_M16_32(2);
    else if (ARG_R32_M32) LSS_R32_M16_32(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void BTR() {
    uint8_t rid = 0x06;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("BTR");
    if (ARG_RM16_R16) BTR_RM32_R32(2);
    else if (ARG_RM32_R32) BTR_RM32_R32(4);
    else if (ARG_RM16_I8) INS_0F_BA(rid, 2);
    else if (ARG_RM32_I8) INS_0F_BA(rid, 4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LFS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LFS");
    if (ARG_R16_M16) LFS_R32_M16_32(2);
    else if (ARG_R32_M32) LFS_R32_M16_32(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LGS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LGS");
    if (ARG_R16_M16) LGS_R32_M16_32(2);
    else if (ARG_R32_M32) LGS_R32_M16_32(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOVZX() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOVZX");
    if (ARG_R16_RM8s) MOVZX_R32_RM8(2);
    else if (ARG_R32_RM8s) MOVZX_R32_RM8(4);
    else if (ARG_R32_RM16s) MOVZX_R32_RM16();
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void BTC() {
    uint8_t rid = 0x07;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("BTC");
    if (ARG_RM16_R16) BTC_RM32_R32(2);
    else if (ARG_RM32_R32) BTC_RM32_R32(4);
    else if (ARG_RM16_I8) INS_0F_BA(rid, 2);
    else if (ARG_RM32_I8) INS_0F_BA(rid, 4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void BSF() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("BSF");
    if (ARG_R16_RM16) BSF_R32_RM32(2);
    else if (ARG_R32_RM32) BSF_R32_RM32(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void BSR() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("BSR");
    if (ARG_R16_RM16) BSR_R32_RM32(2);
    else if (ARG_R32_RM32) BSR_R32_RM32(4);
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOVSX() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOVSX");
    if (ARG_R16_RM8s) MOVSX_R32_RM8(2);
    else if (ARG_R32_RM8s) MOVSX_R32_RM8(4);
    else if (ARG_R32_RM16s) MOVSX_R32_RM16();
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}

/* main routines */
static int is_end(char c) {
    return (!c || c == '\n' || c == ';');
}
static int is_space(char c) {
    return (c == ' ' || c == '\t');
}
static int is_prefix() {
    if (!STD_STRCMP(rop, "es:") || !STD_STRCMP(rop, "cs:") ||
            !STD_STRCMP(rop, "ss:") || !STD_STRCMP(rop, "ds:") ||
            !STD_STRCMP(rop, "fs:") || !STD_STRCMP(rop, "gs:") ||
            !STD_STRCMP(rop, "lock:") || !STD_STRCMP(rop, "rep:") ||
            !STD_STRCMP(rop, "repne:") || !STD_STRCMP(rop, "repnz:") ||
            !STD_STRCMP(rop, "repe:") || !STD_STRCMP(rop, "repz:")) {
        return 1;
    } else {
        return 0;
    }
}
static void exec() {
    /* assemble single statement */
    NTVDM64_TYPE_TRACE_CALL_BEGIN("exec");
    if (!rop || is_end(rop[0])) ;
    else if (!STD_STRCMP(rop, "add")) ADD();
    else if (!STD_STRCMP(rop,"push")) PUSH();
    else if (!STD_STRCMP(rop, "pop")) POP();
    else if (!STD_STRCMP(rop, "or" )) OR();
    else if (!STD_STRCMP(rop, "adc")) ADC();
    else if (!STD_STRCMP(rop, "sbb")) SBB();
    else if (!STD_STRCMP(rop, "and")) AND();
    else if (!STD_STRCMP(rop, "es:")) PREFIX_ES();
    else if (!STD_STRCMP(rop, "daa")) DAA();
    else if (!STD_STRCMP(rop, "sub")) SUB();
    else if (!STD_STRCMP(rop, "cs:")) PREFIX_CS();
    else if (!STD_STRCMP(rop, "das")) DAS();
    else if (!STD_STRCMP(rop, "xor")) XOR();
    else if (!STD_STRCMP(rop, "ss:")) PREFIX_SS();
    else if (!STD_STRCMP(rop, "aaa")) AAA();
    else if (!STD_STRCMP(rop, "cmp")) CMP();
    else if (!STD_STRCMP(rop, "ds:")) PREFIX_DS();
    else if (!STD_STRCMP(rop, "aas")) AAS();
    else if (!STD_STRCMP(rop, "inc")) INC();
    else if (!STD_STRCMP(rop, "dec")) DEC();
    else if (!STD_STRCMP(rop, "pusha")) PUSHA(2);
    else if (!STD_STRCMP(rop, "pushad")) PUSHA(4);
    else if (!STD_STRCMP(rop, "popa")) POPA(2);
    else if (!STD_STRCMP(rop, "popad")) POPA(4);
    else if (!STD_STRCMP(rop, "bound")) BOUND();
    else if (!STD_STRCMP(rop, "arpl")) ARPL_RM16_R16();
    else if (!STD_STRCMP(rop, "fs:")) PREFIX_FS();
    else if (!STD_STRCMP(rop, "gs:")) PREFIX_GS();
    else if (!STD_STRCMP(rop, "op+:")) PREFIX_OprSize();
    else if (!STD_STRCMP(rop, "az+:")) PREFIX_AddrSize();
    else if (!STD_STRCMP(rop,"imul")) IMUL();
    else if (!STD_STRCMP(rop,"ins"))  INS();
    else if (!STD_STRCMP(rop,"insb")) INSB();
    else if (!STD_STRCMP(rop,"insw")) INSW(2);
    else if (!STD_STRCMP(rop,"insd")) INSW(4);
    else if (!STD_STRCMP(rop,"outs"))  OUTS();
    else if (!STD_STRCMP(rop,"outsb")) OUTSB();
    else if (!STD_STRCMP(rop,"outsw")) OUTSW(2);
    else if (!STD_STRCMP(rop,"outsd")) OUTSW(4);
    else if (!STD_STRCMP(rop, "jo" )) JCC_REL(0x70);
    else if (!STD_STRCMP(rop, "jno")) JCC_REL(0x71);
    else if (!STD_STRCMP(rop, "jb" )) JCC_REL(0x72);
    else if (!STD_STRCMP(rop, "jc" )) JCC_REL(0x72);
    else if (!STD_STRCMP(rop,"jnae")) JCC_REL(0x72);
    else if (!STD_STRCMP(rop, "jae")) JCC_REL(0x73);
    else if (!STD_STRCMP(rop, "jnb")) JCC_REL(0x73);
    else if (!STD_STRCMP(rop, "jnc")) JCC_REL(0x73);
    else if (!STD_STRCMP(rop, "je" )) JCC_REL(0x74);
    else if (!STD_STRCMP(rop, "jz" )) JCC_REL(0x74);
    else if (!STD_STRCMP(rop, "jne")) JCC_REL(0x75);
    else if (!STD_STRCMP(rop, "jnz")) JCC_REL(0x75);
    else if (!STD_STRCMP(rop, "jbe")) JCC_REL(0x76);
    else if (!STD_STRCMP(rop, "jna")) JCC_REL(0x76);
    else if (!STD_STRCMP(rop, "ja" )) JCC_REL(0x77);
    else if (!STD_STRCMP(rop,"jnbe")) JCC_REL(0x77);
    else if (!STD_STRCMP(rop, "js" )) JCC_REL(0x78);
    else if (!STD_STRCMP(rop, "jns")) JCC_REL(0x79);
    else if (!STD_STRCMP(rop, "jp" )) JCC_REL(0x7a);
    else if (!STD_STRCMP(rop, "jpe")) JCC_REL(0x7a);
    else if (!STD_STRCMP(rop, "jnp")) JCC_REL(0x7b);
    else if (!STD_STRCMP(rop, "jpo")) JCC_REL(0x7b);
    else if (!STD_STRCMP(rop, "jl" )) JCC_REL(0x7c);
    else if (!STD_STRCMP(rop,"jnge")) JCC_REL(0x7c);
    else if (!STD_STRCMP(rop, "jge")) JCC_REL(0x7d);
    else if (!STD_STRCMP(rop, "jnl")) JCC_REL(0x7d);
    else if (!STD_STRCMP(rop, "jle")) JCC_REL(0x7e);
    else if (!STD_STRCMP(rop, "jng")) JCC_REL(0x7e);
    else if (!STD_STRCMP(rop, "jg" )) JCC_REL(0x7f);
    else if (!STD_STRCMP(rop,"jnle")) JCC_REL(0x7f);
    else if (!STD_STRCMP(rop,"test")) TEST();
    else if (!STD_STRCMP(rop,"xchg")) XCHG();
    else if (!STD_STRCMP(rop, "mov")) MOV();
    else if (!STD_STRCMP(rop, "lea")) LEA();
    else if (!STD_STRCMP(rop, "nop")) NOP();
    else if (!STD_STRCMP(rop, "cbw"))  CBW(2);
    else if (!STD_STRCMP(rop, "cwde")) CBW(4);
    else if (!STD_STRCMP(rop, "cwd")) CWD(2);
    else if (!STD_STRCMP(rop, "cdq")) CWD(4);
    else if (!STD_STRCMP(rop,"call")) CALL();
    else if (!STD_STRCMP(rop,"wait")) WAIT();
    else if (!STD_STRCMP(rop,"pushf"))  PUSHF(2);
    else if (!STD_STRCMP(rop,"pushfd")) PUSHF(4);
    else if (!STD_STRCMP(rop,"popf"))  POPF(2);
    else if (!STD_STRCMP(rop,"popfd")) POPF(4);
    else if (!STD_STRCMP(rop,"sahf")) SAHF();
    else if (!STD_STRCMP(rop,"lahf")) LAHF();
    else if (!STD_STRCMP(rop,"movs"))  MOVS();
    else if (!STD_STRCMP(rop,"movsb")) MOVSB();
    else if (!STD_STRCMP(rop,"movsw")) MOVSW(2);
    else if (!STD_STRCMP(rop,"movsd")) MOVSW(4);
    else if (!STD_STRCMP(rop,"cmps"))  CMPS();
    else if (!STD_STRCMP(rop,"cmpsb")) CMPSB();
    else if (!STD_STRCMP(rop,"cmpsw")) CMPSW(2);
    else if (!STD_STRCMP(rop,"cmpsd")) CMPSW(4);
    else if (!STD_STRCMP(rop,"stos"))  STOS();
    else if (!STD_STRCMP(rop,"stosb")) STOSB();
    else if (!STD_STRCMP(rop,"stosw")) STOSW(2);
    else if (!STD_STRCMP(rop,"stosd")) STOSW(4);
    else if (!STD_STRCMP(rop,"lods"))  LODS();
    else if (!STD_STRCMP(rop,"lodsb")) LODSB();
    else if (!STD_STRCMP(rop,"lodsw")) LODSW(2);
    else if (!STD_STRCMP(rop,"lodsd")) LODSW(4);
    else if (!STD_STRCMP(rop,"scas"))  SCAS();
    else if (!STD_STRCMP(rop,"scasb")) SCASB();
    else if (!STD_STRCMP(rop,"scasw")) SCASW(2);
    else if (!STD_STRCMP(rop,"scasd")) SCASW(4);
    else if (!STD_STRCMP(rop, "ret")) RET();
    else if (!STD_STRCMP(rop, "les")) LES();
    else if (!STD_STRCMP(rop, "lds")) LDS();
    else if (!STD_STRCMP(rop, "enter")) ENTER();
    else if (!STD_STRCMP(rop, "leave")) LEAVE();
    else if (!STD_STRCMP(rop,"retf")) RETF();
    else if (!STD_STRCMP(rop, "int")) INT();
    else if (!STD_STRCMP(rop,"into")) INTO();
    else if (!STD_STRCMP(rop,"iret"))  IRET(2);
    else if (!STD_STRCMP(rop,"iretd")) IRET(4);
    else if (!STD_STRCMP(rop, "rol")) ROL();
    else if (!STD_STRCMP(rop, "ror")) ROR();
    else if (!STD_STRCMP(rop, "rcl")) RCL();
    else if (!STD_STRCMP(rop, "rcr")) RCR();
    else if (!STD_STRCMP(rop, "shl")) SHL();
    else if (!STD_STRCMP(rop, "shr")) SHR();
    else if (!STD_STRCMP(rop, "sal")) SAL();
    else if (!STD_STRCMP(rop, "sar")) SAR();
    else if (!STD_STRCMP(rop, "aam")) AAM();
    else if (!STD_STRCMP(rop, "aad")) AAD();
    else if (!STD_STRCMP(rop,"xlat"))  XLAT();
    else if (!STD_STRCMP(rop,"xlatb")) XLATB();
    else if (!STD_STRCMP(rop,"loopne")) JCC_REL(0xe0);
    else if (!STD_STRCMP(rop,"loopnz")) JCC_REL(0xe0);
    else if (!STD_STRCMP(rop,"loope"))  JCC_REL(0xe1);
    else if (!STD_STRCMP(rop,"loopz"))  JCC_REL(0xe1);
    else if (!STD_STRCMP(rop,"loop"))   JCC_REL(0xe2);
    else if (!STD_STRCMP(rop,"jcxz"))   JCC_REL(0xe3);
    else if (!STD_STRCMP(rop, "in" )) IN();
    else if (!STD_STRCMP(rop, "out")) OUT();
    else if (!STD_STRCMP(rop, "jmp")) JMP();
    else if (!STD_STRCMP(rop, "lock"))  PREFIX_LOCK();
    else if (!STD_STRCMP(rop,"repne:")) PREFIX_REPNZ();
    else if (!STD_STRCMP(rop,"repnz:")) PREFIX_REPNZ();
    else if (!STD_STRCMP(rop, "rep:")) PREFIX_REPZ();
    else if (!STD_STRCMP(rop,"repe:")) PREFIX_REPZ();
    else if (!STD_STRCMP(rop,"repz:")) PREFIX_REPZ();
    else if (!STD_STRCMP(rop, "hlt")) HLT();
    else if (!STD_STRCMP(rop, "cmc")) CMC();
    else if (!STD_STRCMP(rop, "not")) NOT();
    else if (!STD_STRCMP(rop, "neg")) NEG();
    else if (!STD_STRCMP(rop, "mul")) MUL();
    else if (!STD_STRCMP(rop, "div")) DIV();
    else if (!STD_STRCMP(rop,"idiv")) IDIV();
    else if (!STD_STRCMP(rop, "clc")) CLC();
    else if (!STD_STRCMP(rop, "stc")) STC();
    else if (!STD_STRCMP(rop, "cli")) CLI();
    else if (!STD_STRCMP(rop, "sti")) STI();
    else if (!STD_STRCMP(rop, "cld")) CLD();
    else if (!STD_STRCMP(rop, "std")) STD();
    else if (!STD_STRCMP(rop, "sldt"))  SLDT();
    else if (!STD_STRCMP(rop, "str"))   STR();
    else if (!STD_STRCMP(rop, "lldt"))  LLDT();
    else if (!STD_STRCMP(rop, "ltr"))   LTR();
    else if (!STD_STRCMP(rop, "verr"))  VERR();
    else if (!STD_STRCMP(rop, "verw"))  VERW();
    else if (!STD_STRCMP(rop, "sgdt"))  SGDT();
    else if (!STD_STRCMP(rop, "sidt"))  SIDT();
    else if (!STD_STRCMP(rop, "lgdt"))  LGDT();
    else if (!STD_STRCMP(rop, "lidt"))  LIDT();
    else if (!STD_STRCMP(rop, "smsw"))  SMSW();
    else if (!STD_STRCMP(rop, "lmsw"))  LMSW();
    else if (!STD_STRCMP(rop, "lar"))   LAR();
    else if (!STD_STRCMP(rop, "lsl"))   LSL();
    else if (!STD_STRCMP(rop, "clts"))  CLTS();
    else if (!STD_STRCMP(rop, "seto" )) SETCC_RM8(0x90);
    else if (!STD_STRCMP(rop, "setno")) SETCC_RM8(0x91);
    else if (!STD_STRCMP(rop, "setb" )) SETCC_RM8(0x92);
    else if (!STD_STRCMP(rop, "setc" )) SETCC_RM8(0x92);
    else if (!STD_STRCMP(rop,"setnae")) SETCC_RM8(0x92);
    else if (!STD_STRCMP(rop, "setae")) SETCC_RM8(0x93);
    else if (!STD_STRCMP(rop, "setnb")) SETCC_RM8(0x93);
    else if (!STD_STRCMP(rop, "setnc")) SETCC_RM8(0x93);
    else if (!STD_STRCMP(rop, "sete" )) SETCC_RM8(0x94);
    else if (!STD_STRCMP(rop, "setz" )) SETCC_RM8(0x94);
    else if (!STD_STRCMP(rop, "setne")) SETCC_RM8(0x95);
    else if (!STD_STRCMP(rop, "setnz")) SETCC_RM8(0x95);
    else if (!STD_STRCMP(rop, "setbe")) SETCC_RM8(0x96);
    else if (!STD_STRCMP(rop, "setna")) SETCC_RM8(0x96);
    else if (!STD_STRCMP(rop, "seta" )) SETCC_RM8(0x97);
    else if (!STD_STRCMP(rop,"setnbe")) SETCC_RM8(0x97);
    else if (!STD_STRCMP(rop, "sets" )) SETCC_RM8(0x98);
    else if (!STD_STRCMP(rop, "setns")) SETCC_RM8(0x99);
    else if (!STD_STRCMP(rop, "setp" )) SETCC_RM8(0x9a);
    else if (!STD_STRCMP(rop, "setpe")) SETCC_RM8(0x9a);
    else if (!STD_STRCMP(rop, "setnp")) SETCC_RM8(0x9b);
    else if (!STD_STRCMP(rop, "setpo")) SETCC_RM8(0x9b);
    else if (!STD_STRCMP(rop, "setl" )) SETCC_RM8(0x9c);
    else if (!STD_STRCMP(rop,"setnge")) SETCC_RM8(0x9c);
    else if (!STD_STRCMP(rop, "setge")) SETCC_RM8(0x9d);
    else if (!STD_STRCMP(rop, "setnl")) SETCC_RM8(0x9d);
    else if (!STD_STRCMP(rop, "setle")) SETCC_RM8(0x9e);
    else if (!STD_STRCMP(rop, "setng")) SETCC_RM8(0x9e);
    else if (!STD_STRCMP(rop, "setg" )) SETCC_RM8(0x9f);
    else if (!STD_STRCMP(rop,"setnle")) SETCC_RM8(0x9f);
    else if (!STD_STRCMP(rop, "bt"))    BT();
    else if (!STD_STRCMP(rop, "shld"))  SHLD();
    else if (!STD_STRCMP(rop, "bts"))   BTS();
    else if (!STD_STRCMP(rop, "shrd"))  SHRD();
    else if (!STD_STRCMP(rop, "lss"))   LSS();
    else if (!STD_STRCMP(rop, "btr"))   BTR();
    else if (!STD_STRCMP(rop, "lfs"))   LFS();
    else if (!STD_STRCMP(rop, "lgs"))   LGS();
    else if (!STD_STRCMP(rop, "movzx")) MOVZX();
    else if (!STD_STRCMP(rop, "btc"))   BTC();
    else if (!STD_STRCMP(rop, "bsf"))   BSF();
    else if (!STD_STRCMP(rop, "bsr"))   BSR();
    else if (!STD_STRCMP(rop, "movsx")) MOVSX();
    else if (!STD_STRCMP(rop, "qdx"))   QDX();
    else _ser_;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static char *take_arg(char *s) {
    char *rend, *rresult;
    if (s) {
        rstart = s;
    }
    if (!rstart) {
        return NULL;
    }
    while (!is_end(*rstart) && is_space(*rstart)) {
        rstart++;
    }
    if (*rstart == ',' || is_end(*rstart)) {
        return NULL;
    }
    rresult = rstart;
    while (!is_end(*rstart) && (*rstart) != ',') {
        rstart++;
    }
    rend = rstart - 1;
    if (is_end(*rstart)) {
        rstart = NULL;
    } else {
        rstart++;
    }
    while (!is_end(*rend) && is_space(*rend)) {
        rend--;
    }
    *(rend + 1) = 0;
    return rresult;
}
static uint8_t aasm32_execute(const char *stmt, uint8_t *rcode, int flag32) {
    uint8_t len;
    char astmt[0x100];
    char *rstmt;
    uint8_t flagprefix;

    if (!stmt || is_end(stmt[0])) {
        return 0;
    }

#if AASM_TRACE == 1
    ntvdm64_type_trace_initialize(&trace);
#endif

    STD_MEMCPY((void *) astmt, (void *) stmt, 0x100);
    ntvdm64_type_string_lower(astmt);
    rstmt = astmt;

    defsize = !!flag32;

    prefix_oprsize = prefix_addrsize = 0;
    prefix_lock = prefix_repz = prefix_repnz = 0;
    flagError = 0;

    iop = 0;
    STD_MEMSET((void *)(&aopri1), 0x00, sizeof(t_aasm_oprinfo));
    STD_MEMSET((void *)(&aopri2), 0x00, sizeof(t_aasm_oprinfo));
    STD_MEMSET((void *)(&aopri3), 0x00, sizeof(t_aasm_oprinfo));
    STD_MEMSET((void *)(&aoprig), 0x00, sizeof(t_aasm_oprinfo));
    prefix_oprsizeg = prefix_addrsizeg = 0;

    rop = ropr1 = ropr2 = ropr3 = NULL;

    /* process prefixes */
    do {
        while (!is_end(*rstmt) && is_space(*rstmt)) {
            rstmt++;
        }
        rop = rstmt;
        while (!is_end(*rstmt) && !is_space(*rstmt)) {
            rstmt++;
        }
        if (!is_end(*rstmt)) {
            *rstmt = 0;
            rstmt++;
        }
        flagprefix = is_prefix();
        if (flagprefix) {
            exec();
        }
    } while (flagprefix && !flagError);

    /* process assembly statement */
    ropr1 = take_arg(rstmt);
    ropr2 = take_arg(NULL);
    ropr3 = take_arg(NULL);

    aopri1 = parsearg(ropr1);
    aopri2 = parsearg(ropr2);
    aopri3 = parsearg(ropr3);

    if (isM(aopri1)) {
        rinfo = &aopri1;
    } else if (isM(aopri2)) {
        rinfo = &aopri2;
    } else if (isM(aopri3)) {
        rinfo = &aopri3;
    } else {
        rinfo = NULL;
    }

    exec();
    len = 0;

    if (!flagError) {
        if (prefix_repz)  {
            (*(rcode + len)) = 0xf3;
            len++;
        }
        if (prefix_repnz) {
            (*(rcode + len)) = 0xf2;
            len++;
        }
        if (prefix_lock)  {
            (*(rcode + len)) = 0xf0;
            len++;
        }
        if ((rinfo && rinfo->flages) || aoprig.flages) {
            (*(rcode + len)) = 0x26;
            len++;
        }
        if ((rinfo && rinfo->flagcs) || aoprig.flagcs) {
            (*(rcode + len)) = 0x2e;
            len++;
        }
        if ((rinfo && rinfo->flagss) || aoprig.flagss) {
            (*(rcode + len)) = 0x36;
            len++;
        }
        if ((rinfo && rinfo->flagds) || aoprig.flagds) {
            (*(rcode + len)) = 0x3e;
            len++;
        }
        if ((rinfo && rinfo->flagfs) || aoprig.flagfs) {
            (*(rcode + len)) = 0x64;
            len++;
        }
        if ((rinfo && rinfo->flaggs) || aoprig.flaggs) {
            (*(rcode + len)) = 0x65;
            len++;
        }
        if (prefix_addrsize || prefix_addrsizeg) {
            (*(rcode + len)) = 0x67;
            len++;
        }
        if (prefix_oprsize || prefix_oprsizeg) {
            (*(rcode + len)) = 0x66;
            len++;
        }
        STD_MEMCPY((void *)(rcode + len), (void *) acode, iop);
        len += iop;
    } else {
#if AASM_TRACE == 1
        STD_PRINTF("aasm32: bad instruction '%s'\n", stmt);
        STD_PRINTF("aasm32: [%s] [%s/%d] [%s/%d] [%s/%d]\n",
               rop, ropr1, aopri1.type, ropr2, aopri2.type, ropr3, aopri3.type);
#endif
    }

#if AASM_TRACE == 1
    if (trace.callCount || trace.flagError) {
        STD_PRINTF("aasm32: bad instruction '%s'\n", stmt);
    }
    ntvdm64_type_trace_finalize(&trace);
#endif

    return len;
}

uint8_t aasm32(const char *stmt, uint8_t *rcode, int flag32) {
    aasm32_context local_context;
    aasm32_context *previous = aasmContext;
    uint8_t result;

    if (previous != NULL) return aasm32_execute(stmt, rcode, flag32);
    STD_MEMSET(&local_context, 0, sizeof(local_context));
    aasmContext = &local_context;
    result = aasm32_execute(stmt, rcode, flag32);
    aasmContext = previous;
    return result;
}

/* extended routines - assemble a paragraph with call/jmp labels */
typedef struct {
    char stmt[0x100];
    uint32_t stmt_id;
    uint8_t code_array[15];
    uint8_t code_len;
    uint8_t flag_is_label;
    uint8_t flag_has_label;
    char label_str[0x100];
    char op_str[0x100];
    t_aasm_oprptr ptr;
} t_aasm_instr;
/* default operand size */
#define _GetOperandSize (defsize ? 4 : 2)
static void asmx_get_label(t_aasm_instr *rinstr) {
    size_t i = 0, j = 0;
    rinstr->label_str[0] = 0;
    rinstr->flag_has_label = 0;
    rinstr->flag_is_label = 0;
    while (rinstr->stmt[i] && rinstr->stmt[i] != '$') {
        i++;
    }
    if (rinstr->stmt[i] != '$') {
        return;
    }
    i++;
    if (rinstr->stmt[i] != '(') {
        return;
    }
    i++;
    while (rinstr->stmt[i] && rinstr->stmt[i] != ')') {
        rinstr->label_str[j++] = rinstr->stmt[i++];
    }
    rinstr->label_str[j] = 0;
    if (rinstr->stmt[i] != ')') {
        return;
    }
    rinstr->flag_has_label = 1;
    if (rinstr->stmt[0] == '$' && rinstr->stmt[1] == '(' &&
            rinstr->stmt[STD_STRLEN(rinstr->stmt) - 1] == ':' &&
            rinstr->stmt[STD_STRLEN(rinstr->stmt) - 2] == ')') {
        rinstr->flag_is_label = 1;
    }
}
static void asmx_parse_instr(t_aasm_instr *rinstr) {
    size_t i;
    char *rstmt;
    t_aasm_token token;
    i = 0;
    rinstr->code_len = 0;
    while (!is_end(rinstr->stmt[i]) && !is_space(rinstr->stmt[i])) {
        rinstr->op_str[i] = rinstr->stmt[i];
        i++;
    }
    rinstr->op_str[i] = 0;
    rstmt = rinstr->stmt + i;
    if (is_end(*rstmt)) {
        return;
    }
    token = gettoken(rstmt);
    switch (token) {
    case TOKEN_SHORT:
        token = gettoken(NULL);
        if (token == TOKEN_PTR) {
            token = gettoken(NULL);
        }
        if (token != TOKEN_DOLLAR) {
            return;
        }
        rinstr->ptr = PTR_SHORT;
        break;
    case TOKEN_NEAR:
        token = gettoken(NULL);
        if (token == TOKEN_PTR) {
            token = gettoken(NULL);
        }
        if (token != TOKEN_DOLLAR) {
            return;
        }
        rinstr->ptr = PTR_NEAR;
        break;
    case TOKEN_DOLLAR:
        rinstr->ptr = PTR_NONE;
        break;
    default:
        return;
    }
    if (!STD_STRCMP(rinstr->op_str,"loopne") || !STD_STRCMP(rinstr->op_str,"loopnz") || !STD_STRCMP(rinstr->op_str,"loope") ||
            !STD_STRCMP(rinstr->op_str,"loopz")  || !STD_STRCMP(rinstr->op_str,"loop")   || !STD_STRCMP(rinstr->op_str,"jcxz")) {
        switch (rinstr->ptr) {
        case PTR_NONE:
            rinstr->ptr = PTR_SHORT;
        case PTR_SHORT:
            rinstr->code_len = 1/*opcode*/ + 1/*rel_imm8*/;
            break;
        default:
            return;
        }
    }
    if (!STD_STRCMP(rinstr->op_str, "jo")  || !STD_STRCMP(rinstr->op_str, "jno") || !STD_STRCMP(rinstr->op_str, "jb" ) ||
            !STD_STRCMP(rinstr->op_str, "jc" ) || !STD_STRCMP(rinstr->op_str,"jnae") || !STD_STRCMP(rinstr->op_str, "jae") ||
            !STD_STRCMP(rinstr->op_str, "jnb") || !STD_STRCMP(rinstr->op_str, "jnc") || !STD_STRCMP(rinstr->op_str, "je" ) ||
            !STD_STRCMP(rinstr->op_str, "jz" ) || !STD_STRCMP(rinstr->op_str, "jne") || !STD_STRCMP(rinstr->op_str, "jnz") ||
            !STD_STRCMP(rinstr->op_str, "jbe") || !STD_STRCMP(rinstr->op_str, "jna") || !STD_STRCMP(rinstr->op_str, "ja" ) ||
            !STD_STRCMP(rinstr->op_str,"jnbe") || !STD_STRCMP(rinstr->op_str, "js" ) || !STD_STRCMP(rinstr->op_str, "jns") ||
            !STD_STRCMP(rinstr->op_str, "jp" ) || !STD_STRCMP(rinstr->op_str, "jpe") || !STD_STRCMP(rinstr->op_str, "jnp") ||
            !STD_STRCMP(rinstr->op_str, "jpo") || !STD_STRCMP(rinstr->op_str, "jl" ) || !STD_STRCMP(rinstr->op_str,"jnge") ||
            !STD_STRCMP(rinstr->op_str, "jge") || !STD_STRCMP(rinstr->op_str, "jnl") || !STD_STRCMP(rinstr->op_str, "jle") ||
            !STD_STRCMP(rinstr->op_str, "jng") || !STD_STRCMP(rinstr->op_str, "jg" ) || !STD_STRCMP(rinstr->op_str,"jnle")) {
        switch (rinstr->ptr) {
        case PTR_NONE:
            rinstr->ptr = PTR_SHORT;
        case PTR_SHORT:
            rinstr->code_len = 1/*opcode*/ + 1/*rel_imm8*/;
            break;
        case PTR_NEAR:
            rinstr->code_len = 1/*0x0f*/ + 1/*opcode*/ + _GetOperandSize/*rel_immx*/;
            break;
        default:
            return;
        }
    }
    if (!STD_STRCMP(rinstr->op_str, "jmp")) {
        switch (rinstr->ptr) {
        case PTR_SHORT:
            rinstr->code_len = 1/*opcode*/ + 1/*rel_imm8*/;
            break;
        case PTR_NONE:
            rinstr->ptr = PTR_NEAR;
        case PTR_NEAR:
            rinstr->code_len = 1/*opcode*/ + _GetOperandSize/*rel_immx*/;
            break;
        default:
            return;
        }
    }
    if (!STD_STRCMP(rinstr->op_str, "call")) {
        switch (rinstr->ptr) {
        case PTR_NONE:
            rinstr->ptr = PTR_NEAR;
        case PTR_NEAR:
            rinstr->code_len = 1/*opcode*/ + _GetOperandSize/*rel_immx*/;
            break;
        default:
            return;
        }
    }
}
static uint32_t aasm32x_execute(const char *stmt, uint8_t *rcode, int flag32) {
    int32_t i, j, k, count;
    uint32_t len;
    uint32_t offset;
    char imm[0x100];
    t_aasm_instr *instr;
    count = 1;
    flagError = 0;
    for (i = 0; i < (int32_t) STD_STRLEN(stmt); ++i) {
        if (stmt[i] == '\n') {
            count++;
        }
    }
    instr = (t_aasm_instr *) STD_MALLOC(count * sizeof(t_aasm_instr));
    i = j = k = 0;
    while (is_space(stmt[i])) {
        i++;
    }
    while (1) {
        if (is_end(stmt[i])) {
            if (j) {
                j--;
                if (j) {
                    while (is_space(instr[k].stmt[j])) {
                        j--;
                    }
                    if (j) {
                        instr[k].stmt[j + 1] = 0;
                        ntvdm64_type_string_lower(instr[k].stmt);
                        instr[k].stmt_id = (uint32_t) k;
                        j = 0;
                        k++;
                    }
                }
            }
            while (stmt[i] && stmt[i] != '\n') {
                i++;
            }
            if (!stmt[i]) {
                break;
            } else if (stmt[i] == '\n') {
                i++;
                while (is_space(stmt[i])) {
                    i++;
                }
            }
        } else {
            instr[k].stmt[j] = stmt[i];
            i++;
            j++;
        }
    }
    count = k;
    for (i = 0; i < count; ++i) {
        STD_MEMSET((void *) instr[i].code_array, 0x00, 15);
        asmx_get_label(&instr[i]);
        if (instr[i].flag_has_label) {
            if (instr[i].flag_is_label) {
                instr[i].code_array[0] = 0x90;
                instr[i].code_len = 1;
            } else {
                asmx_parse_instr(&instr[i]);
            }
        } else {
            instr[i].code_len = aasm32(instr[i].stmt, instr[i].code_array, flag32);
        }
        if (flagError) {
            STD_FREE((void *) instr);
            return 0;
        }
        if (!instr[i].code_len) {
            flagError = 1;
            STD_PRINTF("bad instruction in first round:\n#%d: [%s], %x", instr[i].stmt_id, instr[i].stmt, instr[i].code_len);
            if (instr[i].code_len) {
                STD_PRINTF(", code: [");
                for (j = 0; j < instr[i].code_len; ++j) {
                    STD_PRINTF("%02X", instr[i].code_array[j]);
                }
                STD_PRINTF("]");
            }
            if (instr[i].flag_has_label) {
                STD_PRINTF(", label: [%s], is=%s",
                       instr[i].label_str,
                       instr[i].flag_is_label ? "yes" : "no");
            }
            STD_PRINTF("\n");
        }
        if (flagError) {
            STD_FREE((void *) instr);
            return 0;
        }
    }
    /* i: label; j: instr to be materialized; k: size iterator */
    for (i = 0; i < count; ++i) {
        if (!instr[i].flag_is_label) {
            continue;
        }
        if (i) {
            for (j = i - 1; j >= 0; --j) {
                if (instr[j].flag_has_label && !STD_STRCMP(instr[j].label_str, instr[i].label_str)) {
                    if (instr[j].flag_is_label) {
                        flagError = 1;
                        STD_PRINTF("aasm32x: duplicate label '%s'.\n", instr[i].label_str);
                    } else {
                        offset = 0;
                        /* for (k = j + 1;k < i;++k) {
                            offset += instr[k].code_len;
                        }*/
                        for (k = j + 1; k <= i; ++k) {
                            offset += instr[k].code_len;
                        }
                        STD_STRCPY(instr[j].stmt, instr[j].op_str);
                        switch (instr[j].ptr) {
                        case PTR_SHORT:
                            STD_STRCAT(instr[j].stmt, " short ");
                            if (offset < 0x80) {
                                STD_SPRINTF(imm, "+%02x", (uint8_t) offset);
                            } else {
                                flagError = 1;
                                STD_PRINTF("aasm32x: invalid short pointer 8+.\n");
                            }
                            break;
                        case PTR_NEAR:
                            STD_STRCAT(instr[j].stmt, " near ");
                            switch (_GetOperandSize) {
                            case 2:
                                if (offset < 0x8000) {
                                    STD_SPRINTF(imm, "+%04x", (uint16_t) offset);
                                } else {
                                    flagError = 1;
                                    STD_PRINTF("aasm32x: invalid near pointer 16+.\n");
                                }
                                break;
                            case 4:
                                if (offset < 0x80000000) {
                                    STD_SPRINTF(imm, "+%08x", (uint32_t) offset);
                                } else {
                                    flagError = 1;
                                    STD_PRINTF("aasm32x: invalid near pointer 32+.\n");
                                }
                                break;
                            default:
                                break;
                            }
                            break;
                        default:
                            flagError = 1;
                            break;
                        }
                        STD_STRCAT(instr[j].stmt, imm);
                        aasm32(instr[j].stmt, instr[j].code_array, flag32);
                    }
                    if (flagError) {
                        STD_FREE((void *) instr);
                        return 0;
                    }
                }
                if (!j) break;
            }
        }
        if (i != count - 1) {
            for (j = i + 1; j < count; ++j) {
                if (instr[j].flag_has_label && !STD_STRCMP(instr[j].label_str, instr[i].label_str)) {
                    if (instr[j].flag_is_label) {
                        flagError = 1;
                        STD_PRINTF("aasm32x: duplicated label '%s'.\n", instr[i].label_str);
                    } else {
                        offset = 0;
                        for (k = i + 1; k < j + 1; ++k) {
                            offset += instr[k].code_len;
                        }
                        STD_STRCPY(instr[j].stmt, instr[j].op_str);
                        switch (instr[j].ptr) {
                        case PTR_SHORT:
                            STD_STRCAT(instr[j].stmt, " short ");
                            if (offset < 0x80) {
                                STD_SPRINTF(imm, "-%02x", (uint8_t) offset);
                            } else {
                                flagError = 1;
                                STD_PRINTF("aasm32x: invalid short pointer 8-.\n");
                            }
                            break;
                        case PTR_NEAR:
                            STD_STRCAT(instr[j].stmt, " near ");
                            switch (_GetOperandSize) {
                            case 2:
                                if (offset < 0x8000) {
                                    STD_SPRINTF(imm, "-%04x", (uint16_t) offset);
                                } else {
                                    flagError = 1;
                                    STD_PRINTF("aasm32x: invalid near pointer 16-.\n");
                                }
                                break;
                            case 4:
                                if (offset < 0x80000000) {
                                    STD_SPRINTF(imm, "-%08x", (uint32_t) offset);
                                } else {
                                    flagError = 1;
                                    STD_PRINTF("aasm32x: invalid near pointer 32-.\n");
                                }
                                break;
                            default:
                                break;
                            }
                            break;
                        default:
                            flagError = 1;
                            break;
                        }
                        STD_STRCAT(instr[j].stmt, imm);
                        aasm32(instr[j].stmt, instr[j].code_array, flag32);
                    }
                    if (flagError) {
                        STD_FREE((void *) instr);
                        return 0;
                    }
                }
            }
        }
    }
    len = 0;
    for (i = 0; i < count; ++i) {
        /*STD_PRINTF("%04X: %s", len, instr[i].stmt);
        for (j = (int32_t) STD_STRLEN(instr[i].stmt);j < 50;++j) STD_PRINTF(" ");
        STD_PRINTF("[");
        for (j = 0;j < instr[i].code_len;++j) STD_PRINTF("%02X", instr[i].code_array[j]);
        STD_PRINTF("]\n");*/
        STD_MEMCPY((void *)(rcode + len), (void *) instr[i].code_array, instr[i].code_len);
        len += instr[i].code_len;
    }
    STD_FREE((void *) instr);
    return len;
}

uint32_t aasm32x(const char *stmt, uint8_t *rcode, int flag32) {
    aasm32_context local_context;
    aasm32_context *previous = aasmContext;
    uint32_t result;

    if (previous != NULL) return aasm32x_execute(stmt, rcode, flag32);
    STD_MEMSET(&local_context, 0, sizeof(local_context));
    aasmContext = &local_context;
    result = aasm32x_execute(stmt, rcode, flag32);
    aasmContext = previous;
    return result;
}
