#include <stdio.h>

#include "vm/profile/default_profile/profile.h"
#include "products/nxvm/session.h"

static int verify(const char *fdd_path, const char *hdd_path,
                  nxvm_product_nxvm_boot_target target)
{
    static const nxvm_product_nxvm_media_identity fdd = {
        "fdd.img", 1474560u,
        "fadeb3a27c6a0e1cf582dde0b9aecb7e5d30678f2f967f2f4562f167cc0cb1d5"
    };
    static const nxvm_product_nxvm_media_identity hdd = {
        "hdd.img", 51609600u,
        "f4d1e81bc410bb9a7558667b7c3741a9664e84077a3774e73104cd24b631d688"
    };
    nxvm_product_nxvm_session_config config = {
        fdd_path, &fdd, hdd_path, &hdd, 0, 0u, target
    };
    nxvm_product_nxvm_session session;
    nxvm_core_cpu_capability_manifest capabilities;
    nxvm_product_nxvm_reset_vector firmware_vector;
    nxvm_product_nxvm_reset_vector execution_vector;

    nxvm_core_cpu_capability_manifest_initialize(&capabilities);
    if (nxvm_product_nxvm_session_create(&session, &config) != NXVM_CORE_STATUS_OK ||
        nxvm_runtime_registry_find_profile(&session.registry,
            NXVM_PRODUCT_NXVM_PC_AT_PROFILE_ID, NXVM_RUNTIME_PROFILE_MACHINE,
            &capabilities) == NULL ||
        nxvm_runtime_registry_find_firmware_provider(&session.registry,
            NXVM_PRODUCT_NXVM_PC_AT_PROVIDER_ID,
            NXVM_PRODUCT_NXVM_PC_AT_PROFILE_ID) == NULL ||
        session.firmware.frozen == 0 || session.media.frozen == 0 ||
        nxvm_product_nxvm_session_get_firmware_reset_vector(&session,
            &firmware_vector) != NXVM_CORE_STATUS_OK ||
        nxvm_product_nxvm_session_get_execution_reset_vector(&session,
            &execution_vector) != NXVM_CORE_STATUS_OK ||
        firmware_vector.cs != 0xf000u || firmware_vector.ip != 0xfff0u ||
        execution_vector.cs != 0xf000u || execution_vector.ip != 0xfff0u) {
        nxvm_product_nxvm_session_destroy(&session);
        return 1;
    }
    nxvm_product_nxvm_session_destroy(&session);
    return 0;
}

static int verify_created(void)
{
    nxvm_product_nxvm_session_config config = {
        NULL, NULL, NULL, NULL, 1, 1u, NXVM_PRODUCT_NXVM_BOOT_HDD
    };
    nxvm_product_nxvm_session session;
    nxvm_product_nxvm_reset_vector vector;

    if (nxvm_product_nxvm_session_create(&session, &config) != NXVM_CORE_STATUS_OK ||
        !session.media.fdd.created || !session.media.hdd.created ||
        session.media.hdd.cylinders != 1u ||
        nxvm_product_nxvm_session_get_execution_reset_vector(&session, &vector) !=
            NXVM_CORE_STATUS_OK || vector.cs != 0xf000u || vector.ip != 0xfff0u) {
        nxvm_product_nxvm_session_destroy(&session);
        return 1;
    }
    nxvm_product_nxvm_session_destroy(&session);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 3 || verify(argv[1], argv[2], NXVM_PRODUCT_NXVM_BOOT_FDD) != 0 ||
        verify(argv[1], argv[2], NXVM_PRODUCT_NXVM_BOOT_HDD) != 0 ||
        verify_created() != 0) return 1;
    puts("M5:T7:S1:NXVM-SESSION:OK");
    return 0;
}
