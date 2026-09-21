#ifndef KVM_BASE_MAILBOX_INTERFACE_H
#define KVM_BASE_MAILBOX_INTERFACE_H

#include "lib/base/sync_interface.h"

#define KVM_COMPONENT_CONTROL_CAPACITY 32u
#define KVM_COMPONENT_CONTROL_STORAGE_CAPACITY (KVM_COMPONENT_CONTROL_CAPACITY + 1u)
#define KVM_COMPONENT_CONTROL_PAYLOAD_CAPACITY 128u
#define KVM_COMPONENT_CONTROL_STOP 0u

/* Zero is the terminal transport marker. All other kinds and payload bytes
 * belong to the consumer; initialize unused bytes before enqueueing. */
typedef struct kvm_component_control {
    lib_u32 kind;
    lib_u8 payload[KVM_COMPONENT_CONTROL_PAYLOAD_CAPACITY];
} kvm_component_control;

typedef lib_status (*kvm_mailbox_notify_fn)(void *context);

/* Each KVM leaf owns exactly one of these. It contains two independent
 * mailboxes: a latest-wins copied frame and a FIFO control queue. The native
 * wake object is merely their shared wait primitive, never a third mailbox.
 * Independent locks protect frames and controls. Terminal closure takes
 * frame then control; ordinary control never waits for a frame copy. */
typedef struct kvm_component_mailboxes {
    base_sync_mutex *frame_lock;
    base_sync_mutex *control_lock;
    void *frame;
    lib_size frame_capacity;
    lib_size frame_size;
    lib_u32 frame_generation;
    lib_bool frame_pending;
    kvm_component_control controls[KVM_COMPONENT_CONTROL_STORAGE_CAPACITY];
    lib_u32 control_head;
    lib_u32 control_count;
    lib_bool closed;
    /* Selected once during startup; non-NULL notify means selection succeeded. */
    base_sync_event *wake;
    kvm_mailbox_notify_fn notify;
    void *notify_context;
} kvm_component_mailboxes;

/* Creates independent blocking frame/control locks, without a wake object.
 * Failure leaves an empty, destroyable mailbox; no operation is then valid. */
/* Nonempty storage is consumer-owned, correctly aligned for its own type,
 * and remains alive through worker join. */
lib_status kvm_component_mailboxes_create(kvm_component_mailboxes *mailboxes,
    void *frame_storage, lib_size frame_capacity);
/* One-time startup selection, before publishing the component to any caller.
 * NULL notify creates the default wait primitive; otherwise no wake is allocated.
 * Failure leaves selection unset. Context lives until worker join and caller
 * quiescence. After selection, notification failure is after enqueue: do not replay. */
lib_status kvm_component_mailboxes_select_notify(kvm_component_mailboxes *mailboxes,
    kvm_mailbox_notify_fn notify, void *context);
lib_status kvm_component_mailboxes_notify(kvm_component_mailboxes *mailboxes);
void kvm_component_mailboxes_close(kvm_component_mailboxes *mailboxes);
void kvm_component_mailboxes_destroy(kvm_component_mailboxes *mailboxes);
lib_status kvm_component_mailboxes_publish_frame(kvm_component_mailboxes *mailboxes,
    const void *frame, lib_size bytes);
/* Appends one copied control. Capacity failure leaves the FIFO unchanged.
 * STOP is terminal, idempotent and has one reserved slot. */
lib_status kvm_component_mailboxes_enqueue_control(
    kvm_component_mailboxes *mailboxes, const kvm_component_control *control);
lib_bool kvm_component_mailboxes_take_control(kvm_component_mailboxes *mailboxes,
    kvm_component_control *out_control);
lib_bool kvm_component_mailboxes_capture_frame(kvm_component_mailboxes *mailboxes,
    lib_u32 *out_generation, void *out_frame, lib_size capacity);
/* Capture leaves the latest frame pending. Acknowledge only after success;
 * acknowledging an older capture never clears a newer publication. */
void kvm_component_mailboxes_acknowledge_frame(kvm_component_mailboxes *mailboxes,
    lib_u32 generation);

#endif
