/* Copyright 2012-2014 Neko. */

/* VMACHINE is the hub that assembles all devices. */

#include "../utils.h"

#include "vdebug.h"
#include "vport.h"
#include "vram.h"
#include "vcpu.h"
#include "vbios.h"
#include "vpit.h"
#include "vdma.h"
#include "vpic.h"
#include "vcmos.h"
#include "vfdc.h"
#include "vfdd.h"
#include "vhdc.h"
#include "vhdd.h"
#include "vkbc.h"
#include "vvadp.h"
#include "qdx/qdx.h"

#include "vmachine.h"

#define _empty_
#define _vdebug_
#define _vport_
#define _vram_
#define _vcpu_
#define _vbios_
#define _vpit_
#define _vdma_
#define _vpic_
#define _vcmos_
#define _vfdc_
#define _vfdd_
#define _vhdc_
#define _vhdd_
#define _vkbc_
#define _vvadp_
#define _qdx_

/* Initializes all devices, allocates space */
void vmachineInit() {
    vcpuInit();
    vdebugInit();
    vfddInit();
    vhddInit();
    vbiosInit();
    vvadpInit();
    _vbios_
    vportInit();
    vcmosInit();
    _vbios_;
    _vport_
    vkbcInit();
    _vbios_;
    _vport_
    vdmaInit();
    _vbios_;
    _vport_
    vfdcInit();
    _vbios_;
    _vport_;
    _vdma_
    vhdcInit();
    _vbios_;
    _vport_;
    _vdma_;
    _vfdc_
    vpitInit();
    _vbios_;
    _vport_;
    vpicInit();
    _vbios_;
    _vport_;
    _vpic_
    vramInit();
    _vbios_;
    _vport_;
    _vpit_
    qdxInit();
    _vbios_;
    _vcpu_;
    _vram_
}

/* Resets all devices to initial values */
void vmachineReset() {
    vdebugReset();
    _empty_
    vhdcReset();
    _empty_
    vkbcReset();
    _empty_

    vcmosReset();
    vcpuReset();
    vdmaReset();
    vfdcReset();
    vfddReset();
    vhddReset();
    vpicReset();
    vpitReset();
    vportReset();
    vvadpReset();
    vramReset();
    vbiosReset();
    _vram_
    qdxReset();
    _vram_
}

/* Executes all devices in one loop */
void vmachineRefresh() {
    qdxRefresh();
    _empty_
    vbiosRefresh();
    _empty_
    vfddRefresh();
    _empty_
    vhdcRefresh();
    _empty_
    vhddRefresh();
    _empty_
    vkbcRefresh();
    _empty_
    vportRefresh();
    _empty_
    vvadpRefresh();
    _empty_
    vramRefresh();
    _empty_

    vdebugRefresh();
    vcmosRefresh();
    vfdcRefresh();
    vdmaRefresh();
    _vfdc_
    vpicRefresh();
    vpitRefresh();
    _vpic_
    vcpuRefresh();
    _vpic_
}

/* Finalize all devices, deallocates space */
void vmachineFinal() {
    qdxFinal();
    _empty_
    vbiosFinal();
    _empty_
    vcmosFinal();
    _empty_
    vdebugFinal();
    _empty_
    vdmaFinal();
    _empty_
    vfdcFinal();
    _empty_
    vhdcFinal();
    _empty_
    vkbcFinal();
    _empty_
    vpicFinal();
    _empty_
    vpitFinal();
    _empty_
    vportFinal();
    _empty_
    vvadpFinal();
    _empty_

    vcpuFinal();
    vfddFinal();
    vhddFinal();
    vramFinal();
}

/* Print machine info */
void devicePrintMachine() {
    PRINTF("Machine:           %s\n", NXVM_DEVICE_MACHINE);
    PRINTF("CPU:               %s\n", NXVM_DEVICE_CPU);
    PRINTF("RAM Size:          %d MB\n", vram.connect.size >> 20);
    PRINTF("Floppy Disk Drive: %s, %.2f MB, %s\n", NXVM_DEVICE_FDD,
           vfddGetImageSize * 1. / VFDD_BYTE_PER_MB,
           vfdd.connect.flagDiskExist ? "inserted" : "not inserted");
    PRINTF("Hard Disk Drive:   %d cylinders, %.2f MB, %s\n", vhdd.data.ncyl,
           vhddGetImageSize * 1. / VHDD_BYTE_PER_MB,
           vhdd.connect.flagDiskExist ? "connected" : "disconnected");
}
