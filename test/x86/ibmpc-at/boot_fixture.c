#include "boot_fixture.h"
#include "x86/ibmpc-at/kbc.h"

lib_bool test_at_boot_bat_ready(const t_kbc *kbc)
{
    return x86_keyboard_get_signals(kbc->connect.keyboard).bat_ready;
}

lib_status test_at_boot_keyboard_repeat(const t_kbc *kbc, lib_u64 *ticks)
{
    return x86_keyboard_ticks_until_repeat(kbc->connect.keyboard, ticks);
}
