/* This file is a part of NXVM project. */

#include "memory.h"

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
	t_bool flagtextonly = 0;
	t_nubit8 i, lend1, lend2, lena;
	t_string dstr1, dstr2;
	t_nubit8 ins1[15], ins2[15];
	total++;
	vcpuinsReadLinear(vcpu.cs.base + vcpu.eip, (t_vaddrcc)ins1, 15);
/*	ins1[0] = 0x67;
	ins1[1] = 0xc6;
	ins1[2] = 0x44;
	ins1[3] = 0xf2;
	ins1[4] = 0x05;
	ins1[5] = 0x8e;
	ins1[6] = 0x00;*/
	switch (d_nubit8(ins1)) {
	case 0x88: case 0x89: case 0x8a: case 0x8b:
	case 0x00: case 0x01: case 0x02: case 0x03:
	case 0x08: case 0x09: case 0x0a: case 0x0b:
	case 0x10: case 0x11: case 0x12: case 0x13:
	case 0x18: case 0x19: case 0x1a: case 0x1b:
	case 0x20: case 0x21: case 0x22: case 0x23:
	case 0x28: case 0x29: case 0x2a: case 0x2b:
	case 0x30: case 0x31: case 0x32: case 0x33:
	case 0x38: case 0x39: case 0x3a: case 0x3b:
		flagtextonly = 1;
		break;
	}
	switch (d_nubit8(ins1+1)) {
	case 0x90:
		flagtextonly = 1;
		break;
	}
	switch (d_nubit16(ins1)) {
	case 0x2e66:
		flagtextonly = 1;
		break;
	}
	switch (GetMax24(d_nubit24(ins1))) {
	case 0xb70f66:
		flagtextonly = 1;
		break;
	}
	switch (GetMax24(d_nubit24(ins1+1))) {
	case 0xb70f66:
		flagtextonly = 1;
		break;
	}
	switch (d_nubit32(ins1)) {
	}
	lend1 = dasm32(dstr1, (t_vaddrcc)ins1);
	lena  = aasm32(dstr1, (t_vaddrcc)ins2);
	lend2 = dasm32(dstr2, (t_vaddrcc)ins2);
	if ((!flagtextonly && (lena != lend1 || lena != lend2 || lend1 != lend2 ||
		memcmp(ins1, ins2, lend1))) || STRCMP(dstr1, dstr2)) {
		vapiPrint("diff at #%d %04X:%08X(L%08X), len(a=%x,d1=%x,d2=%x), CodeSegDefSize=%d\n",
			total, _cs, _eip, vcpu.cs.base + vcpu.eip, lena, lend1, lend2, vcpu.cs.seg.exec.defsize ? 32 : 16);
		for (i = 0;i < lend1;++i) vapiPrint("%02X", ins1[i]);
		vapiPrint("\t%s\n", dstr1);
		for (i = 0;i < lend2;++i) vapiPrint("%02X", ins2[i]);
		vapiPrint("\t%s\n", dstr2);
		vmachineStop();
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
