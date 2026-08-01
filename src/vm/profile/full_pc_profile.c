#include "core/product/runtime/profile.h"

static const nxvm_runtime_profile_descriptor nxvm_runtime_profiles[] = {
    {
        NXVM_RUNTIME_PROFILE_NXVM_FULL_PC,
        "nxvm.full_pc",
        NXVM_RUNTIME_DEVICE_CPU | NXVM_RUNTIME_DEVICE_RAM |
            NXVM_RUNTIME_DEVICE_PORTS | NXVM_RUNTIME_DEVICE_PIC |
            NXVM_RUNTIME_DEVICE_PIT | NXVM_RUNTIME_DEVICE_KEYBOARD |
            NXVM_RUNTIME_DEVICE_TEXT_DISPLAY | NXVM_RUNTIME_DEVICE_BIOS |
            NXVM_RUNTIME_DEVICE_CMOS | NXVM_RUNTIME_DEVICE_DMA |
            NXVM_RUNTIME_DEVICE_FDC | NXVM_RUNTIME_DEVICE_FDD |
            NXVM_RUNTIME_DEVICE_HDC | NXVM_RUNTIME_DEVICE_HDD |
            NXVM_RUNTIME_DEVICE_VADP,
        1,
        1
    },
    {
        NXVM_RUNTIME_PROFILE_NTVDM64_DOS_MINIMAL,
        "ntvdm64.dos_minimal",
        NXVM_RUNTIME_DEVICE_CPU | NXVM_RUNTIME_DEVICE_RAM |
            NXVM_RUNTIME_DEVICE_PORTS | NXVM_RUNTIME_DEVICE_PIC |
            NXVM_RUNTIME_DEVICE_PIT | NXVM_RUNTIME_DEVICE_KEYBOARD |
            NXVM_RUNTIME_DEVICE_TEXT_DISPLAY,
        0,
        0
    }
};

const nxvm_runtime_profile_descriptor *nxvm_runtime_profile_get(
    nxvm_runtime_profile_id id)
{
    unsigned index;

    for (index = 0u; index < sizeof(nxvm_runtime_profiles) /
                                sizeof(nxvm_runtime_profiles[0]);
         ++index) {
        if (nxvm_runtime_profiles[index].id == id) {
            return &nxvm_runtime_profiles[index];
        }
    }

    return NULL;
}
