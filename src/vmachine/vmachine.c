/* This file is a part of NXVM project. */

#include "system/vapi.h"
#include "vmachine.h"

t_bool vmachineinitflag = 0;
t_bool vmachinerunflag = 0;

void vmachineRefresh()
{
/*
	vcmosRefresh();
	vdispRefresh();
	vvadpRefresh();
	vkeybRefresh();
	vkbcRefresh();
*/
	vfddRefresh();
	vfdcRefresh();
	vdmaRefresh();
	vpitRefresh();
	vpicRefresh();
	vramRefresh();
	vcpuRefresh();
	vmachinerunflag = !vcpu.term;
}

void vmachineInit()
{
	if(!vmachineinitflag) {
		vcpuInit();
		vramInit();
		vpicInit();
		vpitInit();
		vdmaInit();
		vfdcInit();
		vfddInit();
/*
		vkbcInit();
		vkeybInit();
		vvadpInit();
		vdispInit();
*/
		vcmosInit();
		vmachineinitflag = 1;
	} else
		vapiPrint("ERROR:\tNeko's x86 Virtual Machine is already initialized.\n");
}
void vmachinePowerOn()
{
	if(!vmachinerunflag) {
		vapiPrint("NXVM:\tNeko's x86 Virtual Machine is powered on.\n");
		vmachinerunflag = 1;
		forceNone = 0;
		if(!vmachineinitflag) vmachineInit();
		// Create Screen Here
		vmachineRunLoop();
	} else
		vapiPrint("NXVM:\tNeko's x86 Virtual Machine is already running.\n");
}
void vmachineRunLoop()
{
	if(!vmachineinitflag || !vmachinerunflag)
		vapiPrint("ERROR:\tNeko's x86 Virtual Machine is not running.\n");
	else {
		while (vmachinerunflag) vmachineRefresh();
	}
}
void vmachinePowerOff()
{
	if(vmachinerunflag) {
		if(vmachineinitflag) vmachineFinal();
		forceNone = 1;
		vmachinerunflag = 0;
		vapiPrint("NXVM:\tNeko's x86 Virtual Machine is terminated.\n");
	} else
		vapiPrint("NXVM:\tNeko's x86 Virtual Machine is already powered off.\n");
}
void vmachineFinal()
{
	if(vmachineinitflag) {
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
		vmachineinitflag = 0;
	} else
		vapiPrint("ERROR:\tNeko's x86 Virtual Machine is not initialized.\n");
}
