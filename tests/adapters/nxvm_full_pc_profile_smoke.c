#include <stdio.h>
#include <string.h>

#include "vm/product/full_pc.h"
#include "core/product/runtime/profile.h"

static int verify_profile(void)
{
    const nxvm_runtime_profile_descriptor *profile = nxvm_runtime_profile_get(
        NXVM_RUNTIME_PROFILE_NXVM_FULL_PC);
    const nxvm_runtime_profile_descriptor *minimal = nxvm_runtime_profile_get(
        NXVM_RUNTIME_PROFILE_NTVDM64_DOS_MINIMAL);

    return profile == NULL || strcmp(profile->name, "nxvm.full_pc") != 0 ||
           !profile->permits_disk_boot || !profile->uses_legacy_adapter ||
           (profile->devices & (NXVM_RUNTIME_DEVICE_BIOS |
                                NXVM_RUNTIME_DEVICE_CMOS |
                                NXVM_RUNTIME_DEVICE_DMA |
                                NXVM_RUNTIME_DEVICE_FDC |
                                NXVM_RUNTIME_DEVICE_FDD |
                                NXVM_RUNTIME_DEVICE_HDC |
                                NXVM_RUNTIME_DEVICE_HDD |
                                NXVM_RUNTIME_DEVICE_VADP)) == 0u ||
           minimal == NULL || minimal->permits_disk_boot ||
           minimal->uses_legacy_adapter ||
           (minimal->devices & (NXVM_RUNTIME_DEVICE_BIOS |
                                NXVM_RUNTIME_DEVICE_FDD |
                                NXVM_RUNTIME_DEVICE_HDD)) != 0u;
}

static int verify_image(const char *fdd, const char *hdd, int boot_hdd)
{
    nxvm_full_pc_config config = { fdd, hdd, 0, 0u, boot_hdd };
    nxvm_vm_reset_vector vector;

    if (nxvm_full_pc_create(&config) != NXVM_CORE_STATUS_OK) {
        return 1;
    }
    if (nxvm_full_pc_get_reset_vector(&vector) != NXVM_CORE_STATUS_OK ||
        vector.cs != 0xf000u || vector.ip != 0xfff0u) {
        nxvm_full_pc_destroy();
        return 1;
    }
    nxvm_full_pc_destroy();
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
