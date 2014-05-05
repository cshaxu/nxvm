/* Copyright 2012-2014 Neko. */

/* VCPU defines the Central Processing Unit. */

#include "../utils.h"

#include "vcpuins.h"

#include "vcpu.h"

t_cpu vcpu;

void vcpuInit() {
    vcpuinsInit();
}

void vcpuReset() {
    MEMSET((void *)(&vcpu), Zero8, sizeof(t_cpu));

    vcpu.data.eip = 0x0000fff0;
    vcpu.data.eflags = 0x00000002;

    vcpu.data.cs.base = 0xffff0000;
    vcpu.data.cs.dpl = Zero4;
    vcpu.data.cs.limit = Max32;
    vcpu.data.cs.seg.accessed = True;
    vcpu.data.cs.seg.executable = True;
    vcpu.data.cs.seg.exec.conform = False;
    vcpu.data.cs.seg.exec.defsize = False;
    vcpu.data.cs.seg.exec.readable = True;
    vcpu.data.cs.selector = 0xf000;
    vcpu.data.cs.sregtype = SREG_CODE;
    vcpu.data.cs.flagValid = True;

    vcpu.data.ss.base = Zero32;
    vcpu.data.ss.dpl = Zero4;
    vcpu.data.ss.limit = Max16;
    vcpu.data.ss.seg.accessed = True;
    vcpu.data.ss.seg.executable = False;
    vcpu.data.ss.seg.data.big = False;
    vcpu.data.ss.seg.data.expdown = False;
    vcpu.data.ss.seg.data.writable = True;
    vcpu.data.ss.selector = Zero16;
    vcpu.data.ss.sregtype = SREG_STACK;
    vcpu.data.ss.flagValid = True;

    vcpu.data.ds.base = Zero32;
    vcpu.data.ds.dpl = Zero4;
    vcpu.data.ds.limit = Max16;
    vcpu.data.ds.seg.accessed = True;
    vcpu.data.ss.seg.executable = False;
    vcpu.data.ds.seg.data.big = False;
    vcpu.data.ds.seg.data.expdown = False;
    vcpu.data.ds.seg.data.writable = True;
    vcpu.data.ds.selector = Zero16;
    vcpu.data.ds.sregtype = SREG_DATA;
    vcpu.data.ds.flagValid = True;
    vcpu.data.gs = vcpu.data.fs = vcpu.data.es = vcpu.data.ds;

    vcpu.data.ldtr.base = Zero32;
    vcpu.data.ldtr.dpl = Zero4;
    vcpu.data.ldtr.limit = Max16;
    vcpu.data.ldtr.selector = Zero16;
    vcpu.data.ldtr.sregtype = SREG_LDTR;
    vcpu.data.ldtr.sys.type = VCPU_DESC_SYS_TYPE_LDT;
    vcpu.data.ldtr.flagValid = True;

    vcpu.data.tr.base = Zero32;
    vcpu.data.tr.dpl = Zero4;
    vcpu.data.tr.limit = Max16;
    vcpu.data.tr.selector = Zero16;
    vcpu.data.tr.sregtype = SREG_TR;
    vcpu.data.tr.sys.type = VCPU_DESC_SYS_TYPE_TSS_16_AVL;
    vcpu.data.tr.flagValid = True;

    vcpu.data.idtr.base = Zero32;
    vcpu.data.idtr.limit = 0x03ff;
    vcpu.data.idtr.sregtype = SREG_IDTR;
    vcpu.data.idtr.flagValid = True;

    vcpu.data.gdtr.base = Zero32;
    vcpu.data.gdtr.limit = Max16;
    vcpu.data.gdtr.sregtype = SREG_GDTR;
    vcpu.data.gdtr.flagValid = True;

    vcpuinsReset();
}

void vcpuRefresh() {
    vcpuinsRefresh();
}

void vcpuFinal() {
    vcpuinsFinal();
}

/* Prints user segment registers (ES, CS, SS, DS, FS, GS) */
static void print_sreg_seg(t_cpu_data_sreg *rsreg, const t_strptr label) {
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
static void print_sreg_sys(t_cpu_data_sreg *rsreg, const t_strptr label) {
    PRINTF("%s=%04X, Base=%08X, Limit=%08X, DPL=%01X, Type=%04X\n", label,
           rsreg->selector, rsreg->base, rsreg->limit,
           rsreg->dpl, rsreg->sys.type);
}

/* Prints segment registers */
void devicePrintCpuSreg() {
    print_sreg_seg(&vcpu.data.es, "ES");
    print_sreg_seg(&vcpu.data.cs, "CS");
    print_sreg_seg(&vcpu.data.ss, "SS");
    print_sreg_seg(&vcpu.data.ds, "DS");
    print_sreg_seg(&vcpu.data.fs, "FS");
    print_sreg_seg(&vcpu.data.gs, "GS");
    print_sreg_sys(&vcpu.data.tr, "TR  ");
    print_sreg_sys(&vcpu.data.ldtr, "LDTR");
    PRINTF("GDTR Base=%08X, Limit=%04X\n",
           _gdtr.base, _gdtr.limit);
    PRINTF("IDTR Base=%08X, Limit=%04X\n",
           _idtr.base, _idtr.limit);
}

/* Prints control registers */
void devicePrintCpuCreg() {
    PRINTF("CR0=%08X: %s %s %s %s %s %s\n", vcpu.data.cr0,
           _GetCR0_PG ? "PG" : "pg",
           _GetCR0_ET ? "ET" : "et",
           _GetCR0_TS ? "TS" : "ts",
           _GetCR0_EM ? "EM" : "em",
           _GetCR0_MP ? "MP" : "mp",
           _GetCR0_PE ? "PE" : "pe");
    PRINTF("CR2=PFLR=%08X\n", vcpu.data.cr2);
    PRINTF("CR3=PDBR=%08X\n", vcpu.data.cr3);
}

/* Prints regular registers */
void devicePrintCpuReg() {
    PRINTF( "EAX=%08X", vcpu.data.eax);
    PRINTF(" EBX=%08X", vcpu.data.ebx);
    PRINTF(" ECX=%08X", vcpu.data.ecx);
    PRINTF(" EDX=%08X", vcpu.data.edx);
    PRINTF("\nESP=%08X",vcpu.data.esp);
    PRINTF(" EBP=%08X", vcpu.data.ebp);
    PRINTF(" ESI=%08X", vcpu.data.esi);
    PRINTF(" EDI=%08X", vcpu.data.edi);
    PRINTF("\nEIP=%08X",vcpu.data.eip);
    PRINTF(" EFL=%08X", vcpu.data.eflags);
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
    for (i = 0; i < vcpuins.data.msize; ++i) {
        PRINTF("%s: Lin=%08x, Data=%08x, Bytes=%1x\n",
               vcpuins.data.mem[i].flagWrite ? "Write" : "Read",
               vcpuins.data.mem[i].linear, vcpuins.data.mem[i].data, vcpuins.data.mem[i].byte);
    }
}
