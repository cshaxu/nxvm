#include "app-nxvm/profiles/machine_factory_interface.h"
#include "../../../support/media.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "lib/storage/file_interface.h"

#include "ibmpc/machine/control.h"
#include "ibmpc/machine/machine_private.h"
#include "ibmpc/machine/machine_interface.h"
#include "support/common_machine_fixture.h"
#include "support/rom/session_assets.h"

static lib_i32 vm_machine_media_create_floppy(const char *path)
{
    lib_storage_file_writer *writer = LIB_NULL;
    static const lib_u8 zeroes[4096u];
    lib_size remaining = 1440u * 1024u;
    lib_i32 failed = 0;

    if (path == LIB_NULL || lib_storage_file_writer_open(path,
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
    if (failed) (void)remove(path);
    return failed ? -1 : 0;
}

lib_i32 main(void)
{
    vm_machine_config config = {
        .create_fdd = 1,
        .create_hdd_cylinders = 1u
    };
    vm_machine *session = LIB_NULL;
    lib_u64 fdd_generation;
    static const char floppy_path[] = "t531-media-lifecycle.img";
    lib_i32 failed = 0;

    if (vm_machine_media_create_floppy(floppy_path) != 0) return 1;
    if (vm_test_default_pc_at_session_create(&config, &session) != LIB_STATUS_OK ||
        session == LIB_NULL || vm_test_common_machine_bind(session) != LIB_STATUS_OK) {
        vm_test_common_machine_unbind(session);
        vm_machine_destroy(session);
        (void)remove(floppy_path);
        return 1;
    }
    fdd_generation = vm_test_fdd_info(session->fdd).generation;
    failed |= vm_machine_insert_fdd(session, floppy_path) != 0 ||
        vm_test_fdd_info(session->fdd).generation != fdd_generation + 1u ||
        session->fdd_image_path[0] == '\0';
    fdd_generation = vm_test_fdd_info(session->fdd).generation;
    vm_machine_executor_state_start(session->control.state);
    failed |= vm_machine_insert_fdd(session, "t404-running-removable.img") == 0 ||
        vm_test_fdd_info(session->fdd).generation != fdd_generation ||
        session->fdd_image_path[0] == '\0';
    failed |= vm_machine_eject_fdd(session) == 0 ||
        vm_test_fdd_info(session->fdd).generation != fdd_generation ||
        session->fdd_image_path[0] == '\0';
    vm_machine_executor_state_stop(session->control.state);
    failed |= vm_machine_eject_fdd(session) != 0 ||
        vm_test_fdd_info(session->fdd).present || session->fdd_image_path[0] != '\0';
    vm_test_common_machine_unbind(session);
    vm_machine_destroy(session);
    (void)remove(floppy_path);
    if (failed) return 1;
    printf("M5:T404:S3:MEDIA-LIFECYCLE:OK\n");
    return 0;
}
