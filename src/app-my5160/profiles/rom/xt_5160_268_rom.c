#include "lib/types/types_interface.h"

#include "app-my5160/profiles/xt_5160_268.h"
#include "core/board-base/rom_validation_interface.h"
#include "core/board-base/rom_mapping_interface.h"

lib_status vm_profile_xt_5160_268_external_rom_create(
    const lib_u8 *system, lib_size system_bytes,
    const lib_u8 *xebec, lib_size xebec_bytes,
    const lib_u8 *video, lib_size video_bytes,
    vm_profile_xt_5160_268_external_rom *out_rom)
{
    if (out_rom == LIB_NULL || system == LIB_NULL ||
        system_bytes != VM_PROFILE_XT_5160_268_SYSTEM_ROM_BYTES ||
        (xebec == LIB_NULL && xebec_bytes != 0u) ||
        (xebec != LIB_NULL && xebec_bytes != VM_PROFILE_XT_5160_268_XEBEC_ROM_BYTES) ||
        (video == LIB_NULL && video_bytes != 0u) ||
        (video != LIB_NULL && !vm_profile_byob_option_rom_is_valid(video,
            video_bytes, VM_PROFILE_BYOB_OPTION_ROM_MAX_BYTES))) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_rom = (vm_profile_xt_5160_268_external_rom) { system, xebec, video,
        video == LIB_NULL ? 0u : video_bytes, xebec != LIB_NULL };
    return LIB_STATUS_OK;
}

static lib_status vm_profile_xt_5160_268_firmware_configure(void *opaque,
    core_machine_firmware_context *firmware)
{
    const vm_profile_xt_5160_268_external_rom *rom = opaque;
    vm_profile_rom_region regions[3u];
    lib_size count = 1u;

    if (rom == LIB_NULL || rom->system_bytes == LIB_NULL ||
        (rom->xebec_present && rom->xebec_bytes == LIB_NULL))
        return LIB_STATUS_INVALID_ARGUMENT;
    regions[0] = (vm_profile_rom_region) {
        VM_PROFILE_XT_5160_268_SYSTEM_ROM_PHYSICAL_START, rom->system_bytes,
        VM_PROFILE_XT_5160_268_SYSTEM_ROM_BYTES };
    if (rom->video_bytes != LIB_NULL)
        regions[count++] = (vm_profile_rom_region) {0x000c0000u,
            rom->video_bytes, rom->video_byte_count};
    if (rom->xebec_present)
        regions[count++] = (vm_profile_rom_region) {
            VM_PROFILE_XT_5160_268_XEBEC_ROM_PHYSICAL_START, rom->xebec_bytes,
            VM_PROFILE_XT_5160_268_XEBEC_ROM_BYTES };
    return vm_profile_rom_register(firmware, regions, count, LIB_NULL, 0u);
}

static lib_status vm_profile_xt_5160_268_firmware_reset(void *opaque,
    core_machine_firmware_context *firmware)
{
    const vm_profile_xt_5160_268_external_rom *rom = opaque;

    (void)firmware;
    return rom == LIB_NULL || rom->system_bytes == LIB_NULL ||
        (rom->xebec_present && rom->xebec_bytes == LIB_NULL) ||
        (rom->video_bytes != LIB_NULL && !vm_profile_byob_option_rom_is_valid(
            rom->video_bytes, rom->video_byte_count,
            VM_PROFILE_BYOB_OPTION_ROM_MAX_BYTES)) ?
        LIB_STATUS_INVALID_ARGUMENT : LIB_STATUS_OK;
}

static const core_machine_firmware_provider vm_profile_xt_5160_268_provider = {
    vm_profile_xt_5160_268_firmware_configure,
    vm_profile_xt_5160_268_firmware_reset,
    LIB_NULL
};

const core_machine_firmware_provider *vm_profile_xt_5160_268_firmware_provider(void)
{
    return &vm_profile_xt_5160_268_provider;
}
