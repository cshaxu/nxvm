/* Copyright 2012-2014 Neko. */

#ifndef NXVM_VDEBUG_H
#define NXVM_VDEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "core/machine/vglobal.h"

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
} t_debug_connect;

typedef struct {
    t_bool flagBreak; /* breakpoint set (1) or not (0) */
    t_bool flagBreak32;
    t_bool flagTrace; /* tracer set(1) or not (0) */
    t_nubitcc breakCount, traceCount;
    t_nubit16 breakCS, breakIP;
    t_nubit32 breakLinear;
} t_debug_data;

typedef struct {
    t_debug_data data;
    t_debug_connect connect;
} t_debug;

t_debug *vm_machine_debug_current(void);
void vm_machine_debug_bind_live(t_debug *debug);
void vm_machine_debug_unbind_live(void);

/* Transitional direct alias to the one composition-owned debug object. */
#define vdebug (*vm_machine_debug_current())

void vdebugInit();
void vdebugReset();
void vdebugRefresh();
void vdebugFinal();
void vm_machine_debug_bind_pause(vm_machine_debug_pause_callback callback,
    void *context);
void vm_machine_debug_set_breakpoint_real(uint16_t segment, uint16_t offset);
void vm_machine_debug_clear_breakpoint_real(void);
void vm_machine_debug_set_breakpoint_linear(uint32_t linear);
void vm_machine_debug_clear_breakpoint_linear(void);
size_t vm_machine_debug_get_breakpoint_count(void);
void vm_machine_debug_set_trace(size_t instruction_count);
void vm_machine_debug_clear_trace(void);
void vm_machine_debug_record_start(const char *file_name);
void vm_machine_debug_record_stop(void);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
