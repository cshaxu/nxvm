#include "type.h"

#include <stdio.h>


#include "vm/profile/default_profile/profile.h"

#include "vm/composition_session_model.h"

static C_INT verify(const C_CHAR *fdd_path, const C_CHAR *hdd_path,
                  vm_product_boot_target target)
{
    static const vm_product_media_identity fdd = {
        "fdd.img", 1474560u,
        "fadeb3a27c6a0e1cf582dde0b9aecb7e5d30678f2f967f2f4562f167cc0cb1d5"
    };
    static const vm_product_media_identity hdd = {
        "hdd.img", 51609600u,
        "f4d1e81bc410bb9a7558667b7c3741a9664e84077a3774e73104cd24b631d688"
    };
    vm_composition_session_model_config config = {
        fdd_path, &fdd, hdd_path, &hdd, 0, 0u, target
    };
    vm_composition_session_model session;
    vm_composition_default_profile_reset_vector firmware_vector;
    vm_composition_default_profile_reset_vector execution_vector;

    if (vm_composition_session_model_create(&session, &config) != NTVDM64_STATUS_OK ||
        core_product_runtime_registry_find_profile(&session.registry,
            VM_PROFILE_PC_AT_PROFILE_ID, NXVM_RUNTIME_PROFILE_MACHINE,
            NULL, NULL) == NULL ||
        core_product_runtime_registry_find_firmware_provider(&session.registry,
            VM_PROFILE_PC_AT_PROVIDER_ID,
            VM_PROFILE_PC_AT_PROFILE_ID) == NULL ||
        session.firmware.frozen == 0 || session.media.frozen == 0 ||
        vm_composition_session_model_get_firmware_reset_vector(&session,
            &firmware_vector) != NTVDM64_STATUS_OK ||
        vm_composition_session_model_get_execution_reset_vector(&session,
            &execution_vector) != NTVDM64_STATUS_OK ||
        firmware_vector.cs != 0xf000u || firmware_vector.ip != 0xfff0u ||
        execution_vector.cs != 0xf000u || execution_vector.ip != 0xfff0u) {
        vm_composition_session_model_destroy(&session);
        return 1;
    }
    vm_composition_session_model_destroy(&session);
    return 0;
}

static C_INT verify_created(C_VOID)
{
    vm_composition_session_model_config config = {
        NULL, NULL, NULL, NULL, 1, 1u, VM_PRODUCT_BOOT_HDD
    };
    vm_composition_session_model session;
    vm_composition_default_profile_reset_vector vector;

    if (vm_composition_session_model_create(&session, &config) != NTVDM64_STATUS_OK ||
        !session.media.fdd.created || !session.media.hdd.created ||
        session.media.hdd.cylinders != 1u ||
        vm_composition_session_model_get_execution_reset_vector(&session, &vector) !=
            NTVDM64_STATUS_OK || vector.cs != 0xf000u || vector.ip != 0xfff0u) {
        vm_composition_session_model_destroy(&session);
        return 1;
    }
    vm_composition_session_model_destroy(&session);
    return 0;
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    if (argc != 3 || verify(argv[1], argv[2], VM_PRODUCT_BOOT_FDD) != 0 ||
        verify(argv[1], argv[2], VM_PRODUCT_BOOT_HDD) != 0 ||
        verify_created() != 0) return 1;
    puts("M5:T7:S1:NXVM-SESSION:OK");
    return 0;
}
