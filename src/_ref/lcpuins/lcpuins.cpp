/* This file is a part of NXLM project. */

#include "memory.h"

#include "vcpu.h"
#include "lcpuins.h"
#include "vapi.h"
#include "vpic.h"

CentreProcessorUnit *pcpu;

static t_bool lcpuinsHasDiff()
{
	t_bool flagdiff = 0x00;
	if (lcpu.ax != _ax) {vapiPrint("%d\tdiff ax\n", lcpu.icount);flagdiff = 0x01;}
	if (lcpu.bx != _bx) {vapiPrint("%d\tdiff bx\n", lcpu.icount);flagdiff = 0x01;}
	if (lcpu.cx != _cx) {vapiPrint("%d\tdiff cx\n", lcpu.icount);flagdiff = 0x01;}
	if (lcpu.dx != _dx) {vapiPrint("%d\tdiff dx\n", lcpu.icount);flagdiff = 0x01;}
	if (lcpu.sp != _sp) {vapiPrint("%d\tdiff sp\n", lcpu.icount);flagdiff = 0x01;}
	if (lcpu.bp != _bp) {vapiPrint("%d\tdiff bp\n", lcpu.icount);flagdiff = 0x01;}
	if (lcpu.si != _si) {vapiPrint("%d\tdiff si\n", lcpu.icount);flagdiff = 0x01;}
	if (lcpu.di != _di) {vapiPrint("%d\tdiff di\n", lcpu.icount);flagdiff = 0x01;}
	if (lcpu.ip != _ip) {vapiPrint("%d\tdiff ip\n", lcpu.icount);flagdiff = 0x01;}
	if (lcpu.cs != _cs) {vapiPrint("%d\tdiff cs\n", lcpu.icount);flagdiff = 0x01;}
	if (lcpu.ds != _ds) {vapiPrint("%d\tdiff ds\n", lcpu.icount);flagdiff = 0x01;}
	if (lcpu.es != _es) {vapiPrint("%d\tdiff es\n", lcpu.icount);flagdiff = 0x01;}
	if (lcpu.ss != _ss) {vapiPrint("%d\tdiff ss\n", lcpu.icount);flagdiff = 0x01;}
	if (lcpu.iobyte != vcpu.iobyte) {vapiPrint("%d\tdiff iobyte\n", lcpu.icount);flagdiff = 0x01;}
	if (lcpu.getAF_Flag() != _af) {vapiPrint("%d\tdiff af\n", lcpu.icount);flagdiff = 0x01;}
	if (lcpu.getCF_Flag() != _cf) {vapiPrint("%d\tdiff cf\n", lcpu.icount);flagdiff = 0x01;}
	if (lcpu.getDF_Flag() != _df) {vapiPrint("%d\tdiff df\n", lcpu.icount);flagdiff = 0x01;}
	if (lcpu.getIF_Flag() != _if) {vapiPrint("%d\tdiff if\n", lcpu.icount);flagdiff = 0x01;}
	if (lcpu.getOF_Flag() != _of) {vapiPrint("%d\tdiff of\n", lcpu.icount);flagdiff = 0x01;}
	if (lcpu.getPF_Flag() != _pf) {vapiPrint("%d\tdiff pf\n", lcpu.icount);flagdiff = 0x01;}
	if (lcpu.getSF_Flag() != _sf) {vapiPrint("%d\tdiff sf\n", lcpu.icount);flagdiff = 0x01;}
	if (lcpu.getTF_Flag() != _tf) {vapiPrint("%d\tdiff tf\n", lcpu.icount);flagdiff = 0x01;}
	if (lcpu.getZF_Flag() != _zf) {vapiPrint("%d\tdiff zf\n", lcpu.icount);flagdiff = 0x01;}
	return flagdiff;
}

/*
 * insert into console/debug::rprintregs()
 * at first line
 */
void lcpuinsDebugPrintlregs()
{
	t_nubit16 f = lcpu.generateFLAG();
	vapiPrint("CPU 2\n");
	vapiPrint("AX=");
	vapiPrintWord(lcpu.ax);
	vapiPrint("  BX=");
	vapiPrintWord(lcpu.bx);
	vapiPrint("  CX=");
	vapiPrintWord(lcpu.cx);
	vapiPrint("  DX=");
	vapiPrintWord(lcpu.dx);
	vapiPrint("  SP=");
	vapiPrintWord(lcpu.sp);
	vapiPrint("  BP=");
	vapiPrintWord(lcpu.bp);
	vapiPrint("  SI=");
	vapiPrintWord(lcpu.si);
	vapiPrint("  DI=");
	vapiPrintWord(lcpu.di);
	vapiPrint("\nDS=");
	vapiPrintWord(lcpu.ds);
	vapiPrint("  ES=");
	vapiPrintWord(lcpu.es);
	vapiPrint("  SS=");
	vapiPrintWord(lcpu.ss);
	vapiPrint("  CS=");
	vapiPrintWord(lcpu.cs);
	vapiPrint("  IP=");
	vapiPrintWord(lcpu.ip);
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
	vapiPrint("\n");
}

/*
 * insert into vcpuinsExecIns() 
 * before:
 * ExecFun(vcpuins.table[opcode]);
 */
void lcpuinsSyncRegs()
{
	lcpu.ax = _ax;
	lcpu.bx = _bx;
	lcpu.cx = _cx;
	lcpu.dx = _dx;
	lcpu.sp = _sp;
	lcpu.bp = _bp;
	lcpu.si = _si;
	lcpu.di = _di;
	lcpu.ip = _ip;
	lcpu.cs = _cs;
	lcpu.ds = _ds;
	lcpu.es = _es;
	lcpu.ss = _ss;
	lcpu.overds = vcpu.overds;
	lcpu.overss = vcpu.overss;
	lcpu.iobyte = vcpu.iobyte;
	lcpu.setAF_Flag(_af);
	lcpu.setCF_Flag(_cf);
	lcpu.setDF_Flag(_df);
	lcpu.setIF_Flag(_if);
	lcpu.setOF_Flag(_of);
	lcpu.setPF_Flag(_pf);
	lcpu.setSF_Flag(_sf);
	lcpu.setTF_Flag(_tf);
	lcpu.setZF_Flag(_zf);
}

/*
 * insert into vcpuinsExecIns()
 * after:
 * if(!IsPrefix(opcode)) ClrPrefix();
 */
void lcpuinsExecIns()
{
	lcpu.deCodeBlock.deCodeInstruction();
	lcpu.icount++;
	if (lcpuinsHasDiff()) vcpu.flagterm = 0x01;
}
void lcpuinsExecInt()
{
//	t_nubit8 intid;
//	if(GetBit(lcpu.generateFLAG() , VCPU_FLAG_IF) && vpicScanINTR()) {
//		intid = vpicGetINTR();
		lcpu.deCodeBlock.pexeCodeBlock->atomMethod_INT(0x09);//intid);	
//	}
	if(GetBit(lcpu.generateFLAG(), VCPU_FLAG_TF))
		lcpu.deCodeBlock.pexeCodeBlock->atomMethod_INT(0x01);
}

/*
 * insert into vcpuinsInit()
 * at first line
 */
void lcpuinsInit()
{
	pcpu = new CentreProcessorUnit;
	lcpu.init();
	lcpu.cs = 0xf000;
	lcpu.ip = 0xfff0;
}
/*
 * insert into vcpuinsFinal()
 * at first line
 */
void lcpuinsFinal()
{
	delete pcpu;
}
