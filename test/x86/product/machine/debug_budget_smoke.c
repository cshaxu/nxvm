#include "lib/types/types_interface.h"
#include "x86/product/machine/debug.h"

static lib_bool trace_budgets(void)
{
    static const lib_u64 counts[] = {1u, 10u, 4096u};
    t_debug debug;
    vm_machine_debug_stop_reason reason;
    lib_u64 executed;

    for (lib_size index = 0u; index < sizeof(counts) / sizeof(counts[0]); ++index) {
        x86_debug_request request = {
            .execution_kind = X86_DEBUG_EXECUTION_TRACE,
            .instruction_count = counts[index]
        };
        vm_machine_debug_initialize(&debug);
        if (vm_machine_debug_set_execution_plan(&debug, &request) != LIB_STATUS_OK ||
            vm_machine_debug_limit_instruction_budget(&debug, 8192u) != counts[index])
            return LIB_FALSE;
        vm_machine_debug_complete_run(&debug, 0u);
        if (vm_machine_debug_completion_pending(&debug, &reason)) return LIB_FALSE;
        if (counts[index] > 1u) {
            vm_machine_debug_complete_run(&debug, counts[index] - 1u);
            if (vm_machine_debug_completion_pending(&debug, &reason) ||
                vm_machine_debug_limit_instruction_budget(&debug, 8192u) != 1u)
                return LIB_FALSE;
        }
        vm_machine_debug_complete_run(&debug, 1u);
        if (!vm_machine_debug_completion_pending(&debug, &reason) ||
            reason != VM_MACHINE_DEBUG_STOP_TRACE ||
            !vm_machine_debug_take_completion(&debug, &reason, &executed) ||
            executed != counts[index] ||
            vm_machine_debug_take_completion(&debug, &reason, &executed))
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool breakpoints(void)
{
    static const x86_debug_execution_plan_kind kinds[] = {
        X86_DEBUG_EXECUTION_BREAK_REAL, X86_DEBUG_EXECUTION_BREAK_LINEAR
    };
    t_debug debug;
    vm_machine_debug_stop_reason reason;
    lib_u64 executed;
    core_machine_debug_instruction_observation observation = {
        .cs_base = 0x1000u, .eip = 0x20u
    };

    for (lib_size index = 0u; index < sizeof(kinds) / sizeof(kinds[0]); ++index) {
        x86_debug_request request = {
            .execution_kind = kinds[index], .address = 0x1020u
        };
        vm_machine_debug_initialize(&debug);
        if (vm_machine_debug_set_execution_plan(&debug, &request) != LIB_STATUS_OK ||
            vm_machine_debug_breakpoint_due(&debug) ||
            vm_machine_debug_limit_instruction_budget(&debug, 256u) != 1u)
            return LIB_FALSE;
        observation.eip = 0x1fu;
        vm_machine_debug_refresh(&debug, &observation);
        if (vm_machine_debug_breakpoint_due(&debug)) return LIB_FALSE;
        vm_machine_debug_complete_run(&debug, 1u);
        observation.eip = 0x20u;
        vm_machine_debug_refresh(&debug, &observation);
        if (!vm_machine_debug_breakpoint_due(&debug)) return LIB_FALSE;
        vm_machine_debug_complete_breakpoint(&debug);
        if (!vm_machine_debug_take_completion(&debug, &reason, &executed) ||
            reason != VM_MACHINE_DEBUG_STOP_BREAKPOINT || executed != 1u)
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

lib_i32 main(void)
{
    t_debug debug;
    vm_machine_debug_stop_reason reason;
    lib_u64 executed;
    x86_debug_request request = {.execution_kind = X86_DEBUG_EXECUTION_TRACE};
    core_machine_debug_instruction_observation observation = {.watch_hit = LIB_TRUE};

    if (!trace_budgets() || !breakpoints()) return 1;
    vm_machine_debug_initialize(&debug);
    if (vm_machine_debug_set_execution_plan(&debug, &request) != LIB_STATUS_INVALID_ARGUMENT)
        return 1;
    request.instruction_count = 10u;
    if (vm_machine_debug_set_execution_plan(&debug, &request) != LIB_STATUS_OK) return 1;
    vm_machine_debug_complete_watchpoint(&debug);
    if (vm_machine_debug_completion_pending(&debug, &reason)) return 1;
    vm_machine_debug_refresh(&debug, &observation);
    vm_machine_debug_complete_watchpoint(&debug);
    if (!vm_machine_debug_take_completion(&debug, &reason, &executed) ||
        reason != VM_MACHINE_DEBUG_STOP_WATCHPOINT || executed != 0u) return 1;
    vm_machine_debug_reset(&debug);
    if (debug.observation_valid || vm_machine_debug_completion_pending(&debug, &reason) ||
        vm_machine_debug_limit_instruction_budget(&debug, 256u) != 256u) return 1;
    return 0;
}
