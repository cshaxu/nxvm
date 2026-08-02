#include "vm/composition_debug.h"

#include "vm/composition_machine.h"
#include "vm/composition_control.h"
#include "vm/machine/device.h"

static int vm_debug_running(void *context) { (void)context; return vm_composition_control_is_running(); }
static void vm_debug_resume(void *context) { (void)context; machineResume(); }

static int vm_debug_read_register(void *context, core_product_debug_register reg,
                                  uint32_t *value)
{
    (void)context;
    if (value == NULL) return 1;
    switch (reg) {
    case CORE_PRODUCT_DEBUG_EAX: *value = _eax; break; case CORE_PRODUCT_DEBUG_ECX: *value = _ecx; break;
    case CORE_PRODUCT_DEBUG_EDX: *value = _edx; break; case CORE_PRODUCT_DEBUG_EBX: *value = _ebx; break;
    case CORE_PRODUCT_DEBUG_ESP: *value = _esp; break; case CORE_PRODUCT_DEBUG_EBP: *value = _ebp; break;
    case CORE_PRODUCT_DEBUG_ESI: *value = _esi; break; case CORE_PRODUCT_DEBUG_EDI: *value = _edi; break;
    case CORE_PRODUCT_DEBUG_EIP: *value = _eip; break; case CORE_PRODUCT_DEBUG_EFLAGS: *value = _eflags; break;
    case CORE_PRODUCT_DEBUG_ES: *value = _es; break; case CORE_PRODUCT_DEBUG_CS: *value = _cs; break;
    case CORE_PRODUCT_DEBUG_SS: *value = _ss; break; case CORE_PRODUCT_DEBUG_DS: *value = _ds; break;
    case CORE_PRODUCT_DEBUG_FS: *value = _fs; break; case CORE_PRODUCT_DEBUG_GS: *value = _gs; break;
    case CORE_PRODUCT_DEBUG_CR0: *value = _cr(0); break; case CORE_PRODUCT_DEBUG_CR1: *value = _cr(1); break;
    case CORE_PRODUCT_DEBUG_CR2: *value = _cr(2); break; case CORE_PRODUCT_DEBUG_CR3: *value = _cr(3); break;
    case CORE_PRODUCT_DEBUG_CR4: *value = _cr(4); break; default: return 1;
    }
    return 0;
}

static int vm_debug_write_register(void *context, core_product_debug_register reg,
                                   uint32_t value)
{
    (void)context;
    switch (reg) {
    case CORE_PRODUCT_DEBUG_EAX: _eax = value; break; case CORE_PRODUCT_DEBUG_ECX: _ecx = value; break;
    case CORE_PRODUCT_DEBUG_EDX: _edx = value; break; case CORE_PRODUCT_DEBUG_EBX: _ebx = value; break;
    case CORE_PRODUCT_DEBUG_ESP: _esp = value; break; case CORE_PRODUCT_DEBUG_EBP: _ebp = value; break;
    case CORE_PRODUCT_DEBUG_ESI: _esi = value; break; case CORE_PRODUCT_DEBUG_EDI: _edi = value; break;
    case CORE_PRODUCT_DEBUG_EIP: _eip = value; break; case CORE_PRODUCT_DEBUG_EFLAGS: _eflags = value; break;
    case CORE_PRODUCT_DEBUG_ES: return deviceConnectCpuLoadES((uint16_t)value);
    case CORE_PRODUCT_DEBUG_CS: return deviceConnectCpuLoadCS((uint16_t)value);
    case CORE_PRODUCT_DEBUG_SS: return deviceConnectCpuLoadSS((uint16_t)value);
    case CORE_PRODUCT_DEBUG_DS: return deviceConnectCpuLoadDS((uint16_t)value);
    case CORE_PRODUCT_DEBUG_FS: return deviceConnectCpuLoadFS((uint16_t)value);
    case CORE_PRODUCT_DEBUG_GS: return deviceConnectCpuLoadGS((uint16_t)value);
    case CORE_PRODUCT_DEBUG_CR0: _cr(0) = value; break; case CORE_PRODUCT_DEBUG_CR1: _cr(1) = value; break;
    case CORE_PRODUCT_DEBUG_CR2: _cr(2) = value; break; case CORE_PRODUCT_DEBUG_CR3: _cr(3) = value; break;
    case CORE_PRODUCT_DEBUG_CR4: _cr(4) = value; break; default: return 1;
    }
    return 0;
}

static int vm_debug_code_default_size(void *context)
{ (void)context; return deviceConnectCpuGetCsDefSize(); }
static uint32_t vm_debug_code_base(void *context)
{ (void)context; return deviceConnectCpuGetCsBase(); }

static int vm_debug_read_linear(void *context, uint32_t address, void *out, uint8_t size)
{ (void)context; return deviceConnectCpuReadLinear(address, out, size); }
static int vm_debug_write_linear(void *context, uint32_t address, const void *in, uint8_t size)
{ (void)context; return deviceConnectCpuWriteLinear(address, (void *)in, size); }
static int vm_debug_read_real(void *context, uint16_t seg, uint16_t off, void *out, size_t size)
{ (void)context; deviceConnectRamRealRead(seg, off, out, size); return 0; }
static int vm_debug_write_real(void *context, uint16_t seg, uint16_t off, const void *in, size_t size)
{ (void)context; deviceConnectRamRealWrite(seg, off, (void *)in, size); return 0; }
static uint32_t vm_debug_read_port(void *context, uint16_t port)
{ (void)context; deviceConnectPortRead(port); return deviceConnectPortGetValue(); }
static void vm_debug_write_port(void *context, uint16_t port, uint32_t value)
{ (void)context; deviceConnectPortSetValue(value); deviceConnectPortWrite(port); }
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
    if (kind == CORE_PRODUCT_DEBUG_WATCH_READ) deviceConnectCpuSetWR(address);
    else if (kind == CORE_PRODUCT_DEBUG_WATCH_WRITE) deviceConnectCpuSetWW(address);
    else deviceConnectCpuSetWE(address);
}
static void vm_debug_clear_watch(void *context, core_product_debug_watch_kind kind)
{
    (void)context;
    if (kind == CORE_PRODUCT_DEBUG_WATCH_READ) deviceConnectCpuClearWR();
    else if (kind == CORE_PRODUCT_DEBUG_WATCH_WRITE) deviceConnectCpuClearWW();
    else deviceConnectCpuClearWE();
}
static void vm_debug_print_registers(void *context)
{ (void)context; devicePrintCpuReg(); }
static void vm_debug_print_segment_registers(void *context)
{ (void)context; devicePrintCpuSreg(); }
static void vm_debug_print_control_registers(void *context)
{ (void)context; devicePrintCpuCreg(); }
static void vm_debug_print_memory(void *context)
{ (void)context; devicePrintCpuMem(); }
static void vm_debug_print_watchpoints(void *context)
{ (void)context; devicePrintCpuWatch(); }

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
