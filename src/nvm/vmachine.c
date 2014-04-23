/* This file is a part of NVMx86 project. */

#include "system/vapi.h"
#include "vmachine.h"

t_bool initFlag = 0;
t_bool runFlag = 0;

void NVMInit()
{
	if(!initFlag) {
		CPUInit();
		MemoryInit();
		initFlag = 1;
	} else
		nvmprint("ERROR:\tNeko's Virtual Machine is already initialized.\n");
}

void NVMPowerOn()
{
	if(!runFlag) {
		nvmprint("NVM:\tNeko's Virtual Machine is powered on.\n");
		runFlag = 1;
		forceNone = 0;
		if(!initFlag) NVMInit();
		// Create Screen Here
		NVMRun();
	} else
		nvmprint("NVM:\tNeko's Virtual Machine is already running.\n");
}

void NVMRun()
{
	if(!initFlag || !runFlag)
		nvmprint("ERROR:\tNeko's Virtual Machine is not running.\n");
	else {
	}
}

void NVMPowerOff()
{
	if(runFlag) {
		// Delete Screen Here
		if(initFlag) NVMTerm();
		forceNone = 1;
		runFlag = 0;
		nvmprint("NVM:\tNeko's Virtual Machine is terminated.\n");
	} else
		nvmprint("NVM:\tNeko's Virtual Machine is already powered off.\n");
}

void NVMTerm()
{
	if(initFlag) {
		MemoryTerm();
		CPUTerm();
		initFlag = 0;
	} else
		nvmprint("ERROR:\tNeko's Virtual Machine is not initialized.\n");
}