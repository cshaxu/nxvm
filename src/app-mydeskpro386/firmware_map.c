#include "lib/types/types_interface.h"
#include "app-mydeskpro386/machine_config.h"

#include "core/board-base/rom_validation_interface.h"
#include "core/board-base/rom_mapping_interface.h"

lib_i32 mydeskpro386_external_rom_is_valid(
    const mydeskpro386_external_rom *rom)
{
    return rom != LIB_NULL && rom->even_bytes != LIB_NULL &&
        rom->odd_bytes != LIB_NULL &&
        rom->chip_byte_count == MYDESKPRO386_ROM_CHIP_BYTES;
}

lib_status mydeskpro386_external_rom_create(
    const lib_u8 *even, lib_size even_bytes,
    const lib_u8 *odd, lib_size odd_bytes,
    const lib_u8 *video, lib_size video_bytes,
    mydeskpro386_external_rom *out_rom)
{
    mydeskpro386_external_rom rom;

    if (out_rom == LIB_NULL || even == LIB_NULL || odd == LIB_NULL ||
        even_bytes != MYDESKPRO386_ROM_CHIP_BYTES ||
        odd_bytes != MYDESKPRO386_ROM_CHIP_BYTES ||
        (video == LIB_NULL && video_bytes != 0u) ||
        (video != LIB_NULL && !vm_profile_byob_option_rom_is_valid(video,
            video_bytes, MYDESKPRO386_VIDEO_ROM_BYTES))) return LIB_STATUS_INVALID_ARGUMENT;
    rom = (mydeskpro386_external_rom) { even, odd,
        MYDESKPRO386_ROM_CHIP_BYTES, video, video == LIB_NULL ? 0u : video_bytes };
    *out_rom = rom;
    return LIB_STATUS_OK;
}

static lib_status mydeskpro386_firmware_configure(void *opaque,
    core_machine_firmware_context *firmware)
{
    const mydeskpro386_external_rom *rom =
        (const mydeskpro386_external_rom *)opaque;
    lib_u8 window[MYDESKPRO386_ROM_WINDOW_BYTES];
    lib_status status;

    if (!mydeskpro386_external_rom_is_valid(rom)) return LIB_STATUS_INVALID_ARGUMENT;
    status = vm_profile_rom_interleave(window, sizeof(window),
        rom->even_bytes, rom->chip_byte_count, rom->odd_bytes, rom->chip_byte_count);
    if (status != LIB_STATUS_OK) return status;
    vm_profile_rom_region regions[2u] = {
        {MYDESKPRO386_ROM_LOW_PHYSICAL_START, window, sizeof(window)}
    };
    vm_profile_rom_alias aliases[4u];
    lib_size region_count = 1u;
    lib_size alias_count = 0u;

    if (rom->video_bytes != LIB_NULL &&
        vm_profile_byob_option_rom_is_valid(rom->video_bytes, rom->video_byte_count,
            MYDESKPRO386_VIDEO_ROM_BYTES)) {
        regions[region_count++] = (vm_profile_rom_region) {
            MYDESKPRO386_VIDEO_ROM_PHYSICAL_START,
            rom->video_bytes, rom->video_byte_count };
        aliases[alias_count++] = (vm_profile_rom_alias) {
            MYDESKPRO386_VIDEO_ROM_PHYSICAL_START +
                MYDESKPRO386_VIDEO_ROM_ALIAS_SKIP_BYTES,
            MYDESKPRO386_VIDEO_ROM_COMPATIBILITY_ALIAS_START +
                MYDESKPRO386_VIDEO_ROM_ALIAS_SKIP_BYTES,
            rom->video_byte_count - MYDESKPRO386_VIDEO_ROM_ALIAS_SKIP_BYTES };
    }
    aliases[alias_count++] = (vm_profile_rom_alias) {
        MYDESKPRO386_ROM_LOW_PHYSICAL_START,
        MYDESKPRO386_ROM_COMPATIBILITY_ALIAS_START, sizeof(window) };
    aliases[alias_count++] = (vm_profile_rom_alias) {
        MYDESKPRO386_ROM_LOW_PHYSICAL_START,
        MYDESKPRO386_ROM_HIGH_ALIAS_START, sizeof(window) };
    aliases[alias_count++] = (vm_profile_rom_alias) {
        MYDESKPRO386_ROM_LOW_PHYSICAL_START,
        MYDESKPRO386_ROM_HIGH_RESET_ALIAS_START, sizeof(window) };
    return vm_profile_rom_register(firmware, regions, region_count, aliases, alias_count);
}

static lib_status mydeskpro386_firmware_reset(void *opaque,
    core_machine_firmware_context *firmware)
{
    (void)firmware;
    return mydeskpro386_external_rom_is_valid(
        (const mydeskpro386_external_rom *)opaque) ? LIB_STATUS_OK :
        LIB_STATUS_INVALID_ARGUMENT;
}

static const core_machine_firmware_provider mydeskpro386_provider = {
    mydeskpro386_firmware_configure,
    mydeskpro386_firmware_reset,
    LIB_NULL
};

const core_machine_firmware_provider *mydeskpro386_firmware_provider(void)
{
    return &mydeskpro386_provider;
}
