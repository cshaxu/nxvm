#include "core/machine_impl.h"

nxvm_core_status nxvm_core_cpu_reset(nxvm_core_machine *machine)
{
    nxvm_core_cpu_state *state;

    if (machine == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }

    state = &machine->cpu.state;
    state->cs = 0xf000u;
    state->cs_base = 0xffff0000u;
    state->eip = 0x0000fff0u;
    state->eflags = 0x00000002u;
    state->halted = 0u;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_core_machine_get_cpu_state(
    const nxvm_core_machine *machine,
    nxvm_core_cpu_state *out_state)
{
    if (machine == NULL || out_state == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }

    *out_state = machine->cpu.state;
    return NXVM_CORE_STATUS_OK;
}
