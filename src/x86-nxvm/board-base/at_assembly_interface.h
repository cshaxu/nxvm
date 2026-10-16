#ifndef X86_NXVM_BOARD_AT_ASSEMBLY_INTERFACE_H
#define X86_NXVM_BOARD_AT_ASSEMBLY_INTERFACE_H

#include "x86-nxvm/board-at/wiring_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"

/* Project explicit endpoints into existing copied configs. No machine defaults
 * or memory/video policy are selected. Missing endpoints leave output intact.
 * Topology input supplies display/RTC values; AT display/RTC/DMA are enabled. */
lib_status x86_nxvm_board_at_topology_materialize(const x86_nxvm_board_at_port_leaf *leaves,
    lib_size leaf_count, const x86_nxvm_board_at_route *routes, lib_size route_count,
    x86_nxvm_board_plan_topology *topology);
lib_status x86_nxvm_board_at_fdc_materialize(const x86_nxvm_board_at_port_leaf *leaves,
    lib_size leaf_count, const x86_nxvm_board_at_route *routes, lib_size route_count,
    x86_nxvm_board_fdc_config *fdc);

#endif
