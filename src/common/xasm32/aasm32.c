#include "common/xasm32/xasm32.h"

#include "common/xasm32/aasm32.h"

/* Preserve the original assembler table body while routing every C-runtime
 * operation through the sole public lib/types vocabulary. */
#define memset lib_memory_set
#define memcpy lib_memory_copy
#define strlen lib_text_length
#define strcmp lib_text_compare
#define malloc lib_allocate
#define free lib_release
#define snprintf lib_text_format

#define _chrf(n)              \
    do                        \
    {                         \
        (n);                  \
        if (XASM32_TRACE_ERROR) \
        {                     \
            return info;      \
        }                     \
    } while (0)

/* set error and return */
#define _ser_          \
    do                 \
    {                  \
        flagError = 1; \
        return;        \
    } while (0)
#define _sert_         \
    do                 \
    {                  \
        flagError = 1; \
        return token;  \
    } while (0)
#define _serf_         \
    do                 \
    {                  \
        flagError = 1; \
        return info;   \
    } while (0)

/* operand size */
#define _SetOperandSize(n) (prefix_oprsize = (n) ? ((defsize ? 4 : 2) != (n)) : 0)
/* address size of the source operand */
#define _SetAddressSize(n) (prefix_addrsize = (n) ? ((defsize ? 4 : 2) != (n)) : 0)

typedef enum
{
    XASM32_OPERAND_NONE,
    XASM32_OPERAND_SREG,
    XASM32_OPERAND_CREG,
    XASM32_OPERAND_DREG,
    XASM32_OPERAND_TREG,
    XASM32_OPERAND_I16_16,
    XASM32_OPERAND_I16_32,
    XASM32_OPERAND_R8,
    XASM32_OPERAND_R16,
    XASM32_OPERAND_R32,
    XASM32_OPERAND_I8,
    XASM32_OPERAND_I16,
    XASM32_OPERAND_I32,
    XASM32_OPERAND_M,
    XASM32_OPERAND_M8,
    XASM32_OPERAND_M16,
    XASM32_OPERAND_M32
} t_aasm_oprtype;
typedef enum
{
    MOD_M,
    MOD_M_DISP8,
    MOD_M_DISP16,
    MOD_M_DISP32,
    MOD_R
} t_aasm_oprmod;
typedef enum
{
    MEM_BX_SI,
    MEM_BX_DI,
    MEM_BP_SI,
    MEM_BP_DI,
    MEM_SI,
    MEM_DI,
    MEM_BP,
    MEM_BX,
    MEM_EAX,
    MEM_ECX,
    MEM_EDX,
    MEM_EBX,
    MEM_SIB,
    MEM_EBP,
    MEM_ESI,
    MEM_EDI,
    MEM_BX_AL,
    MEM_EBX_AL
} t_aasm_oprmem;
typedef enum
{
    R8_AL,
    R8_CL,
    R8_DL,
    R8_BL,
    R8_AH,
    R8_CH,
    R8_DH,
    R8_BH
} t_aasm_oprreg8;
typedef enum
{
    R16_AX,
    R16_CX,
    R16_DX,
    R16_BX,
    R16_SP,
    R16_BP,
    R16_SI,
    R16_DI
} t_aasm_oprreg16;
typedef enum
{
    R32_EAX,
    R32_ECX,
    R32_EDX,
    R32_EBX,
    R32_ESP,
    R32_EBP,
    R32_ESI,
    R32_EDI
} t_aasm_oprreg32;
typedef enum
{
    SREG_ES,
    SREG_CS,
    SREG_SS,
    SREG_DS,
    SREG_FS,
    SREG_GS
} t_aasm_oprsreg;
typedef enum
{
    CREG_CR0,
    CREG_CR1,
    CREG_CR2,
    CREG_CR3,
    CREG_CR4,
    CREG_CR5,
    CREG_CR6,
    CREG_CR7
} t_aasm_oprcreg;
typedef enum
{
    DREG_DR0,
    DREG_DR1,
    DREG_DR2,
    DREG_DR3,
    DREG_DR4,
    DREG_DR5,
    DREG_DR6,
    DREG_DR7
} t_aasm_oprdreg;
typedef enum
{
    TREG_TR0,
    TREG_TR1,
    TREG_TR2,
    TREG_TR3,
    TREG_TR4,
    TREG_TR5,
    TREG_TR6,
    TREG_TR7
} t_aasm_oprtreg;
typedef enum
{
    PTR_NONE,
    PTR_SHORT,
    PTR_NEAR,
    PTR_FAR
} t_aasm_oprptr;
typedef struct
{
    t_aasm_oprreg32 base;
    t_aasm_oprreg32 index;
    lib_u8 scale;
} t_aasm_oprsib;
typedef struct
{
    t_aasm_oprtype type;
    t_aasm_oprmod mod;     /* active when type = 1, 2, 3, 6, 7, 8
                            * 0 = mem; 1 = mem+disp8; 2 = mem+disp16; 3 = reg */
    t_aasm_oprmem mem;     /* active when mod = 0, 1, 2
                            * 0 = [BX+SI], 1 = [BX+DI], 2 = [BP+SI], 3 = [BP+DI],
                            * 4 = [SI], 5 = [DI], 6 = [BP], 7 = [BX] */
    t_aasm_oprsib sib;     /* active when mem = MEM_SIB */
    t_aasm_oprreg8 reg8;   /* active when type = 1, mod = 3
                            * 0 = AL, 1 = CL, 2 = DL, 3 = BL,
                            * 4 = AH, 5 = CH, 6 = DH, 7 = BH */
    t_aasm_oprreg16 reg16; /* active when type = 2, mod = 3
                            * 0 = AX, 1 = CX, 2 = DX, 3 = BX,
                            * 4 = SP, 5 = BP, 6 = SI, 7 = DI */
    t_aasm_oprreg32 reg32; /* active when type = 3, mod = 3
                            * 0 = EAX, 1 = ECX, 2 = EDX, 3 = EBX,
                            * 4 = ESP, 5 = EBP, 6 = ESI, 7 = EDI */
    t_aasm_oprsreg sreg;   /* active when type = 3
                            * 0 = ES, 1 = CS, 2 = SS, 3 = DS, 4 = FS, 5 = GS */
    t_aasm_oprcreg creg;
    t_aasm_oprdreg dreg;
    t_aasm_oprtreg treg;
    lib_u8 imms; /* if imm is int */
    lib_u8 immn; /* if imm is negative */
    lib_u8 imm8;
    lib_u16 imm16;
    lib_u32 imm32;
    char disp8;
    lib_u16 disp16;   /* use as imm when type = 6; use by modrm as disp when mod = 0,1,2; */
    lib_u32 disp32;   /* use as imm when type = 7; use by modrm as disp when mod = 0,1,2; */
    t_aasm_oprptr ptr; /* 0 = near; 1 = far */
    lib_u16 rcs;
    lib_u32 reip;
    char label[0x100];
    lib_u8 flages, flagcs, flagss, flagds, flagfs, flaggs;
} t_aasm_oprinfo;
/* global variables */

typedef lib_u8 t_aasm_prefix;

typedef struct aasm32_context
{
    lib_u8 defsize;
    t_aasm_prefix prefix_oprsizeg, prefix_addrsizeg;
    t_aasm_prefix prefix_oprsize, prefix_addrsize;
    t_aasm_prefix prefix_lock, prefix_repz, prefix_repnz;
    lib_u8 acode[15];
    lib_u8 iop;
    char *rop, *ropr1, *ropr2, *ropr3;
    lib_u16 avcs, avip;
    char *aop, *aopr1, *aopr2;
    lib_u8 flagError;
    t_aasm_oprinfo aoprig, aopri1, aopri2, aopri3;
    t_aasm_oprinfo *rinfo;
    lib_u8 tokimm8;
    lib_u16 tokimm16;
    lib_u32 tokimm32;
    char tokchar;
    char tokstring[0x100], toklabel[0x100];
    char *tokptr;
    char *rstart;
} aasm32_context;

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
#define isNONE(oprinf) ((oprinf).type == XASM32_OPERAND_NONE)
#define isR8(oprinf) ((oprinf).type == XASM32_OPERAND_R8 && (oprinf).mod == MOD_R)
#define isR16(oprinf) ((oprinf).type == XASM32_OPERAND_R16 && (oprinf).mod == MOD_R)
#define isR32(oprinf) ((oprinf).type == XASM32_OPERAND_R32 && (oprinf).mod == MOD_R)
#define isSREG(oprinf) ((oprinf).type == XASM32_OPERAND_SREG && (oprinf).mod == MOD_R)
#define isCREG(oprinf) ((oprinf).type == XASM32_OPERAND_CREG)
#define isDREG(oprinf) ((oprinf).type == XASM32_OPERAND_DREG)
#define isTREG(oprinf) ((oprinf).type == XASM32_OPERAND_TREG)
#define isI8(oprinf) ((oprinf).type == XASM32_OPERAND_I8)
#define isI8u(oprinf) (isI8(oprinf) && !(oprinf).imms)
#define isI8s(oprinf) (isI8(oprinf) && (oprinf).imms)
#define isI16(oprinf) ((oprinf).type == XASM32_OPERAND_I16)
#define isI16u(oprinf) (isI16(oprinf) && !(oprinf).imms)
#define isI16s(oprinf) (isI16(oprinf) && (oprinf).imms)
#define isI32(oprinf) ((oprinf).type == XASM32_OPERAND_I32)
#define isI32u(oprinf) (isI32(oprinf) && !(oprinf).imms)
#define isI32s(oprinf) (isI32(oprinf) && (oprinf).imms)
#define isI16p(oprinf) ((oprinf).type == XASM32_OPERAND_I16_16)
#define isI32p(oprinf) ((oprinf).type == XASM32_OPERAND_I16_32)
#define isM(oprinf) (((oprinf).type == XASM32_OPERAND_M || (oprinf).type == XASM32_OPERAND_M8 ||     \
                      (oprinf).type == XASM32_OPERAND_M16 || (oprinf).type == XASM32_OPERAND_M32) && \
                     (oprinf).mod != MOD_R)
#define isM8(oprinf) (((oprinf).type == XASM32_OPERAND_M || (oprinf).type == XASM32_OPERAND_M8) && (oprinf).mod != MOD_R)
#define isM16(oprinf) (((oprinf).type == XASM32_OPERAND_M || (oprinf).type == XASM32_OPERAND_M16) && (oprinf).mod != MOD_R)
#define isM32(oprinf) (((oprinf).type == XASM32_OPERAND_M || (oprinf).type == XASM32_OPERAND_M32) && (oprinf).mod != MOD_R)
#define isMs(oprinf) ((oprinf).type == XASM32_OPERAND_M && (oprinf).mod != MOD_R)
#define isM8s(oprinf) ((oprinf).type == XASM32_OPERAND_M8 && (oprinf).mod != MOD_R)
#define isM16s(oprinf) ((oprinf).type == XASM32_OPERAND_M16 && (oprinf).mod != MOD_R)
#define isM32s(oprinf) ((oprinf).type == XASM32_OPERAND_M32 && (oprinf).mod != MOD_R)
#define isPNONE(oprinf) ((oprinf).ptr == PTR_NONE)
#define isNEAR(oprinf) ((oprinf).ptr == PTR_NEAR)
#define isSHORT(oprinf) ((oprinf).ptr == PTR_SHORT)
#define isFAR(oprinf) ((oprinf).ptr == PTR_FAR)
/* arg flag level 1 */
#define isRM8s(oprinf) (isR8(oprinf) || isM8s(oprinf))
#define isRM16s(oprinf) (isR16(oprinf) || isM16s(oprinf))
#define isRM32s(oprinf) (isR32(oprinf) || isM32s(oprinf))
#define isRM8(oprinf) (isR8(oprinf) || isM8(oprinf))
#define isRM16(oprinf) (isR16(oprinf) || isM16(oprinf))
#define isRM32(oprinf) (isR32(oprinf) || isM32(oprinf))
#define isRM(oprinf) (isRM8(oprinf) || isRM16(oprinf))
#define isCR0(oprinf) (isCREG(oprinf) && (oprinf).creg == CREG_CR0)
#define isCR2(oprinf) (isCREG(oprinf) && (oprinf).creg == CREG_CR2)
#define isCR3(oprinf) (isCREG(oprinf) && (oprinf).creg == CREG_CR3)
#define isDR0(oprinf) (isDREG(oprinf) && (oprinf).creg == DREG_DR0)
#define isDR1(oprinf) (isDREG(oprinf) && (oprinf).creg == DREG_DR1)
#define isDR2(oprinf) (isDREG(oprinf) && (oprinf).creg == DREG_DR2)
#define isDR3(oprinf) (isDREG(oprinf) && (oprinf).creg == DREG_DR3)
#define isDR6(oprinf) (isDREG(oprinf) && (oprinf).creg == DREG_DR6)
#define isDR7(oprinf) (isDREG(oprinf) && (oprinf).creg == DREG_DR7)
#define isTR6(oprinf) (isTREG(oprinf) && (oprinf).creg == TREG_TR6)
#define isTR7(oprinf) (isTREG(oprinf) && (oprinf).creg == TREG_TR7)
#define isAL(oprinf) (isR8(oprinf) && (oprinf).reg8 == R8_AL)
#define isCL(oprinf) (isR8(oprinf) && (oprinf).reg8 == R8_CL)
#define isDL(oprinf) (isR8(oprinf) && (oprinf).reg8 == R8_DL)
#define isBL(oprinf) (isR8(oprinf) && (oprinf).reg8 == R8_BL)
#define isAH(oprinf) (isR8(oprinf) && (oprinf).reg8 == R8_AH)
#define isCH(oprinf) (isR8(oprinf) && (oprinf).reg8 == R8_CH)
#define isDH(oprinf) (isR8(oprinf) && (oprinf).reg8 == R8_DH)
#define isBH(oprinf) (isR8(oprinf) && (oprinf).reg8 == R8_BH)
#define isAX(oprinf) (isR16(oprinf) && (oprinf).reg16 == R16_AX)
#define isCX(oprinf) (isR16(oprinf) && (oprinf).reg16 == R16_CX)
#define isDX(oprinf) (isR16(oprinf) && (oprinf).reg16 == R16_DX)
#define isBX(oprinf) (isR16(oprinf) && (oprinf).reg16 == R16_BX)
#define isSP(oprinf) (isR16(oprinf) && (oprinf).reg16 == R16_SP)
#define isBP(oprinf) (isR16(oprinf) && (oprinf).reg16 == R16_BP)
#define isSI(oprinf) (isR16(oprinf) && (oprinf).reg16 == R16_SI)
#define isDI(oprinf) (isR16(oprinf) && (oprinf).reg16 == R16_DI)
#define isEAX(oprinf) (isR32(oprinf) && (oprinf).reg32 == R32_EAX)
#define isECX(oprinf) (isR32(oprinf) && (oprinf).reg32 == R32_ECX)
#define isEDX(oprinf) (isR32(oprinf) && (oprinf).reg32 == R32_EDX)
#define isEBX(oprinf) (isR32(oprinf) && (oprinf).reg32 == R32_EBX)
#define isESP(oprinf) (isR32(oprinf) && (oprinf).reg32 == R32_ESP)
#define isEBP(oprinf) (isR32(oprinf) && (oprinf).reg32 == R32_EBP)
#define isESI(oprinf) (isR32(oprinf) && (oprinf).reg32 == R32_ESI)
#define isEDI(oprinf) (isR32(oprinf) && (oprinf).reg32 == R32_EDI)
#define isES(oprinf) (isSREG(oprinf) && (oprinf).sreg == SREG_ES)
#define isCS(oprinf) (isSREG(oprinf) && (oprinf).sreg == SREG_CS)
#define isSS(oprinf) (isSREG(oprinf) && (oprinf).sreg == SREG_SS)
#define isDS(oprinf) (isSREG(oprinf) && (oprinf).sreg == SREG_DS)
#define isFS(oprinf) (isSREG(oprinf) && (oprinf).sreg == SREG_FS)
#define isGS(oprinf) (isSREG(oprinf) && (oprinf).sreg == SREG_GS)
#define isESDI8(oprinf) (isM8(oprinf) && (oprinf).flages && (oprinf).mem == MEM_DI && (oprinf).mod == MOD_M)
#define isESDI16(oprinf) (isM16(oprinf) && (oprinf).flages && (oprinf).mem == MEM_DI && (oprinf).mod == MOD_M)
#define isESDI32(oprinf) (isM32(oprinf) && (oprinf).flages && (oprinf).mem == MEM_DI && (oprinf).mod == MOD_M)
#define isESEDI8(oprinf) (isM8(oprinf) && (oprinf).flages && (oprinf).mem == MEM_EDI && (oprinf).mod == MOD_M)
#define isESEDI16(oprinf) (isM16(oprinf) && (oprinf).flages && (oprinf).mem == MEM_EDI && (oprinf).mod == MOD_M)
#define isESEDI32(oprinf) (isM32(oprinf) && (oprinf).flages && (oprinf).mem == MEM_EDI && (oprinf).mod == MOD_M)
#define isDSSI8(oprinf) (isM8(oprinf) && (oprinf).mem == MEM_SI && (oprinf).mod == MOD_M)
#define isDSSI16(oprinf) (isM16(oprinf) && (oprinf).mem == MEM_SI && (oprinf).mod == MOD_M)
#define isDSSI32(oprinf) (isM32(oprinf) && (oprinf).mem == MEM_SI && (oprinf).mod == MOD_M)
#define isDSESI8(oprinf) (isM8(oprinf) && (oprinf).mem == MEM_ESI && (oprinf).mod == MOD_M)
#define isDSESI16(oprinf) (isM16(oprinf) && (oprinf).mem == MEM_ESI && (oprinf).mod == MOD_M)
#define isDSESI32(oprinf) (isM32(oprinf) && (oprinf).mem == MEM_ESI && (oprinf).mod == MOD_M)
#define isDSBXAL8(oprinf) (isM8(oprinf) && (oprinf).mem == MEM_BX_AL)
#define isDSEBXAL8(oprinf) (isM8(oprinf) && (oprinf).mem == MEM_EBX_AL)
#define isESDI8s(oprinf) (isM8s(oprinf) && (oprinf).flages && (oprinf).mem == MEM_DI && (oprinf).mod == MOD_M)
#define isESDI16s(oprinf) (isM16s(oprinf) && (oprinf).flages && (oprinf).mem == MEM_DI && (oprinf).mod == MOD_M)
#define isESDI32s(oprinf) (isM32s(oprinf) && (oprinf).flages && (oprinf).mem == MEM_DI && (oprinf).mod == MOD_M)
#define isESEDI8s(oprinf) (isM8s(oprinf) && (oprinf).flages && (oprinf).mem == MEM_EDI && (oprinf).mod == MOD_M)
#define isESEDI16s(oprinf) (isM16s(oprinf) && (oprinf).flages && (oprinf).mem == MEM_EDI && (oprinf).mod == MOD_M)
#define isESEDI32s(oprinf) (isM32s(oprinf) && (oprinf).flages && (oprinf).mem == MEM_EDI && (oprinf).mod == MOD_M)
#define isDSSI8s(oprinf) (isM8s(oprinf) && (oprinf).mem == MEM_SI && (oprinf).mod == MOD_M)
#define isDSSI16s(oprinf) (isM16s(oprinf) && (oprinf).mem == MEM_SI && (oprinf).mod == MOD_M)
#define isDSSI32s(oprinf) (isM32s(oprinf) && (oprinf).mem == MEM_SI && (oprinf).mod == MOD_M)
#define isDSESI8s(oprinf) (isM8s(oprinf) && (oprinf).mem == MEM_ESI && (oprinf).mod == MOD_M)
#define isDSESI16s(oprinf) (isM16s(oprinf) && (oprinf).mem == MEM_ESI && (oprinf).mod == MOD_M)
#define isDSESI32s(oprinf) (isM32s(oprinf) && (oprinf).mem == MEM_ESI && (oprinf).mod == MOD_M)
#define isDSBXAL8s(oprinf) (isM8s(oprinf) && (oprinf).mem == MEM_BX_AL)
#define isDSEBXAL8s(oprinf) (isM8s(oprinf) && (oprinf).mem == MEM_EBX_AL)

/* arg flag level 2 */
#define ARG_NONE (isNONE(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_I8 (isI8(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_I16 (isI16(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_I32 (isI32(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_I8s (isI8s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_I16s (isI16s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_I32s (isI32s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_R32 (isR32(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_M32 (isM32(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_M16s (isM16s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_M32s (isM32s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_RM8 (isRM8(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_RM8s (isRM8s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_RM16s (isRM16s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_RM32s (isRM32s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_RM8_R8 (isRM8(aopri1) && isR8(aopri2) && isNONE(aopri3))
#define ARG_RM16_R16 (isRM16(aopri1) && isR16(aopri2) && isNONE(aopri3))
#define ARG_RM32_R32 (isRM32(aopri1) && isR32(aopri2) && isNONE(aopri3))
#define ARG_R8_RM8 (isR8(aopri1) && isRM8(aopri2) && isNONE(aopri3))
#define ARG_R16_RM16 (isR16(aopri1) && isRM16(aopri2) && isNONE(aopri3))
#define ARG_R32_RM32 (isR32(aopri1) && isRM32(aopri2) && isNONE(aopri3))
#define ARG_R16_RM8s (isR16(aopri1) && isRM8s(aopri2) && isNONE(aopri3))
#define ARG_R32_RM8s (isR32(aopri1) && isRM8s(aopri2) && isNONE(aopri3))
#define ARG_R32_RM16s (isR32(aopri1) && isRM16s(aopri2) && isNONE(aopri3))
#define ARG_CR0_R32 (isCR0(aopri1) && isR32(aopri2) && isNONE(aopri3))
#define ARG_CR2_R32 (isCR2(aopri1) && isR32(aopri2) && isNONE(aopri3))
#define ARG_CR3_R32 (isCR3(aopri1) && isR32(aopri2) && isNONE(aopri3))
#define ARG_R32_CR0 (isR32(aopri1) && isCR0(aopri2) && isNONE(aopri3))
#define ARG_R32_CR2 (isR32(aopri1) && isCR2(aopri2) && isNONE(aopri3))
#define ARG_R32_CR3 (isR32(aopri1) && isCR3(aopri2) && isNONE(aopri3))
#define ARG_DR0_R32 (isDR0(aopri1) && isR32(aopri2) && isNONE(aopri3))
#define ARG_DR1_R32 (isDR1(aopri1) && isR32(aopri2) && isNONE(aopri3))
#define ARG_DR2_R32 (isDR2(aopri1) && isR32(aopri2) && isNONE(aopri3))
#define ARG_DR3_R32 (isDR3(aopri1) && isR32(aopri2) && isNONE(aopri3))
#define ARG_DR6_R32 (isDR6(aopri1) && isR32(aopri2) && isNONE(aopri3))
#define ARG_DR7_R32 (isDR7(aopri1) && isR32(aopri2) && isNONE(aopri3))
#define ARG_R32_DR0 (isR32(aopri1) && isDR0(aopri2) && isNONE(aopri3))
#define ARG_R32_DR1 (isR32(aopri1) && isDR1(aopri2) && isNONE(aopri3))
#define ARG_R32_DR2 (isR32(aopri1) && isDR2(aopri2) && isNONE(aopri3))
#define ARG_R32_DR3 (isR32(aopri1) && isDR3(aopri2) && isNONE(aopri3))
#define ARG_R32_DR6 (isR32(aopri1) && isDR6(aopri2) && isNONE(aopri3))
#define ARG_R32_DR7 (isR32(aopri1) && isDR7(aopri2) && isNONE(aopri3))
#define ARG_TR6_R32 (isTR6(aopri1) && isR32(aopri2) && isNONE(aopri3))
#define ARG_TR7_R32 (isTR7(aopri1) && isR32(aopri2) && isNONE(aopri3))
#define ARG_R32_TR6 (isR32(aopri1) && isTR6(aopri2) && isNONE(aopri3))
#define ARG_R32_TR7 (isR32(aopri1) && isTR7(aopri2) && isNONE(aopri3))
#define ARG_ES (isES(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_CS (isCS(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_SS (isSS(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_DS (isDS(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_FS (isFS(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_GS (isGS(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_AX (isAX(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_CX (isCX(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_DX (isDX(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_BX (isBX(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_SP (isSP(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_BP (isBP(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_SI (isSI(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_DI (isDI(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_EAX (isEAX(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_ECX (isECX(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_EDX (isEDX(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_EBX (isEBX(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_ESP (isESP(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_EBP (isEBP(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_ESI (isESI(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_EDI (isEDI(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_AL_I8u (isAL(aopri1) && isI8u(aopri2) && isNONE(aopri3))
#define ARG_AX_I8u (isAX(aopri1) && isI8u(aopri2) && isNONE(aopri3))
#define ARG_EAX_I8u (isEAX(aopri1) && isI8u(aopri2) && isNONE(aopri3))
#define ARG_I8u_AL (isI8u(aopri1) && isAL(aopri2) && isNONE(aopri3))
#define ARG_I8u_AX (isI8u(aopri1) && isAX(aopri2) && isNONE(aopri3))
#define ARG_I8u_EAX (isI8u(aopri1) && isEAX(aopri2) && isNONE(aopri3))
#define ARG_AL_I8 (isAL(aopri1) && isI8(aopri2) && isNONE(aopri3))
#define ARG_CL_I8 (isCL(aopri1) && isI8(aopri2) && isNONE(aopri3))
#define ARG_DL_I8 (isDL(aopri1) && isI8(aopri2) && isNONE(aopri3))
#define ARG_BL_I8 (isBL(aopri1) && isI8(aopri2) && isNONE(aopri3))
#define ARG_AH_I8 (isAH(aopri1) && isI8(aopri2) && isNONE(aopri3))
#define ARG_CH_I8 (isCH(aopri1) && isI8(aopri2) && isNONE(aopri3))
#define ARG_DH_I8 (isDH(aopri1) && isI8(aopri2) && isNONE(aopri3))
#define ARG_BH_I8 (isBH(aopri1) && isI8(aopri2) && isNONE(aopri3))
#define ARG_AX_I16 (isAX(aopri1) && isI16(aopri2) && isNONE(aopri3))
#define ARG_CX_I16 (isCX(aopri1) && isI16(aopri2) && isNONE(aopri3))
#define ARG_DX_I16 (isDX(aopri1) && isI16(aopri2) && isNONE(aopri3))
#define ARG_BX_I16 (isBX(aopri1) && isI16(aopri2) && isNONE(aopri3))
#define ARG_SP_I16 (isSP(aopri1) && isI16(aopri2) && isNONE(aopri3))
#define ARG_BP_I16 (isBP(aopri1) && isI16(aopri2) && isNONE(aopri3))
#define ARG_SI_I16 (isSI(aopri1) && isI16(aopri2) && isNONE(aopri3))
#define ARG_DI_I16 (isDI(aopri1) && isI16(aopri2) && isNONE(aopri3))
#define ARG_EAX_I32 (isEAX(aopri1) && isI32(aopri2) && isNONE(aopri3))
#define ARG_ECX_I32 (isECX(aopri1) && isI32(aopri2) && isNONE(aopri3))
#define ARG_EDX_I32 (isEDX(aopri1) && isI32(aopri2) && isNONE(aopri3))
#define ARG_EBX_I32 (isEBX(aopri1) && isI32(aopri2) && isNONE(aopri3))
#define ARG_ESP_I32 (isESP(aopri1) && isI32(aopri2) && isNONE(aopri3))
#define ARG_EBP_I32 (isEBP(aopri1) && isI32(aopri2) && isNONE(aopri3))
#define ARG_ESI_I32 (isESI(aopri1) && isI32(aopri2) && isNONE(aopri3))
#define ARG_EDI_I32 (isEDI(aopri1) && isI32(aopri2) && isNONE(aopri3))
#define ARG_RM8_I8 (isRM8s(aopri1) && isI8(aopri2) && isNONE(aopri3))
#define ARG_RM16_I16 (isRM16s(aopri1) && isI16(aopri2) && isNONE(aopri3))
#define ARG_RM32_I32 (isRM32s(aopri1) && isI32(aopri2) && isNONE(aopri3))
#define ARG_RM16_I8 (isRM16s(aopri1) && isI8(aopri2) && isNONE(aopri3))
#define ARG_RM32_I8 (isRM32s(aopri1) && isI8(aopri2) && isNONE(aopri3))
#define ARG_RM16_I8 (isRM16s(aopri1) && isI8(aopri2) && isNONE(aopri3))
#define ARG_I16_I8 (isI16(aopri1) && isI8(aopri2) && isNONE(aopri3))
#define ARG_RM16_SREG (isRM16(aopri1) && isSREG(aopri2) && isNONE(aopri3))
#define ARG_RM32_SREG (isRM32(aopri1) && isSREG(aopri2) && isNONE(aopri3))
#define ARG_SREG_RM16 (isSREG(aopri1) && isRM16(aopri2) && isNONE(aopri3))
#define ARG_SREG_RM32 (isSREG(aopri1) && isRM32(aopri2) && isNONE(aopri3))
#define ARG_RM16 (isRM16(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_RM32 (isRM32(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_AX_AX (isAX(aopri1) && isAX(aopri2) && isNONE(aopri3))
#define ARG_CX_AX (isCX(aopri1) && isAX(aopri2) && isNONE(aopri3))
#define ARG_DX_AX (isDX(aopri1) && isAX(aopri2) && isNONE(aopri3))
#define ARG_BX_AX (isBX(aopri1) && isAX(aopri2) && isNONE(aopri3))
#define ARG_SP_AX (isSP(aopri1) && isAX(aopri2) && isNONE(aopri3))
#define ARG_BP_AX (isBP(aopri1) && isAX(aopri2) && isNONE(aopri3))
#define ARG_SI_AX (isSI(aopri1) && isAX(aopri2) && isNONE(aopri3))
#define ARG_DI_AX (isDI(aopri1) && isAX(aopri2) && isNONE(aopri3))
#define ARG_EAX_EAX (isEAX(aopri1) && isEAX(aopri2) && isNONE(aopri3))
#define ARG_ECX_EAX (isECX(aopri1) && isEAX(aopri2) && isNONE(aopri3))
#define ARG_EDX_EAX (isEDX(aopri1) && isEAX(aopri2) && isNONE(aopri3))
#define ARG_EBX_EAX (isEBX(aopri1) && isEAX(aopri2) && isNONE(aopri3))
#define ARG_ESP_EAX (isESP(aopri1) && isEAX(aopri2) && isNONE(aopri3))
#define ARG_EBP_EAX (isEBP(aopri1) && isEAX(aopri2) && isNONE(aopri3))
#define ARG_ESI_EAX (isESI(aopri1) && isEAX(aopri2) && isNONE(aopri3))
#define ARG_EDI_EAX (isEDI(aopri1) && isEAX(aopri2) && isNONE(aopri3))
#define ARG_AL_MOFFS8 (isAL(aopri1) && isM8(aopri2) && (aopri2.mod == MOD_M && (aopri2.mem == MEM_BP || aopri2.mem == MEM_EBP)) && isNONE(aopri3))
#define ARG_MOFFS8_AL (isM8(aopri1) && isAL(aopri2) && (aopri1.mod == MOD_M && (aopri1.mem == MEM_BP || aopri2.mem == MEM_EBP)) && isNONE(aopri3))
#define ARG_AX_MOFFS16 (isAX(aopri1) && isM16(aopri2) && (aopri2.mod == MOD_M && (aopri2.mem == MEM_BP || aopri2.mem == MEM_EBP)) && isNONE(aopri3))
#define ARG_MOFFS16_AX (isM16(aopri1) && isAX(aopri2) && (aopri1.mod == MOD_M && (aopri1.mem == MEM_BP || aopri1.mem == MEM_EBP)) && isNONE(aopri3))
#define ARG_EAX_MOFFS32 (isEAX(aopri1) && isM32(aopri2) && (aopri2.mod == MOD_M && (aopri2.mem == MEM_BP || aopri2.mem == MEM_EBP)) && isNONE(aopri3))
#define ARG_MOFFS32_EAX (isM32(aopri1) && isEAX(aopri2) && (aopri1.mod == MOD_M && (aopri1.mem == MEM_BP || aopri1.mem == MEM_EBP)) && isNONE(aopri3))
#define ARG_R16_M16 (isR16(aopri1) && isM16(aopri2) && isNONE(aopri3))
#define ARG_R32_M32 (isR32(aopri1) && isM32(aopri2) && isNONE(aopri3))
#define ARG_I16u (isI16u(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_PNONE_I8s (isPNONE(aopri1) && isI8s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_PNONE_I16s (isPNONE(aopri1) && isI16s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_PNONE_I32s (isPNONE(aopri1) && isI32s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_SHORT_I8s (isSHORT(aopri1) && isI8s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_NEAR_I16s (isNEAR(aopri1) && isI16s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_NEAR_I32s (isNEAR(aopri1) && isI32s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_FAR_I16_16 (isFAR(aopri1) && isI16p(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_FAR_I16_32 (isFAR(aopri1) && isI32p(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_PNONE_RM16s (isPNONE(aopri1) && isRM16s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_NEAR_RM16s (isNEAR(aopri1) && isRM16s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_PNONE_RM32s (isPNONE(aopri1) && isRM32s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_NEAR_RM32s (isNEAR(aopri1) && isRM32s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_FAR_M16_16 (isFAR(aopri1) && isM16s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_FAR_M16_32 (isFAR(aopri1) && isM32s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_RM8_CL (isRM8s(aopri1) && isCL(aopri2) && isNONE(aopri3))
#define ARG_RM16_CL (isRM16s(aopri1) && isCL(aopri2) && isNONE(aopri3))
#define ARG_RM32_CL (isRM32s(aopri1) && isCL(aopri2) && isNONE(aopri3))
#define ARG_AL_DX (isAL(aopri1) && isDX(aopri2) && isNONE(aopri3))
#define ARG_DX_AL (isDX(aopri1) && isAL(aopri2) && isNONE(aopri3))
#define ARG_AX_DX (isAX(aopri1) && isDX(aopri2) && isNONE(aopri3))
#define ARG_EAX_DX (isEAX(aopri1) && isDX(aopri2) && isNONE(aopri3))
#define ARG_DX_EAX (isDX(aopri1) && isEAX(aopri2) && isNONE(aopri3))
#define ARG_ESDI8_DSSI8 (isESDI8(aopri1) && isDSSI8(aopri2) && isNONE(aopri3))
#define ARG_ESDI16_DSSI16 (isESDI16(aopri1) && isDSSI16(aopri2) && isNONE(aopri3))
#define ARG_ESDI32_DSSI32 (isESDI32(aopri1) && isDSSI32(aopri2) && isNONE(aopri3))
#define ARG_DSSI8_ESDI8 (isDSSI8(aopri1) && isESDI8(aopri2) && isNONE(aopri3))
#define ARG_DSSI16_ESDI16 (isDSSI16(aopri1) && isESDI16(aopri2) && isNONE(aopri3))
#define ARG_DSSI32_ESDI32 (isDSSI32(aopri1) && isESDI32(aopri2) && isNONE(aopri3))
#define ARG_ESDI8_DX (isESDI8(aopri1) && isDX(aopri2) && isNONE(aopri3))
#define ARG_ESDI16_DX (isESDI16(aopri1) && isDX(aopri2) && isNONE(aopri3))
#define ARG_ESDI32_DX (isESDI32(aopri1) && isDX(aopri2) && isNONE(aopri3))
#define ARG_DX_DSSI8 (isDX(aopri1) && isDSSI8(aopri2) && isNONE(aopri3))
#define ARG_DX_DSSI16 (isDX(aopri1) && isDSSI16(aopri2) && isNONE(aopri3))
#define ARG_DX_DSSI32 (isDX(aopri1) && isDSSI32(aopri2) && isNONE(aopri3))
#define ARG_DSSI8 (isDSSI8(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_DSSI16 (isDSSI16(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_DSSI32 (isDSSI32(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_ESDI8 (isESDI8(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_ESDI16 (isESDI16(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_ESDI32 (isESDI32(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_ESEDI8_DSESI8 (isESEDI8(aopri1) && isDSESI8(aopri2) && isNONE(aopri3))
#define ARG_ESEDI16_DSESI16 (isESEDI16(aopri1) && isDSESI16(aopri2) && isNONE(aopri3))
#define ARG_ESEDI32_DSESI32 (isESEDI32(aopri1) && isDSESI32(aopri2) && isNONE(aopri3))
#define ARG_DSESI8_ESEDI8 (isDSESI8(aopri1) && isESEDI8(aopri2) && isNONE(aopri3))
#define ARG_DSESI16_ESEDI16 (isDSESI16(aopri1) && isESEDI16(aopri2) && isNONE(aopri3))
#define ARG_DSESI32_ESEDI32 (isDSESI32(aopri1) && isESEDI32(aopri2) && isNONE(aopri3))
#define ARG_ESEDI8_DX (isESEDI8(aopri1) && isDX(aopri2) && isNONE(aopri3))
#define ARG_ESEDI16_DX (isESEDI16(aopri1) && isDX(aopri2) && isNONE(aopri3))
#define ARG_ESEDI32_DX (isESEDI32(aopri1) && isDX(aopri2) && isNONE(aopri3))
#define ARG_DX_DSESI8 (isDX(aopri1) && isDSESI8(aopri2) && isNONE(aopri3))
#define ARG_DX_DSESI16 (isDX(aopri1) && isDSESI16(aopri2) && isNONE(aopri3))
#define ARG_DX_DSESI32 (isDX(aopri1) && isDSESI32(aopri2) && isNONE(aopri3))
#define ARG_DSESI8 (isDSESI8(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_DSESI16 (isDSESI16(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_DSESI32 (isDSESI32(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_ESEDI8 (isESEDI8(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_ESEDI16 (isESEDI16(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_ESEDI32 (isESEDI32(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_DSBXAL8 (isDSBXAL8(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_DSEBXAL8 (isDSEBXAL8(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_ESDI8_DSSI8 (isESDI8(aopri1) && isDSSI8(aopri2) && isNONE(aopri3))
#define ARG_ESDI16_DSSI16 (isESDI16(aopri1) && isDSSI16(aopri2) && isNONE(aopri3))
#define ARG_ESDI32_DSSI32 (isESDI32(aopri1) && isDSSI32(aopri2) && isNONE(aopri3))
#define ARG_DSSI8_ESDI8 (isDSSI8(aopri1) && isESDI8(aopri2) && isNONE(aopri3))
#define ARG_DSSI16_ESDI16 (isDSSI16(aopri1) && isESDI16(aopri2) && isNONE(aopri3))
#define ARG_DSSI32_ESDI32 (isDSSI32(aopri1) && isESDI32(aopri2) && isNONE(aopri3))
#define ARG_ESDI8s_DSSI8s (isESDI8s(aopri1) && isDSSI8s(aopri2) && isNONE(aopri3))
#define ARG_ESDI16s_DSSI16s (isESDI16s(aopri1) && isDSSI16s(aopri2) && isNONE(aopri3))
#define ARG_ESDI32s_DSSI32s (isESDI32s(aopri1) && isDSSI32s(aopri2) && isNONE(aopri3))
#define ARG_DSSI8s_ESDI8s (isDSSI8s(aopri1) && isESDI8s(aopri2) && isNONE(aopri3))
#define ARG_DSSI16s_ESDI16s (isDSSI16s(aopri1) && isESDI16s(aopri2) && isNONE(aopri3))
#define ARG_DSSI32s_ESDI32s (isDSSI32s(aopri1) && isESDI32s(aopri2) && isNONE(aopri3))
#define ARG_ESDI8s_DX (isESDI8s(aopri1) && isDX(aopri2) && isNONE(aopri3))
#define ARG_ESDI16s_DX (isESDI16s(aopri1) && isDX(aopri2) && isNONE(aopri3))
#define ARG_ESDI32s_DX (isESDI32s(aopri1) && isDX(aopri2) && isNONE(aopri3))
#define ARG_DX_DSSI8s (isDX(aopri1) && isDSSI8s(aopri2) && isNONE(aopri3))
#define ARG_DX_DSSI16s (isDX(aopri1) && isDSSI16s(aopri2) && isNONE(aopri3))
#define ARG_DX_DSSI32s (isDX(aopri1) && isDSSI32s(aopri2) && isNONE(aopri3))
#define ARG_DSSI8s (isDSSI8s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_DSSI16s (isDSSI16s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_DSSI32s (isDSSI32s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_ESDI8s (isESDI8s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_ESDI16s (isESDI16s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_ESDI32s (isESDI32s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_ESEDI8s_DSESI8s (isESEDI8s(aopri1) && isDSESI8s(aopri2) && isNONE(aopri3))
#define ARG_ESEDI16s_DSESI16s (isESEDI16s(aopri1) && isDSESI16s(aopri2) && isNONE(aopri3))
#define ARG_ESEDI32s_DSESI32s (isESEDI32s(aopri1) && isDSESI32s(aopri2) && isNONE(aopri3))
#define ARG_DSESI8s_ESEDI8s (isDSESI8s(aopri1) && isESEDI8s(aopri2) && isNONE(aopri3))
#define ARG_DSESI16s_ESEDI16s (isDSESI16s(aopri1) && isESEDI16s(aopri2) && isNONE(aopri3))
#define ARG_DSESI32s_ESEDI32s (isDSESI32s(aopri1) && isESEDI32s(aopri2) && isNONE(aopri3))
#define ARG_ESEDI8s_DX (isESEDI8s(aopri1) && isDX(aopri2) && isNONE(aopri3))
#define ARG_ESEDI16s_DX (isESEDI16s(aopri1) && isDX(aopri2) && isNONE(aopri3))
#define ARG_ESEDI32s_DX (isESEDI32s(aopri1) && isDX(aopri2) && isNONE(aopri3))
#define ARG_DX_DSESI8s (isDX(aopri1) && isDSESI8s(aopri2) && isNONE(aopri3))
#define ARG_DX_DSESI16s (isDX(aopri1) && isDSESI16s(aopri2) && isNONE(aopri3))
#define ARG_DX_DSESI32s (isDX(aopri1) && isDSESI32s(aopri2) && isNONE(aopri3))
#define ARG_DSESI8s (isDSESI8s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_DSESI16s (isDSESI16s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_DSESI32s (isDSESI32s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_ESEDI8s (isESEDI8s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_ESEDI16s (isESEDI16s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_ESEDI32s (isESEDI32s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_DSBXAL8s (isDSBXAL8s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_DSEBXAL8s (isDSEBXAL8s(aopri1) && isNONE(aopri2) && isNONE(aopri3))
#define ARG_R16_RM16_I8 (isR16(aopri1) && isRM16(aopri2) && isI8(aopri3))
#define ARG_R32_RM32_I8 (isR32(aopri1) && isRM32(aopri2) && isI8(aopri3))
#define ARG_R16_RM16_I16 (isR16(aopri1) && isRM16(aopri2) && isI16(aopri3))
#define ARG_R32_RM32_I32 (isR32(aopri1) && isRM32(aopri2) && isI32(aopri3))
#define ARG_RM16_R16_I8 (isRM16(aopri1) && isR16(aopri2) && isI8(aopri3))
#define ARG_RM32_R32_I8 (isRM32(aopri1) && isR32(aopri2) && isI8(aopri3))
#define ARG_RM16_R16_CL (isRM16(aopri1) && isR16(aopri2) && isCL(aopri3))
#define ARG_RM32_R32_CL (isRM32(aopri1) && isR32(aopri2) && isCL(aopri3))
/* assembly compiler: lexical scanner */
typedef enum
{
    STATE_START,
    STATE_BY,
    STATE_BYT, /* BYTE */
    STATE_W,
    STATE_WO,
    STATE_WOR, /* WORD */
    STATE_DW,
    STATE_DWO, /* DWORD */
    STATE_DWOR,
    STATE_P,
    STATE_PT, /* PTR */
    STATE_N,
    STATE_NE,
    STATE_NEA, /* NEAR */
    STATE_EA,  /* EAX */
    STATE_EC,  /* ECX */
    STATE_ED,  /* EDX, EDI */
    STATE_EB,  /* EBX, EBP */
    STATE_ES,  /* ESP, ESI */
    STATE_FA,  /* FAR */
    STATE_SH,
    STATE_SHO,
    STATE_SHOR, /* SHORT */
    STATE_CR,   /* CRn */
    STATE_DR,   /* DRn */
    STATE_T,
    STATE_TR,   /* TRn */
    STATE_A,    /* AX, AH, AL, NUM */
    STATE_B,    /* BX, BH, BL, BP, NUM */
    STATE_C,    /* CX, CH, CL, CS, NUM */
    STATE_D,    /* DX, DH, DL, DS, DI, NUM, DWORD */
    STATE_E,    /* ES, NUM */
    STATE_F,    /* FS, NUM, FAR */
    STATE_G,    /* GS, NUM, FAR */
    STATE_S,    /* SS, SP, SI, SHORT */
    STATE_NUM1, /* NUM */
    STATE_NUM2,
    STATE_NUM3,
    STATE_NUM4,
    STATE_NUM5,
    STATE_NUM6,
    STATE_NUM7,
    STATE_NUM8
} t_aasm_scan_state;
typedef enum
{
    TOKEN_NULL,
    TOKEN_END,
    TOKEN_LSPAREN,
    TOKEN_RSPAREN,
    TOKEN_COLON,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_TIMES,
    TOKEN_BYTE,
    TOKEN_WORD,
    TOKEN_DWORD,
    TOKEN_SHORT,
    TOKEN_NEAR,
    TOKEN_FAR,
    TOKEN_PTR,
    TOKEN_IMM8,
    TOKEN_IMM16,
    TOKEN_IMM32,
    TOKEN_AH,
    TOKEN_BH,
    TOKEN_CH,
    TOKEN_DH,
    TOKEN_AL,
    TOKEN_BL,
    TOKEN_CL,
    TOKEN_DL,
    TOKEN_AX,
    TOKEN_BX,
    TOKEN_CX,
    TOKEN_DX,
    TOKEN_SP,
    TOKEN_BP,
    TOKEN_SI,
    TOKEN_DI,
    TOKEN_EAX,
    TOKEN_EBX,
    TOKEN_ECX,
    TOKEN_EDX,
    TOKEN_ESP,
    TOKEN_EBP,
    TOKEN_ESI,
    TOKEN_EDI,
    TOKEN_ES,
    TOKEN_CS,
    TOKEN_SS,
    TOKEN_DS,
    TOKEN_FS,
    TOKEN_GS,
    TOKEN_CR0,
    TOKEN_CR2,
    TOKEN_CR3,
    TOKEN_DR0,
    TOKEN_DR1,
    TOKEN_DR2,
    TOKEN_DR3,
    TOKEN_DR6,
    TOKEN_DR7,
    TOKEN_TR6,
    TOKEN_TR7,
    TOKEN_DOLLAR
} t_aasm_token;
/* token variables */
#define tokch (*tokptr)
#define take(n) (flagend = 1, token = (n))
static t_aasm_token gettoken(aasm32_context *aasmContext, char *str)
{
    lib_u32 tokimm = 0;
    lib_u8 flagend = 0;
    t_aasm_token token = TOKEN_NULL;
    t_aasm_scan_state state = STATE_START;
    XASM32_TRACE_CALL_BEGIN("gettoken");
    tokimm8 = 0x00;
    tokimm16 = 0x0000;
    tokimm32 = 0x00000000;
    if (str)
        tokptr = str;
    if (!tokptr)
    {
        XASM32_TRACE_CALL_END;
        return token;
    }
    do
    {
        switch (state)
        {
        case STATE_START:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_START)");
            switch (tokch)
            {
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
                state = STATE_NUM1;
                break;
            case '1':
                tokimm = 0x1;
                state = STATE_NUM1;
                break;
            case '2':
                tokimm = 0x2;
                state = STATE_NUM1;
                break;
            case '3':
                tokimm = 0x3;
                state = STATE_NUM1;
                break;
            case '4':
                tokimm = 0x4;
                state = STATE_NUM1;
                break;
            case '5':
                tokimm = 0x5;
                state = STATE_NUM1;
                break;
            case '6':
                tokimm = 0x6;
                state = STATE_NUM1;
                break;
            case '7':
                tokimm = 0x7;
                state = STATE_NUM1;
                break;
            case '8':
                tokimm = 0x8;
                state = STATE_NUM1;
                break;
            case '9':
                tokimm = 0x9;
                state = STATE_NUM1;
                break;
            case 'a':
                tokimm = 0xa;
                state = STATE_A;
                break;
            case 'b':
                tokimm = 0xb;
                state = STATE_B;
                break;
            case 'c':
                tokimm = 0xc;
                state = STATE_C;
                break;
            case 'd':
                tokimm = 0xd;
                state = STATE_D;
                break;
            case 'e':
                tokimm = 0xe;
                state = STATE_E;
                break;
            case 'f':
                tokimm = 0xf;
                state = STATE_F;
                break;
            case 'g':
                tokimm = 0xf;
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
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_NUM1:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_NUM1)");
            switch (tokch)
            {
            case '0':
                tokimm = (tokimm << 4) | 0x0;
                state = STATE_NUM2;
                break;
            case '1':
                tokimm = (tokimm << 4) | 0x1;
                state = STATE_NUM2;
                break;
            case '2':
                tokimm = (tokimm << 4) | 0x2;
                state = STATE_NUM2;
                break;
            case '3':
                tokimm = (tokimm << 4) | 0x3;
                state = STATE_NUM2;
                break;
            case '4':
                tokimm = (tokimm << 4) | 0x4;
                state = STATE_NUM2;
                break;
            case '5':
                tokimm = (tokimm << 4) | 0x5;
                state = STATE_NUM2;
                break;
            case '6':
                tokimm = (tokimm << 4) | 0x6;
                state = STATE_NUM2;
                break;
            case '7':
                tokimm = (tokimm << 4) | 0x7;
                state = STATE_NUM2;
                break;
            case '8':
                tokimm = (tokimm << 4) | 0x8;
                state = STATE_NUM2;
                break;
            case '9':
                tokimm = (tokimm << 4) | 0x9;
                state = STATE_NUM2;
                break;
            case 'a':
                tokimm = (tokimm << 4) | 0xa;
                state = STATE_NUM2;
                break;
            case 'b':
                tokimm = (tokimm << 4) | 0xb;
                state = STATE_NUM2;
                break;
            case 'c':
                tokimm = (tokimm << 4) | 0xc;
                state = STATE_NUM2;
                break;
            case 'd':
                tokimm = (tokimm << 4) | 0xd;
                state = STATE_NUM2;
                break;
            case 'e':
                tokimm = (tokimm << 4) | 0xe;
                state = STATE_NUM2;
                break;
            case 'f':
                tokimm = (tokimm << 4) | 0xf;
                state = STATE_NUM2;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_NUM2:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_NUM2)");
            switch (tokch)
            {
            case '0':
                tokimm = (tokimm << 4) | 0x0;
                state = STATE_NUM3;
                break;
            case '1':
                tokimm = (tokimm << 4) | 0x1;
                state = STATE_NUM3;
                break;
            case '2':
                tokimm = (tokimm << 4) | 0x2;
                state = STATE_NUM3;
                break;
            case '3':
                tokimm = (tokimm << 4) | 0x3;
                state = STATE_NUM3;
                break;
            case '4':
                tokimm = (tokimm << 4) | 0x4;
                state = STATE_NUM3;
                break;
            case '5':
                tokimm = (tokimm << 4) | 0x5;
                state = STATE_NUM3;
                break;
            case '6':
                tokimm = (tokimm << 4) | 0x6;
                state = STATE_NUM3;
                break;
            case '7':
                tokimm = (tokimm << 4) | 0x7;
                state = STATE_NUM3;
                break;
            case '8':
                tokimm = (tokimm << 4) | 0x8;
                state = STATE_NUM3;
                break;
            case '9':
                tokimm = (tokimm << 4) | 0x9;
                state = STATE_NUM3;
                break;
            case 'a':
                tokimm = (tokimm << 4) | 0xa;
                state = STATE_NUM3;
                break;
            case 'b':
                tokimm = (tokimm << 4) | 0xb;
                state = STATE_NUM3;
                break;
            case 'c':
                tokimm = (tokimm << 4) | 0xc;
                state = STATE_NUM3;
                break;
            case 'd':
                tokimm = (tokimm << 4) | 0xd;
                state = STATE_NUM3;
                break;
            case 'e':
                tokimm = (tokimm << 4) | 0xe;
                state = STATE_NUM3;
                break;
            case 'f':
                tokimm = (tokimm << 4) | 0xf;
                state = STATE_NUM3;
                break;
            default:
                tokptr--;
                tokimm8 = (lib_u8)tokimm;
                take(TOKEN_IMM8);
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_NUM3:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_NUM3)");
            switch (tokch)
            {
            case '0':
                tokimm = (tokimm << 4) | 0x0;
                state = STATE_NUM4;
                break;
            case '1':
                tokimm = (tokimm << 4) | 0x1;
                state = STATE_NUM4;
                break;
            case '2':
                tokimm = (tokimm << 4) | 0x2;
                state = STATE_NUM4;
                break;
            case '3':
                tokimm = (tokimm << 4) | 0x3;
                state = STATE_NUM4;
                break;
            case '4':
                tokimm = (tokimm << 4) | 0x4;
                state = STATE_NUM4;
                break;
            case '5':
                tokimm = (tokimm << 4) | 0x5;
                state = STATE_NUM4;
                break;
            case '6':
                tokimm = (tokimm << 4) | 0x6;
                state = STATE_NUM4;
                break;
            case '7':
                tokimm = (tokimm << 4) | 0x7;
                state = STATE_NUM4;
                break;
            case '8':
                tokimm = (tokimm << 4) | 0x8;
                state = STATE_NUM4;
                break;
            case '9':
                tokimm = (tokimm << 4) | 0x9;
                state = STATE_NUM4;
                break;
            case 'a':
                tokimm = (tokimm << 4) | 0xa;
                state = STATE_NUM4;
                break;
            case 'b':
                tokimm = (tokimm << 4) | 0xb;
                state = STATE_NUM4;
                break;
            case 'c':
                tokimm = (tokimm << 4) | 0xc;
                state = STATE_NUM4;
                break;
            case 'd':
                tokimm = (tokimm << 4) | 0xd;
                state = STATE_NUM4;
                break;
            case 'e':
                tokimm = (tokimm << 4) | 0xe;
                state = STATE_NUM4;
                break;
            case 'f':
                tokimm = (tokimm << 4) | 0xf;
                state = STATE_NUM4;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_NUM4:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_NUM4)");
            switch (tokch)
            {
            case '0':
                tokimm = (tokimm << 4) | 0x0;
                state = STATE_NUM5;
                break;
            case '1':
                tokimm = (tokimm << 4) | 0x1;
                state = STATE_NUM5;
                break;
            case '2':
                tokimm = (tokimm << 4) | 0x2;
                state = STATE_NUM5;
                break;
            case '3':
                tokimm = (tokimm << 4) | 0x3;
                state = STATE_NUM5;
                break;
            case '4':
                tokimm = (tokimm << 4) | 0x4;
                state = STATE_NUM5;
                break;
            case '5':
                tokimm = (tokimm << 4) | 0x5;
                state = STATE_NUM5;
                break;
            case '6':
                tokimm = (tokimm << 4) | 0x6;
                state = STATE_NUM5;
                break;
            case '7':
                tokimm = (tokimm << 4) | 0x7;
                state = STATE_NUM5;
                break;
            case '8':
                tokimm = (tokimm << 4) | 0x8;
                state = STATE_NUM5;
                break;
            case '9':
                tokimm = (tokimm << 4) | 0x9;
                state = STATE_NUM5;
                break;
            case 'a':
                tokimm = (tokimm << 4) | 0xa;
                state = STATE_NUM5;
                break;
            case 'b':
                tokimm = (tokimm << 4) | 0xb;
                state = STATE_NUM5;
                break;
            case 'c':
                tokimm = (tokimm << 4) | 0xc;
                state = STATE_NUM5;
                break;
            case 'd':
                tokimm = (tokimm << 4) | 0xd;
                state = STATE_NUM5;
                break;
            case 'e':
                tokimm = (tokimm << 4) | 0xe;
                state = STATE_NUM5;
                break;
            case 'f':
                tokimm = (tokimm << 4) | 0xf;
                state = STATE_NUM5;
                break;
            default:
                tokptr--;
                tokimm16 = (lib_u16)tokimm;
                take(TOKEN_IMM16);
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_NUM5:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_NUM5)");
            switch (tokch)
            {
            case '0':
                tokimm = (tokimm << 4) | 0x0;
                state = STATE_NUM6;
                break;
            case '1':
                tokimm = (tokimm << 4) | 0x1;
                state = STATE_NUM6;
                break;
            case '2':
                tokimm = (tokimm << 4) | 0x2;
                state = STATE_NUM6;
                break;
            case '3':
                tokimm = (tokimm << 4) | 0x3;
                state = STATE_NUM6;
                break;
            case '4':
                tokimm = (tokimm << 4) | 0x4;
                state = STATE_NUM6;
                break;
            case '5':
                tokimm = (tokimm << 4) | 0x5;
                state = STATE_NUM6;
                break;
            case '6':
                tokimm = (tokimm << 4) | 0x6;
                state = STATE_NUM6;
                break;
            case '7':
                tokimm = (tokimm << 4) | 0x7;
                state = STATE_NUM6;
                break;
            case '8':
                tokimm = (tokimm << 4) | 0x8;
                state = STATE_NUM6;
                break;
            case '9':
                tokimm = (tokimm << 4) | 0x9;
                state = STATE_NUM6;
                break;
            case 'a':
                tokimm = (tokimm << 4) | 0xa;
                state = STATE_NUM6;
                break;
            case 'b':
                tokimm = (tokimm << 4) | 0xb;
                state = STATE_NUM6;
                break;
            case 'c':
                tokimm = (tokimm << 4) | 0xc;
                state = STATE_NUM6;
                break;
            case 'd':
                tokimm = (tokimm << 4) | 0xd;
                state = STATE_NUM6;
                break;
            case 'e':
                tokimm = (tokimm << 4) | 0xe;
                state = STATE_NUM6;
                break;
            case 'f':
                tokimm = (tokimm << 4) | 0xf;
                state = STATE_NUM6;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_NUM6:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_NUM6)");
            switch (tokch)
            {
            case '0':
                tokimm = (tokimm << 4) | 0x0;
                state = STATE_NUM7;
                break;
            case '1':
                tokimm = (tokimm << 4) | 0x1;
                state = STATE_NUM7;
                break;
            case '2':
                tokimm = (tokimm << 4) | 0x2;
                state = STATE_NUM7;
                break;
            case '3':
                tokimm = (tokimm << 4) | 0x3;
                state = STATE_NUM7;
                break;
            case '4':
                tokimm = (tokimm << 4) | 0x4;
                state = STATE_NUM7;
                break;
            case '5':
                tokimm = (tokimm << 4) | 0x5;
                state = STATE_NUM7;
                break;
            case '6':
                tokimm = (tokimm << 4) | 0x6;
                state = STATE_NUM7;
                break;
            case '7':
                tokimm = (tokimm << 4) | 0x7;
                state = STATE_NUM7;
                break;
            case '8':
                tokimm = (tokimm << 4) | 0x8;
                state = STATE_NUM7;
                break;
            case '9':
                tokimm = (tokimm << 4) | 0x9;
                state = STATE_NUM7;
                break;
            case 'a':
                tokimm = (tokimm << 4) | 0xa;
                state = STATE_NUM7;
                break;
            case 'b':
                tokimm = (tokimm << 4) | 0xb;
                state = STATE_NUM7;
                break;
            case 'c':
                tokimm = (tokimm << 4) | 0xc;
                state = STATE_NUM7;
                break;
            case 'd':
                tokimm = (tokimm << 4) | 0xd;
                state = STATE_NUM7;
                break;
            case 'e':
                tokimm = (tokimm << 4) | 0xe;
                state = STATE_NUM7;
                break;
            case 'f':
                tokimm = (tokimm << 4) | 0xf;
                state = STATE_NUM7;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_NUM7:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_NUM7)");
            switch (tokch)
            {
            case '0':
                tokimm = (tokimm << 4) | 0x0;
                state = STATE_NUM8;
                break;
            case '1':
                tokimm = (tokimm << 4) | 0x1;
                state = STATE_NUM8;
                break;
            case '2':
                tokimm = (tokimm << 4) | 0x2;
                state = STATE_NUM8;
                break;
            case '3':
                tokimm = (tokimm << 4) | 0x3;
                state = STATE_NUM8;
                break;
            case '4':
                tokimm = (tokimm << 4) | 0x4;
                state = STATE_NUM8;
                break;
            case '5':
                tokimm = (tokimm << 4) | 0x5;
                state = STATE_NUM8;
                break;
            case '6':
                tokimm = (tokimm << 4) | 0x6;
                state = STATE_NUM8;
                break;
            case '7':
                tokimm = (tokimm << 4) | 0x7;
                state = STATE_NUM8;
                break;
            case '8':
                tokimm = (tokimm << 4) | 0x8;
                state = STATE_NUM8;
                break;
            case '9':
                tokimm = (tokimm << 4) | 0x9;
                state = STATE_NUM8;
                break;
            case 'a':
                tokimm = (tokimm << 4) | 0xa;
                state = STATE_NUM8;
                break;
            case 'b':
                tokimm = (tokimm << 4) | 0xb;
                state = STATE_NUM8;
                break;
            case 'c':
                tokimm = (tokimm << 4) | 0xc;
                state = STATE_NUM8;
                break;
            case 'd':
                tokimm = (tokimm << 4) | 0xd;
                state = STATE_NUM8;
                break;
            case 'e':
                tokimm = (tokimm << 4) | 0xe;
                state = STATE_NUM8;
                break;
            case 'f':
                tokimm = (tokimm << 4) | 0xf;
                state = STATE_NUM8;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_NUM8:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_NUM8)");
            switch (tokch)
            {
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
                tokimm32 = (lib_u32)tokimm;
                take(TOKEN_IMM32);
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_A:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_A)");
            switch (tokch)
            {
            case '0':
                tokimm = (tokimm << 4) | 0x0;
                state = STATE_NUM2;
                break;
            case '1':
                tokimm = (tokimm << 4) | 0x1;
                state = STATE_NUM2;
                break;
            case '2':
                tokimm = (tokimm << 4) | 0x2;
                state = STATE_NUM2;
                break;
            case '3':
                tokimm = (tokimm << 4) | 0x3;
                state = STATE_NUM2;
                break;
            case '4':
                tokimm = (tokimm << 4) | 0x4;
                state = STATE_NUM2;
                break;
            case '5':
                tokimm = (tokimm << 4) | 0x5;
                state = STATE_NUM2;
                break;
            case '6':
                tokimm = (tokimm << 4) | 0x6;
                state = STATE_NUM2;
                break;
            case '7':
                tokimm = (tokimm << 4) | 0x7;
                state = STATE_NUM2;
                break;
            case '8':
                tokimm = (tokimm << 4) | 0x8;
                state = STATE_NUM2;
                break;
            case '9':
                tokimm = (tokimm << 4) | 0x9;
                state = STATE_NUM2;
                break;
            case 'a':
                tokimm = (tokimm << 4) | 0xa;
                state = STATE_NUM2;
                break;
            case 'b':
                tokimm = (tokimm << 4) | 0xb;
                state = STATE_NUM2;
                break;
            case 'c':
                tokimm = (tokimm << 4) | 0xc;
                state = STATE_NUM2;
                break;
            case 'd':
                tokimm = (tokimm << 4) | 0xd;
                state = STATE_NUM2;
                break;
            case 'e':
                tokimm = (tokimm << 4) | 0xe;
                state = STATE_NUM2;
                break;
            case 'f':
                tokimm = (tokimm << 4) | 0xf;
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
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_B:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_B)");
            switch (tokch)
            {
            case '0':
                tokimm = (tokimm << 4) | 0x0;
                state = STATE_NUM2;
                break;
            case '1':
                tokimm = (tokimm << 4) | 0x1;
                state = STATE_NUM2;
                break;
            case '2':
                tokimm = (tokimm << 4) | 0x2;
                state = STATE_NUM2;
                break;
            case '3':
                tokimm = (tokimm << 4) | 0x3;
                state = STATE_NUM2;
                break;
            case '4':
                tokimm = (tokimm << 4) | 0x4;
                state = STATE_NUM2;
                break;
            case '5':
                tokimm = (tokimm << 4) | 0x5;
                state = STATE_NUM2;
                break;
            case '6':
                tokimm = (tokimm << 4) | 0x6;
                state = STATE_NUM2;
                break;
            case '7':
                tokimm = (tokimm << 4) | 0x7;
                state = STATE_NUM2;
                break;
            case '8':
                tokimm = (tokimm << 4) | 0x8;
                state = STATE_NUM2;
                break;
            case '9':
                tokimm = (tokimm << 4) | 0x9;
                state = STATE_NUM2;
                break;
            case 'a':
                tokimm = (tokimm << 4) | 0xa;
                state = STATE_NUM2;
                break;
            case 'b':
                tokimm = (tokimm << 4) | 0xb;
                state = STATE_NUM2;
                break;
            case 'c':
                tokimm = (tokimm << 4) | 0xc;
                state = STATE_NUM2;
                break;
            case 'd':
                tokimm = (tokimm << 4) | 0xd;
                state = STATE_NUM2;
                break;
            case 'e':
                tokimm = (tokimm << 4) | 0xe;
                state = STATE_NUM2;
                break;
            case 'f':
                tokimm = (tokimm << 4) | 0xf;
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
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_C:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_C)");
            switch (tokch)
            {
            case '0':
                tokimm = (tokimm << 4) | 0x0;
                state = STATE_NUM2;
                break;
            case '1':
                tokimm = (tokimm << 4) | 0x1;
                state = STATE_NUM2;
                break;
            case '2':
                tokimm = (tokimm << 4) | 0x2;
                state = STATE_NUM2;
                break;
            case '3':
                tokimm = (tokimm << 4) | 0x3;
                state = STATE_NUM2;
                break;
            case '4':
                tokimm = (tokimm << 4) | 0x4;
                state = STATE_NUM2;
                break;
            case '5':
                tokimm = (tokimm << 4) | 0x5;
                state = STATE_NUM2;
                break;
            case '6':
                tokimm = (tokimm << 4) | 0x6;
                state = STATE_NUM2;
                break;
            case '7':
                tokimm = (tokimm << 4) | 0x7;
                state = STATE_NUM2;
                break;
            case '8':
                tokimm = (tokimm << 4) | 0x8;
                state = STATE_NUM2;
                break;
            case '9':
                tokimm = (tokimm << 4) | 0x9;
                state = STATE_NUM2;
                break;
            case 'a':
                tokimm = (tokimm << 4) | 0xa;
                state = STATE_NUM2;
                break;
            case 'b':
                tokimm = (tokimm << 4) | 0xb;
                state = STATE_NUM2;
                break;
            case 'c':
                tokimm = (tokimm << 4) | 0xc;
                state = STATE_NUM2;
                break;
            case 'd':
                tokimm = (tokimm << 4) | 0xd;
                state = STATE_NUM2;
                break;
            case 'e':
                tokimm = (tokimm << 4) | 0xe;
                state = STATE_NUM2;
                break;
            case 'f':
                tokimm = (tokimm << 4) | 0xf;
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
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_D:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_D)");
            switch (tokch)
            {
            case '0':
                tokimm = (tokimm << 4) | 0x0;
                state = STATE_NUM2;
                break;
            case '1':
                tokimm = (tokimm << 4) | 0x1;
                state = STATE_NUM2;
                break;
            case '2':
                tokimm = (tokimm << 4) | 0x2;
                state = STATE_NUM2;
                break;
            case '3':
                tokimm = (tokimm << 4) | 0x3;
                state = STATE_NUM2;
                break;
            case '4':
                tokimm = (tokimm << 4) | 0x4;
                state = STATE_NUM2;
                break;
            case '5':
                tokimm = (tokimm << 4) | 0x5;
                state = STATE_NUM2;
                break;
            case '6':
                tokimm = (tokimm << 4) | 0x6;
                state = STATE_NUM2;
                break;
            case '7':
                tokimm = (tokimm << 4) | 0x7;
                state = STATE_NUM2;
                break;
            case '8':
                tokimm = (tokimm << 4) | 0x8;
                state = STATE_NUM2;
                break;
            case '9':
                tokimm = (tokimm << 4) | 0x9;
                state = STATE_NUM2;
                break;
            case 'a':
                tokimm = (tokimm << 4) | 0xa;
                state = STATE_NUM2;
                break;
            case 'b':
                tokimm = (tokimm << 4) | 0xb;
                state = STATE_NUM2;
                break;
            case 'c':
                tokimm = (tokimm << 4) | 0xc;
                state = STATE_NUM2;
                break;
            case 'd':
                tokimm = (tokimm << 4) | 0xd;
                state = STATE_NUM2;
                break;
            case 'e':
                tokimm = (tokimm << 4) | 0xe;
                state = STATE_NUM2;
                break;
            case 'f':
                tokimm = (tokimm << 4) | 0xf;
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
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_E:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_E)");
            switch (tokch)
            {
            case '0':
                tokimm = (tokimm << 4) | 0x0;
                state = STATE_NUM2;
                break;
            case '1':
                tokimm = (tokimm << 4) | 0x1;
                state = STATE_NUM2;
                break;
            case '2':
                tokimm = (tokimm << 4) | 0x2;
                state = STATE_NUM2;
                break;
            case '3':
                tokimm = (tokimm << 4) | 0x3;
                state = STATE_NUM2;
                break;
            case '4':
                tokimm = (tokimm << 4) | 0x4;
                state = STATE_NUM2;
                break;
            case '5':
                tokimm = (tokimm << 4) | 0x5;
                state = STATE_NUM2;
                break;
            case '6':
                tokimm = (tokimm << 4) | 0x6;
                state = STATE_NUM2;
                break;
            case '7':
                tokimm = (tokimm << 4) | 0x7;
                state = STATE_NUM2;
                break;
            case '8':
                tokimm = (tokimm << 4) | 0x8;
                state = STATE_NUM2;
                break;
            case '9':
                tokimm = (tokimm << 4) | 0x9;
                state = STATE_NUM2;
                break;
            case 'a':
                tokimm = (tokimm << 4) | 0xa;
                state = STATE_EA;
                break;
            case 'b':
                tokimm = (tokimm << 4) | 0xb;
                state = STATE_EB;
                break;
            case 'c':
                tokimm = (tokimm << 4) | 0xc;
                state = STATE_EC;
                break;
            case 'd':
                tokimm = (tokimm << 4) | 0xd;
                state = STATE_ED;
                break;
            case 'e':
                tokimm = (tokimm << 4) | 0xe;
                state = STATE_NUM2;
                break;
            case 'f':
                tokimm = (tokimm << 4) | 0xf;
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
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_F:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_F)");
            switch (tokch)
            {
            case '0':
                tokimm = (tokimm << 4) | 0x0;
                state = STATE_NUM2;
                break;
            case '1':
                tokimm = (tokimm << 4) | 0x1;
                state = STATE_NUM2;
                break;
            case '2':
                tokimm = (tokimm << 4) | 0x2;
                state = STATE_NUM2;
                break;
            case '3':
                tokimm = (tokimm << 4) | 0x3;
                state = STATE_NUM2;
                break;
            case '4':
                tokimm = (tokimm << 4) | 0x4;
                state = STATE_NUM2;
                break;
            case '5':
                tokimm = (tokimm << 4) | 0x5;
                state = STATE_NUM2;
                break;
            case '6':
                tokimm = (tokimm << 4) | 0x6;
                state = STATE_NUM2;
                break;
            case '7':
                tokimm = (tokimm << 4) | 0x7;
                state = STATE_NUM2;
                break;
            case '8':
                tokimm = (tokimm << 4) | 0x8;
                state = STATE_NUM2;
                break;
            case '9':
                tokimm = (tokimm << 4) | 0x9;
                state = STATE_NUM2;
                break;
            case 'a':
                tokimm = (tokimm << 4) | 0xa;
                state = STATE_FA;
                break;
            case 'b':
                tokimm = (tokimm << 4) | 0xb;
                state = STATE_NUM2;
                break;
            case 'c':
                tokimm = (tokimm << 4) | 0xc;
                state = STATE_NUM2;
                break;
            case 'd':
                tokimm = (tokimm << 4) | 0xd;
                state = STATE_NUM2;
                break;
            case 'e':
                tokimm = (tokimm << 4) | 0xe;
                state = STATE_NUM2;
                break;
            case 'f':
                tokimm = (tokimm << 4) | 0xf;
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
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_G:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_G)");
            switch (tokch)
            {
            case 's':
                take(TOKEN_GS);
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_N:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_N)");
            switch (tokch)
            {
            case 'e':
                state = STATE_NE;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_P:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_P)");
            switch (tokch)
            {
            case 't':
                state = STATE_PT;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_S:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_S)");
            switch (tokch)
            {
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
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_T:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_T)");
            switch (tokch)
            {
            case 'r':
                state = STATE_TR;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_W:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_W)");
            switch (tokch)
            {
            case 'o':
                state = STATE_WO;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_BY:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_BY)");
            switch (tokch)
            {
            case 't':
                state = STATE_BYT;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_CR:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_CR)");
            switch (tokch)
            {
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
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_DR:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_DR)");
            switch (tokch)
            {
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
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_DW:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_DW)");
            switch (tokch)
            {
            case 'o':
                state = STATE_DWO;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_EA:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_EA)");
            switch (tokch)
            {
            case 'x':
                take(TOKEN_EAX);
                break;
            default:
                tokptr--;
                tokimm8 = (lib_u8)tokimm;
                take(TOKEN_IMM8);
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_EB:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_EB)");
            switch (tokch)
            {
            case 'p':
                take(TOKEN_EBP);
                break;
            case 'x':
                take(TOKEN_EBX);
                break;
            default:
                tokptr--;
                tokimm8 = (lib_u8)tokimm;
                take(TOKEN_IMM8);
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_EC:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_EC)");
            switch (tokch)
            {
            case 'x':
                take(TOKEN_ECX);
                break;
            default:
                tokptr--;
                tokimm8 = (lib_u8)tokimm;
                take(TOKEN_IMM8);
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_ED:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_ED)");
            switch (tokch)
            {
            case 'i':
                take(TOKEN_EDI);
                break;
            case 'x':
                take(TOKEN_EDX);
                break;
            default:
                tokptr--;
                tokimm8 = (lib_u8)tokimm;
                take(TOKEN_IMM8);
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_ES:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_ES)");
            switch (tokch)
            {
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
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_FA:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_FA)");
            switch (tokch)
            {
            case '0':
                tokimm = (tokimm << 4) | 0x0;
                state = STATE_NUM3;
                break;
            case '1':
                tokimm = (tokimm << 4) | 0x1;
                state = STATE_NUM3;
                break;
            case '2':
                tokimm = (tokimm << 4) | 0x2;
                state = STATE_NUM3;
                break;
            case '3':
                tokimm = (tokimm << 4) | 0x3;
                state = STATE_NUM3;
                break;
            case '4':
                tokimm = (tokimm << 4) | 0x4;
                state = STATE_NUM3;
                break;
            case '5':
                tokimm = (tokimm << 4) | 0x5;
                state = STATE_NUM3;
                break;
            case '6':
                tokimm = (tokimm << 4) | 0x6;
                state = STATE_NUM3;
                break;
            case '7':
                tokimm = (tokimm << 4) | 0x7;
                state = STATE_NUM3;
                break;
            case '8':
                tokimm = (tokimm << 4) | 0x8;
                state = STATE_NUM3;
                break;
            case '9':
                tokimm = (tokimm << 4) | 0x9;
                state = STATE_NUM3;
                break;
            case 'a':
                tokimm = (tokimm << 4) | 0xa;
                state = STATE_NUM3;
                break;
            case 'b':
                tokimm = (tokimm << 4) | 0xb;
                state = STATE_NUM3;
                break;
            case 'c':
                tokimm = (tokimm << 4) | 0xc;
                state = STATE_NUM3;
                break;
            case 'd':
                tokimm = (tokimm << 4) | 0xd;
                state = STATE_NUM3;
                break;
            case 'e':
                tokimm = (tokimm << 4) | 0xe;
                state = STATE_NUM3;
                break;
            case 'f':
                tokimm = (tokimm << 4) | 0xf;
                state = STATE_NUM3;
                break;
            case 'r':
                take(TOKEN_FAR);
                break;
            default:
                tokptr--;
                tokimm8 = (lib_u8)tokimm;
                take(TOKEN_IMM8);
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_NE:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_NE)");
            switch (tokch)
            {
            case 'a':
                state = STATE_NEA;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_PT:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_PT)");
            switch (tokch)
            {
            case 'r':
                take(TOKEN_PTR);
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_SH:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_SH)");
            switch (tokch)
            {
            case 'o':
                state = STATE_SHO;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_TR:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_TR)");
            switch (tokch)
            {
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
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_WO:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_WO)");
            switch (tokch)
            {
            case 'r':
                state = STATE_WOR;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_BYT:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_BYT)");
            switch (tokch)
            {
            case 'e':
                take(TOKEN_BYTE);
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_DWO:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_DWO)");
            switch (tokch)
            {
            case 'r':
                state = STATE_DWOR;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_NEA:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_NEA)");
            switch (tokch)
            {
            case 'r':
                take(TOKEN_NEAR);
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_SHO:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_SHO)");
            switch (tokch)
            {
            case 'r':
                state = STATE_SHOR;
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_WOR:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_WOR)");
            switch (tokch)
            {
            case 'd':
                take(TOKEN_WORD);
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_DWOR:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_DWOR)");
            switch (tokch)
            {
            case 'd':
                take(TOKEN_DWORD);
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case STATE_SHOR:
            XASM32_TRACE_BLOCK_BEGIN("state(STATE_SHOR)");
            switch (tokch)
            {
            case 't':
                take(TOKEN_SHORT);
                break;
            default:
                tokptr--;
                _sert_;
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        default:
            XASM32_TRACE_BLOCK_BEGIN("state(default)");
            tokptr--;
            _sert_;
            XASM32_TRACE_BLOCK_END;
            break;
        }
        tokptr++;
    } while (!flagend);
    XASM32_TRACE_CALL_END;
    return token;
}
static void matchtoken(aasm32_context *aasmContext, t_aasm_token token)
{
    XASM32_TRACE_CALL_BEGIN("matchtoken");
    if (gettoken(aasmContext, LIB_NULL) != token)
        _ser_;
    XASM32_TRACE_CALL_END;
}

/* assembly compiler: parser / grammar */
static t_aasm_oprinfo parsearg_mem(aasm32_context *aasmContext, t_aasm_token token)
{
    t_aasm_oprinfo info;
    lib_u8 oldtoken;
    lib_u8 bx, bp, si, di, neg, al;
    lib_u8 eax, ecx, edx, ebx, esp, ebp, esi, edi;
    lib_u8 ieax, iecx, iedx, iebx, iebp, iesi, iedi;
    XASM32_TRACE_CALL_BEGIN("parsearg_mem");
    memset((void *)(&info), 0x00, sizeof(t_aasm_oprinfo));
    bx = bp = si = di = neg = al = 0;
    eax = ecx = edx = ebx = esp = ebp = esi = edi = 0;
    ieax = iecx = iedx = iebx = iebp = iesi = iedi = 0;
    info.type = XASM32_OPERAND_M;
    info.mod = MOD_M;
    info.sib.base = R32_EBP;  /* EBP for NULL Base */
    info.sib.index = R32_ESP; /* ESP for NULL Index */
    info.sib.scale = 0;
    oldtoken = token;
    _chrf(token = gettoken(aasmContext, LIB_NULL));
    if (token == TOKEN_COLON)
    {
        XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_COLON");
        switch (oldtoken)
        {
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
        XASM32_TRACE_BLOCK_END;
    }
    else if (token == TOKEN_NULL || token == TOKEN_END)
    {
        XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_NULL/TOKEN_END)");
        switch (oldtoken)
        {
        case TOKEN_ES:
            info.type = XASM32_OPERAND_SREG;
            info.mod = MOD_R;
            info.sreg = SREG_ES;
            break;
        case TOKEN_CS:
            info.type = XASM32_OPERAND_SREG;
            info.mod = MOD_R;
            info.sreg = SREG_CS;
            break;
        case TOKEN_SS:
            info.type = XASM32_OPERAND_SREG;
            info.mod = MOD_R;
            info.sreg = SREG_SS;
            break;
        case TOKEN_DS:
            info.type = XASM32_OPERAND_SREG;
            info.mod = MOD_R;
            info.sreg = SREG_DS;
            break;
        case TOKEN_FS:
            info.type = XASM32_OPERAND_SREG;
            info.mod = MOD_R;
            info.sreg = SREG_FS;
            break;
        case TOKEN_GS:
            info.type = XASM32_OPERAND_SREG;
            info.mod = MOD_R;
            info.sreg = SREG_GS;
            break;
        default:
            _serf_;
            break;
        }
        XASM32_TRACE_BLOCK_END;
        XASM32_TRACE_CALL_END;
        return info;
    }
    else
        _serf_;
    _chrf(matchtoken(aasmContext, TOKEN_LSPAREN));
    _chrf(token = gettoken(aasmContext, LIB_NULL));
    while (token != TOKEN_RSPAREN)
    {
        switch (token)
        {
        case TOKEN_PLUS:
            break;
        case TOKEN_MINUS:
            XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_MINUS)");
            _chrf(token = gettoken(aasmContext, LIB_NULL));
            neg = 1;
            switch (token)
            {
            case TOKEN_IMM8:
                XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_IMM8)");
                if (info.mod != MOD_M)
                    _serf_;
                if (tokimm8 > 0x80)
                    _serf_;
                else
                {
                    tokimm8 = (~tokimm8) + 1;
                    info.disp8 = tokimm8;
                    info.mod = MOD_M_DISP8;
                }
                XASM32_TRACE_BLOCK_END;
                break;
            case TOKEN_IMM16:
                XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_IMM16)");
                if (info.mod != MOD_M)
                    _serf_;
                if (tokimm16 > 0xff80)
                    _serf_;
                else
                {
                    tokimm16 = (~tokimm16) + 1;
                    info.disp16 = tokimm16;
                    info.mod = MOD_M_DISP16;
                }
                XASM32_TRACE_BLOCK_END;
                break;
            case TOKEN_IMM32:
                XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_IMM32)");
                if (info.mod != MOD_M)
                    _serf_;
                if (tokimm32 > 0xffffff80)
                    _serf_;
                else
                {
                    tokimm32 = (~tokimm32) + 1;
                    info.disp32 = tokimm32;
                    info.mod = MOD_M_DISP32;
                }
                XASM32_TRACE_BLOCK_END;
                break;
            default:
                _serf_;
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case TOKEN_BX:
            XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_BX)");
            if (bx)
                _serf_;
            else
                bx = 1;
            XASM32_TRACE_BLOCK_END;
            break;
        case TOKEN_SI:
            XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_SI)");
            if (si)
                _serf_;
            else
                si = 1;
            XASM32_TRACE_BLOCK_END;
            break;
        case TOKEN_BP:
            XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_BP)");
            if (bp)
                _serf_;
            else
                bp = 1;
            XASM32_TRACE_BLOCK_END;
            break;
        case TOKEN_DI:
            XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_DI)");
            if (di)
                _serf_;
            else
                di = 1;
            XASM32_TRACE_BLOCK_END;
            break;
        case TOKEN_AL:
            XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_AL)");
            if (al)
                _serf_;
            else
                al = 1;
            XASM32_TRACE_BLOCK_END;
            break;
        case TOKEN_IMM8:
            XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_IMM8)");
            if (info.mod != MOD_M)
                _serf_;
            info.mod = MOD_M_DISP8;
            info.disp8 = tokimm8;
            XASM32_TRACE_BLOCK_END;
            break;
        case TOKEN_IMM16:
            XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_IMM16)");
            if (info.mod != MOD_M)
                _serf_;
            info.mod = MOD_M_DISP16;
            info.disp16 = tokimm16;
            XASM32_TRACE_BLOCK_END;
            break;
        case TOKEN_IMM32:
            XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_IMM32)");
            if (info.mod != MOD_M)
                _serf_;
            info.mod = MOD_M_DISP32;
            info.disp32 = tokimm32;
            XASM32_TRACE_BLOCK_END;
            break;
        case TOKEN_EAX:
            XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_EAX)");
            _chrf(token = gettoken(aasmContext, LIB_NULL));
            if (token == TOKEN_TIMES)
            {
                XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_TIMES)");
                if (ieax)
                    _serf_;
                else
                {
                    XASM32_TRACE_BLOCK_BEGIN("!ieax");
                    ieax = 1;
                    _chrf(token = gettoken(aasmContext, LIB_NULL));
                    if (token != TOKEN_IMM8)
                        _serf_;
                    else
                    {
                        info.sib.scale = tokimm8;
                        info.sib.index = R32_EAX;
                        _chrf(token = gettoken(aasmContext, LIB_NULL));
                    }
                    XASM32_TRACE_BLOCK_END;
                }
                XASM32_TRACE_BLOCK_END;
            }
            else
            {
                XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_!TIMES)");
                if (eax)
                    _serf_;
                else
                {
                    eax = 1;
                    info.sib.base = R32_EAX;
                }
                XASM32_TRACE_BLOCK_END;
            }
            XASM32_TRACE_BLOCK_END;
            continue;
            break;
        case TOKEN_ECX:
            XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_ECX)");
            _chrf(token = gettoken(aasmContext, LIB_NULL));
            if (token == TOKEN_TIMES)
            {
                XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_TIMES)");
                if (iecx)
                    _serf_;
                else
                {
                    XASM32_TRACE_BLOCK_BEGIN("!iecx");
                    iecx = 1;
                    _chrf(token = gettoken(aasmContext, LIB_NULL));
                    if (token != TOKEN_IMM8)
                        _serf_;
                    else
                    {
                        info.sib.scale = tokimm8;
                        info.sib.index = R32_ECX;
                        _chrf(token = gettoken(aasmContext, LIB_NULL));
                    }
                    XASM32_TRACE_BLOCK_END;
                }
                XASM32_TRACE_BLOCK_END;
            }
            else
            {
                XASM32_TRACE_BLOCK_BEGIN("token(!TOKEN_TIMES)");
                if (ecx)
                    _serf_;
                else
                {
                    ecx = 1;
                    info.sib.base = R32_ECX;
                }
                XASM32_TRACE_BLOCK_END;
            }
            XASM32_TRACE_BLOCK_END;
            continue;
            break;
        case TOKEN_EDX:
            XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_EDX)");
            _chrf(token = gettoken(aasmContext, LIB_NULL));
            if (token == TOKEN_TIMES)
            {
                XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_TIMES)");
                if (iedx)
                    _serf_;
                else
                {
                    XASM32_TRACE_BLOCK_BEGIN("!iedx");
                    iedx = 1;
                    _chrf(token = gettoken(aasmContext, LIB_NULL));
                    if (token != TOKEN_IMM8)
                        _serf_;
                    else
                    {
                        info.sib.scale = tokimm8;
                        info.sib.index = R32_EDX;
                        _chrf(token = gettoken(aasmContext, LIB_NULL));
                    }
                    XASM32_TRACE_BLOCK_END;
                }
                XASM32_TRACE_BLOCK_END;
            }
            else
            {
                XASM32_TRACE_BLOCK_BEGIN("token(!TOKEN_TIMES)");
                if (edx)
                    _serf_;
                else
                {
                    edx = 1;
                    info.sib.base = R32_EDX;
                }
                XASM32_TRACE_BLOCK_END;
            }
            XASM32_TRACE_BLOCK_END;
            continue;
            break;
        case TOKEN_EBX:
            XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_EBX)");
            _chrf(token = gettoken(aasmContext, LIB_NULL));
            if (token == TOKEN_TIMES)
            {
                XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_TIMES)");
                if (iebx)
                    _serf_;
                else
                {
                    XASM32_TRACE_BLOCK_BEGIN("!iebx");
                    iebx = 1;
                    _chrf(token = gettoken(aasmContext, LIB_NULL));
                    if (token != TOKEN_IMM8)
                        _serf_;
                    else
                    {
                        info.sib.scale = tokimm8;
                        info.sib.index = R32_EBX;
                        _chrf(token = gettoken(aasmContext, LIB_NULL));
                    }
                    XASM32_TRACE_BLOCK_END;
                }
                XASM32_TRACE_BLOCK_END;
            }
            else
            {
                XASM32_TRACE_BLOCK_BEGIN("token(!TOKEN_TIMES)");
                if (ebx)
                    _serf_;
                else
                {
                    ebx = 1;
                    info.sib.base = R32_EBX;
                }
                XASM32_TRACE_BLOCK_END;
            }
            XASM32_TRACE_BLOCK_END;
            continue;
            break;
        case TOKEN_ESP:
            XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_ESP)");
            _chrf(token = gettoken(aasmContext, LIB_NULL));
            if (token == TOKEN_TIMES)
                _serf_;
            else
            {
                XASM32_TRACE_BLOCK_BEGIN("token(!TOKEN_TIMES)");
                if (esp)
                    _serf_;
                else
                {
                    esp = 1;
                    info.sib.base = R32_ESP;
                }
                XASM32_TRACE_BLOCK_END;
            }
            XASM32_TRACE_BLOCK_END;
            continue;
            break;
        case TOKEN_EBP:
            XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_EBP)");
            _chrf(token = gettoken(aasmContext, LIB_NULL));
            if (token == TOKEN_TIMES)
            {
                XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_TIMES)");
                if (iebp)
                    _serf_;
                else
                {
                    XASM32_TRACE_BLOCK_BEGIN("!iebp");
                    iebx = 1;
                    _chrf(token = gettoken(aasmContext, LIB_NULL));
                    if (token != TOKEN_IMM8)
                        _serf_;
                    else
                    {
                        info.sib.scale = tokimm8;
                        info.sib.index = R32_EBP;
                        _chrf(token = gettoken(aasmContext, LIB_NULL));
                    }
                    XASM32_TRACE_BLOCK_END;
                }
                XASM32_TRACE_BLOCK_END;
            }
            else
            {
                XASM32_TRACE_BLOCK_BEGIN("token(!TOKEN_TIMES)");
                if (ebp)
                    _serf_;
                else
                {
                    ebp = 1;
                    info.sib.base = R32_EBP;
                }
                XASM32_TRACE_BLOCK_END;
            }
            XASM32_TRACE_BLOCK_END;
            continue;
            break;
        case TOKEN_ESI:
            XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_ESI)");
            _chrf(token = gettoken(aasmContext, LIB_NULL));
            if (token == TOKEN_TIMES)
            {
                XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_TIMES)");
                if (iesi)
                    _serf_;
                else
                {
                    XASM32_TRACE_BLOCK_BEGIN("!iesi");
                    iesi = 1;
                    _chrf(token = gettoken(aasmContext, LIB_NULL));
                    if (token != TOKEN_IMM8)
                        _serf_;
                    else
                    {
                        info.sib.scale = tokimm8;
                        info.sib.index = R32_ESI;
                        _chrf(token = gettoken(aasmContext, LIB_NULL));
                    }
                    XASM32_TRACE_BLOCK_END;
                }
                XASM32_TRACE_BLOCK_END;
            }
            else
            {
                XASM32_TRACE_BLOCK_BEGIN("token(!TOKEN_TIMES)");
                if (esi)
                    _serf_;
                else
                {
                    esi = 1;
                    info.sib.base = R32_ESI;
                }
                XASM32_TRACE_BLOCK_END;
            }
            XASM32_TRACE_BLOCK_END;
            continue;
            break;
        case TOKEN_EDI:
            XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_EDI)");
            _chrf(token = gettoken(aasmContext, LIB_NULL));
            if (token == TOKEN_TIMES)
            {
                XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_TIMES)");
                if (iedi)
                    _serf_;
                else
                {
                    XASM32_TRACE_BLOCK_BEGIN("!iedi");
                    iedi = 1;
                    _chrf(token = gettoken(aasmContext, LIB_NULL));
                    if (token != TOKEN_IMM8)
                        _serf_;
                    else
                    {
                        info.sib.scale = tokimm8;
                        info.sib.index = R32_EDI;
                        _chrf(token = gettoken(aasmContext, LIB_NULL));
                    }
                    XASM32_TRACE_BLOCK_END;
                }
                XASM32_TRACE_BLOCK_END;
            }
            else
            {
                XASM32_TRACE_BLOCK_BEGIN("token(!TOKEN_TIMES)");
                if (edi)
                    _serf_;
                else
                {
                    edi = 1;
                    info.sib.base = R32_EDI;
                }
                XASM32_TRACE_BLOCK_END;
            }
            XASM32_TRACE_BLOCK_END;
            continue;
            break;
        default:
            _serf_;
            break;
        }
        _chrf(token = gettoken(aasmContext, LIB_NULL));
    }
    _chrf(token = gettoken(aasmContext, LIB_NULL));
    if (token != TOKEN_END)
        _serf_;

    if (al)
    {
        XASM32_TRACE_BLOCK_BEGIN("al");
        if (bp || si || di || eax || ecx || edx || esp || ebp || esi || edi ||
            ieax || iecx || iedx || iebx || iebp || iesi || iedi || info.mod != MOD_M)
            _serf_;
        if (bx && !ebx)
        {
            info.mem = MEM_BX_AL;
        }
        else if (!bx && ebx)
        {
            info.mem = MEM_EBX_AL;
        }
        else
            _serf_;
        XASM32_TRACE_BLOCK_END;
    }
    else
    {
        if (bx || bp || si || di || info.mod == MOD_M_DISP16)
        {
            XASM32_TRACE_BLOCK_BEGIN("16-bit Addressing");
            if (!bx && !si && !bp && !di)
            {
                XASM32_TRACE_BLOCK_BEGIN("[DISP16]");
                info.mem = MEM_BP;
                if (info.mod == MOD_M_DISP16)
                    info.mod = MOD_M;
                else
                    _serf_;
                XASM32_TRACE_BLOCK_END;
            }
            else
            {
                XASM32_TRACE_BLOCK_BEGIN("bx/bp/si/di");
                if (bx && si && !bp && !di)
                    info.mem = MEM_BX_SI;
                else if (bx && !si && !bp && di)
                    info.mem = MEM_BX_DI;
                else if (!bx && si && bp && !di)
                    info.mem = MEM_BP_SI;
                else if (!bx && !si && bp && di)
                    info.mem = MEM_BP_DI;
                else if (bx && !si && !bp && !di)
                    info.mem = MEM_BX;
                else if (!bx && si && !bp && !di)
                    info.mem = MEM_SI;
                else if (!bx && !si && bp && !di)
                {
                    info.mem = MEM_BP;
                    if (info.mod == MOD_M)
                    {
                        info.mod = MOD_M_DISP8;
                        info.disp8 = 0x00;
                    }
                }
                else if (!bx && !si && !bp && di)
                    info.mem = MEM_DI;
                else
                    _serf_;
                XASM32_TRACE_BLOCK_END;
            }
            XASM32_TRACE_BLOCK_END;
        }
        else if (eax || ecx || edx || ebx || esp || ebp || esi || edi ||
                 ieax || iecx || iedx || iebx || iebp || iesi || iedi || info.mod == MOD_M_DISP32)
        {
            XASM32_TRACE_BLOCK_BEGIN("32-bit Addressing");
            if (!eax && !ecx && !edx && !ebx && !esp && !ebp && !esi && !edi)
            {
                XASM32_TRACE_BLOCK_BEGIN("!base");
                if (info.mod == MOD_M_DISP32)
                {
                    info.mod = MOD_M;
                    if (ieax || iecx || iedx || iebx || iebp || iesi || iedi)
                        info.mem = MEM_SIB;
                    else
                        info.mem = MEM_EBP;
                }
                else
                    _serf_;
                XASM32_TRACE_BLOCK_END;
            }
            else
            {
                XASM32_TRACE_BLOCK_BEGIN("base");
                if (esp || ieax || iecx || iedx || iebx || iebp || iesi || iedi)
                {
                    info.mem = MEM_SIB;
                }
                else if (eax)
                    info.mem = MEM_EAX;
                else if (ecx)
                    info.mem = MEM_ECX;
                else if (edx)
                    info.mem = MEM_EDX;
                else if (ebx)
                    info.mem = MEM_EBX;
                else if (ebp)
                    info.mem = MEM_EBP;
                else if (esi)
                    info.mem = MEM_ESI;
                else if (edi)
                    info.mem = MEM_EDI;
                else
                    _serf_;
                XASM32_TRACE_BLOCK_END;
            }
            XASM32_TRACE_BLOCK_END;
        }
        else
            _serf_;
    }
    switch (info.mem)
    {
    case MEM_BX_SI:
    case MEM_BX_DI:
    case MEM_BX:
    case MEM_SI:
    case MEM_DI:
        if (info.flagds)
            info.flagds = 0;
        break;
    case MEM_BP_SI:
    case MEM_BP_DI:
        if (info.flagss)
            info.flagss = 0;
        break;
    case MEM_BP:
        if (!bp && info.flagds)
            info.flagds = 0;
        else if (bp && info.flagss)
            info.flagss = 0;
        break;
    case MEM_EAX:
    case MEM_ECX:
    case MEM_EDX:
    case MEM_EBX:
    case MEM_ESI:
    case MEM_EDI:
        if (info.flagds)
            info.flagds = 0;
        break;
    case MEM_EBP:
        if (info.flagss)
            info.flagss = 0;
        break;
    case MEM_SIB:
        if (info.sib.base == R32_ESP ||
            (info.sib.base == R32_EBP && info.mod != MOD_M))
        {
            if (info.flagss)
                info.flagss = 0;
        }
        else if (info.flagds)
            info.flagds = 0;
        break;
    case MEM_BX_AL:
    case MEM_EBX_AL:
        break;
    default:
        _serf_;
        break;
    }
    info.type = XASM32_OPERAND_M;
    XASM32_TRACE_CALL_END;
    return info;
}
static t_aasm_oprinfo parsearg_imm(aasm32_context *aasmContext, t_aasm_token token)
{
    t_aasm_oprinfo info;
    XASM32_TRACE_CALL_BEGIN("parsearg_imm");

    memset((void *)(&info), 0x00, sizeof(t_aasm_oprinfo));

    if (token == TOKEN_PLUS)
    {
        XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_PLUS)");
        info.imms = 1;
        info.immn = 0;
        _chrf(token = gettoken(aasmContext, LIB_NULL));
        XASM32_TRACE_BLOCK_END;
    }
    else if (token == TOKEN_MINUS)
    {
        XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_MINUS)");
        info.imms = 1;
        info.immn = 1;
        _chrf(token = gettoken(aasmContext, LIB_NULL));
        XASM32_TRACE_BLOCK_END;
    }

    if (token == TOKEN_IMM8)
    {
        info.type = XASM32_OPERAND_I8;
        if (!info.immn)
            info.imm8 = tokimm8;
        else
            info.imm8 = (~tokimm8) + 1;
    }
    else if (token == TOKEN_IMM16)
    {
        info.type = XASM32_OPERAND_I16;
        if (!info.immn)
            info.imm16 = tokimm16;
        else
            info.imm16 = (~tokimm16) + 1;
    }
    else if (token == TOKEN_IMM32)
    {
        info.type = XASM32_OPERAND_I32;
        if (!info.immn)
            info.imm32 = tokimm32;
        else
            info.imm32 = (~tokimm32) + 1;
    }
    else
        _serf_;

    _chrf(token = gettoken(aasmContext, LIB_NULL));
    if (token == TOKEN_COLON)
    {
        XASM32_TRACE_BLOCK_BEGIN("token(!TOKEN_END)");
        if (info.imms)
            _serf_;
        if (info.type == XASM32_OPERAND_I16)
            info.rcs = info.imm16;
        else
        {
            info.type = XASM32_OPERAND_NONE;
            _serf_;
        }
        info.type = XASM32_OPERAND_NONE;
        _chrf(token = gettoken(aasmContext, LIB_NULL));
        if (token == TOKEN_IMM16)
        {
            info.reip = tokimm16;
            info.type = XASM32_OPERAND_I16_16;
        }
        else if (token == TOKEN_IMM32)
        {
            info.reip = tokimm32;
            info.type = XASM32_OPERAND_I16_32;
        }
        else
            _serf_;
        XASM32_TRACE_BLOCK_END;
    }
    else if (token != TOKEN_END)
        _serf_;
    XASM32_TRACE_CALL_END;
    return info;
}
static t_aasm_oprinfo parsearg(aasm32_context *aasmContext, char *arg)
{
    t_aasm_token token;
    t_aasm_oprinfo info;
    XASM32_TRACE_CALL_BEGIN("parsearg");
    memset((void *)(&info), 0x00, sizeof(t_aasm_oprinfo));
    if (!arg || !arg[0])
    {
        info.type = XASM32_OPERAND_NONE;
        XASM32_TRACE_CALL_END;
        return info;
    }
    _chrf(token = gettoken(aasmContext, arg));
    switch (token)
    {
    case TOKEN_NULL:
    case TOKEN_END:
        info.type = XASM32_OPERAND_NONE;
        break;
    case TOKEN_BYTE:
        XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_BYTE)");
        _chrf(token = gettoken(aasmContext, LIB_NULL));
        if (token == TOKEN_PTR)
            _chrf(token = gettoken(aasmContext, LIB_NULL));
        _chrf(info = parsearg_mem(aasmContext, token));
        info.type = XASM32_OPERAND_M8;
        XASM32_TRACE_BLOCK_END;
        break;
    case TOKEN_WORD:
        XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_WORD)");
        _chrf(token = gettoken(aasmContext, LIB_NULL));
        if (token == TOKEN_PTR)
            _chrf(token = gettoken(aasmContext, LIB_NULL));
        _chrf(info = parsearg_mem(aasmContext, token));
        info.type = XASM32_OPERAND_M16;
        info.ptr = PTR_NEAR;
        XASM32_TRACE_BLOCK_END;
        break;
    case TOKEN_DWORD:
        XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_DWORD)");
        _chrf(token = gettoken(aasmContext, LIB_NULL));
        if (token == TOKEN_PTR)
            _chrf(token = gettoken(aasmContext, LIB_NULL));
        _chrf(info = parsearg_mem(aasmContext, token));
        info.type = XASM32_OPERAND_M32;
        info.ptr = PTR_FAR;
        XASM32_TRACE_BLOCK_END;
        break;
    case TOKEN_AL:
        info.type = XASM32_OPERAND_R8;
        info.mod = MOD_R;
        info.reg8 = R8_AL;
        break;
    case TOKEN_CL:
        info.type = XASM32_OPERAND_R8;
        info.mod = MOD_R;
        info.reg8 = R8_CL;
        break;
    case TOKEN_DL:
        info.type = XASM32_OPERAND_R8;
        info.mod = MOD_R;
        info.reg8 = R8_DL;
        break;
    case TOKEN_BL:
        info.type = XASM32_OPERAND_R8;
        info.mod = MOD_R;
        info.reg8 = R8_BL;
        break;
    case TOKEN_AH:
        info.type = XASM32_OPERAND_R8;
        info.mod = MOD_R;
        info.reg8 = R8_AH;
        break;
    case TOKEN_CH:
        info.type = XASM32_OPERAND_R8;
        info.mod = MOD_R;
        info.reg8 = R8_CH;
        break;
    case TOKEN_DH:
        info.type = XASM32_OPERAND_R8;
        info.mod = MOD_R;
        info.reg8 = R8_DH;
        break;
    case TOKEN_BH:
        info.type = XASM32_OPERAND_R8;
        info.mod = MOD_R;
        info.reg8 = R8_BH;
        break;
    case TOKEN_AX:
        info.type = XASM32_OPERAND_R16;
        info.mod = MOD_R;
        info.reg16 = R16_AX;
        info.ptr = PTR_NEAR;
        break;
    case TOKEN_CX:
        info.type = XASM32_OPERAND_R16;
        info.mod = MOD_R;
        info.reg16 = R16_CX;
        info.ptr = PTR_NEAR;
        break;
    case TOKEN_DX:
        info.type = XASM32_OPERAND_R16;
        info.mod = MOD_R;
        info.reg16 = R16_DX;
        info.ptr = PTR_NEAR;
        break;
    case TOKEN_BX:
        info.type = XASM32_OPERAND_R16;
        info.mod = MOD_R;
        info.reg16 = R16_BX;
        info.ptr = PTR_NEAR;
        break;
    case TOKEN_SP:
        info.type = XASM32_OPERAND_R16;
        info.mod = MOD_R;
        info.reg16 = R16_SP;
        info.ptr = PTR_NEAR;
        break;
    case TOKEN_BP:
        info.type = XASM32_OPERAND_R16;
        info.mod = MOD_R;
        info.reg16 = R16_BP;
        info.ptr = PTR_NEAR;
        break;
    case TOKEN_SI:
        info.type = XASM32_OPERAND_R16;
        info.mod = MOD_R;
        info.reg16 = R16_SI;
        info.ptr = PTR_NEAR;
        break;
    case TOKEN_DI:
        info.type = XASM32_OPERAND_R16;
        info.mod = MOD_R;
        info.reg16 = R16_DI;
        info.ptr = PTR_NEAR;
        break;
    case TOKEN_EAX:
        info.type = XASM32_OPERAND_R32;
        info.mod = MOD_R;
        info.reg32 = R32_EAX;
        info.ptr = PTR_NEAR;
        break;
    case TOKEN_ECX:
        info.type = XASM32_OPERAND_R32;
        info.mod = MOD_R;
        info.reg32 = R32_ECX;
        info.ptr = PTR_NEAR;
        break;
    case TOKEN_EDX:
        info.type = XASM32_OPERAND_R32;
        info.mod = MOD_R;
        info.reg32 = R32_EDX;
        info.ptr = PTR_NEAR;
        break;
    case TOKEN_EBX:
        info.type = XASM32_OPERAND_R32;
        info.mod = MOD_R;
        info.reg32 = R32_EBX;
        info.ptr = PTR_NEAR;
        break;
    case TOKEN_ESP:
        info.type = XASM32_OPERAND_R32;
        info.mod = MOD_R;
        info.reg32 = R32_ESP;
        info.ptr = PTR_NEAR;
        break;
    case TOKEN_EBP:
        info.type = XASM32_OPERAND_R32;
        info.mod = MOD_R;
        info.reg32 = R32_EBP;
        info.ptr = PTR_NEAR;
        break;
    case TOKEN_ESI:
        info.type = XASM32_OPERAND_R32;
        info.mod = MOD_R;
        info.reg32 = R32_ESI;
        info.ptr = PTR_NEAR;
        break;
    case TOKEN_EDI:
        info.type = XASM32_OPERAND_R32;
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
        XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_SREGs)");
        _chrf(info = parsearg_mem(aasmContext, token));
        XASM32_TRACE_BLOCK_END;
        break;
    case TOKEN_PLUS:
    case TOKEN_MINUS:
    case TOKEN_IMM8:
    case TOKEN_IMM16:
    case TOKEN_IMM32:
        XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_IMMs)");
        _chrf(info = parsearg_imm(aasmContext, token));
        if (info.type == XASM32_OPERAND_I16_16)
            info.ptr = PTR_FAR;
        else if (info.type == XASM32_OPERAND_I16_32)
            info.ptr = PTR_FAR;
        else
            info.ptr = PTR_NONE;
        XASM32_TRACE_BLOCK_END;
        break;
    case TOKEN_SHORT:
        XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_SHORT)");
        _chrf(token = gettoken(aasmContext, LIB_NULL));
        if (token == TOKEN_PTR)
            _chrf(token = gettoken(aasmContext, LIB_NULL));
        if (token == TOKEN_PLUS || token == TOKEN_MINUS)
        {
            XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_SIGNs)");
            _chrf(info = parsearg_imm(aasmContext, token));
            if (info.type != XASM32_OPERAND_I8)
                _serf_;
            XASM32_TRACE_BLOCK_END;
        }
        else
            _serf_;
        info.ptr = PTR_SHORT;
        XASM32_TRACE_BLOCK_END;
        break;
    case TOKEN_NEAR:
        XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_NEAR)");
        _chrf(token = gettoken(aasmContext, LIB_NULL));
        if (token == TOKEN_PTR)
            _chrf(token = gettoken(aasmContext, LIB_NULL));
        switch (token)
        {
        case TOKEN_WORD:
            XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_WORD)");
            _chrf(token = gettoken(aasmContext, LIB_NULL));
            if (token == TOKEN_PTR)
                _chrf(token = gettoken(aasmContext, LIB_NULL));
            _chrf(info = parsearg_mem(aasmContext, token));
            info.type = XASM32_OPERAND_M16;
            XASM32_TRACE_BLOCK_END;
            break;
        case TOKEN_DWORD:
            XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_DWORD)");
            _chrf(token = gettoken(aasmContext, LIB_NULL));
            if (token == TOKEN_PTR)
                _chrf(token = gettoken(aasmContext, LIB_NULL));
            _chrf(info = parsearg_mem(aasmContext, token));
            info.type = XASM32_OPERAND_M32;
            XASM32_TRACE_BLOCK_END;
            break;
        case TOKEN_PLUS:
        case TOKEN_MINUS:
            XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_PLUS/TOKEN_MINUS)");
            _chrf(info = parsearg_imm(aasmContext, token));
            if (info.type != XASM32_OPERAND_I16 && info.type != XASM32_OPERAND_I32)
                _serf_;
            XASM32_TRACE_BLOCK_END;
            break;
        case TOKEN_ES:
        case TOKEN_CS:
        case TOKEN_SS:
        case TOKEN_DS:
        case TOKEN_FS:
        case TOKEN_GS:
            XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_SREGs)");
            _chrf(info = parsearg_mem(aasmContext, token));
            if (info.type != XASM32_OPERAND_M)
                _serf_;
            XASM32_TRACE_BLOCK_END;
            break;
        default:
            _serf_;
            break;
        }
        info.ptr = PTR_NEAR;
        XASM32_TRACE_BLOCK_END;
        break;
    case TOKEN_FAR:
        XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_FAR)");
        _chrf(token = gettoken(aasmContext, LIB_NULL));
        if (token == TOKEN_PTR)
            _chrf(token = gettoken(aasmContext, LIB_NULL));
        switch (token)
        {
        case TOKEN_WORD:
            XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_WORD)");
            _chrf(token = gettoken(aasmContext, LIB_NULL));
            if (token == TOKEN_PTR)
                _chrf(token = gettoken(aasmContext, LIB_NULL));
            _chrf(info = parsearg_mem(aasmContext, token));
            info.type = XASM32_OPERAND_M16;
            info.ptr = PTR_FAR;
            XASM32_TRACE_BLOCK_END;
            break;
        case TOKEN_DWORD:
            XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_DWORD)");
            _chrf(token = gettoken(aasmContext, LIB_NULL));
            if (token == TOKEN_PTR)
                _chrf(token = gettoken(aasmContext, LIB_NULL));
            _chrf(info = parsearg_mem(aasmContext, token));
            info.type = XASM32_OPERAND_M32;
            info.ptr = PTR_FAR;
            XASM32_TRACE_BLOCK_END;
            break;
        case TOKEN_IMM16:
            XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_IMM16)");
            _chrf(info = parsearg_imm(aasmContext, token));
            if (info.type != XASM32_OPERAND_I16_16 && info.type != XASM32_OPERAND_I16_32)
                _serf_;
            XASM32_TRACE_BLOCK_END;
            break;
        case TOKEN_ES:
        case TOKEN_CS:
        case TOKEN_SS:
        case TOKEN_DS:
        case TOKEN_FS:
        case TOKEN_GS:
            XASM32_TRACE_BLOCK_BEGIN("token(TOKEN_SREGs)");
            _chrf(info = parsearg_mem(aasmContext, token));
            if (info.type != XASM32_OPERAND_M)
                _serf_;
            XASM32_TRACE_BLOCK_END;
            break;
        default:
            _serf_;
            break;
        }
        info.ptr = PTR_FAR;
        XASM32_TRACE_BLOCK_END;
        break;
    case TOKEN_CR0:
        info.type = XASM32_OPERAND_CREG;
        info.creg = CREG_CR0;
        break;
    case TOKEN_CR2:
        info.type = XASM32_OPERAND_CREG;
        info.creg = CREG_CR2;
        break;
    case TOKEN_CR3:
        info.type = XASM32_OPERAND_CREG;
        info.creg = CREG_CR3;
        break;
    case TOKEN_DR0:
        info.type = XASM32_OPERAND_DREG;
        info.dreg = DREG_DR0;
        break;
    case TOKEN_DR1:
        info.type = XASM32_OPERAND_DREG;
        info.dreg = DREG_DR1;
        break;
    case TOKEN_DR2:
        info.type = XASM32_OPERAND_DREG;
        info.dreg = DREG_DR2;
        break;
    case TOKEN_DR3:
        info.type = XASM32_OPERAND_DREG;
        info.dreg = DREG_DR3;
        break;
    case TOKEN_DR6:
        info.type = XASM32_OPERAND_DREG;
        info.dreg = DREG_DR6;
        break;
    case TOKEN_DR7:
        info.type = XASM32_OPERAND_DREG;
        info.dreg = DREG_DR7;
        break;
    case TOKEN_TR6:
        info.type = XASM32_OPERAND_TREG;
        info.treg = TREG_TR6;
        break;
    case TOKEN_TR7:
        info.type = XASM32_OPERAND_TREG;
        info.treg = TREG_TR7;
        break;
    default:
        _serf_;
        break;
    }
    XASM32_TRACE_CALL_END;
    return info;
}
/* assembly compiler: code generator */
static void _c_setbyte(aasm32_context *aasmContext, lib_u8 byte)
{
    (*(lib_u8 *)(acode + iop)) = byte;
    iop += 1;
}
static void _c_setword(aasm32_context *aasmContext, lib_u16 word)
{
    (*(lib_u16 *)(acode + iop)) = word;
    iop += 2;
}
static void _c_setdword(aasm32_context *aasmContext, lib_u32 dword)
{
    (*(lib_u32 *)(acode + iop)) = dword;
    iop += 4;
}
static void _c_imm8(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("_c_imm8");
    XASM32_TRACE_CHECK_RETURN(_c_setbyte(aasmContext, byte));
    XASM32_TRACE_CALL_END;
}
static void _c_imm16(aasm32_context *aasmContext, lib_u16 word)
{
    XASM32_TRACE_CALL_BEGIN("_c_imm16");
    XASM32_TRACE_CHECK_RETURN(_c_setword(aasmContext, word));
    XASM32_TRACE_CALL_END;
}
static void _c_imm32(aasm32_context *aasmContext, lib_u32 dword)
{
    XASM32_TRACE_CALL_BEGIN("_c_imm32");
    XASM32_TRACE_CHECK_RETURN(_c_setdword(aasmContext, dword));
    XASM32_TRACE_CALL_END;
}
static void _c_modrm(aasm32_context *aasmContext, t_aasm_oprinfo rminfo, lib_u8 reg)
{
    lib_u8 sibval;
    lib_u8 modrmval = (reg << 3);

    XASM32_TRACE_CALL_BEGIN("_c_rminfo");

    switch (rminfo.mem)
    {
    case MEM_BX_SI:
    case MEM_BX_DI:
    case MEM_BP_SI:
    case MEM_BP_DI:
    case MEM_SI:
    case MEM_DI:
    case MEM_BP:
    case MEM_BX:
        XASM32_TRACE_BLOCK_BEGIN("16-bit Addressing");
        _SetAddressSize(2);
        switch (rminfo.mod)
        {
        case MOD_M:
            modrmval |= (0 << 6);
            modrmval |= (lib_u8)rminfo.mem;
            _c_setbyte(aasmContext, modrmval);
            switch (rminfo.mem)
            {
            case MEM_BP:
                _c_setword(aasmContext, rminfo.disp16);
                break;
            default:
                break;
            }
            break;
        case MOD_M_DISP8:
            modrmval |= (1 << 6);
            modrmval |= (lib_u8)rminfo.mem;
            _c_setbyte(aasmContext, modrmval);
            _c_setbyte(aasmContext, rminfo.disp8);
            break;
        case MOD_M_DISP16:
            modrmval |= (2 << 6);
            modrmval |= (lib_u8)rminfo.mem;
            _c_setbyte(aasmContext, modrmval);
            _c_setword(aasmContext, rminfo.disp16);
            break;
        case MOD_R:
            XASM32_TRACE_BLOCK_BEGIN("mod(MOD_R)");
            modrmval |= (3 << 6);
            switch (rminfo.type)
            {
            case XASM32_OPERAND_R8:
                modrmval |= (lib_u8)rminfo.reg8;
                _c_setbyte(aasmContext, modrmval);
                break;
            case XASM32_OPERAND_R16:
                modrmval |= (lib_u8)rminfo.reg16;
                _c_setbyte(aasmContext, modrmval);
                break;
            case XASM32_OPERAND_R32:
                modrmval |= (lib_u8)rminfo.reg32;
                _c_setbyte(aasmContext, modrmval);
                break;
            default:
                _ser_;
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        default:
            _ser_;
            break;
        }
        XASM32_TRACE_BLOCK_END;
        break;
    case MEM_EAX:
    case MEM_ECX:
    case MEM_EDX:
    case MEM_EBX:
    case MEM_SIB:
    case MEM_EBP:
    case MEM_ESI:
    case MEM_EDI:
        XASM32_TRACE_BLOCK_BEGIN("32-bit Addressing");
        _SetAddressSize(4);
        switch (rminfo.mod)
        {
        case MOD_M:
            modrmval |= (0 << 6);
            modrmval |= (lib_u8)rminfo.mem & 0x07;
            _c_setbyte(aasmContext, modrmval);
            switch (rminfo.mem)
            {
            case MEM_SIB:
                sibval = (lib_u8)rminfo.sib.base;
                sibval |= ((lib_u8)rminfo.sib.index << 3);
                switch (rminfo.sib.scale)
                {
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
                _c_setbyte(aasmContext, sibval);
                switch (rminfo.sib.base)
                {
                case R32_EBP:
                    _c_setdword(aasmContext, rminfo.disp32);
                    break;
                default:
                    break;
                }
                break;
            case MEM_EBP:
                _c_setdword(aasmContext, rminfo.disp32);
                break;
            default:
                break;
            }
            break;
        case MOD_M_DISP8:
            modrmval |= (1 << 6);
            modrmval |= (lib_u8)rminfo.mem & 0x07;
            _c_setbyte(aasmContext, modrmval);
            switch (rminfo.mem)
            {
            case MEM_SIB:
                sibval = (lib_u8)rminfo.sib.base;
                sibval |= ((lib_u8)rminfo.sib.index << 3);
                switch (rminfo.sib.scale)
                {
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
                _c_setbyte(aasmContext, sibval);
                break;
            default:
                break;
            }
            _c_setbyte(aasmContext, rminfo.disp8);
            break;
        case MOD_M_DISP32:
            modrmval |= (2 << 6);
            modrmval |= (lib_u8)rminfo.mem & 0x07;
            _c_setbyte(aasmContext, modrmval);
            switch (rminfo.mem)
            {
            case MEM_SIB:
                sibval = (lib_u8)rminfo.sib.base;
                sibval |= ((lib_u8)rminfo.sib.index << 3);
                switch (rminfo.sib.scale)
                {
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
                _c_setbyte(aasmContext, sibval);
                break;
            default:
                break;
            }
            _c_setdword(aasmContext, rminfo.disp32);
            break;
        case MOD_R:
            XASM32_TRACE_BLOCK_BEGIN("mod(MOD_R)");
            modrmval |= (3 << 6);
            switch (rminfo.type)
            {
            case XASM32_OPERAND_R8:
                modrmval |= (lib_u8)rminfo.reg8;
                _c_setbyte(aasmContext, modrmval);
                break;
            case XASM32_OPERAND_R16:
                modrmval |= (lib_u8)rminfo.reg16;
                _c_setbyte(aasmContext, modrmval);
                break;
            case XASM32_OPERAND_R32:
                modrmval |= (lib_u8)rminfo.reg32;
                _c_setbyte(aasmContext, modrmval);
                break;
            default:
                _ser_;
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        default:
            _ser_;
            break;
        }
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}

/* concrete instructions */
static void ADD_RM8_R8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("ADD_RM8_R8");
    _c_setbyte(aasmContext, 0x00);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg8));
    XASM32_TRACE_CALL_END;
}
static void ADD_RM32_R32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("ADD_RM32_R32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x01);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void ADD_R8_RM8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("ADD_R8_RM8");
    _c_setbyte(aasmContext, 0x02);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg8));
    XASM32_TRACE_CALL_END;
}
static void ADD_R32_RM32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("ADD_R32_RM32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x03);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void ADD_AL_I8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("ADD_AL_I8");
    _c_setbyte(aasmContext, 0x04);
    XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, aopri2.imm8));
    XASM32_TRACE_CALL_END;
}
static void ADD_EAX_I32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("ADD_EAX_I32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x05);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri2.imm16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_imm32(aasmContext, aopri2.imm32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void PUSH_ES(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PUSH_ES");
    _c_setbyte(aasmContext, 0x06);
    XASM32_TRACE_CALL_END;
}
static void POP_ES(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("POP_ES");
    _c_setbyte(aasmContext, 0x07);
    XASM32_TRACE_CALL_END;
}
static void OR_RM8_R8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("OR_RM8_R8");
    _c_setbyte(aasmContext, 0x08);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg8));
    XASM32_TRACE_CALL_END;
}
static void OR_RM32_R32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("OR_RM32_R32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x09);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void OR_R8_RM8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("OR_R8_RM8");
    _c_setbyte(aasmContext, 0x0a);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg8));
    XASM32_TRACE_CALL_END;
}
static void OR_R32_RM32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("OR_R32_RM32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x0b);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void OR_AL_I8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("OR_AL_I8");
    _c_setbyte(aasmContext, 0x0c);
    _c_imm8(aasmContext, aopri2.imm8);
    XASM32_TRACE_CALL_END;
}
static void OR_EAX_I32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("OR_EAX_I32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x0d);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri2.imm16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_imm32(aasmContext, aopri2.imm32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void PUSH_CS(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PUSH_CS");
    _c_setbyte(aasmContext, 0x0e);
    XASM32_TRACE_CALL_END;
}
static void POP_CS(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("POP_CS");
    _c_setbyte(aasmContext, 0x0f);
    XASM32_TRACE_CALL_END;
}
static void INS_0F(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("INS_0F");
    _c_setbyte(aasmContext, 0x0f);
    XASM32_TRACE_CALL_END;
}
static void ADC_RM8_R8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("ADC_RM8_R8");
    _c_setbyte(aasmContext, 0x10);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg8));
    XASM32_TRACE_CALL_END;
}
static void ADC_RM32_R32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("ADC_RM32_R32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x11);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void ADC_R8_RM8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("ADC_R8_RM8");
    _c_setbyte(aasmContext, 0x12);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg8));
    XASM32_TRACE_CALL_END;
}
static void ADC_R32_RM32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("ADC_R32_RM32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x13);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void ADC_AL_I8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("ADC_AL_I8");
    _c_setbyte(aasmContext, 0x14);
    XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, aopri2.imm8));
    XASM32_TRACE_CALL_END;
}
static void ADC_EAX_I32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("ADC_EAX_I32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x15);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri2.imm16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_imm32(aasmContext, aopri2.imm32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void PUSH_SS(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PUSH_SS");
    _c_setbyte(aasmContext, 0x16);
    XASM32_TRACE_CALL_END;
}
static void POP_SS(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("POP_SS");
    _c_setbyte(aasmContext, 0x17);
    XASM32_TRACE_CALL_END;
}
static void SBB_RM8_R8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SBB_RM8_R8");
    _c_setbyte(aasmContext, 0x18);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg8));
    XASM32_TRACE_CALL_END;
}
static void SBB_RM32_R32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("SBB_RM32_R32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x19);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void SBB_R8_RM8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SBB_R8_RM8");
    _c_setbyte(aasmContext, 0x1a);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg8));
    XASM32_TRACE_CALL_END;
}
static void SBB_R32_RM32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("SBB_R32_RM32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x1b);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void SBB_AL_I8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SBB_AL_I8");
    _c_setbyte(aasmContext, 0x1c);
    _c_imm8(aasmContext, aopri2.imm8);
    XASM32_TRACE_CALL_END;
}
static void SBB_EAX_I32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("SBB_EAX_I32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x1d);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri2.imm16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_imm32(aasmContext, aopri2.imm32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void PUSH_DS(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PUSH_DS");
    _c_setbyte(aasmContext, 0x1e);
    XASM32_TRACE_CALL_END;
}
static void POP_DS(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("POP_DS");
    _c_setbyte(aasmContext, 0x1f);
    XASM32_TRACE_CALL_END;
}
static void AND_RM8_R8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("AND_RM8_R8");
    _c_setbyte(aasmContext, 0x20);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg8));
    XASM32_TRACE_CALL_END;
}
static void AND_RM32_R32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("AND_RM32_R32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x21);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void AND_R8_RM8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("AND_R8_RM8");
    _c_setbyte(aasmContext, 0x22);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg8));
    XASM32_TRACE_CALL_END;
}
static void AND_R32_RM32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("AND_R32_RM32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x23);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void AND_AL_I8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("AND_AL_I8");
    _c_setbyte(aasmContext, 0x24);
    XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, aopri2.imm8));
    XASM32_TRACE_CALL_END;
}
static void AND_EAX_I32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("AND_EAX_I32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x25);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri2.imm16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_imm32(aasmContext, aopri2.imm32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void PREFIX_ES(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PREFIX_ES");
    if (ARG_NONE)
        aoprig.flages = 1;
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void DAA(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("DAA");
    if (ARG_NONE)
        _c_setbyte(aasmContext, 0x27);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void SUB_RM8_R8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SUB_RM8_R8");
    _c_setbyte(aasmContext, 0x28);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg8));
    XASM32_TRACE_CALL_END;
}
static void SUB_RM32_R32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("SUB_RM32_R32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x29);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void SUB_R8_RM8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SUB_R8_RM8");
    _c_setbyte(aasmContext, 0x2a);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg8));
    XASM32_TRACE_CALL_END;
}
static void SUB_R32_RM32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("SUB_R32_RM32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x2b);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void SUB_AL_I8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SUB_AL_I8");
    _c_setbyte(aasmContext, 0x2c);
    _c_imm8(aasmContext, aopri2.imm8);
    XASM32_TRACE_CALL_END;
}
static void SUB_EAX_I32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("SUB_EAX_I32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x2d);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri2.imm16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_imm32(aasmContext, aopri2.imm32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void PREFIX_CS(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PREFIX_CS");
    if (ARG_NONE)
        aoprig.flagcs = 1;
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void DAS(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("DAS");
    if (ARG_NONE)
        _c_setbyte(aasmContext, 0x2f);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void XOR_RM8_R8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("XOR_RM8_R8");
    _c_setbyte(aasmContext, 0x30);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg8));
    XASM32_TRACE_CALL_END;
}
static void XOR_RM32_R32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("XOR_RM32_R32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x31);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void XOR_R8_RM8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("XOR_R8_RM8");
    _c_setbyte(aasmContext, 0x32);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg8));
    XASM32_TRACE_CALL_END;
}
static void XOR_R32_RM32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("XOR_R32_RM32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x33);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void XOR_AL_I8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("XOR_AL_I8");
    _c_setbyte(aasmContext, 0x34);
    XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, aopri2.imm8));
    XASM32_TRACE_CALL_END;
}
static void XOR_EAX_I32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("XOR_EAX_I32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x35);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri2.imm16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_imm32(aasmContext, aopri2.imm32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void PREFIX_SS(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PREFIX_SS");
    if (ARG_NONE)
        aoprig.flagss = 1;
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void AAA(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("AAA");
    if (ARG_NONE)
        _c_setbyte(aasmContext, 0x37);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void CMP_RM8_R8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("CMP_RM8_R8");
    _c_setbyte(aasmContext, 0x38);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg8));
    XASM32_TRACE_CALL_END;
}
static void CMP_RM32_R32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("CMP_RM32_R32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x39);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void CMP_R8_RM8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("CMP_R8_RM8");
    _c_setbyte(aasmContext, 0x3a);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg8));
    XASM32_TRACE_CALL_END;
}
static void CMP_R32_RM32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("CMP_R32_RM32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x3b);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void CMP_AL_I8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("CMP_AL_I8");
    _c_setbyte(aasmContext, 0x3c);
    _c_imm8(aasmContext, aopri2.imm8);
    XASM32_TRACE_CALL_END;
}
static void CMP_EAX_I32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("CMP_EAX_I32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x3d);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri2.imm16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_imm32(aasmContext, aopri2.imm32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void PREFIX_DS(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PREFIX_DS");
    if (ARG_NONE)
        aoprig.flagds = 1;
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void AAS(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("AAS");
    if (ARG_NONE)
        _c_setbyte(aasmContext, 0x3f);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void INC_EAX(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("INC_EAX");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x40);
    XASM32_TRACE_CALL_END;
}
static void INC_ECX(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("INC_ECX");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x41);
    XASM32_TRACE_CALL_END;
}
static void INC_EDX(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("INC_EDX");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x42);
    XASM32_TRACE_CALL_END;
}
static void INC_EBX(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("INC_EBX");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x43);
    XASM32_TRACE_CALL_END;
}
static void INC_ESP(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("INC_ESP");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x44);
    XASM32_TRACE_CALL_END;
}
static void INC_EBP(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("INC_EBP");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x45);
    XASM32_TRACE_CALL_END;
}
static void INC_ESI(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("INC_ESI");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x46);
    XASM32_TRACE_CALL_END;
}
static void INC_EDI(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("INC_EDI");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x47);
    XASM32_TRACE_CALL_END;
}
static void DEC_EAX(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("DEC_EAX");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x48);
    XASM32_TRACE_CALL_END;
}
static void DEC_ECX(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("DEC_ECX");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x49);
    XASM32_TRACE_CALL_END;
}
static void DEC_EDX(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("DEC_EDX");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x4a);
    XASM32_TRACE_CALL_END;
}
static void DEC_EBX(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("DEC_EBX");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x4b);
    XASM32_TRACE_CALL_END;
}
static void DEC_ESP(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("DEC_ESP");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x4c);
    XASM32_TRACE_CALL_END;
}
static void DEC_EBP(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("DEC_EBP");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x4d);
    XASM32_TRACE_CALL_END;
}
static void DEC_ESI(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("DEC_ESI");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x4e);
    XASM32_TRACE_CALL_END;
}
static void DEC_EDI(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("DEC_EDI");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x4f);
    XASM32_TRACE_CALL_END;
}
static void PUSH_EAX(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("PUSH_EAX");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x50);
    XASM32_TRACE_CALL_END;
}
static void PUSH_ECX(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("PUSH_ECX");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x51);
    XASM32_TRACE_CALL_END;
}
static void PUSH_EDX(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("PUSH_EDX");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x52);
    XASM32_TRACE_CALL_END;
}
static void PUSH_EBX(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("PUSH_EBX");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x53);
    XASM32_TRACE_CALL_END;
}
static void PUSH_ESP(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("PUSH_ESP");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x54);
    XASM32_TRACE_CALL_END;
}
static void PUSH_EBP(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("PUSH_EBP");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x55);
    XASM32_TRACE_CALL_END;
}
static void PUSH_ESI(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("PUSH_ESI");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x56);
    XASM32_TRACE_CALL_END;
}
static void PUSH_EDI(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("PUSH_EDI");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x57);
    XASM32_TRACE_CALL_END;
}
static void POP_EAX(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("POP_EAX");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x58);
    XASM32_TRACE_CALL_END;
}
static void POP_ECX(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("POP_ECX");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x59);
    XASM32_TRACE_CALL_END;
}
static void POP_EDX(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("POP_EDX");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x5a);
    XASM32_TRACE_CALL_END;
}
static void POP_EBX(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("POP_EBX");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x5b);
    XASM32_TRACE_CALL_END;
}
static void POP_ESP(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("POP_ESP");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x5c);
    XASM32_TRACE_CALL_END;
}
static void POP_EBP(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("POP_EBP");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x5d);
    XASM32_TRACE_CALL_END;
}
static void POP_ESI(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("POP_ESI");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x5e);
    XASM32_TRACE_CALL_END;
}
static void POP_EDI(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("POP_EDI");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x5f);
    XASM32_TRACE_CALL_END;
}
static void PUSHA(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("PUSHA");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x60);
    XASM32_TRACE_CALL_END;
}
static void POPA(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("POPA");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x61);
    XASM32_TRACE_CALL_END;
}

static void BOUND_R32_M32_32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("BOUND_R32_M32_32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x62);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void ARPL_RM16_R16(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("ARPL_RM16_R16");
    if (ARG_RM16_R16)
    {
        XASM32_TRACE_BLOCK_BEGIN("ARG_RM16_R16");
        _c_setbyte(aasmContext, 0x63);
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg16));
        XASM32_TRACE_BLOCK_END;
    }
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void PREFIX_FS(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PREFIX_FS");
    if (ARG_NONE)
        aoprig.flagfs = 1;
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void PREFIX_GS(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PREFIX_GS");
    if (ARG_NONE)
        aoprig.flaggs = 1;
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void PREFIX_OprSize(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PREFIX_OprSize");
    if (ARG_NONE)
        prefix_oprsizeg = 1;
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void PREFIX_AddrSize(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PREFIX_AddrSize");
    if (ARG_NONE)
        prefix_addrsizeg = 1;
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void PUSH_I32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("PUSH_I32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x68);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri1.imm16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_imm32(aasmContext, aopri1.imm32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void IMUL_R32_RM32_I32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("IMUL_R32_RM32_I32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x69);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg16));
        XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri3.imm16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg32));
        XASM32_TRACE_CHECK_RETURN(_c_imm32(aasmContext, aopri3.imm32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
    ;
}
static void PUSH_I8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PUSH_I8");
    _c_setbyte(aasmContext, 0x6a);
    XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, aopri1.imm8));
    XASM32_TRACE_CALL_END;
}
static void IMUL_R32_RM32_I8(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("IMUL_R32_RM32_I32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x6b);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg16));
        XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, aopri3.imm8));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg32));
        XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, aopri3.imm8));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
    ;
}
static void INSB(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("INSB");
    _c_setbyte(aasmContext, 0x6c);
    rinfo = LIB_NULL;
    if (ARG_NONE)
        ;
    else if (ARG_ESDI8_DX)
        _SetAddressSize(2);
    else if (ARG_ESEDI8_DX)
        _SetAddressSize(4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void INSW(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("INSW");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x6d);
    rinfo = LIB_NULL;
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        if (ARG_NONE)
            ;
        else if (ARG_ESDI16_DX)
            _SetAddressSize(2);
        else if (ARG_ESEDI16_DX)
            _SetAddressSize(4);
        else
            _ser_;
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        if (ARG_NONE)
            ;
        else if (ARG_ESDI32_DX)
            _SetAddressSize(2);
        else if (ARG_ESEDI32_DX)
            _SetAddressSize(4);
        else
            _ser_;
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void OUTSB(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("OUTSB");
    _c_setbyte(aasmContext, 0x6e);
    rinfo = &aopri1;
    if (rinfo->flagds)
        rinfo->flagds = 0;
    if (ARG_NONE)
        rinfo = LIB_NULL;
    else if (ARG_DX_DSSI8)
        _SetAddressSize(2);
    else if (ARG_DSESI8)
        _SetAddressSize(4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void OUTSW(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("OUTSW");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x6f);
    rinfo = &aopri1;
    if (rinfo->flagds)
        rinfo->flagds = 0;
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        if (ARG_NONE)
            rinfo = LIB_NULL;
        else if (ARG_DX_DSSI16)
            _SetAddressSize(2);
        else if (ARG_DX_DSESI16)
            _SetAddressSize(4);
        else
            _ser_;
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        if (ARG_NONE)
            rinfo = LIB_NULL;
        else if (ARG_DX_DSSI32)
            _SetAddressSize(2);
        else if (ARG_DX_DSESI32)
            _SetAddressSize(4);
        else
            _ser_;
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void INS_80(aasm32_context *aasmContext, lib_u8 rid)
{
    XASM32_TRACE_CALL_BEGIN("INS_80");
    _c_setbyte(aasmContext, 0x80);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, rid));
    XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, aopri2.imm8));
    XASM32_TRACE_CALL_END;
}
static void INS_81(aasm32_context *aasmContext, lib_u8 rid, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("INS_81");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x81);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, rid));
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri2.imm16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_imm32(aasmContext, aopri2.imm32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void INS_83(aasm32_context *aasmContext, lib_u8 rid, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("INS_83");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x83);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, rid));
    XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, aopri2.imm8));
    XASM32_TRACE_CALL_END;
}
static void TEST_RM8_R8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("TEST_RM8_R8");
    _c_setbyte(aasmContext, 0x84);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg8));
    XASM32_TRACE_CALL_END;
}
static void TEST_RM32_R32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("TEST_RM32_R32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x85);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void XCHG_RM8_R8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("XCHG_RM8_R8");
    _c_setbyte(aasmContext, 0x86);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg8));
    XASM32_TRACE_CALL_END;
}
static void XCHG_RM32_R32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("XCHG_RM32_R32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x87);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void MOV_RM8_R8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_RM8_R8");
    _c_setbyte(aasmContext, 0x88);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg8));
    XASM32_TRACE_CALL_END;
}
static void MOV_RM32_R32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("MOV_RM32_R32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x89);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void MOV_R8_RM8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_RM8_R8");
    _c_setbyte(aasmContext, 0x8a);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg8));
    XASM32_TRACE_CALL_END;
}
static void MOV_R32_RM32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("MOV_R32_RM32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x8b);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void MOV_RM16_SREG(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("MOV_RM16_SREG");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x8c);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.sreg));
    XASM32_TRACE_CALL_END;
}
static void LEA_R32_M32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("LEA_R32_M32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x8d);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void MOV_SREG_RM16(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("MOV_SREG_RM16");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x8e);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.sreg));
    XASM32_TRACE_CALL_END;
}
static void INS_8F(aasm32_context *aasmContext, lib_u8 rid, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("INS_8F");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x8f);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, rid));
    XASM32_TRACE_CALL_END;
}
static void NOP(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("NOP");
    if (ARG_NONE)
        _c_setbyte(aasmContext, 0x90);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void XCHG_EAX_EAX(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("XCHG_EAX_EAX");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x90);
    XASM32_TRACE_CALL_END;
}
static void XCHG_ECX_EAX(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("XCHG_ECX_EAX");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x91);
    XASM32_TRACE_CALL_END;
}
static void XCHG_EDX_EAX(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("XCHG_EDX_EAX");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x92);
    XASM32_TRACE_CALL_END;
}
static void XCHG_EBX_EAX(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("XCHG_EBX_EAX");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x93);
    XASM32_TRACE_CALL_END;
}
static void XCHG_ESP_EAX(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("XCHG_ESP_EAX");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x94);
    XASM32_TRACE_CALL_END;
}
static void XCHG_EBP_EAX(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("XCHG_EBP_EAX");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x95);
    XASM32_TRACE_CALL_END;
}
static void XCHG_ESI_EAX(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("XCHG_ESI_EAX");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x96);
    XASM32_TRACE_CALL_END;
}
static void XCHG_EDI_EAX(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("XCHG_EDI_EAX");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x97);
    XASM32_TRACE_CALL_END;
}
static void CBW(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("CBW");
    _SetOperandSize(byte);
    if (ARG_NONE)
        _c_setbyte(aasmContext, 0x98);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void CWD(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("CWD");
    _SetOperandSize(byte);
    if (ARG_NONE)
        _c_setbyte(aasmContext, 0x99);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void CALL_PTR16_32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("CALL_PTR16_32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x9a);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, (lib_u16)aopri1.reip));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_imm32(aasmContext, (lib_u32)aopri1.reip));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri1.rcs));
    XASM32_TRACE_CALL_END;
}
static void WAIT(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("WAIT");
    _c_setbyte(aasmContext, 0x9b);
    XASM32_TRACE_CALL_END;
}
static void PUSHF(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("PUSHF");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x9c);
    XASM32_TRACE_CALL_END;
}
static void POPF(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("POPF");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0x9d);
    XASM32_TRACE_CALL_END;
}
static void SAHF(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SAHF");
    _c_setbyte(aasmContext, 0x9e);
    XASM32_TRACE_CALL_END;
}
static void LAHF(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("LAHF");
    _c_setbyte(aasmContext, 0x9f);
    XASM32_TRACE_CALL_END;
}
static void MOV_AL_MOFFS8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_AL_MOFFS8");
    _c_setbyte(aasmContext, 0xa0);
    if (aopri2.mem == MEM_BP)
    {
        XASM32_TRACE_BLOCK_BEGIN("16-bit Addressing");
        _SetAddressSize(2);
        XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri2.disp16));
        XASM32_TRACE_BLOCK_END;
    }
    else if (aopri2.mem == MEM_EBP)
    {
        XASM32_TRACE_BLOCK_BEGIN("32-bit Addressing");
        _SetAddressSize(4);
        XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri2.disp32));
        XASM32_TRACE_BLOCK_END;
    }
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void MOV_EAX_MOFFS32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("MOV_EAX_MOFFS32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0xa1);
    if (aopri2.mem == MEM_BP)
    {
        XASM32_TRACE_BLOCK_BEGIN("16-bit Addressing");
        _SetAddressSize(2);
        XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri2.disp16));
        XASM32_TRACE_BLOCK_END;
    }
    else if (aopri2.mem == MEM_EBP)
    {
        XASM32_TRACE_BLOCK_BEGIN("32-bit Addressing");
        _SetAddressSize(4);
        XASM32_TRACE_CHECK_RETURN(_c_imm32(aasmContext, aopri2.disp32));
        XASM32_TRACE_BLOCK_END;
    }
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void MOV_MOFFS8_AL(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_MOFFS8_AL");
    _c_setbyte(aasmContext, 0xa2);
    if (aopri1.mem == MEM_BP)
    {
        XASM32_TRACE_BLOCK_BEGIN("16-bit Addressing");
        _SetAddressSize(2);
        XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri1.disp16));
        XASM32_TRACE_BLOCK_END;
    }
    else if (aopri1.mem == MEM_EBP)
    {
        XASM32_TRACE_BLOCK_BEGIN("32-bit Addressing");
        _SetAddressSize(4);
        XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri1.disp32));
        XASM32_TRACE_BLOCK_END;
    }
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void MOV_MOFFS32_EAX(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("MOV_MOFFS32_EAX");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0xa3);
    if (aopri1.mem == MEM_BP)
    {
        XASM32_TRACE_BLOCK_BEGIN("16-bit Addressing");
        _SetAddressSize(2);
        XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri1.disp16));
        XASM32_TRACE_BLOCK_END;
    }
    else if (aopri1.mem == MEM_EBP)
    {
        XASM32_TRACE_BLOCK_BEGIN("32-bit Addressing");
        _SetAddressSize(4);
        XASM32_TRACE_CHECK_RETURN(_c_imm32(aasmContext, aopri1.disp32));
        XASM32_TRACE_BLOCK_END;
    }
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void MOVSB(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOVSB");
    _c_setbyte(aasmContext, 0xa4);
    rinfo = &aopri2;
    if (rinfo->flagds)
        rinfo->flagds = 0;
    if (ARG_NONE)
        rinfo = LIB_NULL;
    else if (ARG_ESDI8_DSSI8)
        _SetAddressSize(2);
    else if (ARG_ESEDI8_DSESI8)
        _SetAddressSize(4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void MOVSW(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("MOVSW");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0xa5);
    rinfo = &aopri2;
    if (rinfo->flagds)
        rinfo->flagds = 0;
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        if (ARG_NONE)
            rinfo = LIB_NULL;
        else if (ARG_ESDI16_DSSI16)
            _SetAddressSize(2);
        else if (ARG_ESEDI16_DSESI16)
            _SetAddressSize(4);
        else
            _ser_;
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        if (ARG_NONE)
            rinfo = LIB_NULL;
        else if (ARG_ESDI32_DSSI32)
            _SetAddressSize(2);
        else if (ARG_ESEDI32_DSESI32)
            _SetAddressSize(4);
        else
            _ser_;
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void CMPSB(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("CMPSB");
    _c_setbyte(aasmContext, 0xa6);
    rinfo = &aopri1;
    if (rinfo->flagds)
        rinfo->flagds = 0;
    if (ARG_NONE)
        rinfo = LIB_NULL;
    else if (ARG_DSSI8_ESDI8)
        _SetAddressSize(2);
    else if (ARG_DSESI8_ESEDI8)
        _SetAddressSize(4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void CMPSW(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("CMPSW");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0xa7);
    rinfo = &aopri1;
    if (rinfo->flagds)
        rinfo->flagds = 0;
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        if (ARG_NONE)
            rinfo = LIB_NULL;
        else if (ARG_DSSI16_ESDI16)
            _SetAddressSize(2);
        else if (ARG_DSESI16_ESEDI16)
            _SetAddressSize(4);
        else
            _ser_;
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        if (ARG_NONE)
            rinfo = LIB_NULL;
        else if (ARG_DSSI32_ESDI32)
            _SetAddressSize(2);
        else if (ARG_DSESI32_ESEDI32)
            _SetAddressSize(4);
        else
            _ser_;
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void TEST_AL_I8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("TEST_AL_I8");
    _c_setbyte(aasmContext, 0xa8);
    XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, aopri2.imm8));
    XASM32_TRACE_CALL_END;
}
static void TEST_EAX_I32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("TEST_EAX_I32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0xa9);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri2.imm16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_imm32(aasmContext, aopri2.imm32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void STOSB(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("STOSB");
    _c_setbyte(aasmContext, 0xaa);
    rinfo = LIB_NULL;
    if (ARG_NONE)
        ;
    else if (ARG_ESDI8)
        _SetAddressSize(2);
    else if (ARG_ESEDI8)
        _SetAddressSize(4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void STOSW(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("STOSW");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0xab);
    rinfo = LIB_NULL;
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        if (ARG_NONE)
            ;
        else if (ARG_ESDI16)
            _SetAddressSize(2);
        else if (ARG_ESEDI16)
            _SetAddressSize(4);
        else
            _ser_;
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        if (ARG_NONE)
            ;
        else if (ARG_ESDI32)
            _SetAddressSize(2);
        else if (ARG_ESEDI32)
            _SetAddressSize(4);
        else
            _ser_;
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void LODSB(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("LODSB");
    _c_setbyte(aasmContext, 0xac);
    rinfo = &aopri1;
    if (rinfo->flagds)
        rinfo->flagds = 0;
    if (ARG_NONE)
        rinfo = LIB_NULL;
    else if (ARG_DSSI8)
        _SetAddressSize(2);
    else if (ARG_DSESI8)
        _SetAddressSize(4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void LODSW(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("LODSW");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0xad);
    rinfo = &aopri1;
    if (rinfo->flagds)
        rinfo->flagds = 0;
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        if (ARG_NONE)
            rinfo = LIB_NULL;
        else if (ARG_DSSI16)
            _SetAddressSize(2);
        else if (ARG_DSESI16)
            _SetAddressSize(4);
        else
            _ser_;
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        if (ARG_NONE)
            rinfo = LIB_NULL;
        else if (ARG_DSSI32)
            _SetAddressSize(2);
        else if (ARG_DSESI32)
            _SetAddressSize(4);
        else
            _ser_;
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void SCASB(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SCASB");
    _c_setbyte(aasmContext, 0xae);
    rinfo = LIB_NULL;
    if (ARG_NONE)
        ;
    else if (ARG_ESDI8)
        _SetAddressSize(2);
    else if (ARG_ESEDI8)
        _SetAddressSize(4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void SCASW(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("SCASW");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0xaf);
    rinfo = LIB_NULL;
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        if (ARG_NONE)
            ;
        else if (ARG_ESDI16)
            _SetAddressSize(2);
        else if (ARG_ESEDI16)
            _SetAddressSize(4);
        else
            _ser_;
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        if (ARG_NONE)
            ;
        else if (ARG_ESDI32)
            _SetAddressSize(2);
        else if (ARG_ESEDI32)
            _SetAddressSize(4);
        else
            _ser_;
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void MOV_AL_I8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_AL_I8");
    _c_setbyte(aasmContext, 0xb0);
    XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, aopri2.imm8));
    XASM32_TRACE_CALL_END;
}
static void MOV_CL_I8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_CL_I8");
    _c_setbyte(aasmContext, 0xb1);
    XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, aopri2.imm8));
    XASM32_TRACE_CALL_END;
}
static void MOV_DL_I8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_DL_I8");
    _c_setbyte(aasmContext, 0xb2);
    XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, aopri2.imm8));
    XASM32_TRACE_CALL_END;
}
static void MOV_BL_I8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_BL_I8");
    _c_setbyte(aasmContext, 0xb3);
    XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, aopri2.imm8));
    XASM32_TRACE_CALL_END;
}
static void MOV_AH_I8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_AH_I8");
    _c_setbyte(aasmContext, 0xb4);
    XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, aopri2.imm8));
    XASM32_TRACE_CALL_END;
}
static void MOV_CH_I8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_CH_I8");
    _c_setbyte(aasmContext, 0xb5);
    XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, aopri2.imm8));
    XASM32_TRACE_CALL_END;
}
static void MOV_DH_I8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_DH_I8");
    _c_setbyte(aasmContext, 0xb6);
    XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, aopri2.imm8));
    XASM32_TRACE_CALL_END;
}
static void MOV_BH_I8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_BH_I8");
    _c_setbyte(aasmContext, 0xb7);
    XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, aopri2.imm8));
    XASM32_TRACE_CALL_END;
}
static void MOV_EAX_I32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("MOV_EAX_I32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0xb8);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri2.imm16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_imm32(aasmContext, aopri2.imm32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void MOV_ECX_I32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("MOV_ECX_I32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0xb9);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri2.imm16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_imm32(aasmContext, aopri2.imm32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void MOV_EDX_I32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("MOV_EDX_I32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0xba);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri2.imm16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_imm32(aasmContext, aopri2.imm32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void MOV_EBX_I32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("MOV_EBX_I32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0xbb);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri2.imm16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_imm32(aasmContext, aopri2.imm32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void MOV_ESP_I32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("MOV_ESP_I32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0xbc);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri2.imm16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_imm32(aasmContext, aopri2.imm32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void MOV_EBP_I32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("MOV_EBP_I32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0xbd);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri2.imm16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_imm32(aasmContext, aopri2.imm32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void MOV_ESI_I32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("MOV_ESI_I32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0xbe);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri2.imm16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_imm32(aasmContext, aopri2.imm32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void MOV_EDI_I32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("MOV_EDI_I32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0xbf);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri2.imm16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_imm32(aasmContext, aopri2.imm32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void INS_C0(aasm32_context *aasmContext, lib_u8 rid)
{
    XASM32_TRACE_CALL_BEGIN("INS_C0");
    _c_setbyte(aasmContext, 0xc0);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, rid));
    XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, aopri2.imm8));
    XASM32_TRACE_CALL_END;
}
static void INS_C1(aasm32_context *aasmContext, lib_u8 rid, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("INS_C1");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0xc1);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, rid));
    XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, aopri2.imm8));
    XASM32_TRACE_CALL_END;
}
static void RET_I16(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("RET_I16");
    _c_setbyte(aasmContext, 0xc2);
    XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri1.imm16));
    XASM32_TRACE_CALL_END;
}
static void RET_(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("RET");
    _c_setbyte(aasmContext, 0xc3);
    XASM32_TRACE_CALL_END;
}
static void LES_R32_M16_32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("LES_R32_M16_32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0xc4);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void LDS_R32_M16_32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("LDS_R32_M16_32");
    _c_setbyte(aasmContext, 0xc5);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void INS_C6(aasm32_context *aasmContext, lib_u8 rid)
{
    XASM32_TRACE_CALL_BEGIN("INS_C6");
    _c_setbyte(aasmContext, 0xc6);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, rid));
    XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, aopri2.imm8));
    XASM32_TRACE_CALL_END;
}
static void INS_C7(aasm32_context *aasmContext, lib_u8 rid, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("INS_C7");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0xc7);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, rid));
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri2.imm16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_imm32(aasmContext, aopri2.imm32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void ENTER(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("ENTER");
    if (ARG_I16_I8)
    {
        XASM32_TRACE_BLOCK_BEGIN("ARG_I16_I8");
        _c_setbyte(aasmContext, 0xc8);
        XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri1.imm16));
        XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, aopri2.imm8));
        XASM32_TRACE_BLOCK_END;
    }
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void LEAVE(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("LEAVE");
    if (ARG_NONE)
        _c_setbyte(aasmContext, 0xc9);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void RETF_I16(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("RETF_I16");
    _c_setbyte(aasmContext, 0xca);
    XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri1.imm16));
    XASM32_TRACE_CALL_END;
}
static void RETF_(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("RETF_");
    _c_setbyte(aasmContext, 0xcb);
    XASM32_TRACE_CALL_END;
}
static void INT3(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("INT3");
    _c_setbyte(aasmContext, 0xcc);
    XASM32_TRACE_CALL_END;
}
static void INT_I8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("INT_I8");
    _c_setbyte(aasmContext, 0xcd);
    XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, aopri1.imm8));
    XASM32_TRACE_CALL_END;
}
static void INTO(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("INTO");
    _c_setbyte(aasmContext, 0xcd);
    XASM32_TRACE_CALL_END;
}
static void IRET(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("IRET");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0xcf);
    XASM32_TRACE_CALL_END;
}
static void INS_D0(aasm32_context *aasmContext, lib_u8 rid)
{
    XASM32_TRACE_CALL_BEGIN("INS_DO");
    _c_setbyte(aasmContext, 0xd0);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, rid));
    XASM32_TRACE_CALL_END;
}
static void INS_D1(aasm32_context *aasmContext, lib_u8 rid, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("INS_D1");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0xd1);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, rid));
    XASM32_TRACE_CALL_END;
}
static void INS_D2(aasm32_context *aasmContext, lib_u8 rid)
{
    XASM32_TRACE_CALL_BEGIN("INS_D2");
    _c_setbyte(aasmContext, 0xd2);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, rid));
    XASM32_TRACE_CALL_END;
}
static void INS_D3(aasm32_context *aasmContext, lib_u8 rid, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("INS_D3");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0xd3);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, rid));
    XASM32_TRACE_CALL_END;
}
static void AAM(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("AAM");
    if (ARG_NONE)
    {
        XASM32_TRACE_BLOCK_BEGIN("ARG_NONE");
        _c_setbyte(aasmContext, 0xd4);
        XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, 0x0a));
        XASM32_TRACE_BLOCK_END;
    }
    else if (ARG_I8)
    {
        XASM32_TRACE_BLOCK_BEGIN("ARG_I8");
        _c_setbyte(aasmContext, 0xd4);
        XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, aopri1.imm8));
        XASM32_TRACE_BLOCK_END;
    }
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void AAD(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("AAD");
    if (ARG_NONE)
    {
        XASM32_TRACE_BLOCK_BEGIN("ARG_NONE");
        _c_setbyte(aasmContext, 0xd5);
        XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, 0x0a));
        XASM32_TRACE_BLOCK_END;
    }
    else if (ARG_I8)
    {
        XASM32_TRACE_BLOCK_BEGIN("ARG_I8");
        _c_setbyte(aasmContext, 0xd5);
        XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, aopri1.imm8));
        XASM32_TRACE_BLOCK_END;
    }
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void XLATB(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("XLATB");
    rinfo = &aopri1;
    if (rinfo->flagds)
        rinfo->flagds = 0;
    _c_setbyte(aasmContext, 0xd7);
    if (ARG_DSBXAL8)
    {
        _SetAddressSize(2);
    }
    else if (ARG_DSEBXAL8)
    {
        _SetAddressSize(4);
    }
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void IN_AL_I8(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("IN_AL_I8");
    _c_setbyte(aasmContext, 0xe4);
    XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, aopri2.imm8));
    XASM32_TRACE_CALL_END;
}
static void IN_EAX_I8(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("IN_AL_I8");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0xe5);
    XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, aopri2.imm8));
    XASM32_TRACE_CALL_END;
}
static void OUT_I8_AL(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("OUT_I8_AL");
    _c_setbyte(aasmContext, 0xe6);
    XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, aopri1.imm8));
    XASM32_TRACE_CALL_END;
}
static void OUT_I8_EAX(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("OUT_I8_EAX");
    _c_setbyte(aasmContext, 0xe7);
    XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, aopri1.imm8));
    XASM32_TRACE_CALL_END;
}
static void CALL_REL32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("CALL_REL32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0xe8);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri1.imm16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_imm32(aasmContext, aopri1.imm32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void JMP_REL32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("JMP_REL32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0xe9);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri1.imm16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_imm32(aasmContext, aopri1.imm32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void JMP_PTR16_32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("JMP_PTR16_32");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0xea);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, (lib_u16)aopri1.reip));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_imm32(aasmContext, (lib_u32)aopri1.reip));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri1.rcs));
    XASM32_TRACE_CALL_END;
}
static void IN_AL_DX(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("IN_AL_DX");
    _c_setbyte(aasmContext, 0xec);
    XASM32_TRACE_CALL_END;
}
static void IN_EAX_DX(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("IN_EAX_DX");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0xed);
    XASM32_TRACE_CALL_END;
}
static void OUT_DX_AL(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("OUT_DX_AL");
    _c_setbyte(aasmContext, 0xee);
    XASM32_TRACE_CALL_END;
}
static void OUT_DX_EAX(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("OUT_DX_EAX");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0xef);
    XASM32_TRACE_CALL_END;
}
static void PREFIX_LOCK(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PREFIX_LOCK");
    if (ARG_NONE)
        prefix_lock = 1;
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void PREFIX_REPNZ(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PREFIX_REPNZ");
    if (ARG_NONE)
        prefix_repnz = 1;
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void PREFIX_REPZ(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PREFIX_REPZ");
    if (ARG_NONE)
        prefix_repz = 1;
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void HLT(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("HLT");
    if (ARG_NONE)
        _c_setbyte(aasmContext, 0xf4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void CMC(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("CMC");
    if (ARG_NONE)
        _c_setbyte(aasmContext, 0xf5);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void INS_F6(aasm32_context *aasmContext, lib_u8 rid)
{
    XASM32_TRACE_CALL_BEGIN("INS_F6");
    _c_setbyte(aasmContext, 0xf6);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, rid));
    if (!rid)
        XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, aopri2.imm8));
    XASM32_TRACE_CALL_END;
}
static void INS_F7(aasm32_context *aasmContext, lib_u8 rid, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("INS_F7");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0xf7);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, rid));
    if (!rid)
    {
        XASM32_TRACE_BLOCK_BEGIN("!rid");
        switch (byte)
        {
        case 2:
            XASM32_TRACE_BLOCK_BEGIN("byte(2)");
            XASM32_TRACE_CHECK_RETURN(_c_imm16(aasmContext, aopri2.imm16));
            XASM32_TRACE_BLOCK_END;
            break;
        case 4:
            XASM32_TRACE_BLOCK_BEGIN("byte(4)");
            XASM32_TRACE_CHECK_RETURN(_c_imm32(aasmContext, aopri2.imm32));
            XASM32_TRACE_BLOCK_END;
            break;
        default:
            _ser_;
            break;
        }
        XASM32_TRACE_BLOCK_END;
    }
    XASM32_TRACE_CALL_END;
}
static void CLC(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("CLC");
    if (ARG_NONE)
        _c_setbyte(aasmContext, 0xf8);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void STC(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("STC");
    if (ARG_NONE)
        _c_setbyte(aasmContext, 0xf9);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void CLI(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("CLI");
    if (ARG_NONE)
        _c_setbyte(aasmContext, 0xfa);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void STI(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("STI");
    if (ARG_NONE)
        _c_setbyte(aasmContext, 0xfb);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void CLD(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("CLD");
    if (ARG_NONE)
        _c_setbyte(aasmContext, 0xfc);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void STD(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("STD");
    if (ARG_NONE)
        _c_setbyte(aasmContext, 0xfd);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void INS_FE(aasm32_context *aasmContext, lib_u8 rid)
{
    XASM32_TRACE_CALL_BEGIN("INS_FE");
    _c_setbyte(aasmContext, 0xfe);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, rid));
    XASM32_TRACE_CALL_END;
}
static void INS_FF(aasm32_context *aasmContext, lib_u8 rid, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("INS_FF");
    _SetOperandSize(byte);
    _c_setbyte(aasmContext, 0xff);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, rid));
    XASM32_TRACE_CALL_END;
}
/* concrete extended instructions */
static void INS_0F_00(aasm32_context *aasmContext, lib_u8 rid, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("INS_0F_00");
    _SetOperandSize(byte);
    INS_0F(aasmContext);
    _c_setbyte(aasmContext, 0x00);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, rid));
    XASM32_TRACE_CALL_END;
}
static void INS_0F_01(aasm32_context *aasmContext, lib_u8 rid, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("INS_0F_01");
    _SetOperandSize(byte);
    INS_0F(aasmContext);
    _c_setbyte(aasmContext, 0x01);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, rid));
    XASM32_TRACE_CALL_END;
}
static void LAR_R32_RM32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("LAR_R32_RM32");
    _SetOperandSize(byte);
    INS_0F(aasmContext);
    _c_setbyte(aasmContext, 0x02);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void LSL_R32_RM32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("LSL_R32_RM32");
    _SetOperandSize(byte);
    INS_0F(aasmContext);
    _c_setbyte(aasmContext, 0x03);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void CLTS(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("CLTS");
    INS_0F(aasmContext);
    _c_setbyte(aasmContext, 0x06);
    XASM32_TRACE_CALL_END;
}
static void MOV_R32_CR(aasm32_context *aasmContext, lib_u8 crid)
{
    XASM32_TRACE_CALL_BEGIN("MOV_R32_CR");
    INS_0F(aasmContext);
    _c_setbyte(aasmContext, 0x20);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, crid));
    XASM32_TRACE_CALL_END;
}
static void MOV_R32_DR(aasm32_context *aasmContext, lib_u8 drid)
{
    XASM32_TRACE_CALL_BEGIN("MOV_R32_DR");
    INS_0F(aasmContext);
    _c_setbyte(aasmContext, 0x21);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, drid));
    XASM32_TRACE_CALL_END;
}
static void MOV_CR_R32(aasm32_context *aasmContext, lib_u8 crid)
{
    XASM32_TRACE_CALL_BEGIN("MOV_CR_R32");
    INS_0F(aasmContext);
    _c_setbyte(aasmContext, 0x22);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, crid));
    XASM32_TRACE_CALL_END;
}
static void MOV_DR_R32(aasm32_context *aasmContext, lib_u8 drid)
{
    XASM32_TRACE_CALL_BEGIN("MOV_DR_R32");
    INS_0F(aasmContext);
    _c_setbyte(aasmContext, 0x23);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, drid));
    XASM32_TRACE_CALL_END;
}
static void MOV_R32_TR(aasm32_context *aasmContext, lib_u8 trid)
{
    XASM32_TRACE_CALL_BEGIN("MOV_R32_TR");
    INS_0F(aasmContext);
    _c_setbyte(aasmContext, 0x24);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, trid));
    XASM32_TRACE_CALL_END;
}
static void MOV_TR_R32(aasm32_context *aasmContext, lib_u8 trid)
{
    XASM32_TRACE_CALL_BEGIN("MOV_TR_R32");
    INS_0F(aasmContext);
    _c_setbyte(aasmContext, 0x26);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, trid));
    XASM32_TRACE_CALL_END;
}
static void SETCC_RM8(aasm32_context *aasmContext, lib_u8 opcode)
{
    XASM32_TRACE_CALL_BEGIN("SETCC_RM8");
    if (ARG_RM8)
    {
        INS_0F(aasmContext);
        _c_setbyte(aasmContext, opcode);
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, 0));
    }
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void PUSH_FS(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PUSH_FS");
    INS_0F(aasmContext);
    _c_setbyte(aasmContext, 0xa0);
    XASM32_TRACE_CALL_END;
}
static void POP_FS(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("POP_FS");
    INS_0F(aasmContext);
    _c_setbyte(aasmContext, 0xa1);
    XASM32_TRACE_CALL_END;
}
static void BT_RM32_R32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("BT_RM32_R32");
    _SetOperandSize(byte);
    INS_0F(aasmContext);
    _c_setbyte(aasmContext, 0xa3);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg16));
        break;
    case 4:
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg32));
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void SHLD_RM32_R32_I8(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("SHLD_RM32_R32_I8");
    _SetOperandSize(byte);
    INS_0F(aasmContext);
    _c_setbyte(aasmContext, 0xa4);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg16));
        break;
    case 4:
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg32));
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, aopri3.imm8));
    XASM32_TRACE_CALL_END;
}
static void SHLD_RM32_R32_CL(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("SHLD_RM32_R32_CL");
    _SetOperandSize(byte);
    INS_0F(aasmContext);
    _c_setbyte(aasmContext, 0xa5);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg16));
        break;
    case 4:
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg32));
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void PUSH_GS(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PUSH_GS");
    INS_0F(aasmContext);
    _c_setbyte(aasmContext, 0xa8);
    XASM32_TRACE_CALL_END;
}
static void POP_GS(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("POP_GS");
    INS_0F(aasmContext);
    _c_setbyte(aasmContext, 0xa9);
    XASM32_TRACE_CALL_END;
}
static void BTS_RM32_R32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("BTS_RM32_R32");
    _SetOperandSize(byte);
    INS_0F(aasmContext);
    _c_setbyte(aasmContext, 0xab);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg16));
        break;
    case 4:
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg32));
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void SHRD_RM32_R32_I8(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("SHRD_RM32_R32_I8");
    _SetOperandSize(byte);
    INS_0F(aasmContext);
    _c_setbyte(aasmContext, 0xac);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg16));
        break;
    case 4:
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg32));
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, aopri3.imm8));
    XASM32_TRACE_CALL_END;
}
static void SHRD_RM32_R32_CL(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("SHRD_RM32_R32_CL");
    _SetOperandSize(byte);
    INS_0F(aasmContext);
    _c_setbyte(aasmContext, 0xad);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg16));
        break;
    case 4:
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg32));
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void IMUL_R32_RM32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("IMUL_R32_RM32");
    _SetOperandSize(byte);
    INS_0F(aasmContext);
    _c_setbyte(aasmContext, 0xab);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg16));
        break;
    case 4:
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg32));
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void LSS_R32_M16_32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("LSS_R32_M16_32");
    _SetOperandSize(byte);
    INS_0F(aasmContext);
    _c_setbyte(aasmContext, 0xb2);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void BTR_RM32_R32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("BTR_RM32_R32");
    _SetOperandSize(byte);
    INS_0F(aasmContext);
    _c_setbyte(aasmContext, 0xb3);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg16));
        break;
    case 4:
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg32));
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void LFS_R32_M16_32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("LFS_R32_M16_32");
    _SetOperandSize(byte);
    INS_0F(aasmContext);
    _c_setbyte(aasmContext, 0xb4);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void LGS_R32_M16_32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("LGS_R32_M16_32");
    _SetOperandSize(byte);
    INS_0F(aasmContext);
    _c_setbyte(aasmContext, 0xb5);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void MOVZX_R32_RM8(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("MOVZX_R32_RM8");
    _SetOperandSize(byte);
    INS_0F(aasmContext);
    _c_setbyte(aasmContext, 0xb6);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void MOVZX_R32_RM16(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOVZX_R32_RM16");
    INS_0F(aasmContext);
    _c_setbyte(aasmContext, 0xb7);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg32));
    XASM32_TRACE_CALL_END;
}
static void INS_0F_BA(aasm32_context *aasmContext, lib_u8 rid, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("INS_0F_BA");
    _SetOperandSize(byte);
    INS_0F(aasmContext);
    _c_setbyte(aasmContext, 0xba);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, rid));
    XASM32_TRACE_CHECK_RETURN(_c_imm8(aasmContext, aopri2.imm8));
    XASM32_TRACE_CALL_END;
}
static void BTC_RM32_R32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("BTC_RM32_R32");
    _SetOperandSize(byte);
    INS_0F(aasmContext);
    _c_setbyte(aasmContext, 0xbb);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg16));
        break;
    case 4:
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri1, aopri2.reg32));
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void BSF_R32_RM32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("BSF_R32_RM32");
    _SetOperandSize(byte);
    INS_0F(aasmContext);
    _c_setbyte(aasmContext, 0xbc);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void BSR_R32_RM32(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("BSR_R32_RM32");
    _SetOperandSize(byte);
    INS_0F(aasmContext);
    _c_setbyte(aasmContext, 0xbd);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void MOVSX_R32_RM8(aasm32_context *aasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("MOVSX_R32_RM8");
    _SetOperandSize(byte);
    INS_0F(aasmContext);
    _c_setbyte(aasmContext, 0xbe);
    switch (byte)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("byte(2)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg16));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("byte(4)");
        XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg32));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        _ser_;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void MOVSX_R32_RM16(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOVSX_R32_RM16");
    INS_0F(aasmContext);
    _c_setbyte(aasmContext, 0xbf);
    XASM32_TRACE_CHECK_RETURN(_c_modrm(aasmContext, aopri2, aopri1.reg32));
    XASM32_TRACE_CALL_END;
}

/* abstract instructions */
static void PUSH(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PUSH");
    if (ARG_ES)
        PUSH_ES(aasmContext);
    else if (ARG_CS)
        PUSH_CS(aasmContext);
    else if (ARG_SS)
        PUSH_SS(aasmContext);
    else if (ARG_DS)
        PUSH_DS(aasmContext);
    else if (ARG_FS)
        PUSH_FS(aasmContext);
    else if (ARG_GS)
        PUSH_GS(aasmContext);
    else if (ARG_AX)
        PUSH_EAX(aasmContext, 2);
    else if (ARG_CX)
        PUSH_ECX(aasmContext, 2);
    else if (ARG_DX)
        PUSH_EDX(aasmContext, 2);
    else if (ARG_BX)
        PUSH_EBX(aasmContext, 2);
    else if (ARG_SP)
        PUSH_ESP(aasmContext, 2);
    else if (ARG_BP)
        PUSH_EBP(aasmContext, 2);
    else if (ARG_SI)
        PUSH_ESI(aasmContext, 2);
    else if (ARG_DI)
        PUSH_EDI(aasmContext, 2);
    else if (ARG_EAX)
        PUSH_EAX(aasmContext, 4);
    else if (ARG_ECX)
        PUSH_ECX(aasmContext, 4);
    else if (ARG_EDX)
        PUSH_EDX(aasmContext, 4);
    else if (ARG_EBX)
        PUSH_EBX(aasmContext, 4);
    else if (ARG_ESP)
        PUSH_ESP(aasmContext, 4);
    else if (ARG_EBP)
        PUSH_EBP(aasmContext, 4);
    else if (ARG_ESI)
        PUSH_ESI(aasmContext, 4);
    else if (ARG_EDI)
        PUSH_EDI(aasmContext, 4);
    else if (ARG_RM16)
        INS_FF(aasmContext, 0x06, 2);
    else if (ARG_RM32)
        INS_FF(aasmContext, 0x06, 4);
    else if (ARG_I8)
        PUSH_I8(aasmContext);
    else if (ARG_I16)
        PUSH_I32(aasmContext, 2);
    else if (ARG_I32)
        PUSH_I32(aasmContext, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void POP(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("POP");
    if (ARG_ES)
        POP_ES(aasmContext);
    else if (ARG_CS)
        POP_CS(aasmContext);
    else if (ARG_SS)
        POP_SS(aasmContext);
    else if (ARG_DS)
        POP_DS(aasmContext);
    else if (ARG_FS)
        POP_FS(aasmContext);
    else if (ARG_GS)
        POP_GS(aasmContext);
    else if (ARG_AX)
        POP_EAX(aasmContext, 2);
    else if (ARG_CX)
        POP_ECX(aasmContext, 2);
    else if (ARG_DX)
        POP_EDX(aasmContext, 2);
    else if (ARG_BX)
        POP_EBX(aasmContext, 2);
    else if (ARG_SP)
        POP_ESP(aasmContext, 2);
    else if (ARG_BP)
        POP_EBP(aasmContext, 2);
    else if (ARG_SI)
        POP_ESI(aasmContext, 2);
    else if (ARG_DI)
        POP_EDI(aasmContext, 2);
    else if (ARG_EAX)
        POP_EAX(aasmContext, 4);
    else if (ARG_ECX)
        POP_ECX(aasmContext, 4);
    else if (ARG_EDX)
        POP_EDX(aasmContext, 4);
    else if (ARG_EBX)
        POP_EBX(aasmContext, 4);
    else if (ARG_ESP)
        POP_ESP(aasmContext, 4);
    else if (ARG_EBP)
        POP_EBP(aasmContext, 4);
    else if (ARG_ESI)
        POP_ESI(aasmContext, 4);
    else if (ARG_EDI)
        POP_EDI(aasmContext, 4);
    else if (ARG_RM16)
        INS_8F(aasmContext, 0x00, 2);
    else if (ARG_RM32)
        INS_8F(aasmContext, 0x00, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void ADD(aasm32_context *aasmContext)
{
    lib_u8 rid = 0x00;
    XASM32_TRACE_CALL_BEGIN("ADD");
    if (ARG_AL_I8)
        ADD_AL_I8(aasmContext);
    else if (ARG_AX_I16)
        ADD_EAX_I32(aasmContext, 2);
    else if (ARG_EAX_I32)
        ADD_EAX_I32(aasmContext, 4);
    else if (ARG_R8_RM8)
        ADD_R8_RM8(aasmContext);
    else if (ARG_R16_RM16)
        ADD_R32_RM32(aasmContext, 2);
    else if (ARG_R32_RM32)
        ADD_R32_RM32(aasmContext, 4);
    else if (ARG_RM8_I8)
        INS_80(aasmContext, rid);
    else if (ARG_RM16_I8)
    {
        if (!aopri2.immn)
            if (aopri2.imm8 > 0x7f)
            {
                aopri2.type = XASM32_OPERAND_I16;
                aopri2.imm16 = aopri2.imm8;
                INS_81(aasmContext, rid, 2);
            }
            else
                INS_83(aasmContext, rid, 2);
        else if (aopri2.imm8 > 0x7f)
            INS_83(aasmContext, rid, 2);
        else
        {
            aopri2.type = XASM32_OPERAND_I16;
            aopri2.imm16 = 0xff00 | aopri2.imm8;
            INS_81(aasmContext, rid, 2);
        }
    }
    else if (ARG_RM32_I8)
    {
        if (!aopri2.immn)
            if (aopri2.imm8 > 0x7f)
            {
                aopri2.type = XASM32_OPERAND_I32;
                aopri2.imm32 = aopri2.imm8;
                INS_81(aasmContext, rid, 4);
            }
            else
                INS_83(aasmContext, rid, 4);
        else if (aopri2.imm8 > 0x7f)
            INS_83(aasmContext, rid, 4);
        else
        {
            aopri2.type = XASM32_OPERAND_I32;
            aopri2.imm32 = 0xffffff00 | aopri2.imm8;
            INS_81(aasmContext, rid, 4);
        }
    }
    else if (ARG_RM16_I16)
        INS_81(aasmContext, rid, 2);
    else if (ARG_RM32_I32)
        INS_81(aasmContext, rid, 4);
    else if (ARG_RM8_R8)
        ADD_RM8_R8(aasmContext);
    else if (ARG_RM16_R16)
        ADD_RM32_R32(aasmContext, 2);
    else if (ARG_RM32_R32)
        ADD_RM32_R32(aasmContext, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void OR(aasm32_context *aasmContext)
{
    lib_u8 rid = 0x01;
    XASM32_TRACE_CALL_BEGIN("OR");
    if (ARG_AL_I8)
        OR_AL_I8(aasmContext);
    else if (ARG_AX_I16)
        OR_EAX_I32(aasmContext, 2);
    else if (ARG_EAX_I32)
        OR_EAX_I32(aasmContext, 4);
    else if (ARG_R8_RM8)
        OR_R8_RM8(aasmContext);
    else if (ARG_R16_RM16)
        OR_R32_RM32(aasmContext, 2);
    else if (ARG_R32_RM32)
        OR_R32_RM32(aasmContext, 4);
    else if (ARG_RM8_I8)
        INS_80(aasmContext, rid);
    else if (ARG_RM16_I8)
    {
        if (!aopri2.immn)
            if (aopri2.imm8 > 0x7f)
            {
                aopri2.type = XASM32_OPERAND_I16;
                aopri2.imm16 = aopri2.imm8;
                INS_81(aasmContext, rid, 2);
            }
            else
                INS_83(aasmContext, rid, 2);
        else if (aopri2.imm8 > 0x7f)
            INS_83(aasmContext, rid, 2);
        else
        {
            aopri2.type = XASM32_OPERAND_I16;
            aopri2.imm16 = 0xff00 | aopri2.imm8;
            INS_81(aasmContext, rid, 2);
        }
    }
    else if (ARG_RM32_I8)
    {
        if (!aopri2.immn)
            if (aopri2.imm8 > 0x7f)
            {
                aopri2.type = XASM32_OPERAND_I32;
                aopri2.imm32 = aopri2.imm8;
                INS_81(aasmContext, rid, 4);
            }
            else
                INS_83(aasmContext, rid, 4);
        else if (aopri2.imm8 > 0x7f)
            INS_83(aasmContext, rid, 4);
        else
        {
            aopri2.type = XASM32_OPERAND_I32;
            aopri2.imm32 = 0xffffff00 | aopri2.imm8;
            INS_81(aasmContext, rid, 4);
        }
    }
    else if (ARG_RM16_I16)
        INS_81(aasmContext, rid, 2);
    else if (ARG_RM32_I32)
        INS_81(aasmContext, rid, 4);
    else if (ARG_RM8_R8)
        OR_RM8_R8(aasmContext);
    else if (ARG_RM16_R16)
        OR_RM32_R32(aasmContext, 2);
    else if (ARG_RM32_R32)
        OR_RM32_R32(aasmContext, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void ADC(aasm32_context *aasmContext)
{
    lib_u8 rid = 0x02;
    XASM32_TRACE_CALL_BEGIN("ADC");
    if (ARG_AL_I8)
        ADC_AL_I8(aasmContext);
    else if (ARG_AX_I16)
        ADC_EAX_I32(aasmContext, 2);
    else if (ARG_EAX_I32)
        ADC_EAX_I32(aasmContext, 4);
    else if (ARG_R8_RM8)
        ADC_R8_RM8(aasmContext);
    else if (ARG_R16_RM16)
        ADC_R32_RM32(aasmContext, 2);
    else if (ARG_R32_RM32)
        ADC_R32_RM32(aasmContext, 4);
    else if (ARG_RM8_I8)
        INS_80(aasmContext, rid);
    else if (ARG_RM16_I8)
    {
        if (!aopri2.immn)
            if (aopri2.imm8 > 0x7f)
            {
                aopri2.type = XASM32_OPERAND_I16;
                aopri2.imm16 = aopri2.imm8;
                INS_81(aasmContext, rid, 2);
            }
            else
                INS_83(aasmContext, rid, 2);
        else if (aopri2.imm8 > 0x7f)
            INS_83(aasmContext, rid, 2);
        else
        {
            aopri2.type = XASM32_OPERAND_I16;
            aopri2.imm16 = 0xff00 | aopri2.imm8;
            INS_81(aasmContext, rid, 2);
        }
    }
    else if (ARG_RM32_I8)
    {
        if (!aopri2.immn)
            if (aopri2.imm8 > 0x7f)
            {
                aopri2.type = XASM32_OPERAND_I32;
                aopri2.imm32 = aopri2.imm8;
                INS_81(aasmContext, rid, 4);
            }
            else
                INS_83(aasmContext, rid, 4);
        else if (aopri2.imm8 > 0x7f)
            INS_83(aasmContext, rid, 4);
        else
        {
            aopri2.type = XASM32_OPERAND_I32;
            aopri2.imm32 = 0xffffff00 | aopri2.imm8;
            INS_81(aasmContext, rid, 4);
        }
    }
    else if (ARG_RM16_I16)
        INS_81(aasmContext, rid, 2);
    else if (ARG_RM32_I32)
        INS_81(aasmContext, rid, 4);
    else if (ARG_RM8_R8)
        ADC_RM8_R8(aasmContext);
    else if (ARG_RM16_R16)
        ADC_RM32_R32(aasmContext, 2);
    else if (ARG_RM32_R32)
        ADC_RM32_R32(aasmContext, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void SBB(aasm32_context *aasmContext)
{
    lib_u8 rid = 0x03;
    XASM32_TRACE_CALL_BEGIN("SBB");
    if (ARG_AL_I8)
        SBB_AL_I8(aasmContext);
    else if (ARG_AX_I16)
        SBB_EAX_I32(aasmContext, 2);
    else if (ARG_EAX_I32)
        SBB_EAX_I32(aasmContext, 4);
    else if (ARG_R8_RM8)
        SBB_R8_RM8(aasmContext);
    else if (ARG_R16_RM16)
        SBB_R32_RM32(aasmContext, 2);
    else if (ARG_R32_RM32)
        SBB_R32_RM32(aasmContext, 4);
    else if (ARG_RM8_I8)
        INS_80(aasmContext, rid);
    else if (ARG_RM16_I8)
    {
        if (!aopri2.immn)
            if (aopri2.imm8 > 0x7f)
            {
                aopri2.type = XASM32_OPERAND_I16;
                aopri2.imm16 = aopri2.imm8;
                INS_81(aasmContext, rid, 2);
            }
            else
                INS_83(aasmContext, rid, 2);
        else if (aopri2.imm8 > 0x7f)
            INS_83(aasmContext, rid, 2);
        else
        {
            aopri2.type = XASM32_OPERAND_I16;
            aopri2.imm16 = 0xff00 | aopri2.imm8;
            INS_81(aasmContext, rid, 2);
        }
    }
    else if (ARG_RM32_I8)
    {
        if (!aopri2.immn)
            if (aopri2.imm8 > 0x7f)
            {
                aopri2.type = XASM32_OPERAND_I32;
                aopri2.imm32 = aopri2.imm8;
                INS_81(aasmContext, rid, 4);
            }
            else
                INS_83(aasmContext, rid, 4);
        else if (aopri2.imm8 > 0x7f)
            INS_83(aasmContext, rid, 4);
        else
        {
            aopri2.type = XASM32_OPERAND_I32;
            aopri2.imm32 = 0xffffff00 | aopri2.imm8;
            INS_81(aasmContext, rid, 4);
        }
    }
    else if (ARG_RM16_I16)
        INS_81(aasmContext, rid, 2);
    else if (ARG_RM32_I32)
        INS_81(aasmContext, rid, 4);
    else if (ARG_RM8_R8)
        SBB_RM8_R8(aasmContext);
    else if (ARG_RM16_R16)
        SBB_RM32_R32(aasmContext, 2);
    else if (ARG_RM32_R32)
        SBB_RM32_R32(aasmContext, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void AND(aasm32_context *aasmContext)
{
    lib_u8 rid = 0x04;
    XASM32_TRACE_CALL_BEGIN("AND");
    if (ARG_AL_I8)
        AND_AL_I8(aasmContext);
    else if (ARG_AX_I16)
        AND_EAX_I32(aasmContext, 2);
    else if (ARG_EAX_I32)
        AND_EAX_I32(aasmContext, 4);
    else if (ARG_R8_RM8)
        AND_R8_RM8(aasmContext);
    else if (ARG_R16_RM16)
        AND_R32_RM32(aasmContext, 2);
    else if (ARG_R32_RM32)
        AND_R32_RM32(aasmContext, 4);
    else if (ARG_RM8_I8)
        INS_80(aasmContext, rid);
    else if (ARG_RM16_I8)
    {
        if (!aopri2.immn)
            if (aopri2.imm8 > 0x7f)
            {
                aopri2.type = XASM32_OPERAND_I16;
                aopri2.imm16 = aopri2.imm8;
                INS_81(aasmContext, rid, 2);
            }
            else
                INS_83(aasmContext, rid, 2);
        else if (aopri2.imm8 > 0x7f)
            INS_83(aasmContext, rid, 2);
        else
        {
            aopri2.type = XASM32_OPERAND_I16;
            aopri2.imm16 = 0xff00 | aopri2.imm8;
            INS_81(aasmContext, rid, 2);
        }
    }
    else if (ARG_RM32_I8)
    {
        if (!aopri2.immn)
            if (aopri2.imm8 > 0x7f)
            {
                aopri2.type = XASM32_OPERAND_I32;
                aopri2.imm32 = aopri2.imm8;
                INS_81(aasmContext, rid, 4);
            }
            else
                INS_83(aasmContext, rid, 4);
        else if (aopri2.imm8 > 0x7f)
            INS_83(aasmContext, rid, 4);
        else
        {
            aopri2.type = XASM32_OPERAND_I32;
            aopri2.imm32 = 0xffffff00 | aopri2.imm8;
            INS_81(aasmContext, rid, 4);
        }
    }
    else if (ARG_RM16_I16)
        INS_81(aasmContext, rid, 2);
    else if (ARG_RM32_I32)
        INS_81(aasmContext, rid, 4);
    else if (ARG_RM8_R8)
        AND_RM8_R8(aasmContext);
    else if (ARG_RM16_R16)
        AND_RM32_R32(aasmContext, 2);
    else if (ARG_RM32_R32)
        AND_RM32_R32(aasmContext, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void SUB(aasm32_context *aasmContext)
{
    lib_u8 rid = 0x05;
    XASM32_TRACE_CALL_BEGIN("SUB");
    if (ARG_AL_I8)
        SUB_AL_I8(aasmContext);
    else if (ARG_AX_I16)
        SUB_EAX_I32(aasmContext, 2);
    else if (ARG_EAX_I32)
        SUB_EAX_I32(aasmContext, 4);
    else if (ARG_R8_RM8)
        SUB_R8_RM8(aasmContext);
    else if (ARG_R16_RM16)
        SUB_R32_RM32(aasmContext, 2);
    else if (ARG_R32_RM32)
        SUB_R32_RM32(aasmContext, 4);
    else if (ARG_RM8_I8)
        INS_80(aasmContext, rid);
    else if (ARG_RM16_I8)
    {
        if (!aopri2.immn)
            if (aopri2.imm8 > 0x7f)
            {
                aopri2.type = XASM32_OPERAND_I16;
                aopri2.imm16 = aopri2.imm8;
                INS_81(aasmContext, rid, 2);
            }
            else
                INS_83(aasmContext, rid, 2);
        else if (aopri2.imm8 > 0x7f)
            INS_83(aasmContext, rid, 2);
        else
        {
            aopri2.type = XASM32_OPERAND_I16;
            aopri2.imm16 = 0xff00 | aopri2.imm8;
            INS_81(aasmContext, rid, 2);
        }
    }
    else if (ARG_RM32_I8)
    {
        if (!aopri2.immn)
            if (aopri2.imm8 > 0x7f)
            {
                aopri2.type = XASM32_OPERAND_I32;
                aopri2.imm32 = aopri2.imm8;
                INS_81(aasmContext, rid, 4);
            }
            else
                INS_83(aasmContext, rid, 4);
        else if (aopri2.imm8 > 0x7f)
            INS_83(aasmContext, rid, 4);
        else
        {
            aopri2.type = XASM32_OPERAND_I32;
            aopri2.imm32 = 0xffffff00 | aopri2.imm8;
            INS_81(aasmContext, rid, 4);
        }
    }
    else if (ARG_RM16_I16)
        INS_81(aasmContext, rid, 2);
    else if (ARG_RM32_I32)
        INS_81(aasmContext, rid, 4);
    else if (ARG_RM8_R8)
        SUB_RM8_R8(aasmContext);
    else if (ARG_RM16_R16)
        SUB_RM32_R32(aasmContext, 2);
    else if (ARG_RM32_R32)
        SUB_RM32_R32(aasmContext, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void XOR(aasm32_context *aasmContext)
{
    lib_u8 rid = 0x06;
    XASM32_TRACE_CALL_BEGIN("XOR");
    if (ARG_AL_I8)
        XOR_AL_I8(aasmContext);
    else if (ARG_AX_I16)
        XOR_EAX_I32(aasmContext, 2);
    else if (ARG_EAX_I32)
        XOR_EAX_I32(aasmContext, 4);
    else if (ARG_R8_RM8)
        XOR_R8_RM8(aasmContext);
    else if (ARG_R16_RM16)
        XOR_R32_RM32(aasmContext, 2);
    else if (ARG_R32_RM32)
        XOR_R32_RM32(aasmContext, 4);
    else if (ARG_RM8_I8)
        INS_80(aasmContext, rid);
    else if (ARG_RM16_I8)
    {
        if (!aopri2.immn)
            if (aopri2.imm8 > 0x7f)
            {
                aopri2.type = XASM32_OPERAND_I16;
                aopri2.imm16 = aopri2.imm8;
                INS_81(aasmContext, rid, 2);
            }
            else
                INS_83(aasmContext, rid, 2);
        else if (aopri2.imm8 > 0x7f)
            INS_83(aasmContext, rid, 2);
        else
        {
            aopri2.type = XASM32_OPERAND_I16;
            aopri2.imm16 = 0xff00 | aopri2.imm8;
            INS_81(aasmContext, rid, 2);
        }
    }
    else if (ARG_RM32_I8)
    {
        if (!aopri2.immn)
            if (aopri2.imm8 > 0x7f)
            {
                aopri2.type = XASM32_OPERAND_I32;
                aopri2.imm32 = aopri2.imm8;
                INS_81(aasmContext, rid, 4);
            }
            else
                INS_83(aasmContext, rid, 4);
        else if (aopri2.imm8 > 0x7f)
            INS_83(aasmContext, rid, 4);
        else
        {
            aopri2.type = XASM32_OPERAND_I32;
            aopri2.imm32 = 0xffffff00 | aopri2.imm8;
            INS_81(aasmContext, rid, 4);
        }
    }
    else if (ARG_RM16_I16)
        INS_81(aasmContext, rid, 2);
    else if (ARG_RM32_I32)
        INS_81(aasmContext, rid, 4);
    else if (ARG_RM8_R8)
        XOR_RM8_R8(aasmContext);
    else if (ARG_RM16_R16)
        XOR_RM32_R32(aasmContext, 2);
    else if (ARG_RM32_R32)
        XOR_RM32_R32(aasmContext, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void CMP(aasm32_context *aasmContext)
{
    lib_u8 rid = 0x07;
    XASM32_TRACE_CALL_BEGIN("CMP");
    if (ARG_AL_I8)
        CMP_AL_I8(aasmContext);
    else if (ARG_AX_I16)
        CMP_EAX_I32(aasmContext, 2);
    else if (ARG_EAX_I32)
        CMP_EAX_I32(aasmContext, 4);
    else if (ARG_R8_RM8)
        CMP_R8_RM8(aasmContext);
    else if (ARG_R16_RM16)
        CMP_R32_RM32(aasmContext, 2);
    else if (ARG_R32_RM32)
        CMP_R32_RM32(aasmContext, 4);
    else if (ARG_RM8_I8)
        INS_80(aasmContext, rid);
    else if (ARG_RM16_I8)
    {
        if (!aopri2.immn)
            if (aopri2.imm8 > 0x7f)
            {
                aopri2.type = XASM32_OPERAND_I16;
                aopri2.imm16 = aopri2.imm8;
                INS_81(aasmContext, rid, 2);
            }
            else
                INS_83(aasmContext, rid, 2);
        else if (aopri2.imm8 > 0x7f)
            INS_83(aasmContext, rid, 2);
        else
        {
            aopri2.type = XASM32_OPERAND_I16;
            aopri2.imm16 = 0xff00 | aopri2.imm8;
            INS_81(aasmContext, rid, 2);
        }
    }
    else if (ARG_RM32_I8)
    {
        if (!aopri2.immn)
            if (aopri2.imm8 > 0x7f)
            {
                aopri2.type = XASM32_OPERAND_I32;
                aopri2.imm32 = aopri2.imm8;
                INS_81(aasmContext, rid, 4);
            }
            else
                INS_83(aasmContext, rid, 4);
        else if (aopri2.imm8 > 0x7f)
            INS_83(aasmContext, rid, 4);
        else
        {
            aopri2.type = XASM32_OPERAND_I32;
            aopri2.imm32 = 0xffffff00 | aopri2.imm8;
            INS_81(aasmContext, rid, 4);
        }
    }
    else if (ARG_RM16_I16)
        INS_81(aasmContext, rid, 2);
    else if (ARG_RM32_I32)
        INS_81(aasmContext, rid, 4);
    else if (ARG_RM8_R8)
        CMP_RM8_R8(aasmContext);
    else if (ARG_RM16_R16)
        CMP_RM32_R32(aasmContext, 2);
    else if (ARG_RM32_R32)
        CMP_RM32_R32(aasmContext, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void INC(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("INC");
    if (ARG_AX)
        INC_EAX(aasmContext, 2);
    else if (ARG_CX)
        INC_ECX(aasmContext, 2);
    else if (ARG_DX)
        INC_EDX(aasmContext, 2);
    else if (ARG_BX)
        INC_EBX(aasmContext, 2);
    else if (ARG_SP)
        INC_ESP(aasmContext, 2);
    else if (ARG_BP)
        INC_EBP(aasmContext, 2);
    else if (ARG_SI)
        INC_ESI(aasmContext, 2);
    else if (ARG_DI)
        INC_EDI(aasmContext, 2);
    else if (ARG_EAX)
        INC_EAX(aasmContext, 4);
    else if (ARG_ECX)
        INC_ECX(aasmContext, 4);
    else if (ARG_EDX)
        INC_EDX(aasmContext, 4);
    else if (ARG_EBX)
        INC_EBX(aasmContext, 4);
    else if (ARG_ESP)
        INC_ESP(aasmContext, 4);
    else if (ARG_EBP)
        INC_EBP(aasmContext, 4);
    else if (ARG_ESI)
        INC_ESI(aasmContext, 4);
    else if (ARG_EDI)
        INC_EDI(aasmContext, 4);
    else if (ARG_RM8s)
        INS_FE(aasmContext, 0x00);
    else if (ARG_RM16s)
        INS_FF(aasmContext, 0x00, 2);
    else if (ARG_RM32s)
        INS_FF(aasmContext, 0x00, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void DEC(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("DEC");
    if (ARG_AX)
        DEC_EAX(aasmContext, 2);
    else if (ARG_CX)
        DEC_ECX(aasmContext, 2);
    else if (ARG_DX)
        DEC_EDX(aasmContext, 2);
    else if (ARG_BX)
        DEC_EBX(aasmContext, 2);
    else if (ARG_SP)
        DEC_ESP(aasmContext, 2);
    else if (ARG_BP)
        DEC_EBP(aasmContext, 2);
    else if (ARG_SI)
        DEC_ESI(aasmContext, 2);
    else if (ARG_DI)
        DEC_EDI(aasmContext, 2);
    else if (ARG_EAX)
        DEC_EAX(aasmContext, 4);
    else if (ARG_ECX)
        DEC_ECX(aasmContext, 4);
    else if (ARG_EDX)
        DEC_EDX(aasmContext, 4);
    else if (ARG_EBX)
        DEC_EBX(aasmContext, 4);
    else if (ARG_ESP)
        DEC_ESP(aasmContext, 4);
    else if (ARG_EBP)
        DEC_EBP(aasmContext, 4);
    else if (ARG_ESI)
        DEC_ESI(aasmContext, 4);
    else if (ARG_EDI)
        DEC_EDI(aasmContext, 4);
    else if (ARG_RM8s)
        INS_FE(aasmContext, 0x01);
    else if (ARG_RM16s)
        INS_FF(aasmContext, 0x01, 2);
    else if (ARG_RM32s)
        INS_FF(aasmContext, 0x01, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void BOUND(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("BOUND");
    if (ARG_R16_M16)
        BOUND_R32_M32_32(aasmContext, 2);
    else if (ARG_R32_M32)
        BOUND_R32_M32_32(aasmContext, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void IMUL(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("IMUL");
    if (ARG_RM8s)
        INS_F6(aasmContext, 0x05);
    else if (ARG_RM16s)
        INS_F7(aasmContext, 0x05, 2);
    else if (ARG_RM32s)
        INS_F7(aasmContext, 0x05, 4);
    else if (ARG_R16_RM16)
        IMUL_R32_RM32(aasmContext, 2);
    else if (ARG_R32_RM32)
        IMUL_R32_RM32(aasmContext, 4);
    else if (ARG_R16_RM16_I8)
        IMUL_R32_RM32_I8(aasmContext, 2);
    else if (ARG_R32_RM32_I8)
        IMUL_R32_RM32_I8(aasmContext, 4);
    else if (ARG_R16_RM16_I16)
        IMUL_R32_RM32_I32(aasmContext, 2);
    else if (ARG_R32_RM32_I32)
        IMUL_R32_RM32_I32(aasmContext, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void INS(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("INS");
    if (ARG_ESDI8s_DX || ARG_ESEDI8s_DX)
        INSB(aasmContext);
    else if (ARG_ESDI16s_DX || ARG_ESEDI16s_DX)
        INSW(aasmContext, 2);
    else if (ARG_ESDI32s_DX || ARG_ESEDI32s_DX)
        INSW(aasmContext, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void OUTS(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("OUTS");
    if (ARG_DX_DSSI8s || ARG_DX_DSESI8s)
        OUTSB(aasmContext);
    else if (ARG_DX_DSSI16s || ARG_DX_DSESI16s)
        OUTSW(aasmContext, 2);
    else if (ARG_DX_DSSI32s || ARG_DX_DSESI32s)
        OUTSW(aasmContext, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void JCC_REL(aasm32_context *aasmContext, lib_u8 opcode)
{
    XASM32_TRACE_CALL_BEGIN("JCC_REL");
    if (ARG_PNONE_I8s || ARG_SHORT_I8s)
    {
        _c_setbyte(aasmContext, opcode);
        _c_imm8(aasmContext, aopri1.imm8);
    }
    else if (ARG_PNONE_I16s || ARG_NEAR_I16s)
    {
        _SetOperandSize(2);
        INS_0F(aasmContext);
        _c_setbyte(aasmContext, opcode + 0x10);
        _c_imm16(aasmContext, aopri1.imm16);
    }
    else if (ARG_PNONE_I32s || ARG_NEAR_I32s)
    {
        _SetOperandSize(4);
        INS_0F(aasmContext);
        _c_setbyte(aasmContext, opcode + 0x10);
        _c_imm16(aasmContext, aopri1.imm32);
    }
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void TEST(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("TEST");
    if (ARG_AL_I8)
        TEST_AL_I8(aasmContext);
    else if (ARG_AX_I16)
        TEST_EAX_I32(aasmContext, 2);
    else if (ARG_EAX_I32)
        TEST_EAX_I32(aasmContext, 4);
    else if (ARG_RM8_R8)
        TEST_RM8_R8(aasmContext);
    else if (ARG_RM16_R16)
        TEST_RM32_R32(aasmContext, 2);
    else if (ARG_RM32_R32)
        TEST_RM32_R32(aasmContext, 4);
    else if (ARG_RM8_I8)
        INS_F6(aasmContext, 0x00);
    else if (ARG_RM16_I16)
        INS_F7(aasmContext, 0x00, 2);
    else if (ARG_RM32_I32)
        INS_F7(aasmContext, 0x00, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void XCHG(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("XCHG");
    if (ARG_AX_AX)
        XCHG_EAX_EAX(aasmContext, 2);
    else if (ARG_CX_AX)
        XCHG_ECX_EAX(aasmContext, 2);
    else if (ARG_DX_AX)
        XCHG_EDX_EAX(aasmContext, 2);
    else if (ARG_BX_AX)
        XCHG_EBX_EAX(aasmContext, 2);
    else if (ARG_SP_AX)
        XCHG_ESP_EAX(aasmContext, 2);
    else if (ARG_BP_AX)
        XCHG_EBP_EAX(aasmContext, 2);
    else if (ARG_SI_AX)
        XCHG_ESI_EAX(aasmContext, 2);
    else if (ARG_DI_AX)
        XCHG_EDI_EAX(aasmContext, 2);
    else if (ARG_EAX_EAX)
        XCHG_EAX_EAX(aasmContext, 4);
    else if (ARG_ECX_EAX)
        XCHG_ECX_EAX(aasmContext, 4);
    else if (ARG_EDX_EAX)
        XCHG_EDX_EAX(aasmContext, 4);
    else if (ARG_EBX_EAX)
        XCHG_EBX_EAX(aasmContext, 4);
    else if (ARG_ESP_EAX)
        XCHG_ESP_EAX(aasmContext, 4);
    else if (ARG_EBP_EAX)
        XCHG_EBP_EAX(aasmContext, 4);
    else if (ARG_ESI_EAX)
        XCHG_ESI_EAX(aasmContext, 4);
    else if (ARG_EDI_EAX)
        XCHG_EDI_EAX(aasmContext, 4);
    else if (ARG_RM8_R8)
        XCHG_RM8_R8(aasmContext);
    else if (ARG_RM16_R16)
        XCHG_RM32_R32(aasmContext, 2);
    else if (ARG_RM32_R32)
        XCHG_RM32_R32(aasmContext, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void MOV(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV");
    if (ARG_AL_I8)
        MOV_AL_I8(aasmContext);
    else if (ARG_CL_I8)
        MOV_CL_I8(aasmContext);
    else if (ARG_DL_I8)
        MOV_DL_I8(aasmContext);
    else if (ARG_BL_I8)
        MOV_BL_I8(aasmContext);
    else if (ARG_AH_I8)
        MOV_AH_I8(aasmContext);
    else if (ARG_CH_I8)
        MOV_CH_I8(aasmContext);
    else if (ARG_DH_I8)
        MOV_DH_I8(aasmContext);
    else if (ARG_BH_I8)
        MOV_BH_I8(aasmContext);
    else if (ARG_AX_I16)
        MOV_EAX_I32(aasmContext, 2);
    else if (ARG_CX_I16)
        MOV_ECX_I32(aasmContext, 2);
    else if (ARG_DX_I16)
        MOV_EDX_I32(aasmContext, 2);
    else if (ARG_BX_I16)
        MOV_EBX_I32(aasmContext, 2);
    else if (ARG_SP_I16)
        MOV_ESP_I32(aasmContext, 2);
    else if (ARG_BP_I16)
        MOV_EBP_I32(aasmContext, 2);
    else if (ARG_SI_I16)
        MOV_ESI_I32(aasmContext, 2);
    else if (ARG_DI_I16)
        MOV_EDI_I32(aasmContext, 2);
    else if (ARG_EAX_I32)
        MOV_EAX_I32(aasmContext, 4);
    else if (ARG_ECX_I32)
        MOV_ECX_I32(aasmContext, 4);
    else if (ARG_EDX_I32)
        MOV_EDX_I32(aasmContext, 4);
    else if (ARG_EBX_I32)
        MOV_EBX_I32(aasmContext, 4);
    else if (ARG_ESP_I32)
        MOV_ESP_I32(aasmContext, 4);
    else if (ARG_EBP_I32)
        MOV_EBP_I32(aasmContext, 4);
    else if (ARG_ESI_I32)
        MOV_ESI_I32(aasmContext, 4);
    else if (ARG_EDI_I32)
        MOV_EDI_I32(aasmContext, 4);
    else if (ARG_AL_MOFFS8)
        MOV_AL_MOFFS8(aasmContext);
    else if (ARG_MOFFS8_AL)
        MOV_MOFFS8_AL(aasmContext);
    else if (ARG_AX_MOFFS16)
        MOV_EAX_MOFFS32(aasmContext, 2);
    else if (ARG_MOFFS16_AX)
        MOV_MOFFS32_EAX(aasmContext, 2);
    else if (ARG_EAX_MOFFS32)
        MOV_EAX_MOFFS32(aasmContext, 4);
    else if (ARG_MOFFS32_EAX)
        MOV_MOFFS32_EAX(aasmContext, 4);
    else if (ARG_R8_RM8)
        MOV_R8_RM8(aasmContext);
    else if (ARG_R16_RM16)
        MOV_R32_RM32(aasmContext, 2);
    else if (ARG_R32_RM32)
        MOV_R32_RM32(aasmContext, 4);
    else if (ARG_RM8_R8)
        MOV_RM8_R8(aasmContext);
    else if (ARG_RM16_R16)
        MOV_RM32_R32(aasmContext, 2);
    else if (ARG_RM32_R32)
        MOV_RM32_R32(aasmContext, 4);
    else if (ARG_RM16_SREG)
        MOV_RM16_SREG(aasmContext, 2);
    else if (ARG_RM32_SREG)
        MOV_RM16_SREG(aasmContext, 4);
    else if (ARG_SREG_RM16)
        MOV_SREG_RM16(aasmContext, 2);
    else if (ARG_SREG_RM32)
        MOV_SREG_RM16(aasmContext, 4);
    else if (ARG_RM8_I8)
        INS_C6(aasmContext, 0x00);
    else if (ARG_RM16_I16)
        INS_C7(aasmContext, 0x00, 2);
    else if (ARG_RM32_I32)
        INS_C7(aasmContext, 0x00, 4);
    else if (ARG_R32_CR0)
        MOV_R32_CR(aasmContext, 0);
    else if (ARG_R32_CR2)
        MOV_R32_CR(aasmContext, 2);
    else if (ARG_R32_CR3)
        MOV_R32_CR(aasmContext, 3);
    else if (ARG_CR0_R32)
        MOV_CR_R32(aasmContext, 0);
    else if (ARG_CR2_R32)
        MOV_CR_R32(aasmContext, 2);
    else if (ARG_CR3_R32)
        MOV_CR_R32(aasmContext, 3);
    else if (ARG_R32_DR0)
        MOV_R32_DR(aasmContext, 0);
    else if (ARG_R32_DR1)
        MOV_R32_DR(aasmContext, 1);
    else if (ARG_R32_DR2)
        MOV_R32_DR(aasmContext, 2);
    else if (ARG_R32_DR3)
        MOV_R32_DR(aasmContext, 3);
    else if (ARG_R32_DR6)
        MOV_R32_DR(aasmContext, 6);
    else if (ARG_R32_DR7)
        MOV_R32_DR(aasmContext, 7);
    else if (ARG_DR0_R32)
        MOV_DR_R32(aasmContext, 0);
    else if (ARG_DR1_R32)
        MOV_DR_R32(aasmContext, 1);
    else if (ARG_DR2_R32)
        MOV_DR_R32(aasmContext, 2);
    else if (ARG_DR3_R32)
        MOV_DR_R32(aasmContext, 3);
    else if (ARG_DR6_R32)
        MOV_DR_R32(aasmContext, 6);
    else if (ARG_DR7_R32)
        MOV_DR_R32(aasmContext, 7);
    else if (ARG_R32_DR6)
        MOV_R32_TR(aasmContext, 6);
    else if (ARG_R32_DR7)
        MOV_R32_TR(aasmContext, 7);
    else if (ARG_TR6_R32)
        MOV_TR_R32(aasmContext, 6);
    else if (ARG_TR7_R32)
        MOV_TR_R32(aasmContext, 7);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void LEA(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("LEA");
    if (ARG_R16_M16)
        LEA_R32_M32(aasmContext, 2);
    else if (ARG_R32_M32)
        LEA_R32_M32(aasmContext, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void CALL(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("CALL");
    if (ARG_FAR_I16_16)
        CALL_PTR16_32(aasmContext, 2);
    else if (ARG_FAR_I16_32)
        CALL_PTR16_32(aasmContext, 4);
    else if (ARG_NEAR_I16s || ARG_PNONE_I16s)
        CALL_REL32(aasmContext, 2);
    else if (ARG_NEAR_I32s || ARG_PNONE_I32s)
        CALL_REL32(aasmContext, 4);
    else if (ARG_NEAR_RM16s || ARG_PNONE_RM16s)
        INS_FF(aasmContext, 0x02, 2);
    else if (ARG_NEAR_RM32s || ARG_PNONE_RM32s)
        INS_FF(aasmContext, 0x02, 4);
    else if (ARG_FAR_M16_16)
        INS_FF(aasmContext, 0x03, 2);
    else if (ARG_FAR_M16_32)
        INS_FF(aasmContext, 0x03, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void MOVS(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOVS");
    if (ARG_ESDI8s_DSSI8s || ARG_ESEDI8s_DSESI8s)
        MOVSB(aasmContext);
    else if (ARG_ESDI16s_DSSI16s || ARG_ESEDI16s_DSESI16s)
        MOVSW(aasmContext, 2);
    else if (ARG_ESDI32s_DSSI32s || ARG_ESEDI32s_DSESI32s)
        MOVSW(aasmContext, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void CMPS(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("CMPS");
    if (ARG_DSSI8s_ESDI8s || ARG_DSESI8s_ESEDI8s)
        CMPSB(aasmContext);
    else if (ARG_DSSI16s_ESDI16s || ARG_DSESI16s_ESEDI16s)
        CMPSW(aasmContext, 2);
    else if (ARG_DSSI32s_ESDI32s || ARG_DSESI32s_ESEDI32s)
        CMPSW(aasmContext, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void STOS(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("STOS");
    if (ARG_ESDI8s || ARG_ESEDI8s)
        STOSB(aasmContext);
    else if (ARG_ESDI16s || ARG_ESEDI16s)
        STOSW(aasmContext, 2);
    else if (ARG_ESDI32s || ARG_ESEDI32s)
        STOSW(aasmContext, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void LODS(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("LODS");
    if (ARG_DSSI8s || ARG_DSESI8s)
        LODSB(aasmContext);
    else if (ARG_DSSI16s || ARG_DSESI16s)
        LODSW(aasmContext, 2);
    else if (ARG_DSSI32s || ARG_DSESI32s)
        LODSW(aasmContext, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void SCAS(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SCAS");
    if (ARG_ESDI8s || ARG_ESEDI8s)
        SCASB(aasmContext);
    else if (ARG_ESDI16s || ARG_ESEDI16s)
        SCASW(aasmContext, 2);
    else if (ARG_ESDI32s || ARG_ESEDI32s)
        SCASW(aasmContext, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void RET(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("RET");
    if (ARG_I16u)
        RET_I16(aasmContext);
    else if (ARG_NONE)
        RET_(aasmContext);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void LES(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("LES");
    if (ARG_R16_M16)
        LES_R32_M16_32(aasmContext, 2);
    else if (ARG_R32_M32)
        LES_R32_M16_32(aasmContext, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void LDS(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("LDS");
    if (ARG_R16_M16)
        LDS_R32_M16_32(aasmContext, 2);
    else if (ARG_R32_M32)
        LDS_R32_M16_32(aasmContext, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void RETF(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("RETF");
    if (ARG_I16u)
        RETF_I16(aasmContext);
    else if (ARG_NONE)
        RETF_(aasmContext);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void INT(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("INT");
    if (ARG_I8)
        INT_I8(aasmContext);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void XLAT(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("XLAT");
    if (ARG_DSBXAL8 || ARG_DSEBXAL8)
        XLATB(aasmContext);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void ROL(aasm32_context *aasmContext)
{
    lib_u8 rid = 0x00;
    XASM32_TRACE_CALL_BEGIN("ROL");
    if (ARG_RM8_I8 && aopri2.imm8 == 1)
        INS_D0(aasmContext, rid);
    else if (ARG_RM16_I8 && aopri2.imm8 == 1)
        INS_D1(aasmContext, rid, 2);
    else if (ARG_RM32_I8 && aopri2.imm8 == 1)
        INS_D1(aasmContext, rid, 4);
    else if (ARG_RM8_CL)
        INS_D2(aasmContext, rid);
    else if (ARG_RM16_CL)
        INS_D3(aasmContext, rid, 2);
    else if (ARG_RM32_CL)
        INS_D3(aasmContext, rid, 4);
    else if (ARG_RM8_I8)
        INS_C0(aasmContext, rid);
    else if (ARG_RM16_I8)
        INS_C1(aasmContext, rid, 2);
    else if (ARG_RM32_I8)
        INS_C1(aasmContext, rid, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void ROR(aasm32_context *aasmContext)
{
    lib_u8 rid = 0x01;
    XASM32_TRACE_CALL_BEGIN("ROR");
    if (ARG_RM8_I8 && aopri2.imm8 == 1)
        INS_D0(aasmContext, rid);
    else if (ARG_RM16_I8 && aopri2.imm8 == 1)
        INS_D1(aasmContext, rid, 2);
    else if (ARG_RM32_I8 && aopri2.imm8 == 1)
        INS_D1(aasmContext, rid, 4);
    else if (ARG_RM8_CL)
        INS_D2(aasmContext, rid);
    else if (ARG_RM16_CL)
        INS_D3(aasmContext, rid, 2);
    else if (ARG_RM32_CL)
        INS_D3(aasmContext, rid, 4);
    else if (ARG_RM8_I8)
        INS_C0(aasmContext, rid);
    else if (ARG_RM16_I8)
        INS_C1(aasmContext, rid, 2);
    else if (ARG_RM32_I8)
        INS_C1(aasmContext, rid, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void RCL(aasm32_context *aasmContext)
{
    lib_u8 rid = 0x02;
    XASM32_TRACE_CALL_BEGIN("RCL");
    if (ARG_RM8_I8 && aopri2.imm8 == 1)
        INS_D0(aasmContext, rid);
    else if (ARG_RM16_I8 && aopri2.imm8 == 1)
        INS_D1(aasmContext, rid, 2);
    else if (ARG_RM32_I8 && aopri2.imm8 == 1)
        INS_D1(aasmContext, rid, 4);
    else if (ARG_RM8_CL)
        INS_D2(aasmContext, rid);
    else if (ARG_RM16_CL)
        INS_D3(aasmContext, rid, 2);
    else if (ARG_RM32_CL)
        INS_D3(aasmContext, rid, 4);
    else if (ARG_RM8_I8)
        INS_C0(aasmContext, rid);
    else if (ARG_RM16_I8)
        INS_C1(aasmContext, rid, 2);
    else if (ARG_RM32_I8)
        INS_C1(aasmContext, rid, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void RCR(aasm32_context *aasmContext)
{
    lib_u8 rid = 0x03;
    XASM32_TRACE_CALL_BEGIN("IN");
    if (ARG_RM8_I8 && aopri2.imm8 == 1)
        INS_D0(aasmContext, rid);
    else if (ARG_RM16_I8 && aopri2.imm8 == 1)
        INS_D1(aasmContext, rid, 2);
    else if (ARG_RM32_I8 && aopri2.imm8 == 1)
        INS_D1(aasmContext, rid, 4);
    else if (ARG_RM8_CL)
        INS_D2(aasmContext, rid);
    else if (ARG_RM16_CL)
        INS_D3(aasmContext, rid, 2);
    else if (ARG_RM32_CL)
        INS_D3(aasmContext, rid, 4);
    else if (ARG_RM8_I8)
        INS_C0(aasmContext, rid);
    else if (ARG_RM16_I8)
        INS_C1(aasmContext, rid, 2);
    else if (ARG_RM32_I8)
        INS_C1(aasmContext, rid, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void SHL(aasm32_context *aasmContext)
{
    lib_u8 rid = 0x04;
    XASM32_TRACE_CALL_BEGIN("SHL");
    if (ARG_RM8_I8 && aopri2.imm8 == 1)
        INS_D0(aasmContext, rid);
    else if (ARG_RM16_I8 && aopri2.imm8 == 1)
        INS_D1(aasmContext, rid, 2);
    else if (ARG_RM32_I8 && aopri2.imm8 == 1)
        INS_D1(aasmContext, rid, 4);
    else if (ARG_RM8_CL)
        INS_D2(aasmContext, rid);
    else if (ARG_RM16_CL)
        INS_D3(aasmContext, rid, 2);
    else if (ARG_RM32_CL)
        INS_D3(aasmContext, rid, 4);
    else if (ARG_RM8_I8)
        INS_C0(aasmContext, rid);
    else if (ARG_RM16_I8)
        INS_C1(aasmContext, rid, 2);
    else if (ARG_RM32_I8)
        INS_C1(aasmContext, rid, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void SHR(aasm32_context *aasmContext)
{
    lib_u8 rid = 0x05;
    XASM32_TRACE_CALL_BEGIN("SHR");
    if (ARG_RM8_I8 && aopri2.imm8 == 1)
        INS_D0(aasmContext, rid);
    else if (ARG_RM16_I8 && aopri2.imm8 == 1)
        INS_D1(aasmContext, rid, 2);
    else if (ARG_RM32_I8 && aopri2.imm8 == 1)
        INS_D1(aasmContext, rid, 4);
    else if (ARG_RM8_CL)
        INS_D2(aasmContext, rid);
    else if (ARG_RM16_CL)
        INS_D3(aasmContext, rid, 2);
    else if (ARG_RM32_CL)
        INS_D3(aasmContext, rid, 4);
    else if (ARG_RM8_I8)
        INS_C0(aasmContext, rid);
    else if (ARG_RM16_I8)
        INS_C1(aasmContext, rid, 2);
    else if (ARG_RM32_I8)
        INS_C1(aasmContext, rid, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void SAL(aasm32_context *aasmContext)
{
    lib_u8 rid = 0x04;
    XASM32_TRACE_CALL_BEGIN("SAL");
    if (ARG_RM8_I8 && aopri2.imm8 == 1)
        INS_D0(aasmContext, rid);
    else if (ARG_RM16_I8 && aopri2.imm8 == 1)
        INS_D1(aasmContext, rid, 2);
    else if (ARG_RM32_I8 && aopri2.imm8 == 1)
        INS_D1(aasmContext, rid, 4);
    else if (ARG_RM8_CL)
        INS_D2(aasmContext, rid);
    else if (ARG_RM16_CL)
        INS_D3(aasmContext, rid, 2);
    else if (ARG_RM32_CL)
        INS_D3(aasmContext, rid, 4);
    else if (ARG_RM8_I8)
        INS_C0(aasmContext, rid);
    else if (ARG_RM16_I8)
        INS_C1(aasmContext, rid, 2);
    else if (ARG_RM32_I8)
        INS_C1(aasmContext, rid, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void SAR(aasm32_context *aasmContext)
{
    lib_u8 rid = 0x07;
    XASM32_TRACE_CALL_BEGIN("SAR");
    if (ARG_RM8_I8 && aopri2.imm8 == 1)
        INS_D0(aasmContext, rid);
    else if (ARG_RM16_I8 && aopri2.imm8 == 1)
        INS_D1(aasmContext, rid, 2);
    else if (ARG_RM32_I8 && aopri2.imm8 == 1)
        INS_D1(aasmContext, rid, 4);
    else if (ARG_RM8_CL)
        INS_D2(aasmContext, rid);
    else if (ARG_RM16_CL)
        INS_D3(aasmContext, rid, 2);
    else if (ARG_RM32_CL)
        INS_D3(aasmContext, rid, 4);
    else if (ARG_RM8_I8)
        INS_C0(aasmContext, rid);
    else if (ARG_RM16_I8)
        INS_C1(aasmContext, rid, 2);
    else if (ARG_RM32_I8)
        INS_C1(aasmContext, rid, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void IN(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("IN");
    if (ARG_AL_I8u)
        IN_AL_I8(aasmContext);
    else if (ARG_AX_I8u)
        IN_EAX_I8(aasmContext, 2);
    else if (ARG_EAX_I8u)
        IN_EAX_I8(aasmContext, 4);
    else if (ARG_AL_DX)
        IN_AL_DX(aasmContext);
    else if (ARG_AX_DX)
        IN_EAX_DX(aasmContext, 2);
    else if (ARG_EAX_DX)
        IN_EAX_DX(aasmContext, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void OUT(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("OUT");
    if (ARG_I8u_AL)
        OUT_I8_AL(aasmContext);
    else if (ARG_I8u_AX)
        OUT_I8_EAX(aasmContext, 2);
    else if (ARG_I8u_EAX)
        OUT_I8_EAX(aasmContext, 4);
    else if (ARG_DX_AL)
        OUT_DX_AL(aasmContext);
    else if (ARG_DX_AX)
        OUT_DX_EAX(aasmContext, 2);
    else if (ARG_DX_EAX)
        OUT_DX_EAX(aasmContext, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void NOT(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("NOT");
    if (ARG_RM8s)
        INS_F6(aasmContext, 0x02);
    else if (ARG_RM16s)
        INS_F7(aasmContext, 0x02, 2);
    else if (ARG_RM32s)
        INS_F7(aasmContext, 0x02, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void NEG(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("NEG");
    if (ARG_RM8s)
        INS_F6(aasmContext, 0x03);
    else if (ARG_RM16s)
        INS_F7(aasmContext, 0x03, 2);
    else if (ARG_RM32s)
        INS_F7(aasmContext, 0x03, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void MUL(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MUL");
    if (ARG_RM8s)
        INS_F6(aasmContext, 0x04);
    else if (ARG_RM16s)
        INS_F7(aasmContext, 0x04, 2);
    else if (ARG_RM32s)
        INS_F7(aasmContext, 0x04, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void DIV(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("DIV");
    if (ARG_RM8s)
        INS_F6(aasmContext, 0x06);
    else if (ARG_RM16s)
        INS_F7(aasmContext, 0x06, 2);
    else if (ARG_RM32s)
        INS_F7(aasmContext, 0x06, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void IDIV(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("IDIV");
    if (ARG_RM8s)
        INS_F6(aasmContext, 0x07);
    else if (ARG_RM16s)
        INS_F7(aasmContext, 0x07, 2);
    else if (ARG_RM32s)
        INS_F7(aasmContext, 0x07, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void JMP(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JMP");
    if (ARG_FAR_I16_16)
        JMP_PTR16_32(aasmContext, 2);
    else if (ARG_FAR_I16_32)
        JMP_PTR16_32(aasmContext, 4);
    else if (ARG_SHORT_I8s || ARG_PNONE_I8s)
        JCC_REL(aasmContext, 0xeb);
    else if (ARG_NEAR_I16s || ARG_PNONE_I16s)
        JMP_REL32(aasmContext, 2);
    else if (ARG_NEAR_I32s || ARG_PNONE_I32s)
        JMP_REL32(aasmContext, 4);
    else if (ARG_NEAR_RM16s || ARG_PNONE_RM16s)
        INS_FF(aasmContext, 0x04, 2);
    else if (ARG_NEAR_RM32s || ARG_PNONE_RM32s)
        INS_FF(aasmContext, 0x04, 4);
    else if (ARG_FAR_M16_16)
        INS_FF(aasmContext, 0x05, 2);
    else if (ARG_FAR_M16_32)
        INS_FF(aasmContext, 0x05, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
/* abstract extended instructions */
static void SLDT(aasm32_context *aasmContext)
{
    lib_u8 rid = 0x00;
    XASM32_TRACE_CALL_BEGIN("SLDT");
    if (ARG_RM16)
        INS_0F_00(aasmContext, rid, 2);
    else if (ARG_R32)
        INS_0F_00(aasmContext, rid, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void STR(aasm32_context *aasmContext)
{
    lib_u8 rid = 0x01;
    XASM32_TRACE_CALL_BEGIN("STR");
    if (ARG_RM16)
        INS_0F_00(aasmContext, rid, 2);
    else if (ARG_R32)
        INS_0F_00(aasmContext, rid, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void LLDT(aasm32_context *aasmContext)
{
    lib_u8 rid = 0x02;
    XASM32_TRACE_CALL_BEGIN("LLDT");
    if (ARG_RM16)
        INS_0F_00(aasmContext, rid, 0);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void LTR(aasm32_context *aasmContext)
{
    lib_u8 rid = 0x03;
    XASM32_TRACE_CALL_BEGIN("LTR");
    if (ARG_RM16)
        INS_0F_00(aasmContext, rid, 0);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void VERR(aasm32_context *aasmContext)
{
    lib_u8 rid = 0x04;
    XASM32_TRACE_CALL_BEGIN("VERR");
    if (ARG_RM16)
        INS_0F_00(aasmContext, rid, 0);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void VERW(aasm32_context *aasmContext)
{
    lib_u8 rid = 0x05;
    XASM32_TRACE_CALL_BEGIN("VERW");
    if (ARG_RM16)
        INS_0F_00(aasmContext, rid, 0);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void SGDT(aasm32_context *aasmContext)
{
    lib_u8 rid = 0x00;
    XASM32_TRACE_CALL_BEGIN("SGDT");
    if (ARG_M16s)
        INS_0F_01(aasmContext, rid, 2);
    else if (ARG_M32s)
        INS_0F_01(aasmContext, rid, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void SIDT(aasm32_context *aasmContext)
{
    lib_u8 rid = 0x01;
    XASM32_TRACE_CALL_BEGIN("SIDT");
    if (ARG_M16s)
        INS_0F_01(aasmContext, rid, 2);
    else if (ARG_M32s)
        INS_0F_01(aasmContext, rid, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void LGDT(aasm32_context *aasmContext)
{
    lib_u8 rid = 0x02;
    XASM32_TRACE_CALL_BEGIN("SIDT");
    if (ARG_M16s)
        INS_0F_01(aasmContext, rid, 2);
    else if (ARG_M32)
        INS_0F_01(aasmContext, rid, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void LIDT(aasm32_context *aasmContext)
{
    lib_u8 rid = 0x03;
    XASM32_TRACE_CALL_BEGIN("LIDT");
    if (ARG_M16s)
        INS_0F_01(aasmContext, rid, 2);
    else if (ARG_M32)
        INS_0F_01(aasmContext, rid, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void SMSW(aasm32_context *aasmContext)
{
    lib_u8 rid = 0x04;
    XASM32_TRACE_CALL_BEGIN("SMSW");
    if (ARG_RM16)
        INS_0F_01(aasmContext, rid, 2);
    else if (ARG_R32)
        INS_0F_01(aasmContext, rid, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void LMSW(aasm32_context *aasmContext)
{
    lib_u8 rid = 0x06;
    XASM32_TRACE_CALL_BEGIN("LMSW");
    if (ARG_RM16)
        INS_0F_01(aasmContext, rid, 0);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void LAR(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("LAR");
    if (ARG_R16_RM16)
        LAR_R32_RM32(aasmContext, 2);
    else if (ARG_R32_RM32)
        LAR_R32_RM32(aasmContext, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void LSL(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("LSL");
    if (ARG_R16_RM16)
        LSL_R32_RM32(aasmContext, 2);
    else if (ARG_R32_RM32)
        LSL_R32_RM32(aasmContext, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void BT(aasm32_context *aasmContext)
{
    lib_u8 rid = 0x04;
    XASM32_TRACE_CALL_BEGIN("BT");
    if (ARG_RM16_R16)
        BT_RM32_R32(aasmContext, 2);
    else if (ARG_RM32_R32)
        BT_RM32_R32(aasmContext, 4);
    else if (ARG_RM16_I8)
        INS_0F_BA(aasmContext, rid, 2);
    else if (ARG_RM32_I8)
        INS_0F_BA(aasmContext, rid, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void SHLD(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SHLD");
    if (ARG_RM16_R16_I8)
        SHLD_RM32_R32_I8(aasmContext, 2);
    if (ARG_RM16_R16_CL)
        SHLD_RM32_R32_CL(aasmContext, 2);
    else if (ARG_RM32_R32_I8)
        SHLD_RM32_R32_I8(aasmContext, 4);
    else if (ARG_RM32_R32_CL)
        SHLD_RM32_R32_CL(aasmContext, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void BTS(aasm32_context *aasmContext)
{
    lib_u8 rid = 0x05;
    XASM32_TRACE_CALL_BEGIN("BTS");
    if (ARG_RM16_R16)
        BTS_RM32_R32(aasmContext, 2);
    else if (ARG_RM32_R32)
        BTS_RM32_R32(aasmContext, 4);
    else if (ARG_RM16_I8)
        INS_0F_BA(aasmContext, rid, 2);
    else if (ARG_RM32_I8)
        INS_0F_BA(aasmContext, rid, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void SHRD(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SHRD");
    if (ARG_RM16_R16_I8)
        SHRD_RM32_R32_I8(aasmContext, 2);
    if (ARG_RM16_R16_CL)
        SHRD_RM32_R32_CL(aasmContext, 2);
    else if (ARG_RM32_R32_I8)
        SHRD_RM32_R32_I8(aasmContext, 4);
    else if (ARG_RM32_R32_CL)
        SHRD_RM32_R32_CL(aasmContext, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void LSS(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("LSS");
    if (ARG_R16_M16)
        LSS_R32_M16_32(aasmContext, 2);
    else if (ARG_R32_M32)
        LSS_R32_M16_32(aasmContext, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void BTR(aasm32_context *aasmContext)
{
    lib_u8 rid = 0x06;
    XASM32_TRACE_CALL_BEGIN("BTR");
    if (ARG_RM16_R16)
        BTR_RM32_R32(aasmContext, 2);
    else if (ARG_RM32_R32)
        BTR_RM32_R32(aasmContext, 4);
    else if (ARG_RM16_I8)
        INS_0F_BA(aasmContext, rid, 2);
    else if (ARG_RM32_I8)
        INS_0F_BA(aasmContext, rid, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void LFS(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("LFS");
    if (ARG_R16_M16)
        LFS_R32_M16_32(aasmContext, 2);
    else if (ARG_R32_M32)
        LFS_R32_M16_32(aasmContext, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void LGS(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("LGS");
    if (ARG_R16_M16)
        LGS_R32_M16_32(aasmContext, 2);
    else if (ARG_R32_M32)
        LGS_R32_M16_32(aasmContext, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void MOVZX(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOVZX");
    if (ARG_R16_RM8s)
        MOVZX_R32_RM8(aasmContext, 2);
    else if (ARG_R32_RM8s)
        MOVZX_R32_RM8(aasmContext, 4);
    else if (ARG_R32_RM16s)
        MOVZX_R32_RM16(aasmContext);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void BTC(aasm32_context *aasmContext)
{
    lib_u8 rid = 0x07;
    XASM32_TRACE_CALL_BEGIN("BTC");
    if (ARG_RM16_R16)
        BTC_RM32_R32(aasmContext, 2);
    else if (ARG_RM32_R32)
        BTC_RM32_R32(aasmContext, 4);
    else if (ARG_RM16_I8)
        INS_0F_BA(aasmContext, rid, 2);
    else if (ARG_RM32_I8)
        INS_0F_BA(aasmContext, rid, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void BSF(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("BSF");
    if (ARG_R16_RM16)
        BSF_R32_RM32(aasmContext, 2);
    else if (ARG_R32_RM32)
        BSF_R32_RM32(aasmContext, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void BSR(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("BSR");
    if (ARG_R16_RM16)
        BSR_R32_RM32(aasmContext, 2);
    else if (ARG_R32_RM32)
        BSR_R32_RM32(aasmContext, 4);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static void MOVSX(aasm32_context *aasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOVSX");
    if (ARG_R16_RM8s)
        MOVSX_R32_RM8(aasmContext, 2);
    else if (ARG_R32_RM8s)
        MOVSX_R32_RM8(aasmContext, 4);
    else if (ARG_R32_RM16s)
        MOVSX_R32_RM16(aasmContext);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}

/* main routines */
static int is_end(aasm32_context *aasmContext, char c)
{
    return (!c || c == '\n' || c == ';');
}
static int is_space(aasm32_context *aasmContext, char c)
{
    return (c == ' ' || c == '\t');
}
static int is_prefix(aasm32_context *aasmContext)
{
    if (!strcmp(rop, "es:") || !strcmp(rop, "cs:") ||
        !strcmp(rop, "ss:") || !strcmp(rop, "ds:") ||
        !strcmp(rop, "fs:") || !strcmp(rop, "gs:") ||
        !strcmp(rop, "lock:") || !strcmp(rop, "rep:") ||
        !strcmp(rop, "repne:") || !strcmp(rop, "repnz:") ||
        !strcmp(rop, "repe:") || !strcmp(rop, "repz:"))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
static void exec(aasm32_context *aasmContext)
{
    /* assemble single statement */
    XASM32_TRACE_CALL_BEGIN("exec");
    if (!rop || is_end(aasmContext, rop[0]))
        ;
    else if (!strcmp(rop, "add"))
        ADD(aasmContext);
    else if (!strcmp(rop, "push"))
        PUSH(aasmContext);
    else if (!strcmp(rop, "pop"))
        POP(aasmContext);
    else if (!strcmp(rop, "or"))
        OR(aasmContext);
    else if (!strcmp(rop, "adc"))
        ADC(aasmContext);
    else if (!strcmp(rop, "sbb"))
        SBB(aasmContext);
    else if (!strcmp(rop, "and"))
        AND(aasmContext);
    else if (!strcmp(rop, "es:"))
        PREFIX_ES(aasmContext);
    else if (!strcmp(rop, "daa"))
        DAA(aasmContext);
    else if (!strcmp(rop, "sub"))
        SUB(aasmContext);
    else if (!strcmp(rop, "cs:"))
        PREFIX_CS(aasmContext);
    else if (!strcmp(rop, "das"))
        DAS(aasmContext);
    else if (!strcmp(rop, "xor"))
        XOR(aasmContext);
    else if (!strcmp(rop, "ss:"))
        PREFIX_SS(aasmContext);
    else if (!strcmp(rop, "aaa"))
        AAA(aasmContext);
    else if (!strcmp(rop, "cmp"))
        CMP(aasmContext);
    else if (!strcmp(rop, "ds:"))
        PREFIX_DS(aasmContext);
    else if (!strcmp(rop, "aas"))
        AAS(aasmContext);
    else if (!strcmp(rop, "inc"))
        INC(aasmContext);
    else if (!strcmp(rop, "dec"))
        DEC(aasmContext);
    else if (!strcmp(rop, "pusha"))
        PUSHA(aasmContext, 2);
    else if (!strcmp(rop, "pushad"))
        PUSHA(aasmContext, 4);
    else if (!strcmp(rop, "popa"))
        POPA(aasmContext, 2);
    else if (!strcmp(rop, "popad"))
        POPA(aasmContext, 4);
    else if (!strcmp(rop, "bound"))
        BOUND(aasmContext);
    else if (!strcmp(rop, "arpl"))
        ARPL_RM16_R16(aasmContext);
    else if (!strcmp(rop, "fs:"))
        PREFIX_FS(aasmContext);
    else if (!strcmp(rop, "gs:"))
        PREFIX_GS(aasmContext);
    else if (!strcmp(rop, "op+:"))
        PREFIX_OprSize(aasmContext);
    else if (!strcmp(rop, "az+:"))
        PREFIX_AddrSize(aasmContext);
    else if (!strcmp(rop, "imul"))
        IMUL(aasmContext);
    else if (!strcmp(rop, "ins"))
        INS(aasmContext);
    else if (!strcmp(rop, "insb"))
        INSB(aasmContext);
    else if (!strcmp(rop, "insw"))
        INSW(aasmContext, 2);
    else if (!strcmp(rop, "insd"))
        INSW(aasmContext, 4);
    else if (!strcmp(rop, "outs"))
        OUTS(aasmContext);
    else if (!strcmp(rop, "outsb"))
        OUTSB(aasmContext);
    else if (!strcmp(rop, "outsw"))
        OUTSW(aasmContext, 2);
    else if (!strcmp(rop, "outsd"))
        OUTSW(aasmContext, 4);
    else if (!strcmp(rop, "jo"))
        JCC_REL(aasmContext, 0x70);
    else if (!strcmp(rop, "jno"))
        JCC_REL(aasmContext, 0x71);
    else if (!strcmp(rop, "jb"))
        JCC_REL(aasmContext, 0x72);
    else if (!strcmp(rop, "jc"))
        JCC_REL(aasmContext, 0x72);
    else if (!strcmp(rop, "jnae"))
        JCC_REL(aasmContext, 0x72);
    else if (!strcmp(rop, "jae"))
        JCC_REL(aasmContext, 0x73);
    else if (!strcmp(rop, "jnb"))
        JCC_REL(aasmContext, 0x73);
    else if (!strcmp(rop, "jnc"))
        JCC_REL(aasmContext, 0x73);
    else if (!strcmp(rop, "je"))
        JCC_REL(aasmContext, 0x74);
    else if (!strcmp(rop, "jz"))
        JCC_REL(aasmContext, 0x74);
    else if (!strcmp(rop, "jne"))
        JCC_REL(aasmContext, 0x75);
    else if (!strcmp(rop, "jnz"))
        JCC_REL(aasmContext, 0x75);
    else if (!strcmp(rop, "jbe"))
        JCC_REL(aasmContext, 0x76);
    else if (!strcmp(rop, "jna"))
        JCC_REL(aasmContext, 0x76);
    else if (!strcmp(rop, "ja"))
        JCC_REL(aasmContext, 0x77);
    else if (!strcmp(rop, "jnbe"))
        JCC_REL(aasmContext, 0x77);
    else if (!strcmp(rop, "js"))
        JCC_REL(aasmContext, 0x78);
    else if (!strcmp(rop, "jns"))
        JCC_REL(aasmContext, 0x79);
    else if (!strcmp(rop, "jp"))
        JCC_REL(aasmContext, 0x7a);
    else if (!strcmp(rop, "jpe"))
        JCC_REL(aasmContext, 0x7a);
    else if (!strcmp(rop, "jnp"))
        JCC_REL(aasmContext, 0x7b);
    else if (!strcmp(rop, "jpo"))
        JCC_REL(aasmContext, 0x7b);
    else if (!strcmp(rop, "jl"))
        JCC_REL(aasmContext, 0x7c);
    else if (!strcmp(rop, "jnge"))
        JCC_REL(aasmContext, 0x7c);
    else if (!strcmp(rop, "jge"))
        JCC_REL(aasmContext, 0x7d);
    else if (!strcmp(rop, "jnl"))
        JCC_REL(aasmContext, 0x7d);
    else if (!strcmp(rop, "jle"))
        JCC_REL(aasmContext, 0x7e);
    else if (!strcmp(rop, "jng"))
        JCC_REL(aasmContext, 0x7e);
    else if (!strcmp(rop, "jg"))
        JCC_REL(aasmContext, 0x7f);
    else if (!strcmp(rop, "jnle"))
        JCC_REL(aasmContext, 0x7f);
    else if (!strcmp(rop, "test"))
        TEST(aasmContext);
    else if (!strcmp(rop, "xchg"))
        XCHG(aasmContext);
    else if (!strcmp(rop, "mov"))
        MOV(aasmContext);
    else if (!strcmp(rop, "lea"))
        LEA(aasmContext);
    else if (!strcmp(rop, "nop"))
        NOP(aasmContext);
    else if (!strcmp(rop, "cbw"))
        CBW(aasmContext, 2);
    else if (!strcmp(rop, "cwde"))
        CBW(aasmContext, 4);
    else if (!strcmp(rop, "cwd"))
        CWD(aasmContext, 2);
    else if (!strcmp(rop, "cdq"))
        CWD(aasmContext, 4);
    else if (!strcmp(rop, "call"))
        CALL(aasmContext);
    else if (!strcmp(rop, "wait"))
        WAIT(aasmContext);
    else if (!strcmp(rop, "pushf"))
        PUSHF(aasmContext, 2);
    else if (!strcmp(rop, "pushfd"))
        PUSHF(aasmContext, 4);
    else if (!strcmp(rop, "popf"))
        POPF(aasmContext, 2);
    else if (!strcmp(rop, "popfd"))
        POPF(aasmContext, 4);
    else if (!strcmp(rop, "sahf"))
        SAHF(aasmContext);
    else if (!strcmp(rop, "lahf"))
        LAHF(aasmContext);
    else if (!strcmp(rop, "movs"))
        MOVS(aasmContext);
    else if (!strcmp(rop, "movsb"))
        MOVSB(aasmContext);
    else if (!strcmp(rop, "movsw"))
        MOVSW(aasmContext, 2);
    else if (!strcmp(rop, "movsd"))
        MOVSW(aasmContext, 4);
    else if (!strcmp(rop, "cmps"))
        CMPS(aasmContext);
    else if (!strcmp(rop, "cmpsb"))
        CMPSB(aasmContext);
    else if (!strcmp(rop, "cmpsw"))
        CMPSW(aasmContext, 2);
    else if (!strcmp(rop, "cmpsd"))
        CMPSW(aasmContext, 4);
    else if (!strcmp(rop, "stos"))
        STOS(aasmContext);
    else if (!strcmp(rop, "stosb"))
        STOSB(aasmContext);
    else if (!strcmp(rop, "stosw"))
        STOSW(aasmContext, 2);
    else if (!strcmp(rop, "stosd"))
        STOSW(aasmContext, 4);
    else if (!strcmp(rop, "lods"))
        LODS(aasmContext);
    else if (!strcmp(rop, "lodsb"))
        LODSB(aasmContext);
    else if (!strcmp(rop, "lodsw"))
        LODSW(aasmContext, 2);
    else if (!strcmp(rop, "lodsd"))
        LODSW(aasmContext, 4);
    else if (!strcmp(rop, "scas"))
        SCAS(aasmContext);
    else if (!strcmp(rop, "scasb"))
        SCASB(aasmContext);
    else if (!strcmp(rop, "scasw"))
        SCASW(aasmContext, 2);
    else if (!strcmp(rop, "scasd"))
        SCASW(aasmContext, 4);
    else if (!strcmp(rop, "ret"))
        RET(aasmContext);
    else if (!strcmp(rop, "les"))
        LES(aasmContext);
    else if (!strcmp(rop, "lds"))
        LDS(aasmContext);
    else if (!strcmp(rop, "enter"))
        ENTER(aasmContext);
    else if (!strcmp(rop, "leave"))
        LEAVE(aasmContext);
    else if (!strcmp(rop, "retf"))
        RETF(aasmContext);
    else if (!strcmp(rop, "int"))
        INT(aasmContext);
    else if (!strcmp(rop, "into"))
        INTO(aasmContext);
    else if (!strcmp(rop, "iret"))
        IRET(aasmContext, 2);
    else if (!strcmp(rop, "iretd"))
        IRET(aasmContext, 4);
    else if (!strcmp(rop, "rol"))
        ROL(aasmContext);
    else if (!strcmp(rop, "ror"))
        ROR(aasmContext);
    else if (!strcmp(rop, "rcl"))
        RCL(aasmContext);
    else if (!strcmp(rop, "rcr"))
        RCR(aasmContext);
    else if (!strcmp(rop, "shl"))
        SHL(aasmContext);
    else if (!strcmp(rop, "shr"))
        SHR(aasmContext);
    else if (!strcmp(rop, "sal"))
        SAL(aasmContext);
    else if (!strcmp(rop, "sar"))
        SAR(aasmContext);
    else if (!strcmp(rop, "aam"))
        AAM(aasmContext);
    else if (!strcmp(rop, "aad"))
        AAD(aasmContext);
    else if (!strcmp(rop, "xlat"))
        XLAT(aasmContext);
    else if (!strcmp(rop, "xlatb"))
        XLATB(aasmContext);
    else if (!strcmp(rop, "loopne"))
        JCC_REL(aasmContext, 0xe0);
    else if (!strcmp(rop, "loopnz"))
        JCC_REL(aasmContext, 0xe0);
    else if (!strcmp(rop, "loope"))
        JCC_REL(aasmContext, 0xe1);
    else if (!strcmp(rop, "loopz"))
        JCC_REL(aasmContext, 0xe1);
    else if (!strcmp(rop, "loop"))
        JCC_REL(aasmContext, 0xe2);
    else if (!strcmp(rop, "jcxz"))
        JCC_REL(aasmContext, 0xe3);
    else if (!strcmp(rop, "in"))
        IN(aasmContext);
    else if (!strcmp(rop, "out"))
        OUT(aasmContext);
    else if (!strcmp(rop, "jmp"))
        JMP(aasmContext);
    else if (!strcmp(rop, "lock"))
        PREFIX_LOCK(aasmContext);
    else if (!strcmp(rop, "repne:"))
        PREFIX_REPNZ(aasmContext);
    else if (!strcmp(rop, "repnz:"))
        PREFIX_REPNZ(aasmContext);
    else if (!strcmp(rop, "rep:"))
        PREFIX_REPZ(aasmContext);
    else if (!strcmp(rop, "repe:"))
        PREFIX_REPZ(aasmContext);
    else if (!strcmp(rop, "repz:"))
        PREFIX_REPZ(aasmContext);
    else if (!strcmp(rop, "hlt"))
        HLT(aasmContext);
    else if (!strcmp(rop, "cmc"))
        CMC(aasmContext);
    else if (!strcmp(rop, "not"))
        NOT(aasmContext);
    else if (!strcmp(rop, "neg"))
        NEG(aasmContext);
    else if (!strcmp(rop, "mul"))
        MUL(aasmContext);
    else if (!strcmp(rop, "div"))
        DIV(aasmContext);
    else if (!strcmp(rop, "idiv"))
        IDIV(aasmContext);
    else if (!strcmp(rop, "clc"))
        CLC(aasmContext);
    else if (!strcmp(rop, "stc"))
        STC(aasmContext);
    else if (!strcmp(rop, "cli"))
        CLI(aasmContext);
    else if (!strcmp(rop, "sti"))
        STI(aasmContext);
    else if (!strcmp(rop, "cld"))
        CLD(aasmContext);
    else if (!strcmp(rop, "std"))
        STD(aasmContext);
    else if (!strcmp(rop, "sldt"))
        SLDT(aasmContext);
    else if (!strcmp(rop, "str"))
        STR(aasmContext);
    else if (!strcmp(rop, "lldt"))
        LLDT(aasmContext);
    else if (!strcmp(rop, "ltr"))
        LTR(aasmContext);
    else if (!strcmp(rop, "verr"))
        VERR(aasmContext);
    else if (!strcmp(rop, "verw"))
        VERW(aasmContext);
    else if (!strcmp(rop, "sgdt"))
        SGDT(aasmContext);
    else if (!strcmp(rop, "sidt"))
        SIDT(aasmContext);
    else if (!strcmp(rop, "lgdt"))
        LGDT(aasmContext);
    else if (!strcmp(rop, "lidt"))
        LIDT(aasmContext);
    else if (!strcmp(rop, "smsw"))
        SMSW(aasmContext);
    else if (!strcmp(rop, "lmsw"))
        LMSW(aasmContext);
    else if (!strcmp(rop, "lar"))
        LAR(aasmContext);
    else if (!strcmp(rop, "lsl"))
        LSL(aasmContext);
    else if (!strcmp(rop, "clts"))
        CLTS(aasmContext);
    else if (!strcmp(rop, "seto"))
        SETCC_RM8(aasmContext, 0x90);
    else if (!strcmp(rop, "setno"))
        SETCC_RM8(aasmContext, 0x91);
    else if (!strcmp(rop, "setb"))
        SETCC_RM8(aasmContext, 0x92);
    else if (!strcmp(rop, "setc"))
        SETCC_RM8(aasmContext, 0x92);
    else if (!strcmp(rop, "setnae"))
        SETCC_RM8(aasmContext, 0x92);
    else if (!strcmp(rop, "setae"))
        SETCC_RM8(aasmContext, 0x93);
    else if (!strcmp(rop, "setnb"))
        SETCC_RM8(aasmContext, 0x93);
    else if (!strcmp(rop, "setnc"))
        SETCC_RM8(aasmContext, 0x93);
    else if (!strcmp(rop, "sete"))
        SETCC_RM8(aasmContext, 0x94);
    else if (!strcmp(rop, "setz"))
        SETCC_RM8(aasmContext, 0x94);
    else if (!strcmp(rop, "setne"))
        SETCC_RM8(aasmContext, 0x95);
    else if (!strcmp(rop, "setnz"))
        SETCC_RM8(aasmContext, 0x95);
    else if (!strcmp(rop, "setbe"))
        SETCC_RM8(aasmContext, 0x96);
    else if (!strcmp(rop, "setna"))
        SETCC_RM8(aasmContext, 0x96);
    else if (!strcmp(rop, "seta"))
        SETCC_RM8(aasmContext, 0x97);
    else if (!strcmp(rop, "setnbe"))
        SETCC_RM8(aasmContext, 0x97);
    else if (!strcmp(rop, "sets"))
        SETCC_RM8(aasmContext, 0x98);
    else if (!strcmp(rop, "setns"))
        SETCC_RM8(aasmContext, 0x99);
    else if (!strcmp(rop, "setp"))
        SETCC_RM8(aasmContext, 0x9a);
    else if (!strcmp(rop, "setpe"))
        SETCC_RM8(aasmContext, 0x9a);
    else if (!strcmp(rop, "setnp"))
        SETCC_RM8(aasmContext, 0x9b);
    else if (!strcmp(rop, "setpo"))
        SETCC_RM8(aasmContext, 0x9b);
    else if (!strcmp(rop, "setl"))
        SETCC_RM8(aasmContext, 0x9c);
    else if (!strcmp(rop, "setnge"))
        SETCC_RM8(aasmContext, 0x9c);
    else if (!strcmp(rop, "setge"))
        SETCC_RM8(aasmContext, 0x9d);
    else if (!strcmp(rop, "setnl"))
        SETCC_RM8(aasmContext, 0x9d);
    else if (!strcmp(rop, "setle"))
        SETCC_RM8(aasmContext, 0x9e);
    else if (!strcmp(rop, "setng"))
        SETCC_RM8(aasmContext, 0x9e);
    else if (!strcmp(rop, "setg"))
        SETCC_RM8(aasmContext, 0x9f);
    else if (!strcmp(rop, "setnle"))
        SETCC_RM8(aasmContext, 0x9f);
    else if (!strcmp(rop, "bt"))
        BT(aasmContext);
    else if (!strcmp(rop, "shld"))
        SHLD(aasmContext);
    else if (!strcmp(rop, "bts"))
        BTS(aasmContext);
    else if (!strcmp(rop, "shrd"))
        SHRD(aasmContext);
    else if (!strcmp(rop, "lss"))
        LSS(aasmContext);
    else if (!strcmp(rop, "btr"))
        BTR(aasmContext);
    else if (!strcmp(rop, "lfs"))
        LFS(aasmContext);
    else if (!strcmp(rop, "lgs"))
        LGS(aasmContext);
    else if (!strcmp(rop, "movzx"))
        MOVZX(aasmContext);
    else if (!strcmp(rop, "btc"))
        BTC(aasmContext);
    else if (!strcmp(rop, "bsf"))
        BSF(aasmContext);
    else if (!strcmp(rop, "bsr"))
        BSR(aasmContext);
    else if (!strcmp(rop, "movsx"))
        MOVSX(aasmContext);
    else
        _ser_;
    XASM32_TRACE_CALL_END;
}
static char *take_arg(aasm32_context *aasmContext, char *s)
{
    char *rend, *rresult;
    if (s)
    {
        rstart = s;
    }
    if (!rstart)
    {
        return LIB_NULL;
    }
    while (!is_end(aasmContext, *rstart) && is_space(aasmContext, *rstart))
    {
        rstart++;
    }
    if (*rstart == ',' || is_end(aasmContext, *rstart))
    {
        return LIB_NULL;
    }
    rresult = rstart;
    while (!is_end(aasmContext, *rstart) && (*rstart) != ',')
    {
        rstart++;
    }
    rend = rstart - 1;
    if (is_end(aasmContext, *rstart))
    {
        rstart = LIB_NULL;
    }
    else
    {
        rstart++;
    }
    while (!is_end(aasmContext, *rend) && is_space(aasmContext, *rend))
    {
        rend--;
    }
    *(rend + 1) = 0;
    return rresult;
}
static lib_u8 aasm32_execute(aasm32_context *aasmContext, const char *stmt, lib_u8 *rcode, int flag32)
{
    lib_u8 len;
    char astmt[0x100];
    char *rstmt;
    lib_u8 flagprefix;

    if (!stmt || is_end(aasmContext, stmt[0]))
    {
        return 0;
    }

    memcpy((void *)astmt, (void *)stmt, 0x100);
    xasm32_string_lower(astmt);
    rstmt = astmt;

    defsize = !!flag32;

    prefix_oprsize = prefix_addrsize = 0;
    prefix_lock = prefix_repz = prefix_repnz = 0;
    flagError = 0;

    iop = 0;
    memset((void *)(&aopri1), 0x00, sizeof(t_aasm_oprinfo));
    memset((void *)(&aopri2), 0x00, sizeof(t_aasm_oprinfo));
    memset((void *)(&aopri3), 0x00, sizeof(t_aasm_oprinfo));
    memset((void *)(&aoprig), 0x00, sizeof(t_aasm_oprinfo));
    prefix_oprsizeg = prefix_addrsizeg = 0;

    rop = ropr1 = ropr2 = ropr3 = LIB_NULL;

    /* process prefixes */
    do
    {
        while (!is_end(aasmContext, *rstmt) && is_space(aasmContext, *rstmt))
        {
            rstmt++;
        }
        rop = rstmt;
        while (!is_end(aasmContext, *rstmt) && !is_space(aasmContext, *rstmt))
        {
            rstmt++;
        }
        if (!is_end(aasmContext, *rstmt))
        {
            *rstmt = 0;
            rstmt++;
        }
        flagprefix = is_prefix(aasmContext);
        if (flagprefix)
        {
            exec(aasmContext);
        }
    } while (flagprefix && !flagError);

    /* process assembly statement */
    ropr1 = take_arg(aasmContext, rstmt);
    ropr2 = take_arg(aasmContext, LIB_NULL);
    ropr3 = take_arg(aasmContext, LIB_NULL);

    aopri1 = parsearg(aasmContext, ropr1);
    aopri2 = parsearg(aasmContext, ropr2);
    aopri3 = parsearg(aasmContext, ropr3);

    if (isM(aopri1))
    {
        rinfo = &aopri1;
    }
    else if (isM(aopri2))
    {
        rinfo = &aopri2;
    }
    else if (isM(aopri3))
    {
        rinfo = &aopri3;
    }
    else
    {
        rinfo = LIB_NULL;
    }

    exec(aasmContext);
    len = 0;

    if (!flagError)
    {
        if (prefix_repz)
        {
            (*(rcode + len)) = 0xf3;
            len++;
        }
        if (prefix_repnz)
        {
            (*(rcode + len)) = 0xf2;
            len++;
        }
        if (prefix_lock)
        {
            (*(rcode + len)) = 0xf0;
            len++;
        }
        if ((rinfo && rinfo->flages) || aoprig.flages)
        {
            (*(rcode + len)) = 0x26;
            len++;
        }
        if ((rinfo && rinfo->flagcs) || aoprig.flagcs)
        {
            (*(rcode + len)) = 0x2e;
            len++;
        }
        if ((rinfo && rinfo->flagss) || aoprig.flagss)
        {
            (*(rcode + len)) = 0x36;
            len++;
        }
        if ((rinfo && rinfo->flagds) || aoprig.flagds)
        {
            (*(rcode + len)) = 0x3e;
            len++;
        }
        if ((rinfo && rinfo->flagfs) || aoprig.flagfs)
        {
            (*(rcode + len)) = 0x64;
            len++;
        }
        if ((rinfo && rinfo->flaggs) || aoprig.flaggs)
        {
            (*(rcode + len)) = 0x65;
            len++;
        }
        if (prefix_addrsize || prefix_addrsizeg)
        {
            (*(rcode + len)) = 0x67;
            len++;
        }
        if (prefix_oprsize || prefix_oprsizeg)
        {
            (*(rcode + len)) = 0x66;
            len++;
        }
        memcpy((void *)(rcode + len), (void *)acode, iop);
        len += iop;
    }
    return len;
}

lib_u8 aasm32(const char *stmt, lib_u8 *rcode, int flag32)
{
    aasm32_context local_context;

    memset(&local_context, 0, sizeof(local_context));
    return aasm32_execute(&local_context, stmt, rcode, flag32);
}

/* extended routines - assemble a paragraph with call/jmp labels */
typedef struct
{
    char stmt[0x100];
    lib_u32 stmt_id;
    lib_u8 code_array[15];
    lib_u8 code_len;
    lib_u8 flag_is_label;
    lib_u8 flag_has_label;
    char label_str[0x100];
    char op_str[0x100];
    t_aasm_oprptr ptr;
} t_aasm_instr;
/* default operand size */
#define _GetOperandSize (defsize ? 4 : 2)
static void asmx_get_label(aasm32_context *aasmContext, t_aasm_instr *rinstr)
{
    lib_size i = 0, j = 0;
    rinstr->label_str[0] = 0;
    rinstr->flag_has_label = 0;
    rinstr->flag_is_label = 0;
    while (rinstr->stmt[i] && rinstr->stmt[i] != '$')
    {
        i++;
    }
    if (rinstr->stmt[i] != '$')
    {
        return;
    }
    i++;
    if (rinstr->stmt[i] != '(')
    {
        return;
    }
    i++;
    while (rinstr->stmt[i] && rinstr->stmt[i] != ')')
    {
        rinstr->label_str[j++] = rinstr->stmt[i++];
    }
    rinstr->label_str[j] = 0;
    if (rinstr->stmt[i] != ')')
    {
        return;
    }
    rinstr->flag_has_label = 1;
    if (rinstr->stmt[0] == '$' && rinstr->stmt[1] == '(' &&
        rinstr->stmt[strlen(rinstr->stmt) - 1] == ':' &&
        rinstr->stmt[strlen(rinstr->stmt) - 2] == ')')
    {
        rinstr->flag_is_label = 1;
    }
}
static void asmx_parse_instr(aasm32_context *aasmContext, t_aasm_instr *rinstr)
{
    lib_size i;
    char *rstmt;
    t_aasm_token token;
    i = 0;
    rinstr->code_len = 0;
    while (!is_end(aasmContext, rinstr->stmt[i]) && !is_space(aasmContext, rinstr->stmt[i]))
    {
        rinstr->op_str[i] = rinstr->stmt[i];
        i++;
    }
    rinstr->op_str[i] = 0;
    rstmt = rinstr->stmt + i;
    if (is_end(aasmContext, *rstmt))
    {
        return;
    }
    token = gettoken(aasmContext, rstmt);
    switch (token)
    {
    case TOKEN_SHORT:
        token = gettoken(aasmContext, LIB_NULL);
        if (token == TOKEN_PTR)
        {
            token = gettoken(aasmContext, LIB_NULL);
        }
        if (token != TOKEN_DOLLAR)
        {
            return;
        }
        rinstr->ptr = PTR_SHORT;
        break;
    case TOKEN_NEAR:
        token = gettoken(aasmContext, LIB_NULL);
        if (token == TOKEN_PTR)
        {
            token = gettoken(aasmContext, LIB_NULL);
        }
        if (token != TOKEN_DOLLAR)
        {
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
    if (!strcmp(rinstr->op_str, "loopne") || !strcmp(rinstr->op_str, "loopnz") || !strcmp(rinstr->op_str, "loope") ||
        !strcmp(rinstr->op_str, "loopz") || !strcmp(rinstr->op_str, "loop") || !strcmp(rinstr->op_str, "jcxz"))
    {
        switch (rinstr->ptr)
        {
        case PTR_NONE:
            rinstr->ptr = PTR_SHORT;
        case PTR_SHORT:
            rinstr->code_len = 1 /*opcode*/ + 1 /*rel_imm8*/;
            break;
        default:
            return;
        }
    }
    if (!strcmp(rinstr->op_str, "jo") || !strcmp(rinstr->op_str, "jno") || !strcmp(rinstr->op_str, "jb") ||
        !strcmp(rinstr->op_str, "jc") || !strcmp(rinstr->op_str, "jnae") || !strcmp(rinstr->op_str, "jae") ||
        !strcmp(rinstr->op_str, "jnb") || !strcmp(rinstr->op_str, "jnc") || !strcmp(rinstr->op_str, "je") ||
        !strcmp(rinstr->op_str, "jz") || !strcmp(rinstr->op_str, "jne") || !strcmp(rinstr->op_str, "jnz") ||
        !strcmp(rinstr->op_str, "jbe") || !strcmp(rinstr->op_str, "jna") || !strcmp(rinstr->op_str, "ja") ||
        !strcmp(rinstr->op_str, "jnbe") || !strcmp(rinstr->op_str, "js") || !strcmp(rinstr->op_str, "jns") ||
        !strcmp(rinstr->op_str, "jp") || !strcmp(rinstr->op_str, "jpe") || !strcmp(rinstr->op_str, "jnp") ||
        !strcmp(rinstr->op_str, "jpo") || !strcmp(rinstr->op_str, "jl") || !strcmp(rinstr->op_str, "jnge") ||
        !strcmp(rinstr->op_str, "jge") || !strcmp(rinstr->op_str, "jnl") || !strcmp(rinstr->op_str, "jle") ||
        !strcmp(rinstr->op_str, "jng") || !strcmp(rinstr->op_str, "jg") || !strcmp(rinstr->op_str, "jnle"))
    {
        switch (rinstr->ptr)
        {
        case PTR_NONE:
            rinstr->ptr = PTR_SHORT;
        case PTR_SHORT:
            rinstr->code_len = 1 /*opcode*/ + 1 /*rel_imm8*/;
            break;
        case PTR_NEAR:
            rinstr->code_len = 1 /*0x0f*/ + 1 /*opcode*/ + _GetOperandSize /*rel_immx*/;
            break;
        default:
            return;
        }
    }
    if (!strcmp(rinstr->op_str, "jmp"))
    {
        switch (rinstr->ptr)
        {
        case PTR_SHORT:
            rinstr->code_len = 1 /*opcode*/ + 1 /*rel_imm8*/;
            break;
        case PTR_NONE:
            rinstr->ptr = PTR_NEAR;
        case PTR_NEAR:
            rinstr->code_len = 1 /*opcode*/ + _GetOperandSize /*rel_immx*/;
            break;
        default:
            return;
        }
    }
    if (!strcmp(rinstr->op_str, "call"))
    {
        switch (rinstr->ptr)
        {
        case PTR_NONE:
            rinstr->ptr = PTR_NEAR;
        case PTR_NEAR:
            rinstr->code_len = 1 /*opcode*/ + _GetOperandSize /*rel_immx*/;
            break;
        default:
            return;
        }
    }
}
static lib_status aasm32x_execute(aasm32_context *aasmContext,
    const char *stmt, lib_size code_capacity, lib_u8 *rcode,
    lib_size *out_code_bytes, int flag32)
{
    lib_i32 i, j, k, count;
    lib_size len;
    lib_u32 offset;
    lib_size statement_bytes;
    char imm[0x100];
    t_aasm_instr *instr;
    if (stmt == LIB_NULL || rcode == LIB_NULL || out_code_bytes == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    statement_bytes = strlen(stmt);
    if (statement_bytes > 0x7fffffffu) return LIB_STATUS_INVALID_ARGUMENT;
    count = 1;
    flagError = 0;
    for (i = 0; i < (lib_i32)statement_bytes; ++i)
    {
        if (stmt[i] == '\n')
        {
            if (count == 0x7fffffff) return LIB_STATUS_NO_MEMORY;
            count++;
        }
    }
    if ((lib_size)count > (lib_size)-1 / sizeof(*instr)) {
        return LIB_STATUS_NO_MEMORY;
    }
    instr = (t_aasm_instr *)malloc((lib_size)count * sizeof(*instr));
    if (instr == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    i = j = k = 0;
    while (is_space(aasmContext, stmt[i]))
    {
        i++;
    }
    while (1)
    {
        if (is_end(aasmContext, stmt[i]))
        {
            if (j)
            {
                j--;
                if (j)
                {
                    while (is_space(aasmContext, instr[k].stmt[j]))
                    {
                        j--;
                    }
                    if (j)
                    {
                        instr[k].stmt[j + 1] = 0;
                        xasm32_string_lower(instr[k].stmt);
                        instr[k].stmt_id = (lib_u32)k;
                        j = 0;
                        k++;
                    }
                }
            }
            while (stmt[i] && stmt[i] != '\n')
            {
                i++;
            }
            if (!stmt[i])
            {
                break;
            }
            else if (stmt[i] == '\n')
            {
                i++;
                while (is_space(aasmContext, stmt[i]))
                {
                    i++;
                }
            }
        }
        else
        {
            if (j >= (lib_i32)(sizeof(instr[k].stmt) - 1u)) {
                free((void *)instr);
                return LIB_STATUS_INVALID_ARGUMENT;
            }
            instr[k].stmt[j] = stmt[i];
            i++;
            j++;
        }
    }
    count = k;
    if (count == 0) {
        free((void *)instr);
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    for (i = 0; i < count; ++i)
    {
        memset((void *)instr[i].code_array, 0x00, 15);
        asmx_get_label(aasmContext, &instr[i]);
        if (instr[i].flag_has_label)
        {
            if (instr[i].flag_is_label)
            {
                instr[i].code_array[0] = 0x90;
                instr[i].code_len = 1;
            }
            else
            {
                asmx_parse_instr(aasmContext, &instr[i]);
            }
        }
        else
        {
            instr[i].code_len = aasm32_execute(aasmContext, instr[i].stmt, instr[i].code_array, flag32);
        }
        if (flagError)
        {
            free((void *)instr);
            return LIB_STATUS_UNSUPPORTED;
        }
        if (!instr[i].code_len)
        {
            flagError = 1;
        }
        if (flagError)
        {
            free((void *)instr);
            return LIB_STATUS_UNSUPPORTED;
        }
    }
    /* i: label; j: instr to be materialized; k: size iterator */
    for (i = 0; i < count; ++i)
    {
        if (!instr[i].flag_is_label)
        {
            continue;
        }
        if (i)
        {
            for (j = i - 1; j >= 0; --j)
            {
                if (instr[j].flag_has_label && !strcmp(instr[j].label_str, instr[i].label_str))
                {
                    if (instr[j].flag_is_label)
                    {
                        flagError = 1;
                    }
                    else
                    {
                        offset = 0;
                        /* for (k = j + 1;k < i;++k) {
                            offset += instr[k].code_len;
                        }*/
                        for (k = j + 1; k <= i; ++k)
                        {
                            offset += instr[k].code_len;
                        }
                        switch (instr[j].ptr)
                        {
                        case PTR_SHORT:
                            if (offset < 0x80)
                            {
                                snprintf(imm, sizeof(imm), "short +%02x", (lib_u8)offset);
                            }
                            else
                            {
                                flagError = 1;
                            }
                            break;
                        case PTR_NEAR:
                            switch (_GetOperandSize)
                            {
                            case 2:
                                if (offset < 0x8000)
                                {
                                    snprintf(imm, sizeof(imm), "near +%04x", (lib_u16)offset);
                                }
                                else
                                {
                                    flagError = 1;
                                }
                                break;
                            case 4:
                                if (offset < 0x80000000)
                                {
                                    snprintf(imm, sizeof(imm), "near +%08x", (lib_u32)offset);
                                }
                                else
                                {
                                    flagError = 1;
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
                        if (snprintf(instr[j].stmt, sizeof(instr[j].stmt),
                                "%s %s", instr[j].op_str, imm) < 0 ||
                            strlen(instr[j].op_str) + 1u + strlen(imm) >=
                                sizeof(instr[j].stmt)) {
                            flagError = 1;
                        }
                        aasm32_execute(aasmContext, instr[j].stmt, instr[j].code_array, flag32);
                    }
                    if (flagError)
                    {
                        free((void *)instr);
                        return LIB_STATUS_UNSUPPORTED;
                    }
                }
                if (!j)
                    break;
            }
        }
        if (i != count - 1)
        {
            for (j = i + 1; j < count; ++j)
            {
                if (instr[j].flag_has_label && !strcmp(instr[j].label_str, instr[i].label_str))
                {
                    if (instr[j].flag_is_label)
                    {
                        flagError = 1;
                    }
                    else
                    {
                        offset = 0;
                        for (k = i + 1; k < j + 1; ++k)
                        {
                            offset += instr[k].code_len;
                        }
                        switch (instr[j].ptr)
                        {
                        case PTR_SHORT:
                            if (offset < 0x80)
                            {
                                snprintf(imm, sizeof(imm), "short -%02x", (lib_u8)offset);
                            }
                            else
                            {
                                flagError = 1;
                            }
                            break;
                        case PTR_NEAR:
                            switch (_GetOperandSize)
                            {
                            case 2:
                                if (offset < 0x8000)
                                {
                                    snprintf(imm, sizeof(imm), "near -%04x", (lib_u16)offset);
                                }
                                else
                                {
                                    flagError = 1;
                                }
                                break;
                            case 4:
                                if (offset < 0x80000000)
                                {
                                    snprintf(imm, sizeof(imm), "near -%08x", (lib_u32)offset);
                                }
                                else
                                {
                                    flagError = 1;
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
                        if (snprintf(instr[j].stmt, sizeof(instr[j].stmt),
                                "%s %s", instr[j].op_str, imm) < 0 ||
                            strlen(instr[j].op_str) + 1u + strlen(imm) >=
                                sizeof(instr[j].stmt)) {
                            flagError = 1;
                        }
                        aasm32_execute(aasmContext, instr[j].stmt, instr[j].code_array, flag32);
                    }
                    if (flagError)
                    {
                        free((void *)instr);
                        return LIB_STATUS_UNSUPPORTED;
                    }
                }
            }
        }
    }
    len = 0;
    for (i = 0; i < count; ++i)
    {
        if ((lib_size)instr[i].code_len > code_capacity - len) {
            free((void *)instr);
            return LIB_STATUS_LIMIT_EXCEEDED;
        }
        len += instr[i].code_len;
    }
    len = 0;
    for (i = 0; i < count; ++i)
    {
        memcpy((void *)(rcode + len), (void *)instr[i].code_array,
            instr[i].code_len);
        len += instr[i].code_len;
    }
    free((void *)instr);
    *out_code_bytes = len;
    return LIB_STATUS_OK;
}

lib_status aasm32x(const char *stmt, lib_size code_capacity,
    lib_u8 *rcode, lib_size *out_code_bytes, int flag32)
{
    aasm32_context local_context;

    memset(&local_context, 0, sizeof(local_context));
    return aasm32x_execute(&local_context, stmt, code_capacity, rcode,
        out_code_bytes, flag32);
}
