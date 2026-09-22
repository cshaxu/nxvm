#include "core/machine/debug.h"

C_VOID vm_machine_debug_initialize(t_debug *debug)
{
    if (debug != STD_NULL) STD_MEMSET(debug, 0, sizeof(*debug));
}

C_VOID vm_machine_debug_reset(t_debug *debug)
{
    if (debug == STD_NULL) return;
    STD_MEMSET(&debug->plan, 0, sizeof(debug->plan));
    STD_MEMSET(&debug->observation, 0, sizeof(debug->observation));
    debug->observation_valid = TYPE_FALSE;
}

C_VOID vm_machine_debug_refresh(t_debug *debug,
    const core_machine_debug_instruction_observation *observation)
{
    if (debug == STD_NULL || observation == STD_NULL) return;
    debug->observation = *observation;
    debug->observation_valid = TYPE_TRUE;
}

C_VOID vm_machine_debug_finalize(t_debug *debug)
{
    (C_VOID)debug;
}

type_status vm_machine_debug_set_execution_plan(t_debug *debug,
    const x86_debug_request *request)
{
    if (debug == STD_NULL || request == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (request->execution_kind != X86_DEBUG_EXECUTION_TRACE &&
        request->execution_kind != X86_DEBUG_EXECUTION_BREAK_REAL &&
        request->execution_kind != X86_DEBUG_EXECUTION_BREAK_LINEAR)
        return TYPE_STATUS_INVALID_ARGUMENT;
    if (request->execution_kind == X86_DEBUG_EXECUTION_TRACE &&
        request->instruction_count == 0u) return TYPE_STATUS_INVALID_ARGUMENT;
    debug->plan = (t_debug_execution_plan) {
        .kind = request->execution_kind,
        .remaining = request->instruction_count,
        .breakpoint_linear = request->address
    };
    return TYPE_STATUS_OK;
}

C_VOID vm_machine_debug_clear_execution_plan(t_debug *debug)
{
    if (debug != STD_NULL) STD_MEMSET(&debug->plan, 0, sizeof(debug->plan));
}

type_unsigned_64 vm_machine_debug_limit_instruction_budget(
    const t_debug *debug, type_unsigned_64 requested)
{
    if (debug == STD_NULL || requested == 0u) return requested;
    if (debug->plan.kind == X86_DEBUG_EXECUTION_TRACE &&
        debug->plan.remaining < requested) return debug->plan.remaining;
    if (debug->plan.kind == X86_DEBUG_EXECUTION_BREAK_REAL ||
        debug->plan.kind == X86_DEBUG_EXECUTION_BREAK_LINEAR)
        return 1u;
    return requested;
}

C_INT vm_machine_debug_breakpoint_due(const t_debug *debug)
{
    if (debug == STD_NULL || !debug->observation_valid ||
        (debug->plan.kind != X86_DEBUG_EXECUTION_BREAK_REAL &&
         debug->plan.kind != X86_DEBUG_EXECUTION_BREAK_LINEAR)) return TYPE_FALSE;
    return debug->observation.cs_base + debug->observation.eip ==
        debug->plan.breakpoint_linear;
}

C_VOID vm_machine_debug_complete_breakpoint(t_debug *debug)
{
    if (debug == STD_NULL || (debug->plan.kind !=
        X86_DEBUG_EXECUTION_BREAK_REAL && debug->plan.kind !=
        X86_DEBUG_EXECUTION_BREAK_LINEAR)) return;
    debug->plan.completion_pending = TYPE_TRUE;
    debug->plan.completion_reason = VM_MACHINE_DEBUG_STOP_BREAKPOINT;
    debug->plan.completion_executed = debug->plan.executed;
    debug->plan.kind = X86_DEBUG_EXECUTION_NONE;
}

C_VOID vm_machine_debug_complete_watchpoint(t_debug *debug)
{
    type_unsigned_64 executed;

    if (debug == STD_NULL || !debug->observation_valid ||
        !debug->observation.watch_hit) return;
    executed = debug->plan.executed;
    debug->plan = (t_debug_execution_plan) {
        .completion_pending = TYPE_TRUE,
        .completion_reason = VM_MACHINE_DEBUG_STOP_WATCHPOINT,
        .completion_executed = executed
    };
}

C_VOID vm_machine_debug_complete_run(t_debug *debug, type_unsigned_64 executed)
{
    if (debug == STD_NULL || executed == 0u) return;
    if (debug->plan.kind == X86_DEBUG_EXECUTION_BREAK_REAL ||
        debug->plan.kind == X86_DEBUG_EXECUTION_BREAK_LINEAR) {
        debug->plan.executed += executed;
        return;
    }
    if (debug->plan.kind != X86_DEBUG_EXECUTION_TRACE) return;
    debug->plan.executed += executed;
    if (executed < debug->plan.remaining) {
        debug->plan.remaining -= executed;
        return;
    }
    debug->plan.completion_pending = TYPE_TRUE;
    debug->plan.completion_reason = VM_MACHINE_DEBUG_STOP_TRACE;
    debug->plan.completion_executed = debug->plan.executed;
    debug->plan.kind = X86_DEBUG_EXECUTION_NONE;
}

C_INT vm_machine_debug_completion_pending(const t_debug *debug,
    vm_machine_debug_stop_reason *out_reason)
{
    if (debug == STD_NULL || out_reason == STD_NULL ||
        !debug->plan.completion_pending) return TYPE_FALSE;
    *out_reason = debug->plan.completion_reason;
    return TYPE_TRUE;
}

C_INT vm_machine_debug_take_completion(t_debug *debug,
    vm_machine_debug_stop_reason *out_reason, type_unsigned_64 *out_executed)
{
    if (debug == STD_NULL || out_reason == STD_NULL || out_executed == STD_NULL ||
        !debug->plan.completion_pending) return TYPE_FALSE;
    *out_reason = debug->plan.completion_reason;
    *out_executed = debug->plan.completion_executed;
    debug->plan.completion_pending = TYPE_FALSE;
    debug->plan.completion_reason = VM_MACHINE_DEBUG_STOP_NONE;
    debug->plan.completion_executed = 0u;
    return TYPE_TRUE;
}
