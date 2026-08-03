#ifndef NXVM_RUNTIME_REGISTRY_H
#define NXVM_RUNTIME_REGISTRY_H

#include <stddef.h>

#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NXVM_RUNTIME_REGISTRY_CAPACITY 16u

typedef enum core_product_runtime_profile_family {
    NXVM_RUNTIME_PROFILE_MACHINE = 1,
    NXVM_RUNTIME_PROFILE_EXECUTION = 2
} core_product_runtime_profile_family;

typedef enum core_product_runtime_firmware_provider_kind {
    NXVM_RUNTIME_FIRMWARE_PROVIDER_BUILTIN = 1,
    NXVM_RUNTIME_FIRMWARE_PROVIDER_EXTERNAL_ROM_BUNDLE = 2,
    NXVM_RUNTIME_FIRMWARE_PROVIDER_ABSENT = 3
} core_product_runtime_firmware_provider_kind;

typedef int (*core_product_runtime_capability_query)(void *context,
                                              unsigned capability);

typedef struct core_product_runtime_profile_descriptor_v1 {
    const char *id;
    core_product_runtime_profile_family family;
    const char *owner;
    const unsigned *required_capabilities;
    size_t required_capability_count;
    const char *firmware_provider_id;
} core_product_runtime_profile_descriptor_v1;

typedef struct core_product_runtime_firmware_provider_descriptor_v1 {
    const char *id;
    core_product_runtime_firmware_provider_kind kind;
    const char *owner;
    const char *machine_profile_id;
} core_product_runtime_firmware_provider_descriptor_v1;

typedef struct core_product_runtime_registry {
    const core_product_runtime_profile_descriptor_v1 *profiles[NXVM_RUNTIME_REGISTRY_CAPACITY];
    const core_product_runtime_firmware_provider_descriptor_v1 *providers[NXVM_RUNTIME_REGISTRY_CAPACITY];
    size_t profile_count;
    size_t provider_count;
    int frozen;
} core_product_runtime_registry;

void core_product_runtime_registry_initialize(core_product_runtime_registry *registry);
ntvdm64_status core_product_runtime_registry_register_profile(
    core_product_runtime_registry *registry,
    const core_product_runtime_profile_descriptor_v1 *descriptor);
ntvdm64_status core_product_runtime_registry_register_firmware_provider(
    core_product_runtime_registry *registry,
    const core_product_runtime_firmware_provider_descriptor_v1 *descriptor);
ntvdm64_status core_product_runtime_registry_freeze(core_product_runtime_registry *registry);
const core_product_runtime_profile_descriptor_v1 *core_product_runtime_registry_find_profile(
    const core_product_runtime_registry *registry,
    const char *id,
    core_product_runtime_profile_family family,
    core_product_runtime_capability_query capability_query,
    void *capability_context);
const core_product_runtime_firmware_provider_descriptor_v1 *
core_product_runtime_registry_find_firmware_provider(
    const core_product_runtime_registry *registry,
    const char *id,
    const char *machine_profile_id);

#ifdef __cplusplus
}
#endif

#endif
