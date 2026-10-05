#include "app-nxvm/profiles/machine_factory_interface.h"
#include VM_PROFILE_CONSTRUCTION_HEADER
#include "app-nxvm/profiles/machine_plan_interface.h"

lib_status vm_machine_create_from_assets(const vm_machine_config *config,
    const vm_machine_assets *assets, vm_machine **out_machine)
{
    vm_profile_machine_plan *plan = LIB_NULL;
    lib_status status;

    if (out_machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_machine = LIB_NULL;
    status = VM_PROFILE_PLAN_CREATE(config, assets, &plan);
    if (status != LIB_STATUS_OK) return status;
    return vm_machine_create_from_plan(config, plan, out_machine);
}

lib_status vm_machine_create_from_plan(const vm_machine_config *config,
    vm_profile_machine_plan *plan, vm_machine **out_machine)
{
    vm_machine_construction construction;
    vm_machine_runtime_config runtime = {0};
    lib_status status;

    if (out_machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_machine = LIB_NULL;
    if (config == LIB_NULL || plan == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = vm_profile_machine_plan_describe(plan, &construction);
    if (status != LIB_STATUS_OK) {
        vm_profile_machine_plan_destroy(plan);
        return status;
    }
    for (lib_size slot = 0u; slot < VM_MACHINE_FLOPPY_SLOT_COUNT; ++slot) {
        runtime.floppy_image[slot] = config->floppy_image[slot];
        runtime.floppy_mode[slot] = config->floppy_mode[slot];
    }
    for (lib_size slot = 0u; slot < VM_MACHINE_FIXED_DISK_SLOT_COUNT; ++slot) {
        runtime.fixed_disk_image[slot] = config->fixed_disk_image[slot];
        runtime.fixed_disk_mode[slot] = config->fixed_disk_mode[slot];
    }
    runtime.create_fdd = config->create_fdd ? LIB_TRUE : LIB_FALSE;
    runtime.create_hdd_cylinders = config->create_hdd_cylinders;
    return vm_machine_create(&runtime, &construction, out_machine);
}
