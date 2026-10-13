#ifndef MYDESKPRO386_MACHINE_PLAN_INTERFACE_H
#define MYDESKPRO386_MACHINE_PLAN_INTERFACE_H

#include "core/machine/input_interface.h"

lib_status mydeskpro386_machine_plan_create(const vm_machine_config *config,
    const vm_machine_assets *assets, vm_machine_construction *out_construction);

#endif
