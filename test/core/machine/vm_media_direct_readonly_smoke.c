#include "type.h"

#include <stdio.h>

#include "core/devices/media_interface.h"
#include "core/machine/media/fdd_private.h"
#include "core/machine/media/hdd_private.h"
#include "lib/storage/file_interface.h"

static const C_CHAR vm_media_direct_fdd_path[] = "vm_media_direct_fdd.img";
static const C_CHAR vm_media_direct_hdd_path[] = "vm_media_direct_hdd.img";
static type_unsigned_8 vm_media_direct_fdd_bytes[80u * 2u * 18u * 512u];
static const core_machine_media_geometry vm_media_direct_fdd_geometry = {
    2880u, 512u, 80u, 2u, 18u
};

static C_INT vm_media_direct_write(const C_CHAR *path, const C_VOID *bytes,
    STD_SIZE_T byte_count)
{
    lib_storage_file_writer *writer = LIB_NULL;
    C_INT failed = lib_storage_file_writer_open(path,
        LIB_STORAGE_FILE_WRITER_TRUNCATE, &writer) != LIB_STATUS_OK ||
        (byte_count != 0u && lib_storage_file_writer_write(writer, bytes,
            byte_count) != LIB_STATUS_OK);

    if (writer != LIB_NULL && lib_storage_file_writer_close(writer) !=
            LIB_STATUS_OK) failed = 1;
    return failed;
}

static C_INT vm_media_direct_read_first(const C_CHAR *path,
    type_unsigned_8 expected)
{
    lib_storage_file_reader *reader = LIB_NULL;
    type_unsigned_8 value = 0u;
    C_INT failed = lib_storage_file_reader_open(path, &reader) != LIB_STATUS_OK ||
        lib_storage_file_reader_read(reader, &value, 1u) != LIB_STATUS_OK ||
        value != expected;

    if (reader != LIB_NULL && lib_storage_file_reader_close(reader) !=
            LIB_STATUS_OK) failed = 1;
    return failed;
}

C_INT main(C_VOID)
{
    t_fdd fdd;
    t_hdd hdd;
    type_unsigned_8 hdd_bytes[512] = {0x5au};
    type_unsigned_8 direct_value = 0x3cu;
    type_unsigned_8 byte = 0u;
    C_INT failed = 0;

    vm_media_direct_fdd_bytes[0u] = 0xa5u;
    if (vm_media_direct_write(vm_media_direct_fdd_path, vm_media_direct_fdd_bytes,
            sizeof(vm_media_direct_fdd_bytes)) ||
        vm_media_direct_write(vm_media_direct_hdd_path, hdd_bytes, sizeof(hdd_bytes))) {
        return 1;
    }
    if (vm_machine_fdd_initialize_with_geometry(&fdd,
            &vm_media_direct_fdd_geometry) != TYPE_FALSE) return 1;
    vm_machine_hdd_initialize(&hdd);
    if (vm_machine_fdd_insert_for(&fdd, vm_media_direct_fdd_path,
            LIB_STORAGE_MEDIUM_READONLY) != TYPE_FALSE ||
        !fdd.connect.flagReadOnly || vm_machine_fdd_read_byte(&fdd, 0u, 0u, 1u, 0u,
            &byte) != TYPE_FALSE || byte != 0xa5u ||
        vm_machine_fdd_write_byte(&fdd, 0u, 0u, 1u, 0u, 0u) != TYPE_TRUE ||
        vm_machine_hdd_insert(&hdd, vm_media_direct_hdd_path,
            LIB_STORAGE_MEDIUM_READONLY) != TYPE_FALSE ||
        !hdd.connect.flagReadOnly || vm_machine_hdd_media_provider()->write_bytes(&hdd,
            0u, &byte, 1u) != CORE_MACHINE_MEDIA_RESULT_READ_ONLY ||
        vm_machine_hdd_media_provider()->read_bytes(&hdd, 0u, &byte, 1u) !=
            CORE_MACHINE_MEDIA_RESULT_OK || byte != 0x5au) {
        failed = 1;
    }
    if (vm_machine_fdd_remove_for(&fdd) != TYPE_FALSE ||
        vm_machine_hdd_remove(&hdd) != TYPE_FALSE) failed = 1;
    if (!failed && (vm_machine_fdd_insert_for(&fdd, vm_media_direct_fdd_path,
            LIB_STORAGE_MEDIUM_DIRECT) != TYPE_FALSE ||
        vm_machine_fdd_write_byte(&fdd, 0u, 0u, 1u, 0u, direct_value) != TYPE_FALSE ||
        vm_machine_hdd_insert(&hdd, vm_media_direct_hdd_path,
            LIB_STORAGE_MEDIUM_DIRECT) != TYPE_FALSE ||
        vm_machine_hdd_media_provider()->write_bytes(&hdd, 0u, &direct_value,
            1u) != CORE_MACHINE_MEDIA_RESULT_OK ||
        vm_machine_fdd_remove_for(&fdd) != TYPE_FALSE ||
        vm_machine_hdd_remove(&hdd) != TYPE_FALSE ||
        vm_media_direct_read_first(vm_media_direct_fdd_path, direct_value) ||
        vm_media_direct_read_first(vm_media_direct_hdd_path, direct_value))) failed = 1;
    vm_machine_fdd_finalize(&fdd);
    vm_machine_hdd_finalize(&hdd);
    (C_VOID)remove(vm_media_direct_fdd_path);
    (C_VOID)remove(vm_media_direct_hdd_path);
    if (failed) return 1;
    STD_PRINTF("M5:T524:S10:MEDIA-DIRECT-READONLY:OK\n");
    return 0;
}
