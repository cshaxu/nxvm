#ifndef TEST_BOARD_ABSENT_MEMORY_FIXTURE_H
#define TEST_BOARD_ABSENT_MEMORY_FIXTURE_H
#include "x86-nxvm/board-base/machine_board_interface.h"

lib_bool test_board_absent_memory_is_configured(x86_nxvm_board_state *board);
lib_status test_board_absent_memory_registration_failure(x86_nxvm_board_state *board,
    const x86_nxvm_board_absent_memory_config *config, lib_size *out_attempts);
#endif
