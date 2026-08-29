#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/media_interface.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/media.h"
#include "vm/composition/session/session_private.h"
#include "../support/vm_model40_byob_fixture.h"

#define MODEL40_FDD_BYTES (80u * 2u * 15u * 512u)

C_INT main(C_VOID)
{
    static type_unsigned_8 image[MODEL40_FDD_BYTES];
    vm_session_config model339_config = {0};
    vm_session *model40 = STD_NULL;
    vm_session *default_session = STD_NULL;
    vm_session *model339 = STD_NULL;
    core_machine_media_info info;
    core_machine_media_result result;
    C_INT failed = 0;

    if (vm_model40_fixture_create("t386-s18-even.bin", "t386-s18-odd.bin", &model40) != TYPE_STATUS_OK ||
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

    model339_config.profile_kind = VM_SESSION_PROFILE_IBM_5170_MODEL_339;
    if (vm_session_create(STD_NULL, &default_session) != TYPE_STATUS_OK ||
        vm_session_create(&model339_config, &model339) != TYPE_STATUS_OK ||
        default_session->fdd.data.nsector != 18u ||
        model339->fdd.data.nsector != 15u) {
        failed = 1;
    }

done:
    vm_session_destroy(model339);
    vm_session_destroy(default_session);
    vm_session_destroy(model40);
    vm_model40_fixture_remove("t386-s18-even.bin", "t386-s18-odd.bin");
    if (failed) return 1;
    STD_PRINTF("M5:T386:S18:MODEL40-FDD-GEOMETRY:OK\n");
    STD_PRINTF("M5:T386:S18:MODEL40-FDD-MEDIA:OK\n");
    STD_PRINTF("M5:T386:S18:MODEL40-FDD-RESET-BINDING:OK\n");
    return 0;
}
