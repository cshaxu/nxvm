/* Copyright 2012-2014 Neko. */

/* VMACHINE is the hub that assembles all devices. */

#include "core/product/utils.h"
#include "core/platform/sleep.h"
#include "core/platform/sleep.h"
#include "core/platform/sleep.h"

#include "core/machine/vport.h"
#include "core/machine/vram.h"
#include "core/machine/cpu.h"
#include "vm/profile/default_profile/firmware/vbios.h"
#include "core/machine/vpit.h"
#include "core/machine/vdma.h"
#include "core/machine/vpic.h"
#include "vm/machine/vcmos.h"
#include "vm/machine/vfdc.h"
#include "vm/machine/vfdd.h"
#include "vm/machine/vhdc.h"
#include "vm/machine/vhdd.h"
#include "vm/composition_block.h"
#include "vm/composition_display.h"
#include "core/machine/vkbc.h"
#include "core/machine/vvadp.h"
#include "vm/profile/default_profile/firmware/qdx.h"

#include "vm/composition.h"

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
    vfddInit();
    vhddInit();
    vmCompositionBindBlock();
    vm_composition_bind_display();
    vbiosInit();
    vvadpInit();
    vbiosAddInt("qdx 10\niret", 0x10);
    _vbios_
    vportInit();
    vcmosInit();
    vbiosAddPost(VCMOS_POST);
    vbiosAddInt(VCMOS_INT_HARD_RTC_08, 0x08);
    vbiosAddInt(VCMOS_INT_SOFT_RTC_1A, 0x1a);
    vcmosReset();
    vcmosRefresh();
    _vbios_ _vport_
    vkbcInit();
    vbiosAddInt("qdx 09\niret", 0x09);
    vbiosAddInt("qdx 16\niret", 0x16);
    _vbios_ _vport_
    vdmaInit();
    vbiosAddPost(VDMA_POST);
    _vbios_ _vport_
    vfdcInit();
    vbiosAddPost(VFDC_POST);
    vbiosAddInt(VFDC_INT_HARD_FDD_0E, 0x0e);
    /* overwritten below by the hard-disk INT 13 service, as before. */
    vbiosAddInt(VFDC_INT_SOFT_FDD_40, 0x13);
    vbiosAddInt(VFDC_INT_SOFT_FDD_40, 0x40);
    _vbios_ _vport_ _vdma_
    vhdcInit();
    vbiosAddInt(VHDC_INT_SOFT_HDD_13, 0x13);
    _vbios_ _vport_ _vdma_ _vfdc_
    vpitInit();
    vbiosAddPost(VPIT_POST);
    _vbios_ _vport_
    vpicInit();
    vbiosAddPost(VPIC_POST);
    _vbios_ _vport_ _vpic_
    vramInit();
    _vbios_ _vport_ _vpit_
    qdxInit();
    _vbios_ _vcpu_ _vram_
}
/* Resets all devices to initial values */
void vmachineReset() {
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

    vcmosRefresh();
    vfdcRefresh();
    vdmaRefresh();
    _vfdc_
    vpicRefresh();
    vpitRefresh();
    _vpic_
    if (vcpu.data.flagHalt) {
        core_platform_sleep_milliseconds(1);
    }
    vcpuRefresh();
    _vpic_
    vm_composition_publish_display(False);
}
/* Finalize all devices, deallocates space */
void vmachineFinal() {
    qdxFinal();
    _empty_
    vbiosFinal();
    _empty_
    vcmosFinal();
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
