/* This file is a part of NXVM project. */

#include "memory.h"

#include "vapi.h"

#include "vcpuins.h"
#include "vcpu.h"

#ifndef ECPUACT
t_cpu vcpu;
#endif

void vcpuRefresh()
{
#ifdef ECPUACT
	ecpuRefresh();
#else
//	ecpuapiSyncRegs();
//	ecpuinsExecIns();
	vcpuinsExecIns();
//	if (ecpuapiHasDiff()) {
//		vapiCallBackMachineStop();
//		return;
//	}
	vcpuinsExecInt();
#endif
}

void vcpuInit()
{
#ifndef ECPUACT
	memset(&vcpu, 0, sizeof(t_cpu));
	vcpu.cs = 0xf000;
	vcpu.ip = 0xfff0;
	vcpuinsInit();
#else
	ecpuInit();
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