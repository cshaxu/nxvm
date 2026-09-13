#ifndef KVM_BASE_MAILBOX_INTERFACE_H
#define KVM_BASE_MAILBOX_INTERFACE_H

#include "lib/kvm-base/frame_interface.h"
#include "lib/kvm-base/mailbox_wake_interface.h"

#include "lib/types/atomic.h"

#define KVM_COMPONENT_CONTROL_CAPACITY 32u
#define KVM_COMPONENT_CONTROL_STORAGE_CAPACITY (KVM_COMPONENT_CONTROL_CAPACITY + 1u)
#define KVM_COMPONENT_WINDOW_TITLE_CAPACITY 128u

typedef enum kvm_component_control_kind {
    KVM_COMPONENT_CONTROL_STOP,
    KVM_COMPONENT_CONTROL_SET_WINDOW_TITLE,
    KVM_COMPONENT_CONTROL_SET_WINDOW_FROZEN,
    KVM_COMPONENT_CONTROL_RELEASE_WINDOW_MOUSE
} kvm_component_control_kind;

typedef struct kvm_component_control {
    kvm_component_control_kind kind;
    union {
        char title[KVM_COMPONENT_WINDOW_TITLE_CAPACITY];
        lib_bool window_frozen;
    } value;
} kvm_component_control;

typedef lib_status (*kvm_mailbox_notify_fn)(void *context);

/* Each KVM leaf owns exactly one of these. It contains two independent
 * mailboxes: a latest-wins copied frame and a FIFO control queue. The native
 * wake object is merely their shared wait primitive, never a third mailbox.
 * Independent locks protect frames and controls. Terminal closure takes
 * frame then control; ordinary control never waits for a frame copy. */
typedef struct kvm_component_mailboxes {
    lib_atomic_flag frame_lock;
    lib_atomic_flag control_lock;
    kvm_frame frame;
    lib_u32 frame_generation;
    lib_bool frame_pending;
    kvm_component_control controls[KVM_COMPONENT_CONTROL_STORAGE_CAPACITY];
    lib_u32 control_head;
    lib_u32 control_count;
    lib_bool closed;
    /* The native leaf may replace the default wait primitive once, during
     * startup, with its own notifier.  It cannot be changed after that. */
    lib_bool notifier_selected;
    kvm_mailbox_wake *wake;
    kvm_mailbox_notify_fn notify;
    void *notify_context;
} kvm_component_mailboxes;

lib_status kvm_component_mailboxes_create(kvm_component_mailboxes *mailboxes);
/* One-time startup selection, before publishing the component to any caller.
 * Replaces the default wait primitive. Context lives until worker join and
 * caller quiescence. Failure is after enqueue: do not replay the request. */
lib_status kvm_component_mailboxes_select_notify(kvm_component_mailboxes *mailboxes,
    kvm_mailbox_notify_fn notify, void *context);
lib_status kvm_component_mailboxes_notify(kvm_component_mailboxes *mailboxes);
void kvm_component_mailboxes_close(kvm_component_mailboxes *mailboxes);
void kvm_component_mailboxes_destroy(kvm_component_mailboxes *mailboxes);
lib_status kvm_component_mailboxes_publish_frame(kvm_component_mailboxes *mailboxes,
    const kvm_frame *frame);
/* Appends a non-empty control batch atomically. A capacity failure leaves the
 * existing FIFO and every requested control unchanged. STOP is terminal and
 * must be submitted as its own one-record batch. */
lib_status kvm_component_mailboxes_enqueue_controls(
    kvm_component_mailboxes *mailboxes, const kvm_component_control *controls,
    lib_u32 control_count);
lib_bool kvm_component_mailboxes_take_control(kvm_component_mailboxes *mailboxes,
    kvm_component_control *out_control);
lib_bool kvm_component_mailboxes_capture_frame(kvm_component_mailboxes *mailboxes,
    lib_u32 *out_generation, kvm_frame *out_frame);
/* Capture leaves the latest frame pending. Acknowledge only after success;
 * acknowledging an older capture never clears a newer publication. */
void kvm_component_mailboxes_acknowledge_frame(kvm_component_mailboxes *mailboxes,
    lib_u32 generation);
kvm_mailbox_wake *kvm_component_mailboxes_wake(
    const kvm_component_mailboxes *mailboxes);

#endif
