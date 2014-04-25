/* This file is a part of NXVM project. */

#include "../vapi.h"
#include "../vcpu.h"

#include "aasm32.h"

/* DEBUGGING OPTIONS ******************************************************* */
#define AASM_TRACE 0
/* ************************************************************************* */

#if AASM_TRACE == 1

static t_api_trace_call trace;

#define _cb(s) vapiTraceCallBegin(&trace, s)
#define _ce    vapiTraceCallEnd(&trace)
#define _bb(s) vapiTraceBlockBegin(&trace, s)
#define _be    vapiTraceBlockEnd(&trace)
#define _chb(n)  if (1) {(n);if (flagerror) {trace.flagerror = 1;vapiTraceFinal(&trace);break;   }} while (0)
#define _chk(n)  do     {(n);if (flagerror) {trace.flagerror = 1;vapiTraceFinal(&trace);return;  }} while (0)
#define _chr(n)  do     {(n);if (flagerror) {trace.flagerror = 1;vapiTraceFinal(&trace);return 0;}} while (0)
#define _chrt(n) do     {(n);if (flagerror) {trace.flagerror = 1;vapiTraceFinal(&trace);return token;}} while (0)
#define _chrf(n) do     {(n);if (flagerror) {trace.flagerror = 1;vapiTraceFinal(&trace);return info;}} while (0)
#else
#define _cb(s)
#define _ce
#define _be
#define _bb(s)
#define _chb(n)  if (1) {(n);if (flagerror) break;   } while (0)
#define _chk(n)  do     {(n);if (flagerror) return;  } while (0)
#define _chr(n)  do     {(n);if (flagerror) return 0;} while (0)
#define _chrt(n) do     {(n);if (flagerror) return token;} while (0)
#define _chrf(n) do     {(n);if (flagerror) return info;} while (0)
#endif

/* set error */
#define _se_  _chk(flagerror = 1)
#define _ser_ _chr(flagerror = 1)
#define _sert_ _chrt(flagerror = 1)
#define _serf_ _chrf(flagerror = 1)

/* operand size */
#define _SetOperandSize(n) (prefix_oprsize = (n) ? ((vcpu.cs.seg.exec.defsize ? 4 : 2) != (n)) : 0)
/* address size of the source operand */
#define _SetAddressSize(n) (prefix_addrsize = (n) ? ((vcpu.cs.seg.exec.defsize ? 4 : 2) != (n)) : 0)

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
	MOD_M_DISP32,//
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
} t_aasm_oprreg32;//
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
	t_nubit8 scale;
} t_aasm_oprsib;
typedef struct {
	t_aasm_oprtype  type;
	t_aasm_oprmod   mod;   // active when type = 1,2,3,6,7,8
	                       // 0 = mem; 1 = mem+disp8; 2 = mem+disp16; 3 = reg
	t_aasm_oprmem   mem;   // active when mod = 0,1,2
                           // 0 = [BX+SI], 1 = [BX+DI], 2 = [BP+SI], 3 = [BP+DI],
                           // 4 = [SI], 5 = [DI], 6 = [BP], 7 = [BX]
	t_aasm_oprsib   sib;   // active when mem = MEM_SIB
	t_aasm_oprreg8  reg8;  // active when type = 1, mod = 3
                           // 0 = AL, 1 = CL, 2 = DL, 3 = BL,
                           // 4 = AH, 5 = CH, 6 = DH, 7 = BH
	t_aasm_oprreg16 reg16; // active when type = 2, mod = 3
                           // 0 = AX, 1 = CX, 2 = DX, 3 = BX,
                           // 4 = SP, 5 = BP, 6 = SI, 7 = DI
	t_aasm_oprreg32 reg32; // active when type = 3, mod = 3
                           // 0 = EAX, 1 = ECX, 2 = EDX, 3 = EBX,
                           // 4 = ESP, 5 = EBP, 6 = ESI, 7 = EDI   
	t_aasm_oprsreg  sreg;   // active when type = 3
                           // 0 = ES, 1 = CS, 2 = SS, 3 = DS, 4 = FS, 5 = GS
	t_aasm_oprcreg  creg;
	t_aasm_oprdreg  dreg;
	t_aasm_oprtreg  treg;
	t_bool          imms;  // if imm is signed
	t_bool          immn;  // if imm is negative
	t_nubit8        imm8;
	t_nubit16       imm16;
	t_nubit32       imm32;
	t_nsbit8        disp8;
	t_nubit16       disp16;// use as imm when type = 6; use by modrm as disp when mod = 0,1,2;
	t_nubit32       disp32;// use as imm when type = 7; use by modrm as disp when mod = 0,1,2;
	t_aasm_oprptr   ptr; // 0 = near; 1 = far
	t_nubit16       rcs;
	t_nubit32       reip;
	char            label[0x100];
	t_bool flages, flagcs, flagss, flagds, flagfs, flaggs;
} t_aasm_oprinfo;
/* global variables */

typedef t_bool t_aasm_prefix;

static t_aasm_prefix prefix_oprsizeg, prefix_addrsizeg;
static t_aasm_prefix prefix_oprsize, prefix_addrsize;
static t_aasm_prefix prefix_lock, prefix_repz, prefix_repnz;
static t_nubit8 acode[15];
static t_nubit8 iop;
static t_strptr rop, ropr1, ropr2, ropr3;
static t_nubit16 avcs, avip;
static t_strptr aop, aopr1, aopr2;
static t_bool flagerror;
static t_aasm_oprinfo aoprig, aopri1, aopri2, aopri3;
static t_aasm_oprinfo *rinfo = NULL;
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
static t_nubit8 tokimm8;
static t_nubit16 tokimm16;
static t_nubit32 tokimm32;
static t_nsbit8 tokchar;
t_string tokstring, toklabel;
#define tokch  (*tokptr)
#define take(n) (flagend = 1, token = (n))
static t_aasm_token gettoken(t_strptr str)
{
	static t_strptr tokptr = NULL;
	t_nubit8 toklen = 0;
	t_nubit32 tokimm = 0;
	t_bool flagend = 0;
	t_aasm_token token = TOKEN_NULL;
	t_aasm_scan_state state = STATE_START;
	t_strptr tokptrbak;
	_cb("gettoken");
	tokimm8 = 0x00;
	tokimm16 = 0x0000;
	tokimm32 = 0x00000000;
	if (str) tokptr = str;
	if (!tokptr) {
		_ce;
		return token;
	}
	tokptrbak = tokptr;
	do {
		switch (state) {
		case STATE_START: _bb("state(STATE_START)");
			switch (tokch) {
			case '[': take(TOKEN_LSPAREN); break;
			case ']': take(TOKEN_RSPAREN); break;
			case ':': take(TOKEN_COLON);   break;
			case '+': take(TOKEN_PLUS);    break;
			case '-': take(TOKEN_MINUS);   break;
			case '*': take(TOKEN_TIMES);   break;
			case '$': take(TOKEN_DOLLAR);  break;
			case '0': tokimm = 0x0;toklen = 1;state = STATE_NUM1;break;
			case '1': tokimm = 0x1;toklen = 1;state = STATE_NUM1;break;
			case '2': tokimm = 0x2;toklen = 1;state = STATE_NUM1;break;
			case '3': tokimm = 0x3;toklen = 1;state = STATE_NUM1;break;
			case '4': tokimm = 0x4;toklen = 1;state = STATE_NUM1;break;
			case '5': tokimm = 0x5;toklen = 1;state = STATE_NUM1;break;
			case '6': tokimm = 0x6;toklen = 1;state = STATE_NUM1;break;
			case '7': tokimm = 0x7;toklen = 1;state = STATE_NUM1;break;
			case '8': tokimm = 0x8;toklen = 1;state = STATE_NUM1;break;
			case '9': tokimm = 0x9;toklen = 1;state = STATE_NUM1;break;
			case 'a': tokimm = 0xa;toklen = 1;state = STATE_A;break;
			case 'b': tokimm = 0xb;toklen = 1;state = STATE_B;break;
			case 'c': tokimm = 0xc;toklen = 1;state = STATE_C;break;
			case 'd': tokimm = 0xd;toklen = 1;state = STATE_D;break;
			case 'e': tokimm = 0xe;toklen = 1;state = STATE_E;break;
			case 'f': tokimm = 0xf;toklen = 1;state = STATE_F;break;
			case 'g': tokimm = 0xf;toklen = 1;state = STATE_G;break;
			case 'n': state = STATE_N;break;
			case 'p': state = STATE_P;break;
			case 's': state = STATE_S;break;
			case 't': state = STATE_T;break;
			case 'w': state = STATE_W;break;
			case ' ':
			case '\t': break;
			case '\0': tokptr--;take(TOKEN_END);break;
			default: tokptr--;_sert_;break;
			}
			_be;break;
		case STATE_NUM1: _bb("state(STATE_NUM1)");
			switch (tokch) {
			case '0': tokimm = (tokimm << 4) | 0x0;toklen = 2;state = STATE_NUM2;break;
			case '1': tokimm = (tokimm << 4) | 0x1;toklen = 2;state = STATE_NUM2;break;
			case '2': tokimm = (tokimm << 4) | 0x2;toklen = 2;state = STATE_NUM2;break;
			case '3': tokimm = (tokimm << 4) | 0x3;toklen = 2;state = STATE_NUM2;break;
			case '4': tokimm = (tokimm << 4) | 0x4;toklen = 2;state = STATE_NUM2;break;
			case '5': tokimm = (tokimm << 4) | 0x5;toklen = 2;state = STATE_NUM2;break;
			case '6': tokimm = (tokimm << 4) | 0x6;toklen = 2;state = STATE_NUM2;break;
			case '7': tokimm = (tokimm << 4) | 0x7;toklen = 2;state = STATE_NUM2;break;
			case '8': tokimm = (tokimm << 4) | 0x8;toklen = 2;state = STATE_NUM2;break;
			case '9': tokimm = (tokimm << 4) | 0x9;toklen = 2;state = STATE_NUM2;break;
			case 'a': tokimm = (tokimm << 4) | 0xa;toklen = 2;state = STATE_NUM2;break;
			case 'b': tokimm = (tokimm << 4) | 0xb;toklen = 2;state = STATE_NUM2;break;
			case 'c': tokimm = (tokimm << 4) | 0xc;toklen = 2;state = STATE_NUM2;break;
			case 'd': tokimm = (tokimm << 4) | 0xd;toklen = 2;state = STATE_NUM2;break;
			case 'e': tokimm = (tokimm << 4) | 0xe;toklen = 2;state = STATE_NUM2;break;
			case 'f': tokimm = (tokimm << 4) | 0xf;toklen = 2;state = STATE_NUM2;break;
			default: tokptr--;_sert_;break;
			}
			_be;break;
		case STATE_NUM2: _bb("state(STATE_NUM2)");
			switch (tokch) {
			case '0': tokimm = (tokimm << 4) | 0x0;toklen = 3;state = STATE_NUM3;break;
			case '1': tokimm = (tokimm << 4) | 0x1;toklen = 3;state = STATE_NUM3;break;
			case '2': tokimm = (tokimm << 4) | 0x2;toklen = 3;state = STATE_NUM3;break;
			case '3': tokimm = (tokimm << 4) | 0x3;toklen = 3;state = STATE_NUM3;break;
			case '4': tokimm = (tokimm << 4) | 0x4;toklen = 3;state = STATE_NUM3;break;
			case '5': tokimm = (tokimm << 4) | 0x5;toklen = 3;state = STATE_NUM3;break;
			case '6': tokimm = (tokimm << 4) | 0x6;toklen = 3;state = STATE_NUM3;break;
			case '7': tokimm = (tokimm << 4) | 0x7;toklen = 3;state = STATE_NUM3;break;
			case '8': tokimm = (tokimm << 4) | 0x8;toklen = 3;state = STATE_NUM3;break;
			case '9': tokimm = (tokimm << 4) | 0x9;toklen = 3;state = STATE_NUM3;break;
			case 'a': tokimm = (tokimm << 4) | 0xa;toklen = 3;state = STATE_NUM3;break;
			case 'b': tokimm = (tokimm << 4) | 0xb;toklen = 3;state = STATE_NUM3;break;
			case 'c': tokimm = (tokimm << 4) | 0xc;toklen = 3;state = STATE_NUM3;break;
			case 'd': tokimm = (tokimm << 4) | 0xd;toklen = 3;state = STATE_NUM3;break;
			case 'e': tokimm = (tokimm << 4) | 0xe;toklen = 3;state = STATE_NUM3;break;
			case 'f': tokimm = (tokimm << 4) | 0xf;toklen = 3;state = STATE_NUM3;break;
			default: tokptr--;tokimm8 = GetMax8(tokimm);take(TOKEN_IMM8);break;
			}
			_be;break;
		case STATE_NUM3: _bb("state(STATE_NUM3)");
			switch (tokch) {
			case '0': tokimm = (tokimm << 4) | 0x0;toklen = 4;state = STATE_NUM4;break;
			case '1': tokimm = (tokimm << 4) | 0x1;toklen = 4;state = STATE_NUM4;break;
			case '2': tokimm = (tokimm << 4) | 0x2;toklen = 4;state = STATE_NUM4;break;
			case '3': tokimm = (tokimm << 4) | 0x3;toklen = 4;state = STATE_NUM4;break;
			case '4': tokimm = (tokimm << 4) | 0x4;toklen = 4;state = STATE_NUM4;break;
			case '5': tokimm = (tokimm << 4) | 0x5;toklen = 4;state = STATE_NUM4;break;
			case '6': tokimm = (tokimm << 4) | 0x6;toklen = 4;state = STATE_NUM4;break;
			case '7': tokimm = (tokimm << 4) | 0x7;toklen = 4;state = STATE_NUM4;break;
			case '8': tokimm = (tokimm << 4) | 0x8;toklen = 4;state = STATE_NUM4;break;
			case '9': tokimm = (tokimm << 4) | 0x9;toklen = 4;state = STATE_NUM4;break;
			case 'a': tokimm = (tokimm << 4) | 0xa;toklen = 4;state = STATE_NUM4;break;
			case 'b': tokimm = (tokimm << 4) | 0xb;toklen = 4;state = STATE_NUM4;break;
			case 'c': tokimm = (tokimm << 4) | 0xc;toklen = 4;state = STATE_NUM4;break;
			case 'd': tokimm = (tokimm << 4) | 0xd;toklen = 4;state = STATE_NUM4;break;
			case 'e': tokimm = (tokimm << 4) | 0xe;toklen = 4;state = STATE_NUM4;break;
			case 'f': tokimm = (tokimm << 4) | 0xf;toklen = 4;state = STATE_NUM4;break;
			default: tokptr--;_sert_;break;
			}
			_be;break;
		case STATE_NUM4: _bb("state(STATE_NUM4)");
			switch (tokch) {
			case '0': tokimm = (tokimm << 4) | 0x0;toklen = 5;state = STATE_NUM5;break;
			case '1': tokimm = (tokimm << 4) | 0x1;toklen = 5;state = STATE_NUM5;break;
			case '2': tokimm = (tokimm << 4) | 0x2;toklen = 5;state = STATE_NUM5;break;
			case '3': tokimm = (tokimm << 4) | 0x3;toklen = 5;state = STATE_NUM5;break;
			case '4': tokimm = (tokimm << 4) | 0x4;toklen = 5;state = STATE_NUM5;break;
			case '5': tokimm = (tokimm << 4) | 0x5;toklen = 5;state = STATE_NUM5;break;
			case '6': tokimm = (tokimm << 4) | 0x6;toklen = 5;state = STATE_NUM5;break;
			case '7': tokimm = (tokimm << 4) | 0x7;toklen = 5;state = STATE_NUM5;break;
			case '8': tokimm = (tokimm << 4) | 0x8;toklen = 5;state = STATE_NUM5;break;
			case '9': tokimm = (tokimm << 4) | 0x9;toklen = 5;state = STATE_NUM5;break;
			case 'a': tokimm = (tokimm << 4) | 0xa;toklen = 5;state = STATE_NUM5;break;
			case 'b': tokimm = (tokimm << 4) | 0xb;toklen = 5;state = STATE_NUM5;break;
			case 'c': tokimm = (tokimm << 4) | 0xc;toklen = 5;state = STATE_NUM5;break;
			case 'd': tokimm = (tokimm << 4) | 0xd;toklen = 5;state = STATE_NUM5;break;
			case 'e': tokimm = (tokimm << 4) | 0xe;toklen = 5;state = STATE_NUM5;break;
			case 'f': tokimm = (tokimm << 4) | 0xf;toklen = 5;state = STATE_NUM5;break;
			default: tokptr--;tokimm16 = GetMax16(tokimm);take(TOKEN_IMM16);break;
			}
			_be;break;
		case STATE_NUM5: _bb("state(STATE_NUM5)");
			switch (tokch) {
			case '0': tokimm = (tokimm << 4) | 0x0;toklen = 6;state = STATE_NUM6;break;
			case '1': tokimm = (tokimm << 4) | 0x1;toklen = 6;state = STATE_NUM6;break;
			case '2': tokimm = (tokimm << 4) | 0x2;toklen = 6;state = STATE_NUM6;break;
			case '3': tokimm = (tokimm << 4) | 0x3;toklen = 6;state = STATE_NUM6;break;
			case '4': tokimm = (tokimm << 4) | 0x4;toklen = 6;state = STATE_NUM6;break;
			case '5': tokimm = (tokimm << 4) | 0x5;toklen = 6;state = STATE_NUM6;break;
			case '6': tokimm = (tokimm << 4) | 0x6;toklen = 6;state = STATE_NUM6;break;
			case '7': tokimm = (tokimm << 4) | 0x7;toklen = 6;state = STATE_NUM6;break;
			case '8': tokimm = (tokimm << 4) | 0x8;toklen = 6;state = STATE_NUM6;break;
			case '9': tokimm = (tokimm << 4) | 0x9;toklen = 6;state = STATE_NUM6;break;
			case 'a': tokimm = (tokimm << 4) | 0xa;toklen = 6;state = STATE_NUM6;break;
			case 'b': tokimm = (tokimm << 4) | 0xb;toklen = 6;state = STATE_NUM6;break;
			case 'c': tokimm = (tokimm << 4) | 0xc;toklen = 6;state = STATE_NUM6;break;
			case 'd': tokimm = (tokimm << 4) | 0xd;toklen = 6;state = STATE_NUM6;break;
			case 'e': tokimm = (tokimm << 4) | 0xe;toklen = 6;state = STATE_NUM6;break;
			case 'f': tokimm = (tokimm << 4) | 0xf;toklen = 6;state = STATE_NUM6;break;
			default: tokptr--;_sert_;break;
			}
			_be;break;
		case STATE_NUM6: _bb("state(STATE_NUM6)");
			switch (tokch) {
			case '0': tokimm = (tokimm << 4) | 0x0;toklen = 7;state = STATE_NUM7;break;
			case '1': tokimm = (tokimm << 4) | 0x1;toklen = 7;state = STATE_NUM7;break;
			case '2': tokimm = (tokimm << 4) | 0x2;toklen = 7;state = STATE_NUM7;break;
			case '3': tokimm = (tokimm << 4) | 0x3;toklen = 7;state = STATE_NUM7;break;
			case '4': tokimm = (tokimm << 4) | 0x4;toklen = 7;state = STATE_NUM7;break;
			case '5': tokimm = (tokimm << 4) | 0x5;toklen = 7;state = STATE_NUM7;break;
			case '6': tokimm = (tokimm << 4) | 0x6;toklen = 7;state = STATE_NUM7;break;
			case '7': tokimm = (tokimm << 4) | 0x7;toklen = 7;state = STATE_NUM7;break;
			case '8': tokimm = (tokimm << 4) | 0x8;toklen = 7;state = STATE_NUM7;break;
			case '9': tokimm = (tokimm << 4) | 0x9;toklen = 7;state = STATE_NUM7;break;
			case 'a': tokimm = (tokimm << 4) | 0xa;toklen = 7;state = STATE_NUM7;break;
			case 'b': tokimm = (tokimm << 4) | 0xb;toklen = 7;state = STATE_NUM7;break;
			case 'c': tokimm = (tokimm << 4) | 0xc;toklen = 7;state = STATE_NUM7;break;
			case 'd': tokimm = (tokimm << 4) | 0xd;toklen = 7;state = STATE_NUM7;break;
			case 'e': tokimm = (tokimm << 4) | 0xe;toklen = 7;state = STATE_NUM7;break;
			case 'f': tokimm = (tokimm << 4) | 0xf;toklen = 7;state = STATE_NUM7;break;
			default: tokptr--;_sert_;break;
			}
			_be;break;
		case STATE_NUM7: _bb("state(STATE_NUM7)");
			switch (tokch) {
			case '0': tokimm = (tokimm << 4) | 0x0;toklen = 8;state = STATE_NUM8;break;
			case '1': tokimm = (tokimm << 4) | 0x1;toklen = 8;state = STATE_NUM8;break;
			case '2': tokimm = (tokimm << 4) | 0x2;toklen = 8;state = STATE_NUM8;break;
			case '3': tokimm = (tokimm << 4) | 0x3;toklen = 8;state = STATE_NUM8;break;
			case '4': tokimm = (tokimm << 4) | 0x4;toklen = 8;state = STATE_NUM8;break;
			case '5': tokimm = (tokimm << 4) | 0x5;toklen = 8;state = STATE_NUM8;break;
			case '6': tokimm = (tokimm << 4) | 0x6;toklen = 8;state = STATE_NUM8;break;
			case '7': tokimm = (tokimm << 4) | 0x7;toklen = 8;state = STATE_NUM8;break;
			case '8': tokimm = (tokimm << 4) | 0x8;toklen = 8;state = STATE_NUM8;break;
			case '9': tokimm = (tokimm << 4) | 0x9;toklen = 8;state = STATE_NUM8;break;
			case 'a': tokimm = (tokimm << 4) | 0xa;toklen = 8;state = STATE_NUM8;break;
			case 'b': tokimm = (tokimm << 4) | 0xb;toklen = 8;state = STATE_NUM8;break;
			case 'c': tokimm = (tokimm << 4) | 0xc;toklen = 8;state = STATE_NUM8;break;
			case 'd': tokimm = (tokimm << 4) | 0xd;toklen = 8;state = STATE_NUM8;break;
			case 'e': tokimm = (tokimm << 4) | 0xe;toklen = 8;state = STATE_NUM8;break;
			case 'f': tokimm = (tokimm << 4) | 0xf;toklen = 8;state = STATE_NUM8;break;
			default: tokptr--;_sert_;break;
			}
			_be;break;
		case STATE_NUM8: _bb("state(STATE_NUM8)");
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
				tokptr--;_sert_;break;
				break;
			default: tokptr--;tokimm32 = GetMax32(tokimm);take(TOKEN_IMM32);break;
			}
			_be;break;
		case STATE_A: _bb("state(STATE_A)");
			switch (tokch) {
			case '0': tokimm = (tokimm << 4) | 0x0;toklen = 2;state = STATE_NUM2;break;
			case '1': tokimm = (tokimm << 4) | 0x1;toklen = 2;state = STATE_NUM2;break;
			case '2': tokimm = (tokimm << 4) | 0x2;toklen = 2;state = STATE_NUM2;break;
			case '3': tokimm = (tokimm << 4) | 0x3;toklen = 2;state = STATE_NUM2;break;
			case '4': tokimm = (tokimm << 4) | 0x4;toklen = 2;state = STATE_NUM2;break;
			case '5': tokimm = (tokimm << 4) | 0x5;toklen = 2;state = STATE_NUM2;break;
			case '6': tokimm = (tokimm << 4) | 0x6;toklen = 2;state = STATE_NUM2;break;
			case '7': tokimm = (tokimm << 4) | 0x7;toklen = 2;state = STATE_NUM2;break;
			case '8': tokimm = (tokimm << 4) | 0x8;toklen = 2;state = STATE_NUM2;break;
			case '9': tokimm = (tokimm << 4) | 0x9;toklen = 2;state = STATE_NUM2;break;
			case 'a': tokimm = (tokimm << 4) | 0xa;toklen = 2;state = STATE_NUM2;break;
			case 'b': tokimm = (tokimm << 4) | 0xb;toklen = 2;state = STATE_NUM2;break;
			case 'c': tokimm = (tokimm << 4) | 0xc;toklen = 2;state = STATE_NUM2;break;
			case 'd': tokimm = (tokimm << 4) | 0xd;toklen = 2;state = STATE_NUM2;break;
			case 'e': tokimm = (tokimm << 4) | 0xe;toklen = 2;state = STATE_NUM2;break;
			case 'f': tokimm = (tokimm << 4) | 0xf;toklen = 2;state = STATE_NUM2;break;
			case 'x': take(TOKEN_AX);break;
			case 'h': take(TOKEN_AH);break;
			case 'l': take(TOKEN_AL);break;
			default: tokptr--;_sert_;break;
			}
			_be;break;
		case STATE_B: _bb("state(STATE_B)");
			switch (tokch) {
			case '0': tokimm = (tokimm << 4) | 0x0;toklen = 2;state = STATE_NUM2;break;
			case '1': tokimm = (tokimm << 4) | 0x1;toklen = 2;state = STATE_NUM2;break;
			case '2': tokimm = (tokimm << 4) | 0x2;toklen = 2;state = STATE_NUM2;break;
			case '3': tokimm = (tokimm << 4) | 0x3;toklen = 2;state = STATE_NUM2;break;
			case '4': tokimm = (tokimm << 4) | 0x4;toklen = 2;state = STATE_NUM2;break;
			case '5': tokimm = (tokimm << 4) | 0x5;toklen = 2;state = STATE_NUM2;break;
			case '6': tokimm = (tokimm << 4) | 0x6;toklen = 2;state = STATE_NUM2;break;
			case '7': tokimm = (tokimm << 4) | 0x7;toklen = 2;state = STATE_NUM2;break;
			case '8': tokimm = (tokimm << 4) | 0x8;toklen = 2;state = STATE_NUM2;break;
			case '9': tokimm = (tokimm << 4) | 0x9;toklen = 2;state = STATE_NUM2;break;
			case 'a': tokimm = (tokimm << 4) | 0xa;toklen = 2;state = STATE_NUM2;break;
			case 'b': tokimm = (tokimm << 4) | 0xb;toklen = 2;state = STATE_NUM2;break;
			case 'c': tokimm = (tokimm << 4) | 0xc;toklen = 2;state = STATE_NUM2;break;
			case 'd': tokimm = (tokimm << 4) | 0xd;toklen = 2;state = STATE_NUM2;break;
			case 'e': tokimm = (tokimm << 4) | 0xe;toklen = 2;state = STATE_NUM2;break;
			case 'f': tokimm = (tokimm << 4) | 0xf;toklen = 2;state = STATE_NUM2;break;
			case 'x': take(TOKEN_BX);break;
			case 'h': take(TOKEN_BH);break;
			case 'l': take(TOKEN_BL);break;
			case 'p': take(TOKEN_BP);break;
			case 'y': state = STATE_BY;break;
			default: tokptr--;_sert_;break;
			}
			_be;break;
		case STATE_C: _bb("state(STATE_C)");
			switch (tokch) {
			case '0': tokimm = (tokimm << 4) | 0x0;toklen = 2;state = STATE_NUM2;break;
			case '1': tokimm = (tokimm << 4) | 0x1;toklen = 2;state = STATE_NUM2;break;
			case '2': tokimm = (tokimm << 4) | 0x2;toklen = 2;state = STATE_NUM2;break;
			case '3': tokimm = (tokimm << 4) | 0x3;toklen = 2;state = STATE_NUM2;break;
			case '4': tokimm = (tokimm << 4) | 0x4;toklen = 2;state = STATE_NUM2;break;
			case '5': tokimm = (tokimm << 4) | 0x5;toklen = 2;state = STATE_NUM2;break;
			case '6': tokimm = (tokimm << 4) | 0x6;toklen = 2;state = STATE_NUM2;break;
			case '7': tokimm = (tokimm << 4) | 0x7;toklen = 2;state = STATE_NUM2;break;
			case '8': tokimm = (tokimm << 4) | 0x8;toklen = 2;state = STATE_NUM2;break;
			case '9': tokimm = (tokimm << 4) | 0x9;toklen = 2;state = STATE_NUM2;break;
			case 'a': tokimm = (tokimm << 4) | 0xa;toklen = 2;state = STATE_NUM2;break;
			case 'b': tokimm = (tokimm << 4) | 0xb;toklen = 2;state = STATE_NUM2;break;
			case 'c': tokimm = (tokimm << 4) | 0xc;toklen = 2;state = STATE_NUM2;break;
			case 'd': tokimm = (tokimm << 4) | 0xd;toklen = 2;state = STATE_NUM2;break;
			case 'e': tokimm = (tokimm << 4) | 0xe;toklen = 2;state = STATE_NUM2;break;
			case 'f': tokimm = (tokimm << 4) | 0xf;toklen = 2;state = STATE_NUM2;break;
			case 'x': take(TOKEN_CX);break;
			case 'h': take(TOKEN_CH);break;
			case 'l': take(TOKEN_CL);break;
			case 'r': state = STATE_CR;break;
			case 's': take(TOKEN_CS);break;
			default: tokptr--;_sert_;break;
			}
			_be;break;
		case STATE_D: _bb("state(STATE_D)");
			switch (tokch) {
			case '0': tokimm = (tokimm << 4) | 0x0;toklen = 2;state = STATE_NUM2;break;
			case '1': tokimm = (tokimm << 4) | 0x1;toklen = 2;state = STATE_NUM2;break;
			case '2': tokimm = (tokimm << 4) | 0x2;toklen = 2;state = STATE_NUM2;break;
			case '3': tokimm = (tokimm << 4) | 0x3;toklen = 2;state = STATE_NUM2;break;
			case '4': tokimm = (tokimm << 4) | 0x4;toklen = 2;state = STATE_NUM2;break;
			case '5': tokimm = (tokimm << 4) | 0x5;toklen = 2;state = STATE_NUM2;break;
			case '6': tokimm = (tokimm << 4) | 0x6;toklen = 2;state = STATE_NUM2;break;
			case '7': tokimm = (tokimm << 4) | 0x7;toklen = 2;state = STATE_NUM2;break;
			case '8': tokimm = (tokimm << 4) | 0x8;toklen = 2;state = STATE_NUM2;break;
			case '9': tokimm = (tokimm << 4) | 0x9;toklen = 2;state = STATE_NUM2;break;
			case 'a': tokimm = (tokimm << 4) | 0xa;toklen = 2;state = STATE_NUM2;break;
			case 'b': tokimm = (tokimm << 4) | 0xb;toklen = 2;state = STATE_NUM2;break;
			case 'c': tokimm = (tokimm << 4) | 0xc;toklen = 2;state = STATE_NUM2;break;
			case 'd': tokimm = (tokimm << 4) | 0xd;toklen = 2;state = STATE_NUM2;break;
			case 'e': tokimm = (tokimm << 4) | 0xe;toklen = 2;state = STATE_NUM2;break;
			case 'f': tokimm = (tokimm << 4) | 0xf;toklen = 2;state = STATE_NUM2;break;
			case 'x': take(TOKEN_DX);break;
			case 'h': take(TOKEN_DH);break;
			case 'l': take(TOKEN_DL);break;
			case 'r': state = STATE_DR;break;
			case 's': take(TOKEN_DS);break;
			case 'i': take(TOKEN_DI);break;
			case 'w': state = STATE_DW;break;
			default: tokptr--;_sert_;break;
			}
			_be;break;
		case STATE_E: _bb("state(STATE_E)");
			switch (tokch) {
			case '0': tokimm = (tokimm << 4) | 0x0;toklen = 2;state = STATE_NUM2;break;
			case '1': tokimm = (tokimm << 4) | 0x1;toklen = 2;state = STATE_NUM2;break;
			case '2': tokimm = (tokimm << 4) | 0x2;toklen = 2;state = STATE_NUM2;break;
			case '3': tokimm = (tokimm << 4) | 0x3;toklen = 2;state = STATE_NUM2;break;
			case '4': tokimm = (tokimm << 4) | 0x4;toklen = 2;state = STATE_NUM2;break;
			case '5': tokimm = (tokimm << 4) | 0x5;toklen = 2;state = STATE_NUM2;break;
			case '6': tokimm = (tokimm << 4) | 0x6;toklen = 2;state = STATE_NUM2;break;
			case '7': tokimm = (tokimm << 4) | 0x7;toklen = 2;state = STATE_NUM2;break;
			case '8': tokimm = (tokimm << 4) | 0x8;toklen = 2;state = STATE_NUM2;break;
			case '9': tokimm = (tokimm << 4) | 0x9;toklen = 2;state = STATE_NUM2;break;
			case 'a': tokimm = (tokimm << 4) | 0xa;toklen = 2;state = STATE_EA;  break;
			case 'b': tokimm = (tokimm << 4) | 0xb;toklen = 2;state = STATE_EB;  break;
			case 'c': tokimm = (tokimm << 4) | 0xc;toklen = 2;state = STATE_EC;  break;
			case 'd': tokimm = (tokimm << 4) | 0xd;toklen = 2;state = STATE_ED;  break;
			case 'e': tokimm = (tokimm << 4) | 0xe;toklen = 2;state = STATE_NUM2;break;
			case 'f': tokimm = (tokimm << 4) | 0xf;toklen = 2;state = STATE_NUM2;break;
			case 's': state = STATE_ES;break;
			default: tokptr--;_sert_;break;
			}
			_be;break;
		case STATE_F: _bb("state(STATE_F)");
			switch (tokch) {
			case '0': tokimm = (tokimm << 4) | 0x0;toklen = 2;state = STATE_NUM2;break;
			case '1': tokimm = (tokimm << 4) | 0x1;toklen = 2;state = STATE_NUM2;break;
			case '2': tokimm = (tokimm << 4) | 0x2;toklen = 2;state = STATE_NUM2;break;
			case '3': tokimm = (tokimm << 4) | 0x3;toklen = 2;state = STATE_NUM2;break;
			case '4': tokimm = (tokimm << 4) | 0x4;toklen = 2;state = STATE_NUM2;break;
			case '5': tokimm = (tokimm << 4) | 0x5;toklen = 2;state = STATE_NUM2;break;
			case '6': tokimm = (tokimm << 4) | 0x6;toklen = 2;state = STATE_NUM2;break;
			case '7': tokimm = (tokimm << 4) | 0x7;toklen = 2;state = STATE_NUM2;break;
			case '8': tokimm = (tokimm << 4) | 0x8;toklen = 2;state = STATE_NUM2;break;
			case '9': tokimm = (tokimm << 4) | 0x9;toklen = 2;state = STATE_NUM2;break;
			case 'a': tokimm = (tokimm << 4) | 0xa;toklen = 2;state = STATE_FA;  break;
			case 'b': tokimm = (tokimm << 4) | 0xb;toklen = 2;state = STATE_NUM2;break;
			case 'c': tokimm = (tokimm << 4) | 0xc;toklen = 2;state = STATE_NUM2;break;
			case 'd': tokimm = (tokimm << 4) | 0xd;toklen = 2;state = STATE_NUM2;break;
			case 'e': tokimm = (tokimm << 4) | 0xe;toklen = 2;state = STATE_NUM2;break;
			case 'f': tokimm = (tokimm << 4) | 0xf;toklen = 2;state = STATE_NUM2;break;
			case 's': take(TOKEN_FS);break;
			default: tokptr--;_sert_;break;
			}
			_be;break;
		case STATE_G: _bb("state(STATE_G)");
			switch (tokch) {
			case 's': take(TOKEN_GS);break;
			default: tokptr--;_sert_;break;
			}
			_be;break;
		case STATE_N: _bb("state(STATE_N)");
			switch (tokch) {
			case 'e': state = STATE_NE;break;
			default: tokptr--;_sert_;break;
			}
			_be;break;
		case STATE_P: _bb("state(STATE_P)");
			switch (tokch) {
			case 't': state = STATE_PT;break;
			default: tokptr--;_sert_;break;
			}
			_be;break;
		case STATE_S: _bb("state(STATE_S)");
			switch (tokch) {
			case 'i': take(TOKEN_SI);break;
			case 'p': take(TOKEN_SP);break;
			case 's': take(TOKEN_SS);break;
			case 'h': state = STATE_SH;break;
			default: tokptr--;_sert_;break;
			}
			_be;break;
		case STATE_T: _bb("state(STATE_T)");
			switch (tokch) {
			case 'r': state = STATE_TR;break;
			default: tokptr--;_sert_;break;
			}
			_be;break;
		case STATE_W: _bb("state(STATE_W)");
			switch (tokch) {
			case 'o': state = STATE_WO;break;
			default: tokptr--;_sert_;break;
			}
			_be;break;
		case STATE_BY: _bb("state(STATE_BY)");
			switch (tokch) {
			case 't': state = STATE_BYT;break;
			default: tokptr--;_sert_;break;
			}
			_be;break;
		case STATE_CR: _bb("state(STATE_CR)");
			switch (tokch) {
			case '0': take(TOKEN_CR0);break;
			case '2': take(TOKEN_CR2);break;
			case '3': take(TOKEN_CR3);break;
			default: tokptr--;_sert_;break;
			}
			_be;break;
		case STATE_DR: _bb("state(STATE_DR)");
			switch (tokch) {
			case '0': take(TOKEN_DR0);break;
			case '1': take(TOKEN_DR1);break;
			case '2': take(TOKEN_DR2);break;
			case '3': take(TOKEN_DR3);break;
			case '6': take(TOKEN_DR6);break;
			case '7': take(TOKEN_DR7);break;
			default: tokptr--;_sert_;break;
			}
			_be;break;
		case STATE_DW: _bb("state(STATE_DW)");
			switch (tokch) {
			case 'o': state = STATE_DWO;break;
			default: tokptr--;_sert_;break;
			}
			_be;break;
		case STATE_EA: _bb("state(STATE_EA)");
			switch (tokch) {
			case 'x': take(TOKEN_EAX);break;
			default: tokptr--;tokimm8 = GetMax8(tokimm);take(TOKEN_IMM8);break;
			}
			_be;break;
		case STATE_EB: _bb("state(STATE_EB)");
			switch (tokch) {
			case 'p': take(TOKEN_EBP);break;
			case 'x': take(TOKEN_EBX);break;
			default: tokptr--;tokimm8 = GetMax8(tokimm);take(TOKEN_IMM8);break;
			}
			_be;break;
		case STATE_EC: _bb("state(STATE_EC)");
			switch (tokch) {
			case 'x': take(TOKEN_ECX);break;
			default: tokptr--;tokimm8 = GetMax8(tokimm);take(TOKEN_IMM8);break;
			}
			_be;break;
		case STATE_ED: _bb("state(STATE_ED)");
			switch (tokch) {
			case 'i': take(TOKEN_EDI);break;
			case 'x': take(TOKEN_EDX);break;
			default: tokptr--;tokimm8 = GetMax8(tokimm);take(TOKEN_IMM8);break;
			}
			_be;break;
		case STATE_ES: _bb("state(STATE_ES)");
			switch (tokch) {
			case 'i': take(TOKEN_ESI);break;
			case 'p': take(TOKEN_ESP);break;
			default: tokptr--;take(TOKEN_ES);break;
			}
			_be;break;
		case STATE_FA: _bb("state(STATE_FA)");
			switch (tokch) {
			case '0': tokimm = (tokimm << 4) | 0x0;toklen = 3;state = STATE_NUM3;break;
			case '1': tokimm = (tokimm << 4) | 0x1;toklen = 3;state = STATE_NUM3;break;
			case '2': tokimm = (tokimm << 4) | 0x2;toklen = 3;state = STATE_NUM3;break;
			case '3': tokimm = (tokimm << 4) | 0x3;toklen = 3;state = STATE_NUM3;break;
			case '4': tokimm = (tokimm << 4) | 0x4;toklen = 3;state = STATE_NUM3;break;
			case '5': tokimm = (tokimm << 4) | 0x5;toklen = 3;state = STATE_NUM3;break;
			case '6': tokimm = (tokimm << 4) | 0x6;toklen = 3;state = STATE_NUM3;break;
			case '7': tokimm = (tokimm << 4) | 0x7;toklen = 3;state = STATE_NUM3;break;
			case '8': tokimm = (tokimm << 4) | 0x8;toklen = 3;state = STATE_NUM3;break;
			case '9': tokimm = (tokimm << 4) | 0x9;toklen = 3;state = STATE_NUM3;break;
			case 'a': tokimm = (tokimm << 4) | 0xa;toklen = 3;state = STATE_NUM3;break;
			case 'b': tokimm = (tokimm << 4) | 0xb;toklen = 3;state = STATE_NUM3;break;
			case 'c': tokimm = (tokimm << 4) | 0xc;toklen = 3;state = STATE_NUM3;break;
			case 'd': tokimm = (tokimm << 4) | 0xd;toklen = 3;state = STATE_NUM3;break;
			case 'e': tokimm = (tokimm << 4) | 0xe;toklen = 3;state = STATE_NUM3;break;
			case 'f': tokimm = (tokimm << 4) | 0xf;toklen = 3;state = STATE_NUM3;break;
			case 'r': take(TOKEN_FAR);break;
			default: tokptr--;tokimm8 = GetMax8(tokimm);take(TOKEN_IMM8);break;
			}
			_be;break;
		case STATE_NE: _bb("state(STATE_NE)");
			switch (tokch) {
			case 'a': state = STATE_NEA;break;
			default: tokptr--;_sert_;break;
			}
			_be;break;
		case STATE_PT: _bb("state(STATE_PT)");
			switch (tokch) {
			case 'r': take(TOKEN_PTR);break;
			default: tokptr--;_sert_;break;
			}
			_be;break;
		case STATE_SH: _bb("state(STATE_SH)");
			switch (tokch) {
			case 'o': state = STATE_SHO;break;
			default: tokptr--;_sert_;break;
			}
			_be;break;
		case STATE_TR: _bb("state(STATE_TR)");
			switch (tokch) {
			case '6': take(TOKEN_TR6);break;
			case '7': take(TOKEN_TR7);break;
			default: tokptr--;_sert_;break;
			}
			_be;break;
		case STATE_WO: _bb("state(STATE_WO)");
			switch (tokch) {
			case 'r': state = STATE_WOR;break;
			default: tokptr--;_sert_;break;
			}
			_be;break;
		case STATE_BYT: _bb("state(STATE_BYT)");
			switch (tokch) {
			case 'e': take(TOKEN_BYTE);break;
			default: tokptr--;_sert_;break;
			}
			_be;break;
		case STATE_DWO: _bb("state(STATE_DWO)");
			switch (tokch) {
			case 'r': state = STATE_DWOR;break;
			default: tokptr--;_sert_;break;
			}
			_be;break;
		case STATE_NEA: _bb("state(STATE_NEA)");
			switch (tokch) {
			case 'r': take(TOKEN_NEAR);break;
			default: tokptr--;_sert_;break;
			}
			_be;break;
		case STATE_SHO: _bb("state(STATE_SHO)");
			switch (tokch) {
			case 'r': state = STATE_SHOR;break;
			default: tokptr--;_sert_;break;
			}
			_be;break;
		case STATE_WOR: _bb("state(STATE_WOR)");
			switch (tokch) {
			case 'd': take(TOKEN_WORD);break;
			default: tokptr--;_sert_;break;
			}
			_be;break;
		case STATE_DWOR: _bb("state(STATE_DWOR)");
			switch (tokch) {
			case 'd': take(TOKEN_DWORD);break;
			default: tokptr--;_sert_;break;
			}
			_be;break;
		case STATE_SHOR: _bb("state(STATE_SHOR)");
			switch (tokch) {
			case 't': take(TOKEN_SHORT);break;
			default: tokptr--;_sert_;break;
			}
			_be;break;
		default:  _bb("state(default)");
			tokptr--;_sert_;
			_be;break;
		}
		tokptr++;
	} while (!flagend);
	_ce;
	return token;
}
static void printtoken(t_aasm_token token)
{
	switch (token) {
	case TOKEN_NULL:    vapiPrint(" NULL "); break;
	case TOKEN_END:     vapiPrint(" END ");  break;
	case TOKEN_LSPAREN: vapiPrint(" [[ ");   break;
	case TOKEN_RSPAREN: vapiPrint(" ]] ");   break;
	case TOKEN_COLON:   vapiPrint(" :: ");   break;
	case TOKEN_PLUS:    vapiPrint(" ++ ");   break;
	case TOKEN_MINUS:   vapiPrint(" -- ");   break;
	case TOKEN_TIMES:   vapiPrint(" ** ");   break;
	case TOKEN_BYTE:    vapiPrint(" BYTE "); break;
	case TOKEN_WORD:    vapiPrint(" WORD "); break;
	case TOKEN_DWORD:   vapiPrint(" DWORD ");break;
	case TOKEN_PTR:     vapiPrint(" PTR ");  break;
	case TOKEN_NEAR:    vapiPrint(" NEAR "); break;
	case TOKEN_FAR:     vapiPrint(" FAR ");  break;
	case TOKEN_SHORT:   vapiPrint(" SHORT ");break;
	case TOKEN_IMM8:    vapiPrint(" I8(%02X) ",  tokimm8);break;
	case TOKEN_IMM16:   vapiPrint(" I16(%04X) ", tokimm16);break;
	case TOKEN_IMM32:   vapiPrint(" I32(%08X) ", tokimm32);break;
	case TOKEN_AH: vapiPrint(" AH ");break;
	case TOKEN_BH: vapiPrint(" BH ");break;
	case TOKEN_CH: vapiPrint(" CH ");break;
	case TOKEN_DH: vapiPrint(" DH ");break;
	case TOKEN_AL: vapiPrint(" AL ");break;
	case TOKEN_BL: vapiPrint(" BL ");break;
	case TOKEN_CL: vapiPrint(" CL ");break;
	case TOKEN_DL: vapiPrint(" DL ");break;
	case TOKEN_AX: vapiPrint(" AX ");break;
	case TOKEN_BX: vapiPrint(" BX ");break;
	case TOKEN_CX: vapiPrint(" CX ");break;
	case TOKEN_DX: vapiPrint(" DX ");break;
	case TOKEN_SP: vapiPrint(" SP ");break;
	case TOKEN_BP: vapiPrint(" BP ");break;
	case TOKEN_SI: vapiPrint(" SI ");break;
	case TOKEN_DI: vapiPrint(" DI ");break;
	case TOKEN_ES: vapiPrint(" ES ");break;
	case TOKEN_CS: vapiPrint(" CS ");break;
	case TOKEN_SS: vapiPrint(" SS ");break;
	case TOKEN_DS: vapiPrint(" DS ");break;
	case TOKEN_FS: vapiPrint(" FS ");break;
	case TOKEN_GS: vapiPrint(" GS ");break;
	case TOKEN_EAX: vapiPrint(" EAX ");break;
	case TOKEN_EBX: vapiPrint(" EBX ");break;
	case TOKEN_ECX: vapiPrint(" ECX ");break;
	case TOKEN_EDX: vapiPrint(" EDX ");break;
	case TOKEN_ESP: vapiPrint(" ESP ");break;
	case TOKEN_EBP: vapiPrint(" EBP ");break;
	case TOKEN_ESI: vapiPrint(" ESI ");break;
	case TOKEN_EDI: vapiPrint(" EDI ");break;
	case TOKEN_CR0: vapiPrint(" CR0 ");break;
	case TOKEN_CR2: vapiPrint(" CR2 ");break;
	case TOKEN_CR3: vapiPrint(" CR3 ");break;
	case TOKEN_DR0: vapiPrint(" DR0 ");break;
	case TOKEN_DR1: vapiPrint(" DR1 ");break;
	case TOKEN_DR2: vapiPrint(" DR2 ");break;
	case TOKEN_DR3: vapiPrint(" DR3 ");break;
	case TOKEN_DR6: vapiPrint(" DR6 ");break;
	case TOKEN_DR7: vapiPrint(" DR7 ");break;
	case TOKEN_TR6: vapiPrint(" TR6 ");break;
	case TOKEN_TR7: vapiPrint(" TR7 ");break;
	default: vapiPrint(" <ERROR> ");break;
		break;
	}
}
static void matchtoken(t_aasm_token token)
{
	_cb("matchtoken");
	if (gettoken(NULL) != token) _se_;
	_ce;
}

/* assembly compiler: parser / grammar */
static t_aasm_oprinfo parsearg_mem(t_aasm_token token)
{
	t_aasm_oprinfo info;
	t_bool oldtoken;
	t_bool bx,bp,si,di,neg,al;
	t_bool eax,ecx,edx,ebx,esp,ebp,esi,edi;
	t_bool ieax,iecx,iedx,iebx,iebp,iesi,iedi;
	_cb("parsearg_mem");
	memset(&info, 0x00, sizeof(t_aasm_oprinfo));
	bx = bp = si = di = neg = al = 0;
	eax = ecx = edx = ebx = esp = ebp = esi = edi = 0;
	ieax = iecx = iedx = iebx = iebp = iesi = iedi = 0;
	info.type = TYPE_M;
	info.mod = MOD_M;
	info.sib.base = R32_EBP; //EBP for NULL Base
	info.sib.index = R32_ESP; //ESP for NULL Index
	info.sib.scale = 0;
	oldtoken = token;
	_chrf(token = gettoken(NULL));
	if (token == TOKEN_COLON) {
		_bb("token(TOKEN_COLON");
		switch (oldtoken) {
		case TOKEN_ES: info.flages = 1;break;
		case TOKEN_CS: info.flagcs = 1;break;
		case TOKEN_SS: info.flagss = 1;break;
		case TOKEN_DS: info.flagds = 1;break;
		case TOKEN_FS: info.flagfs = 1;break;
		case TOKEN_GS: info.flaggs = 1;break;
		default: _serf_;break;}
		_be;
	} else if (token == TOKEN_NULL || token == TOKEN_END) {
		_bb("token(TOKEN_NULL/TOKEN_END)");
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
		default: _serf_;break;}
		_be;
		_ce;
		return info;
	} else _serf_;
	_chrf(matchtoken(TOKEN_LSPAREN));
	_chrf(token = gettoken(NULL));
	while (token != TOKEN_RSPAREN) {
		switch (token) {
		case TOKEN_PLUS:break;
		case TOKEN_MINUS: _bb("token(TOKEN_MINUS)");
			_chrf(token = gettoken(NULL));
			neg = 1;
			switch (token) {
			case TOKEN_IMM8: _bb("token(TOKEN_IMM8)");
				if (info.mod != MOD_M) _serf_;
				if (tokimm8 > 0x80) _serf_;
				else {
					tokimm8 = (~tokimm8) + 1;
					info.disp8 = tokimm8;
					info.mod = MOD_M_DISP8;
				}
				_be;break;
			case TOKEN_IMM16: _bb("token(TOKEN_IMM16)");
				if (info.mod != MOD_M) _serf_;
				if (tokimm16 > 0xff80) _serf_;
				else {
					tokimm16 = (~tokimm16) + 1;
					info.disp16 = tokimm16;
					info.mod = MOD_M_DISP16;
				}
				_be;break;
			case TOKEN_IMM32: _bb("token(TOKEN_IMM32)");
				if (info.mod != MOD_M) _serf_;
				if (tokimm32 > 0xffffff80) _serf_;
				else {
					tokimm32 = (~tokimm32) + 1;
					info.disp32 = tokimm32;
					info.mod = MOD_M_DISP32;
				}
				_be;break;
			default: _serf_;break;
			}
			_be;break;
		case TOKEN_BX: _bb("token(TOKEN_BX)");
			if (bx) _serf_;
			else bx = 1;
			_be;break;
		case TOKEN_SI: _bb("token(TOKEN_SI)");
			if (si) _serf_;
			else si = 1;
			_be;break;
		case TOKEN_BP: _bb("token(TOKEN_BP)");
			if (bp) _serf_;
			else bp = 1;
			_be;break;
		case TOKEN_DI: _bb("token(TOKEN_DI)");
			if (di) _serf_;
			else di = 1;
			_be;break;
		case TOKEN_AL: _bb("token(TOKEN_AL)");
			if (al) _serf_;
			else al = 1;
			_be;break;
		case TOKEN_IMM8: _bb("token(TOKEN_IMM8)");
			if (info.mod != MOD_M) _serf_;
			info.mod = MOD_M_DISP8;
			info.disp8 = tokimm8;
			_be;break;
		case TOKEN_IMM16: _bb("token(TOKEN_IMM16)");
			if (info.mod != MOD_M) _serf_;
			info.mod = MOD_M_DISP16;
			info.disp16 = tokimm16;
			_be;break;
		case TOKEN_IMM32: _bb("token(TOKEN_IMM32)");
			if (info.mod != MOD_M) _serf_;
			info.mod = MOD_M_DISP32;
			info.disp32 = tokimm32;
			_be;break;
		case TOKEN_EAX: _bb("token(TOKEN_EAX)");
			_chrf(token = gettoken(NULL));
			if (token == TOKEN_TIMES) {
				_bb("token(TOKEN_TIMES)");
				if (ieax) _serf_;
				else {
					_bb("!ieax");
					ieax = 1;
					_chrf(token = gettoken(NULL));
					if (token != TOKEN_IMM8) _serf_;
					else {
						info.sib.scale = tokimm8;
						info.sib.index = R32_EAX;
						_chrf(token = gettoken(NULL));
					}
					_be;
				}
				_be;
			} else {
				_bb("token(TOKEN_!TIMES)");
				if (eax) _serf_;
				else {
					eax = 1;
					info.sib.base = R32_EAX;
				}
				_be;
			}
			_be;continue;
			break;
		case TOKEN_ECX: _bb("token(TOKEN_ECX)");
			_chrf(token = gettoken(NULL));
			if (token == TOKEN_TIMES) {
				_bb("token(TOKEN_TIMES)");
				if (iecx) _serf_;
				else {
					_bb("!iecx");
					iecx = 1;
					_chrf(token = gettoken(NULL));
					if (token != TOKEN_IMM8) _serf_;
					else {
						info.sib.scale = tokimm8;
						info.sib.index = R32_ECX;
						_chrf(token = gettoken(NULL));
					}
					_be;
				}
				_be;
			} else {
				_bb("token(!TOKEN_TIMES)");
				if (ecx) _serf_;
				else {
					ecx = 1;
					info.sib.base = R32_ECX;
				}
				_be;
			}
			_be;continue;
			break;
		case TOKEN_EDX: _bb("token(TOKEN_EDX)");
			_chrf(token = gettoken(NULL));
			if (token == TOKEN_TIMES) {
				_bb("token(TOKEN_TIMES)");
				if (iedx) _serf_;
				else {
					_bb("!iedx");
					iedx = 1;
					_chrf(token = gettoken(NULL));
					if (token != TOKEN_IMM8) _serf_;
					else {
						info.sib.scale = tokimm8;
						info.sib.index = R32_EDX;
						_chrf(token = gettoken(NULL));
					}
					_be;
				}
				_be;
			} else {
				_bb("token(!TOKEN_TIMES)");
				if (edx) _serf_;
				else {
					edx = 1;
					info.sib.base = R32_EDX;
				}
				_be;
			}
			_be;continue;
			break;
		case TOKEN_EBX: _bb("token(TOKEN_EBX)");
			_chrf(token = gettoken(NULL));
			if (token == TOKEN_TIMES) {
				_bb("token(TOKEN_TIMES)");
				if (iebx) _serf_;
				else {
					_bb("!iebx");
					iebx = 1;
					_chrf(token = gettoken(NULL));
					if (token != TOKEN_IMM8) _serf_;
					else {
						info.sib.scale = tokimm8;
						info.sib.index = R32_EBX;
						_chrf(token = gettoken(NULL));
					}
					_be;
				}
				_be;
			} else {
				_bb("token(!TOKEN_TIMES)");
				if (ebx) _serf_;
				else {
					ebx = 1;
					info.sib.base = R32_EBX;
				}
				_be;
			}
			_be;continue;
			break;
		case TOKEN_ESP: _bb("token(TOKEN_ESP)");
			_chrf(token = gettoken(NULL));
			if (token == TOKEN_TIMES) _serf_;
			else {
				_bb("token(!TOKEN_TIMES)");
				if (esp) _serf_;
				else {
					esp = 1;
					info.sib.base = R32_ESP;
				}
				_be;
			}
			_be;continue;
			break;
		case TOKEN_EBP: _bb("token(TOKEN_EBP)");
			_chrf(token = gettoken(NULL));
			if (token == TOKEN_TIMES) {
				_bb("token(TOKEN_TIMES)");
				if (iebp) _serf_;
				else {
					_bb("!iebp");
					iebx = 1;
					_chrf(token = gettoken(NULL));
					if (token != TOKEN_IMM8) _serf_;
					else {
						info.sib.scale = tokimm8;
						info.sib.index = R32_EBP;
						_chrf(token = gettoken(NULL));
					}
					_be;
				}
				_be;
			} else {
				_bb("token(!TOKEN_TIMES)");
				if (ebp) _serf_;
				else {
					ebp = 1;
					info.sib.base = R32_EBP;
				}
				_be;
			}
			_be;continue;
			break;
		case TOKEN_ESI: _bb("token(TOKEN_ESI)");
			_chrf(token = gettoken(NULL));
			if (token == TOKEN_TIMES) {
				_bb("token(TOKEN_TIMES)");
				if (iesi) _serf_;
				else {
					_bb("!iesi");
					iesi = 1;
					_chrf(token = gettoken(NULL));
					if (token != TOKEN_IMM8) _serf_;
					else {
						info.sib.scale = tokimm8;
						info.sib.index = R32_ESI;
						_chrf(token = gettoken(NULL));
					}
					_be;
				}
				_be;
			} else {
				_bb("token(!TOKEN_TIMES)");
				if (esi) _serf_;
				else {
					esi = 1;
					info.sib.base = R32_ESI;
				}
				_be;
			}
			_be;continue;
			break;
		case TOKEN_EDI: _bb("token(TOKEN_EDI)");
			_chrf(token = gettoken(NULL));
			if (token == TOKEN_TIMES) {
				_bb("token(TOKEN_TIMES)");
				if (iedi) _serf_;
				else {
					_bb("!iedi");
					iedi = 1;
					_chrf(token = gettoken(NULL));
					if (token != TOKEN_IMM8) _serf_;
					else {
						info.sib.scale = tokimm8;
						info.sib.index = R32_EDI;
						_chrf(token = gettoken(NULL));
					}
					_be;
				}
				_be;
			} else {
				_bb("token(!TOKEN_TIMES)");
				if (edi) _serf_;
				else {
					edi = 1;
					info.sib.base = R32_EDI;
				}
				_be;
			}
			_be;continue;
			break;
		default: _serf_;break;}
		_chrf(token = gettoken(NULL));
	}
	_chrf(token = gettoken(NULL));
	if (token != TOKEN_END) _serf_;

	if (al) {
		_bb("al");
		if (bp || si || di || eax || ecx || edx || esp || ebp || esi || edi ||
			ieax || iecx || iedx || iebx || iebp || iesi || iedi || info.mod != MOD_M)
			_serf_;
		if (bx && !ebx) {
			info.mem = MEM_BX_AL;
		} else if (!bx && ebx) {
			info.mem = MEM_EBX_AL;
		} else _serf_;
		_be;
	} else {
		if (bx || bp || si || di || info.mod == MOD_M_DISP16) {
			 _bb("16-bit Addressing");
			if (!bx && !si && !bp && !di) {
				_bb("[DISP16]");
				info.mem = MEM_BP;
				if (info.mod == MOD_M_DISP16)
					info.mod = MOD_M;
				else _serf_;
				_be;
			} else {
				_bb("bx/bp/si/di");
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
				_be;
			}
			_be;
		} else if (eax || ecx || edx || ebx || esp || ebp || esi || edi ||
			ieax || iecx || iedx || iebx || iebp || iesi || iedi || info.mod == MOD_M_DISP32) {
			 _bb("32-bit Addressing");
			if (!eax && !ecx && !edx && !ebx && !esp && !ebp && !esi && !edi) {
				_bb("!base");
				if (info.mod == MOD_M_DISP32) {
					info.mod = MOD_M;
					if ( ieax || iecx || iedx || iebx || iebp || iesi || iedi)
						info.mem = MEM_SIB;
					else
						info.mem = MEM_EBP;
				} else _serf_;
				_be;
			} else {
				_bb("base");
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
				_be;
			}
			_be;
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
	default: _serf_;break;}
	info.type = TYPE_M;
	_ce;
	return info;
}
static t_aasm_oprinfo parsearg_imm(t_aasm_token token)
{
	t_aasm_oprinfo info;
	_cb("parsearg_imm");

	memset(&info, 0x00, sizeof(t_aasm_oprinfo));

	if (token == TOKEN_PLUS) {
		_bb("token(TOKEN_PLUS)");
		info.imms = 1;
		info.immn = 0;
		_chrf(token = gettoken(NULL));
		_be;
	} else if (token == TOKEN_MINUS) {
		_bb("token(TOKEN_MINUS)");
		info.imms = 1;
		info.immn = 1;
		_chrf(token = gettoken(NULL));
		_be;
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
		_bb("token(!TOKEN_END)");
		if (info.imms) _serf_;
		if (info.type == TYPE_I16) info.rcs = info.imm16;
		else {info.type = TYPE_NONE;_serf_;}
		info.type = TYPE_NONE;
		_chrf(token = gettoken(NULL));
		if (token == TOKEN_IMM16) {
			info.reip = tokimm16;
			info.type = TYPE_I16_16;
		} else if (token == TOKEN_IMM32) {
			info.reip = tokimm32;
			info.type = TYPE_I16_32;
		} else _serf_;
		_be;
	} else if (token != TOKEN_END) _serf_;
	_ce;
	return info;
}
static t_aasm_oprinfo parsearg(t_strptr arg)
{
	t_aasm_token token;
	t_aasm_oprinfo info;
	_cb("parsearg");
	memset(&info, 0x00 ,sizeof(t_aasm_oprinfo));
	if (!arg || !arg[0]) {
		info.type = TYPE_NONE;
		_ce;
		return info;
	}
	_chrf(token = gettoken(arg));
	switch (token) {
	case TOKEN_NULL:
	case TOKEN_END:
		info.type = TYPE_NONE;
		break;
	case TOKEN_BYTE: _bb("token(TOKEN_BYTE)");
		_chrf(token = gettoken(NULL));
		if (token == TOKEN_PTR) _chrf(token = gettoken(NULL));
		_chrf(info = parsearg_mem(token));
		info.type = TYPE_M8;
		_be;break;
	case TOKEN_WORD: _bb("token(TOKEN_WORD)");
		_chrf(token = gettoken(NULL));
		if (token == TOKEN_PTR) _chrf(token = gettoken(NULL));
		_chrf(info = parsearg_mem(token));
		info.type = TYPE_M16;
		info.ptr = PTR_NEAR;
		_be;break;
	case TOKEN_DWORD: _bb("token(TOKEN_DWORD)");
		_chrf(token = gettoken(NULL));
		if (token == TOKEN_PTR) _chrf(token = gettoken(NULL));
		_chrf(info = parsearg_mem(token));
		info.type = TYPE_M32;
		info.ptr = PTR_FAR;
		_be;break;
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
	case TOKEN_GS: _bb("token(TOKEN_SREGs)");
		_chrf(info = parsearg_mem(token));
		_be;break;
	case TOKEN_PLUS:
	case TOKEN_MINUS:
	case TOKEN_IMM8:
	case TOKEN_IMM16:
	case TOKEN_IMM32: _bb("token(TOKEN_IMMs)");
		_chrf(info = parsearg_imm(token));
		if (info.type == TYPE_I16_16)
			info.ptr = PTR_FAR;
		else if (info.type == TYPE_I16_32)
			info.ptr = PTR_FAR;
		else
			info.ptr = PTR_NONE;
		_be;break;
	case TOKEN_SHORT: _bb("token(TOKEN_SHORT)");
		_chrf(token = gettoken(NULL));
		if (token == TOKEN_PTR) _chrf(token = gettoken(NULL));
		if (token == TOKEN_PLUS || token == TOKEN_MINUS) {
			_bb("token(TOKEN_SIGNs)");
			_chrf(info = parsearg_imm(token));
			if (info.type != TYPE_I8) _serf_;
			_be;
		} else _serf_;
		info.ptr = PTR_SHORT;
		_be;break;
	case TOKEN_NEAR: _bb("token(TOKEN_NEAR)");
		_chrf(token = gettoken(NULL));
		if (token == TOKEN_PTR) _chrf(token = gettoken(NULL));
		switch (token) {
		case TOKEN_WORD:
			_bb("token(TOKEN_WORD)");
			_chrf(token = gettoken(NULL));
			if (token == TOKEN_PTR) _chrf(token = gettoken(NULL));
			_chrf(info = parsearg_mem(token));
			info.type = TYPE_M16;
			_be;break;
		case TOKEN_DWORD: _bb("token(TOKEN_DWORD)");
			_chrf(token = gettoken(NULL));
			if (token == TOKEN_PTR) _chrf(token = gettoken(NULL));
			_chrf(info = parsearg_mem(token));
			info.type = TYPE_M32;
			_be;break;
		case TOKEN_PLUS:
		case TOKEN_MINUS: _bb("token(TOKEN_PLUS/TOKEN_MINUS)");
			_chrf(info = parsearg_imm(token));
			if (info.type != TYPE_I16 && info.type != TYPE_I32) _serf_;
			_be;break;
		case TOKEN_ES:
		case TOKEN_CS:
		case TOKEN_SS:
		case TOKEN_DS:
		case TOKEN_FS:
		case TOKEN_GS: _bb("token(TOKEN_SREGs)");
			_chrf(info = parsearg_mem(token));
			if (info.type != TYPE_M) _serf_;
			_be;break;
		default: _serf_;break;}
		info.ptr = PTR_NEAR;
		_be;break;
	case TOKEN_FAR: _bb("token(TOKEN_FAR)");
		_chrf(token = gettoken(NULL));
		if (token == TOKEN_PTR) _chrf(token = gettoken(NULL));
		switch (token) {
		case TOKEN_WORD: _bb("token(TOKEN_WORD)");
			_chrf(token = gettoken(NULL));
			if (token == TOKEN_PTR) _chrf(token = gettoken(NULL));
			_chrf(info = parsearg_mem(token));
			info.type = TYPE_M16;
			info.ptr = PTR_FAR;
			_be;break;
		case TOKEN_DWORD: _bb("token(TOKEN_DWORD)");
			_chrf(token = gettoken(NULL));
			if (token == TOKEN_PTR) _chrf(token = gettoken(NULL));
			_chrf(info = parsearg_mem(token));
			info.type = TYPE_M32;
			info.ptr = PTR_FAR;
			_be;break;
		case TOKEN_IMM16: _bb("token(TOKEN_IMM16)");
			_chrf(info = parsearg_imm(token));
			if (info.type != TYPE_I16_16 && info.type != TYPE_I16_32) _serf_;
			_be;break;
		case TOKEN_ES:
		case TOKEN_CS:
		case TOKEN_SS:
		case TOKEN_DS:
		case TOKEN_FS:
		case TOKEN_GS: _bb("token(TOKEN_SREGs)");
			_chrf(info = parsearg_mem(token));
			if (info.type != TYPE_M) _serf_;
			_be;break;
		default: _serf_;break;}
		info.ptr = PTR_FAR;
		_be;break;
	case TOKEN_CR0: info.type = TYPE_CREG;info.creg = CREG_CR0;break;
	case TOKEN_CR2: info.type = TYPE_CREG;info.creg = CREG_CR2;break;
	case TOKEN_CR3: info.type = TYPE_CREG;info.creg = CREG_CR3;break;
	case TOKEN_DR0: info.type = TYPE_DREG;info.dreg = DREG_DR0;break;
	case TOKEN_DR1: info.type = TYPE_DREG;info.dreg = DREG_DR1;break;
	case TOKEN_DR2: info.type = TYPE_DREG;info.dreg = DREG_DR2;break;
	case TOKEN_DR3: info.type = TYPE_DREG;info.dreg = DREG_DR3;break;
	case TOKEN_DR6: info.type = TYPE_DREG;info.dreg = DREG_DR6;break;
	case TOKEN_DR7: info.type = TYPE_DREG;info.dreg = DREG_DR7;break;
	case TOKEN_TR6: info.type = TYPE_TREG;info.treg = TREG_TR6;break;
	case TOKEN_TR7: info.type = TYPE_TREG;info.treg = TREG_TR7;break;
	default: _serf_;break;}
	_ce;
	return info;
}
/* assembly compiler: code generator */
static void _c_setbyte(t_nubit8 byte)
{
	d_nubit8(acode + iop) = byte;
	iop += 1;
}
static void _c_setword(t_nubit16 word)
{
	d_nubit16(acode + iop) = word;
	iop += 2;
}
static void _c_setdword(t_nubit32 dword)
{
	d_nubit32(acode + iop) = dword;
	iop += 4;
}
static void _c_imm8(t_nubit8 byte)
{
	_cb("_c_imm8");
	_chk(_c_setbyte(byte));
	_ce;
}
static void _c_imm16(t_nubit16 word)
{
	_cb("_c_imm16");
	_chk(_c_setword(word));
	_ce;
}
static void _c_imm32(t_nubit32 dword)
{
	_cb("_c_imm32");
	_chk(_c_setdword(dword));
	_ce;
}
static void _c_modrm(t_aasm_oprinfo rminfo, t_nubit8 reg)
{
	t_nubit8 sibval;
	t_nubit8 modrmval = (reg << 3);
	_cb("_c_rminfo");

	switch (rminfo.mem) {
	case MEM_BX_SI:
	case MEM_BX_DI:
	case MEM_BP_SI:
	case MEM_BP_DI:
	case MEM_SI:
	case MEM_DI:
	case MEM_BP:
	case MEM_BX: _bb("16-bit Addressing");
		_SetAddressSize(2);
		switch(rminfo.mod) {
		case MOD_M:
			modrmval |= (0 << 6);
			modrmval |= (t_nubit8)rminfo.mem;
			_c_setbyte(modrmval);
			switch(rminfo.mem) {
			case MEM_BP: _c_setword(rminfo.disp16);break;
			default:break;}
			break;
		case MOD_M_DISP8:
			modrmval |= (1 << 6);
			modrmval |= (t_nubit8)rminfo.mem;
			_c_setbyte(modrmval);
			_c_setbyte(rminfo.disp8);
			break;
		case MOD_M_DISP16:
			modrmval |= (2 << 6);
			modrmval |= (t_nubit8)rminfo.mem;
			_c_setbyte(modrmval);
			_c_setword(rminfo.disp16);
			break;
		case MOD_R: _bb("mod(MOD_R)");
			modrmval |= (3 << 6);
			switch (rminfo.type) {
			case TYPE_R8:
				modrmval |= (t_nubit8)rminfo.reg8;
				_c_setbyte(modrmval);
				break;
			case TYPE_R16:
				modrmval |= (t_nubit8)rminfo.reg16;
				_c_setbyte(modrmval);
				break;
			case TYPE_R32:
				modrmval |= (t_nubit8)rminfo.reg32;
				_c_setbyte(modrmval);
				break;
			default: _se_;break;}
			_be;break;
		default: _se_;break;}
		_be;break;
	case MEM_EAX:
	case MEM_ECX:
	case MEM_EDX:
	case MEM_EBX:
	case MEM_SIB:
	case MEM_EBP:
	case MEM_ESI:
	case MEM_EDI: _bb("32-bit Addressing");
		_SetAddressSize(4);
		switch(rminfo.mod) {
		case MOD_M:
			modrmval |= (0 << 6);
			modrmval |= (t_nubit8)rminfo.mem & 0x07;
			_c_setbyte(modrmval);
			switch(rminfo.mem) {
			case MEM_SIB:
				sibval = (t_nubit8)rminfo.sib.base;
				sibval |= ((t_nubit8)rminfo.sib.index << 3);
				switch (rminfo.sib.scale) {
				case 0: rminfo.sib.scale = 0;break;
				case 1: rminfo.sib.scale = 0;break;
				case 2: rminfo.sib.scale = 1;break;
				case 4: rminfo.sib.scale = 2;break;
				case 8: rminfo.sib.scale = 3;break;
				default: _se_;break;}
				sibval |= (rminfo.sib.scale << 6);
				_c_setbyte(sibval);
				switch (rminfo.sib.base) {
				case R32_EBP: _c_setdword(rminfo.disp32);break;
				default: break;}
				break;
			case MEM_EBP: _c_setdword(rminfo.disp32);break;
			default:break;}
			break;
		case MOD_M_DISP8:
			modrmval |= (1 << 6);
			modrmval |= (t_nubit8)rminfo.mem & 0x07;
			_c_setbyte(modrmval);
			switch(rminfo.mem) {
			case MEM_SIB:
				sibval = (t_nubit8)rminfo.sib.base;
				sibval |= ((t_nubit8)rminfo.sib.index << 3);
				switch (rminfo.sib.scale) {
				case 0: rminfo.sib.scale = 0;break;
				case 1: rminfo.sib.scale = 0;break;
				case 2: rminfo.sib.scale = 1;break;
				case 4: rminfo.sib.scale = 2;break;
				case 8: rminfo.sib.scale = 3;break;
				default: _se_;break;}
				sibval |= (rminfo.sib.scale << 6);
				_c_setbyte(sibval);
				break;
			default:break;}
			_c_setbyte(rminfo.disp8);
			break;
		case MOD_M_DISP32:
			modrmval |= (2 << 6);
			modrmval |= (t_nubit8)rminfo.mem & 0x07;
			_c_setbyte(modrmval);
			switch(rminfo.mem) {
			case MEM_SIB:
				sibval = (t_nubit8)rminfo.sib.base;
				sibval |= ((t_nubit8)rminfo.sib.index << 3);
				switch (rminfo.sib.scale) {
				case 0: rminfo.sib.scale = 0;break;
				case 1: rminfo.sib.scale = 0;break;
				case 2: rminfo.sib.scale = 1;break;
				case 4: rminfo.sib.scale = 2;break;
				case 8: rminfo.sib.scale = 3;break;
				default: _se_;break;}
				sibval |= (rminfo.sib.scale << 6);
				_c_setbyte(sibval);
				break;
			default:break;}
			_c_setdword(rminfo.disp32);
			break;
		case MOD_R: _bb("mod(MOD_R)");
			modrmval |= (3 << 6);
			switch (rminfo.type) {
			case TYPE_R8:
				modrmval |= (t_nubit8)rminfo.reg8;
				_c_setbyte(modrmval);
				break;
			case TYPE_R16:
				modrmval |= (t_nubit8)rminfo.reg16;
				_c_setbyte(modrmval);
				break;
			case TYPE_R32:
				modrmval |= (t_nubit8)rminfo.reg32;
				_c_setbyte(modrmval);
				break;
			default: _se_;break;}
			_be;break;
		default: _se_;break;}
		_be;break;
	default: _se_;break;}
	_ce;
}

/* concrete instructions */
static void ADD_RM8_R8()
{
	_cb("ADD_RM8_R8");
	_c_setbyte(0x00);
	_chk(_c_modrm(aopri1, aopri2.reg8));
	_ce;
}
static void ADD_RM32_R32(t_nubit8 byte)
{
	_cb("ADD_RM32_R32");
	_SetOperandSize(byte);
	_c_setbyte(0x01);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri1, aopri2.reg16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_modrm(aopri1, aopri2.reg32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void ADD_R8_RM8()
{
	_cb("ADD_R8_RM8");
	_c_setbyte(0x02);
	_chk(_c_modrm(aopri2, aopri1.reg8));
	_ce;
}
static void ADD_R32_RM32(t_nubit8 byte)
{
	_cb("ADD_R32_RM32");
	_SetOperandSize(byte);
	_c_setbyte(0x03);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri2, aopri1.reg16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_modrm(aopri2, aopri1.reg32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void ADD_AL_I8()
{
	_cb("ADD_AL_I8");
	_c_setbyte(0x04);
	_chk(_c_imm8(aopri2.imm8));
	_ce;
}
static void ADD_EAX_I32(t_nubit8 byte)
{
	_cb("ADD_EAX_I32");
	_SetOperandSize(byte);
	_c_setbyte(0x05);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_imm16(aopri2.imm16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_imm32(aopri2.imm32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void PUSH_ES()
{
	_cb("PUSH_ES");
	_c_setbyte(0x06);
	_ce;
}
static void POP_ES()
{
	_cb("POP_ES");
	_c_setbyte(0x07);
	_ce;
}
static void OR_RM8_R8()
{
	_cb("OR_RM8_R8");
	_c_setbyte(0x08);
	_chk(_c_modrm(aopri1, aopri2.reg8));
	_ce;
}
static void OR_RM32_R32(t_nubit8 byte)
{
	_cb("OR_RM32_R32");
	_SetOperandSize(byte);
	_c_setbyte(0x09);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri1, aopri2.reg16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_modrm(aopri1, aopri2.reg32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void OR_R8_RM8()
{
	_cb("OR_R8_RM8");
	_c_setbyte(0x0a);
	_chk(_c_modrm(aopri2, aopri1.reg8));
	_ce;
}
static void OR_R32_RM32(t_nubit8 byte)
{
	_cb("OR_R32_RM32");
	_SetOperandSize(byte);
	_c_setbyte(0x0b);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri2, aopri1.reg16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_modrm(aopri2, aopri1.reg32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void OR_AL_I8()
{
	_cb("OR_AL_I8");
	_c_setbyte(0x0c);
	_c_imm8(aopri2.imm8);
	_ce;
}
static void OR_EAX_I32(t_nubit8 byte)
{
	_cb("OR_EAX_I32");
	_SetOperandSize(byte);
	_c_setbyte(0x0d);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_imm16(aopri2.imm16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_imm32(aopri2.imm32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void PUSH_CS()
{
	_cb("PUSH_CS");
	_c_setbyte(0x0e);
	_ce;
}
static void POP_CS()
{
	_cb("POP_CS");
	_c_setbyte(0x0f);
	_ce;
}
static void INS_0F()
{
	_cb("INS_0F");
	_c_setbyte(0x0f);
	_ce;
}
static void ADC_RM8_R8()
{
	_cb("ADC_RM8_R8");
	_c_setbyte(0x10);
	_chk(_c_modrm(aopri1, aopri2.reg8));
	_ce;
}
static void ADC_RM32_R32(t_nubit8 byte)
{
	_cb("ADC_RM32_R32");
	_SetOperandSize(byte);
	_c_setbyte(0x11);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri1, aopri2.reg16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_modrm(aopri1, aopri2.reg32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void ADC_R8_RM8()
{
	_cb("ADC_R8_RM8");
	_c_setbyte(0x12);
	_chk(_c_modrm(aopri2, aopri1.reg8));
	_ce;
}
static void ADC_R32_RM32(t_nubit8 byte)
{
	_cb("ADC_R32_RM32");
	_SetOperandSize(byte);
	_c_setbyte(0x13);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri2, aopri1.reg16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_modrm(aopri2, aopri1.reg32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void ADC_AL_I8()
{
	_cb("ADC_AL_I8");
	_c_setbyte(0x14);
	_chk(_c_imm8(aopri2.imm8));
	_ce;
}
static void ADC_EAX_I32(t_nubit8 byte)
{
	_cb("ADC_EAX_I32");
	_SetOperandSize(byte);
	_c_setbyte(0x15);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_imm16(aopri2.imm16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_imm32(aopri2.imm32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void PUSH_SS()
{
	_cb("PUSH_SS");
	_c_setbyte(0x16);
	_ce;
}
static void POP_SS()
{
	_cb("POP_SS");
	_c_setbyte(0x17);
	_ce;
}
static void SBB_RM8_R8()
{
	_cb("SBB_RM8_R8");
	_c_setbyte(0x18);
	_chk(_c_modrm(aopri1, aopri2.reg8));
	_ce;
}
static void SBB_RM32_R32(t_nubit8 byte)
{
	_cb("SBB_RM32_R32");
	_SetOperandSize(byte);
	_c_setbyte(0x19);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri1, aopri2.reg16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_modrm(aopri1, aopri2.reg32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void SBB_R8_RM8()
{
	_cb("SBB_R8_RM8");
	_c_setbyte(0x1a);
	_chk(_c_modrm(aopri2, aopri1.reg8));
	_ce;
}
static void SBB_R32_RM32(t_nubit8 byte)
{
	_cb("SBB_R32_RM32");
	_SetOperandSize(byte);
	_c_setbyte(0x1b);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri2, aopri1.reg16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_modrm(aopri2, aopri1.reg32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void SBB_AL_I8()
{
	_cb("SBB_AL_I8");
	_c_setbyte(0x1c);
	_c_imm8(aopri2.imm8);
	_ce;
}
static void SBB_EAX_I32(t_nubit8 byte)
{
	_cb("SBB_EAX_I32");
	_SetOperandSize(byte);
	_c_setbyte(0x1d);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_imm16(aopri2.imm16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_imm32(aopri2.imm32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void PUSH_DS()
{
	_cb("PUSH_DS");
	_c_setbyte(0x1e);
	_ce;
}
static void POP_DS()
{
	_cb("POP_DS");
	_c_setbyte(0x1f);
	_ce;
}
static void AND_RM8_R8()
{
	_cb("AND_RM8_R8");
	_c_setbyte(0x20);
	_chk(_c_modrm(aopri1, aopri2.reg8));
	_ce;
}
static void AND_RM32_R32(t_nubit8 byte)
{
	_cb("AND_RM32_R32");
	_SetOperandSize(byte);
	_c_setbyte(0x21);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri1, aopri2.reg16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_modrm(aopri1, aopri2.reg32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void AND_R8_RM8()
{
	_cb("AND_R8_RM8");
	_c_setbyte(0x22);
	_chk(_c_modrm(aopri2, aopri1.reg8));
	_ce;
}
static void AND_R32_RM32(t_nubit8 byte)
{
	_cb("AND_R32_RM32");
	_SetOperandSize(byte);
	_c_setbyte(0x23);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri2, aopri1.reg16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_modrm(aopri2, aopri1.reg32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void AND_AL_I8()
{
	_cb("AND_AL_I8");
	_c_setbyte(0x24);
	_chk(_c_imm8(aopri2.imm8));
	_ce;
}
static void AND_EAX_I32(t_nubit8 byte)
{
	_cb("AND_EAX_I32");
	_SetOperandSize(byte);
	_c_setbyte(0x25);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_imm16(aopri2.imm16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_imm32(aopri2.imm32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void PREFIX_ES()
{
	_cb("PREFIX_ES");
	if (ARG_NONE) aoprig.flages = 1;
	else _se_;
	_ce;
}
static void DAA()
{
	_cb("DAA");
	if (ARG_NONE) _c_setbyte(0x27);
	else _se_;
	_ce;
}
static void SUB_RM8_R8()
{
	_cb("SUB_RM8_R8");
	_c_setbyte(0x28);
	_chk(_c_modrm(aopri1, aopri2.reg8));
	_ce;
}
static void SUB_RM32_R32(t_nubit8 byte)
{
	_cb("SUB_RM32_R32");
	_SetOperandSize(byte);
	_c_setbyte(0x29);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri1, aopri2.reg16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_modrm(aopri1, aopri2.reg32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void SUB_R8_RM8()
{
	_cb("SUB_R8_RM8");
	_c_setbyte(0x2a);
	_chk(_c_modrm(aopri2, aopri1.reg8));
	_ce;
}
static void SUB_R32_RM32(t_nubit8 byte)
{
	_cb("SUB_R32_RM32");
	_SetOperandSize(byte);
	_c_setbyte(0x2b);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri2, aopri1.reg16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_modrm(aopri2, aopri1.reg32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void SUB_AL_I8()
{
	_cb("SUB_AL_I8");
	_c_setbyte(0x2c);
	_c_imm8(aopri2.imm8);
	_ce;
}
static void SUB_EAX_I32(t_nubit8 byte)
{
	_cb("SUB_EAX_I32");
	_SetOperandSize(byte);
	_c_setbyte(0x2d);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_imm16(aopri2.imm16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_imm32(aopri2.imm32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void PREFIX_CS()
{
	_cb("PREFIX_CS");
	if (ARG_NONE) aoprig.flagcs = 1;
	else _se_;
	_ce;
}
static void DAS()
{
	_cb("DAS");
	if (ARG_NONE) _c_setbyte(0x2f);
	else _se_;
	_ce;
}
static void XOR_RM8_R8()
{
	_cb("XOR_RM8_R8");
	_c_setbyte(0x30);
	_chk(_c_modrm(aopri1, aopri2.reg8));
	_ce;
}
static void XOR_RM32_R32(t_nubit8 byte)
{
	_cb("XOR_RM32_R32");
	_SetOperandSize(byte);
	_c_setbyte(0x31);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri1, aopri2.reg16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_modrm(aopri1, aopri2.reg32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void XOR_R8_RM8()
{
	_cb("XOR_R8_RM8");
	_c_setbyte(0x32);
	_chk(_c_modrm(aopri2, aopri1.reg8));
	_ce;
}
static void XOR_R32_RM32(t_nubit8 byte)
{
	_cb("XOR_R32_RM32");
	_SetOperandSize(byte);
	_c_setbyte(0x33);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri2, aopri1.reg16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_modrm(aopri2, aopri1.reg32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void XOR_AL_I8()
{
	_cb("XOR_AL_I8");
	_c_setbyte(0x34);
	_chk(_c_imm8(aopri2.imm8));
	_ce;
}
static void XOR_EAX_I32(t_nubit8 byte)
{
	_cb("XOR_EAX_I32");
	_SetOperandSize(byte);
	_c_setbyte(0x35);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_imm16(aopri2.imm16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_imm32(aopri2.imm32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void PREFIX_SS()
{
	_cb("PREFIX_SS");
	if (ARG_NONE) aoprig.flagss = 1;
	else _se_;
	_ce;
}
static void AAA()
{
	_cb("AAA");
	if (ARG_NONE) _c_setbyte(0x37);
	else _se_;
	_ce;
}
static void CMP_RM8_R8()
{
	_cb("CMP_RM8_R8");
	_c_setbyte(0x38);
	_chk(_c_modrm(aopri1, aopri2.reg8));
	_ce;
}
static void CMP_RM32_R32(t_nubit8 byte)
{
	_cb("CMP_RM32_R32");
	_SetOperandSize(byte);
	_c_setbyte(0x39);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri1, aopri2.reg16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_modrm(aopri1, aopri2.reg32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void CMP_R8_RM8()
{
	_cb("CMP_R8_RM8");
	_c_setbyte(0x3a);
	_chk(_c_modrm(aopri2, aopri1.reg8));
	_ce;
}
static void CMP_R32_RM32(t_nubit8 byte)
{
	_cb("CMP_R32_RM32");
	_SetOperandSize(byte);
	_c_setbyte(0x3b);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri2, aopri1.reg16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_modrm(aopri2, aopri1.reg32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void CMP_AL_I8()
{
	_cb("CMP_AL_I8");
	_c_setbyte(0x3c);
	_c_imm8(aopri2.imm8);
	_ce;
}
static void CMP_EAX_I32(t_nubit8 byte)
{
	_cb("CMP_EAX_I32");
	_SetOperandSize(byte);
	_c_setbyte(0x3d);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_imm16(aopri2.imm16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_imm32(aopri2.imm32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void PREFIX_DS()
{
	_cb("PREFIX_DS");
	if (ARG_NONE) aoprig.flagds = 1;
	else _se_;
	_ce;
}
static void AAS()
{
	_cb("AAS");
	if (ARG_NONE) _c_setbyte(0x3f);
	else _se_;
	_ce;
}
static void INC_EAX(t_nubit8 byte)
{
	_cb("INC_EAX");
	_SetOperandSize(byte);
	_c_setbyte(0x40);
	_ce;
}
static void INC_ECX(t_nubit8 byte)
{
	_cb("INC_ECX");
	_SetOperandSize(byte);
	_c_setbyte(0x41);
	_ce;
}
static void INC_EDX(t_nubit8 byte)
{
	_cb("INC_EDX");
	_SetOperandSize(byte);
	_c_setbyte(0x42);
	_ce;
}
static void INC_EBX(t_nubit8 byte)
{
	_cb("INC_EBX");
	_SetOperandSize(byte);
	_c_setbyte(0x43);
	_ce;
}
static void INC_ESP(t_nubit8 byte)
{
	_cb("INC_ESP");
	_SetOperandSize(byte);
	_c_setbyte(0x44);
	_ce;
}
static void INC_EBP(t_nubit8 byte)
{
	_cb("INC_EBP");
	_SetOperandSize(byte);
	_c_setbyte(0x45);
	_ce;
}
static void INC_ESI(t_nubit8 byte)
{
	_cb("INC_ESI");
	_SetOperandSize(byte);
	_c_setbyte(0x46);
	_ce;
}
static void INC_EDI(t_nubit8 byte)
{
	_cb("INC_EDI");
	_SetOperandSize(byte);
	_c_setbyte(0x47);
	_ce;
}
static void DEC_EAX(t_nubit8 byte)
{
	_cb("DEC_EAX");
	_SetOperandSize(byte);
	_c_setbyte(0x48);
	_ce;
}
static void DEC_ECX(t_nubit8 byte)
{
	_cb("DEC_ECX");
	_SetOperandSize(byte);
	_c_setbyte(0x49);
	_ce;
}
static void DEC_EDX(t_nubit8 byte)
{
	_cb("DEC_EDX");
	_SetOperandSize(byte);
	_c_setbyte(0x4a);
	_ce;
}
static void DEC_EBX(t_nubit8 byte)
{
	_cb("DEC_EBX");
	_SetOperandSize(byte);
	_c_setbyte(0x4b);
	_ce;
}
static void DEC_ESP(t_nubit8 byte)
{
	_cb("DEC_ESP");
	_SetOperandSize(byte);
	_c_setbyte(0x4c);
	_ce;
}
static void DEC_EBP(t_nubit8 byte)
{
	_cb("DEC_EBP");
	_SetOperandSize(byte);
	_c_setbyte(0x4d);
	_ce;
}
static void DEC_ESI(t_nubit8 byte)
{
	_cb("DEC_ESI");
	_SetOperandSize(byte);
	_c_setbyte(0x4e);
	_ce;
}
static void DEC_EDI(t_nubit8 byte)
{
	_cb("DEC_EDI");
	_SetOperandSize(byte);
	_c_setbyte(0x4f);
	_ce;
}
static void PUSH_EAX(t_nubit8 byte)
{
	_cb("PUSH_EAX");
	_SetOperandSize(byte);
	_c_setbyte(0x50);
	_ce;
}
static void PUSH_ECX(t_nubit8 byte)
{
	_cb("PUSH_ECX");
	_SetOperandSize(byte);
	_c_setbyte(0x51);
	_ce;
}
static void PUSH_EDX(t_nubit8 byte)
{
	_cb("PUSH_EDX");
	_SetOperandSize(byte);
	_c_setbyte(0x52);
	_ce;
}
static void PUSH_EBX(t_nubit8 byte)
{
	_cb("PUSH_EBX");
	_SetOperandSize(byte);
	_c_setbyte(0x53);
	_ce;
}
static void PUSH_ESP(t_nubit8 byte)
{
	_cb("PUSH_ESP");
	_SetOperandSize(byte);
	_c_setbyte(0x54);
	_ce;
}
static void PUSH_EBP(t_nubit8 byte)
{
	_cb("PUSH_EBP");
	_SetOperandSize(byte);
	_c_setbyte(0x55);
	_ce;
}
static void PUSH_ESI(t_nubit8 byte)
{
	_cb("PUSH_ESI");
	_SetOperandSize(byte);
	_c_setbyte(0x56);
	_ce;
}
static void PUSH_EDI(t_nubit8 byte)
{
	_cb("PUSH_EDI");
	_SetOperandSize(byte);
	_c_setbyte(0x57);
	_ce;
}
static void POP_EAX(t_nubit8 byte)
{
	_cb("POP_EAX");
	_SetOperandSize(byte);
	_c_setbyte(0x58);
	_ce;
}
static void POP_ECX(t_nubit8 byte)
{
	_cb("POP_ECX");
	_SetOperandSize(byte);
	_c_setbyte(0x59);
	_ce;
}
static void POP_EDX(t_nubit8 byte)
{
	_cb("POP_EDX");
	_SetOperandSize(byte);
	_c_setbyte(0x5a);
	_ce;
}
static void POP_EBX(t_nubit8 byte)
{
	_cb("POP_EBX");
	_SetOperandSize(byte);
	_c_setbyte(0x5b);
	_ce;
}
static void POP_ESP(t_nubit8 byte)
{
	_cb("POP_ESP");
	_SetOperandSize(byte);
	_c_setbyte(0x5c);
	_ce;
}
static void POP_EBP(t_nubit8 byte)
{
	_cb("POP_EBP");
	_SetOperandSize(byte);
	_c_setbyte(0x5d);
	_ce;
}
static void POP_ESI(t_nubit8 byte)
{
	_cb("POP_ESI");
	_SetOperandSize(byte);
	_c_setbyte(0x5e);
	_ce;
}
static void POP_EDI(t_nubit8 byte)
{
	_cb("POP_EDI");
	_SetOperandSize(byte);
	_c_setbyte(0x5f);
	_ce;
}
static void PUSHA(t_nubit8 byte)
{
	_cb("PUSHA");
	_SetOperandSize(byte);
	_c_setbyte(0x60);
	_ce;
}
static void POPA(t_nubit8 byte)
{
	_cb("POPA");
	_SetOperandSize(byte);
	_c_setbyte(0x61);
	_ce;
}

static void BOUND_R32_M32_32(t_nubit8 byte)
{
	_cb("BOUND_R32_M32_32");
	_SetOperandSize(byte);
	_c_setbyte(0x62);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri2, aopri1.reg16));
		_be;break;
	case 4: _bb("byte(2)");
		_chk(_c_modrm(aopri2, aopri1.reg32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void ARPL_RM16_R16()
{
	_cb("ARPL_RM16_R16");
	if (ARG_RM16_R16) {
		_bb("ARG_RM16_R16");
		_c_setbyte(0x63);
		_chk(_c_modrm(aopri1, aopri2.reg16));
		_be;
	} else _se_;
	_ce;
}
static void PREFIX_FS()
{
	_cb("PREFIX_FS");
	if (ARG_NONE) aoprig.flagfs = 1;
	else _se_;
	_ce;
}
static void PREFIX_GS()
{
	_cb("PREFIX_GS");
	if (ARG_NONE) aoprig.flaggs = 1;
	else _se_;
	_ce;
}
static void PREFIX_OprSize()
{
	_cb("PREFIX_OprSize");
	if (ARG_NONE) prefix_oprsizeg = 1;
	else _se_;
	_ce;
}
static void PREFIX_AddrSize()
{
	_cb("PREFIX_AddrSize");
	if (ARG_NONE) prefix_addrsizeg = 1;
	else _se_;
	_ce;
}
static void PUSH_I32(t_nubit8 byte)
{
	_cb("PUSH_I32");
	_SetOperandSize(byte);
	_c_setbyte(0x68);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_imm16(aopri1.imm16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_imm32(aopri1.imm32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void IMUL_R32_RM32_I32(t_nubit8 byte)
{
	_cb("IMUL_R32_RM32_I32");
	_SetOperandSize(byte);
	_c_setbyte(0x69);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri2, aopri1.reg16));
		_chk(_c_imm16(aopri3.imm16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_modrm(aopri2, aopri1.reg32));
		_chk(_c_imm32(aopri3.imm32));
		_be;break;
	default: _se_;break;}
	_ce;;
}
static void PUSH_I8()
{
	_cb("PUSH_I8");
	_c_setbyte(0x6a);
	_chk(_c_imm8(aopri1.imm8));
	_ce;
}
static void IMUL_R32_RM32_I8(t_nubit8 byte)
{
	_cb("IMUL_R32_RM32_I32");
	_SetOperandSize(byte);
	_c_setbyte(0x6b);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri2, aopri1.reg16));
		_chk(_c_imm8(aopri3.imm8));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_modrm(aopri2, aopri1.reg32));
		_chk(_c_imm8(aopri3.imm8));
		_be;break;
	default: _se_;break;}
	_ce;;
}
static void INSB()
{
	_cb("INSB");
	_c_setbyte(0x6c);
	rinfo = NULL;
	if (ARG_NONE) ;
	else if (ARG_ESDI8_DX) _SetAddressSize(2);
	else if (ARG_ESEDI8_DX) _SetAddressSize(4);
	else _se_;
	_ce;
}
static void INSW(t_nubit8 byte)
{
	_cb("INSW");
	_SetOperandSize(byte);
	_c_setbyte(0x6d);
	rinfo = NULL;
	switch (byte) {
	case 2: _bb("byte(2)");
		if (ARG_NONE) ;
		else if (ARG_ESDI16_DX) _SetAddressSize(2);
		else if (ARG_ESEDI16_DX) _SetAddressSize(4);
		else _se_;
		_be;break;
	case 4: _bb("byte(4)");
		if (ARG_NONE) ;
		else if (ARG_ESDI32_DX) _SetAddressSize(2);
		else if (ARG_ESEDI32_DX) _SetAddressSize(4);
		else _se_;
		_be;break;
	default: _se_;break;}
	_ce;
}
static void OUTSB()
{
	_cb("OUTSB");
	_c_setbyte(0x6e);
	rinfo = &aopri1;
	if (rinfo->flagds) rinfo->flagds = 0;
	if (ARG_NONE) rinfo = NULL;
	else if (ARG_DX_DSSI8) _SetAddressSize(2);
	else if (ARG_DSESI8) _SetAddressSize(4);
	else _se_;
	_ce;
}
static void OUTSW(t_nubit8 byte)
{
	_cb("OUTSW");
	_SetOperandSize(byte);
	_c_setbyte(0x6f);
	rinfo = &aopri1;
	if (rinfo->flagds) rinfo->flagds = 0;
	switch (byte) {
	case 2: _bb("byte(2)");
		if (ARG_NONE) rinfo = NULL;
		else if (ARG_DX_DSSI16) _SetAddressSize(2);
		else if (ARG_DX_DSESI16) _SetAddressSize(4);
		else _se_;
		_be;break;
	case 4: _bb("byte(4)");
		if (ARG_NONE) rinfo = NULL;
		else if (ARG_DX_DSSI32) _SetAddressSize(2);
		else if (ARG_DX_DSESI32) _SetAddressSize(4);
		else _se_;
		_be;break;
	default: _se_;break;}
	_ce;
}
static void INS_80(t_nubit8 rid)
{
	_cb("INS_80");
	_c_setbyte(0x80);
	_chk(_c_modrm(aopri1, rid));
	_chk(_c_imm8(aopri2.imm8));
	_ce;
}
static void INS_81(t_nubit8 rid, t_nubit8 byte)
{
	_cb("INS_81");
	_SetOperandSize(byte);
	_c_setbyte(0x81);
	_chk(_c_modrm(aopri1, rid));
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_imm16(aopri2.imm16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_imm32(aopri2.imm32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void INS_83(t_nubit8 rid, t_nubit8 byte)
{
	_cb("INS_83");
	_SetOperandSize(byte);
	_c_setbyte(0x83);
	_chk(_c_modrm(aopri1, rid));
	_chk(_c_imm8(aopri2.imm8));
	_ce;
}
static void TEST_RM8_R8()
{
	_cb("TEST_RM8_R8");
	_c_setbyte(0x84);
	_chk(_c_modrm(aopri1, aopri2.reg8));
	_ce;
}
static void TEST_RM32_R32(t_nubit8 byte)
{
	_cb("TEST_RM32_R32");
	_SetOperandSize(byte);
	_c_setbyte(0x85);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri1, aopri2.reg16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_modrm(aopri1, aopri2.reg32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void XCHG_RM8_R8()
{
	_cb("XCHG_RM8_R8");
	_c_setbyte(0x86);
	_chk(_c_modrm(aopri1, aopri2.reg8));
	_ce;
}
static void XCHG_RM32_R32(t_nubit8 byte)
{
	_cb("XCHG_RM32_R32");
	_SetOperandSize(byte);
	_c_setbyte(0x87);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri1, aopri2.reg16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_modrm(aopri1, aopri2.reg32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void MOV_RM8_R8()
{
	_cb("MOV_RM8_R8");
	_c_setbyte(0x88);
	_chk(_c_modrm(aopri1, aopri2.reg8));
	_ce;
}
static void MOV_RM32_R32(t_nubit8 byte)
{
	_cb("MOV_RM32_R32");
	_SetOperandSize(byte);
	_c_setbyte(0x89);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri1, aopri2.reg16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_modrm(aopri1, aopri2.reg32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void MOV_R8_RM8()
{
	_cb("MOV_RM8_R8");
	_c_setbyte(0x8a);
	_chk(_c_modrm(aopri2, aopri1.reg8));
	_ce;
}
static void MOV_R32_RM32(t_nubit8 byte)
{
	_cb("MOV_R32_RM32");
	_SetOperandSize(byte);
	_c_setbyte(0x8b);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri2, aopri1.reg16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_modrm(aopri2, aopri1.reg32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void MOV_RM16_SREG(t_nubit8 byte)
{
	_cb("MOV_RM16_SREG");
	_SetOperandSize(byte);
	_c_setbyte(0x8c);
	_chk(_c_modrm(aopri1, aopri2.sreg));
	_ce;
}
static void LEA_R32_M32(t_nubit8 byte)
{
	_cb("LEA_R32_M32");
	_SetOperandSize(byte);
	_c_setbyte(0x8d);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri2, aopri1.reg16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_modrm(aopri2, aopri1.reg32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void MOV_SREG_RM16(t_nubit8 byte)
{
	_cb("MOV_SREG_RM16");
	_SetOperandSize(byte);
	_c_setbyte(0x8e);
	_chk(_c_modrm(aopri2, aopri1.sreg));
	_ce;
}
static void INS_8F(t_nubit8 rid, t_nubit8 byte)
{
	_cb("INS_8F");
	_SetOperandSize(byte);
	_c_setbyte(0x8f);
	_chk(_c_modrm(aopri1, rid));
	_ce;
}
static void NOP()
{
	_cb("NOP");
	if (ARG_NONE) _c_setbyte(0x90);
	else _se_;
	_ce;
}
static void XCHG_EAX_EAX(t_nubit8 byte)
{
	_cb("XCHG_EAX_EAX");
	_SetOperandSize(byte);
	_c_setbyte(0x90);
	_ce;
}
static void XCHG_ECX_EAX(t_nubit8 byte)
{
	_cb("XCHG_ECX_EAX");
	_SetOperandSize(byte);
	_c_setbyte(0x91);
	_ce;
}
static void XCHG_EDX_EAX(t_nubit8 byte)
{
	_cb("XCHG_EDX_EAX");
	_SetOperandSize(byte);
	_c_setbyte(0x92);
	_ce;
}
static void XCHG_EBX_EAX(t_nubit8 byte)
{
	_cb("XCHG_EBX_EAX");
	_SetOperandSize(byte);
	_c_setbyte(0x93);
	_ce;
}
static void XCHG_ESP_EAX(t_nubit8 byte)
{
	_cb("XCHG_ESP_EAX");
	_SetOperandSize(byte);
	_c_setbyte(0x94);
	_ce;
}
static void XCHG_EBP_EAX(t_nubit8 byte)
{
	_cb("XCHG_EBP_EAX");
	_SetOperandSize(byte);
	_c_setbyte(0x95);
	_ce;
}
static void XCHG_ESI_EAX(t_nubit8 byte)
{
	_cb("XCHG_ESI_EAX");
	_SetOperandSize(byte);
	_c_setbyte(0x96);
	_ce;
}
static void XCHG_EDI_EAX(t_nubit8 byte)
{
	_cb("XCHG_EDI_EAX");
	_SetOperandSize(byte);
	_c_setbyte(0x97);
	_ce;
}
static void CBW(t_nubit8 byte)
{
	_cb("CBW");
	_SetOperandSize(byte);
	if (ARG_NONE) _c_setbyte(0x98);
	else _se_;
	_ce;
}
static void CWD(t_nubit8 byte)
{
	_cb("CWD");
	_SetOperandSize(byte);
	if (ARG_NONE) _c_setbyte(0x99);
	else _se_;
	_ce;
}
static void CALL_PTR16_32(t_nubit8 byte)
{
	_cb("CALL_PTR16_32");
	_SetOperandSize(byte);
	_c_setbyte(0x9a);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_imm16(GetMax16(aopri1.reip)));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_imm32(GetMax32(aopri1.reip)));
		_be;break;
	default: _se_;break;}
	_chk(_c_imm16(aopri1.rcs));
	_ce;
}
static void WAIT()
{
	_cb("WAIT");
	_c_setbyte(0x9b);
	_ce;
}
static void PUSHF(t_nubit8 byte)
{
	_cb("PUSHF");
	_SetOperandSize(byte);
	_c_setbyte(0x9c);
	_ce;
}
static void POPF(t_nubit8 byte)
{
	_cb("POPF");
	_SetOperandSize(byte);
	_c_setbyte(0x9d);
	_ce;
}
static void SAHF()
{
	_cb("SAHF");
	_c_setbyte(0x9e);
	_ce;
}
static void LAHF()
{
	_cb("LAHF");
	_c_setbyte(0x9f);
	_ce;
}
static void MOV_AL_MOFFS8()
{
	_cb("MOV_AL_MOFFS8");
	_c_setbyte(0xa0);
	if (aopri2.mem == MEM_BP) {
		_bb("16-bit Addressing");
		_SetAddressSize(2);
		_chk(_c_imm16(aopri2.disp16));
		_be;
	} else if (aopri2.mem == MEM_EBP) {
		_bb("32-bit Addressing");
		_SetAddressSize(4);
		_chk(_c_imm16(aopri2.disp32));
		_be;
	} else _se_;
	_ce;
}
static void MOV_EAX_MOFFS32(t_nubit8 byte)
{
	_cb("MOV_EAX_MOFFS32");
	_SetOperandSize(byte);
	_c_setbyte(0xa1);
	if (aopri2.mem == MEM_BP) {
		_bb("16-bit Addressing");
		_SetAddressSize(2);
		_chk(_c_imm16(aopri2.disp16));
		_be;
	} else if (aopri2.mem == MEM_EBP) {
		_bb("32-bit Addressing");
		_SetAddressSize(4);
		_chk(_c_imm32(aopri2.disp32));
		_be;
	} else _se_;
	_ce;
}
static void MOV_MOFFS8_AL()
{
	_cb("MOV_MOFFS8_AL");
	_c_setbyte(0xa2);
	if (aopri1.mem == MEM_BP) {
		_bb("16-bit Addressing");
		_SetAddressSize(2);
		_chk(_c_imm16(aopri1.disp16));
		_be;
	} else if (aopri1.mem == MEM_EBP) {
		_bb("32-bit Addressing");
		_SetAddressSize(4);
		_chk(_c_imm16(aopri1.disp32));
		_be;
	} else _se_;
	_ce;
}
static void MOV_MOFFS32_EAX(t_nubit8 byte)
{
	_cb("MOV_MOFFS32_EAX");
	_SetOperandSize(byte);
	_c_setbyte(0xa3);
	if (aopri1.mem == MEM_BP) {
		_bb("16-bit Addressing");
		_SetAddressSize(2);
		_chk(_c_imm16(aopri1.disp16));
		_be;
	} else if (aopri1.mem == MEM_EBP) {
		_bb("32-bit Addressing");
		_SetAddressSize(4);
		_chk(_c_imm32(aopri1.disp32));
		_be;
	} else _se_;
	_ce;
}
static void MOVSB()
{
	_cb("MOVSB");
	_c_setbyte(0xa4);
	rinfo = &aopri2;
	if (rinfo->flagds) rinfo->flagds = 0;
	if (ARG_NONE) rinfo = NULL;
	else if (ARG_ESDI8_DSSI8) _SetAddressSize(2);
	else if (ARG_ESEDI8_DSESI8) _SetAddressSize(4);
	else _se_;
	_ce;
}
static void MOVSW(t_nubit8 byte)
{
	_cb("MOVSW");
	_SetOperandSize(byte);
	_c_setbyte(0xa5);
	rinfo = &aopri2;
	if (rinfo->flagds) rinfo->flagds = 0;
	switch (byte) {
	case 2: _bb("byte(2)");
		if (ARG_NONE) rinfo = NULL;
		else if (ARG_ESDI16_DSSI16) _SetAddressSize(2);
		else if (ARG_ESEDI16_DSESI16) _SetAddressSize(4);
		else _se_;
		_be;break;
	case 4: _bb("byte(4)");
		if (ARG_NONE) rinfo = NULL;
		else if (ARG_ESDI32_DSSI32) _SetAddressSize(2);
		else if (ARG_ESEDI32_DSESI32) _SetAddressSize(4);
		else _se_;
		_be;break;
	default: _se_;break;}
	_ce;
}
static void CMPSB()
{
	_cb("CMPSB");
	_c_setbyte(0xa6);
	rinfo = &aopri1;
	if (rinfo->flagds) rinfo->flagds = 0;
	if (ARG_NONE) rinfo = NULL;
	else if (ARG_DSSI8_ESDI8) _SetAddressSize(2);
	else if (ARG_DSESI8_ESEDI8) _SetAddressSize(4);
	else _se_;
	_ce;
}
static void CMPSW(t_nubit8 byte)
{
	_cb("CMPSW");
	_SetOperandSize(byte);
	_c_setbyte(0xa7);
	rinfo = &aopri1;
	if (rinfo->flagds) rinfo->flagds = 0;
	switch (byte) {
	case 2: _bb("byte(2)");
		if (ARG_NONE) rinfo = NULL;
		else if (ARG_DSSI16_ESDI16) _SetAddressSize(2);
		else if (ARG_DSESI16_ESEDI16) _SetAddressSize(4);
		else _se_;
		_be;break;
	case 4: _bb("byte(4)");
		if (ARG_NONE) rinfo = NULL;
		else if (ARG_DSSI32_ESDI32) _SetAddressSize(2);
		else if (ARG_DSESI32_ESEDI32) _SetAddressSize(4);
		else _se_;
		_be;break;
	default: _se_;break;}
	_ce;
}
static void TEST_AL_I8()
{
	_cb("TEST_AL_I8");
	_c_setbyte(0xa8);
	_chk(_c_imm8(aopri2.imm8));
	_ce;
}
static void TEST_EAX_I32(t_nubit8 byte)
{
	_cb("TEST_EAX_I32");
	_SetOperandSize(byte);
	_c_setbyte(0xa9);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_imm16(aopri2.imm16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_imm32(aopri2.imm32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void STOSB()
{
	_cb("STOSB");
	_c_setbyte(0xaa);
	rinfo = NULL;
	if (ARG_NONE) ;
	else if (ARG_ESDI8) _SetAddressSize(2);
	else if (ARG_ESEDI8) _SetAddressSize(4);
	else _se_;
	_ce;
}
static void STOSW(t_nubit8 byte)
{
	_cb("STOSW");
	_SetOperandSize(byte);
	_c_setbyte(0xab);
	rinfo = NULL;
	switch (byte) {
	case 2: _bb("byte(2)");
		if (ARG_NONE) ;
		else if (ARG_ESDI16) _SetAddressSize(2);
		else if (ARG_ESEDI16) _SetAddressSize(4);
		else _se_;
		_be;break;
	case 4: _bb("byte(4)");
		if (ARG_NONE) ;
		else if (ARG_ESDI32) _SetAddressSize(2);
		else if (ARG_ESEDI32) _SetAddressSize(4);
		else _se_;
		_be;break;
	default: _se_;break;}
	_ce;
}
static void LODSB()
{
	_cb("LODSB");
	_c_setbyte(0xac);
	rinfo = &aopri1;
	if (rinfo->flagds) rinfo->flagds = 0;
	if (ARG_NONE) rinfo = NULL;
	else if (ARG_DSSI8) _SetAddressSize(2);
	else if (ARG_DSESI8) _SetAddressSize(4);
	else _se_;
	_ce;
}
static void LODSW(t_nubit8 byte)
{
	_cb("LODSW");
	_SetOperandSize(byte);
	_c_setbyte(0xad);
	rinfo = &aopri1;
	if (rinfo->flagds) rinfo->flagds = 0;
	switch (byte) {
	case 2: _bb("byte(2)");
		if (ARG_NONE) rinfo = NULL;
		else if (ARG_DSSI16) _SetAddressSize(2);
		else if (ARG_DSESI16) _SetAddressSize(4);
		else _se_;
		_be;break;
	case 4: _bb("byte(4)");
		if (ARG_NONE) rinfo = NULL;
		else if (ARG_DSSI32) _SetAddressSize(2);
		else if (ARG_DSESI32) _SetAddressSize(4);
		else _se_;
		_be;break;
	default: _se_;break;}
	_ce;
}
static void SCASB()
{
	_cb("SCASB");
	_c_setbyte(0xae);
	rinfo = NULL;
	if (ARG_NONE) ;
	else if (ARG_ESDI8) _SetAddressSize(2);
	else if (ARG_ESEDI8) _SetAddressSize(4);
	else _se_;
	_ce;
}
static void SCASW(t_nubit8 byte)
{
	_cb("SCASW");
	_SetOperandSize(byte);
	_c_setbyte(0xaf);
	rinfo = NULL;
	switch (byte) {
	case 2: _bb("byte(2)");
		if (ARG_NONE) ;
		else if (ARG_ESDI16) _SetAddressSize(2);
		else if (ARG_ESEDI16) _SetAddressSize(4);
		else _se_;
		_be;break;
	case 4: _bb("byte(4)");
		if (ARG_NONE) ;
		else if (ARG_ESDI32) _SetAddressSize(2);
		else if (ARG_ESEDI32) _SetAddressSize(4);
		else _se_;
		_be;break;
	default: _se_;break;}
	_ce;
}
static void MOV_AL_I8()
{
	_cb("MOV_AL_I8");
	_c_setbyte(0xb0);
	_chk(_c_imm8(aopri2.imm8));
	_ce;
}
static void MOV_CL_I8()
{
	_cb("MOV_CL_I8");
	_c_setbyte(0xb1);
	_chk(_c_imm8(aopri2.imm8));
	_ce;
}
static void MOV_DL_I8()
{
	_cb("MOV_DL_I8");
	_c_setbyte(0xb2);
	_chk(_c_imm8(aopri2.imm8));
	_ce;
}
static void MOV_BL_I8()
{
	_cb("MOV_BL_I8");
	_c_setbyte(0xb3);
	_chk(_c_imm8(aopri2.imm8));
	_ce;
}
static void MOV_AH_I8()
{
	_cb("MOV_AH_I8");
	_c_setbyte(0xb4);
	_chk(_c_imm8(aopri2.imm8));
	_ce;
}
static void MOV_CH_I8()
{
	_cb("MOV_CH_I8");
	_c_setbyte(0xb5);
	_chk(_c_imm8(aopri2.imm8));
	_ce;
}
static void MOV_DH_I8()
{
	_cb("MOV_DH_I8");
	_c_setbyte(0xb6);
	_chk(_c_imm8(aopri2.imm8));
	_ce;
}
static void MOV_BH_I8()
{
	_cb("MOV_BH_I8");
	_c_setbyte(0xb7);
	_chk(_c_imm8(aopri2.imm8));
	_ce;
}
static void MOV_EAX_I32(t_nubit8 byte)
{
	_cb("MOV_EAX_I32");
	_SetOperandSize(byte);
	_c_setbyte(0xb8);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_imm16(aopri2.imm16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_imm32(aopri2.imm32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void MOV_ECX_I32(t_nubit8 byte)
{
	_cb("MOV_ECX_I32");
	_SetOperandSize(byte);
	_c_setbyte(0xb9);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_imm16(aopri2.imm16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_imm32(aopri2.imm32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void MOV_EDX_I32(t_nubit8 byte)
{
	_cb("MOV_EDX_I32");
	_SetOperandSize(byte);
	_c_setbyte(0xba);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_imm16(aopri2.imm16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_imm32(aopri2.imm32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void MOV_EBX_I32(t_nubit8 byte)
{
	_cb("MOV_EBX_I32");
	_SetOperandSize(byte);
	_c_setbyte(0xbb);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_imm16(aopri2.imm16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_imm32(aopri2.imm32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void MOV_ESP_I32(t_nubit8 byte)
{
	_cb("MOV_ESP_I32");
	_SetOperandSize(byte);
	_c_setbyte(0xbc);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_imm16(aopri2.imm16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_imm32(aopri2.imm32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void MOV_EBP_I32(t_nubit8 byte)
{
	_cb("MOV_EBP_I32");
	_SetOperandSize(byte);
	_c_setbyte(0xbd);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_imm16(aopri2.imm16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_imm32(aopri2.imm32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void MOV_ESI_I32(t_nubit8 byte)
{
	_cb("MOV_ESI_I32");
	_SetOperandSize(byte);
	_c_setbyte(0xbe);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_imm16(aopri2.imm16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_imm32(aopri2.imm32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void MOV_EDI_I32(t_nubit8 byte)
{
	_cb("MOV_EDI_I32");
	_SetOperandSize(byte);
	_c_setbyte(0xbf);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_imm16(aopri2.imm16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_imm32(aopri2.imm32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void INS_C0(t_nubit8 rid)
{
	_cb("INS_C0");
	_c_setbyte(0xc0);
	_chk(_c_modrm(aopri1, rid));
	_chk(_c_imm8(aopri2.imm8));
	_ce;
}
static void INS_C1(t_nubit8 rid, t_nubit8 byte)
{
	_cb("INS_C1");
	_SetOperandSize(byte);
	_c_setbyte(0xc1);
	_chk(_c_modrm(aopri1, rid));
	_chk(_c_imm8(aopri2.imm8));
	_ce;
}
static void RET_I16()
{
	_cb("RET_I16");
	_c_setbyte(0xc2);
	_chk(_c_imm16(aopri1.imm16));
	_ce;
}
static void RET_()
{
	_cb("RET");
	_c_setbyte(0xc3);
	_ce;
}
static void LES_R32_M16_32(t_nubit8 byte)
{
	_cb("LES_R32_M16_32");
	_SetOperandSize(byte);
	_c_setbyte(0xc4);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri2, aopri1.reg16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_modrm(aopri2, aopri1.reg32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void LDS_R32_M16_32(t_nubit8 byte)
{
	_cb("LDS_R32_M16_32");
	_c_setbyte(0xc5);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri2, aopri1.reg16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_modrm(aopri2, aopri1.reg32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void INS_C6(t_nubit8 rid)
{
	_cb("INS_C6");
	_c_setbyte(0xc6);
	_chk(_c_modrm(aopri1, rid));
	_chk(_c_imm8(aopri2.imm8));
	_ce;
}
static void INS_C7(t_nubit8 rid, t_nubit8 byte)
{
	_cb("INS_C7");
	_SetOperandSize(byte);
	_c_setbyte(0xc7);
	_chk(_c_modrm(aopri1, rid));
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_imm16(aopri2.imm16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_imm32(aopri2.imm32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void ENTER()
{
	_cb("ENTER");
	if (ARG_I16_I8) {
		_bb("ARG_I16_I8");
		_c_setbyte(0xc8);
		_chk(_c_imm16(aopri1.imm16));
		_chk(_c_imm8(aopri2.imm8));
		_be;
	} else _se_;
	_ce;
}
static void LEAVE()
{
	_cb("LEAVE");
	if (ARG_NONE) _c_setbyte(0xc9);
	else _se_;
	_ce;
}
static void RETF_I16()
{
	_cb("RETF_I16");
	_c_setbyte(0xca);
	_chk(_c_imm16(aopri1.imm16));
	_ce;
}
static void RETF_()
{
	_cb("RETF_");
	_c_setbyte(0xcb);
	_ce;
}
static void INT3()
{
	_cb("INT3");
	_c_setbyte(0xcc);
	_ce;
}
static void INT_I8()
{
	_cb("INT_I8");
	_c_setbyte(0xcd);
	_chk(_c_imm8(aopri1.imm8));
	_ce;
}
static void INTO()
{
	_cb("INTO");
	_c_setbyte(0xcd);
	_ce;
}
static void IRET(t_nubit8 byte)
{
	_cb("IRET");
	_SetOperandSize(byte);
	_c_setbyte(0xcf);
	_ce;
}
static void INS_D0(t_nubit8 rid)
{
	_cb("INS_DO");
	_c_setbyte(0xd0);
	_chk(_c_modrm(aopri1, rid));
	_ce;
}
static void INS_D1(t_nubit8 rid, t_nubit8 byte)
{
	_cb("INS_D1");
	_SetOperandSize(byte);
	_c_setbyte(0xd1);
	_chk(_c_modrm(aopri1, rid));
	_ce;
}
static void INS_D2(t_nubit8 rid)
{
	_cb("INS_D2");
	_c_setbyte(0xd2);
	_chk(_c_modrm(aopri1, rid));
	_ce;
}
static void INS_D3(t_nubit8 rid, t_nubit8 byte)
{
	_cb("INS_D3");
	_SetOperandSize(byte);
	_c_setbyte(0xd3);
	_chk(_c_modrm(aopri1, rid));
	_ce;
}
static void AAM()
{
	_cb("AAM");
	if (ARG_NONE) {
		_bb("ARG_NONE");
		_c_setbyte(0xd4);
		_chk(_c_imm8(0x0a));
		_be;
	} else if (ARG_I8) {
		_bb("ARG_I8");
		_c_setbyte(0xd4);
		_chk(_c_imm8(aopri1.imm8));
		_be;
	} else _se_;
	_ce;
}
static void AAD()
{
	_cb("AAD");
	if (ARG_NONE) {
		_bb("ARG_NONE");
		_c_setbyte(0xd5);
		_chk(_c_imm8(0x0a));
		_be;
	} else if (ARG_I8) {
		_bb("ARG_I8");
		_c_setbyte(0xd5);
		_chk(_c_imm8(aopri1.imm8));
		_be;
	} else _se_;
	_ce;
}
static void XLATB()
{
	_cb("XLATB");
	rinfo = &aopri1;
	if (rinfo->flagds) rinfo->flagds = 0;
	_c_setbyte(0xd7);
	if (ARG_DSBXAL8) {
		_SetAddressSize(2);
	} else if (ARG_DSEBXAL8) {
		_SetAddressSize(4);
	} else _se_;
	_ce;
}
static void IN_AL_I8()
{
	_cb("IN_AL_I8");
	_c_setbyte(0xe4);
	_chk(_c_imm8(aopri2.imm8));
	_ce;
}
static void IN_EAX_I8(t_nubit8 byte)
{
	_cb("IN_AL_I8");
	_SetOperandSize(byte);
	_c_setbyte(0xe5);
	_chk(_c_imm8(aopri2.imm8));
	_ce;
}
static void OUT_I8_AL()
{
	_cb("OUT_I8_AL");
	_c_setbyte(0xe6);
	_chk(_c_imm8(aopri1.imm8));
	_ce;
}
static void OUT_I8_EAX(t_nubit8 byte)
{
	_cb("OUT_I8_EAX");
	_c_setbyte(0xe7);
	_chk(_c_imm8(aopri1.imm8));
	_ce;
}
static void CALL_REL32(t_nubit8 byte)
{
	_cb("CALL_REL32");
	_SetOperandSize(byte);
	_c_setbyte(0xe8);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_imm16(aopri1.imm16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_imm32(aopri1.imm32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void JMP_REL32(t_nubit8 byte)
{
	_cb("JMP_REL32");
	_SetOperandSize(byte);
	_c_setbyte(0xe9);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_imm16(aopri1.imm16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_imm32(aopri1.imm32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void JMP_PTR16_32(t_nubit8 byte)
{
	_cb("JMP_PTR16_32");
	_SetOperandSize(byte);
	_c_setbyte(0xea);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_imm16(GetMax16(aopri1.reip)));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_imm32(GetMax32(aopri1.reip)));
		_be;break;
	default: _se_;break;}
	_chk(_c_imm16(aopri1.rcs));
	_ce;
}
static void IN_AL_DX()
{
	_cb("IN_AL_DX");
	_c_setbyte(0xec);
	_ce;
}
static void IN_EAX_DX(t_nubit8 byte)
{
	_cb("IN_EAX_DX");
	_SetOperandSize(byte);
	_c_setbyte(0xed);
	_ce;
}
static void OUT_DX_AL()
{
	_cb("OUT_DX_AL");
	_c_setbyte(0xee);
	_ce;
}
static void OUT_DX_EAX(t_nubit8 byte)
{
	_cb("OUT_DX_EAX");
	_SetOperandSize(byte);
	_c_setbyte(0xef);
	_ce;
}
static void PREFIX_LOCK()
{
	_cb("PREFIX_LOCK");
	if (ARG_NONE) prefix_lock = 1;
	else _se_;
	_ce;
}
static void QDX()
{
	_cb("PREFIX_LOCK");
	if (ARG_I8) {
		_c_setbyte(0xf1);
		_chk(_c_imm8(aopri1.imm8));
	} else _se_;
	_ce;
}
static void PREFIX_REPNZ()
{
	_cb("PREFIX_REPNZ");
	if (ARG_NONE) prefix_repnz = 1;
	else _se_;
	_ce;
}
static void PREFIX_REPZ()
{
	_cb("PREFIX_REPZ");
	if (ARG_NONE) prefix_repz = 1;
	else _se_;
	_ce;
}
static void HLT()
{
	_cb("HLT");
	if (ARG_NONE) _c_setbyte(0xf4);
	else _se_;
	_ce;
}
static void CMC()
{
	_cb("CMC");
	if (ARG_NONE) _c_setbyte(0xf5);
	else _se_;
	_ce;
}
static void INS_F6(t_nubit8 rid)
{
	_cb("INS_F6");
	_c_setbyte(0xf6);
	_chk(_c_modrm(aopri1, rid));
	if (!rid) _chk(_c_imm8(aopri2.imm8));
	_ce;
}
static void INS_F7(t_nubit8 rid, t_nubit8 byte)
{
	_cb("INS_F7");
	_SetOperandSize(byte);
	_c_setbyte(0xf7);
	_chk(_c_modrm(aopri1, rid));
	if (!rid) {
		_bb("!rid");
		switch (byte) {
		case 2: _bb("byte(2)");
			_chk(_c_imm16(aopri2.imm16));
			_be;break;
		case 4: _bb("byte(4)");
			_chk(_c_imm32(aopri2.imm32));
			_be;break;
		default: _se_;break;}
		_be;
	}
	_ce;
}
static void CLC()
{
	_cb("CLC");
	if (ARG_NONE) _c_setbyte(0xf8);
	else _se_;
	_ce;
}
static void STC()
{
	_cb("STC");
	if (ARG_NONE) _c_setbyte(0xf9);
	else _se_;
	_ce;
}
static void CLI()
{
	_cb("CLI");
	if (ARG_NONE) _c_setbyte(0xfa);
	else _se_;
	_ce;
}
static void STI()
{
	_cb("STI");
	if (ARG_NONE) _c_setbyte(0xfb);
	else _se_;
	_ce;
}
static void CLD()
{
	_cb("CLD");
	if (ARG_NONE) _c_setbyte(0xfc);
	else _se_;
	_ce;
}
static void STD()
{
	_cb("STD");
	if (ARG_NONE) _c_setbyte(0xfd);
	else _se_;
	_ce;
}
static void INS_FE(t_nubit8 rid)
{
	_cb("INS_FE");
	_c_setbyte(0xfe);
	_chk(_c_modrm(aopri1, rid));
	_ce;
}
static void INS_FF(t_nubit8 rid, t_nubit8 byte)
{
	_cb("INS_FF");
	_SetOperandSize(byte);
	_c_setbyte(0xff);
	_chk(_c_modrm(aopri1, rid));
	_ce;
}
/* concrete extended instructions */
static void INS_0F_00(t_nubit8 rid, t_nubit8 byte)
{
	_cb("INS_0F_00");
	_SetOperandSize(byte);
	INS_0F();
	_c_setbyte(0x00);
	_chk(_c_modrm(aopri1, rid));
	_ce;
}
static void INS_0F_01(t_nubit8 rid, t_nubit8 byte)
{
	_cb("INS_0F_01");
	_SetOperandSize(byte);
	INS_0F();
	_c_setbyte(0x01);
	_chk(_c_modrm(aopri1, rid));
	_ce;
}
static void LAR_R32_RM32(t_nubit8 byte)
{
	_cb("LAR_R32_RM32");
	_SetOperandSize(byte);
	INS_0F();
	_c_setbyte(0x02);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri2, aopri1.reg16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_modrm(aopri2, aopri1.reg32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void LSL_R32_RM32(t_nubit8 byte)
{
	_cb("LSL_R32_RM32");
	_SetOperandSize(byte);
	INS_0F();
	_c_setbyte(0x03);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri2, aopri1.reg16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_modrm(aopri2, aopri1.reg32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void CLTS()
{
	_cb("CLTS");
	INS_0F();
	_c_setbyte(0x06);
	_ce;
}
static void MOV_R32_CR(t_nubit8 crid)
{
	_cb("MOV_R32_CR");
	INS_0F();
	_c_setbyte(0x20);
	_chk(_c_modrm(aopri1, crid));
	_ce;
}
static void MOV_R32_DR(t_nubit8 drid)
{
	_cb("MOV_R32_DR");
	INS_0F();
	_c_setbyte(0x21);
	_chk(_c_modrm(aopri1, drid));
	_ce;
}
static void MOV_CR_R32(t_nubit8 crid)
{
	_cb("MOV_CR_R32");
	INS_0F();
	_c_setbyte(0x22);
	_chk(_c_modrm(aopri2, crid));
	_ce;
}
static void MOV_DR_R32(t_nubit8 drid)
{
	_cb("MOV_DR_R32");
	INS_0F();
	_c_setbyte(0x23);
	_chk(_c_modrm(aopri2, drid));
	_ce;
}
static void MOV_R32_TR(t_nubit8 trid)
{
	_cb("MOV_R32_TR");
	INS_0F();
	_c_setbyte(0x24);
	_chk(_c_modrm(aopri1, trid));
	_ce;
}
static void MOV_TR_R32(t_nubit8 trid)
{
	_cb("MOV_TR_R32");
	INS_0F();
	_c_setbyte(0x26);
	_chk(_c_modrm(aopri2, trid));
	_ce;
}
static void SETCC_RM8(t_nubit8 opcode)
{
	_cb("SETCC_RM8");
	if (ARG_RM8) {
		INS_0F();
		_c_setbyte(opcode);
		_chk(_c_modrm(aopri1, 0));
	} else _se_;
	_ce;
}
static void PUSH_FS()
{
	_cb("PUSH_FS");
	INS_0F();
	_c_setbyte(0xa0);
	_ce;
}
static void POP_FS()
{
	_cb("POP_FS");
	INS_0F();
	_c_setbyte(0xa1);
	_ce;
}
static void BT_RM32_R32(t_nubit8 byte)
{
	_cb("BT_RM32_R32");
	_SetOperandSize(byte);
	INS_0F();
	_c_setbyte(0xa3);
	switch (byte) {
	case 2: _chk(_c_modrm(aopri1, aopri2.reg16));break;
	case 4: _chk(_c_modrm(aopri1, aopri2.reg32));break;
	default: _se_;break;}
	_ce;
}
static void SHLD_RM32_R32_I8(t_nubit8 byte)
{
	_cb("SHLD_RM32_R32_I8");
	_SetOperandSize(byte);
	INS_0F();
	_c_setbyte(0xa4);
	switch (byte) {
	case 2: _chk(_c_modrm(aopri1, aopri2.reg16));break;
	case 4: _chk(_c_modrm(aopri1, aopri2.reg32));break;
	default: _se_;break;}
	_chk(_c_imm8(aopri3.imm8));
	_ce;
}
static void SHLD_RM32_R32_CL(t_nubit8 byte)
{
	_cb("SHLD_RM32_R32_CL");
	_SetOperandSize(byte);
	INS_0F();
	_c_setbyte(0xa5);
	switch (byte) {
	case 2: _chk(_c_modrm(aopri1, aopri2.reg16));break;
	case 4: _chk(_c_modrm(aopri1, aopri2.reg32));break;
	default: _se_;break;}
	_ce;
}
static void PUSH_GS()
{
	_cb("PUSH_GS");
	INS_0F();
	_c_setbyte(0xa8);
	_ce;
}
static void POP_GS()
{
	_cb("POP_GS");
	INS_0F();
	_c_setbyte(0xa9);
	_ce;
}
static void BTS_RM32_R32(t_nubit8 byte)
{
	_cb("BTS_RM32_R32");
	_SetOperandSize(byte);
	INS_0F();
	_c_setbyte(0xab);
	switch (byte) {
	case 2: _chk(_c_modrm(aopri1, aopri2.reg16));break;
	case 4: _chk(_c_modrm(aopri1, aopri2.reg32));break;
	default: _se_;break;}
	_ce;
}
static void SHRD_RM32_R32_I8(t_nubit8 byte)
{
	_cb("SHRD_RM32_R32_I8");
	_SetOperandSize(byte);
	INS_0F();
	_c_setbyte(0xac);
	switch (byte) {
	case 2: _chk(_c_modrm(aopri1, aopri2.reg16));break;
	case 4: _chk(_c_modrm(aopri1, aopri2.reg32));break;
	default: _se_;break;}
	_chk(_c_imm8(aopri3.imm8));
	_ce;
}
static void SHRD_RM32_R32_CL(t_nubit8 byte)
{
	_cb("SHRD_RM32_R32_CL");
	_SetOperandSize(byte);
	INS_0F();
	_c_setbyte(0xad);
	switch (byte) {
	case 2: _chk(_c_modrm(aopri1, aopri2.reg16));break;
	case 4: _chk(_c_modrm(aopri1, aopri2.reg32));break;
	default: _se_;break;}
	_ce;
}
static void IMUL_R32_RM32(t_nubit8 byte)
{
	_cb("IMUL_R32_RM32");
	_SetOperandSize(byte);
	INS_0F();
	_c_setbyte(0xab);
	switch (byte) {
	case 2: _chk(_c_modrm(aopri2, aopri1.reg16));break;
	case 4: _chk(_c_modrm(aopri2, aopri1.reg32));break;
	default: _se_;break;}
	_ce;
}
static void LSS_R32_M16_32(t_nubit8 byte)
{
	_cb("LSS_R32_M16_32");
	_SetOperandSize(byte);
	INS_0F();
	_c_setbyte(0xb2);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri2, aopri1.reg16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_modrm(aopri2, aopri1.reg32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void BTR_RM32_R32(t_nubit8 byte)
{
	_cb("BTR_RM32_R32");
	_SetOperandSize(byte);
	INS_0F();
	_c_setbyte(0xb3);
	switch (byte) {
	case 2: _chk(_c_modrm(aopri1, aopri2.reg16));break;
	case 4: _chk(_c_modrm(aopri1, aopri2.reg32));break;
	default: _se_;break;}
	_ce;
}
static void LFS_R32_M16_32(t_nubit8 byte)
{
	_cb("LFS_R32_M16_32");
	_SetOperandSize(byte);
	INS_0F();
	_c_setbyte(0xb4);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri2, aopri1.reg16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_modrm(aopri2, aopri1.reg32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void LGS_R32_M16_32(t_nubit8 byte)
{
	_cb("LGS_R32_M16_32");
	_SetOperandSize(byte);
	INS_0F();
	_c_setbyte(0xb5);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri2, aopri1.reg16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_modrm(aopri2, aopri1.reg32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void MOVZX_R32_RM8(t_nubit8 byte)
{
	_cb("MOVZX_R32_RM8");
	_SetOperandSize(byte);
	INS_0F();
	_c_setbyte(0xb6);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri2, aopri1.reg16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_modrm(aopri2, aopri1.reg32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void MOVZX_R32_RM16()
{
	_cb("MOVZX_R32_RM16");
	INS_0F();
	_c_setbyte(0xb7);
	_chk(_c_modrm(aopri2, aopri1.reg32));
	_ce;
}
static void INS_0F_BA(t_nubit8 rid, t_nubit8 byte)
{
	_cb("INS_0F_BA");
	_SetOperandSize(byte);
	INS_0F();
	_c_setbyte(0xba);
	_chk(_c_modrm(aopri1, rid));
	_chk(_c_imm8(aopri2.imm8));
	_ce;
}
static void BTC_RM32_R32(t_nubit8 byte)
{
	_cb("BTC_RM32_R32");
	_SetOperandSize(byte);
	INS_0F();
	_c_setbyte(0xbb);
	switch (byte) {
	case 2: _chk(_c_modrm(aopri1, aopri2.reg16));break;
	case 4: _chk(_c_modrm(aopri1, aopri2.reg32));break;
	default: _se_;break;}
	_ce;
}
static void BSF_R32_RM32(t_nubit8 byte)
{
	_cb("BSF_R32_RM32");
	_SetOperandSize(byte);
	INS_0F();
	_c_setbyte(0xbc);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri2, aopri1.reg16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_modrm(aopri2, aopri1.reg32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void BSR_R32_RM32(t_nubit8 byte)
{
	_cb("BSR_R32_RM32");
	_SetOperandSize(byte);
	INS_0F();
	_c_setbyte(0xbd);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri2, aopri1.reg16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_modrm(aopri2, aopri1.reg32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void MOVSX_R32_RM8(t_nubit8 byte)
{
	_cb("MOVSX_R32_RM8");
	_SetOperandSize(byte);
	INS_0F();
	_c_setbyte(0xbe);
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_c_modrm(aopri2, aopri1.reg16));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_c_modrm(aopri2, aopri1.reg32));
		_be;break;
	default: _se_;break;}
	_ce;
}
static void MOVSX_R32_RM16()
{
	_cb("MOVSX_R32_RM16");
	INS_0F();
	_c_setbyte(0xbf);
	_chk(_c_modrm(aopri2, aopri1.reg32));
	_ce;
}

/* abstract instructions */
static void PUSH()
{
	_cb("PUSH");
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
	else _se_;
	_ce;
}
static void POP()
{
	_cb("POP");
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
	else _se_;
	_ce;
}
static void ADD()
{
	t_nubit8 rid = 0x00;
	_cb("ADD");
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
		else
			if (aopri2.imm8 > 0x7f)
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
		else
			if (aopri2.imm8 > 0x7f)
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
	else _se_;
	_ce;
}
static void OR()
{
	t_nubit8 rid = 0x01;
	_cb("OR");
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
		else
			if (aopri2.imm8 > 0x7f)
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
		else
			if (aopri2.imm8 > 0x7f)
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
	else _se_;
	_ce;
}
static void ADC()
{
	t_nubit8 rid = 0x02;
	_cb("ADC");
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
		else
			if (aopri2.imm8 > 0x7f)
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
		else
			if (aopri2.imm8 > 0x7f)
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
	else _se_;
	_ce;
}
static void SBB()
{
	t_nubit8 rid = 0x03;
	_cb("SBB");
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
		else
			if (aopri2.imm8 > 0x7f)
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
		else
			if (aopri2.imm8 > 0x7f)
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
	else _se_;
	_ce;
}
static void AND()
{
	t_nubit8 rid = 0x04;
	_cb("AND");
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
		else
			if (aopri2.imm8 > 0x7f)
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
		else
			if (aopri2.imm8 > 0x7f)
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
	else _se_;
	_ce;
}
static void SUB()
{
	t_nubit8 rid = 0x05;
	_cb("SUB");
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
		else
			if (aopri2.imm8 > 0x7f)
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
		else
			if (aopri2.imm8 > 0x7f)
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
	else _se_;
	_ce;
}
static void XOR()
{
	t_nubit8 rid = 0x06;
	_cb("XOR");
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
		else
			if (aopri2.imm8 > 0x7f)
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
		else
			if (aopri2.imm8 > 0x7f)
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
	else _se_;
	_ce;
}
static void CMP()
{
	t_nubit8 rid = 0x07;
	_cb("CMP");
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
		else
			if (aopri2.imm8 > 0x7f)
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
		else
			if (aopri2.imm8 > 0x7f)
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
	else _se_;
	_ce;
}
static void INC()
{
	_cb("INC");
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
	else _se_;
	_ce;
}
static void DEC()
{
	_cb("DEC");
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
	else _se_;
	_ce;
}
static void BOUND()
{
	_cb("BOUND");
	if (ARG_R16_M16) BOUND_R32_M32_32(2);
	else if (ARG_R32_M32) BOUND_R32_M32_32(4);
	else _se_;
	_ce;
}
static void IMUL()
{
	_cb("IMUL");
	if      (ARG_RM8s) INS_F6(0x05);
	else if (ARG_RM16s) INS_F7(0x05, 2);
	else if (ARG_RM32s) INS_F7(0x05, 4);
	else if (ARG_R16_RM16) IMUL_R32_RM32(2);
	else if (ARG_R32_RM32) IMUL_R32_RM32(4);
	else if (ARG_R16_RM16_I8)  IMUL_R32_RM32_I8(2);
	else if (ARG_R32_RM32_I8)  IMUL_R32_RM32_I8(4);
	else if (ARG_R16_RM16_I16) IMUL_R32_RM32_I32(2);
	else if (ARG_R32_RM32_I32) IMUL_R32_RM32_I32(4);
	else _se_;
	_ce;
}
static void INS()
{
	_cb("INS");
	if (ARG_ESDI8s_DX || ARG_ESEDI8s_DX) INSB();
	else if (ARG_ESDI16s_DX || ARG_ESEDI16s_DX) INSW(2);
	else if (ARG_ESDI32s_DX || ARG_ESEDI32s_DX) INSW(4);
	else _se_;
	_ce;
}
static void OUTS()
{
	_cb("OUTS");
	if (ARG_DX_DSSI8s || ARG_DX_DSESI8s) OUTSB();
	else if (ARG_DX_DSSI16s || ARG_DX_DSESI16s) OUTSW(2);
	else if (ARG_DX_DSSI32s || ARG_DX_DSESI32s) OUTSW(4);
	else _se_;
	_ce;
}
static void JCC_REL(t_nubit8 opcode)
{
	_cb("JCC_REL");
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
	} else _se_;
	_ce;
}
static void TEST()
{
	_cb("TEST");
	if      (ARG_AL_I8) TEST_AL_I8();
	else if (ARG_AX_I16) TEST_EAX_I32(2);
	else if (ARG_EAX_I32) TEST_EAX_I32(4);
	else if (ARG_RM8_R8) TEST_RM8_R8();
	else if (ARG_RM16_R16) TEST_RM32_R32(2);
	else if (ARG_RM32_R32) TEST_RM32_R32(4);
	else if (ARG_RM8_I8) INS_F6(0x00);
	else if (ARG_RM16_I16) INS_F7(0x00, 2);
	else if (ARG_RM32_I32) INS_F7(0x00, 4);
	else _se_;
	_ce;
}
static void XCHG()
{
	_cb("XCHG");
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
	else _se_;
	_ce;
}
static void MOV()
{
	_cb("MOV");
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
	else _se_;
	_ce;
}
static void LEA()
{
	_cb("LEA");
	if (ARG_R16_M16) LEA_R32_M32(2);
	else if (ARG_R32_M32) LEA_R32_M32(4);
	else _se_;
	_ce;
}
static void CALL()
{
	_cb("CALL");
	if      (ARG_FAR_I16_16) CALL_PTR16_32(2);
	else if (ARG_FAR_I16_32) CALL_PTR16_32(4);
	else if (ARG_NEAR_I16s  || ARG_PNONE_I16s) CALL_REL32(2);
	else if (ARG_NEAR_I32s  || ARG_PNONE_I32s) CALL_REL32(4);
	else if (ARG_NEAR_RM16s || ARG_PNONE_RM16s) INS_FF(0x02, 2);
	else if (ARG_NEAR_RM32s || ARG_PNONE_RM32s) INS_FF(0x02, 4);
	else if (ARG_FAR_M16_16) INS_FF(0x03, 2);
	else if (ARG_FAR_M16_32) INS_FF(0x03, 4);
	else _se_;
	_ce;
}
static void MOVS()
{
	_cb("MOVS");
	if (ARG_ESDI8s_DSSI8s || ARG_ESEDI8s_DSESI8s) MOVSB();
	else if (ARG_ESDI16s_DSSI16s || ARG_ESEDI16s_DSESI16s) MOVSW(2);
	else if (ARG_ESDI32s_DSSI32s || ARG_ESEDI32s_DSESI32s) MOVSW(4);
	else _se_;
	_ce;
}
static void CMPS()
{
	_cb("CMPS");
	if (ARG_DSSI8s_ESDI8s || ARG_DSESI8s_ESEDI8s) CMPSB();
	else if (ARG_DSSI16s_ESDI16s || ARG_DSESI16s_ESEDI16s) CMPSW(2);
	else if (ARG_DSSI32s_ESDI32s || ARG_DSESI32s_ESEDI32s) CMPSW(4);
	else _se_;
	_ce;
}
static void STOS()
{
	_cb("STOS");
	if (ARG_ESDI8s || ARG_ESEDI8s) STOSB();
	else if (ARG_ESDI16s || ARG_ESEDI16s) STOSW(2);
	else if (ARG_ESDI32s || ARG_ESEDI32s) STOSW(4);
	else _se_;
	_ce;
}
static void LODS()
{
	_cb("LODS");
	if (ARG_DSSI8s || ARG_DSESI8s) LODSB();
	else if (ARG_DSSI16s || ARG_DSESI16s) LODSW(2);
	else if (ARG_DSSI32s || ARG_DSESI32s) LODSW(4);
	else _se_;
	_ce;
}
static void SCAS()
{
	_cb("SCAS");
	if (ARG_ESDI8s || ARG_ESEDI8s) SCASB();
	else if (ARG_ESDI16s || ARG_ESEDI16s) SCASW(2);
	else if (ARG_ESDI32s || ARG_ESEDI32s) SCASW(4);
	else _se_;
	_ce;
}
static void RET()
{
	_cb("RET");
	if (ARG_I16u) RET_I16();
	else if (ARG_NONE) RET_();
	else _se_;
	_ce;
}
static void LES()
{
	_cb("LES");
	if (ARG_R16_M16) LES_R32_M16_32(2);
	else if (ARG_R32_M32) LES_R32_M16_32(4);
	else _se_;
	_ce;
}
static void LDS()
{
	_cb("LDS");
	if (ARG_R16_M16) LDS_R32_M16_32(2);
	else if (ARG_R32_M32) LDS_R32_M16_32(4);
	else _se_;
	_ce;
}
static void RETF()
{
	_cb("RETF");
	if      (ARG_I16u) RETF_I16();
	else if (ARG_NONE) RETF_();
	else _se_;
	_ce;
}
static void INT()
{
	_cb("INT");
	if (ARG_I8) INT_I8();
	else _se_;
	_ce;
}
static void XLAT()
{
	_cb("XLAT");
	if (ARG_DSBXAL8 || ARG_DSEBXAL8) XLATB();
	else _se_;
	_ce;
}
static void ROL()
{
	t_nubit8 rid = 0x00;
	_cb("ROL");
	if (ARG_RM8_I8 && aopri2.imm8 == 1) INS_D0(rid);
	else if (ARG_RM16_I8 && aopri2.imm8 == 1) INS_D1(rid, 2);
	else if (ARG_RM32_I8 && aopri2.imm8 == 1) INS_D1(rid, 4);
	else if (ARG_RM8_CL)  INS_D2(rid);
	else if (ARG_RM16_CL) INS_D3(rid, 2);
	else if (ARG_RM32_CL) INS_D3(rid, 4);
	else if (ARG_RM8_I8)  INS_C0(rid);
	else if (ARG_RM16_I8) INS_C1(rid, 2);
	else if (ARG_RM32_I8) INS_C1(rid, 4);
	else _se_;
	_ce;
}
static void ROR()
{
	t_nubit8 rid = 0x01;
	_cb("ROR");
	if (ARG_RM8_I8 && aopri2.imm8 == 1) INS_D0(rid);
	else if (ARG_RM16_I8 && aopri2.imm8 == 1) INS_D1(rid, 2);
	else if (ARG_RM32_I8 && aopri2.imm8 == 1) INS_D1(rid, 4);
	else if (ARG_RM8_CL)  INS_D2(rid);
	else if (ARG_RM16_CL) INS_D3(rid, 2);
	else if (ARG_RM32_CL) INS_D3(rid, 4);
	else if (ARG_RM8_I8)  INS_C0(rid);
	else if (ARG_RM16_I8) INS_C1(rid, 2);
	else if (ARG_RM32_I8) INS_C1(rid, 4);
	else _se_;
	_ce;
}
static void RCL()
{
	t_nubit8 rid = 0x02;
	_cb("RCL");
	if (ARG_RM8_I8 && aopri2.imm8 == 1) INS_D0(rid);
	else if (ARG_RM16_I8 && aopri2.imm8 == 1) INS_D1(rid, 2);
	else if (ARG_RM32_I8 && aopri2.imm8 == 1) INS_D1(rid, 4);
	else if (ARG_RM8_CL)  INS_D2(rid);
	else if (ARG_RM16_CL) INS_D3(rid, 2);
	else if (ARG_RM32_CL) INS_D3(rid, 4);
	else if (ARG_RM8_I8)  INS_C0(rid);
	else if (ARG_RM16_I8) INS_C1(rid, 2);
	else if (ARG_RM32_I8) INS_C1(rid, 4);
	else _se_;
	_ce;
}
static void RCR()
{
	t_nubit8 rid = 0x03;
	_cb("IN");
	if (ARG_RM8_I8 && aopri2.imm8 == 1) INS_D0(rid);
	else if (ARG_RM16_I8 && aopri2.imm8 == 1) INS_D1(rid, 2);
	else if (ARG_RM32_I8 && aopri2.imm8 == 1) INS_D1(rid, 4);
	else if (ARG_RM8_CL)  INS_D2(rid);
	else if (ARG_RM16_CL) INS_D3(rid, 2);
	else if (ARG_RM32_CL) INS_D3(rid, 4);
	else if (ARG_RM8_I8)  INS_C0(rid);
	else if (ARG_RM16_I8) INS_C1(rid, 2);
	else if (ARG_RM32_I8) INS_C1(rid, 4);
	else _se_;
	_ce;
}
static void SHL()
{
	t_nubit8 rid = 0x04;
	_cb("SHL");
	if (ARG_RM8_I8 && aopri2.imm8 == 1) INS_D0(rid);
	else if (ARG_RM16_I8 && aopri2.imm8 == 1) INS_D1(rid, 2);
	else if (ARG_RM32_I8 && aopri2.imm8 == 1) INS_D1(rid, 4);
	else if (ARG_RM8_CL)  INS_D2(rid);
	else if (ARG_RM16_CL) INS_D3(rid, 2);
	else if (ARG_RM32_CL) INS_D3(rid, 4);
	else if (ARG_RM8_I8)  INS_C0(rid);
	else if (ARG_RM16_I8) INS_C1(rid, 2);
	else if (ARG_RM32_I8) INS_C1(rid, 4);
	else _se_;
	_ce;
}
static void SHR()
{
	t_nubit8 rid = 0x05;
	_cb("SHR");
	if (ARG_RM8_I8 && aopri2.imm8 == 1) INS_D0(rid);
	else if (ARG_RM16_I8 && aopri2.imm8 == 1) INS_D1(rid, 2);
	else if (ARG_RM32_I8 && aopri2.imm8 == 1) INS_D1(rid, 4);
	else if (ARG_RM8_CL)  INS_D2(rid);
	else if (ARG_RM16_CL) INS_D3(rid, 2);
	else if (ARG_RM32_CL) INS_D3(rid, 4);
	else if (ARG_RM8_I8)  INS_C0(rid);
	else if (ARG_RM16_I8) INS_C1(rid, 2);
	else if (ARG_RM32_I8) INS_C1(rid, 4);
	else _se_;
	_ce;
}
static void SAL()
{
	t_nubit8 rid = 0x04;
	_cb("SAL");
	if (ARG_RM8_I8 && aopri2.imm8 == 1) INS_D0(rid);
	else if (ARG_RM16_I8 && aopri2.imm8 == 1) INS_D1(rid, 2);
	else if (ARG_RM32_I8 && aopri2.imm8 == 1) INS_D1(rid, 4);
	else if (ARG_RM8_CL)  INS_D2(rid);
	else if (ARG_RM16_CL) INS_D3(rid, 2);
	else if (ARG_RM32_CL) INS_D3(rid, 4);
	else if (ARG_RM8_I8)  INS_C0(rid);
	else if (ARG_RM16_I8) INS_C1(rid, 2);
	else if (ARG_RM32_I8) INS_C1(rid, 4);
	else _se_;
	_ce;
}
static void SAR()
{
	t_nubit8 rid = 0x07;
	_cb("SAR");
	if (ARG_RM8_I8 && aopri2.imm8 == 1) INS_D0(rid);
	else if (ARG_RM16_I8 && aopri2.imm8 == 1) INS_D1(rid, 2);
	else if (ARG_RM32_I8 && aopri2.imm8 == 1) INS_D1(rid, 4);
	else if (ARG_RM8_CL)  INS_D2(rid);
	else if (ARG_RM16_CL) INS_D3(rid, 2);
	else if (ARG_RM32_CL) INS_D3(rid, 4);
	else if (ARG_RM8_I8)  INS_C0(rid);
	else if (ARG_RM16_I8) INS_C1(rid, 2);
	else if (ARG_RM32_I8) INS_C1(rid, 4);
	else _se_;
	_ce;
}
static void IN()
{
	_cb("IN");
	if      (ARG_AL_I8u)  IN_AL_I8();
	else if (ARG_AX_I8u)  IN_EAX_I8(2);
	else if (ARG_EAX_I8u) IN_EAX_I8(4);
	else if (ARG_AL_DX)   IN_AL_DX();
	else if (ARG_AX_DX)   IN_EAX_DX(2);
	else if (ARG_EAX_DX)  IN_EAX_DX(4);
	else _se_;
	_ce;
}
static void OUT()
{
	_cb("OUT");
	if      (ARG_I8u_AL)  OUT_I8_AL();
	else if (ARG_I8u_AX)  OUT_I8_EAX(2);
	else if (ARG_I8u_EAX) OUT_I8_EAX(4);
	else if (ARG_DX_AL)   OUT_DX_AL();
	else if (ARG_DX_AX)   OUT_DX_EAX(2);
	else if (ARG_DX_EAX)  OUT_DX_EAX(4);
	else _se_;
	_ce;
}
static void NOT()
{
	_cb("NOT");
	if      (ARG_RM8s) INS_F6(0x02);
	else if (ARG_RM16s) INS_F7(0x02, 2);
	else if (ARG_RM32s) INS_F7(0x02, 4);
	else _se_;
	_ce;
}
static void NEG()
{
	_cb("NEG");
	if      (ARG_RM8s) INS_F6(0x03);
	else if (ARG_RM16s) INS_F7(0x03, 2);
	else if (ARG_RM32s) INS_F7(0x03, 4);
	else _se_;
	_ce;
}
static void MUL()
{
	_cb("MUL");
	if      (ARG_RM8s) INS_F6(0x04);
	else if (ARG_RM16s) INS_F7(0x04, 2);
	else if (ARG_RM32s) INS_F7(0x04, 4);
	else _se_;
	_ce;
}
static void DIV()
{
	_cb("DIV");
	if      (ARG_RM8s) INS_F6(0x06);
	else if (ARG_RM16s) INS_F7(0x06, 2);
	else if (ARG_RM32s) INS_F7(0x06, 4);
	else _se_;
	_ce;
}
static void IDIV()
{
	_cb("IDIV");
	if      (ARG_RM8s) INS_F6(0x07);
	else if (ARG_RM16s) INS_F7(0x07, 2);
	else if (ARG_RM32s) INS_F7(0x07, 4);
	else _se_;
	_ce;
}
static void JMP()
{
	_cb("JMP");
	if      (ARG_FAR_I16_16) JMP_PTR16_32(2);
	else if (ARG_FAR_I16_32) JMP_PTR16_32(4);
	else if (ARG_SHORT_I8s  || ARG_PNONE_I8s)  JCC_REL(0xeb);
	else if (ARG_NEAR_I16s  || ARG_PNONE_I16s) JMP_REL32(2);
	else if (ARG_NEAR_I32s  || ARG_PNONE_I32s) JMP_REL32(4);
	else if (ARG_NEAR_RM16s || ARG_PNONE_RM16s) INS_FF(0x04, 2);
	else if (ARG_NEAR_RM32s || ARG_PNONE_RM32s) INS_FF(0x04, 4);
	else if (ARG_FAR_M16_16) INS_FF(0x05, 2);
	else if (ARG_FAR_M16_32) INS_FF(0x05, 4);
	else _se_;
	_ce;
}
/* abstract extended instructions */
static void SLDT()
{
	t_nubit8 rid = 0x00;
	_cb("SLDT");
	if (ARG_RM16) INS_0F_00(rid, 2);
	else if (ARG_R32) INS_0F_00(rid, 4);
	else _se_;
	_ce;
}
static void STR()
{
	t_nubit8 rid = 0x01;
	_cb("STR");
	if (ARG_RM16) INS_0F_00(rid, 2);
	else if (ARG_R32) INS_0F_00(rid, 4);
	else _se_;
	_ce;
}
static void LLDT()
{
	t_nubit8 rid = 0x02;
	_cb("LLDT");
	if (ARG_RM16) INS_0F_00(rid, 0);
	else _se_;
	_ce;
}
static void LTR()
{
	t_nubit8 rid = 0x03;
	_cb("LTR");
	if (ARG_RM16) INS_0F_00(rid, 0);
	else _se_;
	_ce;
}
static void VERR()
{
	t_nubit8 rid = 0x04;
	_cb("VERR");
	if (ARG_RM16) INS_0F_00(rid, 0);
	else _se_;
	_ce;
}
static void VERW()
{
	t_nubit8 rid = 0x05;
	_cb("VERW");
	if (ARG_RM16) INS_0F_00(rid, 0);
	else _se_;
	_ce;
}
static void SGDT()
{
	t_nubit8 rid = 0x00;
	_cb("SGDT");
	if (ARG_M16s) INS_0F_01(rid, 2);
	else if (ARG_M32s) INS_0F_01(rid, 4);
	else _se_;
	_ce;
}
static void SIDT()
{
	t_nubit8 rid = 0x01;
	_cb("SIDT");
	if (ARG_M16s) INS_0F_01(rid, 2);
	else if (ARG_M32s) INS_0F_01(rid, 4);
	else _se_;
	_ce;
}
static void LGDT()
{
	t_nubit8 rid = 0x02;
	_cb("SIDT");
	if (ARG_M16s) INS_0F_01(rid, 2);
	else if (ARG_M32) INS_0F_01(rid, 4);
	else _se_;
	_ce;
}
static void LIDT()
{
	t_nubit8 rid = 0x03;
	_cb("LIDT");
	if (ARG_M16s) INS_0F_01(rid, 2);
	else if (ARG_M32) INS_0F_01(rid, 4);
	else _se_;
	_ce;
}
static void SMSW()
{
	t_nubit8 rid = 0x04;
	_cb("SMSW");
	if (ARG_RM16) INS_0F_01(rid, 2);
	else if (ARG_R32) INS_0F_01(rid, 4);
	else _se_;
	_ce;
}
static void LMSW()
{
	t_nubit8 rid = 0x06;
	_cb("LMSW");
	if (ARG_RM16) INS_0F_01(rid, 0);
	else _se_;
	_ce;
}
static void LAR()
{
	_cb("LAR");
	if (ARG_R16_RM16) LAR_R32_RM32(2);
	else if (ARG_R32_RM32) LAR_R32_RM32(4);
	else _se_;
	_ce;
}
static void LSL()
{
	_cb("LSL");
	if (ARG_R16_RM16) LSL_R32_RM32(2);
	else if (ARG_R32_RM32) LSL_R32_RM32(4);
	else _se_;
	_ce;
}
static void BT()
{
	t_nubit8 rid = 0x04;
	_cb("BT");
	if (ARG_RM16_R16) BT_RM32_R32(2);
	else if (ARG_RM32_R32) BT_RM32_R32(4);
	else if (ARG_RM16_I8) INS_0F_BA(rid, 2);
	else if (ARG_RM32_I8) INS_0F_BA(rid, 4);
	else _se_;
	_ce;
}
static void SHLD()
{
	_cb("SHLD");
	if (ARG_RM16_R16_I8) SHLD_RM32_R32_I8(2);
	if (ARG_RM16_R16_CL) SHLD_RM32_R32_CL(2);
	else if (ARG_RM32_R32_I8) SHLD_RM32_R32_I8(4);
	else if (ARG_RM32_R32_CL) SHLD_RM32_R32_CL(4);
	else _se_;
	_ce;
}
static void BTS()
{
	t_nubit8 rid = 0x05;
	_cb("BTS");
	if (ARG_RM16_R16) BTS_RM32_R32(2);
	else if (ARG_RM32_R32) BTS_RM32_R32(4);
	else if (ARG_RM16_I8) INS_0F_BA(rid, 2);
	else if (ARG_RM32_I8) INS_0F_BA(rid, 4);
	else _se_;
	_ce;
}
static void SHRD()
{
	_cb("SHRD");
	if (ARG_RM16_R16_I8) SHRD_RM32_R32_I8(2);
	if (ARG_RM16_R16_CL) SHRD_RM32_R32_CL(2);
	else if (ARG_RM32_R32_I8) SHRD_RM32_R32_I8(4);
	else if (ARG_RM32_R32_CL) SHRD_RM32_R32_CL(4);
	else _se_;
	_ce;
}
static void LSS()
{
	_cb("LSS");
	if (ARG_R16_M16) LSS_R32_M16_32(2);
	else if (ARG_R32_M32) LSS_R32_M16_32(4);
	else _se_;
	_ce;
}
static void BTR()
{
	t_nubit8 rid = 0x06;
	_cb("BTR");
	if (ARG_RM16_R16) BTR_RM32_R32(2);
	else if (ARG_RM32_R32) BTR_RM32_R32(4);
	else if (ARG_RM16_I8) INS_0F_BA(rid, 2);
	else if (ARG_RM32_I8) INS_0F_BA(rid, 4);
	else _se_;
	_ce;
}
static void LFS()
{
	_cb("LFS");
	if (ARG_R16_M16) LFS_R32_M16_32(2);
	else if (ARG_R32_M32) LFS_R32_M16_32(4);
	else _se_;
	_ce;
}
static void LGS()
{
	_cb("LGS");
	if (ARG_R16_M16) LGS_R32_M16_32(2);
	else if (ARG_R32_M32) LGS_R32_M16_32(4);
	else _se_;
	_ce;
}
static void MOVZX()
{
	_cb("MOVZX");
	if (ARG_R16_RM8s) MOVZX_R32_RM8(2);
	else if (ARG_R32_RM8s) MOVZX_R32_RM8(4);
	else if (ARG_R32_RM16s) MOVZX_R32_RM16();
	else _se_;
	_ce;
}
static void BTC()
{
	t_nubit8 rid = 0x07;
	_cb("BTC");
	if (ARG_RM16_R16) BTC_RM32_R32(2);
	else if (ARG_RM32_R32) BTC_RM32_R32(4);
	else if (ARG_RM16_I8) INS_0F_BA(rid, 2);
	else if (ARG_RM32_I8) INS_0F_BA(rid, 4);
	else _se_;
	_ce;
}
static void BSF()
{
	_cb("BSF");
	if (ARG_R16_RM16) BSF_R32_RM32(2);
	else if (ARG_R32_RM32) BSF_R32_RM32(4);
	else _se_;
	_ce;
}
static void BSR()
{
	_cb("BSR");
	if (ARG_R16_RM16) BSR_R32_RM32(2);
	else if (ARG_R32_RM32) BSR_R32_RM32(4);
	else _se_;
	_ce;
}
static void MOVSX()
{
	_cb("MOVSX");
	if (ARG_R16_RM8s) MOVSX_R32_RM8(2);
	else if (ARG_R32_RM8s) MOVSX_R32_RM8(4);
	else if (ARG_R32_RM16s) MOVSX_R32_RM16();
	else _se_;
	_ce;
}
/* main routines */
static t_bool is_end(char c)
{
	return (!c || c == '\n' || c == ';');
}
static t_bool is_space(char c)
{
	return (c == ' ' || c == '\t');
}
static t_bool is_prefix()
{
	if (!strcmp(rop, "es:") || !strcmp(rop, "cs:") ||
		!strcmp(rop, "ss:") || !strcmp(rop, "ds:") ||
		!strcmp(rop, "fs:") || !strcmp(rop, "gs:") ||
		!strcmp(rop, "lock:") || !strcmp(rop, "rep:") ||
		!strcmp(rop, "repne:") || !strcmp(rop, "repnz:") ||
		!strcmp(rop, "repe:") || !strcmp(rop, "repz:")) return 1;
	return 0;
}
static void exec()
{
	/* assemble single statement */
	_cb("exec");
	if (!rop || is_end(rop[0])) ;
	else if (!strcmp(rop, "add")) ADD();
	else if (!strcmp(rop,"push")) PUSH();
	else if (!strcmp(rop, "pop")) POP();
	else if (!strcmp(rop, "or" )) OR();
	else if (!strcmp(rop, "adc")) ADC();
	else if (!strcmp(rop, "sbb")) SBB();
	else if (!strcmp(rop, "and")) AND();
	else if (!strcmp(rop, "es:")) PREFIX_ES();
	else if (!strcmp(rop, "daa")) DAA();
	else if (!strcmp(rop, "sub")) SUB();
	else if (!strcmp(rop, "cs:")) PREFIX_CS();
	else if (!strcmp(rop, "das")) DAS();
	else if (!strcmp(rop, "xor")) XOR();
	else if (!strcmp(rop, "ss:")) PREFIX_SS();
	else if (!strcmp(rop, "aaa")) AAA();
	else if (!strcmp(rop, "cmp")) CMP();
	else if (!strcmp(rop, "ds:")) PREFIX_DS();
	else if (!strcmp(rop, "aas")) AAS();
	else if (!strcmp(rop, "inc")) INC();
	else if (!strcmp(rop, "dec")) DEC();
	else if (!strcmp(rop, "pusha")) PUSHA(2);
	else if (!strcmp(rop, "pushad")) PUSHA(4);
	else if (!strcmp(rop, "popa")) POPA(2);
	else if (!strcmp(rop, "popad")) POPA(4);
	else if (!strcmp(rop, "bound")) BOUND();
	else if (!strcmp(rop, "arpl")) ARPL_RM16_R16();
	else if (!strcmp(rop, "fs:")) PREFIX_FS();
	else if (!strcmp(rop, "gs:")) PREFIX_GS();
	else if (!strcmp(rop, "op+:")) PREFIX_OprSize();
	else if (!strcmp(rop, "az+:")) PREFIX_AddrSize();
	else if (!strcmp(rop,"imul")) IMUL();
	else if (!strcmp(rop,"ins"))  INS();
	else if (!strcmp(rop,"insb")) INSB();
	else if (!strcmp(rop,"insw")) INSW(2);
	else if (!strcmp(rop,"insd")) INSW(4);
	else if (!strcmp(rop,"outs"))  OUTS();
	else if (!strcmp(rop,"outsb")) OUTSB();
	else if (!strcmp(rop,"outsw")) OUTSW(2);
	else if (!strcmp(rop,"outsd")) OUTSW(4);
	else if (!strcmp(rop, "jo" )) JCC_REL(0x70);
	else if (!strcmp(rop, "jno")) JCC_REL(0x71);
	else if (!strcmp(rop, "jb" )) JCC_REL(0x72);
	else if (!strcmp(rop, "jc" )) JCC_REL(0x72);
	else if (!strcmp(rop,"jnae")) JCC_REL(0x72);
	else if (!strcmp(rop, "jae")) JCC_REL(0x73);
	else if (!strcmp(rop, "jnb")) JCC_REL(0x73);
	else if (!strcmp(rop, "jnc")) JCC_REL(0x73);
	else if (!strcmp(rop, "je" )) JCC_REL(0x74);
	else if (!strcmp(rop, "jz" )) JCC_REL(0x74);
	else if (!strcmp(rop, "jne")) JCC_REL(0x75);
	else if (!strcmp(rop, "jnz")) JCC_REL(0x75);
	else if (!strcmp(rop, "jbe")) JCC_REL(0x76);
	else if (!strcmp(rop, "jna")) JCC_REL(0x76);
	else if (!strcmp(rop, "ja" )) JCC_REL(0x77);
	else if (!strcmp(rop,"jnbe")) JCC_REL(0x77);
	else if (!strcmp(rop, "js" )) JCC_REL(0x78);
	else if (!strcmp(rop, "jns")) JCC_REL(0x79);
	else if (!strcmp(rop, "jp" )) JCC_REL(0x7a);
	else if (!strcmp(rop, "jpe")) JCC_REL(0x7a);
	else if (!strcmp(rop, "jnp")) JCC_REL(0x7b);
	else if (!strcmp(rop, "jpo")) JCC_REL(0x7b);
	else if (!strcmp(rop, "jl" )) JCC_REL(0x7c);
	else if (!strcmp(rop,"jnge")) JCC_REL(0x7c);
	else if (!strcmp(rop, "jge")) JCC_REL(0x7d);
	else if (!strcmp(rop, "jnl")) JCC_REL(0x7d);
	else if (!strcmp(rop, "jle")) JCC_REL(0x7e);
	else if (!strcmp(rop, "jng")) JCC_REL(0x7e);
	else if (!strcmp(rop, "jg" )) JCC_REL(0x7f);
	else if (!strcmp(rop,"jnle")) JCC_REL(0x7f);
	else if (!strcmp(rop,"test")) TEST();
	else if (!strcmp(rop,"xchg")) XCHG();
	else if (!strcmp(rop, "mov")) MOV();
	else if (!strcmp(rop, "lea")) LEA();
	else if (!strcmp(rop, "nop")) NOP();
	else if (!strcmp(rop, "cbw"))  CBW(2);
	else if (!strcmp(rop, "cwde")) CBW(4);
	else if (!strcmp(rop, "cwd")) CWD(2);
	else if (!strcmp(rop, "cdq")) CWD(4);
	else if (!strcmp(rop,"call")) CALL();
	else if (!strcmp(rop,"wait")) WAIT();
	else if (!strcmp(rop,"pushf"))  PUSHF(2);
	else if (!strcmp(rop,"pushfd")) PUSHF(4);
	else if (!strcmp(rop,"popf"))  POPF(2);
	else if (!strcmp(rop,"popfd")) POPF(4);
	else if (!strcmp(rop,"sahf")) SAHF();
	else if (!strcmp(rop,"lahf")) LAHF();
	else if (!strcmp(rop,"movs"))  MOVS();
	else if (!strcmp(rop,"movsb")) MOVSB();
	else if (!strcmp(rop,"movsw")) MOVSW(2);
	else if (!strcmp(rop,"movsd")) MOVSW(4);
	else if (!strcmp(rop,"cmps"))  CMPS();
	else if (!strcmp(rop,"cmpsb")) CMPSB();
	else if (!strcmp(rop,"cmpsw")) CMPSW(2);
	else if (!strcmp(rop,"cmpsd")) CMPSW(4);
	else if (!strcmp(rop,"stos"))  STOS();
	else if (!strcmp(rop,"stosb")) STOSB();
	else if (!strcmp(rop,"stosw")) STOSW(2);
	else if (!strcmp(rop,"stosd")) STOSW(4);
	else if (!strcmp(rop,"lods"))  LODS();
	else if (!strcmp(rop,"lodsb")) LODSB();
	else if (!strcmp(rop,"lodsw")) LODSW(2);
	else if (!strcmp(rop,"lodsd")) LODSW(4);
	else if (!strcmp(rop,"scas"))  SCAS();
	else if (!strcmp(rop,"scasb")) SCASB();
	else if (!strcmp(rop,"scasw")) SCASW(2);
	else if (!strcmp(rop,"scasd")) SCASW(4);
	else if (!strcmp(rop, "ret")) RET();
	else if (!strcmp(rop, "les")) LES();
	else if (!strcmp(rop, "lds")) LDS();
	else if (!strcmp(rop, "enter")) ENTER();
	else if (!strcmp(rop, "leave")) LEAVE();
	else if (!strcmp(rop,"retf")) RETF();
	else if (!strcmp(rop, "int")) INT();
	else if (!strcmp(rop,"into")) INTO();
	else if (!strcmp(rop,"iret"))  IRET(2);
	else if (!strcmp(rop,"iretd")) IRET(4);
	else if (!strcmp(rop, "rol")) ROL();
	else if (!strcmp(rop, "ror")) ROR();
	else if (!strcmp(rop, "rcl")) RCL();
	else if (!strcmp(rop, "rcr")) RCR();
	else if (!strcmp(rop, "shl")) SHL();
	else if (!strcmp(rop, "shr")) SHR();
	else if (!strcmp(rop, "sal")) SAL();
	else if (!strcmp(rop, "sar")) SAR();
	else if (!strcmp(rop, "aam")) AAM();
	else if (!strcmp(rop, "aad")) AAD();
	else if (!strcmp(rop,"xlat"))  XLAT();
	else if (!strcmp(rop,"xlatb")) XLATB();
	else if (!strcmp(rop,"loopne")) JCC_REL(0xe0);
	else if (!strcmp(rop,"loopnz")) JCC_REL(0xe0);
	else if (!strcmp(rop,"loope"))  JCC_REL(0xe1);
	else if (!strcmp(rop,"loopz"))  JCC_REL(0xe1);
	else if (!strcmp(rop,"loop"))   JCC_REL(0xe2);
	else if (!strcmp(rop,"jcxz"))   JCC_REL(0xe3);
	else if (!strcmp(rop, "in" )) IN();
	else if (!strcmp(rop, "out")) OUT();
	else if (!strcmp(rop, "jmp")) JMP();
	else if (!strcmp(rop, "lock"))  PREFIX_LOCK();
	else if (!strcmp(rop,"repne:")) PREFIX_REPNZ();
	else if (!strcmp(rop,"repnz:")) PREFIX_REPNZ();
	else if (!strcmp(rop, "rep:")) PREFIX_REPZ();
	else if (!strcmp(rop,"repe:")) PREFIX_REPZ();
	else if (!strcmp(rop,"repz:")) PREFIX_REPZ();
	else if (!strcmp(rop, "hlt")) HLT();
	else if (!strcmp(rop, "cmc")) CMC();
	else if (!strcmp(rop, "not")) NOT();
	else if (!strcmp(rop, "neg")) NEG();
	else if (!strcmp(rop, "mul")) MUL();
	else if (!strcmp(rop, "div")) DIV();
	else if (!strcmp(rop,"idiv")) IDIV();
	else if (!strcmp(rop, "clc")) CLC();
	else if (!strcmp(rop, "stc")) STC();
	else if (!strcmp(rop, "cli")) CLI();
	else if (!strcmp(rop, "sti")) STI();
	else if (!strcmp(rop, "cld")) CLD();
	else if (!strcmp(rop, "std")) STD();
	else if (!strcmp(rop, "sldt"))  SLDT();
	else if (!strcmp(rop, "str"))   STR();
	else if (!strcmp(rop, "lldt"))  LLDT();
	else if (!strcmp(rop, "ltr"))   LTR();
	else if (!strcmp(rop, "verr"))  VERR();
	else if (!strcmp(rop, "verw"))  VERW();
	else if (!strcmp(rop, "sgdt"))  SGDT();
	else if (!strcmp(rop, "sidt"))  SIDT();
	else if (!strcmp(rop, "lgdt"))  LGDT();
	else if (!strcmp(rop, "lidt"))  LIDT();
	else if (!strcmp(rop, "smsw"))  SMSW();
	else if (!strcmp(rop, "lmsw"))  LMSW();
	else if (!strcmp(rop, "lar"))   LAR();
	else if (!strcmp(rop, "lsl"))   LSL();
	else if (!strcmp(rop, "clts"))  CLTS();
	else if (!strcmp(rop, "seto" )) SETCC_RM8(0x90);
	else if (!strcmp(rop, "setno")) SETCC_RM8(0x91);
	else if (!strcmp(rop, "setb" )) SETCC_RM8(0x92);
	else if (!strcmp(rop, "setc" )) SETCC_RM8(0x92);
	else if (!strcmp(rop,"setnae")) SETCC_RM8(0x92);
	else if (!strcmp(rop, "setae")) SETCC_RM8(0x93);
	else if (!strcmp(rop, "setnb")) SETCC_RM8(0x93);
	else if (!strcmp(rop, "setnc")) SETCC_RM8(0x93);
	else if (!strcmp(rop, "sete" )) SETCC_RM8(0x94);
	else if (!strcmp(rop, "setz" )) SETCC_RM8(0x94);
	else if (!strcmp(rop, "setne")) SETCC_RM8(0x95);
	else if (!strcmp(rop, "setnz")) SETCC_RM8(0x95);
	else if (!strcmp(rop, "setbe")) SETCC_RM8(0x96);
	else if (!strcmp(rop, "setna")) SETCC_RM8(0x96);
	else if (!strcmp(rop, "seta" )) SETCC_RM8(0x97);
	else if (!strcmp(rop,"setnbe")) SETCC_RM8(0x97);
	else if (!strcmp(rop, "sets" )) SETCC_RM8(0x98);
	else if (!strcmp(rop, "setns")) SETCC_RM8(0x99);
	else if (!strcmp(rop, "setp" )) SETCC_RM8(0x9a);
	else if (!strcmp(rop, "setpe")) SETCC_RM8(0x9a);
	else if (!strcmp(rop, "setnp")) SETCC_RM8(0x9b);
	else if (!strcmp(rop, "setpo")) SETCC_RM8(0x9b);
	else if (!strcmp(rop, "setl" )) SETCC_RM8(0x9c);
	else if (!strcmp(rop,"setnge")) SETCC_RM8(0x9c);
	else if (!strcmp(rop, "setge")) SETCC_RM8(0x9d);
	else if (!strcmp(rop, "setnl")) SETCC_RM8(0x9d);
	else if (!strcmp(rop, "setle")) SETCC_RM8(0x9e);
	else if (!strcmp(rop, "setng")) SETCC_RM8(0x9e);
	else if (!strcmp(rop, "setg" )) SETCC_RM8(0x9f);
	else if (!strcmp(rop,"setnle")) SETCC_RM8(0x9f);
	else if (!strcmp(rop, "bt"))    BT();
	else if (!strcmp(rop, "shld"))  SHLD();
	else if (!strcmp(rop, "bts"))   BTS();
	else if (!strcmp(rop, "shrd"))  SHRD();
	else if (!strcmp(rop, "lss"))   LSS();
	else if (!strcmp(rop, "btr"))   BTR();
	else if (!strcmp(rop, "lfs"))   LFS();
	else if (!strcmp(rop, "lgs"))   LGS();
	else if (!strcmp(rop, "movzx")) MOVZX();
	else if (!strcmp(rop, "btc"))   BTC();
	else if (!strcmp(rop, "bsf"))   BSF();
	else if (!strcmp(rop, "bsr"))   BSR();
	else if (!strcmp(rop, "movsx")) MOVSX();
	else if (!strcmp(rop, "qdx"))  QDX();
	else _se_;
	_ce;
}
static t_strptr take_arg(t_strptr s)
{
	static t_strptr rstart = NULL;
	t_strptr rend, rresult;
	if (s) rstart = s;
	if (!rstart) return NULL;
	while (!is_end(*rstart) && is_space(*rstart)) rstart++;
	if (*rstart == ',' || is_end(*rstart)) return NULL;
	rresult = rstart;
	while (!is_end(*rstart) && (*rstart) != ',') rstart++;
	rend = rstart - 1;
	if (is_end(*rstart)) rstart = NULL;
	else rstart++;
	while (!is_end(*rend) && is_space(*rend)) rend--;
	*(rend + 1) = 0;
	return rresult;
}
t_nubit8 aasm32(const t_strptr stmt, t_vaddrcc rcode)
{
	t_nubit8 len;
	t_string astmt;
	t_strptr rstmt;
	t_bool flagprefix;

	if (!stmt || is_end(stmt[0])) return 0;

#if AASM_TRACE == 1
	vapiTraceInit(&trace);
#endif

	memcpy(astmt, stmt, MAXLINE);
	lcase(astmt);
	rstmt = astmt;

	prefix_oprsize = prefix_addrsize = 0;
	prefix_lock = prefix_repz = prefix_repnz = 0;
	flagerror = 0;

	iop = 0;
	memset(&aopri1, 0x00, sizeof(t_aasm_oprinfo));
	memset(&aopri2, 0x00, sizeof(t_aasm_oprinfo));
	memset(&aopri3, 0x00, sizeof(t_aasm_oprinfo));
	memset(&aoprig, 0x00, sizeof(t_aasm_oprinfo));
	prefix_oprsizeg = prefix_addrsizeg = 0;

	rop = ropr1 = ropr2 = ropr3 = NULL;

	/* process prefixes */
	do {
		while (!is_end(*rstmt) && is_space(*rstmt)) rstmt++;
		rop = rstmt;
		while (!is_end(*rstmt) && !is_space(*rstmt)) rstmt++;
		if (!is_end(*rstmt)) {
			*rstmt = 0;
			rstmt++;
		}
		flagprefix = is_prefix();
		if (flagprefix) exec();
	} while (flagprefix && !flagerror);

	/* process assembly statement */
	ropr1 = take_arg(rstmt);
	ropr2 = take_arg(NULL);
	ropr3 = take_arg(NULL);

	aopri1 = parsearg(ropr1);
	aopri2 = parsearg(ropr2);
	aopri3 = parsearg(ropr3);

	if (isM(aopri1)) rinfo = &aopri1;
	else if (isM(aopri2)) rinfo = &aopri2;
	else if (isM(aopri3)) rinfo = &aopri3;
	else rinfo = NULL;

	exec();
	len = 0;

	if (!flagerror) {
		if (prefix_repz)  {d_nubit8(rcode + len) = 0xf3;len++;}
		if (prefix_repnz) {d_nubit8(rcode + len) = 0xf2;len++;}
		if (prefix_lock)  {d_nubit8(rcode + len) = 0xf0;len++;}
		if ((rinfo && rinfo->flages) || aoprig.flages) {d_nubit8(rcode + len) = 0x26;len++;}
		if ((rinfo && rinfo->flagcs) || aoprig.flagcs) {d_nubit8(rcode + len) = 0x2e;len++;}
		if ((rinfo && rinfo->flagss) || aoprig.flagss) {d_nubit8(rcode + len) = 0x36;len++;}
		if ((rinfo && rinfo->flagds) || aoprig.flagds) {d_nubit8(rcode + len) = 0x3e;len++;}
		if ((rinfo && rinfo->flagfs) || aoprig.flagfs) {d_nubit8(rcode + len) = 0x64;len++;}
		if ((rinfo && rinfo->flaggs) || aoprig.flaggs) {d_nubit8(rcode + len) = 0x65;len++;}
		if (prefix_addrsize || prefix_addrsizeg) {d_nubit8(rcode + len) = 0x67;len++;}
		if (prefix_oprsize || prefix_oprsizeg) {d_nubit8(rcode + len) = 0x66;len++;}	
		memcpy((void *)(rcode + len), (void *)acode, iop);
		len += iop;
	} else {
#if AASM_TRACE == 1
		vapiPrint("aasm32: bad instruction '%s'\n", stmt);
		vapiPrint("aasm32: [%s] [%s/%d] [%s/%d] [%s/%d]\n",
			rop, ropr1, aopri1.type, ropr2, aopri2.type, ropr3, aopri3.type);
#endif
	}

#if AASM_TRACE == 1
	if (trace.cid) {
		vapiPrint("aasm32: bad instruction '%s'\n", stmt);
		vapiCallBackMachineStop();
	}
	if (trace.flagerror) vapiCallBackMachineStop();
	vapiTraceFinal(&trace);
#endif

	return len;
}

/* extended routines - assemble a paragraph with call/jmp labels */
typedef struct {
	t_string stmt;
	t_nubit32 stmt_id;
	t_nubit8 code_array[15];
	t_nubit8 code_len;
	t_bool flag_is_label;
	t_bool flag_has_label;
	t_string label_str;
	t_string op_str;
	t_aasm_oprptr ptr;
} t_aasm_instr;
/* default operand size */
#define _GetOperandSize (vcpu.cs.seg.exec.defsize ? 4 : 2)
static void asmx_get_label(t_aasm_instr *rinstr)
{
	t_nubit8 i = 0, j = 0;
	rinstr->label_str[0] = 0;
	rinstr->flag_has_label = 0;
	rinstr->flag_is_label = 0;
	while (rinstr->stmt[i] && rinstr->stmt[i] != '$') i++;
	if (rinstr->stmt[i] != '$') return;
	i++;
	if (rinstr->stmt[i] != '(') return;
	i++;
	while (rinstr->stmt[i] && rinstr->stmt[i] != ')')
		rinstr->label_str[j++] = rinstr->stmt[i++];
	rinstr->label_str[j] = 0;
	if (rinstr->stmt[i] != ')') return;
	rinstr->flag_has_label = 1;
	if (rinstr->stmt[0] == '$' && rinstr->stmt[1] == '(' &&
		rinstr->stmt[strlen(rinstr->stmt) - 1] == ':' &&
		rinstr->stmt[strlen(rinstr->stmt) - 2] == ')')
		rinstr->flag_is_label = 1;
}
static void asmx_parse_instr(t_aasm_instr *rinstr)
{
	t_nubit8 i;
	t_strptr rstmt;
	t_aasm_token token;
	i = 0;
	rinstr->code_len = 0;
	while (!is_end(rinstr->stmt[i]) && !is_space(rinstr->stmt[i])) {
		rinstr->op_str[i] = rinstr->stmt[i];
		i++;
	}
	rinstr->op_str[i] = 0;
	rstmt = rinstr->stmt + i;
	if (is_end(*rstmt)) return;
	token = gettoken(rstmt);
	switch (token) {
	case TOKEN_SHORT:
		token = gettoken(NULL);
		if (token == TOKEN_PTR) token = gettoken(NULL);
		if (token != TOKEN_DOLLAR) return;
		rinstr->ptr = PTR_SHORT;
		break;
	case TOKEN_NEAR:
		token = gettoken(NULL);
		if (token == TOKEN_PTR) token = gettoken(NULL);
		if (token != TOKEN_DOLLAR) return;
		rinstr->ptr = PTR_NEAR;
		break;
	case TOKEN_DOLLAR:
		rinstr->ptr = PTR_NONE;
		break;
	default: return;}
	if (!strcmp(rinstr->op_str,"loopne") || !strcmp(rinstr->op_str,"loopnz") || !strcmp(rinstr->op_str,"loope") ||
		!strcmp(rinstr->op_str,"loopz")  || !strcmp(rinstr->op_str,"loop")   || !strcmp(rinstr->op_str,"jcxz")) {
		switch (rinstr->ptr) {
		case PTR_NONE:
			rinstr->ptr = PTR_SHORT;
		case PTR_SHORT:
			rinstr->code_len = 1/*opcode*/ + 1/*rel_imm8*/;
			break;
		default: return;}
	}
	if (!strcmp(rinstr->op_str, "jo")  || !strcmp(rinstr->op_str, "jno") || !strcmp(rinstr->op_str, "jb" ) ||
		!strcmp(rinstr->op_str, "jc" ) || !strcmp(rinstr->op_str,"jnae") || !strcmp(rinstr->op_str, "jae") ||
		!strcmp(rinstr->op_str, "jnb") || !strcmp(rinstr->op_str, "jnc") || !strcmp(rinstr->op_str, "je" ) ||
		!strcmp(rinstr->op_str, "jz" ) || !strcmp(rinstr->op_str, "jne") || !strcmp(rinstr->op_str, "jnz") ||
		!strcmp(rinstr->op_str, "jbe") || !strcmp(rinstr->op_str, "jna") || !strcmp(rinstr->op_str, "ja" ) ||
		!strcmp(rinstr->op_str,"jnbe") || !strcmp(rinstr->op_str, "js" ) || !strcmp(rinstr->op_str, "jns") ||
		!strcmp(rinstr->op_str, "jp" ) || !strcmp(rinstr->op_str, "jpe") || !strcmp(rinstr->op_str, "jnp") ||
		!strcmp(rinstr->op_str, "jpo") || !strcmp(rinstr->op_str, "jl" ) || !strcmp(rinstr->op_str,"jnge") ||
		!strcmp(rinstr->op_str, "jge") || !strcmp(rinstr->op_str, "jnl") || !strcmp(rinstr->op_str, "jle") ||
		!strcmp(rinstr->op_str, "jng") || !strcmp(rinstr->op_str, "jg" ) || !strcmp(rinstr->op_str,"jnle")) {
		switch (rinstr->ptr) {
		case PTR_NONE:
			rinstr->ptr = PTR_SHORT;
		case PTR_SHORT:
			rinstr->code_len = 1/*opcode*/ + 1/*rel_imm8*/;
			break;
		case PTR_NEAR:
			rinstr->code_len = 1/*0x0f*/ + 1/*opcode*/ + _GetOperandSize/*rel_immx*/;
			break;
		default: return;}
	}
	if (!strcmp(rinstr->op_str, "jmp")) {
		switch (rinstr->ptr) {
		case PTR_SHORT:
			rinstr->code_len = 1/*opcode*/ + 1/*rel_imm8*/;
			break;
		case PTR_NONE:
			rinstr->ptr = PTR_NEAR;
		case PTR_NEAR:
			rinstr->code_len = 1/*opcode*/ + _GetOperandSize/*rel_immx*/;
			break;
		default: return;}
	}
	if (!strcmp(rinstr->op_str, "call")) {
		switch (rinstr->ptr) {
		case PTR_NONE:
			rinstr->ptr = PTR_NEAR;
		case PTR_NEAR:
			rinstr->code_len = 1/*opcode*/ + _GetOperandSize/*rel_immx*/;
			break;
		default: return;}
	}
}
t_nubit32 aasm32x(const t_strptr stmt, t_vaddrcc rcode)
{
	t_nsbitcc i, j, k;
	t_nubit32 count;
	t_nubit32 len;
	t_nubit32 offset;
	t_string imm;
	t_aasm_instr *instr;
	count = 1;
	flagerror = 0;
	for (i = 0;i < (t_nsbitcc) strlen(stmt);++i)
		if (stmt[i] == '\n') count++;
	instr = (t_aasm_instr *)malloc(count * sizeof(t_aasm_instr));
	i = j = k = 0;
	while (is_space(stmt[i])) i++;
	while (1) {
		if (is_end(stmt[i])) {
			if (j) {
				j--;
				if (j) {
					while (is_space(instr[k].stmt[j])) j--;
					if (j) {
						instr[k].stmt[j + 1] = 0;
						lcase(instr[k].stmt);
						instr[k].stmt_id = (t_nubit32) k;
						j = 0;
						k++;
					}
				}
			}
			while (stmt[i] && stmt[i] != '\n') i++;
			if (!stmt[i]) break;
			else if (stmt[i] == '\n') {
				i++;
				while (is_space(stmt[i])) i++;
			}
		} else {
			instr[k].stmt[j] = stmt[i];
			i++;
			j++;
		}
	}
	count = (t_nubit32) k;
	for (i = 0;i < count;++i) {
		memset(instr[i].code_array, 0x00, 15);
		asmx_get_label(&instr[i]);
		if (instr[i].flag_has_label) {
			if (instr[i].flag_is_label) {
				instr[i].code_array[0] = 0x90;
				instr[i].code_len = 1;
			} else
				asmx_parse_instr(&instr[i]);
		} else instr[i].code_len = aasm32(instr[i].stmt, (t_vaddrcc)instr[i].code_array);
		if (flagerror) {
			free(instr);
			return 0;
		}
		if (!instr[i].code_len) {
			flagerror = 1;
			vapiPrint("bad instruction in first round:\n#%d: [%s], %x", instr[i].stmt_id, instr[i].stmt, instr[i].code_len);
			if (instr[i].code_len) {
				vapiPrint(", code: [");
				for (j = 0;j < instr[i].code_len;++j)
					vapiPrint("%02X", instr[i].code_array[j]);
				vapiPrint("]");
			}
			if (instr[i].flag_has_label) {
				vapiPrint(", label: [%s], is=%s",
					instr[i].label_str,
					instr[i].flag_is_label ? "yes" : "no");
			}
			vapiPrint("\n");
		}	
		if (flagerror) {
			free(instr);
			return 0;
		}
	}
	/* i: label; j: instr to be materialized; k: size iterator */
	for (i = 0;i < count;++i) {
		if (!instr[i].flag_is_label) continue;
		if (i) {
			for (j = i - 1;j >= 0;--j) {
				if (instr[j].flag_has_label && !strcmp(instr[j].label_str, instr[i].label_str)) {
					if (instr[j].flag_is_label) {
						flagerror = 1;
						vapiPrint("aasm32x: duplicate label '%s'.\n", instr[i].label_str);
					} else {
						offset = 0;
						//for (k = j + 1;k < i;++k) offset += instr[k].code_len;
						for (k = j + 1;k <= i;++k) offset += instr[k].code_len;
						STRCPY(instr[j].stmt, instr[j].op_str);
						switch (instr[j].ptr) {
						case PTR_SHORT:
							STRCAT(instr[j].stmt, " short ");
							if (offset < 0x80) SPRINTF(imm, "+%02x", GetMax8(offset));
							else {flagerror = 1;vapiPrint("aasm32x: invalid short pointer 8+.\n");}
							break;
						case PTR_NEAR:
							STRCAT(instr[j].stmt, " near ");
							switch (_GetOperandSize) {
							case 2:
								if (offset < 0x8000) SPRINTF(imm, "+%04x", GetMax16(offset));
								else {flagerror = 1;vapiPrint("aasm32x: invalid near pointer 16+.\n");}
								break;
							case 4:
								if (offset < 0x80000000) SPRINTF(imm, "+%08X", GetMax32(offset));
								else {flagerror = 1;vapiPrint("aasm32x: invalid near pointer 32+.\n");}
								break;
							default: break;}
							break;
						default: flagerror = 1;break;}
						STRCAT(instr[j].stmt, imm);
						aasm32(instr[j].stmt, (t_vaddrcc)instr[j].code_array);
					}
					if (flagerror) {
						free(instr);
						return 0;
					}
				}
				if (!j) break;
			}
		}
		if (i != count - 1) {
			for (j = i + 1;j < count;++j) {
				if (instr[j].flag_has_label && !strcmp(instr[j].label_str, instr[i].label_str)) {
					if (instr[j].flag_is_label) {
						flagerror = 1;
						vapiPrint("aasm32x: duplicate label '%s'.\n", instr[i].label_str);
					} else {
						offset = 0;
						for (k = i + 1;k < j + 1;++k) offset += instr[k].code_len;
						STRCPY(instr[j].stmt, instr[j].op_str);
						switch (instr[j].ptr) {
						case PTR_SHORT:
							STRCAT(instr[j].stmt, " short ");
							if (offset < 0x80) SPRINTF(imm, "-%02x", GetMax8(offset));
							else {flagerror = 1;vapiPrint("aasm32x: invalid short pointer 8-.\n");}
							break;
						case PTR_NEAR:
							STRCAT(instr[j].stmt, " near ");
							switch (_GetOperandSize) {
							case 2:
								if (offset < 0x8000) SPRINTF(imm, "-%04x", GetMax16(offset));
								else {flagerror = 1;vapiPrint("aasm32x: invalid near pointer 16-.\n");}
								break;
							case 4:
								if (offset < 0x80000000) SPRINTF(imm, "-%08x", GetMax32(offset));
								else {flagerror = 1;vapiPrint("aasm32x: invalid near pointer 32-.\n");}
								break;
							default: break;}
							break;
						default: flagerror = 1;break;}
						STRCAT(instr[j].stmt, imm);
						aasm32(instr[j].stmt, (t_vaddrcc)instr[j].code_array);
					}
					if (flagerror) {
						free(instr);
						return 0;
					}
				}
			}
		}
	}
	len = 0;
	for (i = 0;i < count;++i) {
		/*vapiPrint("%04X: %s", len, instr[i].stmt);
		for (j = GetMax32(strlen(instr[i].stmt));j < 50;++j) vapiPrint(" ");
		vapiPrint("[");
		for (j = 0;j < instr[i].code_len;++j) vapiPrint("%02X", instr[i].code_array[j]);
		vapiPrint("]\n");*/
		memcpy((void *)(rcode + len), instr[i].code_array, instr[i].code_len);
		len += instr[i].code_len;
	}
	free(instr);
	return len;
}
