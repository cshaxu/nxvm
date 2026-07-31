#include "core/machine.h"
#include "core/machine_impl.h"

#include <stdlib.h>

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

    if (config->abi_version != NXVM_CORE_ABI_VERSION ||
        !nxvm_core_profile_is_supported(config->profile)) {
        return NXVM_CORE_STATUS_UNSUPPORTED;
    }

    machine = (nxvm_core_machine *)calloc(1u, sizeof(*machine));
    if (machine == NULL) {
        return NXVM_CORE_STATUS_NO_MEMORY;
    }

    machine->config = *config;
    machine->state = NXVM_CORE_MACHINE_NEW;

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

    if (nxvm_core_cpu_reset(machine) != NXVM_CORE_STATUS_OK ||
        nxvm_core_memory_reset(machine) != NXVM_CORE_STATUS_OK) {
        return NXVM_CORE_STATUS_FAULT;
    }

    machine->state = NXVM_CORE_MACHINE_RESET;
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
    result->linear_pc = 0u;
    result->detail = 0u;

    if (machine->state == NXVM_CORE_MACHINE_STOP_REQUESTED) {
        result->reason = NXVM_CORE_STOP_REQUESTED;
        return NXVM_CORE_STATUS_OK;
    }

    if (machine->state != NXVM_CORE_MACHINE_RESET) {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }

    (void)budget;
    result->reason = NXVM_CORE_STOP_BUDGET;
    return NXVM_CORE_STATUS_UNSUPPORTED;
}

nxvm_core_status nxvm_core_machine_request_stop(nxvm_core_machine *machine)
{
    if (machine == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }

    machine->state = NXVM_CORE_MACHINE_STOP_REQUESTED;
    return NXVM_CORE_STATUS_OK;
}

void nxvm_core_machine_destroy(nxvm_core_machine *machine)
{
    nxvm_core_port_finalize(machine);
    nxvm_core_memory_finalize(machine);
    free(machine);
}
