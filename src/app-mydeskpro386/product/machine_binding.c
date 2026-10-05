#include "app-mydeskpro386/product/binding_interface.h"
#include "app-mydeskpro386/profiles/construction_interface.h"

const vm_app_machine_binding vm_app_machine = {
    .name = "compaq-deskpro-386-model-40",
    .cpu = CORE_MACHINE_CPU_PROFILE_DEFAULT,
    .fpu = X86_FPU_PROFILE_NONE,
    .floppy_format = VM_MACHINE_FLOPPY_FORMAT_1200K,
    .bios_count = 2u,
    .firmware = &vm_app_firmware,
    .prepare = vm_profile_machine_plan_create_model40
};
