/* This file is a part of NXVM project. */

#include "memory.h"

#include "vapi.h"
#include "vmachine.h"

t_machine vmachine;

void vmachineDumpRecordFile(t_string fname) {vapiRecordDump(fname);}
void vmachineInsertFloppy(t_string fname)  {vapiFloppyInsert(fname);}
void vmachineRemoveFloppy(t_string fname)  {vapiFloppyRemove(fname);}

void vmachineStart() {vmachine.flagrun = 0x01;vapiStartMachine();}
void vmachineStop()  {vmachine.flagrun = 0x00;}

#define _expression "cs:ip=%x:%x opcode=%x %x %x %x %x %x %x %x \
ax=%x bx=%x cx=%x dx=%x sp=%x bp=%x si=%x di=%x ds=%x es=%x ss=%x \
of=%1x sf=%1x zf=%1x cf=%1x af=%1x pf=%1x df=%1x if=%1x tf=%1x\n"

void vapiCallBackMachineRun()
{
//FILE *fp;
	if(vmachine.flaginit) {
		if (vmachine.flagrecord) vapiRecordStart();
//fp = fopen("d:/nxvm.log","w");
		while (vmachine.flagrun) {
			if (vmachine.flagbreak &&
				vcpu.cs == vmachine.breakcs && vcpu.ip == vmachine.breakip) {
				vmachineStop();
				break;
			}
			if (vmachine.flagrecord) vapiRecordExec();
//fprintf(fp, _expression,
//_cs, _ip,
//vramVarByte(_cs,_ip+0),vramVarByte(_cs,_ip+1),
//vramVarByte(_cs,_ip+2),vramVarByte(_cs,_ip+3),
//vramVarByte(_cs,_ip+4),vramVarByte(_cs,_ip+5),
//vramVarByte(_cs,_ip+6),vramVarByte(_cs,_ip+7),
//_ax,_bx,_cx,_dx,
//_sp,_bp,_si,_di,
//_ds,_es,_ss,
//_of,_sf,_zf,_cf,
//_af,_pf,_df,_if,_tf);
			vmachineRefresh();
			if (vmachine.flagtrace) vmachineStop();
		}
//fclose(fp);
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
	vdispRefresh();
	vvadpRefresh();
	vkeybRefresh();
	vkbcRefresh();
*/
	vpitRefresh();
//	qdbiosRefresh();
//	vfddRefresh();
	vfdcRefresh();
	vdmaRefresh();
	vpicRefresh();
	vcmosRefresh();
	vcpuRefresh();
//	vramRefresh();
//	vportRefresh();
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
#ifdef VMACHINE_DEBUG
	qdbiosInit();
#endif
/*
	vkbcInit();
	vkeybInit();
	vvadpInit();
	vdispInit();
*/
	vmachine.flagmode   = 0x01;
	vmachine.flagrecord = 0x01;
	vmachine.flaginit   = 0x01;
}
void vmachineFinal()
{
/*
	vkbcFinal();
	vkeybFinal();
	vvadpFinal();
	vdispFinal();
*/
	vpitFinal();
#ifdef VMACHINE_DEBUG
	qdbiosFinal();
#endif
	vfddFinal();
	vfdcFinal();
	vdmaFinal();
	vcmosFinal();
	vpicFinal();
	vcpuFinal();
	vramFinal();
	vportFinal();
	memset(&vmachine, 0x00, sizeof(t_machine));
}
