#ifndef VM_PROFILE_MODEL40_COMPOSITION_INTERFACE_H
#define VM_PROFILE_MODEL40_COMPOSITION_INTERFACE_H
#include "lib/types/types_interface.h"

#include "ibmpc/board-common/machine_board_interface.h"
#include "app-nxvm/profiles/model40/d4_platform_interface.h"
lib_status vm_profile_model40_topology_materialize(
    core_machine_plan_topology *out_topology);
lib_status vm_profile_model40_materialize_plan(core_machine_plan *plan,
    core_machine_fdc_terminal_observation_provider terminal_observation,
    core_machine_d4_platform **construction_output);

#endif
