#include <assert.h>

#include "core/driver.h"
#include "core/machine.h"
#include "lib/storage/file_interface.h"

#define FIXTURE_PATH "mynes-motion-rom-fixture.nes"

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

static void write_fixture(void)
{
    lib_u8 bytes[16u + 16384u];
    lib_u8 program[1024];
    lib_storage_file_writer *writer = LIB_NULL;
    lib_u32 size = 0u;
    lib_u32 loop;
    lib_u32 branch;
    lib_u32 idle;
    lib_u32 index;

    /* The program owns every byte: tile zero is palette color one, tile one
     * is palette color two, and sprite tile one uses sprite palette color one.
     * A changes scroll X from zero to eight and moves the sprite from X=0 to 16. */
    emit(program, &size, 0x78u);
    load_store(program, &size, 0u, 0x2000u);
    ppu_address(program, &size, 0x3f00u);
    load_store(program, &size, 0x0fu, 0x2007u);
    load_store(program, &size, 0x21u, 0x2007u);
    load_store(program, &size, 0x22u, 0x2007u);
    load_store(program, &size, 0x23u, 0x2007u);
    ppu_address(program, &size, 0x3f11u);
    load_store(program, &size, 0x19u, 0x2007u);
    ppu_address(program, &size, 0x0000u);
    for (index = 0u; index < 8u; ++index) load_store(program, &size, 0xffu, 0x2007u);
    for (index = 0u; index < 8u; ++index) load_store(program, &size, 0u, 0x2007u);
    ppu_address(program, &size, 0x0010u);
    for (index = 0u; index < 8u; ++index) load_store(program, &size, 0u, 0x2007u);
    for (index = 0u; index < 8u; ++index) load_store(program, &size, 0xffu, 0x2007u);
    ppu_address(program, &size, 0x2000u);
    load_store(program, &size, 0u, 0x2007u);
    load_store(program, &size, 1u, 0x2007u);
    load_store(program, &size, 0u, 0x2003u);
    load_store(program, &size, 0u, 0x2004u);
    load_store(program, &size, 0u, 0x2004u);
    for (index = 1u; index < 64u; ++index) {
        load_store(program, &size, (lib_u8)(index * 4u), 0x2003u);
        load_store(program, &size, 20u, 0x2004u);
    }
    load_store(program, &size, 0u, 0x2004u);
    load_store(program, &size, 0u, 0x2004u);
    load_store(program, &size, 0x1eu, 0x2001u);
    loop = size;
    load_store(program, &size, 1u, 0x4016u);
    load_store(program, &size, 0u, 0x4016u);
    emit(program, &size, 0xadu); emit(program, &size, 0x16u); emit(program, &size, 0x40u);
    emit(program, &size, 0x29u); emit(program, &size, 1u);
    branch = size; emit(program, &size, 0xf0u); emit(program, &size, 0u);
    load_store(program, &size, 8u, 0x2005u);
    load_store(program, &size, 0u, 0x2005u);
    load_store(program, &size, 3u, 0x2003u);
    load_store(program, &size, 16u, 0x2004u);
    emit(program, &size, 0x4cu); emit(program, &size, (lib_u8)loop);
    emit(program, &size, (lib_u8)(0x80u + (loop >> 8u)));
    idle = size;
    load_store(program, &size, 0u, 0x2005u);
    load_store(program, &size, 0u, 0x2005u);
    load_store(program, &size, 3u, 0x2003u);
    load_store(program, &size, 0u, 0x2004u);
    emit(program, &size, 0x4cu); emit(program, &size, (lib_u8)loop);
    emit(program, &size, (lib_u8)(0x80u + (loop >> 8u)));
    program[branch + 1u] = (lib_u8)(idle - (branch + 2u));

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

static void run_frame(core_driver *driver)
{
    lib_u32 index;
    for (index = 0u; index < 64u; ++index) assert(core_driver_run(driver));
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

static void prove_console(void)
{
    core_driver *driver = LIB_NULL;
    common_machine_frame frame = { 0 };
    kvm_input_event input = { .type = KVM_EVENT_KEY, .source_identity = 2u };
    kvm_text_cell idle;

    assert(core_driver_create(&driver, &(core_driver_options) { .text_output = LIB_TRUE }) ==
        LIB_STATUS_OK);
    assert(core_driver_set_media(driver, FIXTURE_PATH, LIB_STORAGE_MEDIUM_READONLY));
    run_frame(driver);
    assert(core_driver_copy_frame(driver, &frame) == LIB_STATUS_OK && !frame.window.graphics);
    idle = frame.window.text.base.cells[7u];
    input.data.key.key = 'K'; input.data.key.pressed = 1u;
    core_driver_deliver_input(driver, &input);
    run_fresh_frames(driver, 2u);
    assert(core_driver_copy_frame(driver, &frame) == LIB_STATUS_OK && !frame.window.graphics);
    assert(frame.window.text.base.cells[7u].glyph_index != idle.glyph_index ||
        frame.window.text.base.cells[7u].foreground != idle.foreground);
    assert(core_driver_destroy(driver) == LIB_STATUS_OK);
}

int main(void)
{
    core_driver *driver = LIB_NULL;
    common_machine_frame frame = { 0 };
    kvm_input_event input = { .type = KVM_EVENT_KEY, .source_identity = 1u };

    write_fixture();
    assert(core_driver_create(&driver, &(core_driver_options) { 0 }) == LIB_STATUS_OK);
    assert(core_driver_set_media(driver, FIXTURE_PATH, LIB_STORAGE_MEDIUM_READONLY));
    run_frame(driver);
    run_fresh_frames(driver, 2u);
    assert(core_driver_copy_frame(driver, &frame) == LIB_STATUS_OK && frame.window.graphics);
    assert(rgb_at(&frame, 0u, 1u) == 0x388700u);
    assert(rgb_at(&frame, 8u, 1u) == 0x9290ffu);
    input.data.key.key = 'K'; input.data.key.pressed = 1u;
    core_driver_deliver_input(driver, &input);
    run_fresh_frames(driver, 2u);
    assert(!driver->machine->trap.trap_valid);
    assert(core_driver_copy_frame(driver, &frame) == LIB_STATUS_OK && frame.window.graphics);
    assert(rgb_at(&frame, 0u, 1u) == 0x9290ffu);
    assert(rgb_at(&frame, 16u, 1u) == 0x388700u);
    assert(core_driver_destroy(driver) == LIB_STATUS_OK);
    prove_console();
    return 0;
}
