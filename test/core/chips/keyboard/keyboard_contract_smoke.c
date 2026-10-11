#include "core/chips/keyboard/keyboard_interface.h"

typedef struct keyboard_probe {
    x86_keyboard *keyboard;
    lib_u8 bytes[3];
    lib_u8 count;
    lib_u32 resets;
    lib_u32 repeats;
    lib_u8 repeated;
    lib_u8 repeat_bytes[8];
    lib_size repeat_count;
    lib_status repeat_status;
    x86_keyboard_signals at_reply;
    lib_u32 resets_at_reply;
} keyboard_probe;

static lib_status admit_byte(x86_keyboard *keyboard, lib_u8 byte)
{
    lib_u8 output;
    lib_size count;
    return x86_keyboard_admit(keyboard, &byte, 1u, &output, 1u, &count);
}

static void receive_reply(void *context, const lib_u8 *bytes, lib_u8 count)
{
    keyboard_probe *probe = context;
    probe->count = count;
    probe->at_reply = x86_keyboard_get_signals(probe->keyboard);
    probe->resets_at_reply = probe->resets;
    if (count <= sizeof(probe->bytes)) {
        lib_memory_copy(probe->bytes, bytes, count);
    }
}

static void reset_stream(void *context)
{
    keyboard_probe *probe = context;
    ++probe->resets;
}

static lib_status receive_repeat(void *context, const lib_u8 *bytes, lib_size count)
{
    keyboard_probe *probe = context;
    if (count > sizeof(probe->repeat_bytes)) return LIB_STATUS_INVALID_ARGUMENT;
    if (probe->repeat_status != LIB_STATUS_OK) return probe->repeat_status;
    lib_memory_copy(probe->repeat_bytes, bytes, count);
    probe->repeat_count = count;
    probe->repeated = bytes[count - 1u];
    ++probe->repeats;
    return LIB_STATUS_OK;
}

static lib_bool reply_is(x86_keyboard *keyboard, lib_u8 command,
    const x86_keyboard_link *link, lib_u8 count,
    lib_u8 first, lib_u8 second, lib_u8 third)
{
    keyboard_probe *probe = link->context;
    const lib_u8 expected[3] = { first, second, third };
    probe->count = 0u;
    x86_keyboard_write(keyboard, command, link);
    return probe->count == count &&
        lib_memory_compare(probe->bytes, expected, count) == 0;
}

static lib_bool command_matrix(x86_keyboard *keyboard,
    const x86_keyboard_link *link)
{
    lib_u16 value;
    lib_bool failed = LIB_FALSE;
    /* The first matrix covers every command byte. */
    for (value = 0u; value < 256u; ++value) {
        lib_u8 count = 1u;
        lib_u8 first = 0xfau;
        lib_u8 second = 0u;
        lib_u8 third = 0u;
        x86_keyboard_reset(keyboard);
        switch (value) {
        case 0xedu: case 0xf0u: case 0xf3u: case 0xf4u: case 0xf5u:
        case 0xf6u: case 0xf7u: case 0xf8u: case 0xf9u: case 0xfau:
        case 0xfbu: case 0xfcu: case 0xfdu:
        case 0xffu: break;
        case 0xeeu: first = 0xeeu; break;
        case 0xfeu: first = 0u; break;
        case 0xf2u: count = 3u; second = 0xabu; third = 0x83u; break;
        default: first = 0xfeu; break;
        }
        failed |= !reply_is(keyboard, (lib_u8)value, link, count, first, second, third);
    }
    /* Reserved options retain the model; ED--FF instead begin commands. */
    for (value = 0u; value < 0xedu; ++value) {
        x86_keyboard_reset(keyboard);
        failed |= !reply_is(keyboard, 0xedu, link, 1u, 0xfau, 0u, 0u);
        failed |= !reply_is(keyboard, (lib_u8)value, link, 1u, 0xfau, 0u, 0u);
        failed |= x86_keyboard_get_signals(keyboard).leds != (value & 7u);
        failed |= !reply_is(keyboard, 0xf0u, link, 1u, 0xfau, 0u, 0u);
        failed |= !reply_is(keyboard, (lib_u8)value, link,
            value == 0u ? 2u : 1u, value <= 3u ? 0xfau : 0xfeu, 2u, 0u);
        failed |= x86_keyboard_get_signals(keyboard).scan_set !=
            (value >= 1u && value <= 3u ? value : 2u);
    }
    return failed;
}

static lib_bool repeat_matrix(x86_keyboard *keyboard,
    const x86_keyboard_link *link)
{
    keyboard_probe *probe = link->context;
    lib_u16 value;
    lib_bool failed = LIB_FALSE;
    lib_u64 ticks;
    for (value = 0u; value < 0xedu; ++value) {
        const lib_u64 initial = 120u * (1u + ((value >> 5u) & 3u));
        const lib_u64 interval = 2u * ((8u + (value & 7u)) << ((value >> 3u) & 3u));
        x86_keyboard_reset(keyboard);
        x86_keyboard_set_typematic_timing(keyboard, 240u, 48u);
        failed |= !reply_is(keyboard, 0xf3u, link, 1u, 0xfau, 0u, 0u);
        failed |= !reply_is(keyboard, (lib_u8)value, link, 1u, 0xfau, 0u, 0u);
        failed |= admit_byte(keyboard, 0x1cu) != LIB_STATUS_OK;
        failed |= x86_keyboard_ticks_until_repeat(keyboard, &ticks) != LIB_STATUS_OK;
        failed |= ticks != initial;
        probe->repeats = 0u;
        failed |= x86_keyboard_advance(keyboard, initial - 1u, receive_repeat, probe);
        failed |= probe->repeats != 0u;
        failed |= !x86_keyboard_advance(keyboard, 1u, receive_repeat, probe);
        failed |= probe->repeats != 1u || probe->repeated != 0x1cu;
        failed |= x86_keyboard_ticks_until_repeat(keyboard, &ticks) != LIB_STATUS_OK;
        failed |= ticks != interval;
        failed |= !x86_keyboard_advance(keyboard, interval * 3u + 1u, receive_repeat, probe);
        failed |= probe->repeats != 4u;
        failed |= x86_keyboard_ticks_until_repeat(keyboard, &ticks) != LIB_STATUS_OK;
        failed |= ticks != interval - 1u;
        failed |= admit_byte(keyboard, 0xf0u) != LIB_STATUS_OK;
        failed |= admit_byte(keyboard, 0x1cu) != LIB_STATUS_OK;
        failed |= x86_keyboard_ticks_until_repeat(keyboard, &ticks) != LIB_STATUS_INVALID_STATE;
        failed |= x86_keyboard_advance(keyboard, 10000u, receive_repeat, probe);
    }
    return failed;
}

static lib_bool lifecycle(x86_keyboard *keyboard, const x86_keyboard_link *link)
{
    keyboard_probe *probe = link->context;
    lib_bool failed = LIB_FALSE;
    lib_u64 ticks = 0u;
    x86_keyboard_reset(keyboard);
    failed |= !x86_keyboard_start(keyboard) || x86_keyboard_start(keyboard);
    failed |= !x86_keyboard_get_signals(keyboard).bat_ready;
    failed |= !x86_keyboard_take_bat(keyboard) || x86_keyboard_take_bat(keyboard);
    failed |= !reply_is(keyboard, 0xffu, link, 1u, 0xfau, 0u, 0u);
    failed |= !x86_keyboard_take_bat(keyboard) || x86_keyboard_start(keyboard);
    failed |= !reply_is(keyboard, 0xf5u, link, 1u, 0xfau, 0u, 0u);
    failed |= x86_keyboard_get_signals(keyboard).scanning;
    failed |= admit_byte(keyboard, 0x1cu) != LIB_STATUS_INVALID_STATE;
    failed |= !reply_is(keyboard, 0xf6u, link, 1u, 0xfau, 0u, 0u);
    failed |= !x86_keyboard_get_signals(keyboard).scanning;
    failed |= !reply_is(keyboard, 0xf4u, link, 1u, 0xfau, 0u, 0u);
    failed |= !x86_keyboard_get_signals(keyboard).scanning;
    failed |= !reply_is(keyboard, 0xedu, link, 1u, 0xfau, 0u, 0u);
    x86_keyboard_cancel_parameter(keyboard);
    failed |= !reply_is(keyboard, 0xeeu, link, 1u, 0xeeu, 0u, 0u);
    failed |= x86_keyboard_get_signals(keyboard).leds != 0u;
    x86_keyboard_note_output(keyboard, 0x9cu);
    x86_keyboard_note_output(keyboard, 0xfeu);
    failed |= !reply_is(keyboard, 0xfeu, link, 1u, 0x9cu, 0u, 0u);
    x86_keyboard_set_typematic_timing(keyboard, 240u, 48u);
    x86_keyboard_reset(keyboard);
    failed |= admit_byte(keyboard, 0x1cu) != LIB_STATUS_OK;
    failed |= x86_keyboard_ticks_until_repeat(keyboard, &ticks) != LIB_STATUS_OK || ticks != 240u;
    failed |= !reply_is(keyboard, 0xf0u, link, 1u, 0xfau, 0u, 0u);
    failed |= !reply_is(keyboard, 1u, link, 1u, 0xfau, 0u, 0u);
    failed |= admit_byte(keyboard, 0x30u) != LIB_STATUS_OK;
    probe->repeats = 0u;
    failed |= !x86_keyboard_advance(keyboard, 240u, receive_repeat, probe);
    failed |= probe->repeats != 1u || probe->repeated != 0x30u;
    failed |= admit_byte(keyboard, 0xb0u) != LIB_STATUS_OK;
    failed |= x86_keyboard_ticks_until_repeat(keyboard, &ticks) != LIB_STATUS_INVALID_STATE;
    return failed;
}

static lib_bool command_order(x86_keyboard *keyboard, const x86_keyboard_link *link)
{
    keyboard_probe *probe = link->context;
    lib_bool failed = LIB_FALSE;
    lib_u32 resets;
    lib_u8 leds;
    x86_keyboard_reset(keyboard);
    failed |= !reply_is(keyboard, 0xf5u, link, 1u, 0xfau, 0u, 0u);
    resets = probe->resets;
    failed |= probe->at_reply.scanning || probe->resets_at_reply != resets;
    failed |= !reply_is(keyboard, 0xffu, link, 1u, 0xfau, 0u, 0u);
    failed |= probe->at_reply.scanning || probe->at_reply.bat_ready ||
        probe->resets_at_reply != resets;
    failed |= !x86_keyboard_get_signals(keyboard).scanning ||
        !x86_keyboard_get_signals(keyboard).bat_ready || probe->resets != resets + 1u;
    x86_keyboard_clear_bat(keyboard);
    failed |= x86_keyboard_take_bat(keyboard);
    failed |= !reply_is(keyboard, 0xedu, link, 1u, 0xfau, 0u, 0u);
    failed |= !reply_is(keyboard, 7u, link, 1u, 0xfau, 0u, 0u);
    resets = probe->resets;
    failed |= !reply_is(keyboard, 0xf6u, link, 1u, 0xfau, 0u, 0u);
    failed |= probe->at_reply.leds != 7u || probe->at_reply.scan_set != 2u ||
        probe->resets_at_reply != resets + 1u;
    failed |= !reply_is(keyboard, 0xf5u, link, 1u, 0xfau, 0u, 0u);
    failed |= x86_keyboard_get_signals(keyboard).leds != 7u ||
        x86_keyboard_get_signals(keyboard).scanning;
    failed |= !reply_is(keyboard, 0xf6u, link, 1u, 0xfau, 0u, 0u);
    failed |= x86_keyboard_get_signals(keyboard).leds != 7u ||
        !x86_keyboard_get_signals(keyboard).scanning;
    failed |= !reply_is(keyboard, 0xffu, link, 1u, 0xfau, 0u, 0u);
    failed |= x86_keyboard_get_signals(keyboard).leds != 0u;
    for (leds = 0u; leds < 8u; ++leds) {
        failed |= !reply_is(keyboard, 0xedu, link, 1u, 0xfau, 0u, 0u);
        failed |= !reply_is(keyboard, leds, link, 1u, 0xfau, 0u, 0u);
        failed |= !reply_is(keyboard, 0xf5u, link, 1u, 0xfau, 0u, 0u);
        failed |= x86_keyboard_get_signals(keyboard).leds != leds ||
            x86_keyboard_get_signals(keyboard).scanning;
        failed |= !reply_is(keyboard, 0xf6u, link, 1u, 0xfau, 0u, 0u);
        failed |= x86_keyboard_get_signals(keyboard).leds != leds ||
            !x86_keyboard_get_signals(keyboard).scanning;
    }
    return failed;
}

static lib_bool parameter_scanning(x86_keyboard *keyboard,
    const x86_keyboard_link *link)
{
    static const lib_u8 parameters[] = { 0xedu, 0xf0u, 0xf3u };
    keyboard_probe *probe = link->context;
    lib_bool failed = LIB_FALSE;
    lib_u8 enabled;
    lib_size parameter;
    lib_size command;
    lib_u64 ticks;
    for (enabled = 0u; enabled < 2u; ++enabled) {
        for (parameter = 0u; parameter < sizeof(parameters); ++parameter) {
            x86_keyboard_reset(keyboard);
            x86_keyboard_set_typematic_timing(keyboard, 240u, 48u);
            if (enabled == 0u) {
                failed |= !reply_is(keyboard, 0xf5u, link, 1u, 0xfau, 0u, 0u);
            } else failed |= admit_byte(keyboard, 0x1cu) != LIB_STATUS_OK;
            failed |= !reply_is(keyboard, parameters[parameter], link, 1u, 0xfau, 0u, 0u);
            failed |= x86_keyboard_get_signals(keyboard).scanning;
            failed |= admit_byte(keyboard, 0x1cu) != LIB_STATUS_INVALID_STATE;
            probe->repeats = 0u;
            failed |= x86_keyboard_advance(keyboard, 10000u, receive_repeat, probe);
            failed |= probe->repeats != 0u;
            failed |= x86_keyboard_ticks_until_repeat(keyboard, &ticks) != LIB_STATUS_INVALID_STATE;
            failed |= !reply_is(keyboard, 1u, link, 1u, 0xfau, 0u, 0u);
            failed |= x86_keyboard_get_signals(keyboard).scanning != (enabled != 0u);
            if (parameters[parameter] == 0xf0u) {
                failed |= x86_keyboard_ticks_until_repeat(keyboard, &ticks) != LIB_STATUS_INVALID_STATE;
            }
            failed |= !reply_is(keyboard, parameters[parameter], link, 1u, 0xfau, 0u, 0u);
            x86_keyboard_cancel_parameter(keyboard);
            failed |= x86_keyboard_get_signals(keyboard).scanning != (enabled != 0u);
            for (command = 0xedu; command <= 0xffu; ++command) {
                lib_u8 expected[3];
                lib_u8 count = 0u;
                x86_keyboard_signals signals = { 0 };
                lib_u8 pass;
                /* Interrupted dispatch must match the same command at idle. */
                for (pass = 0u; pass < 2u; ++pass) {
                    x86_keyboard_reset(keyboard);
                    failed |= !reply_is(keyboard, 0xedu, link, 1u, 0xfau, 0u, 0u);
                    failed |= !reply_is(keyboard, 7u, link, 1u, 0xfau, 0u, 0u);
                    if (enabled == 0u) {
                        failed |= !reply_is(keyboard, 0xf5u, link, 1u, 0xfau, 0u, 0u);
                    }
                    if (pass != 0u) {
                        failed |= !reply_is(keyboard, parameters[parameter], link, 1u, 0xfau, 0u, 0u);
                    }
                    x86_keyboard_write(keyboard, (lib_u8)command, link);
                    if (pass == 0u) {
                        count = probe->count;
                        lib_memory_copy(expected, probe->bytes, count);
                        signals = x86_keyboard_get_signals(keyboard);
                    } else {
                        const x86_keyboard_signals actual = x86_keyboard_get_signals(keyboard);
                        failed |= probe->count != count ||
                            lib_memory_compare(probe->bytes, expected, count) != 0;
                        failed |= actual.leds != signals.leds ||
                            actual.scanning != signals.scanning || actual.scan_set != signals.scan_set;
                    }
                }
            }
        }
    }
    return failed;
}

static lib_bool sequence_admission(x86_keyboard *keyboard,
    const x86_keyboard_link *link)
{
    keyboard_probe *probe = link->context;
    lib_bool failed = LIB_FALSE;
    lib_u8 output[8];
    lib_size count;
    lib_u64 ticks;
    const lib_u8 invalid[] = { 0xe0u, 0xf0u, 0xf0u };
    const lib_u8 pause[] = { 0xe1u, 0x14u, 0x77u, 0xe1u, 0xf0u, 0x14u, 0xf0u, 0x77u };

    for (lib_u8 set = 1u; set <= 2u; ++set) {
        const lib_u8 make[] = { 0xe0u, set == 1u ? 0x48u : 0x75u };
        const lib_u8 plain_break[] = { set == 1u ? 0xc8u : 0xf0u, 0x75u };
        const lib_u8 extended_break[] = { 0xe0u,
            set == 1u ? 0xc8u : 0xf0u, 0x75u };
        x86_keyboard_reset(keyboard);
        x86_keyboard_set_typematic_timing(keyboard, 100u, 24u);
        failed |= !reply_is(keyboard, 0xf0u, link, 1u, 0xfau, 0u, 0u);
        failed |= !reply_is(keyboard, set, link, 1u, 0xfau, 0u, 0u);
        failed |= x86_keyboard_admit(keyboard, make, sizeof(make), output,
            sizeof(output), &count) != LIB_STATUS_OK || count != sizeof(make);
        failed |= lib_memory_compare(output, make, sizeof(make)) != 0;
        failed |= x86_keyboard_get_signals(keyboard).repeat_bytes != 2u;
        failed |= x86_keyboard_advance(keyboard, 25u, receive_repeat, probe);
        failed |= x86_keyboard_admit(keyboard, make, sizeof(make), output,
            1u, &count) != LIB_STATUS_NO_MEMORY || count != 0u;
        failed |= x86_keyboard_ticks_until_repeat(keyboard, &ticks) != LIB_STATUS_OK || ticks != 75u;
        failed |= x86_keyboard_admit(keyboard, plain_break, set == 1u ? 1u : 2u,
            output, sizeof(output), &count) != LIB_STATUS_OK;
        failed |= x86_keyboard_get_signals(keyboard).repeat_bytes != 2u;
        probe->repeat_status = LIB_STATUS_NO_MEMORY;
        failed |= x86_keyboard_advance(keyboard, 75u, receive_repeat, probe);
        failed |= x86_keyboard_ticks_until_repeat(keyboard, &ticks) != LIB_STATUS_OK || ticks != 0u;
        probe->repeat_status = LIB_STATUS_OK;
        failed |= !x86_keyboard_advance(keyboard, 0u, receive_repeat, probe);
        failed |= probe->repeat_count != 2u ||
            lib_memory_compare(probe->repeat_bytes, make, sizeof(make)) != 0;
        failed |= x86_keyboard_ticks_until_repeat(keyboard, &ticks) != LIB_STATUS_OK || ticks != 24u;
        failed |= x86_keyboard_admit(keyboard, extended_break, set == 1u ? 2u : 3u,
            output, sizeof(output), &count) != LIB_STATUS_OK;
        failed |= x86_keyboard_ticks_until_repeat(keyboard, &ticks) != LIB_STATUS_INVALID_STATE;
    }
    x86_keyboard_reset(keyboard);
    failed |= admit_byte(keyboard, 0x1cu) != LIB_STATUS_OK;
    failed |= x86_keyboard_admit(keyboard, pause, sizeof(pause), output,
        sizeof(output), &count) != LIB_STATUS_OK || count != sizeof(pause);
    failed |= !x86_keyboard_advance(keyboard, 100u, receive_repeat, probe);
    failed |= probe->repeat_count != 1u || probe->repeated != 0x1cu;
    failed |= x86_keyboard_admit(keyboard, invalid, sizeof(invalid), output,
        sizeof(output), &count) != LIB_STATUS_INVALID_ARGUMENT || count != 0u;
    failed |= x86_keyboard_ticks_until_repeat(keyboard, &ticks) != LIB_STATUS_OK || ticks != 24u;
    /* A rejected suffix must not consume the previously accepted prefix. */
    failed |= admit_byte(keyboard, 0xe0u) != LIB_STATUS_OK;
    failed |= x86_keyboard_admit(keyboard, invalid + 1u, 2u, output,
        sizeof(output), &count) != LIB_STATUS_INVALID_ARGUMENT || count != 0u;
    failed |= admit_byte(keyboard, 0x75u) != LIB_STATUS_OK;
    failed |= x86_keyboard_get_signals(keyboard).repeat_bytes != 2u;
    x86_keyboard_set_typematic_timing(keyboard, 0u, 0u);
    failed |= x86_keyboard_advance(keyboard, 10000u, receive_repeat, probe);
    failed |= x86_keyboard_ticks_until_repeat(keyboard, &ticks) != LIB_STATUS_INVALID_STATE;
    /* A rate option can round a small caller-configured period to zero. */
    x86_keyboard_set_typematic_timing(keyboard, 100u, 1u);
    failed |= admit_byte(keyboard, 0x1cu) != LIB_STATUS_OK;
    failed |= !reply_is(keyboard, 0xf3u, link, 1u, 0xfau, 0u, 0u);
    failed |= !reply_is(keyboard, 0u, link, 1u, 0xfau, 0u, 0u);
    failed |= x86_keyboard_advance(keyboard, 10000u, receive_repeat, probe);
    failed |= x86_keyboard_ticks_until_repeat(keyboard, &ticks) != LIB_STATUS_INVALID_STATE;
    return failed;
}

static lib_bool enable_clears_prefix(x86_keyboard *keyboard,
    const x86_keyboard_link *link)
{
    keyboard_probe *probe = link->context;
    lib_bool failed = LIB_FALSE;
    lib_u64 ticks;
    lib_u32 resets;
    x86_keyboard_reset(keyboard);
    x86_keyboard_set_typematic_timing(keyboard, 100u, 24u);
    failed |= admit_byte(keyboard, 0xe0u) != LIB_STATUS_OK;
    resets = probe->resets;
    failed |= !reply_is(keyboard, 0xf4u, link, 1u, 0xfau, 0u, 0u);
    failed |= probe->resets_at_reply != resets + 1u;
    failed |= admit_byte(keyboard, 0x75u) != LIB_STATUS_OK;
    failed |= x86_keyboard_get_signals(keyboard).repeat_bytes != 1u;
    failed |= x86_keyboard_ticks_until_repeat(keyboard, &ticks) != LIB_STATUS_OK || ticks != 100u;
    failed |= admit_byte(keyboard, 0xf0u) != LIB_STATUS_OK;
    failed |= !reply_is(keyboard, 0xf4u, link, 1u, 0xfau, 0u, 0u);
    failed |= x86_keyboard_ticks_until_repeat(keyboard, &ticks) != LIB_STATUS_INVALID_STATE;
    failed |= admit_byte(keyboard, 0x1cu) != LIB_STATUS_OK;
    failed |= x86_keyboard_get_signals(keyboard).repeat_bytes != 1u;
    return failed;
}

static lib_u8 set3_source_type(lib_u8 code)
{
    /* Independent source groups from IBM 4-58--60, excluding 102-key-only
     * codes 13h/53h. Values describe repeat/break behavior, not lib booleans. */
    static const lib_u8 typematic[] = {
        0x0e,0x16,0x1e,0x26,0x25,0x2e,0x36,0x3d,0x3e,0x46,0x45,0x4e,0x55,0x66,
        0x0d,0x15,0x1d,0x24,0x2d,0x2c,0x35,0x3c,0x43,0x44,0x4d,0x54,0x5b,0x5c,
        0x1c,0x1b,0x23,0x2b,0x34,0x33,0x3b,0x42,0x4b,0x4c,0x52,0x5a,0x1a,0x22,
        0x21,0x2a,0x32,0x31,0x3a,0x41,0x49,0x4a,0x29,0x64,0x61,0x63,0x60,0x6a,0x7c
    };
    static const lib_u8 make_break[] = { 0x14,0x12,0x59,0x11,0x19 };
    static const lib_u8 make_only[] = {
        0x39,0x58,0x67,0x6e,0x65,0x6f,0x6d,0x76,0x6c,0x6b,0x69,0x77,0x75,
        0x73,0x72,0x70,0x7e,0x7d,0x74,0x7a,0x71,0x84,0x79,0x08,0x07,0x0f,
        0x17,0x1f,0x27,0x2f,0x37,0x3f,0x47,0x4f,0x56,0x5e,0x57,0x5f,0x62
    };
    for (lib_size index = 0u; index < sizeof(typematic); ++index)
        if (typematic[index] == code) return 5u;
    for (lib_size index = 0u; index < sizeof(make_break); ++index)
        if (make_break[index] == code) return 3u;
    for (lib_size index = 0u; index < sizeof(make_only); ++index)
        if (make_only[index] == code) return 1u;
    return 0u;
}

static lib_bool set3_key_contract(x86_keyboard *keyboard,
    const x86_keyboard_link *link, lib_u8 code, lib_u8 type)
{
    keyboard_probe *probe = link->context;
    const lib_u8 released[] = { 0xf0u, code };
    lib_u8 output[2] = { 0xa5u, 0xa5u };
    lib_size count;
    lib_u64 ticks;
    lib_bool failed = LIB_FALSE;
    probe->repeat_status = LIB_STATUS_OK;
    probe->repeats = 0u;
    failed |= x86_keyboard_admit(keyboard, &code, 1u, output, sizeof(output), &count) !=
        LIB_STATUS_OK || count != 1u || output[0] != code;
    failed |= x86_keyboard_advance(keyboard, 100u, receive_repeat, probe) != ((type & 4u) != 0u);
    failed |= probe->repeats != ((type & 4u) != 0u ? 1u : 0u);
    if ((type & 4u) != 0u)
        failed |= probe->repeat_count != 1u || probe->repeat_bytes[0] != code;
    output[0] = output[1] = 0xa5u;
    failed |= x86_keyboard_admit(keyboard, released, sizeof(released), output,
        (type & 2u) != 0u ? sizeof(output) : 0u, &count) != LIB_STATUS_OK;
    if ((type & 2u) != 0u) {
        failed |= count != 2u || lib_memory_compare(output, released, sizeof(released)) != 0;
    } else failed |= count != 0u || output[0] != 0xa5u || output[1] != 0xa5u;
    failed |= x86_keyboard_ticks_until_repeat(keyboard, &ticks) != LIB_STATUS_INVALID_STATE;
    return failed;
}

static lib_bool set3_key_types(x86_keyboard *keyboard,
    const x86_keyboard_link *link)
{
    static const lib_u8 commands[] = { 0u, 0xf7u, 0xf8u, 0xf9u, 0xfau };
    static const lib_u8 types[] = { 0u, 5u, 3u, 1u, 7u };
    lib_bool failed = LIB_FALSE;
    lib_size keys = 0u;
    for (lib_u16 code = 0u; code <= 0x84u; ++code) {
        const lib_u8 source_type = set3_source_type((lib_u8)code);
        if (source_type == 0u) continue;
        ++keys;
        for (lib_size mode = 0u; mode < sizeof(commands); ++mode) {
            x86_keyboard_reset(keyboard);
            x86_keyboard_set_typematic_timing(keyboard, 100u, 24u);
            failed |= !reply_is(keyboard, 0xf0u, link, 1u, 0xfau, 0u, 0u);
            failed |= !reply_is(keyboard, 3u, link, 1u, 0xfau, 0u, 0u);
            if (commands[mode] != 0u)
                failed |= !reply_is(keyboard, commands[mode], link, 1u, 0xfau, 0u, 0u);
            failed |= set3_key_contract(keyboard, link, (lib_u8)code,
                mode == 0u ? source_type : types[mode]);
        }
        for (lib_u8 mode = 0u; mode < 3u; ++mode) {
            x86_keyboard_reset(keyboard);
            x86_keyboard_set_typematic_timing(keyboard, 100u, 24u);
            failed |= !reply_is(keyboard, 0xf0u, link, 1u, 0xfau, 0u, 0u);
            failed |= !reply_is(keyboard, 3u, link, 1u, 0xfau, 0u, 0u);
            failed |= !reply_is(keyboard, (lib_u8)(0xfbu + mode), link, 1u, 0xfau, 0u, 0u);
            failed |= !reply_is(keyboard, (lib_u8)code, link, 1u, 0xfau, 0u, 0u);
            failed |= x86_keyboard_get_signals(keyboard).scanning;
            failed |= !reply_is(keyboard, 0xf4u, link, 1u, 0xfau, 0u, 0u);
            failed |= set3_key_contract(keyboard, link, (lib_u8)code, types[mode + 1u]);
        }
    }
    failed |= keys != 101u;
    return failed;
}

static lib_bool set3_capacity_and_reset(x86_keyboard *keyboard,
    const x86_keyboard_link *link)
{
    const lib_u8 released[] = { 0xf0u, 0x1cu };
    const lib_u8 malformed[] = { 0xf0u, 0xf0u };
    lib_u8 output[4];
    lib_size count;
    lib_u64 ticks;
    lib_bool failed = LIB_FALSE;
    x86_keyboard_reset(keyboard);
    x86_keyboard_set_typematic_timing(keyboard, 100u, 24u);
    failed |= !reply_is(keyboard, 0xf0u, link, 1u, 0xfau, 0u, 0u);
    failed |= !reply_is(keyboard, 3u, link, 1u, 0xfau, 0u, 0u);
    failed |= !reply_is(keyboard, 0xfau, link, 1u, 0xfau, 0u, 0u);
    failed |= x86_keyboard_admit(keyboard, released + 1u, 1u, output, 1u, &count) != LIB_STATUS_OK;
    failed |= x86_keyboard_admit(keyboard, released, 2u, output, 1u, &count) !=
        LIB_STATUS_NO_MEMORY || count != 0u;
    failed |= x86_keyboard_ticks_until_repeat(keyboard, &ticks) != LIB_STATUS_OK || ticks != 100u;
    failed |= x86_keyboard_admit(keyboard, released, 1u, output, 0u, &count) !=
        LIB_STATUS_OK || count != 0u;
    failed |= x86_keyboard_admit(keyboard, released + 1u, 1u, output, 1u, &count) !=
        LIB_STATUS_NO_MEMORY || count != 0u;
    failed |= x86_keyboard_admit(keyboard, released + 1u, 1u, output, 2u, &count) !=
        LIB_STATUS_OK || count != 2u || lib_memory_compare(output, released, 2u) != 0;
    failed |= x86_keyboard_admit(keyboard, malformed, 2u, output, sizeof(output), &count) !=
        LIB_STATUS_INVALID_ARGUMENT || count != 0u;
    failed |= x86_keyboard_admit(keyboard, released + 1u, 1u, output, 1u, &count) !=
        LIB_STATUS_OK || count != 1u || output[0] != 0x1cu;
    failed |= !reply_is(keyboard, 0xf6u, link, 1u, 0xfau, 0u, 0u);
    failed |= x86_keyboard_get_signals(keyboard).scan_set != 2u;
    failed |= !reply_is(keyboard, 0xf0u, link, 1u, 0xfau, 0u, 0u);
    failed |= !reply_is(keyboard, 3u, link, 1u, 0xfau, 0u, 0u);
    failed |= !reply_is(keyboard, 0xf0u, link, 1u, 0xfau, 0u, 0u);
    failed |= !reply_is(keyboard, 0u, link, 2u, 0xfau, 3u, 0u);
    failed |= set3_key_contract(keyboard, link, 0x1cu, 5u);
    failed |= set3_key_contract(keyboard, link, 0x07u, 1u);
    for (lib_u16 code = 0u; code < 0xedu; ++code) {
        if (set3_source_type((lib_u8)code) != 0u) continue;
        failed |= !reply_is(keyboard, 0xfcu, link, 1u, 0xfau, 0u, 0u);
        failed |= !reply_is(keyboard, (lib_u8)code, link, 1u, 0xfeu, 0u, 0u);
        failed |= x86_keyboard_get_signals(keyboard).scanning;
        x86_keyboard_cancel_parameter(keyboard);
        failed |= !reply_is(keyboard, 0xf4u, link, 1u, 0xfau, 0u, 0u);
    }
    return failed;
}

static lib_bool set3_legacy_and_transactions(x86_keyboard *keyboard,
    const x86_keyboard_link *link)
{
    lib_bool failed = LIB_FALSE;
    lib_u8 output[2];
    lib_size count;
    lib_u64 ticks;
    for (lib_u8 set = 1u; set <= 2u; ++set) {
        const lib_u8 make = set == 1u ? 0x1eu : 0x1cu;
        for (lib_u8 command = 0xf7u; command <= 0xfau; ++command) {
            x86_keyboard_reset(keyboard);
            x86_keyboard_set_typematic_timing(keyboard, 100u, 24u);
            failed |= !reply_is(keyboard, 0xf0u, link, 1u, 0xfau, 0u, 0u);
            failed |= !reply_is(keyboard, set, link, 1u, 0xfau, 0u, 0u);
            failed |= !reply_is(keyboard, command, link, 1u, 0xfau, 0u, 0u);
            failed |= x86_keyboard_admit(keyboard, &make, 1u, output, sizeof(output), &count) !=
                LIB_STATUS_OK || count != 1u || output[0] != make;
            /* Set-3 key types do not remove native Set-1/Set-2 typematic. */
            failed |= x86_keyboard_ticks_until_repeat(keyboard, &ticks) != LIB_STATUS_OK || ticks != 100u;
            failed |= !reply_is(keyboard, 0xf0u, link, 1u, 0xfau, 0u, 0u);
            failed |= !reply_is(keyboard, 3u, link, 1u, 0xfau, 0u, 0u);
            failed |= set3_key_contract(keyboard, link, 0x1cu,
                command == 0xf7u ? 5u : command == 0xf8u ? 3u : command == 0xf9u ? 1u : 7u);
        }
    }
    for (lib_u8 command = 0xf7u; command <= 0xfau; ++command) {
        x86_keyboard_reset(keyboard);
        failed |= !reply_is(keyboard, 0xf5u, link, 1u, 0xfau, 0u, 0u);
        failed |= !reply_is(keyboard, command, link, 1u, 0xfau, 0u, 0u);
        failed |= x86_keyboard_get_signals(keyboard).scanning;
    }
    for (lib_u8 parameter = 0xfbu; parameter <= 0xfdu; ++parameter) {
        for (lib_u16 command = 0xedu; command <= 0xffu; ++command) {
            keyboard_probe *probe = link->context;
            lib_u8 expected[3];
            lib_u8 count = 0u;
            x86_keyboard_signals expected_signals = {0};
            for (lib_u8 interrupted = 0u; interrupted < 2u; ++interrupted) {
                x86_keyboard_reset(keyboard);
                failed |= !reply_is(keyboard, 0xf5u, link, 1u, 0xfau, 0u, 0u);
                if (interrupted != 0u)
                    failed |= !reply_is(keyboard, parameter, link, 1u, 0xfau, 0u, 0u);
                x86_keyboard_write(keyboard, (lib_u8)command, link);
                const x86_keyboard_signals signals = x86_keyboard_get_signals(keyboard);
                if (interrupted == 0u) {
                    count = probe->count;
                    lib_memory_copy(expected, probe->bytes, count);
                    expected_signals = signals;
                } else failed |= probe->count != count ||
                    lib_memory_compare(probe->bytes, expected, count) != 0 ||
                    signals.scanning != expected_signals.scanning ||
                    signals.scan_set != expected_signals.scan_set || signals.leds != expected_signals.leds;
            }
        }
    }
    return failed;
}

int main(void)
{
    x86_keyboard *keyboard = LIB_NULL;
    keyboard_probe probe = { 0 };
    const x86_keyboard_link link = { receive_reply, reset_stream, &probe };
    lib_bool failed;
    if (x86_keyboard_create(&keyboard) != LIB_STATUS_OK) return 1;
    probe.keyboard = keyboard;
    failed = command_matrix(keyboard, &link);
    failed |= repeat_matrix(keyboard, &link);
    failed |= lifecycle(keyboard, &link);
    failed |= command_order(keyboard, &link);
    failed |= parameter_scanning(keyboard, &link);
    failed |= sequence_admission(keyboard, &link);
    failed |= enable_clears_prefix(keyboard, &link);
    failed |= set3_key_types(keyboard, &link);
    failed |= set3_capacity_and_reset(keyboard, &link);
    failed |= set3_legacy_and_transactions(keyboard, &link);
    x86_keyboard_destroy(keyboard);
    return failed ? 1 : 0;
}
