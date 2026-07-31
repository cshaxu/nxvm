#include "core/debug.h"

static nxvm_core_status nxvm_core_debug_require_paused(
    const nxvm_core_machine *machine)
{
    nxvm_core_machine_lifecycle lifecycle;
    nxvm_core_status status;

    status = nxvm_core_machine_get_lifecycle(machine, &lifecycle);
    if (status != NXVM_CORE_STATUS_OK) {
        return status;
    }
    return lifecycle == NXVM_CORE_MACHINE_PAUSED ? NXVM_CORE_STATUS_OK :
                                                   NXVM_CORE_STATUS_INVALID_STATE;
}

nxvm_core_status nxvm_core_debug_read_cpu(
    const nxvm_core_machine *machine,
    nxvm_core_cpu_state *out_state)
{
    nxvm_core_status status = nxvm_core_debug_require_paused(machine);

    return status == NXVM_CORE_STATUS_OK ?
               nxvm_core_machine_get_cpu_state(machine, out_state) : status;
}

nxvm_core_status nxvm_core_debug_read_memory(
    const nxvm_core_machine *machine,
    uint32_t physical,
    void *out_data,
    size_t size)
{
    nxvm_core_status status = nxvm_core_debug_require_paused(machine);

    return status == NXVM_CORE_STATUS_OK ?
               nxvm_core_machine_memory_read(machine, physical, out_data, size) :
               status;
}

nxvm_core_status nxvm_core_debug_step(
    nxvm_core_machine *machine,
    nxvm_core_run_result *out_result)
{
    const nxvm_core_run_budget budget = { 1u, 0u };
    nxvm_core_status status = nxvm_core_debug_require_paused(machine);

    return status == NXVM_CORE_STATUS_OK ?
               nxvm_core_machine_run(machine, budget, out_result) : status;
}

nxvm_core_status nxvm_core_debug_continue(
    nxvm_core_machine *machine,
    nxvm_core_run_budget budget,
    nxvm_core_run_result *out_result)
{
    nxvm_core_status status = nxvm_core_debug_require_paused(machine);

    return status == NXVM_CORE_STATUS_OK ?
               nxvm_core_machine_run(machine, budget, out_result) : status;
}
