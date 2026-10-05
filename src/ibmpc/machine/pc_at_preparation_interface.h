#ifndef VM_PC_AT_PREPARATION_INTERFACE_H
#define VM_PC_AT_PREPARATION_INTERFACE_H

#include "ibmpc/machine/input_interface.h"
#include "ibmpc/board-common/pc_at_profile_interface.h"

/* App fills copied board choices in one owned candidate. It retains neither
 * output pointer; the shared owner prepares ROM bytes and finishes publication. */
typedef lib_status (*vm_pc_at_profile_prepare)(const vm_machine_config *config,
    vm_profile_default_pc_at_plan_snapshot *profile,
    vm_machine_construction *construction);
lib_status vm_pc_at_construction_create(const vm_machine_config *config,
    const vm_machine_assets *assets, vm_pc_at_profile_prepare prepare,
    vm_machine_construction *out_construction);

#endif
