/* Copyright 2012-2014 Neko. */

/* VM composition assembles the retained full-PC providers. */

#include "core/product/utils.h"

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
void vm_composition_providers_initialize(vm_composition_live_machine *machine) {
    if (machine == NULL) return;
    core_machine_cpu_state_initialize(machine->cpu_execution);
    vm_machine_fdd_initialize(machine->fdd);
    vm_machine_hdd_initialize(machine->hdd);
    vm_composition_bind_block(machine);
    vm_profile_default_bios_initialize(machine->default_bios);
    core_machine_vadp_initialize(machine->vadp);
    vm_profile_default_bios_add_interrupt(machine->default_bios,
        "qdx 10\niret", 0x10);
    _vbios_
    core_machine_port_initialize(machine->port);
    vm_machine_cmos_initialize(machine->cmos, machine->cpu, machine->port);
    vm_profile_default_bios_add_post(machine->default_bios, VCMOS_POST);
    vm_profile_default_bios_add_interrupt(machine->default_bios, VCMOS_INT_HARD_RTC_08, 0x08);
    vm_profile_default_bios_add_interrupt(machine->default_bios, VCMOS_INT_SOFT_RTC_1A, 0x1a);
    vm_machine_cmos_reset(machine->cmos);
    vm_machine_cmos_refresh(machine->cmos);
    _vbios_ _vport_
    core_machine_kbc_initialize(machine->kbc, machine->port);
    vm_profile_default_bios_add_interrupt(machine->default_bios, "qdx 09\niret", 0x09);
    vm_profile_default_bios_add_interrupt(machine->default_bios, "qdx 16\niret", 0x16);
    _vbios_ _vport_
    core_machine_dma_initialize(machine->dma_latch, machine->dma_primary,
        machine->dma_secondary, machine->port);
    vm_profile_default_bios_add_post(machine->default_bios, VDMA_POST);
    _vbios_ _vport_
    vm_machine_fdc_connect(machine->fdc, machine->fdd, machine->dma_latch,
        machine->dma_primary, machine->dma_secondary, machine->pic_master,
        machine->pic_slave, machine->port);
    vm_machine_fdc_initialize(machine->fdc);
    vm_profile_default_bios_add_post(machine->default_bios, VFDC_POST);
    vm_profile_default_bios_add_interrupt(machine->default_bios, VFDC_INT_HARD_FDD_0E, 0x0e);
    /* overwritten below by the hard-disk INT 13 service, as before. */
    vm_profile_default_bios_add_interrupt(machine->default_bios, VFDC_INT_SOFT_FDD_40, 0x13);
    vm_profile_default_bios_add_interrupt(machine->default_bios, VFDC_INT_SOFT_FDD_40, 0x40);
    _vbios_ _vport_ _vdma_
    vm_machine_hdc_initialize();
    vm_profile_default_bios_add_interrupt(machine->default_bios, VHDC_INT_SOFT_HDD_13, 0x13);
    _vbios_ _vport_ _vdma_ _vfdc_
    core_machine_pit_initialize(machine->pit, machine->port);
    core_machine_pit_set_output(machine->pit, 0,
        core_machine_pic_timer_output, machine->pic_master);
    vm_profile_default_bios_add_post(machine->default_bios, VPIT_POST);
    _vbios_ _vport_
    core_machine_pic_initialize(machine->pic_master, machine->pic_slave,
        machine->port);
    vm_profile_default_bios_add_post(machine->default_bios, VPIC_POST);
    _vbios_ _vport_ _vpic_
    core_machine_memory_initialize(machine->ram);
    core_machine_memory_register_ports(machine->ram, machine->port);
    core_machine_pit_set_output(machine->pit, 1, NULL, NULL);
    _vbios_ _vport_ _vpit_
    vm_profile_default_qdx_initialize(machine->default_profile_context,
        machine->cpu_execution);
    _vbios_ _vcpu_ _vram_
}

void vm_composition_providers_refresh(vm_composition_live_machine *machine) {
    if (machine == NULL) return;
    vm_profile_default_qdx_refresh(machine->default_profile_context);
    _empty_
    vm_profile_default_bios_refresh(machine->default_bios);
    _empty_
    vm_machine_fdd_refresh(machine->fdd);
    _empty_
    vm_machine_hdc_refresh();
    _empty_
    vm_machine_hdd_refresh(machine->hdd);
    _empty_
    vm_machine_cmos_refresh(machine->cmos);
    vm_machine_fdc_refresh(machine->fdc);
}

void vm_composition_providers_reset(vm_composition_live_machine *machine) {
    if (machine == NULL) return;
    vm_machine_hdc_reset();
    vm_machine_cmos_reset(machine->cmos);
    vm_machine_fdc_reset(machine->fdc);
    vm_machine_fdd_reset(machine->fdd);
    vm_machine_hdd_reset(machine->hdd);
    vm_profile_default_bios_reset(machine->default_bios, machine->ram,
        machine->block_provider);
    vm_profile_default_qdx_reset(machine->default_profile_context);
}

/* Finalize all devices, deallocates space */
void vm_composition_providers_finalize(vm_composition_live_machine *machine) {
    if (machine == NULL) return;
    vm_profile_default_qdx_finalize(machine->default_profile_context);
    _empty_
    vm_profile_default_bios_finalize(machine->default_bios);
    _empty_
    vm_machine_cmos_finalize(machine->cmos);
    _empty_
    core_machine_dma_finalize(machine->dma_latch, machine->dma_primary,
        machine->dma_secondary);
    _empty_
    vm_machine_fdc_finalize(machine->fdc);
    _empty_
    vm_machine_hdc_finalize();
    _empty_
    core_machine_kbc_finalize(machine->kbc);
    _empty_
    core_machine_pic_finalize(machine->pic_master, machine->pic_slave);
    _empty_
    core_machine_pit_finalize(machine->pit);
    _empty_
    core_machine_port_finalize(machine->port);
    _empty_
    core_machine_vadp_finalize(machine->vadp);
    _empty_

    core_machine_cpu_execution_finalize(machine->cpu_execution);
    vm_machine_fdd_finalize(machine->fdd);
    vm_machine_hdd_finalize(machine->hdd);
    core_machine_memory_finalize(machine->ram);
}
/* Print machine info */
void vm_composition_print_machine(const vm_composition_live_machine *machine) {
    if (machine == NULL) return;
    STD_PRINTF("Machine:           %s\n", VM_COMPOSITION_MACHINE_NAME);
    STD_PRINTF("CPU:               %s\n", NXVM_DEVICE_CPU);
    STD_PRINTF("RAM Size:          %d MB\n", machine->ram->connect.size >> 20);
    STD_PRINTF("Floppy Disk Drive: %s, %.2f MB, %s\n", NXVM_DEVICE_FDD,
           vm_machine_fdd_image_size(machine->fdd) * 1. / VFDD_BYTE_PER_MB,
           machine->fdd->connect.flagDiskExist ? "inserted" : "not inserted");
    STD_PRINTF("Hard Disk Drive:   %d cylinders, %.2f MB, %s\n",
           machine->hdd->data.ncyl,
           vm_machine_hdd_image_size(machine->hdd) * 1. / VHDD_BYTE_PER_MB,
           machine->hdd->connect.flagDiskExist ? "connected" : "disconnected");
}
