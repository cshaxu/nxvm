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

int deviceConnectCpuReadLinear(uint32_t linear, void *rdest, uint8_t size) {
    return vcpuinsReadLinear(linear, (t_vaddrcc) rdest, size);
}
int deviceConnectCpuWriteLinear(uint32_t linear, void *rsrc, uint8_t size) {
    return vcpuinsWriteLinear(linear, (t_vaddrcc) rsrc, size);
}
int deviceConnectCpuLoadES(uint16_t selector) {
    return vcpuinsLoadSreg(&vcpu.data.es, selector);
}
int deviceConnectCpuLoadCS(uint16_t selector) {
    return vcpuinsLoadSreg(&vcpu.data.cs, selector);
}
int deviceConnectCpuLoadSS(uint16_t selector) {
    return vcpuinsLoadSreg(&vcpu.data.ss, selector);
}
int deviceConnectCpuLoadDS(uint16_t selector) {
    return vcpuinsLoadSreg(&vcpu.data.ds, selector);
}
int deviceConnectCpuLoadFS(uint16_t selector) {
    return vcpuinsLoadSreg(&vcpu.data.fs, selector);
}
int deviceConnectCpuLoadGS(uint16_t selector) {
    return vcpuinsLoadSreg(&vcpu.data.gs, selector);
}
void deviceConnectCpuSetWR(uint32_t linear) {
    vcpuins.data.wrLinear = linear;
    vcpuins.data.flagWR = True;
}
void deviceConnectCpuSetWW(uint32_t linear) {
    vcpuins.data.wwLinear = linear;
    vcpuins.data.flagWW = True;
}
void deviceConnectCpuSetWE(uint32_t linear) {
    vcpuins.data.weLinear = linear;
    vcpuins.data.flagWE = True;
}
void deviceConnectCpuClearWR() {
    vcpuins.data.flagWR = False;
}
void deviceConnectCpuClearWW() {
    vcpuins.data.flagWW = False;
}
void deviceConnectCpuClearWE() {
    vcpuins.data.flagWE = False;
}

int deviceConnectCpuGetCsDefSize() {
    return vcpu.data.cs.seg.exec.defsize;
}
uint32_t deviceConnectCpuGetCsBase() {
    return vcpu.data.cs.base;
}

int deviceConnectCpuGetCF() {
    return _GetEFLAGS_CF;
}
int deviceConnectCpuGetPF() {
    return _GetEFLAGS_PF;
}
int deviceConnectCpuGetAF() {
    return _GetEFLAGS_AF;
}
int deviceConnectCpuGetZF() {
    return _GetEFLAGS_ZF;
}
int deviceConnectCpuGetSF() {
    return _GetEFLAGS_SF;
}
int deviceConnectCpuGetTF() {
    return _GetEFLAGS_TF;
}
int deviceConnectCpuGetIF() {
    return _GetEFLAGS_IF;
}
int deviceConnectCpuGetDF() {
    return _GetEFLAGS_DF;
}
int deviceConnectCpuGetOF() {
    return _GetEFLAGS_OF;
}
int deviceConnectCpuGetNT() {
    return _GetEFLAGS_NT;
}
int deviceConnectCpuGetRF() {
    return _GetEFLAGS_RF;
}
int deviceConnectCpuGetVM() {
    return _GetEFLAGS_VM;
}

void deviceConnectCpuSetCF() {
    _SetEFLAGS_CF;
}
void deviceConnectCpuSetPF() {
    _SetEFLAGS_PF;
}
void deviceConnectCpuSetAF() {
    _SetEFLAGS_AF;
}
void deviceConnectCpuSetZF() {
    _SetEFLAGS_ZF;
}
void deviceConnectCpuSetSF() {
    _SetEFLAGS_SF;
}
void deviceConnectCpuSetTF() {
    _SetEFLAGS_TF;
}
void deviceConnectCpuSetIF() {
    _SetEFLAGS_IF;
}
void deviceConnectCpuSetDF() {
    _SetEFLAGS_DF;
}
void deviceConnectCpuSetOF() {
    _SetEFLAGS_OF;
}
void deviceConnectCpuSetNT() {
    _SetEFLAGS_NT;
}
void deviceConnectCpuSetRF() {
    _SetEFLAGS_RF;
}
void deviceConnectCpuSetVM() {
    _SetEFLAGS_VM;
}

void deviceConnectCpuClearCF() {
    _ClrEFLAGS_CF;
}
void deviceConnectCpuClearPF() {
    _ClrEFLAGS_PF;
}
void deviceConnectCpuClearAF() {
    _ClrEFLAGS_AF;
}
void deviceConnectCpuClearZF() {
    _ClrEFLAGS_ZF;
}
void deviceConnectCpuClearSF() {
    _ClrEFLAGS_SF;
}
void deviceConnectCpuClearTF() {
    _ClrEFLAGS_TF;
}
void deviceConnectCpuClearIF() {
    _ClrEFLAGS_IF;
}
void deviceConnectCpuClearDF() {
    _ClrEFLAGS_DF;
}
void deviceConnectCpuClearOF() {
    _ClrEFLAGS_OF;
}
void deviceConnectCpuClearNT() {
    _ClrEFLAGS_NT;
}
void deviceConnectCpuClearRF() {
    _ClrEFLAGS_RF;
}
void deviceConnectCpuClearVM() {
    _ClrEFLAGS_VM;
}

void *deviceConnectCpuGetRefEAX() {
    return (void *)(&vcpu.data.eax);
}
void *deviceConnectCpuGetRefECX() {
    return (void *)(&vcpu.data.ecx);
}
void *deviceConnectCpuGetRefEDX() {
    return (void *)(&vcpu.data.edx);
}
void *deviceConnectCpuGetRefEBX() {
    return (void *)(&vcpu.data.ebx);
}
void *deviceConnectCpuGetRefESP() {
    return (void *)(&vcpu.data.esp);
}
void *deviceConnectCpuGetRefEBP() {
    return (void *)(&vcpu.data.ebp);
}
void *deviceConnectCpuGetRefESI() {
    return (void *)(&vcpu.data.esi);
}
void *deviceConnectCpuGetRefEDI() {
    return (void *)(&vcpu.data.edi);
}
void *deviceConnectCpuGetRefEFLAGS() {
    return (void *)(&vcpu.data.eflags);
}
void *deviceConnectCpuGetRefEIP() {
    return (void *)(&vcpu.data.eip);
}
void *deviceConnectCpuGetRefCR(t_nubit8 crId) {
    switch (crId) {
    case 0:
        return (void *)(&vcpu.data.cr0);
    case 1:
        return (void *)(&vcpu.data.cr1);
    case 2:
        return (void *)(&vcpu.data.cr2);
    case 3:
        return (void *)(&vcpu.data.cr3);
    }
    return (void *) NULL;
}

void *deviceConnectCpuGetRefAX() {
    return (void *)(&vcpu.data.ax);
}
void *deviceConnectCpuGetRefCX() {
    return (void *)(&vcpu.data.cx);
}
void *deviceConnectCpuGetRefDX() {
    return (void *)(&vcpu.data.dx);
}
void *deviceConnectCpuGetRefBX() {
    return (void *)(&vcpu.data.bx);
}
void *deviceConnectCpuGetRefSP() {
    return (void *)(&vcpu.data.sp);
}
void *deviceConnectCpuGetRefBP() {
    return (void *)(&vcpu.data.bp);
}
void *deviceConnectCpuGetRefSI() {
    return (void *)(&vcpu.data.si);
}
void *deviceConnectCpuGetRefDI() {
    return (void *)(&vcpu.data.di);
}
void *deviceConnectCpuGetRefFLAGS() {
    return (void *)(&vcpu.data.flags);
}
void *deviceConnectCpuGetRefIP() {
    return (void *)(&vcpu.data.ip);
}

void *deviceConnectCpuGetRefAH() {
    return (void *)(&vcpu.data.ah);
}
void *deviceConnectCpuGetRefCH() {
    return (void *)(&vcpu.data.ch);
}
void *deviceConnectCpuGetRefDH() {
    return (void *)(&vcpu.data.dh);
}
void *deviceConnectCpuGetRefBH() {
    return (void *)(&vcpu.data.bh);
}
void *deviceConnectCpuGetRefAL() {
    return (void *)(&vcpu.data.al);
}
void *deviceConnectCpuGetRefCL() {
    return (void *)(&vcpu.data.cl);
}
void *deviceConnectCpuGetRefDL() {
    return (void *)(&vcpu.data.dl);
}
void *deviceConnectCpuGetRefBL() {
    return (void *)(&vcpu.data.bl);
}

void *deviceConnectCpuGetRefES() {
    return (void *)(&vcpu.data.es.selector);
}
void *deviceConnectCpuGetRefCS() {
    return (void *)(&vcpu.data.cs.selector);
}
void *deviceConnectCpuGetRefSS() {
    return (void *)(&vcpu.data.ss.selector);
}
void *deviceConnectCpuGetRefDS() {
    return (void *)(&vcpu.data.ds.selector);
}
void *deviceConnectCpuGetRefFS() {
    return (void *)(&vcpu.data.fs.selector);
}
void *deviceConnectCpuGetRefGS() {
    return (void *)(&vcpu.data.gs.selector);
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
           vcpu.data.gdtr.base, vcpu.data.gdtr.limit);
    PRINTF("IDTR Base=%08X, Limit=%04X\n",
           vcpu.data.idtr.base, vcpu.data.idtr.limit);
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
    t_nubitcc i;
    for (i = 0; i < vcpuins.data.msize; ++i) {
        PRINTF("%s: Lin=%08x, Data=%08x, Bytes=%1x\n",
               vcpuins.data.mem[i].flagWrite ? "Write" : "Read",
               vcpuins.data.mem[i].linear, vcpuins.data.mem[i].data, vcpuins.data.mem[i].byte);
    }
}
void devicePrintCpuWatch() {
    if (vcpuins.data.flagWR) {
        PRINTF("Watch-read point: Lin=%08x\n", vcpuins.data.wrLinear);
    }
    if (vcpuins.data.flagWW) {
        PRINTF("Watch-write point: Lin=%08x\n", vcpuins.data.wwLinear);
    }
    if (vcpuins.data.flagWE) {
        PRINTF("Watch-exec point: Lin=%08x\n", vcpuins.data.weLinear);
    }
}
