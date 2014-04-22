/* This file is a part of NekoVMac project. */

#include "stdio.h"
#include "stdlib.h"
#include "memory.h"

#include "vcpu.h"

t_cpu vCPU;
//t_vaddrcc curIP;

/*static void IO_NOP()
{
#ifdef NVM_DEBUG
	nvmprint("IO not supported - %4x:%4x %4x%3x\n",vCPU.cs,vCPU.ip,*(t_nubit8 *)(curIP+memoryBase-1),*(t_nubit8 *)(curIP+memoryBase));
#endif
}*/

void CPUInit()
{
	memset(&vCPU,0,sizeof(vCPU));
	vCPU.cs = 0xf000;
	vCPU.ip = 0xfff0;
	//curIP = (vCPU.cs << 4) + vCPU.ip;
	//SetIns86Table();
	//SetInOutTable();
}

void CPUTerm()
{}