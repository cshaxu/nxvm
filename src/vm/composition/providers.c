/* Copyright 2012-2014 Neko. */

/* VM composition assembles the retained full-PC providers. */

#include "type.h"

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

#include "vm/profile/default_profile/firmware/hdc.h"

#include "vm/machine/hdd.h"

#include "vm/composition/block_provider.h"

#include "vm/composition/display_bridge.h"

#include "core/machine/kbc.h"

#include "core/machine/vadp.h"

#include "vm/profile/default_profile/firmware/qdx.h"

#include "vm/composition/session.h"


#include "vm/composition/providers.h"

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
C_VOID vm_session_providers_initialize(vm_session *machine) {
    if (machine == STD_NULL) return;
    vm_machine_fdd_initialize(machine->fdd);
    vm_machine_hdd_initialize(machine->hdd);
    vm_session_bind_block(machine);
    vm_profile_default_bios_initialize(machine->default_bios);
    vm_profile_default_bios_add_interrupt(machine->default_bios,
        "qdx 10\niret", 0x10);
    _vbios_
    vm_machine_cmos_initialize(machine->cmos,
        vm_composition_machine_access_cpu(machine->core_access),
        vm_composition_machine_access_port(machine->core_access));
    vm_profile_default_bios_add_post(machine->default_bios, VCMOS_POST);
    vm_profile_default_bios_add_interrupt(machine->default_bios, VCMOS_INT_HARD_RTC_08, 0x08);
    vm_profile_default_bios_add_interrupt(machine->default_bios, VCMOS_INT_SOFT_RTC_1A, 0x1a);
    vm_machine_cmos_reset(machine->cmos);
    vm_machine_cmos_refresh(machine->cmos);
    _vbios_ _vport_
    vm_profile_default_bios_add_interrupt(machine->default_bios, "qdx 09\niret", 0x09);
    vm_profile_default_bios_add_interrupt(machine->default_bios, "qdx 16\niret", 0x16);
    _vbios_ _vport_
    vm_profile_default_bios_add_post(machine->default_bios, VDMA_POST);
    _vbios_ _vport_
    vm_machine_fdc_connect(machine->fdc, machine->fdd, machine->dma_latch,
        machine->dma_primary, machine->dma_secondary, machine->pic_master,
        machine->pic_slave, vm_composition_machine_access_port(machine->core_access));
    vm_machine_fdc_initialize(machine->fdc);
    vm_profile_default_bios_add_post(machine->default_bios, VFDC_POST);
    vm_profile_default_bios_add_interrupt(machine->default_bios, VFDC_INT_HARD_FDD_0E, 0x0e);
    /* overwritten below by the hard-disk INT 13 service, as before. */
    vm_profile_default_bios_add_interrupt(machine->default_bios, VFDC_INT_SOFT_FDD_40, 0x13);
    vm_profile_default_bios_add_interrupt(machine->default_bios, VFDC_INT_SOFT_FDD_40, 0x40);
    _vbios_ _vport_ _vdma_
    vm_profile_default_bios_add_interrupt(machine->default_bios, VHDC_INT_SOFT_HDD_13, 0x13);
    _vbios_ _vport_ _vdma_ _vfdc_
    vm_profile_default_bios_add_post(machine->default_bios, VPIT_POST);
    _vbios_ _vport_
    vm_profile_default_bios_add_post(machine->default_bios, VPIC_POST);
    _vbios_ _vport_ _vpic_
    _vbios_ _vport_ _vpit_
    vm_profile_default_qdx_initialize(machine->default_profile_context,
        vm_composition_machine_access_execution(machine->core_access));
    _vbios_ _vcpu_ _vram_
}

C_VOID vm_session_providers_refresh(vm_session *machine) {
    if (machine == STD_NULL) return;
    vm_profile_default_qdx_refresh(machine->default_profile_context);
    _empty_
    vm_profile_default_bios_refresh(machine->default_bios);
    _empty_
    vm_machine_fdd_refresh(machine->fdd);
    _empty_
    vm_machine_hdd_refresh(machine->hdd);
    _empty_
    vm_machine_cmos_refresh(machine->cmos);
    vm_machine_fdc_refresh(machine->fdc);
}

C_VOID vm_session_providers_reset(vm_session *machine) {
    if (machine == STD_NULL) return;
    vm_machine_cmos_reset(machine->cmos);
    vm_machine_fdc_reset(machine->fdc);
    vm_machine_fdd_reset(machine->fdd);
    vm_machine_hdd_reset(machine->hdd);
    vm_profile_default_bios_reset(machine->default_bios,
        vm_composition_machine_access_memory(machine->core_access),
        machine->block_provider);
    vm_profile_default_qdx_reset(machine->default_profile_context);
}

/* Finalize all devices, deallocates space */
C_VOID vm_session_providers_finalize(vm_session *machine) {
    if (machine == STD_NULL) return;
    vm_profile_default_qdx_finalize(machine->default_profile_context);
    _empty_
    vm_profile_default_bios_finalize(machine->default_bios);
    _empty_
    vm_machine_cmos_finalize(machine->cmos);
    _empty_
    vm_machine_fdc_finalize(machine->fdc);
    vm_machine_fdd_finalize(machine->fdd);
    vm_machine_hdd_finalize(machine->hdd);
}
/* Print machine info */
C_VOID vm_session_print_machine(const vm_session *machine) {
    if (machine == STD_NULL) return;
    STD_PRINTF("Machine:           %s\n", VM_SESSION_MACHINE_NAME);
    STD_PRINTF("CPU:               %s\n", NXVM_DEVICE_CPU);
    STD_PRINTF("RAM Size:          %d MB\n",
        vm_composition_machine_access_memory(machine->core_access)->connect.size >> 20);
    STD_PRINTF("Floppy Disk Drive: %s, %.2f MB, %s\n", NXVM_DEVICE_FDD,
           vm_machine_fdd_image_size(machine->fdd) * 1. / VFDD_BYTE_PER_MB,
           machine->fdd->connect.flagDiskExist ? "inserted" : "not inserted");
    STD_PRINTF("Hard Disk Drive:   %d cylinders, %.2f MB, %s\n",
           machine->hdd->data.ncyl,
           vm_machine_hdd_image_size(machine->hdd) * 1. / VHDD_BYTE_PER_MB,
           machine->hdd->connect.flagDiskExist ? "connected" : "disconnected");
}
