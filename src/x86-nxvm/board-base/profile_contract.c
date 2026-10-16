#include "lib/types/types_interface.h"
#include "x86-nxvm/board-base/profile_contract_interface.h"
#include "x86-nxvm/board-base/at_contract_interface.h"

static lib_i32 x86_nxvm_board_contract_catalog_contains(
    const x86_nxvm_board_contract_catalog *catalog, lib_u32 id)
{
    lib_size first;

    if (catalog == LIB_NULL || catalog->ids == LIB_NULL || id == 0u) return 0;
    for (first = 0u; first < catalog->count; ++first) {
        if (catalog->ids[first] == id) return 1;
    }
    return 0;
}

static lib_i32 x86_nxvm_board_contract_windows_are_valid(
    const x86_nxvm_board_contract_window *windows, lib_size count, lib_size capacity,
    lib_u32 enabled_devices)
{
    lib_size first;
    lib_size second;

    if (count > capacity) return 0;
    for (first = 0u; first < count; ++first) {
        if (windows[first].first > windows[first].last || windows[first].device == 0u ||
            (windows[first].device & ~enabled_devices) != 0u) return 0;
        for (second = first + 1u; second < count; ++second) {
            if (windows[first].first <= windows[second].last &&
                windows[second].first <= windows[first].last) return 0;
        }
    }
    return 1;
}

static lib_i32 x86_nxvm_board_contract_port_leaves_are_valid(
    const x86_nxvm_board_contract_port_leaf *leaves, lib_size count,
    lib_u32 enabled_devices)
{
    lib_size first;
    lib_size second;

    if (count > X86_NXVM_BOARD_CONTRACT_PORT_LEAF_CAPACITY) return 0;
    for (first = 0u; first < count; ++first) {
        if (leaves[first].device == 0u ||
            (leaves[first].device & ~enabled_devices) != 0u ||
            (!leaves[first].read && !leaves[first].write)) return 0;
        for (second = first + 1u; second < count; ++second) {
            if (leaves[first].port == leaves[second].port) return 0;
        }
    }
    return 1;
}

static lib_i32 x86_nxvm_board_contract_routes_are_valid(
    const x86_nxvm_board_contract_route *routes, lib_size count, lib_size capacity,
    lib_u32 enabled_devices)
{
    lib_size first;
    lib_size second;

    if (count > capacity) return 0;
    for (first = 0u; first < count; ++first) {
        if (routes[first].device == 0u ||
            (routes[first].device & ~enabled_devices) != 0u) return 0;
        for (second = first + 1u; second < count; ++second) {
            if (routes[first].line == routes[second].line) return 0;
        }
    }
    return 1;
}

static lib_bool x86_nxvm_board_contract_wiring_is_valid(const x86_nxvm_board_contract_values *values)
{
    return values->enabled_devices != 0u &&
        values->core.configuration.memory_bytes != 0u &&
        values->core.configuration.cpu_profile != X86_CORE_CPU_PROFILE_DEFAULT &&
        x86_nxvm_board_contract_port_leaves_are_valid(values->port_leaves,
            values->port_leaf_count, values->enabled_devices) &&
        x86_nxvm_board_contract_windows_are_valid(values->memory_windows,
            values->memory_window_count, X86_NXVM_BOARD_CONTRACT_MEMORY_WINDOW_CAPACITY,
            values->enabled_devices) &&
        x86_nxvm_board_contract_routes_are_valid(values->irq_routes,
            values->irq_route_count, X86_NXVM_BOARD_CONTRACT_ROUTE_CAPACITY,
            values->enabled_devices) &&
        x86_nxvm_board_contract_routes_are_valid(values->drq_routes,
            values->drq_route_count, X86_NXVM_BOARD_CONTRACT_ROUTE_CAPACITY,
            values->enabled_devices);
}

static lib_status x86_nxvm_board_at_route_device(x86_nxvm_board_at_route_source source, x86_nxvm_board_at_device_role *out_device)
{
    switch (source) {
    case X86_NXVM_BOARD_AT_ROUTE_PIT_IRQ0: *out_device = X86_NXVM_BOARD_AT_DEVICE_PIT; break;
    case X86_NXVM_BOARD_AT_ROUTE_KBC_KEYBOARD_IRQ1:
    case X86_NXVM_BOARD_AT_ROUTE_KBC_AUX_IRQ12: *out_device = X86_NXVM_BOARD_AT_DEVICE_KBC; break;
    case X86_NXVM_BOARD_AT_ROUTE_CMOS_IRQ8: *out_device = X86_NXVM_BOARD_AT_DEVICE_CMOS; break;
    case X86_NXVM_BOARD_AT_ROUTE_FDC_IRQ6_DMA2: *out_device = X86_NXVM_BOARD_AT_DEVICE_FDC; break;
    default: return LIB_STATUS_INVALID_ARGUMENT;
    }
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_board_at_contract_materialize(const x86_nxvm_board_contract_core_input *core,
    const x86_nxvm_board_at_port_leaf *leaves, lib_size leaf_count,
    const x86_nxvm_board_at_route *routes, lib_size route_count, lib_u32 enabled_devices,
    lib_bool cga_vram_present, x86_nxvm_board_contract_values *out_values)
{
    x86_nxvm_board_contract_values values = {0};

    if (out_values == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_values = (x86_nxvm_board_contract_values){0};
    if (core == LIB_NULL || leaves == LIB_NULL || routes == LIB_NULL ||
        leaf_count > X86_NXVM_BOARD_CONTRACT_PORT_LEAF_CAPACITY ||
        route_count > X86_NXVM_BOARD_CONTRACT_ROUTE_CAPACITY ||
        (enabled_devices >> (X86_NXVM_BOARD_AT_DEVICE_BOARD + 1u)) != 0u) return LIB_STATUS_INVALID_ARGUMENT;
    values.core = *core;
    for (lib_size role = 0u; role <= X86_NXVM_BOARD_AT_DEVICE_BOARD; ++role) {
        for (lib_size index = 0u; index < leaf_count; ++index) {
            const x86_nxvm_board_at_port_leaf *leaf = &leaves[index];
            if (leaf->device < X86_NXVM_BOARD_AT_DEVICE_PIC || leaf->device > X86_NXVM_BOARD_AT_DEVICE_BOARD)
                return LIB_STATUS_INVALID_ARGUMENT;
            if ((lib_size)leaf->device != role || (enabled_devices & (1u << role)) == 0u) continue;
            values.enabled_devices |= 1u << role;
            values.port_leaves[values.port_leaf_count++] = (x86_nxvm_board_contract_port_leaf) {
                1u << role, leaf->port, leaf->read, leaf->write};
        }
    }
    if (cga_vram_present) {
        values.memory_windows[0] = (x86_nxvm_board_contract_window) {
            0x000b8000u, 0x000bffffu, 1u << X86_NXVM_BOARD_AT_DEVICE_VADP};
        values.memory_window_count = 1u;
    }
    for (lib_size index = 0u; index < route_count; ++index) {
        const x86_nxvm_board_at_route *route = &routes[index];
        x86_nxvm_board_at_device_role role;
        if (x86_nxvm_board_at_route_device(route->source, &role) != LIB_STATUS_OK)
            return LIB_STATUS_INVALID_ARGUMENT;
        const lib_u32 device = 1u << role;
        values.irq_routes[values.irq_route_count++] = (x86_nxvm_board_contract_route){device, route->irq};
        if (route->dma_channel != X86_NXVM_BOARD_AT_NO_DMA_CHANNEL)
            values.drq_routes[values.drq_route_count++] = (x86_nxvm_board_contract_route){device, route->dma_channel};
    }
    if (!x86_nxvm_board_contract_wiring_is_valid(&values)) return LIB_STATUS_INVALID_ARGUMENT;
    *out_values = values;
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_board_contract_validate(const x86_nxvm_board_contract_values *values,
    const x86_nxvm_board_contract_catalog *catalog, lib_u32 requested_options)
{
    if (values == LIB_NULL || catalog == LIB_NULL || !x86_nxvm_board_contract_wiring_is_valid(values) ||
        (values->firmware_policy != X86_NXVM_BOARD_CONTRACT_FIRMWARE_POLICY_BUILTIN &&
         values->firmware_policy != X86_NXVM_BOARD_CONTRACT_FIRMWARE_POLICY_BYOB) ||
        (values->media_policy != X86_NXVM_BOARD_CONTRACT_MEDIA_POLICY_NONE &&
         values->media_policy != X86_NXVM_BOARD_CONTRACT_MEDIA_POLICY_SESSION) ||
        !x86_nxvm_board_contract_catalog_contains(catalog, values->core.id) ||
        (requested_options & ~values->allowed_session_options) != 0u) return LIB_STATUS_INVALID_ARGUMENT;
    return LIB_STATUS_OK;
}
