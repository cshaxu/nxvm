/* This file is a part of NekoVMac project. */

#include "stdio.h"
#include "stdlib.h"
#include "memory.h"

#include "vcpuins.h"
#include "vmemory.h"
#include "vcpu.h"

t_cpu vcpu;
t_bool cpuTermFlag;

void InsExec()
{InsTable[*(t_nubit8 *)(memoryBase+(((t_vaddrcc)vcpu.cs)<<4)+vcpu.ip)]();}

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