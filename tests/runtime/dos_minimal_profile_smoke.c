#include <stdio.h>

#include "vdm/machine/dos_minimal.h"
#include "core/product/runtime/profile.h"

int main(void)
{
    const nxvm_runtime_profile_descriptor *profile;
    nxvm_runtime_dos_minimal *session = NULL;
    nxvm_runtime_text_snapshot snapshot;
    uint32_t value;

    profile = nxvm_runtime_profile_get(NXVM_RUNTIME_PROFILE_NTVDM64_DOS_MINIMAL);
    if (profile == NULL || profile->permits_disk_boot || profile->uses_legacy_adapter ||
        (profile->devices & (NXVM_RUNTIME_DEVICE_BIOS |
                             NXVM_RUNTIME_DEVICE_FDD |
                             NXVM_RUNTIME_DEVICE_HDD |
                             NXVM_RUNTIME_DEVICE_VADP)) != 0u ||
        nxvm_runtime_dos_minimal_create(&session) != NXVM_CORE_STATUS_OK ||
        nxvm_runtime_dos_minimal_tick(session, 3u) != NXVM_CORE_STATUS_OK ||
        nxvm_runtime_dos_minimal_inject_key(session, 0x1eu) != NXVM_CORE_STATUS_OK ||
        nxvm_runtime_dos_minimal_port_read(session, 0x20u, &value) != NXVM_CORE_STATUS_OK ||
        value != 0x02u ||
        nxvm_runtime_dos_minimal_port_read(session, 0x60u, &value) != NXVM_CORE_STATUS_OK ||
        value != 0x1eu ||
        nxvm_runtime_dos_minimal_write_text(session, 0u, 'N', 0x1fu) != NXVM_CORE_STATUS_OK ||
        nxvm_runtime_dos_minimal_get_snapshot(session, &snapshot) != NXVM_CORE_STATUS_OK ||
        snapshot.pit_ticks != 3u || snapshot.keyboard_irq_pending != 0u ||
        snapshot.characters[0] != 'N' || snapshot.attributes[0] != 0x1fu) {
        nxvm_runtime_dos_minimal_destroy(session);
        return 1;
    }

    nxvm_runtime_dos_minimal_destroy(session);
    puts("M3:T3:S2:DOS-MINIMAL-PROFILE:OK");
    return 0;
}
