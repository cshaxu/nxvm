#include <stdio.h>

#include "core/product/debug/debug_access.h"
#include "core/product/debug/debug_target.h"

typedef struct debug_target_fixture {
    uint32_t eax;
    size_t break_count;
} debug_target_fixture;

static int debug_target_read_register(void *context,
                                      core_product_debug_register reg,
                                      uint32_t *value)
{
    debug_target_fixture *fixture = context;

    if (reg != CORE_PRODUCT_DEBUG_EAX || value == NULL) return 1;
    *value = fixture->eax;
    return 0;
}

static int debug_target_write_register(void *context,
                                       core_product_debug_register reg,
                                       uint32_t value)
{
    debug_target_fixture *fixture = context;

    if (reg != CORE_PRODUCT_DEBUG_EAX) return 1;
    fixture->eax = value;
    return 0;
}

static size_t debug_target_get_break_count(void *context)
{
    return ((debug_target_fixture *)context)->break_count;
}

static int debug_target_is_running(void *context)
{
    (void)context;
    return 1;
}

static int debug_target_code_default_size(void *context)
{
    (void)context;
    return 16;
}

static uint32_t debug_target_code_base(void *context)
{
    (void)context;
    return 0xf0000u;
}

int main(void)
{
    debug_target_fixture fixture = {0x12345678u, 7u};
    core_product_debug_target target = {0};
    uint32_t value = 0u;

    target.is_running = debug_target_is_running;
    target.read_register = debug_target_read_register;
    target.write_register = debug_target_write_register;
    target.get_code_default_size = debug_target_code_default_size;
    target.get_code_base = debug_target_code_base;
    target.get_break_count = debug_target_get_break_count;
    target.context = &fixture;
    if (target.read_register(target.context, CORE_PRODUCT_DEBUG_EAX, &value) ||
        value != 0x12345678u ||
        target.write_register(target.context, CORE_PRODUCT_DEBUG_EAX, 0x87654321u) ||
        fixture.eax != 0x87654321u ||
        target.get_break_count(target.context) != 7u) return 1;
    core_product_debug_bind_target(&target);
    if (!core_product_debug_is_running() ||
        core_product_debug_read_register(CORE_PRODUCT_DEBUG_EAX, &value) ||
        value != fixture.eax ||
        core_product_debug_write_register(CORE_PRODUCT_DEBUG_EAX, 0x10203040u) ||
        fixture.eax != 0x10203040u ||
        core_product_debug_get_code_default_size() != 16 ||
        core_product_debug_get_code_base() != 0xf0000u ||
        core_product_debug_get_break_count() != 7u) return 1;
    core_product_debug_bind_target(NULL);
    puts("M5:T14:S3:CORE-PRODUCT-DEBUG-TARGET:OK");
    return 0;
}
