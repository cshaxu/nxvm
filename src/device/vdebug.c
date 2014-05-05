/* Copyright 2012-2014 Neko. */

/*
 * VDEBUG provides hardware debug device which collects
 * device status and stops device thread at breakpoints.
 */

#include "../utils.h"
#include "device.h"

#include "vcpuins.h"

#include "vmachine.h"
#include "vdebug.h"

t_debug vdebug;

static void xasmTest() {
    static t_nubitcc total = 0; /* total number of instructions tested */
    t_bool flagStop = True; /* stop vmachine if comparison fails */
    t_nubit8 i, lenDasm1, lenDasm2, lenAasm;
    t_string strDasm1, strDasm2;
    t_nubit8 ins1[15], ins2[15];
    total++;
    vcpuinsReadLinear(vcpu.cs.base + vcpu.eip, (t_vaddrcc) ins1, 15);
    /* ins1[0] = 0x67;
    ins1[1] = 0xc6;
    ins1[2] = 0x44;
    ins1[3] = 0xf2;
    ins1[4] = 0x05;
    ins1[5] = 0x8e;
    ins1[6] = 0x00;*/
    switch (d_nubit8(ins1)) {
    case 0x88:
    case 0x89:
    case 0x8a:
    case 0x8b:
    case 0x00:
    case 0x01:
    case 0x02:
    case 0x03:
    case 0x08:
    case 0x09:
    case 0x0a:
    case 0x0b:
    case 0x10:
    case 0x11:
    case 0x12:
    case 0x13:
    case 0x18:
    case 0x19:
    case 0x1a:
    case 0x1b:
    case 0x20:
    case 0x21:
    case 0x22:
    case 0x23:
    case 0x28:
    case 0x29:
    case 0x2a:
    case 0x2b:
    case 0x30:
    case 0x31:
    case 0x32:
    case 0x33:
    case 0x38:
    case 0x39:
    case 0x3a:
    case 0x3b:
        flagStop = True;
        break;
    }
    switch (d_nubit8(ins1+1)) {
    case 0x90:
        flagStop = True;
        break;
    }
    switch (d_nubit16(ins1)) {
    case 0x2e66:
        flagStop = True;
        break;
    }
    switch (GetMax24(d_nubit24(ins1))) {
    case 0xb70f66:
        flagStop = True;
        break;
    }
    switch (GetMax24(d_nubit24(ins1+1))) {
    case 0xb70f66:
        flagStop = True;
        break;
    }
    switch (d_nubit32(ins1)) {
    }
    lenDasm1 = utilsDasm32(strDasm1, ins1, vcpu.cs.seg.exec.defsize);
    lenAasm  = utilsAasm32(strDasm1, ins2, vcpu.cs.seg.exec.defsize);
    lenDasm2 = utilsDasm32(strDasm2, ins2, vcpu.cs.seg.exec.defsize);
    if ((!flagStop && (lenAasm != lenDasm1 || lenAasm != lenDasm2 || lenDasm1 != lenDasm2 ||
                       MEMCMP(ins1, ins2, lenDasm1))) || STRCMP(strDasm1, strDasm2)) {
        PRINTF("diff at #%d %04X:%08X(L%08X), len(a=%x,d1=%x,d2=%x), CodeSegDefSize=%d\n",
               total, _cs, _eip, vcpu.cs.base + vcpu.eip, lenAasm, lenDasm1, lenDasm2, vcpu.cs.seg.exec.defsize ? 32 : 16);
        for (i = 0; i < lenDasm1; ++i) {
            PRINTF("%02X", ins1[i]);
        }
        PRINTF("\t%s\n", strDasm1);
        for (i = 0; i < lenDasm2; ++i) {
            PRINTF("%02X", ins2[i]);
        }
        PRINTF("\t%s\n", strDasm2);
        deviceStop();
    }
}

static void init() {
    MEMSET(&vdebug, Zero8, sizeof(t_debug));
}

static void reset() {}

#define _expression "%scs:eip=%04x:%08x(L%08x) ss:esp=%04x:%08x(L%08x) \
eax=%08x ecx=%08x edx=%08x ebx=%08x ebp=%08x esi=%08x edi=%08x ds=%04x es=%04x fs=%04x gs=%04x \
eflags=%08x %s %s %s %s %s %s %s %s %s %s %s %s | cs:eip=%04x:%08x(L%08x)"

static void refresh() {
    vdebug.breakCount++;
    if (vdebug.breakCount && (
                (vdebug.flagBreak && _cs == vdebug.breakCS && _ip == vdebug.breakIP) ||
                (vdebug.flagBreak32 && (vcpu.cs.base + vcpu.eip == vdebug.breakLinear)))) {
        deviceStop();
    }
    if (vdebug.traceCount) {
        vdebug.traceCount--;
        if (!vdebug.traceCount) {
            deviceStop();
        }
    }
    /* TODO: xasmTest(); */
    /* dump cpu status before execution */
    if (vdebug.recordFile) {
        int i;
        FPRINTF(vdebug.recordFile, _expression,
                vcpu.svcextl ? "* " : "",
                _cs, _eip, vcpu.cs.base + _eip,
                _ss, _esp, vcpu.ss.base + _esp,
                _eax, _ecx, _edx, _ebx, _ebp, _esi, _edi,
                _ds, _es, _fs, _gs, _eflags,
                _GetEFLAGS_OF ? "OF" : "of",
                _GetEFLAGS_SF ? "SF" : "sf",
                _GetEFLAGS_ZF ? "ZF" : "zf",
                _GetEFLAGS_CF ? "CF" : "cf",
                _GetEFLAGS_AF ? "AF" : "af",
                _GetEFLAGS_PF ? "PF" : "pf",
                _GetEFLAGS_DF ? "DF" : "df",
                _GetEFLAGS_IF ? "IF" : "if",
                _GetEFLAGS_TF ? "TF" : "tf",
                _GetEFLAGS_VM ? "VM" : "vm",
                _GetEFLAGS_RF ? "RF" : "rf",
                _GetEFLAGS_NT ? "NT" : "nt",
                vcpu.reccs, vcpu.receip, vcpu.linear);

        /* disassemble opcode */
        if (vcpu.oplen) {
            vcpu.oplen = utilsDasm32(vcpu.stmt, vcpu.opcodes, vcpu.cs.seg.exec.defsize);
            for (i = 0; i < strlen(vcpu.stmt); ++i) {
                if (vcpu.stmt[i] == '\n') {
                    vcpu.stmt[i] = ' ';
                }
            }
        } else {
            SPRINTF(vcpu.stmt, "<ERROR>");
        }

        /* print opcode, at least print 8 bytes */
        for (i = 0; i < vcpu.oplen; ++i) {
            FPRINTF(vdebug.recordFile, "%02X", vcpu.opcodes[i]);
        }
        for (i = vcpu.oplen; i < 8; ++i) {
            FPRINTF(vdebug.recordFile, "  ");
        }

        /* print assembly, at least 40 char in length */
        FPRINTF(vdebug.recordFile, "%s ", vcpu.stmt);
        for (i = (int) strlen(vcpu.stmt); i < 40; ++i) {
            FPRINTF(vdebug.recordFile, " ");
        }

        /* print memory usage */
        for (i = 0; i < vcpu.msize; ++i) {
            FPRINTF(vdebug.recordFile, "[%c:L%08x/%1d/%016llx] ",
                    vcpu.mem[i].flagwrite ? 'W' : 'R', vcpu.mem[i].linear,
                    vcpu.mem[i].byte, vcpu.mem[i].data);
        }

        FPRINTF(vdebug.recordFile, "\n");
    }
}

static void final() {}

void vdebugRegister() {
    vmachineAddMe;
}

void devicePrintDebug() {
    PRINTF("Recorder:    %s\n", vdebug.recordFile ? "On" : "Off");
    PRINTF("Trace:       %s\n", vdebug.traceCount ? "On" : "Off");
    PRINTF("Break Point: ");
    if (vdebug.flagBreak) {
        PRINTF("%04X:%04X\n", vdebug.breakCS,vdebug.breakIP);
    } else if (vdebug.flagBreak32) {
        PRINTF("L%08X\n", vdebug.breakLinear);
    } else {
        PRINTF("Off\n");
    }
}
