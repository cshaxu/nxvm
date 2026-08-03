#include "type.h"

#include "core/machine/machine.h"



static uint32_t core_machine_trace_linear_pc(const core_machine *machine)
{
    return machine->cpu.state.cs_base + machine->cpu.state.eip;
}

static C_VOID core_machine_trace_flush(core_machine *machine)
{
    core_machine_trace_state *trace = &machine->trace;
    STD_SIZE_T index = 0u;

    if (trace->provider.callback == STD_NULL || trace->flushing) {
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

C_VOID core_machine_trace_initialize(core_machine *machine)
{
    if (machine != STD_NULL) {
        STD_MEMSET(&machine->trace, 0, sizeof(machine->trace));
    }
}

C_VOID core_machine_trace_finalize(core_machine *machine)
{
    if (machine != STD_NULL) {
        STD_MEMSET(&machine->trace, 0, sizeof(machine->trace));
    }
}

ntvdm64_status core_machine_set_trace_provider(
    core_machine *machine,
    const core_machine_trace_provider *provider)
{
    if (machine == STD_NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }

    if (machine->lifecycle == CORE_MACHINE_RUNNING) {
        return NTVDM64_STATUS_INVALID_STATE;
    }

    STD_MEMSET(&machine->trace.provider, 0, sizeof(machine->trace.provider));
    if (provider != STD_NULL) {
        if (provider->callback == STD_NULL) {
            return NTVDM64_STATUS_INVALID_ARGUMENT;
        }
        machine->trace.provider = *provider;
    }
    machine->trace.count = 0u;
    machine->trace.flushing = 0;
    return NTVDM64_STATUS_OK;
}

C_VOID core_machine_trace_record(
    core_machine *machine,
    core_machine_trace_event_type type,
    uint32_t address,
    uint32_t value,
    uint32_t detail)
{
    core_machine_trace_state *trace;
    core_machine_trace_event *event;

    if (machine == STD_NULL) {
        return;
    }

    trace = &machine->trace;
    if (trace->provider.callback == STD_NULL) {
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
