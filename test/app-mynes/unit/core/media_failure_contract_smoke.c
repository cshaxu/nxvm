#include <assert.h>
#include <string.h>

#include "core/driver.h"
#include "core/machine.h"
#include "core/debug_interface.h"
#include "common/machine/frame_interface.h"
#include "lib/storage/file_interface.h"

static lib_u8 fixture[16u + 131072u];
static lib_size fixture_size;

static void write_fixture(void)
{
    lib_storage_file_writer *writer = LIB_NULL;

    assert(lib_storage_file_writer_open("fixture.nes", LIB_STORAGE_FILE_WRITER_TRUNCATE,
        &writer) == LIB_STATUS_OK);
    assert(lib_storage_file_writer_write(writer, fixture, fixture_size) == LIB_STATUS_OK);
    assert(lib_storage_file_writer_close(writer) == LIB_STATUS_OK);
}

static void make_fixture(void)
{
    fixture_size = 16u + 16384u;
    memset(fixture, 0, fixture_size);
    fixture[0] = 'N'; fixture[1] = 'E'; fixture[2] = 'S'; fixture[3] = 0x1au;
    fixture[4] = 1u;
    fixture[16u + 0x3ffcu] = 0x00u;
    fixture[16u + 0x3ffdu] = 0x80u;
}

static void make_uxrom_fixture(void)
{
    lib_size prg_offset = 16u + 7u * 16384u;

    fixture_size = sizeof(fixture);
    memset(fixture, 0, fixture_size);
    fixture[0] = 'N'; fixture[1] = 'E'; fixture[2] = 'S'; fixture[3] = 0x1au;
    fixture[4] = 8u;
    fixture[6] = 0x20u;
    fixture[prg_offset] = 0x4cu;
    fixture[prg_offset + 1u] = 0x00u;
    fixture[prg_offset + 2u] = 0xc0u;
    fixture[prg_offset + 0x3ffcu] = 0x00u;
    fixture[prg_offset + 0x3ffdu] = 0xc0u;
}

static void make_cnrom_fixture(void)
{
    lib_size prg_offset = 16u;

    fixture_size = 16u + 32768u + 8192u;
    memset(fixture, 0, fixture_size);
    fixture[0] = 'N'; fixture[1] = 'E'; fixture[2] = 'S'; fixture[3] = 0x1au;
    fixture[4] = 2u; fixture[5] = 1u; fixture[6] = 0x30u;
    fixture[prg_offset] = 0x4cu;
    fixture[prg_offset + 1u] = 0x00u;
    fixture[prg_offset + 2u] = 0x80u;
    fixture[prg_offset + 0x7ffcu] = 0x00u;
    fixture[prg_offset + 0x7ffdu] = 0x80u;
}

int main(void)
{
    core_driver *driver = LIB_NULL;
    core_machine *accepted;
    lib_size index;
    static common_machine_frame frame;

    make_fixture();
    write_fixture();
    assert(core_driver_create(&driver, &(core_driver_options) { 0 }) == LIB_STATUS_OK);
    assert(core_driver_set_media(driver, "fixture.nes", LIB_STORAGE_MEDIUM_READONLY));
    accepted = driver->machine;
    assert(accepted != LIB_NULL);
    {
        kvm_input_event input = { .type = KVM_EVENT_KEY };
        input.source_identity = 101u;
        input.data.key.key = 'K'; input.data.key.pressed = 1u;
        core_driver_deliver_input(driver, &input);
        core_controller_write_strobe(&driver->machine->controller, 1u);
        core_controller_write_strobe(&driver->machine->controller, 0u);
        assert(core_controller_read(&driver->machine->controller) == 1u);
        input.data.key.pressed = 0u;
        core_driver_deliver_input(driver, &input);
        /* Both physical modifier sides and scan-less virtual/RDP keys map
         * through KVM's neutral virtual-key contract. */
        input.source_identity = 101u;
        input.data.key.key = KVM_KEY_SHIFT; input.data.key.scan_code = 0x2au;
        input.data.key.flags = 0u; input.data.key.pressed = 1u;
        core_driver_deliver_input(driver, &input);
        input.data.key.scan_code = 0x36u;
        core_driver_deliver_input(driver, &input);
        input.data.key.scan_code = 0x2au; input.data.key.pressed = 0u;
        core_driver_deliver_input(driver, &input);
        assert((driver->machine->controller.live & 0x04u) != 0u);
        input.data.key.scan_code = 0x36u;
        core_driver_deliver_input(driver, &input);
        core_controller_write_strobe(&driver->machine->controller, 1u);
        core_controller_write_strobe(&driver->machine->controller, 0u);
        assert((driver->machine->controller.live & 0x04u) == 0u);
        input.data.key.key = KVM_KEY_CONTROL; input.data.key.scan_code = 0x1du;
        input.data.key.flags = 0u; input.data.key.pressed = 1u;
        core_driver_deliver_input(driver, &input);
        input.data.key.flags = KVM_KEY_FLAG_EXTENDED;
        core_driver_deliver_input(driver, &input);
        input.data.key.flags = 0u; input.data.key.pressed = 0u;
        core_driver_deliver_input(driver, &input);
        assert((driver->machine->controller.live & 0x02u) != 0u);
        input.data.key.flags = KVM_KEY_FLAG_EXTENDED;
        core_driver_deliver_input(driver, &input);
        core_controller_write_strobe(&driver->machine->controller, 1u);
        core_controller_write_strobe(&driver->machine->controller, 0u);
        assert((driver->machine->controller.live & 0x02u) == 0u);
        input.data.key.key = KVM_KEY_ALT; input.data.key.scan_code = 0x38u;
        input.data.key.flags = 0u; input.data.key.pressed = 1u;
        core_driver_deliver_input(driver, &input);
        input.data.key.flags = KVM_KEY_FLAG_EXTENDED;
        core_driver_deliver_input(driver, &input);
        input.data.key.flags = 0u; input.data.key.pressed = 0u;
        core_driver_deliver_input(driver, &input);
        assert((driver->machine->controller.live & 0x01u) != 0u);
        input.data.key.flags = KVM_KEY_FLAG_EXTENDED;
        core_driver_deliver_input(driver, &input);
        core_controller_write_strobe(&driver->machine->controller, 1u);
        core_controller_write_strobe(&driver->machine->controller, 0u);
        assert((driver->machine->controller.live & 0x01u) == 0u);
        input.data.key.key = KVM_KEY_ALT; input.data.key.scan_code = 0u;
        input.data.key.flags = 0u; input.data.key.pressed = 1u;
        core_driver_deliver_input(driver, &input);
        assert((driver->machine->controller.live & 0x01u) != 0u);
        input.data.key.pressed = 0u;
        core_driver_deliver_input(driver, &input);
        core_controller_write_strobe(&driver->machine->controller, 1u);
        core_controller_write_strobe(&driver->machine->controller, 0u);
        assert((driver->machine->controller.live & 0x01u) == 0u);
        input.data.key.key = 'K'; input.data.key.pressed = 1u;
        input.source_identity = 202u;
        core_driver_deliver_input(driver, &input);
        input.source_identity = 101u;
        input.data.key.pressed = 0u;
        core_driver_deliver_input(driver, &input);
        core_controller_write_strobe(&driver->machine->controller, 1u);
        core_controller_write_strobe(&driver->machine->controller, 0u);
        assert(core_controller_read(&driver->machine->controller) == 1u);
        input.type = KVM_EVENT_SOURCE_RETIRED;
        input.source_identity = 202u;
        core_driver_deliver_input(driver, &input);
        core_controller_write_strobe(&driver->machine->controller, 1u);
        core_controller_write_strobe(&driver->machine->controller, 0u);
        assert(core_controller_read(&driver->machine->controller) == 0u);
        {
            static const struct { lib_u32 scalar; lib_u8 button; } text_map[] = {
                { '\r', 0x08u }, { '\n', 0x08u }, { 'w', 0x10u },
                { 's', 0x20u }, { 'a', 0x40u }, { 'd', 0x80u },
                { 'j', 0x02u }, { 'k', 0x01u }
            };
            input = (kvm_input_event) { .type = KVM_EVENT_TEXT };
            for (lib_u32 text_index = 0u;
                text_index < sizeof(text_map) / sizeof(text_map[0]); ++text_index) {
                input.data.text.scalar = text_map[text_index].scalar;
                core_driver_deliver_input(driver, &input);
                assert(driver->machine->controller.live == text_map[text_index].button);
                core_controller_write_strobe(&driver->machine->controller, 1u);
                core_controller_write_strobe(&driver->machine->controller, 0u);
                assert(driver->machine->controller.captured == text_map[text_index].button);
                assert(driver->machine->controller.live == 0u);
                core_controller_write_strobe(&driver->machine->controller, 1u);
                core_controller_write_strobe(&driver->machine->controller, 0u);
                assert(driver->machine->controller.captured == 0u);
            }
        }
        {
            /* A physical make/break may both arrive before the NES next
             * samples $4016. Every controller binding must still be observed
             * once, just as a text-only record is. This includes the normal
             * KVM_KEY_ENTER path used by Win32/RDP. */
            static const struct { kvm_key key; lib_u16 scan; lib_u32 flags; lib_u8 button; }
                physical_map[] = {
                    { 'K', 0x25u, 0u, 0x01u }, { KVM_KEY_ALT, 0x38u, 0u, 0x01u },
                    { 'J', 0x24u, 0u, 0x02u }, { KVM_KEY_CONTROL, 0x1du, 0u, 0x02u },
                    { KVM_KEY_SHIFT, 0x2au, 0u, 0x04u }, { KVM_KEY_ENTER, 0x1cu, 0u, 0x08u },
                    { 'W', 0x11u, 0u, 0x10u }, { 'S', 0x1fu, 0u, 0x20u },
                    { 'A', 0x1eu, 0u, 0x40u }, { 'D', 0x20u, 0u, 0x80u }
                };
            /* The modifier-side coverage above deliberately left short makes
             * pending; model the guest's next ordinary controller latch
             * before testing each independent input. */
            core_controller_write_strobe(&driver->machine->controller, 1u);
            core_controller_write_strobe(&driver->machine->controller, 0u);
            assert(driver->machine->controller.live == 0u);
            input = (kvm_input_event) { .type = KVM_EVENT_KEY, .source_identity = 404u };
            for (lib_u32 key_index = 0u;
                key_index < sizeof(physical_map) / sizeof(physical_map[0]); ++key_index) {
                input.data.key.key = physical_map[key_index].key;
                input.data.key.scan_code = physical_map[key_index].scan;
                input.data.key.flags = physical_map[key_index].flags;
                input.data.key.pressed = 1u;
                core_driver_deliver_input(driver, &input);
                input.data.key.pressed = 0u;
                core_driver_deliver_input(driver, &input);
                assert(driver->machine->controller.live == physical_map[key_index].button);
                core_controller_write_strobe(&driver->machine->controller, 1u);
                core_controller_write_strobe(&driver->machine->controller, 0u);
                assert(driver->machine->controller.captured == physical_map[key_index].button);
                assert(driver->machine->controller.live == 0u);
            }
            input.type = KVM_EVENT_SOURCE_RETIRED;
            core_driver_deliver_input(driver, &input);
        }
        input.type = KVM_EVENT_KEY;
        input.source_identity = 101u;
        input.data.key.key = 'W'; input.data.key.pressed = 1u;
        core_driver_deliver_input(driver, &input);
        input.source_identity = 202u;
        input.data.key.key = 'S'; input.data.key.pressed = 1u;
        core_driver_deliver_input(driver, &input);
        assert(driver->machine->controller.live == 0x30u);
        core_controller_write_strobe(&driver->machine->controller, 1u);
        core_controller_write_strobe(&driver->machine->controller, 0u);
        assert(driver->machine->controller.captured == 0x30u);
        assert(driver->machine->controller.live == 0u);
        input.data.key.pressed = 0u;
        core_driver_deliver_input(driver, &input);
        assert(driver->machine->controller.live == 0x10u);
        input.source_identity = 101u;
        input.data.key.key = 'A'; input.data.key.pressed = 1u;
        core_driver_deliver_input(driver, &input);
        input.source_identity = 202u;
        input.data.key.key = 'D'; input.data.key.pressed = 1u;
        core_driver_deliver_input(driver, &input);
        assert(driver->machine->controller.live == 0xd0u);
        core_controller_write_strobe(&driver->machine->controller, 1u);
        core_controller_write_strobe(&driver->machine->controller, 0u);
        assert(driver->machine->controller.captured == 0xd0u);
        assert(driver->machine->controller.live == 0x10u);
        input.data.key.pressed = 0u;
        core_driver_deliver_input(driver, &input);
        assert(driver->machine->controller.live == 0x50u);
        input.source_identity = 303u;
        input.data.key.key = 'K'; input.data.key.pressed = 1u;
        core_driver_deliver_input(driver, &input);
        assert(driver->machine->controller.live == 0x50u);
        /* A pause/stop sink requests this reset.  The first resumed run must
         * clear held state before guest code observes it, and queued releases
         * cannot recreate a retired held state. */
        input.source_identity = 101u;
        input.data.key.key = 'K'; input.data.key.pressed = 1u;
        core_driver_deliver_input(driver, &input);
        assert(driver->machine->controller.live != 0u);
        core_driver_request_input_reset(driver);
        input.data.key.pressed = 0u;
        core_driver_deliver_input(driver, &input);
        assert(core_driver_run(driver));
        assert(driver->machine->controller.live == 0u);
    }
    for (index = 0u; index < 64u; ++index)
        assert(core_driver_run(driver));
    assert(core_driver_copy_frame(driver, &frame) == LIB_STATUS_OK);
    assert(frame.window.valid == 1u && frame.window.graphics == 1u);
    assert(frame.window.image.width == 256u && frame.window.image.height == 240u &&
        frame.window.image.stride == 256u);
    assert(frame.window.image.pixels[0] == 0u &&
        frame.window.image.pixels[256u * 239u + 255u] == 0u);
    assert(frame.window.image.palette[0] == 0x666666u);
    assert(core_driver_copy_frame(driver, &frame) == LIB_STATUS_OK && frame.window.valid == 0u);
    driver->machine->ppu.completed[0] = 0x1c3u;
    ++driver->machine->ppu.frame_revision;
    assert(core_driver_copy_frame(driver, &frame) == LIB_STATUS_OK && frame.window.valid == 1u);
    /* Palette index 3 with every emphasis bit uses the immutable combined
     * attenuation table: 0x3b00a4 -> 0x21005c. */
    assert(frame.window.image.pixels[0] == 0u && frame.window.image.palette[0] == 0x21005cu);
    {
        lib_u8 request[16] = { CORE_DEBUG_VERSION, 0u, CORE_DEBUG_OUTPUT_SET, 0u, 8u };
        lib_u8 response[16]; lib_size response_size = 0u;
        request[8] = 1u;
        assert(core_driver_debug_execute()(driver, request, sizeof(request), response,
            sizeof(response), &response_size) == LIB_STATUS_OK && response_size == 12u);
        assert(core_driver_copy_frame(driver, &frame) == LIB_STATUS_OK &&
            frame.window.valid == 1u && frame.window.graphics == 0u);
        assert(frame.window.text.base.cells[0].glyph_index == ' ' &&
            frame.window.text.base.cells[0].foreground == 0u &&
            frame.window.text.base.cells[7].glyph_index == '-' &&
            frame.window.text.base.cells[7].foreground == 8u &&
            frame.characters.primary['-'] == '-' && frame.characters.primary[0x80u] == ' ');
        request[8] = 2u;
        assert(core_driver_debug_execute()(driver, request, sizeof(request), response,
            sizeof(response), &response_size) == LIB_STATUS_INVALID_ARGUMENT);
        assert(driver->text_output);
        request[8] = 0u;
        assert(core_driver_debug_execute()(driver, request, sizeof(request), response,
            sizeof(response), &response_size) == LIB_STATUS_OK);
    }
    assert(core_driver_set_media(driver, "fixture.nes", LIB_STORAGE_MEDIUM_READONLY));
    accepted = driver->machine;
    for (index = 0u; index < 64u; ++index)
        assert(core_driver_run(driver));
    assert(core_driver_copy_frame(driver, &frame) == LIB_STATUS_OK && frame.window.valid == 1u);
    assert(!core_driver_set_media(driver, "missing.nes", LIB_STORAGE_MEDIUM_READONLY));
    assert(driver->machine == accepted);
    make_uxrom_fixture();
    write_fixture();
    assert(core_driver_set_media(driver, "fixture.nes", LIB_STORAGE_MEDIUM_READONLY));
    accepted = driver->machine;
    assert(accepted != LIB_NULL && accepted->cartridge->mapper == 2u);
    for (index = 0u; index < 64u; ++index)
        assert(core_driver_run(driver));
    assert(!accepted->trap.trap_valid);
    make_cnrom_fixture();
    write_fixture();
    assert(core_driver_set_media(driver, "fixture.nes", LIB_STORAGE_MEDIUM_READONLY));
    accepted = driver->machine;
    assert(accepted != LIB_NULL && accepted->cartridge->mapper == 3u);
    for (index = 0u; index < 64u; ++index)
        assert(core_driver_run(driver));
    assert(!accepted->trap.trap_valid);
    assert(!core_driver_set_media(driver, "fixture.nes", LIB_STORAGE_MEDIUM_DIRECT));
    assert(driver->machine == accepted);
    assert(core_driver_set_media(driver, LIB_NULL, LIB_STORAGE_MEDIUM_READONLY));
    assert(driver->machine == LIB_NULL);
    assert(core_driver_destroy(driver) == LIB_STATUS_OK);
    return 0;
}
