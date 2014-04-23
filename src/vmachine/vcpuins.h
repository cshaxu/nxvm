/* This file is a part of NXVM project. */

// CPU Instruction Set: Intel 8086

#ifndef NXVM_VCPUINS_H
#define NXVM_VCPUINS_H

#define VCPUINS_DEBUG

#include "vglobal.h"

extern t_faddrcc vcpuinsInPort[0x10000];	// 65536 In Ports
extern t_faddrcc vcpuinsOutPort[0x10000];	// 65536 Out Ports
extern t_faddrcc vcpuinsInsSet[0x100];	// 256 Instructions

t_bool vcpuinsIsPrefix(t_nubit8 opcode);
void vcpuinsClearPrefix();
void vcpuinsExecIns();
void vcpuinsExecInt();

void vcpuinsInit();
void vcpuinsFinal();

#endif