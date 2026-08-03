#include <stdio.h>

#include "core/product/runtime/registry.h"

static int capability_is_proven(void *context, unsigned capability)
{
    return capability == 0u && *(const int *)context != 0;
}

int main(void)
{
    const unsigned required[] = {
        0u
    };
    const nxvm_runtime_profile_descriptor_v1 profile = {
        "nxvm.machine.test",
        NXVM_RUNTIME_PROFILE_MACHINE, "test", required, 1u,
        "firmware.provider.test"
    };
    const nxvm_runtime_firmware_provider_descriptor_v1 provider = {
        "firmware.provider.test",
        NXVM_RUNTIME_FIRMWARE_PROVIDER_BUILTIN, "test", "nxvm.machine.test"
    };
    int capability_is_available = 0;
    nxvm_runtime_registry registry;

    nxvm_runtime_registry_initialize(&registry);
    if (nxvm_runtime_registry_register_profile(&registry, &profile) !=
            NTVDM64_STATUS_OK ||
        nxvm_runtime_registry_register_firmware_provider(&registry, &provider) !=
            NTVDM64_STATUS_OK ||
        nxvm_runtime_registry_register_profile(&registry, &profile) !=
            NTVDM64_STATUS_UNSUPPORTED ||
        nxvm_runtime_registry_find_profile(&registry, profile.id,
            NXVM_RUNTIME_PROFILE_MACHINE, capability_is_proven,
            &capability_is_available) != NULL ||
        (capability_is_available = 1) == 0 ||
        nxvm_runtime_registry_find_profile(&registry, profile.id,
            NXVM_RUNTIME_PROFILE_MACHINE, capability_is_proven,
            &capability_is_available) != &profile ||
        nxvm_runtime_registry_find_firmware_provider(&registry, provider.id,
            profile.id) != &provider ||
        nxvm_runtime_registry_find_firmware_provider(&registry, provider.id,
            "nxvm.machine.other") != NULL ||
        nxvm_runtime_registry_freeze(&registry) != NTVDM64_STATUS_OK ||
        nxvm_runtime_registry_register_profile(&registry, &profile) !=
            NTVDM64_STATUS_INVALID_STATE) {
        return 1;
    }
    puts("M5:T2:S1:REGISTRY:OK");
    return 0;
}
