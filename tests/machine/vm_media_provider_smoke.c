#include "type.h"

#include "core/machine/media_interface.h"
#include "vm/machine/fdd_private.h"
#include "vm/machine/hdd_private.h"
#include "vm/machine/media_save.h"

static type_unsigned_8 vm_media_provider_fdd_image[80u * 2u * 18u * 512u];
static const C_CHAR vm_media_provider_save_target[] = "vm_media_provider_t283.img";
static const C_CHAR vm_media_provider_save_collision[] =
    "vm_media_provider_t283.img.ntvdm64.tmp.000";
static const C_CHAR vm_media_provider_sidecar_image[] = "vm_media_provider_t376.img";
static const C_CHAR vm_media_provider_sidecar_metadata[] = "vm_media_provider_t376.img.json";
static const C_CHAR vm_media_provider_malformed_image[] =
    "vm_media_provider_t376_malformed.img";
static const C_CHAR vm_media_provider_malformed_metadata[] =
    "vm_media_provider_t376_malformed.img.json";
static const C_CHAR vm_media_provider_sidecar_backup_collision[] =
    "vm_media_provider_t376.img.json.ntvdm64.bak";
static const C_CHAR vm_media_provider_sidecar_image_temporary[] =
    "vm_media_provider_t376.img.ntvdm64.tmp.000";
static const C_CHAR vm_media_provider_sidecar_metadata_temporary[] =
    "vm_media_provider_t376.img.json.ntvdm64.tmp.000";
static const C_CHAR vm_media_provider_invalid_path_temporary[] =
    "..ntvdm64.tmp.000";

static C_INT vm_media_provider_write_byte_file(const C_CHAR *file_name, type_unsigned_8 byte)
{
    STD_FILE *file = STD_FOPEN(file_name, "wb");
    C_INT failed = file == STD_NULL || STD_FWRITE(&byte, sizeof(byte), 1u, file) != 1u;

    if (file != STD_NULL && STD_FCLOSE(file) != 0) failed = 1;
    return failed;
}

static C_INT vm_media_provider_read_byte_file(const C_CHAR *file_name, type_unsigned_8 expected)
{
    type_unsigned_8 byte = 0u;
    STD_FILE *file = STD_FOPEN(file_name, "rb");
    C_INT failed = file == STD_NULL || STD_FREAD(&byte, sizeof(byte), 1u, file) != 1u ||
        byte != expected;

    if (file != STD_NULL && STD_FCLOSE(file) != 0) failed = 1;
    return failed;
}

static C_INT vm_media_provider_file_exists(const C_CHAR *file_name)
{
    STD_FILE *file = STD_FOPEN(file_name, "rb");

    if (file == STD_NULL) return 0;
    (C_VOID)STD_FCLOSE(file);
    return 1;
}

static C_INT vm_media_provider_expect_hdd_capacity(t_hdd *hdd,
    core_machine_media_registry *registry, STD_SIZE_T raw_byte_count,
    type_unsigned_8 expected_first_byte)
{
    core_machine_media_info info;
    core_machine_media_result result;
    type_unsigned_8 tail_byte = 0xffu;
    STD_SIZE_T expected_virtual_byte_count = raw_byte_count == 0u ? 0u :
        ((raw_byte_count + 511u) / 512u) * 512u;

    if (core_machine_media_query(registry, 2u, &info, &result) != TYPE_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK || !info.present ||
        hdd->connect.raw_byte_count != raw_byte_count ||
        hdd->connect.virtual_byte_count != expected_virtual_byte_count ||
        info.geometry.logical_sector_count != expected_virtual_byte_count / 512u) {
        return 1;
    }
    if (raw_byte_count == 0u) {
        return 0;
    }
    if (core_machine_media_read_bytes(registry, 2u, 0u, &tail_byte, 1u, &result) !=
            TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        tail_byte != expected_first_byte) {
        return 1;
    }
    if (expected_virtual_byte_count > raw_byte_count &&
        (core_machine_media_read_bytes(registry, 2u, raw_byte_count, &tail_byte, 1u,
            &result) != TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        tail_byte != 0u)) {
        return 1;
    }
    return 0;
}

static C_INT vm_media_provider_expect_null_backing(
    core_machine_media_registry *registry, core_machine_media_id id,
    C_VOID *context, const core_machine_media_provider *provider,
    type_unsigned_8 *byte)
{
    core_machine_media_result result;

    if (provider->read_bytes(context, 0u, byte, 1u) !=
            CORE_MACHINE_MEDIA_RESULT_PERMANENT ||
        provider->write_bytes(context, 0u, byte, 1u) !=
            CORE_MACHINE_MEDIA_RESULT_PERMANENT ||
        provider->format_sectors(context, 0u, 1u, 0xa5u) !=
            CORE_MACHINE_MEDIA_RESULT_PERMANENT ||
        core_machine_media_read_bytes(registry, id, 0u, byte, 1u, &result) !=
            TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_PERMANENT ||
        core_machine_media_write_bytes(registry, id, 0u, byte, 1u, &result) !=
            TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_PERMANENT ||
        core_machine_media_format_sectors(registry, id, 0u, 1u, 0xa5u,
            &result) != TYPE_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_PERMANENT) {
        return 1;
    }
    return 0;
}

C_INT main(C_VOID)
{
    core_machine_media_registry *registry = STD_NULL;
    core_machine_media_info info;
    core_machine_media_result result;
    core_machine_media_address_mark mark;
    t_fdd fdd;
    t_fdd failed_fdd;
    t_fdd stale_fdd;
    t_fdd malformed_fdd;
    t_fdd null_fdd;
    t_hdd hdd;
    t_hdd null_hdd;
    type_unsigned_8 bytes[512] = {0};
    type_unsigned_8 hdd_bytes[513] = {0x5au};
    type_unsigned_8 tail_write = 0xa7u;
    type_unsigned_8 save_byte = 0x5eu;
    type_unsigned_8 collision_byte = 0xc3u;
    type_unsigned_64 fdd_generation;
    type_unsigned_64 hdd_generation;
    type_virtual_address fdd_image;
    type_virtual_address hdd_image;
    STD_FILE *stale_image;
    STD_FILE *malformed_image;
    C_INT stale_write_failed;
    C_INT failed = 0;

    vm_machine_fdd_initialize(&fdd);
    vm_machine_fdd_initialize(&failed_fdd);
    vm_machine_fdd_initialize(&stale_fdd);
    vm_machine_fdd_initialize(&malformed_fdd);
    vm_machine_fdd_finalize(&failed_fdd);
    vm_machine_hdd_initialize(&hdd);
    STD_MEMSET(&null_fdd, TYPE_ZERO_8, sizeof(null_fdd));
    null_fdd.data.ncyl = 80u;
    null_fdd.data.nhead = 2u;
    null_fdd.data.nsector = 18u;
    null_fdd.data.nbyte = 512u;
    null_fdd.connect.flagDiskExist = TYPE_TRUE;
    null_fdd.connect.media_generation = 3u;
    STD_MEMSET(&null_hdd, TYPE_ZERO_8, sizeof(null_hdd));
    null_hdd.data.nhead = 16u;
    null_hdd.data.nsector = 63u;
    null_hdd.data.nbyte = 512u;
    null_hdd.connect.flagDiskExist = TYPE_TRUE;
    null_hdd.connect.virtual_byte_count = 512u;
    null_hdd.connect.media_generation = 7u;
    if (vm_media_provider_write_byte_file(vm_media_provider_save_target, 0x39u) ||
        vm_media_provider_write_byte_file(vm_media_provider_save_collision, collision_byte) ||
        vm_machine_media_save_atomically(vm_media_provider_save_target, &save_byte,
            sizeof(save_byte)) != TYPE_FALSE ||
        vm_media_provider_read_byte_file(vm_media_provider_save_target, save_byte) ||
        vm_media_provider_read_byte_file(vm_media_provider_save_collision, collision_byte) ||
        vm_machine_media_save_atomically(vm_media_provider_save_target, STD_NULL, 1u) !=
            TYPE_TRUE ||
        vm_media_provider_read_byte_file(vm_media_provider_save_target, save_byte)) {
        failed = 1;
    }
    vm_machine_fdd_create_for(&failed_fdd);
    if (!failed && (failed_fdd.connect.flagDiskExist ||
        failed_fdd.connect.media_generation != 0u ||
        failed_fdd.connect.pImgBase != (type_virtual_address)STD_NULL)) {
        failed = 1;
    }
    vm_machine_fdd_create_for(&fdd);
    vm_machine_hdd_create(&hdd, 2u);
    if (vm_machine_hdd_set_geometry(&hdd, 1u, 32u, 63u) != TYPE_FALSE ||
        vm_machine_hdd_set_geometry(&hdd, 1u, 16u, 63u) != TYPE_TRUE ||
        hdd.data.ncyl != 1u || hdd.data.nhead != 32u || hdd.data.nsector != 63u) {
        failed = 1;
    }
    vm_machine_hdd_reset(&hdd);
    if (!failed && (hdd.data.ncyl != 1u || hdd.data.nhead != 32u ||
        hdd.data.nsector != 63u || hdd.data.nbyte != 512u)) {
        failed = 1;
    }
    if (core_machine_media_registry_create(&registry) != TYPE_STATUS_OK ||
        core_machine_media_registry_bind(registry, 1u, &fdd,
            vm_machine_fdd_media_provider()) != TYPE_STATUS_OK ||
        core_machine_media_registry_bind(registry, 2u, &hdd,
            vm_machine_hdd_media_provider()) != TYPE_STATUS_OK ||
        core_machine_media_registry_bind(registry, 3u, &null_fdd,
            vm_machine_fdd_media_provider()) != TYPE_STATUS_OK ||
        core_machine_media_registry_bind(registry, 4u, &null_hdd,
            vm_machine_hdd_media_provider()) != TYPE_STATUS_OK ||
        core_machine_media_registry_freeze(registry) != TYPE_STATUS_OK ||
        core_machine_media_query(registry, 1u, &info, &result) != TYPE_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK || !info.present ||
        info.geometry.bytes_per_sector != 512u ||
        (info.capabilities & CORE_MACHINE_MEDIA_CAPABILITY_ADDRESS_MARKS) == 0u ||
        core_machine_media_get_address_mark(registry, 1u, 0u, &mark,
            &result) != TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        mark != CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA ||
        core_machine_media_query(registry, 2u, &info, &result) != TYPE_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK || info.geometry.cylinders != 1u ||
        (info.capabilities & CORE_MACHINE_MEDIA_CAPABILITY_ADDRESS_MARKS) != 0u ||
        core_machine_media_set_address_mark(registry, 2u, 0u,
            CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA, &result) != TYPE_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_UNSUPPORTED) {
        failed = 1;
    }
    if (!failed && (vm_media_provider_expect_null_backing(registry, 3u,
        &null_fdd, vm_machine_fdd_media_provider(), &bytes[0]) ||
        vm_media_provider_expect_null_backing(registry, 4u, &null_hdd,
            vm_machine_hdd_media_provider(), &bytes[0]))) {
        failed = 1;
    }
    fdd_generation = fdd.connect.media_generation;
    if (!failed && (core_machine_media_write_sectors(registry, 1u, 0u, 1u,
            bytes, &result) != TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        core_machine_media_format_sectors(registry, 1u, 0u, 1u, 0xa5u,
            &result) != TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        fdd.connect.media_generation != fdd_generation + 1u ||
        core_machine_media_read_sectors(registry, 1u, 0u, 1u, bytes,
            &result) != TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        bytes[0] != 0xa5u ||
        core_machine_media_read_sectors(registry, 2u,
            info.geometry.logical_sector_count, 1u, bytes,
            &result) != TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE)) {
        failed = 1;
    }
    if (!failed) {
        vm_media_provider_fdd_image[0] = 0x6cu;
        hdd_bytes[0] = 0x5au;
        if (vm_machine_fdd_replace_bytes(&fdd, vm_media_provider_fdd_image,
                sizeof(vm_media_provider_fdd_image)) != TYPE_FALSE ||
            vm_machine_fdd_read_byte(&fdd, 0u, 0u, 1u, 0u, &bytes[0]) != TYPE_FALSE ||
            bytes[0] != 0x6cu || vm_machine_fdd_replace_bytes(&fdd, STD_NULL,
                sizeof(vm_media_provider_fdd_image)) != TYPE_TRUE) {
            failed = 1;
        }
    }
    if (!failed && (core_machine_media_set_address_mark(registry, 1u, 0u,
            CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA, &result) != TYPE_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK || vm_machine_fdd_remove_for(&fdd,
            vm_media_provider_sidecar_image) != TYPE_FALSE || fdd.connect.flagDiskExist ||
        vm_machine_fdd_insert_for(&fdd, vm_media_provider_sidecar_image) != TYPE_FALSE ||
        core_machine_media_get_address_mark(registry, 1u, 0u, &mark,
            &result) != TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        mark != CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA ||
        vm_machine_fdd_read_byte(&fdd, 0u, 0u, 1u, 0u, &bytes[0]) != TYPE_FALSE ||
        bytes[0] != 0x6cu)) {
        failed = 1;
    }
    stale_image = STD_NULL;
    stale_write_failed = TYPE_TRUE;
    if (!failed && (stale_image = STD_FOPEN(vm_media_provider_sidecar_image, "r+b")) !=
        STD_NULL) {
        stale_write_failed = STD_FPUTC(0x6du, stale_image) == STD_EOF ||
            STD_FCLOSE(stale_image) != 0;
        stale_image = STD_NULL;
    }
    if (!failed && (stale_write_failed || vm_machine_fdd_insert_for(&stale_fdd,
            vm_media_provider_sidecar_image) != TYPE_TRUE || stale_fdd.connect.flagDiskExist)) {
        failed = 1;
    }
    malformed_image = STD_FOPEN(vm_media_provider_malformed_image, "wb");
    if (!failed && (malformed_image == STD_NULL || STD_FWRITE(vm_media_provider_fdd_image,
            sizeof(vm_media_provider_fdd_image), 1u, malformed_image) != 1u)) {
        failed = 1;
    }
    if (malformed_image != STD_NULL && STD_FCLOSE(malformed_image) != 0) failed = 1;
    malformed_image = STD_NULL;
    if (!failed && (vm_media_provider_write_byte_file(
            vm_media_provider_malformed_metadata, '{') ||
        vm_machine_fdd_insert_for(&malformed_fdd,
            vm_media_provider_malformed_image) != TYPE_TRUE ||
        malformed_fdd.connect.flagDiskExist ||
        !fdd.connect.flagDiskExist ||
        vm_machine_fdd_read_byte(&fdd, 0u, 0u, 1u, 0u, &bytes[0]) != TYPE_FALSE ||
        bytes[0] != 0x6cu)) {
        failed = 1;
    }
    if (!failed && (vm_media_provider_write_byte_file(
            vm_media_provider_sidecar_backup_collision, 0x31u) ||
        vm_machine_fdd_remove_for(&fdd, vm_media_provider_sidecar_image) != TYPE_TRUE ||
        !fdd.connect.flagDiskExist ||
        vm_media_provider_read_byte_file(vm_media_provider_sidecar_image, 0x6du) ||
        !vm_media_provider_file_exists(vm_media_provider_sidecar_metadata) ||
        vm_media_provider_file_exists(vm_media_provider_sidecar_image_temporary) ||
        vm_media_provider_file_exists(vm_media_provider_sidecar_metadata_temporary))) {
        failed = 1;
    }
    (C_VOID)STD_REMOVE(vm_media_provider_sidecar_backup_collision);
    fdd_generation = fdd.connect.media_generation;
    fdd_image = fdd.connect.pImgBase;
    if (!failed && (vm_machine_fdd_remove_for(&fdd, ".") != TYPE_TRUE ||
        !fdd.connect.flagDiskExist || fdd.connect.media_generation != fdd_generation ||
        fdd.connect.pImgBase != fdd_image)) {
        failed = 1;
    }
    if (!failed && vm_media_provider_file_exists(
            vm_media_provider_invalid_path_temporary)) failed = 1;
    if (!failed && (vm_machine_hdd_replace_bytes(&hdd, STD_NULL, 0u) != TYPE_FALSE ||
        hdd.connect.geometry_cylinders != 0u || hdd.connect.geometry_heads != 16u ||
        hdd.connect.geometry_sectors_per_track != 63u ||
        vm_media_provider_expect_hdd_capacity(&hdd, registry, 0u, 0u) ||
        vm_machine_hdd_replace_bytes(&hdd, hdd_bytes, 1u) != TYPE_FALSE ||
        vm_media_provider_expect_hdd_capacity(&hdd, registry, 1u, 0x5au) ||
        vm_machine_hdd_replace_bytes(&hdd, hdd_bytes, 511u) != TYPE_FALSE ||
        vm_media_provider_expect_hdd_capacity(&hdd, registry, 511u, 0x5au) ||
        vm_machine_hdd_replace_bytes(&hdd, hdd_bytes, 512u) != TYPE_FALSE ||
        vm_media_provider_expect_hdd_capacity(&hdd, registry, 512u, 0x5au) ||
        vm_machine_hdd_replace_bytes(&hdd, hdd_bytes, 513u) != TYPE_FALSE ||
        vm_media_provider_expect_hdd_capacity(&hdd, registry, 513u, 0x5au))) {
        failed = 1;
    }
    hdd_generation = hdd.connect.media_generation;
    hdd_image = hdd.connect.pImgBase;
    if (!failed && (vm_machine_hdd_replace_bytes(&hdd, STD_NULL, 1u) != TYPE_TRUE ||
        hdd.connect.media_generation != hdd_generation || hdd.connect.pImgBase != hdd_image ||
        hdd.connect.raw_byte_count != 513u || hdd.connect.virtual_byte_count != 1024u ||
        core_machine_media_write_bytes(registry, 2u, 513u, &tail_write, 1u,
            &result) != TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        !hdd.connect.flagPaddingWritten)) {
        failed = 1;
    }
    if (!failed && (vm_machine_hdd_remove(&hdd, "vm_media_provider_t280.img") !=
            TYPE_FALSE || hdd.connect.flagDiskExist || hdd.connect.raw_byte_count != 1024u ||
        hdd.connect.flagPaddingWritten || vm_machine_hdd_insert(&hdd,
            "vm_media_provider_t280.img") != TYPE_FALSE ||
        vm_media_provider_expect_hdd_capacity(&hdd, registry, 1024u, 0x5au) ||
        core_machine_media_read_bytes(registry, 2u, 513u, &bytes[0], 1u,
            &result) != TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        bytes[0] != 0xa7u)) {
        failed = 1;
    }
    hdd_generation = hdd.connect.media_generation;
    hdd_image = hdd.connect.pImgBase;
    if (!failed && (vm_machine_hdd_remove(&hdd, ".") != TYPE_TRUE ||
        !hdd.connect.flagDiskExist || hdd.connect.media_generation != hdd_generation ||
        hdd.connect.pImgBase != hdd_image || hdd.connect.raw_byte_count != 1024u)) {
        failed = 1;
    }
    if (!failed && vm_media_provider_file_exists(
            vm_media_provider_invalid_path_temporary)) failed = 1;
    core_machine_media_registry_destroy(registry);
    vm_machine_fdd_finalize(&fdd);
    vm_machine_fdd_finalize(&failed_fdd);
    vm_machine_fdd_finalize(&stale_fdd);
    vm_machine_fdd_finalize(&malformed_fdd);
    vm_machine_hdd_finalize(&hdd);
    (C_VOID)STD_REMOVE("vm_media_provider_t280.img");
    (C_VOID)STD_REMOVE(vm_media_provider_sidecar_image);
    (C_VOID)STD_REMOVE(vm_media_provider_sidecar_metadata);
    (C_VOID)STD_REMOVE(vm_media_provider_sidecar_backup_collision);
    (C_VOID)STD_REMOVE(vm_media_provider_malformed_image);
    (C_VOID)STD_REMOVE(vm_media_provider_malformed_metadata);
    (C_VOID)STD_REMOVE(vm_media_provider_save_target);
    (C_VOID)STD_REMOVE(vm_media_provider_save_collision);
    if (failed) return 1;
    STD_PRINTF("M5:T272:S2:VM-MEDIA-PROVIDER:OK\n");
    STD_PRINTF("M5:T280:S2:ATOMIC-MEDIA:OK\n");
    STD_PRINTF("M5:T283:S6:ATOMIC-SAVE:OK\n");
    STD_PRINTF("M5:T376:S2:RAW-IMG-SIDECAR-LIFECYCLE:OK\n");
    STD_PRINTF("M5:T376:S6:MALFORMED-SIDECAR:OK\n");
    STD_PRINTF("M5:T441:S1:MEDIA-SAVE-FAILURE-HYGIENE:OK\n");
    return 0;
}
