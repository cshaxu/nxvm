/* Copyright 2012-2014 Neko. */

#ifndef NXVM_VM_MACHINE_DEBUG_H
#define NXVM_VM_MACHINE_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"
#include "core/machine/cpu.h"
#include "core/machine/cpu_instructions.h"

#define NXVM_DEVICE_DEBUG "Unknown Hardware Debugger"

typedef enum vm_machine_debug_pause_reason {
    VM_MACHINE_DEBUG_PAUSE_BREAKPOINT,
    VM_MACHINE_DEBUG_PAUSE_TRACE
} vm_machine_debug_pause_reason;

typedef void (*vm_machine_debug_pause_callback)(void *context,
    vm_machine_debug_pause_reason reason);

typedef struct {
    FILE *recordFile; /* pointer to dump file */
    vm_machine_debug_pause_callback pauseCallback;
    void *pauseContext;
    t_cpu *cpu;
    t_cpuins *cpuins;
} t_debug_connect;

typedef struct {
    ntvdm64_type_bool flagBreak; /* breakpoint set (1) or not (0) */
    ntvdm64_type_bool flagBreak32;
    ntvdm64_type_bool flagTrace; /* tracer set(1) or not (0) */
    ntvdm64_type_native_unsigned breakCount, traceCount;
    ntvdm64_type_unsigned_16 breakCS, breakIP;
    ntvdm64_type_unsigned_32 breakLinear;
} t_debug_data;

typedef struct {
    t_debug_data data;
    t_debug_connect connect;
} t_debug;

void vm_machine_debug_initialize(t_debug *debug, t_cpu *cpu, t_cpuins *cpuins);
void vm_machine_debug_reset(t_debug *debug);
void vm_machine_debug_refresh(t_debug *debug);
void vm_machine_debug_finalize(t_debug *debug);
void vm_machine_debug_bind_pause(t_debug *debug,
    vm_machine_debug_pause_callback callback, void *context);
void vm_machine_debug_set_breakpoint_real(t_debug *debug, uint16_t segment,
    uint16_t offset);
void vm_machine_debug_clear_breakpoint_real(t_debug *debug);
void vm_machine_debug_set_breakpoint_linear(t_debug *debug, uint32_t linear);
void vm_machine_debug_clear_breakpoint_linear(t_debug *debug);
size_t vm_machine_debug_get_breakpoint_count(const t_debug *debug);
void vm_machine_debug_set_trace(t_debug *debug, size_t instruction_count);
void vm_machine_debug_clear_trace(t_debug *debug);
void vm_machine_debug_record_start(t_debug *debug, const char *file_name);
void vm_machine_debug_record_stop(t_debug *debug);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
