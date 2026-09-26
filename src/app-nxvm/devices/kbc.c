/* Copyright 2012-2014 Neko. */
#include "lib/types/types_interface.h"
#include "app-nxvm/devices/device_support.h"


#include "app-nxvm/devices/machine_interface.h"

#include "app-nxvm/devices/cpu.h"
#include "app-nxvm/devices/memory.h"
#include "app-nxvm/devices/pic.h"
#include "app-nxvm/devices/port.h"

#include "app-nxvm/devices/kbc.h"

#define CORE_MACHINE_KBC_COMMAND_IRQ1 0x01u
#define CORE_MACHINE_KBC_COMMAND_SYSTEM 0x04u
#define CORE_MACHINE_KBC_OUTPUT_RESET 0x01u
#define CORE_MACHINE_KBC_OUTPUT_A20 0x02u

#define CORE_MACHINE_KBC_ACK 0xfau
#define CORE_MACHINE_KBC_BAT_OK 0xaau
#define CORE_MACHINE_KBC_IDENTIFY_0 0xabu
#define CORE_MACHINE_KBC_IDENTIFY_1 0x83u
#define CORE_MACHINE_KBC_IDENTIFY_1_TRANSLATED 0x41u
#define CORE_MACHINE_KBC_RESEND 0xfeu
#define CORE_MACHINE_KBC_ECHO 0xeeu
#define CORE_MACHINE_KBC_DEFAULT_TYPEMATIC 0x2cu
#define CORE_MACHINE_KBC_DEFAULT_DELAY_FACTOR 2u
#define CORE_MACHINE_KBC_DEFAULT_RATE_UNITS 24u
#define CORE_MACHINE_KBC_AUX_DEFAULT_RESOLUTION 2u
#define CORE_MACHINE_KBC_AUX_DEFAULT_SAMPLE_RATE 100u
#define CORE_MACHINE_KBC_AUX_STATUS_REPORTING 0x20u
#define CORE_MACHINE_KBC_AUX_STATUS_SCALING_2_TO_1 0x10u

static void core_machine_kbc_drain_keyboard_serial(t_kbc *controller);
static void core_machine_kbc_refresh_current_irq(t_kbc *controller);

/* Ordinary input and automatic repeats share the keyboard-side backlog.
 * Only drain_keyboard_serial may promote a scan byte into the output buffer. */
static lib_status core_machine_kbc_queue_native_byte(t_kbc *controller,
    lib_u8 native_byte)
{
    lib_u8 tail;

    if (controller->data.keyboard_serial_count >=
        CORE_MACHINE_KBC_KEYBOARD_SERIAL_CAPACITY) return LIB_STATUS_NO_MEMORY;
    tail = (lib_u8)((controller->data.keyboard_serial_head +
        controller->data.keyboard_serial_count) % CORE_MACHINE_KBC_KEYBOARD_SERIAL_CAPACITY);
    controller->data.keyboard_serial[tail] = native_byte;
    ++controller->data.keyboard_serial_count;
    if (controller->data.serial_delivery_ticks != 0u &&
        controller->data.serial_delivery_remaining_ticks == 0u) {
        controller->data.serial_delivery_remaining_ticks = controller->data.serial_delivery_ticks;
    }
    return LIB_STATUS_OK;
}

static void core_machine_kbc_deassert_irq1(t_kbc *controller)
{
    if (controller == LIB_NULL || !controller->data.irq1_asserted) return;
    core_machine_pic_irq_source_deassert(&controller->connect.irq1_source);
    controller->data.irq1_asserted = LIB_FALSE;
}

static void core_machine_kbc_deassert_irq12(t_kbc *controller)
{
    if (controller == LIB_NULL || !controller->data.irq12_asserted) return;
    core_machine_pic_irq_source_deassert(&controller->connect.irq12_source);
    controller->data.irq12_asserted = LIB_FALSE;
}

/* C0h bit 7 is the board keyboard-inhibit switch.  It gates ordinary scan
 * codes unless command-byte bit 3 overrides the switch; it is not the 8042
 * data line and therefore cannot suppress the keyboard's BAT edge. */
static lib_u8 core_machine_kbc_keyboard_scan_delivery_enabled(const t_kbc *controller,
    lib_u8 command_byte)
{
    return controller != LIB_NULL &&
        (command_byte & CORE_MACHINE_KBC_COMMAND_DISABLE_KEYBOARD) == 0u &&
        ((command_byte & CORE_MACHINE_KBC_COMMAND_INHIBIT_OVERRIDE) != 0u ||
        (controller->data.input_port & 0x80u) != 0u);
}

static void core_machine_kbc_set_command_byte(t_kbc *controller,
    lib_u8 value)
{
    const lib_u8 previous_command_byte = controller->data.command_byte;
    const lib_u8 keyboard_clock_was_enabled =
        (previous_command_byte & CORE_MACHINE_KBC_COMMAND_DISABLE_KEYBOARD) == 0u;

    controller->data.command_byte = value & 0x7du;
    if (controller->connect.aux_present) {
        controller->data.command_byte |= value & CORE_MACHINE_KBC_COMMAND_IRQ12;
    } else {
        controller->data.command_byte |= CORE_MACHINE_KBC_COMMAND_DISABLE_AUX;
    }
    controller->data.keyboard_enabled =
        (controller->data.command_byte & CORE_MACHINE_KBC_COMMAND_DISABLE_KEYBOARD) == 0u;
    controller->data.aux_enabled = controller->connect.aux_present &&
        (controller->data.command_byte & CORE_MACHINE_KBC_COMMAND_DISABLE_AUX) == 0u;
    /* The IBM AT keyboard starts BAT when the controller first either raises
     * the command-byte inhibit override (45h -> 4Dh) or releases a previously
     * held clock while the board switch permits the interface (AEh).  The
     * switch is an input observation; it is not a synthetic data-line state. */
    if (!controller->data.keyboard_startup_released &&
        (controller->data.command_byte & CORE_MACHINE_KBC_COMMAND_DISABLE_KEYBOARD) == 0u &&
        (((previous_command_byte & CORE_MACHINE_KBC_COMMAND_INHIBIT_OVERRIDE) == 0u &&
            (controller->data.command_byte & CORE_MACHINE_KBC_COMMAND_INHIBIT_OVERRIDE) != 0u) ||
         (!keyboard_clock_was_enabled &&
            (controller->data.input_port & 0x80u) != 0u))) {
        controller->data.keyboard_startup_released = LIB_TRUE;
        controller->data.keyboard_bat_pending = LIB_TRUE;
        core_machine_kbc_advance(controller, 0u);
    }
    core_machine_kbc_refresh_current_irq(controller);
}

static void core_machine_kbc_refresh_current_irq(t_kbc *controller)
{
    core_machine_kbc_output_origin origin;

    if (controller == LIB_NULL || controller->data.fifo_count == 0u) {
        core_machine_kbc_deassert_irq1(controller);
        core_machine_kbc_deassert_irq12(controller);
        return;
    }
    origin = controller->data.fifo_origin[controller->data.fifo_head];
    if (origin == CORE_MACHINE_KBC_OUTPUT_KEYBOARD) {
        core_machine_kbc_deassert_irq12(controller);
        /* ADh inhibits new keyboard serial input.  It does not suppress an
         * already-generated keyboard-controller reply: IBM's POST sends ADh,
         * resets the keyboard, then waits for BAT AAh through IRQ1. */
        if (!controller->data.irq1_asserted &&
            (controller->data.command_byte & CORE_MACHINE_KBC_COMMAND_IRQ1) != 0u) {
            core_machine_pic_irq_source_assert(&controller->connect.irq1_source);
            controller->data.irq1_asserted = LIB_TRUE;
        }
    } else if (origin == CORE_MACHINE_KBC_OUTPUT_AUX) {
        core_machine_kbc_deassert_irq1(controller);
        if (!controller->data.irq12_asserted && controller->connect.aux_present &&
            controller->data.aux_enabled &&
            (controller->data.command_byte & CORE_MACHINE_KBC_COMMAND_IRQ12) != 0u &&
            (controller->data.command_byte & CORE_MACHINE_KBC_COMMAND_DISABLE_AUX) == 0u) {
            core_machine_pic_irq_source_assert(&controller->connect.irq12_source);
            controller->data.irq12_asserted = LIB_TRUE;
        }
    } else {
        core_machine_kbc_deassert_irq1(controller);
        core_machine_kbc_deassert_irq12(controller);
    }
}

/* An 8042 self-test starts from an empty output path.  In particular, an
 * older keyboard byte must not be mistaken for the command's 55h result. */
static void core_machine_kbc_flush_output(t_kbc *controller)
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
    controller->data.keyboard_bat_pending = LIB_FALSE;
    core_machine_kbc_deassert_irq1(controller);
    core_machine_kbc_deassert_irq12(controller);
}

static lib_status core_machine_kbc_enqueue(t_kbc *controller, lib_u8 value,
    core_machine_kbc_output_origin origin)
{
    lib_u8 tail;

    if (controller == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (controller->data.fifo_count >= CORE_MACHINE_KBC_FIFO_CAPACITY) {
        return LIB_STATUS_INVALID_STATE;
    }
    tail = (lib_u8)((controller->data.fifo_head +
        controller->data.fifo_count) % CORE_MACHINE_KBC_FIFO_CAPACITY);
    controller->data.fifo[tail] = value;
    controller->data.fifo_origin[tail] = origin;
    ++controller->data.fifo_count;
    if (origin == CORE_MACHINE_KBC_OUTPUT_KEYBOARD) {
        if (controller->data.keyboard_has_output) {
            controller->data.previous_keyboard_output_byte =
                controller->data.last_keyboard_output_byte;
        }
        controller->data.last_keyboard_output_byte = value;
        controller->data.keyboard_has_output = LIB_TRUE;
    }
    core_machine_kbc_refresh_current_irq(controller);
    return LIB_STATUS_OK;
}

static void core_machine_kbc_schedule_response(t_kbc *controller,
    const lib_u8 *bytes, lib_u8 count, core_machine_kbc_output_origin origin)
{
    lib_u8 index;

    if (controller == LIB_NULL || bytes == LIB_NULL || count == 0u ||
        count > CORE_MACHINE_KBC_RESPONSE_CAPACITY ||
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
        core_machine_kbc_advance(controller, 0u);
    }
}

static void core_machine_kbc_schedule_response_byte(t_kbc *controller,
    lib_u8 value, core_machine_kbc_output_origin origin)
{
    core_machine_kbc_schedule_response(controller, &value, 1u, origin);
}

static void core_machine_kbc_apply_typematic_timing(t_kbc *controller)
{
    lib_u32 delay_factor;
    lib_u32 rate_units;

    if (controller == LIB_NULL) return;
    delay_factor = 1u + ((controller->data.typematic >> 5u) & 0x03u);
    rate_units = (8u + (controller->data.typematic & 0x07u)) <<
        ((controller->data.typematic >> 3u) & 0x03u);
    controller->data.typematic_initial_ticks = (lib_u32)(
        ((lib_u64)controller->data.typematic_nominal_initial_ticks *
            delay_factor) / CORE_MACHINE_KBC_DEFAULT_DELAY_FACTOR);
    controller->data.typematic_repeat_ticks = (lib_u32)(
        ((lib_u64)controller->data.typematic_nominal_repeat_ticks *
            rate_units) / CORE_MACHINE_KBC_DEFAULT_RATE_UNITS);
}

static void core_machine_kbc_set_typematic(t_kbc *controller, lib_u8 value)
{
    if (controller == LIB_NULL) return;
    controller->data.typematic = value;
    core_machine_kbc_apply_typematic_timing(controller);
}

static void core_machine_kbc_set_defaults(t_kbc *controller)
{
    if (controller == LIB_NULL) return;
    /* The selected 101-key AT keyboard emits Set 2; the 8042's command-byte
     * translation, when enabled by firmware, is the separate guest boundary. */
    controller->data.scan_set = CORE_MACHINE_KEYBOARD_SCAN_SET_2;
    controller->data.led_state = 0u;
    core_machine_kbc_set_typematic(controller, CORE_MACHINE_KBC_DEFAULT_TYPEMATIC);
    controller->data.typematic_active = LIB_FALSE;
    controller->data.typematic_remaining_ticks = 0u;
    controller->data.typematic_scan_code = 0u;
    controller->data.set2_break_pending = LIB_FALSE;
    controller->data.set2_typematic_break_pending = LIB_FALSE;
    controller->data.set2_extended_pending = LIB_FALSE;
    controller->data.set2_pause_count = 0u;
}

static lib_u8 core_machine_kbc_set2_to_set1(lib_u8 set2,
    lib_u8 *out_known)
{
    static const lib_u8 map[0x84] = {
        [0x01] = 0x43u, [0x03] = 0x3fu, [0x04] = 0x3du, [0x05] = 0x3bu,
        [0x06] = 0x3cu, [0x07] = 0x58u, [0x09] = 0x44u, [0x0a] = 0x42u,
        [0x0b] = 0x40u, [0x0c] = 0x3eu, [0x0d] = 0x0fu, [0x0e] = 0x29u,
        [0x11] = 0x38u, [0x12] = 0x2au, [0x14] = 0x1du, [0x15] = 0x10u,
        [0x16] = 0x02u, [0x1a] = 0x2cu, [0x1b] = 0x1fu, [0x1c] = 0x1eu,
        [0x1d] = 0x11u, [0x1e] = 0x03u, [0x21] = 0x2eu, [0x22] = 0x2du,
        [0x23] = 0x20u, [0x24] = 0x12u, [0x25] = 0x05u, [0x26] = 0x04u,
        [0x29] = 0x39u, [0x2a] = 0x2fu, [0x2b] = 0x21u, [0x2c] = 0x14u,
        [0x2d] = 0x13u, [0x2e] = 0x06u, [0x31] = 0x31u, [0x32] = 0x30u,
        [0x33] = 0x23u, [0x34] = 0x22u, [0x35] = 0x15u, [0x36] = 0x07u,
        [0x3a] = 0x32u, [0x3b] = 0x24u, [0x3c] = 0x16u, [0x3d] = 0x08u,
        [0x3e] = 0x09u, [0x41] = 0x33u, [0x42] = 0x25u, [0x43] = 0x17u,
        [0x44] = 0x18u, [0x45] = 0x0bu, [0x46] = 0x0au, [0x49] = 0x34u,
        [0x4a] = 0x35u, [0x4b] = 0x26u, [0x4c] = 0x27u, [0x4d] = 0x19u,
        [0x4e] = 0x0cu, [0x52] = 0x28u, [0x54] = 0x1au, [0x55] = 0x0du,
        [0x58] = 0x3au, [0x59] = 0x36u, [0x5a] = 0x1cu, [0x5b] = 0x1bu,
        [0x5d] = 0x2bu, [0x66] = 0x0eu, [0x69] = 0x4fu, [0x6b] = 0x4bu,
        [0x6c] = 0x47u, [0x70] = 0x52u, [0x71] = 0x53u, [0x72] = 0x50u,
        [0x73] = 0x4cu, [0x74] = 0x4du, [0x75] = 0x48u, [0x76] = 0x01u,
        [0x77] = 0x45u, [0x78] = 0x57u, [0x79] = 0x4eu, [0x7a] = 0x51u,
        [0x7b] = 0x4au, [0x7c] = 0x37u, [0x7d] = 0x49u, [0x7e] = 0x46u,
        [0x83] = 0x41u
    };

    if (out_known == LIB_NULL) return 0u;
    *out_known = set2 < sizeof(map) && map[set2] != 0u;
    return *out_known ? map[set2] : set2;
}

static lib_status core_machine_kbc_enqueue_set1_pause(t_kbc *controller)
{
    static const lib_u8 pause[] = { 0xe1u, 0x1du, 0x45u,
        0xe1u, 0x9du, 0xc5u };
    lib_size index;

    if (controller == LIB_NULL || CORE_MACHINE_KBC_FIFO_CAPACITY -
        controller->data.fifo_count < sizeof(pause)) return LIB_STATUS_INVALID_STATE;
    for (index = 0u; index < sizeof(pause); ++index) {
        (void)core_machine_kbc_enqueue(controller, pause[index],
            CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
    }
    return LIB_STATUS_OK;
}

static lib_status core_machine_kbc_translate_set2_byte(t_kbc *controller,
    lib_u8 native_byte)
{
    static const lib_u8 pause_set2[] = { 0xe1u, 0x14u, 0x77u,
        0xe1u, 0xf0u, 0x14u, 0xf0u, 0x77u };
    lib_u8 known;
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
        return core_machine_kbc_enqueue_set1_pause(controller);
    }
    if (native_byte == 0xe0u) {
        controller->data.set2_extended_pending = LIB_TRUE;
        return core_machine_kbc_enqueue(controller, native_byte,
            CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
    }
    if (native_byte == 0xf0u) {
        controller->data.set2_break_pending = LIB_TRUE;
        return LIB_STATUS_OK;
    }
    translated = core_machine_kbc_set2_to_set1(native_byte, &known);
    if (!known) return LIB_STATUS_UNSUPPORTED;
    if (controller->data.set2_break_pending) translated |= 0x80u;
    controller->data.set2_break_pending = LIB_FALSE;
    controller->data.set2_extended_pending = LIB_FALSE;
    return core_machine_kbc_enqueue(controller, translated,
        CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
}

static lib_status core_machine_kbc_publish_native_byte(t_kbc *controller,
    lib_u8 native_byte)
{
    if (controller == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (controller->data.scan_set == CORE_MACHINE_KEYBOARD_SCAN_SET_2 &&
        (controller->data.command_byte & CORE_MACHINE_KBC_COMMAND_TRANSLATION) != 0u) {
        return core_machine_kbc_translate_set2_byte(controller, native_byte);
    }
    return core_machine_kbc_enqueue(controller, native_byte,
        CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
}

static void core_machine_kbc_drain_keyboard_serial(t_kbc *controller)
{
    lib_u8 native_byte;
    lib_status status;

    if (controller == LIB_NULL ||
        controller->data.serial_delivery_remaining_ticks != 0u ||
        !controller->data.scanning_enabled ||
        !core_machine_kbc_keyboard_scan_delivery_enabled(controller,
            controller->data.command_byte)) return;
    /* The keyboard serial stream may have private backlog, but only one
     * scan byte may enter the controller output path at a time.  Firmware
     * such as the 5170 POST disables the keyboard and clears one OBF byte;
     * admitting a whole host key chord past that boundary is not hardware. */
    if (controller->data.keyboard_serial_count != 0u &&
        controller->data.fifo_count == 0u) {
        native_byte = controller->data.keyboard_serial[
            controller->data.keyboard_serial_head];
        status = core_machine_kbc_publish_native_byte(controller, native_byte);
        if (status != LIB_STATUS_OK && status != LIB_STATUS_UNSUPPORTED) return;
        controller->data.keyboard_serial_head = (lib_u8)(
            (controller->data.keyboard_serial_head + 1u) %
            CORE_MACHINE_KBC_KEYBOARD_SERIAL_CAPACITY);
        --controller->data.keyboard_serial_count;
        if (controller->data.serial_delivery_ticks != 0u) {
            controller->data.serial_delivery_remaining_ticks =
                controller->data.serial_delivery_ticks;
        }
    }
}
static lib_u8 core_machine_kbc_is_typematic_scan_code(lib_u8 scan_code)
{
    switch (scan_code) {
    case 0x1du: case 0x2au: case 0x36u: case 0x38u:
    case 0x3au: case 0x45u: case 0x46u:
        return LIB_FALSE;
    default:
        return scan_code != 0xe0u && scan_code != 0xe1u;
    }
}

static lib_u8 core_machine_kbc_dequeue(t_kbc *controller)
{
    lib_u8 value = 0u;
    core_machine_kbc_output_origin origin;

    if (controller == LIB_NULL || controller->data.fifo_count == 0u) return 0u;
    value = controller->data.fifo[controller->data.fifo_head];
    origin = controller->data.fifo_origin[controller->data.fifo_head];
    controller->data.fifo_head = (lib_u8)((controller->data.fifo_head + 1u) %
        CORE_MACHINE_KBC_FIFO_CAPACITY);
    --controller->data.fifo_count;
    /* Reading 60h acknowledges exactly the current origin. A queued successor
     * gets a fresh edge only after promotion through the one PIC boundary. */
    if (origin == CORE_MACHINE_KBC_OUTPUT_KEYBOARD) {
        core_machine_kbc_deassert_irq1(controller);
    } else if (origin == CORE_MACHINE_KBC_OUTPUT_AUX) {
        core_machine_kbc_deassert_irq12(controller);
    }
    core_machine_kbc_refresh_current_irq(controller);
    /* Consuming ACK makes its pending BAT eligible for the empty output
     * buffer. PIC masking/delivery owns when the resulting IRQ is serviced. */
    core_machine_kbc_advance(controller, 0u);
    core_machine_kbc_drain_keyboard_serial(controller);
    return value;
}

static lib_u8 core_machine_kbc_status(const t_kbc *controller)
{
    lib_u8 status = 0u;

    if (controller == LIB_NULL) return status;
    if (controller->data.fifo_count != 0u) status |= VKBC_STATUS_OBF;
    if (controller->data.input_buffer_full) status |= VKBC_STATUS_IBF;
    if ((controller->data.command_byte & CORE_MACHINE_KBC_COMMAND_SYSTEM) != 0u) {
        status |= VKBC_STATUS_SYS;
    }
    if (controller->data.last_write_command) status |= VKBC_STATUS_CD;
    if ((controller->data.input_port & 0x80u) != 0u) status |= VKBC_STATUS_INHIBIT;
    if (controller->data.fifo_count != 0u &&
        controller->data.fifo_origin[controller->data.fifo_head] ==
            CORE_MACHINE_KBC_OUTPUT_AUX) status |= VKBC_STATUS_AUX;
    return status;
}

static void core_machine_kbc_apply_output_port(t_kbc *controller, lib_u8 value)
{
    if (controller == LIB_NULL) return;
    controller->data.output_port = value;
    if (controller->connect.memory != LIB_NULL) {
        controller->connect.memory->data.flagA20 =
            (value & CORE_MACHINE_KBC_OUTPUT_A20) != 0u;
    }
    if ((value & CORE_MACHINE_KBC_OUTPUT_RESET) == 0u &&
        controller->connect.execution != LIB_NULL) {
        core_machine_cpu_execution_request_reset(controller->connect.execution);
    }
}

static void core_machine_kbc_handle_keyboard_command(t_kbc *controller,
    lib_u8 command)
{
    lib_u8 identify[] = { CORE_MACHINE_KBC_ACK, CORE_MACHINE_KBC_IDENTIFY_0,
        CORE_MACHINE_KBC_IDENTIFY_1 };
    static const lib_u8 reset_ok[] = {
        CORE_MACHINE_KBC_ACK, CORE_MACHINE_KBC_BAT_OK
    };
    lib_u8 resend;

    switch (command) {
    case 0xffu:
        core_machine_kbc_schedule_response_byte(controller, reset_ok[0u],
            CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
        /* An explicit keyboard reset already owns its BAT completion.  Do not
         * manufacture a second power-on BAT when firmware subsequently
         * releases the controller line: Compaq's POST performs precisely
         * that FFh/AEh sequence and treats an extra byte as a 301 failure. */
        controller->data.keyboard_startup_released = LIB_TRUE;
        controller->data.keyboard_bat_pending = LIB_TRUE;
        core_machine_kbc_set_defaults(controller);
        controller->data.scanning_enabled = LIB_TRUE;
        break;
    case 0xedu:
        core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_ACK,
            CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
        controller->data.pending_write = CORE_MACHINE_KBC_PENDING_KEYBOARD_LEDS;
        break;
    case 0xeeu:
        core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_ECHO,
            CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
        break;
    case 0xf0u:
        core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_ACK,
            CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
        controller->data.pending_write = CORE_MACHINE_KBC_PENDING_KEYBOARD_SCAN_SET;
        break;
    case 0xf3u:
        core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_ACK,
            CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
        controller->data.pending_write = CORE_MACHINE_KBC_PENDING_KEYBOARD_TYPEMATIC;
        break;
    case 0xf4u:
        controller->data.scanning_enabled = LIB_TRUE;
        controller->data.typematic_active = LIB_FALSE;
        controller->data.typematic_scan_code = 0u;
        core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_ACK,
            CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
        break;
    case 0xf5u:
        core_machine_kbc_set_defaults(controller);
        controller->data.scanning_enabled = LIB_FALSE;
        core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_ACK,
            CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
        break;
    case 0xf6u:
        core_machine_kbc_set_defaults(controller);
        core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_ACK,
            CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
        break;
    case 0xf2u:
        /* The keyboard's native MF-II identity ends in 83h.  The 8042
         * translation selection is also observable on this reply: AT BIOSes
         * see 41h when translation is enabled. */
        if ((controller->data.command_byte & CORE_MACHINE_KBC_COMMAND_TRANSLATION) != 0u) {
            identify[2u] = CORE_MACHINE_KBC_IDENTIFY_1_TRANSLATED;
        }
        core_machine_kbc_schedule_response(controller, identify, sizeof(identify),
            CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
        break;
    case 0xfeu:
        resend = controller->data.last_keyboard_output_byte;
        if (resend == CORE_MACHINE_KBC_RESEND &&
            controller->data.keyboard_has_output) {
            resend = controller->data.previous_keyboard_output_byte;
        }
        core_machine_kbc_schedule_response_byte(controller, resend,
            CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
        break;
    case 0xfdu:
    case 0xfcu:
    case 0xfbu:
    case 0xf7u:
        core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_ACK,
            CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
        break;
    default:
        core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_RESEND,
            CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
        break;
    }
}

void core_machine_kbc_set_input_port(t_kbc *controller, lib_u8 value)
{
    if (controller != LIB_NULL) controller->data.input_port = value;
}

void core_machine_kbc_set_test_inputs(t_kbc *controller, lib_u8 value)
{
    if (controller != LIB_NULL) controller->data.test_inputs = value & 0x03u;
}

static void core_machine_kbc_set_aux_defaults(t_kbc *controller)
{
    if (controller == LIB_NULL) return;
    controller->data.aux_reporting_enabled = LIB_FALSE;
    controller->data.aux_scaling_2_to_1 = LIB_FALSE;
    controller->data.aux_button_state = 0u;
    controller->data.aux_resolution = CORE_MACHINE_KBC_AUX_DEFAULT_RESOLUTION;
    controller->data.aux_sample_rate = CORE_MACHINE_KBC_AUX_DEFAULT_SAMPLE_RATE;
    controller->data.aux_pending_parameter = CORE_MACHINE_KBC_AUX_PENDING_NONE;
}

static lib_u8 core_machine_kbc_is_aux_sample_rate(lib_u8 value)
{
    switch (value) {
    case 10u: case 20u: case 40u: case 60u: case 80u: case 100u: case 200u:
        return LIB_TRUE;
    default:
        return LIB_FALSE;
    }
}

static lib_u8 core_machine_kbc_aux_status(const t_kbc *controller)
{
    lib_u8 status;

    if (controller == LIB_NULL) return 0u;
    status = controller->data.aux_button_state & 0x07u;
    if (controller->data.aux_reporting_enabled) {
        status |= CORE_MACHINE_KBC_AUX_STATUS_REPORTING;
    }
    if (controller->data.aux_scaling_2_to_1) {
        status |= CORE_MACHINE_KBC_AUX_STATUS_SCALING_2_TO_1;
    }
    return status;
}

static void core_machine_kbc_handle_aux_command(t_kbc *controller,
    lib_u8 command)
{
    static const lib_u8 identify[] = { CORE_MACHINE_KBC_ACK, 0x00u };
    static const lib_u8 reset_ok[] = {
        CORE_MACHINE_KBC_ACK, CORE_MACHINE_KBC_BAT_OK, 0x00u
    };
    lib_u8 status_reply[4];

    if (controller == LIB_NULL) return;
    if (controller->data.aux_pending_parameter ==
        CORE_MACHINE_KBC_AUX_PENDING_SAMPLE_RATE) {
        controller->data.aux_pending_parameter = CORE_MACHINE_KBC_AUX_PENDING_NONE;
        if (core_machine_kbc_is_aux_sample_rate(command)) {
            controller->data.aux_sample_rate = command;
            core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_ACK,
                CORE_MACHINE_KBC_OUTPUT_AUX);
        } else {
            core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_RESEND,
                CORE_MACHINE_KBC_OUTPUT_AUX);
        }
        return;
    }
    if (controller->data.aux_pending_parameter ==
        CORE_MACHINE_KBC_AUX_PENDING_RESOLUTION) {
        controller->data.aux_pending_parameter = CORE_MACHINE_KBC_AUX_PENDING_NONE;
        if (command <= 3u) {
            controller->data.aux_resolution = command;
            core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_ACK,
                CORE_MACHINE_KBC_OUTPUT_AUX);
        } else {
            core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_RESEND,
                CORE_MACHINE_KBC_OUTPUT_AUX);
        }
        return;
    }
    switch (command) {
    case 0xffu:
        core_machine_kbc_set_aux_defaults(controller);
        core_machine_kbc_schedule_response(controller, reset_ok, sizeof(reset_ok),
            CORE_MACHINE_KBC_OUTPUT_AUX);
        break;
    case 0xf6u:
        core_machine_kbc_set_aux_defaults(controller);
        core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_ACK,
            CORE_MACHINE_KBC_OUTPUT_AUX);
        break;
    case 0xf5u:
        controller->data.aux_reporting_enabled = LIB_FALSE;
        core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_ACK,
            CORE_MACHINE_KBC_OUTPUT_AUX);
        break;
    case 0xf4u:
        controller->data.aux_reporting_enabled = LIB_TRUE;
        core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_ACK,
            CORE_MACHINE_KBC_OUTPUT_AUX);
        break;
    case 0xf2u:
        core_machine_kbc_schedule_response(controller, identify, sizeof(identify),
            CORE_MACHINE_KBC_OUTPUT_AUX);
        break;
    case 0xf3u:
        core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_ACK,
            CORE_MACHINE_KBC_OUTPUT_AUX);
        controller->data.aux_pending_parameter =
            CORE_MACHINE_KBC_AUX_PENDING_SAMPLE_RATE;
        break;
    case 0xe8u:
        core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_ACK,
            CORE_MACHINE_KBC_OUTPUT_AUX);
        controller->data.aux_pending_parameter =
            CORE_MACHINE_KBC_AUX_PENDING_RESOLUTION;
        break;
    case 0xe9u:
        status_reply[0] = CORE_MACHINE_KBC_ACK;
        status_reply[1] = core_machine_kbc_aux_status(controller);
        status_reply[2] = controller->data.aux_resolution;
        status_reply[3] = controller->data.aux_sample_rate;
        core_machine_kbc_schedule_response(controller, status_reply,
            sizeof(status_reply), CORE_MACHINE_KBC_OUTPUT_AUX);
        break;
    default:
        core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_RESEND,
            CORE_MACHINE_KBC_OUTPUT_AUX);
        break;
    }
}

static void core_machine_kbc_read_data(t_port *port, lib_u16 port_id,
    void *owner)
{
    t_kbc *controller = (t_kbc *)owner;

    (void)port_id;
    port->data.ioByte = core_machine_kbc_dequeue(controller);
}

static void core_machine_kbc_read_status(t_port *port, lib_u16 port_id,
    void *owner)
{
    t_kbc *controller = (t_kbc *)owner;
    lib_u8 status;

    (void)port_id;
    status = core_machine_kbc_status(controller);
    if (controller != LIB_NULL && controller->data.delayed_response_count != 0u &&
        controller->data.response_remaining_ticks == 0u &&
        controller->data.response_status_polls_remaining != 0u) {
        --controller->data.response_status_polls_remaining;
        if (controller->data.response_status_polls_remaining == 0u) {
            core_machine_kbc_advance(controller, 0u);
        }
    }
    port->data.ioByte = status;
}

static void core_machine_kbc_write_data(t_port *port, lib_u16 port_id,
    void *owner)
{
    t_kbc *controller = (t_kbc *)owner;
    lib_u8 value = port->data.ioByte;

    (void)port_id;
    if (controller == LIB_NULL) return;
    controller->data.input_buffer_full = LIB_TRUE;
    controller->data.last_write_command = LIB_FALSE;
    switch (controller->data.pending_write) {
    case CORE_MACHINE_KBC_PENDING_COMMAND_BYTE:
    {
        controller->data.pending_write = CORE_MACHINE_KBC_PENDING_NONE;
        core_machine_kbc_set_command_byte(controller, value);
        break;
    }
    case CORE_MACHINE_KBC_PENDING_OUTPUT_PORT:
        core_machine_kbc_apply_output_port(controller, value);
        controller->data.pending_write = CORE_MACHINE_KBC_PENDING_NONE;
        break;
    case CORE_MACHINE_KBC_PENDING_KEYBOARD_LEDS:
        controller->data.led_state = value & 0x07u;
        controller->data.pending_write = CORE_MACHINE_KBC_PENDING_NONE;
        core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_ACK,
            CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
        break;
    case CORE_MACHINE_KBC_PENDING_KEYBOARD_TYPEMATIC:
        core_machine_kbc_set_typematic(controller, value);
        controller->data.pending_write = CORE_MACHINE_KBC_PENDING_NONE;
        core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_ACK,
            CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
        break;
    case CORE_MACHINE_KBC_PENDING_KEYBOARD_SCAN_SET:
        controller->data.pending_write = CORE_MACHINE_KBC_PENDING_NONE;
        if (value == 0u) {
            lib_u8 response[] = { CORE_MACHINE_KBC_ACK,
                controller->data.scan_set };
            core_machine_kbc_schedule_response(controller, response,
                sizeof(response), CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
        } else if (value == CORE_MACHINE_KEYBOARD_SCAN_SET_1 ||
            value == CORE_MACHINE_KEYBOARD_SCAN_SET_2) {
            controller->data.scan_set = value;
            core_machine_kbc_schedule_response_byte(controller,
                CORE_MACHINE_KBC_ACK, CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
        } else {
            core_machine_kbc_schedule_response_byte(controller,
                CORE_MACHINE_KBC_RESEND, CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
        }
        break;
    case CORE_MACHINE_KBC_PENDING_AUX_DEVICE:
        controller->data.pending_write = CORE_MACHINE_KBC_PENDING_NONE;
        core_machine_kbc_handle_aux_command(controller, value);
        break;
    case CORE_MACHINE_KBC_PENDING_AUX_DISCARD:
        controller->data.pending_write = CORE_MACHINE_KBC_PENDING_NONE;
        break;
    default:
        core_machine_kbc_handle_keyboard_command(controller, value);
        break;
    }
    controller->data.input_buffer_full = LIB_FALSE;
    controller->data.last_write_command = LIB_FALSE;
}

static void core_machine_kbc_write_command(t_port *port,
    lib_u16 port_id, void *owner)
{
    t_kbc *controller = (t_kbc *)owner;
    lib_u8 command = port->data.ioByte;

    (void)port_id;
    if (controller == LIB_NULL) return;
    controller->data.input_buffer_full = LIB_TRUE;
    controller->data.last_write_command = LIB_TRUE;
    switch (command) {
    case 0x20u:
        core_machine_kbc_schedule_response_byte(controller,
            controller->data.command_byte, CORE_MACHINE_KBC_OUTPUT_CONTROLLER);
        break;
    case 0x60u:
        controller->data.pending_write = CORE_MACHINE_KBC_PENDING_COMMAND_BYTE;
        break;
    case 0xaau:
        /* IBM PC/AT initialization specifies that a successful controller
         * self-test inhibits the keyboard interface before reporting 55h.
         * The host must explicitly re-enable it with AEh or a command byte. */
        core_machine_kbc_flush_output(controller);
        core_machine_kbc_set_command_byte(controller,
            controller->data.command_byte | CORE_MACHINE_KBC_COMMAND_DISABLE_KEYBOARD);
        controller->data.command_byte |= CORE_MACHINE_KBC_COMMAND_SYSTEM;
        core_machine_kbc_schedule_response_byte(controller, 0x55u,
            CORE_MACHINE_KBC_OUTPUT_CONTROLLER);
        break;
    case 0xabu:
        core_machine_kbc_schedule_response_byte(controller, 0x00u,
            CORE_MACHINE_KBC_OUTPUT_CONTROLLER);
        break;
    case 0xadu:
        core_machine_kbc_set_command_byte(controller,
            controller->data.command_byte | CORE_MACHINE_KBC_COMMAND_DISABLE_KEYBOARD);
        break;
    case 0xaeu:
        core_machine_kbc_set_command_byte(controller,
            controller->data.command_byte & ~CORE_MACHINE_KBC_COMMAND_DISABLE_KEYBOARD);
        break;
    case 0xa7u:
        controller->data.aux_enabled = LIB_FALSE;
        controller->data.command_byte |= CORE_MACHINE_KBC_COMMAND_DISABLE_AUX;
        core_machine_kbc_refresh_current_irq(controller);
        break;
    case 0xa8u:
        controller->data.aux_enabled = controller->connect.aux_present;
        if (controller->connect.aux_present) {
            controller->data.command_byte &= ~CORE_MACHINE_KBC_COMMAND_DISABLE_AUX;
        } else {
            controller->data.command_byte |= CORE_MACHINE_KBC_COMMAND_DISABLE_AUX;
        }
        core_machine_kbc_refresh_current_irq(controller);
        break;
    case 0xa9u:
        core_machine_kbc_schedule_response_byte(controller,
            controller->connect.aux_present ? 0x00u : 0x01u,
            CORE_MACHINE_KBC_OUTPUT_CONTROLLER);
        break;
    case 0xc0u:
        core_machine_kbc_schedule_response_byte(controller,
            controller->data.input_port, CORE_MACHINE_KBC_OUTPUT_CONTROLLER);
        break;
    case 0xd0u:
        core_machine_kbc_schedule_response_byte(controller,
            controller->data.output_port, CORE_MACHINE_KBC_OUTPUT_CONTROLLER);
        break;
    case 0xd1u:
        controller->data.pending_write = CORE_MACHINE_KBC_PENDING_OUTPUT_PORT;
        break;
    case 0xd4u:
        controller->data.pending_write = controller->connect.aux_present ?
            CORE_MACHINE_KBC_PENDING_AUX_DEVICE :
            CORE_MACHINE_KBC_PENDING_AUX_DISCARD;
        break;
    case 0xe0u:
        core_machine_kbc_schedule_response_byte(controller,
            controller->data.test_inputs, CORE_MACHINE_KBC_OUTPUT_CONTROLLER);
        break;
    default:
        /* IBM PC/AT 8042 commands F0h--FFh pulse output-port bits selected
         * by zero command bits. Bit 0 is the reset line. The pulse must not
         * overwrite the persistent D1h output-port/A20 state. Its duration
         * belongs to the board timing contract, not this functional owner. */
        if (command >= 0xf0u && (command & 0x01u) == 0u &&
            controller->connect.execution != LIB_NULL) {
            core_machine_cpu_execution_request_reset(controller->connect.execution);
        }
        break;
    }
    controller->data.input_buffer_full = LIB_FALSE;
    controller->data.last_write_command = LIB_FALSE;
}

static void core_machine_kbc_register_ports(t_kbc *controller, t_port *port)
{
    core_machine_port_add_read(port, 0x0060,
        core_machine_kbc_read_data, controller);
    core_machine_port_add_read(port, 0x0064,
        core_machine_kbc_read_status, controller);
    core_machine_port_add_write(port, 0x0060,
        core_machine_kbc_write_data, controller);
    core_machine_port_add_write(port, 0x0064,
        core_machine_kbc_write_command, controller);
}

void core_machine_kbc_initialize(t_kbc *controller, t_port *port) {
    if (controller == LIB_NULL || port == LIB_NULL) return;
    lib_memory_set(controller, 0u, sizeof(*controller));
    controller->connect.aux_present = LIB_TRUE;
    controller->connect.reset_output_port = CORE_MACHINE_KBC_OUTPUT_RESET;
    core_machine_kbc_register_ports(controller, port);
    core_machine_kbc_reset(controller);
}
void core_machine_kbc_bind_core_services(t_kbc *controller, t_pic *pic_master,
    t_pic *pic_slave, t_ram *memory,
    core_machine_cpu_execution_context *execution, lib_u8 aux_present)
{
    if (controller == LIB_NULL) return;
    core_machine_pic_irq_source_bind(&controller->connect.irq1_source,
        pic_master, pic_slave, 1u);
    core_machine_pic_irq_source_bind(&controller->connect.irq12_source,
        pic_master, pic_slave, 12u);
    controller->connect.memory = memory;
    controller->connect.execution = execution;
    controller->connect.aux_present = aux_present;
    if (!aux_present) {
        controller->data.aux_enabled = LIB_FALSE;
        controller->data.command_byte &= ~CORE_MACHINE_KBC_COMMAND_IRQ12;
        controller->data.command_byte |= CORE_MACHINE_KBC_COMMAND_DISABLE_AUX;
        core_machine_kbc_deassert_irq12(controller);
    }
}
void core_machine_kbc_set_reset_output_port(t_kbc *controller,
    lib_u8 value)
{
    if (controller == LIB_NULL) return;
    controller->connect.reset_output_port = value;
    core_machine_kbc_apply_output_port(controller, value);
}
void core_machine_kbc_reset(t_kbc *controller)
{
    lib_u32 typematic_nominal_initial_ticks;
    lib_u32 typematic_nominal_repeat_ticks;
    lib_u32 command_response_ticks;
    lib_u8 command_response_status_polls;
    lib_u32 serial_delivery_ticks;

    if (controller == LIB_NULL) return;
    typematic_nominal_initial_ticks = controller->data.typematic_nominal_initial_ticks;
    typematic_nominal_repeat_ticks = controller->data.typematic_nominal_repeat_ticks;
    command_response_ticks = controller->data.command_response_ticks;
    command_response_status_polls =
        controller->data.command_response_status_polls;
    serial_delivery_ticks = controller->data.serial_delivery_ticks;
    lib_memory_set(&controller->data, 0u, sizeof(controller->data));
    controller->data.typematic_nominal_initial_ticks = typematic_nominal_initial_ticks;
    controller->data.typematic_nominal_repeat_ticks = typematic_nominal_repeat_ticks;
    controller->data.command_response_ticks = command_response_ticks;
    controller->data.command_response_status_polls =
        command_response_status_polls;
    controller->data.serial_delivery_ticks = serial_delivery_ticks;
    core_machine_pic_irq_source_deassert(&controller->connect.irq1_source);
    core_machine_pic_irq_source_deassert(&controller->connect.irq12_source);
    controller->data.command_byte = CORE_MACHINE_KBC_COMMAND_IRQ1 |
        (controller->connect.aux_present ? CORE_MACHINE_KBC_COMMAND_IRQ12 :
            CORE_MACHINE_KBC_COMMAND_DISABLE_AUX) |
        CORE_MACHINE_KBC_COMMAND_TRANSLATION;
    controller->data.output_port = controller->connect.reset_output_port;
    controller->data.keyboard_enabled = LIB_TRUE;
    controller->data.aux_enabled = controller->connect.aux_present;
    core_machine_kbc_set_defaults(controller);
    controller->data.scanning_enabled = LIB_TRUE;
    core_machine_kbc_set_aux_defaults(controller);
    controller->data.input_port = 0x80u;
    core_machine_kbc_apply_output_port(controller, controller->data.output_port);
}
void core_machine_kbc_advance(t_kbc *controller, lib_u64 elapsed_ticks)
{
    if (controller == LIB_NULL) return;
    if (elapsed_ticks >= controller->data.serial_delivery_remaining_ticks) {
        controller->data.serial_delivery_remaining_ticks = 0u;
    } else {
        controller->data.serial_delivery_remaining_ticks -= elapsed_ticks;
    }
    core_machine_kbc_drain_keyboard_serial(controller);
    if (controller->data.keyboard_bat_pending && controller->data.fifo_count == 0u &&
        controller->data.delayed_response_count == 0u) {
        controller->data.keyboard_bat_pending = LIB_FALSE;
        (void)core_machine_kbc_enqueue(controller, CORE_MACHINE_KBC_BAT_OK,
            CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
    }
    if (controller->data.delayed_response_count != 0u &&
        controller->data.response_status_polls_remaining == 0u) {
        if (elapsed_ticks < controller->data.response_remaining_ticks) {
            controller->data.response_remaining_ticks -= elapsed_ticks;
        } else {
            controller->data.response_remaining_ticks = 0u;
            if ((controller->data.keyboard_serial_count == 0u ||
                    !controller->data.scanning_enabled ||
                    !core_machine_kbc_keyboard_scan_delivery_enabled(controller,
                        controller->data.command_byte)) &&
                controller->data.delayed_response_count <=
                CORE_MACHINE_KBC_FIFO_CAPACITY - controller->data.fifo_count) {
                while (controller->data.delayed_response_index <
                        controller->data.delayed_response_count) {
                    (void)core_machine_kbc_enqueue(controller,
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
    if (elapsed_ticks == 0u) return;
    if (!controller->data.typematic_active) return;
    if (elapsed_ticks < controller->data.typematic_remaining_ticks) {
        controller->data.typematic_remaining_ticks -= elapsed_ticks;
        return;
    }
    elapsed_ticks -= controller->data.typematic_remaining_ticks;
    controller->data.typematic_remaining_ticks =
        controller->data.typematic_repeat_ticks;
    (void)core_machine_kbc_queue_native_byte(controller,
        controller->data.typematic_scan_code);
    while (controller->data.typematic_repeat_ticks != 0u &&
        elapsed_ticks >= controller->data.typematic_repeat_ticks) {
        elapsed_ticks -= controller->data.typematic_repeat_ticks;
        (void)core_machine_kbc_queue_native_byte(controller,
            controller->data.typematic_scan_code);
    }
    if (controller->data.typematic_repeat_ticks != 0u) {
        controller->data.typematic_remaining_ticks -= elapsed_ticks;
    }
    core_machine_kbc_drain_keyboard_serial(controller);
}

lib_status core_machine_kbc_ticks_until_event(const t_kbc *controller,
    lib_u64 *out_ticks)
{
    lib_u64 ticks = UINT64_MAX;

    if (controller == LIB_NULL || out_ticks == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (controller->data.serial_delivery_remaining_ticks != 0u) {
        ticks = controller->data.serial_delivery_remaining_ticks;
    }
    if (controller->data.keyboard_bat_pending && controller->data.fifo_count == 0u &&
        controller->data.delayed_response_count == 0u) {
        ticks = 0u;
    }
    if (controller->data.delayed_response_count != 0u && controller->data.fifo_count == 0u &&
        controller->data.response_status_polls_remaining == 0u &&
        controller->data.response_remaining_ticks < ticks) {
        ticks = controller->data.response_remaining_ticks;
    }
    if (controller->data.typematic_active &&
        controller->data.typematic_remaining_ticks < ticks) {
        ticks = controller->data.typematic_remaining_ticks;
    }
    if (ticks == UINT64_MAX) return LIB_STATUS_INVALID_STATE;
    *out_ticks = ticks;
    return LIB_STATUS_OK;
}

void core_machine_kbc_set_typematic_timing(t_kbc *controller,
    lib_u32 initial_ticks, lib_u32 repeat_ticks)
{
    if (controller == LIB_NULL) return;
    controller->data.typematic_nominal_initial_ticks = initial_ticks;
    controller->data.typematic_nominal_repeat_ticks = repeat_ticks;
    core_machine_kbc_apply_typematic_timing(controller);
}

void core_machine_kbc_set_command_response_timing(t_kbc *controller,
    lib_u32 response_ticks)
{
    if (controller == LIB_NULL) return;
    controller->data.command_response_ticks = response_ticks;
}
void core_machine_kbc_set_command_response_status_polls(t_kbc *controller,
    lib_u8 status_polls)
{
    if (controller == LIB_NULL) return;
    controller->data.command_response_status_polls = status_polls;
}
void core_machine_kbc_set_serial_delivery_timing(t_kbc *controller,
    lib_u32 delivery_ticks)
{
    if (controller == LIB_NULL) return;
    controller->data.serial_delivery_ticks = delivery_ticks;
    controller->data.serial_delivery_remaining_ticks = 0u;
    core_machine_kbc_drain_keyboard_serial(controller);
}
void core_machine_kbc_finalize(t_kbc *controller)
{
    if (controller != LIB_NULL) {
        core_machine_pic_irq_source_deassert(&controller->connect.irq1_source);
        core_machine_pic_irq_source_deassert(&controller->connect.irq12_source);
        controller->data.irq1_asserted = LIB_FALSE;
        controller->data.irq12_asserted = LIB_FALSE;
    }
}
static lib_status core_machine_kbc_admit_native_byte(t_kbc *controller,
    lib_u8 native_byte)
{
    lib_u8 known;
    lib_u8 set1;

    if (controller == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (!controller->data.scanning_enabled ||
        !core_machine_kbc_keyboard_scan_delivery_enabled(controller,
            controller->data.command_byte)) {
        return LIB_STATUS_INVALID_STATE;
    }
    /* Break-prefix state belongs to the native keyboard stream even while
     * firmware has disabled 8042 translation; typematic must still see it. */
    if (controller->data.scan_set == CORE_MACHINE_KEYBOARD_SCAN_SET_2 &&
        native_byte == 0xf0u) {
        controller->data.set2_typematic_break_pending = LIB_TRUE;
    }
    if (controller->data.scan_set == CORE_MACHINE_KEYBOARD_SCAN_SET_1 &&
        (native_byte & 0x80u) != 0u &&
        (native_byte & 0x7fu) == controller->data.typematic_scan_code) {
        controller->data.typematic_active = LIB_FALSE;
    } else if (controller->data.scan_set == CORE_MACHINE_KEYBOARD_SCAN_SET_1 &&
        controller->data.typematic_initial_ticks != 0u &&
        controller->data.typematic_repeat_ticks != 0u &&
        (native_byte & 0x80u) == 0u &&
        core_machine_kbc_is_typematic_scan_code(native_byte)) {
        controller->data.typematic_scan_code = native_byte;
        controller->data.typematic_remaining_ticks =
            controller->data.typematic_initial_ticks;
        controller->data.typematic_active = LIB_TRUE;
    }
    if (controller->data.scan_set == CORE_MACHINE_KEYBOARD_SCAN_SET_2) {
        set1 = core_machine_kbc_set2_to_set1(native_byte, &known);
        if (controller->data.set2_typematic_break_pending && known &&
            native_byte == controller->data.typematic_scan_code) {
            controller->data.typematic_active = LIB_FALSE;
        } else if (!controller->data.set2_typematic_break_pending &&
            native_byte != 0xe0u && native_byte != 0xe1u &&
            native_byte != 0xf0u && known &&
            controller->data.typematic_initial_ticks != 0u &&
            controller->data.typematic_repeat_ticks != 0u &&
            core_machine_kbc_is_typematic_scan_code(set1)) {
            controller->data.typematic_scan_code = native_byte;
            controller->data.typematic_remaining_ticks =
                controller->data.typematic_initial_ticks;
            controller->data.typematic_active = LIB_TRUE;
        }
    }
    if (controller->data.scan_set == CORE_MACHINE_KEYBOARD_SCAN_SET_2 &&
        native_byte != 0xe0u && native_byte != 0xe1u && native_byte != 0xf0u) {
        controller->data.set2_typematic_break_pending = LIB_FALSE;
    }
    return LIB_STATUS_OK;
}

lib_status core_machine_kbc_submit_native_byte(t_kbc *controller,
    lib_u8 native_byte)
{
    lib_status status;

    if (controller == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (controller->data.keyboard_serial_count >=
        CORE_MACHINE_KBC_KEYBOARD_SERIAL_CAPACITY) return LIB_STATUS_NO_MEMORY;
    status = core_machine_kbc_admit_native_byte(controller, native_byte);
    if (status != LIB_STATUS_OK) return status;
    status = core_machine_kbc_queue_native_byte(controller, native_byte);
    if (status != LIB_STATUS_OK) return status;
    core_machine_kbc_drain_keyboard_serial(controller);
    return LIB_STATUS_OK;
}
lib_status core_machine_kbc_submit_native_bytes(t_kbc *controller,
    const lib_u8 *native_bytes, lib_size count)
{
    lib_size index;

    if (controller == LIB_NULL || (native_bytes == LIB_NULL && count != 0u)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (!controller->data.scanning_enabled ||
        !core_machine_kbc_keyboard_scan_delivery_enabled(controller,
            controller->data.command_byte)) {
        return LIB_STATUS_INVALID_STATE;
    }
    if (count > CORE_MACHINE_KBC_KEYBOARD_SERIAL_CAPACITY -
        controller->data.keyboard_serial_count) return LIB_STATUS_NO_MEMORY;
    for (index = 0u; index < count; ++index) {
        if (core_machine_kbc_admit_native_byte(controller, native_bytes[index]) !=
            LIB_STATUS_OK) return LIB_STATUS_INVALID_STATE;
        (void)core_machine_kbc_queue_native_byte(controller, native_bytes[index]);
    }
    core_machine_kbc_drain_keyboard_serial(controller);
    return LIB_STATUS_OK;
}

static void core_machine_kbc_encode_aux_delta(lib_i16 delta, lib_u8 sign_bit,
    lib_u8 overflow_bit, lib_u8 *packet_first, lib_u8 *packet_data)
{
    if (delta > 255) {
        *packet_first |= overflow_bit;
        *packet_data = 0xffu;
    } else if (delta < -256) {
        *packet_first |= sign_bit | overflow_bit;
        *packet_data = 0x00u;
    } else {
        *packet_data = (lib_u8)delta;
        if (delta < 0) *packet_first |= sign_bit;
    }
}

lib_status core_machine_kbc_submit_aux_report(t_kbc *controller,
    lib_i16 delta_x, lib_i16 delta_y, lib_u8 buttons)
{
    lib_u8 packet[3];

    if (controller == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    buttons &= 0x07u;
    if (!controller->connect.aux_present || !controller->data.aux_enabled ||
        !controller->data.aux_reporting_enabled) {
        return LIB_STATUS_INVALID_STATE;
    }
    if (delta_x == 0 && delta_y == 0 && buttons == controller->data.aux_button_state) {
        return LIB_STATUS_OK;
    }
    if (controller->data.delayed_response_count != 0u ||
        CORE_MACHINE_KBC_FIFO_CAPACITY - controller->data.fifo_count <
            sizeof(packet)) return LIB_STATUS_INVALID_STATE;
    packet[0] = (lib_u8)(0x08u | buttons);
    core_machine_kbc_encode_aux_delta(delta_x, 0x10u, 0x40u,
        &packet[0], &packet[1]);
    core_machine_kbc_encode_aux_delta(delta_y, 0x20u, 0x80u,
        &packet[0], &packet[2]);
    (void)core_machine_kbc_enqueue(controller, packet[0],
        CORE_MACHINE_KBC_OUTPUT_AUX);
    (void)core_machine_kbc_enqueue(controller, packet[1],
        CORE_MACHINE_KBC_OUTPUT_AUX);
    (void)core_machine_kbc_enqueue(controller, packet[2],
        CORE_MACHINE_KBC_OUTPUT_AUX);
    controller->data.aux_button_state = buttons;
    return LIB_STATUS_OK;
}
