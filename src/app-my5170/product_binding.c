#include "app-my5170/product_binding_interface.h"
#include "app-my5170/machine_construction_interface.h"

const x86_nxvm_product_machine_binding vm_app_machine = {
    .name = "ibm-5170-model-339",
    .cpu = X86_CORE_CPU_PROFILE_DEFAULT,
    .fpu = X86_FPU_PROFILE_NONE,
    .floppy_format = X86_NXVM_MACHINE_FLOPPY_FORMAT_1200K,
    .bios_count = 2u,
    .firmware = &vm_app_firmware,
    .prepare = vm_profile_machine_plan_create_5170
};
