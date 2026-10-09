#ifndef TEST_BOARD_CONTROLLER_FIXTURE_H
#define TEST_BOARD_CONTROLLER_FIXTURE_H
#include "core/board-base/machine_board_interface.h"
#include "core/chips/fdc8272/fdc8272_interface.h"
#include "core/chips/hdc/hdc_interface.h"
#include "../../../../../core/board-base/fdc_values.h"

lib_bool test_board_fdc_binding_matches(const core_machine_board_state *board,
    const core_machine *machine, core_machine_media_id media_id, lib_u8 dma_channel);
core_machine_fdc_config test_board_fdc_connection_config(
    const core_machine_board_state *board);
core_machine_fdc_drive_bindings test_board_fdc_drive_bindings(
    const core_machine_board_state *board);
lib_bool test_board_fdc_advance_ticks(core_machine_board_state *board, lib_u64 ticks);
void test_board_fdc_reset(core_machine_board_state *board);
void test_board_dma_transfers(core_machine_board_state *board, core_machine *machine,
    lib_u64 transfers, lib_u8 controllers);
void test_board_fdc_advance(core_machine_board_state *board);
void test_board_fdc_refresh(core_machine_board_state *board);
lib_bool test_board_fdc_interrupt_matches(const core_machine_board_state *board,
    lib_bool asserted);
lib_status test_board_fdc_capture(const core_machine_board_state *board,
    x86_fdc_observation *observation);
lib_u8 test_board_controller_scan_interrupt(core_machine_board_state *board);
lib_bool test_board_hdc_binding_is_present(const core_machine_board_state *board);
void test_board_hdc_service(core_machine_board_state *board);
x86_hdc_observation test_board_hdc_observe(const core_machine_board_state *board);
lib_u8 test_board_hdc_irq_pending(const core_machine_board_state *board);
core_machine_media_id test_board_hdc_slave_media_id(const core_machine_board_state *board);
core_machine_hdc_config test_board_hdc_connection_config(const core_machine_board_state *board);
lib_bool test_board_fdc_advance_due(core_machine_board_state *board);
lib_bool test_board_fdc_irq_source_is_asserted(const core_machine_board_state *board);
#endif
