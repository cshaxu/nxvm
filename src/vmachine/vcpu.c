/* This file is a part of NXVM project. */

#include "memory.h"

#include "vcpuins.h"
#include "vmemory.h"
#include "vcpu.h"

//#include "vpic.h"

t_cpu vcpu;
t_bool cpuTermFlag;

void vcpuInsExec()
{
	vcpuinsExecIns();
	vcpuinsExecINT();
	//RefreshVideoRAM();
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
	//8259A Test
	//masterpic.irr = 0x40;
	//slavepic.irr = 0x10;
	//vcpu.flags |= IF;
	while(!cpuTermFlag) vcpuInsExec();
}
void CPUTerm()
{
	CPUInsTerm();
}