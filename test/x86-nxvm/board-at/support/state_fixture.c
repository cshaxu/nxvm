#include "state_fixture.h"
#include "x86-nxvm/board-at/kbc.h"

static lib_status admit_byte(x86_keyboard *keyboard, lib_u8 byte)
{
    lib_u8 output;
    lib_size count;
    return x86_keyboard_admit(keyboard, &byte, 1u, &output, 1u, &count);
}

static lib_status keyboard_repeat(void *context, const lib_u8 *bytes, lib_size count)
{
    if (count != 1u) return LIB_STATUS_INVALID_ARGUMENT;
    *(lib_u8 *)context = bytes[0];
    return LIB_STATUS_OK;
}

lib_i32 test_keyboard_repeat_cadence(t_kbc *kbc, lib_u64 initial_ticks,
    lib_u64 repeat_ticks)
{
    x86_keyboard *keyboard = kbc->connect.keyboard;
    lib_u64 ticks = 0u;
    lib_u8 repeated = 0u;
    lib_i32 failed = 0;
    failed = failed || admit_byte(keyboard, 0x1cu) != LIB_STATUS_OK;
    failed = failed || x86_keyboard_ticks_until_repeat(keyboard, &ticks) != LIB_STATUS_OK ||
        ticks != initial_ticks;
    failed = failed || x86_keyboard_advance(keyboard, initial_ticks - 1u, keyboard_repeat, &repeated) ||
        repeated != 0u;
    failed = failed || !x86_keyboard_advance(keyboard, 1u, keyboard_repeat, &repeated) ||
        repeated != 0x1cu;
    failed = failed || x86_keyboard_ticks_until_repeat(keyboard, &ticks) != LIB_STATUS_OK ||
        ticks != repeat_ticks;
    failed = failed || admit_byte(keyboard, 0xf0u) != LIB_STATUS_OK;
    failed = failed || admit_byte(keyboard, 0x1cu) != LIB_STATUS_OK;
    return failed;
}

lib_bool test_kbc_aux_enabled(const t_kbc *kbc)
{
    return x86_kbc8042_aux_enabled(kbc->chip);
}

lib_bool test_keyboard_scanning(const t_kbc *kbc)
{
    return x86_keyboard_get_signals(kbc->connect.keyboard).scanning;
}
