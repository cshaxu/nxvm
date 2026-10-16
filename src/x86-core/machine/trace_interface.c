#include "lib/types/types_interface.h"

#define X86_CORE_MACHINE_TRACE_IMPLEMENTATION 1
#include "x86-core/machine/machine.h"



static void x86_core_machine_trace_flush(x86_core_machine *machine)
{
    x86_core_machine_trace_state *trace = &machine->trace;
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

void x86_core_machine_trace_initialize(x86_core_machine *machine)
{
    if (machine != LIB_NULL) {
        lib_memory_set(&machine->trace, 0, sizeof(machine->trace));
    }
}

void x86_core_machine_trace_finalize(x86_core_machine *machine)
{
    if (machine != LIB_NULL) {
        lib_memory_set(&machine->trace, 0, sizeof(machine->trace));
    }
}

lib_status x86_core_machine_set_trace_provider(
    x86_core_machine *machine,
    const x86_core_machine_trace_provider *provider)
{
    if (machine == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }

    if (machine->lifecycle == X86_CORE_MACHINE_RUNNING) {
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

void x86_core_machine_trace_record(
    x86_core_machine *machine,
    lib_u32 type,
    lib_u32 address,
    lib_u32 value,
    lib_u32 detail)
{
    x86_core_machine_trace_state *trace;
    x86_core_machine_trace_event *event;

    if (machine == LIB_NULL) {
        return;
    }

    trace = &machine->trace;
    if (trace->provider.callback == LIB_NULL) {
        return;
    }

    if (trace->count == X86_CORE_MACHINE_TRACE_CAPACITY) {
        if (trace->flushing) {
            return;
        }
        x86_core_machine_trace_flush(machine);
    }

    event = &trace->events[trace->count++];
    event->type = type;
    event->sequence = trace->next_sequence++;
    event->elapsed_ticks = machine->elapsed_ticks;
    event->timeline_ticks = machine->timeline.now;
    event->linear_pc = x86_core_cpu_linear_pc(machine->executor_cpu_execution);
    event->address = address;
    event->value = value;
    event->detail = detail;
    x86_core_machine_trace_flush(machine);
}
