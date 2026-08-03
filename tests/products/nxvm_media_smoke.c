#include <stdio.h>
#include <string.h>

#include "vm/product/media.h"

int main(void)
{
    static const nxvm_product_nxvm_media_identity fdd = {
        "fdd.img", 1474560u,
        "fadeb3a27c6a0e1cf582dde0b9aecb7e5d30678f2f967f2f4562f167cc0cb1d5"
    };
    static const nxvm_product_nxvm_media_identity hdd = {
        "hdd.img", 51609600u,
        "f4d1e81bc410bb9a7558667b7c3741a9664e84077a3774e73104cd24b631d688"
    };
    nxvm_product_nxvm_media_policy policy;
    const nxvm_product_nxvm_block_provider *provider;

    nxvm_product_nxvm_media_policy_initialize(&policy);
    if (nxvm_product_nxvm_media_freeze(&policy) != NTVDM64_STATUS_INVALID_STATE ||
        nxvm_product_nxvm_media_configure(&policy, NXVM_PRODUCT_NXVM_BOOT_FDD,
            "D:/fixtures/fdd.img", &fdd) != NTVDM64_STATUS_OK ||
        nxvm_product_nxvm_media_configure(&policy, NXVM_PRODUCT_NXVM_BOOT_HDD,
            "D:/fixtures/hdd.img", &hdd) != NTVDM64_STATUS_OK ||
        nxvm_product_nxvm_media_set_boot_target(&policy,
            NXVM_PRODUCT_NXVM_BOOT_HDD) != NTVDM64_STATUS_OK ||
        nxvm_product_nxvm_media_freeze(&policy) != NTVDM64_STATUS_OK) return 1;
    provider = nxvm_product_nxvm_media_provider(&policy, NXVM_PRODUCT_NXVM_BOOT_HDD);
    if (provider == NULL || !provider->configured ||
        strcmp(provider->logical_name, "hdd.img") != 0 ||
        strcmp(provider->expected_sha256, hdd.expected_sha256) != 0 ||
        provider->expected_bytes != hdd.expected_bytes ||
        nxvm_product_nxvm_media_set_boot_target(&policy,
            NXVM_PRODUCT_NXVM_BOOT_FDD) != NTVDM64_STATUS_INVALID_STATE ||
        nxvm_product_nxvm_media_configure(&policy, NXVM_PRODUCT_NXVM_BOOT_HDD,
            "D:/fixtures/hdd.img", &hdd) != NTVDM64_STATUS_INVALID_STATE) return 1;

    nxvm_product_nxvm_media_policy_initialize(&policy);
    if (nxvm_product_nxvm_media_configure_created(&policy,
            NXVM_PRODUCT_NXVM_BOOT_FDD, 0u) != NTVDM64_STATUS_OK ||
        nxvm_product_nxvm_media_configure_created(&policy,
            NXVM_PRODUCT_NXVM_BOOT_HDD, 1u) != NTVDM64_STATUS_OK ||
        nxvm_product_nxvm_media_set_boot_target(&policy,
            NXVM_PRODUCT_NXVM_BOOT_HDD) != NTVDM64_STATUS_OK ||
        nxvm_product_nxvm_media_freeze(&policy) != NTVDM64_STATUS_OK) return 1;
    provider = nxvm_product_nxvm_media_provider(&policy, NXVM_PRODUCT_NXVM_BOOT_HDD);
    if (provider == NULL || !provider->configured || !provider->created ||
        provider->cylinders != 1u || provider->path[0] != '\0') return 1;
    puts("M5:T5:S1:NXVM-MEDIA:OK");
    return 0;
}
