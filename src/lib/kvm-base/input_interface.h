#ifndef KVM_INPUT_INTERFACE_H
#define KVM_INPUT_INTERFACE_H

/* Leaf-support contract for copied platform records, not an application ABI.
 * Raw key values are decoded by the selected component-local implementation.
 * Callers own one zero-initialized normalizer per input source. */

#include "lib/kvm-base/event_interface.h"
#include "lib/kvm-base/hotkey_interface.h"

/* Copied adapter flags, never a raw platform control-state word. */
enum {
    KVM_INPUT_MODIFIER_CONTROL = 0x01u,
    KVM_INPUT_MODIFIER_ALT = 0x02u,
    KVM_INPUT_MODIFIER_SHIFT = 0x04u,
    KVM_INPUT_FLAG_EXTENDED = 0x01u
};

/* Common normalization owns state and delivery. Platform decoding owns layout
 * and scan recovery. A platform without a physical text mapping emits TEXT. */

typedef struct kvm_keyboard_normalizer {
    lib_u16 pending_high_surrogate;
    lib_u16 pending_repeat_count;
} kvm_keyboard_normalizer;

/* Native adapters copy either separate transition/character messages or one
 * combined record. Translate a separate native transition into characters only
 * on KVM_KEYBOARD_UNMAPPED; accepted physical keys must not generate a second
 * character record. CHARACTER is independent text, not a physical echo.
 * held_keys is the same matcher used by sink, NULL only for isolated decoding. */
enum { KVM_KEYBOARD_REJECTED = 0, KVM_KEYBOARD_ACCEPTED = 1, KVM_KEYBOARD_UNMAPPED = 2 };
typedef enum kvm_keyboard_record_kind {
    KVM_KEYBOARD_TRANSITION,
    KVM_KEYBOARD_CHARACTER,
    KVM_KEYBOARD_COMBINED
} kvm_keyboard_record_kind;

typedef struct kvm_keyboard_record {
    kvm_keyboard_record_kind kind;
    lib_u16 scan;
    lib_u16 key;
    lib_u16 utf16;
    lib_u8 flags;
    lib_u8 modifiers;
    lib_bool pressed;
    /* Zero means one. Decode complete characters before expanding repeats.
     * Physical breaks are single. Surrogate halves must have equal counts. */
    lib_u16 repeat_count;
} kvm_keyboard_record;

lib_i32 kvm_keyboard_submit_record(kvm_keyboard_normalizer *state,
    const kvm_hotkey_matcher *held_keys, void *context,
    kvm_input_sink sink, const kvm_keyboard_record *record);

#endif
