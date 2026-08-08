#include "type.h"
#include "vm/composition/session/session.h"




#include "core/product/debug/debug_target.h"

#include "vm/composition/session/debug_target.h"

#include "vm/composition/session/lifecycle.h"

C_INT main(C_VOID)
{
    vm_session *session;
    const core_product_debug_target *target;
    uint32_t value = 0u;
    uint8_t byte = 0x5au;
    uint8_t linear_byte = 0xa5u;

    session = ((vm_session *)STD_CALLOC(1u, sizeof(vm_session)));
    if (session == STD_NULL) return 1;
    vm_session_initialize(session);
    vm_session_reset(session);
    target = vm_session_debug_target(session);
    if (target == STD_NULL ||
        target->read_register(target->context,
            CORE_PRODUCT_DEBUG_EIP, &value) ||
        target->write_register(target->context,
            CORE_PRODUCT_DEBUG_EAX, 0x12345678u) ||
        target->read_register(target->context,
            CORE_PRODUCT_DEBUG_EAX, &value) || value != 0x12345678u ||
        target->get_code_default_size(target->context) < 0 ||
        target->write_linear(target->context, 0x600u, &linear_byte, 1u) ||
        target->read_linear(target->context, 0x600u, &value, 1u) ||
        (uint8_t)value != linear_byte ||
        target->write_real(target->context, 0u, 0x500u, &byte, 1u) ||
        target->read_real(target->context, 0u, 0x500u, &value, 1u) ||
        (uint8_t)value != byte) {
        vm_session_finalize(session);
        STD_FREE(session);
        return 1;
    }
    target->write_port(target->context, 0x80u, 0u);
    (C_VOID)target->read_port(target->context, 0x80u);
    target->set_watch(target->context, CORE_PRODUCT_DEBUG_WATCH_READ, 0x600u);
    target->clear_watch(target->context, CORE_PRODUCT_DEBUG_WATCH_READ);
    target->set_break_real(target->context, 0xf000u, 0xfff0u);
    target->clear_break(target->context, TYPE_FALSE);
    target->set_trace(target->context, 1u);
    target->clear_trace(target->context);
    vm_session_finalize(session);
    STD_FREE(session);
    puts("M5:T14:S3:VM-DEBUG-TARGET:OK");
    return 0;
}
