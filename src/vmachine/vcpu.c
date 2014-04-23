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
	vcpu.cs = 0xf000;
	vcpu.eip = 0x0000fff0;
	vcpu.eflags = 0x00000002;
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
