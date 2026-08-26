#include "type.h"

#include "core/machine/machine_interface.h"
#include "vm/profile/profile_resolver_interface.h"

static const type_unsigned_32 vm_profile_resolver_contracts[] = {7u};

static vm_profile_resolver_declaration vm_profile_resolver_root(void)
{
    vm_profile_resolver_declaration declaration = {0};

    declaration.identity = "neutral-root";
    declaration.provided_fields = VM_PROFILE_RESOLVER_FIELD_ALL;
    declaration.owned_fields = VM_PROFILE_RESOLVER_FIELD_ALL;
    declaration.values.core.contract_id = 7u;
    declaration.values.core.configuration.memory_bytes = 1024u * 1024u;
    declaration.values.core.configuration.cpu_profile = CORE_MACHINE_CPU_PROFILE_80286;
    declaration.values.enabled_devices = 1u;
    declaration.values.port_leaves[0] =
        (vm_profile_resolver_port_leaf) {1u, 0x20u, TYPE_TRUE, TYPE_TRUE};
    declaration.values.port_leaf_count = 1u;
    declaration.values.memory_windows[0] = (vm_profile_resolver_window) {
        0x000a0000u, 0x000affffu, 1u};
    declaration.values.memory_window_count = 1u;
    declaration.values.irq_routes[0] = (vm_profile_resolver_route) {1u, 5u};
    declaration.values.irq_route_count = 1u;
    declaration.values.drq_routes[0] = (vm_profile_resolver_route) {1u, 2u};
    declaration.values.drq_route_count = 1u;
    declaration.values.firmware_policy = VM_PROFILE_RESOLVER_FIRMWARE_POLICY_BUILTIN;
    declaration.values.media_policy = VM_PROFILE_RESOLVER_MEDIA_POLICY_SESSION;
    declaration.values.allowed_session_options = 3u;
    return declaration;
}

static C_INT vm_profile_resolver_resolves_copy(void)
{
    vm_profile_resolver_declaration root = vm_profile_resolver_root();
    vm_profile_resolver_declaration child = {0};
    const vm_profile_resolver_contract_catalog catalog = {
        vm_profile_resolver_contracts, sizeof(vm_profile_resolver_contracts) /
            sizeof(vm_profile_resolver_contracts[0])};
    const vm_profile_resolver_session_request request = {1u};
    vm_resolved_profile result;

    child.identity = "neutral-child";
    child.parent = &root;
    child.provided_fields = VM_PROFILE_RESOLVER_FIELD_CORE;
    child.owned_fields = VM_PROFILE_RESOLVER_FIELD_CORE;
    child.values.core = root.values.core;
    child.values.core.configuration.memory_bytes = 2u * 1024u * 1024u;
    if (vm_profile_resolver_resolve(&child, &catalog, &request, &result) !=
        TYPE_STATUS_OK || STD_STRCMP(result.identity, "neutral-child") ||
        STD_STRCMP(result.parent_identity, "neutral-root") ||
        STD_STRCMP(result.field_owner[0], "neutral-child") ||
        STD_STRCMP(result.field_owner[6], "neutral-root") ||
        result.values.core.configuration.memory_bytes != 2u * 1024u * 1024u ||
        result.values.firmware_policy != root.values.firmware_policy) return 1;
    root.values.core.configuration.memory_bytes = 4u * 1024u * 1024u;
    return result.values.core.configuration.memory_bytes != 2u * 1024u * 1024u;
}

static C_INT vm_profile_resolver_hands_off_copied_core_input(void)
{
    vm_profile_resolver_declaration root = vm_profile_resolver_root();
    const vm_profile_resolver_contract_catalog catalog = {
        vm_profile_resolver_contracts, sizeof(vm_profile_resolver_contracts) /
            sizeof(vm_profile_resolver_contracts[0])};
    const vm_profile_resolver_session_request request = {1u};
    vm_resolved_profile result;
    core_machine_plan *plan = STD_NULL;
    type_status status;

    if (vm_profile_resolver_resolve(&root, &catalog, &request, &result) !=
        TYPE_STATUS_OK) return 1;
    status = core_machine_plan_create(&result.values.core.configuration, &plan);
    if (status == TYPE_STATUS_OK) {
        status = core_machine_plan_set_controller_timing_rules(plan,
            &result.values.core.controller_timing_rules);
    }
    core_machine_plan_destroy(plan);
    return status != TYPE_STATUS_OK;
}

static C_INT vm_profile_resolver_rejects_invalid(void)
{
    vm_profile_resolver_declaration root = vm_profile_resolver_root();
    vm_profile_resolver_declaration child = {0};
    const vm_profile_resolver_contract_catalog catalog = {
        vm_profile_resolver_contracts, sizeof(vm_profile_resolver_contracts) /
            sizeof(vm_profile_resolver_contracts[0])};
    const vm_profile_resolver_session_request request = {1u};
    vm_resolved_profile result;

    child.identity = "invalid-owner";
    child.parent = &root;
    child.owned_fields = VM_PROFILE_RESOLVER_FIELD_CORE;
    if (vm_profile_resolver_resolve(&child, &catalog, &request, &result) ==
        TYPE_STATUS_OK) return 1;
    root.values.port_leaves[0].device = 2u;
    if (vm_profile_resolver_resolve(&root, &catalog, &request, &result) ==
        TYPE_STATUS_OK) return 1;
    root = vm_profile_resolver_root();
    root.values.port_leaves[1] =
        (vm_profile_resolver_port_leaf) {1u, 0x20u, TYPE_TRUE, TYPE_TRUE};
    root.values.port_leaf_count = 2u;
    if (vm_profile_resolver_resolve(&root, &catalog, &request, &result) ==
        TYPE_STATUS_OK) return 1;
    root = vm_profile_resolver_root();
    root.values.port_leaves[0].read = TYPE_FALSE;
    root.values.port_leaves[0].write = TYPE_FALSE;
    if (vm_profile_resolver_resolve(&root, &catalog, &request, &result) ==
        TYPE_STATUS_OK) return 1;
    root = vm_profile_resolver_root();
    root.values.irq_routes[1] = (vm_profile_resolver_route) {1u, 5u};
    root.values.irq_route_count = 2u;
    if (vm_profile_resolver_resolve(&root, &catalog, &request, &result) ==
        TYPE_STATUS_OK) return 1;
    root = vm_profile_resolver_root();
    root.values.drq_routes[1] = (vm_profile_resolver_route) {1u, 2u};
    root.values.drq_route_count = 2u;
    if (vm_profile_resolver_resolve(&root, &catalog, &request, &result) ==
        TYPE_STATUS_OK) return 1;
    root = vm_profile_resolver_root();
    child.identity = "middle";
    child.parent = &root;
    child.provided_fields = VM_PROFILE_RESOLVER_FIELD_POLICY;
    child.owned_fields = VM_PROFILE_RESOLVER_FIELD_POLICY;
    child.values = root.values;
    root.parent = &child;
    if (vm_profile_resolver_resolve(&root, &catalog, &request, &result) ==
        TYPE_STATUS_OK) return 1;
    root = vm_profile_resolver_root();
    root.values.core.contract_id = 8u;
    if (vm_profile_resolver_resolve(&root, &catalog, &request, &result) ==
        TYPE_STATUS_OK) return 1;
    root = vm_profile_resolver_root();
    root.values.firmware_policy = 0u;
    if (vm_profile_resolver_resolve(&root, &catalog, &request, &result) ==
        TYPE_STATUS_OK) return 1;
    root = vm_profile_resolver_root();
    return vm_profile_resolver_resolve(&root, &catalog,
        &(vm_profile_resolver_session_request) {4u}, &result) == TYPE_STATUS_OK;
}

int main(void)
{
    if (vm_profile_resolver_resolves_copy() ||
        vm_profile_resolver_hands_off_copied_core_input() ||
        vm_profile_resolver_rejects_invalid()) {
        return 1;
    }
    puts("M5:T475:S2:PROFILE-RESOLVER:OK");
    puts("M5:T475:S2:PROFILE-RESOLVER-NEGATIVE:OK");
    return 0;
}
