/* This file is a part of NXVM project. */

#include "vapi.h"
#include "debug/record.h"
#include "debug/debug.h"
#include "vmachine.h"

t_machine vmachine;

static void DoReset()
{
	vportReset();
	vramReset();
	vcpuReset();
	vpicReset();
	vpitReset();
	vcmosReset();
	vdmaReset();
	vfdcReset();
	vfddReset();
	vhddReset();
	vkbcReset();
	//vkeybReset();
	vvadpReset();
	//vdispReset();
	qdbiosReset();
	vmachine.flagreset = 0;
}
void vapiCallBackMachineRun()
{
	while (vmachine.flagrun)
		vmachineRefresh();
}
t_bool vapiCallBackMachineGetFlagRun()
{
	return vmachine.flagrun;
}
void vapiCallBackMachineReset() {vmachineReset();}
void vapiCallBackMachineStop() {vmachineStop();}

void vmachineStart()
{
	DoReset();
	vmachineResume();
}
void vmachineReset()
{
	if (vmachine.flagrun)
		vmachine.flagreset = 1;
	else
		DoReset();
}
void vmachineResume() {
	if (vmachine.flagrun) return;
	vmachine.flagrun = 1;
	vapiStartMachine();
}
void vmachineStop()  {vmachine.flagrun = 0;}
void vmachineRefresh()
{
	if (vmachine.flagreset) DoReset();
	//vdispRefresh();
	vvadpRefresh();
	//vkeybRefresh();
	vkbcRefresh();
	vhddRefresh();
	vfddRefresh();
	vfdcRefresh();
	vdmaRefresh();
	vpitRefresh();
	vpicRefresh();
	vcmosRefresh();
	vcpuRefresh();
	vramRefresh();
	vportRefresh();
	debugRefresh();
	recordRefresh();
}
void vmachineInit()
{
	memset(&vmachine, 0x00, sizeof(t_machine));
	recordInit();
	debugInit();
	vportInit();
	vramInit();
	vcpuInit();
	vpicInit();
	vpitInit();
	vcmosInit();
	vdmaInit();
	vfdcInit();
	vfddInit();
	vhddInit();
	vkbcInit();
	//vkeybInit();
	vvadpInit();
	//vdispInit();
	//vmachine.flagmode = 1;
}
void vmachineFinal()
{
//	vdispFinal();
	vvadpFinal();
//	vkeybFinal();
	vkbcFinal();
	vhddFinal();
	vfddFinal();
	vfdcFinal();
	vdmaFinal();
	vcmosFinal();
	vpitFinal();
	vpicFinal();
	vcpuFinal();
	vramFinal();
	vportFinal();
	debugFinal();
	recordFinal();
}
