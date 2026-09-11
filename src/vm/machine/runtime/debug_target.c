#include "type.h"

#include "vm/machine/runtime/machine_private.h"

#include "vm/machine/runtime/debug_target.h"




#include "vm/machine/runtime/lifecycle.h"
#include "vm/machine/runtime/fault.h"

#include "vm/machine/runtime/control.h"

#include "core/machine/debug_interface.h"
#include "core/debug/debug.h"

#include "vm/machine/debug.h"

static C_INT vm_debug_running(C_VOID *context) { return vm_machine_control_is_running(&((vm_machine *)context)->control); }
static C_VOID vm_debug_resume(C_VOID *context) { (C_VOID)vm_machine_resume((vm_machine *)context); }
static C_INT vm_debug_wait_for_completion(C_VOID *context)
{
    vm_machine *session = (vm_machine *)context;

    return session == STD_NULL || !vm_machine_control_wait_for_completion(
        &session->control);
}
static C_INT vm_debug_paused(C_VOID *context) { return vm_machine_control_is_paused(&((vm_machine *)context)->control); }
static core_debug_pause_reason vm_debug_pause_reason(C_VOID *context)
{
    switch (vm_machine_control_get_pause_reason(&((vm_machine *)context)->control)) {
    case VM_MACHINE_PAUSE_EXPLICIT: return CORE_DEBUG_PAUSE_EXPLICIT;
    case VM_MACHINE_PAUSE_BREAKPOINT: return CORE_DEBUG_PAUSE_BREAKPOINT;
    case VM_MACHINE_PAUSE_TRACE: return CORE_DEBUG_PAUSE_TRACE;
    case VM_MACHINE_PAUSE_STEP: return CORE_DEBUG_PAUSE_STEP;
    default: return CORE_DEBUG_PAUSE_NONE;
    }
}
static C_INT vm_debug_request_pause(C_VOID *context, core_debug_pause_reason reason)
{
    vm_machine_pause_reason mapped = VM_MACHINE_PAUSE_EXPLICIT;
    if (reason == CORE_DEBUG_PAUSE_BREAKPOINT) mapped = VM_MACHINE_PAUSE_BREAKPOINT;
    else if (reason == CORE_DEBUG_PAUSE_TRACE) mapped = VM_MACHINE_PAUSE_TRACE;
    else if (reason == CORE_DEBUG_PAUSE_STEP) mapped = VM_MACHINE_PAUSE_STEP;
    return vm_machine_request_pause_reason((vm_machine *)context, mapped) == TYPE_STATUS_OK ?
        0 : 1;
}
static C_VOID vm_debug_continue(C_VOID *context)
{ (C_VOID)vm_machine_resume((vm_machine *)context); }
static C_INT vm_debug_step(C_VOID *context)
{ return vm_machine_request_step((vm_machine *)context) == TYPE_STATUS_OK ? 0 : 1; }

static type_status vm_debug_map_register(core_debug_register source,
    core_machine_debug_register *out_target)
{
    if (out_target == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    switch (source) {
    case CORE_DEBUG_EAX: *out_target = CORE_MACHINE_DEBUG_EAX; break;
    case CORE_DEBUG_ECX: *out_target = CORE_MACHINE_DEBUG_ECX; break;
    case CORE_DEBUG_EDX: *out_target = CORE_MACHINE_DEBUG_EDX; break;
    case CORE_DEBUG_EBX: *out_target = CORE_MACHINE_DEBUG_EBX; break;
    case CORE_DEBUG_ESP: *out_target = CORE_MACHINE_DEBUG_ESP; break;
    case CORE_DEBUG_EBP: *out_target = CORE_MACHINE_DEBUG_EBP; break;
    case CORE_DEBUG_ESI: *out_target = CORE_MACHINE_DEBUG_ESI; break;
    case CORE_DEBUG_EDI: *out_target = CORE_MACHINE_DEBUG_EDI; break;
    case CORE_DEBUG_EIP: *out_target = CORE_MACHINE_DEBUG_EIP; break;
    case CORE_DEBUG_EFLAGS: *out_target = CORE_MACHINE_DEBUG_EFLAGS; break;
    case CORE_DEBUG_ES: *out_target = CORE_MACHINE_DEBUG_ES; break;
    case CORE_DEBUG_CS: *out_target = CORE_MACHINE_DEBUG_CS; break;
    case CORE_DEBUG_SS: *out_target = CORE_MACHINE_DEBUG_SS; break;
    case CORE_DEBUG_DS: *out_target = CORE_MACHINE_DEBUG_DS; break;
    case CORE_DEBUG_FS: *out_target = CORE_MACHINE_DEBUG_FS; break;
    case CORE_DEBUG_GS: *out_target = CORE_MACHINE_DEBUG_GS; break;
    case CORE_DEBUG_CR0: *out_target = CORE_MACHINE_DEBUG_CR0; break;
    case CORE_DEBUG_CR1: *out_target = CORE_MACHINE_DEBUG_CR1; break;
    case CORE_DEBUG_CR2: *out_target = CORE_MACHINE_DEBUG_CR2; break;
    case CORE_DEBUG_CR3: *out_target = CORE_MACHINE_DEBUG_CR3; break;
    case CORE_DEBUG_CR4: *out_target = CORE_MACHINE_DEBUG_CR4; break;
    default: return TYPE_STATUS_INVALID_ARGUMENT;
    }
    return TYPE_STATUS_OK;
}

static type_status vm_debug_map_watch(core_debug_watch_kind source,
    core_machine_debug_watch_kind *out_target)
{
    if (out_target == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    switch (source) {
    case CORE_DEBUG_WATCH_READ:
        *out_target = CORE_MACHINE_DEBUG_WATCH_READ;
        break;
    case CORE_DEBUG_WATCH_WRITE:
        *out_target = CORE_MACHINE_DEBUG_WATCH_WRITE;
        break;
    case CORE_DEBUG_WATCH_EXECUTE:
        *out_target = CORE_MACHINE_DEBUG_WATCH_EXECUTE;
        break;
    default: return TYPE_STATUS_INVALID_ARGUMENT;
    }
    return TYPE_STATUS_OK;
}

static C_INT vm_debug_read_register(C_VOID *context, core_debug_register reg,
                                  type_unsigned_32 *value)
{
    vm_machine *machine =
        (vm_machine *)context;
    core_machine_debug_register mapped;

    if (vm_debug_map_register(reg, &mapped) != TYPE_STATUS_OK) return 1;
    return machine == STD_NULL || core_machine_debug_read_register(
        machine->core_machine, mapped, value) !=
        TYPE_STATUS_OK;
}

static C_INT vm_debug_write_register(C_VOID *context, core_debug_register reg,
                                   type_unsigned_32 value)
{
    vm_machine *machine =
        (vm_machine *)context;
    core_machine_debug_register mapped;

    if (vm_debug_map_register(reg, &mapped) != TYPE_STATUS_OK) return 1;
    return machine == STD_NULL || core_machine_debug_write_register(
        machine->core_machine, mapped, value) !=
        TYPE_STATUS_OK;
}

static C_INT vm_debug_code_default_size(C_VOID *context)
{
    vm_machine *machine = (vm_machine *)context;
    C_INT value = 0;
    return machine == STD_NULL || core_machine_debug_get_code_default_size(
        machine->core_machine, &value) != TYPE_STATUS_OK ? 0 : value;
}
static type_unsigned_32 vm_debug_code_base(C_VOID *context)
{
    vm_machine *machine = (vm_machine *)context;
    type_unsigned_32 value = 0u;
    return machine == STD_NULL || core_machine_debug_get_code_base(
        machine->core_machine, &value) != TYPE_STATUS_OK ? 0u : value;
}

static C_INT vm_debug_read_linear(C_VOID *context, type_unsigned_32 address, C_VOID *out, type_unsigned_8 size)
{
    vm_machine *machine = (vm_machine *)context;
    return machine == STD_NULL || core_machine_debug_read_linear(
        machine->core_machine, address, out, size) != TYPE_STATUS_OK;
}
static C_INT vm_debug_write_linear(C_VOID *context, type_unsigned_32 address, const C_VOID *in, type_unsigned_8 size)
{
    vm_machine *machine = (vm_machine *)context;
    return machine == STD_NULL || core_machine_debug_write_linear(
        machine->core_machine, address, in, size) != TYPE_STATUS_OK;
}
static C_INT vm_debug_read_real(C_VOID *context, type_unsigned_16 seg, type_unsigned_16 off, C_VOID *out, STD_SIZE_T size)
{ vm_machine *machine = (vm_machine *)context; return machine == STD_NULL || core_machine_debug_read_real(machine->core_machine, seg, off, out, size) != TYPE_STATUS_OK; }
static C_INT vm_debug_write_real(C_VOID *context, type_unsigned_16 seg, type_unsigned_16 off, const C_VOID *in, STD_SIZE_T size)
{ vm_machine *machine = (vm_machine *)context; return machine == STD_NULL || core_machine_debug_write_real(machine->core_machine, seg, off, in, size) != TYPE_STATUS_OK; }
static type_unsigned_32 vm_debug_read_port(C_VOID *context, type_unsigned_16 port)
{ vm_machine *machine = (vm_machine *)context; type_unsigned_32 value = 0u; return machine == STD_NULL || core_machine_debug_read_port(machine->core_machine, port, &value) != TYPE_STATUS_OK ? 0u : value; }
static C_VOID vm_debug_write_port(C_VOID *context, type_unsigned_16 port, type_unsigned_32 value)
{ vm_machine *machine = (vm_machine *)context; if (machine != STD_NULL) (C_VOID)core_machine_debug_write_port(machine->core_machine, port, value); }
static C_VOID vm_debug_set_break_real(C_VOID *context, type_unsigned_16 seg, type_unsigned_16 off)
{ vm_machine *machine = (vm_machine *)context; if (machine != STD_NULL) vm_machine_debug_set_breakpoint_real(&machine->debug, seg, off); }
static C_VOID vm_debug_set_break_linear(C_VOID *context, type_unsigned_32 address)
{ vm_machine *machine = (vm_machine *)context; if (machine != STD_NULL) vm_machine_debug_set_breakpoint_linear(&machine->debug, address); }
static C_VOID vm_debug_clear_break(C_VOID *context, C_INT linear)
{ vm_machine *machine = (vm_machine *)context; if (machine == STD_NULL) return; if (linear) vm_machine_debug_clear_breakpoint_linear(&machine->debug); else vm_machine_debug_clear_breakpoint_real(&machine->debug); }
static C_VOID vm_debug_set_trace(C_VOID *context, STD_SIZE_T count)
{ vm_machine *machine = (vm_machine *)context; if (machine != STD_NULL) vm_machine_debug_set_trace(&machine->debug, count); }
static C_VOID vm_debug_clear_trace(C_VOID *context)
{ vm_machine *machine = (vm_machine *)context; if (machine != STD_NULL) vm_machine_debug_clear_trace(&machine->debug); }
static STD_SIZE_T vm_debug_break_count(C_VOID *context)
{ vm_machine *machine = (vm_machine *)context; return machine == STD_NULL ? 0u : vm_machine_debug_get_breakpoint_count(&machine->debug); }
static C_VOID vm_debug_set_watch(C_VOID *context, core_debug_watch_kind kind, type_unsigned_32 address)
{
    vm_machine *machine = (vm_machine *)context;
    core_machine_debug_watch_kind mapped;

    if (machine == STD_NULL || vm_debug_map_watch(kind, &mapped) !=
            TYPE_STATUS_OK) return;
    (C_VOID)core_machine_debug_set_watchpoint(machine->core_machine,
        mapped, address);
}
static C_VOID vm_debug_clear_watch(C_VOID *context, core_debug_watch_kind kind)
{
    vm_machine *machine = (vm_machine *)context;
    core_machine_debug_watch_kind mapped;

    if (machine != STD_NULL && vm_debug_map_watch(kind, &mapped) ==
            TYPE_STATUS_OK) (C_VOID)core_machine_debug_clear_watchpoint(
        machine->core_machine, mapped);
}
static C_VOID vm_debug_print_registers(C_VOID *context)
{ vm_machine *machine = (vm_machine *)context; if (machine != STD_NULL) (C_VOID)core_machine_debug_print_registers(machine->core_machine); }
static C_VOID vm_debug_print_segment_registers(C_VOID *context)
{ vm_machine *machine = (vm_machine *)context; if (machine != STD_NULL) (C_VOID)core_machine_debug_print_segment_registers(machine->core_machine); }
static C_VOID vm_debug_print_control_registers(C_VOID *context)
{ vm_machine *machine = (vm_machine *)context; if (machine != STD_NULL) (C_VOID)core_machine_debug_print_control_registers(machine->core_machine); }
static C_VOID vm_debug_print_memory(C_VOID *context)
{ vm_machine *machine = (vm_machine *)context; if (machine != STD_NULL) (C_VOID)core_machine_debug_print_memory_accesses(machine->core_machine); }
static C_VOID vm_debug_print_watchpoints(C_VOID *context)
{ vm_machine *machine = (vm_machine *)context; if (machine != STD_NULL) (C_VOID)core_machine_debug_print_watchpoints(machine->core_machine); }

static C_INT vm_debug_get_fault_outcome(C_VOID *context,
    core_debug_fault_outcome *out_outcome)
{
    vm_machine_fault_outcome outcome;

    if (out_outcome == STD_NULL || vm_machine_fault_get((vm_machine *)context,
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

lib_status vm_machine_common_debug_execute(void *context,
    const common_machine_debug_request *request,
    common_machine_debug_result *out_result)
{
    vm_machine *machine = context;
    core_machine_debug_register register_id;
    type_unsigned_32 value = 0u;

    if (machine == STD_NULL || request == LIB_NULL || out_result == LIB_NULL ||
        request->bytes > COMMON_MACHINE_DEBUG_BYTES) return LIB_STATUS_INVALID_ARGUMENT;
    STD_MEMSET(out_result, 0, sizeof(*out_result));
    if (request->operation == COMMON_MACHINE_DEBUG_READ_REGISTER ||
        request->operation == COMMON_MACHINE_DEBUG_WRITE_REGISTER) {
        if (request->register_id >= CORE_MACHINE_DEBUG_REGISTER_COUNT) return LIB_STATUS_INVALID_ARGUMENT;
        register_id = (core_machine_debug_register)request->register_id;
        if (request->operation == COMMON_MACHINE_DEBUG_READ_REGISTER) {
            if (core_machine_debug_read_register(machine->core_machine, register_id,
                    &out_result->value) != TYPE_STATUS_OK) return LIB_STATUS_INVALID_STATE;
        } else {
            if (core_machine_debug_write_register(machine->core_machine, register_id,
                    request->address) != TYPE_STATUS_OK) return LIB_STATUS_INVALID_STATE;
        }
        return LIB_STATUS_OK;
    }
    if (request->operation == COMMON_MACHINE_DEBUG_READ_LINEAR ||
        request->operation == COMMON_MACHINE_DEBUG_WRITE_LINEAR) {
        if (request->bytes == 0u) return LIB_STATUS_INVALID_ARGUMENT;
        if ((request->operation == COMMON_MACHINE_DEBUG_READ_LINEAR ?
                core_machine_debug_read_linear(machine->core_machine, request->address,
                    out_result->data, request->bytes) :
                core_machine_debug_write_linear(machine->core_machine, request->address,
                    request->data, request->bytes)) != TYPE_STATUS_OK) return LIB_STATUS_INVALID_STATE;
        out_result->bytes = request->bytes;
        return LIB_STATUS_OK;
    }
    if (request->operation == COMMON_MACHINE_DEBUG_READ_PORT) {
        if (core_machine_debug_read_port(machine->core_machine, request->port,
                &value) != TYPE_STATUS_OK) return LIB_STATUS_INVALID_STATE;
        out_result->value = value;
        return LIB_STATUS_OK;
    }
    if (request->operation == COMMON_MACHINE_DEBUG_WRITE_PORT) {
        if (core_machine_debug_write_port(machine->core_machine, request->port,
                request->address) != TYPE_STATUS_OK) return LIB_STATUS_INVALID_STATE;
        return LIB_STATUS_OK;
    }
    return LIB_STATUS_INVALID_ARGUMENT;
}

static const core_debug_target vmDebugTargetTemplate = {
    .is_running = vm_debug_running,
    .resume = vm_debug_resume,
    .wait_for_completion = vm_debug_wait_for_completion,
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

const core_debug_target *vm_machine_debug_target(
    vm_machine *machine)
{
    if (machine == STD_NULL) return STD_NULL;
    if (machine->debug_target == STD_NULL) {
        machine->debug_target = (core_debug_target *)STD_MALLOC(
            sizeof(*machine->debug_target));
        if (machine->debug_target == STD_NULL) return STD_NULL;
    }
    *machine->debug_target = vmDebugTargetTemplate;
    machine->debug_target->context = machine;
    return machine->debug_target;
}

C_VOID vm_machine_debug_target_finalize(vm_machine *machine)
{
    if (machine == STD_NULL) return;
    STD_FREE(machine->debug_target);
    machine->debug_target = STD_NULL;
}

type_status vm_machine_run_debugger(vm_machine *session)
{
    vm_machine_result result = {0};

    if (session == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (vm_machine_control_is_running(&session->control)) {
        (C_VOID)vm_machine_request_pause(session);
        if (!vm_machine_control_wait_for_pause(&session->control, 2000u))
            return TYPE_STATUS_INVALID_STATE;
    }
    core_debugger_run(session->debugger, vm_machine_debug_target(session));
    result.kind = VM_MACHINE_RESULT_DEBUG_COMPLETED;
    result.status = TYPE_STATUS_OK;
    vm_machine_publish_result(session, &result);
    return TYPE_STATUS_OK;
}

type_status vm_machine_record_start(vm_machine *session, const C_CHAR *path)
{
    if (session == STD_NULL || path == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    return vm_machine_debug_record_start(&session->debug, path);
}

type_status vm_machine_record_stop(vm_machine *session)
{
    if (session == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    return vm_machine_debug_record_stop(&session->debug);
}
