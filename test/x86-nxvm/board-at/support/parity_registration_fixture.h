#ifndef TEST_X86_NXVM_BOARD_AT_PARITY_REGISTRATION_FIXTURE_H
#define TEST_X86_NXVM_BOARD_AT_PARITY_REGISTRATION_FIXTURE_H
#include "x86-nxvm/board-base/machine_board_interface.h"

lib_status test_at_parity_try(x86_nxvm_board_state *board,
    const x86_nxvm_board_planar_parity_config *config, lib_bool reject_ports);
#endif
