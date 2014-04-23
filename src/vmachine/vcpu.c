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
	t_vaddrcc pal = (t_vaddrcc)(&vcpu.al);
	t_vaddrcc pah = (t_vaddrcc)(&vcpu.ah);
	t_vaddrcc pbl = (t_vaddrcc)(&vcpu.bl);
	t_vaddrcc pbh = (t_vaddrcc)(&vcpu.bh);
	t_vaddrcc pcl = (t_vaddrcc)(&vcpu.cl);
	t_vaddrcc pch = (t_vaddrcc)(&vcpu.ch);
	t_vaddrcc pdl = (t_vaddrcc)(&vcpu.dl);
	t_vaddrcc pdh = (t_vaddrcc)(&vcpu.dh);
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
