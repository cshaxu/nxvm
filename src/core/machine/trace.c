#include "machine/core/machine_impl.h"

#include <string.h>

static uint32_t nxvm_core_trace_linear_pc(const nxvm_core_machine *machine)
{
    return machine->cpu.state.cs_base + machine->cpu.state.eip;
}

static void nxvm_core_trace_flush(nxvm_core_machine *machine)
{
    nxvm_core_trace_state *trace = &machine->trace;
    size_t index = 0u;

    if (trace->sink.callback == NULL || trace->flushing) {
        return;
    }

    trace->flushing = 1;
    while (index < trace->count) {
        trace->sink.callback(trace->sink.context, &trace->events[index]);
        ++index;
    }
    trace->count = 0u;
    trace->flushing = 0;
}

void nxvm_core_trace_initialize(nxvm_core_machine *machine)
{
    if (machine != NULL) {
        memset(&machine->trace, 0, sizeof(machine->trace));
    }
}

void nxvm_core_trace_finalize(nxvm_core_machine *machine)
{
    if (machine != NULL) {
        memset(&machine->trace, 0, sizeof(machine->trace));
    }
}

nxvm_core_status nxvm_core_machine_set_trace_sink(
    nxvm_core_machine *machine,
    const nxvm_core_trace_sink *sink)
{
    if (machine == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }

    if (machine->lifecycle == NXVM_CORE_MACHINE_RUNNING) {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }

    memset(&machine->trace.sink, 0, sizeof(machine->trace.sink));
    if (sink != NULL) {
        if (sink->callback == NULL) {
            return NXVM_CORE_STATUS_INVALID_ARGUMENT;
        }
        machine->trace.sink = *sink;
    }
    machine->trace.count = 0u;
    machine->trace.flushing = 0;
    return NXVM_CORE_STATUS_OK;
}

void nxvm_core_trace_record(
    nxvm_core_machine *machine,
    nxvm_core_trace_event_type type,
    uint32_t address,
    uint32_t value,
    uint32_t detail)
{
    nxvm_core_trace_state *trace;
    nxvm_core_trace_event *event;

    if (machine == NULL) {
        return;
    }

    trace = &machine->trace;
    if (trace->sink.callback == NULL) {
        return;
    }

    if (trace->count == NXVM_CORE_TRACE_CAPACITY) {
        if (trace->flushing) {
            return;
        }
        nxvm_core_trace_flush(machine);
    }

    event = &trace->events[trace->count++];
    event->type = type;
    event->sequence = trace->next_sequence++;
    event->linear_pc = nxvm_core_trace_linear_pc(machine);
    event->address = address;
    event->value = value;
    event->detail = detail;
    nxvm_core_trace_flush(machine);
}
