/* This file is a part of NXVM project. */

#include "memory.h"

#include "debug/aasm.h"
#include "debug/dasm.h"
#include "debug/debug.h"
#include "vapi.h"
#include "vmachine.h"

t_machine vmachine;

#define _expression "cs:ip=%x:%x opcode=%x %x %x %x %x %x %x %x \
ax=%x bx=%x cx=%x dx=%x sp=%x bp=%x si=%x di=%x ds=%x es=%x ss=%x \
of=%1x sf=%1x zf=%1x cf=%1x af=%1x pf=%1x df=%1x if=%1x tf=%1x\n"

static t_bool StopBeforeRefresh()
{
	if (vmachine.flagbreak && _cs == vmachine.breakcs && _ip == vmachine.breakip)
		return 1;
	if (vmachine.flagbreakx && (vcpu.cs.base + vcpu.eip == vmachine.breaklinear))
		return 1;
	return 0;
}
static t_bool StopAfterRefresh()
{
	if (vmachine.tracecnt) {
		vmachine.tracecnt--;
		if (!vmachine.tracecnt) return 1;
	}
	return 0;
}
static void DoBeforeRunLoop()
{
	if (vmachine.flagrecord) vapiRecordInit();
}
static void DoAfterRunLoop()
{
	if (vmachine.flagrecord) vapiRecordFinal();
}
static void DoBeforeRefresh()
{
	if (vmachine.flagrecord) vapiRecordExec();
}
static void DoAfterRefresh()
{}

void vapiCallBackMachineRun()
{
	DoBeforeRunLoop();
	while (vmachine.flagrun) {
		if (StopBeforeRefresh()) {
			vmachineStop();
			break;
		}
		DoBeforeRefresh();
		vmachineRefresh();
		DoAfterRefresh();
		if (StopAfterRefresh()) {
			vmachineStop();
			break;
		}
	}
	DoAfterRunLoop();
}
t_nubit8 vapiCallBackMachineGetFlagRun() {return vmachine.flagrun;}
void vapiCallBackMachineStart() {vmachineStart();}
void vapiCallBackMachineResume() {vmachineResume();}
void vapiCallBackMachineReset() {vmachineReset();}
void vapiCallBackMachineStop() {vmachineStop();}

static void vmachineAsmTest()
{
	static t_nubitcc total = 0;
	t_nubitcc i,lend, lena;
	char fetch[0x50], result[0x50];
	t_nubit8 ins[0x20];
	total++;
	lend = dasm(fetch, _cs, _ip, 0x00);
	memcpy(ins, (void *)vramGetRealAddr(_cs, _ip), lend);
	lena = aasm(fetch, _cs, _ip);
	dasm(result, _cs, _ip, 0x00);
	for (i = 0;i < 0x50;++i) {
		if (fetch[i] == '\n') fetch[i] = ' ';
		if (result[i] == '\n') result[i] = ' ';
	}
	if (lena != lend || memcmp(ins, (void *)vramGetRealAddr(_cs, _ip), lend) || STRCMP(fetch,result)) {
		vapiPrint("diff at #%d\t%04X:%04X\n", total, _cs, _ip);
		for (i = 0;i < lend;++i) vapiPrint("%02X", ins[i]);
		vapiPrint("\t%s\n", fetch);
		for (i = 0;i < lena;++i) vapiPrint("%02X", vramRealByte(_cs, _ip+i));
		vapiPrint("\t%s\n", result);
		if (lena < lend) {
			for (i = lena;i < lend;++i)
				vramRealByte(_cs, _ip + i) = 0x90;
		}
	}
}

void vmachineStart()
{
	vmachineReset();
	vmachineResume();
}
void vmachineResume() {
	if (vmachine.flagrun) return;
	vmachine.flagrun = 0x01;
	vapiStartMachine();
}
void vmachineStop()  {vmachine.flagrun = 0x00;}
void vmachineReset()
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
}
void vmachineRefresh()
{
	//vmachineAsmTest();
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
}
void vmachineInit()
{
	memset(&vmachine, 0x00, sizeof(t_machine));
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
	//vmachine.flagmode = 0x01;
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
}
