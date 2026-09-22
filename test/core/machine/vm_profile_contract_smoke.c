#include "core/profiles/profile_contract_interface.h"

static const type_unsigned_32 vm_profile_contract_ids[] = {7u};

static vm_profile_contract_values vm_profile_contract_values_create(C_VOID)
{
    vm_profile_contract_values values = {0};

    values.core.id = 7u;
    values.core.configuration.memory_bytes = 1024u;
    values.core.configuration.cpu_profile = CORE_MACHINE_CPU_PROFILE_8086;
    values.enabled_devices = 1u;
    values.port_leaves[0] = (vm_profile_contract_port_leaf) {
        1u, 0x20u, TYPE_TRUE, TYPE_TRUE};
    values.port_leaf_count = 1u;
    values.memory_windows[0] = (vm_profile_contract_window) {0u, 0xffu, 1u};
    values.memory_window_count = 1u;
    values.irq_routes[0] = (vm_profile_contract_route) {1u, 5u};
    values.irq_route_count = 1u;
    values.drq_routes[0] = (vm_profile_contract_route) {1u, 2u};
    values.drq_route_count = 1u;
    values.firmware_policy = VM_PROFILE_CONTRACT_FIRMWARE_POLICY_BUILTIN;
    values.media_policy = VM_PROFILE_CONTRACT_MEDIA_POLICY_SESSION;
    values.allowed_session_options = 1u;
    return values;
}

static C_INT vm_profile_contract_accepts_direct_values(C_VOID)
{
    const vm_profile_contract_catalog catalog = {vm_profile_contract_ids,
        sizeof(vm_profile_contract_ids) / sizeof(vm_profile_contract_ids[0])};
    const vm_profile_contract_values values = vm_profile_contract_values_create();

    return vm_profile_contract_validate(&values, &catalog, 1u) != TYPE_STATUS_OK;
}

static C_INT vm_profile_contract_rejects_invalid_values(C_VOID)
{
    const vm_profile_contract_catalog catalog = {vm_profile_contract_ids,
        sizeof(vm_profile_contract_ids) / sizeof(vm_profile_contract_ids[0])};
    vm_profile_contract_values values = vm_profile_contract_values_create();

    values.port_leaves[0].device = 2u;
    if (vm_profile_contract_validate(&values, &catalog, 1u) == TYPE_STATUS_OK) return 1;
    values = vm_profile_contract_values_create();
    values.irq_route_count = VM_PROFILE_CONTRACT_ROUTE_CAPACITY + 1u;
    if (vm_profile_contract_validate(&values, &catalog, 1u) == TYPE_STATUS_OK) return 1;
    values = vm_profile_contract_values_create();
    values.port_leaves[1] = values.port_leaves[0];
    values.port_leaf_count = 2u;
    if (vm_profile_contract_validate(&values, &catalog, 1u) == TYPE_STATUS_OK) return 1;
    values = vm_profile_contract_values_create();
    values.irq_routes[1] = values.irq_routes[0];
    values.irq_route_count = 2u;
    if (vm_profile_contract_validate(&values, &catalog, 1u) == TYPE_STATUS_OK) return 1;
    values = vm_profile_contract_values_create();
    values.memory_windows[1] = (vm_profile_contract_window) {0x80u, 0x17fu, 1u};
    values.memory_window_count = 2u;
    if (vm_profile_contract_validate(&values, &catalog, 1u) == TYPE_STATUS_OK) return 1;
    values = vm_profile_contract_values_create();
    values.core.id = 0u;
    if (vm_profile_contract_validate(&values, &catalog, 1u) == TYPE_STATUS_OK) return 1;
    values = vm_profile_contract_values_create();
    return vm_profile_contract_validate(&values, &catalog, 2u) == TYPE_STATUS_OK;
}

C_INT main(C_VOID)
{
    return vm_profile_contract_accepts_direct_values() ||
        vm_profile_contract_rejects_invalid_values();
}
