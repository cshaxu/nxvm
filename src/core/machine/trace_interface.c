#include "core/machine/machine.h"

#include <string.h>

static uint32_t core_machine_trace_linear_pc(const core_machine *machine)
{
    return machine->cpu.state.cs_base + machine->cpu.state.eip;
}

static void core_machine_trace_flush(core_machine *machine)
{
    core_machine_trace_state *trace = &machine->trace;
    size_t index = 0u;

    if (trace->provider.callback == NULL || trace->flushing) {
        return;
    }

    trace->flushing = 1;
    while (index < trace->count) {
        trace->provider.callback(trace->provider.context, &trace->events[index]);
        ++index;
    }
    trace->count = 0u;
    trace->flushing = 0;
}

void core_machine_trace_initialize(core_machine *machine)
{
    if (machine != NULL) {
        memset(&machine->trace, 0, sizeof(machine->trace));
    }
}

void core_machine_trace_finalize(core_machine *machine)
{
    if (machine != NULL) {
        memset(&machine->trace, 0, sizeof(machine->trace));
    }
}

nxvm_core_status core_machine_set_trace_provider(
    core_machine *machine,
    const core_machine_trace_provider *provider)
{
    if (machine == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }

    if (machine->lifecycle == CORE_MACHINE_RUNNING) {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }

    memset(&machine->trace.provider, 0, sizeof(machine->trace.provider));
    if (provider != NULL) {
        if (provider->callback == NULL) {
            return NXVM_CORE_STATUS_INVALID_ARGUMENT;
        }
        machine->trace.provider = *provider;
    }
    machine->trace.count = 0u;
    machine->trace.flushing = 0;
    return NXVM_CORE_STATUS_OK;
}

void core_machine_trace_record(
    core_machine *machine,
    core_machine_trace_event_type type,
    uint32_t address,
    uint32_t value,
    uint32_t detail)
{
    core_machine_trace_state *trace;
    core_machine_trace_event *event;

    if (machine == NULL) {
        return;
    }

    trace = &machine->trace;
    if (trace->provider.callback == NULL) {
        return;
    }

    if (trace->count == CORE_MACHINE_TRACE_CAPACITY) {
        if (trace->flushing) {
            return;
        }
        core_machine_trace_flush(machine);
    }

    event = &trace->events[trace->count++];
    event->type = type;
    event->sequence = trace->next_sequence++;
    event->linear_pc = core_machine_trace_linear_pc(machine);
    event->address = address;
    event->value = value;
    event->detail = detail;
    core_machine_trace_flush(machine);
}
