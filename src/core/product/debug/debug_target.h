#ifndef NTVDM64_CORE_PRODUCT_DEBUG_TARGET_H
#define NTVDM64_CORE_PRODUCT_DEBUG_TARGET_H

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

typedef struct core_product_debug_target {
    int (*is_running)(void *context);
    void (*resume)(void *context);
    int (*read_register)(void *context, core_product_debug_register reg,
                         uint32_t *value);
    int (*write_register)(void *context, core_product_debug_register reg,
                          uint32_t value);
    int (*read_linear)(void *context, uint32_t address, void *out, uint8_t size);
    int (*write_linear)(void *context, uint32_t address, const void *in, uint8_t size);
    int (*read_real)(void *context, uint16_t segment, uint16_t offset,
                     void *out, size_t size);
    int (*write_real)(void *context, uint16_t segment, uint16_t offset,
                      const void *in, size_t size);
    uint32_t (*read_port)(void *context, uint16_t port);
    void (*write_port)(void *context, uint16_t port, uint32_t value);
    void (*set_break_real)(void *context, uint16_t segment, uint16_t offset);
    void (*set_break_linear)(void *context, uint32_t address);
    void (*clear_break)(void *context, int linear);
    void (*set_trace)(void *context, size_t count);
    void (*clear_trace)(void *context);
    size_t (*get_break_count)(void *context);
    void (*set_watch)(void *context, core_product_debug_watch_kind kind,
                      uint32_t address);
    void (*clear_watch)(void *context, core_product_debug_watch_kind kind);
    void *context;
} core_product_debug_target;

void core_product_debug_bind_target(const core_product_debug_target *target);
const core_product_debug_target *core_product_debug_get_target(void);

#endif
