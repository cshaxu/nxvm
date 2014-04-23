/* This file is a part of NXVM project. */

#include "memory.h"

#include "vmachine.h"
#include "vapi.h"

t_machine vmachine;

void vmachineDumpRecordFile(t_string fname) {vapiRecordDump(fname);}
void vmachineInsertFloppy(t_string fname)  {vapiFloppyInsert(fname);}
void vmachineRemoveFloppy(t_string fname)  {vapiFloppyRemove(fname);}

void vmachineStart() {vmachine.flagrun = 0x01;vapiStartMachine();}
void vmachineStop()  {vmachine.flagrun = 0x00;}

void vapiCallBackMachineRun()
{
	if(vmachine.flaginit) {
		if (vmachine.flagrecord) vapiRecordStart();
		while (vmachine.flagrun) {
			if (vmachine.flagbreak &&
				vcpu.cs == vmachine.breakcs && vcpu.ip == vmachine.breakip) {
				vmachineStop();
				break;
			}
			if (vmachine.flagrecord) vapiRecordExec();
			vmachineRefresh();
			if (vmachine.flagtrace) vmachineStop();
		}
		if (vmachine.flagrecord) vapiRecordEnd();
	} else {
		vmachineStop();
		vapiPrint("NXVM is not initialized.\n");
	}
}
t_nubit8 vapiCallBackMachineGetFlagRun() {return vmachine.flagrun;}
void vapiCallBackMachineStop() {vmachineStop();}

void vmachineRefresh()
{
/*
	vcmosRefresh();
	vdispRefresh();
	vvadpRefresh();
	vkeybRefresh();
	vkbcRefresh();
	vpitRefresh();
*/
	qdbiosRefresh();
	vfddRefresh();
	vfdcRefresh();
	vdmaRefresh();
	vpicRefresh();
	vcpuRefresh();
	vramRefresh();
	vportRefresh();
}
void vmachineInit()
{
	memset(&vmachine, 0x00, sizeof(t_machine));
	vportInit();
	vramInit();
	vcpuInit();
	vpicInit();
	vdmaInit();
	vfdcInit();
	vfddInit();
#ifdef VMACHINE_DEBUG
	qdbiosInit();
#endif
// temp
/*
	vpitInit();
	vkbcInit();
	vkeybInit();
	vvadpInit();
	vdispInit();
	vcmosInit();
*/
	vmachine.flagrecord = 0x01;
	vmachine.flaginit   = 0x01;
}
void vmachineFinal()
{
/*
	vcmosFinal();
	vkbcFinal();
	vkeybFinal();
	vvadpFinal();
	vdispFinal();
	vpitFinal();
*/
#ifdef VMACHINE_DEBUG
	qdbiosFinal();
#endif
	vfddFinal();
	vfdcFinal();
	vdmaFinal();
	vpicFinal();
	vcpuFinal();
	vramFinal();
	vportFinal();
	memset(&vmachine, 0x00, sizeof(t_machine));
}
