#include "core/chips/kbc8042/kbc8042_interface.h"
#include "core/chips/kbc8042/controller.h"
#include "core/chips/keyboard/keyboard_interface.h"
#include "core/chips/ps2mouse/ps2mouse_interface.h"

typedef struct controller_fixture {
    x86_kbc8042 *controller;
    x86_keyboard *keyboard;
    x86_ps2_mouse *mouse;
    lib_bool irq[2];
    lib_u32 assertions[2];
    lib_u32 releases[2];
    lib_u32 reset_pulses;
    lib_u8 output;
} controller_fixture;

static x86_kbc8042_keyboard_inputs keyboard_inputs(void *context)
{
    controller_fixture *f = context;
    const x86_keyboard_signals signals = x86_keyboard_get_signals(f->keyboard);
    x86_kbc8042_keyboard_inputs inputs = {
        .scan_set = signals.scan_set, .scanning = signals.scanning,
        .bat_ready = signals.bat_ready
    };
    inputs.repeat_pending = x86_keyboard_ticks_until_repeat(f->keyboard,
        &inputs.repeat_ticks) == LIB_STATUS_OK;
    return inputs;
}

static lib_bool keyboard_start(void *context)
{
    return x86_keyboard_start(((controller_fixture *)context)->keyboard);
}

static lib_bool keyboard_take_bat(void *context)
{
    return x86_keyboard_take_bat(((controller_fixture *)context)->keyboard);
}

static void keyboard_clear_bat(void *context)
{
    x86_keyboard_clear_bat(((controller_fixture *)context)->keyboard);
}

static void keyboard_cancel(void *context)
{
    x86_keyboard_cancel_parameter(((controller_fixture *)context)->keyboard);
}

static void keyboard_reply(void *context, const lib_u8 *bytes, lib_u8 count)
{
    controller_fixture *f = context;
    x86_kbc8042_receive_reply(f->controller, X86_KBC8042_OUTPUT_KEYBOARD, bytes, count);
}

static void keyboard_stream_reset(void *context)
{
    x86_kbc8042_reset_keyboard_stream(((controller_fixture *)context)->controller);
}

static void keyboard_write(void *context, lib_u8 byte)
{
    controller_fixture *f = context;
    const x86_keyboard_link link = { keyboard_reply, keyboard_stream_reset, f };
    x86_keyboard_write(f->keyboard, byte, &link);
}

static void aux_write(void *context, lib_u8 byte)
{
    controller_fixture *f = context;
    x86_ps2_mouse_reply reply;
    if (x86_ps2_mouse_write(f->mouse, byte, &reply) == LIB_STATUS_OK) {
        x86_kbc8042_receive_reply(f->controller, X86_KBC8042_OUTPUT_AUX,
            reply.bytes, reply.count);
    }
}

static void keyboard_accepted(void *context, lib_u8 byte)
{
    x86_keyboard_note_output(((controller_fixture *)context)->keyboard, byte);
}

static lib_status keyboard_admit(void *context, lib_u8 byte)
{
    return x86_keyboard_admit(((controller_fixture *)context)->keyboard, byte);
}

static lib_bool keyboard_advance(void *context, lib_u64 ticks,
    void (*emit)(void *context, lib_u8 byte), void *emit_context)
{
    return x86_keyboard_advance(((controller_fixture *)context)->keyboard,
        ticks, emit, emit_context);
}

static void irq(void *context, lib_bool auxiliary, lib_bool asserted)
{
    controller_fixture *f = context;
    const lib_u8 index = auxiliary ? 1u : 0u;
    f->irq[index] = asserted;
    if (asserted) ++f->assertions[index];
    else ++f->releases[index];
}

static void output_port(void *context, lib_u8 byte)
{
    ((controller_fixture *)context)->output = byte;
}

static void reset_pulse(void *context)
{
    ++((controller_fixture *)context)->reset_pulses;
}

static lib_status create(controller_fixture *f)
{
    const x86_kbc8042_link link = {
        .keyboard_inputs = keyboard_inputs, .keyboard_start = keyboard_start,
        .keyboard_take_bat = keyboard_take_bat, .keyboard_clear_bat = keyboard_clear_bat,
        .keyboard_cancel_parameter = keyboard_cancel, .keyboard_write = keyboard_write,
        .aux_write = aux_write, .keyboard_accepted = keyboard_accepted,
        .keyboard_admit = keyboard_admit, .keyboard_advance = keyboard_advance,
        .irq = irq, .output_port = output_port, .reset_pulse = reset_pulse, .context = f
    };
    lib_status status = x86_keyboard_create(&f->keyboard);
    if (status == LIB_STATUS_OK) status = x86_ps2_mouse_create(&f->mouse);
    if (status == LIB_STATUS_OK) status = x86_kbc8042_create(&link, &f->controller);
    return status;
}

static void destroy(controller_fixture *f)
{
    x86_kbc8042_destroy(f->controller);
    f->controller = LIB_NULL;
    x86_keyboard_destroy(f->keyboard);
    f->keyboard = LIB_NULL;
    x86_ps2_mouse_destroy(f->mouse);
    f->mouse = LIB_NULL;
}

static lib_bool transport(controller_fixture *f)
{
    x86_kbc8042 *c = f->controller;
    lib_bool failed = LIB_FALSE;
    lib_u64 ticks = 0u;
    lib_u32 edges;
    x86_kbc8042_write_command(c, 0x20u);
    failed |= x86_kbc8042_read_data(c) != 0x43u || f->irq[0] || f->irq[1];
    x86_kbc8042_write_command(c, 0xadu);
    x86_kbc8042_write_command(c, 0xaeu);
    failed |= !f->irq[0] || x86_kbc8042_read_data(c) != 0xaau || f->irq[0];
    x86_kbc8042_set_command_response_timing(c, 2u);
    x86_kbc8042_write_data(c, 0xffu);
    failed |= (x86_kbc8042_read_status(c) & 1u) != 0u;
    failed |= x86_kbc8042_ticks_until_event(c, &ticks) != LIB_STATUS_OK || ticks != 2u;
    x86_kbc8042_advance(c, 1u);
    failed |= (x86_kbc8042_read_status(c) & 1u) != 0u;
    x86_kbc8042_advance(c, 1u);
    edges = f->assertions[0];
    failed |= !f->irq[0] || x86_kbc8042_read_data(c) != 0xfau;
    failed |= f->assertions[0] != edges + 1u || !f->irq[0];
    failed |= x86_kbc8042_read_data(c) != 0xaau || f->irq[0];
    x86_kbc8042_set_command_response_timing(c, 0u);
    x86_kbc8042_write_command(c, 0xd4u);
    x86_kbc8042_write_data(c, 0xffu);
    failed |= !f->irq[1] || f->irq[0] || (x86_kbc8042_read_status(c) & 0x21u) != 0x21u;
    failed |= x86_kbc8042_read_data(c) != 0xfau || !f->irq[1];
    failed |= x86_kbc8042_read_data(c) != 0xaau || !f->irq[1];
    failed |= x86_kbc8042_read_data(c) != 0u || f->irq[1];
    x86_kbc8042_write_command(c, 0xd1u);
    x86_kbc8042_write_data(c, 3u);
    x86_kbc8042_write_command(c, 0xfeu);
    failed |= f->reset_pulses != 1u || f->output != 3u;
    x86_kbc8042_write_command(c, 0xd0u);
    failed |= x86_kbc8042_read_data(c) != 3u;
    x86_kbc8042_set_aux_present(c, LIB_FALSE);
    {
        const lib_u8 packet[3] = { 8u, 1u, 1u };
        failed |= x86_kbc8042_receive_aux_packet(c, packet) != LIB_STATUS_INVALID_STATE;
    }
    x86_kbc8042_write_command(c, 0xa8u);
    failed |= x86_kbc8042_aux_enabled(c);
    x86_kbc8042_write_command(c, 0xa9u);
    failed |= x86_kbc8042_read_data(c) != 1u;
    x86_kbc8042_write_command(c, 0xaau);
    failed |= x86_kbc8042_read_data(c) != 0x55u;
    failed |= x86_kbc8042_receive_keyboard_byte(c, 0x1cu) != LIB_STATUS_INVALID_STATE;
    x86_kbc8042_write_command(c, 0xaeu);
    failed |= x86_kbc8042_receive_keyboard_byte(c, 0x1cu) != LIB_STATUS_OK;
    failed |= x86_kbc8042_read_data(c) != 0x1eu;
    failed |= x86_kbc8042_receive_keyboard_byte(c, 0xf0u) != LIB_STATUS_OK;
    failed |= x86_kbc8042_receive_keyboard_byte(c, 0x1cu) != LIB_STATUS_OK;
    failed |= x86_kbc8042_read_data(c) != 0x9eu;
    return failed;
}

static lib_bool parameter_interleaving(controller_fixture *f)
{
    static const lib_u8 commands[] = { 0xedu, 0xf0u, 0xf3u };
    static const struct {
        lib_u8 command;
        lib_bool takes_data;
        lib_u8 data;
    } interventions[] = {
        { 0x20u, LIB_FALSE, 0u }, { 0xaau, LIB_FALSE, 0u },
        { 0xabu, LIB_FALSE, 0u }, { 0xadu, LIB_FALSE, 0u },
        { 0xaeu, LIB_FALSE, 0u }, { 0xa7u, LIB_FALSE, 0u },
        { 0xa8u, LIB_FALSE, 0u }, { 0xa9u, LIB_FALSE, 0u },
        { 0xc0u, LIB_FALSE, 0u }, { 0xd0u, LIB_FALSE, 0u },
        { 0xe0u, LIB_FALSE, 0u }, { 0xfeu, LIB_FALSE, 0u },
        { 0xffu, LIB_FALSE, 0u }, { 0x60u, LIB_TRUE, 0x07u },
        { 0xd1u, LIB_TRUE, 0x03u }, { 0xd4u, LIB_TRUE, 0xf2u }
    };
    x86_kbc8042 *c = f->controller;
    lib_bool failed = LIB_FALSE;
    lib_u8 topology;
    lib_size key;
    lib_size item;
    for (topology = 0u; topology < 2u; ++topology) {
        for (key = 0u; key < sizeof(commands); ++key) {
            for (item = 0u; item < sizeof(interventions) / sizeof(interventions[0]); ++item) {
                const lib_u8 expected = interventions[item].takes_data ? 0xeeu :
                    (commands[key] == 0xf0u ? 0xfeu : 0xfau);
                lib_u8 drain;
                x86_keyboard_reset(f->keyboard);
                x86_ps2_mouse_reset(f->mouse);
                x86_kbc8042_set_aux_present(c, topology != 0u);
                x86_kbc8042_reset(c);
                x86_kbc8042_write_data(c, commands[key]);
                failed |= x86_kbc8042_read_data(c) != 0xfau;
                x86_kbc8042_write_command(c, interventions[item].command);
                if (interventions[item].takes_data) {
                    x86_kbc8042_write_data(c, interventions[item].data);
                }
                for (drain = 0u; drain < 8u && (x86_kbc8042_read_status(c) & 1u) != 0u; ++drain) {
                    (void)x86_kbc8042_read_data(c);
                }
                failed |= (x86_kbc8042_read_status(c) & 1u) != 0u;
                x86_kbc8042_write_data(c, 0xeeu);
                failed |= (x86_kbc8042_read_status(c) & 1u) == 0u ||
                    x86_kbc8042_read_data(c) != expected ||
                    (x86_kbc8042_read_status(c) & 1u) != 0u;
            }
        }
    }
    return failed;
}

static lib_bool packet_saturation(controller_fixture *f)
{
    const lib_u8 packet[3] = { 8u, 2u, 3u };
    lib_bool failed = LIB_FALSE;
    lib_u8 index;
    x86_keyboard_reset(f->keyboard);
    x86_kbc8042_set_aux_present(f->controller, LIB_TRUE);
    x86_kbc8042_reset(f->controller);
    for (index = 0u; index < 21u; ++index) {
        failed |= x86_kbc8042_receive_aux_packet(f->controller, packet) != LIB_STATUS_OK;
    }
    failed |= x86_kbc8042_receive_aux_packet(f->controller, packet) != LIB_STATUS_INVALID_STATE;
    for (index = 0u; index < 63u; ++index) {
        failed |= (x86_kbc8042_read_status(f->controller) & 0x21u) != 0x21u ||
            x86_kbc8042_read_data(f->controller) != packet[index % 3u];
    }
    failed |= (x86_kbc8042_read_status(f->controller) & 0x21u) != 0u;
    return failed;
}

static lib_bool configuration_and_serial(controller_fixture *f)
{
    static const lib_u8 pin_values[] = {0u, 3u, 0xa5u, 0xffu};
    const lib_u8 keys[] = {0x1cu, 0x32u};
    const lib_u8 overflow[KBC_KEYBOARD_SERIAL_CAPACITY + 1u] = {0};
    x86_kbc8042 *c = f->controller;
    lib_u64 ticks = 0u;
    lib_bool failed = LIB_FALSE;
    x86_keyboard_reset(f->keyboard);
    x86_kbc8042_reset(c);
    for (lib_size i = 0u; i < sizeof(pin_values); ++i) {
        x86_kbc8042_set_input_port(c, pin_values[i]);
        x86_kbc8042_write_command(c, 0xc0u);
        failed |= x86_kbc8042_read_data(c) != pin_values[i];
        x86_kbc8042_set_test_inputs(c, pin_values[i]);
        x86_kbc8042_write_command(c, 0xe0u);
        failed |= x86_kbc8042_read_data(c) != (pin_values[i] & 3u);
    }
    x86_kbc8042_set_reset_output_port(c, 3u);
    x86_kbc8042_set_command_response_status_polls(c, 2u);
    x86_kbc8042_reset(c);
    failed |= f->output != 3u;
    x86_kbc8042_write_command(c, 0x20u);
    failed |= (x86_kbc8042_read_status(c) & 1u) != 0u;
    failed |= (x86_kbc8042_read_status(c) & 1u) != 0u;
    failed |= (x86_kbc8042_read_status(c) & 1u) == 0u || x86_kbc8042_read_data(c) != 0x43u;
    x86_kbc8042_set_command_response_status_polls(c, 0u);
    x86_kbc8042_set_serial_delivery_timing(c, 3u);
    x86_kbc8042_reset(c); /* Configured serial delay survives reset. */
    failed |= x86_kbc8042_receive_keyboard_bytes(c, LIB_NULL, 1u) != LIB_STATUS_INVALID_ARGUMENT;
    failed |= x86_kbc8042_receive_keyboard_bytes(c, LIB_NULL, 0u) != LIB_STATUS_OK;
    failed |= x86_kbc8042_receive_keyboard_bytes(c, keys, sizeof(keys)) != LIB_STATUS_OK;
    failed |= x86_kbc8042_receive_keyboard_bytes(c, overflow, sizeof(overflow)) != LIB_STATUS_NO_MEMORY;
    failed |= x86_kbc8042_ticks_until_event(c, &ticks) != LIB_STATUS_OK || ticks != 3u;
    x86_kbc8042_advance(c, 2u);
    failed |= (x86_kbc8042_read_status(c) & 1u) != 0u;
    x86_kbc8042_advance(c, 1u);
    failed |= x86_kbc8042_read_data(c) != 0x1eu;
    x86_kbc8042_advance(c, 3u);
    failed |= x86_kbc8042_read_data(c) != 0x30u || (x86_kbc8042_read_status(c) & 1u) != 0u;
    x86_kbc8042_set_serial_delivery_timing(c, 0u);
    return failed;
}

int main(void)
{
    controller_fixture fixture = { 0 };
    lib_bool failed;
    if (create(&fixture) != LIB_STATUS_OK) {
        destroy(&fixture);
        return 1;
    }
    failed = transport(&fixture);
    failed |= parameter_interleaving(&fixture);
    failed |= packet_saturation(&fixture);
    failed |= configuration_and_serial(&fixture);
    destroy(&fixture);
    failed |= fixture.irq[0] || fixture.irq[1];
    return failed ? 1 : 0;
}
