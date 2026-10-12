#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "core/board-base/media_interface.h"
#include "core/machine/media/fdd.h"
#include "core/machine/media/hdd_interface.h"
#include "core/machine/media/hdd.h"
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

static lib_i32 vm_media_fdd_marks_survive_reset(t_fdd *fdd,
    const core_machine_media_registry *registry)
{
    core_machine_media_info before = {0}, after = {0};
    core_machine_media_address_mark mark;
    core_machine_media_result result;
    lib_u8 byte = 0u;

    if (core_machine_media_query(registry, 1u, &before, &result) != LIB_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK ||
        core_machine_media_set_address_mark(registry, 1u, 0u,
            CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA, &result) != LIB_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK) return LIB_TRUE;
    vm_machine_fdd_reset(fdd);
    if (core_machine_media_query(registry, 1u, &after, &result) != LIB_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK || after.generation != before.generation ||
        core_machine_media_get_address_mark(registry, 1u, 0u, &mark, &result) != LIB_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK || mark != CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA ||
        core_machine_media_read_bytes(registry, 1u, 0u, &byte, 1u, &result) != LIB_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK || byte != 0x11u) return LIB_TRUE;
    /* A rejected replacement cannot erase the installed record metadata. */
    if (vm_machine_fdd_insert_for(fdd, vm_media_hdd_path,
            LIB_STORAGE_MEDIUM_OVERLAY) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_media_get_address_mark(registry, 1u, 0u, &mark, &result) != LIB_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK || mark != CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA)
        return LIB_TRUE;
    if (vm_machine_fdd_insert_for(fdd, vm_media_fdd_path,
            LIB_STORAGE_MEDIUM_OVERLAY) != LIB_STATUS_OK ||
        core_machine_media_query(registry, 1u, &after, &result) != LIB_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK || after.generation != before.generation + 1u ||
        core_machine_media_get_address_mark(registry, 1u, 0u, &mark, &result) != LIB_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK || mark != CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA)
        return LIB_TRUE;
    return LIB_FALSE;
}

static lib_bool vm_media_hdd_record_lifetime(t_hdd *hdd,
    const core_machine_media_registry *registry)
{
    core_machine_media_record record = {0}, observed = {0};
    core_machine_media_result result;
    const lib_u8 ecc[] = {0x13u, 0x57u, 0x9bu, 0xdfu};
    lib_u8 byte = 0x31u;
    static const lib_u8 replacement[1024] = {0x22u};

    if (core_machine_media_read_record(registry, 2u, 0u, &observed, &result) !=
            LIB_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        observed.ecc_present || observed.data[0] != 0x22u) return LIB_TRUE;
    record.data[0] = 0x42u;
    record.ecc_present = LIB_TRUE;
    lib_memory_copy(record.ecc, ecc, sizeof(ecc));
    if (core_machine_media_write_record(registry, 2u, 0u, &record, &result) !=
            LIB_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK)
        return LIB_TRUE;
    vm_machine_hdd_reset(hdd);
    if (vm_machine_hdd_insert(hdd, "missing-hdd-record-image.img",
            LIB_STORAGE_MEDIUM_OVERLAY) == LIB_STATUS_OK ||
        core_machine_media_read_record(registry, 2u, 0u, &observed, &result) !=
            LIB_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        !observed.ecc_present || observed.data[0] != 0x42u ||
        lib_memory_compare(observed.ecc, ecc, sizeof(ecc)) != 0)
        return LIB_TRUE;
    /* Partial byte writes and format cannot retain ECC for changed payload. */
    if (core_machine_media_write_bytes(registry, 2u, 7u, &byte, 1u, &result) !=
            LIB_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        core_machine_media_read_record(registry, 2u, 0u, &observed, &result) !=
            LIB_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        observed.ecc_present || observed.data[7] != byte ||
        core_machine_media_write_record(registry, 2u, 0u, &record, &result) !=
            LIB_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        core_machine_media_format_sectors(registry, 2u, 0u, 1u, 0x11u, &result) !=
            LIB_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        core_machine_media_read_record(registry, 2u, 0u, &observed, &result) !=
            LIB_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        observed.ecc_present || observed.data[0] != 0x11u)
        return LIB_TRUE;
    if (core_machine_media_write_record(registry, 2u, 0u, &record, &result) !=
            LIB_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        vm_machine_hdd_replace_bytes(hdd, replacement, sizeof(replacement)) !=
            LIB_STATUS_OK ||
        core_machine_media_read_record(registry, 2u, 0u, &observed, &result) !=
            LIB_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        observed.ecc_present || observed.data[0] != 0x22u)
        return LIB_TRUE;
    /* A failed read never publishes a partial or stale record. */
    observed.data[0] = 0xabu;
    observed.ecc_present = LIB_TRUE;
    if (core_machine_media_read_record(registry, 2u, 2u, &observed, &result) !=
            LIB_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE ||
        observed.data[0] != 0xabu || !observed.ecc_present ||
        vm_machine_hdd_remove(hdd) != LIB_STATUS_OK ||
        core_machine_media_read_record(registry, 2u, 0u, &observed, &result) !=
            LIB_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_ABSENT ||
        observed.data[0] != 0xabu || !observed.ecc_present)
        return LIB_TRUE;
    if (vm_machine_hdd_insert(hdd, vm_media_hdd_path,
            LIB_STORAGE_MEDIUM_READONLY) != LIB_STATUS_OK ||
        core_machine_media_write_record(registry, 2u, 0u, &record, &result) !=
            LIB_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_READ_ONLY ||
        core_machine_media_read_record(registry, 2u, 0u, &observed, &result) !=
            LIB_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        observed.ecc_present || observed.data[0] != 0x22u ||
        vm_machine_hdd_remove(hdd) != LIB_STATUS_OK)
        return LIB_TRUE;
    /* Direct stores payload in the file, but ECC still belongs to the lease. */
    record.data[0] = 0x22u;
    if (vm_machine_hdd_insert(hdd, vm_media_hdd_path,
            LIB_STORAGE_MEDIUM_DIRECT) != LIB_STATUS_OK ||
        core_machine_media_write_record(registry, 2u, 0u, &record, &result) !=
            LIB_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        core_machine_media_read_record(registry, 2u, 0u, &observed, &result) !=
            LIB_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        !observed.ecc_present || lib_memory_compare(observed.ecc, ecc, sizeof(ecc)) != 0 ||
        vm_machine_hdd_remove(hdd) != LIB_STATUS_OK ||
        vm_machine_hdd_insert(hdd, vm_media_hdd_path,
            LIB_STORAGE_MEDIUM_OVERLAY) != LIB_STATUS_OK ||
        core_machine_media_read_record(registry, 2u, 0u, &observed, &result) !=
            LIB_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK || observed.ecc_present)
        return LIB_TRUE;
    return LIB_FALSE;
}

static lib_bool vm_media_hdd_track_lifetime(t_hdd *hdd,
    const core_machine_media_registry *registry)
{
    core_machine_media_track track = {.sector_count = 2u, .ids = {1u, 0u},
        .bad = {LIB_FALSE, LIB_TRUE}};
    core_machine_media_record observed = {.data = {0xabu}};
    const core_machine_media_record tagged = {.data = {0x77u},
        .ecc = {0x12u, 0x34u, 0x56u, 0x78u}, .ecc_present = LIB_TRUE};
    core_machine_media_result result;
    lib_u8 byte = 0x55u;

    if (core_machine_media_write_record(registry, 2u, 1u, &tagged, &result) != LIB_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK ||
        core_machine_media_read_record(registry, 2u, 1u, &observed, &result) != LIB_STATUS_OK ||
        !observed.ecc_present || observed.data[0] != 0x77u) return LIB_TRUE;
    observed.data[0] = 0xabu;
    if (core_machine_media_format_track(registry, 2u, &track, &result) != LIB_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK || hdd->connect.tracks == LIB_NULL ||
        hdd->connect.tracks->format.ids[0] != 1u ||
        core_machine_media_read_record(registry, 2u, 0u, &observed, &result) != LIB_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_BAD_TRACK || observed.data[0] != 0xabu ||
        core_machine_media_read_record(registry, 2u, 1u, &observed, &result) != LIB_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK || observed.ecc_present ||
        observed.data[0] != 0x77u) return LIB_TRUE;
    vm_machine_hdd_reset(hdd);
    if (core_machine_media_write_bytes(registry, 2u, 0u, &byte, 1u, &result) != LIB_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK ||
        core_machine_media_read_record(registry, 2u, 0u, &observed, &result) != LIB_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_BAD_TRACK ||
        core_machine_media_write_record(registry, 2u, 0u, &observed, &result) != LIB_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_BAD_TRACK) return LIB_TRUE;
    track.ids[1] = 1u;
    if (core_machine_media_format_track(registry, 2u, &track, &result) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_media_read_record(registry, 2u, 0u, &observed, &result) != LIB_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_BAD_TRACK) return LIB_TRUE;
    track.ids[1] = 0u;
    track.first_sector = 2u;
    if (core_machine_media_format_track(registry, 2u, &track, &result) != LIB_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE) return LIB_TRUE;
    track.first_sector = 0u;
    if (vm_machine_hdd_insert(hdd, "missing-track-replacement.img",
            LIB_STORAGE_MEDIUM_OVERLAY) == LIB_STATUS_OK ||
        core_machine_media_read_record(registry, 2u, 0u, &observed, &result) != LIB_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_BAD_TRACK) return LIB_TRUE;
    track.bad[1] = LIB_FALSE;
    track.fill_payload = LIB_TRUE;
    track.fill = 0x66u;
    if (core_machine_media_format_track(registry, 2u, &track, &result) != LIB_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK ||
        core_machine_media_read_record(registry, 2u, 0u, &observed, &result) != LIB_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK || observed.data[0] != 0x66u || observed.ecc_present ||
        vm_machine_hdd_remove(hdd) != LIB_STATUS_OK || hdd->connect.tracks != LIB_NULL ||
        vm_machine_hdd_insert(hdd, vm_media_hdd_path, LIB_STORAGE_MEDIUM_READONLY) != LIB_STATUS_OK ||
        core_machine_media_format_track(registry, 2u, &track, &result) != LIB_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_READ_ONLY || hdd->connect.tracks != LIB_NULL ||
        vm_machine_hdd_insert(hdd, vm_media_hdd_path, LIB_STORAGE_MEDIUM_OVERLAY) != LIB_STATUS_OK ||
        hdd->connect.tracks != LIB_NULL ||
        core_machine_media_read_record(registry, 2u, 0u, &observed, &result) != LIB_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK || observed.data[0] != 0x22u)
        return LIB_TRUE;
    return LIB_FALSE;
}

lib_i32 main(void)
{
    static lib_u8 fdd_bytes[80u * 2u * 18u * 512u];
    static lib_u8 hdd_bytes[1024u];
    t_fdd *fdd = LIB_NULL;
    t_hdd *hdd = LIB_NULL;
    core_machine_media_registry *registry = LIB_NULL;
    core_machine_media_info hdd_info = {0};
    core_machine_media_result result;
    lib_u8 value = 0x5au;
    lib_i32 failed = LIB_FALSE;

    fdd_bytes[0] = 0x11u;
    hdd_bytes[0] = 0x22u;
    if (vm_media_write_file(vm_media_fdd_path, fdd_bytes, sizeof(fdd_bytes)) ||
        vm_media_write_file(vm_media_hdd_path, hdd_bytes, sizeof(hdd_bytes))) failed = LIB_TRUE;
    /* Rejected construction never publishes an object; destroy accepts empty slots. */
    vm_machine_fdd_destroy(&fdd);
    vm_machine_hdd_destroy(&hdd);
    if (vm_machine_fdd_allocate(LIB_NULL, &fdd) != LIB_STATUS_INVALID_ARGUMENT ||
        fdd != LIB_NULL || vm_machine_hdd_allocate(LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT)
        return 1;
    if (vm_machine_fdd_allocate(&vm_media_fdd_geometry, &fdd) != LIB_STATUS_OK ||
        vm_machine_hdd_allocate(&hdd) != LIB_STATUS_OK) {
        vm_machine_fdd_destroy(&fdd);
        vm_machine_hdd_destroy(&hdd);
        return 1;
    }
    {
        t_fdd *identity = fdd;
        t_hdd *disk_identity = hdd;
        if (vm_machine_fdd_allocate(&vm_media_fdd_geometry, &fdd) !=
                LIB_STATUS_INVALID_ARGUMENT || fdd != identity ||
            vm_machine_hdd_allocate(&hdd) != LIB_STATUS_INVALID_ARGUMENT ||
            hdd != disk_identity) failed = LIB_TRUE;
    }
    if (vm_machine_fdd_has_media(fdd)) failed = LIB_TRUE;
    vm_machine_fdd_create_for(fdd);
    if (!vm_machine_fdd_has_media(fdd)) failed = LIB_TRUE;
    if (!failed && (vm_machine_fdd_remove_for(fdd) != LIB_FALSE ||
        vm_machine_hdd_create(hdd, 0u) != LIB_TRUE ||
        vm_machine_hdd_create(hdd, 1u) != LIB_FALSE ||
        !vm_machine_hdd_has_media(hdd) ||
        vm_machine_hdd_remove(hdd) != LIB_FALSE ||
        vm_machine_fdd_insert_for(fdd, vm_media_fdd_path,
        LIB_STORAGE_MEDIUM_OVERLAY) != LIB_FALSE ||
        vm_machine_hdd_insert(hdd, vm_media_hdd_path,
            LIB_STORAGE_MEDIUM_OVERLAY) != LIB_FALSE ||
        core_machine_media_registry_create(&registry) != LIB_STATUS_OK ||
        core_machine_media_registry_bind(registry, 1u, fdd,
            vm_machine_fdd_media_provider()) != LIB_STATUS_OK ||
        core_machine_media_registry_bind(registry, 2u, hdd,
            vm_machine_hdd_media_provider()) != LIB_STATUS_OK ||
        core_machine_media_registry_freeze(registry) != LIB_STATUS_OK ||
        vm_media_fdd_marks_survive_reset(fdd, registry) ||
        vm_media_hdd_record_lifetime(hdd, registry) ||
        vm_media_hdd_track_lifetime(hdd, registry) ||
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
        vm_machine_fdd_remove_for(fdd) != LIB_FALSE ||
        vm_machine_hdd_remove(hdd) != LIB_FALSE ||
        vm_media_read_first(vm_media_fdd_path, 0x11u) ||
        vm_media_read_first(vm_media_hdd_path, 0x22u))) failed = LIB_TRUE;
    core_machine_media_registry_destroy(registry);
    vm_machine_fdd_destroy(&fdd);
    vm_machine_hdd_destroy(&hdd);
    if (fdd != LIB_NULL || hdd != LIB_NULL) failed = LIB_TRUE;
    vm_machine_fdd_destroy(&fdd);
    vm_machine_hdd_destroy(&hdd);
    (void)lib_c_remove(vm_media_fdd_path);
    (void)lib_c_remove(vm_media_hdd_path);
    return failed;
}
