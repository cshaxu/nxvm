#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/media_interface.h"
#include "app-nxvm/machine/media/fdd_private.h"
#include "app-nxvm/machine/media/hdd_private.h"
#include "lib/storage/file_interface.h"

static const char vm_media_fdd_path[] = "vm-media-overlay-fdd.img";
static const char vm_media_hdd_path[] = "vm-media-overlay-hdd.img";
static const core_machine_media_geometry vm_media_fdd_geometry = {
    2880u, 512u, 80u, 2u, 18u
};

static lib_i32 vm_media_write_file(const char *path, const void *bytes,
    lib_size count)
{
    lib_storage_file_writer *writer = LIB_NULL;
    lib_i32 failed = lib_storage_file_writer_open(path,
        LIB_STORAGE_FILE_WRITER_TRUNCATE, &writer) != LIB_STATUS_OK ||
        (count != 0u && lib_storage_file_writer_write(writer, bytes, count) !=
            LIB_STATUS_OK);

    if (writer != LIB_NULL && lib_storage_file_writer_close(writer) !=
            LIB_STATUS_OK) failed = LIB_TRUE;
    return failed;
}

static lib_i32 vm_media_read_first(const char *path, lib_u8 expected)
{
    lib_storage_file_reader *reader = LIB_NULL;
    lib_u8 value = 0u;
    lib_i32 failed = lib_storage_file_reader_open(path, &reader) != LIB_STATUS_OK ||
        lib_storage_file_reader_read(reader, &value, 1u) != LIB_STATUS_OK ||
        value != expected;

    if (reader != LIB_NULL && lib_storage_file_reader_close(reader) !=
            LIB_STATUS_OK) failed = LIB_TRUE;
    return failed;
}

lib_i32 main(void)
{
    static lib_u8 fdd_bytes[80u * 2u * 18u * 512u];
    static lib_u8 hdd_bytes[1024u];
    t_fdd fdd;
    t_hdd hdd;
    core_machine_media_registry *registry = LIB_NULL;
    core_machine_media_info hdd_info = {0};
    core_machine_media_result result;
    lib_u8 value = 0x5au;
    lib_i32 failed = LIB_FALSE;

    fdd_bytes[0] = 0x11u;
    hdd_bytes[0] = 0x22u;
    if (vm_media_write_file(vm_media_fdd_path, fdd_bytes, sizeof(fdd_bytes)) ||
        vm_media_write_file(vm_media_hdd_path, hdd_bytes, sizeof(hdd_bytes))) failed = LIB_TRUE;
    if (vm_machine_fdd_initialize_with_geometry(&fdd,
            &vm_media_fdd_geometry) != LIB_FALSE) return 1;
    vm_machine_hdd_initialize(&hdd);
    if (vm_machine_fdd_has_media(&fdd)) failed = LIB_TRUE;
    vm_machine_fdd_create_for(&fdd);
    if (!vm_machine_fdd_has_media(&fdd)) failed = LIB_TRUE;
    if (!failed && (vm_machine_fdd_remove_for(&fdd) != LIB_FALSE ||
        vm_machine_hdd_create(&hdd, 0u) != LIB_TRUE ||
        vm_machine_hdd_create(&hdd, 1u) != LIB_FALSE ||
        !vm_machine_hdd_has_media(&hdd) ||
        vm_machine_hdd_remove(&hdd) != LIB_FALSE ||
        vm_machine_fdd_insert_for(&fdd, vm_media_fdd_path,
        LIB_STORAGE_MEDIUM_OVERLAY) != LIB_FALSE ||
        vm_machine_hdd_insert(&hdd, vm_media_hdd_path,
            LIB_STORAGE_MEDIUM_OVERLAY) != LIB_FALSE ||
        core_machine_media_registry_create(&registry) != LIB_STATUS_OK ||
        core_machine_media_registry_bind(registry, 1u, &fdd,
            vm_machine_fdd_media_provider()) != LIB_STATUS_OK ||
        core_machine_media_registry_bind(registry, 2u, &hdd,
            vm_machine_hdd_media_provider()) != LIB_STATUS_OK ||
        core_machine_media_registry_freeze(registry) != LIB_STATUS_OK ||
        core_machine_media_query(registry, 2u, &hdd_info, &result) != LIB_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK ||
        (hdd_info.capabilities & (CORE_MACHINE_MEDIA_CAPABILITY_REMOVABLE |
            CORE_MACHINE_MEDIA_CAPABILITY_CHANGE_DETECTABLE)) != 0u ||
        (hdd_info.capabilities & (CORE_MACHINE_MEDIA_CAPABILITY_GEOMETRY_KNOWN |
            CORE_MACHINE_MEDIA_CAPABILITY_FORMATTABLE)) !=
            (CORE_MACHINE_MEDIA_CAPABILITY_GEOMETRY_KNOWN |
                CORE_MACHINE_MEDIA_CAPABILITY_FORMATTABLE) ||
        core_machine_media_write_bytes(registry, 1u, 0u, &value, 1u,
            &result) != LIB_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        core_machine_media_write_bytes(registry, 2u, 0u, &value, 1u,
            &result) != LIB_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        vm_machine_fdd_remove_for(&fdd) != LIB_FALSE ||
        vm_machine_hdd_remove(&hdd) != LIB_FALSE ||
        vm_media_read_first(vm_media_fdd_path, 0x11u) ||
        vm_media_read_first(vm_media_hdd_path, 0x22u))) failed = LIB_TRUE;
    core_machine_media_registry_destroy(registry);
    vm_machine_fdd_finalize(&fdd);
    vm_machine_hdd_finalize(&hdd);
    (void)remove(vm_media_fdd_path);
    (void)remove(vm_media_hdd_path);
    return failed;
}
