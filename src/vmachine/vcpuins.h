/* This file is a part of NXVM project. */

// Instruction Set of Virtual CPU

#ifndef NXVM_VCPUINS_H
#define NXVM_VCPUINS_H

#ifdef __cplusplus
/*extern "C" {*/
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
} t_cpuins_type;
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
	t_bool flag2;
	t_nubit32 ph1, ph2;
	t_nubit32 pl1, pl2;
} t_cpuins_physical;
typedef struct {
	t_nubit16 sel;
	t_nubit64 desc;
	t_cpuins_logical logical;
} t_cpuins_selector;
typedef struct {
	t_cpu_sreg oldcs, oldss;
	t_nubit32 oldeip, oldesp;
	t_bool    flaginsloop;
	t_nubit32 opr1, opr2, result, bit;
	t_cpuins_type type;
	t_cpuins_prefix_rep  prefix_rep;
	t_cpuins_prefix      prefix_lock;
	t_cpuins_prefix      prefix_oprsize;
	t_cpuins_prefix      prefix_addrsize;
	t_cpu_sreg *roverds, *roverss, *rmovsreg;
	t_nubit32 except, excode;
	t_faddrcc table[0x100], table_0f[0x100];
	t_cpuins_selector descsel;
	t_cpuins_logical rmlog;
	t_nubit64 cimm, crm, cr;
	t_vaddrcc rimm, rrm, rr;
	t_nubit32 pimm, prm;
	t_nubit32 limm, lrm;
	t_nubit32 eimm, erm;
	t_bool    flagmem; /* if rm is in memory */

	t_bool    flagmss; /* if rm is in stack segment */
	t_bool    flagmaskint; /* if int is disabled once */
	t_bool    flagrespondint; /* if intr is responded in one Refresh */
	t_nubit8  isrs; /* if inside int serv rout */
	t_nubit32 udf; /* undefined eflags bits */
} t_cpuins;

#define VCPUINS_EXCEPT_DE 0x00000001 /* 00 - divide error */
#define VCPUINS_EXCEPT_BR 0x00000020 /* 05 - boundary check fail */
#define VCPUINS_EXCEPT_UD 0x00000040 /* 06 - invalid opcode */
#define VCPUINS_EXCEPT_NM 0x00000080 /* 07 - coprocessor not available */
#define VCPUINS_EXCEPT_DF 0x00000100 /* 08 - double fault */
#define VCPUINS_EXCEPT_TS 0x00000400 /* 10 - task state segment fail */
#define VCPUINS_EXCEPT_NP 0x00000800 /* 11 - segment not present */
#define VCPUINS_EXCEPT_SS 0x00001000 /* 12 - stack segment fault */
#define VCPUINS_EXCEPT_GP 0x00002000 /* 13 - general protection */
#define VCPUINS_EXCEPT_PF 0x00004000 /* 14 - page fault */

#define VCPUINS_EXCEPT_CE 0x80000000 /* 31 - internal case error */

extern t_cpuins vcpuins;

void vcpuinsInit();
void vcpuinsReset();
void vcpuinsRefresh();
void vcpuinsFinal();

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
