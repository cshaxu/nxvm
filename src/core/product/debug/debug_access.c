#include "type.h"

#include "core/product/debug/debug_access.h"

static const core_product_debug_target *debug_target(C_VOID)
{
    return core_product_debug_scope_target();
}

C_INT core_product_debug_is_running(C_VOID)
{
    const core_product_debug_target *target = debug_target();
    return target != STD_NULL && target->is_running != STD_NULL &&
           target->is_running(target->context);
}

C_VOID core_product_debug_resume(C_VOID)
{
    const core_product_debug_target *target = debug_target();
    if (target != STD_NULL && target->is_paused != STD_NULL &&
        target->is_paused(target->context) && target->continue_execution != STD_NULL) {
        target->continue_execution(target->context);
    } else if (target != STD_NULL && target->resume != STD_NULL) {
        target->resume(target->context);
    }
}

C_INT core_product_debug_is_paused(C_VOID)
{
    const core_product_debug_target *target = debug_target();
    return target != STD_NULL && target->is_paused != STD_NULL &&
           target->is_paused(target->context);
}

core_product_debug_pause_reason core_product_debug_get_pause_reason(C_VOID)
{
    const core_product_debug_target *target = debug_target();
    return target != STD_NULL && target->get_pause_reason != STD_NULL ?
        target->get_pause_reason(target->context) : CORE_PRODUCT_DEBUG_PAUSE_NONE;
}

C_INT core_product_debug_request_pause(core_product_debug_pause_reason reason)
{
    const core_product_debug_target *target = debug_target();
    return target != STD_NULL && target->request_pause != STD_NULL &&
           target->request_pause(target->context, reason) == 0;
}

C_VOID core_product_debug_continue(C_VOID)
{
    const core_product_debug_target *target = debug_target();
    if (target != STD_NULL && target->continue_execution != STD_NULL) {
        target->continue_execution(target->context);
    }
}

C_INT core_product_debug_step(C_VOID)
{
    const core_product_debug_target *target = debug_target();
    return target != STD_NULL && target->step != STD_NULL && target->step(target->context) == 0;
}

C_INT core_product_debug_read_register(core_product_debug_register reg, uint32_t *value)
{
    const core_product_debug_target *target = debug_target();
    return target == STD_NULL || target->read_register == STD_NULL ||
           target->read_register(target->context, reg, value);
}

C_INT core_product_debug_write_register(core_product_debug_register reg, uint32_t value)
{
    const core_product_debug_target *target = debug_target();
    return target == STD_NULL || target->write_register == STD_NULL ||
           target->write_register(target->context, reg, value);
}

C_INT core_product_debug_get_code_default_size(C_VOID)
{
    const core_product_debug_target *target = debug_target();
    return target != STD_NULL && target->get_code_default_size != STD_NULL ?
           target->get_code_default_size(target->context) : 0;
}

uint32_t core_product_debug_get_code_base(C_VOID)
{
    const core_product_debug_target *target = debug_target();
    return target != STD_NULL && target->get_code_base != STD_NULL ?
           target->get_code_base(target->context) : 0u;
}

C_INT core_product_debug_read_linear(uint32_t address, C_VOID *out, uint8_t size)
{
    const core_product_debug_target *target = debug_target();
    return target == STD_NULL || target->read_linear == STD_NULL ||
           target->read_linear(target->context, address, out, size);
}

C_INT core_product_debug_write_linear(uint32_t address, const C_VOID *in, uint8_t size)
{
    const core_product_debug_target *target = debug_target();
    return target == STD_NULL || target->write_linear == STD_NULL ||
           target->write_linear(target->context, address, in, size);
}

C_INT core_product_debug_read_real(uint16_t segment, uint16_t offset, C_VOID *out, STD_SIZE_T size)
{
    const core_product_debug_target *target = debug_target();
    return target == STD_NULL || target->read_real == STD_NULL ||
           target->read_real(target->context, segment, offset, out, size);
}

C_INT core_product_debug_write_real(uint16_t segment, uint16_t offset, const C_VOID *in, STD_SIZE_T size)
{
    const core_product_debug_target *target = debug_target();
    return target == STD_NULL || target->write_real == STD_NULL ||
           target->write_real(target->context, segment, offset, in, size);
}

uint32_t core_product_debug_read_port(uint16_t port)
{
    const core_product_debug_target *target = debug_target();
    return target != STD_NULL && target->read_port != STD_NULL ?
           target->read_port(target->context, port) : 0u;
}

C_VOID core_product_debug_write_port(uint16_t port, uint32_t value)
{
    const core_product_debug_target *target = debug_target();
    if (target != STD_NULL && target->write_port != STD_NULL) target->write_port(target->context, port, value);
}

C_VOID core_product_debug_set_break_real(uint16_t segment, uint16_t offset)
{
    const core_product_debug_target *target = debug_target();
    if (target != STD_NULL && target->set_break_real != STD_NULL) target->set_break_real(target->context, segment, offset);
}

C_VOID core_product_debug_set_break_linear(uint32_t address)
{
    const core_product_debug_target *target = debug_target();
    if (target != STD_NULL && target->set_break_linear != STD_NULL) target->set_break_linear(target->context, address);
}

C_VOID core_product_debug_clear_break(C_INT linear)
{
    const core_product_debug_target *target = debug_target();
    if (target != STD_NULL && target->clear_break != STD_NULL) target->clear_break(target->context, linear);
}

C_VOID core_product_debug_set_trace(STD_SIZE_T count)
{
    const core_product_debug_target *target = debug_target();
    if (target != STD_NULL && target->set_trace != STD_NULL) target->set_trace(target->context, count);
}

C_VOID core_product_debug_clear_trace(C_VOID)
{
    const core_product_debug_target *target = debug_target();
    if (target != STD_NULL && target->clear_trace != STD_NULL) target->clear_trace(target->context);
}

STD_SIZE_T core_product_debug_get_break_count(C_VOID)
{
    const core_product_debug_target *target = debug_target();
    return target != STD_NULL && target->get_break_count != STD_NULL ?
           target->get_break_count(target->context) : 0u;
}

C_VOID core_product_debug_set_watch(core_product_debug_watch_kind kind, uint32_t address)
{
    const core_product_debug_target *target = debug_target();
    if (target != STD_NULL && target->set_watch != STD_NULL) target->set_watch(target->context, kind, address);
}

C_VOID core_product_debug_clear_watch(core_product_debug_watch_kind kind)
{
    const core_product_debug_target *target = debug_target();
    if (target != STD_NULL && target->clear_watch != STD_NULL) target->clear_watch(target->context, kind);
}

C_VOID core_product_debug_print_registers(C_VOID)
{
    const core_product_debug_target *target = debug_target();
    if (target != STD_NULL && target->print_registers != STD_NULL) target->print_registers(target->context);
}

C_VOID core_product_debug_print_segment_registers(C_VOID)
{
    const core_product_debug_target *target = debug_target();
    if (target != STD_NULL && target->print_segment_registers != STD_NULL) target->print_segment_registers(target->context);
}

C_VOID core_product_debug_print_control_registers(C_VOID)
{
    const core_product_debug_target *target = debug_target();
    if (target != STD_NULL && target->print_control_registers != STD_NULL) target->print_control_registers(target->context);
}

C_VOID core_product_debug_print_memory(C_VOID)
{
    const core_product_debug_target *target = debug_target();
    if (target != STD_NULL && target->print_memory != STD_NULL) target->print_memory(target->context);
}

C_VOID core_product_debug_print_watchpoints(C_VOID)
{
    const core_product_debug_target *target = debug_target();
    if (target != STD_NULL && target->print_watchpoints != STD_NULL) target->print_watchpoints(target->context);
}
