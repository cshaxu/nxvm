#include "type.h"

#include "vm/composition/session/control.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/session_interface.h"
#include "../support/rom/session_assets.h"

C_INT main(C_VOID)
{
    vm_session_config config = {
        .create_fdd = 1,
        .create_hdd_cylinders = 1u
    };
    vm_session *session = STD_NULL;
    type_unsigned_64 fdd_generation;
    type_unsigned_64 hdd_generation;
    C_INT failed = 0;

    if (vm_test_default_pc_at_session_create(&config, &session) != TYPE_STATUS_OK ||
        session == STD_NULL)
        return 1;
    fdd_generation = session->fdd.connect.media_generation;
    hdd_generation = session->hdd.connect.media_generation;
    failed |= vm_session_insert_hdd(session, "t404-fixed.img") == 0 ||
        session->hdd.connect.media_generation != hdd_generation ||
        session->hdd_image_path[0] != '\0';
    failed |= vm_session_insert_fdd(session, "t404-removable.img") == 0 ||
        session->fdd.connect.media_generation != fdd_generation ||
        session->fdd_image_path[0] != '\0';
    STD_ATOMIC_STORE(&session->control.flagRun, TYPE_TRUE);
    failed |= vm_session_insert_fdd(session, "t404-running-removable.img") == 0 ||
        session->fdd.connect.media_generation != fdd_generation ||
        session->fdd_image_path[0] != '\0';
    failed |= vm_session_remove_fdd(session, STD_NULL) == 0 ||
        session->fdd.connect.media_generation != fdd_generation ||
        session->fdd_image_path[0] != '\0';
    STD_ATOMIC_STORE(&session->control.flagRun, TYPE_FALSE);
    failed |= vm_session_remove_fdd(session, STD_NULL) != 0 ||
        session->fdd.connect.flagDiskExist || session->fdd_image_path[0] != '\0' ||
        session->retained_config.floppy_image[0u] != STD_NULL;
    vm_session_destroy(session);
    if (failed) return 1;
    STD_PRINTF("M5:T404:S3:MEDIA-LIFECYCLE:OK\n");
    return 0;
}
