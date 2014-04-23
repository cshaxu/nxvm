/* This file is a part of NXVM project. */

#include "stdio.h"

#include "vcpuapi.h"
#include "vcpuins.h"

static t_cpu oldbcpu, newbcpu;

#ifdef VGLOBAL_BOCHS

t_nubit32 vapiPrint(const t_string format, ...)
{
	t_nubit32 nWrittenBytes = 0;
	va_list arg_ptr;
	va_start(arg_ptr, format);
	nWrittenBytes = vfprintf(stdout, format,arg_ptr);
	//nWrittenBytes = vsprintf(stringBuffer,format,arg_ptr);
	va_end(arg_ptr);
	fflush(stdout);
	return nWrittenBytes;
}
void vapiCallBackMachineStop() {}
void vapiSleep(t_nubit32 milisec) {}
void vapiCallBackDebugPrintRegs(t_bool bit32) {vcpuapiPrintReg(&vcpu);}
#else
#include "vapi.h"
#endif

static void xsregseg(t_cpu_sreg *rsreg, const t_string label)
{
	vapiPrint("%s=%04X, Base=%08X, Limit=%08X, DPL=%01X, %s, ", label,
		rsreg->selector, rsreg->base, rsreg->limit,
		rsreg->dpl, rsreg->seg.accessed ? "A" : "a");
	if (rsreg->seg.executable) {
		vapiPrint("Code, %s, %s, %s\n",
			rsreg->seg.exec.conform ? "C" : "c",
			rsreg->seg.exec.readable ? "Rw" : "rw",
			rsreg->seg.exec.defsize ? "32" : "16");
	} else {
		vapiPrint("Data, %s, %s, %s\n",
			rsreg->seg.data.expdown ? "E" : "e",
			rsreg->seg.data.writable ? "RW" : "Rw",
			rsreg->seg.data.big ? "BIG" : "big");
	} 
}
static void xsregsys(t_cpu_sreg *rsreg, const t_string label)
{
	vapiPrint("%s=%04X, Base=%08X, Limit=%08X, DPL=%01X, Type=%04X\n", label,
		rsreg->selector, rsreg->base, rsreg->limit,
		rsreg->dpl, rsreg->sys.type);
}
void vcpuapiPrintReg(t_cpu *rcpu)
{
	vapiPrint( "EAX=%08X",  rcpu->eax);
	vapiPrint(" EBX=%08X",  rcpu->ebx);
	vapiPrint(" ECX=%08X",  rcpu->ecx);
	vapiPrint(" EDX=%08X",  rcpu->edx);
	vapiPrint("\nESP=%08X", rcpu->esp);
	vapiPrint(" EBP=%08X",  rcpu->ebp);
	vapiPrint(" ESI=%08X",  rcpu->esi);
	vapiPrint(" EDI=%08X",  rcpu->edi);
	vapiPrint("\nEIP=%08X", rcpu->ip);
	vapiPrint(" EFL=%08X",  rcpu->eflags);
	vapiPrint(": ");
	vapiPrint("%s ", GetBit(rcpu->eflags, VCPU_EFLAGS_VM) ? "VM" : "vm");
	vapiPrint("%s ", GetBit(rcpu->eflags, VCPU_EFLAGS_RF) ? "RF" : "rf");
	vapiPrint("%s ", GetBit(rcpu->eflags, VCPU_EFLAGS_NT) ? "NT" : "nt");
	vapiPrint("IOPL=%01X ", ((rcpu->eflags & VCPU_EFLAGS_IOPL) >> 12));
	vapiPrint("%s ", GetBit(rcpu->eflags, VCPU_EFLAGS_OF) ? "OF" : "of");
	vapiPrint("%s ", GetBit(rcpu->eflags, VCPU_EFLAGS_DF) ? "DF" : "df");
	vapiPrint("%s ", GetBit(rcpu->eflags, VCPU_EFLAGS_IF) ? "IF" : "if");
	vapiPrint("%s ", GetBit(rcpu->eflags, VCPU_EFLAGS_TF) ? "TF" : "tf");
	vapiPrint("%s ", GetBit(rcpu->eflags, VCPU_EFLAGS_SF) ? "SF" : "sf");
	vapiPrint("%s ", GetBit(rcpu->eflags, VCPU_EFLAGS_ZF) ? "ZF" : "zf");
	vapiPrint("%s ", GetBit(rcpu->eflags, VCPU_EFLAGS_AF) ? "AF" : "af");
	vapiPrint("%s ", GetBit(rcpu->eflags, VCPU_EFLAGS_PF) ? "PF" : "pf");
	vapiPrint("%s ", GetBit(rcpu->eflags, VCPU_EFLAGS_CF) ? "CF" : "cf");
	vapiPrint("\n");
}

void vcpuapiPrintSreg(t_cpu *rcpu)
{
	xsregseg(&rcpu->es, "ES");
	xsregseg(&rcpu->cs, "CS");
	xsregseg(&rcpu->ss, "SS");
	xsregseg(&rcpu->ds, "DS");
	xsregseg(&rcpu->fs, "FS");
	xsregseg(&rcpu->gs, "GS");
	xsregsys(&rcpu->tr, "TR  ");
	xsregsys(&rcpu->ldtr, "LDTR");
	vapiPrint("GDTR Base=%08X, Limit=%04X\n",
		rcpu->gdtr.base, rcpu->gdtr.limit);
	vapiPrint("IDTR Base=%08X, Limit=%04X\n",
		rcpu->idtr.base, rcpu->idtr.limit);
}
void vcpuapiPrintCreg(t_cpu *rcpu)
{
	vapiPrint("CR0=%08X: %s %s %s %s %s %s\n", rcpu->cr0,
		GetBit(rcpu->cr0, VCPU_CR0_PG) ? "PG" : "pg",
		GetBit(rcpu->cr0, VCPU_CR0_ET) ? "ET" : "et",
		GetBit(rcpu->cr0, VCPU_CR0_TS) ? "TS" : "ts",
		GetBit(rcpu->cr0, VCPU_CR0_EM) ? "EM" : "em",
		GetBit(rcpu->cr0, VCPU_CR0_MP) ? "MP" : "mp",
		GetBit(rcpu->cr0, VCPU_CR0_PE) ? "PE" : "pe");
	vapiPrint("CR2=PFLR=%08X\n", rcpu->cr2);
	vapiPrint("CR3=PDBR=%08X\n", rcpu->cr3);
}

void vcpuapiLoadSreg(t_cpu_sreg *rsreg, t_nubit32 lo, t_nubit32 hi)
{
	t_nubit64 cdesc = ((t_nubit64)hi << 32) | lo;
	rsreg->dpl = (t_nubit4)_GetDesc_DPL(cdesc);
	if (_IsDescUser(cdesc)) {
		rsreg->base = (t_nubit32)_GetDescSeg_Base(cdesc);
		rsreg->limit = (t_nubit32)((_IsDescSegGranularLarge(cdesc) ?
			((_GetDescSeg_Limit(cdesc) << 12) | 0x0fff) : (_GetDescSeg_Limit(cdesc))));
		rsreg->seg.accessed = _IsDescUserAccessed(cdesc);
		rsreg->seg.executable = _IsDescCode(cdesc);
		if (rsreg->seg.executable) {
			rsreg->seg.exec.conform = _IsDescCodeConform(cdesc);
			rsreg->seg.exec.defsize = _IsDescCode32(cdesc);
			rsreg->seg.exec.readable = _IsDescCodeReadable(cdesc);
		} else {
			rsreg->seg.data.big = _IsDescDataBig(cdesc);
			rsreg->seg.data.expdown = _IsDescDataExpDown(cdesc);
			rsreg->seg.data.writable = _IsDescDataWritable(cdesc);
		}
	} else {
		rsreg->base = (t_nubit32)_GetDescSeg_Base(cdesc);
		rsreg->limit = (t_nubit32)((_IsDescSegGranularLarge(cdesc) ?
				(_GetDescSeg_Limit(cdesc) << 12 | 0x0fff) : (_GetDescSeg_Limit(cdesc))));
		rsreg->sys.type = (t_nubit4)_GetDesc_Type(cdesc);
	}
}

static t_bool vcpuapiCheckDiff()
{
	t_bool flagdiff = 0x00;
	t_nubit32 mask = vcpuins.udf;
	if (!vcpu.flagignore) {
		if (vcpu.eax != newbcpu.eax) {vapiPrint("diff eax\n");flagdiff = 0x01;}
		if (vcpu.ebx != newbcpu.ebx) {vapiPrint("diff ebx\n");flagdiff = 0x01;}
		if (vcpu.ecx != newbcpu.ecx) {vapiPrint("diff ecx\n");flagdiff = 0x01;}
		if (vcpu.edx != newbcpu.edx) {vapiPrint("diff edx\n");flagdiff = 0x01;}
		if (vcpu.esp != newbcpu.esp) {vapiPrint("diff esp\n");flagdiff = 0x01;}
		if (vcpu.ebp != newbcpu.ebp) {vapiPrint("diff ebp\n");flagdiff = 0x01;}
		if (vcpu.esi != newbcpu.esi) {vapiPrint("diff esi\n");flagdiff = 0x01;}
		if (vcpu.edi != newbcpu.edi) {vapiPrint("diff edi\n");flagdiff = 0x01;}
		if (vcpu.eip != newbcpu.eip) {vapiPrint("diff eip\n");flagdiff = 0x01;}
		if (vcpu.es.selector != newbcpu.es.selector ||
			vcpu.es.base != newbcpu.es.base ||
			vcpu.es.limit != newbcpu.es.limit ||
			vcpu.es.dpl != newbcpu.es.dpl) {vapiPrint("diff es\n");flagdiff = 0x01;}
		if (vcpu.cs.selector != newbcpu.cs.selector ||
			vcpu.cs.base != newbcpu.cs.base ||
			vcpu.cs.limit != newbcpu.cs.limit ||
			vcpu.cs.dpl != newbcpu.cs.dpl) {vapiPrint("diff cs\n");flagdiff = 0x01;}
		if (vcpu.ss.selector != newbcpu.ss.selector ||
			vcpu.ss.base != newbcpu.ss.base ||
			vcpu.ss.limit != newbcpu.ss.limit ||
			vcpu.ss.dpl != newbcpu.ss.dpl) {vapiPrint("diff ss\n");flagdiff = 0x01;}
		if (vcpu.ds.selector != newbcpu.ds.selector ||
			vcpu.ds.base != newbcpu.ds.base ||
			vcpu.ds.limit != newbcpu.ds.limit ||
			vcpu.ds.dpl != newbcpu.ds.dpl) {vapiPrint("diff ds\n");flagdiff = 0x01;}
		if (vcpu.fs.selector != newbcpu.fs.selector ||
			vcpu.fs.base != newbcpu.fs.base ||
			vcpu.fs.limit != newbcpu.fs.limit ||
			vcpu.fs.dpl != newbcpu.fs.dpl) {vapiPrint("diff fs\n");flagdiff = 0x01;}
		if (vcpu.gs.selector != newbcpu.gs.selector ||
			vcpu.gs.base != newbcpu.gs.base ||
			vcpu.gs.limit != newbcpu.gs.limit ||
			vcpu.gs.dpl != newbcpu.gs.dpl) {vapiPrint("diff gs\n");flagdiff = 0x01;}
		if ((vcpu.eflags & ~mask) != (newbcpu.eflags & ~mask)) {
			vapiPrint("diff flags: V=%08X, B=%08X\n", vcpu.eflags, newbcpu.eflags);
			flagdiff = 0x01;
		}
	}
	if (flagdiff) {
		vapiPrint("BEFORE EXECUTION:\n");
		vcpuapiPrintReg(&oldbcpu);
		vapiPrint("-----------------------------------------\n");
		vapiPrint("AFTER EXECUTION:\n");
		vcpuapiPrintReg(&newbcpu);
		vapiPrint("CURRENT VCPU:\n");
		vcpuapiPrintReg(&vcpu);
		vapiPrint("-----------------------------------------\n");
	}
	return flagdiff;
}


void vcpuapiInit()
{
	vcpuInit();
}
void vcpuapiFinal()
{
	vcpuFinal();
}
t_bool vcpuapiExecBefore(t_cpu *rcpu)
{
	oldbcpu = *rcpu;
	vcpu = oldbcpu;
	vcpuinsRefresh();
	return 0;
}
t_bool vcpuapiExecAfter(t_cpu *rcpu)
{
	newbcpu = *rcpu;
	return vcpuapiCheckDiff();
}

t_nubit64 vcpuapiReadPhysical(t_nubit32 phy, t_nubit8 byte)
{
	return 0;
}
void vcpuapiWritePhysical(t_nubit32 phy, t_nubit64 data, t_nubit8 byte)
{}
