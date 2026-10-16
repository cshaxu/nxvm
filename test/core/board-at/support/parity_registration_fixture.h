#ifndef TEST_CORE_BOARD_AT_PARITY_REGISTRATION_FIXTURE_H
#define TEST_CORE_BOARD_AT_PARITY_REGISTRATION_FIXTURE_H
#include "core/board-base/machine_board_interface.h"

lib_status test_at_parity_try(core_machine_board_state *board,
    const core_machine_planar_parity_config *config, lib_bool reject_ports);
#endif
