#ifndef VM_AT_CONTRACT_INTERFACE_H
#define VM_AT_CONTRACT_INTERFACE_H

#include "core/board-at/wiring_interface.h"
#include "core/board-base/profile_contract_interface.h"

/* Project explicit AT endpoints and complete Core input into one copied
 * electrical contract. No model defaults or firmware/media policy are chosen.
 * Failure clears output; inputs must not alias it. App validates its final
 * policy/options with vm_profile_contract_validate before publication. */
lib_status vm_at_contract_materialize(const vm_profile_contract_core_input *core,
    const vm_at_port_leaf *leaves, lib_size leaf_count,
    const vm_at_route *routes, lib_size route_count, lib_u32 enabled_devices,
    lib_bool cga_vram_present, vm_profile_contract_values *out_values);

#endif
