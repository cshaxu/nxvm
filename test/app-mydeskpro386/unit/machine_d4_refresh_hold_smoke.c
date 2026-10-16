#include "app-mydeskpro386/d4_platform_interface.h"
#include "app-mydeskpro386/d4_platform.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "d4_refresh_fixture.h"

#include "core/board-base/dma_bus_interface.h"
#include "../../core/board-base/composition/composition_fixture.h"
#include "../../core/board-base/support/composition_fixture.h"
#include "core/board-base/machine_board_interface.h"
#include "x86-core/machine/trace_interface.h"
#include "../../core/board-base/core_machine_board_fixture.h"

#define REFRESH_PROBE_EVENT_CAPACITY 1024u

typedef struct refresh_probe {
    x86_core_machine_trace_event events[REFRESH_PROBE_EVENT_CAPACITY];
    lib_u32 count;
} refresh_probe;

typedef struct refresh_dma_source {
    lib_u8 value;
} refresh_dma_source;

static void refresh_trace(void *opaque, const x86_core_machine_trace_event *event)
{
    refresh_probe *probe = (refresh_probe *)opaque;

    if (probe != LIB_NULL && event != LIB_NULL &&
        probe->count < REFRESH_PROBE_EVENT_CAPACITY) {
        probe->events[probe->count++] = *event;
    }
}

static void refresh_dma_read(void *opaque, t_latch *latch)
{
    refresh_dma_source *source = (refresh_dma_source *)opaque;

    if (source != LIB_NULL && latch != LIB_NULL) {
        latch->data.byte = source->value;
    }
}

static void refresh_program_dma_channel2(x86_core_machine *machine)
{
    test_core_machine_fixture_write_port(machine, 0x000cu, 0u);
    test_core_machine_fixture_write_port(machine, 0x0004u, 0x34u);
    test_core_machine_fixture_write_port(machine, 0x0004u, 0x12u);
    test_core_machine_fixture_write_port(machine, 0x0005u, 0u);
    test_core_machine_fixture_write_port(machine, 0x0005u, 0u);
    test_core_machine_fixture_write_port(machine, 0x0081u, 1u);
    test_core_machine_fixture_write_port(machine, 0x000bu, 0x46u);
    test_core_machine_fixture_write_port(machine, 0x000au, 0x02u);
    test_core_machine_fixture_write_port(machine, 0x00d4u, 0x00u);
}

static lib_i32 refresh_find_transaction_after(const refresh_probe *probe,
    x86_core_machine_trace_event_type phase, x86_core_machine_transaction_owner owner,
    x86_core_machine_transaction_kind kind, lib_u32 start,
    lib_u32 *out_index)
{
    lib_u32 index;

    if (probe == LIB_NULL || out_index == LIB_NULL) return 0;
    for (index = start; index < probe->count; ++index) {
        const x86_core_machine_trace_event *event = &probe->events[index];

        if (event->type == phase && (event->detail & 0xffu) == owner &&
            ((event->detail >> 8u) & 0xffu) == kind) {
            *out_index = index;
            return 1;
        }
    }
    return 0;
}

static lib_i32 refresh_find_hold_after(const refresh_probe *probe,
    x86_core_machine_trace_event_type phase, x86_core_machine_transaction_owner owner,
    lib_u32 start, lib_u32 *out_index)
{
    lib_u32 index;

    if (probe == LIB_NULL || out_index == LIB_NULL) return 0;
    for (index = start; index < probe->count; ++index) {
        const x86_core_machine_trace_event *event = &probe->events[index];

        if (event->type == phase && (event->detail & 0xffu) == owner) {
            *out_index = index;
            return 1;
        }
    }
    return 0;
}

static lib_i32 refresh_has_cpu_transaction_between(const refresh_probe *probe,
    lib_u32 first, lib_u32 last)
{
    lib_u32 index;

    if (probe == LIB_NULL || first >= last) return 0;
    for (index = first + 1u; index < last; ++index) {
        const x86_core_machine_trace_event *event = &probe->events[index];

        if ((event->type == X86_CORE_MACHINE_TRACE_TRANSACTION_BEGIN ||
             event->type == X86_CORE_MACHINE_TRACE_TRANSACTION_COMMIT) &&
            (event->detail & 0xffu) == X86_CORE_MACHINE_TRANSACTION_OWNER_CPU) {
            return 1;
        }
    }
    return 0;
}

static lib_i32 refresh_non_d4_contract(void)
{
    const lib_u8 program[] = {0x90u, 0x90u, 0x90u, 0x90u, 0x90u, 0x90u, 0x90u};
    x86_core_machine_run_result result = {0};
    core_machine_config config = {0};
    x86_core_machine_trace_provider trace;
    refresh_probe probe = {{{0}}, 0u};
    x86_core_machine *machine = LIB_NULL;
    lib_u32 index = 0u;
    lib_i32 failed = 0;

    config.cpu_profile = X86_CORE_CPU_PROFILE_80386;
    trace.callback = refresh_trace;
    trace.context = &probe;
    failed = failed || core_machine_create(&config, &machine, LIB_NULL) != LIB_STATUS_OK;
    failed = failed || test_core_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
        0x000ffff0u, 16u) != LIB_STATUS_OK;
    failed = failed || x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed = failed || x86_core_machine_reset(machine) != LIB_STATUS_OK;
    failed = failed || x86_core_machine_set_trace_provider(machine, &trace) != LIB_STATUS_OK;
    failed = failed || x86_core_machine_memory_write(machine, 0xfffffff0u,
        program, sizeof(program)) != LIB_STATUS_OK ||
        x86_core_machine_run(machine, (x86_core_machine_run_budget){7u, 0u}, &result) != LIB_STATUS_OK ||
        result.reason != X86_CORE_MACHINE_STOP_BUDGET || result.executed != 7u ||
        result.elapsed_ticks < 20u;
    failed = failed || refresh_find_transaction_after(&probe,
        X86_CORE_MACHINE_TRACE_TRANSACTION_BEGIN,
        X86_CORE_MACHINE_TRANSACTION_OWNER_REFRESH,
        X86_CORE_MACHINE_TRANSACTION_REFRESH_MEMORY_CYCLE, 0u, &index);
    x86_core_machine_destroy(machine);
    return !failed;
}
lib_i32 main(void)
{
    static const core_machine_dma_channel_provider dma_provider = {
        refresh_dma_read, LIB_NULL, LIB_NULL
    };
    core_machine_config config = {0};
    mydeskpro386_d4_platform_config d4 = {CORE_MACHINE_PC_AT_PORT_B, 0u, 2u, 18u};
    x86_core_machine_trace_provider trace;
    core_machine_dma_request_binding binding = {0};
    refresh_probe probe = {{{0}}, 0u};
    refresh_dma_source source = {0xa5u};
    x86_core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    mydeskpro386_d4_platform *d4_board = LIB_NULL;
    lib_u8 byte = 0u;
    lib_u8 refresh_address = 0xffu;
    lib_u8 special_read = 0u;
    lib_u32 start;
    lib_u32 refresh_request = 0u;
    lib_u32 refresh_acknowledge = 0u;
    lib_u32 refresh_begin = 0u;
    lib_u32 refresh_commit = 0u;
    lib_u32 refresh_release = 0u;
    lib_u32 dma_begin = 0u;
    x86_core_machine_time_observation observation;
    lib_i32 failed = 0;

    config.cpu_profile = X86_CORE_CPU_PROFILE_80386;
    config.transaction_contract.dma_cycle_wait_quanta = 1u;
    config.transaction_contract.dma_cycle_bus_ready_gate_enabled = LIB_TRUE;
    config.l1_compatibility_policy = X86_CORE_MACHINE_L1_COMPATIBILITY_BOUNDED_PROGRESS;
    config.auxiliary_pit_present = LIB_TRUE;
    config.auxiliary_pit_base_port = 0x0048u;
    trace.callback = refresh_trace;
    trace.context = &probe;
    failed = failed || core_machine_create(&config, &machine, &board) != LIB_STATUS_OK;
    failed = failed || mydeskpro386_d4_platform_attach(board, &d4, &d4_board) != LIB_STATUS_OK;
    failed = failed || test_core_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
        0x000ffff0u, 16u) != LIB_STATUS_OK;
    failed = failed || test_board_dma_bind_channel(board, 2u,
        &dma_provider, &source, &binding) != LIB_STATUS_OK;
    failed = failed || x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed = failed || x86_core_machine_reset(machine) != LIB_STATUS_OK;
    failed = failed || x86_core_machine_set_trace_provider(machine, &trace) != LIB_STATUS_OK;
    if (!failed) refresh_program_dma_channel2(machine);
    if (!failed) test_board_dma_request_assert(board, &binding);
    failed = failed || x86_core_machine_set_dma_bus_ready(machine, 0) != LIB_STATUS_OK;
    if (!failed) test_core_machine_fixture_write_port(machine, 0x0064u, 0xd1u);
    if (!failed) test_core_machine_fixture_write_port(machine, 0x0060u, 0x01u);
    failed = failed || test_core_a20_is_enabled(machine);
    failed = failed || x86_core_machine_reset(machine) != LIB_STATUS_OK;
    if (!failed) refresh_program_dma_channel2(machine);
    if (!failed) test_board_dma_request_assert(board, &binding);
    failed = failed || x86_core_machine_set_dma_bus_ready(machine, 0) != LIB_STATUS_OK;
    failed = failed || mydeskpro386_d4_platform_kbc_command(d4_board, 0xa3u,
        0xb4u, 0x43u, &special_read) !=
        X86_KBC8042_COMMAND_EXTENSION_HANDLED;
    start = probe.count;
    /* The real refresh PIT output supplies the App HOLD request. Its
     * deadline-to-clock route has a separate public scheduler regression. */
    if (!failed) {
        x86_pit_advance(d4_board->shared_pit, 18u);
        x86_pit_advance(d4_board->auxiliary_pit, 18u);
    }
    failed = failed || !test_model40_refresh_hold_matches(d4_board, LIB_TRUE, 0u) ||
        !mydeskpro386_d4_platform_cpu_hold_asserted(d4_board);
    failed = failed || !mydeskpro386_d4_platform_refresh_request(d4_board,
        &refresh_address) || refresh_address != 0u;
    failed = failed || x86_core_machine_capture_time_observation(machine, &observation) !=
        LIB_STATUS_OK || observation.next_deadline_valid ||
        observation.progress_disposition != X86_CORE_MACHINE_TIME_PROGRESS_L1_COMPATIBILITY;
    failed = failed || x86_core_machine_set_dma_bus_ready(machine, 1) != LIB_STATUS_OK;
    /* With no qualified deadline, use the existing bounded idle policy.
     * The actual DMA trace below must still meet the original 11-tick bound. */
    if (!failed) {
        lib_u8 advanced = LIB_FALSE;

        failed = x86_core_machine_advance_l1_compatibility(machine, &advanced) != LIB_STATUS_OK ||
            !advanced;
    }
    failed = failed || !test_model40_refresh_hold_matches(d4_board, LIB_FALSE, 1u);
    refresh_address = 0xffu;
    failed = failed || mydeskpro386_d4_platform_refresh_request(d4_board,
        &refresh_address) || refresh_address != 0xffu;
    failed = failed || x86_core_machine_memory_read(machine, 0x11234u, &byte, 1u) !=
        LIB_STATUS_OK || byte != 0xa5u;
    failed = failed || !refresh_find_hold_after(&probe,
        X86_CORE_MACHINE_TRACE_TRANSACTION_HOLD_REQUEST,
        X86_CORE_MACHINE_TRANSACTION_OWNER_REFRESH, start, &refresh_request);
    failed = failed || !refresh_find_hold_after(&probe,
        X86_CORE_MACHINE_TRACE_TRANSACTION_HOLD_ACKNOWLEDGE,
        X86_CORE_MACHINE_TRANSACTION_OWNER_REFRESH, start, &refresh_acknowledge);
    failed = failed || !refresh_find_transaction_after(&probe,
        X86_CORE_MACHINE_TRACE_TRANSACTION_BEGIN,
        X86_CORE_MACHINE_TRANSACTION_OWNER_REFRESH,
        X86_CORE_MACHINE_TRANSACTION_REFRESH_MEMORY_CYCLE, start, &refresh_begin);
    failed = failed || !refresh_find_transaction_after(&probe,
        X86_CORE_MACHINE_TRACE_TRANSACTION_COMMIT,
        X86_CORE_MACHINE_TRANSACTION_OWNER_REFRESH,
        X86_CORE_MACHINE_TRANSACTION_REFRESH_MEMORY_CYCLE, start, &refresh_commit);
    failed = failed || !refresh_find_hold_after(&probe,
        X86_CORE_MACHINE_TRACE_TRANSACTION_HOLD_RELEASE,
        X86_CORE_MACHINE_TRANSACTION_OWNER_REFRESH, start, &refresh_release);
    failed = failed || !refresh_find_transaction_after(&probe,
        X86_CORE_MACHINE_TRACE_TRANSACTION_BEGIN,
        X86_CORE_MACHINE_TRANSACTION_OWNER_DMA,
        X86_CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, start, &dma_begin);
    failed = failed || refresh_request >= refresh_acknowledge ||
        refresh_acknowledge >= refresh_begin || refresh_begin >= refresh_commit ||
        refresh_commit >= refresh_release || refresh_release >= dma_begin ||
        refresh_has_cpu_transaction_between(&probe, refresh_release, dma_begin);
    if (!failed) {
        lib_u32 dma_commit = 0u;

        failed = !refresh_find_transaction_after(&probe,
            X86_CORE_MACHINE_TRACE_TRANSACTION_COMMIT, X86_CORE_MACHINE_TRANSACTION_OWNER_DMA,
            X86_CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, start, &dma_commit) ||
            probe.events[dma_commit].timeline_ticks > 11u;
    }
    failed = failed || x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        !test_model40_refresh_reset_is_clear(d4_board) ||
        mydeskpro386_d4_platform_cpu_hold_asserted(d4_board);

    x86_core_machine_destroy(machine);
    failed = failed || !refresh_non_d4_contract();
    if (failed) return 1;
    lib_c_printf("D4-REFRESH-HOLD:OK\n");
    return 0;
}
