#include "type.h"
#include "vm/composition/session/session_private.h"




#include "core/product/debug/debug_target.h"

#include "core/machine/machine.h"

#include "vm/composition/session/debug_target.h"

#include "vm/composition/session/lifecycle.h"
#include "../support/rom/session_assets.h"

C_INT main(C_VOID)
{
    vm_session *session;
    const core_product_debug_target *target;
    type_unsigned_32 value = 0u;
    type_unsigned_32 before_eax = 0u;
    type_unsigned_8 byte = 0x5au;
    type_unsigned_8 linear_byte = 0xa5u;

    if (vm_test_default_pc_at_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
        session == STD_NULL) return 1;
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
        (type_unsigned_8)value != linear_byte ||
        target->write_real(target->context, 0u, 0x500u, &byte, 1u) ||
        target->read_real(target->context, 0u, 0x500u, &value, 1u) ||
        (type_unsigned_8)value != byte) {
        vm_session_destroy(session);
        return 1;
    }
    for (C_INT register_id = CORE_PRODUCT_DEBUG_EAX;
         register_id <= CORE_PRODUCT_DEBUG_CR4; ++register_id) {
        core_product_debug_register product_register =
            (core_product_debug_register)register_id;

        if (target->read_register(target->context, product_register, &value) ||
            target->write_register(target->context, product_register, value)) {
            vm_session_destroy(session);
            return 1;
        }
    }
    if (target->read_register(target->context, CORE_PRODUCT_DEBUG_EAX,
            &before_eax) || !target->read_register(target->context,
            (core_product_debug_register)99, &value) ||
        !target->write_register(target->context,
            (core_product_debug_register)99, 0xffffffffu) ||
        target->read_register(target->context, CORE_PRODUCT_DEBUG_EAX,
            &value) || value != before_eax) {
        vm_session_destroy(session);
        return 1;
    }
    target->write_port(target->context, 0x80u, 0u);
    (C_VOID)target->read_port(target->context, 0x80u);
    target->set_watch(target->context, CORE_PRODUCT_DEBUG_WATCH_READ, 0x600u);
    if (!session->core_machine->executor_cpu_instructions.data.flagWR ||
        session->core_machine->executor_cpu_instructions.data.wrLinear != 0x600u) {
        vm_session_destroy(session);
        return 1;
    }
    target->set_watch(target->context, (core_product_debug_watch_kind)99, 0x700u);
    target->clear_watch(target->context, (core_product_debug_watch_kind)99);
    if (!session->core_machine->executor_cpu_instructions.data.flagWR ||
        session->core_machine->executor_cpu_instructions.data.wrLinear != 0x600u) {
        vm_session_destroy(session);
        return 1;
    }
    target->clear_watch(target->context, CORE_PRODUCT_DEBUG_WATCH_READ);
    if (session->core_machine->executor_cpu_instructions.data.flagWR) {
        vm_session_finalize(session);
        STD_FREE(session);
        return 1;
    }
    target->set_watch(target->context, CORE_PRODUCT_DEBUG_WATCH_WRITE, 0x700u);
    target->set_watch(target->context, CORE_PRODUCT_DEBUG_WATCH_EXECUTE, 0x800u);
    if (!session->core_machine->executor_cpu_instructions.data.flagWW ||
        !session->core_machine->executor_cpu_instructions.data.flagWE ||
        session->core_machine->executor_cpu_instructions.data.wwLinear != 0x700u ||
        session->core_machine->executor_cpu_instructions.data.weLinear != 0x800u) {
        vm_session_finalize(session);
        STD_FREE(session);
        return 1;
    }
    target->clear_watch(target->context, CORE_PRODUCT_DEBUG_WATCH_WRITE);
    target->clear_watch(target->context, CORE_PRODUCT_DEBUG_WATCH_EXECUTE);
    target->set_break_real(target->context, 0xf000u, 0xfff0u);
    target->clear_break(target->context, TYPE_FALSE);
    target->set_trace(target->context, 1u);
    target->clear_trace(target->context);
    vm_session_destroy(session);
    puts("M5:T313:S6:DEBUG-MAPPING:OK");
    return 0;
}
