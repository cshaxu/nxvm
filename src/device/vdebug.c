/* Copyright 2012-2014 Neko. */

/*
 * VDEBUG provides hardware debug device which collects
 * device status and stops device thread at breakpoints.
 */

#include "../utils.h"
#include "device.h"

#include "vcpuins.h"

#include "vdebug.h"

t_debug vdebug;

static void xasmTest() {
    static t_nubitcc total = 0; /* total number of instructions tested */
    t_bool flagStop = True; /* stop vmachine if comparison fails */
    t_nubitcc i, lenDasm1, lenDasm2, lenAasm;
    t_string strDasm1, strDasm2;
    t_nubit8 ins1[15], ins2[15];
    total++;
    vcpuinsReadLinear(vcpu.data.cs.base + vcpu.data.eip, (t_vaddrcc) ins1, 15);
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
        flagStop = False;
        break;
    }
    switch (d_nubit8(ins1+1)) {
    case 0x90:
        flagStop = False;
        break;
    }
    switch (d_nubit16(ins1)) {
    case 0x2e66:
        flagStop = False;
        break;
    }
    switch (GetMax24(d_nubit24(ins1))) {
    case 0xb70f66:
        flagStop = False;
        break;
    }
    switch (GetMax24(d_nubit24(ins1+1))) {
    case 0xb70f66:
        flagStop = False;
        break;
    }
    switch (d_nubit32(ins1)) {
    }
    lenDasm1 = utilsDasm32(strDasm1, ins1, vcpu.data.cs.seg.exec.defsize);
    lenAasm  = utilsAasm32(strDasm1, ins2, vcpu.data.cs.seg.exec.defsize);
    lenDasm2 = utilsDasm32(strDasm2, ins2, vcpu.data.cs.seg.exec.defsize);
    if ((flagStop && (lenAasm != lenDasm1 || lenAasm != lenDasm2 || lenDasm1 != lenDasm2 ||
                      MEMCMP((void *) ins1, (void *) ins2, lenDasm1))) || STRCMP(strDasm1, strDasm2)) {
        PRINTF("diff at #%d %04X:%08X(L%08X), len(a=%x,d1=%x,d2=%x), CodeSegDefSize=%d\n",
               total, vcpu.data.cs.selector, vcpu.data.eip, vcpu.data.cs.base + vcpu.data.eip,
               lenAasm, lenDasm1, lenDasm2, vcpu.data.cs.seg.exec.defsize ? 32 : 16);
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

void vdebugInit() {
    MEMSET((void *)(&vdebug), Zero8, sizeof(t_debug));
}
void vdebugReset() {
    MEMSET((void *)(&vdebug.data), Zero8, sizeof(t_debug_data));
}
#define _expression "cs:eip=%04x:%08x(L%08x) ss:esp=%04x:%08x(L%08x) \
eax=%08x ecx=%08x edx=%08x ebx=%08x ebp=%08x esi=%08x edi=%08x ds=%04x es=%04x fs=%04x gs=%04x \
eflags=%08x %s %s %s %s %s %s %s %s %s %s %s %s | cs:eip=%04x:%08x(L%08x)"
void vdebugRefresh() {
    if ((vdebug.data.flagBreak && vcpu.data.cs.selector == vdebug.data.breakCS && vcpu.data.ip == vdebug.data.breakIP) ||
            (vdebug.data.flagBreak32 && (vcpu.data.cs.base + vcpu.data.eip == vdebug.data.breakLinear))) {
        if (vdebug.data.breakCount) {
            deviceStop();
        }
    }
    vdebug.data.breakCount++;
    if (vdebug.data.flagTrace) {
        if (!vdebug.data.traceCount) {
            vdebug.data.flagTrace = False;
            deviceStop();
        } else {
            vdebug.data.traceCount--;
        }
    }
    /* TODO: xasmTest(); */
    /* dump cpu status before execution */
    if (vdebug.connect.recordFile) {
        t_nubitcc i;
        t_string stmt;
        FPRINTF(vdebug.connect.recordFile, _expression,
                vcpu.data.cs.selector, vcpu.data.eip, vcpu.data.cs.base + vcpu.data.eip,
                vcpu.data.ss.selector, vcpu.data.esp, vcpu.data.ss.base + vcpu.data.esp,
                vcpu.data.eax, vcpu.data.ecx, vcpu.data.edx, vcpu.data.ebx,
                vcpu.data.ebp, vcpu.data.esi, vcpu.data.edi,
                vcpu.data.ds.selector, vcpu.data.es,
                vcpu.data.fs.selector, vcpu.data.gs.selector,
                vcpu.data.eflags,
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
                vcpuins.data.reccs, vcpuins.data.receip, vcpuins.data.linear);

        /* disassemble opcode */
        if (vcpuins.data.oplen) {
            vcpuins.data.oplen = utilsDasm32(stmt, vcpuins.data.opcodes, vcpu.data.cs.seg.exec.defsize);
            for (i = 0; i < strlen(stmt); ++i) {
                if (stmt[i] == '\n') {
                    stmt[i] = ' ';
                }
            }
        } else {
            SPRINTF(stmt, "<ERROR>");
        }

        /* print opcode, at least print 8 bytes */
        for (i = 0; i < vcpuins.data.oplen; ++i) {
            FPRINTF(vdebug.connect.recordFile, "%02X", vcpuins.data.opcodes[i]);
        }
        for (i = vcpuins.data.oplen; i < 8; ++i) {
            FPRINTF(vdebug.connect.recordFile, "  ");
        }

        /* print assembly, at least 40 char in length */
        FPRINTF(vdebug.connect.recordFile, "%s ", stmt);
        for (i = strlen(stmt); i < 40; ++i) {
            FPRINTF(vdebug.connect.recordFile, " ");
        }

        /* print memory usage */
        for (i = 0; i < vcpuins.data.msize; ++i) {
            FPRINTF(vdebug.connect.recordFile, "[%c:L%08x/%1d/%016llx] ",
                    vcpuins.data.mem[i].flagWrite ? 'W' : 'R', vcpuins.data.mem[i].linear,
                    vcpuins.data.mem[i].byte, vcpuins.data.mem[i].data);
        }

        FPRINTF(vdebug.connect.recordFile, "\n");
    }
}
void vdebugFinal() {}

void deviceConnectDebugSetBreak(uint16_t breakCS, uint16_t breakIP) {
    vdebug.data.breakCS = breakCS;
    vdebug.data.breakIP = breakIP;
    vdebug.data.flagBreak = True;
}
void deviceConnectDebugClearBreak() {
    vdebug.data.flagBreak = False;
}
void deviceConnectDebugSetBreak32(uint32_t breakLinear) {
    vdebug.data.breakLinear = breakLinear;
    vdebug.data.flagBreak32 = True;
    vdebug.data.breakCount = 0;
}
void deviceConnectDebugClearBreak32() {
    vdebug.data.flagBreak32 = False;
}
size_t deviceConnectDebugGetBreakCount() {
    return vdebug.data.breakCount;
}
void deviceConnectDebugSetTrace(size_t traceCount) {
    vdebug.data.traceCount = traceCount;
    vdebug.data.flagTrace = True;
}
void deviceConnectDebugClearTrace() {
    vdebug.data.flagTrace = False;
}
void deviceConnectDebugRecordStart(const char *fileName) {
    if (vdebug.connect.recordFile) {
        FCLOSE(vdebug.connect.recordFile);
    }
    vdebug.connect.recordFile = FOPEN(fileName, "w");
    if (!vdebug.connect.recordFile) {
        PRINTF("ERROR:\tcannot write dump file.\n");
    } else {
        PRINTF("Record started.\n");
    }
}
void deviceConnectDebugRecordStop() {
    if (!vdebug.connect.recordFile) {
        PRINTF("ERROR:\trecorder not turned on.\n");
    } else {
        PRINTF("Record finished.\n");
        FCLOSE(vdebug.connect.recordFile);
        vdebug.connect.recordFile = (FILE *) NULL;
    }
}
