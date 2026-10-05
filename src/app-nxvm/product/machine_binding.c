#include "app-nxvm/product/profile_binding.h"
#include VM_PROFILE_CONSTRUCTION_HEADER

const vm_app_machine_binding vm_app_machine = {
    .name = VM_APP_PROFILE_MONITOR_NAME,
    .cpu = VM_APP_PROFILE_CPU,
    .fpu = VM_APP_PROFILE_FPU,
    .floppy_format = VM_APP_PROFILE_FLOPPY_FORMAT,
    .bios_count = VM_APP_PROFILE_BIOS_COUNT,
    .firmware = &vm_app_firmware,
    .prepare = VM_PROFILE_PLAN_CREATE
};
