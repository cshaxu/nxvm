#include "type.h"

#include "core/machine/debug_interface.h"

static ntvdm64_status core_machine_debug_require_paused(
    const core_machine *machine)
{
    core_machine_lifecycle lifecycle;
    ntvdm64_status status;

    status = core_machine_get_lifecycle(machine, &lifecycle);
    if (status != NTVDM64_STATUS_OK) {
        return status;
    }
    return lifecycle == CORE_MACHINE_PAUSED ? NTVDM64_STATUS_OK :
                                                   NTVDM64_STATUS_INVALID_STATE;
}

ntvdm64_status core_machine_debug_read_cpu(
    const core_machine *machine,
    core_machine_cpu_state *out_state)
{
    ntvdm64_status status = core_machine_debug_require_paused(machine);

    return status == NTVDM64_STATUS_OK ?
               core_machine_get_cpu_state(machine, out_state) : status;
}

ntvdm64_status core_machine_debug_read_memory(
    const core_machine *machine,
    uint32_t physical,
    C_VOID *out_data,
    size_t size)
{
    ntvdm64_status status = core_machine_debug_require_paused(machine);

    return status == NTVDM64_STATUS_OK ?
               core_machine_memory_read(machine, physical, out_data, size) :
               status;
}

ntvdm64_status core_machine_debug_step(
    core_machine *machine,
    core_machine_run_result *out_result)
{
    const core_machine_run_budget budget = { 1u, 0u };
    ntvdm64_status status = core_machine_debug_require_paused(machine);

    return status == NTVDM64_STATUS_OK ?
               core_machine_run(machine, budget, out_result) : status;
}

ntvdm64_status core_machine_debug_continue(
    core_machine *machine,
    core_machine_run_budget budget,
    core_machine_run_result *out_result)
{
    ntvdm64_status status = core_machine_debug_require_paused(machine);

    return status == NTVDM64_STATUS_OK ?
               core_machine_run(machine, budget, out_result) : status;
}
