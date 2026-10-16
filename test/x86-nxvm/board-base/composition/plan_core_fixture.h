#ifndef TEST_X86_CORE_PLAN_CORE_FIXTURE_H
#define TEST_X86_CORE_PLAN_CORE_FIXTURE_H
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-nxvm/board-base/dma_bus_interface.h"

/* Board wiring uses only public Machine construction, routes and observation. */
lib_i32 test_plan_board_device_failures(const x86_nxvm_board_config *configuration);
lib_i32 test_plan_core_xt_routes(x86_core_machine *machine, lib_bool after_reset);
lib_i32 test_plan_core_pit_deadline(x86_core_machine *machine,
    x86_nxvm_board_state *board);
lib_i32 test_plan_core_dma_deadline(x86_core_machine *machine,
    x86_nxvm_board_dma_bus *dma, const x86_nxvm_board_dma_request_binding *binding);
#endif
