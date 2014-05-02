/* Copyright 2012-2014 Neko. */

/* VCPU defines the Central Processing Unit. */

#include "../utils.h"

#include "vcpuins.h"

#include "vmachine.h"
#include "vcpu.h"

t_cpu vcpu;

static void init() {vcpuinsInit();}

static void reset() {
	MEMSET(&vcpu, Zero8, sizeof(t_cpu));

	vcpu.eip = 0x0000fff0;
	vcpu.eflags = 0x00000002;

	vcpu.cs.base = 0xffff0000;
	vcpu.cs.dpl = Zero4;
	vcpu.cs.limit = Max32;
	vcpu.cs.seg.accessed = True;
	vcpu.cs.seg.executable = True;
	vcpu.cs.seg.exec.conform = False;
	vcpu.cs.seg.exec.defsize = False;
	vcpu.cs.seg.exec.readable = True;
	vcpu.cs.selector = 0xf000;
	vcpu.cs.sregtype = SREG_CODE;
	vcpu.cs.flagvalid = True;

	vcpu.ss.base = Zero32;
	vcpu.ss.dpl = Zero4;
	vcpu.ss.limit = Max16;
	vcpu.ss.seg.accessed = True;
	vcpu.ss.seg.executable = False;
	vcpu.ss.seg.data.big = False;
	vcpu.ss.seg.data.expdown = False;
	vcpu.ss.seg.data.writable = True;
	vcpu.ss.selector = Zero16;
	vcpu.ss.sregtype = SREG_STACK;
	vcpu.ss.flagvalid = True;

	vcpu.ds.base = Zero32;
	vcpu.ds.dpl = Zero4;
	vcpu.ds.limit = Max16;
	vcpu.ds.seg.accessed = True;
	vcpu.ss.seg.executable = False;
	vcpu.ds.seg.data.big = False;
	vcpu.ds.seg.data.expdown = False;
	vcpu.ds.seg.data.writable = True;
	vcpu.ds.selector = Zero16;
	vcpu.ds.sregtype = SREG_DATA;
	vcpu.ds.flagvalid = True;
	vcpu.gs = vcpu.fs = vcpu.es = vcpu.ds;

	vcpu.ldtr.base = Zero32;
	vcpu.ldtr.dpl = Zero4;
	vcpu.ldtr.limit = Max16;
	vcpu.ldtr.selector = Zero16;
	vcpu.ldtr.sregtype = SREG_LDTR;
	vcpu.ldtr.sys.type = VCPU_DESC_SYS_TYPE_LDT;
	vcpu.ldtr.flagvalid = True;

	vcpu.tr.base = Zero32;
	vcpu.tr.dpl = Zero4;
	vcpu.tr.limit = Max16;
	vcpu.tr.selector = Zero16;
	vcpu.tr.sregtype = SREG_TR;
	vcpu.tr.sys.type = VCPU_DESC_SYS_TYPE_TSS_16_AVL;
	vcpu.tr.flagvalid = True;

	vcpu.idtr.base = Zero32;
	vcpu.idtr.limit = 0x03ff;
	vcpu.idtr.sregtype = SREG_IDTR;
	vcpu.idtr.flagvalid = True;

	vcpu.gdtr.base = Zero32;
	vcpu.gdtr.limit = Max16;
	vcpu.gdtr.sregtype = SREG_GDTR;
	vcpu.gdtr.flagvalid = True;

	vcpuinsReset();
}

static void refresh() {vcpuinsRefresh();}

static void final() {vcpuinsFinal();}

void vcpuRegister() {vmachineAddMe;}

/* Prints user segment registers (ES, CS, SS, DS, FS, GS) */
static void print_sreg_seg(t_cpu_sreg *rsreg, const t_strptr label) {
	PRINTF("%s=%04X, Base=%08X, Limit=%08X, DPL=%01X, %s, ", label,
		rsreg->selector, rsreg->base, rsreg->limit,
		rsreg->dpl, rsreg->seg.accessed ? "A" : "a");
	if (rsreg->seg.executable) {
		PRINTF("Code, %s, %s, %s\n",
			rsreg->seg.exec.conform ? "C" : "c",
			rsreg->seg.exec.readable ? "Rw" : "rw",
			rsreg->seg.exec.defsize ? "32" : "16");
	} else {
		PRINTF("Data, %s, %s, %s\n",
			rsreg->seg.data.expdown ? "E" : "e",
			rsreg->seg.data.writable ? "RW" : "Rw",
			rsreg->seg.data.big ? "BIG" : "big");
	} 
}

/* Prints system segment registers (TR, LDTR) */
static void print_sreg_sys(t_cpu_sreg *rsreg, const t_strptr label) {
	PRINTF("%s=%04X, Base=%08X, Limit=%08X, DPL=%01X, Type=%04X\n", label,
		rsreg->selector, rsreg->base, rsreg->limit,
		rsreg->dpl, rsreg->sys.type);
}

/* Prints segment registers */
void devicePrintCpuSreg() {
	print_sreg_seg(&vcpu.es, "ES");
	print_sreg_seg(&vcpu.cs, "CS");
	print_sreg_seg(&vcpu.ss, "SS");
	print_sreg_seg(&vcpu.ds, "DS");
	print_sreg_seg(&vcpu.fs, "FS");
	print_sreg_seg(&vcpu.gs, "GS");
	print_sreg_sys(&vcpu.tr, "TR  ");
	print_sreg_sys(&vcpu.ldtr, "LDTR");
	PRINTF("GDTR Base=%08X, Limit=%04X\n",
		_gdtr.base, _gdtr.limit);
	PRINTF("IDTR Base=%08X, Limit=%04X\n",
		_idtr.base, _idtr.limit);
}

/* Prints control registers */
void devicePrintCpuCreg() {
	PRINTF("CR0=%08X: %s %s %s %s %s %s\n", vcpu.cr0,
		_GetCR0_PG ? "PG" : "pg",
		_GetCR0_ET ? "ET" : "et",
		_GetCR0_TS ? "TS" : "ts",
		_GetCR0_EM ? "EM" : "em",
		_GetCR0_MP ? "MP" : "mp",
		_GetCR0_PE ? "PE" : "pe");
	PRINTF("CR2=PFLR=%08X\n", vcpu.cr2);
	PRINTF("CR3=PDBR=%08X\n", vcpu.cr3);
}

/* Prints regular registers */
void devicePrintCpuReg() {
	PRINTF( "EAX=%08X", vcpu.eax);
	PRINTF(" EBX=%08X", vcpu.ebx);
	PRINTF(" ECX=%08X", vcpu.ecx);
	PRINTF(" EDX=%08X", vcpu.edx);
	PRINTF("\nESP=%08X",vcpu.esp);
	PRINTF(" EBP=%08X", vcpu.ebp);
	PRINTF(" ESI=%08X", vcpu.esi);
	PRINTF(" EDI=%08X", vcpu.edi);
	PRINTF("\nEIP=%08X",vcpu.eip);
	PRINTF(" EFL=%08X", vcpu.eflags);
	PRINTF(": ");
	PRINTF("%s ", _GetEFLAGS_VM ? "VM" : "vm");
	PRINTF("%s ", _GetEFLAGS_RF ? "RF" : "rf");
	PRINTF("%s ", _GetEFLAGS_NT ? "NT" : "nt");
	PRINTF("IOPL=%01X ", _GetEFLAGS_IOPL);
	PRINTF("%s ", _GetEFLAGS_OF ? "OF" : "of");
	PRINTF("%s ", _GetEFLAGS_DF ? "DF" : "df");
	PRINTF("%s ", _GetEFLAGS_IF ? "IF" : "if");
	PRINTF("%s ", _GetEFLAGS_TF ? "TF" : "tf");
	PRINTF("%s ", _GetEFLAGS_SF ? "SF" : "sf");
	PRINTF("%s ", _GetEFLAGS_ZF ? "ZF" : "zf");
	PRINTF("%s ", _GetEFLAGS_AF ? "AF" : "af");
	PRINTF("%s ", _GetEFLAGS_PF ? "PF" : "pf");
	PRINTF("%s ", _GetEFLAGS_CF ? "CF" : "cf");
	PRINTF("\n");
}

/* Prints active memory info */
void devicePrintCpuMem() {
	t_nsbitcc i;
	for (i = 0;i < vcpu.msize;++i) {
		PRINTF("%s: Lin=%08x, Data=%08x, Bytes=%1x\n",
			vcpu.mem[i].flagwrite ? "Write" : "Read",
			vcpu.mem[i].linear, vcpu.mem[i].data, vcpu.mem[i].byte);
	}
}
