#ifndef KVM_INPUT_INTERFACE_H
#define KVM_INPUT_INTERFACE_H

#include "lib/kvm-base/event_interface.h"
#include "lib/kvm-base/hotkey_interface.h"
#include "lib/base/input_adapter_interface.h"

/* Neutral input delivery. held_keys is the same matcher used by sink; text
 * synthesis borrows that ledger rather than creating another held-key owner. */
lib_i32 kvm_keyboard_submit_input(const kvm_hotkey_matcher *held_keys,
    void *context, kvm_input_sink sink, const lib_key_input *input);

/* Internal native-leaf support: decode once in Base, then use the same neutral
 * entry. Translate Window characters only for UNMAPPED native transitions. */
lib_i32 kvm_keyboard_submit_record(base_keyboard_decoder *state,
    const kvm_hotkey_matcher *held_keys, void *context,
    kvm_input_sink sink, const base_keyboard_record *record);

#endif
