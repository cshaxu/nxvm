#ifndef KVM_EVENT_INTERFACE_H
#define KVM_EVENT_INTERFACE_H

#include "lib/base/input_interface.h"

/* Product-neutral input emitted by the presentation library. Keys and PC
 * set-1 scan identities follow base/input_interface.h, not native OS encodings.
 * Text is a Unicode scalar not represented by a physical transition. */
typedef enum kvm_event_type {
    KVM_EVENT_KEY,
    KVM_EVENT_TEXT,
    KVM_EVENT_MOUSE,
    KVM_EVENT_HOTKEY,
    KVM_EVENT_WINDOW_CLOSE,
    /* This source can no longer observe balanced host releases. It carries no
     * guest input; the receiving session releases only keys it recorded from
     * this source. */
    KVM_EVENT_INPUT_RESET,
    /* A raw component is permanently retiring. This is its final asynchronous
     * lifetime fact; the application clears pressed state by source_identity
     * and must not dereference the borrowed source handle. */
    KVM_EVENT_SOURCE_RETIRED
} kvm_event_type;

#define KVM_HOTKEY_IDENTIFIER_CAPACITY 64u

typedef struct kvm_input_event {
    /* Borrowed opaque component handle. It is for lifetime tracing only;
     * product action policy never branches on input source. */
    const void *source;
    /* Monotonic instance identity.  Unlike the borrowed address above this
     * value remains unambiguous after the component has been retired and its
     * storage can be reused.  Consumers use it only for lifetime hygiene. */
    lib_u64 source_identity;
    kvm_event_type type;
    union {
        struct {
            lib_u16 scan_code;
            /* A Base-defined key identity. Zero means absent. */
            base_key key;
            /* Platform-neutral physical-key facts. */
            lib_u32 flags;
            /* Current Ctrl/Alt/Shift state for the generic matcher. */
            lib_u8 modifiers;
            lib_bool pressed;
        } key;
        struct {
            lib_u32 scalar;
        } text;
        struct {
            lib_i32 delta_x;
            lib_i32 delta_y;
            lib_i32 absolute_x;
            lib_i32 absolute_y;
            lib_i32 wheel_x;
            lib_i32 wheel_y;
            lib_u32 buttons;
            lib_bool relative;
        } mouse;
        struct {
            lib_u8 identifier[KVM_HOTKEY_IDENTIFIER_CAPACITY];
        } hotkey;
    } data;
} kvm_input_event;

typedef lib_bool (*kvm_input_sink)(void *context, const kvm_input_event *event);

static inline void kvm_input_event_set_source(kvm_input_event *event,
    const void *source, lib_u64 source_identity)
{
    if (event != LIB_NULL) {
        event->source = source;
        event->source_identity = source_identity;
    }
}

#endif
