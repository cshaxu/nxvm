#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "../board-xt/controller_fixture.h"
#include "../core/time_fixture.h"
#include "ibmpc/board-common/machine_board_interface.h"
#include "core_machine_board_fixture.h"
#include "ibmpc/board-common/machine_board_state.h"
#include "x86/core/debug_interface.h"

static lib_i32 core_machine_xt_ppi_keyboard_path(void)
{
    const core_machine_config configuration = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .time_axis = {CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL, 1000000u},
        .keyboard_topology = CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI,
        .xt_ppi_keyboard = {0x0060u, 0x0061u, 0x0062u, 0x0063u, 1u,
            0x0du, 0x02u}
    };
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    lib_u32 value = 0u;
    lib_u8 scan_set = 0u;
    core_machine_speaker_observation speaker;
    lib_i32 failed = 0;

    failed |= core_machine_create(&configuration, &machine, &board) != LIB_STATUS_OK;
    failed |= !failed && core_machine_freeze_execution_providers(machine) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= !failed && core_machine_bus_read(machine, 0x0064u, &value) !=
        LIB_STATUS_UNSUPPORTED;
    failed |= !failed && core_machine_keyboard_get_native_scan_set(board,
        &scan_set) != LIB_STATUS_OK;
    failed |= !failed && scan_set != CORE_MACHINE_KEYBOARD_SCAN_SET_1;
    failed |= !failed && core_machine_bus_write(machine, 0x0063u, 0x99u) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_bus_read(machine, 0x0062u, &value) !=
        LIB_STATUS_OK;
    failed |= !failed && value != 0x0du;
    failed |= !failed && core_machine_bus_write(machine, 0x0061u, 0x08u) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_bus_read(machine, 0x0062u, &value) !=
        LIB_STATUS_OK;
    failed |= !failed && value != 0x02u;
    failed |= !failed && (core_machine_get_speaker_observation(board, &speaker) !=
        LIB_STATUS_OK || !speaker.configured || speaker.timer_gate ||
        speaker.data_enabled || speaker.output);
    failed |= !failed && (core_machine_bus_write(machine, 0x0061u, 0x02u) !=
        LIB_STATUS_OK || core_machine_get_speaker_observation(board, &speaker) !=
        LIB_STATUS_OK || speaker.timer_gate || !speaker.data_enabled ||
        !speaker.output);
    failed |= !failed && (core_machine_bus_write(machine, 0x0061u, 0x03u) !=
        LIB_STATUS_OK || core_machine_get_speaker_observation(board, &speaker) !=
        LIB_STATUS_OK || !speaker.timer_gate || !speaker.data_enabled);
    failed |= !failed && (core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_get_speaker_observation(board, &speaker) != LIB_STATUS_OK ||
        !speaker.configured || speaker.timer_gate || speaker.data_enabled ||
        speaker.output);
    failed |= !failed && core_machine_bus_read(machine, 0x0063u, &value) !=
        LIB_STATUS_UNSUPPORTED;
    failed |= !failed && core_machine_bus_write(machine, 0x0063u, 0x98u) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_bus_write(machine, 0x0063u, 0x01u) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_bus_read(machine, 0x0062u, &value) !=
        LIB_STATUS_OK;
    failed |= !failed && value != 0x01u;
    failed |= !failed && core_machine_bus_write(machine, 0x0063u, 0x99u) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_bus_write(machine, 0x0061u, 0x40u) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_xt_ppi_keyboard_receive_device_byte(
        board->xt_ppi_keyboard, 0x1eu) !=
        LIB_STATUS_OK;
    failed |= !failed && test_xt_ppi_expect_byte_ready(board->xt_ppi_keyboard, LIB_TRUE);
    failed |= !failed && test_xt_ppi_expect_irq1(board->xt_ppi_keyboard, LIB_TRUE);
    failed |= !failed && core_machine_bus_read(machine, 0x0060u, &value) !=
        LIB_STATUS_OK;
    failed |= !failed && value != 0x1eu;
    /* IBM's IRQ handler clears the PPI shift register through PB7; reading
     * PA alone must not create a second consumption rule. */
    failed |= !failed && core_machine_bus_read(machine, 0x0060u, &value) !=
        LIB_STATUS_OK;
    failed |= !failed && value != 0x1eu;
    failed |= !failed && core_machine_bus_write(machine, 0x0061u, 0xc0u) !=
        LIB_STATUS_OK;
    failed |= !failed && test_xt_ppi_expect_byte_ready(board->xt_ppi_keyboard, LIB_FALSE);
    failed |= !failed && test_xt_ppi_expect_irq1(board->xt_ppi_keyboard, LIB_FALSE);
    failed |= !failed && core_machine_bus_write(machine, 0x0061u, 0x40u) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_xt_ppi_keyboard_receive_device_byte(
        board->xt_ppi_keyboard, 0x9eu) !=
        LIB_STATUS_OK;
    failed |= !failed && test_xt_ppi_expect_byte_ready(board->xt_ppi_keyboard, LIB_TRUE);
    failed |= !failed && core_machine_bus_read(machine, 0x0060u, &value) !=
        LIB_STATUS_OK;
    failed |= !failed && value != 0x9eu;
    failed |= !failed && core_machine_bus_write(machine, 0x0061u, 0xc0u) !=
        LIB_STATUS_OK;
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 core_machine_xt_ppi_does_not_change_at_8042(void)
{
    const core_machine_config configuration = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES
    };
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    lib_u32 value = 0u;
    lib_u8 scan_set = 0u;
    lib_i32 failed = 0;

    failed |= core_machine_create(&configuration, &machine, &board) != LIB_STATUS_OK;
    failed |= !failed && core_machine_freeze_execution_providers(machine) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= !failed && core_machine_bus_read(machine, 0x0064u, &value) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_keyboard_get_native_scan_set(board,
        &scan_set) != LIB_STATUS_OK;
    failed |= !failed && scan_set != CORE_MACHINE_KEYBOARD_SCAN_SET_2;
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 core_machine_xt_ppi_parity_nmi_path(void)
{
    const core_machine_config configuration = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .keyboard_topology = CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI,
        .xt_ppi_keyboard = {0x0060u, 0x0061u, 0x0062u, 0x0063u, 1u}
    };
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    lib_u32 value = 0u;
    lib_i32 failed = 0;

    failed |= core_machine_create(&configuration, &machine, &board) != LIB_STATUS_OK;
    failed |= !failed && core_machine_freeze_execution_providers(machine) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= !failed && core_machine_bus_write(machine, 0x0063u, 0x99u) !=
        LIB_STATUS_OK;
    /* PB4/PB5 are active-low parity/I/O-check enables. A live PC7 status line
     * remains observable while PB4 suppresses its NMI request. */
    failed |= !failed && core_machine_bus_write(machine, 0x0061u, 0x30u) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_set_xt_ppi_fault_input(board,
        CORE_MACHINE_XT_PPI_FAULT_RAM_PARITY, LIB_TRUE) != LIB_STATUS_OK;
    failed |= !failed && test_core_machine_fixture_nmi_prepare(machine);
    failed |= !failed && core_machine_bus_read(machine, 0x0062u, &value) !=
        LIB_STATUS_OK;
    failed |= !failed && value != 0x80u;
    failed |= !failed && test_core_machine_fixture_nmi_execute(machine, LIB_FALSE);
    failed |= !failed && core_machine_bus_write(machine, 0x0061u, 0x20u) !=
        LIB_STATUS_OK;
    failed |= !failed && test_xt_ppi_expect_nmi(board->xt_ppi_keyboard, LIB_TRUE);
    failed |= !failed && test_core_machine_fixture_nmi_execute(machine, LIB_TRUE);
    failed |= !failed && core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= !failed && core_machine_bus_write(machine, 0x0063u, 0x99u) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_bus_write(machine, 0x0061u, 0x30u) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_set_nmi_mask(machine, LIB_TRUE) != LIB_STATUS_OK;
    failed |= !failed && core_machine_set_xt_ppi_fault_input(board,
        CORE_MACHINE_XT_PPI_FAULT_IO_CHECK, LIB_TRUE) != LIB_STATUS_OK;
    failed |= !failed && test_core_machine_fixture_nmi_prepare(machine);
    failed |= !failed && core_machine_bus_read(machine, 0x0062u, &value) !=
        LIB_STATUS_OK;
    failed |= !failed && value != 0x40u;
    failed |= !failed && core_machine_bus_write(machine, 0x0061u, 0x10u) !=
        LIB_STATUS_OK;
    failed |= !failed && test_core_machine_fixture_nmi_execute(machine, LIB_FALSE);
    failed |= !failed && core_machine_set_nmi_mask(machine, LIB_FALSE) != LIB_STATUS_OK;
    failed |= !failed && test_core_machine_fixture_nmi_execute(machine, LIB_TRUE);
    failed |= !failed && core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= !failed && core_machine_bus_write(machine, 0x0063u, 0x99u) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_bus_read(machine, 0x0062u, &value) !=
        LIB_STATUS_OK;
    failed |= !failed && value != 0u;
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 core_machine_xt_keyboard_reset_bat_path(void)
{
    const core_machine_config configuration = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .time_axis = {CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL, 1000000u},
        .keyboard_topology = CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI,
        .xt_ppi_keyboard = {0x0060u, 0x0061u, 0x0062u, 0x0063u, 1u}
    };
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    core_machine_time_observation time_observation;
    lib_u32 value = 0u;
    const lib_u8 full_fifo[16] = {
        0x10u, 0x11u, 0x12u, 0x13u, 0x14u, 0x15u, 0x16u, 0x17u,
        0x18u, 0x19u, 0x1au, 0x1bu, 0x1cu, 0x1du, 0x1eu, 0x1fu
    };
    lib_i32 failed = 0;

    failed |= core_machine_create(&configuration, &machine, &board) != LIB_STATUS_OK;
    failed |= !failed && core_machine_freeze_execution_providers(machine) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= !failed && core_machine_bus_write(machine, 0x0063u, 0x99u) !=
        LIB_STATUS_OK;
    failed |= !failed && test_core_machine_advance_time(machine, 12499u) != LIB_STATUS_OK;
    failed |= !failed && core_machine_bus_write(machine, 0x0061u, 0x40u) !=
        LIB_STATUS_OK;
    failed |= !failed && test_core_machine_advance_time(machine, 300300u) != LIB_STATUS_OK;
    failed |= !failed && test_xt_ppi_expect_byte_ready(board->xt_ppi_keyboard, LIB_FALSE);
    failed |= !failed && core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= !failed && core_machine_bus_write(machine, 0x0063u, 0x99u) !=
        LIB_STATUS_OK;
    failed |= !failed && test_core_machine_advance_time(machine, 12500u) != LIB_STATUS_OK;
    failed |= !failed && core_machine_bus_write(machine, 0x0061u, 0x40u) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_capture_time_observation(machine,
        &time_observation) != LIB_STATUS_OK;
    failed |= !failed && !time_observation.next_deadline_valid;
    failed |= !failed && test_core_machine_advance_time(machine, 300000u) != LIB_STATUS_OK;
    failed |= !failed && test_xt_ppi_expect_byte_ready(board->xt_ppi_keyboard, LIB_FALSE);
    failed |= !failed && core_machine_capture_time_observation(machine,
        &time_observation) != LIB_STATUS_OK;
    failed |= !failed && (!time_observation.next_deadline_valid ||
        time_observation.next_deadline_tick != time_observation.elapsed_ticks + 60u);
    failed |= !failed && test_core_machine_advance_time(machine, 300u) != LIB_STATUS_OK;
    failed |= !failed && test_xt_ppi_expect_byte_ready(board->xt_ppi_keyboard, LIB_TRUE);
    failed |= !failed && test_xt_ppi_expect_irq1(board->xt_ppi_keyboard, LIB_TRUE);
    failed |= !failed && core_machine_bus_read(machine, 0x0060u, &value) !=
        LIB_STATUS_OK;
    failed |= !failed && value != 0xaau;
    failed |= !failed && core_machine_bus_write(machine, 0x0061u, 0xc0u) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_bus_write(machine, 0x0061u, 0x40u) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_keyboard_receive_native_byte(board, 0x1eu) !=
        LIB_STATUS_OK;
    failed |= !failed && test_core_machine_advance_time(machine, 300u) != LIB_STATUS_OK;
    failed |= !failed && core_machine_bus_read(machine, 0x0060u, &value) !=
        LIB_STATUS_OK;
    failed |= !failed && value != 0x1eu;
    failed |= !failed && core_machine_bus_write(machine, 0x0061u, 0x00u) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_keyboard_receive_native_bytes(board, full_fifo,
        sizeof(full_fifo)) != LIB_STATUS_OK;
    failed |= !failed && core_machine_keyboard_receive_native_byte(board, 0x20u) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_bus_write(machine, 0x0061u, 0xc0u) != LIB_STATUS_OK;
    failed |= !failed && core_machine_bus_write(machine, 0x0061u, 0x40u) != LIB_STATUS_OK;
    for (lib_size index = 0u; index < 16u && !failed; ++index) {
        failed |= test_core_machine_advance_time(machine, 260u) != LIB_STATUS_OK;
        failed |= core_machine_bus_read(machine, 0x0060u, &value) != LIB_STATUS_OK;
        failed |= value != (index == 15u ? 0xffu : full_fifo[index]);
        failed |= core_machine_bus_write(machine, 0x0061u, 0xc0u) != LIB_STATUS_OK;
        failed |= core_machine_bus_write(machine, 0x0061u, 0x40u) != LIB_STATUS_OK;
    }
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 core_machine_xt_keyboard_refused_completion(lib_bool bat,
    lib_bool reset_pending, lib_bool mode_release)
{
    const core_machine_config configuration = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .time_axis = {CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL, 1000000u},
        .keyboard_topology = CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI,
        .xt_ppi_keyboard = {0x0060u, 0x0061u, 0x0062u, 0x0063u, 1u}
    };
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    lib_u32 value = 0u;
    lib_u64 deadline = 0u;
    lib_i32 failed = 0;

    failed |= core_machine_create(&configuration, &machine, &board) != LIB_STATUS_OK;
    failed |= !failed && core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= !failed && core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= !failed && core_machine_bus_write(machine, 0x0063u, 0x99u) != LIB_STATUS_OK;
    if (bat) {
        failed |= !failed && test_core_machine_advance_time(machine, 12500u) != LIB_STATUS_OK;
    }
    failed |= !failed && core_machine_bus_write(machine, 0x0061u, 0x40u) != LIB_STATUS_OK;
    /* Occupy the board latch independently of the frame under test. */
    failed |= !failed && core_machine_xt_ppi_keyboard_receive_device_byte(
        board->xt_ppi_keyboard, 0x1eu) != LIB_STATUS_OK;
    if (bat) {
        failed |= !failed && test_core_machine_advance_time(machine, 300000u) != LIB_STATUS_OK;
    } else {
        failed |= !failed && core_machine_keyboard_receive_native_byte(board, 0x9eu) != LIB_STATUS_OK;
    }
    failed |= !failed && test_core_machine_advance_time(machine, 260u) != LIB_STATUS_OK;
    failed |= !failed && (core_machine_bus_read(machine, 0x0060u, &value) != LIB_STATUS_OK || value != 0x1eu);
    failed |= !failed && test_core_machine_advance_time(machine, 25u) != LIB_STATUS_OK;
    /* A refused completed frame cannot acquire another 255 serial bits. */
    failed |= !failed && x86_xt_keyboard_ticks_until_event(
        board->xt_keyboard, &deadline) != LIB_STATUS_UNSUPPORTED;
    if (reset_pending) {
        failed |= !failed && core_machine_reset(machine) != LIB_STATUS_OK;
        failed |= !failed && core_machine_bus_write(machine, 0x0063u, 0x99u) != LIB_STATUS_OK;
    }
    /* PB7 acknowledgement while PB6 holds the clock cannot deliver a byte. */
    failed |= !failed && core_machine_bus_write(machine, 0x0061u, 0x80u) != LIB_STATUS_OK;
    failed |= !failed && core_machine_bus_write(machine, 0x0061u, 0x00u) != LIB_STATUS_OK;
    failed |= !failed && test_core_machine_advance_time(machine, 1u) != LIB_STATUS_OK;
    failed |= !failed && (core_machine_bus_read(machine, 0x0060u, &value) != LIB_STATUS_OK || value != 0u);
    if (mode_release) {
        failed |= !failed && core_machine_bus_write(machine, 0x0063u, 0x9bu) != LIB_STATUS_OK;
    }
    failed |= !failed && core_machine_bus_write(machine, 0x0061u, 0x40u) != LIB_STATUS_OK;
    if (mode_release) {
        failed |= !failed && (core_machine_bus_read(machine, 0x0060u, &value) != LIB_STATUS_OK || value != 0u);
        failed |= !failed && core_machine_bus_write(machine, 0x0063u, 0x99u) != LIB_STATUS_OK;
    }
    failed |= !failed && (core_machine_bus_read(machine, 0x0060u, &value) != LIB_STATUS_OK ||
        value != (reset_pending ? 0u : bat ? 0xaau : 0x9eu));
    /* Once accepted, neither clock advance nor another clear duplicates it. */
    failed |= !failed && core_machine_bus_write(machine, 0x0061u, 0xc0u) != LIB_STATUS_OK;
    failed |= !failed && core_machine_bus_write(machine, 0x0061u, 0x40u) != LIB_STATUS_OK;
    failed |= !failed && test_core_machine_advance_time(machine, 10000u) != LIB_STATUS_OK;
    failed |= !failed && (core_machine_bus_read(machine, 0x0060u, &value) != LIB_STATUS_OK || value != 0u);
    if (failed) lib_c_printf("XT refused completion failed: BAT=%u reset=%u\n", bat, reset_pending);
    core_machine_destroy(machine);
    return failed;
}

typedef struct xt_teardown_observation {
    lib_bool pit_stopped;
    lib_bool speaker_after_pit;
    lib_u32 speaker_calls;
} xt_teardown_observation;

static void xt_teardown_pit_output(void *owner, lib_u8 asserted)
{
    xt_teardown_observation *observation = owner;
    if (!asserted) observation->pit_stopped = LIB_TRUE;
}

static void xt_teardown_speaker(void *owner, lib_u8 gate, lib_u8 data)
{
    xt_teardown_observation *observation = owner;
    (void)gate;
    (void)data;
    ++observation->speaker_calls;
    if (observation->pit_stopped) observation->speaker_after_pit = LIB_TRUE;
}

static lib_i32 core_machine_xt_teardown_order(void)
{
    const core_machine_config configuration = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .keyboard_topology = CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI,
        .xt_ppi_keyboard = {0x60u, 0x61u, 0x62u, 0x63u, 1u, 0u, 0u}
    };
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    xt_teardown_observation observation = {0};
    lib_u32 before;
    lib_i32 failed = core_machine_create(&configuration, &machine, &board) != LIB_STATUS_OK;

    if (failed) return failed;
    /* Mode 2 starts high: PIT destruction must deliver the final low edge. */
    failed |= x86_pit_write_register(board->shared_pit, 3u, 0xb4u) != LIB_STATUS_OK;
    failed |= x86_pit_write_register(board->shared_pit, 2u, 2u) != LIB_STATUS_OK;
    failed |= x86_pit_write_register(board->shared_pit, 2u, 0u) != LIB_STATUS_OK;
    failed |= !x86_pit_get_output(board->shared_pit, 2u);
    x86_pit_set_output(board->shared_pit, 2u, xt_teardown_pit_output, &observation);
    core_machine_xt_ppi_keyboard_bind_speaker(board->xt_ppi_keyboard,
        xt_teardown_speaker, &observation);
    before = observation.speaker_calls;
    core_machine_destroy(machine);
    failed |= observation.speaker_calls != before + 1u ||
        !observation.pit_stopped || observation.speaker_after_pit;
    return failed;
}

int main(void)
{
    if (core_machine_xt_teardown_order() || xt_construction_rollback() || core_machine_xt_ppi_keyboard_path() ||
        core_machine_xt_ppi_does_not_change_at_8042() ||
        core_machine_xt_ppi_parity_nmi_path() ||
        core_machine_xt_keyboard_reset_bat_path() ||
        core_machine_xt_keyboard_refused_completion(LIB_FALSE, LIB_FALSE, LIB_FALSE) ||
        core_machine_xt_keyboard_refused_completion(LIB_TRUE, LIB_FALSE, LIB_FALSE) ||
        core_machine_xt_keyboard_refused_completion(LIB_FALSE, LIB_TRUE, LIB_FALSE) ||
        core_machine_xt_keyboard_refused_completion(LIB_TRUE, LIB_TRUE, LIB_FALSE) ||
        core_machine_xt_keyboard_refused_completion(LIB_FALSE, LIB_FALSE, LIB_TRUE) ||
        core_machine_xt_keyboard_refused_completion(LIB_TRUE, LIB_FALSE, LIB_TRUE)) return 1;
    lib_c_printf("XT-PPI-KEYBOARD:OK\n");
    lib_c_printf("XT-IRQ1-RESET:OK\n");
    lib_c_printf("NO-8042-ALIAS:OK\n");
    lib_c_printf("XT-PPI-PARITY:OK\n");
    lib_c_printf("XT-KEYBOARD-BAT:OK\n");
    return 0;
}
