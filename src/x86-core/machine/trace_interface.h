#ifndef X86_CORE_MACHINE_TRACE_INTERFACE_H
#define X86_CORE_MACHINE_TRACE_INTERFACE_H
#include "lib/types/types_interface.h"



#ifdef __cplusplus
extern "C" {
#endif

typedef struct x86_core_machine x86_core_machine;

/* Copied transaction trace detail encodes owner in bits 0-7 and kind in
 * bits 8-15. These values describe events, not mutable transaction state. */
typedef enum x86_core_machine_transaction_owner {
    X86_CORE_MACHINE_TRANSACTION_OWNER_NONE = 0,
    X86_CORE_MACHINE_TRANSACTION_OWNER_CPU,
    X86_CORE_MACHINE_TRANSACTION_OWNER_DMA,
    X86_CORE_MACHINE_TRANSACTION_OWNER_REFRESH
} x86_core_machine_transaction_owner;

typedef enum x86_core_machine_transaction_kind {
    X86_CORE_MACHINE_TRANSACTION_CPU_MEMORY_READ = 1,
    X86_CORE_MACHINE_TRANSACTION_CPU_MEMORY_WRITE,
    X86_CORE_MACHINE_TRANSACTION_CPU_PORT_READ,
    X86_CORE_MACHINE_TRANSACTION_CPU_PORT_WRITE,
    X86_CORE_MACHINE_TRANSACTION_DMA_MEMORY_READ,
    X86_CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE,
    X86_CORE_MACHINE_TRANSACTION_DMA_MEMORY_COPY,
    X86_CORE_MACHINE_TRANSACTION_REFRESH_MEMORY_CYCLE,
    /* ESC command issue is CPU-owned control traffic.  It deliberately is
     * not a HOLD grant: an 80287/80387 asks its paired CPU to move operands
     * through the processor-extension channel. */
    X86_CORE_MACHINE_TRANSACTION_CPU_FPU_COMMAND,
    /* The first logical INTA acknowledges the selected PIC request.  Vector
     * delivery remains the CPU interrupt-entry operation that follows. */
    X86_CORE_MACHINE_TRANSACTION_CPU_INTERRUPT_ACKNOWLEDGE,
    X86_CORE_MACHINE_TRANSACTION_CPU_CACHE_OPERATION,
    X86_CORE_MACHINE_TRANSACTION_CPU_MEMORY_LOCK,
    X86_CORE_MACHINE_TRANSACTION_CPU_SPECIAL_CYCLE
} x86_core_machine_transaction_kind;

#ifndef X86_CORE_MACHINE_RUNTIME_TRACE_ENABLED
#define X86_CORE_MACHINE_RUNTIME_TRACE_ENABLED 1
#endif

typedef enum x86_core_machine_trace_event_type {
    X86_CORE_MACHINE_TRACE_RESET = 1,
    X86_CORE_MACHINE_TRACE_PORT_READ,
    X86_CORE_MACHINE_TRACE_PORT_WRITE,
    X86_CORE_MACHINE_TRACE_MEMORY_READ,
    X86_CORE_MACHINE_TRACE_MEMORY_WRITE,
    X86_CORE_MACHINE_TRACE_CPU_RETIRE,
    X86_CORE_MACHINE_TRACE_EXTERNAL_TIME,
    X86_CORE_MACHINE_TRACE_DMA_ADVANCE,
    X86_CORE_MACHINE_TRACE_RUN_BOUNDARY = 14,
    X86_CORE_MACHINE_TRACE_STOP,
    X86_CORE_MACHINE_TRACE_FAULT,
    X86_CORE_MACHINE_TRACE_TRANSACTION_BEGIN = 19,
    X86_CORE_MACHINE_TRACE_TRANSACTION_COMMIT,
    X86_CORE_MACHINE_TRACE_TRANSACTION_CANCEL,
    X86_CORE_MACHINE_TRACE_TRANSACTION_HOLD_REQUEST,
    X86_CORE_MACHINE_TRACE_TRANSACTION_HOLD_ACKNOWLEDGE,
    X86_CORE_MACHINE_TRACE_TRANSACTION_HOLD_RELEASE,
    X86_CORE_MACHINE_TRACE_CPU_EXTERNAL_CYCLE_BEGIN,
    X86_CORE_MACHINE_TRACE_CPU_EXTERNAL_CYCLE_COMMIT,
    X86_CORE_MACHINE_TRACE_CPU_EXTERNAL_CYCLE_CANCEL,
    X86_CORE_MACHINE_TRACE_CPU_EXTERNAL_CYCLE_OVERLAP_DECLARE
} x86_core_machine_trace_event_type;

typedef struct x86_core_machine_trace_event {
    lib_u32 type;
    lib_u64 sequence;
    lib_u64 elapsed_ticks;
    lib_u64 timeline_ticks;
    lib_u32 linear_pc;
    lib_u32 address;
    lib_u32 value;
    lib_u32 detail;
} x86_core_machine_trace_event;

typedef void (*x86_core_machine_trace_event_provider)(
    void *context,
    const x86_core_machine_trace_event *event);

typedef struct x86_core_machine_trace_provider {
    x86_core_machine_trace_event_provider callback;
    void *context;
} x86_core_machine_trace_provider;

lib_status x86_core_machine_set_trace_provider(
    x86_core_machine *machine,
    const x86_core_machine_trace_provider *provider);

/* Attachments emit copied diagnostics without borrowing Core state.
 * Production Release retains the same compile-time no-op. */
#if X86_CORE_MACHINE_RUNTIME_TRACE_ENABLED || defined(X86_CORE_MACHINE_TRACE_IMPLEMENTATION)
void x86_core_machine_trace_record(
    x86_core_machine *machine,
    lib_u32 type,
    lib_u32 address,
    lib_u32 value,
    lib_u32 detail);
#else
#define x86_core_machine_trace_record(machine, type, address, value, detail) \
    do { \
        (void)sizeof(machine); \
        (void)sizeof(type); \
        (void)sizeof(address); \
        (void)sizeof(value); \
        (void)sizeof(detail); \
    } while (0)
#endif

#ifdef __cplusplus
}
#endif

#endif
