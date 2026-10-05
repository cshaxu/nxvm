#include "ibmpc/machine/machine_interface.h"
#include "../../../app-nxvm/unit/support/ibmpc/machine/support/media.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "../../../app-nxvm/unit/support/ibmpc/board-common/controller_fixture.h"
#include "ibmpc/board-common/media_interface.h"
#include "ibmpc/machine/lifecycle.h"
#include "ibmpc/machine/media/media_interface.h"
#include "ibmpc/machine/machine_private.h"
#include "../../support/rom/model40_session_assets.h"
#include "../../../app-nxvm/unit/support/rom/session_assets.h"

#define MODEL40_FDD_BYTES (80u * 2u * 15u * 512u)
#define MODEL40_COMPATIBLE_MEDIA_BYTES (40u * 2u * 9u * 512u)

lib_i32 main(void)
{
    static lib_u8 image[MODEL40_FDD_BYTES];
    vm_machine_config model339_config = {0};
    vm_machine *model40 = LIB_NULL;
    vm_machine *model40_360k = LIB_NULL;
    vm_machine *default_session = LIB_NULL;
    vm_machine *model339 = LIB_NULL;
    core_machine_media_info info;
    core_machine_media_result result;
    lib_i32 failed = 1;

    if (vm_model40_fixture_create(&model40) != LIB_STATUS_OK ||
        model40 == LIB_NULL || model40->construction.floppy_kind != VM_PROFILE_FLOPPY_525_1200K ||
        vm_test_fdd_info(model40->floppy[0u]).geometry.cylinders != 80u || vm_test_fdd_info(model40->floppy[0u]).geometry.heads != 2u ||
        vm_test_fdd_info(model40->floppy[0u]).geometry.sectors_per_track != 15u || vm_test_fdd_info(model40->floppy[0u]).geometry.bytes_per_sector != 512u ||
        vm_machine_fdd_image_size(model40->floppy[0u]) != MODEL40_FDD_BYTES ||
        vm_machine_fdd_replace_bytes(model40->floppy[0u], image, sizeof(image) - 1u) ==
            LIB_FALSE ||
        vm_machine_fdd_replace_bytes(model40->floppy[0u], image, sizeof(image)) !=
            LIB_FALSE ||
        core_machine_media_query(model40->media_registry, VM_MACHINE_MEDIA_FDD_ID,
            &info, &result) != LIB_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        !info.present || info.geometry.cylinders != 80u || info.geometry.heads != 2u ||
        info.geometry.sectors_per_track != 15u || info.geometry.bytes_per_sector != 512u)
        goto done;
    const core_machine_fdc_config config = test_board_fdc_connection_config(model40->board);
    if (config.irq != 6u || config.dma_channel != 2u) goto done;

    if (vm_machine_reset(model40) != LIB_STATUS_OK ||
        vm_test_fdd_info(model40->floppy[0u]).geometry.cylinders != 80u || vm_test_fdd_info(model40->floppy[0u]).geometry.heads != 2u ||
        vm_test_fdd_info(model40->floppy[0u]).geometry.sectors_per_track != 15u || vm_test_fdd_info(model40->floppy[0u]).geometry.bytes_per_sector != 512u ||
        !vm_test_fdd_info(model40->floppy[0u]).present) goto done;

    {
        lib_u8 even_bytes[VM_PROFILE_MODEL40_ROM_CHIP_BYTES] = {0};
        lib_u8 odd_bytes[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
        static lib_u8 compatible_media[MODEL40_COMPATIBLE_MEDIA_BYTES];

        lib_memory_set(odd_bytes, 1, sizeof(odd_bytes));
        if (vm_model40_fixture_create_bytes_with_floppy_format(even_bytes, odd_bytes,
                VM_MACHINE_FLOPPY_FORMAT_360K, &model40_360k) !=
                LIB_STATUS_OK || model40_360k == LIB_NULL ||
            model40_360k->construction.floppy_kind != VM_PROFILE_FLOPPY_525_1200K ||
            model40_360k->construction.media_kind != VM_PROFILE_FLOPPY_525_360K ||
            vm_test_fdd_info(model40_360k->floppy[0u]).geometry.cylinders != 40u ||
            vm_machine_fdd_replace_bytes(model40_360k->floppy[0u], compatible_media,
                sizeof(compatible_media)) != LIB_FALSE) goto done;
    }

    if (vm_test_default_pc_at_session_create(LIB_NULL, &default_session) != LIB_STATUS_OK ||
        default_session == LIB_NULL ||
        vm_test_ibm_5170_session_create(&model339_config, &model339) != LIB_STATUS_OK ||
        model339 == LIB_NULL ||
        vm_test_fdd_info(default_session->floppy[0u]).geometry.sectors_per_track != 18u ||
        vm_test_fdd_info(model339->floppy[0u]).geometry.sectors_per_track != 15u) goto done;
    failed = 0;

done:
    vm_machine_destroy(model40_360k);
    vm_machine_destroy(model339);
    vm_machine_destroy(default_session);
    vm_machine_destroy(model40);
    if (failed) return 1;
    printf("M5:T386:S18:MODEL40-FDD-GEOMETRY:OK\n");
    printf("M5:T386:S18:MODEL40-FDD-MEDIA:OK\n");
    printf("M5:T386:S18:MODEL40-FDD-RESET-BINDING:OK\n");
    return 0;
}
