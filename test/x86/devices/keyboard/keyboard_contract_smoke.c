#include "x86/devices/keyboard/keyboard_interface.h"

typedef struct keyboard_probe {
    x86_keyboard *keyboard;
    lib_u8 bytes[3];
    lib_u8 count;
    lib_u32 resets;
    lib_u32 repeats;
    lib_u8 repeated;
    x86_keyboard_signals at_reply;
    lib_u32 resets_at_reply;
} keyboard_probe;

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

static void receive_repeat(void *context, lib_u8 byte)
{
    keyboard_probe *probe = context;
    probe->repeated = byte;
    ++probe->repeats;
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
    for (value = 0u; value < 256u; ++value) {
        lib_u8 count = 1u;
        lib_u8 first = 0xfau;
        lib_u8 second = 0u;
        lib_u8 third = 0u;
        x86_keyboard_reset(keyboard);
        switch (value) {
        case 0xedu: case 0xf0u: case 0xf3u: case 0xf4u: case 0xf5u:
        case 0xf6u: case 0xf7u: case 0xfbu: case 0xfcu: case 0xfdu:
        case 0xffu: break;
        case 0xeeu: first = 0xeeu; break;
        case 0xfeu: first = 0u; break;
        case 0xf2u: count = 3u; second = 0xabu; third = 0x83u; break;
        default: first = 0xfeu; break;
        }
        failed |= !reply_is(keyboard, (lib_u8)value, link, count, first, second, third);
    }
    for (value = 0u; value < 256u; ++value) {
        x86_keyboard_reset(keyboard);
        failed |= !reply_is(keyboard, 0xedu, link, 1u, 0xfau, 0u, 0u);
        failed |= !reply_is(keyboard, (lib_u8)value, link, 1u, 0xfau, 0u, 0u);
        failed |= x86_keyboard_get_signals(keyboard).leds != (value & 7u);
        failed |= !reply_is(keyboard, 0xf0u, link, 1u, 0xfau, 0u, 0u);
        failed |= !reply_is(keyboard, (lib_u8)value, link,
            value == 0u ? 2u : 1u, value <= 2u ? 0xfau : 0xfeu, 2u, 0u);
        failed |= x86_keyboard_get_signals(keyboard).scan_set != (value == 1u ? 1u : 2u);
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
    for (value = 0u; value < 256u; ++value) {
        const lib_u64 initial = 120u * (1u + ((value >> 5u) & 3u));
        const lib_u64 interval = 2u * ((8u + (value & 7u)) << ((value >> 3u) & 3u));
        x86_keyboard_reset(keyboard);
        x86_keyboard_set_typematic_timing(keyboard, 240u, 48u);
        failed |= !reply_is(keyboard, 0xf3u, link, 1u, 0xfau, 0u, 0u);
        failed |= !reply_is(keyboard, (lib_u8)value, link, 1u, 0xfau, 0u, 0u);
        failed |= x86_keyboard_admit(keyboard, 0x1cu) != LIB_STATUS_OK;
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
        failed |= x86_keyboard_admit(keyboard, 0xf0u) != LIB_STATUS_OK;
        failed |= x86_keyboard_admit(keyboard, 0x1cu) != LIB_STATUS_OK;
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
    failed |= x86_keyboard_admit(keyboard, 0x1cu) != LIB_STATUS_INVALID_STATE;
    failed |= !reply_is(keyboard, 0xf6u, link, 1u, 0xfau, 0u, 0u);
    /* The qualified existing F6 model preserves the scan-enable latch. */
    failed |= x86_keyboard_get_signals(keyboard).scanning;
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
    failed |= x86_keyboard_admit(keyboard, 0x1cu) != LIB_STATUS_OK;
    failed |= x86_keyboard_ticks_until_repeat(keyboard, &ticks) != LIB_STATUS_OK || ticks != 240u;
    failed |= !reply_is(keyboard, 0xf0u, link, 1u, 0xfau, 0u, 0u);
    failed |= !reply_is(keyboard, 1u, link, 1u, 0xfau, 0u, 0u);
    failed |= x86_keyboard_admit(keyboard, 0x30u) != LIB_STATUS_OK;
    probe->repeats = 0u;
    failed |= !x86_keyboard_advance(keyboard, 240u, receive_repeat, probe);
    failed |= probe->repeats != 1u || probe->repeated != 0x30u;
    failed |= x86_keyboard_admit(keyboard, 0xb0u) != LIB_STATUS_OK;
    failed |= x86_keyboard_ticks_until_repeat(keyboard, &ticks) != LIB_STATUS_INVALID_STATE;
    return failed;
}

static lib_bool command_order(x86_keyboard *keyboard, const x86_keyboard_link *link)
{
    keyboard_probe *probe = link->context;
    lib_bool failed = LIB_FALSE;
    lib_u32 resets;
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
    failed |= probe->at_reply.leds != 0u || probe->at_reply.scan_set != 2u ||
        probe->resets_at_reply != resets + 1u;
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
    x86_keyboard_destroy(keyboard);
    return failed ? 1 : 0;
}
