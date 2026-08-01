#include "vm/product/session_debugger.h"

static nxvm_core_status nxvm_product_nxvm_debugger_require_boundary(
    const nxvm_product_nxvm_debugger *debugger)
{
    if (debugger == NULL || debugger->machine == NULL) return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    return debugger->command_boundary_open ? NXVM_CORE_STATUS_OK : NXVM_CORE_STATUS_INVALID_STATE;
}

nxvm_core_status nxvm_product_nxvm_debugger_initialize(
    nxvm_product_nxvm_debugger *debugger, nxvm_core_machine *machine)
{
    if (debugger == NULL || machine == NULL) return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    debugger->machine = machine;
    debugger->command_boundary_open = 0;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_product_nxvm_debugger_open_command_boundary(
    nxvm_product_nxvm_debugger *debugger)
{
    nxvm_core_status status = nxvm_product_nxvm_debugger_require_boundary(debugger);
    if (status == NXVM_CORE_STATUS_OK) return NXVM_CORE_STATUS_INVALID_STATE;
    if (debugger == NULL || debugger->machine == NULL) return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    debugger->command_boundary_open = 1;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_product_nxvm_debugger_read_cpu(
    const nxvm_product_nxvm_debugger *debugger, nxvm_core_cpu_state *out_state)
{
    nxvm_core_status status = nxvm_product_nxvm_debugger_require_boundary(debugger);
    return status == NXVM_CORE_STATUS_OK ? nxvm_core_debug_read_cpu(debugger->machine, out_state) : status;
}

nxvm_core_status nxvm_product_nxvm_debugger_read_memory(
    const nxvm_product_nxvm_debugger *debugger, uint32_t physical,
    void *out_data, size_t size)
{
    nxvm_core_status status = nxvm_product_nxvm_debugger_require_boundary(debugger);
    return status == NXVM_CORE_STATUS_OK ?
        nxvm_core_debug_read_memory(debugger->machine, physical, out_data, size) : status;
}

nxvm_core_status nxvm_product_nxvm_debugger_step(
    nxvm_product_nxvm_debugger *debugger, nxvm_core_run_result *out_result)
{
    nxvm_core_status status = nxvm_product_nxvm_debugger_require_boundary(debugger);
    return status == NXVM_CORE_STATUS_OK ? nxvm_core_debug_step(debugger->machine, out_result) : status;
}

nxvm_core_status nxvm_product_nxvm_debugger_continue(
    nxvm_product_nxvm_debugger *debugger, nxvm_core_run_budget budget,
    nxvm_core_run_result *out_result)
{
    nxvm_core_status status = nxvm_product_nxvm_debugger_require_boundary(debugger);
    return status == NXVM_CORE_STATUS_OK ?
        nxvm_core_debug_continue(debugger->machine, budget, out_result) : status;
}

void nxvm_product_nxvm_debugger_close_command_boundary(
    nxvm_product_nxvm_debugger *debugger)
{
    if (debugger != NULL) debugger->command_boundary_open = 0;
}
