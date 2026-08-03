#include "type.h"

#include <stdio.h>

#include <string.h>


#include "vm/composition_full_pc.h"

#include "vm/profile/full_pc_profile.h"

static C_INT verify_profile(C_VOID)
{
    const core_product_runtime_profile_descriptor *profile =
        vm_profile_full_pc_profile_descriptor();

    return profile == STD_NULL || STD_STRCMP(profile->name, "nxvm.full_pc") != 0 ||
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

static C_INT verify_image(const C_CHAR *fdd, const C_CHAR *hdd, C_INT boot_hdd)
{
    vm_composition_full_pc_config config = { fdd, hdd, 0, 0u, boot_hdd };
    vm_composition_reset_vector vector;
    vm_composition_full_pc *full_pc = STD_NULL;

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

C_INT main(C_INT argc, C_CHAR **argv)
{
    if (argc != 3 || verify_profile() != 0 ||
        verify_image(argv[1], STD_NULL, 0) != 0 ||
        verify_image(STD_NULL, argv[2], 1) != 0) {
        return 1;
    }

    puts("M3:T3:S1:FULL-PC-PROFILE:OK");
    return 0;
}
