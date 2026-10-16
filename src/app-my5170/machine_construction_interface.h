#ifndef MY5170_CONSTRUCTION_INTERFACE_H
#define MY5170_CONSTRUCTION_INTERFACE_H

#include "x86-nxvm/machine/input_interface.h"

lib_status vm_profile_machine_plan_create_5170(const x86_nxvm_machine_config *config,
    const x86_nxvm_machine_assets *assets, x86_nxvm_machine_construction *out_construction);

#endif
