#ifndef MY5170_CONSTRUCTION_INTERFACE_H
#define MY5170_CONSTRUCTION_INTERFACE_H

#include "core/machine/input_interface.h"

lib_status vm_profile_machine_plan_create_5170(const vm_machine_config *config,
    const vm_machine_assets *assets, vm_machine_construction *out_construction);

#endif
