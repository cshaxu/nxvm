#include "vdm/profile/dos_minimal_profile.h"

static const nxvm_runtime_profile_descriptor ntvdm64_dos_minimal_profile = {
    "ntvdm64.dos_minimal",
    NXVM_RUNTIME_DEVICE_CPU | NXVM_RUNTIME_DEVICE_RAM |
        NXVM_RUNTIME_DEVICE_PORTS | NXVM_RUNTIME_DEVICE_PIC |
        NXVM_RUNTIME_DEVICE_PIT | NXVM_RUNTIME_DEVICE_KEYBOARD |
        NXVM_RUNTIME_DEVICE_TEXT_DISPLAY,
    0,
    0
};

const nxvm_runtime_profile_descriptor *ntvdm64_dos_minimal_profile_descriptor(void)
{
    return &ntvdm64_dos_minimal_profile;
}
