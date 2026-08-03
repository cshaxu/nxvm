#include "core/machine/machine.h"
#include "core/machine/machine.h"

#include <stdlib.h>

static uint32_t core_machine_linear_pc(const core_machine *machine)
{
    if (machine->legacy_executor_enabled) {
        return machine->legacy_cpu.data.cs.base + machine->legacy_cpu.data.eip;
    }
    return machine->cpu.state.cs_base + machine->cpu.state.eip;
}

static int core_machine_profile_is_supported(core_machine_profile profile)
{
    return profile == CORE_MACHINE_PROFILE_CUSTOM ||
           profile == CORE_MACHINE_PROFILE_TEST_MINIMAL;
}

nxvm_core_status core_machine_enable_legacy_executor(core_machine *machine)
{
    if (machine == NULL) return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    if (machine->legacy_executor_enabled) return NXVM_CORE_STATUS_OK;
    if (machine->lifecycle == CORE_MACHINE_RUNNING) {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }
    core_machine_cpu_execution_context_initialize(&machine->legacy_cpu_execution,
        &machine->legacy_cpu, &machine->legacy_cpu_instructions,
        &machine->legacy_memory, &machine->legacy_port);
    machine->legacy_executor_enabled = 1;
    machine->lifecycle = CORE_MACHINE_PAUSED;
    return NXVM_CORE_STATUS_OK;
}

t_cpu *core_machine_legacy_cpu_borrow(core_machine *machine)
{ return machine != NULL && machine->legacy_executor_enabled ? &machine->legacy_cpu : NULL; }

t_cpuins *core_machine_legacy_cpu_instructions_borrow(core_machine *machine)
{ return machine != NULL && machine->legacy_executor_enabled ? &machine->legacy_cpu_instructions : NULL; }

core_machine_cpu_execution_context *core_machine_legacy_cpu_execution_borrow(
    core_machine *machine)
{ return machine != NULL && machine->legacy_executor_enabled ? &machine->legacy_cpu_execution : NULL; }

t_ram *core_machine_legacy_memory_borrow(core_machine *machine)
{ return machine != NULL && machine->legacy_executor_enabled ? &machine->legacy_memory : NULL; }

t_port *core_machine_legacy_port_borrow(core_machine *machine)
{ return machine != NULL && machine->legacy_executor_enabled ? &machine->legacy_port : NULL; }

nxvm_core_status core_machine_cpu_reset(core_machine *machine)
{
    core_machine_cpu_state *state;

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

nxvm_core_status core_machine_get_cpu_state(
    const core_machine *machine,
    core_machine_cpu_state *out_state)
{
    if (machine == NULL || out_state == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }

    if (machine->legacy_executor_enabled) {
        out_state->cs = machine->legacy_cpu.data.cs.selector;
        out_state->cs_base = machine->legacy_cpu.data.cs.base;
        out_state->eip = machine->legacy_cpu.data.eip;
        out_state->eflags = machine->legacy_cpu.data.eflags;
        out_state->halted = machine->legacy_cpu.data.flagHalt;
    } else {
        *out_state = machine->cpu.state;
    }
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status core_machine_create(
    const core_machine_config *config,
    core_machine **out_machine)
{
    core_machine *machine;
    nxvm_core_status status;

    if (config == NULL || out_machine == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }

    *out_machine = NULL;

    if (!core_machine_profile_is_supported(config->profile)) {
        return NXVM_CORE_STATUS_UNSUPPORTED;
    }

    machine = (core_machine *)calloc(1u, sizeof(*machine));
    if (machine == NULL) {
        return NXVM_CORE_STATUS_NO_MEMORY;
    }

    machine->config = *config;
    machine->lifecycle = CORE_MACHINE_INITIALIZED;
    atomic_init(&machine->stop_requested, 0);
    core_machine_trace_initialize(machine);

    status = core_machine_instance_memory_initialize(machine);
    if (status != NXVM_CORE_STATUS_OK) {
        core_machine_destroy(machine);
        return status;
    }

    status = core_machine_bus_initialize(machine);
    if (status != NXVM_CORE_STATUS_OK) {
        core_machine_destroy(machine);
        return status;
    }

    *out_machine = machine;

    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status core_machine_reset(core_machine *machine)
{
    if (machine == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }

    if (machine->lifecycle == CORE_MACHINE_RUNNING) {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }

    if (machine->legacy_executor_enabled) {
        core_machine_cpu_state_reset(&machine->legacy_cpu_execution);
    } else if (core_machine_cpu_reset(machine) != NXVM_CORE_STATUS_OK ||
               core_machine_instance_memory_reset(machine) !=
                   NXVM_CORE_STATUS_OK) {
        return NXVM_CORE_STATUS_FAULT;
    }

    atomic_store(&machine->stop_requested, 0);
    machine->fault_detail = 0u;
    machine->lifecycle = CORE_MACHINE_PAUSED;
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_RESET, 0u, 0u, 0u);
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status core_machine_get_lifecycle(
    const core_machine *machine,
    core_machine_lifecycle *out_lifecycle)
{
    if (machine == NULL || out_lifecycle == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }

    *out_lifecycle = machine->lifecycle;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status core_machine_run(
    core_machine *machine,
    core_machine_run_budget budget,
    core_machine_run_result *result)
{
    if (machine == NULL || result == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }

    result->reason = CORE_MACHINE_STOP_NONE;
    result->executed = 0u;
    result->linear_pc = core_machine_linear_pc(machine);
    result->detail = 0u;

    if (machine->lifecycle == CORE_MACHINE_FAULTED) {
        result->reason = CORE_MACHINE_STOP_FAULT;
        result->detail = machine->fault_detail;
        return NXVM_CORE_STATUS_FAULT;
    }

    if (machine->lifecycle != CORE_MACHINE_PAUSED) {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }

    if (budget.instructions == 0u && budget.ticks == 0u) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }

    if (atomic_load(&machine->stop_requested)) {
        result->reason = CORE_MACHINE_STOP_REQUESTED;
        machine->lifecycle = CORE_MACHINE_STOPPED;
        core_machine_trace_record(machine, CORE_MACHINE_TRACE_STOP, 0u, 0u,
                               (uint32_t)result->reason);
        return NXVM_CORE_STATUS_OK;
    }

    machine->lifecycle = CORE_MACHINE_RUNNING;
    if (machine->legacy_executor_enabled) {
        uint64_t limit = budget.instructions == 0u ? 1u : budget.instructions;

        while (result->executed < limit) {
            if (atomic_load(&machine->stop_requested) ||
                core_machine_cpu_execution_consume_stop_request(
                    &machine->legacy_cpu_execution)) {
                machine->lifecycle = CORE_MACHINE_STOPPED;
                result->reason = CORE_MACHINE_STOP_REQUESTED;
                result->linear_pc = core_machine_linear_pc(machine);
                core_machine_trace_record(machine, CORE_MACHINE_TRACE_STOP, 0u,
                    0u, (uint32_t)result->reason);
                return NXVM_CORE_STATUS_OK;
            }
            if (core_machine_cpu_execution_consume_reset_request(
                    &machine->legacy_cpu_execution)) {
                machine->lifecycle = CORE_MACHINE_PAUSED;
                result->reason = CORE_MACHINE_STOP_RESET_REQUESTED;
                result->linear_pc = core_machine_linear_pc(machine);
                return NXVM_CORE_STATUS_OK;
            }
            if (machine->legacy_cpu.data.flagHalt) {
                machine->lifecycle = CORE_MACHINE_PAUSED;
                result->reason = CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
                result->linear_pc = core_machine_linear_pc(machine);
                return NXVM_CORE_STATUS_OK;
            }
            core_machine_cpu_execution_refresh(&machine->legacy_cpu_execution);
            ++result->executed;
        }
        machine->lifecycle = CORE_MACHINE_PAUSED;
        result->reason = CORE_MACHINE_STOP_BUDGET;
        result->linear_pc = core_machine_linear_pc(machine);
        core_machine_trace_record(machine, CORE_MACHINE_TRACE_RUN_BOUNDARY,
            result->linear_pc, (uint32_t)result->executed,
            (uint32_t)result->reason);
        return NXVM_CORE_STATUS_OK;
    }
    machine->lifecycle = CORE_MACHINE_PAUSED;
    result->reason = CORE_MACHINE_STOP_BUDGET;
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_RUN_BOUNDARY, 0u, 0u,
                           (uint32_t)result->reason);
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status core_machine_request_stop(core_machine *machine)
{
    if (machine == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }

    atomic_store(&machine->stop_requested, 1);
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status core_machine_report_fault(
    core_machine *machine,
    uint32_t detail)
{
    if (machine == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }

    if (machine->lifecycle != CORE_MACHINE_PAUSED &&
        machine->lifecycle != CORE_MACHINE_RUNNING) {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }

    machine->fault_detail = detail;
    machine->lifecycle = CORE_MACHINE_FAULTED;
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_FAULT, 0u, 0u, detail);
    return NXVM_CORE_STATUS_OK;
}

void core_machine_destroy(core_machine *machine)
{
    core_machine_trace_finalize(machine);
    core_machine_bus_finalize(machine);
    core_machine_instance_memory_finalize(machine);
    free(machine);
}
