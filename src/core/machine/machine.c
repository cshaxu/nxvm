#include "core/machine/machine.h"
#include "core/machine/machine_impl.h"

#include <stdlib.h>

static uint32_t nxvm_core_machine_linear_pc(const nxvm_core_machine *machine)
{
    return machine->cpu.state.cs_base + machine->cpu.state.eip;
}

static int nxvm_core_profile_is_supported(nxvm_core_profile profile)
{
    return profile == NXVM_CORE_PROFILE_CUSTOM ||
           profile == NXVM_CORE_PROFILE_TEST_MINIMAL;
}

nxvm_core_status nxvm_core_machine_create(
    const nxvm_core_machine_config *config,
    nxvm_core_machine **out_machine)
{
    nxvm_core_machine *machine;
    nxvm_core_status status;

    if (config == NULL || out_machine == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }

    *out_machine = NULL;

    if (!nxvm_core_profile_is_supported(config->profile)) {
        return NXVM_CORE_STATUS_UNSUPPORTED;
    }

    machine = (nxvm_core_machine *)calloc(1u, sizeof(*machine));
    if (machine == NULL) {
        return NXVM_CORE_STATUS_NO_MEMORY;
    }

    machine->config = *config;
    machine->lifecycle = NXVM_CORE_MACHINE_INITIALIZED;
    atomic_init(&machine->stop_requested, 0);
    nxvm_core_trace_initialize(machine);

    status = nxvm_core_memory_initialize(machine);
    if (status != NXVM_CORE_STATUS_OK) {
        nxvm_core_machine_destroy(machine);
        return status;
    }

    status = nxvm_core_port_initialize(machine);
    if (status != NXVM_CORE_STATUS_OK) {
        nxvm_core_machine_destroy(machine);
        return status;
    }

    *out_machine = machine;

    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_core_machine_reset(nxvm_core_machine *machine)
{
    if (machine == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }

    if (machine->lifecycle == NXVM_CORE_MACHINE_RUNNING) {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }

    if (nxvm_core_cpu_reset(machine) != NXVM_CORE_STATUS_OK ||
        nxvm_core_memory_reset(machine) != NXVM_CORE_STATUS_OK) {
        return NXVM_CORE_STATUS_FAULT;
    }

    atomic_store(&machine->stop_requested, 0);
    machine->fault_detail = 0u;
    machine->lifecycle = NXVM_CORE_MACHINE_PAUSED;
    nxvm_core_trace_record(machine, NXVM_CORE_TRACE_RESET, 0u, 0u, 0u);
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_core_machine_get_lifecycle(
    const nxvm_core_machine *machine,
    nxvm_core_machine_lifecycle *out_lifecycle)
{
    if (machine == NULL || out_lifecycle == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }

    *out_lifecycle = machine->lifecycle;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_core_machine_run(
    nxvm_core_machine *machine,
    nxvm_core_run_budget budget,
    nxvm_core_run_result *result)
{
    if (machine == NULL || result == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }

    result->reason = NXVM_CORE_STOP_NONE;
    result->executed = 0u;
    result->linear_pc = nxvm_core_machine_linear_pc(machine);
    result->detail = 0u;

    if (machine->lifecycle == NXVM_CORE_MACHINE_FAULTED) {
        result->reason = NXVM_CORE_STOP_FAULT;
        result->detail = machine->fault_detail;
        return NXVM_CORE_STATUS_FAULT;
    }

    if (machine->lifecycle != NXVM_CORE_MACHINE_PAUSED) {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }

    if (budget.instructions == 0u && budget.ticks == 0u) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }

    if (atomic_load(&machine->stop_requested)) {
        result->reason = NXVM_CORE_STOP_REQUESTED;
        machine->lifecycle = NXVM_CORE_MACHINE_STOPPED;
        nxvm_core_trace_record(machine, NXVM_CORE_TRACE_STOP, 0u, 0u,
                               (uint32_t)result->reason);
        return NXVM_CORE_STATUS_OK;
    }

    machine->lifecycle = NXVM_CORE_MACHINE_RUNNING;
    machine->lifecycle = NXVM_CORE_MACHINE_PAUSED;
    result->reason = NXVM_CORE_STOP_BUDGET;
    nxvm_core_trace_record(machine, NXVM_CORE_TRACE_RUN_BOUNDARY, 0u, 0u,
                           (uint32_t)result->reason);
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_core_machine_request_stop(nxvm_core_machine *machine)
{
    if (machine == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }

    atomic_store(&machine->stop_requested, 1);
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_core_machine_report_fault(
    nxvm_core_machine *machine,
    uint32_t detail)
{
    if (machine == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }

    if (machine->lifecycle != NXVM_CORE_MACHINE_PAUSED &&
        machine->lifecycle != NXVM_CORE_MACHINE_RUNNING) {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }

    machine->fault_detail = detail;
    machine->lifecycle = NXVM_CORE_MACHINE_FAULTED;
    nxvm_core_trace_record(machine, NXVM_CORE_TRACE_FAULT, 0u, 0u, detail);
    return NXVM_CORE_STATUS_OK;
}

void nxvm_core_machine_destroy(nxvm_core_machine *machine)
{
    nxvm_core_trace_finalize(machine);
    nxvm_core_port_finalize(machine);
    nxvm_core_memory_finalize(machine);
    free(machine);
}
