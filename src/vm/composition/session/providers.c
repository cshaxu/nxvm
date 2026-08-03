/* Copyright 2012-2014 Neko. */

/* VM composition assembles the retained full-PC providers. */

#include "type.h"

#include "core/product/utils.h"


#include "core/machine/machine_interface.h"

#include "vm/profile/default_profile/firmware/bios.h"

#include "vm/machine/cmos.h"

#include "vm/machine/fdc.h"

#include "vm/machine/fdd.h"

#include "vm/profile/default_profile/firmware/hdc.h"

#include "vm/machine/hdd.h"

#include "vm/composition/session/block.h"

#include "vm/composition/session/display.h"

#include "vm/profile/default_profile/firmware/qdx.h"

#include "vm/composition/session/session.h"


#include "vm/composition/session/providers.h"

/* Initializes all devices, allocates space */
C_VOID vm_session_providers_initialize(vm_session *machine) {
    if (machine == STD_NULL) return;
    vm_machine_fdd_initialize(machine->fdd);
    vm_machine_hdd_initialize(machine->hdd);
    vm_session_bind_block(machine);
    vm_profile_default_bios_initialize(machine->default_bios);
    vm_profile_default_bios_add_interrupt(machine->default_bios,
        "qdx 10\niret", 0x10);
    vm_machine_cmos_initialize(machine->cmos,
        core_machine_executor_cpu_borrow(machine->core_machine),
        core_machine_executor_port_borrow(machine->core_machine));
    vm_profile_default_bios_add_post(machine->default_bios, VCMOS_POST);
    vm_profile_default_bios_add_interrupt(machine->default_bios, VCMOS_INT_HARD_RTC_08, 0x08);
    vm_profile_default_bios_add_interrupt(machine->default_bios, VCMOS_INT_SOFT_RTC_1A, 0x1a);
    vm_machine_cmos_reset(machine->cmos);
    vm_machine_cmos_refresh(machine->cmos);
    vm_profile_default_bios_add_interrupt(machine->default_bios, "qdx 09\niret", 0x09);
    vm_profile_default_bios_add_interrupt(machine->default_bios, "qdx 16\niret", 0x16);
    vm_profile_default_bios_add_post(machine->default_bios, VDMA_POST);
    vm_machine_fdc_connect(machine->fdc, machine->fdd,
        core_machine_shared_dma_latch_borrow(machine->core_machine),
        core_machine_shared_dma_primary_borrow(machine->core_machine),
        core_machine_shared_dma_secondary_borrow(machine->core_machine),
        core_machine_shared_pic_master_borrow(machine->core_machine),
        core_machine_shared_pic_slave_borrow(machine->core_machine),
        core_machine_executor_port_borrow(machine->core_machine));
    vm_machine_fdc_initialize(machine->fdc);
    vm_profile_default_bios_add_post(machine->default_bios, VFDC_POST);
    vm_profile_default_bios_add_interrupt(machine->default_bios, VFDC_INT_HARD_FDD_0E, 0x0e);
    /* overwritten below by the hard-disk INT 13 service, as before. */
    vm_profile_default_bios_add_interrupt(machine->default_bios, VFDC_INT_SOFT_FDD_40, 0x13);
    vm_profile_default_bios_add_interrupt(machine->default_bios, VFDC_INT_SOFT_FDD_40, 0x40);
    vm_profile_default_bios_add_interrupt(machine->default_bios, VHDC_INT_SOFT_HDD_13, 0x13);
    vm_profile_default_bios_add_post(machine->default_bios, VPIT_POST);
    vm_profile_default_bios_add_post(machine->default_bios, VPIC_POST);
    vm_profile_default_qdx_initialize(machine->default_profile_context,
        core_machine_executor_cpu_execution_borrow(machine->core_machine));
}

C_VOID vm_session_providers_refresh(vm_session *machine) {
    if (machine == STD_NULL) return;
    vm_profile_default_qdx_refresh(machine->default_profile_context);
    vm_profile_default_bios_refresh(machine->default_bios);
    vm_machine_fdd_refresh(machine->fdd);
    vm_machine_hdd_refresh(machine->hdd);
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
        core_machine_executor_memory_borrow(machine->core_machine),
        machine->block_provider);
    vm_profile_default_qdx_reset(machine->default_profile_context);
}

/* Finalize all devices, deallocates space */
C_VOID vm_session_providers_finalize(vm_session *machine) {
    if (machine == STD_NULL) return;
    vm_profile_default_qdx_finalize(machine->default_profile_context);
    vm_profile_default_bios_finalize(machine->default_bios);
    vm_machine_cmos_finalize(machine->cmos);
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
        core_machine_executor_memory_borrow(machine->core_machine)->connect.size >> 20);
    STD_PRINTF("Floppy Disk Drive: %s, %.2f MB, %s\n", NXVM_DEVICE_FDD,
           vm_machine_fdd_image_size(machine->fdd) * 1. / VFDD_BYTE_PER_MB,
           machine->fdd->connect.flagDiskExist ? "inserted" : "not inserted");
    STD_PRINTF("Hard Disk Drive:   %d cylinders, %.2f MB, %s\n",
           machine->hdd->data.ncyl,
           vm_machine_hdd_image_size(machine->hdd) * 1. / VHDD_BYTE_PER_MB,
           machine->hdd->connect.flagDiskExist ? "connected" : "disconnected");
}
