#include "app-mydeskpro386/profiles/d4_platform_interface.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "ibmpc/board-common/dma_bus_interface.h"
#include "../../../x86/core/composition_fixture.h"
#include "../../../x86/core/time_fixture.h"
#include "../composition_fixture.h"
#include "ibmpc/board-common/machine_board_interface.h"
#include "x86/core/trace_interface.h"
#include "../core_machine_board_fixture.h"

typedef struct competition_probe {
    core_machine_trace_event events[256];
    lib_u32 count;
} competition_probe;

typedef struct competition_dma_source {
    lib_u8 value;
} competition_dma_source;

static void competition_trace(void *opaque,
    const core_machine_trace_event *event)
{
    competition_probe *probe = (competition_probe *)opaque;

    if (probe != LIB_NULL && probe->count < 256u) {
        probe->events[probe->count++] = *event;
    }
}

static void competition_dma_read(void *opaque, t_latch *latch)
{
    competition_dma_source *source = (competition_dma_source *)opaque;

    if (source != LIB_NULL && latch != LIB_NULL) latch->data.byte = source->value;
}

static lib_i32 competition_find_event(const competition_probe *probe,
    core_machine_trace_event_type type, lib_u32 *out_index)
{
    lib_u32 index;

    if (probe == LIB_NULL || out_index == LIB_NULL) return 0;
    for (index = 0u; index < probe->count; ++index) {
        if (probe->events[index].type == type) {
            *out_index = index;
            return 1;
        }
    }
    return 0;
}

static lib_i32 competition_find_event_after(const competition_probe *probe,
    core_machine_trace_event_type type, lib_u32 start,
    lib_u32 *out_index)
{
    lib_u32 index;

    if (probe == LIB_NULL || out_index == LIB_NULL) return 0;
    for (index = start; index < probe->count; ++index) {
        if (probe->events[index].type == type) {
            *out_index = index;
            return 1;
        }
    }
    return 0;
}

static lib_i32 competition_find_transaction(const competition_probe *probe,
    core_machine_trace_event_type phase, core_machine_transaction_owner owner,
    core_machine_transaction_kind kind, lib_u32 *out_index)
{
    lib_u32 index;

    if (probe == LIB_NULL || out_index == LIB_NULL) return 0;
    for (index = 0u; index < probe->count; ++index) {
        const core_machine_trace_event *event = &probe->events[index];

        if (event->type == phase && (event->detail & 0xffu) == owner &&
            ((event->detail >> 8u) & 0xffu) == kind) {
            *out_index = index;
            return 1;
        }
    }
    return 0;
}

static void competition_program_dma_channel2(core_machine *machine)
{
    test_core_machine_fixture_write_port(machine, 0x000cu, 0u);
    test_core_machine_fixture_write_port(machine, 0x0004u, 0x34u);
    test_core_machine_fixture_write_port(machine, 0x0004u, 0x12u);
    test_core_machine_fixture_write_port(machine, 0x0005u, 0u);
    test_core_machine_fixture_write_port(machine, 0x0005u, 0u);
    test_core_machine_fixture_write_port(machine, 0x0081u, 1u);
    test_core_machine_fixture_write_port(machine, 0x000bu, 0x46u);
    test_core_machine_fixture_write_port(machine, 0x000au, 0x02u);
}

static lib_i32 competition_dma_wait_contract(void)
{
    static const core_machine_dma_channel_provider provider = {
        competition_dma_read, LIB_NULL, LIB_NULL };
    core_machine_config config = {0};
    core_machine_dma_request_binding binding = {0};
    competition_dma_source source = {0xa5u};
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    lib_u8 value = 0u;
    lib_i32 failed = 0;

    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80386;
    config.transaction_contract.dma_cycle_wait_quanta = 1u;
    config.transaction_contract.dma_cycle_bus_ready_gate_enabled = LIB_TRUE;
    failed = failed || core_machine_create(&config, &machine, &board) != LIB_STATUS_OK;
    failed = failed || test_core_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
        0x000ffff0u, 16u) != LIB_STATUS_OK;
    failed = failed || test_board_dma_bind_channel(board, 2u,
        &provider, &source, &binding) != LIB_STATUS_OK;
    failed = failed || core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed = failed || core_machine_reset(machine) != LIB_STATUS_OK;
    failed = failed || core_machine_memory_write(machine, 0x11234u, &value, 1u) != LIB_STATUS_OK;
    if (!failed) competition_program_dma_channel2(machine);
    if (!failed) test_board_dma_request_assert(board, &binding);
    failed = failed || core_machine_set_dma_bus_ready(machine, 0) != LIB_STATUS_OK;
    failed = failed || test_core_machine_advance_time(machine, 2u) != LIB_STATUS_OK;
    failed = failed || core_machine_memory_read(machine, 0x11234u, &value, 1u) != LIB_STATUS_OK ||
        value != 0u || !test_core_dma_wait_remaining_matches(machine, 0u);
    failed = failed || core_machine_set_dma_bus_ready(machine, 1) != LIB_STATUS_OK;
    failed = failed || test_core_machine_advance_time(machine, 1u) != LIB_STATUS_OK;
    failed = failed || core_machine_memory_read(machine, 0x11234u, &value, 1u) != LIB_STATUS_OK ||
        value != 0u || !test_core_dma_wait_remaining_matches(machine, 1u);
    failed = failed || test_core_machine_advance_time(machine, 9u) != LIB_STATUS_OK;
    failed = failed || core_machine_memory_read(machine, 0x11234u, &value, 1u) != LIB_STATUS_OK ||
        value != 0xa5u || !test_core_dma_wait_remaining_matches(machine, 0u);
    failed = failed || core_machine_reset(machine) != LIB_STATUS_OK ||
        !test_core_dma_wait_remaining_matches(machine, 0u);
    core_machine_destroy(machine);
    return !failed;
}
lib_i32 main(void)
{
    static const core_machine_dma_channel_provider dma_provider = {
        competition_dma_read, LIB_NULL, LIB_NULL
    };
    const lib_u8 nop = 0x90u;
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    core_machine_d4_platform *d4_board = LIB_NULL;
    core_machine_config config = {0};
    core_machine_d4_platform_config d4 = {CORE_MACHINE_PC_AT_PORT_B, 0u};
    core_machine_d4_platform_observation d4_observation;
    core_machine_trace_provider trace;
    core_machine_dma_request_binding binding = {0};
    core_machine_run_budget budget = {1u, 0u};
    core_machine_run_result result;
    competition_probe probe = {{{0}}, 0u};
    competition_dma_source source = {0xa5u};
    lib_u8 byte = 0u;
    lib_u32 cpu_begin = 0u;
    lib_u32 cpu_commit = 0u;
    lib_u32 cpu_retire = 0u;
    lib_u32 dma_begin = 0u;
    lib_u32 dma_commit = 0u;
    lib_u32 dma_advance = 0u;
    lib_u32 pit_advance = 0u;
    lib_u32 pic_refresh = 0u;
    lib_u32 fdc_advance = 0u;
    lib_u32 hdc_advance = 0u;
    lib_u32 hold_request = 0u;
    lib_u32 hold_acknowledge = 0u;
    lib_u32 hold_release = 0u;
    lib_u32 reset_hold_start;
    lib_u32 reset_hold_request = 0u;
    lib_u32 reset_hold_acknowledge = 0u;
    lib_u32 reset_hold_release = 0u;
    lib_i32 failed = 0;

    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80386;
    config.auxiliary_pit_present = LIB_TRUE;
    config.auxiliary_pit_base_port = 0x0048u;
    trace.callback = competition_trace;
    trace.context = &probe;
    failed = failed || core_machine_create(&config, &machine, &board) != LIB_STATUS_OK;
    failed = failed || core_machine_d4_platform_attach(board, &d4, &d4_board) != LIB_STATUS_OK;
    failed = failed || core_machine_d4_platform_observe(d4_board, &d4_observation) !=
        LIB_STATUS_OK || !d4_observation.configured;
    failed = failed || test_core_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
        0x000ffff0u, 16u) != LIB_STATUS_OK;
    failed = failed || test_board_dma_bind_channel(board, 2u,
        &dma_provider, &source, &binding) != LIB_STATUS_OK;
    failed = failed || !test_core_dma_hold_excludes_cpu(machine);
    failed = failed || core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed = failed || core_machine_reset(machine) != LIB_STATUS_OK;
    failed = failed || core_machine_memory_write(machine, 0xfffffff0u, &nop, 1u) !=
        LIB_STATUS_OK;
    if (!failed) competition_program_dma_channel2(machine);
    if (!failed) test_board_dma_request_assert(board, &binding);
    failed = failed || core_machine_set_trace_provider(machine, &trace) != LIB_STATUS_OK;
    failed = failed || core_machine_run(machine, budget, &result) != LIB_STATUS_OK;
    failed = failed || result.reason != CORE_MACHINE_STOP_BUDGET ||
        result.executed != 1u || result.elapsed_ticks != 3u;
    failed = failed || test_core_machine_advance_time(machine, 8u) != LIB_STATUS_OK;
    failed = failed || core_machine_memory_read(machine, 0x11234u, &byte, 1u) !=
        LIB_STATUS_OK || byte != 0xa5u;
    failed = failed || !competition_find_transaction(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_BEGIN,
        CORE_MACHINE_TRANSACTION_OWNER_CPU,
        CORE_MACHINE_TRANSACTION_CPU_MEMORY_READ, &cpu_begin);
    failed = failed || !competition_find_transaction(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_COMMIT,
        CORE_MACHINE_TRANSACTION_OWNER_CPU,
        CORE_MACHINE_TRANSACTION_CPU_MEMORY_READ, &cpu_commit);
    failed = failed || !competition_find_event(&probe, CORE_MACHINE_TRACE_CPU_RETIRE,
        &cpu_retire);
    failed = failed || !competition_find_transaction(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_BEGIN,
        CORE_MACHINE_TRANSACTION_OWNER_DMA,
        CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, &dma_begin);
    failed = failed || !competition_find_transaction(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_COMMIT,
        CORE_MACHINE_TRANSACTION_OWNER_DMA,
        CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, &dma_commit);
    failed = failed || !competition_find_event(&probe, CORE_MACHINE_TRACE_DMA_ADVANCE,
        &dma_advance);
    failed = failed || !competition_find_event(&probe, CORE_MACHINE_TRACE_PIT_ADVANCE,
        &pit_advance);
    failed = failed || !competition_find_event(&probe, CORE_MACHINE_TRACE_PIC_REFRESH,
        &pic_refresh);
    failed = failed || competition_find_event(&probe, CORE_MACHINE_TRACE_FDC_ADVANCE,
        &fdc_advance);
    failed = failed || competition_find_event(&probe, CORE_MACHINE_TRACE_HDC_ADVANCE,
        &hdc_advance);
    failed = failed || !competition_find_event(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_HOLD_REQUEST, &hold_request);
    failed = failed || !competition_find_event(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_HOLD_ACKNOWLEDGE, &hold_acknowledge);
    failed = failed || !competition_find_event(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_HOLD_RELEASE, &hold_release);
    failed = failed || cpu_begin >= cpu_commit || cpu_commit >= cpu_retire ||
        cpu_retire >= dma_begin || dma_begin >= dma_commit ||
        pit_advance >= pic_refresh;
    reset_hold_start = probe.count;
    failed = failed || !test_core_dma_hold_begin(machine);
    failed = failed || core_machine_reset(machine) != LIB_STATUS_OK;
    failed = failed || !competition_find_event_after(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_HOLD_REQUEST, reset_hold_start,
        &reset_hold_request);
    failed = failed || !competition_find_event_after(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_HOLD_ACKNOWLEDGE, reset_hold_start,
        &reset_hold_acknowledge);
    failed = failed || !competition_find_event_after(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_HOLD_RELEASE, reset_hold_start,
        &reset_hold_release);
    failed = failed || reset_hold_request >= reset_hold_acknowledge ||
        reset_hold_acknowledge >= reset_hold_release;

    core_machine_destroy(machine);
    failed = failed || !competition_dma_wait_contract();
    if (failed) return 1;
    printf("M5:T354:S3:COMPETITION:OK\n");
    printf("M5:T419:S1:D4-DMA-NO-WAIT:OK\n");
    printf("M5:T419:S3:D4-DMA-BUSRDY:OK\n");
    printf("M5:T369:S3:PCAT-HOLD:OK\n");
    return 0;
}
