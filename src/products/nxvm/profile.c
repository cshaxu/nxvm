#include "products/nxvm/profile.h"

static const nxvm_runtime_profile_descriptor_v1 profile = {
    NXVM_PRODUCT_NXVM_PC_AT_PROFILE_ID,
    NXVM_RUNTIME_PROFILE_ABI_V1,
    NXVM_RUNTIME_PROFILE_MACHINE,
    "products.nxvm.pc_at_builtin",
    NULL,
    0u,
    NXVM_PRODUCT_NXVM_PC_AT_PROVIDER_ID
};

static const nxvm_runtime_firmware_provider_descriptor_v1 provider = {
    NXVM_PRODUCT_NXVM_PC_AT_PROVIDER_ID,
    NXVM_RUNTIME_PROVIDER_ABI_V1,
    NXVM_RUNTIME_FIRMWARE_PROVIDER_BUILTIN,
    "products.nxvm.pc_at_builtin",
    NXVM_PRODUCT_NXVM_PC_AT_PROFILE_ID
};

nxvm_core_status nxvm_product_nxvm_register_pc_at_builtin(
    nxvm_runtime_registry *registry)
{
    nxvm_core_status status = nxvm_runtime_registry_register_firmware_provider(
        registry, &provider);

    if (status != NXVM_CORE_STATUS_OK) return status;
    return nxvm_runtime_registry_register_profile(registry, &profile);
}
