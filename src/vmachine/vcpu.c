/* This file is a part of NXVM project. */

#include "memory.h"

#include "vapi.h"

#include "vcpuins.h"
#include "vcpu.h"

#ifndef ECPUACT
t_cpu vcpu;
#endif

void vcpuInit()
{
#ifndef ECPUACT
	vcpuinsInit();
#else
	ecpuInit();
#endif
}
void vcpuReset()
{
	memset(&vcpu, 0, sizeof(t_cpu));
	vcpu.eip = 0x0000fff0;
	vcpu.eflags = 0x00000002;
	vcpu.cs.selector = 0xf000;
	vcpu.cs.base = 0xffff0000;
	vcpu.cs.limit = 0xffffffff;
	vcpu.cs.cd = 0x01;
	vcpu.cs.rw = 0x01;
	vcpu.cs.s = 0x01;
	vcpu.cs.p = 0x01;
	vcpu.ss.base = 0x00000000;
	vcpu.ss.limit = 0x0000ffff;
	vcpu.ss.rw = 0x01;
	vcpu.ss.s = 0x01;
	vcpu.ss.p = 0x01;
	vcpu.ds = vcpu.es = vcpu.fs = vcpu.gs = vcpu.ss;
	_LoadIDTR16(0x000000, 0x03ff);
	vcpuinsReset();
}
void vcpuRefresh()
{
#ifdef ECPUACT
	ecpuRefresh();
#else
//	ecpuapiSyncRegs();
//	ecpuinsRefresh();
	vcpuinsRefresh();
//	if (ecpuapiHasDiff()) {
//		vapiCallBackMachineStop();
//		return;
//	}
#endif
}

void vcpuFinal()
{
#ifndef ECPUACT
	vcpuinsFinal();
#else
	ecpuFinal();
#endif
}
