#ifndef TEST_AT_BOOT_FIXTURE_H
#define TEST_AT_BOOT_FIXTURE_H
#include "x86/ibmpc-at/kbc_interface.h"

lib_bool test_at_boot_bat_ready(const t_kbc *kbc);
lib_status test_at_boot_keyboard_repeat(const t_kbc *kbc, lib_u64 *ticks);
#endif
