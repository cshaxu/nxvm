#include "ibmpc/board-common/at_assembly_interface.h"

lib_status vm_at_topology_materialize(const vm_at_port_leaf *leaves,
    lib_size leaf_count, const vm_at_route *routes, lib_size route_count,
    core_machine_plan_topology *topology)
{
    const vm_at_device_role roles[] = { VM_AT_DEVICE_VADP_ATTRIBUTE,
        VM_AT_DEVICE_VADP_SEQUENCER, VM_AT_DEVICE_VADP_GRAPHICS,
        VM_AT_DEVICE_VADP, VM_AT_DEVICE_CMOS };
    const lib_size ordinals[] = {1u, 1u, 1u, 4u, 1u};
    const vm_at_port_leaf *first[5u];
    const vm_at_port_leaf *last[5u];
    const vm_at_route *rtc = vm_at_route_find(routes, route_count, VM_AT_ROUTE_CMOS_IRQ8);
    const vm_at_route *fdc = vm_at_route_find(routes, route_count, VM_AT_ROUTE_FDC_IRQ6_DMA2);
    core_machine_plan_topology result;

    if (topology == LIB_NULL || rtc == LIB_NULL || fdc == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    result = *topology;
    for (lib_size index = 0u; index < 5u; ++index) {
        if (index < 3u && !result.display.ega_present) {
            first[index] = last[index] = LIB_NULL;
            continue;
        }
        first[index] = vm_at_port_leaf_at(leaves, leaf_count, VM_AT_DEVICE_MASK_ALL,
            roles[index], 0u);
        last[index] = vm_at_port_leaf_at(leaves, leaf_count, VM_AT_DEVICE_MASK_ALL,
            roles[index], ordinals[index]);
        if ((index >= 3u || result.display.ega_present) &&
            (first[index] == LIB_NULL || last[index] == LIB_NULL))
            return LIB_STATUS_INVALID_ARGUMENT;
    }
    result.display.ports = (core_machine_display_port_topology) {
        first[0] == LIB_NULL ? 0u : first[0]->port,
        last[0] == LIB_NULL ? 0u : last[0]->port,
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
    result.dma = (core_machine_dma_wiring) {fdc->dma_channel,
        CORE_MACHINE_DMA_CONTROLLER_COUNT, CORE_MACHINE_DMA_CASCADE_CHANNEL};
    result.dma_present = LIB_TRUE;
    *topology = result;
    return LIB_STATUS_OK;
}

lib_status vm_at_fdc_materialize(const vm_at_port_leaf *leaves,
    lib_size leaf_count, const vm_at_route *routes, lib_size route_count,
    core_machine_fdc_config *fdc)
{
    const vm_at_port_leaf *ports[4u];
    const vm_at_route *route = vm_at_route_find(routes, route_count,
        VM_AT_ROUTE_FDC_IRQ6_DMA2);
    core_machine_fdc_config result;

    if (fdc == LIB_NULL || route == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    for (lib_size index = 0u; index < 4u; ++index) {
        ports[index] = vm_at_port_leaf_at(leaves, leaf_count, VM_AT_DEVICE_MASK_ALL,
            VM_AT_DEVICE_FDC, index);
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
