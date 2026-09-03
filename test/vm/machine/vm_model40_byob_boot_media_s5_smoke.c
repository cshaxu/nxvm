#include "type.h"

#include "core/machine/media_interface.h"
#include "vm/composition/session/media.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"
#include "vm/machine/fdd.h"
#include "../support/rom/model40_session_assets.h"

#define MODEL40_FDD_BYTES (80u * 2u * 15u * 512u)

C_INT main(C_VOID)
{
    static type_unsigned_8 even[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    static type_unsigned_8 odd[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    static type_unsigned_8 image[MODEL40_FDD_BYTES];
    type_unsigned_8 cmos_seed[VM_SESSION_CMOS_SEED_BYTES];
    vm_session_config config = {0};
    vm_session_assets assets = {0};
    vm_session *session = STD_NULL;
    core_machine_media_info info;
    core_machine_media_result result;
    C_INT failed = 0;

    STD_MEMSET(odd, 1, sizeof(odd));
    image[0] = 0xebu;
    image[1] = 0x3cu;
    image[510] = 0x55u;
    image[511] = 0xaau;
    config.profile_kind = VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40;
    config.bios_count = 2u;
    vm_model40_fixture_cmos_seed(cmos_seed);
    assets.bios[0u] = (vm_session_asset_bytes) { even, sizeof(even) };
    assets.bios[1u] = (vm_session_asset_bytes) { odd, sizeof(odd) };
    assets.cmos_seed = (vm_session_asset_bytes) { cmos_seed, sizeof(cmos_seed) };
    failed |= vm_session_create_from_assets(&config, &assets, &session) != TYPE_STATUS_OK ||
        session == STD_NULL || vm_machine_fdd_replace_bytes(&session->fdd, image,
            sizeof(image)) != TYPE_FALSE || !session->fdd.connect.flagDiskExist ||
        session->fdd.data.ncyl != 80u || session->fdd.data.nhead != 2u ||
        session->fdd.data.nsector != 15u || session->fdd.data.nbyte != 512u ||
        core_machine_media_query(session->media_registry, VM_SESSION_MEDIA_FDD_ID,
            &info, &result) != TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        !info.present || info.geometry.logical_sector_count != 2400u ||
        info.geometry.bytes_per_sector != 512u;
    vm_session_destroy(session);
    if (!failed) STD_PRINTF("M5:T390:S5:MODEL40-BYOB-BOOT-MEDIA:OK\n");
    return failed;
}
