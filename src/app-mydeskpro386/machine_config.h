#ifndef MYDESKPRO386_MACHINE_CONFIG_H
#define MYDESKPRO386_MACHINE_CONFIG_H
#include "lib/types/types_interface.h"

#include "core/x86/firmware_interface.h"
#include "core/board-base/machine_board_interface.h"
#include "app-mydeskpro386/firmware_layout.h"
#include "core/board-base/profile_contract_interface.h"

#define MYDESKPRO386_ROM_LOGICAL_BYTES (2u * MYDESKPRO386_ROM_CHIP_BYTES)
#define MYDESKPRO386_ROM_WINDOW_BYTES MYDESKPRO386_ROM_LOGICAL_BYTES
#define MYDESKPRO386_ROM_LOW_PHYSICAL_START 0x000f8000u
#define MYDESKPRO386_ROM_COMPATIBILITY_ALIAS_START 0x000f0000u
#define MYDESKPRO386_ROM_HIGH_ALIAS_START 0xffff0000u
#define MYDESKPRO386_ROM_HIGH_RESET_ALIAS_START 0xffff8000u
#define MYDESKPRO386_VIDEO_ROM_PHYSICAL_START 0x000c0000u
#define MYDESKPRO386_VIDEO_ROM_COMPATIBILITY_ALIAS_START 0x000e0000u
#define MYDESKPRO386_VIDEO_ROM_ALIAS_SKIP_BYTES 3u

/* Test-visible board addresses; Profile owns decoding, Core owns dispatch. */
#define MYDESKPRO386_D4_COMPATIBILITY_START 0x00fe0000u
#define MYDESKPRO386_D4_CONTROL_PHYSICAL 0x80c00000u

typedef struct mydeskpro386_external_rom {
    const lib_u8 *even_bytes;
    const lib_u8 *odd_bytes;
    lib_size chip_byte_count;
    const lib_u8 *video_bytes;
    lib_size video_byte_count;
} mydeskpro386_external_rom;

lib_i32 mydeskpro386_external_rom_is_valid(
    const mydeskpro386_external_rom *rom);
lib_status mydeskpro386_external_rom_create(
    const lib_u8 *even, lib_size even_bytes,
    const lib_u8 *odd, lib_size odd_bytes,
    const lib_u8 *video, lib_size video_bytes,
    mydeskpro386_external_rom *out_rom);
void mydeskpro386_core_config_initialize(core_machine_config *out_config);
lib_status mydeskpro386_values_create(vm_profile_contract_values *out_values);
const core_machine_firmware_provider *mydeskpro386_firmware_provider(void);
#endif
