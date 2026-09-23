#include <assert.h>

#include "core/cartridge.h"

static void make_image(lib_u8 *bytes, lib_u8 prg_banks, lib_u8 chr_banks)
{
    lib_size index;
    lib_size size = 16u + (lib_size)prg_banks * 16384u + (lib_size)chr_banks * 8192u;
    lib_memory_set(bytes, 0, size);
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = prg_banks; bytes[5] = chr_banks;
    for (index = 0u; index < (lib_size)prg_banks * 16384u; ++index)
        bytes[16u + index] = (lib_u8)(index >> 14u);
}

static void mmc1_write_value(core_cartridge *cartridge, lib_u16 address,
    lib_u8 value)
{
    lib_u8 bit;
    for (bit = 0u; bit < 5u; ++bit)
        assert(core_cartridge_cpu_write(cartridge, address,
            (lib_u8)((value >> bit) & 1u)));
}

static void check_mmc1(void)
{
    lib_u8 image[16u + 32768u + 32768u];
    core_cartridge *cartridge = LIB_NULL;
    lib_size index;

    make_image(image, 2u, 4u);
    image[6] = 0x10u;
    for (index = 0u; index < 32768u; ++index)
        image[16u + index] = (lib_u8)(index >> 14u);
    for (index = 0u; index < 32768u; ++index)
        image[16u + 32768u + index] = (lib_u8)(index >> 12u);
    assert(core_cartridge_create(&cartridge, image, sizeof(image)) == LIB_STATUS_OK);
    assert(core_cartridge_cpu_read(cartridge, 0x8000u) == 0u);
    assert(core_cartridge_cpu_read(cartridge, 0xc000u) == 1u);
    assert(core_cartridge_ppu_read(cartridge, 0u) == 0u);
    mmc1_write_value(cartridge, 0x8000u, 0x1cu);
    mmc1_write_value(cartridge, 0xa000u, 3u);
    mmc1_write_value(cartridge, 0xc000u, 6u);
    assert(core_cartridge_ppu_read(cartridge, 0u) == 3u);
    assert(core_cartridge_ppu_read(cartridge, 0x1000u) == 6u);
    mmc1_write_value(cartridge, 0xe000u, 1u);
    assert(core_cartridge_cpu_read(cartridge, 0x8000u) == 1u);
    assert(core_cartridge_cpu_read(cartridge, 0xc000u) == 1u);
    assert(core_cartridge_cpu_write(cartridge, 0x8000u, 0x80u));
    assert(core_cartridge_cpu_read(cartridge, 0x8000u) == 1u);
    assert(core_cartridge_cpu_read(cartridge, 0xc000u) == 1u);
    mmc1_write_value(cartridge, 0x8000u, 0u);
    assert(core_cartridge_ciram_address(cartridge, 0x2000u) ==
        core_cartridge_ciram_address(cartridge, 0x2c00u));
    mmc1_write_value(cartridge, 0x8000u, 2u);
    assert(core_cartridge_ciram_address(cartridge, 0x2000u) ==
        core_cartridge_ciram_address(cartridge, 0x2800u));
    assert(core_cartridge_ciram_address(cartridge, 0x2000u) !=
        core_cartridge_ciram_address(cartridge, 0x2400u));
    core_cartridge_destroy(cartridge);
}

int main(void)
{
    lib_u8 one_prg[16u + 16384u];
    lib_u8 one_prg_with_trailing_byte[16u + 16384u + 1u];
    lib_u8 two_prg[16u + 32768u + 8192u];
    lib_u8 uxrom[16u + 131072u];
    lib_u8 cnrom[16u + 32768u + 4u * 8192u];
    lib_u8 suffix[16u + 32768u + 128u];
    static lib_u8 maximum_mmc3[16u + 32u * 16384u + 32u * 8192u];
    core_cartridge *cartridge = LIB_NULL;
    lib_u32 value;
    lib_size index;
    lib_size suffix_size;

    make_image(one_prg, 1u, 0u);
    assert(core_cartridge_create(&cartridge, one_prg, sizeof(one_prg)) == LIB_STATUS_OK);
    assert(core_cartridge_cpu_read(cartridge, 0x8000u) == 0u &&
        core_cartridge_cpu_read(cartridge, 0xc000u) == 0u);
    for (index = 0u; index < cartridge->chr_bytes; ++index)
        assert(cartridge->chr[index] == 0u);
    core_cartridge_destroy(cartridge);

    make_image(one_prg, 1u, 0u);
    one_prg[6] = 1u;
    assert(core_cartridge_create(&cartridge, one_prg, sizeof(one_prg)) == LIB_STATUS_OK);
    assert(core_cartridge_ciram_address(cartridge, 0x2000u) ==
        core_cartridge_ciram_address(cartridge, 0x2800u));
    core_cartridge_destroy(cartridge);

    make_image(two_prg, 2u, 1u);
    assert(core_cartridge_create(&cartridge, two_prg, sizeof(two_prg)) == LIB_STATUS_OK);
    assert(core_cartridge_cpu_read(cartridge, 0x8000u) == 0u &&
        core_cartridge_cpu_read(cartridge, 0xc000u) == 1u);
    core_cartridge_destroy(cartridge);

    make_image(one_prg, 1u, 0u);
    one_prg[0] = 0u;
    assert(core_cartridge_create(&cartridge, one_prg, sizeof(one_prg)) == LIB_STATUS_INVALID_ARGUMENT);
    make_image(one_prg, 1u, 0u);
    one_prg[4] = 0u;
    assert(core_cartridge_create(&cartridge, one_prg, sizeof(one_prg)) == LIB_STATUS_INVALID_ARGUMENT);
    make_image(one_prg, 1u, 0u);
    one_prg[5] = 2u;
    assert(core_cartridge_create(&cartridge, one_prg, sizeof(one_prg)) == LIB_STATUS_INVALID_ARGUMENT);
    make_image(one_prg, 1u, 0u);
    one_prg[6] = 2u;
    assert(core_cartridge_create(&cartridge, one_prg, sizeof(one_prg)) == LIB_STATUS_INVALID_ARGUMENT);
    make_image(one_prg, 1u, 0u);
    one_prg[7] = 1u;
    assert(core_cartridge_create(&cartridge, one_prg, sizeof(one_prg)) == LIB_STATUS_INVALID_ARGUMENT);
    for (index = 0u; index < 16u; ++index)
        assert(core_cartridge_create(&cartridge, one_prg, index) == LIB_STATUS_INVALID_ARGUMENT);
    for (value = 0u; value <= 255u; ++value) {
        make_image(one_prg, 1u, 0u);
        one_prg[4] = (lib_u8)value;
        if (value != 1u && value != 2u)
            assert(core_cartridge_create(&cartridge, one_prg, sizeof(one_prg)) ==
                LIB_STATUS_INVALID_ARGUMENT);
        core_cartridge_destroy(cartridge);
        cartridge = LIB_NULL;
        make_image(one_prg, 1u, 0u);
        one_prg[5] = (lib_u8)value;
        if (value > 1u)
            assert(core_cartridge_create(&cartridge, one_prg, sizeof(one_prg)) ==
                LIB_STATUS_INVALID_ARGUMENT);
        core_cartridge_destroy(cartridge);
        cartridge = LIB_NULL;
        make_image(one_prg, 1u, 0u);
        one_prg[6] = (lib_u8)value;
        if (value > 1u)
            assert(core_cartridge_create(&cartridge, one_prg, sizeof(one_prg)) ==
                LIB_STATUS_INVALID_ARGUMENT);
        core_cartridge_destroy(cartridge);
        cartridge = LIB_NULL;
    }
    for (index = 7u; index < 16u; ++index) {
        for (value = 1u; value <= 255u; ++value) {
            make_image(one_prg, 1u, 0u);
            one_prg[index] = (lib_u8)value;
            assert(core_cartridge_create(&cartridge, one_prg, sizeof(one_prg)) ==
                LIB_STATUS_INVALID_ARGUMENT);
        }
    }
    make_image(one_prg, 1u, 0u);
    assert(core_cartridge_create(&cartridge, one_prg, sizeof(one_prg) - 1u) == LIB_STATUS_INVALID_ARGUMENT);
    make_image(one_prg_with_trailing_byte, 1u, 0u);
    assert(core_cartridge_create(&cartridge, one_prg_with_trailing_byte,
        sizeof(one_prg_with_trailing_byte)) == LIB_STATUS_INVALID_ARGUMENT);
    make_image(uxrom, 8u, 0u);
    uxrom[6] = 0x20u;
    assert(core_cartridge_create(&cartridge, uxrom, sizeof(uxrom)) == LIB_STATUS_OK);
    core_cartridge_destroy(cartridge);
    cartridge = LIB_NULL;
    make_image(cnrom, 2u, 1u);
    cnrom[6] = 0x30u;
    assert(core_cartridge_create(&cartridge, cnrom, 16u + 32768u + 8192u) ==
        LIB_STATUS_OK);
    core_cartridge_destroy(cartridge);
    cartridge = LIB_NULL;
    for (index = 1u; index <= 4u; ++index) {
        make_image(cnrom, 2u, (lib_u8)index);
        cnrom[6] = 0x30u;
        assert(core_cartridge_create(&cartridge, cnrom,
            16u + 32768u + index * 8192u) == LIB_STATUS_OK);
        core_cartridge_destroy(cartridge);
        cartridge = LIB_NULL;
    }
    make_image(maximum_mmc3, 2u, 0u);
    maximum_mmc3[6] = 0x40u;
    maximum_mmc3[8] = 1u;
    assert(core_cartridge_create(&cartridge, maximum_mmc3,
        16u + 2u * 16384u) == LIB_STATUS_OK);
    core_cartridge_destroy(cartridge);
    cartridge = LIB_NULL;
    maximum_mmc3[8] = 2u;
    assert(core_cartridge_create(&cartridge, maximum_mmc3,
        16u + 2u * 16384u) == LIB_STATUS_INVALID_ARGUMENT);
    maximum_mmc3[8] = 0u;
    maximum_mmc3[6] = 0x42u;
    assert(core_cartridge_create(&cartridge, maximum_mmc3,
        16u + 2u * 16384u) == LIB_STATUS_INVALID_ARGUMENT);
    maximum_mmc3[8] = 1u;
    assert(core_cartridge_create(&cartridge, maximum_mmc3,
        16u + 2u * 16384u) == LIB_STATUS_OK);
    core_cartridge_destroy(cartridge);
    cartridge = LIB_NULL;
    make_image(maximum_mmc3, 2u, 32u);
    maximum_mmc3[6] = 0x40u;
    assert(core_cartridge_create(&cartridge, maximum_mmc3,
        16u + 2u * 16384u + 32u * 8192u) == LIB_STATUS_OK);
    core_cartridge_destroy(cartridge);
    cartridge = LIB_NULL;
    make_image(maximum_mmc3, 32u, 0u);
    maximum_mmc3[6] = 0x40u;
    assert(core_cartridge_create(&cartridge, maximum_mmc3,
        16u + 32u * 16384u) == LIB_STATUS_OK);
    core_cartridge_destroy(cartridge);
    cartridge = LIB_NULL;
    make_image(maximum_mmc3, 32u, 32u);
    maximum_mmc3[6] = 0x40u;
    assert(core_cartridge_create(&cartridge, maximum_mmc3,
        sizeof(maximum_mmc3)) == LIB_STATUS_OK);
    core_cartridge_destroy(cartridge);
    cartridge = LIB_NULL;
    make_image(suffix, 2u, 0u);
    suffix_size = sizeof(suffix);
    assert(core_cartridge_normalize_ines_size(suffix, &suffix_size));
    assert(suffix_size == 16u + 32768u);
    suffix_size = 16u + 32768u + 127u;
    assert(core_cartridge_normalize_ines_size(suffix, &suffix_size));
    assert(suffix_size == 16u + 32768u);
    suffix_size = 16u + 32768u + 126u;
    assert(!core_cartridge_normalize_ines_size(suffix, &suffix_size));
    suffix_size = 16u + 32768u;
    assert(core_cartridge_normalize_ines_size(suffix, &suffix_size));
    assert(core_cartridge_maximum_image_bytes() == 786576u);
    check_mmc1();
    return 0;
}
