/* Copyright 2012-2014 Neko. */

/* VRAM is the random accessing memory module.  */

#include "../utils.h"

#include "vpit.h"
#include "vport.h"
#include "vram.h"

t_ram vram;

/* Allocates memory for virtual machine ram */
static void allocate(t_nubitcc newsize) {
    if (newsize) {
        vram.connect.size = newsize;
        if (vram.connect.pBase) {
            FREE((void *) vram.connect.pBase);
        }
        vram.connect.pBase = (t_vaddrcc) MALLOC(vram.connect.size);
        MEMSET((void *) vram.connect.pBase, Zero8, vram.connect.size);
    }
}

static void io_read_0092() {
    vport.data.ioByte = vram.data.flagA20 ? VRAM_FLAG_A20 : Zero8;
}

static void io_write_0092() {
    vram.data.flagA20 = GetBit(vport.data.ioByte, VRAM_FLAG_A20);
}

#define pitOut ((t_faddrcc) NULL)
void vramInit() {
    MEMSET((void *)(&vram), Zero8, sizeof(t_ram));
    vportAddRead(0x0092, (t_faddrcc) io_read_0092);
    vportAddWrite(0x0092, (t_faddrcc) io_write_0092);
    vpitAddMe(1);
    /* 16 MB */
    allocate(1 << 24);
}

void vramReset() {
    MEMSET((void *)(&vram.data), Zero8, sizeof(t_ram_data));
    MEMSET((void *) vram.connect.pBase, Zero8, vram.connect.size);
}

void vramRefresh() {}

void vramFinal() {
    if (vram.connect.pBase) {
        FREE((void *) vram.connect.pBase);
    }
}

void vramReadPhysical(t_nubit32 physical, t_vaddrcc rdest, t_nubitcc byte) {
    if (physical >= vram.connect.size && physical >= 0xfffe0000) {
        physical &= 0x001fffff;
    }
    MEMCPY((void *) rdest, (void *) VRAM_GetAddr(physical), byte);
}

void vramWritePhysical(t_nubit32 physical, t_vaddrcc rsrc, t_nubitcc byte) {
    MEMCPY((void *) VRAM_GetAddr(physical), (void *) rsrc, byte);
}

void deviceConnectRamAllocate(t_nubitcc newsize) {
    allocate(newsize);
}
