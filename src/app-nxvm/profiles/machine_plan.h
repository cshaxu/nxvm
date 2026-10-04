#ifndef VM_PROFILE_MACHINE_PLAN_H
#define VM_PROFILE_MACHINE_PLAN_H

#include "app-nxvm/profiles/machine_plan_interface.h"

/* The actual profile context embeds this prefix; it owns the firmware and
 * the existing construction binding. There is no model tag or union. */
struct vm_profile_machine_plan {
    vm_machine_construction construction;
};

lib_status vm_profile_machine_plan_validate(const vm_machine_config *config,
    const vm_machine_assets *assets, vm_profile_machine_plan **out_plan);
lib_status vm_profile_machine_plan_publish(vm_profile_machine_plan *plan,
    const vm_machine_config *config, const vm_machine_assets *assets,
    lib_status status, vm_profile_machine_plan **out_plan);
lib_status vm_profile_machine_plan_copy(lib_u8 *destination,
    lib_size expected, vm_machine_asset_bytes source);
lib_status vm_profile_machine_plan_floppy(const vm_machine_config *config,
    vm_profile_floppy_kind default_kind, lib_bool allow_360,
    vm_profile_floppy_kind *out_media);

#endif
