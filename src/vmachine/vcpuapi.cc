/* This file is a part of NXVM project. */

#include "stdio.h"

#include "vcpuapi.h"
#include "vcpuins.h"

static t_cpu oldbcpu, newbcpu;


#ifdef VGLOBAL_BOCHS
static t_bool flagvalid = 0;
#define NEED_CPU_REG_SHORTCUTS 1
#include "d:/bochs-2.6/bochs.h"
#include "d:/bochs-2.6/cpu/cpu.h"

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

static void LoadSreg(t_cpu_sreg *rsreg, bx_dbg_sreg_t *rbsreg)
{
	t_nubit64 cdesc;
	rsreg->selector = rbsreg->sel;
	rsreg->flagvalid = rbsreg->valid;
	cdesc = ((t_nubit64)rbsreg->des_h << 32) | rbsreg->des_l;
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

static void CopyBochsCpu(t_cpu *rcpu)
{
	bx_dbg_sreg_t bsreg;
	bx_dbg_global_sreg_t bgsreg;
	rcpu->eax = EAX;
	rcpu->ecx = ECX;
	rcpu->edx = EDX;
	rcpu->ebx = EBX;
	rcpu->esp = ESP;
	rcpu->ebp = EBP;
	rcpu->esi = ESI;
	rcpu->edi = EDI;
	rcpu->eip = EIP;
	rcpu->eflags = BX_CPU_THIS_PTR read_eflags();

	rcpu->es.sregtype = SREG_DATA;
	BX_CPU_THIS_PTR dbg_get_sreg(&bsreg, 0);
	LoadSreg(&rcpu->es, &bsreg);

	rcpu->cs.sregtype = SREG_CODE;
	BX_CPU_THIS_PTR dbg_get_sreg(&bsreg, 1);
	LoadSreg(&rcpu->cs, &bsreg);

	rcpu->ss.sregtype = SREG_STACK;
	BX_CPU_THIS_PTR dbg_get_sreg(&bsreg, 2);
	LoadSreg(&rcpu->ss, &bsreg);

	rcpu->ds.sregtype = SREG_DATA;
	BX_CPU_THIS_PTR dbg_get_sreg(&bsreg, 3);
	LoadSreg(&rcpu->ds, &bsreg);

	rcpu->fs.sregtype = SREG_DATA;
	BX_CPU_THIS_PTR dbg_get_sreg(&bsreg, 4);
	LoadSreg(&rcpu->fs, &bsreg);

	rcpu->gs.sregtype = SREG_DATA;
	BX_CPU_THIS_PTR dbg_get_sreg(&bsreg, 5);
	LoadSreg(&rcpu->gs, &bsreg);

	rcpu->ldtr.sregtype = SREG_LDTR;
	BX_CPU_THIS_PTR dbg_get_ldtr(&bsreg);
	LoadSreg(&rcpu->ldtr, &bsreg);
	
	rcpu->tr.sregtype = SREG_TR;
	BX_CPU_THIS_PTR dbg_get_tr(&bsreg);
	LoadSreg(&rcpu->tr, &bsreg);

	rcpu->gdtr.sregtype = SREG_GDTR;
	BX_CPU_THIS_PTR dbg_get_gdtr(&bgsreg);
	rcpu->gdtr.base = GetMax32(bgsreg.base);
	rcpu->gdtr.limit = bgsreg.limit;

	rcpu->idtr.sregtype = SREG_IDTR;
	BX_CPU_THIS_PTR dbg_get_idtr(&bgsreg);
	rcpu->idtr.base = GetMax32(bgsreg.base);
	rcpu->idtr.limit = bgsreg.limit;
	
	rcpu->cr0 = BX_CPU_THIS_PTR cr0.get32();
	rcpu->cr2 = GetMax32(BX_CPU_THIS_PTR cr2);
	rcpu->cr3 = GetMax32(BX_CPU_THIS_PTR cr3);
}
static t_bool vcpuapiCheckDiff()
{
	t_nubitcc i;
	t_bool flagdiff = 0x00;
	t_nubit32 mask = vcpuins.udf;
	if (!vcpu.flagignore) {
		if (vcpu.cr0 != newbcpu.cr0) {vapiPrint("diff cr0\n");flagdiff = 0x01;}
		if (vcpu.cr2 != newbcpu.cr2) {vapiPrint("diff cr2\n");flagdiff = 0x01;}
		if (vcpu.cr3 != newbcpu.cr3) {vapiPrint("diff cr3\n");flagdiff = 0x01;}
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
			vcpu.es.dpl != newbcpu.es.dpl) {
				vapiPrint("diff es (V=%04X/%08X/%08X/%1X, B=%04X/%08X/%08X/%1X)\n",
					vcpu.es.selector, vcpu.es.base, vcpu.es.limit, vcpu.es.dpl,
					newbcpu.es.selector, newbcpu.es.base, newbcpu.es.limit, newbcpu.es.dpl);
				flagdiff = 0x01;
		}
		if (vcpu.cs.selector != newbcpu.cs.selector ||
			vcpu.cs.base != newbcpu.cs.base ||
			vcpu.cs.limit != newbcpu.cs.limit ||
			vcpu.cs.dpl != newbcpu.cs.dpl) {
				vapiPrint("diff cs (V=%04X/%08X/%08X/%1X, B=%04X/%08X/%08X/%1X)\n",
					vcpu.cs.selector, vcpu.cs.base, vcpu.cs.limit, vcpu.cs.dpl,
					newbcpu.cs.selector, newbcpu.cs.base, newbcpu.cs.limit, newbcpu.cs.dpl);
				flagdiff = 0x01;
		}
		if (vcpu.ss.selector != newbcpu.ss.selector ||
			vcpu.ss.base != newbcpu.ss.base ||
			vcpu.ss.limit != newbcpu.ss.limit ||
			vcpu.ss.dpl != newbcpu.ss.dpl) {
				vapiPrint("diff ss (V=%04X/%08X/%08X/%1X, B=%04X/%08X/%08X/%1X)\n",
					vcpu.ss.selector, vcpu.ss.base, vcpu.ss.limit, vcpu.ss.dpl,
					newbcpu.ss.selector, newbcpu.ss.base, newbcpu.ss.limit, newbcpu.ss.dpl);
				flagdiff = 0x01;
		}
		if (vcpu.ds.selector != newbcpu.ds.selector ||
			vcpu.ds.base != newbcpu.ds.base ||
			vcpu.ds.limit != newbcpu.ds.limit ||
			vcpu.ds.dpl != newbcpu.ds.dpl) {
				vapiPrint("diff ds (V=%04X/%08X/%08X/%1X, B=%04X/%08X/%08X/%1X)\n",
					vcpu.ds.selector, vcpu.ds.base, vcpu.ds.limit, vcpu.ds.dpl,
					newbcpu.ds.selector, newbcpu.ds.base, newbcpu.ds.limit, newbcpu.ds.dpl);
				flagdiff = 0x01;
		}
		if (vcpu.fs.selector != newbcpu.fs.selector ||
			vcpu.fs.base != newbcpu.fs.base ||
			vcpu.fs.limit != newbcpu.fs.limit ||
			vcpu.fs.dpl != newbcpu.fs.dpl) {
				vapiPrint("diff fs (V=%04X/%08X/%08X/%1X, B=%04X/%08X/%08X/%1X)\n",
					vcpu.fs.selector, vcpu.fs.base, vcpu.fs.limit, vcpu.fs.dpl,
					newbcpu.fs.selector, newbcpu.fs.base, newbcpu.fs.limit, newbcpu.fs.dpl);
				flagdiff = 0x01;
		}
		if (vcpu.gs.selector != newbcpu.gs.selector ||
			vcpu.gs.base != newbcpu.gs.base ||
			vcpu.gs.limit != newbcpu.gs.limit ||
			vcpu.gs.dpl != newbcpu.gs.dpl) {
				vapiPrint("diff gs (V=%04X/%08X/%08X/%1X, B=%04X/%08X/%08X/%1X)\n",
					vcpu.gs.selector, vcpu.gs.base, vcpu.gs.limit, vcpu.gs.dpl,
					newbcpu.gs.selector, newbcpu.gs.base, newbcpu.gs.limit, newbcpu.gs.dpl);
				flagdiff = 0x01;
		}
		if (vcpu.tr.selector != newbcpu.tr.selector ||
			vcpu.tr.base != newbcpu.tr.base ||
			vcpu.tr.limit != newbcpu.tr.limit ||
			vcpu.tr.dpl != newbcpu.tr.dpl) {
				vapiPrint("diff tr (V=%04X/%08X/%08X/%1X, B=%04X/%08X/%08X/%1X)\n",
					vcpu.tr.selector, vcpu.tr.base, vcpu.tr.limit, vcpu.tr.dpl,
					newbcpu.tr.selector, newbcpu.tr.base, newbcpu.tr.limit, newbcpu.tr.dpl);
				flagdiff = 0x01;
		}
		if (vcpu.ldtr.selector != newbcpu.ldtr.selector ||
			vcpu.ldtr.base != newbcpu.ldtr.base ||
			vcpu.ldtr.limit != newbcpu.ldtr.limit ||
			vcpu.ldtr.dpl != newbcpu.ldtr.dpl) {
				vapiPrint("diff ldtr (V=%04X/%08X/%08X/%1X, B=%04X/%08X/%08X/%1X)\n",
					vcpu.ldtr.selector, vcpu.ldtr.base, vcpu.ldtr.limit, vcpu.ldtr.dpl,
					newbcpu.ldtr.selector, newbcpu.ldtr.base, newbcpu.ldtr.limit, newbcpu.ldtr.dpl);
				flagdiff = 0x01;
		}
		if (vcpu.gdtr.base != newbcpu.gdtr.base ||
			vcpu.gdtr.limit != newbcpu.gdtr.limit) {
				vapiPrint("diff gdtr (V=%08X/%08X, B=%08X/%08X)\n",
					vcpu.gdtr.base, vcpu.gdtr.limit,
					newbcpu.gdtr.base, newbcpu.gdtr.limit);
				flagdiff = 0x01;
		}
		if (vcpu.idtr.base != newbcpu.idtr.base ||
			vcpu.idtr.limit != newbcpu.idtr.limit) {
				vapiPrint("diff idtr (V=%08X/%08X, B=%08X/%08X)\n",
					vcpu.idtr.base, vcpu.idtr.limit,
					newbcpu.idtr.base, newbcpu.idtr.limit);
				flagdiff = 0x01;
		}
		if ((vcpu.eflags & ~mask) != (newbcpu.eflags & ~mask)) {
			vapiPrint("diff flags: V=%08X, B=%08X\n", vcpu.eflags, newbcpu.eflags);
			flagdiff = 0x01;
		}
	}
	if (flagdiff) {
		vapiPrint("BEFORE EXECUTION:\n");
		vcpuapiPrintReg(&oldbcpu);
		vcpuapiPrintSreg(&oldbcpu);
		vcpuapiPrintCreg(&oldbcpu);
		vapiPrint("---------------------------------------------------\n");
		vapiPrint("AFTER EXECUTION:\n");
		vcpuapiPrintReg(&newbcpu);
		vcpuapiPrintSreg(&newbcpu);
		vcpuapiPrintCreg(&newbcpu);
		vapiPrint("---------------------------------------------------\n");
		vapiPrint("CURRENT VCPU:\n");
		vapiPrint("[E:L%08X]\n", vcpurec.linear);
		for (i = 0;i < vcpurec.msize;++i) {
			vapiPrint("[%c:L%08x/%1d/%08x]\n",
				vcpurec.mem[i].flagwrite ? 'W' : 'R', vcpurec.mem[i].linear,
				vcpurec.mem[i].byte, vcpurec.mem[i].data);
		}
		vcpuapiPrintReg(&vcpu);
		vcpuapiPrintSreg(&vcpu);
		vcpuapiPrintCreg(&vcpu);
		vapiPrint("---------------------------------------------------\n");
	}
	return flagdiff;
}

void vcpuapiInit()
{
	vcpuInit();
	oldbcpu = vcpu;
	newbcpu = vcpu;
}
void vcpuapiFinal()
{
	vcpuFinal();
}
void vcpuapiExecBefore()
{
	CopyBochsCpu(&oldbcpu);
	if (oldbcpu.cs.base + oldbcpu.eip == 0x1ae8d/*0x7c00*/) {
		flagvalid = 1;
		vapiPrint("NXVM and Bochs comparison starts from here.\n");
	}
	if (flagvalid) {
		vcpu = oldbcpu;
		vcpuinsRefresh();
	}
}
void vcpuapiExecAfter()
{
	if (flagvalid) {
		CopyBochsCpu(&newbcpu);
		if (vcpuapiCheckDiff()) BX_CPU_THIS_PTR magic_break = 1;
	}
}
static void PrintPhysical(t_nubit32 phy, t_nubit64 data, t_nubit8 byte, t_bool write)
{
	vapiPrint("%s phy=%08x, data=", write ? "write" : "read", phy);
	switch (byte) {
	case 1:
		vapiPrint("%02x", GetMax8(data));
		break;
	case 2:
		vapiPrint("%04x", GetMax16(data));
		break;
	case 3:
		vapiPrint("%08x", GetMax24(data));
		break;
	case 4:
		vapiPrint("%08x", GetMax32(data));
		break;
	case 6:
		vapiPrint("%016llx", GetMax48(data));
		break;
	case 8:
		vapiPrint("%016llx", GetMax64(data));
		break;
	default:
		vapiPrint("invalid");
		break;
	}
	vapiPrint(", byte=%01x\n", byte);
}
t_nubit64 vcpuapiReadPhysical(t_nubit32 phy, t_nubit8 byte)
{
	t_nubit64 data;
	BX_CPU_THIS_PTR access_read_physical(phy, byte, &data);
	//BX_MEM(0)->readPhysicalPage(BX_CPU(0), phy,byte, &data);
	if (0) PrintPhysical(phy, data, byte, 0);
	return data;
}
void vcpuapiWritePhysical(t_nubit32 phy, t_nubit64 data, t_nubit8 byte)
{
	//BX_CPU_THIS_PTR access_write_physical(phy, byte, &data);
	//BX_MEM(0)->writePhysicalPage(BX_CPU(0), phy, byte, &data);
	if (0) PrintPhysical(phy, data, byte, 1);
}

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
	vapiPrint("\nEIP=%08X", rcpu->eip);
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
	vapiPrint("GDTR Base=%08X, Limit=%04X; ",
		rcpu->gdtr.base, rcpu->gdtr.limit);
	vapiPrint("IDTR Base=%08X, Limit=%04X\n",
		rcpu->idtr.base, rcpu->idtr.limit);
}
void vcpuapiPrintCreg(t_cpu *rcpu)
{
	vapiPrint("CR0=%08X: %s %s %s %s %s %s; ", rcpu->cr0,
		GetBit(rcpu->cr0, VCPU_CR0_PG) ? "PG" : "pg",
		GetBit(rcpu->cr0, VCPU_CR0_ET) ? "ET" : "et",
		GetBit(rcpu->cr0, VCPU_CR0_TS) ? "TS" : "ts",
		GetBit(rcpu->cr0, VCPU_CR0_EM) ? "EM" : "em",
		GetBit(rcpu->cr0, VCPU_CR0_MP) ? "MP" : "mp",
		GetBit(rcpu->cr0, VCPU_CR0_PE) ? "PE" : "pe");
	vapiPrint("CR2=PFLR=%08X; ", rcpu->cr2);
	vapiPrint("CR3=PDBR=%08X\n", rcpu->cr3);
}

