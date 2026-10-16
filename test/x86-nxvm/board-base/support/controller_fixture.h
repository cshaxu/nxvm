#ifndef TEST_BOARD_CONTROLLER_FIXTURE_H
#define TEST_BOARD_CONTROLLER_FIXTURE_H
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-nxvm/chips/fdc8272/fdc8272_interface.h"
#include "x86-nxvm/chips/hdc/hdc_interface.h"
#include "../fdc_values.h"

lib_bool test_board_fdc_binding_matches(const x86_nxvm_board_state *board,
    const x86_core_machine *machine, x86_nxvm_board_media_id media_id, lib_u8 dma_channel);
x86_nxvm_board_fdc_config test_board_fdc_connection_config(
    const x86_nxvm_board_state *board);
x86_nxvm_board_fdc_drive_bindings test_board_fdc_drive_bindings(
    const x86_nxvm_board_state *board);
lib_bool test_board_fdc_advance_ticks(x86_nxvm_board_state *board, lib_u64 ticks);
void test_board_fdc_reset(x86_nxvm_board_state *board);
void test_board_dma_transfers(x86_nxvm_board_state *board, x86_core_machine *machine,
    lib_u64 transfers, lib_u8 controllers);
void test_board_fdc_advance(x86_nxvm_board_state *board);
void test_board_fdc_refresh(x86_nxvm_board_state *board);
lib_bool test_board_fdc_interrupt_matches(const x86_nxvm_board_state *board,
    lib_bool asserted);
lib_status test_board_fdc_capture(const x86_nxvm_board_state *board,
    x86_fdc_observation *observation);
lib_u8 test_board_controller_scan_interrupt(x86_nxvm_board_state *board);
lib_bool test_board_hdc_binding_is_present(const x86_nxvm_board_state *board);
void test_board_hdc_service(x86_nxvm_board_state *board);
x86_hdc_observation test_board_hdc_observe(const x86_nxvm_board_state *board);
lib_u8 test_board_hdc_irq_pending(const x86_nxvm_board_state *board);
x86_nxvm_board_media_id test_board_hdc_slave_media_id(const x86_nxvm_board_state *board);
x86_nxvm_board_hdc_config test_board_hdc_connection_config(const x86_nxvm_board_state *board);
lib_bool test_board_fdc_advance_due(x86_nxvm_board_state *board);
lib_bool test_board_fdc_finish_command_wait(x86_nxvm_board_state *board);
lib_bool test_board_fdc_irq_source_is_asserted(const x86_nxvm_board_state *board);
#endif
