#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/media_interface.h"
#include "app-nxvm/machine/lifecycle.h"
#include "app-nxvm/machine/media/media.h"
#include "app-nxvm/machine/machine_private.h"
#include "support/rom/model40_session_assets.h"
#include "support/rom/session_assets.h"

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
    lib_i32 failed = 0;

    if (vm_model40_fixture_create(&model40) != LIB_STATUS_OK ||
        model40 == LIB_NULL || model40->floppy_kind != VM_PROFILE_FLOPPY_525_1200K ||
        model40->fdd.data.ncyl != 80u || model40->fdd.data.nhead != 2u ||
        model40->fdd.data.nsector != 15u || model40->fdd.data.nbyte != 512u ||
        vm_machine_fdd_image_size(&model40->fdd) != MODEL40_FDD_BYTES ||
        vm_machine_fdd_replace_bytes(&model40->fdd, image, sizeof(image) - 1u) ==
            LIB_FALSE ||
        vm_machine_fdd_replace_bytes(&model40->fdd, image, sizeof(image)) !=
            LIB_FALSE ||
        core_machine_media_query(model40->media_registry, VM_MACHINE_MEDIA_FDD_ID,
            &info, &result) != LIB_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        !info.present || info.geometry.cylinders != 80u || info.geometry.heads != 2u ||
        info.geometry.sectors_per_track != 15u || info.geometry.bytes_per_sector != 512u ||
        model40->core_machine->fdc.connect.config.irq != 6u ||
        model40->core_machine->fdc.connect.config.dma_channel != 2u) {
        failed = 1;
        goto done;
    }

    vm_machine_reset(model40);
    if (model40->fdd.data.ncyl != 80u || model40->fdd.data.nhead != 2u ||
        model40->fdd.data.nsector != 15u || model40->fdd.data.nbyte != 512u ||
        !model40->fdd.connect.flagDiskExist) {
        failed = 1;
        goto done;
    }

    {
        lib_u8 even_bytes[VM_PROFILE_MODEL40_ROM_CHIP_BYTES] = {0};
        lib_u8 odd_bytes[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
        static lib_u8 compatible_media[MODEL40_COMPATIBLE_MEDIA_BYTES];

        lib_memory_set(odd_bytes, 1, sizeof(odd_bytes));
        if (vm_model40_fixture_create_bytes_with_floppy_format(even_bytes, odd_bytes,
                VM_MACHINE_FLOPPY_FORMAT_360K, &model40_360k) !=
                LIB_STATUS_OK || model40_360k == LIB_NULL ||
            model40_360k->floppy_kind != VM_PROFILE_FLOPPY_525_1200K ||
            model40_360k->fdd_media_kind != VM_PROFILE_FLOPPY_525_360K ||
            model40_360k->fdd.data.ncyl != 40u ||
            vm_machine_fdd_replace_bytes(&model40_360k->fdd, compatible_media,
                sizeof(compatible_media)) != LIB_FALSE) {
            failed = 1;
            goto done;
        }
    }

    model339_config.profile_kind = VM_MACHINE_PROFILE_IBM_5170_MODEL_339;
    if (vm_test_default_pc_at_session_create(LIB_NULL, &default_session) != LIB_STATUS_OK ||
        vm_test_ibm_5170_session_create(&model339_config, &model339) != LIB_STATUS_OK ||
        default_session->fdd.data.nsector != 18u ||
        model339->fdd.data.nsector != 15u) {
        failed = 1;
    }

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
