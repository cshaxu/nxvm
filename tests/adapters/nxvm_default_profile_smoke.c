#include "type.h"



#include "vm/composition/composition_full_pc.h"

#include "vm/profile/default_profile/profile.h"

C_INT main(C_INT argc, C_CHAR **argv)
{
    core_product_runtime_registry registry;
    vm_composition_full_pc_config config;
    vm_composition_reset_vector vector;
    vm_composition_full_pc *full_pc = STD_NULL;

    if (argc != 2) {
        return 1;
    }
    core_product_runtime_registry_initialize(&registry);
    if (vm_profile_register_default_profile_builtin(&registry) != NTVDM64_STATUS_OK ||
        core_product_runtime_registry_find_profile(&registry,
            VM_PROFILE_PC_AT_PROFILE_ID,
            NXVM_RUNTIME_PROFILE_MACHINE, STD_NULL, STD_NULL) == STD_NULL ||
        core_product_runtime_registry_find_firmware_provider(&registry,
            VM_PROFILE_PC_AT_PROVIDER_ID,
            VM_PROFILE_PC_AT_PROFILE_ID) == STD_NULL ||
        core_product_runtime_registry_freeze(&registry) != NTVDM64_STATUS_OK) {
        return 1;
    }
    config.fdd_image = argv[1];
    config.hdd_image = STD_NULL;
    config.create_fdd = 0;
    config.create_hdd_cylinders = 0u;
    config.boot_hdd = 0;
    if (vm_composition_full_pc_create(&config, &full_pc) != NTVDM64_STATUS_OK ||
        vm_composition_full_pc_get_reset_vector(full_pc, &vector) != NTVDM64_STATUS_OK ||
        vector.cs != 0xf000u || vector.ip != 0xfff0u) {
        vm_composition_full_pc_destroy(full_pc);
        return 1;
    }
    vm_composition_full_pc_destroy(full_pc);
    puts("M5:T2:S2:PC-AT-BUILTIN:OK");
    return 0;
}
