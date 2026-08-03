#include <stdio.h>
#include <string.h>

#include "vm/composition_full_pc.h"
#include "vm/profile/full_pc_profile.h"

static int verify_profile(void)
{
    const core_product_runtime_profile_descriptor *profile =
        vm_profile_full_pc_profile_descriptor();

    return profile == NULL || strcmp(profile->name, "nxvm.full_pc") != 0 ||
           !profile->permits_disk_boot || !profile->uses_legacy_adapter ||
           (profile->devices & (NXVM_RUNTIME_DEVICE_BIOS |
                                NXVM_RUNTIME_DEVICE_CMOS |
                                NXVM_RUNTIME_DEVICE_DMA |
                                NXVM_RUNTIME_DEVICE_FDC |
                                NXVM_RUNTIME_DEVICE_FDD |
                                NXVM_RUNTIME_DEVICE_HDC |
                                NXVM_RUNTIME_DEVICE_HDD |
                                NXVM_RUNTIME_DEVICE_VADP)) == 0u;
}

static int verify_image(const char *fdd, const char *hdd, int boot_hdd)
{
    vm_composition_full_pc_config config = { fdd, hdd, 0, 0u, boot_hdd };
    vm_composition_reset_vector vector;
    vm_composition_full_pc *full_pc = NULL;

    if (vm_composition_full_pc_create(&config, &full_pc) != NTVDM64_STATUS_OK) {
        return 1;
    }
    if (vm_composition_full_pc_get_reset_vector(full_pc, &vector) != NTVDM64_STATUS_OK ||
        vector.cs != 0xf000u || vector.ip != 0xfff0u) {
        vm_composition_full_pc_destroy(full_pc);
        return 1;
    }
    vm_composition_full_pc_destroy(full_pc);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 3 || verify_profile() != 0 ||
        verify_image(argv[1], NULL, 0) != 0 ||
        verify_image(NULL, argv[2], 1) != 0) {
        return 1;
    }

    puts("M3:T3:S1:FULL-PC-PROFILE:OK");
    return 0;
}
