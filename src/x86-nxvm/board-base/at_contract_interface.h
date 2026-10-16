#ifndef X86_NXVM_BOARD_AT_CONTRACT_INTERFACE_H
#define X86_NXVM_BOARD_AT_CONTRACT_INTERFACE_H

#include "x86-nxvm/board-at/wiring_interface.h"
#include "x86-nxvm/board-base/profile_contract_interface.h"

/* Project explicit AT endpoints and complete Core input into one copied
 * electrical contract. No model defaults or firmware/media policy are chosen.
 * Failure clears output; inputs must not alias it. App validates its final
 * policy/options with x86_nxvm_board_contract_validate before publication. */
lib_status x86_nxvm_board_at_contract_materialize(const x86_nxvm_board_contract_core_input *core,
    const x86_nxvm_board_at_port_leaf *leaves, lib_size leaf_count,
    const x86_nxvm_board_at_route *routes, lib_size route_count, lib_u32 enabled_devices,
    lib_bool cga_vram_present, x86_nxvm_board_contract_values *out_values);

#endif
