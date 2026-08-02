/* Copyright 2012-2014 Neko. */

/* VMACHINE is the hub that assembles all devices. */

#include "core/product/utils.h"
#include "core/platform/sleep.h"
#include "core/platform/sleep.h"
#include "core/platform/sleep.h"

#include "core/machine/port.h"
#include "core/machine/memory.h"
#include "core/machine/cpu.h"
#include "vm/profile/default_profile/firmware/bios.h"
#include "core/machine/pit.h"
#include "core/machine/dma.h"
#include "core/machine/pic.h"
#include "vm/machine/cmos.h"
#include "vm/machine/fdc.h"
#include "vm/machine/fdd.h"
#include "vm/machine/hdc.h"
#include "vm/machine/hdd.h"
#include "vm/composition_block.h"
#include "vm/composition_display.h"
#include "core/machine/kbc.h"
#include "core/machine/vadp.h"
#include "vm/profile/default_profile/firmware/qdx.h"
#include "vm/composition_live_machine.h"

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
void vmachineInit(vm_composition_live_machine *machine) {
    if (machine == NULL) return;
    vcpuInit();
    vfddInit();
    vhddInit();
    vmCompositionBindBlock(machine);
    vm_composition_bind_display(machine);
    vbiosInit();
    core_machine_vadp_initialize(machine->vadp);
    vbiosAddInt("qdx 10\niret", 0x10);
    _vbios_
    vportInit();
    vm_machine_cmos_initialize(machine->cmos, machine->cpu, machine->port);
    vbiosAddPost(VCMOS_POST);
    vbiosAddInt(VCMOS_INT_HARD_RTC_08, 0x08);
    vbiosAddInt(VCMOS_INT_SOFT_RTC_1A, 0x1a);
    vm_machine_cmos_reset(machine->cmos);
    vm_machine_cmos_refresh(machine->cmos);
    _vbios_ _vport_
    core_machine_kbc_initialize(machine->kbc, machine->port);
    vbiosAddInt("qdx 09\niret", 0x09);
    vbiosAddInt("qdx 16\niret", 0x16);
    _vbios_ _vport_
    core_machine_dma_initialize(machine->dma_latch, machine->dma_primary,
        machine->dma_secondary, machine->port);
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
    core_machine_pit_initialize(machine->pit, machine->port);
    core_machine_pit_set_output(machine->pit, 0,
        core_machine_pic_timer_output, machine->pic_master);
    vbiosAddPost(VPIT_POST);
    _vbios_ _vport_
    core_machine_pic_initialize(machine->pic_master, machine->pic_slave,
        machine->port);
    vbiosAddPost(VPIC_POST);
    _vbios_ _vport_ _vpic_
    vramInit();
    _vbios_ _vport_ _vpit_
    qdxInit();
    _vbios_ _vcpu_ _vram_
}
/* Resets all devices to initial values */
void vmachineReset(vm_composition_live_machine *machine) {
    if (machine == NULL) return;
    vhdcReset();
    _empty_
    core_machine_kbc_reset(machine->kbc);
    _empty_

    vm_machine_cmos_reset(machine->cmos);
    vcpuReset();
    core_machine_dma_reset(machine->dma_latch, machine->dma_primary,
        machine->dma_secondary);
    vfdcReset();
    vfddReset();
    vhddReset();
    core_machine_pic_reset(machine->pic_master, machine->pic_slave);
    core_machine_pit_reset(machine->pit);
    vportReset();
    core_machine_vadp_reset(machine->vadp);
    vramReset();
    vbiosReset();
    _vram_
    qdxReset();
    _vram_
}
/* Executes all devices in one loop */
void vmachineRefresh(vm_composition_live_machine *machine) {
    if (machine == NULL) return;
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
    core_machine_kbc_refresh(machine->kbc);
    _empty_
    vportRefresh();
    _empty_
    core_machine_vadp_refresh(machine->vadp);
    _empty_
    vramRefresh();
    _empty_

    vm_machine_cmos_refresh(machine->cmos);
    vfdcRefresh();
    core_machine_dma_refresh(machine->dma_latch, machine->dma_primary,
        machine->dma_secondary, machine->ram);
    _vfdc_
    core_machine_pic_refresh(machine->pic_master, machine->pic_slave);
    core_machine_pit_refresh(machine->pit);
    _vpic_
    if (vcpu.data.flagHalt) {
        core_platform_sleep_milliseconds(1);
    }
    vcpuRefresh();
    _vpic_
    vm_composition_publish_display(machine, False);
}
/* Finalize all devices, deallocates space */
void vmachineFinal(vm_composition_live_machine *machine) {
    if (machine == NULL) return;
    qdxFinal();
    _empty_
    vbiosFinal();
    _empty_
    vm_machine_cmos_finalize(machine->cmos);
    _empty_
    core_machine_dma_finalize(machine->dma_latch, machine->dma_primary,
        machine->dma_secondary);
    _empty_
    vfdcFinal();
    _empty_
    vhdcFinal();
    _empty_
    core_machine_kbc_finalize(machine->kbc);
    _empty_
    core_machine_pic_finalize(machine->pic_master, machine->pic_slave);
    _empty_
    core_machine_pit_finalize(machine->pit);
    _empty_
    vportFinal();
    _empty_
    core_machine_vadp_finalize(machine->vadp);
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
