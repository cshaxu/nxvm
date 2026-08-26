#include "vm/profile/profile_resolver_interface.h"

static C_INT vm_profile_resolver_copy_identity(C_CHAR destination[
    VM_PROFILE_RESOLVER_IDENTITY_CAPACITY], const C_CHAR *source)
{
    STD_SIZE_T length;

    if (source == STD_NULL) return 0;
    length = STD_STRLEN(source);
    if (length == 0u || length >= VM_PROFILE_RESOLVER_IDENTITY_CAPACITY) return 0;
    STD_MEMCPY(destination, source, length + 1u);
    return 1;
}

static C_INT vm_profile_resolver_catalog_contains(
    const vm_profile_resolver_contract_catalog *catalog, type_unsigned_32 id)
{
    STD_SIZE_T index;

    if (catalog == STD_NULL || catalog->ids == STD_NULL || id == 0u) return 0;
    for (index = 0u; index < catalog->count; ++index) {
        if (catalog->ids[index] == id) return 1;
    }
    return 0;
}

static C_INT vm_profile_resolver_windows_are_valid(
    const vm_profile_resolver_window *windows, STD_SIZE_T count, STD_SIZE_T capacity,
    type_unsigned_32 enabled_devices)
{
    STD_SIZE_T first;
    STD_SIZE_T second;

    if ((count != 0u && windows == STD_NULL) || count > capacity) return 0;
    for (first = 0u; first < count; ++first) {
        if (windows[first].device == 0u ||
            (windows[first].device & enabled_devices) != windows[first].device ||
            windows[first].last < windows[first].first) return 0;
        for (second = first + 1u; second < count; ++second) {
            if (windows[first].first <= windows[second].last &&
                windows[second].first <= windows[first].last) return 0;
        }
    }
    return 1;
}

static C_INT vm_profile_resolver_routes_are_valid(
    const vm_profile_resolver_route *routes, STD_SIZE_T count, STD_SIZE_T capacity,
    type_unsigned_32 enabled_devices)
{
    STD_SIZE_T first;
    STD_SIZE_T second;

    if ((count != 0u && routes == STD_NULL) || count > capacity) return 0;
    for (first = 0u; first < count; ++first) {
        if (routes[first].device == 0u ||
            (routes[first].device & enabled_devices) != routes[first].device) return 0;
        for (second = first + 1u; second < count; ++second) {
            if (routes[first].line == routes[second].line) return 0;
        }
    }
    return 1;
}

static C_INT vm_profile_resolver_values_are_valid(
    const vm_profile_resolver_values *values,
    const vm_profile_resolver_contract_catalog *catalog)
{
    if (values == STD_NULL || values->enabled_devices == 0u ||
        values->core.configuration.memory_bytes == 0u ||
        values->core.configuration.cpu_profile == CORE_MACHINE_CPU_PROFILE_DEFAULT ||
        (values->firmware_policy != VM_PROFILE_RESOLVER_FIRMWARE_POLICY_BUILTIN &&
         values->firmware_policy != VM_PROFILE_RESOLVER_FIRMWARE_POLICY_BYOB) ||
        (values->media_policy != VM_PROFILE_RESOLVER_MEDIA_POLICY_NONE &&
         values->media_policy != VM_PROFILE_RESOLVER_MEDIA_POLICY_SESSION) ||
        !vm_profile_resolver_catalog_contains(catalog, values->core.contract_id) ||
        !vm_profile_resolver_windows_are_valid(values->port_windows,
            values->port_window_count, VM_PROFILE_RESOLVER_PORT_WINDOW_CAPACITY,
            values->enabled_devices) ||
        !vm_profile_resolver_windows_are_valid(values->memory_windows,
            values->memory_window_count, VM_PROFILE_RESOLVER_MEMORY_WINDOW_CAPACITY,
            values->enabled_devices) ||
        !vm_profile_resolver_routes_are_valid(values->irq_routes,
            values->irq_route_count, VM_PROFILE_RESOLVER_ROUTE_CAPACITY,
            values->enabled_devices) ||
        !vm_profile_resolver_routes_are_valid(values->drq_routes,
            values->drq_route_count, VM_PROFILE_RESOLVER_ROUTE_CAPACITY,
            values->enabled_devices)) return 0;
    return 1;
}

static C_INT vm_profile_resolver_declaration_is_valid(
    const vm_profile_resolver_declaration *declaration)
{
    if (declaration == STD_NULL || declaration->identity == STD_NULL ||
        declaration->identity[0] == '\0' ||
        (declaration->provided_fields & ~VM_PROFILE_RESOLVER_FIELD_ALL) != 0u ||
        declaration->provided_fields != declaration->owned_fields) return 0;
    return 1;
}

static C_INT vm_profile_resolver_copy_field_owners(vm_resolved_profile *result,
    type_unsigned_32 fields, const C_CHAR *identity)
{
    STD_SIZE_T index;

    if (result == STD_NULL) return 0;
    for (index = 0u; index < VM_PROFILE_RESOLVER_FIELD_COUNT; ++index) {
        if ((fields & (1u << index)) != 0u &&
            !vm_profile_resolver_copy_identity(result->field_owner[index], identity)) {
            return 0;
        }
    }
    return 1;
}

static C_INT vm_profile_resolver_apply(
    const vm_profile_resolver_declaration *declaration,
    const vm_profile_resolver_contract_catalog *catalog,
    vm_resolved_profile *result, type_unsigned_32 *out_seen, STD_SIZE_T depth)
{
    if (!vm_profile_resolver_declaration_is_valid(declaration) || result == STD_NULL ||
        out_seen == STD_NULL || depth >= VM_PROFILE_RESOLVER_PARENT_DEPTH_CAPACITY) return 0;
    if (declaration->parent != STD_NULL && !vm_profile_resolver_apply(
            declaration->parent, catalog, result, out_seen, depth + 1u)) return 0;
    if (!vm_profile_resolver_copy_identity(result->identity, declaration->identity) ||
        !vm_profile_resolver_copy_field_owners(result, declaration->owned_fields,
            declaration->identity)) return 0;
    if (declaration->parent != STD_NULL && !vm_profile_resolver_copy_identity(
            result->parent_identity, declaration->parent->identity)) return 0;
    if ((declaration->provided_fields & VM_PROFILE_RESOLVER_FIELD_CORE) != 0u) {
        result->values.core = declaration->values.core;
    }
    if ((declaration->provided_fields & VM_PROFILE_RESOLVER_FIELD_DEVICES) != 0u) {
        result->values.enabled_devices = declaration->values.enabled_devices;
    }
    if ((declaration->provided_fields & VM_PROFILE_RESOLVER_FIELD_PORTS) != 0u) {
        STD_MEMCPY(result->values.port_windows, declaration->values.port_windows,
            sizeof(result->values.port_windows));
        result->values.port_window_count = declaration->values.port_window_count;
    }
    if ((declaration->provided_fields & VM_PROFILE_RESOLVER_FIELD_MEMORY) != 0u) {
        STD_MEMCPY(result->values.memory_windows, declaration->values.memory_windows,
            sizeof(result->values.memory_windows));
        result->values.memory_window_count = declaration->values.memory_window_count;
    }
    if ((declaration->provided_fields & VM_PROFILE_RESOLVER_FIELD_IRQ) != 0u) {
        STD_MEMCPY(result->values.irq_routes, declaration->values.irq_routes,
            sizeof(result->values.irq_routes));
        result->values.irq_route_count = declaration->values.irq_route_count;
    }
    if ((declaration->provided_fields & VM_PROFILE_RESOLVER_FIELD_DRQ) != 0u) {
        STD_MEMCPY(result->values.drq_routes, declaration->values.drq_routes,
            sizeof(result->values.drq_routes));
        result->values.drq_route_count = declaration->values.drq_route_count;
    }
    if ((declaration->provided_fields & VM_PROFILE_RESOLVER_FIELD_POLICY) != 0u) {
        result->values.firmware_policy = declaration->values.firmware_policy;
        result->values.media_policy = declaration->values.media_policy;
        result->values.allowed_session_options = declaration->values.allowed_session_options;
    }
    result->owned_fields |= declaration->owned_fields;
    *out_seen |= declaration->provided_fields;
    return vm_profile_resolver_values_are_valid(&result->values, catalog) ||
        declaration->parent != STD_NULL;
}

type_status vm_profile_resolver_resolve(
    const vm_profile_resolver_declaration *declaration,
    const vm_profile_resolver_contract_catalog *catalog,
    const vm_profile_resolver_session_request *request,
    vm_resolved_profile *out_profile)
{
    type_unsigned_32 seen = 0u;

    if (declaration == STD_NULL || catalog == STD_NULL || request == STD_NULL ||
        out_profile == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    STD_MEMSET(out_profile, 0, sizeof(*out_profile));
    if (!vm_profile_resolver_apply(declaration, catalog, out_profile, &seen, 0u) ||
        seen != VM_PROFILE_RESOLVER_FIELD_ALL ||
        !vm_profile_resolver_values_are_valid(&out_profile->values, catalog) ||
        (request->requested_options & ~out_profile->values.allowed_session_options) !=
            0u) {
        STD_MEMSET(out_profile, 0, sizeof(*out_profile));
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    return TYPE_STATUS_OK;
}
