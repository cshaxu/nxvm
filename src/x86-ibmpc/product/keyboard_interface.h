#ifndef X86_IBMPC_PRODUCT_KEYBOARD_H
#define X86_IBMPC_PRODUCT_KEYBOARD_H

#include "x86-ibmpc/product/command_interface.h"
#include "emulator/session/session_interface.h"
#include "lib/kvm-base/event_interface.h"
#include "lib/kvm-base/hotkey_interface.h"

/* Product hotkey registration and neutral key-sequence policy. */
lib_bool x86_ibmpc_product_keyboard_deliver_input(void *context,
    const kvm_input_event *event);
lib_status x86_ibmpc_product_keyboard_configure(x86_ibmpc_product_command_context *command);
lib_bool x86_ibmpc_product_keyboard_release_ctrl_alt(void *context, kvm_input_sink sink);
lib_bool x86_ibmpc_product_keyboard_submit_ctrl_alt_del(void *context, kvm_input_sink sink);

lib_bool x86_ibmpc_product_keyboard_handle_hotkey(emulator_machine *, emulator_session_machine_state,
    const lib_u8 *, emulator_session_command_result *);

#endif
