#ifndef VM_PROFILE_MODEL40_COMPOSITION_INTERFACE_H
#define VM_PROFILE_MODEL40_COMPOSITION_INTERFACE_H

#include "type.h"
#include "core/devices/machine_interface.h"
type_status vm_profile_model40_topology_materialize(
    core_machine_plan_topology *out_topology);
type_status vm_profile_model40_materialize_plan(core_machine_plan *plan,
    core_machine_fdc_terminal_observation_provider terminal_observation);

#endif
