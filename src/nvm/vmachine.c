/* This file is a part of NekoVMac project. */

#include "vmachine.h"
#include "vcpu.h"
#include "vmemory.h"
#include "vbios.h"
#include "vdisplay.h"

t_vaddrcc memoryBase;	// memory base address is 20 bit
t_nubit32 memorySize = 0;	// memory size in byte
t_faddrcc insInTable[0x10000];	// 65536 In Port
t_faddrcc insOutTable[0x10000];	// 65536 Out Port
t_faddrcc ins86Table[0x100];	// 256 8086 Instructions
t_bool testFlag;

void NVMInit()
{
	if(!memorySize)
		memorySize = 1 << 20;	// 1MBytes
	testFlag = 0;
	CPUInit();
	MemoryInit();
	BIOSInit();
	NVMTest();
}

void NVMRun()
{
}

void NVMTerm()
{
	MemoryTerm();
	CPUTerm();
	nvmprint("NVM:\tNeko's Virtual Machine is terminated.\n\n");
}

void NVMError()
{}