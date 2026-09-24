#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/media_interface.h"
#include "app-nxvm/machine/media/fdd_private.h"
#include "app-nxvm/machine/media/hdd_private.h"
#include "lib/storage/file_interface.h"

static const char vm_media_direct_fdd_path[] = "vm_media_direct_fdd.img";
static const char vm_media_direct_hdd_path[] = "vm_media_direct_hdd.img";
static lib_u8 vm_media_direct_fdd_bytes[80u * 2u * 18u * 512u];
static const core_machine_media_geometry vm_media_direct_fdd_geometry = {
    2880u, 512u, 80u, 2u, 18u
};

static lib_i32 vm_media_direct_write(const char *path, const void *bytes,
    lib_size byte_count)
{
    lib_storage_file_writer *writer = LIB_NULL;
    lib_i32 failed = lib_storage_file_writer_open(path,
        LIB_STORAGE_FILE_WRITER_TRUNCATE, &writer) != LIB_STATUS_OK ||
        (byte_count != 0u && lib_storage_file_writer_write(writer, bytes,
            byte_count) != LIB_STATUS_OK);

    if (writer != LIB_NULL && lib_storage_file_writer_close(writer) !=
            LIB_STATUS_OK) failed = 1;
    return failed;
}

static lib_i32 vm_media_direct_read_first(const char *path,
    lib_u8 expected)
{
    lib_storage_file_reader *reader = LIB_NULL;
    lib_u8 value = 0u;
    lib_i32 failed = lib_storage_file_reader_open(path, &reader) != LIB_STATUS_OK ||
        lib_storage_file_reader_read(reader, &value, 1u) != LIB_STATUS_OK ||
        value != expected;

    if (reader != LIB_NULL && lib_storage_file_reader_close(reader) !=
            LIB_STATUS_OK) failed = 1;
    return failed;
}

lib_i32 main(void)
{
    t_fdd fdd;
    t_hdd hdd;
    lib_u8 hdd_bytes[512] = {0x5au};
    lib_u8 direct_value = 0x3cu;
    lib_u8 byte = 0u;
    lib_i32 failed = 0;

    vm_media_direct_fdd_bytes[0u] = 0xa5u;
    if (vm_media_direct_write(vm_media_direct_fdd_path, vm_media_direct_fdd_bytes,
            sizeof(vm_media_direct_fdd_bytes)) ||
        vm_media_direct_write(vm_media_direct_hdd_path, hdd_bytes, sizeof(hdd_bytes))) {
        return 1;
    }
    if (vm_machine_fdd_initialize_with_geometry(&fdd,
            &vm_media_direct_fdd_geometry) != LIB_FALSE) return 1;
    vm_machine_hdd_initialize(&hdd);
    if (vm_machine_fdd_insert_for(&fdd, vm_media_direct_fdd_path,
            LIB_STORAGE_MEDIUM_READONLY) != LIB_FALSE ||
        !fdd.connect.flagReadOnly || vm_machine_fdd_read_byte(&fdd, 0u, 0u, 1u, 0u,
            &byte) != LIB_FALSE || byte != 0xa5u ||
        vm_machine_fdd_write_byte(&fdd, 0u, 0u, 1u, 0u, 0u) != LIB_TRUE ||
        vm_machine_hdd_insert(&hdd, vm_media_direct_hdd_path,
            LIB_STORAGE_MEDIUM_READONLY) != LIB_FALSE ||
        !hdd.connect.flagReadOnly || vm_machine_hdd_media_provider()->write_bytes(&hdd,
            0u, &byte, 1u) != CORE_MACHINE_MEDIA_RESULT_READ_ONLY ||
        vm_machine_hdd_media_provider()->read_bytes(&hdd, 0u, &byte, 1u) !=
            CORE_MACHINE_MEDIA_RESULT_OK || byte != 0x5au) {
        failed = 1;
    }
    if (vm_machine_fdd_remove_for(&fdd) != LIB_FALSE ||
        vm_machine_hdd_remove(&hdd) != LIB_FALSE) failed = 1;
    if (!failed && (vm_machine_fdd_insert_for(&fdd, vm_media_direct_fdd_path,
            LIB_STORAGE_MEDIUM_DIRECT) != LIB_FALSE ||
        vm_machine_fdd_write_byte(&fdd, 0u, 0u, 1u, 0u, direct_value) != LIB_FALSE ||
        vm_machine_hdd_insert(&hdd, vm_media_direct_hdd_path,
            LIB_STORAGE_MEDIUM_DIRECT) != LIB_FALSE ||
        vm_machine_hdd_media_provider()->write_bytes(&hdd, 0u, &direct_value,
            1u) != CORE_MACHINE_MEDIA_RESULT_OK ||
        vm_machine_fdd_remove_for(&fdd) != LIB_FALSE ||
        vm_machine_hdd_remove(&hdd) != LIB_FALSE ||
        vm_media_direct_read_first(vm_media_direct_fdd_path, direct_value) ||
        vm_media_direct_read_first(vm_media_direct_hdd_path, direct_value))) failed = 1;
    vm_machine_fdd_finalize(&fdd);
    vm_machine_hdd_finalize(&hdd);
    (void)remove(vm_media_direct_fdd_path);
    (void)remove(vm_media_direct_hdd_path);
    if (failed) return 1;
    printf("M5:T524:S10:MEDIA-DIRECT-READONLY:OK\n");
    return 0;
}
