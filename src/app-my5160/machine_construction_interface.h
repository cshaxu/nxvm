#ifndef VM_PROFILE_XT_CONSTRUCTION_INTERFACE_H
#define VM_PROFILE_XT_CONSTRUCTION_INTERFACE_H

#include "x86-nxvm/machine/input_interface.h"

lib_status vm_profile_machine_plan_create_xt(const x86_nxvm_machine_config *config,
    const x86_nxvm_machine_assets *assets, x86_nxvm_machine_construction *out_construction);

#endif
