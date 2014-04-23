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
	vcpu.ip = 0xfff0;
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