#ifndef NTVDM64_CORE_PRODUCT_DEBUG_ACCESS_H
#define NTVDM64_CORE_PRODUCT_DEBUG_ACCESS_H

#include "core/product/debug/debug_target.h"

int core_product_debug_is_running(void);
void core_product_debug_resume(void);
int core_product_debug_read_register(core_product_debug_register reg, uint32_t *value);
int core_product_debug_write_register(core_product_debug_register reg, uint32_t value);
int core_product_debug_get_code_default_size(void);
uint32_t core_product_debug_get_code_base(void);
int core_product_debug_read_linear(uint32_t address, void *out, uint8_t size);
int core_product_debug_write_linear(uint32_t address, const void *in, uint8_t size);
int core_product_debug_read_real(uint16_t segment, uint16_t offset, void *out, size_t size);
int core_product_debug_write_real(uint16_t segment, uint16_t offset, const void *in, size_t size);
uint32_t core_product_debug_read_port(uint16_t port);
void core_product_debug_write_port(uint16_t port, uint32_t value);
void core_product_debug_set_break_real(uint16_t segment, uint16_t offset);
void core_product_debug_set_break_linear(uint32_t address);
void core_product_debug_clear_break(int linear);
void core_product_debug_set_trace(size_t count);
void core_product_debug_clear_trace(void);
size_t core_product_debug_get_break_count(void);
void core_product_debug_set_watch(core_product_debug_watch_kind kind, uint32_t address);
void core_product_debug_clear_watch(core_product_debug_watch_kind kind);
void core_product_debug_print_registers(void);
void core_product_debug_print_segment_registers(void);
void core_product_debug_print_control_registers(void);
void core_product_debug_print_memory(void);
void core_product_debug_print_watchpoints(void);

#endif
