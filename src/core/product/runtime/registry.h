#ifndef NXVM_RUNTIME_REGISTRY_H
#define NXVM_RUNTIME_REGISTRY_H


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

typedef C_INT (*core_product_runtime_capability_query)(C_VOID *context,
                                              C_UINT capability);

typedef struct core_product_runtime_profile_descriptor_v1 {
    const C_CHAR *id;
    core_product_runtime_profile_family family;
    const C_CHAR *owner;
    const C_UINT *required_capabilities;
    STD_SIZE_T required_capability_count;
    const C_CHAR *firmware_provider_id;
} core_product_runtime_profile_descriptor_v1;

typedef struct core_product_runtime_firmware_provider_descriptor_v1 {
    const C_CHAR *id;
    core_product_runtime_firmware_provider_kind kind;
    const C_CHAR *owner;
    const C_CHAR *machine_profile_id;
} core_product_runtime_firmware_provider_descriptor_v1;

typedef struct core_product_runtime_registry {
    const core_product_runtime_profile_descriptor_v1 *profiles[NXVM_RUNTIME_REGISTRY_CAPACITY];
    const core_product_runtime_firmware_provider_descriptor_v1 *providers[NXVM_RUNTIME_REGISTRY_CAPACITY];
    STD_SIZE_T profile_count;
    STD_SIZE_T provider_count;
    C_INT frozen;
} core_product_runtime_registry;

C_VOID core_product_runtime_registry_initialize(core_product_runtime_registry *registry);
ntvdm64_status core_product_runtime_registry_register_profile(
    core_product_runtime_registry *registry,
    const core_product_runtime_profile_descriptor_v1 *descriptor);
ntvdm64_status core_product_runtime_registry_register_firmware_provider(
    core_product_runtime_registry *registry,
    const core_product_runtime_firmware_provider_descriptor_v1 *descriptor);
ntvdm64_status core_product_runtime_registry_freeze(core_product_runtime_registry *registry);
const core_product_runtime_profile_descriptor_v1 *core_product_runtime_registry_find_profile(
    const core_product_runtime_registry *registry,
    const C_CHAR *id,
    core_product_runtime_profile_family family,
    core_product_runtime_capability_query capability_query,
    C_VOID *capability_context);
const core_product_runtime_firmware_provider_descriptor_v1 *
core_product_runtime_registry_find_firmware_provider(
    const core_product_runtime_registry *registry,
    const C_CHAR *id,
    const C_CHAR *machine_profile_id);

#ifdef __cplusplus
}
#endif

#endif
