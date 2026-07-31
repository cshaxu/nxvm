#include "runtime/registry.h"

#include <string.h>

static int valid_text(const char *value)
{
    return value != NULL && value[0] != '\0';
}

static int profile_valid(const nxvm_runtime_profile_descriptor_v1 *descriptor)
{
    return descriptor != NULL && valid_text(descriptor->id) &&
        valid_text(descriptor->owner) &&
        descriptor->abi_version == NXVM_RUNTIME_PROFILE_ABI_V1 &&
        (descriptor->family == NXVM_RUNTIME_PROFILE_MACHINE ||
         descriptor->family == NXVM_RUNTIME_PROFILE_EXECUTION) &&
        (descriptor->required_capability_count == 0u ||
         descriptor->required_capabilities != NULL) &&
        (descriptor->family != NXVM_RUNTIME_PROFILE_MACHINE ||
         valid_text(descriptor->firmware_provider_id));
}

static int provider_valid(
    const nxvm_runtime_firmware_provider_descriptor_v1 *descriptor)
{
    return descriptor != NULL && valid_text(descriptor->id) &&
        valid_text(descriptor->owner) && valid_text(descriptor->machine_profile_id) &&
        descriptor->abi_version == NXVM_RUNTIME_PROVIDER_ABI_V1 &&
        (descriptor->kind == NXVM_RUNTIME_FIRMWARE_PROVIDER_BUILTIN ||
         descriptor->kind == NXVM_RUNTIME_FIRMWARE_PROVIDER_EXTERNAL_ROM_BUNDLE ||
         descriptor->kind == NXVM_RUNTIME_FIRMWARE_PROVIDER_ABSENT);
}

void nxvm_runtime_registry_initialize(nxvm_runtime_registry *registry)
{
    if (registry != NULL) {
        memset(registry, 0, sizeof(*registry));
    }
}

nxvm_core_status nxvm_runtime_registry_register_profile(
    nxvm_runtime_registry *registry,
    const nxvm_runtime_profile_descriptor_v1 *descriptor)
{
    size_t index;

    if (registry == NULL || !profile_valid(descriptor)) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }
    if (registry->frozen) {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }
    for (index = 0u; index < registry->profile_count; ++index) {
        if (strcmp(registry->profiles[index]->id, descriptor->id) == 0) {
            return NXVM_CORE_STATUS_UNSUPPORTED;
        }
    }
    if (registry->profile_count == NXVM_RUNTIME_REGISTRY_CAPACITY) {
        return NXVM_CORE_STATUS_NO_MEMORY;
    }
    registry->profiles[registry->profile_count++] = descriptor;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_runtime_registry_register_firmware_provider(
    nxvm_runtime_registry *registry,
    const nxvm_runtime_firmware_provider_descriptor_v1 *descriptor)
{
    size_t index;

    if (registry == NULL || !provider_valid(descriptor)) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }
    if (registry->frozen) {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }
    for (index = 0u; index < registry->provider_count; ++index) {
        if (strcmp(registry->providers[index]->id, descriptor->id) == 0) {
            return NXVM_CORE_STATUS_UNSUPPORTED;
        }
    }
    if (registry->provider_count == NXVM_RUNTIME_REGISTRY_CAPACITY) {
        return NXVM_CORE_STATUS_NO_MEMORY;
    }
    registry->providers[registry->provider_count++] = descriptor;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_runtime_registry_freeze(nxvm_runtime_registry *registry)
{
    if (registry == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }
    registry->frozen = 1;
    return NXVM_CORE_STATUS_OK;
}

const nxvm_runtime_profile_descriptor_v1 *nxvm_runtime_registry_find_profile(
    const nxvm_runtime_registry *registry, const char *id,
    nxvm_runtime_profile_family family,
    const nxvm_core_cpu_capability_manifest *capabilities)
{
    size_t index;
    size_t capability;

    if (registry == NULL || !valid_text(id) || capabilities == NULL) {
        return NULL;
    }
    for (index = 0u; index < registry->profile_count; ++index) {
        const nxvm_runtime_profile_descriptor_v1 *candidate = registry->profiles[index];
        if (candidate->family != family || strcmp(candidate->id, id) != 0) {
            continue;
        }
        for (capability = 0u; capability < candidate->required_capability_count;
             ++capability) {
            if (nxvm_core_cpu_capability_manifest_get(
                    capabilities, candidate->required_capabilities[capability]) !=
                NXVM_CORE_CPU_CAPABILITY_PROVEN) {
                return NULL;
            }
        }
        return candidate;
    }
    return NULL;
}

const nxvm_runtime_firmware_provider_descriptor_v1 *
nxvm_runtime_registry_find_firmware_provider(
    const nxvm_runtime_registry *registry, const char *id,
    const char *machine_profile_id)
{
    size_t index;

    if (registry == NULL || !valid_text(id) || !valid_text(machine_profile_id)) {
        return NULL;
    }
    for (index = 0u; index < registry->provider_count; ++index) {
        const nxvm_runtime_firmware_provider_descriptor_v1 *candidate =
            registry->providers[index];
        if (strcmp(candidate->id, id) == 0 &&
            strcmp(candidate->machine_profile_id, machine_profile_id) == 0) {
            return candidate;
        }
    }
    return NULL;
}
