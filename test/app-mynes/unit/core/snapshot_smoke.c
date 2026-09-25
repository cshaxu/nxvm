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
    return 0;
}
