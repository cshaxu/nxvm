#include "vm/profile/default_profile/profile.h"

static const core_product_runtime_profile_descriptor_v1 profile = {
    VM_PROFILE_PC_AT_PROFILE_ID,
    NXVM_RUNTIME_PROFILE_MACHINE,
    "products.nxvm.default_profile_builtin",
    STD_NULL,
    0u,
    VM_PROFILE_PC_AT_PROVIDER_ID
};

static const core_product_runtime_firmware_provider_descriptor_v1 provider = {
    VM_PROFILE_PC_AT_PROVIDER_ID,
    NXVM_RUNTIME_FIRMWARE_PROVIDER_BUILTIN,
    "products.nxvm.default_profile_builtin",
    VM_PROFILE_PC_AT_PROFILE_ID
};

ntvdm64_status vm_profile_register_default_profile_builtin(
    core_product_runtime_registry *registry)
{
    ntvdm64_status status = core_product_runtime_registry_register_firmware_provider(
        registry, &provider);

    if (status != NTVDM64_STATUS_OK) return status;
    return core_product_runtime_registry_register_profile(registry, &profile);
}
