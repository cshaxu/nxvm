#include "app-mypc110/product_binding_interface.h"
#include "app-mypc110/pc110_plan_interface.h"

#ifndef MYPC110_MACHINE_KEY
#error "MyPC110 build must select one fixed machine key."
#endif

const x86_nxvm_product_machine_binding vm_app_machine = {
    .name = MYPC110_MACHINE_KEY,
    .cpu = X86_CORE_CPU_PROFILE_80486SX,
    .fpu = X86_FPU_PROFILE_NONE,
    .floppy_format = X86_NXVM_MACHINE_FLOPPY_FORMAT_1440K,
    .bios_count = 1u,
    .firmware = &vm_app_firmware,
    .prepare = mypc110_machine_construction_create
};
