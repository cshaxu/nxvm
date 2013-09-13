/* This file is a part of NXVM project. */

// Instruction Set of Virtual CPU

#ifndef NXVM_VCPUINS_H
#define NXVM_VCPUINS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vcpu.h"

typedef enum {
	ARITHTYPE_NULL,
	ADD8,ADD16,ADD32,
	OR8,OR16,OR32,
	ADC8,ADC16,ADC32,
	SBB8,SBB16,SBB32,
	AND8,AND16,AND32,
	SUB8,SUB16,SUB32,
	XOR8,XOR16,XOR32,
	CMP8,CMP16,CMP32,
	TEST8,TEST16,TEST32
} t_cpuins_arithtype;
typedef enum {
	PREFIX_REP_NONE,
	PREFIX_REP_REPZ,
	PREFIX_REP_REPZNZ
} t_cpuins_prefix_rep;
typedef enum {
	PREFIX_SREG_NONE,
	PREFIX_SREG_CS, PREFIX_SREG_SS,
	PREFIX_SREG_DS, PREFIX_SREG_ES,
	PREFIX_SREG_FS, PREFIX_SREG_GS
} t_cpuins_prefix_sreg;
typedef t_bool t_cpuins_prefix;
typedef struct {
	t_cpu_sreg *rsreg;
	t_nubit32 offset;
} t_cpuins_logical;
typedef struct {
	/* prefixes */
	t_cpuins_prefix_rep  prefix_rep;
	t_cpuins_prefix      prefix_oprsize;
	t_cpuins_prefix      prefix_addrsize;
	t_cpu_sreg *roverds, *roverss, *rmovsreg;

	/* execution control */
	t_cpu oldcpu;
	t_bool flaginsloop;
	t_bool flagmaskint; /* if int is disabled once */

	/* memory management */
	t_cpuins_logical mrm;
	t_vaddrcc rrm, rr;
	t_nubit64 crm, cr, cimm;
	t_bool flagmem; /* if rm is in memory */
	t_bool flaglock;

	/* arithmetic operands */
	t_nubit64 opr1, opr2, result;
	t_nubit32 bit;
	t_cpuins_arithtype type;
	t_nubit32 udf; /* undefined eflags bits */

	/* exception handler */
	t_nubit32 except, excode;

	/* debugger */
	t_bool flagwr, flagww, flagwe;
	t_nubit32 wrlin, wwlin, welin;
} t_cpuins;

#define VCPUINS_EXCEPT_DE  0x00000001 /* 00 - fault: divide error */
#define VCPUINS_EXCEPT_DB  0x00000002 /* 01 - trap/fault: debug exception */
#define VCPUINS_EXCEPT_NMI 0x00000004 /* 02 - n/a:   non-maskable interrupt */
#define VCPUINS_EXCEPT_BP  0x00000008 /* 03 - trap:  break point */
#define VCPUINS_EXCEPT_OF  0x00000010 /* 04 - trap:  overflow exception */
#define VCPUINS_EXCEPT_BR  0x00000020 /* 05 - fault: boundary check fail */
#define VCPUINS_EXCEPT_UD  0x00000040 /* 06 - fault: invalid opcode */
#define VCPUINS_EXCEPT_NM  0x00000080 /* 07 - fault: coprocessor not available */
#define VCPUINS_EXCEPT_DF  0x00000100 /* 08 - abort: double fault */
#define VCPUINS_EXCEPT_09  0x00000200 /* 09 - abort: reserved */
#define VCPUINS_EXCEPT_TS  0x00000400 /* 10 - fault: task state segment fail */
#define VCPUINS_EXCEPT_NP  0x00000800 /* 11 - fault: segment not present */
#define VCPUINS_EXCEPT_SS  0x00001000 /* 12 - fault: stack segment fault */
#define VCPUINS_EXCEPT_GP  0x00002000 /* 13 - fault: general protection */
#define VCPUINS_EXCEPT_PF  0x00004000 /* 14 - fault: page fault */
#define VCPUINS_EXCEPT_15  0x00008000 /* 15 - n/a:   reserved */
#define VCPUINS_EXCEPT_MF  0x00010000 /* 16 - fault: x87 fpu floating point error */

#define VCPUINS_EXCEPT_CE  0x80000000 /* 31 - internal case error */

extern t_cpuins vcpuins;

t_bool vcpuinsLoadSreg(t_cpu_sreg *rsreg, t_nubit16 selector);
t_bool vcpuinsReadLinear(t_nubit32 linear, t_vaddrcc rdata, t_nubit8 byte);
t_bool vcpuinsWriteLinear(t_nubit32 linear, t_vaddrcc rdata, t_nubit8 byte);

void vcpuinsInit();
void vcpuinsReset();
void vcpuinsRefresh();
void vcpuinsFinal();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
