#include "type.h"



#include "core/product/debug/debug_access.h"

#include "core/product/debug/debug_target.h"

typedef struct debug_target_fixture {
    type_unsigned_32 eax;
    STD_SIZE_T break_count;
} debug_target_fixture;

static C_INT debug_target_read_register(C_VOID *context,
                                      core_product_debug_register reg,
                                      type_unsigned_32 *value)
{
    debug_target_fixture *fixture = context;

    if (reg != CORE_PRODUCT_DEBUG_EAX || value == STD_NULL) return 1;
    *value = fixture->eax;
    return 0;
}

static C_INT debug_target_write_register(C_VOID *context,
                                       core_product_debug_register reg,
                                       type_unsigned_32 value)
{
    debug_target_fixture *fixture = context;

    if (reg != CORE_PRODUCT_DEBUG_EAX) return 1;
    fixture->eax = value;
    return 0;
}

static STD_SIZE_T debug_target_get_break_count(C_VOID *context)
{
    return ((debug_target_fixture *)context)->break_count;
}

static C_INT debug_target_is_running(C_VOID *context)
{
    (C_VOID)context;
    return 1;
}

static C_INT debug_target_code_default_size(C_VOID *context)
{
    (C_VOID)context;
    return 16;
}

static type_unsigned_32 debug_target_code_base(C_VOID *context)
{
    (C_VOID)context;
    return 0xf0000u;
}

C_INT main(C_VOID)
{
    debug_target_fixture fixture = {0x12345678u, 7u};
    core_product_debug_target target = {0};
    type_unsigned_32 value = 0u;

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
    if (!core_product_debug_is_running(&target) ||
        core_product_debug_read_register(&target, CORE_PRODUCT_DEBUG_EAX, &value) ||
        value != fixture.eax ||
        core_product_debug_write_register(&target, CORE_PRODUCT_DEBUG_EAX, 0x10203040u) ||
        fixture.eax != 0x10203040u ||
        core_product_debug_get_code_default_size(&target) != 16 ||
        core_product_debug_get_code_base(&target) != 0xf0000u ||
        core_product_debug_get_break_count(&target) != 7u) return 1;
    puts("M5:T14:S3:CORE-PRODUCT-DEBUG-TARGET:OK");
    return 0;
}
