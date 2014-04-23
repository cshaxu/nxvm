/* This file is a part of NXVM project. */

#include "memory.h"

#include "vapi.h"

#include "vcpu.h"

void vcpuinsExecIns() {ExecIns();}
void vcpuinsExecInt() {ExecInt();}

void vcpuRefresh()
{
	vcpuinsExecIns();
	vcpuinsExecInt();
}

void vcpuInit()
{
	CPUInit();
}

void vcpuFinal()
{
	CPUTerminate();
}