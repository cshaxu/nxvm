#ifndef TEST_BOARD_CMOS_FIXTURE_H
#define TEST_BOARD_CMOS_FIXTURE_H
#include "core/board-base/machine_board_interface.h"

void test_board_cmos_advance(core_machine_board_state *board, lib_u64 ticks);
lib_u8 test_board_cmos_scan_interrupt(core_machine_board_state *board);
lib_u8 test_board_cmos_get_interrupt(core_machine_board_state *board);
void test_board_cmos_reset(core_machine_board_state *board);
lib_u8 test_board_cmos_read_register(const core_machine_board_state *board, lib_u8 index);
void test_board_cmos_report_failure(core_machine_board_state *board, lib_i32 failed);
#endif
