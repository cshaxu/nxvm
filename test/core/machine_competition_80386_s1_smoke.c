#include "type.h"

#include "core/machine/dma.h"
#include "core/machine/machine.h"
#include "core/machine/machine_interface.h"
#include "core/machine/transaction.h"
#include "support/core_machine_cpu_fixture.h"

typedef struct competition_probe {
    core_machine_trace_event events[256];
    type_unsigned_32 count;
} competition_probe;

typedef struct competition_dma_source {
    type_unsigned_8 value;
} competition_dma_source;

static C_VOID competition_trace(C_VOID *opaque,
    const core_machine_trace_event *event)
{
    competition_probe *probe = (competition_probe *)opaque;

    if (probe != STD_NULL && probe->count < 256u) {
        probe->events[probe->count++] = *event;
    }
}

static C_VOID competition_dma_read(C_VOID *opaque, t_latch *latch)
{
    competition_dma_source *source = (competition_dma_source *)opaque;

    if (source != STD_NULL && latch != STD_NULL) latch->data.byte = source->value;
}

static C_INT competition_find_event(const competition_probe *probe,
    core_machine_trace_event_type type, type_unsigned_32 *out_index)
{
    type_unsigned_32 index;

    if (probe == STD_NULL || out_index == STD_NULL) return 0;
    for (index = 0u; index < probe->count; ++index) {
        if (probe->events[index].type == type) {
            *out_index = index;
            return 1;
        }
    }
    return 0;
}

static C_INT competition_find_event_after(const competition_probe *probe,
    core_machine_trace_event_type type, type_unsigned_32 start,
    type_unsigned_32 *out_index)
{
    type_unsigned_32 index;

    if (probe == STD_NULL || out_index == STD_NULL) return 0;
    for (index = start; index < probe->count; ++index) {
        if (probe->events[index].type == type) {
            *out_index = index;
            return 1;
        }
    }
    return 0;
}

static C_INT competition_find_transaction(const competition_probe *probe,
    core_machine_trace_event_type phase, core_machine_transaction_owner owner,
    core_machine_transaction_kind kind, type_unsigned_32 *out_index)
{
    type_unsigned_32 index;

    if (probe == STD_NULL || out_index == STD_NULL) return 0;
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

static C_VOID competition_program_dma_channel2(t_port *port)
{
    core_machine_port_write(port, 0x000cu, 0u);
    core_machine_port_write(port, 0x0004u, 0x34u);
    core_machine_port_write(port, 0x0004u, 0x12u);
    core_machine_port_write(port, 0x0005u, 0u);
    core_machine_port_write(port, 0x0005u, 0u);
    core_machine_port_write(port, 0x0081u, 1u);
    core_machine_port_write(port, 0x000bu, 0x46u);
    core_machine_port_write(port, 0x000au, 0x02u);
}

C_INT main(C_VOID)
{
    static const core_machine_dma_channel_provider dma_provider = {
        competition_dma_read, STD_NULL, STD_NULL
    };
    const type_unsigned_8 nop = 0x90u;
    core_machine *machine = STD_NULL;
    core_machine_config config = {0};
    core_machine_trace_provider trace;
    core_machine_dma_request_binding binding = {0};
    core_machine_run_budget budget = {1u, 0u};
    core_machine_run_result result;
    competition_probe probe = {{{0}}, 0u};
    competition_dma_source source = {0xa5u};
    type_unsigned_8 byte = 0u;
    type_unsigned_32 cpu_begin = 0u;
    type_unsigned_32 cpu_commit = 0u;
    type_unsigned_32 cpu_retire = 0u;
    type_unsigned_32 dma_begin = 0u;
    type_unsigned_32 dma_commit = 0u;
    type_unsigned_32 dma_advance = 0u;
    type_unsigned_32 pit_advance = 0u;
    type_unsigned_32 pic_refresh = 0u;
    type_unsigned_32 fdc_advance = 0u;
    type_unsigned_32 hdc_advance = 0u;
    type_unsigned_32 hold_request = 0u;
    type_unsigned_32 hold_acknowledge = 0u;
    type_unsigned_32 hold_release = 0u;
    type_unsigned_32 reset_hold_start;
    type_unsigned_32 reset_hold_request = 0u;
    type_unsigned_32 reset_hold_acknowledge = 0u;
    type_unsigned_32 reset_hold_release = 0u;
    C_INT failed = 0;

    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80386;
    trace.callback = competition_trace;
    trace.context = &probe;
    failed |= core_machine_create(&config, &machine) != TYPE_STATUS_OK;
    failed |= test_core_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
        0x000ffff0u, 16u) != TYPE_STATUS_OK;
    failed |= core_machine_dma_bind_channel(&machine->shared_dma_latch,
        &machine->shared_dma_primary, &machine->shared_dma_secondary, 2u,
        &dma_provider, &source, &binding) != TYPE_STATUS_OK;
    failed |= core_machine_transaction_hold_request(&machine->transaction,
        CORE_MACHINE_TRANSACTION_OWNER_DMA, 0u) != TYPE_STATUS_OK;
    failed |= core_machine_transaction_hold_acknowledge(&machine->transaction,
        CORE_MACHINE_TRANSACTION_OWNER_DMA) != TYPE_STATUS_OK;
    failed |= core_machine_transaction_begin(&machine->transaction,
        CORE_MACHINE_TRANSACTION_OWNER_CPU,
        CORE_MACHINE_TRANSACTION_CPU_MEMORY_READ, 0u, 0u, 0u) !=
        TYPE_STATUS_INVALID_ARGUMENT;
    core_machine_transaction_hold_release(&machine->transaction,
        CORE_MACHINE_TRANSACTION_OWNER_DMA);
    failed |= core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK;
    failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0xfffffff0u, &nop, 1u) !=
        TYPE_STATUS_OK;
    competition_program_dma_channel2(&machine->executor_port);
    core_machine_dma_request_assert(&machine->shared_dma_primary,
        &machine->shared_dma_secondary, &binding);
    failed |= core_machine_set_trace_provider(machine, &trace) != TYPE_STATUS_OK;
    failed |= core_machine_run(machine, budget, &result) != TYPE_STATUS_OK;
    failed |= result.reason != CORE_MACHINE_STOP_BUDGET ||
        result.executed != 1u || result.elapsed_ticks != 3u;
    /* A normal 8237A service selects a channel before S1..S4 commits the
     * byte.  CPU retirement has already occurred; advance only device time. */
    failed |= core_machine_advance_time(machine, 8u) != TYPE_STATUS_OK;
    failed |= core_machine_memory_read(machine, 0x11234u, &byte, 1u) !=
        TYPE_STATUS_OK || byte != 0xa5u;
    failed |= !competition_find_transaction(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_BEGIN,
        CORE_MACHINE_TRANSACTION_OWNER_CPU,
        CORE_MACHINE_TRANSACTION_CPU_MEMORY_READ, &cpu_begin);
    failed |= !competition_find_transaction(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_COMMIT,
        CORE_MACHINE_TRANSACTION_OWNER_CPU,
        CORE_MACHINE_TRANSACTION_CPU_MEMORY_READ, &cpu_commit);
    failed |= !competition_find_event(&probe, CORE_MACHINE_TRACE_CPU_RETIRE,
        &cpu_retire);
    failed |= !competition_find_transaction(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_BEGIN,
        CORE_MACHINE_TRANSACTION_OWNER_DMA,
        CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, &dma_begin);
    failed |= !competition_find_transaction(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_COMMIT,
        CORE_MACHINE_TRANSACTION_OWNER_DMA,
        CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, &dma_commit);
    failed |= !competition_find_event_after(&probe, CORE_MACHINE_TRACE_DMA_ADVANCE,
        dma_commit, &dma_advance);
    failed |= !competition_find_event_after(&probe, CORE_MACHINE_TRACE_PIT_ADVANCE,
        dma_advance, &pit_advance);
    failed |= !competition_find_event_after(&probe, CORE_MACHINE_TRACE_PIC_REFRESH,
        pit_advance, &pic_refresh);
    failed |= competition_find_event(&probe, CORE_MACHINE_TRACE_FDC_ADVANCE,
        &fdc_advance);
    failed |= competition_find_event(&probe, CORE_MACHINE_TRACE_HDC_ADVANCE,
        &hdc_advance);
    failed |= !competition_find_event(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_HOLD_REQUEST, &hold_request);
    failed |= !competition_find_event(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_HOLD_ACKNOWLEDGE, &hold_acknowledge);
    failed |= !competition_find_event(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_HOLD_RELEASE, &hold_release);
    failed |= cpu_begin >= cpu_commit || cpu_commit >= cpu_retire ||
        cpu_retire >= dma_begin || dma_begin >= dma_commit ||
        dma_commit >= dma_advance || dma_advance >= pit_advance ||
        pit_advance >= pic_refresh;
    reset_hold_start = probe.count;
    failed |= core_machine_transaction_hold_request(&machine->transaction,
        CORE_MACHINE_TRANSACTION_OWNER_DMA, 0u) != TYPE_STATUS_OK;
    failed |= core_machine_transaction_hold_acknowledge(&machine->transaction,
        CORE_MACHINE_TRANSACTION_OWNER_DMA) != TYPE_STATUS_OK;
    failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= !competition_find_event_after(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_HOLD_REQUEST, reset_hold_start,
        &reset_hold_request);
    failed |= !competition_find_event_after(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_HOLD_ACKNOWLEDGE, reset_hold_start,
        &reset_hold_acknowledge);
    failed |= !competition_find_event_after(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_HOLD_RELEASE, reset_hold_start,
        &reset_hold_release);
    failed |= reset_hold_request >= reset_hold_acknowledge ||
        reset_hold_acknowledge >= reset_hold_release;

    core_machine_destroy(machine);
    if (failed) return 1;
    STD_PRINTF("M5:T354:S3:COMPETITION:OK\n");
    STD_PRINTF("M5:T369:S3:PCAT-HOLD:OK\n");
    STD_PRINTF("M5:T400:S1:80386-DMA-HOLD:OK\n");
    return 0;
}
