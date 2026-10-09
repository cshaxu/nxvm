#ifndef TEST_IBMPC_EMULATOR_BOARD_BINDING_FIXTURE_H
#define TEST_IBMPC_EMULATOR_BOARD_BINDING_FIXTURE_H
#include "core/x86/attachment_interface.h"
#include "core/board-base/machine_board_interface.h"

/* Expected construction values, not a copy read from Core's stored binding.
 * The context is borrowed for identity comparison only; Core owns its lifetime. */
lib_status test_board_binding_create(const core_machine_config *config,
    core_machine **out_machine, core_machine_attachment *out_expected);
#endif
