#include "type.h"



#include "vdm/machine/dos_minimal.h"

#include "vdm/profile/dos_minimal_profile.h"

C_INT main(C_VOID)
{
    const core_product_runtime_profile_descriptor *profile;
    vdm_machine_dos_minimal *session = STD_NULL;
    vdm_machine_text_snapshot snapshot;
    uint32_t value;

    profile = ntvdm64_dos_minimal_profile_descriptor();
    if (profile == STD_NULL || profile->permits_disk_boot || profile->uses_legacy_adapter ||
        (profile->devices & (NXVM_RUNTIME_DEVICE_BIOS |
                             NXVM_RUNTIME_DEVICE_FDD |
                             NXVM_RUNTIME_DEVICE_HDD |
                             NXVM_RUNTIME_DEVICE_VADP)) != 0u ||
        vdm_machine_dos_minimal_create(&session) != NTVDM64_STATUS_OK ||
        vdm_machine_dos_minimal_tick(session, 3u) != NTVDM64_STATUS_OK ||
        vdm_machine_dos_minimal_inject_key(session, 0x1eu) != NTVDM64_STATUS_OK ||
        vdm_machine_dos_minimal_port_read(session, 0x20u, &value) != NTVDM64_STATUS_OK ||
        value != 0x02u ||
        vdm_machine_dos_minimal_port_read(session, 0x60u, &value) != NTVDM64_STATUS_OK ||
        value != 0x1eu ||
        vdm_machine_dos_minimal_write_text(session, 0u, 'N', 0x1fu) != NTVDM64_STATUS_OK ||
        vdm_machine_dos_minimal_get_snapshot(session, &snapshot) != NTVDM64_STATUS_OK ||
        snapshot.pit_ticks != 3u || snapshot.keyboard_irq_pending != 0u ||
        snapshot.text.columns != CORE_MACHINE_TEXT_COLUMNS ||
        snapshot.text.rows != CORE_MACHINE_TEXT_ROWS ||
        snapshot.text.characters[0] != 'N' ||
        snapshot.text.attributes[0] != 0x1fu) {
        vdm_machine_dos_minimal_destroy(session);
        return 1;
    }

    vdm_machine_dos_minimal_destroy(session);
    puts("M3:T3:S2:DOS-MINIMAL-PROFILE:OK");
    return 0;
}

