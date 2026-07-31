#include <stdio.h>

#include "products/nxvm/media.h"
#include "products/nxvm/pc_at.h"

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
    nxvm_product_nxvm_pc_at pc_at = { 0 };
    nxvm_product_nxvm_reset_vector vector;
    int running = 1;

    nxvm_product_nxvm_media_policy_initialize(&media);
    if (nxvm_product_nxvm_media_configure(&media, NXVM_PRODUCT_NXVM_BOOT_FDD,
            fdd_path, &fdd) != NXVM_CORE_STATUS_OK ||
        nxvm_product_nxvm_media_configure(&media, NXVM_PRODUCT_NXVM_BOOT_HDD,
            hdd_path, &hdd) != NXVM_CORE_STATUS_OK ||
        nxvm_product_nxvm_media_set_boot_target(&media, boot_target) != NXVM_CORE_STATUS_OK ||
        nxvm_product_nxvm_media_freeze(&media) != NXVM_CORE_STATUS_OK ||
        nxvm_product_nxvm_pc_at_create(&pc_at, &media) != NXVM_CORE_STATUS_OK ||
        nxvm_product_nxvm_pc_at_set_window_display(&pc_at, 0) != NXVM_CORE_STATUS_OK ||
        nxvm_product_nxvm_pc_at_reset(&pc_at) != NXVM_CORE_STATUS_OK ||
        nxvm_product_nxvm_pc_at_is_running(&pc_at, &running) != NXVM_CORE_STATUS_OK ||
        running != 0 ||
        nxvm_product_nxvm_pc_at_get_reset_vector(&pc_at, &vector) != NXVM_CORE_STATUS_OK ||
        vector.cs != 0xf000u || vector.ip != 0xfff0u) {
        nxvm_product_nxvm_pc_at_destroy(&pc_at);
        return 1;
    }
    nxvm_product_nxvm_pc_at_destroy(&pc_at);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 3 || verify(argv[1], argv[2], NXVM_PRODUCT_NXVM_BOOT_FDD) != 0 ||
        verify(argv[1], argv[2], NXVM_PRODUCT_NXVM_BOOT_HDD) != 0) return 1;
    puts("M5:T5:S2:NXVM-PC-AT:OK");
    return 0;
}
