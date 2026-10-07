#include "core/cartridge.h"
#include "lib/types/test.h"

static lib_size reference_prg(const core_cartridge *c, lib_u16 address)
{
    lib_size banks = c->prg_bytes / 8192u;
    lib_size bank;
    if (address < 0xa000u) bank = (c->mmc3_bank_select & 0x40u) ?
        banks - 2u : c->mmc3_bank_data[6] % banks;
    else if (address < 0xc000u) bank = c->mmc3_bank_data[7] % banks;
    else if (address < 0xe000u) bank = (c->mmc3_bank_select & 0x40u) ?
        c->mmc3_bank_data[6] % banks : banks - 2u;
    else bank = banks - 1u;
    return bank * 8192u + (address & 0x1fffu);
}

static lib_size reference_chr(const core_cartridge *c, lib_u16 address)
{
    lib_u8 index;
    lib_u8 bank;
    if ((c->mmc3_bank_select & 0x80u) == 0u) {
        if (address < 0x0800u) index = 0u;
        else if (address < 0x1000u) index = 1u;
        else index = (lib_u8)(2u + ((address - 0x1000u) >> 10u));
    } else {
        if (address < 0x1000u) index = (lib_u8)(2u + (address >> 10u));
        else if (address < 0x1800u) index = 0u;
        else index = 1u;
    }
    bank = c->mmc3_bank_data[index];
    if (index < 2u) bank = (lib_u8)((bank & 0xfeu) + ((address >> 10u) & 1u));
    return ((lib_size)bank % (c->chr_bytes / 1024u)) * 1024u + (address & 1023u);
}

static void compare_mapping(core_cartridge *c)
{
    for (lib_u32 slot = 0u; slot < 4u; ++slot)
        for (lib_u32 offset = 0u; offset < 8192u; offset += 8191u) {
            lib_u16 address = (lib_u16)(0x8000u + slot * 8192u + offset);
            lib_test_assert(c->mmc3_prg_offsets[slot] == reference_prg(c, address) - offset);
            lib_test_assert(core_cartridge_cpu_read(c, address) == c->prg[reference_prg(c, address)]);
        }
    for (lib_u32 slot = 0u; slot < 8u; ++slot)
        for (lib_u32 offset = 0u; offset < 1024u; offset += 1023u) {
            lib_u16 address = (lib_u16)(slot * 1024u + offset);
            lib_test_assert(c->mmc3_chr_offsets[slot] == reference_chr(c, address) - offset);
            lib_test_assert(core_cartridge_ppu_read(c, address) == c->chr[reference_chr(c, address)]);
        }
}

static void verify_capacity(lib_u8 prg_banks, lib_u8 chr_banks)
{
    lib_size size = 16u + (lib_size)prg_banks * 16384u + (lib_size)chr_banks * 8192u;
    lib_u8 *rom = lib_allocate_zero(1u, size);
    core_cartridge *c = LIB_NULL;
    lib_test_assert(rom != LIB_NULL);
    rom[0] = 'N'; rom[1] = 'E'; rom[2] = 'S'; rom[3] = 0x1au;
    rom[4] = prg_banks; rom[5] = chr_banks; rom[6] = 0x40u;
    for (lib_size index = 16u; index < size; ++index)
        rom[index] = (lib_u8)((index >> 10u) ^ index ^ (index >> 8u));
    lib_test_assert(core_cartridge_create(&c, rom, size) == LIB_STATUS_OK);
    lib_release(rom);
    if (c->chr_ram) for (lib_size index = 0u; index < c->chr_bytes; ++index)
        c->chr[index] = (lib_u8)((index >> 10u) ^ index);
    compare_mapping(c);
    /* PRG and CHR capacity axes are independent. Cover each accepted size,
     * all register values and mode combinations, not 1023 duplicate images. */
    for (lib_u32 mode = 0u; mode < 4u; ++mode)
        for (lib_u32 index = 0u; index < 8u; ++index)
            for (lib_u32 value = 0u; value < 256u; ++value) {
                lib_test_assert(core_cartridge_cpu_write(c, 0x8000u, (lib_u8)(mode * 64u + index)));
                lib_test_assert(core_cartridge_cpu_write(c, 0x8001u, (lib_u8)value));
                compare_mapping(c);
                if (c->chr_ram) {
                    lib_size offset = reference_chr(c, 0x1403u);
                    lib_test_assert(core_cartridge_ppu_write(c, 0x1403u, (lib_u8)value));
                    lib_test_assert(c->chr[offset] == (lib_u8)value);
                }
            }
    for (lib_u32 select = 0u; select < 256u; ++select) {
        lib_test_assert(core_cartridge_cpu_write(c, 0x8000u, (lib_u8)select));
        compare_mapping(c);
    }
    core_cartridge_destroy(c);
}

static void write_serial(core_cartridge *c, lib_u16 address, lib_u8 value)
{
    for (lib_u8 bit = 0u; bit < 5u; ++bit)
        lib_test_assert(core_cartridge_cpu_write(c, address, (value >> bit) & 1u));
}

static void verify_fixed_profiles(void)
{
    static lib_u8 rom[16u + 8u * 16384u];
    core_cartridge *c = LIB_NULL;
    lib_memory_set(rom, 0, sizeof(rom));
    rom[0] = 'N'; rom[1] = 'E'; rom[2] = 'S'; rom[3] = 0x1au;
    rom[4] = 2u; rom[5] = 4u; rom[6] = 0x10u;
    for (lib_size index = 0u; index < 32768u; ++index) {
        rom[16u + index] = (lib_u8)(index >> 14u);
        rom[16u + 32768u + index] = (lib_u8)(index >> 12u);
    }
    lib_test_assert(core_cartridge_create(&c, rom, 16u + 65536u) == LIB_STATUS_OK);
    for (lib_u8 control = 0u; control < 32u; ++control)
        for (lib_u8 bank = 0u; bank < 32u; ++bank) {
            write_serial(c, 0x8000u, control);
            write_serial(c, 0xe000u, bank);
            write_serial(c, 0xa000u, bank);
            write_serial(c, 0xc000u, (lib_u8)(31u - bank));
            for (lib_u32 slot = 0u; slot < 2u; ++slot) {
                lib_u32 mode = control & 12u;
                lib_u32 expected = mode <= 4u ? slot : mode == 8u ?
                    (slot == 0u ? 0u : (bank & 15u) % 2u) :
                    (slot == 0u ? (bank & 15u) % 2u : 1u);
                lib_u32 page = (control & 16u) == 0u ?
                    ((bank & 30u) % 8u) + slot :
                    (slot == 0u ? bank : 31u - bank) % 8u;
                lib_test_assert(core_cartridge_cpu_read(c, (lib_u16)(0x8000u + slot * 16384u)) == expected);
                lib_test_assert(core_cartridge_ppu_read(c, (lib_u16)(slot * 4096u)) == page);
            }
        }
    core_cartridge_destroy(c);
    rom[4] = 8u; rom[5] = 0u; rom[6] = 0x20u;
    for (lib_size index = 0u; index < 8u * 16384u; ++index)
        rom[16u + index] = (lib_u8)(index >> 14u);
    lib_test_assert(core_cartridge_create(&c, rom, sizeof(rom)) == LIB_STATUS_OK);
    /* Legacy snapshots may contain raw bank bytes; wrapping must remain
     * equivalent even outside values produced by the normalized write path. */
    for (lib_u32 bank = 0u; bank < 256u; ++bank) {
        c->uxrom_prg_bank = (lib_u8)bank;
        lib_test_assert(core_cartridge_cpu_read(c, 0x8000u) == bank % 8u);
        lib_test_assert(core_cartridge_cpu_read(c, 0xc000u) == 7u);
    }
    core_cartridge_destroy(c);
}

int main(void)
{
    verify_fixed_profiles();
    for (lib_u8 banks = 2u; banks <= 32u; ++banks) verify_capacity(banks, 3u);
    for (lib_u8 banks = 0u; banks <= 32u; ++banks) verify_capacity(3u, banks);
    return 0;
}
