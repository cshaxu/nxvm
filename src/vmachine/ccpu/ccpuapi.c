/* This file is a part of NXLM project. */

#include "memory.h"

#include "../vapi.h"
#include "../vmachine.h"

#include "ccpuapi.h"

t_bool ccpuapiHasDiff()
{
	t_nubitcc i;
	t_bool flagdiff = 0x00;
	if (!ccpu.flagignore) {
		if (ccpu.ax != _ax) {vapiPrint("diff ax\n");flagdiff = 0x01;}
		if (ccpu.bx != _bx) {vapiPrint("diff bx\n");flagdiff = 0x01;}
		if (ccpu.cx != _cx) {vapiPrint("diff cx\n");flagdiff = 0x01;}
		if (ccpu.dx != _dx) {vapiPrint("diff dx\n");flagdiff = 0x01;}
		if (ccpu.sp != _sp) {vapiPrint("diff sp\n");flagdiff = 0x01;}
		if (ccpu.bp != _bp) {vapiPrint("diff bp\n");flagdiff = 0x01;}
		if (ccpu.si != _si) {vapiPrint("diff si\n");flagdiff = 0x01;}
		if (ccpu.di != _di) {vapiPrint("diff di\n");flagdiff = 0x01;}
		if (ccpu.ip != _ip) {vapiPrint("diff ip\n");flagdiff = 0x01;}
		if (ccpu.cs != _cs) {vapiPrint("diff cs\n");flagdiff = 0x01;}
		if (ccpu.ds != _ds) {vapiPrint("diff ds\n");flagdiff = 0x01;}
		if (ccpu.es != _es) {vapiPrint("diff es\n");flagdiff = 0x01;}
		if (ccpu.ss != _ss) {vapiPrint("diff ss\n");flagdiff = 0x01;}
		if (ccpu.iobyte != vcpu.iobyte) {vapiPrint("diff iobyte\n");flagdiff = 0x01;}
		if (ccpu_getAF_Flag() != _af) {vapiPrint("diff af\n");flagdiff = 0x01;}
		if (ccpu_getCF_Flag() != _cf) {vapiPrint("diff cf\n");flagdiff = 0x01;}
		if (ccpu_getDF_Flag() != _df) {vapiPrint("diff df\n");flagdiff = 0x01;}
		if (ccpu_getIF_Flag() != _if) {vapiPrint("diff if\n");flagdiff = 0x01;}
		if (ccpu_getOF_Flag() != _of) {vapiPrint("diff of\n");flagdiff = 0x01;}
		if (ccpu_getPF_Flag() != _pf) {vapiPrint("diff pf\n");flagdiff = 0x01;}
		if (ccpu_getSF_Flag() != _sf) {vapiPrint("diff sf\n");flagdiff = 0x01;}
		if (ccpu_getTF_Flag() != _tf) {vapiPrint("diff tf\n");flagdiff = 0x01;}
		if (ccpu_getZF_Flag() != _zf) {vapiPrint("diff zf\n");flagdiff = 0x01;}
	} else {
		memcpy((void *)cram.base, (void *)vram.base, vram.size);
	}
#if CCPU_RAM == CRAM
	if (memcmp((void *)cram.base, (void *)vram.base, vram.size)) {
		//if (cramVarByte(0,0x046c) + 1 == vramVarByte(0,0x46c)) return flagdiff;
		vapiPrint("diff ram\n");flagdiff = 0x01;
		for(i = 0;i < vram.size;++i)
			if (cramVarByte(0,i) != vramVarByte(0,i))
				vapiPrint("%08X C:%02X V:%02X\n",i,
					cramVarByte(0,i),vramVarByte(0,i));
		memcpy((void *)cram.base, (void *)vram.base, vram.size);
	}
#endif
	if (flagdiff) vapiPrint("Above diff is #%d\n",ccpu.icount);
//	if (ccpu.icount % 10000 == 0) vapiPrint("Current at #%d\n",ccpu.icount);
	return flagdiff;
}

/* insert into console/debug::rprintregs() */
void ccpuapiDebugPrintRegs()
{
	t_nubit16 f = ccpu_generateFLAG();
	vapiPrint("AX=");
	vapiPrintWord(ccpu.ax);
	vapiPrint("  BX=");
	vapiPrintWord(ccpu.bx);
	vapiPrint("  CX=");
	vapiPrintWord(ccpu.cx);
	vapiPrint("  DX=");
	vapiPrintWord(ccpu.dx);
	vapiPrint("  SP=");
	vapiPrintWord(ccpu.sp);
	vapiPrint("  BP=");
	vapiPrintWord(ccpu.bp);
	vapiPrint("  SI=");
	vapiPrintWord(ccpu.si);
	vapiPrint("  DI=");
	vapiPrintWord(ccpu.di);
	vapiPrint("\nDS=");
	vapiPrintWord(ccpu.ds);
	vapiPrint("  ES=");
	vapiPrintWord(ccpu.es);
	vapiPrint("  SS=");
	vapiPrintWord(ccpu.ss);
	vapiPrint("  CS=");
	vapiPrintWord(ccpu.cs);
	vapiPrint("  IP=");
	vapiPrintWord(ccpu.ip);
	vapiPrint("   ");
	if(f & VCPU_FLAG_OF)
		vapiPrint("OV ");
	else
		vapiPrint("NV ");
	if(f & VCPU_FLAG_DF)
		vapiPrint("DN ");
	else
		vapiPrint("UP ");
	if(f & VCPU_FLAG_IF)
		vapiPrint("EI ");
	else
		vapiPrint("DI ");
	if(f & VCPU_FLAG_SF)
		vapiPrint("NG ");
	else
		vapiPrint("PL ");
	if(f & VCPU_FLAG_ZF)
		vapiPrint("ZR ");
	else
		vapiPrint("NZ ");
	if(f & VCPU_FLAG_AF)
		vapiPrint("AC ");
	else
		vapiPrint("NA ");
	if(f & VCPU_FLAG_PF)
		vapiPrint("PE ");
	else
		vapiPrint("PO ");
	if(f & VCPU_FLAG_CF)
		vapiPrint("CY ");
	else
		vapiPrint("NC ");
	vapiPrint("\n\n");
}

/* insert into vcpuinsExecIns() */
void ccpuapiSyncRegs()
{
	ccpu.flagignore = 0x00;
	ccpu.ax = _ax;
	ccpu.bx = _bx;
	ccpu.cx = _cx;
	ccpu.dx = _dx;
	ccpu.sp = _sp;
	ccpu.bp = _bp;
	ccpu.si = _si;
	ccpu.di = _di;
	ccpu.ip = _ip;
	ccpu.cs = _cs;
	ccpu.ds = _ds;
	ccpu.es = _es;
	ccpu.ss = _ss;
	ccpu.overds = vcpu.overds;
	ccpu.overss = vcpu.overss;
	ccpu.iobyte = vcpu.iobyte;
	ccpu_setAF_Flag_flag(_af);
	ccpu_setCF_Flag_flag(_cf);
	ccpu_setDF_Flag_flag(_df);
	ccpu_setIF_Flag_flag(_if);
	ccpu_setOF_Flag_flag(_of);
	ccpu_setPF_Flag_flag(_pf);
	ccpu_setSF_Flag_flag(_sf);
	ccpu_setTF_Flag_flag(_tf);
	ccpu_setZF_Flag_flag(_zf);
}

/* insert into vcpuinsExecIns() */
void ccpuapiExecIns()
{
	decode_deCodeInstruction();
	++ccpu.icount;
}

/* insert into vcpuinsExecInt() */
void ccpuapiExecInt(t_nubit8 intid)
{
	ins_atomMethod_INT(intid);
}

/* insert into vcpuinsInit() */
void ccpuapiInit()
{
	ccpu_init();
	ccpu.cs = 0xf000;
	ccpu.ip = 0xfff0;
}
/* insert into vcpuinsFinal() */
void ccpuapiFinal() {ccpu_final();}
