#include "type.h"

#include "core/machine/media_interface.h"
#include "vm/machine/fdd.h"
#include "vm/machine/hdd.h"

static uint8_t vm_media_provider_fdd_image[80u * 2u * 18u * 512u];

static C_INT vm_media_provider_expect_hdd_capacity(t_hdd *hdd,
    core_machine_media_registry *registry, STD_SIZE_T raw_byte_count,
    uint8_t expected_first_byte)
{
    core_machine_media_info info;
    core_machine_media_result result;
    uint8_t tail_byte = 0xffu;
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

C_INT main(C_VOID)
{
    core_machine_media_registry registry;
    core_machine_media_info info;
    core_machine_media_result result;
    t_fdd fdd;
    t_hdd hdd;
    uint8_t bytes[512] = {0};
    uint8_t hdd_bytes[513] = {0x5au};
    uint8_t tail_write = 0xa7u;
    uint64_t fdd_generation;
    uint64_t hdd_generation;
    type_virtual_address hdd_image;
    C_INT failed = 0;

    vm_machine_fdd_initialize(&fdd);
    vm_machine_hdd_initialize(&hdd);
    vm_machine_fdd_create_for(&fdd);
    vm_machine_hdd_create(&hdd, 2u);
    core_machine_media_registry_initialize(&registry);
    if (core_machine_media_registry_bind(&registry, 1u, &fdd,
            vm_machine_fdd_media_provider()) != TYPE_STATUS_OK ||
        core_machine_media_registry_bind(&registry, 2u, &hdd,
            vm_machine_hdd_media_provider()) != TYPE_STATUS_OK ||
        core_machine_media_registry_freeze(&registry) != TYPE_STATUS_OK ||
        core_machine_media_query(&registry, 1u, &info, &result) != TYPE_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK || !info.present ||
        info.geometry.bytes_per_sector != 512u ||
        core_machine_media_query(&registry, 2u, &info, &result) != TYPE_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK || info.geometry.cylinders != 2u) {
        failed = 1;
    }
    fdd_generation = fdd.connect.media_generation;
    if (!failed && (core_machine_media_write_sectors(&registry, 1u, 0u, 1u,
            bytes, &result) != TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        core_machine_media_format_sectors(&registry, 1u, 0u, 1u, 0xa5u,
            &result) != TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        fdd.connect.media_generation != fdd_generation + 1u ||
        core_machine_media_read_sectors(&registry, 1u, 0u, 1u, bytes,
            &result) != TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        bytes[0] != 0xa5u ||
        core_machine_media_read_sectors(&registry, 2u,
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
    if (!failed && (vm_machine_hdd_replace_bytes(&hdd, STD_NULL, 0u) != TYPE_FALSE ||
        vm_media_provider_expect_hdd_capacity(&hdd, &registry, 0u, 0u) ||
        vm_machine_hdd_replace_bytes(&hdd, hdd_bytes, 1u) != TYPE_FALSE ||
        vm_media_provider_expect_hdd_capacity(&hdd, &registry, 1u, 0x5au) ||
        vm_machine_hdd_replace_bytes(&hdd, hdd_bytes, 511u) != TYPE_FALSE ||
        vm_media_provider_expect_hdd_capacity(&hdd, &registry, 511u, 0x5au) ||
        vm_machine_hdd_replace_bytes(&hdd, hdd_bytes, 512u) != TYPE_FALSE ||
        vm_media_provider_expect_hdd_capacity(&hdd, &registry, 512u, 0x5au) ||
        vm_machine_hdd_replace_bytes(&hdd, hdd_bytes, 513u) != TYPE_FALSE ||
        vm_media_provider_expect_hdd_capacity(&hdd, &registry, 513u, 0x5au))) {
        failed = 1;
    }
    hdd_generation = hdd.connect.media_generation;
    hdd_image = hdd.connect.pImgBase;
    if (!failed && (vm_machine_hdd_replace_bytes(&hdd, STD_NULL, 1u) != TYPE_TRUE ||
        hdd.connect.media_generation != hdd_generation || hdd.connect.pImgBase != hdd_image ||
        hdd.connect.raw_byte_count != 513u || hdd.connect.virtual_byte_count != 1024u ||
        core_machine_media_write_bytes(&registry, 2u, 513u, &tail_write, 1u,
            &result) != TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        !hdd.connect.flagPaddingWritten)) {
        failed = 1;
    }
    if (!failed && (vm_machine_hdd_remove(&hdd, "vm_media_provider_t280.img") !=
            TYPE_FALSE || hdd.connect.flagDiskExist || hdd.connect.raw_byte_count != 1024u ||
        hdd.connect.flagPaddingWritten || vm_machine_hdd_insert(&hdd,
            "vm_media_provider_t280.img") != TYPE_FALSE ||
        vm_media_provider_expect_hdd_capacity(&hdd, &registry, 1024u, 0x5au) ||
        core_machine_media_read_bytes(&registry, 2u, 513u, &bytes[0], 1u,
            &result) != TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        bytes[0] != 0xa7u)) {
        failed = 1;
    }
    core_machine_media_registry_finalize(&registry);
    vm_machine_fdd_finalize(&fdd);
    vm_machine_hdd_finalize(&hdd);
    (C_VOID)STD_REMOVE("vm_media_provider_t280.img");
    if (failed) return 1;
    STD_PRINTF("M5:T272:S2:VM-MEDIA-PROVIDER:OK\n");
    STD_PRINTF("M5:T280:S2:ATOMIC-MEDIA:OK\n");
    return 0;
}
