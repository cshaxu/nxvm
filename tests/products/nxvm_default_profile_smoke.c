#include <stdio.h>

#include "vm/product/media.h"
#include "vm/profile/default_profile/default_profile.h"

static int verify(const char *fdd_path, const char *hdd_path,
                  nxvm_product_nxvm_boot_target boot_target)
{
    static const nxvm_product_nxvm_media_identity fdd = {
        "fdd.img", 1474560u,
        "fadeb3a27c6a0e1cf582dde0b9aecb7e5d30678f2f967f2f4562f167cc0cb1d5"
    };
    static const nxvm_product_nxvm_media_identity hdd = {
        "hdd.img", 51609600u,
        "f4d1e81bc410bb9a7558667b7c3741a9664e84077a3774e73104cd24b631d688"
    };
    nxvm_product_nxvm_media_policy media;
    nxvm_product_nxvm_default_profile default_profile = { 0 };
    nxvm_product_nxvm_reset_vector vector;
    int running = 1;

    nxvm_product_nxvm_media_policy_initialize(&media);
    if (nxvm_product_nxvm_media_configure(&media, NXVM_PRODUCT_NXVM_BOOT_FDD,
            fdd_path, &fdd) != NXVM_CORE_STATUS_OK ||
        nxvm_product_nxvm_media_configure(&media, NXVM_PRODUCT_NXVM_BOOT_HDD,
            hdd_path, &hdd) != NXVM_CORE_STATUS_OK ||
        nxvm_product_nxvm_media_set_boot_target(&media, boot_target) != NXVM_CORE_STATUS_OK ||
        nxvm_product_nxvm_media_freeze(&media) != NXVM_CORE_STATUS_OK ||
        nxvm_product_nxvm_default_profile_create(&default_profile, &media) != NXVM_CORE_STATUS_OK ||
        nxvm_product_nxvm_default_profile_set_window_display(&default_profile, 0) != NXVM_CORE_STATUS_OK ||
        nxvm_product_nxvm_default_profile_set_memory_kb(&default_profile, 16384u) != NXVM_CORE_STATUS_OK ||
        nxvm_product_nxvm_default_profile_reset(&default_profile) != NXVM_CORE_STATUS_OK ||
        nxvm_product_nxvm_default_profile_is_running(&default_profile, &running) != NXVM_CORE_STATUS_OK ||
        running != 0 ||
        nxvm_product_nxvm_default_profile_remove_fdd(&default_profile, NULL) != NXVM_CORE_STATUS_OK ||
        nxvm_product_nxvm_default_profile_disconnect_hdd(&default_profile, NULL) != NXVM_CORE_STATUS_OK ||
        nxvm_product_nxvm_default_profile_get_reset_vector(&default_profile, &vector) != NXVM_CORE_STATUS_OK ||
        vector.cs != 0xf000u || vector.ip != 0xfff0u) {
        nxvm_product_nxvm_default_profile_destroy(&default_profile);
        return 1;
    }
    nxvm_product_nxvm_default_profile_destroy(&default_profile);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 3 || verify(argv[1], argv[2], NXVM_PRODUCT_NXVM_BOOT_FDD) != 0 ||
        verify(argv[1], argv[2], NXVM_PRODUCT_NXVM_BOOT_HDD) != 0) return 1;
    puts("M5:T5:S2:NXVM-PC-AT:OK");
    return 0;
}
