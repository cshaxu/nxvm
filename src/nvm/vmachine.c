/* This file is a part of NekoVMac project. */

#include "system/vapi.h"
#include "vmachine.h"
#include "vcpu.h"
#include "vmemory.h"
#include "vbios.h"

t_vaddrcc memoryBase;	// memory base address is 20 bit
t_nubit32 memorySize = 0;	// memory size in byte
t_faddrcc InTable[0x10000];	// 65536 In Port
t_faddrcc OutTable[0x10000];	// 65536 Out Port
t_faddrcc InsTable[0x100];	// 256 8086 Instructions

t_bool initFlag = 0;
t_bool runFlag = 0;

void NVMInit()
{
	if(!initFlag) {
		if(!memorySize)
			memorySize = 1 << 20;	// 1MBytes
		CPUInit();
		MemoryInit();
		BIOSInit();
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