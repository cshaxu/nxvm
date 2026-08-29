#include "type.h"

#include "core/machine/dma.h"
#include "core/machine/machine.h"
#include "core/machine/machine_interface.h"
#include "core/machine/transaction.h"
#include "support/core_machine_cpu_fixture.h"

#define REFRESH_PROBE_EVENT_CAPACITY 1024u

typedef struct refresh_probe {
    core_machine_trace_event events[REFRESH_PROBE_EVENT_CAPACITY];
    type_unsigned_32 count;
} refresh_probe;

typedef struct refresh_dma_source {
    type_unsigned_8 value;
} refresh_dma_source;

static C_VOID refresh_trace(C_VOID *opaque, const core_machine_trace_event *event)
{
    refresh_probe *probe = (refresh_probe *)opaque;

    if (probe != STD_NULL && event != STD_NULL &&
        probe->count < REFRESH_PROBE_EVENT_CAPACITY) {
        probe->events[probe->count++] = *event;
    }
}

static C_VOID refresh_dma_read(C_VOID *opaque, t_latch *latch)
{
    refresh_dma_source *source = (refresh_dma_source *)opaque;

    if (source != STD_NULL && latch != STD_NULL) {
        latch->data.byte = source->value;
    }
}

static C_VOID refresh_program_dma_channel2(t_port *port)
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

static C_INT refresh_find_transaction_after(const refresh_probe *probe,
    core_machine_trace_event_type phase, core_machine_transaction_owner owner,
    core_machine_transaction_kind kind, type_unsigned_32 start,
    type_unsigned_32 *out_index)
{
    type_unsigned_32 index;

    if (probe == STD_NULL || out_index == STD_NULL) return 0;
    for (index = start; index < probe->count; ++index) {
        const core_machine_trace_event *event = &probe->events[index];

        if (event->type == phase && (event->detail & 0xffu) == owner &&
            ((event->detail >> 8u) & 0xffu) == kind) {
            *out_index = index;
            return 1;
        }
    }
    return 0;
}

static C_INT refresh_find_hold_after(const refresh_probe *probe,
    core_machine_trace_event_type phase, core_machine_transaction_owner owner,
    type_unsigned_32 start, type_unsigned_32 *out_index)
{
    type_unsigned_32 index;

    if (probe == STD_NULL || out_index == STD_NULL) return 0;
    for (index = start; index < probe->count; ++index) {
        const core_machine_trace_event *event = &probe->events[index];

        if (event->type == phase && (event->detail & 0xffu) == owner) {
            *out_index = index;
            return 1;
        }
    }
    return 0;
}

static C_INT refresh_has_cpu_transaction_between(const refresh_probe *probe,
    type_unsigned_32 first, type_unsigned_32 last)
{
    type_unsigned_32 index;

    if (probe == STD_NULL || first >= last) return 0;
    for (index = first + 1u; index < last; ++index) {
        const core_machine_trace_event *event = &probe->events[index];

        if ((event->type == CORE_MACHINE_TRACE_TRANSACTION_BEGIN ||
             event->type == CORE_MACHINE_TRACE_TRANSACTION_COMMIT) &&
            (event->detail & 0xffu) == CORE_MACHINE_TRANSACTION_OWNER_CPU) {
            return 1;
        }
    }
    return 0;
}

static C_INT refresh_non_d4_contract(C_VOID)
{
    core_machine_config config = {0};
    core_machine_trace_provider trace;
    refresh_probe probe = {{{0}}, 0u};
    core_machine *machine = STD_NULL;
    type_unsigned_32 index = 0u;
    C_INT failed = 0;

    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80386;
    trace.callback = refresh_trace;
    trace.context = &probe;
    failed |= core_machine_create(&config, &machine) != TYPE_STATUS_OK;
    failed |= test_core_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
        0x000ffff0u, 16u) != TYPE_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK;
    failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= core_machine_set_trace_provider(machine, &trace) != TYPE_STATUS_OK;
    failed |= core_machine_advance_time(machine, 20u) != TYPE_STATUS_OK;
    failed |= refresh_find_transaction_after(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_BEGIN,
        CORE_MACHINE_TRANSACTION_OWNER_REFRESH,
        CORE_MACHINE_TRANSACTION_REFRESH_MEMORY_CYCLE, 0u, &index);
    core_machine_destroy(machine);
    return !failed;
}
C_INT main(C_VOID)
{
    static const core_machine_dma_channel_provider dma_provider = {
        refresh_dma_read, STD_NULL, STD_NULL
    };
    core_machine_config config = {0};
    core_machine_d4_platform_config d4 = {CORE_MACHINE_PC_AT_PORT_B, 0u};
    core_machine_trace_provider trace;
    core_machine_dma_request_binding binding = {0};
    refresh_probe probe = {{{0}}, 0u};
    refresh_dma_source source = {0xa5u};
    core_machine *machine = STD_NULL;
    type_unsigned_8 byte = 0u;
    type_unsigned_32 start;
    type_unsigned_32 refresh_request = 0u;
    type_unsigned_32 refresh_acknowledge = 0u;
    type_unsigned_32 refresh_begin = 0u;
    type_unsigned_32 refresh_commit = 0u;
    type_unsigned_32 refresh_release = 0u;
    type_unsigned_32 dma_begin = 0u;
    C_INT failed = 0;

    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80386;
    config.transaction_contract.dma_cycle_wait_quanta = 1u;
    config.transaction_contract.dma_cycle_bus_ready_gate_enabled = TYPE_TRUE;
    config.auxiliary_pit_present = TYPE_TRUE;
    config.auxiliary_pit_base_port = 0x0048u;
    trace.callback = refresh_trace;
    trace.context = &probe;
    failed |= core_machine_create(&config, &machine) != TYPE_STATUS_OK;
    failed |= core_machine_configure_d4_platform(machine, &d4) != TYPE_STATUS_OK;
    failed |= test_core_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
        0x000ffff0u, 16u) != TYPE_STATUS_OK;
    failed |= core_machine_dma_bind_channel(&machine->shared_dma_latch,
        &machine->shared_dma_primary, &machine->shared_dma_secondary, 2u,
        &dma_provider, &source, &binding) != TYPE_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK;
    failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= core_machine_set_trace_provider(machine, &trace) != TYPE_STATUS_OK;
    refresh_program_dma_channel2(&machine->executor_port);
    core_machine_dma_request_assert(&machine->shared_dma_primary,
        &machine->shared_dma_secondary, &binding);
    failed |= core_machine_set_dma_bus_ready(machine, 0) != TYPE_STATUS_OK;
    core_machine_port_write(&machine->executor_port, 0x0064u, 0xd1u);
    core_machine_port_write(&machine->executor_port, 0x0060u, 0x01u);
    failed |= machine->executor_memory.data.flagA20;
    failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
    refresh_program_dma_channel2(&machine->executor_port);
    core_machine_dma_request_assert(&machine->shared_dma_primary,
        &machine->shared_dma_secondary, &binding);
    failed |= core_machine_set_dma_bus_ready(machine, 0) != TYPE_STATUS_OK;
    start = probe.count;
    failed |= core_machine_advance_time(machine, 19u) != TYPE_STATUS_OK;
    failed |= !machine->d4_refresh_hold_pending ||
        machine->d4_refresh_address != 0u ||
        machine->dma_cycle_wait_remaining != 0u;
    failed |= core_machine_set_dma_bus_ready(machine, 1) != TYPE_STATUS_OK;
    /* BUSRDY releases the DMA cycle gate; normal 8237A timing then needs
     * channel selection plus S1..S4, with this contract's one wait quantum
     * per controller step. */
    failed |= core_machine_advance_time(machine, 11u) != TYPE_STATUS_OK;
    failed |= machine->d4_refresh_hold_pending ||
        machine->d4_refresh_address != 1u ||
        machine->dma_cycle_wait_remaining != 0u;
    failed |= core_machine_memory_read(machine, 0x11234u, &byte, 1u) !=
        TYPE_STATUS_OK || byte != 0xa5u;
    failed |= !refresh_find_hold_after(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_HOLD_REQUEST,
        CORE_MACHINE_TRANSACTION_OWNER_REFRESH, start, &refresh_request);
    failed |= !refresh_find_hold_after(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_HOLD_ACKNOWLEDGE,
        CORE_MACHINE_TRANSACTION_OWNER_REFRESH, start, &refresh_acknowledge);
    failed |= !refresh_find_transaction_after(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_BEGIN,
        CORE_MACHINE_TRANSACTION_OWNER_REFRESH,
        CORE_MACHINE_TRANSACTION_REFRESH_MEMORY_CYCLE, start, &refresh_begin);
    failed |= !refresh_find_transaction_after(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_COMMIT,
        CORE_MACHINE_TRANSACTION_OWNER_REFRESH,
        CORE_MACHINE_TRANSACTION_REFRESH_MEMORY_CYCLE, start, &refresh_commit);
    failed |= !refresh_find_hold_after(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_HOLD_RELEASE,
        CORE_MACHINE_TRANSACTION_OWNER_REFRESH, start, &refresh_release);
    failed |= !refresh_find_transaction_after(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_BEGIN,
        CORE_MACHINE_TRANSACTION_OWNER_DMA,
        CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, start, &dma_begin);
    failed |= refresh_request >= refresh_acknowledge ||
        refresh_acknowledge >= refresh_begin || refresh_begin >= refresh_commit ||
        refresh_commit >= refresh_release || refresh_release >= dma_begin ||
        refresh_has_cpu_transaction_between(&probe, refresh_release, dma_begin);
    failed |= core_machine_reset(machine) != TYPE_STATUS_OK ||
        machine->d4_refresh_hold_pending || machine->d4_refresh_pulse_active ||
        machine->d4_refresh_address != 0u;

    core_machine_destroy(machine);
    failed |= !refresh_non_d4_contract();
    if (failed) return 1;
    STD_PRINTF("M5:T419:S4:D4-REFRESH-HOLD:OK\n");
    return 0;
}
