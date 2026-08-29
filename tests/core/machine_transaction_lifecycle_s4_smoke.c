#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/machine_interface.h"
#include "core/machine/transaction.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct lifecycle_probe {
    core_machine_trace_event events[32];
    type_unsigned_32 count;
} lifecycle_probe;

static C_VOID lifecycle_trace(C_VOID *opaque,
    const core_machine_trace_event *event)
{
    lifecycle_probe *probe = (lifecycle_probe *)opaque;

    if (probe != STD_NULL && probe->count < 32u) {
        probe->events[probe->count++] = *event;
    }
}

static C_INT lifecycle_find_transaction(const lifecycle_probe *probe,
    core_machine_trace_event_type type, core_machine_transaction_owner owner,
    core_machine_transaction_kind kind, type_unsigned_32 *out_index)
{
    type_unsigned_32 index;

    if (probe == STD_NULL || out_index == STD_NULL) return 0;
    for (index = 0u; index < probe->count; ++index) {
        const core_machine_trace_event *event = &probe->events[index];

        if (event->type == type && (event->detail & 0xffu) == owner &&
            ((event->detail >> 8u) & 0xffu) == kind) {
            *out_index = index;
            return 1;
        }
    }
    return 0;
}

static C_INT lifecycle_find_event(const lifecycle_probe *probe,
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

C_INT main(C_VOID)
{
    const type_unsigned_8 nop = 0x90u;
    core_machine *machine = STD_NULL;
    core_machine_config config = {0};
    core_machine_trace_provider trace;
    core_machine_run_budget budget = {1u, 0u};
    core_machine_run_result result;
    lifecycle_probe probe = {{{0}}, 0u};
    type_unsigned_32 begin = 0u;
    type_unsigned_32 cancel = 0u;
    type_unsigned_32 reset = 0u;
    type_unsigned_32 cpu_begin = 0u;
    type_unsigned_32 cpu_commit = 0u;
    type_unsigned_32 cpu_retire = 0u;
    C_INT failed = 0;

    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80286;
    trace.callback = lifecycle_trace;
    trace.context = &probe;
    failed |= core_machine_create(&config, &machine) != TYPE_STATUS_OK;
    failed |= test_core_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
        0x000ffff0u, 16u) != TYPE_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK;
    failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= core_machine_set_trace_provider(machine, &trace) != TYPE_STATUS_OK;
    failed |= core_machine_transaction_begin(&machine->transaction,
        CORE_MACHINE_TRANSACTION_OWNER_DMA,
        CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, 0x11234u, 1u, 2u) !=
        TYPE_STATUS_OK;
    failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_NONE ||
        machine->transaction.committed_count != 0u ||
        machine->transaction.cancelled_count != 0u;
    failed |= !lifecycle_find_transaction(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_BEGIN,
        CORE_MACHINE_TRANSACTION_OWNER_DMA,
        CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, &begin);
    failed |= !lifecycle_find_transaction(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_CANCEL,
        CORE_MACHINE_TRANSACTION_OWNER_DMA,
        CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, &cancel);
    failed |= !lifecycle_find_event(&probe, CORE_MACHINE_TRACE_RESET, &reset);
    failed |= begin >= cancel || cancel >= reset;

    failed |= core_machine_memory_write(machine, 0xfffffff0u, &nop, 1u) !=
        TYPE_STATUS_OK;
    failed |= core_machine_run(machine, budget, &result) != TYPE_STATUS_OK;
    failed |= result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
        result.elapsed_ticks != 3u;
    failed |= !lifecycle_find_transaction(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_BEGIN,
        CORE_MACHINE_TRANSACTION_OWNER_CPU,
        CORE_MACHINE_TRANSACTION_CPU_MEMORY_READ, &cpu_begin);
    failed |= !lifecycle_find_transaction(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_COMMIT,
        CORE_MACHINE_TRANSACTION_OWNER_CPU,
        CORE_MACHINE_TRANSACTION_CPU_MEMORY_READ, &cpu_commit);
    failed |= !lifecycle_find_event(&probe, CORE_MACHINE_TRACE_CPU_RETIRE,
        &cpu_retire);
    failed |= reset >= cpu_begin || cpu_begin >= cpu_commit ||
        cpu_commit >= cpu_retire;

    core_machine_destroy(machine);
    if (failed) return 1;
    STD_PRINTF("M5:T354:S4:TRANSACTION-LIFECYCLE:OK\n");
    return 0;
}
