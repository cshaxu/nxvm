#ifndef TEST_X86_CORE_PLAN_CORE_FIXTURE_H
#define TEST_X86_CORE_PLAN_CORE_FIXTURE_H
#include "x86/ibmpc-common/machine_board_interface.h"
#include "x86/ibmpc-common/dma_bus_interface.h"

/* Core-owned assertions; no Board layout is read by this source. */
lib_i32 test_plan_core_attachment(core_machine *machine,
    core_machine_board_state *board);
lib_i32 test_plan_core_allocation_failures(const core_machine_config *configuration);
lib_i32 test_plan_core_xt_routes(core_machine *machine, lib_bool after_reset);
lib_i32 test_plan_core_pit_deadline(core_machine *machine);
lib_i32 test_plan_core_dma_deadline(core_machine *machine,
    core_machine_dma_bus *dma, const core_machine_dma_request_binding *binding);
#endif
