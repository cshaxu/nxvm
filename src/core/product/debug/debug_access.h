#ifndef NTVDM64_CORE_PRODUCT_DEBUG_ACCESS_H
#define NTVDM64_CORE_PRODUCT_DEBUG_ACCESS_H

#include "type.h"

#include "core/product/debug/debug_target.h"

C_INT core_product_debug_is_running(C_VOID);
C_VOID core_product_debug_resume(C_VOID);
C_INT core_product_debug_is_paused(C_VOID);
core_product_debug_pause_reason core_product_debug_get_pause_reason(C_VOID);
C_INT core_product_debug_request_pause(core_product_debug_pause_reason reason);
C_VOID core_product_debug_continue(C_VOID);
C_INT core_product_debug_step(C_VOID);
C_INT core_product_debug_read_register(core_product_debug_register reg, uint32_t *value);
C_INT core_product_debug_write_register(core_product_debug_register reg, uint32_t value);
C_INT core_product_debug_get_code_default_size(C_VOID);
uint32_t core_product_debug_get_code_base(C_VOID);
C_INT core_product_debug_read_linear(uint32_t address, C_VOID *out, uint8_t size);
C_INT core_product_debug_write_linear(uint32_t address, const C_VOID *in, uint8_t size);
C_INT core_product_debug_read_real(uint16_t segment, uint16_t offset, C_VOID *out, size_t size);
C_INT core_product_debug_write_real(uint16_t segment, uint16_t offset, const C_VOID *in, size_t size);
uint32_t core_product_debug_read_port(uint16_t port);
C_VOID core_product_debug_write_port(uint16_t port, uint32_t value);
C_VOID core_product_debug_set_break_real(uint16_t segment, uint16_t offset);
C_VOID core_product_debug_set_break_linear(uint32_t address);
C_VOID core_product_debug_clear_break(C_INT linear);
C_VOID core_product_debug_set_trace(size_t count);
C_VOID core_product_debug_clear_trace(C_VOID);
size_t core_product_debug_get_break_count(C_VOID);
C_VOID core_product_debug_set_watch(core_product_debug_watch_kind kind, uint32_t address);
C_VOID core_product_debug_clear_watch(core_product_debug_watch_kind kind);
C_VOID core_product_debug_print_registers(C_VOID);
C_VOID core_product_debug_print_segment_registers(C_VOID);
C_VOID core_product_debug_print_control_registers(C_VOID);
C_VOID core_product_debug_print_memory(C_VOID);
C_VOID core_product_debug_print_watchpoints(C_VOID);

#endif
