/* This file is a part of NXVM project. */

#include "memory.h"

#include "vmachine.h"

t_machine vmachine;

void vmachineRunLoop()
{
	if(vmachine.flaginit && !vmachine.flagrun) {
		vmachine.flagrun = 0x01;
		while (vmachine.flagrun) vmachineRefresh();
		vmachineFinal();
	}
}

void vmachineRefresh()
{
/*
	vcmosRefresh();
	vdispRefresh();
	vvadpRefresh();
*/
	vkeybRefresh();
	vkbcRefresh();
	vfddRefresh();
	vfdcRefresh();
	vdmaRefresh();
	vpitRefresh();
	vpicRefresh();
	vramRefresh();
	vcpuRefresh();
	if (vmachine.flagrun) vmachine.flagrun = !vcpu.term;
}
void vmachineInit()
{
	memset(&vmachine, 0x00, sizeof(t_machine));
	vcpuInit();
	vramInit();
	vpicInit();
	vpitInit();
	vdmaInit();
	vfdcInit();
	vfddInit();
	vkbcInit();
	vkeybInit();
/*
	vvgaInit();
	vdispInit();
*/
	vcmosInit();
	vmachine.flaginit = 0x01;
}
void vmachineFinal()
{
	vcmosFinal();
/*
	vkbcFinal();
	vkeybFinal();
	vvadpFinal();
	vdispFinal();
*/
	vfddFinal();
	vfdcFinal();
	vdmaFinal();
	vpitFinal();
	vpicFinal();
	vramFinal();
	vcpuFinal();
	memset(&vmachine, 0x00, sizeof(t_machine));
}
