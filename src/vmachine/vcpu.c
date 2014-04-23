/* This file is a part of NXVM project. */

#include "memory.h"

#include "vapi.h"

#include "vcpuins.h"
#include "vcpu.h"

#include "ccpu/ccpuapi.h"

#if DEBUGMODE != CCPU
t_cpu vcpu;
#endif

void vcpuRefresh()
{
#if (DEBUGMODE != VCPU && DEBUGMODE != CCPU)
	ccpuapiSyncRegs();
#endif
#if DEBUGMODE != VCPU
	ccpuapiExecIns();
#endif
#if DEBUGMODE != CCPU
	vcpuinsExecIns();
#endif
#if (DEBUGMODE != VCPU && DEBUGMODE != CCPU)
	if (ccpuapiHasDiff()) vapiCallBackMachineStop();
#endif
	vcpuinsExecInt();
}

void vcpuInit()
{
#if DEBUGMODE != CCPU
	memset(&vcpu, 0, sizeof(t_cpu));
	vcpu.cs = 0xf000;
	vcpu.ip = 0xfff0;
#endif
	vcpuinsInit();
}

void vcpuFinal()
{
	vcpuinsFinal();
}