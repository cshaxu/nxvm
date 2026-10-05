#ifndef VM_PROFILE_PC_AT_CONSTRUCTION_INTERFACE_H
#define VM_PROFILE_PC_AT_CONSTRUCTION_INTERFACE_H

#include "app-nxvm/profiles/machine_plan_interface.h"

lib_status vm_profile_machine_plan_create_default(const vm_machine_config *config,
    const vm_machine_assets *assets, vm_profile_machine_plan **out_plan);
lib_status vm_profile_machine_plan_create_5170(const vm_machine_config *config,
    const vm_machine_assets *assets, vm_profile_machine_plan **out_plan);

#endif
