#include "type.h"




#include "core/product/debug/debug_target.h"

#include "vm/composition/debug_target.h"

#include "vm/composition/session_lifecycle.h"

C_INT main(C_VOID)
{
    vm_session *session;
    const core_product_debug_target *target;
    uint32_t value = 0u;
    uint8_t byte = 0x5au;

    session = (vm_session *)STD_CALLOC(1u, sizeof(*session));
    if (session == STD_NULL) return 1;
    vm_session_initialize(session);
    target = vm_session_debug_target(session);
    if (target == STD_NULL || core_product_debug_scope_target() != STD_NULL ||
        target->read_register(target->context,
            CORE_PRODUCT_DEBUG_EIP, &value) ||
        target->write_real(target->context, 0u, 0x500u, &byte, 1u) ||
        target->read_real(target->context, 0u, 0x500u, &value, 1u) ||
        (uint8_t)value != byte) {
        vm_session_finalize(session);
        STD_FREE(session);
        return 1;
    }
    vm_session_finalize(session);
    STD_FREE(session);
    if (core_product_debug_scope_target() != STD_NULL) return 1;
    puts("M5:T14:S3:VM-DEBUG-TARGET:OK");
    return 0;
}
