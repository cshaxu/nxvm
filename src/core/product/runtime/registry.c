#include "core/product/runtime/registry.h"

#include <string.h>

static int valid_text(const char *value)
{
    return value != NULL && value[0] != '\0';
}

static int profile_valid(const core_product_runtime_profile_descriptor_v1 *descriptor)
{
    return descriptor != NULL && valid_text(descriptor->id) &&
        valid_text(descriptor->owner) &&
        (descriptor->family == NXVM_RUNTIME_PROFILE_MACHINE ||
         descriptor->family == NXVM_RUNTIME_PROFILE_EXECUTION) &&
        (descriptor->required_capability_count == 0u ||
         descriptor->required_capabilities != NULL) &&
        (descriptor->family != NXVM_RUNTIME_PROFILE_MACHINE ||
         valid_text(descriptor->firmware_provider_id));
}

static int provider_valid(
    const core_product_runtime_firmware_provider_descriptor_v1 *descriptor)
{
    return descriptor != NULL && valid_text(descriptor->id) &&
        valid_text(descriptor->owner) && valid_text(descriptor->machine_profile_id) &&
        (descriptor->kind == NXVM_RUNTIME_FIRMWARE_PROVIDER_BUILTIN ||
         descriptor->kind == NXVM_RUNTIME_FIRMWARE_PROVIDER_EXTERNAL_ROM_BUNDLE ||
         descriptor->kind == NXVM_RUNTIME_FIRMWARE_PROVIDER_ABSENT);
}

void core_product_runtime_registry_initialize(core_product_runtime_registry *registry)
{
    if (registry != NULL) {
        STD_MEMSET(registry, 0, sizeof(*registry));
    }
}

ntvdm64_status core_product_runtime_registry_register_profile(
    core_product_runtime_registry *registry,
    const core_product_runtime_profile_descriptor_v1 *descriptor)
{
    size_t index;

    if (registry == NULL || !profile_valid(descriptor)) {
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
    size_t index;

    if (registry == NULL || !provider_valid(descriptor)) {
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
    if (registry == NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    registry->frozen = 1;
    return NTVDM64_STATUS_OK;
}

const core_product_runtime_profile_descriptor_v1 *core_product_runtime_registry_find_profile(
    const core_product_runtime_registry *registry, const char *id,
    core_product_runtime_profile_family family,
    core_product_runtime_capability_query capability_query,
    void *capability_context)
{
    size_t index;
    size_t capability;

    if (registry == NULL || !valid_text(id)) {
        return NULL;
    }
    for (index = 0u; index < registry->profile_count; ++index) {
        const core_product_runtime_profile_descriptor_v1 *candidate = registry->profiles[index];
        if (candidate->family != family || STD_STRCMP(candidate->id, id) != 0) {
            continue;
        }
        for (capability = 0u; capability < candidate->required_capability_count;
             ++capability) {
            if (capability_query == NULL || !capability_query(
                    capability_context, candidate->required_capabilities[capability])) {
                return NULL;
            }
        }
        return candidate;
    }
    return NULL;
}

const core_product_runtime_firmware_provider_descriptor_v1 *
core_product_runtime_registry_find_firmware_provider(
    const core_product_runtime_registry *registry, const char *id,
    const char *machine_profile_id)
{
    size_t index;

    if (registry == NULL || !valid_text(id) || !valid_text(machine_profile_id)) {
        return NULL;
    }
    for (index = 0u; index < registry->provider_count; ++index) {
        const core_product_runtime_firmware_provider_descriptor_v1 *candidate =
            registry->providers[index];
        if (STD_STRCMP(candidate->id, id) == 0 &&
            STD_STRCMP(candidate->machine_profile_id, machine_profile_id) == 0) {
            return candidate;
        }
    }
    return NULL;
}
