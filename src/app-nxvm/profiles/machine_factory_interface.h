#ifndef VM_PROFILE_MACHINE_FACTORY_INTERFACE_H
#define VM_PROFILE_MACHINE_FACTORY_INTERFACE_H

#include "app-nxvm/profiles/selection_interface.h"
#include "app-nxvm/profiles/machine_plan_interface.h"
#include "ibmpc/machine/machine_interface.h"

/* Resolve actual App firmware/topology before the neutral Machine takes
 * ownership. Failure leaves the output empty and destroys the candidate. */
lib_status vm_machine_create_from_assets(const vm_machine_config *config,
    const vm_machine_assets *assets, vm_machine **out_machine);
/* A prepared plan transfers on valid entry, including failed publication.
 * Tests select real constructors explicitly; products use the fixed binding. */
lib_status vm_machine_create_from_plan(const vm_machine_config *config,
    vm_profile_machine_plan *plan, vm_machine **out_machine);

#endif
