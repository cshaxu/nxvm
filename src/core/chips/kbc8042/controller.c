/* Copyright 2012-2026 Neko. */
#include "core/chips/kbc8042/controller.h"
#include "core/chips/keyboard/keyboard_interface.h"

#define KBC_COMMAND_IRQ1 0x01u
#define KBC_COMMAND_SYSTEM 0x04u
#define KBC_BAT_OK 0xaau

static void x86_kbc8042_drain_keyboard_serial(x86_kbc8042 *controller);
static void x86_kbc8042_refresh_current_irq(x86_kbc8042 *controller);

static lib_status x86_kbc8042_queue_native_byte(x86_kbc8042 *controller,
    lib_u8 native_byte)
{
    lib_u8 tail;

    if (controller->data.keyboard_serial_count >=
        KBC_KEYBOARD_SERIAL_CAPACITY) return LIB_STATUS_NO_MEMORY;
    tail = (lib_u8)((controller->data.keyboard_serial_head +
        controller->data.keyboard_serial_count) % KBC_KEYBOARD_SERIAL_CAPACITY);
    controller->data.keyboard_serial[tail] = native_byte;
    ++controller->data.keyboard_serial_count;
    if (controller->data.serial_delivery_ticks != 0u &&
        controller->data.serial_delivery_remaining_ticks == 0u) {
        controller->data.serial_delivery_remaining_ticks = controller->data.serial_delivery_ticks;
    }
    return LIB_STATUS_OK;
}

static void x86_kbc8042_deassert_irq1(x86_kbc8042 *controller)
{
    if (controller == LIB_NULL || !controller->data.irq1_asserted) return;
    controller->link.irq(controller->link.context, LIB_FALSE, LIB_FALSE);
    controller->data.irq1_asserted = LIB_FALSE;
}

static void x86_kbc8042_deassert_irq12(x86_kbc8042 *controller)
{
    if (controller == LIB_NULL || !controller->data.irq12_asserted) return;
    controller->link.irq(controller->link.context, LIB_TRUE, LIB_FALSE);
    controller->data.irq12_asserted = LIB_FALSE;
}

static lib_bool x86_kbc8042_keyboard_scan_delivery_enabled(const x86_kbc8042 *controller,
    lib_u8 command_byte)
{
    return controller != LIB_NULL &&
        (command_byte & KBC_COMMAND_DISABLE_KEYBOARD) == 0u &&
        ((command_byte & KBC_COMMAND_INHIBIT_OVERRIDE) != 0u ||
        (controller->data.input_port & 0x80u) != 0u);
}

static void x86_kbc8042_set_command_byte(x86_kbc8042 *controller,
    lib_u8 value)
{
    const lib_u8 previous_command_byte = controller->data.command_byte;
    const lib_bool keyboard_clock_was_enabled =
        (previous_command_byte & KBC_COMMAND_DISABLE_KEYBOARD) == 0u;

    controller->data.command_byte = value & 0x7du;
    if (controller->aux_present) {
        controller->data.command_byte |= value & KBC_COMMAND_IRQ12;
    } else {
        controller->data.command_byte |= KBC_COMMAND_DISABLE_AUX;
    }
    controller->data.aux_enabled = controller->aux_present &&
        (controller->data.command_byte & KBC_COMMAND_DISABLE_AUX) == 0u;
    /* The IBM AT keyboard starts BAT when the controller first either raises
     * the command-byte inhibit override (45h -> 4Dh) or releases a previously
     * held clock while the board switch permits the interface (AEh).  The
     * switch is an input observation; it is not a synthetic data-line state. */
    if ((controller->data.command_byte & KBC_COMMAND_DISABLE_KEYBOARD) == 0u &&
        (((previous_command_byte & KBC_COMMAND_INHIBIT_OVERRIDE) == 0u &&
            (controller->data.command_byte & KBC_COMMAND_INHIBIT_OVERRIDE) != 0u) ||
         (!keyboard_clock_was_enabled &&
            (controller->data.input_port & 0x80u) != 0u))) {
        if (controller->link.keyboard_start(controller->link.context)) {
            x86_kbc8042_advance(controller, 0u);
        }
    }
    x86_kbc8042_refresh_current_irq(controller);
}

static void x86_kbc8042_refresh_current_irq(x86_kbc8042 *controller)
{
    x86_kbc8042_origin origin;

    if (controller == LIB_NULL || controller->data.fifo_count == 0u) {
        x86_kbc8042_deassert_irq1(controller);
        x86_kbc8042_deassert_irq12(controller);
        return;
    }
    origin = controller->data.fifo_origin[controller->data.fifo_head];
    if (origin == X86_KBC8042_OUTPUT_KEYBOARD) {
        x86_kbc8042_deassert_irq12(controller);
        /* ADh inhibits new keyboard serial input.  It does not suppress an
         * already-generated keyboard-controller reply: IBM's POST sends ADh,
         * resets the keyboard, then waits for BAT AAh through IRQ1. */
        if (!controller->data.irq1_asserted &&
            (controller->data.command_byte & KBC_COMMAND_IRQ1) != 0u) {
            controller->link.irq(controller->link.context, LIB_FALSE, LIB_TRUE);
            controller->data.irq1_asserted = LIB_TRUE;
        }
    } else if (origin == X86_KBC8042_OUTPUT_AUX) {
        x86_kbc8042_deassert_irq1(controller);
        if (!controller->data.irq12_asserted && controller->aux_present &&
            controller->data.aux_enabled &&
            (controller->data.command_byte & KBC_COMMAND_IRQ12) != 0u &&
            (controller->data.command_byte & KBC_COMMAND_DISABLE_AUX) == 0u) {
            controller->link.irq(controller->link.context, LIB_TRUE, LIB_TRUE);
            controller->data.irq12_asserted = LIB_TRUE;
        }
    } else {
        x86_kbc8042_deassert_irq1(controller);
        x86_kbc8042_deassert_irq12(controller);
    }
}

static void x86_kbc8042_flush_output(x86_kbc8042 *controller)
{
    if (controller == LIB_NULL) return;
    controller->data.fifo_head = 0u;
    controller->data.fifo_count = 0u;
    controller->data.keyboard_serial_head = 0u;
    controller->data.keyboard_serial_count = 0u;
    controller->data.delayed_response_count = 0u;
    controller->data.delayed_response_index = 0u;
    controller->data.response_remaining_ticks = 0u;
    controller->data.response_status_polls_remaining = 0u;
    controller->data.serial_delivery_remaining_ticks = 0u;
    controller->link.keyboard_clear_bat(controller->link.context);
    x86_kbc8042_deassert_irq1(controller);
    x86_kbc8042_deassert_irq12(controller);
}

static lib_status x86_kbc8042_enqueue(x86_kbc8042 *controller, lib_u8 value,
    x86_kbc8042_origin origin)
{
    lib_u8 tail;

    if (controller == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (controller->data.fifo_count >= KBC_FIFO_CAPACITY) {
        return LIB_STATUS_INVALID_STATE;
    }
    tail = (lib_u8)((controller->data.fifo_head +
        controller->data.fifo_count) % KBC_FIFO_CAPACITY);
    controller->data.fifo[tail] = value;
    controller->data.fifo_origin[tail] = origin;
    ++controller->data.fifo_count;
    if (origin == X86_KBC8042_OUTPUT_KEYBOARD) {
        controller->link.keyboard_accepted(controller->link.context, value);
    }
    x86_kbc8042_refresh_current_irq(controller);
    return LIB_STATUS_OK;
}

static void x86_kbc8042_schedule_response(x86_kbc8042 *controller,
    const lib_u8 *bytes, lib_u8 count, x86_kbc8042_origin origin)
{
    lib_u8 index;

    if (controller == LIB_NULL || bytes == LIB_NULL || count == 0u ||
        count > KBC_RESPONSE_CAPACITY ||
        controller->data.delayed_response_count != 0u) return;
    for (index = 0u; index < count; ++index) {
        controller->data.delayed_response[index] = bytes[index];
    }
    controller->data.delayed_response_count = count;
    controller->data.delayed_response_index = 0u;
    controller->data.delayed_response_origin = origin;
    controller->data.response_remaining_ticks =
        controller->data.command_response_ticks;
    /* A response must not become visible until the host has observed the
     * completed write.  The 5170 ROM's shared command routine deliberately
     * flushes an already-full output buffer before it starts waiting for a
     * reply; this applies to keyboard ACK as well as controller self-test
     * output.  One configured status poll is the profile-owned L2 ordering
     * contract, not a separate keyboard path. */
    controller->data.response_status_polls_remaining =
        controller->data.command_response_status_polls;

    /* The response bytes remain KBC-owned until the guest-visible FIFO has
     * room.  A full rapid-typeahead FIFO must delay a command reply, never
     * silently discard it.  A zero delay still drains synchronously for
     * controller commands that are observed in the same I/O sequence. */
    if (controller->data.command_response_ticks == 0u &&
        controller->data.response_status_polls_remaining == 0u) {
        x86_kbc8042_advance(controller, 0u);
    }
}

static void x86_kbc8042_schedule_response_byte(x86_kbc8042 *controller,
    lib_u8 value, x86_kbc8042_origin origin)
{
    x86_kbc8042_schedule_response(controller, &value, 1u, origin);
}

static lib_status x86_kbc8042_enqueue_set1_pause(x86_kbc8042 *controller)
{
    static const lib_u8 pause[] = { 0xe1u, 0x1du, 0x45u,
        0xe1u, 0x9du, 0xc5u };
    lib_size index;

    if (controller == LIB_NULL || KBC_FIFO_CAPACITY -
        controller->data.fifo_count < sizeof(pause)) return LIB_STATUS_INVALID_STATE;
    for (index = 0u; index < sizeof(pause); ++index) {
        (void)x86_kbc8042_enqueue(controller, pause[index],
            X86_KBC8042_OUTPUT_KEYBOARD);
    }
    return LIB_STATUS_OK;
}

static lib_status x86_kbc8042_translate_set2_byte(x86_kbc8042 *controller,
    lib_u8 native_byte)
{
    static const lib_u8 pause_set2[] = { 0xe1u, 0x14u, 0x77u,
        0xe1u, 0xf0u, 0x14u, 0xf0u, 0x77u };
    lib_bool known;
    lib_u8 translated;

    if (controller == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (controller->data.set2_pause_count != 0u || native_byte == 0xe1u) {
        if (controller->data.set2_pause_count >=
            sizeof(controller->data.set2_pause_bytes)) return LIB_STATUS_INVALID_STATE;
        controller->data.set2_pause_bytes[controller->data.set2_pause_count++] = native_byte;
        if (controller->data.set2_pause_count <
            sizeof(controller->data.set2_pause_bytes)) return LIB_STATUS_OK;
        controller->data.set2_pause_count = 0u;
        if (lib_memory_compare(controller->data.set2_pause_bytes, pause_set2,
                sizeof(pause_set2)) != 0) return LIB_STATUS_UNSUPPORTED;
        return x86_kbc8042_enqueue_set1_pause(controller);
    }
    if (native_byte == 0xe0u) {
        return x86_kbc8042_enqueue(controller, native_byte,
            X86_KBC8042_OUTPUT_KEYBOARD);
    }
    if (native_byte == 0xf0u) {
        controller->data.set2_break_pending = LIB_TRUE;
        return LIB_STATUS_OK;
    }
    translated = x86_keyboard_set2_to_set1(native_byte, &known);
    if (!known) return LIB_STATUS_UNSUPPORTED;
    if (controller->data.set2_break_pending) translated |= 0x80u;
    controller->data.set2_break_pending = LIB_FALSE;
    return x86_kbc8042_enqueue(controller, translated,
        X86_KBC8042_OUTPUT_KEYBOARD);
}

static lib_status x86_kbc8042_publish_native_byte(x86_kbc8042 *controller,
    lib_u8 native_byte)
{
    if (controller == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (controller->link.keyboard_inputs(controller->link.context).scan_set == 2u &&
        (controller->data.command_byte & KBC_COMMAND_TRANSLATION) != 0u) {
        return x86_kbc8042_translate_set2_byte(controller, native_byte);
    }
    return x86_kbc8042_enqueue(controller, native_byte,
        X86_KBC8042_OUTPUT_KEYBOARD);
}

static void x86_kbc8042_drain_keyboard_serial(x86_kbc8042 *controller)
{
    lib_u8 native_byte;
    lib_status status;

    if (controller == LIB_NULL ||
        controller->data.serial_delivery_remaining_ticks != 0u ||
        !controller->link.keyboard_inputs(controller->link.context).scanning ||
        !x86_kbc8042_keyboard_scan_delivery_enabled(controller,
            controller->data.command_byte)) return;
    /* The keyboard serial stream may have private backlog, but only one
     * scan byte may enter the controller output path at a time.  Firmware
     * such as the 5170 POST disables the keyboard and clears one OBF byte;
     * admitting a whole host key chord past that boundary is not hardware. */
    if (controller->data.keyboard_serial_count != 0u &&
        controller->data.fifo_count == 0u) {
        native_byte = controller->data.keyboard_serial[
            controller->data.keyboard_serial_head];
        status = x86_kbc8042_publish_native_byte(controller, native_byte);
        if (status != LIB_STATUS_OK && status != LIB_STATUS_UNSUPPORTED) return;
        controller->data.keyboard_serial_head = (lib_u8)(
            (controller->data.keyboard_serial_head + 1u) %
            KBC_KEYBOARD_SERIAL_CAPACITY);
        --controller->data.keyboard_serial_count;
        if (controller->data.serial_delivery_ticks != 0u) {
            controller->data.serial_delivery_remaining_ticks =
                controller->data.serial_delivery_ticks;
        }
    }
}

static lib_u8 x86_kbc8042_dequeue(x86_kbc8042 *controller)
{
    lib_u8 value = 0u;
    x86_kbc8042_origin origin;

    if (controller == LIB_NULL || controller->data.fifo_count == 0u) return 0u;
    value = controller->data.fifo[controller->data.fifo_head];
    origin = controller->data.fifo_origin[controller->data.fifo_head];
    controller->data.fifo_head = (lib_u8)((controller->data.fifo_head + 1u) %
        KBC_FIFO_CAPACITY);
    --controller->data.fifo_count;
    /* Reading 60h acknowledges exactly the current origin. A queued successor
     * gets a fresh edge only after promotion through the one PIC boundary. */
    if (origin == X86_KBC8042_OUTPUT_KEYBOARD) {
        x86_kbc8042_deassert_irq1(controller);
    } else if (origin == X86_KBC8042_OUTPUT_AUX) {
        x86_kbc8042_deassert_irq12(controller);
    }
    x86_kbc8042_refresh_current_irq(controller);
    /* Consuming ACK makes its pending BAT eligible for the empty output
     * buffer. PIC masking/delivery owns when the resulting IRQ is serviced. */
    x86_kbc8042_advance(controller, 0u);
    x86_kbc8042_drain_keyboard_serial(controller);
    return value;
}

static lib_u8 x86_kbc8042_status(const x86_kbc8042 *controller)
{
    lib_u8 status = 0u;

    if (controller == LIB_NULL) return status;
    if (controller->data.fifo_count != 0u) status |= KBC_STATUS_OBF;
    if (controller->data.input_buffer_full) status |= KBC_STATUS_IBF;
    if ((controller->data.command_byte & KBC_COMMAND_SYSTEM) != 0u) {
        status |= KBC_STATUS_SYS;
    }
    if (controller->data.last_write_command) status |= KBC_STATUS_CD;
    if ((controller->data.input_port & 0x80u) != 0u) status |= KBC_STATUS_INHIBIT;
    if (controller->data.fifo_count != 0u &&
        controller->data.fifo_origin[controller->data.fifo_head] ==
            X86_KBC8042_OUTPUT_AUX) status |= KBC_STATUS_AUX;
    return status;
}

static void x86_kbc8042_apply_output_port(x86_kbc8042 *controller, lib_u8 value)
{
    if (controller == LIB_NULL) return;
    controller->data.output_port = value;
    controller->link.output_port(controller->link.context, value);
}

void x86_kbc8042_receive_reply(x86_kbc8042 *controller,
    x86_kbc8042_origin origin, const lib_u8 *bytes, lib_u8 count)
{
    lib_u8 translated[3];

    /* Identity translation belongs to the controller, not the keyboard. */
    if (controller == LIB_NULL || bytes == LIB_NULL) return;
    if (origin == X86_KBC8042_OUTPUT_KEYBOARD && count == 3u && bytes[0] == 0xfau && bytes[1] == 0xabu &&
        bytes[2] == 0x83u &&
        (controller->data.command_byte & KBC_COMMAND_TRANSLATION) != 0u) {
        lib_memory_copy(translated, bytes, sizeof(translated));
        translated[2] = 0x41u;
        bytes = translated;
    }
    x86_kbc8042_schedule_response(controller, bytes, count, origin);
}

void x86_kbc8042_reset_keyboard_stream(x86_kbc8042 *controller)
{
    if (controller == LIB_NULL) return;
    controller->data.set2_break_pending = LIB_FALSE;
    controller->data.set2_pause_count = 0u;
}

void x86_kbc8042_set_input_port(x86_kbc8042 *controller, lib_u8 value)
{
    if (controller != LIB_NULL) controller->data.input_port = value;
}

void x86_kbc8042_set_test_inputs(x86_kbc8042 *controller, lib_u8 value)
{
    if (controller != LIB_NULL) controller->data.test_inputs = value & 0x03u;
}

lib_u8 x86_kbc8042_read_data(x86_kbc8042 *controller)
{
    return x86_kbc8042_dequeue(controller);
}

lib_u8 x86_kbc8042_read_status(x86_kbc8042 *controller)
{
    lib_u8 status;
    status = x86_kbc8042_status(controller);
    if (controller != LIB_NULL && controller->data.delayed_response_count != 0u &&
        controller->data.response_remaining_ticks == 0u &&
        controller->data.response_status_polls_remaining != 0u) {
        --controller->data.response_status_polls_remaining;
        if (controller->data.response_status_polls_remaining == 0u) {
            x86_kbc8042_advance(controller, 0u);
        }
    }
    return status;
}

void x86_kbc8042_write_data(x86_kbc8042 *controller, lib_u8 value)
{
    if (controller == LIB_NULL) return;
    controller->data.input_buffer_full = LIB_TRUE;
    controller->data.last_write_command = LIB_FALSE;
    switch (controller->data.pending_write) {
    case KBC_PENDING_COMMAND_BYTE:
    {
        controller->data.pending_write = KBC_PENDING_NONE;
        x86_kbc8042_set_command_byte(controller, value);
        break;
    }
    case KBC_PENDING_OUTPUT_PORT:
        x86_kbc8042_apply_output_port(controller, value);
        controller->data.pending_write = KBC_PENDING_NONE;
        break;
    case KBC_PENDING_AUX_DEVICE:
        controller->data.pending_write = KBC_PENDING_NONE;
        controller->link.aux_write(controller->link.context, value);
        break;
    case KBC_PENDING_AUX_DISCARD:
        controller->data.pending_write = KBC_PENDING_NONE;
        break;
    default:
        controller->link.keyboard_write(controller->link.context, value);
        break;
    }
    controller->data.input_buffer_full = LIB_FALSE;
    controller->data.last_write_command = LIB_FALSE;
}

void x86_kbc8042_write_command(x86_kbc8042 *controller, lib_u8 command)
{
    if (controller == LIB_NULL) return;
    controller->data.input_buffer_full = LIB_TRUE;
    controller->data.last_write_command = LIB_TRUE;
    switch (command) {
    case 0x20u:
        x86_kbc8042_schedule_response_byte(controller,
            controller->data.command_byte, X86_KBC8042_OUTPUT_CONTROLLER);
        break;
    case 0x60u:
        controller->link.keyboard_cancel_parameter(controller->link.context);
        controller->data.pending_write = KBC_PENDING_COMMAND_BYTE;
        break;
    case 0xaau:
        /* IBM PC/AT initialization specifies that a successful controller
         * self-test inhibits the keyboard interface before reporting 55h.
         * The host must explicitly re-enable it with AEh or a command byte. */
        x86_kbc8042_flush_output(controller);
        x86_kbc8042_set_command_byte(controller,
            controller->data.command_byte | KBC_COMMAND_DISABLE_KEYBOARD);
        controller->data.command_byte |= KBC_COMMAND_SYSTEM;
        x86_kbc8042_schedule_response_byte(controller, 0x55u,
            X86_KBC8042_OUTPUT_CONTROLLER);
        break;
    case 0xabu:
        x86_kbc8042_schedule_response_byte(controller, 0x00u,
            X86_KBC8042_OUTPUT_CONTROLLER);
        break;
    case 0xadu:
        x86_kbc8042_set_command_byte(controller,
            controller->data.command_byte | KBC_COMMAND_DISABLE_KEYBOARD);
        break;
    case 0xaeu:
        x86_kbc8042_set_command_byte(controller,
            controller->data.command_byte & ~KBC_COMMAND_DISABLE_KEYBOARD);
        break;
    case 0xa7u:
        controller->data.aux_enabled = LIB_FALSE;
        controller->data.command_byte |= KBC_COMMAND_DISABLE_AUX;
        x86_kbc8042_refresh_current_irq(controller);
        break;
    case 0xa8u:
        controller->data.aux_enabled = controller->aux_present;
        if (controller->aux_present) {
            controller->data.command_byte &= ~KBC_COMMAND_DISABLE_AUX;
        } else {
            controller->data.command_byte |= KBC_COMMAND_DISABLE_AUX;
        }
        x86_kbc8042_refresh_current_irq(controller);
        break;
    case 0xa9u:
        x86_kbc8042_schedule_response_byte(controller,
            controller->aux_present ? 0x00u : 0x01u,
            X86_KBC8042_OUTPUT_CONTROLLER);
        break;
    case 0xc0u:
        x86_kbc8042_schedule_response_byte(controller,
            controller->data.input_port, X86_KBC8042_OUTPUT_CONTROLLER);
        break;
    case 0xd0u:
        x86_kbc8042_schedule_response_byte(controller,
            controller->data.output_port, X86_KBC8042_OUTPUT_CONTROLLER);
        break;
    case 0xd1u:
        controller->link.keyboard_cancel_parameter(controller->link.context);
        controller->data.pending_write = KBC_PENDING_OUTPUT_PORT;
        break;
    case 0xd4u:
        controller->link.keyboard_cancel_parameter(controller->link.context);
        controller->data.pending_write = controller->aux_present ?
            KBC_PENDING_AUX_DEVICE :
            KBC_PENDING_AUX_DISCARD;
        break;
    case 0xe0u:
        x86_kbc8042_schedule_response_byte(controller,
            controller->data.test_inputs, X86_KBC8042_OUTPUT_CONTROLLER);
        break;
    default:
        /* IBM PC/AT 8042 commands F0h--FFh pulse output-port bits selected
         * by zero command bits. Bit 0 is the reset line. The pulse must not
         * overwrite the persistent D1h output-port/A20 state. Its duration
         * belongs to the board timing contract, not this functional owner. */
        if (command >= 0xf0u && (command & 0x01u) == 0u) {
            controller->link.reset_pulse(controller->link.context);
        }
        break;
    }
    controller->data.input_buffer_full = LIB_FALSE;
    controller->data.last_write_command = LIB_FALSE;
}

void x86_kbc8042_set_reset_output_port(x86_kbc8042 *controller,
    lib_u8 value)
{
    if (controller == LIB_NULL) return;
    controller->reset_output_port = value;
    x86_kbc8042_apply_output_port(controller, value);
}

void x86_kbc8042_reset(x86_kbc8042 *controller)
{
    lib_u32 command_response_ticks;
    lib_u8 command_response_status_polls;
    lib_u32 serial_delivery_ticks;

    if (controller == LIB_NULL) return;
    command_response_ticks = controller->data.command_response_ticks;
    command_response_status_polls =
        controller->data.command_response_status_polls;
    serial_delivery_ticks = controller->data.serial_delivery_ticks;
    lib_memory_set(&controller->data, 0u, sizeof(controller->data));
    controller->data.command_response_ticks = command_response_ticks;
    controller->data.command_response_status_polls =
        command_response_status_polls;
    controller->data.serial_delivery_ticks = serial_delivery_ticks;
    controller->link.irq(controller->link.context, LIB_FALSE, LIB_FALSE);
    controller->link.irq(controller->link.context, LIB_TRUE, LIB_FALSE);
    controller->data.command_byte = KBC_COMMAND_IRQ1 |
        (controller->aux_present ? KBC_COMMAND_IRQ12 :
            KBC_COMMAND_DISABLE_AUX) |
        KBC_COMMAND_TRANSLATION;
    controller->data.output_port = controller->reset_output_port;
    controller->data.aux_enabled = controller->aux_present;
    controller->data.input_port = 0x80u;
    x86_kbc8042_apply_output_port(controller, controller->data.output_port);
}

static void x86_kbc8042_repeat(void *context, lib_u8 byte)
{
    (void)x86_kbc8042_queue_native_byte(context, byte);
}

void x86_kbc8042_advance(x86_kbc8042 *controller, lib_u64 elapsed_ticks)
{
    if (controller == LIB_NULL) return;
    if (elapsed_ticks >= controller->data.serial_delivery_remaining_ticks) {
        controller->data.serial_delivery_remaining_ticks = 0u;
    } else {
        controller->data.serial_delivery_remaining_ticks -= elapsed_ticks;
    }
    x86_kbc8042_drain_keyboard_serial(controller);
    if (controller->data.fifo_count == 0u &&
        controller->data.delayed_response_count == 0u &&
        controller->link.keyboard_take_bat(controller->link.context)) {
        (void)x86_kbc8042_enqueue(controller, KBC_BAT_OK,
            X86_KBC8042_OUTPUT_KEYBOARD);
    }
    if (controller->data.delayed_response_count != 0u &&
        controller->data.response_status_polls_remaining == 0u) {
        if (elapsed_ticks < controller->data.response_remaining_ticks) {
            controller->data.response_remaining_ticks -= elapsed_ticks;
        } else {
            controller->data.response_remaining_ticks = 0u;
            if ((controller->data.keyboard_serial_count == 0u ||
                    !controller->link.keyboard_inputs(controller->link.context).scanning ||
                    !x86_kbc8042_keyboard_scan_delivery_enabled(controller,
                        controller->data.command_byte)) &&
                controller->data.delayed_response_count <=
                KBC_FIFO_CAPACITY - controller->data.fifo_count) {
                while (controller->data.delayed_response_index <
                        controller->data.delayed_response_count) {
                    (void)x86_kbc8042_enqueue(controller,
                        controller->data.delayed_response[
                            controller->data.delayed_response_index],
                        controller->data.delayed_response_origin);
                    ++controller->data.delayed_response_index;
                }
                controller->data.delayed_response_count = 0u;
                controller->data.delayed_response_index = 0u;
            }
        }
    }
    if (controller->link.keyboard_advance(controller->link.context, elapsed_ticks,
            x86_kbc8042_repeat, controller)) {
        x86_kbc8042_drain_keyboard_serial(controller);
    }
}

lib_status x86_kbc8042_ticks_until_event(const x86_kbc8042 *controller,
    lib_u64 *out_ticks)
{
    lib_u64 ticks = LIB_UINT64_MAX;
    x86_kbc8042_keyboard_inputs keyboard;

    if (controller == LIB_NULL || out_ticks == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (controller->data.serial_delivery_remaining_ticks != 0u) {
        ticks = controller->data.serial_delivery_remaining_ticks;
    }
    keyboard = controller->link.keyboard_inputs(controller->link.context);
    if (keyboard.bat_ready && controller->data.fifo_count == 0u &&
        controller->data.delayed_response_count == 0u) {
        ticks = 0u;
    }
    if (controller->data.delayed_response_count != 0u && controller->data.fifo_count == 0u &&
        controller->data.response_status_polls_remaining == 0u &&
        controller->data.response_remaining_ticks < ticks) {
        ticks = controller->data.response_remaining_ticks;
    }
    if (keyboard.repeat_pending && keyboard.repeat_ticks < ticks) {
        ticks = keyboard.repeat_ticks;
    }
    if (ticks == LIB_UINT64_MAX) return LIB_STATUS_INVALID_STATE;
    *out_ticks = ticks;
    return LIB_STATUS_OK;
}

void x86_kbc8042_set_command_response_timing(x86_kbc8042 *controller,
    lib_u32 response_ticks)
{
    if (controller == LIB_NULL) return;
    controller->data.command_response_ticks = response_ticks;
}

void x86_kbc8042_set_command_response_status_polls(x86_kbc8042 *controller,
    lib_u8 status_polls)
{
    if (controller == LIB_NULL) return;
    controller->data.command_response_status_polls = status_polls;
}

void x86_kbc8042_set_serial_delivery_timing(x86_kbc8042 *controller,
    lib_u32 delivery_ticks)
{
    if (controller == LIB_NULL) return;
    controller->data.serial_delivery_ticks = delivery_ticks;
    controller->data.serial_delivery_remaining_ticks = 0u;
    x86_kbc8042_drain_keyboard_serial(controller);
}

static lib_status x86_kbc8042_admit_native_byte(x86_kbc8042 *controller,
    lib_u8 native_byte)
{
    if (controller == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (!x86_kbc8042_keyboard_scan_delivery_enabled(controller,
            controller->data.command_byte)) return LIB_STATUS_INVALID_STATE;
    return controller->link.keyboard_admit(controller->link.context, native_byte);
}

lib_status x86_kbc8042_receive_keyboard_byte(x86_kbc8042 *controller,
    lib_u8 native_byte)
{
    lib_status status;

    if (controller == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (controller->data.keyboard_serial_count >=
        KBC_KEYBOARD_SERIAL_CAPACITY) return LIB_STATUS_NO_MEMORY;
    status = x86_kbc8042_admit_native_byte(controller, native_byte);
    if (status != LIB_STATUS_OK) return status;
    status = x86_kbc8042_queue_native_byte(controller, native_byte);
    if (status != LIB_STATUS_OK) return status;
    x86_kbc8042_drain_keyboard_serial(controller);
    return LIB_STATUS_OK;
}

lib_status x86_kbc8042_receive_keyboard_bytes(x86_kbc8042 *controller,
    const lib_u8 *native_bytes, lib_size count)
{
    lib_size index;

    if (controller == LIB_NULL || (native_bytes == LIB_NULL && count != 0u)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (!controller->link.keyboard_inputs(controller->link.context).scanning ||
        !x86_kbc8042_keyboard_scan_delivery_enabled(controller,
            controller->data.command_byte)) {
        return LIB_STATUS_INVALID_STATE;
    }
    if (count > KBC_KEYBOARD_SERIAL_CAPACITY -
        controller->data.keyboard_serial_count) return LIB_STATUS_NO_MEMORY;
    for (index = 0u; index < count; ++index) {
        if (x86_kbc8042_admit_native_byte(controller, native_bytes[index]) !=
            LIB_STATUS_OK) return LIB_STATUS_INVALID_STATE;
        (void)x86_kbc8042_queue_native_byte(controller, native_bytes[index]);
    }
    x86_kbc8042_drain_keyboard_serial(controller);
    return LIB_STATUS_OK;
}

lib_status x86_kbc8042_receive_aux_packet(x86_kbc8042 *controller,
    const lib_u8 packet[3])
{
    lib_u8 index;

    if (controller == LIB_NULL || packet == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (!controller->aux_present || !controller->data.aux_enabled ||
        controller->data.delayed_response_count != 0u ||
        KBC_FIFO_CAPACITY - controller->data.fifo_count < 3u) {
        return LIB_STATUS_INVALID_STATE;
    }
    for (index = 0u; index < 3u; ++index) {
        (void)x86_kbc8042_enqueue(controller, packet[index],
            X86_KBC8042_OUTPUT_AUX);
    }
    return LIB_STATUS_OK;
}

lib_status x86_kbc8042_create(const x86_kbc8042_link *link, x86_kbc8042 **out_controller)
{
    x86_kbc8042 *controller;
    if (out_controller == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_controller = LIB_NULL;
    if (link == LIB_NULL || link->keyboard_inputs == LIB_NULL ||
        link->keyboard_start == LIB_NULL || link->keyboard_take_bat == LIB_NULL ||
        link->keyboard_clear_bat == LIB_NULL || link->keyboard_cancel_parameter == LIB_NULL ||
        link->keyboard_write == LIB_NULL || link->aux_write == LIB_NULL ||
        link->keyboard_accepted == LIB_NULL || link->keyboard_admit == LIB_NULL ||
        link->keyboard_advance == LIB_NULL || link->irq == LIB_NULL ||
        link->output_port == LIB_NULL || link->reset_pulse == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    controller = lib_allocate_zero(1u, sizeof(*controller));
    if (controller == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    controller->link = *link;
    controller->aux_present = LIB_TRUE;
    controller->reset_output_port = 1u;
    x86_kbc8042_reset(controller);
    *out_controller = controller;
    return LIB_STATUS_OK;
}

void x86_kbc8042_destroy(x86_kbc8042 *controller)
{
    if (controller == LIB_NULL) return;
    x86_kbc8042_deassert_irq1(controller);
    x86_kbc8042_deassert_irq12(controller);
    lib_release(controller);
}

void x86_kbc8042_set_aux_present(x86_kbc8042 *controller, lib_bool present)
{
    if (controller == LIB_NULL) return;
    controller->aux_present = present;
    if (!present) {
        controller->data.aux_enabled = LIB_FALSE;
        controller->data.command_byte &= ~KBC_COMMAND_IRQ12;
        controller->data.command_byte |= KBC_COMMAND_DISABLE_AUX;
        x86_kbc8042_deassert_irq12(controller);
    }
}

lib_bool x86_kbc8042_aux_enabled(const x86_kbc8042 *controller)
{
    return controller != LIB_NULL && controller->aux_present && controller->data.aux_enabled;
}
