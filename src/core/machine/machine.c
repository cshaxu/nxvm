#include "type.h"

#include "core/machine/machine.h"



static uint32_t core_machine_linear_pc(const core_machine *machine)
{
    if (machine->executor_enabled) {
        return machine->executor_cpu.data.cs.base + machine->executor_cpu.data.eip;
    }
    return machine->test_cpu.state.cs_base + machine->test_cpu.state.eip;
}

static C_INT core_machine_profile_is_supported(core_machine_profile profile)
{
    return profile == CORE_MACHINE_PROFILE_CUSTOM ||
           profile == CORE_MACHINE_PROFILE_TEST_MINIMAL;
}

ntvdm64_status core_machine_enable_executor(core_machine *machine)
{
    if (machine == STD_NULL) return NTVDM64_STATUS_INVALID_ARGUMENT;
    if (machine->config.profile != CORE_MACHINE_PROFILE_CUSTOM) {
        return NTVDM64_STATUS_UNSUPPORTED;
    }
    if (machine->executor_enabled) return NTVDM64_STATUS_OK;
    if (machine->lifecycle == CORE_MACHINE_RUNNING) {
        return NTVDM64_STATUS_INVALID_STATE;
    }
    core_machine_cpu_execution_context_initialize(&machine->executor_cpu_execution,
        &machine->executor_cpu, &machine->executor_cpu_instructions,
        &machine->executor_memory, &machine->executor_port);
    machine->executor_enabled = 1;
    machine->lifecycle = CORE_MACHINE_PAUSED;
    return NTVDM64_STATUS_OK;
}

t_cpu *core_machine_executor_cpu_borrow(core_machine *machine)
{ return machine != STD_NULL && machine->executor_enabled ? &machine->executor_cpu : STD_NULL; }

t_cpuins *core_machine_executor_cpu_instructions_borrow(core_machine *machine)
{ return machine != STD_NULL && machine->executor_enabled ? &machine->executor_cpu_instructions : STD_NULL; }

core_machine_cpu_execution_context *core_machine_executor_cpu_execution_borrow(
    core_machine *machine)
{ return machine != STD_NULL && machine->executor_enabled ? &machine->executor_cpu_execution : STD_NULL; }

t_ram *core_machine_executor_memory_borrow(core_machine *machine)
{ return machine != STD_NULL && machine->executor_enabled ? &machine->executor_memory : STD_NULL; }

t_port *core_machine_executor_port_borrow(core_machine *machine)
{ return machine != STD_NULL && machine->executor_enabled ? &machine->executor_port : STD_NULL; }

ntvdm64_status core_machine_enable_shared_devices(core_machine *machine)
{
    if (machine == STD_NULL || !machine->executor_enabled) {
        return NTVDM64_STATUS_INVALID_STATE;
    }
    machine->shared_devices_enabled = 1;
    return NTVDM64_STATUS_OK;
}

t_pic *core_machine_shared_pic_master_borrow(core_machine *machine)
{ return machine != STD_NULL && machine->shared_devices_enabled ? &machine->shared_pic_master : STD_NULL; }
t_pic *core_machine_shared_pic_slave_borrow(core_machine *machine)
{ return machine != STD_NULL && machine->shared_devices_enabled ? &machine->shared_pic_slave : STD_NULL; }
t_pit *core_machine_shared_pit_borrow(core_machine *machine)
{ return machine != STD_NULL && machine->shared_devices_enabled ? &machine->shared_pit : STD_NULL; }
t_latch *core_machine_shared_dma_latch_borrow(core_machine *machine)
{ return machine != STD_NULL && machine->shared_devices_enabled ? &machine->shared_dma_latch : STD_NULL; }
t_dma *core_machine_shared_dma_primary_borrow(core_machine *machine)
{ return machine != STD_NULL && machine->shared_devices_enabled ? &machine->shared_dma_primary : STD_NULL; }
t_dma *core_machine_shared_dma_secondary_borrow(core_machine *machine)
{ return machine != STD_NULL && machine->shared_devices_enabled ? &machine->shared_dma_secondary : STD_NULL; }
t_kbc *core_machine_shared_kbc_borrow(core_machine *machine)
{ return machine != STD_NULL && machine->shared_devices_enabled ? &machine->shared_kbc : STD_NULL; }
t_vadp *core_machine_shared_vadp_borrow(core_machine *machine)
{ return machine != STD_NULL && machine->shared_devices_enabled ? &machine->shared_vadp : STD_NULL; }

ntvdm64_status core_machine_bind_execution_provider(core_machine *machine,
    const core_machine_execution_provider *provider, C_VOID *context)
{
    if (machine == STD_NULL || machine->execution_provider_frozen ||
        machine->lifecycle == CORE_MACHINE_RUNNING) {
        return NTVDM64_STATUS_INVALID_STATE;
    }
    machine->execution_provider = provider;
    machine->execution_provider_context = context;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status core_machine_freeze_execution_providers(core_machine *machine)
{
    if (machine == STD_NULL || machine->lifecycle == CORE_MACHINE_RUNNING) {
        return NTVDM64_STATUS_INVALID_STATE;
    }
    machine->execution_provider_frozen = 1;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status core_machine_cpu_reset(core_machine *machine)
{
    core_machine_cpu_state *state;

    if (machine == STD_NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }

    state = &machine->test_cpu.state;
    state->cs = 0xf000u;
    state->cs_base = 0xffff0000u;
    state->eip = 0x0000fff0u;
    state->eflags = 0x00000002u;
    state->halted = 0u;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status core_machine_get_cpu_state(
    const core_machine *machine,
    core_machine_cpu_state *out_state)
{
    if (machine == STD_NULL || out_state == STD_NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }

    if (machine->executor_enabled) {
        out_state->cs = machine->executor_cpu.data.cs.selector;
        out_state->cs_base = machine->executor_cpu.data.cs.base;
        out_state->eip = machine->executor_cpu.data.eip;
        out_state->eflags = machine->executor_cpu.data.eflags;
        out_state->halted = machine->executor_cpu.data.flagHalt;
    } else {
        *out_state = machine->test_cpu.state;
    }
    return NTVDM64_STATUS_OK;
}

ntvdm64_status core_machine_create(
    const core_machine_config *config,
    core_machine **out_machine)
{
    core_machine *machine;
    ntvdm64_status status;

    if (config == STD_NULL || out_machine == STD_NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }

    *out_machine = STD_NULL;

    if (!core_machine_profile_is_supported(config->profile)) {
        return NTVDM64_STATUS_UNSUPPORTED;
    }

    machine = (core_machine *)STD_CALLOC(1u, sizeof(*machine));
    if (machine == STD_NULL) {
        return NTVDM64_STATUS_NO_MEMORY;
    }

    machine->config = *config;
    machine->lifecycle = CORE_MACHINE_INITIALIZED;
    STD_ATOMIC_INIT(&machine->stop_requested, 0);
    core_machine_trace_initialize(machine);

    if (machine->config.profile == CORE_MACHINE_PROFILE_TEST_MINIMAL) {
        status = core_machine_instance_memory_initialize(machine);
        if (status != NTVDM64_STATUS_OK) {
            core_machine_destroy(machine);
            return status;
        }
        status = core_machine_bus_initialize(machine);
        if (status != NTVDM64_STATUS_OK) {
            core_machine_destroy(machine);
            return status;
        }
    } else {
        status = core_machine_enable_executor(machine);
        if (status != NTVDM64_STATUS_OK) {
            core_machine_destroy(machine);
            return status;
        }
    }

    *out_machine = machine;

    return NTVDM64_STATUS_OK;
}

ntvdm64_status core_machine_reset(core_machine *machine)
{
    if (machine == STD_NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }

    if (machine->lifecycle == CORE_MACHINE_RUNNING) {
        return NTVDM64_STATUS_INVALID_STATE;
    }

    if (machine->executor_enabled) {
        core_machine_cpu_state_reset(&machine->executor_cpu_execution);
        if (machine->shared_devices_enabled) {
            core_machine_kbc_reset(&machine->shared_kbc);
            core_machine_dma_reset(&machine->shared_dma_latch,
                &machine->shared_dma_primary, &machine->shared_dma_secondary);
            core_machine_pic_reset(&machine->shared_pic_master,
                &machine->shared_pic_slave);
            core_machine_pit_reset(&machine->shared_pit);
            core_machine_port_reset(&machine->executor_port);
            core_machine_vadp_reset(&machine->shared_vadp);
            core_machine_memory_reset(&machine->executor_memory);
        }
    } else if (core_machine_cpu_reset(machine) != NTVDM64_STATUS_OK ||
               core_machine_instance_memory_reset(machine) !=
                   NTVDM64_STATUS_OK) {
        return NTVDM64_STATUS_FAULT;
    }

    STD_ATOMIC_STORE(&machine->stop_requested, 0);
    machine->fault_detail = 0u;
    if (machine->execution_provider != STD_NULL &&
        machine->execution_provider->reset != STD_NULL) {
        machine->execution_provider->reset(machine->execution_provider_context);
    }
    machine->lifecycle = CORE_MACHINE_PAUSED;
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_RESET, 0u, 0u, 0u);
    return NTVDM64_STATUS_OK;
}

ntvdm64_status core_machine_get_lifecycle(
    const core_machine *machine,
    core_machine_lifecycle *out_lifecycle)
{
    if (machine == STD_NULL || out_lifecycle == STD_NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }

    *out_lifecycle = machine->lifecycle;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status core_machine_run(
    core_machine *machine,
    core_machine_run_budget budget,
    core_machine_run_result *result)
{
    if (machine == STD_NULL || result == STD_NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }

    result->reason = CORE_MACHINE_STOP_NONE;
    result->executed = 0u;
    result->linear_pc = core_machine_linear_pc(machine);
    result->detail = 0u;

    if (machine->lifecycle == CORE_MACHINE_FAULTED) {
        result->reason = CORE_MACHINE_STOP_FAULT;
        result->detail = machine->fault_detail;
        return NTVDM64_STATUS_FAULT;
    }

    if (machine->lifecycle != CORE_MACHINE_PAUSED) {
        return NTVDM64_STATUS_INVALID_STATE;
    }

    if (budget.instructions == 0u && budget.ticks == 0u) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }

    if (STD_ATOMIC_LOAD(&machine->stop_requested)) {
        result->reason = CORE_MACHINE_STOP_REQUESTED;
        machine->lifecycle = CORE_MACHINE_STOPPED;
        core_machine_trace_record(machine, CORE_MACHINE_TRACE_STOP, 0u, 0u,
                               (uint32_t)result->reason);
        return NTVDM64_STATUS_OK;
    }

    if (!machine->executor_enabled) {
        return NTVDM64_STATUS_UNSUPPORTED;
    }

    machine->lifecycle = CORE_MACHINE_RUNNING;
    {
        uint64_t limit = budget.instructions == 0u ? 1u : budget.instructions;

        while (result->executed < limit) {
            if (STD_ATOMIC_LOAD(&machine->stop_requested) ||
                core_machine_cpu_execution_consume_stop_request(
                    &machine->executor_cpu_execution)) {
                machine->lifecycle = CORE_MACHINE_STOPPED;
                result->reason = CORE_MACHINE_STOP_REQUESTED;
                result->linear_pc = core_machine_linear_pc(machine);
                core_machine_trace_record(machine, CORE_MACHINE_TRACE_STOP, 0u,
                    0u, (uint32_t)result->reason);
                return NTVDM64_STATUS_OK;
            }
            if (core_machine_cpu_execution_consume_reset_request(
                    &machine->executor_cpu_execution)) {
                machine->lifecycle = CORE_MACHINE_PAUSED;
                result->reason = CORE_MACHINE_STOP_RESET_REQUESTED;
                result->linear_pc = core_machine_linear_pc(machine);
                return NTVDM64_STATUS_OK;
            }
            if (machine->executor_cpu.data.flagHalt) {
                machine->lifecycle = CORE_MACHINE_PAUSED;
                result->reason = CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
                result->linear_pc = core_machine_linear_pc(machine);
                return NTVDM64_STATUS_OK;
            }
            if (machine->execution_provider != STD_NULL &&
                machine->execution_provider->refresh != STD_NULL) {
                machine->execution_provider->refresh(
                    machine->execution_provider_context);
            }
            if (machine->shared_devices_enabled) {
                core_machine_kbc_refresh(&machine->shared_kbc);
                core_machine_vadp_refresh(&machine->shared_vadp);
                core_machine_dma_refresh(&machine->shared_dma_latch,
                    &machine->shared_dma_primary, &machine->shared_dma_secondary,
                    &machine->executor_memory);
                core_machine_pic_refresh(&machine->shared_pic_master,
                    &machine->shared_pic_slave);
                core_machine_pit_refresh(&machine->shared_pit);
            }
            core_machine_cpu_execution_refresh(&machine->executor_cpu_execution);
            ++result->executed;
        }
        machine->lifecycle = CORE_MACHINE_PAUSED;
        result->reason = CORE_MACHINE_STOP_BUDGET;
        result->linear_pc = core_machine_linear_pc(machine);
        core_machine_trace_record(machine, CORE_MACHINE_TRACE_RUN_BOUNDARY,
            result->linear_pc, (uint32_t)result->executed,
            (uint32_t)result->reason);
        return NTVDM64_STATUS_OK;
    }
}

ntvdm64_status core_machine_request_stop(core_machine *machine)
{
    if (machine == STD_NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }

    STD_ATOMIC_STORE(&machine->stop_requested, 1);
    return NTVDM64_STATUS_OK;
}

ntvdm64_status core_machine_report_fault(
    core_machine *machine,
    uint32_t detail)
{
    if (machine == STD_NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }

    if (machine->lifecycle != CORE_MACHINE_PAUSED &&
        machine->lifecycle != CORE_MACHINE_RUNNING) {
        return NTVDM64_STATUS_INVALID_STATE;
    }

    machine->fault_detail = detail;
    machine->lifecycle = CORE_MACHINE_FAULTED;
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_FAULT, 0u, 0u, detail);
    return NTVDM64_STATUS_OK;
}

C_VOID core_machine_destroy(core_machine *machine)
{
    if (machine != STD_NULL) {
        core_machine_cpu_execution_finalize(&machine->executor_cpu_execution);
    }
    core_machine_trace_finalize(machine);
    core_machine_bus_finalize(machine);
    core_machine_instance_memory_finalize(machine);
    STD_FREE(machine);
}
