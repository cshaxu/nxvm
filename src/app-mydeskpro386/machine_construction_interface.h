#ifndef MYDESKPRO386_MACHINE_CONSTRUCTION_INTERFACE_H
#define MYDESKPRO386_MACHINE_CONSTRUCTION_INTERFACE_H
#include "lib/types/types_interface.h"

#include "x86-nxvm/board-base/machine_board_interface.h"
#include "app-mydeskpro386/d4_platform_interface.h"
lib_status mydeskpro386_topology_materialize(
    x86_nxvm_board_plan_topology *out_topology);
lib_status mydeskpro386_materialize_plan(x86_nxvm_board_plan *plan,
    x86_nxvm_board_fdc_terminal_observation_provider terminal_observation,
    mydeskpro386_d4_platform **construction_output);

#endif
