/* This file is a part of NXVM project. */

// CPU Instruction Set: Intel 8086

#ifndef NXVM_VCPUINS_H
#define NXVM_VCPUINS_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#include "vglobal.h"

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
	t_vaddrcc rm, r, imm; /* marked for deletion */
	
	t_cpu_sreg oldcs, oldss;
	t_nubit32 oldeip, oldesp;
	t_bool    flaginsloop;
	t_faddrcc table[0x100], table_0f[0x100];
	t_nubitcc opr1, opr2, result, bit;
	t_cpuins_type type;
	t_cpuins_prefix_rep  prefix_rep;
	t_cpuins_prefix_sreg prefix_sreg;
	t_cpuins_prefix      prefix_lock;
	t_cpuins_prefix      prefix_oprsize;
	t_cpuins_prefix      prefix_addrsize;
	t_nubit32 except, excode;
	t_vaddrcc prm, pr, pimm;
	t_bool    flagmem; /* if prm is in memory */
	t_nubit32 lrm; /* linear address if prm is in memory*/
} t_cpuins;

#define VCPUINS_EXCEPT_GP 0x00000001 /* general protection */
#define VCPUINS_EXCEPT_NP 0x00000002 /* segment not present */
#define VCPUINS_EXCEPT_PF 0x00000004 /* page fault */
#define VCPUINS_EXCEPT_SS 0x00000008 /* stack segment fault */
#define VCPUINS_EXCEPT_UD 0x00000010 /* undefined opcode */
#define VCPUINS_EXCEPT_BR 0x00000020 /* boundary check fail */

extern t_cpuins vcpuins;

void vcpuinsInit();
void vcpuinsReset();
void vcpuinsRefresh();
void vcpuinsFinal();

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
