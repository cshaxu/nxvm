/* Copyright 2012-2026 Neko. */
#include "x86-nxvm/chips/keyboard/keyboard.h"

#define KEYBOARD_KEY_MAKE 0x01u
#define KEYBOARD_KEY_BREAK 0x02u
#define KEYBOARD_KEY_REPEAT 0x04u
#define KEYBOARD_KEY_TYPEMATIC (KEYBOARD_KEY_MAKE | KEYBOARD_KEY_REPEAT)
#define KEYBOARD_KEY_MAKE_BREAK (KEYBOARD_KEY_MAKE | KEYBOARD_KEY_BREAK)
#define KEYBOARD_KEY_ALL (KEYBOARD_KEY_MAKE_BREAK | KEYBOARD_KEY_REPEAT)

/* IBM March 1986, 4-58--60: only the selected 101-key layout. */
static const lib_u8 keyboard_default_key_types[0x85] = {
    [0x0e] = KEYBOARD_KEY_TYPEMATIC, [0x16] = KEYBOARD_KEY_TYPEMATIC,
    [0x1e] = KEYBOARD_KEY_TYPEMATIC, [0x26] = KEYBOARD_KEY_TYPEMATIC,
    [0x25] = KEYBOARD_KEY_TYPEMATIC, [0x2e] = KEYBOARD_KEY_TYPEMATIC,
    [0x36] = KEYBOARD_KEY_TYPEMATIC, [0x3d] = KEYBOARD_KEY_TYPEMATIC,
    [0x3e] = KEYBOARD_KEY_TYPEMATIC, [0x46] = KEYBOARD_KEY_TYPEMATIC,
    [0x45] = KEYBOARD_KEY_TYPEMATIC, [0x4e] = KEYBOARD_KEY_TYPEMATIC,
    [0x55] = KEYBOARD_KEY_TYPEMATIC, [0x66] = KEYBOARD_KEY_TYPEMATIC,
    [0x0d] = KEYBOARD_KEY_TYPEMATIC, [0x15] = KEYBOARD_KEY_TYPEMATIC,
    [0x1d] = KEYBOARD_KEY_TYPEMATIC, [0x24] = KEYBOARD_KEY_TYPEMATIC,
    [0x2d] = KEYBOARD_KEY_TYPEMATIC, [0x2c] = KEYBOARD_KEY_TYPEMATIC,
    [0x35] = KEYBOARD_KEY_TYPEMATIC, [0x3c] = KEYBOARD_KEY_TYPEMATIC,
    [0x43] = KEYBOARD_KEY_TYPEMATIC, [0x44] = KEYBOARD_KEY_TYPEMATIC,
    [0x4d] = KEYBOARD_KEY_TYPEMATIC, [0x54] = KEYBOARD_KEY_TYPEMATIC,
    [0x5b] = KEYBOARD_KEY_TYPEMATIC, [0x5c] = KEYBOARD_KEY_TYPEMATIC,
    [0x1c] = KEYBOARD_KEY_TYPEMATIC, [0x1b] = KEYBOARD_KEY_TYPEMATIC,
    [0x23] = KEYBOARD_KEY_TYPEMATIC, [0x2b] = KEYBOARD_KEY_TYPEMATIC,
    [0x34] = KEYBOARD_KEY_TYPEMATIC, [0x33] = KEYBOARD_KEY_TYPEMATIC,
    [0x3b] = KEYBOARD_KEY_TYPEMATIC, [0x42] = KEYBOARD_KEY_TYPEMATIC,
    [0x4b] = KEYBOARD_KEY_TYPEMATIC, [0x4c] = KEYBOARD_KEY_TYPEMATIC,
    [0x52] = KEYBOARD_KEY_TYPEMATIC, [0x5a] = KEYBOARD_KEY_TYPEMATIC,
    [0x1a] = KEYBOARD_KEY_TYPEMATIC, [0x22] = KEYBOARD_KEY_TYPEMATIC,
    [0x21] = KEYBOARD_KEY_TYPEMATIC, [0x2a] = KEYBOARD_KEY_TYPEMATIC,
    [0x32] = KEYBOARD_KEY_TYPEMATIC, [0x31] = KEYBOARD_KEY_TYPEMATIC,
    [0x3a] = KEYBOARD_KEY_TYPEMATIC, [0x41] = KEYBOARD_KEY_TYPEMATIC,
    [0x49] = KEYBOARD_KEY_TYPEMATIC, [0x4a] = KEYBOARD_KEY_TYPEMATIC,
    [0x29] = KEYBOARD_KEY_TYPEMATIC, [0x64] = KEYBOARD_KEY_TYPEMATIC,
    [0x61] = KEYBOARD_KEY_TYPEMATIC, [0x63] = KEYBOARD_KEY_TYPEMATIC,
    [0x60] = KEYBOARD_KEY_TYPEMATIC, [0x6a] = KEYBOARD_KEY_TYPEMATIC,
    [0x7c] = KEYBOARD_KEY_TYPEMATIC,
    [0x14] = KEYBOARD_KEY_MAKE_BREAK, [0x12] = KEYBOARD_KEY_MAKE_BREAK,
    [0x59] = KEYBOARD_KEY_MAKE_BREAK, [0x11] = KEYBOARD_KEY_MAKE_BREAK,
    [0x19] = KEYBOARD_KEY_MAKE_BREAK,
    [0x39] = KEYBOARD_KEY_MAKE, [0x58] = KEYBOARD_KEY_MAKE,
    [0x67] = KEYBOARD_KEY_MAKE, [0x6e] = KEYBOARD_KEY_MAKE,
    [0x65] = KEYBOARD_KEY_MAKE, [0x6f] = KEYBOARD_KEY_MAKE,
    [0x6d] = KEYBOARD_KEY_MAKE, [0x76] = KEYBOARD_KEY_MAKE,
    [0x6c] = KEYBOARD_KEY_MAKE, [0x6b] = KEYBOARD_KEY_MAKE,
    [0x69] = KEYBOARD_KEY_MAKE, [0x77] = KEYBOARD_KEY_MAKE,
    [0x75] = KEYBOARD_KEY_MAKE, [0x73] = KEYBOARD_KEY_MAKE,
    [0x72] = KEYBOARD_KEY_MAKE, [0x70] = KEYBOARD_KEY_MAKE,
    [0x7e] = KEYBOARD_KEY_MAKE, [0x7d] = KEYBOARD_KEY_MAKE,
    [0x74] = KEYBOARD_KEY_MAKE, [0x7a] = KEYBOARD_KEY_MAKE,
    [0x71] = KEYBOARD_KEY_MAKE, [0x84] = KEYBOARD_KEY_MAKE,
    [0x79] = KEYBOARD_KEY_MAKE, [0x08] = KEYBOARD_KEY_MAKE,
    [0x07] = KEYBOARD_KEY_MAKE, [0x0f] = KEYBOARD_KEY_MAKE,
    [0x17] = KEYBOARD_KEY_MAKE, [0x1f] = KEYBOARD_KEY_MAKE,
    [0x27] = KEYBOARD_KEY_MAKE, [0x2f] = KEYBOARD_KEY_MAKE,
    [0x37] = KEYBOARD_KEY_MAKE, [0x3f] = KEYBOARD_KEY_MAKE,
    [0x47] = KEYBOARD_KEY_MAKE, [0x4f] = KEYBOARD_KEY_MAKE,
    [0x56] = KEYBOARD_KEY_MAKE, [0x5e] = KEYBOARD_KEY_MAKE,
    [0x57] = KEYBOARD_KEY_MAKE, [0x5f] = KEYBOARD_KEY_MAKE,
    [0x62] = KEYBOARD_KEY_MAKE
};

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
    if (keyboard->initial_ticks == 0u || keyboard->repeat_ticks == 0u) {
        keyboard->repeat_count = 0u;
        keyboard->remaining_ticks = 0u;
    }
}

static void keyboard_defaults(x86_keyboard *keyboard)
{
    lib_memory_copy(keyboard->key_types, keyboard_default_key_types,
        sizeof(keyboard->key_types));
    keyboard->scan_set = 2u;
    keyboard->typematic = 0x2cu;
    keyboard_apply_typematic(keyboard);
    keyboard->remaining_ticks = 0u;
    keyboard->repeat_count = 0u;
    keyboard->input_count = 0u;
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

static void keyboard_clear_stream(x86_keyboard *keyboard,
    const x86_keyboard_link *link)
{
    keyboard->remaining_ticks = 0u;
    keyboard->repeat_count = 0u;
    keyboard->input_count = 0u;
    keyboard_reset_stream(link);
}

static lib_bool keyboard_scanning(const x86_keyboard *keyboard)
{
    return keyboard->scanning && keyboard->pending == KEYBOARD_PARAMETER_NONE;
}

void x86_keyboard_write(x86_keyboard *keyboard, lib_u8 value,
    const x86_keyboard_link *link)
{
    lib_u8 reply[3] = { 0xfau, 0xabu, 0x83u };
    keyboard_parameter pending;
    if (keyboard == LIB_NULL || link == LIB_NULL || link->reply == LIB_NULL) return;
    pending = keyboard->pending;
    keyboard->pending = KEYBOARD_PARAMETER_NONE;
    /* ED--FF are commands, not options for an incomplete transaction. */
    if (value >= 0xedu) pending = KEYBOARD_PARAMETER_NONE;
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
        } else if (value >= 1u && value <= 3u) {
            keyboard->scan_set = value;
            keyboard_reply(link, 0xfau);
        } else keyboard_reply(link, 0xfeu);
        return;
    case KEYBOARD_PARAMETER_KEY_TYPEMATIC:
    case KEYBOARD_PARAMETER_KEY_MAKE_BREAK:
    case KEYBOARD_PARAMETER_KEY_MAKE:
        if (value >= sizeof(keyboard_default_key_types) ||
            keyboard_default_key_types[value] == 0u) {
            keyboard_reply(link, 0xfeu);
            keyboard->pending = pending;
        } else {
            keyboard->key_types[value] = pending == KEYBOARD_PARAMETER_KEY_TYPEMATIC ?
                KEYBOARD_KEY_TYPEMATIC : pending == KEYBOARD_PARAMETER_KEY_MAKE_BREAK ?
                KEYBOARD_KEY_MAKE_BREAK : KEYBOARD_KEY_MAKE;
            keyboard_reply(link, 0xfau);
        }
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
        keyboard->leds = 0u;
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
        keyboard_clear_stream(keyboard, link);
        keyboard->pending = KEYBOARD_PARAMETER_SCAN_SET;
        keyboard_reply(link, 0xfau);
        break;
    case 0xf3u:
        keyboard_reply(link, 0xfau);
        keyboard->pending = KEYBOARD_PARAMETER_TYPEMATIC;
        break;
    case 0xf4u:
        keyboard->scanning = LIB_TRUE;
        keyboard_clear_stream(keyboard, link);
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
        keyboard->scanning = LIB_TRUE;
        keyboard_reply(link, 0xfau);
        break;
    case 0xf2u:
        link->reply(link->context, reply, 3u);
        break;
    case 0xfeu:
        keyboard_reply(link, keyboard->last_output == 0xfeu && keyboard->has_output ?
            keyboard->previous_output : keyboard->last_output);
        break;
    case 0xf7u: case 0xf8u: case 0xf9u: case 0xfau:
        for (lib_size index = 0u; index < sizeof(keyboard->key_types); ++index) {
            if (keyboard_default_key_types[index] != 0u) {
                keyboard->key_types[index] = value == 0xf7u ? KEYBOARD_KEY_TYPEMATIC :
                    value == 0xf8u ? KEYBOARD_KEY_MAKE_BREAK :
                    value == 0xf9u ? KEYBOARD_KEY_MAKE : KEYBOARD_KEY_ALL;
            }
        }
        keyboard_clear_stream(keyboard, link);
        keyboard_reply(link, 0xfau);
        break;
    case 0xfbu: case 0xfcu: case 0xfdu:
        keyboard_clear_stream(keyboard, link);
        keyboard->scanning = LIB_FALSE;
        keyboard->pending = value == 0xfbu ? KEYBOARD_PARAMETER_KEY_TYPEMATIC :
            value == 0xfcu ? KEYBOARD_PARAMETER_KEY_MAKE_BREAK : KEYBOARD_PARAMETER_KEY_MAKE;
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
    x86_keyboard_signals signals = { 0u, 0u, LIB_FALSE, LIB_FALSE, 0u };
    if (keyboard != LIB_NULL) {
        signals.scan_set = keyboard->scan_set;
        signals.leds = keyboard->leds;
        signals.scanning = keyboard_scanning(keyboard);
        signals.bat_ready = keyboard->bat_pending;
        signals.repeat_bytes = keyboard_scanning(keyboard) ? keyboard->repeat_count : 0u;
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

static lib_status keyboard_track_native_byte(x86_keyboard *keyboard, lib_u8 byte)
{
    static const lib_u8 pause_set1[] = { 0xe1u, 0x1du, 0x45u, 0xe1u, 0x9du, 0xc5u };
    static const lib_u8 pause_set2[] = { 0xe1u, 0x14u, 0x77u, 0xe1u, 0xf0u, 0x14u, 0xf0u, 0x77u };
    lib_bool known;
    lib_bool released;
    lib_bool extended;
    lib_u8 make[2];
    lib_u8 make_count;
    lib_u8 set1;

    if (keyboard->input_count >= sizeof(keyboard->input_prefix))
        return LIB_STATUS_INVALID_ARGUMENT;
    if (keyboard->input_count != 0u && keyboard->input_prefix[0] != 0xe1u &&
        (byte == 0xe0u || byte == 0xe1u ||
        (byte == 0xf0u && keyboard->input_prefix[keyboard->input_count - 1u] == 0xf0u)))
        return LIB_STATUS_INVALID_ARGUMENT;
    keyboard->input_prefix[keyboard->input_count++] = byte;
    if (keyboard->input_prefix[0] == 0xe1u) {
        const lib_u8 *pause = keyboard->scan_set == 1u ? pause_set1 : pause_set2;
        if (byte != pause[keyboard->input_count - 1u]) return LIB_STATUS_INVALID_ARGUMENT;
        if (keyboard->input_count == (keyboard->scan_set == 1u ? 6u : 8u)) {
            keyboard->input_count = 0u; /* Pause is not a typematic make. */
        }
        return LIB_STATUS_OK;
    }
    extended = keyboard->input_prefix[0] == 0xe0u;
    if (extended && keyboard->input_count == 1u) return LIB_STATUS_OK;
    if (keyboard->scan_set == 2u && byte == 0xf0u) return LIB_STATUS_OK;
    make_count = extended ? 2u : 1u;
    make[0] = extended ? 0xe0u : byte;
    make[make_count - 1u] = keyboard->scan_set == 1u ? byte & 0x7fu : byte;
    released = keyboard->scan_set == 1u ? (byte & 0x80u) != 0u :
        keyboard->input_prefix[extended ? 1u : 0u] == 0xf0u;
    keyboard->input_count = 0u;
    if (keyboard->scan_set == 1u) {
        set1 = byte & 0x7fu;
        known = set1 != 0u;
    } else {
        set1 = x86_keyboard_set2_to_set1(byte, &known);
    }
    if (!known) return LIB_STATUS_OK;
    if (released) {
        if (keyboard->repeat_count == make_count &&
            lib_memory_compare(keyboard->repeat_bytes, make, make_count) == 0) {
            keyboard->repeat_count = 0u;
            keyboard->remaining_ticks = 0u;
        }
    } else if (keyboard->initial_ticks != 0u && keyboard->repeat_ticks != 0u &&
        keyboard_is_repeatable(set1) && !(extended && set1 == 0x37u)) {
        lib_memory_copy(keyboard->repeat_bytes, make, make_count);
        keyboard->repeat_count = make_count;
        keyboard->remaining_ticks = keyboard->initial_ticks;
    }
    return LIB_STATUS_OK;
}

static lib_status keyboard_prepare_set3_byte(x86_keyboard *keyboard,
    lib_u8 byte, lib_u8 *output, lib_size capacity, lib_size *count)
{
    lib_bool released;
    lib_u8 type;
    lib_size needed;
    if (byte == 0xf0u) {
        if (keyboard->input_count != 0u) return LIB_STATUS_INVALID_ARGUMENT;
        keyboard->input_prefix[0] = byte;
        keyboard->input_count = 1u;
        return LIB_STATUS_OK;
    }
    if (byte >= sizeof(keyboard->key_types) || keyboard->key_types[byte] == 0u)
        return LIB_STATUS_INVALID_ARGUMENT;
    released = keyboard->input_count != 0u;
    type = keyboard->key_types[byte];
    needed = !released ? 1u : (type & KEYBOARD_KEY_BREAK) != 0u ? 2u : 0u;
    if (needed > capacity - *count) return LIB_STATUS_NO_MEMORY;
    keyboard->input_count = 0u;
    if (released) {
        if (keyboard->repeat_count == 1u && keyboard->repeat_bytes[0] == byte) {
            keyboard->repeat_count = 0u;
            keyboard->remaining_ticks = 0u;
        }
        if (needed != 0u) output[(*count)++] = 0xf0u;
    } else if ((type & KEYBOARD_KEY_REPEAT) != 0u &&
        keyboard->initial_ticks != 0u && keyboard->repeat_ticks != 0u) {
        keyboard->repeat_bytes[0] = byte;
        keyboard->repeat_count = 1u;
        keyboard->remaining_ticks = keyboard->initial_ticks;
    }
    if (needed != 0u) output[(*count)++] = byte;
    return LIB_STATUS_OK;
}

lib_status x86_keyboard_admit(x86_keyboard *keyboard, const lib_u8 *bytes,
    lib_size count, lib_u8 *output, lib_size capacity, lib_size *output_count)
{
    x86_keyboard candidate;
    lib_size prepared = 0u;

    if (output_count == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *output_count = 0u;
    if (keyboard == LIB_NULL || (count != 0u &&
        (bytes == LIB_NULL || output == LIB_NULL))) return LIB_STATUS_INVALID_ARGUMENT;
    if (!keyboard_scanning(keyboard)) return LIB_STATUS_INVALID_STATE;
    if (keyboard->scan_set != 3u && count > capacity) return LIB_STATUS_NO_MEMORY;
    candidate = *keyboard;
    for (lib_size index = 0u; index < count; ++index) {
        const lib_status status = candidate.scan_set == 3u ?
            keyboard_prepare_set3_byte(&candidate, bytes[index], output, capacity, &prepared) :
            keyboard_track_native_byte(&candidate, bytes[index]);
        if (status != LIB_STATUS_OK) return status;
    }
    if (candidate.scan_set != 3u) {
        if (count != 0u) lib_memory_copy(output, bytes, count);
        prepared = count;
    }
    *keyboard = candidate;
    *output_count = prepared;
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
    lib_status (*repeat)(void *context, const lib_u8 *bytes, lib_size count),
    void *context)
{
    lib_bool accepted = LIB_FALSE;
    if (keyboard == LIB_NULL || repeat == LIB_NULL ||
        keyboard->repeat_count == 0u || !keyboard_scanning(keyboard)) return LIB_FALSE;
    while (elapsed_ticks >= keyboard->remaining_ticks) {
        elapsed_ticks -= keyboard->remaining_ticks;
        if (repeat(context, keyboard->repeat_bytes, keyboard->repeat_count) !=
            LIB_STATUS_OK) {
            keyboard->remaining_ticks = 0u;
            return accepted;
        }
        accepted = LIB_TRUE;
        keyboard->remaining_ticks = keyboard->repeat_ticks;
    }
    keyboard->remaining_ticks -= elapsed_ticks;
    return accepted;
}

lib_status x86_keyboard_ticks_until_repeat(const x86_keyboard *keyboard,
    lib_u64 *out_ticks)
{
    if (keyboard == LIB_NULL || out_ticks == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (keyboard->repeat_count == 0u || !keyboard_scanning(keyboard)) return LIB_STATUS_INVALID_STATE;
    *out_ticks = keyboard->remaining_ticks;
    return LIB_STATUS_OK;
}
