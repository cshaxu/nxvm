#ifndef TEST_XT_PPI_CONTROLLER_FIXTURE_H
#define TEST_XT_PPI_CONTROLLER_FIXTURE_H
#include "x86-nxvm/board-xt/xt_ppi_keyboard_interface.h"

lib_status test_xt_ppi_initialize(x86_nxvm_board_xt_ppi_keyboard *keyboard,
    const x86_nxvm_board_xt_ppi_keyboard_config *config, x86_core_machine *machine);
void test_xt_ppi_finalize(x86_nxvm_board_xt_ppi_keyboard *keyboard);
lib_bool test_xt_ppi_unpublished(const x86_nxvm_board_xt_ppi_keyboard *keyboard);
lib_bool test_xt_ppi_expect_byte_ready(const x86_nxvm_board_xt_ppi_keyboard *keyboard,
    lib_bool expected);
lib_bool test_xt_ppi_expect_irq1(const x86_nxvm_board_xt_ppi_keyboard *keyboard,
    lib_bool expected);
lib_bool test_xt_ppi_expect_nmi(const x86_nxvm_board_xt_ppi_keyboard *keyboard,
    lib_bool expected);
lib_bool xt_construction_rollback(void);
#endif
