/* This file is a part of NXVM project. */

#include "memory.h"

#include "vapi.h"

#include "vcpuins.h"
#include "vcpu.h"

t_cpu vcpu;

void vcpuInit()
{
	vcpuinsInit();
}
void vcpuReset()
{
	memset(&vcpu, 0, sizeof(t_cpu));
	vcpu.cs.selector = 0xf000;
	vcpu.ip = 0xfff0;
	vcpuinsReset();
}
void vcpuRefresh()
{
	vcpuinsRefresh();
}

void vcpuFinal()
{
	vcpuinsFinal();
}
