#include "lib/types/types_interface.h"
#include "core/board-base/floppy_interface.h"
#include "core/board-base/rom_validation_interface.h"

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

static lib_i32 interleave_contract(void)
{
    static lib_u8 even[32768u], odd[32768u], image[65536u];
    const lib_size sizes[] = {1u, 16384u, 32768u};
    for (lib_size index = 0u; index < sizeof(even); ++index) {
        even[index] = (lib_u8)index;
        odd[index] = (lib_u8)(index ^ 0xa5u);
    }
    for (lib_size test = 0u; test < sizeof(sizes) / sizeof(sizes[0]); ++test) {
        if (vm_profile_rom_interleave(image, sizes[test] * 2u,
            even, sizes[test], odd, sizes[test]) != LIB_STATUS_OK) return 1;
        for (lib_size index = 0u; index < sizes[test]; ++index)
            if (image[index * 2u] != even[index] || image[index * 2u + 1u] != odd[index]) return 1;
    }
    lib_memory_set(image, 0xcc, sizeof(image));
    if (vm_profile_rom_interleave(LIB_NULL, sizeof(image), even, sizeof(even), odd, sizeof(odd)) != LIB_STATUS_INVALID_ARGUMENT ||
        vm_profile_rom_interleave(image, sizeof(image), LIB_NULL, sizeof(even), odd, sizeof(odd)) != LIB_STATUS_INVALID_ARGUMENT ||
        vm_profile_rom_interleave(image, sizeof(image), even, sizeof(even), LIB_NULL, sizeof(odd)) != LIB_STATUS_INVALID_ARGUMENT ||
        vm_profile_rom_interleave(image, sizeof(image), even, 0u, odd, 0u) != LIB_STATUS_INVALID_ARGUMENT ||
        vm_profile_rom_interleave(image, sizeof(image), even, sizeof(even), odd, sizeof(odd) - 1u) != LIB_STATUS_INVALID_ARGUMENT ||
        vm_profile_rom_interleave(image, sizeof(image) - 1u, even, sizeof(even), odd, sizeof(odd)) != LIB_STATUS_INVALID_ARGUMENT ||
        vm_profile_rom_interleave(image, sizeof(image) - 2u, even, sizeof(even), odd, sizeof(odd)) != LIB_STATUS_INVALID_ARGUMENT ||
        vm_profile_rom_interleave(image, sizeof(image), even, (lib_size)-1, odd, (lib_size)-1) != LIB_STATUS_INVALID_ARGUMENT) return 1;
    for (lib_size index = 0u; index < sizeof(image); ++index) if (image[index] != 0xccu) return 1;
    return 0;
}

lib_i32 main(void)
{
    return floppy_contract() || option_rom_contract() || interleave_contract();
}
