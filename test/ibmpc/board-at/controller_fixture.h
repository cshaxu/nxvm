#ifndef TEST_KBC_CONTROLLER_FIXTURE_H
#define TEST_KBC_CONTROLLER_FIXTURE_H
#include "ibmpc/board-at/kbc_interface.h"
#include "x86/core/debug_interface.h"

/* Separately compiled assertions remain with Core, AT and Board owners. */
lib_status test_kbc_initialize(t_kbc *kbc, core_machine *machine);
void test_kbc_finalize(t_kbc *kbc);
lib_bool test_kbc_unpublished(const t_kbc *kbc);
lib_bool test_kbc_bat_ready(const t_kbc *kbc);
void test_kbc_initialize_pic(core_machine *machine);
lib_bool test_kbc_core_construction(t_kbc *kbc);
lib_status test_kbc_core_reset_entry(core_machine *machine,
    const core_machine_debug_register_patch *entry);
lib_i32 test_kbc_board_cpu_reset_irq1(void);
#endif
