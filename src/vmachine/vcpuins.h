/* This file is a part of NXVM project. */

// Instruction Set of Virtual CPU

#ifndef NXVM_VCPUINS_H
#define NXVM_VCPUINS_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#include "vcpu.h"

typedef enum {
	ADD8,ADD16,ADD32,
	//OR8,OR16,OR32,
	ADC8,ADC16,ADC32,
	SBB8,SBB16,SBB32,
	//AND8,AND16,AND32,
	SUB8,SUB16,SUB32,
	//XOR8,XOR16,XOR32,
	CMP8,CMP16,CMP32
	//TEST8,TEST16,TEST32
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
	t_nubit64 cdesc, cimm, crm, cr;
	t_vaddrcc rdesc, rimm, rrm, rr;
	t_nubit32 pdesc, pimm, prm;
	t_nubit32 ldesc, limm, lrm;
	t_nubit32 eimm, erm;
	t_bool    flagmem; /* if rm is in memory */
	t_bool    flagmss; /* if rm is in stack segment */
	t_bool    flagmaskint; /* if int is disabled once */
	t_bool    flagrespondint; /* if intr is responded in one Refresh */
	t_nubit32 udf; /* undefined eflags bits */
} t_cpuins;

#define VCPUINS_EXCEPT_GP 0x00000001 /* general protection */
#define VCPUINS_EXCEPT_NP 0x00000002 /* segment not present */
#define VCPUINS_EXCEPT_PF 0x00000004 /* page fault */
#define VCPUINS_EXCEPT_SS 0x00000008 /* stack segment fault */
#define VCPUINS_EXCEPT_UD 0x00000010 /* undefined opcode */
#define VCPUINS_EXCEPT_BR 0x00000020 /* boundary check fail */
#define VCPUINS_EXCEPT_TS 0x00000040 /* task state segment fail */
#define VCPUINS_EXCEPT_DE 0x00000080 /* divide error */
#define VCPUINS_EXCEPT_NM 0x00000100 /* numerical error */
#define VCPUINS_EXCEPT_CE 0x80000000 /* case error */

extern t_cpuins vcpuins;

void vcpuinsInit();
void vcpuinsReset();
void vcpuinsRefresh();
void vcpuinsFinal();

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
