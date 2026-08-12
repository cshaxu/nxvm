#include "type.h"

#include "core/product/debug/debug_access.h"

C_INT core_product_debug_is_running(const core_product_debug_target *target)
{
    return target != STD_NULL && target->is_running != STD_NULL &&
           target->is_running(target->context);
}

C_VOID core_product_debug_resume(const core_product_debug_target *target)
{
    if (target != STD_NULL && target->is_paused != STD_NULL &&
        target->is_paused(target->context) && target->continue_execution != STD_NULL) {
        target->continue_execution(target->context);
    } else if (target != STD_NULL && target->resume != STD_NULL) {
        target->resume(target->context);
    }
}

C_INT core_product_debug_is_paused(const core_product_debug_target *target)
{
    return target != STD_NULL && target->is_paused != STD_NULL &&
           target->is_paused(target->context);
}

core_product_debug_pause_reason core_product_debug_get_pause_reason(const core_product_debug_target *target)
{
    return target != STD_NULL && target->get_pause_reason != STD_NULL ?
        target->get_pause_reason(target->context) : CORE_PRODUCT_DEBUG_PAUSE_NONE;
}

C_INT core_product_debug_request_pause(const core_product_debug_target *target, core_product_debug_pause_reason reason)
{
    return target != STD_NULL && target->request_pause != STD_NULL &&
           target->request_pause(target->context, reason) == 0;
}

C_VOID core_product_debug_continue(const core_product_debug_target *target)
{
    if (target != STD_NULL && target->continue_execution != STD_NULL) {
        target->continue_execution(target->context);
    }
}

C_INT core_product_debug_step(const core_product_debug_target *target)
{
    return target != STD_NULL && target->step != STD_NULL && target->step(target->context) == 0;
}

C_INT core_product_debug_read_register(const core_product_debug_target *target, core_product_debug_register reg, type_unsigned_32 *value)
{
    return target == STD_NULL || target->read_register == STD_NULL ||
           target->read_register(target->context, reg, value);
}

C_INT core_product_debug_write_register(const core_product_debug_target *target, core_product_debug_register reg, type_unsigned_32 value)
{
    return target == STD_NULL || target->write_register == STD_NULL ||
           target->write_register(target->context, reg, value);
}

C_INT core_product_debug_get_code_default_size(const core_product_debug_target *target)
{
    return target != STD_NULL && target->get_code_default_size != STD_NULL ?
           target->get_code_default_size(target->context) : 0;
}

type_unsigned_32 core_product_debug_get_code_base(const core_product_debug_target *target)
{
    return target != STD_NULL && target->get_code_base != STD_NULL ?
           target->get_code_base(target->context) : 0u;
}

C_INT core_product_debug_read_linear(const core_product_debug_target *target, type_unsigned_32 address, C_VOID *out, type_unsigned_8 size)
{
    return target == STD_NULL || target->read_linear == STD_NULL ||
           target->read_linear(target->context, address, out, size);
}

C_INT core_product_debug_write_linear(const core_product_debug_target *target, type_unsigned_32 address, const C_VOID *in, type_unsigned_8 size)
{
    return target == STD_NULL || target->write_linear == STD_NULL ||
           target->write_linear(target->context, address, in, size);
}

C_INT core_product_debug_read_real(const core_product_debug_target *target, type_unsigned_16 segment, type_unsigned_16 offset, C_VOID *out, STD_SIZE_T size)
{
    return target == STD_NULL || target->read_real == STD_NULL ||
           target->read_real(target->context, segment, offset, out, size);
}

C_INT core_product_debug_write_real(const core_product_debug_target *target, type_unsigned_16 segment, type_unsigned_16 offset, const C_VOID *in, STD_SIZE_T size)
{
    return target == STD_NULL || target->write_real == STD_NULL ||
           target->write_real(target->context, segment, offset, in, size);
}

type_unsigned_32 core_product_debug_read_port(const core_product_debug_target *target, type_unsigned_16 port)
{
    return target != STD_NULL && target->read_port != STD_NULL ?
           target->read_port(target->context, port) : 0u;
}

C_VOID core_product_debug_write_port(const core_product_debug_target *target, type_unsigned_16 port, type_unsigned_32 value)
{
    if (target != STD_NULL && target->write_port != STD_NULL) target->write_port(target->context, port, value);
}

C_VOID core_product_debug_set_break_real(const core_product_debug_target *target, type_unsigned_16 segment, type_unsigned_16 offset)
{
    if (target != STD_NULL && target->set_break_real != STD_NULL) target->set_break_real(target->context, segment, offset);
}

C_VOID core_product_debug_set_break_linear(const core_product_debug_target *target, type_unsigned_32 address)
{
    if (target != STD_NULL && target->set_break_linear != STD_NULL) target->set_break_linear(target->context, address);
}

C_VOID core_product_debug_clear_break(const core_product_debug_target *target, C_INT linear)
{
    if (target != STD_NULL && target->clear_break != STD_NULL) target->clear_break(target->context, linear);
}

C_VOID core_product_debug_set_trace(const core_product_debug_target *target, STD_SIZE_T count)
{
    if (target != STD_NULL && target->set_trace != STD_NULL) target->set_trace(target->context, count);
}

C_VOID core_product_debug_clear_trace(const core_product_debug_target *target)
{
    if (target != STD_NULL && target->clear_trace != STD_NULL) target->clear_trace(target->context);
}

STD_SIZE_T core_product_debug_get_break_count(const core_product_debug_target *target)
{
    return target != STD_NULL && target->get_break_count != STD_NULL ?
           target->get_break_count(target->context) : 0u;
}

C_VOID core_product_debug_set_watch(const core_product_debug_target *target, core_product_debug_watch_kind kind, type_unsigned_32 address)
{
    if (target != STD_NULL && target->set_watch != STD_NULL) target->set_watch(target->context, kind, address);
}

C_VOID core_product_debug_clear_watch(const core_product_debug_target *target, core_product_debug_watch_kind kind)
{
    if (target != STD_NULL && target->clear_watch != STD_NULL) target->clear_watch(target->context, kind);
}

C_VOID core_product_debug_print_registers(const core_product_debug_target *target)
{
    if (target != STD_NULL && target->print_registers != STD_NULL) target->print_registers(target->context);
}

C_VOID core_product_debug_print_segment_registers(const core_product_debug_target *target)
{
    if (target != STD_NULL && target->print_segment_registers != STD_NULL) target->print_segment_registers(target->context);
}

C_VOID core_product_debug_print_control_registers(const core_product_debug_target *target)
{
    if (target != STD_NULL && target->print_control_registers != STD_NULL) target->print_control_registers(target->context);
}

C_VOID core_product_debug_print_memory(const core_product_debug_target *target)
{
    if (target != STD_NULL && target->print_memory != STD_NULL) target->print_memory(target->context);
}

C_VOID core_product_debug_print_watchpoints(const core_product_debug_target *target)
{
    if (target != STD_NULL && target->print_watchpoints != STD_NULL) target->print_watchpoints(target->context);
}

C_INT core_product_debug_get_fault_outcome(const core_product_debug_target *target,
    core_product_debug_fault_outcome *out_outcome)
{
    return target != STD_NULL && target->get_fault_outcome != STD_NULL &&
        target->get_fault_outcome(target->context, out_outcome) == 0;
}

