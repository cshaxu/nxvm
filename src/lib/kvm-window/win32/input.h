#ifndef KVM_WINDOW_INPUT_H
#define KVM_WINDOW_INPUT_H

#include "lib/kvm-base/hotkey_interface.h"

/* Window messages expose no per-record modifier mask, so the Window leaf
 * samples its own native input state before submitting a transition. Chord
 * ownership and matching remain in kvm-base. */
lib_u8 kvm_window_modifiers_from_key_state(void);
lib_u8 kvm_window_keyboard_flags_from_lparam(lib_u64 message_lparam);

#endif
