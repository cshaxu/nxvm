#ifndef NXVM_RUNTIME_REGISTRY_H
#define NXVM_RUNTIME_REGISTRY_H

#include <stddef.h>

#include "core/machine/cpu_capability.h"
#include "core/machine/status.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NXVM_RUNTIME_PROFILE_ABI_V1 1u
#define NXVM_RUNTIME_PROVIDER_ABI_V1 1u
#define NXVM_RUNTIME_REGISTRY_CAPACITY 16u

typedef enum nxvm_runtime_profile_family {
    NXVM_RUNTIME_PROFILE_MACHINE = 1,
    NXVM_RUNTIME_PROFILE_EXECUTION = 2
} nxvm_runtime_profile_family;

typedef enum nxvm_runtime_firmware_provider_kind {
    NXVM_RUNTIME_FIRMWARE_PROVIDER_BUILTIN = 1,
    NXVM_RUNTIME_FIRMWARE_PROVIDER_EXTERNAL_ROM_BUNDLE = 2,
    NXVM_RUNTIME_FIRMWARE_PROVIDER_ABSENT = 3
} nxvm_runtime_firmware_provider_kind;

typedef struct nxvm_runtime_profile_descriptor_v1 {
    const char *id;
    unsigned abi_version;
    nxvm_runtime_profile_family family;
    const char *owner;
    const nxvm_core_cpu_capability *required_capabilities;
    size_t required_capability_count;
    const char *firmware_provider_id;
} nxvm_runtime_profile_descriptor_v1;

typedef struct nxvm_runtime_firmware_provider_descriptor_v1 {
    const char *id;
    unsigned abi_version;
    nxvm_runtime_firmware_provider_kind kind;
    const char *owner;
    const char *machine_profile_id;
} nxvm_runtime_firmware_provider_descriptor_v1;

typedef struct nxvm_runtime_registry {
    const nxvm_runtime_profile_descriptor_v1 *profiles[NXVM_RUNTIME_REGISTRY_CAPACITY];
    const nxvm_runtime_firmware_provider_descriptor_v1 *providers[NXVM_RUNTIME_REGISTRY_CAPACITY];
    size_t profile_count;
    size_t provider_count;
    int frozen;
} nxvm_runtime_registry;

void nxvm_runtime_registry_initialize(nxvm_runtime_registry *registry);
nxvm_core_status nxvm_runtime_registry_register_profile(
    nxvm_runtime_registry *registry,
    const nxvm_runtime_profile_descriptor_v1 *descriptor);
nxvm_core_status nxvm_runtime_registry_register_firmware_provider(
    nxvm_runtime_registry *registry,
    const nxvm_runtime_firmware_provider_descriptor_v1 *descriptor);
nxvm_core_status nxvm_runtime_registry_freeze(nxvm_runtime_registry *registry);
const nxvm_runtime_profile_descriptor_v1 *nxvm_runtime_registry_find_profile(
    const nxvm_runtime_registry *registry,
    const char *id,
    nxvm_runtime_profile_family family,
    const nxvm_core_cpu_capability_manifest *capabilities);
const nxvm_runtime_firmware_provider_descriptor_v1 *
nxvm_runtime_registry_find_firmware_provider(
    const nxvm_runtime_registry *registry,
    const char *id,
    const char *machine_profile_id);

#ifdef __cplusplus
}
#endif

#endif
