#include "vm/profile/full_pc_profile.h"

static const core_product_runtime_profile_descriptor vm_profile_full_pc_profile = {
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
};

const core_product_runtime_profile_descriptor *vm_profile_full_pc_profile_descriptor(void)
{
    return &vm_profile_full_pc_profile;
}
