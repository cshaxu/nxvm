/* This file is a part of NXVM project. */

#include "memory.h"

#include "debug/aasm.h"
#include "debug/aasm32.h"
#include "debug/dasm32.h"
#include "debug/record.h"
#include "debug/debug.h"
#include "vapi.h"
#include "vmachine.h"

t_machine vmachine;

static t_bool StopBeforeRefresh()
{
	if (vmachine.flagbreak && _cs == vmachine.breakcs && _ip == vmachine.breakip && vmachine.breakcnt)
		return 1;
	if (vmachine.flagbreakx && (vcpu.cs.base + vcpu.eip == vmachine.breaklinear) && vmachine.breakcnt)
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
	if (vmachine.flagrecord) recordInit();
}
static void DoAfterRunLoop()
{
	if (vmachine.flagrecord) recordFinal();
}
static void DoBeforeRefresh() {}
static void DoAfterRefresh()
{
	if (vmachine.flagrecord) recordExec(&vcpurec);
	vmachine.breakcnt++;
}

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
void vapiCallBackMachineReset()
{
	if (vmachine.flagrun)
		vmachine.flagreset = 1;
	else
		vmachineReset();
}
void vapiCallBackMachineStop() {vmachineStop();}

static void vmachineAsmTest()
{
	static t_nubitcc total = 0;
	t_nubit8 i, lend1, lend2, lena;
	t_string dstr1, dstr2;
	t_nubit8 ins1[15], ins2[15];
	total++;
	vcpuinsReadLinear((_cs << 4) + _ip, (t_vaddrcc)ins1, 15);
	switch (d_nubit16(ins1)) {
	case 0xac26:
	case 0xac2e:
	case 0xac36:
	case 0xac3e:
	case 0xad26:
	case 0xad2e:
	case 0xad3e:
	case 0xad36:
	case 0xd726:
	case 0xd72e:
	case 0xd73e:
	case 0xd736:
		return;
	}
	lend1 = dasm32(dstr1, (t_vaddrcc)ins1);
	lena  = aasm32(dstr1, (t_vaddrcc)ins2);
	lend2 = dasm32(dstr2, (t_vaddrcc)ins2);
	if (lena != lend1 || lena != lend2 || lend1 != lend2 ||
		memcmp(ins1, ins2, lend1) || STRCMP(dstr1, dstr2)) {
		vapiPrint("diff at #%d %04X:%04X, len(a=%x,d1=%x,d2=%x)\n",
			total, _cs, _ip, lena, lend1, lend2);
		for (i = 0;i < lend1;++i) vapiPrint("%02X", ins1[i]);
		vapiPrint("\t%s\n", dstr1);
		for (i = 0;i < lend2;++i) vapiPrint("%02X", ins2[i]);
		vapiPrint("\t%s\n", dstr2);
		vmachineStop();
		/*if (lena < lend1) {
			for (i = lena;i < lend1;++i) ins2[i] = 0x90;
			for (i = 0;i < lend1;++i) vramRealByte(_cs, _ip + i) = ins2[i];
		}*/
	}
}

void vmachineStart()
{
	vmachineReset();
	vmachineResume();
}
void vmachineResume() {
	if (vmachine.flagrun) return;
	vmachine.flagrun = 1;
	vapiStartMachine();
}
void vmachineStop()  {vmachine.flagrun = 0;}
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
	vmachine.flagreset = 0;
}
void vmachineRefresh()
{
	if (vmachine.flagreset) vmachineReset();
	vmachineAsmTest();
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
}
