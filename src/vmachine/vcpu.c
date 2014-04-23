/* This file is a part of NXVM project. */

#include "memory.h"

#include "vcpuins.h"
#include "vcpu.h"

#include "vapi.h"

#if DEBUGMODE != CCPU
t_cpu vcpu;
#endif

void vcpuRefresh()
{
	vcpuinsExecIns();
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