#include "type.h"

#include "vm/machine/runtime/control.h"
#include "vm/machine/runtime/machine_private.h"
#include "vm/machine/runtime/machine_interface.h"
#include "../support/rom/session_assets.h"

static C_INT vm_machine_media_create_floppy(const C_CHAR *path)
{
    STD_FILE *file;

    if (path == STD_NULL) return -1;
    file = STD_FOPEN(path, "wb");
    if (file == STD_NULL) return -1;
    if (STD_FSEEK(file, 1440u * 1024u - 1u, SEEK_SET) != 0 ||
        STD_FPUTC(0, file) == EOF) {
        (C_VOID)STD_FCLOSE(file);
        (C_VOID)STD_REMOVE(path);
        return -1;
    }
    if (STD_FCLOSE(file) != 0) {
        (C_VOID)STD_REMOVE(path);
        return -1;
    }
    return 0;
}

C_INT main(C_VOID)
{
    vm_machine_config config = {
        .create_fdd = 1,
        .create_hdd_cylinders = 1u
    };
    vm_machine *session = STD_NULL;
    type_unsigned_64 fdd_generation;
    type_unsigned_64 hdd_generation;
    static const C_CHAR floppy_path[] = "t531-media-lifecycle.img";
    C_INT failed = 0;

    if (vm_machine_media_create_floppy(floppy_path) != 0) return 1;
    if (vm_test_default_pc_at_session_create(&config, &session) != TYPE_STATUS_OK ||
        session == STD_NULL) {
        (C_VOID)STD_REMOVE(floppy_path);
        return 1;
    }
    fdd_generation = session->fdd.connect.media_generation;
    hdd_generation = session->hdd.connect.media_generation;
    failed |= vm_machine_insert_hdd(session, "t404-fixed.img") == 0 ||
        session->hdd.connect.media_generation != hdd_generation ||
        session->hdd_image_path[0] != '\0';
    failed |= vm_machine_insert_fdd(session, floppy_path) != 0 ||
        session->fdd.connect.media_generation != fdd_generation + 1u ||
        session->fdd_image_path[0] == '\0';
    fdd_generation = session->fdd.connect.media_generation;
    vm_machine_executor_state_start(session->control.state);
    failed |= vm_machine_insert_fdd(session, "t404-running-removable.img") == 0 ||
        session->fdd.connect.media_generation != fdd_generation ||
        session->fdd_image_path[0] == '\0';
    failed |= vm_machine_remove_fdd(session, STD_NULL) == 0 ||
        session->fdd.connect.media_generation != fdd_generation ||
        session->fdd_image_path[0] == '\0';
    vm_machine_executor_state_stop(session->control.state);
    failed |= vm_machine_remove_fdd(session, STD_NULL) != 0 ||
        session->fdd.connect.flagDiskExist || session->fdd_image_path[0] != '\0' ||
        session->retained_config.floppy_image[0u] != STD_NULL;
    vm_machine_destroy(session);
    (C_VOID)STD_REMOVE(floppy_path);
    if (failed) return 1;
    STD_PRINTF("M5:T404:S3:MEDIA-LIFECYCLE:OK\n");
    return 0;
}
