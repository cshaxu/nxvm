#include "type.h"

#include "core/product/runtime/registry.h"



static C_INT valid_text(const C_CHAR *value)
{
    return value != STD_NULL && value[0] != '\0';
}

static C_INT profile_valid(const core_product_runtime_profile_descriptor_v1 *descriptor)
{
    return descriptor != STD_NULL && valid_text(descriptor->id) &&
        valid_text(descriptor->owner) &&
        (descriptor->family == NXVM_RUNTIME_PROFILE_MACHINE ||
         descriptor->family == NXVM_RUNTIME_PROFILE_EXECUTION) &&
        (descriptor->required_capability_count == 0u ||
         descriptor->required_capabilities != STD_NULL) &&
        (descriptor->family != NXVM_RUNTIME_PROFILE_MACHINE ||
         valid_text(descriptor->firmware_provider_id));
}

static C_INT provider_valid(
    const core_product_runtime_firmware_provider_descriptor_v1 *descriptor)
{
    return descriptor != STD_NULL && valid_text(descriptor->id) &&
        valid_text(descriptor->owner) && valid_text(descriptor->machine_profile_id) &&
        (descriptor->kind == NXVM_RUNTIME_FIRMWARE_PROVIDER_BUILTIN ||
         descriptor->kind == NXVM_RUNTIME_FIRMWARE_PROVIDER_EXTERNAL_ROM_BUNDLE ||
         descriptor->kind == NXVM_RUNTIME_FIRMWARE_PROVIDER_ABSENT);
}

C_VOID core_product_runtime_registry_initialize(core_product_runtime_registry *registry)
{
    if (registry != STD_NULL) {
        STD_MEMSET(registry, 0, sizeof(*registry));
    }
}

ntvdm64_status core_product_runtime_registry_register_profile(
    core_product_runtime_registry *registry,
    const core_product_runtime_profile_descriptor_v1 *descriptor)
{
    STD_SIZE_T index;

    if (registry == STD_NULL || !profile_valid(descriptor)) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    if (registry->frozen) {
        return NTVDM64_STATUS_INVALID_STATE;
    }
    for (index = 0u; index < registry->profile_count; ++index) {
        if (STD_STRCMP(registry->profiles[index]->id, descriptor->id) == 0) {
            return NTVDM64_STATUS_UNSUPPORTED;
        }
    }
    if (registry->profile_count == NXVM_RUNTIME_REGISTRY_CAPACITY) {
        return NTVDM64_STATUS_NO_MEMORY;
    }
    registry->profiles[registry->profile_count++] = descriptor;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status core_product_runtime_registry_register_firmware_provider(
    core_product_runtime_registry *registry,
    const core_product_runtime_firmware_provider_descriptor_v1 *descriptor)
{
    STD_SIZE_T index;

    if (registry == STD_NULL || !provider_valid(descriptor)) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    if (registry->frozen) {
        return NTVDM64_STATUS_INVALID_STATE;
    }
    for (index = 0u; index < registry->provider_count; ++index) {
        if (STD_STRCMP(registry->providers[index]->id, descriptor->id) == 0) {
            return NTVDM64_STATUS_UNSUPPORTED;
        }
    }
    if (registry->provider_count == NXVM_RUNTIME_REGISTRY_CAPACITY) {
        return NTVDM64_STATUS_NO_MEMORY;
    }
    registry->providers[registry->provider_count++] = descriptor;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status core_product_runtime_registry_freeze(core_product_runtime_registry *registry)
{
    if (registry == STD_NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    registry->frozen = 1;
    return NTVDM64_STATUS_OK;
}

const core_product_runtime_profile_descriptor_v1 *core_product_runtime_registry_find_profile(
    const core_product_runtime_registry *registry, const C_CHAR *id,
    core_product_runtime_profile_family family,
    core_product_runtime_capability_query capability_query,
    C_VOID *capability_context)
{
    STD_SIZE_T index;
    STD_SIZE_T capability;

    if (registry == STD_NULL || !valid_text(id)) {
        return STD_NULL;
    }
    for (index = 0u; index < registry->profile_count; ++index) {
        const core_product_runtime_profile_descriptor_v1 *candidate = registry->profiles[index];
        if (candidate->family != family || STD_STRCMP(candidate->id, id) != 0) {
            continue;
        }
        for (capability = 0u; capability < candidate->required_capability_count;
             ++capability) {
            if (capability_query == STD_NULL || !capability_query(
                    capability_context, candidate->required_capabilities[capability])) {
                return STD_NULL;
            }
        }
        return candidate;
    }
    return STD_NULL;
}

const core_product_runtime_firmware_provider_descriptor_v1 *
core_product_runtime_registry_find_firmware_provider(
    const core_product_runtime_registry *registry, const C_CHAR *id,
    const C_CHAR *machine_profile_id)
{
    STD_SIZE_T index;

    if (registry == STD_NULL || !valid_text(id) || !valid_text(machine_profile_id)) {
        return STD_NULL;
    }
    for (index = 0u; index < registry->provider_count; ++index) {
        const core_product_runtime_firmware_provider_descriptor_v1 *candidate =
            registry->providers[index];
        if (STD_STRCMP(candidate->id, id) == 0 &&
            STD_STRCMP(candidate->machine_profile_id, machine_profile_id) == 0) {
            return candidate;
        }
    }
    return STD_NULL;
}
