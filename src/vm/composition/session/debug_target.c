#include "type.h"

#include "vm/composition/session/debug_target.h"




#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/fault.h"

#include "vm/composition/session/control.h"

#include "core/machine/cpu.h"
#include "core/machine/debug_interface.h"

#include "core/machine/memory.h"

#include "core/machine/port.h"

#include "core/machine/cpu.h"

#include "vm/machine/debug.h"
#include "vm/platform/input.h"

C_VOID vm_session_debug_flush_console_input(C_VOID *context)
{
    (C_VOID)context;
    vm_platform_input_flush_console_input();
}

static t_cpu *vm_debug_cpu(const vm_session *machine)
{ return machine == STD_NULL ? STD_NULL :
    core_machine_debug_cpu_borrow(machine->core_machine); }

static t_cpuins *vm_debug_instructions(const vm_session *machine)
{ return machine == STD_NULL ? STD_NULL :
    core_machine_debug_cpu_instructions_borrow(machine->core_machine); }

static core_machine_cpu_execution_context *vm_debug_execution(
    const vm_session *machine)
{ return machine == STD_NULL ? STD_NULL :
    core_machine_debug_cpu_execution_borrow(machine->core_machine); }

static t_ram *vm_debug_memory(const vm_session *machine)
{ return machine == STD_NULL ? STD_NULL :
    core_machine_debug_memory_borrow(machine->core_machine); }

static t_port *vm_debug_port(const vm_session *machine)
{ return machine == STD_NULL ? STD_NULL :
    core_machine_debug_port_borrow(machine->core_machine); }

static C_INT vm_debug_running(C_VOID *context) { return vm_session_control_is_running(&((vm_session *)context)->control); }
static C_VOID vm_debug_resume(C_VOID *context) { vm_session_resume((vm_session *)context); }
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
    t_cpu *cpu = vm_debug_cpu(machine);
    if (cpu == STD_NULL) return 1;
    if (value == STD_NULL) return 1;
    switch (reg) {
    case CORE_PRODUCT_DEBUG_EAX: *value = cpu->data.eax; break; case CORE_PRODUCT_DEBUG_ECX: *value = cpu->data.ecx; break;
    case CORE_PRODUCT_DEBUG_EDX: *value = cpu->data.edx; break; case CORE_PRODUCT_DEBUG_EBX: *value = cpu->data.ebx; break;
    case CORE_PRODUCT_DEBUG_ESP: *value = cpu->data.esp; break; case CORE_PRODUCT_DEBUG_EBP: *value = cpu->data.ebp; break;
    case CORE_PRODUCT_DEBUG_ESI: *value = cpu->data.esi; break; case CORE_PRODUCT_DEBUG_EDI: *value = cpu->data.edi; break;
    case CORE_PRODUCT_DEBUG_EIP: *value = cpu->data.eip; break; case CORE_PRODUCT_DEBUG_EFLAGS: *value = cpu->data.eflags; break;
    case CORE_PRODUCT_DEBUG_ES: *value = cpu->data.es.selector; break; case CORE_PRODUCT_DEBUG_CS: *value = cpu->data.cs.selector; break;
    case CORE_PRODUCT_DEBUG_SS: *value = cpu->data.ss.selector; break; case CORE_PRODUCT_DEBUG_DS: *value = cpu->data.ds.selector; break;
    case CORE_PRODUCT_DEBUG_FS: *value = cpu->data.fs.selector; break; case CORE_PRODUCT_DEBUG_GS: *value = cpu->data.gs.selector; break;
    case CORE_PRODUCT_DEBUG_CR0: *value = cpu->data.cr0; break; case CORE_PRODUCT_DEBUG_CR1: *value = cpu->data.cr1; break;
    case CORE_PRODUCT_DEBUG_CR2: *value = cpu->data.cr2; break; case CORE_PRODUCT_DEBUG_CR3: *value = cpu->data.cr3; break;
    case CORE_PRODUCT_DEBUG_CR4: *value = cpu->data.cr4; break; default: return 1;
    }
    return 0;
}

static C_INT vm_debug_write_register(C_VOID *context, core_product_debug_register reg,
                                   uint32_t value)
{
    vm_session *machine =
        (vm_session *)context;
    t_cpu *cpu = vm_debug_cpu(machine);
    if (machine == STD_NULL || cpu == STD_NULL) return 1;
    switch (reg) {
    case CORE_PRODUCT_DEBUG_EAX: cpu->data.eax = value; break; case CORE_PRODUCT_DEBUG_ECX: cpu->data.ecx = value; break;
    case CORE_PRODUCT_DEBUG_EDX: cpu->data.edx = value; break; case CORE_PRODUCT_DEBUG_EBX: cpu->data.ebx = value; break;
    case CORE_PRODUCT_DEBUG_ESP: cpu->data.esp = value; break; case CORE_PRODUCT_DEBUG_EBP: cpu->data.ebp = value; break;
    case CORE_PRODUCT_DEBUG_ESI: cpu->data.esi = value; break; case CORE_PRODUCT_DEBUG_EDI: cpu->data.edi = value; break;
    case CORE_PRODUCT_DEBUG_EIP: cpu->data.eip = value; break; case CORE_PRODUCT_DEBUG_EFLAGS: cpu->data.eflags = value; break;
    case CORE_PRODUCT_DEBUG_ES: return core_machine_cpu_execution_load_segment(vm_debug_execution(machine), &cpu->data.es, (uint16_t)value);
    case CORE_PRODUCT_DEBUG_CS: return core_machine_cpu_execution_load_segment(vm_debug_execution(machine), &cpu->data.cs, (uint16_t)value);
    case CORE_PRODUCT_DEBUG_SS: return core_machine_cpu_execution_load_segment(vm_debug_execution(machine), &cpu->data.ss, (uint16_t)value);
    case CORE_PRODUCT_DEBUG_DS: return core_machine_cpu_execution_load_segment(vm_debug_execution(machine), &cpu->data.ds, (uint16_t)value);
    case CORE_PRODUCT_DEBUG_FS: return core_machine_cpu_execution_load_segment(vm_debug_execution(machine), &cpu->data.fs, (uint16_t)value);
    case CORE_PRODUCT_DEBUG_GS: return core_machine_cpu_execution_load_segment(vm_debug_execution(machine), &cpu->data.gs, (uint16_t)value);
    case CORE_PRODUCT_DEBUG_CR0: cpu->data.cr0 = value; break; case CORE_PRODUCT_DEBUG_CR1: cpu->data.cr1 = value; break;
    case CORE_PRODUCT_DEBUG_CR2: cpu->data.cr2 = value; break; case CORE_PRODUCT_DEBUG_CR3: cpu->data.cr3 = value; break;
    case CORE_PRODUCT_DEBUG_CR4: cpu->data.cr4 = value; break; default: return 1;
    }
    return 0;
}

static C_INT vm_debug_code_default_size(C_VOID *context)
{
    vm_session *machine = (vm_session *)context;
    return vm_debug_cpu(machine) == STD_NULL ? 0 : vm_debug_cpu(machine)->data.cs.seg.exec.defsize;
}
static uint32_t vm_debug_code_base(C_VOID *context)
{
    vm_session *machine = (vm_session *)context;
    return vm_debug_cpu(machine) == STD_NULL ? 0u : vm_debug_cpu(machine)->data.cs.base;
}

static C_INT vm_debug_read_linear(C_VOID *context, uint32_t address, C_VOID *out, uint8_t size)
{
    vm_session *machine = (vm_session *)context;
    return machine == STD_NULL ? 1 : core_machine_cpu_execution_read_linear(
        vm_debug_execution(machine), address, (type_virtual_address)out, size);
}
static C_INT vm_debug_write_linear(C_VOID *context, uint32_t address, const C_VOID *in, uint8_t size)
{
    vm_session *machine = (vm_session *)context;
    return machine == STD_NULL ? 1 : core_machine_cpu_execution_write_linear(
        vm_debug_execution(machine), address, (type_virtual_address)in, size);
}
static C_INT vm_debug_read_real(C_VOID *context, uint16_t seg, uint16_t off, C_VOID *out, STD_SIZE_T size)
{ vm_session *machine = (vm_session *)context; if (machine == STD_NULL) return 1; core_machine_memory_read_real_from(vm_debug_memory(machine), seg, off, out, size); return 0; }
static C_INT vm_debug_write_real(C_VOID *context, uint16_t seg, uint16_t off, const C_VOID *in, STD_SIZE_T size)
{ vm_session *machine = (vm_session *)context; if (machine == STD_NULL) return 1; core_machine_memory_write_real_to(vm_debug_memory(machine), seg, off, in, size); return 0; }
static uint32_t vm_debug_read_port(C_VOID *context, uint16_t port)
{ vm_session *machine = (vm_session *)context; return machine == STD_NULL ? 0u : core_machine_port_read(vm_debug_port(machine), port); }
static C_VOID vm_debug_write_port(C_VOID *context, uint16_t port, uint32_t value)
{ vm_session *machine = (vm_session *)context; if (machine != STD_NULL) core_machine_port_write(vm_debug_port(machine), port, value); }
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
    if (kind == CORE_PRODUCT_DEBUG_WATCH_READ) { vm_debug_instructions(machine)->data.wrLinear = address; vm_debug_instructions(machine)->data.flagWR = TYPE_TRUE; }
    else if (kind == CORE_PRODUCT_DEBUG_WATCH_WRITE) { vm_debug_instructions(machine)->data.wwLinear = address; vm_debug_instructions(machine)->data.flagWW = TYPE_TRUE; }
    else { vm_debug_instructions(machine)->data.weLinear = address; vm_debug_instructions(machine)->data.flagWE = TYPE_TRUE; }
}
static C_VOID vm_debug_clear_watch(C_VOID *context, core_product_debug_watch_kind kind)
{
    vm_session *machine = (vm_session *)context;
    if (machine == STD_NULL) return;
    if (kind == CORE_PRODUCT_DEBUG_WATCH_READ) vm_debug_instructions(machine)->data.flagWR = TYPE_FALSE;
    else if (kind == CORE_PRODUCT_DEBUG_WATCH_WRITE) vm_debug_instructions(machine)->data.flagWW = TYPE_FALSE;
    else vm_debug_instructions(machine)->data.flagWE = TYPE_FALSE;
}
static C_VOID vm_debug_print_registers(C_VOID *context)
{ vm_session *machine = (vm_session *)context; if (machine != STD_NULL) core_machine_cpu_print_registers(vm_debug_execution(machine)); }
static C_VOID vm_debug_print_segment_registers(C_VOID *context)
{ vm_session *machine = (vm_session *)context; if (machine != STD_NULL) core_machine_cpu_print_segment_registers(vm_debug_execution(machine)); }
static C_VOID vm_debug_print_control_registers(C_VOID *context)
{ vm_session *machine = (vm_session *)context; if (machine != STD_NULL) core_machine_cpu_print_control_registers(vm_debug_execution(machine)); }
static C_VOID vm_debug_print_memory(C_VOID *context)
{ vm_session *machine = (vm_session *)context; if (machine != STD_NULL) core_machine_cpu_print_memory_accesses(vm_debug_execution(machine)); }
static C_VOID vm_debug_print_watchpoints(C_VOID *context)
{ vm_session *machine = (vm_session *)context; if (machine != STD_NULL) core_machine_cpu_print_watchpoints(vm_debug_execution(machine)); }

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
