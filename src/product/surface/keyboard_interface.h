#ifndef PRODUCT_SURFACE_KEYBOARD_H
#define PRODUCT_SURFACE_KEYBOARD_H

#include "emulator/product/monitor_interface.h"
#include "emulator/session/session_interface.h"
#include "lib/kvm-base/event_interface.h"
#include "lib/kvm-base/hotkey_interface.h"

/* Product hotkey registration and neutral key-sequence policy. */
lib_bool product_surface_keyboard_deliver_input(void *context,
    const kvm_input_event *event);
lib_bool product_surface_keyboard_hotkeys(kvm_hotkey_registry *registry);
emulator_product_help_map product_surface_keyboard_hotkey_help(void);
lib_bool product_surface_keyboard_release_ctrl_alt(void *context, kvm_input_sink sink);
lib_bool product_surface_keyboard_submit_ctrl_alt_del(void *context, kvm_input_sink sink);

lib_bool product_surface_keyboard_handle_hotkey(emulator_machine *, emulator_session_machine_state,
    const lib_u8 *, emulator_session_command_result *);

#endif
