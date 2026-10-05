#ifndef VM_PROFILE_XT_CONSTRUCTION_INTERFACE_H
#define VM_PROFILE_XT_CONSTRUCTION_INTERFACE_H

#include "ibmpc/machine/input_interface.h"

lib_status vm_profile_machine_plan_create_xt(const vm_machine_config *config,
    const vm_machine_assets *assets, vm_machine_construction *out_construction);

#endif
