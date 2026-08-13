#ifndef CORE_PRODUCT_DEBUG_ACCESS_H
#define CORE_PRODUCT_DEBUG_ACCESS_H

#include "type.h"

#include "core/product/debug/debug_target.h"

C_INT core_product_debug_is_running(const core_product_debug_target *target);
C_VOID core_product_debug_resume(const core_product_debug_target *target);
C_INT core_product_debug_is_paused(const core_product_debug_target *target);
core_product_debug_pause_reason core_product_debug_get_pause_reason(const core_product_debug_target *target);
C_INT core_product_debug_request_pause(const core_product_debug_target *target, core_product_debug_pause_reason reason);
C_VOID core_product_debug_continue(const core_product_debug_target *target);
C_INT core_product_debug_step(const core_product_debug_target *target);
C_INT core_product_debug_read_register(const core_product_debug_target *target, core_product_debug_register reg, type_unsigned_32 *value);
C_INT core_product_debug_write_register(const core_product_debug_target *target, core_product_debug_register reg, type_unsigned_32 value);
C_INT core_product_debug_get_code_default_size(const core_product_debug_target *target);
type_unsigned_32 core_product_debug_get_code_base(const core_product_debug_target *target);
C_INT core_product_debug_read_linear(const core_product_debug_target *target, type_unsigned_32 address, C_VOID *out, type_unsigned_8 size);
C_INT core_product_debug_write_linear(const core_product_debug_target *target, type_unsigned_32 address, const C_VOID *in, type_unsigned_8 size);
C_INT core_product_debug_read_real(const core_product_debug_target *target, type_unsigned_16 segment, type_unsigned_16 offset, C_VOID *out, STD_SIZE_T size);
C_INT core_product_debug_write_real(const core_product_debug_target *target, type_unsigned_16 segment, type_unsigned_16 offset, const C_VOID *in, STD_SIZE_T size);
type_unsigned_32 core_product_debug_read_port(const core_product_debug_target *target, type_unsigned_16 port);
C_VOID core_product_debug_write_port(const core_product_debug_target *target, type_unsigned_16 port, type_unsigned_32 value);
C_VOID core_product_debug_set_break_real(const core_product_debug_target *target, type_unsigned_16 segment, type_unsigned_16 offset);
C_VOID core_product_debug_set_break_linear(const core_product_debug_target *target, type_unsigned_32 address);
C_VOID core_product_debug_clear_break(const core_product_debug_target *target, C_INT linear);
C_VOID core_product_debug_set_trace(const core_product_debug_target *target, STD_SIZE_T count);
C_VOID core_product_debug_clear_trace(const core_product_debug_target *target);
STD_SIZE_T core_product_debug_get_break_count(const core_product_debug_target *target);
C_VOID core_product_debug_set_watch(const core_product_debug_target *target, core_product_debug_watch_kind kind, type_unsigned_32 address);
C_VOID core_product_debug_clear_watch(const core_product_debug_target *target, core_product_debug_watch_kind kind);
C_VOID core_product_debug_print_registers(const core_product_debug_target *target);
C_VOID core_product_debug_print_segment_registers(const core_product_debug_target *target);
C_VOID core_product_debug_print_control_registers(const core_product_debug_target *target);
C_VOID core_product_debug_print_memory(const core_product_debug_target *target);
C_VOID core_product_debug_print_watchpoints(const core_product_debug_target *target);
C_INT core_product_debug_get_fault_outcome(const core_product_debug_target *target,
    core_product_debug_fault_outcome *out_outcome);

#endif

