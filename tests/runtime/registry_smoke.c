#include <stdio.h>

#include "core/product/runtime/registry.h"

int main(void)
{
    const nxvm_core_cpu_capability required[] = {
        NXVM_CORE_CPU_CAPABILITY_REAL_MODE_8086
    };
    const nxvm_runtime_profile_descriptor_v1 profile = {
        "nxvm.machine.test", NXVM_RUNTIME_PROFILE_ABI_V1,
        NXVM_RUNTIME_PROFILE_MACHINE, "test", required, 1u,
        "firmware.provider.test"
    };
    const nxvm_runtime_firmware_provider_descriptor_v1 provider = {
        "firmware.provider.test", NXVM_RUNTIME_PROVIDER_ABI_V1,
        NXVM_RUNTIME_FIRMWARE_PROVIDER_BUILTIN, "test", "nxvm.machine.test"
    };
    nxvm_core_cpu_capability_manifest capabilities;
    nxvm_runtime_registry registry;

    nxvm_core_cpu_capability_manifest_initialize(&capabilities);
    nxvm_runtime_registry_initialize(&registry);
    if (nxvm_runtime_registry_register_profile(&registry, &profile) !=
            NXVM_CORE_STATUS_OK ||
        nxvm_runtime_registry_register_firmware_provider(&registry, &provider) !=
            NXVM_CORE_STATUS_OK ||
        nxvm_runtime_registry_register_profile(&registry, &profile) !=
            NXVM_CORE_STATUS_UNSUPPORTED ||
        nxvm_runtime_registry_find_profile(&registry, profile.id,
            NXVM_RUNTIME_PROFILE_MACHINE, &capabilities) != NULL ||
        !nxvm_core_cpu_capability_manifest_set(&capabilities,
            NXVM_CORE_CPU_CAPABILITY_REAL_MODE_8086,
            NXVM_CORE_CPU_CAPABILITY_PROVEN) ||
        nxvm_runtime_registry_find_profile(&registry, profile.id,
            NXVM_RUNTIME_PROFILE_MACHINE, &capabilities) != &profile ||
        nxvm_runtime_registry_find_firmware_provider(&registry, provider.id,
            profile.id) != &provider ||
        nxvm_runtime_registry_find_firmware_provider(&registry, provider.id,
            "nxvm.machine.other") != NULL ||
        nxvm_runtime_registry_freeze(&registry) != NXVM_CORE_STATUS_OK ||
        nxvm_runtime_registry_register_profile(&registry, &profile) !=
            NXVM_CORE_STATUS_INVALID_STATE) {
        return 1;
    }
    puts("M5:T2:S1:REGISTRY:OK");
    return 0;
}
