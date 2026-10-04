#ifndef VM_PROFILE_MACHINE_FACTORY_INTERFACE_H
#define VM_PROFILE_MACHINE_FACTORY_INTERFACE_H

#include "app-nxvm/profiles/selection_interface.h"
#include "x86/product/machine/machine_interface.h"

/* Resolve actual App firmware/topology before the neutral Machine takes
 * ownership. Failure leaves the output empty and destroys the candidate. */
lib_status vm_machine_create_from_assets(const vm_machine_config *config,
    const vm_machine_assets *assets, vm_machine **out_machine);

#endif
