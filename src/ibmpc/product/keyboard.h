#ifndef VM_APP_KEYBOARD_H
#define VM_APP_KEYBOARD_H
#include "lib/types/types_interface.h"


#include "common/session/session_interface.h"
#include "lib/kvm-base/hotkey_interface.h"

#include "common/machine/machine_interface.h"

/* NXVM product key policy: registrations and guest key sequences. */
void vm_app_keyboard_register_hotkeys(kvm_hotkey_registry *registry);
lib_bool vm_app_keyboard_handle_hotkey(common_machine *machine,
    common_session_machine_state state, const lib_u8 *identifier,
    common_session_command_result *result);

#endif
