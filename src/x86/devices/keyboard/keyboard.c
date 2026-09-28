/* Copyright 2012-2026 Neko. */
#include "x86/devices/keyboard/keyboard.h"

lib_u8 x86_keyboard_set2_to_set1(lib_u8 set2,
    lib_bool *out_known)
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

static void keyboard_apply_typematic(x86_keyboard *keyboard)
{
    const lib_u32 delay = 1u + ((keyboard->typematic >> 5u) & 0x03u);
    const lib_u32 rate = (8u + (keyboard->typematic & 0x07u)) <<
        ((keyboard->typematic >> 3u) & 0x03u);
    keyboard->initial_ticks = (lib_u32)(
        ((lib_u64)keyboard->nominal_initial_ticks * delay) / 2u);
    keyboard->repeat_ticks = (lib_u32)(
        ((lib_u64)keyboard->nominal_repeat_ticks * rate) / 24u);
}

static void keyboard_defaults(x86_keyboard *keyboard)
{
    keyboard->scan_set = 2u;
    keyboard->leds = 0u;
    keyboard->typematic = 0x2cu;
    keyboard_apply_typematic(keyboard);
    keyboard->repeat_active = LIB_FALSE;
    keyboard->remaining_ticks = 0u;
    keyboard->repeat_byte = 0u;
    keyboard->break_pending = LIB_FALSE;
}

lib_status x86_keyboard_create(x86_keyboard **out_keyboard)
{
    x86_keyboard *keyboard;
    if (out_keyboard == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_keyboard = LIB_NULL;
    keyboard = lib_allocate_zero(1u, sizeof(*keyboard));
    if (keyboard == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    x86_keyboard_reset(keyboard);
    *out_keyboard = keyboard;
    return LIB_STATUS_OK;
}

void x86_keyboard_destroy(x86_keyboard *keyboard)
{
    lib_release(keyboard);
}

void x86_keyboard_reset(x86_keyboard *keyboard)
{
    lib_u32 initial;
    lib_u32 repeat;
    if (keyboard == LIB_NULL) return;
    initial = keyboard->nominal_initial_ticks;
    repeat = keyboard->nominal_repeat_ticks;
    lib_memory_set(keyboard, 0u, sizeof(*keyboard));
    keyboard->nominal_initial_ticks = initial;
    keyboard->nominal_repeat_ticks = repeat;
    keyboard_defaults(keyboard);
    keyboard->scanning = LIB_TRUE;
}

static void keyboard_reply(const x86_keyboard_link *link, lib_u8 byte)
{
    link->reply(link->context, &byte, 1u);
}

static void keyboard_reset_stream(const x86_keyboard_link *link)
{
    if (link->stream_reset != LIB_NULL) link->stream_reset(link->context);
}

void x86_keyboard_write(x86_keyboard *keyboard, lib_u8 value,
    const x86_keyboard_link *link)
{
    lib_u8 reply[3] = { 0xfau, 0xabu, 0x83u };
    keyboard_parameter pending;
    if (keyboard == LIB_NULL || link == LIB_NULL || link->reply == LIB_NULL) return;
    pending = keyboard->pending;
    keyboard->pending = KEYBOARD_PARAMETER_NONE;
    switch (pending) {
    case KEYBOARD_PARAMETER_LEDS:
        keyboard->leds = value & 0x07u;
        keyboard_reply(link, 0xfau);
        return;
    case KEYBOARD_PARAMETER_TYPEMATIC:
        keyboard->typematic = value;
        keyboard_apply_typematic(keyboard);
        keyboard_reply(link, 0xfau);
        return;
    case KEYBOARD_PARAMETER_SCAN_SET:
        if (value == 0u) {
            reply[1] = keyboard->scan_set;
            link->reply(link->context, reply, 2u);
        } else if (value == 1u || value == 2u) {
            keyboard->scan_set = value;
            keyboard_reply(link, 0xfau);
        } else keyboard_reply(link, 0xfeu);
        return;
    default:
        break;
    }
    switch (value) {
    case 0xffu:
        keyboard_reply(link, 0xfau);
        keyboard->startup_released = LIB_TRUE;
        keyboard->bat_pending = LIB_TRUE;
        keyboard_defaults(keyboard);
        keyboard_reset_stream(link);
        keyboard->scanning = LIB_TRUE;
        break;
    case 0xedu:
        keyboard_reply(link, 0xfau);
        keyboard->pending = KEYBOARD_PARAMETER_LEDS;
        break;
    case 0xeeu:
        keyboard_reply(link, 0xeeu);
        break;
    case 0xf0u:
        keyboard_reply(link, 0xfau);
        keyboard->pending = KEYBOARD_PARAMETER_SCAN_SET;
        break;
    case 0xf3u:
        keyboard_reply(link, 0xfau);
        keyboard->pending = KEYBOARD_PARAMETER_TYPEMATIC;
        break;
    case 0xf4u:
        keyboard->scanning = LIB_TRUE;
        keyboard->repeat_active = LIB_FALSE;
        keyboard->repeat_byte = 0u;
        keyboard_reply(link, 0xfau);
        break;
    case 0xf5u:
        keyboard_defaults(keyboard);
        keyboard_reset_stream(link);
        keyboard->scanning = LIB_FALSE;
        keyboard_reply(link, 0xfau);
        break;
    case 0xf6u:
        keyboard_defaults(keyboard);
        keyboard_reset_stream(link);
        keyboard_reply(link, 0xfau);
        break;
    case 0xf2u:
        link->reply(link->context, reply, 3u);
        break;
    case 0xfeu:
        keyboard_reply(link, keyboard->last_output == 0xfeu && keyboard->has_output ?
            keyboard->previous_output : keyboard->last_output);
        break;
    case 0xfdu: case 0xfcu: case 0xfbu: case 0xf7u:
        keyboard_reply(link, 0xfau);
        break;
    default:
        keyboard_reply(link, 0xfeu);
        break;
    }
}

void x86_keyboard_cancel_parameter(x86_keyboard *keyboard)
{
    if (keyboard != LIB_NULL) keyboard->pending = KEYBOARD_PARAMETER_NONE;
}

x86_keyboard_signals x86_keyboard_get_signals(const x86_keyboard *keyboard)
{
    x86_keyboard_signals signals = { 0u, 0u, LIB_FALSE, LIB_FALSE };
    if (keyboard != LIB_NULL) {
        signals.scan_set = keyboard->scan_set;
        signals.leds = keyboard->leds;
        signals.scanning = keyboard->scanning;
        signals.bat_ready = keyboard->bat_pending;
    }
    return signals;
}

lib_bool x86_keyboard_start(x86_keyboard *keyboard)
{
    if (keyboard == LIB_NULL || keyboard->startup_released) return LIB_FALSE;
    keyboard->startup_released = LIB_TRUE;
    keyboard->bat_pending = LIB_TRUE;
    return LIB_TRUE;
}

lib_bool x86_keyboard_take_bat(x86_keyboard *keyboard)
{
    if (keyboard == LIB_NULL || !keyboard->bat_pending) return LIB_FALSE;
    keyboard->bat_pending = LIB_FALSE;
    return LIB_TRUE;
}

void x86_keyboard_clear_bat(x86_keyboard *keyboard)
{
    if (keyboard != LIB_NULL) keyboard->bat_pending = LIB_FALSE;
}

void x86_keyboard_note_output(x86_keyboard *keyboard, lib_u8 byte)
{
    if (keyboard == LIB_NULL) return;
    if (keyboard->has_output) keyboard->previous_output = keyboard->last_output;
    keyboard->last_output = byte;
    keyboard->has_output = LIB_TRUE;
}

static lib_bool keyboard_is_repeatable(lib_u8 scan_code)
{
    switch (scan_code) {
    case 0x1du: case 0x2au: case 0x36u: case 0x38u:
    case 0x3au: case 0x45u: case 0x46u:
        return LIB_FALSE;
    default:
        return scan_code != 0xe0u && scan_code != 0xe1u;
    }
}

lib_status x86_keyboard_admit(x86_keyboard *keyboard, lib_u8 byte)
{
    lib_bool known;
    lib_u8 set1;
    if (keyboard == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (!keyboard->scanning) return LIB_STATUS_INVALID_STATE;
    if (keyboard->scan_set == 2u && byte == 0xf0u) keyboard->break_pending = LIB_TRUE;
    if (keyboard->scan_set == 1u && (byte & 0x80u) != 0u &&
        (byte & 0x7fu) == keyboard->repeat_byte) {
        keyboard->repeat_active = LIB_FALSE;
    } else if (keyboard->scan_set == 1u && keyboard->initial_ticks != 0u &&
        keyboard->repeat_ticks != 0u && (byte & 0x80u) == 0u &&
        keyboard_is_repeatable(byte)) {
        keyboard->repeat_byte = byte;
        keyboard->remaining_ticks = keyboard->initial_ticks;
        keyboard->repeat_active = LIB_TRUE;
    }
    if (keyboard->scan_set == 2u) {
        set1 = x86_keyboard_set2_to_set1(byte, &known);
        if (keyboard->break_pending && known && byte == keyboard->repeat_byte) {
            keyboard->repeat_active = LIB_FALSE;
        } else if (!keyboard->break_pending && byte != 0xe0u && byte != 0xe1u &&
            byte != 0xf0u && known && keyboard->initial_ticks != 0u &&
            keyboard->repeat_ticks != 0u && keyboard_is_repeatable(set1)) {
            keyboard->repeat_byte = byte;
            keyboard->remaining_ticks = keyboard->initial_ticks;
            keyboard->repeat_active = LIB_TRUE;
        }
    }
    if (keyboard->scan_set == 2u && byte != 0xe0u && byte != 0xe1u && byte != 0xf0u) {
        keyboard->break_pending = LIB_FALSE;
    }
    return LIB_STATUS_OK;
}

void x86_keyboard_set_typematic_timing(x86_keyboard *keyboard,
    lib_u32 initial_ticks, lib_u32 repeat_ticks)
{
    if (keyboard == LIB_NULL) return;
    keyboard->nominal_initial_ticks = initial_ticks;
    keyboard->nominal_repeat_ticks = repeat_ticks;
    keyboard_apply_typematic(keyboard);
}

lib_bool x86_keyboard_advance(x86_keyboard *keyboard, lib_u64 elapsed_ticks,
    void (*repeat)(void *context, lib_u8 byte), void *context)
{
    if (keyboard == LIB_NULL || repeat == LIB_NULL || elapsed_ticks == 0u ||
        !keyboard->repeat_active) return LIB_FALSE;
    if (elapsed_ticks < keyboard->remaining_ticks) {
        keyboard->remaining_ticks -= elapsed_ticks;
        return LIB_FALSE;
    }
    elapsed_ticks -= keyboard->remaining_ticks;
    keyboard->remaining_ticks = keyboard->repeat_ticks;
    repeat(context, keyboard->repeat_byte);
    while (keyboard->repeat_ticks != 0u && elapsed_ticks >= keyboard->repeat_ticks) {
        elapsed_ticks -= keyboard->repeat_ticks;
        repeat(context, keyboard->repeat_byte);
    }
    if (keyboard->repeat_ticks != 0u) keyboard->remaining_ticks -= elapsed_ticks;
    return LIB_TRUE;
}

lib_status x86_keyboard_ticks_until_repeat(const x86_keyboard *keyboard,
    lib_u64 *out_ticks)
{
    if (keyboard == LIB_NULL || out_ticks == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (!keyboard->repeat_active) return LIB_STATUS_INVALID_STATE;
    *out_ticks = keyboard->remaining_ticks;
    return LIB_STATUS_OK;
}
