/* This file is a part of NekoVMac project. */

#include "stdio.h"
#include "stdlib.h"
#include "memory.h"

#include "vcpuins.h"
#include "vmemory.h"
#include "vcpu.h"

t_cpu vcpu;
t_bool cpuTermFlag;

void vcpuInsExec()
{
	t_nubit8 opcode = vmemoryGetByte(vcpu.cs,vcpu.ip);
	InsTable[opcode]();
	if(!vcpuinsIsPrefix(opcode)) vcpuinsSB();
}

void CPUInit()
{
	memset(&vcpu,0,sizeof(vcpu));
	vcpu.cs = 0xf000;
	vcpu.ip = 0xfff0;
	cpuTermFlag = 0;
	CPUInsInit();
}

void CPUTerm()
{
	CPUInsTerm();
}