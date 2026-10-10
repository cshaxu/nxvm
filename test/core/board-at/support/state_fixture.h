#ifndef TEST_AT_KBC_STATE_FIXTURE_H
#define TEST_AT_KBC_STATE_FIXTURE_H
#include "core/board-at/kbc_interface.h"

lib_bool test_kbc_aux_enabled(const t_kbc *kbc);
lib_bool test_keyboard_scanning(const t_kbc *kbc);
lib_i32 test_keyboard_repeat_cadence(t_kbc *kbc, lib_u64 initial_ticks,
    lib_u64 repeat_ticks);
#endif
