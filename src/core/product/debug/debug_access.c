#include "core/product/debug/debug_access.h"

static const core_product_debug_target *debug_target(void)
{
    return core_product_debug_get_target();
}

int core_product_debug_is_running(void)
{
    const core_product_debug_target *target = debug_target();
    return target != NULL && target->is_running != NULL &&
           target->is_running(target->context);
}

void core_product_debug_resume(void)
{
    const core_product_debug_target *target = debug_target();
    if (target != NULL && target->resume != NULL) target->resume(target->context);
}

int core_product_debug_read_register(core_product_debug_register reg, uint32_t *value)
{
    const core_product_debug_target *target = debug_target();
    return target == NULL || target->read_register == NULL ||
           target->read_register(target->context, reg, value);
}

int core_product_debug_write_register(core_product_debug_register reg, uint32_t value)
{
    const core_product_debug_target *target = debug_target();
    return target == NULL || target->write_register == NULL ||
           target->write_register(target->context, reg, value);
}

int core_product_debug_get_code_default_size(void)
{
    const core_product_debug_target *target = debug_target();
    return target != NULL && target->get_code_default_size != NULL ?
           target->get_code_default_size(target->context) : 0;
}

uint32_t core_product_debug_get_code_base(void)
{
    const core_product_debug_target *target = debug_target();
    return target != NULL && target->get_code_base != NULL ?
           target->get_code_base(target->context) : 0u;
}

int core_product_debug_read_linear(uint32_t address, void *out, uint8_t size)
{
    const core_product_debug_target *target = debug_target();
    return target == NULL || target->read_linear == NULL ||
           target->read_linear(target->context, address, out, size);
}

int core_product_debug_write_linear(uint32_t address, const void *in, uint8_t size)
{
    const core_product_debug_target *target = debug_target();
    return target == NULL || target->write_linear == NULL ||
           target->write_linear(target->context, address, in, size);
}

int core_product_debug_read_real(uint16_t segment, uint16_t offset, void *out, size_t size)
{
    const core_product_debug_target *target = debug_target();
    return target == NULL || target->read_real == NULL ||
           target->read_real(target->context, segment, offset, out, size);
}

int core_product_debug_write_real(uint16_t segment, uint16_t offset, const void *in, size_t size)
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

void core_product_debug_write_port(uint16_t port, uint32_t value)
{
    const core_product_debug_target *target = debug_target();
    if (target != NULL && target->write_port != NULL) target->write_port(target->context, port, value);
}

void core_product_debug_set_break_real(uint16_t segment, uint16_t offset)
{
    const core_product_debug_target *target = debug_target();
    if (target != NULL && target->set_break_real != NULL) target->set_break_real(target->context, segment, offset);
}

void core_product_debug_set_break_linear(uint32_t address)
{
    const core_product_debug_target *target = debug_target();
    if (target != NULL && target->set_break_linear != NULL) target->set_break_linear(target->context, address);
}

void core_product_debug_clear_break(int linear)
{
    const core_product_debug_target *target = debug_target();
    if (target != NULL && target->clear_break != NULL) target->clear_break(target->context, linear);
}

void core_product_debug_set_trace(size_t count)
{
    const core_product_debug_target *target = debug_target();
    if (target != NULL && target->set_trace != NULL) target->set_trace(target->context, count);
}

void core_product_debug_clear_trace(void)
{
    const core_product_debug_target *target = debug_target();
    if (target != NULL && target->clear_trace != NULL) target->clear_trace(target->context);
}

size_t core_product_debug_get_break_count(void)
{
    const core_product_debug_target *target = debug_target();
    return target != NULL && target->get_break_count != NULL ?
           target->get_break_count(target->context) : 0u;
}

void core_product_debug_set_watch(core_product_debug_watch_kind kind, uint32_t address)
{
    const core_product_debug_target *target = debug_target();
    if (target != NULL && target->set_watch != NULL) target->set_watch(target->context, kind, address);
}

void core_product_debug_clear_watch(core_product_debug_watch_kind kind)
{
    const core_product_debug_target *target = debug_target();
    if (target != NULL && target->clear_watch != NULL) target->clear_watch(target->context, kind);
}

void core_product_debug_print_registers(void)
{
    const core_product_debug_target *target = debug_target();
    if (target != NULL && target->print_registers != NULL) target->print_registers(target->context);
}

void core_product_debug_print_segment_registers(void)
{
    const core_product_debug_target *target = debug_target();
    if (target != NULL && target->print_segment_registers != NULL) target->print_segment_registers(target->context);
}

void core_product_debug_print_control_registers(void)
{
    const core_product_debug_target *target = debug_target();
    if (target != NULL && target->print_control_registers != NULL) target->print_control_registers(target->context);
}

void core_product_debug_print_memory(void)
{
    const core_product_debug_target *target = debug_target();
    if (target != NULL && target->print_memory != NULL) target->print_memory(target->context);
}

void core_product_debug_print_watchpoints(void)
{
    const core_product_debug_target *target = debug_target();
    if (target != NULL && target->print_watchpoints != NULL) target->print_watchpoints(target->context);
}
