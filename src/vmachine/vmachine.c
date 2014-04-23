/* This file is a part of NXVM project. */

#include "memory.h"

#include "vmachine.h"
#include "../system/vapi.h"

#ifdef VMACHINE_DEBUG
#include "qdbios.h"
#endif

t_machine vmachine;

void vmachineRunLoop()
{
	if(vmachine.flaginit && !vmachine.flagrun) {
		vapiRecordStart();
		vmachine.flagrun = 0x01;
		while (vmachine.flagrun) vmachineRefresh();
		vapiRecordEnd();
	}
}
void vmachineRefresh()
{
	vapiRecordWrite();
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

	vapiRecordWrite();
	if (vapirecord.flag) vapirecord.count++;

	vmachine.flagrun = (vmachine.flagrun && (!vcpu.flagterm));
	if (vmachine.flagtrace) vapiTrace();
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
	vvgaInit();
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
