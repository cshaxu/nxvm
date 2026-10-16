#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "x86-nxvm/board-base/dma_bus_interface.h"
#include "composition/composition_fixture.h"
#include "support/composition_fixture.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-nxvm/board-base/machine_board_state.h"
#include "x86-core/machine/trace_interface.h"
#include "board_board_fixture.h"

typedef struct competition_probe {
    x86_core_machine_trace_event events[256];
    lib_u32 count;
} competition_probe;

typedef struct competition_dma_source {
    lib_u8 value;
} competition_dma_source;

static void competition_trace(void *opaque,
    const x86_core_machine_trace_event *event)
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
    lib_u32 type, lib_u32 *out_index)
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

static lib_i32 competition_find_transaction(const competition_probe *probe,
    lib_u32 phase, x86_core_machine_transaction_owner owner,
    x86_core_machine_transaction_kind kind, lib_u32 *out_index)
{
    lib_u32 index;

    if (probe == LIB_NULL || out_index == LIB_NULL) return 0;
    for (index = 0u; index < probe->count; ++index) {
        const x86_core_machine_trace_event *event = &probe->events[index];

        if (event->type == phase && (event->detail & 0xffu) == owner &&
            ((event->detail >> 8u) & 0xffu) == kind) {
            *out_index = index;
            return 1;
        }
    }
    return 0;
}

static void competition_program_dma_channel2(x86_core_machine *machine)
{
    test_x86_nxvm_machine_fixture_write_port(machine, 0x000cu, 0u);
    test_x86_nxvm_machine_fixture_write_port(machine, 0x0004u, 0x34u);
    test_x86_nxvm_machine_fixture_write_port(machine, 0x0004u, 0x12u);
    test_x86_nxvm_machine_fixture_write_port(machine, 0x0005u, 0u);
    test_x86_nxvm_machine_fixture_write_port(machine, 0x0005u, 0u);
    test_x86_nxvm_machine_fixture_write_port(machine, 0x0081u, 1u);
    test_x86_nxvm_machine_fixture_write_port(machine, 0x000bu, 0x46u);
    test_x86_nxvm_machine_fixture_write_port(machine, 0x000au, 0x02u);
    test_x86_nxvm_machine_fixture_write_port(machine, 0x00d4u, 0u);
}

static lib_i32 competition_dma_wait_contract(void)
{
    static const x86_nxvm_board_dma_channel_provider provider = {
        competition_dma_read, LIB_NULL, LIB_NULL };
    x86_nxvm_board_config config = {0};
    x86_nxvm_board_dma_request_binding binding = {0};
    competition_dma_source source = {0xa5u};
    competition_probe probe = {{{0}}, 0u};
    const x86_core_machine_trace_provider trace = {competition_trace, &probe};
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_state *board = LIB_NULL;
    lib_u8 value = 0u;
    lib_u8 advanced = LIB_FALSE;
    lib_u64 ready_tick = 0u;
    lib_u32 dma_commit = 0u;
    lib_i32 failed = 0;

    config.cpu_profile = X86_CORE_CPU_PROFILE_80386;
    config.transaction_contract.dma_cycle_wait_quanta = 1u;
    config.transaction_contract.dma_cycle_bus_ready_gate_enabled = LIB_TRUE;
    config.l1_compatibility_policy = X86_CORE_MACHINE_L1_COMPATIBILITY_BOUNDED_PROGRESS;
    failed = failed || x86_nxvm_board_create(&config, &machine, &board) != LIB_STATUS_OK;
    failed = failed || test_x86_nxvm_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
        0x000ffff0u, 16u) != LIB_STATUS_OK;
    failed = failed || test_board_dma_bind_channel(board, 2u,
        &provider, &source, &binding) != LIB_STATUS_OK;
    failed = failed || x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed = failed || x86_core_machine_reset(machine) != LIB_STATUS_OK;
    failed = failed || x86_core_machine_memory_write(machine, 0x11234u, &value, 1u) != LIB_STATUS_OK;
    if (!failed) competition_program_dma_channel2(machine);
    if (!failed) test_board_dma_request_assert(board, &binding);
    failed = failed || x86_core_machine_set_dma_bus_ready(machine, 0) != LIB_STATUS_OK;
    failed = failed || x86_core_machine_advance_l1_compatibility(machine, &advanced) != LIB_STATUS_OK ||
        !advanced;
    failed = failed || x86_core_machine_memory_read(machine, 0x11234u, &value, 1u) != LIB_STATUS_OK ||
        value != 0u;
    failed = failed || x86_core_machine_set_dma_bus_ready(machine, 1) != LIB_STATUS_OK;
    failed = failed || x86_core_machine_get_elapsed_ticks(machine, &ready_tick) != LIB_STATUS_OK ||
        x86_core_machine_set_trace_provider(machine, &trace) != LIB_STATUS_OK ||
        x86_core_machine_advance_l1_compatibility(machine, &advanced) != LIB_STATUS_OK || !advanced;
    failed = failed || x86_core_machine_memory_read(machine, 0x11234u, &value, 1u) != LIB_STATUS_OK ||
        value != 0xa5u || !competition_find_transaction(&probe,
            X86_CORE_MACHINE_TRACE_TRANSACTION_COMMIT, X86_CORE_MACHINE_TRANSACTION_OWNER_DMA,
            X86_CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, &dma_commit) ||
        probe.events[dma_commit].timeline_ticks <= ready_tick + 1u ||
        probe.events[dma_commit].timeline_ticks > ready_tick + 10u;
    failed = failed || x86_core_machine_reset(machine) != LIB_STATUS_OK;
    x86_core_machine_destroy(machine);
    return !failed;
}
static lib_i32 competition_at_refresh_contract(void)
{
    const x86_nxvm_board_dma_wiring wiring = {
        .fdc_channel = X86_NXVM_BOARD_DMA_FDC_CHANNEL_UNBOUND,
        .controller_count = X86_NXVM_BOARD_DMA_CONTROLLER_COUNT,
        .cascade_channel = X86_NXVM_BOARD_DMA_CASCADE_CHANNEL
    };
    const x86_nxvm_board_planar_parity_config port_b = {
        .port = 0x0061u,
        .refresh_status_source = X86_NXVM_BOARD_PLANAR_PARITY_REFRESH_STATUS_PIT_COUNTER_1
    };
    static const x86_nxvm_board_dma_channel_provider provider = {
        competition_dma_read, LIB_NULL, LIB_NULL };
    x86_nxvm_board_config config = {0};
    x86_nxvm_board_dma_request_binding binding = {0};
    x86_nxvm_board_dma_request_binding fdc_request = {0};
    competition_dma_source source = {0xa5u};
    competition_probe probe = {{{0}}, 0u};
    const x86_core_machine_trace_provider trace = {competition_trace, &probe};
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_state *board = LIB_NULL;
    lib_u32 refresh_begin = 0u;
    lib_u32 refresh_commit = 0u;
    lib_u32 dma_begin = 0u;
    lib_u8 value = 0u;
    lib_u8 advanced = LIB_FALSE;
    x86_core_machine_run_result result = {0};
    lib_i32 failed = 0;

    config.cpu_profile = X86_CORE_CPU_PROFILE_80386;
    config.transaction_contract.refresh_cycle_source_ticks = 8u;
    config.l1_compatibility_policy = X86_CORE_MACHINE_L1_COMPATIBILITY_BOUNDED_PROGRESS;
    failed = x86_nxvm_board_create(&config, &machine, &board) != LIB_STATUS_OK ||
        x86_nxvm_board_configure_planar_parity(board, &port_b) != LIB_STATUS_OK ||
        x86_nxvm_board_configure_dma(board, &wiring, &fdc_request) != LIB_STATUS_OK ||
        test_x86_nxvm_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
            0x000ffff0u, 16u) != LIB_STATUS_OK ||
        test_board_dma_bind_channel(board, 2u, &provider, &source, &binding) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_run(machine, (x86_core_machine_run_budget){0u, 8u}, &result) != LIB_STATUS_OK ||
        result.executed != 0u || result.ticks != 8u ||
        x86_core_machine_memory_write(machine, 0x11234u, &value, 1u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0x0043u, 0x70u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0x0041u, 1u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0x0041u, 0u) != LIB_STATUS_OK;
    if (!failed) x86_pit_advance(board->shared_pit, 2u);
    failed = failed || x86_core_machine_set_trace_provider(machine, &trace) != LIB_STATUS_OK;
    if (!failed) competition_program_dma_channel2(machine);
    if (!failed) test_board_dma_request_assert(board, &binding);
    failed = failed || x86_core_machine_run(machine,
        (x86_core_machine_run_budget){0u, 7u}, &result) != LIB_STATUS_OK ||
        result.executed != 0u || result.ticks != 7u ||
        x86_core_machine_memory_read(machine, 0x11234u, &value, 1u) != LIB_STATUS_OK || value != 0u ||
        competition_find_transaction(&probe, X86_CORE_MACHINE_TRACE_TRANSACTION_BEGIN,
            X86_CORE_MACHINE_TRANSACTION_OWNER_DMA,
            X86_CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, &dma_begin) ||
        competition_find_transaction(&probe, X86_CORE_MACHINE_TRACE_TRANSACTION_COMMIT,
            X86_CORE_MACHINE_TRANSACTION_OWNER_REFRESH,
            X86_CORE_MACHINE_TRANSACTION_REFRESH_MEMORY_CYCLE, &refresh_commit) ||
        x86_core_machine_run(machine, (x86_core_machine_run_budget){0u, 1u}, &result) != LIB_STATUS_OK ||
        result.executed != 0u || result.ticks != 1u ||
        x86_core_machine_memory_read(machine, 0x11234u, &value, 1u) != LIB_STATUS_OK || value != 0u ||
        !competition_find_transaction(&probe, X86_CORE_MACHINE_TRACE_TRANSACTION_BEGIN,
            X86_CORE_MACHINE_TRANSACTION_OWNER_REFRESH,
            X86_CORE_MACHINE_TRANSACTION_REFRESH_MEMORY_CYCLE, &refresh_begin) ||
        !competition_find_transaction(&probe, X86_CORE_MACHINE_TRACE_TRANSACTION_COMMIT,
            X86_CORE_MACHINE_TRANSACTION_OWNER_REFRESH,
            X86_CORE_MACHINE_TRANSACTION_REFRESH_MEMORY_CYCLE, &refresh_commit) ||
        probe.events[refresh_commit].timeline_ticks -
            probe.events[refresh_begin].timeline_ticks != 8u ||
        x86_core_machine_advance_l1_compatibility(machine, &advanced) != LIB_STATUS_OK || !advanced ||
        x86_core_machine_memory_read(machine, 0x11234u, &value, 1u) != LIB_STATUS_OK || value != 0xa5u ||
        !competition_find_transaction(&probe, X86_CORE_MACHINE_TRACE_TRANSACTION_BEGIN,
            X86_CORE_MACHINE_TRANSACTION_OWNER_DMA,
            X86_CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, &dma_begin) || dma_begin <= refresh_commit;
    if (!failed) {
        lib_u32 dma_commit = 0u;

        failed = !competition_find_transaction(&probe,
            X86_CORE_MACHINE_TRACE_TRANSACTION_COMMIT, X86_CORE_MACHINE_TRANSACTION_OWNER_DMA,
            X86_CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, &dma_commit) ||
            probe.events[dma_commit].timeline_ticks > probe.events[refresh_commit].timeline_ticks + 9u;
    }
    if (failed) lib_c_printf("AT-REFRESH:DMA-EXCLUSION:FAIL events=%u value=%02x begin=%u/%u commit=%u/%u dma=%u\n",
        probe.count, value, refresh_begin,
        (lib_u32)probe.events[refresh_begin].timeline_ticks, refresh_commit,
        (lib_u32)probe.events[refresh_commit].timeline_ticks, dma_begin);
    x86_core_machine_destroy(machine);
    return !failed;
}

lib_i32 main(void)
{
    static const x86_nxvm_board_dma_channel_provider dma_provider = {
        competition_dma_read, LIB_NULL, LIB_NULL
    };
    const lib_u8 nop = 0x90u;
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_state *board = LIB_NULL;
    x86_nxvm_board_config config = {0};
    x86_core_machine_trace_provider trace;
    x86_nxvm_board_dma_request_binding binding = {0};
    x86_core_machine_run_budget budget = {1u, 0u};
    x86_core_machine_run_result result;
    competition_probe probe = {{{0}}, 0u};
    competition_dma_source source = {0xa5u};
    lib_u8 byte = 0u;
    lib_u8 advanced = LIB_FALSE;
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
    lib_i32 failed = 0;

    config.cpu_profile = X86_CORE_CPU_PROFILE_80386;
    trace.callback = competition_trace;
    config.l1_compatibility_policy = X86_CORE_MACHINE_L1_COMPATIBILITY_BOUNDED_PROGRESS;
    trace.context = &probe;
    failed = failed || x86_nxvm_board_create(&config, &machine, &board) != LIB_STATUS_OK;
    failed = failed || test_x86_nxvm_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
        0x000ffff0u, 16u) != LIB_STATUS_OK;
    failed = failed || test_board_dma_bind_channel(board, 2u,
        &dma_provider, &source, &binding) != LIB_STATUS_OK;
    failed = failed || x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed = failed || x86_core_machine_reset(machine) != LIB_STATUS_OK;
    failed = failed || x86_core_machine_memory_write(machine, 0xfffffff0u, &nop, 1u) !=
        LIB_STATUS_OK;
    if (!failed) competition_program_dma_channel2(machine);
    if (!failed) test_board_dma_request_assert(board, &binding);
    failed = failed || x86_core_machine_set_trace_provider(machine, &trace) != LIB_STATUS_OK;
    failed = failed || x86_core_machine_run(machine, budget, &result) != LIB_STATUS_OK;
    failed = failed || result.reason != X86_CORE_MACHINE_STOP_BUDGET ||
        result.executed != 1u || result.elapsed_ticks != 3u;
    failed = failed || x86_core_machine_advance_l1_compatibility(machine, &advanced) != LIB_STATUS_OK ||
        !advanced;
    failed = failed || x86_core_machine_memory_read(machine, 0x11234u, &byte, 1u) !=
        LIB_STATUS_OK || byte != 0xa5u;
    failed = failed || !competition_find_transaction(&probe,
        X86_CORE_MACHINE_TRACE_TRANSACTION_BEGIN,
        X86_CORE_MACHINE_TRANSACTION_OWNER_CPU,
        X86_CORE_MACHINE_TRANSACTION_CPU_MEMORY_READ, &cpu_begin);
    failed = failed || !competition_find_transaction(&probe,
        X86_CORE_MACHINE_TRACE_TRANSACTION_COMMIT,
        X86_CORE_MACHINE_TRANSACTION_OWNER_CPU,
        X86_CORE_MACHINE_TRANSACTION_CPU_MEMORY_READ, &cpu_commit);
    failed = failed || !competition_find_event(&probe, X86_CORE_MACHINE_TRACE_CPU_RETIRE,
        &cpu_retire);
    failed = failed || !competition_find_transaction(&probe,
        X86_CORE_MACHINE_TRACE_TRANSACTION_BEGIN,
        X86_CORE_MACHINE_TRANSACTION_OWNER_DMA,
        X86_CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, &dma_begin);
    failed = failed || !competition_find_transaction(&probe,
        X86_CORE_MACHINE_TRACE_TRANSACTION_COMMIT,
        X86_CORE_MACHINE_TRANSACTION_OWNER_DMA,
        X86_CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, &dma_commit);
    failed = failed || probe.events[dma_commit].timeline_ticks > 11u;
    failed = failed || !competition_find_event(&probe, X86_CORE_MACHINE_TRACE_DMA_ADVANCE,
        &dma_advance);
    failed = failed || !competition_find_event(&probe, X86_NXVM_BOARD_TRACE_PIT_ADVANCE,
        &pit_advance);
    failed = failed || !competition_find_event(&probe, X86_NXVM_BOARD_TRACE_PIC_REFRESH,
        &pic_refresh);
    failed = failed || competition_find_event(&probe, X86_NXVM_BOARD_TRACE_FDC_ADVANCE,
        &fdc_advance);
    failed = failed || competition_find_event(&probe, X86_NXVM_BOARD_TRACE_HDC_ADVANCE,
        &hdc_advance);
    failed = failed || !competition_find_event(&probe,
        X86_CORE_MACHINE_TRACE_TRANSACTION_HOLD_REQUEST, &hold_request);
    failed = failed || !competition_find_event(&probe,
        X86_CORE_MACHINE_TRACE_TRANSACTION_HOLD_ACKNOWLEDGE, &hold_acknowledge);
    failed = failed || !competition_find_event(&probe,
        X86_CORE_MACHINE_TRACE_TRANSACTION_HOLD_RELEASE, &hold_release);
    failed = failed || cpu_begin >= cpu_commit || cpu_commit >= cpu_retire ||
        cpu_retire >= dma_begin || dma_begin >= dma_commit ||
        pit_advance >= pic_refresh;
    failed = failed || x86_core_machine_reset(machine) != LIB_STATUS_OK;

    x86_core_machine_destroy(machine);
    failed = failed || !competition_dma_wait_contract();
    failed = failed || !competition_at_refresh_contract();
    if (failed) return 1;
    lib_c_printf("COMPETITION:OK\n");
    lib_c_printf("DMA-NO-WAIT:OK\n");
    lib_c_printf("DMA-BUSRDY:OK\n");
    lib_c_printf("DMA-HOLD:OK\n");
    return 0;
}
