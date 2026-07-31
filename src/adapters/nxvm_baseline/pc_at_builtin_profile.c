#include "adapters/nxvm_baseline/pc_at_builtin_profile.h"

static const nxvm_runtime_profile_descriptor_v1 profile = {
    NXVM_BASELINE_PC_AT_BUILTIN_PROFILE_ID,
    NXVM_RUNTIME_PROFILE_ABI_V1,
    NXVM_RUNTIME_PROFILE_MACHINE,
    "adapters.nxvm_baseline.pc_at_builtin",
    NULL,
    0u,
    NXVM_BASELINE_PC_AT_BUILTIN_PROVIDER_ID
};

static const nxvm_runtime_firmware_provider_descriptor_v1 provider = {
    NXVM_BASELINE_PC_AT_BUILTIN_PROVIDER_ID,
    NXVM_RUNTIME_PROVIDER_ABI_V1,
    NXVM_RUNTIME_FIRMWARE_PROVIDER_BUILTIN,
    "adapters.nxvm_baseline.pc_at_builtin",
    NXVM_BASELINE_PC_AT_BUILTIN_PROFILE_ID
};

nxvm_core_status nxvm_baseline_pc_at_builtin_register(
    nxvm_runtime_registry *registry)
{
    nxvm_core_status status = nxvm_runtime_registry_register_firmware_provider(
        registry, &provider);

    if (status != NXVM_CORE_STATUS_OK) {
        return status;
    }
    return nxvm_runtime_registry_register_profile(registry, &profile);
}
