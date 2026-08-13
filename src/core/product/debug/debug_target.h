#ifndef CORE_PRODUCT_DEBUG_TARGET_H
#define CORE_PRODUCT_DEBUG_TARGET_H

#include "type.h"



typedef enum core_product_debug_register {
    CORE_PRODUCT_DEBUG_EAX, CORE_PRODUCT_DEBUG_ECX, CORE_PRODUCT_DEBUG_EDX,
    CORE_PRODUCT_DEBUG_EBX, CORE_PRODUCT_DEBUG_ESP, CORE_PRODUCT_DEBUG_EBP,
    CORE_PRODUCT_DEBUG_ESI, CORE_PRODUCT_DEBUG_EDI, CORE_PRODUCT_DEBUG_EIP,
    CORE_PRODUCT_DEBUG_EFLAGS, CORE_PRODUCT_DEBUG_ES, CORE_PRODUCT_DEBUG_CS,
    CORE_PRODUCT_DEBUG_SS, CORE_PRODUCT_DEBUG_DS, CORE_PRODUCT_DEBUG_FS,
    CORE_PRODUCT_DEBUG_GS, CORE_PRODUCT_DEBUG_CR0, CORE_PRODUCT_DEBUG_CR1,
    CORE_PRODUCT_DEBUG_CR2, CORE_PRODUCT_DEBUG_CR3, CORE_PRODUCT_DEBUG_CR4
} core_product_debug_register;

typedef enum core_product_debug_watch_kind {
    CORE_PRODUCT_DEBUG_WATCH_READ,
    CORE_PRODUCT_DEBUG_WATCH_WRITE,
    CORE_PRODUCT_DEBUG_WATCH_EXECUTE
} core_product_debug_watch_kind;

typedef enum core_product_debug_pause_reason {
    CORE_PRODUCT_DEBUG_PAUSE_NONE,
    CORE_PRODUCT_DEBUG_PAUSE_EXPLICIT,
    CORE_PRODUCT_DEBUG_PAUSE_BREAKPOINT,
    CORE_PRODUCT_DEBUG_PAUSE_TRACE,
    CORE_PRODUCT_DEBUG_PAUSE_STEP
} core_product_debug_pause_reason;

typedef struct core_product_debug_fault_outcome {
    C_INT valid;
    type_unsigned_32 detail;
    type_unsigned_32 linear_pc;
    type_unsigned_64 executed;
    C_INT diagnostic_valid;
    type_unsigned_32 exception_mask;
    type_unsigned_32 exception_code;
    type_unsigned_16 cs;
    type_unsigned_32 eip;
} core_product_debug_fault_outcome;

typedef struct core_product_debug_target {
    C_INT (*is_running)(C_VOID *context);
    C_VOID (*resume)(C_VOID *context);
    C_INT (*is_paused)(C_VOID *context);
    core_product_debug_pause_reason (*get_pause_reason)(C_VOID *context);
    C_INT (*request_pause)(C_VOID *context, core_product_debug_pause_reason reason);
    C_VOID (*continue_execution)(C_VOID *context);
    C_INT (*step)(C_VOID *context);
    C_INT (*read_register)(C_VOID *context, core_product_debug_register reg,
                         type_unsigned_32 *value);
    C_INT (*write_register)(C_VOID *context, core_product_debug_register reg,
                          type_unsigned_32 value);
    C_INT (*get_code_default_size)(C_VOID *context);
    type_unsigned_32 (*get_code_base)(C_VOID *context);
    C_INT (*read_linear)(C_VOID *context, type_unsigned_32 address, C_VOID *out, type_unsigned_8 size);
    C_INT (*write_linear)(C_VOID *context, type_unsigned_32 address, const C_VOID *in, type_unsigned_8 size);
    C_INT (*read_real)(C_VOID *context, type_unsigned_16 segment, type_unsigned_16 offset,
                     C_VOID *out, STD_SIZE_T size);
    C_INT (*write_real)(C_VOID *context, type_unsigned_16 segment, type_unsigned_16 offset,
                      const C_VOID *in, STD_SIZE_T size);
    type_unsigned_32 (*read_port)(C_VOID *context, type_unsigned_16 port);
    C_VOID (*write_port)(C_VOID *context, type_unsigned_16 port, type_unsigned_32 value);
    C_VOID (*set_break_real)(C_VOID *context, type_unsigned_16 segment, type_unsigned_16 offset);
    C_VOID (*set_break_linear)(C_VOID *context, type_unsigned_32 address);
    C_VOID (*clear_break)(C_VOID *context, C_INT linear);
    C_VOID (*set_trace)(C_VOID *context, STD_SIZE_T count);
    C_VOID (*clear_trace)(C_VOID *context);
    STD_SIZE_T (*get_break_count)(C_VOID *context);
    C_VOID (*set_watch)(C_VOID *context, core_product_debug_watch_kind kind,
                      type_unsigned_32 address);
    C_VOID (*clear_watch)(C_VOID *context, core_product_debug_watch_kind kind);
    C_VOID (*print_registers)(C_VOID *context);
    C_VOID (*print_segment_registers)(C_VOID *context);
    C_VOID (*print_control_registers)(C_VOID *context);
    C_VOID (*print_memory)(C_VOID *context);
    C_VOID (*print_watchpoints)(C_VOID *context);
    C_INT (*get_fault_outcome)(C_VOID *context,
        core_product_debug_fault_outcome *out_outcome);
    C_VOID *context;
} core_product_debug_target;

#endif
