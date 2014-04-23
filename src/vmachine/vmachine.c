/* This file is a part of NXVM project. */

#include "system/vapi.h"
#include "vmachine.h"

t_bool initFlag = 0;
t_bool runFlag = 0;

void NXVMInit()
{
	if(!initFlag) {
		CPUInit();
		MemoryInit();
		PICInit();
		initFlag = 1;
	} else
		nvmprint("ERROR:\tNeko's x86 Virtual Machine is already initialized.\n");
}

void NXVMPowerOn()
{
	if(!runFlag) {
		nvmprint("NXVM:\tNeko's x86 Virtual Machine is powered on.\n");
		runFlag = 1;
		forceNone = 0;
		if(!initFlag) NXVMInit();
		// Create Screen Here
		NXVMRun();
	} else
		nvmprint("NXVM:\tNeko's x86 Virtual Machine is already running.\n");
}

void NXVMRun()
{
	if(!initFlag || !runFlag)
		nvmprint("ERROR:\tNeko's x86 Virtual Machine is not running.\n");
	else {
		CPURun();
	}
}

void NXVMPowerOff()
{
	if(runFlag) {
		// Delete Screen Here
		if(initFlag) NXVMTerm();
		forceNone = 1;
		runFlag = 0;
		nvmprint("NXVM:\tNeko's x86 Virtual Machine is terminated.\n");
	} else
		nvmprint("NXVM:\tNeko's x86 Virtual Machine is already powered off.\n");
}

void NXVMTerm()
{
	if(initFlag) {
		PICTerm();
		MemoryTerm();
		CPUTerm();
		initFlag = 0;
	} else
		nvmprint("ERROR:\tNeko's x86 Virtual Machine is not initialized.\n");
}