#include <assert.h>

#include "core/driver.h"
#include "core/machine.h"
#include "lib/storage/file_interface.h"

#define FIXTURE_PATH "mynes-ppu-timing-rom-fixture.nes"
#define LIGHT_GREEN 0x64b0ffu
#define BLACK 0x000000u

static void emit(lib_u8 *program, lib_u32 *size, lib_u8 value)
{
    program[(*size)++] = value;
}

static void load_store(lib_u8 *program, lib_u32 *size, lib_u8 value, lib_u16 address)
{
    emit(program, size, 0xa9u); emit(program, size, value);
    emit(program, size, 0x8du); emit(program, size, (lib_u8)address);
    emit(program, size, (lib_u8)(address >> 8u));
}

static void ppu_address(lib_u8 *program, lib_u32 *size, lib_u16 address)
{
    load_store(program, size, (lib_u8)(address >> 8u), 0x2006u);
    load_store(program, size, (lib_u8)address, 0x2006u);
}

/* Produce a minimal, self-owned NROM-128 image. Tile zero is 10110010 on
 * every row; tile one is its inverse. The first two nametable entries are
 * zero and one, so fine X exercises both the left edge and a tile crossing. */
static void write_fixture(lib_u8 fine_x)
{
    lib_u8 bytes[16u + 16384u];
    lib_u8 program[512];
    lib_storage_file_writer *writer = LIB_NULL;
    lib_u32 size = 0u;
    lib_u32 index;
    lib_u32 loop;

    emit(program, &size, 0x78u);
    load_store(program, &size, 0u, 0x2000u);
    ppu_address(program, &size, 0x3f00u);
    load_store(program, &size, 0x0fu, 0x2007u);
    load_store(program, &size, 0x21u, 0x2007u);
    ppu_address(program, &size, 0x0000u);
    for (index = 0u; index < 8u; ++index) load_store(program, &size, 0xb2u, 0x2007u);
    for (index = 0u; index < 8u; ++index) load_store(program, &size, 0u, 0x2007u);
    ppu_address(program, &size, 0x0010u);
    for (index = 0u; index < 8u; ++index) load_store(program, &size, 0x4du, 0x2007u);
    for (index = 0u; index < 8u; ++index) load_store(program, &size, 0u, 0x2007u);
    ppu_address(program, &size, 0x2000u);
    load_store(program, &size, 0u, 0x2007u);
    load_store(program, &size, 1u, 0x2007u);
    load_store(program, &size, fine_x, 0x2005u);
    load_store(program, &size, 0u, 0x2005u);
    load_store(program, &size, 0x0au, 0x2001u);
    loop = size;
    emit(program, &size, 0x4cu); emit(program, &size, (lib_u8)loop);
    emit(program, &size, (lib_u8)(0x80u + (loop >> 8u)));

    lib_memory_set(bytes, 0, sizeof(bytes));
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u;
    lib_memory_copy(bytes + 16u, program, size);
    bytes[16u + 0x3ffcu] = 0u; bytes[16u + 0x3ffdu] = 0x80u;
    assert(lib_storage_file_writer_open(FIXTURE_PATH, LIB_STORAGE_FILE_WRITER_TRUNCATE,
        &writer) == LIB_STATUS_OK);
    assert(lib_storage_file_writer_write(writer, bytes, sizeof(bytes)) == LIB_STATUS_OK);
    assert(lib_storage_file_writer_close(writer) == LIB_STATUS_OK);
}

static void run_fresh_frames(core_driver *driver, lib_u32 count)
{
    lib_u32 revision = driver->machine->ppu.frame_revision + count;
    lib_u32 limit;
    for (limit = 0u; driver->machine->ppu.frame_revision < revision && limit < 256u; ++limit)
        assert(core_driver_run(driver));
    assert(driver->machine->ppu.frame_revision >= revision);
}

static lib_u32 rgb_at(const common_machine_frame *frame, lib_u32 x, lib_u32 y)
{
    return frame->window.image.palette[frame->window.image.pixels[y * 256u + x]];
}

static lib_u8 expected_bit(lib_u8 fine_x, lib_u32 x)
{
    static const lib_u8 patterns[] = { 0xb2u, 0x4du, 0xb2u, 0xb2u };
    lib_u32 source = x + fine_x;
    lib_u8 pattern = patterns[source >> 3u];
    return (lib_u8)((pattern >> (7u - (source & 7u))) & 1u);
}

static void prove_fine_x(lib_u8 fine_x)
{
    core_driver *driver = LIB_NULL;
    common_machine_frame frame = { 0 };
    lib_u32 x;

    write_fixture(fine_x);
    assert(core_driver_create(&driver, &(core_driver_options) { 0 }) == LIB_STATUS_OK);
    assert(core_driver_set_media(driver, FIXTURE_PATH, LIB_STORAGE_MEDIUM_READONLY));
    run_fresh_frames(driver, 3u);
    assert(core_driver_copy_frame(driver, &frame) == LIB_STATUS_OK && frame.window.graphics);
    for (x = 0u; x < 24u; ++x)
        assert(rgb_at(&frame, x, 1u) ==
            (expected_bit(fine_x, x) ? LIGHT_GREEN : BLACK));
    assert(core_driver_destroy(driver) == LIB_STATUS_OK);
}

int main(void)
{
    lib_u8 fine_x;
    for (fine_x = 0u; fine_x < 8u; ++fine_x) prove_fine_x(fine_x);
    return 0;
}
