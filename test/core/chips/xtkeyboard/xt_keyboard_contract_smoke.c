#include "lib/types/test.h"
#include "core/chips/xtkeyboard/xtkeyboard_interface.h"

typedef struct receiver {
    lib_u8 bytes[32];
    lib_size count;
    lib_size attempts;
    lib_bool blocked;
} receiver;

static lib_status receive(void *context, lib_u8 byte)
{
    receiver *sink = context;
    ++sink->attempts;
    if (sink->blocked) return LIB_STATUS_INVALID_STATE;
    lib_test_assert(sink->count < sizeof(sink->bytes));
    sink->bytes[sink->count++] = byte;
    return LIB_STATUS_OK;
}

static void refused_completion(lib_bool bat, lib_bool reset)
{
    const x86_xt_keyboard_timing timing = {12500u, 300000u, 60u, 25u};
    receiver sink = {.blocked = LIB_TRUE};
    x86_xt_keyboard *keyboard = LIB_NULL;
    lib_u8 byte = 0x9eu;
    lib_u64 deadline = 0u;
    lib_test_assert(x86_xt_keyboard_create(&timing, receive, &sink, &keyboard) == LIB_STATUS_OK);
    if (bat) x86_xt_keyboard_advance(keyboard, 12500u);
    x86_xt_keyboard_set_lines(keyboard, LIB_FALSE, LIB_FALSE);
    if (bat) x86_xt_keyboard_advance(keyboard, 300000u);
    else lib_test_assert(x86_xt_keyboard_receive_native_bytes(keyboard, &byte, 1u) == LIB_STATUS_OK);
    lib_test_assert(x86_xt_keyboard_ticks_until_event(keyboard, &deadline) == LIB_STATUS_OK && deadline == 60u);
    x86_xt_keyboard_advance(keyboard, 259u);
    lib_test_assert(sink.attempts == 0u);
    x86_xt_keyboard_advance(keyboard, 1u);
    lib_test_assert(sink.attempts == 1u && sink.count == 0u);
    x86_xt_keyboard_advance(keyboard, 1000000u);
    lib_test_assert(sink.attempts == 1u);
    lib_test_assert(x86_xt_keyboard_ticks_until_event(keyboard, &deadline) == LIB_STATUS_UNSUPPORTED);
    if (reset) x86_xt_keyboard_reset(keyboard);
    sink.blocked = LIB_FALSE;
    x86_xt_keyboard_set_lines(keyboard, LIB_TRUE, LIB_TRUE);
    x86_xt_keyboard_receiver_ready(keyboard);
    lib_test_assert(sink.count == 0u);
    x86_xt_keyboard_set_lines(keyboard, LIB_FALSE, LIB_FALSE);
    lib_test_assert(sink.count == (reset ? 0u : 1u));
    if (!reset) lib_test_assert(sink.bytes[0] == (bat ? 0xaau : byte));
    x86_xt_keyboard_receiver_ready(keyboard);
    x86_xt_keyboard_advance(keyboard, 1000000u);
    lib_test_assert(sink.count == (reset ? 0u : 1u));
    x86_xt_keyboard_destroy(keyboard);
}

static void inhibited_bat(lib_bool hold_clock)
{
    const x86_xt_keyboard_timing timing = {12500u, 300000u, 60u, 25u};
    receiver sink = {0};
    x86_xt_keyboard *keyboard = LIB_NULL;
    lib_u64 deadline = 0u;
    lib_test_assert(x86_xt_keyboard_create(&timing, receive, &sink, &keyboard) == LIB_STATUS_OK);
    x86_xt_keyboard_advance(keyboard, timing.reset_ticks);
    x86_xt_keyboard_set_lines(keyboard, LIB_FALSE, LIB_FALSE);
    x86_xt_keyboard_set_lines(keyboard, hold_clock, !hold_clock);
    x86_xt_keyboard_advance(keyboard, timing.bat_ticks);
    lib_test_assert(sink.count == 0u);
    lib_test_assert(x86_xt_keyboard_ticks_until_event(keyboard, &deadline) == LIB_STATUS_UNSUPPORTED);
    x86_xt_keyboard_set_lines(keyboard, LIB_FALSE, LIB_FALSE);
    lib_test_assert(x86_xt_keyboard_ticks_until_event(keyboard, &deadline) == LIB_STATUS_OK && deadline == 60u);
    x86_xt_keyboard_advance(keyboard, 260u);
    lib_test_assert(sink.count == 1u && sink.bytes[0] == 0xaau);
    x86_xt_keyboard_destroy(keyboard);
}

int main(void)
{
    const x86_xt_keyboard_timing timing = {12500u, 300000u, 60u, 25u};
    const x86_xt_keyboard_timing disabled = {0u, 0u, 0u, 0u};
    const x86_xt_keyboard_timing invalid = {1u, 0u, 1u, 1u};
    receiver sink = {0};
    x86_xt_keyboard *keyboard = LIB_NULL;
    lib_u8 bytes[17];
    lib_u64 deadline = 0u;
    lib_test_assert(x86_xt_keyboard_create(&invalid, receive, &sink, &keyboard) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(keyboard == LIB_NULL);
    lib_test_assert(x86_xt_keyboard_create(&timing, LIB_NULL, &sink, &keyboard) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(x86_xt_keyboard_create(&timing, receive, &sink, &keyboard) == LIB_STATUS_OK);
    for (lib_size index = 0u; index < sizeof(bytes); ++index) bytes[index] = (lib_u8)(0x10u + index);
    x86_xt_keyboard_set_lines(keyboard, LIB_FALSE, LIB_FALSE);
    lib_test_assert(x86_xt_keyboard_receive_native_bytes(keyboard, bytes, 1u) == LIB_STATUS_OK);
    x86_xt_keyboard_advance(keyboard, 10u);
    x86_xt_keyboard_set_lines(keyboard, LIB_TRUE, LIB_FALSE);
    x86_xt_keyboard_advance(keyboard, 100u);
    lib_test_assert(sink.count == 0u);
    lib_test_assert(x86_xt_keyboard_ticks_until_event(keyboard, &deadline) == LIB_STATUS_UNSUPPORTED);
    x86_xt_keyboard_set_lines(keyboard, LIB_FALSE, LIB_FALSE);
    lib_test_assert(x86_xt_keyboard_ticks_until_event(keyboard, &deadline) == LIB_STATUS_OK && deadline == 50u);
    x86_xt_keyboard_advance(keyboard, 250u);
    lib_test_assert(sink.count == 1u && sink.bytes[0] == bytes[0]);
    x86_xt_keyboard_reset(keyboard);
    sink = (receiver){0};
    lib_test_assert(x86_xt_keyboard_receive_native_bytes(keyboard, bytes, 16u) == LIB_STATUS_OK);
    lib_test_assert(x86_xt_keyboard_receive_native_bytes(keyboard, bytes + 16u, 1u) == LIB_STATUS_OK);
    x86_xt_keyboard_set_lines(keyboard, LIB_FALSE, LIB_FALSE);
    for (lib_size index = 0u; index < 16u; ++index) {
        x86_xt_keyboard_advance(keyboard, 260u);
        lib_test_assert(sink.count == index + 1u);
        lib_test_assert(sink.bytes[index] == (index == 15u ? 0xffu : bytes[index]));
        x86_xt_keyboard_receiver_ready(keyboard);
    }
    lib_test_assert(x86_xt_keyboard_ticks_until_event(keyboard, &deadline) == LIB_STATUS_UNSUPPORTED);
    x86_xt_keyboard_reset(keyboard);
    sink = (receiver){0};
    lib_test_assert(x86_xt_keyboard_receive_native_bytes(keyboard, bytes, 17u) == LIB_STATUS_OK);
    x86_xt_keyboard_set_lines(keyboard, LIB_FALSE, LIB_FALSE);
    x86_xt_keyboard_advance(keyboard, 260u);
    lib_test_assert(sink.count == 1u && sink.bytes[0] == 0xffu);
    x86_xt_keyboard_destroy(keyboard);
    lib_test_assert(x86_xt_keyboard_create(&disabled, receive, &sink, &keyboard) == LIB_STATUS_OK);
    x86_xt_keyboard_advance(keyboard, LIB_UINT64_MAX);
    x86_xt_keyboard_set_lines(keyboard, LIB_FALSE, LIB_FALSE);
    lib_test_assert(x86_xt_keyboard_receive_native_bytes(keyboard, bytes, 1u) == LIB_STATUS_OK);
    lib_test_assert(x86_xt_keyboard_ticks_until_event(keyboard, &deadline) == LIB_STATUS_UNSUPPORTED);
    x86_xt_keyboard_destroy(keyboard);
    refused_completion(LIB_FALSE, LIB_FALSE);
    refused_completion(LIB_TRUE, LIB_FALSE);
    refused_completion(LIB_FALSE, LIB_TRUE);
    refused_completion(LIB_TRUE, LIB_TRUE);
    inhibited_bat(LIB_TRUE);
    inhibited_bat(LIB_FALSE);
    return 0;
}
