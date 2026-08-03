#include "type.h"



#include "vm/product/media.h"

#include "vm/composition/composition_default_profile.h"

static C_INT verify(const C_CHAR *fdd_path, const C_CHAR *hdd_path,
                  vm_product_boot_target boot_target)
{
    static const vm_product_media_identity fdd = {
        "fdd.img", 1474560u,
        "fadeb3a27c6a0e1cf582dde0b9aecb7e5d30678f2f967f2f4562f167cc0cb1d5"
    };
    static const vm_product_media_identity hdd = {
        "hdd.img", 51609600u,
        "f4d1e81bc410bb9a7558667b7c3741a9664e84077a3774e73104cd24b631d688"
    };
    vm_product_media_policy media;
    vm_composition_default_profile default_profile = { 0 };
    vm_composition_default_profile_reset_vector vector;
    C_INT running = 1;

    vm_product_media_policy_initialize(&media);
    if (vm_product_media_configure(&media, VM_PRODUCT_BOOT_FDD,
            fdd_path, &fdd) != NTVDM64_STATUS_OK ||
        vm_product_media_configure(&media, VM_PRODUCT_BOOT_HDD,
            hdd_path, &hdd) != NTVDM64_STATUS_OK ||
        vm_product_media_set_boot_target(&media, boot_target) != NTVDM64_STATUS_OK ||
        vm_product_media_freeze(&media) != NTVDM64_STATUS_OK ||
        vm_composition_default_profile_create(&default_profile, &media) != NTVDM64_STATUS_OK ||
        vm_composition_default_profile_set_window_display(&default_profile, 0) != NTVDM64_STATUS_OK ||
        vm_composition_default_profile_set_memory_kb(&default_profile, 16384u) != NTVDM64_STATUS_OK ||
        vm_composition_default_profile_reset(&default_profile) != NTVDM64_STATUS_OK ||
        vm_composition_default_profile_is_running(&default_profile, &running) != NTVDM64_STATUS_OK ||
        running != 0 ||
        vm_composition_default_profile_remove_fdd(&default_profile, STD_NULL) != NTVDM64_STATUS_OK ||
        vm_composition_default_profile_disconnect_hdd(&default_profile, STD_NULL) != NTVDM64_STATUS_OK ||
        vm_composition_default_profile_get_reset_vector(&default_profile, &vector) != NTVDM64_STATUS_OK ||
        vector.cs != 0xf000u || vector.ip != 0xfff0u) {
        vm_composition_default_profile_destroy(&default_profile);
        return 1;
    }
    vm_composition_default_profile_destroy(&default_profile);
    return 0;
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    if (argc != 3 || verify(argv[1], argv[2], VM_PRODUCT_BOOT_FDD) != 0 ||
        verify(argv[1], argv[2], VM_PRODUCT_BOOT_HDD) != 0) return 1;
    puts("M5:T5:S2:NXVM-PC-AT:OK");
    return 0;
}
