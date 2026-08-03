#include "type.h"

#include "vm/composition_debug.h"


#include <stdlib.h>


#include "vm/composition_machine.h"

#include "vm/composition_control.h"

#include "core/machine/cpu.h"

#include "core/machine/memory.h"

#include "core/machine/port.h"

#include "core/machine/cpu.h"

#include "vm/machine/debug.h"

static C_INT vm_debug_running(C_VOID *context) { return vm_composition_control_is_running(((vm_composition_live_machine *)context)->control); }
static C_VOID vm_debug_resume(C_VOID *context) { vm_composition_resume((vm_composition_live_machine *)context); }
static C_INT vm_debug_paused(C_VOID *context) { return vm_composition_control_is_paused(((vm_composition_live_machine *)context)->control); }
static core_product_debug_pause_reason vm_debug_pause_reason(C_VOID *context)
{
    switch (vm_composition_control_get_pause_reason(((vm_composition_live_machine *)context)->control)) {
    case VM_COMPOSITION_PAUSE_EXPLICIT: return CORE_PRODUCT_DEBUG_PAUSE_EXPLICIT;
    case VM_COMPOSITION_PAUSE_BREAKPOINT: return CORE_PRODUCT_DEBUG_PAUSE_BREAKPOINT;
    case VM_COMPOSITION_PAUSE_TRACE: return CORE_PRODUCT_DEBUG_PAUSE_TRACE;
    case VM_COMPOSITION_PAUSE_STEP: return CORE_PRODUCT_DEBUG_PAUSE_STEP;
    default: return CORE_PRODUCT_DEBUG_PAUSE_NONE;
    }
}
static C_INT vm_debug_request_pause(C_VOID *context, core_product_debug_pause_reason reason)
{
    vm_composition_pause_reason mapped = VM_COMPOSITION_PAUSE_EXPLICIT;
    if (reason == CORE_PRODUCT_DEBUG_PAUSE_BREAKPOINT) mapped = VM_COMPOSITION_PAUSE_BREAKPOINT;
    else if (reason == CORE_PRODUCT_DEBUG_PAUSE_TRACE) mapped = VM_COMPOSITION_PAUSE_TRACE;
    else if (reason == CORE_PRODUCT_DEBUG_PAUSE_STEP) mapped = VM_COMPOSITION_PAUSE_STEP;
    vm_composition_control_request_pause(((vm_composition_live_machine *)context)->control, mapped);
    return 0;
}
static C_VOID vm_debug_continue(C_VOID *context)
{ vm_composition_control_continue(((vm_composition_live_machine *)context)->control); }
static C_INT vm_debug_step(C_VOID *context)
{ return vm_composition_control_step(((vm_composition_live_machine *)context)->control) ? 0 : 1; }

static C_INT vm_debug_read_register(C_VOID *context, core_product_debug_register reg,
                                  uint32_t *value)
{
    vm_composition_live_machine *machine =
        (vm_composition_live_machine *)context;
    t_cpu *cpu = machine == STD_NULL ? STD_NULL : machine->cpu;
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
    vm_composition_live_machine *machine =
        (vm_composition_live_machine *)context;
    t_cpu *cpu = machine == STD_NULL ? STD_NULL : machine->cpu;
    if (machine == STD_NULL || cpu == STD_NULL) return 1;
    switch (reg) {
    case CORE_PRODUCT_DEBUG_EAX: cpu->data.eax = value; break; case CORE_PRODUCT_DEBUG_ECX: cpu->data.ecx = value; break;
    case CORE_PRODUCT_DEBUG_EDX: cpu->data.edx = value; break; case CORE_PRODUCT_DEBUG_EBX: cpu->data.ebx = value; break;
    case CORE_PRODUCT_DEBUG_ESP: cpu->data.esp = value; break; case CORE_PRODUCT_DEBUG_EBP: cpu->data.ebp = value; break;
    case CORE_PRODUCT_DEBUG_ESI: cpu->data.esi = value; break; case CORE_PRODUCT_DEBUG_EDI: cpu->data.edi = value; break;
    case CORE_PRODUCT_DEBUG_EIP: cpu->data.eip = value; break; case CORE_PRODUCT_DEBUG_EFLAGS: cpu->data.eflags = value; break;
    case CORE_PRODUCT_DEBUG_ES: return core_machine_cpu_execution_load_segment(machine->cpu_execution, &cpu->data.es, (uint16_t)value);
    case CORE_PRODUCT_DEBUG_CS: return core_machine_cpu_execution_load_segment(machine->cpu_execution, &cpu->data.cs, (uint16_t)value);
    case CORE_PRODUCT_DEBUG_SS: return core_machine_cpu_execution_load_segment(machine->cpu_execution, &cpu->data.ss, (uint16_t)value);
    case CORE_PRODUCT_DEBUG_DS: return core_machine_cpu_execution_load_segment(machine->cpu_execution, &cpu->data.ds, (uint16_t)value);
    case CORE_PRODUCT_DEBUG_FS: return core_machine_cpu_execution_load_segment(machine->cpu_execution, &cpu->data.fs, (uint16_t)value);
    case CORE_PRODUCT_DEBUG_GS: return core_machine_cpu_execution_load_segment(machine->cpu_execution, &cpu->data.gs, (uint16_t)value);
    case CORE_PRODUCT_DEBUG_CR0: cpu->data.cr0 = value; break; case CORE_PRODUCT_DEBUG_CR1: cpu->data.cr1 = value; break;
    case CORE_PRODUCT_DEBUG_CR2: cpu->data.cr2 = value; break; case CORE_PRODUCT_DEBUG_CR3: cpu->data.cr3 = value; break;
    case CORE_PRODUCT_DEBUG_CR4: cpu->data.cr4 = value; break; default: return 1;
    }
    return 0;
}

static C_INT vm_debug_code_default_size(C_VOID *context)
{
    vm_composition_live_machine *machine = (vm_composition_live_machine *)context;
    return machine == STD_NULL ? 0 : machine->cpu->data.cs.seg.exec.defsize;
}
static uint32_t vm_debug_code_base(C_VOID *context)
{
    vm_composition_live_machine *machine = (vm_composition_live_machine *)context;
    return machine == STD_NULL ? 0u : machine->cpu->data.cs.base;
}

static C_INT vm_debug_read_linear(C_VOID *context, uint32_t address, C_VOID *out, uint8_t size)
{
    vm_composition_live_machine *machine = (vm_composition_live_machine *)context;
    return machine == STD_NULL ? 1 : core_machine_cpu_execution_read_linear(
        machine->cpu_execution, address, (ntvdm64_type_virtual_address)out, size);
}
static C_INT vm_debug_write_linear(C_VOID *context, uint32_t address, const C_VOID *in, uint8_t size)
{
    vm_composition_live_machine *machine = (vm_composition_live_machine *)context;
    return machine == STD_NULL ? 1 : core_machine_cpu_execution_write_linear(
        machine->cpu_execution, address, (ntvdm64_type_virtual_address)in, size);
}
static C_INT vm_debug_read_real(C_VOID *context, uint16_t seg, uint16_t off, C_VOID *out, STD_SIZE_T size)
{ vm_composition_live_machine *machine = (vm_composition_live_machine *)context; if (machine == STD_NULL) return 1; core_machine_memory_read_real_from(machine->ram, seg, off, out, size); return 0; }
static C_INT vm_debug_write_real(C_VOID *context, uint16_t seg, uint16_t off, const C_VOID *in, STD_SIZE_T size)
{ vm_composition_live_machine *machine = (vm_composition_live_machine *)context; if (machine == STD_NULL) return 1; core_machine_memory_write_real_to(machine->ram, seg, off, in, size); return 0; }
static uint32_t vm_debug_read_port(C_VOID *context, uint16_t port)
{ vm_composition_live_machine *machine = (vm_composition_live_machine *)context; return machine == STD_NULL ? 0u : core_machine_port_read(machine->port, port); }
static C_VOID vm_debug_write_port(C_VOID *context, uint16_t port, uint32_t value)
{ vm_composition_live_machine *machine = (vm_composition_live_machine *)context; if (machine != STD_NULL) core_machine_port_write(machine->port, port, value); }
static C_VOID vm_debug_set_break_real(C_VOID *context, uint16_t seg, uint16_t off)
{ vm_composition_live_machine *machine = (vm_composition_live_machine *)context; if (machine != STD_NULL) vm_machine_debug_set_breakpoint_real(machine->debug, seg, off); }
static C_VOID vm_debug_set_break_linear(C_VOID *context, uint32_t address)
{ vm_composition_live_machine *machine = (vm_composition_live_machine *)context; if (machine != STD_NULL) vm_machine_debug_set_breakpoint_linear(machine->debug, address); }
static C_VOID vm_debug_clear_break(C_VOID *context, C_INT linear)
{ vm_composition_live_machine *machine = (vm_composition_live_machine *)context; if (machine == STD_NULL) return; if (linear) vm_machine_debug_clear_breakpoint_linear(machine->debug); else vm_machine_debug_clear_breakpoint_real(machine->debug); }
static C_VOID vm_debug_set_trace(C_VOID *context, STD_SIZE_T count)
{ vm_composition_live_machine *machine = (vm_composition_live_machine *)context; if (machine != STD_NULL) vm_machine_debug_set_trace(machine->debug, count); }
static C_VOID vm_debug_clear_trace(C_VOID *context)
{ vm_composition_live_machine *machine = (vm_composition_live_machine *)context; if (machine != STD_NULL) vm_machine_debug_clear_trace(machine->debug); }
static STD_SIZE_T vm_debug_break_count(C_VOID *context)
{ vm_composition_live_machine *machine = (vm_composition_live_machine *)context; return machine == STD_NULL ? 0u : vm_machine_debug_get_breakpoint_count(machine->debug); }
static C_VOID vm_debug_set_watch(C_VOID *context, core_product_debug_watch_kind kind, uint32_t address)
{
    vm_composition_live_machine *machine = (vm_composition_live_machine *)context;
    if (machine == STD_NULL) return;
    if (kind == CORE_PRODUCT_DEBUG_WATCH_READ) { machine->cpuins->data.wrLinear = address; machine->cpuins->data.flagWR = NTVDM64_TYPE_TRUE; }
    else if (kind == CORE_PRODUCT_DEBUG_WATCH_WRITE) { machine->cpuins->data.wwLinear = address; machine->cpuins->data.flagWW = NTVDM64_TYPE_TRUE; }
    else { machine->cpuins->data.weLinear = address; machine->cpuins->data.flagWE = NTVDM64_TYPE_TRUE; }
}
static C_VOID vm_debug_clear_watch(C_VOID *context, core_product_debug_watch_kind kind)
{
    vm_composition_live_machine *machine = (vm_composition_live_machine *)context;
    if (machine == STD_NULL) return;
    if (kind == CORE_PRODUCT_DEBUG_WATCH_READ) machine->cpuins->data.flagWR = NTVDM64_TYPE_FALSE;
    else if (kind == CORE_PRODUCT_DEBUG_WATCH_WRITE) machine->cpuins->data.flagWW = NTVDM64_TYPE_FALSE;
    else machine->cpuins->data.flagWE = NTVDM64_TYPE_FALSE;
}
static C_VOID vm_debug_print_registers(C_VOID *context)
{ vm_composition_live_machine *machine = (vm_composition_live_machine *)context; if (machine != STD_NULL) core_machine_cpu_print_registers(machine->cpu_execution); }
static C_VOID vm_debug_print_segment_registers(C_VOID *context)
{ vm_composition_live_machine *machine = (vm_composition_live_machine *)context; if (machine != STD_NULL) core_machine_cpu_print_segment_registers(machine->cpu_execution); }
static C_VOID vm_debug_print_control_registers(C_VOID *context)
{ vm_composition_live_machine *machine = (vm_composition_live_machine *)context; if (machine != STD_NULL) core_machine_cpu_print_control_registers(machine->cpu_execution); }
static C_VOID vm_debug_print_memory(C_VOID *context)
{ vm_composition_live_machine *machine = (vm_composition_live_machine *)context; if (machine != STD_NULL) core_machine_cpu_print_memory_accesses(machine->cpu_execution); }
static C_VOID vm_debug_print_watchpoints(C_VOID *context)
{ vm_composition_live_machine *machine = (vm_composition_live_machine *)context; if (machine != STD_NULL) core_machine_cpu_print_watchpoints(machine->cpu_execution); }

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
    .context = STD_NULL
};

const core_product_debug_target *vm_composition_debug_target(
    vm_composition_live_machine *machine)
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

C_VOID vm_composition_debug_target_finalize(vm_composition_live_machine *machine)
{
    if (machine == STD_NULL) return;
    STD_FREE(machine->debug_target);
    machine->debug_target = STD_NULL;
}
