/* This file is a part of NXVM project. */

#include "memory.h"

#include "vmachine.h"
#ifdef VMACHINE_DEBUG
#include "../system/vlog.h"
#include "qdbios.h"
#endif

t_machine vmachine;

void vmachineRunLoop()
{
	if(vmachine.flaginit && !vmachine.flagrun) {
		vmachine.flagrun = 0x01;
		while (vmachine.flagrun) vmachineRefresh();
	}
}
void vmachineRefresh()
{
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
	vmachine.flagrun = (vmachine.flagrun && (!vcpu.flagterm));
}
void vmachineInit()
{
	memset(&vmachine, 0x00, sizeof(t_machine));
	vlogInit();
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
	vlogFinal();
	memset(&vmachine, 0x00, sizeof(t_machine));
}
