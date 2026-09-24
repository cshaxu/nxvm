#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/cpu_instructions.h"
#include "app-nxvm/devices/dma.h"
#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/devices/memory.h"
#include "app-nxvm/devices/port.h"
#include "app-nxvm/devices/transaction.h"
#include "support/core_machine_cpu_fixture.h"

typedef struct transaction_probe {
    core_machine_trace_event events[256];
    lib_u32 count;
    lib_u32 port_value;
} transaction_probe;

typedef struct dma_source {
    lib_u8 value;
} dma_source;

typedef struct transaction_state_probe {
    lib_u32 begin_count;
    lib_u32 commit_count;
    lib_u32 cancel_count;
    core_machine_transaction_owner owner;
    core_machine_transaction_kind kind;
} transaction_state_probe;

static void transaction_trace(void *opaque,
    const core_machine_trace_event *event)
{
    transaction_probe *probe = (transaction_probe *)opaque;

    if (probe != LIB_NULL && probe->count < 256u) {
        probe->events[probe->count++] = *event;
    }
}

static lib_status transaction_port_read(void *opaque,
    lib_u16 port, lib_u32 *out_value)
{
    transaction_probe *probe = (transaction_probe *)opaque;

    (void)port;
    if (probe == LIB_NULL || out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_value = probe->port_value;
    return LIB_STATUS_OK;
}

static lib_status transaction_port_write(void *opaque,
    lib_u16 port, lib_u32 value)
{
    transaction_probe *probe = (transaction_probe *)opaque;

    (void)port;
    if (probe == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    probe->port_value = value;
    return LIB_STATUS_OK;
}

static void transaction_dma_read(void *opaque, t_latch *latch)
{
    dma_source *source = (dma_source *)opaque;

    if (source != LIB_NULL && latch != LIB_NULL) {
        latch->data.byte = source->value;
    }
}

static void transaction_state_trace(void *opaque,
    core_machine_transaction_owner owner, core_machine_transaction_kind kind,
    core_machine_transaction_phase phase, lib_u32 address,
    lib_u32 value, lib_u32 detail)
{
    transaction_state_probe *probe = (transaction_state_probe *)opaque;

    (void)address;
    (void)value;
    (void)detail;
    if (probe == LIB_NULL) return;
    probe->owner = owner;
    probe->kind = kind;
    if (phase == CORE_MACHINE_TRANSACTION_PHASE_BEGIN) ++probe->begin_count;
    if (phase == CORE_MACHINE_TRANSACTION_PHASE_COMMIT) ++probe->commit_count;
    if (phase == CORE_MACHINE_TRANSACTION_PHASE_CANCEL) ++probe->cancel_count;
}

static lib_i32 transaction_has_pair(const transaction_probe *probe,
    core_machine_trace_event_type begin_type,
    core_machine_trace_event_type end_type, lib_u8 owner,
    lib_u8 kind)
{
    lib_u32 index;

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

static lib_i32 transaction_has_provenance_pair(const transaction_probe *probe,
    core_machine_cpu_memory_access_provenance provenance)
{
    lib_u32 index;

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

static lib_i32 transaction_has_port_write_value(const transaction_probe *probe,
    lib_u16 port, lib_u8 value)
{
    lib_u32 index;

    if (probe == LIB_NULL) return 0;
    for (index = 0u; index < probe->count; ++index) {
        const core_machine_trace_event *event = &probe->events[index];

        if (event->type == CORE_MACHINE_TRACE_TRANSACTION_BEGIN &&
            (event->detail & 0xffu) == CORE_MACHINE_TRANSACTION_OWNER_CPU &&
            ((event->detail >> 8u) & 0xffu) ==
                CORE_MACHINE_TRANSACTION_CPU_PORT_WRITE &&
            event->address == port && event->value == value) return 1;
    }
    return 0;
}

static lib_i32 transaction_find_external_cycle(const transaction_probe *probe,
    core_machine_trace_event_type type,
    core_machine_cpu_memory_access_provenance provenance,
    lib_u32 *out_index)
{
    lib_u32 index;

    if (probe == LIB_NULL || out_index == LIB_NULL) return 0;
    for (index = 0u; index < probe->count; ++index) {
        if (probe->events[index].type == type &&
            probe->events[index].detail == provenance) {
            *out_index = index;
            return 1;
        }
    }
    return 0;
}
static lib_u32 transaction_count_external_cycles(
    const transaction_probe *probe, core_machine_trace_event_type type,
    core_machine_cpu_memory_access_provenance provenance)
{
    lib_u32 index;
    lib_u32 count = 0u;

    if (probe == LIB_NULL) return 0u;
    for (index = 0u; index < probe->count; ++index) {
        if (probe->events[index].type == type &&
            probe->events[index].detail == provenance) ++count;
    }
    return count;
}

static void transaction_dma_program_channel2(t_port *port)
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

lib_i32 main(void)
{
    static const core_machine_port_provider port_provider = {
        transaction_port_read, transaction_port_write
    };
    static const core_machine_dma_channel_provider dma_provider = {
        transaction_dma_read, LIB_NULL, LIB_NULL
    };
    static const lib_u8 code[] = {
        0xebu, 0x01u, 0x90u, 0xb0u, 0x5au, 0xe6u, 0xe0u,
        0xa0u, 0x10u, 0u, 0xf4u
    };
    static const lib_u8 reset_code[] = {
        0xb0u, 0x6cu, 0xe6u, 0xe0u, 0xf4u
    };
    core_machine *machine = LIB_NULL;
    core_machine_config config = {0};
    core_machine_trace_provider trace;
    core_machine_run_budget budget = {5u, 0u};
    core_machine_run_result result;
    transaction_probe probe = {{{0}}, 0u, 0u};
    lib_u8 data = 0x3cu;
    t_latch latch = {0};
    t_dma primary = {0};
    t_dma secondary = {0};
    t_ram memory = {0};
    t_port port;
    core_machine_dma_request_binding binding = {0};
    core_machine_transaction_state transaction;
    transaction_state_probe state_probe = {0};
    dma_source source = {0xa5u};
    lib_u32 external_begin = 0u;
    lib_u32 external_commit = 0u;
    lib_i32 failed = 0;

    trace.callback = transaction_trace;
    trace.context = &probe;
    failed |= core_machine_create(&config, &machine) != LIB_STATUS_OK;
    failed |= test_core_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
        0x000ffff0u, 16u) != LIB_STATUS_OK;
    failed |= core_machine_install_port_provider(machine, 0x00e0u, 0x00e0u,
        &port_provider, &probe) != LIB_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0x000ffff0u, code, sizeof(code)) !=
        LIB_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0x10u, &data, 1u) != LIB_STATUS_OK;
    failed |= core_machine_set_trace_provider(machine, &trace) != LIB_STATUS_OK;
    failed |= core_machine_run(machine, budget, &result) != LIB_STATUS_OK;
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
    failed |= !transaction_has_port_write_value(&probe, 0x00e0u, 0x5au);
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
    failed |= core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0x000ffff0u, reset_code,
        sizeof(reset_code)) != LIB_STATUS_OK;
    failed |= core_machine_run(machine, (core_machine_run_budget){3u, 0u},
        &result) != LIB_STATUS_OK;
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
        LIB_STATUS_OK;
    core_machine_transaction_reset(&transaction);
    failed |= transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_NONE ||
        transaction.committed_count != 0u || transaction.cancelled_count != 0u;
    failed |= state_probe.begin_count != 1u || state_probe.cancel_count != 1u ||
        state_probe.commit_count != 0u ||
        state_probe.owner != CORE_MACHINE_TRANSACTION_OWNER_DMA ||
        state_probe.kind != CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE;

    core_machine_port_initialize(&port);
    failed |= core_machine_memory_initialize_for(&memory, 2u * 1024u * 1024u,
        LIB_NULL) != LIB_STATUS_OK;
    core_machine_dma_initialize(&latch, &primary, &secondary, &port, 2u);
    core_machine_dma_reset(&latch, &primary, &secondary);
    failed |= core_machine_dma_bind_channel(&latch, &primary, &secondary, 2u,
        &dma_provider, &source, &binding) != LIB_STATUS_OK;
    transaction_dma_program_channel2(&port);
    core_machine_dma_request_assert(&primary, &secondary, &binding);
    core_machine_transaction_initialize(&transaction);
    lib_memory_set(&state_probe, 0, sizeof(state_probe));
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
        (lib_uptr)&data, 1u) != LIB_STATUS_OK || data != 0xa5u;

    core_machine_memory_finalize(&memory);
    core_machine_port_finalize(&port);
    core_machine_destroy(machine);
    if (failed != 0) return 1;
    printf("M5:T354:S2:TRANSACTION:OK\n");
    printf("M5:T409:S1:CPU-MEMORY-PROVENANCE:OK\n");
    printf("M5:T410:S1:CPU-EXTERNAL-CYCLE:OK\n");
    printf("M5:T411:S1:CPU-PREFETCH-WINDOW:OK\n");
    return 0;
}
