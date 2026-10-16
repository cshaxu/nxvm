#include "x86-nxvm/board-base/at_assembly_interface.h"

lib_status x86_nxvm_board_at_topology_materialize(const x86_nxvm_board_at_port_leaf *leaves,
    lib_size leaf_count, const x86_nxvm_board_at_route *routes, lib_size route_count,
    x86_nxvm_board_plan_topology *topology)
{
    const x86_nxvm_board_at_device_role roles[] = { X86_NXVM_BOARD_AT_DEVICE_VADP_ATTRIBUTE,
        X86_NXVM_BOARD_AT_DEVICE_VADP_SEQUENCER, X86_NXVM_BOARD_AT_DEVICE_VADP_GRAPHICS,
        X86_NXVM_BOARD_AT_DEVICE_VADP, X86_NXVM_BOARD_AT_DEVICE_CMOS };
    const lib_size ordinals[] = {0u, 1u, 1u, 4u, 1u};
    const x86_nxvm_board_at_port_leaf *first[5u];
    const x86_nxvm_board_at_port_leaf *last[5u];
    const x86_nxvm_board_at_route *rtc = x86_nxvm_board_at_route_find(routes, route_count, X86_NXVM_BOARD_AT_ROUTE_CMOS_IRQ8);
    const x86_nxvm_board_at_route *fdc = x86_nxvm_board_at_route_find(routes, route_count, X86_NXVM_BOARD_AT_ROUTE_FDC_IRQ6_DMA2);
    x86_nxvm_board_plan_topology result;

    if (topology == LIB_NULL || rtc == LIB_NULL || fdc == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    result = *topology;
    for (lib_size index = 0u; index < 5u; ++index) {
        if (index < 3u && !result.display.ega_present) {
            first[index] = last[index] = LIB_NULL;
            continue;
        }
        first[index] = x86_nxvm_board_at_port_leaf_at(leaves, leaf_count, X86_NXVM_BOARD_AT_DEVICE_MASK_ALL,
            roles[index], 0u);
        last[index] = x86_nxvm_board_at_port_leaf_at(leaves, leaf_count, X86_NXVM_BOARD_AT_DEVICE_MASK_ALL,
            roles[index], ordinals[index]);
        if ((index >= 3u || result.display.ega_present) &&
            (first[index] == LIB_NULL || last[index] == LIB_NULL))
            return LIB_STATUS_INVALID_ARGUMENT;
    }
    result.display.ports = (x86_nxvm_board_display_port_topology) {
        first[0] == LIB_NULL ? 0u : first[0]->port,
        /* Adjacent read-data address is part of the range, not an EGA route. */
        first[0] == LIB_NULL ? 0u : (lib_u16)(first[0]->port + 1u),
        first[1] == LIB_NULL ? 0u : first[1]->port,
        last[1] == LIB_NULL ? 0u : last[1]->port,
        first[2] == LIB_NULL ? 0u : first[2]->port,
        last[2] == LIB_NULL ? 0u : last[2]->port,
        first[3]->port, last[3]->port };
    result.display_present = LIB_TRUE;
    result.rtc_cmos.index_port = first[4]->port;
    result.rtc_cmos.data_port = last[4]->port;
    result.rtc_cmos.irq = rtc->irq;
    result.rtc_cmos_present = LIB_TRUE;
    result.dma = (x86_nxvm_board_dma_wiring) {fdc->dma_channel,
        X86_NXVM_BOARD_DMA_CONTROLLER_COUNT, X86_NXVM_BOARD_DMA_CASCADE_CHANNEL};
    result.dma_present = LIB_TRUE;
    *topology = result;
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_board_at_fdc_materialize(const x86_nxvm_board_at_port_leaf *leaves,
    lib_size leaf_count, const x86_nxvm_board_at_route *routes, lib_size route_count,
    x86_nxvm_board_fdc_config *fdc)
{
    const x86_nxvm_board_at_port_leaf *ports[4u];
    const x86_nxvm_board_at_route *route = x86_nxvm_board_at_route_find(routes, route_count,
        X86_NXVM_BOARD_AT_ROUTE_FDC_IRQ6_DMA2);
    x86_nxvm_board_fdc_config result;

    if (fdc == LIB_NULL || route == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    for (lib_size index = 0u; index < 4u; ++index) {
        ports[index] = x86_nxvm_board_at_port_leaf_at(leaves, leaf_count, X86_NXVM_BOARD_AT_DEVICE_MASK_ALL,
            X86_NXVM_BOARD_AT_DEVICE_FDC, index);
        if (ports[index] == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    }
    result = *fdc;
    result.dor_port = ports[0]->port;
    result.status_port = ports[1]->port;
    result.data_port = ports[2]->port;
    result.direction_port = ports[3]->port;
    result.control_port = ports[3]->port;
    result.irq = route->irq;
    result.dma_channel = route->dma_channel;
    *fdc = result;
    return LIB_STATUS_OK;
}
