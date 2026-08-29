#include "type.h"

#include "core/machine/cpu_instructions.h"
#include "core/machine/dma.h"
#include "core/machine/machine_interface.h"
#include "core/machine/memory.h"
#include "core/machine/port.h"
#include "core/machine/transaction.h"
#include "support/core_machine_cpu_fixture.h"

typedef struct transaction_probe {
    core_machine_trace_event events[256];
    type_unsigned_32 count;
    type_unsigned_32 port_value;
} transaction_probe;

typedef struct dma_source {
    type_unsigned_8 value;
} dma_source;

typedef struct transaction_state_probe {
    type_unsigned_32 begin_count;
    type_unsigned_32 commit_count;
    type_unsigned_32 cancel_count;
    core_machine_transaction_owner owner;
    core_machine_transaction_kind kind;
} transaction_state_probe;

static C_VOID transaction_trace(C_VOID *opaque,
    const core_machine_trace_event *event)
{
    transaction_probe *probe = (transaction_probe *)opaque;

    if (probe != STD_NULL && probe->count < 256u) {
        probe->events[probe->count++] = *event;
    }
}

static type_status transaction_port_read(C_VOID *opaque,
    type_unsigned_16 port, type_unsigned_32 *out_value)
{
    transaction_probe *probe = (transaction_probe *)opaque;

    (C_VOID)port;
    if (probe == STD_NULL || out_value == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_value = probe->port_value;
    return TYPE_STATUS_OK;
}

static type_status transaction_port_write(C_VOID *opaque,
    type_unsigned_16 port, type_unsigned_32 value)
{
    transaction_probe *probe = (transaction_probe *)opaque;

    (C_VOID)port;
    if (probe == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    probe->port_value = value;
    return TYPE_STATUS_OK;
}

static C_VOID transaction_dma_read(C_VOID *opaque, t_latch *latch)
{
    dma_source *source = (dma_source *)opaque;

    if (source != STD_NULL && latch != STD_NULL) {
        latch->data.byte = source->value;
    }
}

static C_VOID transaction_state_trace(C_VOID *opaque,
    core_machine_transaction_owner owner, core_machine_transaction_kind kind,
    core_machine_transaction_phase phase, type_unsigned_32 address,
    type_unsigned_32 value, type_unsigned_32 detail)
{
    transaction_state_probe *probe = (transaction_state_probe *)opaque;

    (C_VOID)address;
    (C_VOID)value;
    (C_VOID)detail;
    if (probe == STD_NULL) return;
    probe->owner = owner;
    probe->kind = kind;
    if (phase == CORE_MACHINE_TRANSACTION_PHASE_BEGIN) ++probe->begin_count;
    if (phase == CORE_MACHINE_TRANSACTION_PHASE_COMMIT) ++probe->commit_count;
    if (phase == CORE_MACHINE_TRANSACTION_PHASE_CANCEL) ++probe->cancel_count;
}

static C_INT transaction_has_pair(const transaction_probe *probe,
    core_machine_trace_event_type begin_type,
    core_machine_trace_event_type end_type, type_unsigned_8 owner,
    type_unsigned_8 kind)
{
    type_unsigned_32 index;

    for (index = 0u; index + 1u < probe->count; ++index) {
        if (probe->events[index].type == begin_type &&
            probe->events[index + 1u].type == end_type &&
            (probe->events[index].detail & 0xffu) == owner &&
            ((probe->events[index].detail >> 8u) & 0xffu) == kind) {
            return 1;
        }
    }
    return 0;
}

static C_INT transaction_has_provenance_pair(const transaction_probe *probe,
    core_machine_cpu_memory_access_provenance provenance)
{
    type_unsigned_32 index;

    for (index = 0u; index + 1u < probe->count; ++index) {
        if (probe->events[index].type == CORE_MACHINE_TRACE_TRANSACTION_BEGIN &&
            probe->events[index + 1u].type == CORE_MACHINE_TRACE_TRANSACTION_COMMIT &&
            (probe->events[index].detail & 0xffu) == CORE_MACHINE_TRANSACTION_OWNER_CPU &&
            ((probe->events[index].detail >> 8u) & 0xffu) ==
                CORE_MACHINE_TRANSACTION_CPU_MEMORY_READ &&
            (probe->events[index].detail >> 16u) == provenance) return 1;
    }
    return 0;
}

static C_INT transaction_find_external_cycle(const transaction_probe *probe,
    core_machine_trace_event_type type,
    core_machine_cpu_memory_access_provenance provenance,
    type_unsigned_32 *out_index)
{
    type_unsigned_32 index;

    if (probe == STD_NULL || out_index == STD_NULL) return 0;
    for (index = 0u; index < probe->count; ++index) {
        if (probe->events[index].type == type &&
            probe->events[index].detail == provenance) {
            *out_index = index;
            return 1;
        }
    }
    return 0;
}
static type_unsigned_32 transaction_count_external_cycles(
    const transaction_probe *probe, core_machine_trace_event_type type,
    core_machine_cpu_memory_access_provenance provenance)
{
    type_unsigned_32 index;
    type_unsigned_32 count = 0u;

    if (probe == STD_NULL) return 0u;
    for (index = 0u; index < probe->count; ++index) {
        if (probe->events[index].type == type &&
            probe->events[index].detail == provenance) ++count;
    }
    return count;
}

static C_VOID transaction_dma_program_channel2(t_port *port)
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
    static const core_machine_port_provider port_provider = {
        transaction_port_read, transaction_port_write
    };
    static const core_machine_dma_channel_provider dma_provider = {
        transaction_dma_read, STD_NULL, STD_NULL
    };
    static const type_unsigned_8 code[] = {
        0xebu, 0x01u, 0x90u, 0xb0u, 0x5au, 0xe6u, 0xe0u,
        0xa0u, 0x10u, 0u, 0xf4u
    };
    static const type_unsigned_8 reset_code[] = {
        0xb0u, 0x6cu, 0xe6u, 0xe0u, 0xf4u
    };
    core_machine *machine = STD_NULL;
    core_machine_config config = {0};
    core_machine_trace_provider trace;
    core_machine_run_budget budget = {5u, 0u};
    core_machine_run_result result;
    transaction_probe probe = {{{0}}, 0u, 0u};
    type_unsigned_8 data = 0x3cu;
    t_latch latch = {0};
    t_dma primary = {0};
    t_dma secondary = {0};
    t_ram memory = {0};
    t_port port;
    core_machine_dma_request_binding binding = {0};
    core_machine_transaction_state transaction;
    transaction_state_probe state_probe = {0};
    dma_source source = {0xa5u};
    type_unsigned_32 external_begin = 0u;
    type_unsigned_32 external_commit = 0u;
    C_INT failed = 0;

    trace.callback = transaction_trace;
    trace.context = &probe;
    failed |= core_machine_create(&config, &machine) != TYPE_STATUS_OK;
    failed |= test_core_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
        0x000ffff0u, 16u) != TYPE_STATUS_OK;
    failed |= core_machine_install_port_provider(machine, 0x00e0u, 0x00e0u,
        &port_provider, &probe) != TYPE_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK;
    failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0x000ffff0u, code, sizeof(code)) !=
        TYPE_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0x10u, &data, 1u) != TYPE_STATUS_OK;
    failed |= core_machine_set_trace_provider(machine, &trace) != TYPE_STATUS_OK;
    failed |= core_machine_run(machine, budget, &result) != TYPE_STATUS_OK;
    failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
    failed |= probe.port_value != 0x5au;
    failed |= !transaction_has_pair(&probe, CORE_MACHINE_TRACE_TRANSACTION_BEGIN,
        CORE_MACHINE_TRACE_TRANSACTION_COMMIT,
        CORE_MACHINE_TRANSACTION_OWNER_CPU,
        CORE_MACHINE_TRANSACTION_CPU_MEMORY_READ);
    failed |= !transaction_has_pair(&probe, CORE_MACHINE_TRACE_TRANSACTION_BEGIN,
        CORE_MACHINE_TRACE_TRANSACTION_COMMIT,
        CORE_MACHINE_TRANSACTION_OWNER_CPU,
        CORE_MACHINE_TRANSACTION_CPU_PORT_WRITE);
    failed |= !transaction_find_external_cycle(&probe,
        CORE_MACHINE_TRACE_CPU_EXTERNAL_CYCLE_BEGIN,
        CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH, &external_begin);
    failed |= !transaction_find_external_cycle(&probe,
        CORE_MACHINE_TRACE_CPU_EXTERNAL_CYCLE_COMMIT,
        CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH, &external_commit);
    failed |= external_begin >= external_commit;
    /* The control transfer starts a second logical window. Its 15-byte range
     * crosses the reset-vector 32-bit wrap, so observation records three
     * physical prefetch cycles: initial, destination, and wrapped tail. */
    failed |= transaction_count_external_cycles(&probe,
        CORE_MACHINE_TRACE_CPU_EXTERNAL_CYCLE_BEGIN,
        CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH) != 3u;
    failed |= transaction_count_external_cycles(&probe,
        CORE_MACHINE_TRACE_CPU_EXTERNAL_CYCLE_COMMIT,
        CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH) != 3u;
    failed |= !transaction_has_provenance_pair(&probe,
        CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH);
    failed |= transaction_has_provenance_pair(&probe,
        CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_FETCH);
    failed |= !transaction_has_provenance_pair(&probe,
        CORE_MACHINE_CPU_MEMORY_ACCESS_DATA);

    probe.count = 0u;
    failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0x000ffff0u, reset_code,
        sizeof(reset_code)) != TYPE_STATUS_OK;
    failed |= core_machine_run(machine, (core_machine_run_budget){3u, 0u},
        &result) != TYPE_STATUS_OK;
    failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
    failed |= probe.port_value != 0x6cu;
    failed |= transaction_count_external_cycles(&probe,
        CORE_MACHINE_TRACE_CPU_EXTERNAL_CYCLE_BEGIN,
        CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH) != 1u;
    core_machine_transaction_initialize(&transaction);
    core_machine_transaction_bind_trace(&transaction, transaction_state_trace,
        &state_probe);
    failed |= core_machine_transaction_begin(&transaction,
        CORE_MACHINE_TRANSACTION_OWNER_DMA,
        CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, 0x11234u, 1u, 2u) !=
        TYPE_STATUS_OK;
    core_machine_transaction_reset(&transaction);
    failed |= transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_NONE ||
        transaction.committed_count != 0u || transaction.cancelled_count != 0u;
    failed |= state_probe.begin_count != 1u || state_probe.cancel_count != 1u ||
        state_probe.commit_count != 0u ||
        state_probe.owner != CORE_MACHINE_TRANSACTION_OWNER_DMA ||
        state_probe.kind != CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE;

    core_machine_port_initialize(&port);
    core_machine_memory_initialize(&memory);
    failed |= core_machine_memory_allocate_for(&memory, 2u * 1024u * 1024u) !=
        TYPE_STATUS_OK;
    core_machine_dma_initialize(&latch, &primary, &secondary, &port, 2u);
    core_machine_dma_reset(&latch, &primary, &secondary);
    failed |= core_machine_dma_bind_channel(&latch, &primary, &secondary, 2u,
        &dma_provider, &source, &binding) != TYPE_STATUS_OK;
    transaction_dma_program_channel2(&port);
    core_machine_dma_request_assert(&primary, &secondary, &binding);
    core_machine_transaction_initialize(&transaction);
    STD_MEMSET(&state_probe, 0, sizeof(state_probe));
    core_machine_transaction_bind_trace(&transaction, transaction_state_trace,
        &state_probe);
    /* 8237A normal timing selects the channel, then completes S1..S4 before
     * the actual memory write.  One transaction tick is one controller phase. */
    core_machine_dma_advance_transaction(&latch, &primary, &secondary, &memory,
        &transaction, 5u);
    failed |= transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_NONE ||
        transaction.committed_count != 1u;
    failed |= state_probe.begin_count != 1u || state_probe.commit_count != 1u ||
        state_probe.cancel_count != 0u ||
        state_probe.owner != CORE_MACHINE_TRANSACTION_OWNER_DMA ||
        state_probe.kind != CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE;
    failed |= core_machine_memory_read_physical(&memory, 0x11234u,
        (type_virtual_address)&data, 1u) != TYPE_STATUS_OK || data != 0xa5u;

    core_machine_memory_finalize(&memory);
    core_machine_port_finalize(&port);
    core_machine_destroy(machine);
    if (failed != 0) return 1;
    STD_PRINTF("M5:T354:S2:TRANSACTION:OK\n");
    STD_PRINTF("M5:T409:S1:CPU-MEMORY-PROVENANCE:OK\n");
    STD_PRINTF("M5:T410:S1:CPU-EXTERNAL-CYCLE:OK\n");
    STD_PRINTF("M5:T411:S1:CPU-PREFETCH-WINDOW:OK\n");
    return 0;
}
