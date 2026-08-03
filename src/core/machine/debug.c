#include "type.h"

#include "core/machine/debug_interface.h"
#include "core/machine/machine.h"

static ntvdm64_status core_machine_debug_require_boundary(
    const core_machine *machine)
{
    core_machine_lifecycle lifecycle;
    ntvdm64_status status;

    status = core_machine_get_lifecycle(machine, &lifecycle);
    if (status != NTVDM64_STATUS_OK) {
        return status;
    }
    return lifecycle == CORE_MACHINE_PAUSED || lifecycle == CORE_MACHINE_STOPPED ||
           lifecycle == CORE_MACHINE_FAULTED ? NTVDM64_STATUS_OK :
                                                NTVDM64_STATUS_INVALID_STATE;
}

t_cpu *core_machine_debug_cpu_borrow(core_machine *machine)
{
    return core_machine_debug_require_boundary(machine) == NTVDM64_STATUS_OK ?
        &machine->executor_cpu : STD_NULL;
}

t_cpuins *core_machine_debug_cpu_instructions_borrow(core_machine *machine)
{
    return core_machine_debug_require_boundary(machine) == NTVDM64_STATUS_OK ?
        &machine->executor_cpu_instructions : STD_NULL;
}

core_machine_cpu_execution_context *core_machine_debug_cpu_execution_borrow(
    core_machine *machine)
{
    return core_machine_debug_require_boundary(machine) == NTVDM64_STATUS_OK ?
        &machine->executor_cpu_execution : STD_NULL;
}

t_ram *core_machine_debug_memory_borrow(core_machine *machine)
{
    return core_machine_debug_require_boundary(machine) == NTVDM64_STATUS_OK ?
        &machine->executor_memory : STD_NULL;
}

t_port *core_machine_debug_port_borrow(core_machine *machine)
{
    return core_machine_debug_require_boundary(machine) == NTVDM64_STATUS_OK ?
        &machine->executor_port : STD_NULL;
}

ntvdm64_status core_machine_debug_read_cpu(
    const core_machine *machine,
    core_machine_cpu_state *out_state)
{
    ntvdm64_status status = core_machine_debug_require_boundary(machine);

    return status == NTVDM64_STATUS_OK ?
               core_machine_get_cpu_state(machine, out_state) : status;
}

ntvdm64_status core_machine_debug_read_memory(
    const core_machine *machine,
    uint32_t physical,
    C_VOID *out_data,
    STD_SIZE_T size)
{
    ntvdm64_status status = core_machine_debug_require_boundary(machine);

    return status == NTVDM64_STATUS_OK ?
               core_machine_memory_read(machine, physical, out_data, size) :
               status;
}

ntvdm64_status core_machine_debug_step(
    core_machine *machine,
    core_machine_run_result *out_result)
{
    const core_machine_run_budget budget = { 1u, 0u };
    ntvdm64_status status = core_machine_debug_require_boundary(machine);

    return status == NTVDM64_STATUS_OK ?
               core_machine_run(machine, budget, out_result) : status;
}

ntvdm64_status core_machine_debug_continue(
    core_machine *machine,
    core_machine_run_budget budget,
    core_machine_run_result *out_result)
{
    ntvdm64_status status = core_machine_debug_require_boundary(machine);

    return status == NTVDM64_STATUS_OK ?
               core_machine_run(machine, budget, out_result) : status;
}
