#include "app-my5160/product/binding_interface.h"
#include "app-my5160/profiles/construction_interface.h"

const vm_app_machine_binding vm_app_machine = {
    .name = "ibm-5160-model-268",
    .cpu = CORE_MACHINE_CPU_PROFILE_DEFAULT,
    .fpu = X86_FPU_PROFILE_NONE,
    .floppy_format = VM_MACHINE_FLOPPY_FORMAT_360K,
    .bios_count = 1u,
    .firmware = &vm_app_firmware,
    .prepare = vm_profile_machine_plan_create_xt
};
