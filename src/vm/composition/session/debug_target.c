#include "type.h"

#include "vm/composition/session/session.h"

#include "vm/composition/session/debug_target.h"




#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/fault.h"

#include "vm/composition/session/control.h"

#include "core/machine/debug_interface.h"

#include "vm/machine/debug.h"
#include "vm/platform/input_flush.h"

C_VOID vm_session_debug_flush_console_input(C_VOID *context)
{
    (C_VOID)context;
    vm_platform_input_flush_console_input();
}

static C_INT vm_debug_running(C_VOID *context) { return vm_session_control_is_running(&((vm_session *)context)->control); }
static C_VOID vm_debug_resume(C_VOID *context) { (C_VOID)vm_session_resume((vm_session *)context); }
static C_INT vm_debug_paused(C_VOID *context) { return vm_session_control_is_paused(&((vm_session *)context)->control); }
static core_product_debug_pause_reason vm_debug_pause_reason(C_VOID *context)
{
    switch (vm_session_control_get_pause_reason(&((vm_session *)context)->control)) {
    case VM_SESSION_PAUSE_EXPLICIT: return CORE_PRODUCT_DEBUG_PAUSE_EXPLICIT;
    case VM_SESSION_PAUSE_BREAKPOINT: return CORE_PRODUCT_DEBUG_PAUSE_BREAKPOINT;
    case VM_SESSION_PAUSE_TRACE: return CORE_PRODUCT_DEBUG_PAUSE_TRACE;
    case VM_SESSION_PAUSE_STEP: return CORE_PRODUCT_DEBUG_PAUSE_STEP;
    default: return CORE_PRODUCT_DEBUG_PAUSE_NONE;
    }
}
static C_INT vm_debug_request_pause(C_VOID *context, core_product_debug_pause_reason reason)
{
    vm_session_pause_reason mapped = VM_SESSION_PAUSE_EXPLICIT;
    if (reason == CORE_PRODUCT_DEBUG_PAUSE_BREAKPOINT) mapped = VM_SESSION_PAUSE_BREAKPOINT;
    else if (reason == CORE_PRODUCT_DEBUG_PAUSE_TRACE) mapped = VM_SESSION_PAUSE_TRACE;
    else if (reason == CORE_PRODUCT_DEBUG_PAUSE_STEP) mapped = VM_SESSION_PAUSE_STEP;
    vm_session_control_request_pause(&((vm_session *)context)->control, mapped);
    return 0;
}
static C_VOID vm_debug_continue(C_VOID *context)
{ vm_session_control_continue(&((vm_session *)context)->control); }
static C_INT vm_debug_step(C_VOID *context)
{ return vm_session_control_step(&((vm_session *)context)->control) ? 0 : 1; }

static C_INT vm_debug_read_register(C_VOID *context, core_product_debug_register reg,
                                  uint32_t *value)
{
    vm_session *machine =
        (vm_session *)context;
    return machine == STD_NULL || core_machine_debug_read_register(
        machine->core_machine, (core_machine_debug_register)reg, value) !=
        TYPE_STATUS_OK;
}

static C_INT vm_debug_write_register(C_VOID *context, core_product_debug_register reg,
                                   uint32_t value)
{
    vm_session *machine =
        (vm_session *)context;
    return machine == STD_NULL || core_machine_debug_write_register(
        machine->core_machine, (core_machine_debug_register)reg, value) !=
        TYPE_STATUS_OK;
}

static C_INT vm_debug_code_default_size(C_VOID *context)
{
    vm_session *machine = (vm_session *)context;
    C_INT value = 0;
    return machine == STD_NULL || core_machine_debug_get_code_default_size(
        machine->core_machine, &value) != TYPE_STATUS_OK ? 0 : value;
}
static uint32_t vm_debug_code_base(C_VOID *context)
{
    vm_session *machine = (vm_session *)context;
    uint32_t value = 0u;
    return machine == STD_NULL || core_machine_debug_get_code_base(
        machine->core_machine, &value) != TYPE_STATUS_OK ? 0u : value;
}

static C_INT vm_debug_read_linear(C_VOID *context, uint32_t address, C_VOID *out, uint8_t size)
{
    vm_session *machine = (vm_session *)context;
    return machine == STD_NULL || core_machine_debug_read_linear(
        machine->core_machine, address, out, size) != TYPE_STATUS_OK;
}
static C_INT vm_debug_write_linear(C_VOID *context, uint32_t address, const C_VOID *in, uint8_t size)
{
    vm_session *machine = (vm_session *)context;
    return machine == STD_NULL || core_machine_debug_write_linear(
        machine->core_machine, address, in, size) != TYPE_STATUS_OK;
}
static C_INT vm_debug_read_real(C_VOID *context, uint16_t seg, uint16_t off, C_VOID *out, STD_SIZE_T size)
{ vm_session *machine = (vm_session *)context; return machine == STD_NULL || core_machine_debug_read_real(machine->core_machine, seg, off, out, size) != TYPE_STATUS_OK; }
static C_INT vm_debug_write_real(C_VOID *context, uint16_t seg, uint16_t off, const C_VOID *in, STD_SIZE_T size)
{ vm_session *machine = (vm_session *)context; return machine == STD_NULL || core_machine_debug_write_real(machine->core_machine, seg, off, in, size) != TYPE_STATUS_OK; }
static uint32_t vm_debug_read_port(C_VOID *context, uint16_t port)
{ vm_session *machine = (vm_session *)context; uint32_t value = 0u; return machine == STD_NULL || core_machine_debug_read_port(machine->core_machine, port, &value) != TYPE_STATUS_OK ? 0u : value; }
static C_VOID vm_debug_write_port(C_VOID *context, uint16_t port, uint32_t value)
{ vm_session *machine = (vm_session *)context; if (machine != STD_NULL) (C_VOID)core_machine_debug_write_port(machine->core_machine, port, value); }
static C_VOID vm_debug_set_break_real(C_VOID *context, uint16_t seg, uint16_t off)
{ vm_session *machine = (vm_session *)context; if (machine != STD_NULL) vm_machine_debug_set_breakpoint_real(&machine->debug, seg, off); }
static C_VOID vm_debug_set_break_linear(C_VOID *context, uint32_t address)
{ vm_session *machine = (vm_session *)context; if (machine != STD_NULL) vm_machine_debug_set_breakpoint_linear(&machine->debug, address); }
static C_VOID vm_debug_clear_break(C_VOID *context, C_INT linear)
{ vm_session *machine = (vm_session *)context; if (machine == STD_NULL) return; if (linear) vm_machine_debug_clear_breakpoint_linear(&machine->debug); else vm_machine_debug_clear_breakpoint_real(&machine->debug); }
static C_VOID vm_debug_set_trace(C_VOID *context, STD_SIZE_T count)
{ vm_session *machine = (vm_session *)context; if (machine != STD_NULL) vm_machine_debug_set_trace(&machine->debug, count); }
static C_VOID vm_debug_clear_trace(C_VOID *context)
{ vm_session *machine = (vm_session *)context; if (machine != STD_NULL) vm_machine_debug_clear_trace(&machine->debug); }
static STD_SIZE_T vm_debug_break_count(C_VOID *context)
{ vm_session *machine = (vm_session *)context; return machine == STD_NULL ? 0u : vm_machine_debug_get_breakpoint_count(&machine->debug); }
static C_VOID vm_debug_set_watch(C_VOID *context, core_product_debug_watch_kind kind, uint32_t address)
{
    vm_session *machine = (vm_session *)context;
    if (machine == STD_NULL) return;
    (C_VOID)core_machine_debug_set_watchpoint(machine->core_machine,
        (core_machine_debug_watch_kind)kind, address);
}
static C_VOID vm_debug_clear_watch(C_VOID *context, core_product_debug_watch_kind kind)
{
    vm_session *machine = (vm_session *)context;
    if (machine != STD_NULL) (C_VOID)core_machine_debug_clear_watchpoint(
        machine->core_machine, (core_machine_debug_watch_kind)kind);
}
static C_VOID vm_debug_print_registers(C_VOID *context)
{ vm_session *machine = (vm_session *)context; if (machine != STD_NULL) (C_VOID)core_machine_debug_print_registers(machine->core_machine); }
static C_VOID vm_debug_print_segment_registers(C_VOID *context)
{ vm_session *machine = (vm_session *)context; if (machine != STD_NULL) (C_VOID)core_machine_debug_print_segment_registers(machine->core_machine); }
static C_VOID vm_debug_print_control_registers(C_VOID *context)
{ vm_session *machine = (vm_session *)context; if (machine != STD_NULL) (C_VOID)core_machine_debug_print_control_registers(machine->core_machine); }
static C_VOID vm_debug_print_memory(C_VOID *context)
{ vm_session *machine = (vm_session *)context; if (machine != STD_NULL) (C_VOID)core_machine_debug_print_memory_accesses(machine->core_machine); }
static C_VOID vm_debug_print_watchpoints(C_VOID *context)
{ vm_session *machine = (vm_session *)context; if (machine != STD_NULL) (C_VOID)core_machine_debug_print_watchpoints(machine->core_machine); }

static C_INT vm_debug_get_fault_outcome(C_VOID *context,
    core_product_debug_fault_outcome *out_outcome)
{
    vm_session_fault_outcome outcome;

    if (out_outcome == STD_NULL || vm_session_fault_get((vm_session *)context,
            &outcome) != 0) return 1;
    STD_MEMSET(out_outcome, 0, sizeof(*out_outcome));
    if (!outcome.valid) return 0;
    out_outcome->valid = TYPE_TRUE;
    out_outcome->detail = outcome.run.detail;
    out_outcome->linear_pc = outcome.run.linear_pc;
    out_outcome->executed = outcome.run.executed;
    if (outcome.diagnostic.first_fault.valid) {
        out_outcome->diagnostic_valid = TYPE_TRUE;
        out_outcome->exception_mask = outcome.diagnostic.first_fault.exception_mask;
        out_outcome->exception_code = outcome.diagnostic.first_fault.exception_code;
        out_outcome->cs = outcome.diagnostic.first_fault.point.cs;
        out_outcome->eip = outcome.diagnostic.first_fault.point.eip;
    }
    return 0;
}

static const core_product_debug_target vmDebugTargetTemplate = {
    .is_running = vm_debug_running,
    .resume = vm_debug_resume,
    .is_paused = vm_debug_paused,
    .get_pause_reason = vm_debug_pause_reason,
    .request_pause = vm_debug_request_pause,
    .continue_execution = vm_debug_continue,
    .step = vm_debug_step,
    .read_register = vm_debug_read_register,
    .write_register = vm_debug_write_register,
    .get_code_default_size = vm_debug_code_default_size,
    .get_code_base = vm_debug_code_base,
    .read_linear = vm_debug_read_linear,
    .write_linear = vm_debug_write_linear,
    .read_real = vm_debug_read_real,
    .write_real = vm_debug_write_real,
    .read_port = vm_debug_read_port,
    .write_port = vm_debug_write_port,
    .set_break_real = vm_debug_set_break_real,
    .set_break_linear = vm_debug_set_break_linear,
    .clear_break = vm_debug_clear_break,
    .set_trace = vm_debug_set_trace,
    .clear_trace = vm_debug_clear_trace,
    .get_break_count = vm_debug_break_count,
    .set_watch = vm_debug_set_watch,
    .clear_watch = vm_debug_clear_watch,
    .print_registers = vm_debug_print_registers,
    .print_segment_registers = vm_debug_print_segment_registers,
    .print_control_registers = vm_debug_print_control_registers,
    .print_memory = vm_debug_print_memory,
    .print_watchpoints = vm_debug_print_watchpoints,
    .get_fault_outcome = vm_debug_get_fault_outcome,
    .context = STD_NULL
};

const core_product_debug_target *vm_session_debug_target(
    vm_session *machine)
{
    if (machine == STD_NULL) return STD_NULL;
    if (machine->debug_target == STD_NULL) {
        machine->debug_target = (core_product_debug_target *)STD_MALLOC(
            sizeof(*machine->debug_target));
        if (machine->debug_target == STD_NULL) return STD_NULL;
    }
    *machine->debug_target = vmDebugTargetTemplate;
    machine->debug_target->context = machine;
    return machine->debug_target;
}

C_VOID vm_session_debug_target_finalize(vm_session *machine)
{
    if (machine == STD_NULL) return;
    STD_FREE(machine->debug_target);
    machine->debug_target = STD_NULL;
}
