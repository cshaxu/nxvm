#include "lib/types/types_interface.h"
#include "app-nxvm/devices/machine_board_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/machine.h"
#include "support/core_machine_board_fixture.h"

typedef struct input_display_trace_probe {
    core_machine_trace_event events[40];
    lib_u32 count;
} input_display_trace_probe;

static void input_display_trace(void *opaque,
    const core_machine_trace_event *event)
{
    input_display_trace_probe *probe = (input_display_trace_probe *)opaque;

    if (probe != LIB_NULL && probe->count <
        sizeof(probe->events) / sizeof(probe->events[0])) {
        probe->events[probe->count++] = *event;
    }
}

static const core_machine_trace_event *input_display_find_event(
    const input_display_trace_probe *probe, core_machine_trace_event_type type)
{
    lib_u32 index;

    for (index = 0u; index < probe->count; ++index) {
        if (probe->events[index].type == type) return &probe->events[index];
    }
    return LIB_NULL;
}

static lib_i32 input_display_lifecycle_guards(core_machine_keyboard_topology topology)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .keyboard_topology = topology,
        .xt_ppi_keyboard = {0x60u, 0x61u, 0x62u, 0x63u, 1u, 0x0du, 0x02u}
    };
    const core_machine_lifecycle states[] = {CORE_MACHINE_INITIALIZED,
        CORE_MACHINE_PAUSED, CORE_MACHINE_RUNNING, CORE_MACHINE_STOPPED,
        CORE_MACHINE_FAULTED};
    const lib_u8 native = 0x1eu;
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    x86_video_snapshot *snapshot = lib_allocate(sizeof(*snapshot));
    x86_video_snapshot_observation observation;
    lib_u8 scan_set = 0u;
    lib_i32 failed = 1;

    if (snapshot == LIB_NULL ||
        core_machine_create(&config, &machine, &board) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK) goto done;
    for (lib_size index = 0u; index < sizeof(states) / sizeof(states[0]); ++index) {
        const lib_bool input = states[index] != CORE_MACHINE_INITIALIZED &&
            states[index] != CORE_MACHINE_FAULTED;
        const lib_bool display = states[index] == CORE_MACHINE_STOPPED ||
            states[index] == CORE_MACHINE_PAUSED;
        lib_status capture;

        /* Same-owner setup isolates each guard without inventing a public
         * transition operation or another production lifecycle. */
        machine->lifecycle = states[index];
        if (core_machine_keyboard_receive_native_byte(board, native) !=
                (input ? LIB_STATUS_OK : LIB_STATUS_INVALID_STATE) ||
            core_machine_keyboard_receive_native_bytes(board, &native, 1u) !=
                (input ? LIB_STATUS_OK : LIB_STATUS_INVALID_STATE) ||
            core_machine_keyboard_get_native_scan_set(board, &scan_set) !=
                (states[index] == CORE_MACHINE_INITIALIZED ?
                    LIB_STATUS_INVALID_STATE : LIB_STATUS_OK) ||
            core_machine_mouse_receive_relative(board, 1, -1, 0u) !=
                (input && topology == CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI ?
                    LIB_STATUS_UNSUPPORTED : LIB_STATUS_INVALID_STATE) ||
            core_machine_observe_display_snapshot(machine, LIB_FALSE, 0u,
                &observation) != (display ? LIB_STATUS_OK : LIB_STATUS_INVALID_STATE)) goto done;
        capture = core_machine_capture_display_snapshot(machine, snapshot);
        if (display ? (capture != LIB_STATUS_OK && capture != LIB_STATUS_UNSUPPORTED) :
                capture != LIB_STATUS_INVALID_STATE) goto done;
        if (core_machine_keyboard_get_native_scan_set(board, LIB_NULL) !=
                LIB_STATUS_INVALID_STATE ||
            core_machine_capture_display_snapshot(machine, LIB_NULL) !=
                LIB_STATUS_INVALID_ARGUMENT ||
            core_machine_observe_display_snapshot(machine, LIB_FALSE, 0u,
                LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT) goto done;
    }
    if (core_machine_keyboard_receive_native_byte(LIB_NULL, native) != LIB_STATUS_INVALID_STATE ||
        core_machine_keyboard_receive_native_bytes(LIB_NULL, &native, 1u) != LIB_STATUS_INVALID_STATE ||
        core_machine_keyboard_get_native_scan_set(LIB_NULL, &scan_set) != LIB_STATUS_INVALID_STATE ||
        core_machine_mouse_receive_relative(LIB_NULL, 1, -1, 0u) != LIB_STATUS_INVALID_STATE ||
        core_machine_capture_display_snapshot(LIB_NULL, snapshot) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_observe_display_snapshot(LIB_NULL, LIB_FALSE, 0u, &observation) !=
            LIB_STATUS_INVALID_ARGUMENT) goto done;
    machine->lifecycle = CORE_MACHINE_PAUSED;
    machine->firmware_operation_active = LIB_TRUE;
    if (core_machine_mutable_operation_is_allowed(machine) ||
        core_machine_mutable_operation_is_allowed(LIB_NULL) ||
        core_machine_keyboard_receive_native_byte(board, native) != LIB_STATUS_INVALID_STATE ||
        core_machine_keyboard_receive_native_bytes(board, &native, 1u) != LIB_STATUS_INVALID_STATE ||
        core_machine_mouse_receive_relative(board, 1, -1, 0u) != LIB_STATUS_INVALID_STATE ||
        core_machine_set_xt_ppi_fault_input(board, CORE_MACHINE_XT_PPI_FAULT_RAM_PARITY,
            LIB_TRUE) != LIB_STATUS_INVALID_STATE ||
        core_machine_keyboard_get_native_scan_set(board, &scan_set) != LIB_STATUS_OK ||
        core_machine_set_xt_ppi_fault_input(LIB_NULL, CORE_MACHINE_XT_PPI_FAULT_RAM_PARITY,
            LIB_TRUE) != LIB_STATUS_INVALID_STATE) goto done;
    machine->firmware_operation_active = LIB_FALSE;
    if (!core_machine_mutable_operation_is_allowed(machine) ||
        core_machine_set_xt_ppi_fault_input(board, CORE_MACHINE_XT_PPI_FAULT_RAM_PARITY,
            LIB_FALSE) != (topology == CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI ?
                LIB_STATUS_OK : LIB_STATUS_INVALID_STATE)) goto done;
    failed = 0;
done:
    core_machine_destroy(machine);
    lib_release(snapshot);
    return failed;
}

lib_i32 main(void)
{
    core_machine *machine = LIB_NULL;
    core_machine_config config = { 0 };
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine_timeline_observation observation;
    input_display_trace_probe probe = { { { 0 } }, 0u };
    core_machine_trace_provider trace = { input_display_trace, &probe };
    const lib_u8 nop = 0x90u;
    lib_i32 failed = 0;

    failed |= input_display_lifecycle_guards(CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI);
    failed |= input_display_lifecycle_guards(CORE_MACHINE_KEYBOARD_TOPOLOGY_8042);
    config.ticks_per_instruction = 1u;
    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80286;
    failed |= core_machine_create(&config, &machine, LIB_NULL) != LIB_STATUS_OK;
    failed |= !failed && test_core_machine_fixture_register_reset_mapping(machine,
        0x00fffff0u, 0x000ffff0u, 16u) != LIB_STATUS_OK;
    failed |= !failed && core_machine_freeze_execution_providers(machine) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= !failed && core_machine_memory_write(machine, 0x00fffff0u, &nop,
        sizeof(nop)) != LIB_STATUS_OK;
    failed |= !failed && core_machine_set_trace_provider(machine, &trace) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_run(machine, budget, &result) != LIB_STATUS_OK;
    failed |= !failed && (result.reason != CORE_MACHINE_STOP_BUDGET ||
        result.elapsed_ticks != 3u);
    failed |= !failed && core_machine_get_timeline_observation(machine,
        &observation) != LIB_STATUS_OK;
    failed |= !failed && (observation.now != 3u || observation.pending_events != 0u ||
        observation.next_sequence != 0u);
    {
        const core_machine_trace_event *retire = input_display_find_event(&probe,
            CORE_MACHINE_TRACE_CPU_RETIRE);
        const core_machine_trace_event *fdc = input_display_find_event(&probe,
            CORE_MACHINE_TRACE_FDC_ADVANCE);
        const core_machine_trace_event *hdc = input_display_find_event(&probe,
            CORE_MACHINE_TRACE_HDC_ADVANCE);
        const core_machine_trace_event *kbc = input_display_find_event(&probe,
            CORE_MACHINE_TRACE_KBC_ADVANCE);
        const core_machine_trace_event *vadp = input_display_find_event(&probe,
            CORE_MACHINE_TRACE_VADP_ADVANCE);
        const core_machine_trace_event *boundary = input_display_find_event(&probe,
            CORE_MACHINE_TRACE_RUN_BOUNDARY);

        failed |= !failed && (retire == LIB_NULL || fdc != LIB_NULL ||
            hdc != LIB_NULL || kbc == LIB_NULL || vadp == LIB_NULL ||
            boundary == LIB_NULL || kbc->timeline_ticks != 3u ||
            vadp->timeline_ticks != 3u || retire->sequence >= kbc->sequence ||
            kbc->sequence >= vadp->sequence || vadp->sequence >= boundary->sequence);
    }
    failed |= !failed && core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= !failed && core_machine_get_timeline_observation(machine,
        &observation) != LIB_STATUS_OK;
    failed |= !failed && (observation.now != 0u || observation.pending_events != 0u ||
        observation.next_sequence != 0u);

    core_machine_destroy(machine);
    if (failed) return 1;
    printf("M5:T346:S5:INPUT-DISPLAY-TIMELINE:OK\n");
    printf("M5:T540:S83:BOARD-INPUT-HANDLE:OK\n");
    return 0;
}
