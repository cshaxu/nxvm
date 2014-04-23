/* This file is a part of NXVM project. */

#include "memory.h"

#include "vcpuins.h"
#include "vram.h"
#include "vcpu.h"

//#include "vpic.h"

t_cpu vcpu;
t_bool vcputermflag;

void vcpuInsExec()
{
	vcpuinsExecIns();
	vcpuinsExecInt();
	//RefreshVideoRAM();
}

void CPUInit()
{
	memset(&vcpu,0,sizeof(t_cpu));
	vcpu.cs = 0xf000;
	vcpu.ip = 0xfff0;
	vcputermflag = 0;
	CPUInsInit();
}
void CPURun()
{
	//8259A Test
	//vpicmaster.irr = 0x40;
	//vpicslave.irr = 0x10;
	//vcpu.flags |= VCPU_FLAG_IF;
	vcputermflag = 0;
	while(!vcputermflag) vcpuInsExec();
}
void CPUTerm()
{
	CPUInsTerm();
}