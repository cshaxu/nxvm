#include "type.h"

#include "vm/product/session_debugger.h"

static ntvdm64_status vm_product_debugger_require_boundary(
    const vm_product_debugger *debugger)
{
    if (debugger == NULL || debugger->machine == NULL) return NTVDM64_STATUS_INVALID_ARGUMENT;
    return debugger->command_boundary_open ? NTVDM64_STATUS_OK : NTVDM64_STATUS_INVALID_STATE;
}

ntvdm64_status vm_product_debugger_initialize(
    vm_product_debugger *debugger, core_machine *machine)
{
    if (debugger == NULL || machine == NULL) return NTVDM64_STATUS_INVALID_ARGUMENT;
    debugger->machine = machine;
    debugger->command_boundary_open = 0;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status vm_product_debugger_open_command_boundary(
    vm_product_debugger *debugger)
{
    ntvdm64_status status = vm_product_debugger_require_boundary(debugger);
    if (status == NTVDM64_STATUS_OK) return NTVDM64_STATUS_INVALID_STATE;
    if (debugger == NULL || debugger->machine == NULL) return NTVDM64_STATUS_INVALID_ARGUMENT;
    debugger->command_boundary_open = 1;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status vm_product_debugger_read_cpu(
    const vm_product_debugger *debugger, core_machine_cpu_state *out_state)
{
    ntvdm64_status status = vm_product_debugger_require_boundary(debugger);
    return status == NTVDM64_STATUS_OK ? core_machine_debug_read_cpu(debugger->machine, out_state) : status;
}

ntvdm64_status vm_product_debugger_read_memory(
    const vm_product_debugger *debugger, uint32_t physical,
    C_VOID *out_data, size_t size)
{
    ntvdm64_status status = vm_product_debugger_require_boundary(debugger);
    return status == NTVDM64_STATUS_OK ?
        core_machine_debug_read_memory(debugger->machine, physical, out_data, size) : status;
}

ntvdm64_status vm_product_debugger_step(
    vm_product_debugger *debugger, core_machine_run_result *out_result)
{
    ntvdm64_status status = vm_product_debugger_require_boundary(debugger);
    return status == NTVDM64_STATUS_OK ? core_machine_debug_step(debugger->machine, out_result) : status;
}

ntvdm64_status vm_product_debugger_continue(
    vm_product_debugger *debugger, core_machine_run_budget budget,
    core_machine_run_result *out_result)
{
    ntvdm64_status status = vm_product_debugger_require_boundary(debugger);
    return status == NTVDM64_STATUS_OK ?
        core_machine_debug_continue(debugger->machine, budget, out_result) : status;
}

C_VOID vm_product_debugger_close_command_boundary(
    vm_product_debugger *debugger)
{
    if (debugger != NULL) debugger->command_boundary_open = 0;
}
