#include "type.h"




#include "core/machine/keyboard_interface.h"

#include "vm/composition/session/lifecycle.h"

C_INT main(C_VOID)
{
    vm_session *session =
        (vm_session *)STD_CALLOC(1u, sizeof(*session));
    if (session == STD_NULL) return 1;
    vm_session_initialize(session);
    core_machine_keyboard_apply_host_state_to(&session->keyboard_provider,
        NXVM_KEYBOARD_ASYNC_ALT, 0u);
    if (!core_machine_keyboard_get_modifier_from(&session->keyboard_provider,
        CORE_MACHINE_KEYBOARD_MODIFIER_ALT)) {
        vm_session_finalize(session);
        STD_FREE(session);
        return 1;
    }
    vm_session_finalize(session);
    STD_FREE(session);
    puts("M5:T41:S1:KEYBOARD-FIRMWARE:OK");
    return 0;
}
