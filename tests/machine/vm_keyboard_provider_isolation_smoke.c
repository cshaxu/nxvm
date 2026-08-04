#include "type.h"




#include "core/machine/keyboard_interface.h"

#include "vm/composition/session/lifecycle.h"

C_INT main(C_VOID)
{
    vm_session *first =
        (vm_session *)STD_CALLOC(1u, sizeof(*first));
    vm_session *second =
        (vm_session *)STD_CALLOC(1u, sizeof(*second));

    if (first == STD_NULL || second == STD_NULL) {
        STD_FREE(second);
        STD_FREE(first);
        return 1;
    }

    vm_session_initialize(first);
    vm_session_initialize(second);
    core_machine_keyboard_apply_host_state_to(&first->keyboard_provider,
        NXVM_KEYBOARD_ASYNC_ALT, 0u);
    core_machine_keyboard_apply_host_state_to(&second->keyboard_provider,
        NXVM_KEYBOARD_ASYNC_CONTROL, 0u);

    if (!core_machine_keyboard_get_modifier_from(&first->keyboard_provider,
            CORE_MACHINE_KEYBOARD_MODIFIER_ALT) ||
        core_machine_keyboard_get_modifier_from(&first->keyboard_provider,
            CORE_MACHINE_KEYBOARD_MODIFIER_CONTROL) ||
        core_machine_keyboard_get_modifier_from(&second->keyboard_provider,
            CORE_MACHINE_KEYBOARD_MODIFIER_ALT) ||
        !core_machine_keyboard_get_modifier_from(&second->keyboard_provider,
            CORE_MACHINE_KEYBOARD_MODIFIER_CONTROL)) {
        vm_session_finalize(second);
        vm_session_finalize(first);
        STD_FREE(second);
        STD_FREE(first);
        return 1;
    }

    vm_session_finalize(second);
    vm_session_finalize(first);
    STD_FREE(second);
    STD_FREE(first);
    puts("M5:T76:S1:KEYBOARD-PROVIDER-ISOLATION:OK");
    return 0;
}
