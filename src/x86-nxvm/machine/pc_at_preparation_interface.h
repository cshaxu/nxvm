#ifndef X86_NXVM_MACHINE_PC_AT_PREPARATION_INTERFACE_H
#define X86_NXVM_MACHINE_PC_AT_PREPARATION_INTERFACE_H

#include "x86-nxvm/machine/input_interface.h"
#include "x86-nxvm/board-base/pc_at_profile_interface.h"

/* App fills copied board choices in one owned candidate. It retains neither
 * output pointer; the shared owner prepares ROM bytes and finishes publication. */
typedef lib_status (*x86_nxvm_machine_pc_at_profile_prepare)(const x86_nxvm_machine_config *config,
    x86_nxvm_board_pc_at_plan_snapshot *profile,
    x86_nxvm_machine_construction *construction);
lib_status x86_nxvm_machine_pc_at_construction_create(const x86_nxvm_machine_config *config,
    const x86_nxvm_machine_assets *assets, x86_nxvm_machine_pc_at_profile_prepare prepare,
    x86_nxvm_machine_construction *out_construction);

#endif
