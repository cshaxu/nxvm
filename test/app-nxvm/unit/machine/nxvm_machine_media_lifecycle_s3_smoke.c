#include "ibmpc/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "lib/storage/file_interface.h"

#include "../support/ibmpc/machine/support/common_machine_fixture.h"
#include "../support/rom/session_assets.h"

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
    if (failed) (void)lib_c_remove(path);
    return failed ? -1 : 0;
}

lib_i32 main(void)
{
    vm_machine_config config = {
        .create_fdd = 1,
        .create_hdd_cylinders = 1u
    };
    vm_machine *session = LIB_NULL;
    vm_machine_information information;
    static const char floppy_path[] = "t531-media-lifecycle.img";
    lib_i32 failed = 0;

    if (vm_machine_media_create_floppy(floppy_path) != 0) return 1;
    if (vm_test_default_pc_at_session_create(&config, &session) != LIB_STATUS_OK ||
        session == LIB_NULL || vm_test_common_machine_bind(session) != LIB_STATUS_OK) {
        vm_test_common_machine_unbind(session);
        vm_machine_destroy(session);
        (void)lib_c_remove(floppy_path);
        return 1;
    }
    /* App owns the selected default factory/media combination, not generic
     * generation or running-admission rules now proven in test/ibmpc. */
    failed |= vm_machine_insert_fdd(session, floppy_path) != 0 ||
        vm_machine_get_information(session, &information) != LIB_STATUS_OK ||
        !information.floppy_media_inserted || information.floppy_image_bytes != 1440u * 1024u ||
        !information.fixed_disk_present;
    vm_test_common_machine_unbind(session);
    vm_machine_destroy(session);
    (void)lib_c_remove(floppy_path);
    if (failed) return 1;
    lib_c_printf("M5:T404:S3:MEDIA-LIFECYCLE:OK\n");
    return 0;
}
