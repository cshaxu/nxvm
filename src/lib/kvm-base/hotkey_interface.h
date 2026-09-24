#ifndef KVM_BASE_HOTKEY_INTERFACE_H
#define KVM_BASE_HOTKEY_INTERFACE_H

#include "lib/kvm-base/event_interface.h"

#define KVM_HOTKEY_CAPACITY 16u

enum {
    KVM_HOTKEY_MODIFIER_CONTROL = KVM_KEY_MODIFIER_CONTROL,
    KVM_HOTKEY_MODIFIER_ALT = KVM_KEY_MODIFIER_ALT,
    KVM_HOTKEY_MODIFIER_SHIFT = KVM_KEY_MODIFIER_SHIFT,
    KVM_HOTKEY_KEY_CONTROL = KVM_KEY_CONTROL,
    KVM_HOTKEY_KEY_ALT = KVM_KEY_ALT,
    KVM_HOTKEY_KEY_SHIFT = KVM_KEY_SHIFT
};

typedef struct kvm_hotkey_registration {
    kvm_key key;
    lib_u8 modifiers;
    lib_u8 identifier[KVM_HOTKEY_IDENTIFIER_CAPACITY];
} kvm_hotkey_registration;

typedef struct kvm_hotkey_registry {
    kvm_hotkey_registration entries[KVM_HOTKEY_CAPACITY];
    lib_u32 count;
} kvm_hotkey_registry;

typedef enum kvm_hotkey_key_state {
    KVM_HOTKEY_PENDING,
    KVM_HOTKEY_DELIVERED,
    KVM_HOTKEY_CONSUMED
} kvm_hotkey_key_state;

typedef struct kvm_hotkey_held_key {
    kvm_input_event make;
    kvm_hotkey_key_state state;
    lib_bool allow_replay;
} kvm_hotkey_held_key;

typedef struct kvm_hotkey_matcher {
    kvm_hotkey_registry registry;
    /* In make order; every held physical key has exactly one disposition. */
    kvm_hotkey_held_key *held;
    lib_size held_count;
    lib_size held_capacity;
    lib_bool failed;
} kvm_hotkey_matcher;

void kvm_hotkey_registry_initialize(kvm_hotkey_registry *registry);
lib_status kvm_hotkey_registry_register(kvm_hotkey_registry *registry,
    kvm_key key, lib_u8 modifiers, const char *identifier);
void kvm_hotkey_matcher_initialize(kvm_hotkey_matcher *matcher,
    const kvm_hotkey_registry *registry);
/* Emits ordinary events and matched KVM_EVENT_HOTKEY values through sink.
 * Repeats retain their original disposition; delivered makes retain breaks.
 * Failure is terminal until discard; no partial replay is retried.
 * allow_replay is captured on the first make, not refreshed by repeats.
 * False suppresses only delayed ordinary make replay, never chord matching
 * or later releases. Initialize once; discard releases held storage. */
lib_bool kvm_hotkey_matcher_submit(kvm_hotkey_matcher *matcher,
    const kvm_input_event *event, kvm_input_sink sink, void *context,
    lib_bool allow_replay);
void kvm_hotkey_matcher_discard(kvm_hotkey_matcher *matcher);
/* Borrowed until the next submit/discard. Includes either physical side of a
 * modifier; synthesis must not release a key already owned by this ledger. */
const kvm_input_event *kvm_hotkey_matcher_held_key(
    const kvm_hotkey_matcher *matcher, kvm_key key);

#endif
