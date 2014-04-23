/* This file is a part of NXLM project. */

#include "stdlib.h"
#include "memory.h"

#include "../vapi.h"
#include "../vmachine.h"
#include "../vcpuins.h"
#include "ccpuins.h"
#include "ccpuapi.h"

#define limit(n)  (min(0x10000, (0x100000 - ((n) << 4))))

#if DEBUGMODE == BCRM
t_ram cram;
#endif
DeCodeBlock ccpuins_deCodeBlock;
t_faddrcc ccpuins_repeatMethod;
t_bool ccpu_flagignore;
t_nubitcc ccpu_icount;
t_cpu ccpu;

void ccpuapiDebugPrintRegs()
{
	t_nubit16 f = ccpu.flags;
	vapiPrint("CCPU %04X\n",ccpu.flags);
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
	vapiPrint("VCPU %04X\n",vcpu.flags);
}
void ccpuapiSyncRegs()
{
	ccpu_flagignore = 0x00;
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
	ccpu.flags = _flags;
#if DEBUGMODE == BCRM
	if (!ccpu_icount) {
		memcpy((void *)cram.base, (void *)vram.base, vram.size);
	} else {
		memcpy((void *)(cram.base+0x0400), (void *)(vram.base+0x0400), 0x0100);
	}
#endif
}
void ccpuapiExecIns()
{
	ccpuinsExecIns();
	++ccpu_icount;
}
void ccpuapiExecInt(t_nubit8 intid) {ccpuinsExecInt(intid);}
t_bool ccpuapiHasDiff()
{
	t_bool flagdiff = 0x00;
#if (DEBUGMODE != VCPU && DEBUGMODE != CCPU)
#if DEBUGMODE == BCRM
	t_bool flagmemdiff = 0x00;
	t_nubit16 s1,s2;
	t_nubitcc i;
	if (ccpu_icount % 10000 == 0) vapiPrint("at #%d\n",ccpu_icount);
#endif
	if (!ccpu_flagignore) {
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
		if (ccpu.flags != vcpu.flags) {vapiPrint("diff flags\n");flagdiff = 0x01;}
#if DEBUGMODE == BCRM
		s1 = cramVarWord(ccpu.ss, ccpu.sp);
		s2 = vramVarWord(_ss,_sp);
		if (s1 != s2) {
			vapiPrint("diff stack: C:%04X V:%04X\n",s1,s2);
			flagdiff = 0x01;
		}
		if (flagdiff) flagmemdiff = 0x01;
		else if (memcmp((void *)cramGetAddr(ccpu.ds,0),(void *)vramGetAddr(_ds,0),limit(_ds)))
			flagmemdiff = 0x01;
		else if (_ss != _ds &&
			memcmp((void *)cramGetAddr(ccpu.ss,0),(void *)vramGetAddr(_ss,0),limit(_ss)))
			flagmemdiff = 0x01;
		else if (_es != _ss && _es != _ds &&
			memcmp((void *)cramGetAddr(ccpu.es,0),(void *)vramGetAddr(_es,0),limit(_es)))
			flagmemdiff = 0x01;
		else if (_cs != _es && _cs != _ss && _cs != _ds &&
			memcmp((void *)cramGetAddr(ccpu.cs,0),(void *)vramGetAddr(_cs,0),limit(_cs)))
			flagmemdiff = 0x01;
#endif
	} else {
#if DEBUGMODE == BCRM
		flagdiff = 0x00;
		flagmemdiff = 0x00;
		//if (vramVarWord(_cs,_ip-2) == 0x13cd) {
			memcpy((void *)cram.base, (void *)vram.base, vram.size);
	//	} else {
	//		memcpy((void *)(cram.base+0x0400), (void *)(vram.base+0x0400), 0x0100);
	//		memcpy((void *)(cram.base+0xb8000),(void *)(vram.base+0xb8000),0x1000);
	//	}
#endif
	}
#if DEBUGMODE == BCRM
	if (flagmemdiff) {
		if (!memcmp((void *)cram.base,(void *)vram.base,0x0417) &&
			!memcmp((void *)(cram.base+0x043e),(void *)(vram.base+0x043e),(vram.size-0x043e)))
			return 0x00;
		flagdiff = 0x01;
		vapiPrint("diff ram\n");flagdiff = 0x01;
		for(i = 0;i < vram.size;++i)
			if (cramVarByte(0,i) != vramVarByte(0,i))
				vapiPrint("%08X C:%02X V:%02X\n",i,
					cramVarByte(0,i),vramVarByte(0,i));
		memcpy((void *)cram.base, (void *)vram.base, vram.size);
	}
#endif
	if (flagdiff) vapiPrint("this diff happens at #%d\n",ccpu_icount);
#endif
	return flagdiff;
}

void ccpuapiInit()
{
	memset(&ccpu, 0x00, sizeof(t_cpu));
#if DEBUGMODE == BCRM
	cram.size = vram.size;
	cram.base = (t_vaddrcc)malloc(cram.size);
	memcpy((void *)cram.base, (void *)vram.base, vram.size);
#endif
	ccpuinsInit();
	ccpu.cs = 0xf000;
	ccpu.ip = 0xfff0;
}
void ccpuapiFinal() {
#if DEBUGMODE == BCRM
	if (cram.base) free((void *)(cram.base));
	cram.base = (t_vaddrcc)NULL;
	cram.size = 0x00;
#endif
}
