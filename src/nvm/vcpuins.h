/* This file is a part of NekoVMac project. */

#ifndef NVM_VCPUINS_H
#define NVM_VCPUINS_H

#include "vglobal.h"

extern t_faddrcc InTable[0x10000];	// 65536 In Port
extern t_faddrcc OutTable[0x10000];	// 65536 Out Port
extern t_faddrcc InsTable[0x100];	// 256 Instructions

t_bool vcpuinsIsPrefix(t_nubit8 opcode);
void vcpuinsSB();

void CPUInsInit();
void CPUInsTerm();

#endif