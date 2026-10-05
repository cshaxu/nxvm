#include "lib/types/types_interface.h"
#include "ibmpc/board-common/floppy_interface.h"
#include "ibmpc/board-common/rom_validation_interface.h"

static lib_i32 floppy_contract(void)
{
    static const lib_u32 sectors[] = {2880u, 2400u, 720u, 1440u};
    static const lib_u8 cmos[] = {0x40u, 0x20u, 0x10u, 0x30u};
    lib_u32 drive, media;

    for (drive = 0u; drive < 4u; ++drive) {
        const core_machine_fdc_channel_provider channel =
            vm_profile_floppy_channel_get((vm_profile_floppy_kind)drive);
        if (channel.sample == LIB_NULL ||
            vm_profile_floppy_cmos_type_get((vm_profile_floppy_kind)drive) != cmos[drive]) return 1;
        for (media = 0u; media < 4u; ++media) {
            const core_machine_media_geometry *geometry =
                vm_profile_floppy_geometry_get((vm_profile_floppy_kind)media);
            const lib_bool accepted = drive == 0u || drive == media ||
                (drive == 1u && media == 2u);
            const lib_bool double_step = drive == 1u && media == 2u;
            const lib_u32 rate = double_step ? 300000u :
                (media < 2u ? 500000u : 250000u);
            lib_u16 cylinder = 0xffffu;
            if (geometry == LIB_NULL || geometry->logical_sector_count != sectors[media]) return 1;
            if (channel.sample(channel.context, 0u, geometry, 2u, rate,
                    LIB_TRUE, &cylinder) != accepted) return 1;
            if (accepted && cylinder != (double_step ? 1u : 2u)) return 1;
            if (channel.sample(channel.context, 0u, geometry, 2u, rate + 1u,
                    LIB_TRUE, &cylinder) ||
                channel.sample(channel.context, 0u, geometry, 2u, rate,
                    LIB_FALSE, &cylinder) ||
                channel.sample(channel.context, 0u, geometry,
                    (lib_u16)(geometry->cylinders * (double_step ? 2u : 1u)),
                    rate, LIB_TRUE, &cylinder)) return 1;
            if (double_step && channel.sample(channel.context, 0u, geometry,
                    1u, rate, LIB_TRUE, &cylinder)) return 1;
        }
    }
    return vm_profile_floppy_geometry_get((vm_profile_floppy_kind)4u) != LIB_NULL ||
        vm_profile_floppy_channel_get((vm_profile_floppy_kind)4u).sample != LIB_NULL;
}

static lib_i32 option_rom_contract(void)
{
    lib_u8 bytes[512] = {0x55u, 0xaau, 1u};
    if (!vm_profile_byob_option_rom_is_valid(bytes, sizeof(bytes), sizeof(bytes))) return 1;
    if (vm_profile_byob_option_rom_is_valid(LIB_NULL, sizeof(bytes), sizeof(bytes)) ||
        vm_profile_byob_option_rom_is_valid(bytes, 2u, sizeof(bytes)) ||
        vm_profile_byob_option_rom_is_valid(bytes, sizeof(bytes), sizeof(bytes) - 1u)) return 1;
    bytes[0u] = 0u;
    if (vm_profile_byob_option_rom_is_valid(bytes, sizeof(bytes), sizeof(bytes))) return 1;
    bytes[0u] = 0x55u;
    bytes[2u] = 2u;
    if (vm_profile_byob_option_rom_is_valid(bytes, sizeof(bytes), sizeof(bytes))) return 1;
    bytes[2u] = 1u;
    bytes[511u] = 1u;
    return vm_profile_byob_option_rom_is_valid(bytes, sizeof(bytes), sizeof(bytes));
}

lib_i32 main(void)
{
    return floppy_contract() || option_rom_contract();
}
