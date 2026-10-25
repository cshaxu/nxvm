#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-core/machine/trace_interface.h"
#include "x86-core/machine/debug_interface.h"

typedef struct input_display_trace_probe {
    x86_core_machine_trace_event events[40];
    lib_u32 count;
} input_display_trace_probe;

static void input_display_trace(void *opaque,
    const x86_core_machine_trace_event *event)
{
    input_display_trace_probe *probe = (input_display_trace_probe *)opaque;

    if (probe != LIB_NULL && probe->count <
        sizeof(probe->events) / sizeof(probe->events[0])) {
        probe->events[probe->count++] = *event;
    }
}

static const x86_core_machine_trace_event *input_display_find_event(
    const input_display_trace_probe *probe, lib_u32 type)
{
    lib_u32 index;

    for (index = 0u; index < probe->count; ++index) {
        if (probe->events[index].type == type) return &probe->events[index];
    }
    return LIB_NULL;
}

typedef struct input_display_guard_probe {
    x86_core_machine *machine;
    x86_nxvm_board_state *board;
    x86_video_snapshot *snapshot;
    x86_nxvm_board_keyboard_topology topology;
    lib_u32 running_calls;
    lib_u32 firmware_calls;
    lib_bool check_firmware;
} input_display_guard_probe;

static lib_i32 input_display_guards_at(input_display_guard_probe *probe,
    x86_core_machine_lifecycle expected)
{
    const lib_bool input = expected != X86_CORE_MACHINE_INITIALIZED &&
        expected != X86_CORE_MACHINE_FAULTED;
    const lib_bool display = expected == X86_CORE_MACHINE_STOPPED ||
        expected == X86_CORE_MACHINE_PAUSED;
    const lib_u8 native = 0x1eu;
    x86_nxvm_board_state *board = probe->board;
    x86_core_machine_lifecycle actual;
    x86_video_snapshot_observation observation;
    lib_u8 scan_set = 0u;
    lib_status capture;

    if (x86_core_machine_get_lifecycle(probe->machine, &actual) != LIB_STATUS_OK ||
        actual != expected ||
        x86_nxvm_board_keyboard_receive_native_byte(board, native) !=
            (input ? LIB_STATUS_OK : LIB_STATUS_INVALID_STATE) ||
        x86_nxvm_board_keyboard_receive_native_bytes(board, &native, 1u) !=
            (input ? LIB_STATUS_OK : LIB_STATUS_INVALID_STATE) ||
        x86_nxvm_board_keyboard_get_native_scan_set(board, &scan_set) !=
            (expected == X86_CORE_MACHINE_INITIALIZED ?
                LIB_STATUS_INVALID_STATE : LIB_STATUS_OK) ||
        x86_nxvm_board_mouse_receive_relative(board, 1, -1, 0u) !=
            (input && probe->topology == X86_NXVM_BOARD_KEYBOARD_TOPOLOGY_XT_PPI ?
                LIB_STATUS_UNSUPPORTED : LIB_STATUS_INVALID_STATE) ||
        x86_nxvm_board_observe_display_snapshot(board, LIB_FALSE, 0u,
            &observation) != (display ? LIB_STATUS_OK : LIB_STATUS_INVALID_STATE)) return 1;
    capture = x86_nxvm_board_capture_display_snapshot(board, probe->snapshot);
    if (display ? (capture != LIB_STATUS_OK && capture != LIB_STATUS_UNSUPPORTED) :
            capture != LIB_STATUS_INVALID_STATE) return 1;
    return x86_nxvm_board_keyboard_get_native_scan_set(board, LIB_NULL) !=
            LIB_STATUS_INVALID_STATE ||
        x86_nxvm_board_capture_display_snapshot(board, LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT ||
        x86_nxvm_board_observe_display_snapshot(board, LIB_FALSE, 0u, LIB_NULL) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        x86_nxvm_board_keyboard_receive_native_byte(LIB_NULL, native) != LIB_STATUS_INVALID_STATE ||
        x86_nxvm_board_keyboard_receive_native_bytes(LIB_NULL, &native, 1u) != LIB_STATUS_INVALID_STATE ||
        x86_nxvm_board_keyboard_get_native_scan_set(LIB_NULL, &scan_set) != LIB_STATUS_INVALID_STATE ||
        x86_nxvm_board_mouse_receive_relative(LIB_NULL, 1, -1, 0u) != LIB_STATUS_INVALID_STATE ||
        x86_nxvm_board_capture_display_snapshot(LIB_NULL, probe->snapshot) != LIB_STATUS_INVALID_ARGUMENT ||
        x86_nxvm_board_observe_display_snapshot(LIB_NULL, LIB_FALSE, 0u, &observation) !=
            LIB_STATUS_INVALID_ARGUMENT;
}

static lib_status input_display_guard_running(void *owner, lib_u16 port,
    lib_u8 access_bytes, lib_u32 value)
{
    input_display_guard_probe *probe = owner;

    (void)access_bytes;
    (void)port;
    (void)value;
    ++probe->running_calls;
    return input_display_guards_at(probe, X86_CORE_MACHINE_RUNNING) ?
        LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK;
}

static lib_status input_display_guard_configure(void *owner,
    x86_core_machine_firmware_context *firmware)
{
    (void)owner;
    (void)firmware;
    return LIB_STATUS_OK;
}

static lib_status input_display_guard_reset(void *owner,
    x86_core_machine_firmware_context *firmware)
{
    input_display_guard_probe *probe = owner;
    const lib_u8 native = 0x1eu;
    x86_core_machine_lifecycle lifecycle;
    x86_video_snapshot_observation observation;
    lib_u8 scan_set = 0u;

    (void)firmware;
    if (!probe->check_firmware) return LIB_STATUS_OK;
    ++probe->firmware_calls;
    if (x86_core_machine_get_lifecycle(probe->machine, &lifecycle) != LIB_STATUS_OK ||
        lifecycle != X86_CORE_MACHINE_PAUSED ||
        x86_nxvm_board_keyboard_receive_native_byte(probe->board, native) != LIB_STATUS_INVALID_STATE ||
        x86_nxvm_board_keyboard_receive_native_bytes(probe->board, &native, 1u) != LIB_STATUS_INVALID_STATE ||
        x86_nxvm_board_mouse_receive_relative(probe->board, 1, -1, 0u) != LIB_STATUS_INVALID_STATE ||
        x86_nxvm_board_set_xt_ppi_fault_input(probe->board, X86_NXVM_BOARD_XT_PPI_FAULT_RAM_PARITY,
            LIB_TRUE) != LIB_STATUS_INVALID_STATE ||
        x86_nxvm_board_keyboard_get_native_scan_set(probe->board, &scan_set) != LIB_STATUS_OK ||
        x86_nxvm_board_observe_display_snapshot(probe->board, LIB_FALSE, 0u,
            &observation) != LIB_STATUS_OK ||
        x86_nxvm_board_set_xt_ppi_fault_input(LIB_NULL, X86_NXVM_BOARD_XT_PPI_FAULT_RAM_PARITY,
            LIB_TRUE) != LIB_STATUS_INVALID_STATE) return LIB_STATUS_INTERNAL_ERROR;
    return LIB_STATUS_OK;
}

static lib_i32 input_display_lifecycle_guards(x86_nxvm_board_keyboard_topology topology)
{
    const x86_nxvm_board_config config = {
        .memory_bytes = (2u * 1024u * 1024u),
        .keyboard_topology = topology,
        .xt_ppi_keyboard = {0x60u, 0x61u, 0x62u, 0x63u, 1u, 0x0du, 0x02u}
    };
    const x86_core_machine_firmware_provider firmware = {
        input_display_guard_configure, input_display_guard_reset, LIB_NULL
    };
    const x86_core_machine_port_provider ports = {LIB_NULL, input_display_guard_running};
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EDX),
        .values = {[X86_CORE_CPU_DEBUG_EIP] = 0x200u,
            [X86_CORE_CPU_DEBUG_EDX] = 0x1234u}
    };
    const lib_u8 out_dx = 0xeeu, nop = 0x90u;
    input_display_guard_probe probe = {.topology = topology};
    x86_core_machine_run_result result;
    lib_i32 failed = 1;

    probe.snapshot = lib_allocate(sizeof(*probe.snapshot));
    if (probe.snapshot == LIB_NULL ||
        x86_nxvm_board_create(&config, &probe.machine, &probe.board) != LIB_STATUS_OK ||
        input_display_guards_at(&probe, X86_CORE_MACHINE_INITIALIZED) ||
        x86_core_machine_install_port_provider(probe.machine, 0x1234u, 0x1234u,
            &ports, &probe) != LIB_STATUS_OK ||
        x86_core_machine_bind_firmware_provider(probe.machine, &firmware, &probe) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(probe.machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(probe.machine) != LIB_STATUS_OK ||
        input_display_guards_at(&probe, X86_CORE_MACHINE_STOPPED) ||
        x86_core_machine_debug_patch_registers(probe.machine, &entry) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(probe.machine, 0x200u, &out_dx, 1u) != LIB_STATUS_OK ||
        x86_core_machine_run(probe.machine, (x86_core_machine_run_budget){1u, 0u},
            &result) != LIB_STATUS_OK || result.reason != X86_CORE_MACHINE_STOP_BUDGET ||
        result.executed != 1u || probe.running_calls != 1u ||
        input_display_guards_at(&probe, X86_CORE_MACHINE_PAUSED)) goto done;
    /* Reset from a genuine paused boundary invokes the real firmware guard. */
    probe.check_firmware = LIB_TRUE;
    if (x86_core_machine_reset(probe.machine) != LIB_STATUS_OK || probe.firmware_calls != 1u ||
        x86_core_machine_debug_patch_registers(probe.machine, &entry) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(probe.machine, 0x200u, &nop, 1u) != LIB_STATUS_OK ||
        x86_core_machine_run(probe.machine, (x86_core_machine_run_budget){1u, 0u},
            &result) != LIB_STATUS_OK || result.reason != X86_CORE_MACHINE_STOP_BUDGET ||
        x86_nxvm_board_set_xt_ppi_fault_input(probe.board, X86_NXVM_BOARD_XT_PPI_FAULT_RAM_PARITY,
            LIB_FALSE) != (topology == X86_NXVM_BOARD_KEYBOARD_TOPOLOGY_XT_PPI ?
                LIB_STATUS_OK : LIB_STATUS_INVALID_STATE) ||
        x86_core_machine_report_fault(probe.machine, 0x1234u) != LIB_STATUS_OK ||
        input_display_guards_at(&probe, X86_CORE_MACHINE_FAULTED)) goto done;
    failed = 0;
done:
    x86_core_machine_destroy(probe.machine);
    lib_release(probe.snapshot);
    return failed;
}

lib_i32 main(void)
{
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_config config = { 0 };
    x86_core_machine_run_budget budget = { 1u, 0u };
    x86_core_machine_run_result result;
    x86_core_machine_timeline_observation observation;
    input_display_trace_probe probe = { { { 0 } }, 0u };
    x86_core_machine_trace_provider trace = { input_display_trace, &probe };
    const lib_u8 nop = 0x90u;
    const x86_core_machine_memory_alias_config reset_alias = {
        0x00fffff0u, 0x000ffff0u, 16u
    };
    lib_i32 failed = 0;

    failed |= input_display_lifecycle_guards(X86_NXVM_BOARD_KEYBOARD_TOPOLOGY_XT_PPI);
    failed |= input_display_lifecycle_guards(X86_NXVM_BOARD_KEYBOARD_TOPOLOGY_8042);
    config.ticks_per_instruction = 1u;
    config.cpu_profile = X86_CORE_CPU_PROFILE_80286;
    failed |= x86_nxvm_board_create(&config, &machine, LIB_NULL) != LIB_STATUS_OK;
    failed |= !failed && x86_core_machine_install_memory_aliases(machine,
        &reset_alias, 1u, LIB_FALSE) != LIB_STATUS_OK;
    failed |= !failed && x86_core_machine_freeze_execution_providers(machine) !=
        LIB_STATUS_OK;
    failed |= !failed && x86_core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= !failed && x86_core_machine_memory_write(machine, 0x00fffff0u, &nop,
        sizeof(nop)) != LIB_STATUS_OK;
    failed |= !failed && x86_core_machine_set_trace_provider(machine, &trace) !=
        LIB_STATUS_OK;
    failed |= !failed && x86_core_machine_run(machine, budget, &result) != LIB_STATUS_OK;
    failed |= !failed && (result.reason != X86_CORE_MACHINE_STOP_BUDGET ||
        result.elapsed_ticks != 3u);
    failed |= !failed && x86_core_machine_get_timeline_observation(machine,
        &observation) != LIB_STATUS_OK;
    failed |= !failed && (observation.now != 3u || observation.pending_events != 0u ||
        observation.next_sequence != 0u);
    {
        const x86_core_machine_trace_event *retire = input_display_find_event(&probe,
            X86_CORE_MACHINE_TRACE_CPU_RETIRE);
        const x86_core_machine_trace_event *fdc = input_display_find_event(&probe,
            X86_NXVM_BOARD_TRACE_FDC_ADVANCE);
        const x86_core_machine_trace_event *hdc = input_display_find_event(&probe,
            X86_NXVM_BOARD_TRACE_HDC_ADVANCE);
        const x86_core_machine_trace_event *kbc = input_display_find_event(&probe,
            X86_NXVM_BOARD_TRACE_KBC_ADVANCE);
        const x86_core_machine_trace_event *vadp = input_display_find_event(&probe,
            X86_NXVM_BOARD_TRACE_VADP_ADVANCE);
        const x86_core_machine_trace_event *boundary = input_display_find_event(&probe,
            X86_CORE_MACHINE_TRACE_RUN_BOUNDARY);

        failed |= !failed && (retire == LIB_NULL || fdc != LIB_NULL ||
            hdc != LIB_NULL || kbc == LIB_NULL || vadp == LIB_NULL ||
            boundary == LIB_NULL || kbc->timeline_ticks != 3u ||
            vadp->timeline_ticks != 3u || retire->sequence >= kbc->sequence ||
            kbc->sequence >= vadp->sequence || vadp->sequence >= boundary->sequence);
    }
    failed |= !failed && x86_core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= !failed && x86_core_machine_get_timeline_observation(machine,
        &observation) != LIB_STATUS_OK;
    failed |= !failed && (observation.now != 0u || observation.pending_events != 0u ||
        observation.next_sequence != 0u);

    x86_core_machine_destroy(machine);
    if (failed) return 1;
    lib_c_printf("INPUT-DISPLAY-TIMELINE:OK\n");
    lib_c_printf("BOARD-INPUT-HANDLE:OK\n");
    return 0;
}
