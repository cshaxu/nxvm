/* Copyright 2012-2014 Neko. */

#include "../../src/utils.h"
#include "../../src/device/device.h"
#include "../../src/device/vcpuins.h"
#include "../../src/device/vram.h"

#include "msdos.h"

/* stack pointer size */
#define _GetStackSize   (vcpu.data.ss.seg.data.big ? 4 : 2)
/* operand size */
#define _GetOperandSize ((vcpu.data.cs.seg.exec.defsize ^ vcpuins.data.prefix_oprsize) ? 4 : 2)
/* address size of the source operand */
#define _GetAddressSize ((vcpu.data.cs.seg.exec.defsize ^ vcpuins.data.prefix_addrsize) ? 4 : 2)

static void INT_OLD(t_nubit8 intId) {
    t_nubit16 newCS, newIP, oldCS, oldIP;
    newIP = vramRealWord(Zero16, intId * 4 + 0);
    newCS = vramRealWord(Zero16, intId * 4 + 2);
    oldIP = vramRealWord(newCS, newIP + 2);
    oldCS = vramRealWord(newCS, newIP + 4);
    deviceConnectCpuLoadCS(oldCS);
    vcpu.data.eip = oldIP;
}

static void PUSH(t_vaddrcc rdata, t_nubit8 byte) {
    t_nubit32 cesp;
    switch (_GetStackSize) {
    case 2:
        cesp = GetMax16(vcpu.data.sp - byte);
        break;
    case 4:
        cesp = GetMax32(vcpu.data.esp - byte);
        break;
    default:
        break;
    }
    vramWritePhysical(vcpu.data.ss.base + cesp, rdata, byte);
    switch (_GetStackSize) {
    case 2:
        vcpu.data.sp -= byte;
        break;
    case 4:
        vcpu.data.esp -= byte;
        break;
    default:
        break;
    }
}
static void POP(t_vaddrcc rdata, t_nubit8 byte) {
    t_nubit32 cesp;
    switch (_GetStackSize) {
    case 2:
        cesp = vcpu.data.sp;
        break;
    case 4:
        cesp = vcpu.data.esp;
        break;
    default:
        break;
    }
    vramReadPhysical(vcpu.data.ss.base + cesp, rdata, byte);
    if (rdata != GetRef(vcpu.data.esp)) {
        switch (_GetStackSize) {
        case 2:
            vcpu.data.sp += byte;
            break;
        case 4:
            vcpu.data.esp += byte;
            break;
        default:
            break;
        }
    }
}
static void INT(t_nubit8 intId) {
    t_nubit16 intCS, intIP;
    t_nubit32 oldcs = vcpu.data.cs.selector;
    switch (_GetOperandSize) {
    case 2:
        PUSH(GetRef(vcpu.data.flags), _GetOperandSize);
        _ClrEFLAGS_IF;
        _ClrEFLAGS_TF;
        PUSH(GetRef(oldcs), _GetOperandSize);
        PUSH(GetRef(vcpu.data.ip), _GetOperandSize);
        break;
    case 4:
        PUSH(GetRef(vcpu.data.eflags), _GetOperandSize);
        _ClrEFLAGS_IF;
        _ClrEFLAGS_TF;
        PUSH(GetRef(oldcs), _GetOperandSize);
        PUSH(GetRef(vcpu.data.eip), _GetOperandSize);
        break;
    default:
        break;
    }
    /* intIP + 4 points to the original vector */
    intIP = vramRealWord(Zero16, intId * 4 + 0);
    intCS = vramRealWord(Zero16, intId * 4 + 2);
    vcpu.data.eip = intIP;
    deviceConnectCpuLoadCS(intCS);
}
static void IRET() {
    t_nubit16 newcs;
    t_nubit32 neweip = Zero32, neweflags = Zero32;
    t_nubit32 xs_sel;
    t_nubit32 mask = VCPU_EFLAGS_RESERVED;
    switch (_GetOperandSize) {
    case 2:
        POP(GetRef(neweip), 2);
        POP(GetRef(xs_sel), 2);
        newcs = GetMax16(xs_sel);
        POP(GetRef(neweflags), 2);
        mask |= 0xffff0000;
        break;
    case 4:
        POP(GetRef(neweip), 4);
        POP(GetRef(xs_sel), 4);
        newcs = GetMax16(xs_sel);
        POP(GetRef(neweflags), 4);
        /* vcpu.data.eflags = (neweflags & 0x00257fd5) | (vcpu.data.eflags & 0x001a0000); */
        break;
    default:
        break;
    }
    deviceConnectCpuLoadCS(newcs);
    vcpu.data.eip = neweip;
    vcpu.data.eflags = (neweflags & ~mask) | (vcpu.data.eflags & mask);
}

void int21() {
    /* if (vcpu.data.ah == 0x4b) {
        vdebug.connect.recordFile = FOPEN("d:/x.log", "w");
        if (!vdebug.connect.recordFile) {
            PRINTF("ERROR:\tcannot write dump file.\n");
        } else {
            PRINTF("Record started.\n");
        }
    }
    if (vdebug.connect.recordFile) {
        FPRINTF(vdebug.connect.recordFile, "INT 21, AH %02X, AL %02X\n", vcpu.data.ah, vcpu.data.al);
    } */
    INT_OLD(0x21);
}
