#include "app-my5170/product/binding_interface.h"
#include "app-my5170/profiles/construction_interface.h"

const vm_app_machine_binding vm_app_machine = {
    .name = "ibm-5170-model-339",
    .cpu = CORE_MACHINE_CPU_PROFILE_DEFAULT,
    .fpu = X86_FPU_PROFILE_NONE,
    .floppy_format = VM_MACHINE_FLOPPY_FORMAT_1200K,
    .bios_count = 2u,
    .firmware = &vm_app_firmware,
    .prepare = vm_profile_machine_plan_create_5170
};
