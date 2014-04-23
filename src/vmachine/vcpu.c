/* This file is a part of NXVM project. */

#include "memory.h"

#include "vcpuins.h"
#include "vcpu.h"

#include "vapi.h"

t_cpu vcpu;

void vcpuRefresh()
{
	vcpuinsExecIns();
	vcpuinsExecInt();
}

void vcpuInit()
{
	memset(&vcpu, 0, sizeof(t_cpu));
	vcpu.cs = 0xf000;
	vcpu.ip = 0xfff0;
	vcpuinsInit();
}

void vcpuFinal()
{
	vcpuinsFinal();
}
