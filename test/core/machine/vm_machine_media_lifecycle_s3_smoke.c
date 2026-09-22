#include "type.h"
#include "type.h"

#include "lib/storage/file_interface.h"

#include "core/machine/control.h"
#include "core/machine/machine_private.h"
#include "core/machine/machine_interface.h"
#include "support/common_machine_fixture.h"
#include "support/rom/session_assets.h"

static C_INT vm_machine_media_create_floppy(const C_CHAR *path)
{
    lib_storage_file_writer *writer = LIB_NULL;
    static const lib_u8 zeroes[4096u];
    lib_size remaining = 1440u * 1024u;
    C_INT failed = 0;

    if (path == STD_NULL || lib_storage_file_writer_open(path,
            LIB_STORAGE_FILE_WRITER_TRUNCATE, &writer) != LIB_STATUS_OK) return -1;
    while (remaining != 0u) {
        const lib_size chunk = remaining < sizeof(zeroes) ? remaining : sizeof(zeroes);

        if (lib_storage_file_writer_write(writer, zeroes, chunk) != LIB_STATUS_OK) {
            failed = 1;
            break;
        }
        remaining -= chunk;
    }
    if (lib_storage_file_writer_close(writer) != LIB_STATUS_OK) failed = 1;
    if (failed) (C_VOID)remove(path);
    return failed ? -1 : 0;
}

C_INT main(C_VOID)
{
    vm_machine_config config = {
        .create_fdd = 1,
        .create_hdd_cylinders = 1u
    };
    vm_machine *session = STD_NULL;
    type_unsigned_64 fdd_generation;
    static const C_CHAR floppy_path[] = "t531-media-lifecycle.img";
    C_INT failed = 0;

    if (vm_machine_media_create_floppy(floppy_path) != 0) return 1;
    if (vm_test_default_pc_at_session_create(&config, &session) != TYPE_STATUS_OK ||
        session == STD_NULL || vm_test_common_machine_bind(session) != TYPE_STATUS_OK) {
        vm_test_common_machine_unbind(session);
        vm_machine_destroy(session);
        (C_VOID)remove(floppy_path);
        return 1;
    }
    fdd_generation = session->fdd.connect.media_generation;
    failed |= vm_machine_insert_fdd(session, floppy_path) != 0 ||
        session->fdd.connect.media_generation != fdd_generation + 1u ||
        session->fdd_image_path[0] == '\0';
    fdd_generation = session->fdd.connect.media_generation;
    vm_machine_executor_state_start(session->control.state);
    failed |= vm_machine_insert_fdd(session, "t404-running-removable.img") == 0 ||
        session->fdd.connect.media_generation != fdd_generation ||
        session->fdd_image_path[0] == '\0';
    failed |= vm_machine_eject_fdd(session) == 0 ||
        session->fdd.connect.media_generation != fdd_generation ||
        session->fdd_image_path[0] == '\0';
    vm_machine_executor_state_stop(session->control.state);
    failed |= vm_machine_eject_fdd(session) != 0 ||
        session->fdd.connect.flagDiskExist || session->fdd_image_path[0] != '\0' ||
        session->retained_config.floppy_image[0u] != STD_NULL;
    vm_test_common_machine_unbind(session);
    vm_machine_destroy(session);
    (C_VOID)remove(floppy_path);
    if (failed) return 1;
    STD_PRINTF("M5:T404:S3:MEDIA-LIFECYCLE:OK\n");
    return 0;
}
