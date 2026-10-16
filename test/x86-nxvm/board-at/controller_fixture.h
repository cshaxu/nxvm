#ifndef TEST_KBC_CONTROLLER_FIXTURE_H
#define TEST_KBC_CONTROLLER_FIXTURE_H
#include "x86-nxvm/board-at/kbc_interface.h"
#include "x86-core/machine/debug_interface.h"
#include "x86-nxvm/chips/keyboard/keyboard_interface.h"

/* Separately compiled assertions remain with Core, AT and Board owners. */
lib_bool test_kbc_bat_ready(const t_kbc *kbc);
lib_bool test_kbc_construction_rollback(void);
lib_bool test_kbc_has_repeat(const t_kbc *kbc);
lib_bool test_kbc_cadence_is(t_kbc *kbc, lib_u64 initial, lib_u64 interval);
lib_bool test_kbc_repeat_byte(t_kbc *kbc, lib_u8 *byte);
x86_keyboard_signals test_kbc_signals(const t_kbc *kbc);
lib_bool test_kbc_aux_enabled(const t_kbc *kbc);
void test_kbc_without_aux(t_kbc *kbc);
void test_kbc_set_test_inputs(t_kbc *kbc, lib_u8 inputs);
#endif
