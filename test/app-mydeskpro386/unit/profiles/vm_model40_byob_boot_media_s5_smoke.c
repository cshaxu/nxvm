#include "../../../app-nxvm/unit/support/profile.h"
#include "core/machine/machine_interface.h"
#include "../../../app-nxvm/unit/support/ibmpc/machine/support/media.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "core/board-base/media_interface.h"
#include "core/machine/media/media_interface.h"
#include "core/machine/machine_interface.h"
#include "core/machine/machine_private.h"
#include "core/machine/media/fdd_interface.h"
#include "../../support/rom/model40_session_assets.h"

#define MODEL40_FDD_BYTES (80u * 2u * 15u * 512u)

lib_i32 main(void)
{
    static lib_u8 even[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    static lib_u8 odd[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    static lib_u8 image[MODEL40_FDD_BYTES];
    lib_u8 cmos_seed[VM_MACHINE_CMOS_SEED_BYTES];
    vm_machine_config config = {0};
    vm_machine_assets assets = {0};
    vm_machine *session = LIB_NULL;
    core_machine_media_info info;
    core_machine_media_result result;
    lib_i32 failed = 0;

    lib_memory_set(odd, 1, sizeof(odd));
    image[0] = 0xebu;
    image[1] = 0x3cu;
    image[510] = 0x55u;
    image[511] = 0xaau;
    config.bios_count = 2u;
    vm_model40_fixture_cmos_seed(cmos_seed);
    assets.bios[0u] = (vm_machine_asset_bytes) { even, sizeof(even) };
    assets.bios[1u] = (vm_machine_asset_bytes) { odd, sizeof(odd) };
    assets.cmos_seed = (vm_machine_asset_bytes) { cmos_seed, sizeof(cmos_seed) };
    failed |= vm_test_machine_create_from_assets(VM_MACHINE_PROFILE_COMPAQ_DESKPRO_386_MODEL_40,
        &config, &assets, &session) != LIB_STATUS_OK ||
        session == LIB_NULL || vm_machine_fdd_replace_bytes(session->floppy[0u], image,
            sizeof(image)) != LIB_FALSE || !vm_test_fdd_info(session->floppy[0u]).present ||
        vm_test_fdd_info(session->floppy[0u]).geometry.cylinders != 80u || vm_test_fdd_info(session->floppy[0u]).geometry.heads != 2u ||
        vm_test_fdd_info(session->floppy[0u]).geometry.sectors_per_track != 15u || vm_test_fdd_info(session->floppy[0u]).geometry.bytes_per_sector != 512u ||
        core_machine_media_query(session->media_registry, VM_MACHINE_MEDIA_FDD_ID,
            &info, &result) != LIB_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        !info.present || info.geometry.logical_sector_count != 2400u ||
        info.geometry.bytes_per_sector != 512u;
    vm_machine_destroy(session);
    if (!failed) printf("M5:T390:S5:MODEL40-BYOB-BOOT-MEDIA:OK\n");
    return failed;
}
