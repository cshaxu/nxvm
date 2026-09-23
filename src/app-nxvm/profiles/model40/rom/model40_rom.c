#include "lib/types/types_interface.h"
#include "app-nxvm/profiles/model40/model40_private.h"

#include "app-nxvm/profiles/byob/blob.h"

C_INT vm_profile_model40_external_rom_is_valid(
    const vm_profile_model40_external_rom *rom)
{
    return rom != LIB_NULL && rom->even_bytes != LIB_NULL &&
        rom->odd_bytes != LIB_NULL &&
        rom->chip_byte_count == VM_PROFILE_MODEL40_ROM_CHIP_BYTES;
}

type_status vm_profile_model40_external_rom_create(
    const lib_u8 *even, lib_size even_bytes,
    const lib_u8 *odd, lib_size odd_bytes,
    const lib_u8 *video, lib_size video_bytes,
    vm_profile_model40_external_rom *out_rom)
{
    vm_profile_model40_external_rom rom;

    if (out_rom == LIB_NULL || even == LIB_NULL || odd == LIB_NULL ||
        even_bytes != VM_PROFILE_MODEL40_ROM_CHIP_BYTES ||
        odd_bytes != VM_PROFILE_MODEL40_ROM_CHIP_BYTES ||
        (video == LIB_NULL && video_bytes != 0u) ||
        (video != LIB_NULL && !vm_profile_byob_option_rom_is_valid(video,
            video_bytes, VM_PROFILE_MODEL40_VIDEO_ROM_BYTES))) return TYPE_STATUS_INVALID_ARGUMENT;
    rom = (vm_profile_model40_external_rom) { even, odd,
        VM_PROFILE_MODEL40_ROM_CHIP_BYTES, video, video == LIB_NULL ? 0u : video_bytes };
    *out_rom = rom;
    return TYPE_STATUS_OK;
}

C_INT vm_profile_model40_byob_manifest_is_valid(
    const vm_profile_model40_byob_manifest *manifest)
{
    return manifest != LIB_NULL && manifest->even_path != LIB_NULL &&
        manifest->odd_path != LIB_NULL && manifest->even_path[0] != '\0' &&
        manifest->odd_path[0] != '\0' &&
        ((manifest->even_sha256 == LIB_NULL && manifest->odd_sha256 == LIB_NULL) ||
         (manifest->even_sha256 != LIB_NULL && manifest->odd_sha256 != LIB_NULL &&
          manifest->even_sha256[0] != '\0' && manifest->odd_sha256[0] != '\0')) &&
        ((manifest->video_path == LIB_NULL && manifest->video_sha256 == LIB_NULL) ||
         (manifest->video_path != LIB_NULL && manifest->video_path[0] != '\0' &&
          (manifest->video_sha256 == LIB_NULL || manifest->video_sha256[0] != '\0')));
}

type_status vm_profile_model40_byob_manifest_load(
    const vm_profile_model40_byob_manifest *manifest,
    lib_u8 *even_bytes, lib_u8 *odd_bytes,
    lib_u8 *video_bytes,
    vm_profile_model40_external_rom *out_rom)
{
    type_status status;

    if (!vm_profile_model40_byob_manifest_is_valid(manifest) || even_bytes == LIB_NULL ||
        odd_bytes == LIB_NULL || video_bytes == LIB_NULL || out_rom == LIB_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    status = vm_profile_byob_blob_load(&(vm_profile_byob_blob) {
        manifest->even_path, manifest->even_sha256,
        VM_PROFILE_MODEL40_ROM_CHIP_BYTES}, even_bytes);
    if (status != TYPE_STATUS_OK) return status;
    status = vm_profile_byob_blob_load(&(vm_profile_byob_blob) {
        manifest->odd_path, manifest->odd_sha256,
        VM_PROFILE_MODEL40_ROM_CHIP_BYTES}, odd_bytes);
    if (status != TYPE_STATUS_OK) {
        lib_memory_set(even_bytes, 0, VM_PROFILE_MODEL40_ROM_CHIP_BYTES);
        return status;
    }
    if (manifest->video_path != LIB_NULL) {
        status = vm_profile_byob_blob_load(&(vm_profile_byob_blob) {
            manifest->video_path, manifest->video_sha256,
            VM_PROFILE_MODEL40_VIDEO_ROM_BYTES}, video_bytes);
        if (status != TYPE_STATUS_OK || !vm_profile_byob_option_rom_is_valid(video_bytes,
                VM_PROFILE_MODEL40_VIDEO_ROM_BYTES, VM_PROFILE_MODEL40_VIDEO_ROM_BYTES)) {
            lib_memory_set(even_bytes, 0, VM_PROFILE_MODEL40_ROM_CHIP_BYTES);
            lib_memory_set(odd_bytes, 0, VM_PROFILE_MODEL40_ROM_CHIP_BYTES);
            lib_memory_set(video_bytes, 0, VM_PROFILE_MODEL40_VIDEO_ROM_BYTES);
            return TYPE_STATUS_FAULT;
        }
    }
    return vm_profile_model40_external_rom_create(even_bytes,
        VM_PROFILE_MODEL40_ROM_CHIP_BYTES, odd_bytes, VM_PROFILE_MODEL40_ROM_CHIP_BYTES,
        manifest->video_path == LIB_NULL ? LIB_NULL : video_bytes,
        manifest->video_path == LIB_NULL ? 0u : VM_PROFILE_MODEL40_VIDEO_ROM_BYTES, out_rom);
}

static C_VOID vm_profile_model40_rom_materialize(
    const vm_profile_model40_external_rom *rom, lib_u8 *window)
{
    lib_size index;

    for (index = 0u; index < VM_PROFILE_MODEL40_ROM_WINDOW_BYTES; ++index) {
        lib_size logical = index % VM_PROFILE_MODEL40_ROM_LOGICAL_BYTES;

        window[index] = (logical & 1u) == 0u ? rom->even_bytes[logical >> 1u] :
            rom->odd_bytes[logical >> 1u];
    }
}

static type_status vm_profile_model40_firmware_configure(C_VOID *opaque,
    core_machine_firmware_context *firmware)
{
    const vm_profile_model40_external_rom *rom =
        (const vm_profile_model40_external_rom *)opaque;
    lib_u8 window[VM_PROFILE_MODEL40_ROM_WINDOW_BYTES];
    type_status status;

    if (!vm_profile_model40_external_rom_is_valid(rom)) return TYPE_STATUS_INVALID_ARGUMENT;
    vm_profile_model40_rom_materialize(rom, window);
    status = core_machine_firmware_register_immutable_rom(firmware,
        VM_PROFILE_MODEL40_ROM_LOW_PHYSICAL_START, window, sizeof(window));
    if (status != TYPE_STATUS_OK) return status;
    if (rom->video_bytes != LIB_NULL &&
        vm_profile_byob_option_rom_is_valid(rom->video_bytes, rom->video_byte_count,
            VM_PROFILE_MODEL40_VIDEO_ROM_BYTES)) {
        status = core_machine_firmware_register_immutable_rom(firmware,
            VM_PROFILE_MODEL40_VIDEO_ROM_PHYSICAL_START, rom->video_bytes,
            rom->video_byte_count);
        if (status != TYPE_STATUS_OK) return status;
        status = core_machine_firmware_register_immutable_rom_alias(firmware,
            VM_PROFILE_MODEL40_VIDEO_ROM_PHYSICAL_START +
                VM_PROFILE_MODEL40_VIDEO_ROM_ALIAS_SKIP_BYTES,
            VM_PROFILE_MODEL40_VIDEO_ROM_COMPATIBILITY_ALIAS_START +
                VM_PROFILE_MODEL40_VIDEO_ROM_ALIAS_SKIP_BYTES,
            rom->video_byte_count - VM_PROFILE_MODEL40_VIDEO_ROM_ALIAS_SKIP_BYTES);
        if (status != TYPE_STATUS_OK) return status;
    }
    status = core_machine_firmware_register_immutable_rom_alias(firmware,
        VM_PROFILE_MODEL40_ROM_LOW_PHYSICAL_START,
        VM_PROFILE_MODEL40_ROM_COMPATIBILITY_ALIAS_START, sizeof(window));
    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_firmware_register_immutable_rom_alias(firmware,
        VM_PROFILE_MODEL40_ROM_LOW_PHYSICAL_START,
        VM_PROFILE_MODEL40_ROM_HIGH_ALIAS_START, sizeof(window));
    if (status != TYPE_STATUS_OK) return status;
    return core_machine_firmware_register_immutable_rom_alias(firmware,
        VM_PROFILE_MODEL40_ROM_LOW_PHYSICAL_START,
        VM_PROFILE_MODEL40_ROM_HIGH_RESET_ALIAS_START, sizeof(window));
}

static type_status vm_profile_model40_firmware_reset(C_VOID *opaque,
    core_machine_firmware_context *firmware)
{
    (C_VOID)firmware;
    return vm_profile_model40_external_rom_is_valid(
        (const vm_profile_model40_external_rom *)opaque) ? TYPE_STATUS_OK :
        TYPE_STATUS_INVALID_ARGUMENT;
}

static const core_machine_firmware_provider vm_profile_model40_provider = {
    vm_profile_model40_firmware_configure,
    vm_profile_model40_firmware_reset,
    LIB_NULL,
    LIB_NULL
};

const core_machine_firmware_provider *vm_profile_model40_firmware_provider(C_VOID)
{
    return &vm_profile_model40_provider;
}
