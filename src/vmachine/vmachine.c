/* This file is a part of NXVM project. */

#include "memory.h"

#include "vmachine.h"
#include "vapi.h"

t_machine vmachine;

void vmachineSetRecordFile(t_string fname) {vapiRecordSetFile(fname);}
void vmachineInsertFloppy(t_string fname) {vapiFloppyInsert(fname);}
void vmachineRemoveFloppy(t_string fname) {vapiFloppyRemove(fname);}
void vmachineStart() {vapiStartMachine();}

void vapiCallBackMachineRun()
{
	if(vmachine.flaginit && !vmachine.flagrun) {
		if (vmachine.flagrecord) vapiRecordStart();
		vmachine.flagrun = 0x01;
		while (vmachine.flagrun) {
			vmachineRefresh();
			if (vmachine.flagtrace) vapiTrace();
		}
		if (vmachine.flagrecord) vapiRecordEnd();
	}
}
t_bool vapiCallBackMachineGetRunFlag()
{
	return vmachine.flagrun;
}
void   vapiCallBackMachineSetRunFlag(t_bool flag)
{
	vmachine.flagrun = flag;
}

void vmachineRefresh()
{
	if (vmachine.flagrecord) vapiRecordWrite();
/*
	vcmosRefresh();
	vdispRefresh();
	vvadpRefresh();
	vkeybRefresh();
	vkbcRefresh();
	vfddRefresh();
	vfdcRefresh();
	vdmaRefresh();
	vpitRefresh();
*/
	vpicRefresh();
	vramRefresh();
	vcpuRefresh();
	vportRefresh();

	if (vmachine.flagrecord) {
		vapiRecordWrite();
		++vapirecord.count;
	}

	vmachine.flagrun = (vmachine.flagrun && (!vcpu.flagterm));
}
void vmachineInit()
{
	memset(&vmachine, 0x00, sizeof(t_machine));
	vportInit();
	vcpuInit();
	vramInit();
	vpicInit();
#ifdef VMACHINE_DEBUG
	qdbiosInit();
#endif
/*
	vpitInit();
	vdmaInit();
	vfdcInit();
	vfddInit();
	vkbcInit();
	vkeybInit();
	vvadpInit();
	vdispInit();
	vcmosInit();
*/
	vmachine.flaginit = 0x01;
}
void vmachineFinal()
{
/*
	vcmosFinal();
	vkbcFinal();
	vkeybFinal();
	vvadpFinal();
	vdispFinal();
	vfddFinal();
	vfdcFinal();
	vdmaFinal();
	vpitFinal();
*/
#ifdef VMACHINE_DEBUG
	qdbiosFinal();
#endif
	vpicFinal();
	vramFinal();
	vcpuFinal();
	vportFinal();
	memset(&vmachine, 0x00, sizeof(t_machine));
}
