#ifndef NTVDM64_CORE_PRODUCT_DEBUG_TARGET_H
#define NTVDM64_CORE_PRODUCT_DEBUG_TARGET_H

#include "type.h"

#include <stddef.h>

#include <stdint.h>

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

typedef struct core_product_debug_target {
    C_INT (*is_running)(C_VOID *context);
    C_VOID (*resume)(C_VOID *context);
    C_INT (*is_paused)(C_VOID *context);
    core_product_debug_pause_reason (*get_pause_reason)(C_VOID *context);
    C_INT (*request_pause)(C_VOID *context, core_product_debug_pause_reason reason);
    C_VOID (*continue_execution)(C_VOID *context);
    C_INT (*step)(C_VOID *context);
    C_INT (*read_register)(C_VOID *context, core_product_debug_register reg,
                         uint32_t *value);
    C_INT (*write_register)(C_VOID *context, core_product_debug_register reg,
                          uint32_t value);
    C_INT (*get_code_default_size)(C_VOID *context);
    uint32_t (*get_code_base)(C_VOID *context);
    C_INT (*read_linear)(C_VOID *context, uint32_t address, C_VOID *out, uint8_t size);
    C_INT (*write_linear)(C_VOID *context, uint32_t address, const C_VOID *in, uint8_t size);
    C_INT (*read_real)(C_VOID *context, uint16_t segment, uint16_t offset,
                     C_VOID *out, STD_SIZE_T size);
    C_INT (*write_real)(C_VOID *context, uint16_t segment, uint16_t offset,
                      const C_VOID *in, STD_SIZE_T size);
    uint32_t (*read_port)(C_VOID *context, uint16_t port);
    C_VOID (*write_port)(C_VOID *context, uint16_t port, uint32_t value);
    C_VOID (*set_break_real)(C_VOID *context, uint16_t segment, uint16_t offset);
    C_VOID (*set_break_linear)(C_VOID *context, uint32_t address);
    C_VOID (*clear_break)(C_VOID *context, C_INT linear);
    C_VOID (*set_trace)(C_VOID *context, STD_SIZE_T count);
    C_VOID (*clear_trace)(C_VOID *context);
    STD_SIZE_T (*get_break_count)(C_VOID *context);
    C_VOID (*set_watch)(C_VOID *context, core_product_debug_watch_kind kind,
                      uint32_t address);
    C_VOID (*clear_watch)(C_VOID *context, core_product_debug_watch_kind kind);
    C_VOID (*print_registers)(C_VOID *context);
    C_VOID (*print_segment_registers)(C_VOID *context);
    C_VOID (*print_control_registers)(C_VOID *context);
    C_VOID (*print_memory)(C_VOID *context);
    C_VOID (*print_watchpoints)(C_VOID *context);
    C_VOID *context;
} core_product_debug_target;

C_VOID core_product_debug_scope_enter(const core_product_debug_target *target);
C_VOID core_product_debug_scope_leave(C_VOID);
const core_product_debug_target *core_product_debug_scope_target(C_VOID);

#endif
