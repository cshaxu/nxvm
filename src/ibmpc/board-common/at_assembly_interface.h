#ifndef VM_AT_ASSEMBLY_INTERFACE_H
#define VM_AT_ASSEMBLY_INTERFACE_H

#include "ibmpc/board-at/wiring_interface.h"
#include "ibmpc/board-common/machine_board_interface.h"

/* Project explicit endpoints into existing copied configs. No machine defaults
 * or memory/video policy are selected. Missing endpoints leave output intact.
 * Topology input supplies display/RTC values; AT display/RTC/DMA are enabled. */
lib_status vm_at_topology_materialize(const vm_at_port_leaf *leaves,
    lib_size leaf_count, const vm_at_route *routes, lib_size route_count,
    core_machine_plan_topology *topology);
lib_status vm_at_fdc_materialize(const vm_at_port_leaf *leaves,
    lib_size leaf_count, const vm_at_route *routes, lib_size route_count,
    core_machine_fdc_config *fdc);

#endif
