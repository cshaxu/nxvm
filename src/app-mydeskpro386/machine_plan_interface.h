#ifndef MYDESKPRO386_MACHINE_PLAN_INTERFACE_H
#define MYDESKPRO386_MACHINE_PLAN_INTERFACE_H

#include "x86-nxvm/machine/input_interface.h"

lib_status mydeskpro386_machine_plan_create(const x86_nxvm_machine_config *config,
    const x86_nxvm_machine_assets *assets, x86_nxvm_machine_construction *out_construction);

#endif
