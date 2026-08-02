#include "core/machine/debug_interface.h"

static nxvm_core_status core_machine_debug_require_paused(
    const core_machine *machine)
{
    core_machine_lifecycle lifecycle;
    nxvm_core_status status;

    status = core_machine_get_lifecycle(machine, &lifecycle);
    if (status != NXVM_CORE_STATUS_OK) {
        return status;
    }
    return lifecycle == CORE_MACHINE_PAUSED ? NXVM_CORE_STATUS_OK :
                                                   NXVM_CORE_STATUS_INVALID_STATE;
}

nxvm_core_status core_machine_debug_read_cpu(
    const core_machine *machine,
    core_machine_cpu_state *out_state)
{
    nxvm_core_status status = core_machine_debug_require_paused(machine);

    return status == NXVM_CORE_STATUS_OK ?
               core_machine_get_cpu_state(machine, out_state) : status;
}

nxvm_core_status core_machine_debug_read_memory(
    const core_machine *machine,
    uint32_t physical,
    void *out_data,
    size_t size)
{
    nxvm_core_status status = core_machine_debug_require_paused(machine);

    return status == NXVM_CORE_STATUS_OK ?
               core_machine_memory_read(machine, physical, out_data, size) :
               status;
}

nxvm_core_status core_machine_debug_step(
    core_machine *machine,
    core_machine_run_result *out_result)
{
    const core_machine_run_budget budget = { 1u, 0u };
    nxvm_core_status status = core_machine_debug_require_paused(machine);

    return status == NXVM_CORE_STATUS_OK ?
               core_machine_run(machine, budget, out_result) : status;
}

nxvm_core_status core_machine_debug_continue(
    core_machine *machine,
    core_machine_run_budget budget,
    core_machine_run_result *out_result)
{
    nxvm_core_status status = core_machine_debug_require_paused(machine);

    return status == NXVM_CORE_STATUS_OK ?
               core_machine_run(machine, budget, out_result) : status;
}
