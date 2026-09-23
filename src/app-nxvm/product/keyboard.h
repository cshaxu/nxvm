#ifndef VM_APP_KEYBOARD_H
#define VM_APP_KEYBOARD_H
#include "lib/types/types_interface.h"

#include "type.h"

#include "common/session/session_interface.h"
#include "lib/kvm-base/hotkey_interface.h"

typedef struct vm_machine vm_machine;

/* NXVM product key policy: registrations and guest key sequences. */
C_VOID vm_app_keyboard_register_hotkeys(kvm_hotkey_registry *registry);
lib_bool vm_app_keyboard_handle_hotkey(vm_machine *machine,
    common_session_machine_state state, const C_CHAR *identifier,
    common_session_command_result *result);

#endif
