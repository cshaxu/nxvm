#include "type.h"



#include "core/product/runtime/registry.h"

static C_INT capability_is_proven(C_VOID *context, C_UINT capability)
{
    return capability == 0u && *(const C_INT *)context != 0;
}

C_INT main(C_VOID)
{
    const C_UINT required[] = {
        0u
    };
    const core_product_runtime_profile_descriptor_v1 profile = {
        "nxvm.machine.test",
        NXVM_RUNTIME_PROFILE_MACHINE, "test", required, 1u,
        "firmware.provider.test"
    };
    const core_product_runtime_firmware_provider_descriptor_v1 provider = {
        "firmware.provider.test",
        NXVM_RUNTIME_FIRMWARE_PROVIDER_BUILTIN, "test", "nxvm.machine.test"
    };
    C_INT capability_is_available = 0;
    core_product_runtime_registry registry;

    core_product_runtime_registry_initialize(&registry);
    if (core_product_runtime_registry_register_profile(&registry, &profile) !=
            NTVDM64_STATUS_OK ||
        core_product_runtime_registry_register_firmware_provider(&registry, &provider) !=
            NTVDM64_STATUS_OK ||
        core_product_runtime_registry_register_profile(&registry, &profile) !=
            NTVDM64_STATUS_UNSUPPORTED ||
        core_product_runtime_registry_find_profile(&registry, profile.id,
            NXVM_RUNTIME_PROFILE_MACHINE, capability_is_proven,
            &capability_is_available) != STD_NULL ||
        (capability_is_available = 1) == 0 ||
        core_product_runtime_registry_find_profile(&registry, profile.id,
            NXVM_RUNTIME_PROFILE_MACHINE, capability_is_proven,
            &capability_is_available) != &profile ||
        core_product_runtime_registry_find_firmware_provider(&registry, provider.id,
            profile.id) != &provider ||
        core_product_runtime_registry_find_firmware_provider(&registry, provider.id,
            "nxvm.machine.other") != STD_NULL ||
        core_product_runtime_registry_freeze(&registry) != NTVDM64_STATUS_OK ||
        core_product_runtime_registry_register_profile(&registry, &profile) !=
            NTVDM64_STATUS_INVALID_STATE) {
        return 1;
    }
    puts("M5:T2:S1:REGISTRY:OK");
    return 0;
}
