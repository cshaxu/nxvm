#include "lib/types/types_interface.h"
#include "app-nxvm/profiles/model40/model40_private.h"

#include "ibmpc/board-common/rom_validation_interface.h"
#include "ibmpc/board-common/rom_mapping_interface.h"

lib_i32 vm_profile_model40_external_rom_is_valid(
    const vm_profile_model40_external_rom *rom)
{
    return rom != LIB_NULL && rom->even_bytes != LIB_NULL &&
        rom->odd_bytes != LIB_NULL &&
        rom->chip_byte_count == VM_PROFILE_MODEL40_ROM_CHIP_BYTES;
}

lib_status vm_profile_model40_external_rom_create(
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
            video_bytes, VM_PROFILE_MODEL40_VIDEO_ROM_BYTES))) return LIB_STATUS_INVALID_ARGUMENT;
    rom = (vm_profile_model40_external_rom) { even, odd,
        VM_PROFILE_MODEL40_ROM_CHIP_BYTES, video, video == LIB_NULL ? 0u : video_bytes };
    *out_rom = rom;
    return LIB_STATUS_OK;
}

static lib_status vm_profile_model40_firmware_configure(void *opaque,
    core_machine_firmware_context *firmware)
{
    const vm_profile_model40_external_rom *rom =
        (const vm_profile_model40_external_rom *)opaque;
    lib_u8 window[VM_PROFILE_MODEL40_ROM_WINDOW_BYTES];
    lib_status status;

    if (!vm_profile_model40_external_rom_is_valid(rom)) return LIB_STATUS_INVALID_ARGUMENT;
    status = vm_profile_rom_interleave(window, sizeof(window),
        rom->even_bytes, rom->chip_byte_count, rom->odd_bytes, rom->chip_byte_count);
    if (status != LIB_STATUS_OK) return status;
    vm_profile_rom_region regions[2u] = {
        {VM_PROFILE_MODEL40_ROM_LOW_PHYSICAL_START, window, sizeof(window)}
    };
    vm_profile_rom_alias aliases[4u];
    lib_size region_count = 1u;
    lib_size alias_count = 0u;

    if (rom->video_bytes != LIB_NULL &&
        vm_profile_byob_option_rom_is_valid(rom->video_bytes, rom->video_byte_count,
            VM_PROFILE_MODEL40_VIDEO_ROM_BYTES)) {
        regions[region_count++] = (vm_profile_rom_region) {
            VM_PROFILE_MODEL40_VIDEO_ROM_PHYSICAL_START,
            rom->video_bytes, rom->video_byte_count };
        aliases[alias_count++] = (vm_profile_rom_alias) {
            VM_PROFILE_MODEL40_VIDEO_ROM_PHYSICAL_START +
                VM_PROFILE_MODEL40_VIDEO_ROM_ALIAS_SKIP_BYTES,
            VM_PROFILE_MODEL40_VIDEO_ROM_COMPATIBILITY_ALIAS_START +
                VM_PROFILE_MODEL40_VIDEO_ROM_ALIAS_SKIP_BYTES,
            rom->video_byte_count - VM_PROFILE_MODEL40_VIDEO_ROM_ALIAS_SKIP_BYTES };
    }
    aliases[alias_count++] = (vm_profile_rom_alias) {
        VM_PROFILE_MODEL40_ROM_LOW_PHYSICAL_START,
        VM_PROFILE_MODEL40_ROM_COMPATIBILITY_ALIAS_START, sizeof(window) };
    aliases[alias_count++] = (vm_profile_rom_alias) {
        VM_PROFILE_MODEL40_ROM_LOW_PHYSICAL_START,
        VM_PROFILE_MODEL40_ROM_HIGH_ALIAS_START, sizeof(window) };
    aliases[alias_count++] = (vm_profile_rom_alias) {
        VM_PROFILE_MODEL40_ROM_LOW_PHYSICAL_START,
        VM_PROFILE_MODEL40_ROM_HIGH_RESET_ALIAS_START, sizeof(window) };
    return vm_profile_rom_register(firmware, regions, region_count, aliases, alias_count);
}

static lib_status vm_profile_model40_firmware_reset(void *opaque,
    core_machine_firmware_context *firmware)
{
    (void)firmware;
    return vm_profile_model40_external_rom_is_valid(
        (const vm_profile_model40_external_rom *)opaque) ? LIB_STATUS_OK :
        LIB_STATUS_INVALID_ARGUMENT;
}

static const core_machine_firmware_provider vm_profile_model40_provider = {
    vm_profile_model40_firmware_configure,
    vm_profile_model40_firmware_reset,
    LIB_NULL
};

const core_machine_firmware_provider *vm_profile_model40_firmware_provider(void)
{
    return &vm_profile_model40_provider;
}
