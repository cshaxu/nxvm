#ifndef APP_KEYBOARD_H
#define APP_KEYBOARD_H

#include "common/session/session_interface.h"
#include "lib/kvm-base/event_interface.h"
#include "lib/kvm-base/hotkey_interface.h"

/* Product hotkey registration and neutral key-sequence policy. */
lib_bool app_keyboard_deliver_input(void *context,
    const kvm_input_event *event);
lib_bool app_keyboard_hotkeys(kvm_hotkey_registry *registry);
lib_bool app_keyboard_release_ctrl_alt(void *context, kvm_input_sink sink);
lib_bool app_keyboard_submit_ctrl_alt_del(void *context, kvm_input_sink sink);

lib_bool app_keyboard_handle_hotkey(common_machine *, common_session_machine_state,
    const lib_u8 *, common_session_command_result *);

#endif
