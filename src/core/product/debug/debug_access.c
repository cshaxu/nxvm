#include "type.h"

#include "core/product/debug/debug_access.h"

static const core_product_debug_target *debug_target(C_VOID)
{
    return core_product_debug_scope_target();
}

C_INT core_product_debug_is_running(C_VOID)
{
    const core_product_debug_target *target = debug_target();
    return target != NULL && target->is_running != NULL &&
           target->is_running(target->context);
}

C_VOID core_product_debug_resume(C_VOID)
{
    const core_product_debug_target *target = debug_target();
    if (target != NULL && target->is_paused != NULL &&
        target->is_paused(target->context) && target->continue_execution != NULL) {
        target->continue_execution(target->context);
    } else if (target != NULL && target->resume != NULL) {
        target->resume(target->context);
    }
}

C_INT core_product_debug_is_paused(C_VOID)
{
    const core_product_debug_target *target = debug_target();
    return target != NULL && target->is_paused != NULL &&
           target->is_paused(target->context);
}

core_product_debug_pause_reason core_product_debug_get_pause_reason(C_VOID)
{
    const core_product_debug_target *target = debug_target();
    return target != NULL && target->get_pause_reason != NULL ?
        target->get_pause_reason(target->context) : CORE_PRODUCT_DEBUG_PAUSE_NONE;
}

C_INT core_product_debug_request_pause(core_product_debug_pause_reason reason)
{
    const core_product_debug_target *target = debug_target();
    return target != NULL && target->request_pause != NULL &&
           target->request_pause(target->context, reason) == 0;
}

C_VOID core_product_debug_continue(C_VOID)
{
    const core_product_debug_target *target = debug_target();
    if (target != NULL && target->continue_execution != NULL) {
        target->continue_execution(target->context);
    }
}

C_INT core_product_debug_step(C_VOID)
{
    const core_product_debug_target *target = debug_target();
    return target != NULL && target->step != NULL && target->step(target->context) == 0;
}

C_INT core_product_debug_read_register(core_product_debug_register reg, uint32_t *value)
{
    const core_product_debug_target *target = debug_target();
    return target == NULL || target->read_register == NULL ||
           target->read_register(target->context, reg, value);
}

C_INT core_product_debug_write_register(core_product_debug_register reg, uint32_t value)
{
    const core_product_debug_target *target = debug_target();
    return target == NULL || target->write_register == NULL ||
           target->write_register(target->context, reg, value);
}

C_INT core_product_debug_get_code_default_size(C_VOID)
{
    const core_product_debug_target *target = debug_target();
    return target != NULL && target->get_code_default_size != NULL ?
           target->get_code_default_size(target->context) : 0;
}

uint32_t core_product_debug_get_code_base(C_VOID)
{
    const core_product_debug_target *target = debug_target();
    return target != NULL && target->get_code_base != NULL ?
           target->get_code_base(target->context) : 0u;
}

C_INT core_product_debug_read_linear(uint32_t address, C_VOID *out, uint8_t size)
{
    const core_product_debug_target *target = debug_target();
    return target == NULL || target->read_linear == NULL ||
           target->read_linear(target->context, address, out, size);
}

C_INT core_product_debug_write_linear(uint32_t address, const C_VOID *in, uint8_t size)
{
    const core_product_debug_target *target = debug_target();
    return target == NULL || target->write_linear == NULL ||
           target->write_linear(target->context, address, in, size);
}

C_INT core_product_debug_read_real(uint16_t segment, uint16_t offset, C_VOID *out, size_t size)
{
    const core_product_debug_target *target = debug_target();
    return target == NULL || target->read_real == NULL ||
           target->read_real(target->context, segment, offset, out, size);
}

C_INT core_product_debug_write_real(uint16_t segment, uint16_t offset, const C_VOID *in, size_t size)
{
    const core_product_debug_target *target = debug_target();
    return target == NULL || target->write_real == NULL ||
           target->write_real(target->context, segment, offset, in, size);
}

uint32_t core_product_debug_read_port(uint16_t port)
{
    const core_product_debug_target *target = debug_target();
    return target != NULL && target->read_port != NULL ?
           target->read_port(target->context, port) : 0u;
}

C_VOID core_product_debug_write_port(uint16_t port, uint32_t value)
{
    const core_product_debug_target *target = debug_target();
    if (target != NULL && target->write_port != NULL) target->write_port(target->context, port, value);
}

C_VOID core_product_debug_set_break_real(uint16_t segment, uint16_t offset)
{
    const core_product_debug_target *target = debug_target();
    if (target != NULL && target->set_break_real != NULL) target->set_break_real(target->context, segment, offset);
}

C_VOID core_product_debug_set_break_linear(uint32_t address)
{
    const core_product_debug_target *target = debug_target();
    if (target != NULL && target->set_break_linear != NULL) target->set_break_linear(target->context, address);
}

C_VOID core_product_debug_clear_break(C_INT linear)
{
    const core_product_debug_target *target = debug_target();
    if (target != NULL && target->clear_break != NULL) target->clear_break(target->context, linear);
}

C_VOID core_product_debug_set_trace(size_t count)
{
    const core_product_debug_target *target = debug_target();
    if (target != NULL && target->set_trace != NULL) target->set_trace(target->context, count);
}

C_VOID core_product_debug_clear_trace(C_VOID)
{
    const core_product_debug_target *target = debug_target();
    if (target != NULL && target->clear_trace != NULL) target->clear_trace(target->context);
}

size_t core_product_debug_get_break_count(C_VOID)
{
    const core_product_debug_target *target = debug_target();
    return target != NULL && target->get_break_count != NULL ?
           target->get_break_count(target->context) : 0u;
}

C_VOID core_product_debug_set_watch(core_product_debug_watch_kind kind, uint32_t address)
{
    const core_product_debug_target *target = debug_target();
    if (target != NULL && target->set_watch != NULL) target->set_watch(target->context, kind, address);
}

C_VOID core_product_debug_clear_watch(core_product_debug_watch_kind kind)
{
    const core_product_debug_target *target = debug_target();
    if (target != NULL && target->clear_watch != NULL) target->clear_watch(target->context, kind);
}

C_VOID core_product_debug_print_registers(C_VOID)
{
    const core_product_debug_target *target = debug_target();
    if (target != NULL && target->print_registers != NULL) target->print_registers(target->context);
}

C_VOID core_product_debug_print_segment_registers(C_VOID)
{
    const core_product_debug_target *target = debug_target();
    if (target != NULL && target->print_segment_registers != NULL) target->print_segment_registers(target->context);
}

C_VOID core_product_debug_print_control_registers(C_VOID)
{
    const core_product_debug_target *target = debug_target();
    if (target != NULL && target->print_control_registers != NULL) target->print_control_registers(target->context);
}

C_VOID core_product_debug_print_memory(C_VOID)
{
    const core_product_debug_target *target = debug_target();
    if (target != NULL && target->print_memory != NULL) target->print_memory(target->context);
}

C_VOID core_product_debug_print_watchpoints(C_VOID)
{
    const core_product_debug_target *target = debug_target();
    if (target != NULL && target->print_watchpoints != NULL) target->print_watchpoints(target->context);
}
