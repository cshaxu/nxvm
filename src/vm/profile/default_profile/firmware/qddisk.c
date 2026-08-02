/* Copyright 2012-2014 Neko. */

/* QDDISK implements quick and dirty hard drive control routines. */

#include "core/product/utils.h"

#include "core/machine/cpu.h"
#include "core/machine/vram.h"
#include "core/machine/block.h"

#include "vm/profile/default_profile/firmware/qdx.h"
#include "qddisk.h"

#define SetHddStatus (vramRealByte(0x0040, 0x0074) = vcpu.data.ah)
#define GetHddStatus (vramRealByte(0x0040, 0x0074))

static void INT_13_02_HDD_ReadSector() {
    t_nubit8 drive  = vcpu.data.dl;
    t_nubit8 head   = vcpu.data.dh;
    t_nubit8 cyl    = vcpu.data.ch | ((vcpu.data.cl & 0xc0) << 8);
    t_nubit8 sector = vcpu.data.cl & 0x3f;
    drive &= 0x7f;
    core_block_geometry geometry;
    coreBlockGetGeometry(&geometry);
    if (drive || !sector || head >= geometry.heads || sector > geometry.sectors || cyl >= geometry.cylinders || !coreBlockRead(cyl, head, sector, (void *)vramGetRealAddr(vcpu.data.es.selector,vcpu.data.bx), vcpu.data.al * geometry.bytes_per_sector)) {
        /* sector not found */
        vcpu.data.ah = 0x04;
        SetBit(vcpu.data.eflags, VCPU_EFLAGS_CF);
    } else {
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
    core_block_geometry geometry;
    coreBlockGetGeometry(&geometry);
    if (drive || !sector || head >= geometry.heads || sector > geometry.sectors || cyl >= geometry.cylinders || !coreBlockWrite(cyl, head, sector, (void *)vramGetRealAddr(vcpu.data.es.selector,vcpu.data.bx), vcpu.data.al * geometry.bytes_per_sector)) {
        /* sector not found */
        vcpu.data.ah = 0x04;
        SetBit(vcpu.data.eflags, VCPU_EFLAGS_CF);
    } else {
        vcpu.data.ah = 0x00;
        ClrBit(vcpu.data.eflags, VCPU_EFLAGS_CF);
    }
}

void qddiskInit() {
    qdxTable[0xa2] = (t_faddrcc) INT_13_02_HDD_ReadSector;
    qdxTable[0xa3] = (t_faddrcc) INT_13_03_HDD_WriteSector;
}
