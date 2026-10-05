#include "app-nxvm/product/profile_binding.h"
#include VM_PROFILE_CONSTRUCTION_HEADER
#include "app-nxvm/profiles/machine_plan_interface.h"

static lib_status vm_profile_prepare_machine(const vm_machine_config *config,
    const vm_machine_assets *assets, vm_machine_construction *out_construction)
{
    vm_profile_machine_plan *plan = LIB_NULL;
    lib_status status;

    if (out_construction == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_construction = (vm_machine_construction){0};
    status = VM_PROFILE_PLAN_CREATE(config, assets, &plan);
    if (status != LIB_STATUS_OK) return status;
    status = vm_profile_machine_plan_describe(plan, out_construction);
    if (status != LIB_STATUS_OK) vm_profile_machine_plan_destroy(plan);
    return status;
}

const vm_app_machine_binding vm_app_machine = {
    .name = VM_APP_PROFILE_MONITOR_NAME,
    .cpu = VM_APP_PROFILE_CPU,
    .fpu = VM_APP_PROFILE_FPU,
    .floppy_format = VM_APP_PROFILE_FLOPPY_FORMAT,
    .bios_count = VM_APP_PROFILE_BIOS_COUNT,
    .firmware = &vm_app_firmware,
    .prepare = vm_profile_prepare_machine
};
