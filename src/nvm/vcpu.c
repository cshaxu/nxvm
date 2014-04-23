/* This file is a part of NekoVMac project. */

#include "memory.h"

#include "vcpuins.h"
#include "vmemory.h"
#include "vcpu.h"

t_cpu vcpu;
t_bool cpuTermFlag;

void vcpuInsExec()
{
	vcpuinsExecIns();
	//RefreshVideoRAM();
	vcpuinsExecINT();
}

void CPUInit()
{
	memset(&vcpu,0,sizeof(vcpu));
	vcpu.cs = 0xf000;
	vcpu.ip = 0xfff0;
	cpuTermFlag = 0;
	CPUInsInit();
}
void CPURun()
{
	while(!cpuTermFlag) vcpuInsExec();
}
void CPUTerm()
{
	CPUInsTerm();
}