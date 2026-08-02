#include "vm/composition_debug.h"

#include "vm/composition_machine.h"
#include "vm/composition_control.h"
#include "core/machine/cpu.h"
#include "core/machine/memory.h"
#include "core/machine/port.h"
#include "core/machine/vcpu.h"
#include "vm/machine/device.h"

static int vm_debug_running(void *context) { (void)context; return vm_composition_control_is_running(); }
static void vm_debug_resume(void *context) { (void)context; machineResume(); }

static int vm_debug_read_register(void *context, core_product_debug_register reg,
                                  uint32_t *value)
{
    (void)context;
    if (value == NULL) return 1;
    switch (reg) {
    case CORE_PRODUCT_DEBUG_EAX: *value = vcpu.data.eax; break; case CORE_PRODUCT_DEBUG_ECX: *value = vcpu.data.ecx; break;
    case CORE_PRODUCT_DEBUG_EDX: *value = vcpu.data.edx; break; case CORE_PRODUCT_DEBUG_EBX: *value = vcpu.data.ebx; break;
    case CORE_PRODUCT_DEBUG_ESP: *value = vcpu.data.esp; break; case CORE_PRODUCT_DEBUG_EBP: *value = vcpu.data.ebp; break;
    case CORE_PRODUCT_DEBUG_ESI: *value = vcpu.data.esi; break; case CORE_PRODUCT_DEBUG_EDI: *value = vcpu.data.edi; break;
    case CORE_PRODUCT_DEBUG_EIP: *value = vcpu.data.eip; break; case CORE_PRODUCT_DEBUG_EFLAGS: *value = vcpu.data.eflags; break;
    case CORE_PRODUCT_DEBUG_ES: *value = vcpu.data.es.selector; break; case CORE_PRODUCT_DEBUG_CS: *value = vcpu.data.cs.selector; break;
    case CORE_PRODUCT_DEBUG_SS: *value = vcpu.data.ss.selector; break; case CORE_PRODUCT_DEBUG_DS: *value = vcpu.data.ds.selector; break;
    case CORE_PRODUCT_DEBUG_FS: *value = vcpu.data.fs.selector; break; case CORE_PRODUCT_DEBUG_GS: *value = vcpu.data.gs.selector; break;
    case CORE_PRODUCT_DEBUG_CR0: *value = vcpu.data.cr0; break; case CORE_PRODUCT_DEBUG_CR1: *value = vcpu.data.cr1; break;
    case CORE_PRODUCT_DEBUG_CR2: *value = vcpu.data.cr2; break; case CORE_PRODUCT_DEBUG_CR3: *value = vcpu.data.cr3; break;
    case CORE_PRODUCT_DEBUG_CR4: *value = vcpu.data.cr4; break; default: return 1;
    }
    return 0;
}

static int vm_debug_write_register(void *context, core_product_debug_register reg,
                                   uint32_t value)
{
    (void)context;
    switch (reg) {
    case CORE_PRODUCT_DEBUG_EAX: vcpu.data.eax = value; break; case CORE_PRODUCT_DEBUG_ECX: vcpu.data.ecx = value; break;
    case CORE_PRODUCT_DEBUG_EDX: vcpu.data.edx = value; break; case CORE_PRODUCT_DEBUG_EBX: vcpu.data.ebx = value; break;
    case CORE_PRODUCT_DEBUG_ESP: vcpu.data.esp = value; break; case CORE_PRODUCT_DEBUG_EBP: vcpu.data.ebp = value; break;
    case CORE_PRODUCT_DEBUG_ESI: vcpu.data.esi = value; break; case CORE_PRODUCT_DEBUG_EDI: vcpu.data.edi = value; break;
    case CORE_PRODUCT_DEBUG_EIP: vcpu.data.eip = value; break; case CORE_PRODUCT_DEBUG_EFLAGS: vcpu.data.eflags = value; break;
    case CORE_PRODUCT_DEBUG_ES: return core_machine_cpu_load_segment(CORE_MACHINE_CPU_SEGMENT_ES, (uint16_t)value);
    case CORE_PRODUCT_DEBUG_CS: return core_machine_cpu_load_segment(CORE_MACHINE_CPU_SEGMENT_CS, (uint16_t)value);
    case CORE_PRODUCT_DEBUG_SS: return core_machine_cpu_load_segment(CORE_MACHINE_CPU_SEGMENT_SS, (uint16_t)value);
    case CORE_PRODUCT_DEBUG_DS: return core_machine_cpu_load_segment(CORE_MACHINE_CPU_SEGMENT_DS, (uint16_t)value);
    case CORE_PRODUCT_DEBUG_FS: return core_machine_cpu_load_segment(CORE_MACHINE_CPU_SEGMENT_FS, (uint16_t)value);
    case CORE_PRODUCT_DEBUG_GS: return core_machine_cpu_load_segment(CORE_MACHINE_CPU_SEGMENT_GS, (uint16_t)value);
    case CORE_PRODUCT_DEBUG_CR0: vcpu.data.cr0 = value; break; case CORE_PRODUCT_DEBUG_CR1: vcpu.data.cr1 = value; break;
    case CORE_PRODUCT_DEBUG_CR2: vcpu.data.cr2 = value; break; case CORE_PRODUCT_DEBUG_CR3: vcpu.data.cr3 = value; break;
    case CORE_PRODUCT_DEBUG_CR4: vcpu.data.cr4 = value; break; default: return 1;
    }
    return 0;
}

static int vm_debug_code_default_size(void *context)
{ (void)context; return core_machine_cpu_get_code_default_size(); }
static uint32_t vm_debug_code_base(void *context)
{ (void)context; return core_machine_cpu_get_code_base(); }

static int vm_debug_read_linear(void *context, uint32_t address, void *out, uint8_t size)
{ (void)context; return core_machine_cpu_read_linear(address, out, size); }
static int vm_debug_write_linear(void *context, uint32_t address, const void *in, uint8_t size)
{ (void)context; return core_machine_cpu_write_linear(address, in, size); }
static int vm_debug_read_real(void *context, uint16_t seg, uint16_t off, void *out, size_t size)
{ (void)context; core_machine_memory_read_real(seg, off, out, size); return 0; }
static int vm_debug_write_real(void *context, uint16_t seg, uint16_t off, const void *in, size_t size)
{ (void)context; core_machine_memory_write_real(seg, off, in, size); return 0; }
static uint32_t vm_debug_read_port(void *context, uint16_t port)
{ (void)context; return core_machine_port_read_legacy(port); }
static void vm_debug_write_port(void *context, uint16_t port, uint32_t value)
{ (void)context; core_machine_port_write_legacy(port, value); }
static void vm_debug_set_break_real(void *context, uint16_t seg, uint16_t off)
{ (void)context; deviceConnectDebugSetBreak(seg, off); }
static void vm_debug_set_break_linear(void *context, uint32_t address)
{ (void)context; deviceConnectDebugSetBreak32(address); }
static void vm_debug_clear_break(void *context, int linear)
{ (void)context; if (linear) deviceConnectDebugClearBreak32(); else deviceConnectDebugClearBreak(); }
static void vm_debug_set_trace(void *context, size_t count)
{ (void)context; deviceConnectDebugSetTrace(count); }
static void vm_debug_clear_trace(void *context)
{ (void)context; deviceConnectDebugClearTrace(); }
static size_t vm_debug_break_count(void *context)
{ (void)context; return deviceConnectDebugGetBreakCount(); }
static void vm_debug_set_watch(void *context, core_product_debug_watch_kind kind, uint32_t address)
{
    (void)context;
    if (kind == CORE_PRODUCT_DEBUG_WATCH_READ) core_machine_cpu_set_watchpoint(CORE_MACHINE_CPU_WATCH_READ, address);
    else if (kind == CORE_PRODUCT_DEBUG_WATCH_WRITE) core_machine_cpu_set_watchpoint(CORE_MACHINE_CPU_WATCH_WRITE, address);
    else core_machine_cpu_set_watchpoint(CORE_MACHINE_CPU_WATCH_EXECUTE, address);
}
static void vm_debug_clear_watch(void *context, core_product_debug_watch_kind kind)
{
    (void)context;
    if (kind == CORE_PRODUCT_DEBUG_WATCH_READ) core_machine_cpu_clear_watchpoint(CORE_MACHINE_CPU_WATCH_READ);
    else if (kind == CORE_PRODUCT_DEBUG_WATCH_WRITE) core_machine_cpu_clear_watchpoint(CORE_MACHINE_CPU_WATCH_WRITE);
    else core_machine_cpu_clear_watchpoint(CORE_MACHINE_CPU_WATCH_EXECUTE);
}
static void vm_debug_print_registers(void *context)
{ (void)context; core_machine_cpu_print_registers(); }
static void vm_debug_print_segment_registers(void *context)
{ (void)context; core_machine_cpu_print_segment_registers(); }
static void vm_debug_print_control_registers(void *context)
{ (void)context; core_machine_cpu_print_control_registers(); }
static void vm_debug_print_memory(void *context)
{ (void)context; core_machine_cpu_print_memory_accesses(); }
static void vm_debug_print_watchpoints(void *context)
{ (void)context; core_machine_cpu_print_watchpoints(); }

static const core_product_debug_target vmDebugTarget = {
    .is_running = vm_debug_running,
    .resume = vm_debug_resume,
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
    .context = NULL
};

const core_product_debug_target *vm_composition_debug_target(void)
{
    return &vmDebugTarget;
}
