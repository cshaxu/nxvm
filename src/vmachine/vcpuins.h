/* This file is a part of NXVM project. */

// CPU Instruction Set: Intel 8086

#ifndef NXVM_VCPUINS_H
#define NXVM_VCPUINS_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#define VCPUINS_DEBUG

#include "vglobal.h"

typedef enum {
	ADD8,ADD16,
	//OR8,OR16,
	ADC8,ADC16,
	SBB8,SBB16,
	/*AND8,AND16,*/
	SUB8,SUB16,
	/*XOR8,XOR16,*/
	CMP8,CMP16
	/*TEST8,TEST16*/
} t_cpuins_type;

typedef enum {
	RT_NONE,RT_REPZ,RT_REPZNZ
} t_cpuins_rep;

typedef struct {
//	t_nubit16 pcs, pip;
	t_faddrcc table[0x100];
	t_vaddrcc rm, r, imm;
	t_nubitcc opr1, opr2, result, bit;
	t_cpuins_type type;
	t_cpuins_rep rep;
} t_cpuins;

extern t_cpuins vcpuins;

void vcpuinsExecIns();
void vcpuinsExecInt();

void vcpuinsInit();
void vcpuinsFinal();

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif