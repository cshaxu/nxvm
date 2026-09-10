#ifndef UX_BASE_MAILBOX_H
#define UX_BASE_MAILBOX_H

#include "lib/ux-base/frame_interface.h"
#include "lib/ux-base/mailbox_wake.h"

#include <stdatomic.h>

#define UX_COMPONENT_CONTROL_CAPACITY 32u
#define UX_COMPONENT_CONTROL_STORAGE_CAPACITY (UX_COMPONENT_CONTROL_CAPACITY + 1u)
#define UX_COMPONENT_WINDOW_TITLE_CAPACITY 128u

typedef enum ux_component_control_kind {
    UX_COMPONENT_CONTROL_STOP,
    UX_COMPONENT_CONTROL_SET_WINDOW_TITLE,
    UX_COMPONENT_CONTROL_SET_WINDOW_FROZEN,
    UX_COMPONENT_CONTROL_RELEASE_WINDOW_MOUSE
} ux_component_control_kind;

typedef struct ux_component_control {
    ux_component_control_kind kind;
    union {
        char title[UX_COMPONENT_WINDOW_TITLE_CAPACITY];
        lib_bool window_frozen;
    } value;
} ux_component_control;

/* Each UX leaf owns exactly one of these. It contains two independent
 * mailboxes: a latest-wins copied frame and a FIFO control queue. The native
 * wake object is merely their shared wait primitive, never a third mailbox. */
typedef struct ux_component_mailboxes {
    atomic_flag frame_lock;
    atomic_flag control_lock;
    ux_frame frame;
    lib_u32 frame_generation;
    ux_component_control controls[UX_COMPONENT_CONTROL_STORAGE_CAPACITY];
    lib_u32 control_head;
    lib_u32 control_count;
    lib_bool stop_queued;
    ux_mailbox_wake *wake;
} ux_component_mailboxes;

lib_status ux_component_mailboxes_create(ux_component_mailboxes *mailboxes);
void ux_component_mailboxes_destroy(ux_component_mailboxes *mailboxes);
lib_status ux_component_mailboxes_publish_frame(ux_component_mailboxes *mailboxes,
    const ux_frame *frame);
lib_status ux_component_mailboxes_enqueue_control(
    ux_component_mailboxes *mailboxes, const ux_component_control *control);
/* Appends a non-empty control batch atomically. A capacity failure leaves the
 * existing FIFO and every requested control unchanged. STOP is terminal and
 * must be submitted as its own one-record batch. */
lib_status ux_component_mailboxes_enqueue_controls(
    ux_component_mailboxes *mailboxes, const ux_component_control *controls,
    lib_u32 control_count);
lib_bool ux_component_mailboxes_take_control(ux_component_mailboxes *mailboxes,
    ux_component_control *out_control);
lib_bool ux_component_mailboxes_capture_frame(ux_component_mailboxes *mailboxes,
    lib_u32 *in_out_generation, ux_frame *out_frame);
ux_mailbox_wake *ux_component_mailboxes_wake(
    const ux_component_mailboxes *mailboxes);

#endif
