#ifndef TEST_XT_PPI_CONTROLLER_FIXTURE_H
#define TEST_XT_PPI_CONTROLLER_FIXTURE_H
#include "x86/ibmpc-xt/xt_ppi_keyboard_interface.h"

lib_status test_xt_ppi_initialize(core_machine_xt_ppi_keyboard *keyboard,
    const core_machine_xt_ppi_keyboard_config *config, core_machine *machine);
void test_xt_ppi_finalize(core_machine_xt_ppi_keyboard *keyboard);
lib_bool test_xt_ppi_unpublished(const core_machine_xt_ppi_keyboard *keyboard);
lib_bool test_xt_ppi_expect_byte_ready(const core_machine_xt_ppi_keyboard *keyboard,
    lib_bool expected);
lib_bool test_xt_ppi_expect_irq1(const core_machine_xt_ppi_keyboard *keyboard,
    lib_bool expected);
lib_bool test_xt_ppi_expect_nmi(const core_machine_xt_ppi_keyboard *keyboard,
    lib_bool expected);
lib_bool test_xt_ppi_core_construction(core_machine_xt_ppi_keyboard *keyboard,
    const core_machine_xt_ppi_keyboard_config *config, lib_size fail_at);
lib_bool xt_construction_rollback(void);
#endif
