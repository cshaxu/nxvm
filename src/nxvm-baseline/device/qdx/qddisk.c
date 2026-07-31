/* Copyright 2012-2014 Neko. */

/* QDDISK implements quick and dirty hard drive control routines. */

#include "../../utils.h"

#include "../vcpu.h"
#include "../vram.h"
#include "../vhdd.h"

#include "qdx.h"
#include "qddisk.h"

#define SetHddStatus (vramRealByte(0x0040, 0x0074) = vcpu.data.ah)
#define GetHddStatus (vramRealByte(0x0040, 0x0074))

static t_vaddrcc vhddGetAddress(t_nubit8 cyl, t_nubit8 head, t_nubit8 sector) {
    vhdd.data.cyl = cyl;
    vhdd.data.head = head;
    vhdd.data.sector = sector;
    vhddSetPointer;
    return vhdd.connect.pCurrByte;
}

static void INT_13_02_HDD_ReadSector() {
    t_nubit8 drive  = vcpu.data.dl;
    t_nubit8 head   = vcpu.data.dh;
    t_nubit8 cyl    = vcpu.data.ch | ((vcpu.data.cl & 0xc0) << 8);
    t_nubit8 sector = vcpu.data.cl & 0x3f;
    drive &= 0x7f;
    if (drive || !sector || head >= vhdd.data.nhead || sector > vhdd.data.nsector || cyl >= vhdd.data.ncyl) {
        /* sector not found */
        vcpu.data.ah = 0x04;
        SetBit(vcpu.data.eflags, VCPU_EFLAGS_CF);
    } else {
        MEMCPY((void *) vramGetRealAddr(vcpu.data.es.selector,vcpu.data.bx),
               (void *) vhddGetAddress(cyl,head,sector), vcpu.data.al * vhdd.data.nbyte);
        vcpu.data.ah = 0x00;
        ClrBit(vcpu.data.eflags, VCPU_EFLAGS_CF);
    }
}

static void INT_13_03_HDD_WriteSector() {
    t_nubit8 drive  = vcpu.data.dl;
    t_nubit8 head   = vcpu.data.dh;
    t_nubit8 cyl    = vcpu.data.ch | ((vcpu.data.cl & 0xc0) << 8);
    t_nubit8 sector = vcpu.data.cl & 0x3f;
    drive &= 0x7f;
    if (drive || !sector || head >= vhdd.data.nhead || sector > vhdd.data.nsector || cyl >= vhdd.data.ncyl) {
        /* sector not found */
        vcpu.data.ah = 0x04;
        SetBit(vcpu.data.eflags, VCPU_EFLAGS_CF);
    } else {
        MEMCPY((void *) vhddGetAddress(cyl,head,sector),
               (void *) vramGetRealAddr(vcpu.data.es.selector,vcpu.data.bx), vcpu.data.al * vhdd.data.nbyte);
        vcpu.data.ah = 0x00;
        ClrBit(vcpu.data.eflags, VCPU_EFLAGS_CF);
    }
}

void qddiskInit() {
    qdxTable[0xa2] = (t_faddrcc) INT_13_02_HDD_ReadSector;
    qdxTable[0xa3] = (t_faddrcc) INT_13_03_HDD_WriteSector;
}
