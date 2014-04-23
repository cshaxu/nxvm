/* This file is a part of NXVM project. */

#include "memory.h"

#include "vglobal.h"

#ifndef VGLOBAL_BOCHS
#include "vapi.h"
#else
#include "vcpuapi.h"
#endif

#include "debug/dasm32.h"
#include "vcpuins.h"
#include "vcpu.h"

t_cpu vcpu;

static void print_sreg_seg(t_cpu_sreg *rsreg, const t_strptr label)
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
static void print_sreg_sys(t_cpu_sreg *rsreg, const t_strptr label)
{
	vapiPrint("%s=%04X, Base=%08X, Limit=%08X, DPL=%01X, Type=%04X\n", label,
		rsreg->selector, rsreg->base, rsreg->limit,
		rsreg->dpl, rsreg->sys.type);
}
void vapiCallBackCpuPrintIns()
{
	t_nubitcc i;
	t_nubit8 len;
	t_nubit8  ucode[15];
	t_string str, stmt, sbin;
	t_nubit32 linear = vcpu.cs.base + vcpu.eip;
	if (vcpuinsReadLinear(linear, (t_vaddrcc)ucode, 15)) {
		len = 0;
		SPRINTF(str, "L%08X <ERROR>", linear);
	} else {
		len = dasm32(stmt, (t_vaddrcc)ucode);
		sbin[0] = 0;
		for (i = 0;i < len;++i) SPRINTF(sbin, "%s%02X", sbin, GetMax8(ucode[i]));
		SPRINTF(str, "L%08X %s ", linear, sbin);
		for (i = strlen(str);i < 24;++i) STRCAT(str, " ");
		STRCAT(str, stmt);
	}
	vapiPrint("%s\n", str);
}
void vapiCallBackCpuPrintSreg()
{
	print_sreg_seg(&vcpu.es, "ES");
	print_sreg_seg(&vcpu.cs, "CS");
	print_sreg_seg(&vcpu.ss, "SS");
	print_sreg_seg(&vcpu.ds, "DS");
	print_sreg_seg(&vcpu.fs, "FS");
	print_sreg_seg(&vcpu.gs, "GS");
	print_sreg_sys(&vcpu.tr, "TR  ");
	print_sreg_sys(&vcpu.ldtr, "LDTR");
	vapiPrint("GDTR Base=%08X, Limit=%04X\n",
		_gdtr.base, _gdtr.limit);
	vapiPrint("IDTR Base=%08X, Limit=%04X\n",
		_idtr.base, _idtr.limit);
}
void vapiCallBackCpuPrintCreg()
{
	vapiPrint("CR0=%08X: %s %s %s %s %s %s\n", vcpu.cr0,
		_GetCR0_PG ? "PG" : "pg",
		_GetCR0_ET ? "ET" : "et",
		_GetCR0_TS ? "TS" : "ts",
		_GetCR0_EM ? "EM" : "em",
		_GetCR0_MP ? "MP" : "mp",
		_GetCR0_PE ? "PE" : "pe");
	vapiPrint("CR2=PFLR=%08X\n", vcpu.cr2);
	vapiPrint("CR3=PDBR=%08X\n", vcpu.cr3);
}
void vapiCallBackCpuPrintReg()
{
	vapiPrint( "EAX=%08X", vcpu.eax);
	vapiPrint(" EBX=%08X", vcpu.ebx);
	vapiPrint(" ECX=%08X", vcpu.ecx);
	vapiPrint(" EDX=%08X", vcpu.edx);
	vapiPrint("\nESP=%08X",vcpu.esp);
	vapiPrint(" EBP=%08X", vcpu.ebp);
	vapiPrint(" ESI=%08X", vcpu.esi);
	vapiPrint(" EDI=%08X", vcpu.edi);
	vapiPrint("\nEIP=%08X",vcpu.eip);
	vapiPrint(" EFL=%08X", vcpu.eflags);
	vapiPrint(": ");
	vapiPrint("%s ", _GetEFLAGS_VM ? "VM" : "vm");
	vapiPrint("%s ", _GetEFLAGS_RF ? "RF" : "rf");
	vapiPrint("%s ", _GetEFLAGS_NT ? "NT" : "nt");
	vapiPrint("IOPL=%01X ", _GetEFLAGS_IOPL);
	vapiPrint("%s ", _GetEFLAGS_OF ? "OF" : "of");
	vapiPrint("%s ", _GetEFLAGS_DF ? "DF" : "df");
	vapiPrint("%s ", _GetEFLAGS_IF ? "IF" : "if");
	vapiPrint("%s ", _GetEFLAGS_TF ? "TF" : "tf");
	vapiPrint("%s ", _GetEFLAGS_SF ? "SF" : "sf");
	vapiPrint("%s ", _GetEFLAGS_ZF ? "ZF" : "zf");
	vapiPrint("%s ", _GetEFLAGS_AF ? "AF" : "af");
	vapiPrint("%s ", _GetEFLAGS_PF ? "PF" : "pf");
	vapiPrint("%s ", _GetEFLAGS_CF ? "CF" : "cf");
	vapiPrint("\n");
	vapiCallBackCpuPrintIns();
}

void vcpuInit()
{
	vcpuinsInit();
}
void vcpuReset()
{
	memset(&vcpu, 0, sizeof(t_cpu));

	vcpu.eip = 0x0000fff0;
	vcpu.eflags = 0x00000002;

	vcpu.cs.base = 0xffff0000;
	vcpu.cs.dpl = 0x00;
	vcpu.cs.limit = 0xffffffff;
	vcpu.cs.seg.accessed = 1;
	vcpu.cs.seg.executable = 1;
	vcpu.cs.seg.exec.conform = 0;
	vcpu.cs.seg.exec.defsize = 0;
	vcpu.cs.seg.exec.readable = 1;
	vcpu.cs.selector = 0xf000;
	vcpu.cs.sregtype = SREG_CODE;
	vcpu.cs.flagvalid = 1;

	vcpu.ss.base = 0x00000000;
	vcpu.ss.dpl = 0x00;
	vcpu.ss.limit = 0x0000ffff;
	vcpu.ss.seg.accessed = 1;
	vcpu.ss.seg.executable = 0;
	vcpu.ss.seg.data.big = 0;
	vcpu.ss.seg.data.expdown = 0;
	vcpu.ss.seg.data.writable = 1;
	vcpu.ss.selector = 0x0000;
	vcpu.ss.sregtype = SREG_STACK;
	vcpu.ss.flagvalid = 1;

	vcpu.ds.base = 0x00000000;
	vcpu.ds.dpl = 0x00;
	vcpu.ds.limit = 0x0000ffff;
	vcpu.ds.seg.accessed = 1;
	vcpu.ss.seg.executable = 0;
	vcpu.ds.seg.data.big = 0;
	vcpu.ds.seg.data.expdown = 0;
	vcpu.ds.seg.data.writable = 1;
	vcpu.ds.selector = 0x0000;
	vcpu.ds.sregtype = SREG_DATA;
	vcpu.ds.flagvalid = 1;
	vcpu.gs = vcpu.fs = vcpu.es = vcpu.ds;

	vcpu.ldtr.base = 0x00000000;
	vcpu.ldtr.dpl = 0x00;
	vcpu.ldtr.limit = 0x0000ffff;
	vcpu.ldtr.selector = 0x0000;
	vcpu.ldtr.sregtype = SREG_LDTR;
	vcpu.ldtr.sys.type = VCPU_DESC_SYS_TYPE_LDT;
	vcpu.ldtr.flagvalid = 1;

	vcpu.tr.base = 0x00000000;
	vcpu.tr.dpl = 0x00;
	vcpu.tr.limit = 0x0000ffff;
	vcpu.tr.selector = 0x0000;
	vcpu.tr.sregtype = SREG_TR;
	vcpu.tr.sys.type = VCPU_DESC_SYS_TYPE_TSS_16_AVL;
	vcpu.tr.flagvalid = 1;

	vcpu.idtr.base = 0x00000000;
	vcpu.idtr.limit = 0x03ff;
	vcpu.idtr.sregtype = SREG_IDTR;
	vcpu.idtr.flagvalid = 1;

	vcpu.gdtr.base = 0x00000000;
	vcpu.gdtr.limit = 0xffff;
	vcpu.gdtr.sregtype = SREG_GDTR;
	vcpu.gdtr.flagvalid = 1;

	vcpuinsReset();
}
void vcpuRefresh()
{
	vcpuinsRefresh();
}
void vcpuFinal()
{
	vcpuinsFinal();
}
