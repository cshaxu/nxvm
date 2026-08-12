/* Copyright 2012-2014 Neko. */

#ifndef VM_MACHINE_DEBUG_H
#define VM_MACHINE_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"
#include "core/machine/debug_interface.h"

#define VM_MACHINE_DEVICE_DEBUG "Unknown Hardware Debugger"

typedef enum vm_machine_debug_pause_reason {
    VM_MACHINE_DEBUG_PAUSE_BREAKPOINT,
    VM_MACHINE_DEBUG_PAUSE_TRACE
} vm_machine_debug_pause_reason;

typedef C_VOID (*vm_machine_debug_pause_callback)(C_VOID *context,
    vm_machine_debug_pause_reason reason);
typedef type_unsigned_8 (*vm_machine_debug_disassemble_provider)(C_VOID *context,
    C_CHAR *statement, type_unsigned_8 *code, C_INT flag32);

typedef struct {
    STD_FILE *recordFile; /* pointer to dump file */
    vm_machine_debug_pause_callback pauseCallback;
    C_VOID *pauseContext;
    vm_machine_debug_disassemble_provider disassembleProvider;
    C_VOID *disassembleContext;
    core_machine_debug_instruction_observation observation;
    type_bool observation_valid;
} t_debug_connect;

typedef struct {
    type_bool flagBreak; /* breakpoint set (1) or not (0) */
    type_bool flagBreak32;
    type_bool flagTrace; /* tracer set(1) or not (0) */
    type_native_unsigned breakCount, traceCount;
    type_unsigned_16 breakCS, breakIP;
    type_unsigned_32 breakLinear;
} t_debug_data;

typedef struct {
    t_debug_data data;
    t_debug_connect connect;
} t_debug;

C_VOID vm_machine_debug_initialize(t_debug *debug);
C_VOID vm_machine_debug_reset(t_debug *debug);
C_VOID vm_machine_debug_refresh(t_debug *debug,
    const core_machine_debug_instruction_observation *observation);
C_VOID vm_machine_debug_finalize(t_debug *debug);
C_VOID vm_machine_debug_bind_pause(t_debug *debug,
    vm_machine_debug_pause_callback callback, C_VOID *context);
C_VOID vm_machine_debug_bind_disassembler(t_debug *debug,
    vm_machine_debug_disassemble_provider provider, C_VOID *context);
C_VOID vm_machine_debug_set_breakpoint_real(t_debug *debug, type_unsigned_16 segment,
    type_unsigned_16 offset);
C_VOID vm_machine_debug_clear_breakpoint_real(t_debug *debug);
C_VOID vm_machine_debug_set_breakpoint_linear(t_debug *debug, type_unsigned_32 linear);
C_VOID vm_machine_debug_clear_breakpoint_linear(t_debug *debug);
STD_SIZE_T vm_machine_debug_get_breakpoint_count(const t_debug *debug);
C_VOID vm_machine_debug_set_trace(t_debug *debug, STD_SIZE_T instruction_count);
C_VOID vm_machine_debug_clear_trace(t_debug *debug);
C_VOID vm_machine_debug_record_start(t_debug *debug, const C_CHAR *file_name);
C_VOID vm_machine_debug_record_stop(t_debug *debug);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
