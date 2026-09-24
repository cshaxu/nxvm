#include "lib/types/types_interface.h"

#define CORE_MACHINE_TRACE_IMPLEMENTATION 1
#include "app-nxvm/devices/machine.h"



static lib_u32 core_machine_trace_linear_pc(const core_machine *machine)
{
    return machine->executor_cpu.data.cs.base + machine->executor_cpu.data.eip;
}

static void core_machine_trace_flush(core_machine *machine)
{
    core_machine_trace_state *trace = &machine->trace;
    lib_size index = 0u;

    if (trace->provider.callback == LIB_NULL || trace->flushing) {
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
    if (machine != LIB_NULL) {
        lib_memory_set(&machine->trace, 0, sizeof(machine->trace));
    }
}

void core_machine_trace_finalize(core_machine *machine)
{
    if (machine != LIB_NULL) {
        lib_memory_set(&machine->trace, 0, sizeof(machine->trace));
    }
}

lib_status core_machine_set_trace_provider(
    core_machine *machine,
    const core_machine_trace_provider *provider)
{
    if (machine == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }

    if (machine->lifecycle == CORE_MACHINE_RUNNING) {
        return LIB_STATUS_INVALID_STATE;
    }

    lib_memory_set(&machine->trace.provider, 0, sizeof(machine->trace.provider));
    if (provider != LIB_NULL) {
        if (provider->callback == LIB_NULL) {
            return LIB_STATUS_INVALID_ARGUMENT;
        }
        machine->trace.provider = *provider;
    }
    machine->trace.count = 0u;
    machine->trace.flushing = 0;
    return LIB_STATUS_OK;
}

void core_machine_trace_record(
    core_machine *machine,
    core_machine_trace_event_type type,
    lib_u32 address,
    lib_u32 value,
    lib_u32 detail)
{
    core_machine_trace_state *trace;
    core_machine_trace_event *event;

    if (machine == LIB_NULL) {
        return;
    }

    trace = &machine->trace;
    if (trace->provider.callback == LIB_NULL) {
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
    event->elapsed_ticks = machine->elapsed_ticks;
    event->timeline_ticks = machine->timeline.now;
    event->linear_pc = core_machine_trace_linear_pc(machine);
    event->address = address;
    event->value = value;
    event->detail = detail;
    core_machine_trace_flush(machine);
}
