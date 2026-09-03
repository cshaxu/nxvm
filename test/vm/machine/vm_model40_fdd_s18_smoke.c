#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/media_interface.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/media.h"
#include "vm/composition/session/session_private.h"
#include "../support/rom/model40_session_assets.h"
#include "../support/rom/session_assets.h"

#define MODEL40_FDD_BYTES (80u * 2u * 15u * 512u)
#define MODEL40_COMPATIBLE_MEDIA_BYTES (40u * 2u * 9u * 512u)

C_INT main(C_VOID)
{
    static type_unsigned_8 image[MODEL40_FDD_BYTES];
    vm_session_config model339_config = {0};
    vm_session *model40 = STD_NULL;
    vm_session *model40_360k = STD_NULL;
    vm_session *default_session = STD_NULL;
    vm_session *model339 = STD_NULL;
    core_machine_media_info info;
    core_machine_media_result result;
    C_INT failed = 0;

    if (vm_model40_fixture_create(&model40) != TYPE_STATUS_OK ||
        model40 == STD_NULL || model40->floppy_kind != VM_PROFILE_FLOPPY_525_1200K ||
        model40->fdd.data.ncyl != 80u || model40->fdd.data.nhead != 2u ||
        model40->fdd.data.nsector != 15u || model40->fdd.data.nbyte != 512u ||
        vm_machine_fdd_image_size(&model40->fdd) != MODEL40_FDD_BYTES ||
        vm_machine_fdd_replace_bytes(&model40->fdd, image, sizeof(image) - 1u) ==
            TYPE_FALSE ||
        vm_machine_fdd_replace_bytes(&model40->fdd, image, sizeof(image)) !=
            TYPE_FALSE ||
        core_machine_media_query(model40->media_registry, VM_SESSION_MEDIA_FDD_ID,
            &info, &result) != TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        !info.present || info.geometry.cylinders != 80u || info.geometry.heads != 2u ||
        info.geometry.sectors_per_track != 15u || info.geometry.bytes_per_sector != 512u ||
        model40->core_machine->fdc.connect.config.irq != 6u ||
        model40->core_machine->fdc.connect.config.dma_channel != 2u) {
        failed = 1;
        goto done;
    }

    vm_session_reset(model40);
    if (model40->fdd.data.ncyl != 80u || model40->fdd.data.nhead != 2u ||
        model40->fdd.data.nsector != 15u || model40->fdd.data.nbyte != 512u ||
        !model40->fdd.connect.flagDiskExist) {
        failed = 1;
        goto done;
    }

    {
        type_unsigned_8 even_bytes[VM_PROFILE_MODEL40_ROM_CHIP_BYTES] = {0};
        type_unsigned_8 odd_bytes[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
        static type_unsigned_8 compatible_media[MODEL40_COMPATIBLE_MEDIA_BYTES];

        STD_MEMSET(odd_bytes, 1, sizeof(odd_bytes));
        if (vm_model40_fixture_create_bytes_with_floppy_format(even_bytes, odd_bytes,
                VM_SESSION_FLOPPY_FORMAT_360K, &model40_360k) !=
                TYPE_STATUS_OK || model40_360k == STD_NULL ||
            model40_360k->floppy_kind != VM_PROFILE_FLOPPY_525_1200K ||
            model40_360k->fdd_media_kind != VM_PROFILE_FLOPPY_525_360K ||
            model40_360k->fdd.data.ncyl != 40u ||
            vm_machine_fdd_replace_bytes(&model40_360k->fdd, compatible_media,
                sizeof(compatible_media)) != TYPE_FALSE) {
            failed = 1;
            goto done;
        }
    }

    model339_config.profile_kind = VM_SESSION_PROFILE_IBM_5170_MODEL_339;
    if (vm_test_default_pc_at_session_create(STD_NULL, &default_session) != TYPE_STATUS_OK ||
        vm_test_ibm_5170_session_create(&model339_config, &model339) != TYPE_STATUS_OK ||
        default_session->fdd.data.nsector != 18u ||
        model339->fdd.data.nsector != 15u) {
        failed = 1;
    }

done:
    vm_session_destroy(model40_360k);
    vm_session_destroy(model339);
    vm_session_destroy(default_session);
    vm_session_destroy(model40);
    if (failed) return 1;
    STD_PRINTF("M5:T386:S18:MODEL40-FDD-GEOMETRY:OK\n");
    STD_PRINTF("M5:T386:S18:MODEL40-FDD-MEDIA:OK\n");
    STD_PRINTF("M5:T386:S18:MODEL40-FDD-RESET-BINDING:OK\n");
    return 0;
}
