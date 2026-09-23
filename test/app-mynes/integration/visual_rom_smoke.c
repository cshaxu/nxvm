#include <assert.h>
#include <string.h>

#include "core/driver.h"
#include "core/machine.h"
#include "lib/storage/file_interface.h"

#define FIXTURE_PATH "mynes-visual-rom-fixture.nes"
#define COLOR_IDLE 0x64b0ffu
#define COLOR_A 0x5ce430u

static void write_fixture(void)
{
    lib_u8 bytes[16u + 16384u];
    lib_storage_file_writer *writer = LIB_NULL;
    /* This NROM owns no external bytes.  It writes a solid tile into CHR-RAM,
     * enables the background and repeatedly changes palette entry $3f01 from
     * the serial controller-A bit.  The zeroed nametable therefore fills the
     * visible background with tile zero. */
    static const lib_u8 program[] = {
        0x78u,
        0xa9u, 0u, 0x8du, 0u, 0x20u,
        0xa9u, 0x3fu, 0x8du, 6u, 0x20u,
        0xa9u, 0u, 0x8du, 6u, 0x20u,
        0xa9u, 0x0fu, 0x8du, 7u, 0x20u,
        0xa9u, 0x21u, 0x8du, 7u, 0x20u,
        0xa9u, 0u, 0x8du, 6u, 0x20u,
        0xa9u, 0u, 0x8du, 6u, 0x20u,
        0xa9u, 0xffu, 0x8du, 7u, 0x20u,
        0xa9u, 0xffu, 0x8du, 7u, 0x20u,
        0xa9u, 0xffu, 0x8du, 7u, 0x20u,
        0xa9u, 0xffu, 0x8du, 7u, 0x20u,
        0xa9u, 0xffu, 0x8du, 7u, 0x20u,
        0xa9u, 0xffu, 0x8du, 7u, 0x20u,
        0xa9u, 0xffu, 0x8du, 7u, 0x20u,
        0xa9u, 0xffu, 0x8du, 7u, 0x20u,
        0xa9u, 0u, 0x8du, 7u, 0x20u,
        0xa9u, 0u, 0x8du, 7u, 0x20u,
        0xa9u, 0u, 0x8du, 7u, 0x20u,
        0xa9u, 0u, 0x8du, 7u, 0x20u,
        0xa9u, 0u, 0x8du, 7u, 0x20u,
        0xa9u, 0u, 0x8du, 7u, 0x20u,
        0xa9u, 0u, 0x8du, 7u, 0x20u,
        0xa9u, 0u, 0x8du, 7u, 0x20u,
        0xa9u, 0x0au, 0x8du, 1u, 0x20u,
        /* loop at $8079: latch pad, retain A in zero page, write $3f01. */
        0xa9u, 1u, 0x8du, 0x16u, 0x40u,
        0xa9u, 0u, 0x8du, 0x16u, 0x40u,
        0xadu, 0x16u, 0x40u, 0x29u, 1u, 0xf0u, 4u,
        0xa9u, 0x2au, 0xd0u, 2u, 0xa9u, 0x21u,
        0x85u, 0u,
        0xa9u, 0x3fu, 0x8du, 6u, 0x20u,
        0xa9u, 1u, 0x8du, 6u, 0x20u,
        0xa5u, 0u, 0x8du, 7u, 0x20u,
        0x4cu, 0x79u, 0x80u
    };

    memset(bytes, 0, sizeof(bytes));
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u;
    memcpy(bytes + 16u, program, sizeof(program));
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

static lib_u32 pixel_rgb(const common_machine_frame *frame, lib_u32 pixel)
{
    return frame->window.image.palette[frame->window.image.pixels[pixel]];
}

static void prove_window(void)
{
    core_driver *driver = LIB_NULL;
    common_machine_frame frame = { 0 };
    kvm_input_event input = { .type = KVM_EVENT_KEY, .source_identity = 1u };

    assert(core_driver_create(&driver, &(core_driver_options) { 0 }) == LIB_STATUS_OK);
    assert(core_driver_set_media(driver, FIXTURE_PATH, LIB_STORAGE_MEDIUM_READONLY));
    run_frame(driver);
    assert(!driver->machine->trap.trap_valid);
    assert(driver->machine->ppu.palette[1] == 0x21u);
    assert(driver->machine->cartridge->chr[0] == 0xffu);
    assert(driver->machine->ppu.mask == 0x0au);
    assert(core_driver_copy_frame(driver, &frame) == LIB_STATUS_OK);
    assert(frame.window.valid && frame.window.graphics);
    assert(frame.window.image.width == 256u && frame.window.image.height == 240u);
    assert(pixel_rgb(&frame, 9u) == COLOR_IDLE);
    input.data.key.key = 'K'; input.data.key.pressed = 1u;
    core_driver_deliver_input(driver, &input);
    run_frame(driver);
    assert(core_driver_copy_frame(driver, &frame) == LIB_STATUS_OK);
    assert(pixel_rgb(&frame, 9u) == COLOR_A);
    assert(core_driver_destroy(driver) == LIB_STATUS_OK);
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
    assert(core_driver_copy_frame(driver, &frame) == LIB_STATUS_OK);
    assert(frame.window.valid && !frame.window.graphics);
    assert(frame.window.text.base.text_columns == 80u && frame.window.text.base.text_rows == 25u);
    assert(frame.window.text.base.cells[0].glyph_index == ' ');
    idle = frame.window.text.base.cells[7];
    input.data.key.key = 'K'; input.data.key.pressed = 1u;
    core_driver_deliver_input(driver, &input);
    run_frame(driver);
    assert(core_driver_copy_frame(driver, &frame) == LIB_STATUS_OK);
    assert(frame.window.text.base.cells[7].glyph_index != idle.glyph_index ||
        frame.window.text.base.cells[7].foreground != idle.foreground);
    assert(core_driver_destroy(driver) == LIB_STATUS_OK);
}

int main(void)
{
    write_fixture();
    prove_window();
    prove_console();
    return 0;
}
