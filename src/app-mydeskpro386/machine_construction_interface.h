#ifndef MYDESKPRO386_MACHINE_CONSTRUCTION_INTERFACE_H
#define MYDESKPRO386_MACHINE_CONSTRUCTION_INTERFACE_H
#include "lib/types/types_interface.h"

#include "core/board-base/machine_board_interface.h"
#include "app-mydeskpro386/d4_platform_interface.h"
lib_status mydeskpro386_topology_materialize(
    core_machine_plan_topology *out_topology);
lib_status mydeskpro386_materialize_plan(core_machine_plan *plan,
    core_machine_fdc_terminal_observation_provider terminal_observation,
    mydeskpro386_d4_platform **construction_output);

#endif
