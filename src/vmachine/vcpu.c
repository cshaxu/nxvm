/* This file is a part of NXVM project. */

#include "memory.h"

#include "vcpuins.h"
#include "vcpu.h"

#ifdef VCPU_DEBUG
#include "vram.h"
#include "../system/vlog.h"
#endif

t_cpu vcpu;

void vcpuRefresh()
{
#ifdef VCPU_DEBUG
	vlogExec();
#endif
	vcpuinsExecIns();
#ifdef VCPU_DEBUG
	vlogExec();
	vlog.line++;
#endif
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
