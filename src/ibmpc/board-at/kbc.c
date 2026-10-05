/* Copyright 2012-2026 Neko. */
#include "ibmpc/board-at/kbc.h"

lib_status core_machine_kbc_create(core_machine *machine, t_kbc **out_controller)
{
    t_kbc *controller;
    lib_status status;
    if (out_controller == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_controller = LIB_NULL;
    if (machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    controller = lib_allocate_zero(1u, sizeof(*controller));
    if (controller == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    status = core_machine_kbc_initialize(controller, machine);
    if (status != LIB_STATUS_OK) {
        lib_release(controller);
        return status;
    }
    *out_controller = controller;
    return LIB_STATUS_OK;
}

void core_machine_kbc_destroy(t_kbc *controller)
{
    core_machine_kbc_finalize(controller);
    lib_release(controller);
}

lib_u8 core_machine_kbc_get_native_scan_set(const t_kbc *controller)
{
    return x86_keyboard_get_signals(controller == LIB_NULL ? LIB_NULL :
        controller->connect.keyboard).scan_set;
}

static x86_kbc8042_keyboard_inputs kbc_keyboard_inputs(void *context)
{
    t_kbc *attachment = context;
    const x86_keyboard_signals signals =
        x86_keyboard_get_signals(attachment->connect.keyboard);
    x86_kbc8042_keyboard_inputs inputs = {
        .scan_set = signals.scan_set,
        .scanning = signals.scanning,
        .bat_ready = signals.bat_ready
    };
    inputs.repeat_pending = x86_keyboard_ticks_until_repeat(
        attachment->connect.keyboard, &inputs.repeat_ticks) == LIB_STATUS_OK;
    return inputs;
}

static lib_bool kbc_keyboard_start(void *context)
{
    t_kbc *attachment = context;
    return x86_keyboard_start(attachment->connect.keyboard);
}

static lib_bool kbc_keyboard_take_bat(void *context)
{
    t_kbc *attachment = context;
    return x86_keyboard_take_bat(attachment->connect.keyboard);
}

static void kbc_keyboard_clear_bat(void *context)
{
    t_kbc *attachment = context;
    x86_keyboard_clear_bat(attachment->connect.keyboard);
}

static void kbc_keyboard_cancel_parameter(void *context)
{
    t_kbc *attachment = context;
    x86_keyboard_cancel_parameter(attachment->connect.keyboard);
}

static void kbc_keyboard_reply(void *context, const lib_u8 *bytes, lib_u8 count)
{
    t_kbc *attachment = context;
    x86_kbc8042_receive_reply(attachment->chip, X86_KBC8042_OUTPUT_KEYBOARD, bytes, count);
}

static void kbc_keyboard_stream_reset(void *context)
{
    t_kbc *attachment = context;
    x86_kbc8042_reset_keyboard_stream(attachment->chip);
}

static void kbc_keyboard_write(void *context, lib_u8 byte)
{
    t_kbc *attachment = context;
    const x86_keyboard_link link = {
        kbc_keyboard_reply, kbc_keyboard_stream_reset, attachment
    };
    x86_keyboard_write(attachment->connect.keyboard, byte, &link);
}

static void kbc_aux_write(void *context, lib_u8 byte)
{
    t_kbc *attachment = context;
    x86_ps2_mouse_reply reply;
    if (x86_ps2_mouse_write(attachment->connect.aux_device, byte, &reply) == LIB_STATUS_OK) {
        x86_kbc8042_receive_reply(attachment->chip, X86_KBC8042_OUTPUT_AUX,
            reply.bytes, reply.count);
    }
}

static void kbc_keyboard_accepted(void *context, lib_u8 byte)
{
    t_kbc *attachment = context;
    x86_keyboard_note_output(attachment->connect.keyboard, byte);
}

static lib_status kbc_keyboard_admit(void *context, lib_u8 byte)
{
    t_kbc *attachment = context;
    return x86_keyboard_admit(attachment->connect.keyboard, byte);
}

static lib_bool kbc_keyboard_advance(void *context, lib_u64 ticks,
    void (*emit)(void *context, lib_u8 byte), void *emit_context)
{
    t_kbc *attachment = context;
    return x86_keyboard_advance(attachment->connect.keyboard, ticks, emit, emit_context);
}

static void kbc_irq(void *context, lib_bool auxiliary, lib_bool asserted)
{
    t_kbc *attachment = context;
    if (attachment->connect.irq != LIB_NULL)
        attachment->connect.irq(attachment->connect.irq_context, auxiliary, asserted);
}

static void kbc_reset_pulse(void *context)
{
    t_kbc *attachment = context;
    if (attachment->connect.request_reset != LIB_NULL) {
        attachment->connect.request_reset(attachment->connect.reset_context);
    }
}

static void kbc_output_port(void *context, lib_u8 value)
{
    t_kbc *attachment = context;
    if (attachment->connect.set_a20 != LIB_NULL)
        attachment->connect.set_a20(attachment->connect.a20_context,
            (value & 0x02u) != 0u);
    if ((value & 0x01u) == 0u) kbc_reset_pulse(context);
}

static lib_status kbc_port_read(void *owner, lib_u16 address, lib_u64 tick,
    lib_u32 *out_value)
{
    (void)tick;
    t_kbc *attachment = owner;
    lib_u8 value;

    if (attachment == LIB_NULL || out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    value = address == 0x60u ? x86_kbc8042_read_data(attachment->chip) :
        x86_kbc8042_read_status(attachment->chip);
    *out_value = (*out_value & ~0xffu) | value;
    return LIB_STATUS_OK;
}

static lib_status kbc_port_write(void *owner, lib_u16 address, lib_u32 value)
{
    t_kbc *attachment = owner;

    if (attachment == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (address == 0x60u) x86_kbc8042_write_data(attachment->chip, (lib_u8)value);
    else x86_kbc8042_write_command(attachment->chip, (lib_u8)value);
    return LIB_STATUS_OK;
}

lib_status core_machine_kbc_initialize(t_kbc *attachment, core_machine *machine)
{
    core_machine_port_route routes[2];
    lib_status status;
    const x86_kbc8042_link link = {
        .keyboard_inputs = kbc_keyboard_inputs,
        .keyboard_start = kbc_keyboard_start,
        .keyboard_take_bat = kbc_keyboard_take_bat,
        .keyboard_clear_bat = kbc_keyboard_clear_bat,
        .keyboard_cancel_parameter = kbc_keyboard_cancel_parameter,
        .keyboard_write = kbc_keyboard_write,
        .aux_write = kbc_aux_write,
        .keyboard_accepted = kbc_keyboard_accepted,
        .keyboard_admit = kbc_keyboard_admit,
        .keyboard_advance = kbc_keyboard_advance,
        .irq = kbc_irq,
        .output_port = kbc_output_port,
        .reset_pulse = kbc_reset_pulse,
        .context = attachment
    };

    if (attachment == LIB_NULL || machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    lib_memory_set(attachment, 0u, sizeof(*attachment));
    status = x86_keyboard_create(&attachment->connect.keyboard);
    if (status == LIB_STATUS_OK) status = x86_ps2_mouse_create(&attachment->connect.aux_device);
    if (status == LIB_STATUS_OK) status = x86_kbc8042_create(&link, &attachment->chip);
    if (status != LIB_STATUS_OK) {
        core_machine_kbc_finalize(attachment);
        return status;
    }
    routes[0] = (core_machine_port_route) {0x60u, kbc_port_read,
        kbc_port_write, attachment, LIB_FALSE, 0u};
    routes[1] = (core_machine_port_route) {0x64u, kbc_port_read,
        kbc_port_write, attachment, LIB_FALSE, 0u};
    status = core_machine_install_port_routes(machine, routes, 2u);
    if (status != LIB_STATUS_OK) core_machine_kbc_finalize(attachment);
    return status;
}

void core_machine_kbc_bind_core_services(t_kbc *attachment,
    core_machine_kbc_irq_line irq, void *irq_context,
    void (*set_a20)(void *context, lib_bool enabled), void *a20_context,
    void (*request_reset)(void *context), void *reset_context,
    lib_u8 aux_present)
{
    if (attachment == LIB_NULL) return;
    attachment->connect.irq = irq;
    attachment->connect.irq_context = irq_context;
    attachment->connect.set_a20 = set_a20;
    attachment->connect.a20_context = a20_context;
    attachment->connect.request_reset = request_reset;
    attachment->connect.reset_context = reset_context;
    x86_kbc8042_set_aux_present(attachment->chip, aux_present);
}

void core_machine_kbc_reset(t_kbc *attachment)
{
    if (attachment == LIB_NULL) return;
    x86_keyboard_reset(attachment->connect.keyboard);
    x86_ps2_mouse_reset(attachment->connect.aux_device);
    x86_kbc8042_reset(attachment->chip);
}

void core_machine_kbc_finalize(t_kbc *attachment)
{
    if (attachment == LIB_NULL) return;
    x86_kbc8042_destroy(attachment->chip);
    attachment->chip = LIB_NULL;
    x86_keyboard_destroy(attachment->connect.keyboard);
    attachment->connect.keyboard = LIB_NULL;
    x86_ps2_mouse_destroy(attachment->connect.aux_device);
    attachment->connect.aux_device = LIB_NULL;
}

void core_machine_kbc_set_input_port(t_kbc *attachment, lib_u8 value)
{
    if (attachment != LIB_NULL) x86_kbc8042_set_input_port(attachment->chip, value);
}

void core_machine_kbc_set_test_inputs(t_kbc *attachment, lib_u8 value)
{
    if (attachment != LIB_NULL) x86_kbc8042_set_test_inputs(attachment->chip, value);
}

void core_machine_kbc_set_reset_output_port(t_kbc *attachment, lib_u8 value)
{
    if (attachment != LIB_NULL) x86_kbc8042_set_reset_output_port(attachment->chip, value);
}

void core_machine_kbc_advance(t_kbc *attachment, lib_u64 ticks)
{
    if (attachment != LIB_NULL) x86_kbc8042_advance(attachment->chip, ticks);
}

lib_status core_machine_kbc_ticks_until_event(const t_kbc *attachment, lib_u64 *ticks)
{
    if (attachment == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return x86_kbc8042_ticks_until_event(attachment->chip, ticks);
}

void core_machine_kbc_set_typematic_timing(t_kbc *attachment, lib_u32 initial, lib_u32 repeat)
{
    if (attachment != LIB_NULL) {
        x86_keyboard_set_typematic_timing(attachment->connect.keyboard, initial, repeat);
    }
}

void core_machine_kbc_set_command_response_timing(t_kbc *attachment, lib_u32 ticks)
{
    if (attachment != LIB_NULL) x86_kbc8042_set_command_response_timing(attachment->chip, ticks);
}

void core_machine_kbc_set_command_response_status_polls(t_kbc *attachment, lib_u8 polls)
{
    if (attachment != LIB_NULL) x86_kbc8042_set_command_response_status_polls(attachment->chip, polls);
}

void core_machine_kbc_set_serial_delivery_timing(t_kbc *attachment, lib_u32 ticks)
{
    if (attachment != LIB_NULL) x86_kbc8042_set_serial_delivery_timing(attachment->chip, ticks);
}

lib_status core_machine_kbc_submit_native_byte(t_kbc *attachment, lib_u8 byte)
{
    if (attachment == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return x86_kbc8042_receive_keyboard_byte(attachment->chip, byte);
}

lib_status core_machine_kbc_submit_native_bytes(t_kbc *attachment, const lib_u8 *bytes, lib_size count)
{
    if (attachment == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return x86_kbc8042_receive_keyboard_bytes(attachment->chip, bytes, count);
}

static lib_status kbc_aux_packet(void *context, const lib_u8 packet[3])
{
    t_kbc *attachment = context;
    return x86_kbc8042_receive_aux_packet(attachment->chip, packet);
}

lib_status core_machine_kbc_submit_aux_report(t_kbc *attachment,
    lib_i16 delta_x, lib_i16 delta_y, lib_u8 buttons)
{
    if (attachment == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (!x86_kbc8042_aux_enabled(attachment->chip)) return LIB_STATUS_INVALID_STATE;
    return x86_ps2_mouse_report(attachment->connect.aux_device,
        delta_x, delta_y, buttons, kbc_aux_packet, attachment);
}
