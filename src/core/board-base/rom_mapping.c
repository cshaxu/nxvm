#include "core/board-base/rom_mapping_interface.h"

lib_status vm_profile_rom_register(core_machine_firmware_context *firmware,
    const vm_profile_rom_region *regions, lib_size region_count,
    const vm_profile_rom_alias *aliases, lib_size alias_count)
{
    lib_status status;

    if (firmware == LIB_NULL || (region_count != 0u && regions == LIB_NULL) ||
        (alias_count != 0u && aliases == LIB_NULL)) return LIB_STATUS_INVALID_ARGUMENT;
    for (lib_size index = 0u; index < region_count; ++index) {
        status = core_machine_firmware_register_immutable_rom(firmware,
            regions[index].physical_start, regions[index].image, regions[index].bytes);
        if (status != LIB_STATUS_OK) return status;
    }
    for (lib_size index = 0u; index < alias_count; ++index) {
        status = core_machine_firmware_register_immutable_rom_alias(firmware,
            aliases[index].source_start, aliases[index].physical_start, aliases[index].bytes);
        if (status != LIB_STATUS_OK) return status;
    }
    return LIB_STATUS_OK;
}
