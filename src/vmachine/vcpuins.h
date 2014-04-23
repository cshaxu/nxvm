/* This file is a part of NXVM project. */

// CPU Instruction Set: Intel 8086

#ifndef NXVM_VCPUINS_H
#define NXVM_VCPUINS_H

#include "vglobal.h"

extern t_faddrcc InTable[0x10000];	// 65536 In Ports
extern t_faddrcc OutTable[0x10000];	// 65536 Out Ports
extern t_faddrcc InsTable[0x100];	// 256 Instructions

t_bool vcpuinsIsPrefix(t_nubit8 opcode);
void vcpuinsClearPrefix();

void vcpuinsExecIns();
void vcpuinsExecINT();

void CPUInsInit();
void CPUInsTerm();

#endif