/* This file is a part of NXVM project. */

#include "system/vapi.h"
#include "vmachine.h"

t_bool vmachineinitflag = 0;
t_bool vmachinerunflag = 0;

void NXVMInit()
{
	if(!vmachineinitflag) {
		CPUInit();
		RAMInit();
		PICInit();
		CMOSInit();
		PITInit();
		FDDInit();
		FDCInit();
		DMACInit();
		vmachineinitflag = 1;
	} else
		vapiPrint("ERROR:\tNeko's x86 Virtual Machine is already initialized.\n");
}
void NXVMPowerOn()
{
	if(!vmachinerunflag) {
		vapiPrint("NXVM:\tNeko's x86 Virtual Machine is powered on.\n");
		vmachinerunflag = 1;
		forceNone = 0;
		if(!vmachineinitflag) NXVMInit();
		// Create Screen Here
		NXVMRun();
		if(vcputermflag) vmachinerunflag = 0;
	} else
		vapiPrint("NXVM:\tNeko's x86 Virtual Machine is already running.\n");
}
void NXVMRun()
{
	if(!vmachineinitflag || !vmachinerunflag)
		vapiPrint("ERROR:\tNeko's x86 Virtual Machine is not running.\n");
	else {
		CPURun();
	}
}
void NXVMPowerOff()
{
	if(vmachinerunflag) {
		if(vmachineinitflag) NXVMTerm();
		forceNone = 1;
		vmachinerunflag = 0;
		vapiPrint("NXVM:\tNeko's x86 Virtual Machine is terminated.\n");
	} else
		vapiPrint("NXVM:\tNeko's x86 Virtual Machine is already powered off.\n");
}
void NXVMTerm()
{
	if(vmachineinitflag) {
		DMACTerm();
		FDCTerm();
		FDDTerm();
		PITTerm();
		CMOSTerm();
		PICTerm();
		RAMTerm();
		CPUTerm();
		vmachineinitflag = 0;
	} else
		vapiPrint("ERROR:\tNeko's x86 Virtual Machine is not initialized.\n");
}