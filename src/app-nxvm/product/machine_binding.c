#include "app-nxvm/product/binding_interface.h"
#include "app-nxvm/profiles/construction_interface.h"

const vm_app_machine_binding vm_app_machine = {
    .name = "default-pc-at",
    .cpu = CORE_MACHINE_CPU_PROFILE_80386,
    .fpu = X86_FPU_PROFILE_NONE,
    .floppy_format = VM_MACHINE_FLOPPY_FORMAT_1440K,
    .bios_count = 1u,
    .firmware = &vm_app_firmware,
    .prepare = vm_profile_machine_plan_create_default
};
