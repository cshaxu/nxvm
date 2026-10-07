#include <assert.h>
#include <stddef.h>

#include "core/machine.h"
#include "core/snapshot_interface.h"

typedef struct snapshot_bytes {
    lib_u8 bytes[262144];
    lib_size size, cursor;
} snapshot_bytes;

static lib_status put(void *opaque, const lib_u8 *bytes, lib_size count)
{
    snapshot_bytes *s = opaque;
    if (count > sizeof(s->bytes) - s->size) return LIB_STATUS_LIMIT_EXCEEDED;
    lib_memory_copy(s->bytes + s->size, bytes, count);
    s->size += count;
    return LIB_STATUS_OK;
}

static lib_status get(void *opaque, lib_u8 *bytes, lib_size count)
{
    snapshot_bytes *s = opaque;
    if (count > s->size - s->cursor) return LIB_STATUS_IO_ERROR;
    lib_memory_copy(bytes, s->bytes + s->cursor, count);
    s->cursor += count;
    return LIB_STATUS_OK;
}

static void capture(core_machine *machine, snapshot_bytes *snapshot)
{
    snapshot->size = snapshot->cursor = 0u;
    assert(core_snapshot_write(machine, put, snapshot) == LIB_STATUS_OK);
}

static void fixture(lib_u8 *rom)
{
    lib_memory_set(rom, 0, 16400u);
    rom[0] = 'N'; rom[1] = 'E'; rom[2] = 'S'; rom[3] = 0x1au;
    rom[4] = 1u;
    rom[16] = 0xa2u; rom[17] = 0u; rom[18] = 0xe8u;
    rom[19] = 0x4cu; rom[20] = 2u; rom[21] = 0x80u;
    rom[16396] = 0u; rom[16397] = 0x80u;
}

static void reject_unchanged(core_machine *machine, snapshot_bytes *bad)
{
    static snapshot_bytes before, after;
    capture(machine, &before);
    bad->cursor = 0u;
    assert(core_snapshot_read(machine, get, bad) != LIB_STATUS_OK);
    capture(machine, &after);
    assert(before.size == after.size);
    assert(lib_memory_compare(before.bytes, after.bytes, before.size) == 0);
}

static void invalid_fields(core_machine *source, core_machine *target)
{
    static snapshot_bytes bad;
    static const struct { lib_size offset; lib_u8 value; } cases[] = {
        {offsetof(core_machine, ppu.next_secondary_oam_count), 32u},
        {offsetof(core_machine, ppu.secondary_oam_count), 9u},
        {offsetof(core_machine, ppu.next_sprite_count), 9u},
        {offsetof(core_machine, ppu.selected_sprite_count), 9u},
        {offsetof(core_machine, ppu.next_secondary_oam_count), 1u},
        {offsetof(core_machine, ppu.fine_x), 255u},
        {offsetof(core_machine, ppu.sprite_evaluation_byte), 4u},
        {offsetof(core_machine, apu.pulse_phase[0]), 8u},
        {offsetof(core_machine, apu.triangle_phase), 32u},
        {offsetof(core_machine, apu.dmc_bits), 9u},
        {offsetof(core_machine, apu.frame_mode), 2u},
        {offsetof(core_machine, apu.dmc_output), 128u},
        {offsetof(core_machine, controller.index), 9u},
        {offsetof(core_machine, dma_phase), 4u},
        {offsetof(core_machine, dmc_dma_phase), 5u},
        {offsetof(core_machine, interrupt_phase), 3u}
    };
    lib_size index;
    for (index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        lib_u8 *field = (lib_u8 *)source + cases[index].offset;
        lib_u8 saved = *field;
        *field = cases[index].value;
        capture(source, &bad);
        *field = saved;
        reject_unchanged(target, &bad);
    }
    source->cartridge->mmc1_shift_count = 32u;
    capture(source, &bad);
    source->cartridge->mmc1_shift_count = 0u;
    reject_unchanged(target, &bad);
    source->cartridge->chr_ram = LIB_FALSE;
    capture(source, &bad);
    source->cartridge->chr_ram = LIB_TRUE;
    reject_unchanged(target, &bad);
}

static void mapper_restore(void)
{
    static lib_u8 rom[16u + 3u * 16384u + 3u * 8192u];
    static snapshot_bytes saved, before, after;
    core_machine *source = LIB_NULL, *target = LIB_NULL;
    lib_memory_set(rom, 0, sizeof(rom));
    rom[0] = 'N'; rom[1] = 'E'; rom[2] = 'S'; rom[3] = 0x1au;
    rom[4] = 3u; rom[5] = 3u; rom[6] = 0x40u;
    for (lib_size index = 16u; index < sizeof(rom); ++index)
        rom[index] = (lib_u8)(index >> 10u);
    for (lib_u8 variant = 0u; variant < 2u; ++variant) {
        lib_size size = 16u + 3u * 16384u + (variant == 0u ? 3u * 8192u : 0u);
        rom[5] = variant == 0u ? 3u : 0u;
        assert(core_machine_create(&source, rom, size, &(core_machine_options){0}) == LIB_STATUS_OK);
        assert(core_machine_create(&target, rom, size, &(core_machine_options){0}) == LIB_STATUS_OK);
        if (source->cartridge->chr_ram)
            for (lib_size index = 0u; index < source->cartridge->chr_bytes; ++index)
                source->cartridge->chr[index] = (lib_u8)(index >> 10u);
        for (lib_u8 index = 0u; index < 8u; ++index) {
            assert(core_cartridge_cpu_write(source->cartridge, 0x8000u, (lib_u8)(0xc0u | index)));
            assert(core_cartridge_cpu_write(source->cartridge, 0x8001u, (lib_u8)(255u - index)));
        }
        capture(source, &saved);
        assert(core_snapshot_read(target, get, &saved) == LIB_STATUS_OK);
        for (lib_u32 address = 0x8000u; address <= 0xffffu; ++address)
            assert(core_cartridge_cpu_read(source->cartridge, (lib_u16)address) ==
                core_cartridge_cpu_read(target->cartridge, (lib_u16)address));
        for (lib_u16 address = 0u; address < 0x2000u; ++address)
            assert(core_cartridge_ppu_read(source->cartridge, address) ==
                core_cartridge_ppu_read(target->cartridge, address));
        capture(target, &after);
        assert(saved.size == after.size && lib_memory_compare(saved.bytes, after.bytes, saved.size) == 0);
        assert(core_machine_reset(target, CORE_RESET_WARM) == LIB_STATUS_OK);
        for (lib_u32 address = 0x8000u; address <= 0xffffu; ++address)
            assert(core_cartridge_cpu_read(source->cartridge, (lib_u16)address) ==
                core_cartridge_cpu_read(target->cartridge, (lib_u16)address));
        core_machine_destroy(target); core_machine_destroy(source);
    }

    /* Every accepted CNROM capacity, including three banks. Unsafe restored
     * bank bytes must reject before touching the live machine. */
    for (lib_u8 banks = 1u; banks <= 4u; ++banks) {
        static lib_u8 cnrom[16u + 32768u + 4u * 8192u];
        lib_size size = 16u + 32768u + (lib_size)banks * 8192u;
        lib_memory_set(cnrom, 0xff, size);
        lib_memory_set(cnrom, 0, 16u);
        cnrom[0] = 'N'; cnrom[1] = 'E'; cnrom[2] = 'S'; cnrom[3] = 0x1au;
        cnrom[4] = 2u; cnrom[5] = banks; cnrom[6] = 0x30u;
        assert(core_machine_create(&source, cnrom, size, &(core_machine_options){0}) == LIB_STATUS_OK);
        assert(core_machine_create(&target, cnrom, size, &(core_machine_options){0}) == LIB_STATUS_OK);
        assert(core_cartridge_cpu_write(source->cartridge, 0x8000u, banks - 1u));
        capture(source, &saved);
        assert(core_snapshot_read(target, get, &saved) == LIB_STATUS_OK);
        capture(target, &before);
        for (lib_u32 value = banks; value <= 255u; ++value) {
            source->cartridge->cnrom_chr_bank = (lib_u8)value;
            capture(source, &saved);
            reject_unchanged(target, &saved);
        }
        capture(target, &after);
        assert(before.size == after.size && lib_memory_compare(before.bytes, after.bytes, before.size) == 0);
        core_machine_destroy(target); core_machine_destroy(source);
    }
}

int main(void)
{
    lib_u8 rom[16400], other_rom[16400];
    core_machine *machine = LIB_NULL, *other = LIB_NULL;
    static snapshot_bytes saved, continued, replayed;
    core_run_result run;
    fixture(rom);
    assert(core_machine_create(&machine, rom, sizeof(rom),
        &(core_machine_options){0}) == LIB_STATUS_OK);
    assert(core_machine_create(&other, rom, sizeof(rom),
        &(core_machine_options){0}) == LIB_STATUS_OK);

    invalid_fields(machine, other);
    assert(core_machine_run(machine, 10u, 100u, &run) == LIB_STATUS_OK);
    capture(machine, &saved);
    assert(saved.size == 136007u && saved.bytes[4] == 3u &&
        saved.bytes[5] == 0u && saved.bytes[6] == 0u && saved.bytes[7] == 0u);
    saved.bytes[0] = 'X';
    reject_unchanged(other, &saved);
    saved.bytes[0] = 'M';
    --saved.size;
    reject_unchanged(other, &saved);
    ++saved.size;
    saved.bytes[4] = 2u;
    reject_unchanged(other, &saved);
    saved.bytes[4] = 3u;
    saved.bytes[saved.size - 8193u] = 2u;
    reject_unchanged(other, &saved);
    saved.bytes[saved.size - 8193u] = 1u;

    /* Resume within overflow evaluation with distinct destination history.
     * The evaluation byte must come from the image, not the destination. */
    machine->ppu.mask = 0x18u;
    machine->ppu.dot = 90u;
    machine->ppu.scanline = 1u;
    machine->ppu.next_sprite_count = 8u;
    machine->ppu.next_secondary_oam_count = 8u;
    machine->ppu.sprite_evaluation_index = 9u;
    machine->ppu.sprite_evaluation_byte = 3u;
    capture(machine, &saved);
    other->ppu.sprite_evaluation_byte = 1u;
    assert(core_snapshot_read(other, get, &saved) == LIB_STATUS_OK);
    assert(other->ppu.sprite_evaluation_byte == 3u);
    assert(core_machine_run(machine, 200u, 2000u, &run) == LIB_STATUS_OK);
    assert(core_machine_run(other, 200u, 2000u, &run) == LIB_STATUS_OK);
    capture(machine, &continued);
    capture(other, &replayed);
    assert(continued.size == replayed.size);
    assert(lib_memory_compare(continued.bytes, replayed.bytes, continued.size) == 0);

    core_machine_destroy(other);
    fixture(other_rom);
    other_rom[17] = 0xeau;
    assert(core_machine_create(&other, other_rom, sizeof(other_rom),
        &(core_machine_options){0}) == LIB_STATUS_OK);
    reject_unchanged(other, &saved);
    core_machine_destroy(other);
    core_machine_destroy(machine);
    mapper_restore();
    return 0;
}
