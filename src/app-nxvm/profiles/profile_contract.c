#include "app-nxvm/profiles/profile_contract_interface.h"

static C_INT vm_profile_contract_catalog_contains(
    const vm_profile_contract_catalog *catalog, type_unsigned_32 id)
{
    STD_SIZE_T first;

    if (catalog == STD_NULL || catalog->ids == STD_NULL || id == 0u) return 0;
    for (first = 0u; first < catalog->count; ++first) {
        if (catalog->ids[first] == id) return 1;
    }
    return 0;
}

static C_INT vm_profile_contract_windows_are_valid(
    const vm_profile_contract_window *windows, STD_SIZE_T count, STD_SIZE_T capacity,
    type_unsigned_32 enabled_devices)
{
    STD_SIZE_T first;
    STD_SIZE_T second;

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

static C_INT vm_profile_contract_port_leaves_are_valid(
    const vm_profile_contract_port_leaf *leaves, STD_SIZE_T count,
    type_unsigned_32 enabled_devices)
{
    STD_SIZE_T first;
    STD_SIZE_T second;

    if (count > VM_PROFILE_CONTRACT_PORT_LEAF_CAPACITY) return 0;
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

static C_INT vm_profile_contract_routes_are_valid(
    const vm_profile_contract_route *routes, STD_SIZE_T count, STD_SIZE_T capacity,
    type_unsigned_32 enabled_devices)
{
    STD_SIZE_T first;
    STD_SIZE_T second;

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

type_status vm_profile_contract_validate(const vm_profile_contract_values *values,
    const vm_profile_contract_catalog *catalog, type_unsigned_32 requested_options)
{
    if (values == STD_NULL || catalog == STD_NULL || values->enabled_devices == 0u ||
        values->core.configuration.memory_bytes == 0u ||
        values->core.configuration.cpu_profile == CORE_MACHINE_CPU_PROFILE_DEFAULT ||
        (values->firmware_policy != VM_PROFILE_CONTRACT_FIRMWARE_POLICY_BUILTIN &&
         values->firmware_policy != VM_PROFILE_CONTRACT_FIRMWARE_POLICY_BYOB) ||
        (values->media_policy != VM_PROFILE_CONTRACT_MEDIA_POLICY_NONE &&
         values->media_policy != VM_PROFILE_CONTRACT_MEDIA_POLICY_SESSION) ||
        !vm_profile_contract_catalog_contains(catalog, values->core.id) ||
        (requested_options & ~values->allowed_session_options) != 0u ||
        !vm_profile_contract_port_leaves_are_valid(values->port_leaves,
            values->port_leaf_count, values->enabled_devices) ||
        !vm_profile_contract_windows_are_valid(values->memory_windows,
            values->memory_window_count, VM_PROFILE_CONTRACT_MEMORY_WINDOW_CAPACITY,
            values->enabled_devices) ||
        !vm_profile_contract_routes_are_valid(values->irq_routes,
            values->irq_route_count, VM_PROFILE_CONTRACT_ROUTE_CAPACITY,
            values->enabled_devices) ||
        !vm_profile_contract_routes_are_valid(values->drq_routes,
            values->drq_route_count, VM_PROFILE_CONTRACT_ROUTE_CAPACITY,
            values->enabled_devices)) return TYPE_STATUS_INVALID_ARGUMENT;
    return TYPE_STATUS_OK;
}
