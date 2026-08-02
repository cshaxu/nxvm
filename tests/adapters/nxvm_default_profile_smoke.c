#include <stdio.h>

#include "vm/composition_full_pc.h"
#include "vm/profile/default_profile/profile.h"

int main(int argc, char **argv)
{
    nxvm_runtime_registry registry;
    nxvm_full_pc_config config;
    nxvm_vm_reset_vector vector;
    nxvm_full_pc *full_pc = NULL;

    if (argc != 2) {
        return 1;
    }
    nxvm_runtime_registry_initialize(&registry);
    if (nxvm_product_nxvm_register_default_profile_builtin(&registry) != NXVM_CORE_STATUS_OK ||
        nxvm_runtime_registry_find_profile(&registry,
            NXVM_PRODUCT_NXVM_PC_AT_PROFILE_ID,
            NXVM_RUNTIME_PROFILE_MACHINE, NULL, NULL) == NULL ||
        nxvm_runtime_registry_find_firmware_provider(&registry,
            NXVM_PRODUCT_NXVM_PC_AT_PROVIDER_ID,
            NXVM_PRODUCT_NXVM_PC_AT_PROFILE_ID) == NULL ||
        nxvm_runtime_registry_freeze(&registry) != NXVM_CORE_STATUS_OK) {
        return 1;
    }
    config.fdd_image = argv[1];
    config.hdd_image = NULL;
    config.create_fdd = 0;
    config.create_hdd_cylinders = 0u;
    config.boot_hdd = 0;
    if (nxvm_full_pc_create(&config, &full_pc) != NXVM_CORE_STATUS_OK ||
        nxvm_full_pc_get_reset_vector(full_pc, &vector) != NXVM_CORE_STATUS_OK ||
        vector.cs != 0xf000u || vector.ip != 0xfff0u) {
        nxvm_full_pc_destroy(full_pc);
        return 1;
    }
    nxvm_full_pc_destroy(full_pc);
    puts("M5:T2:S2:PC-AT-BUILTIN:OK");
    return 0;
}
