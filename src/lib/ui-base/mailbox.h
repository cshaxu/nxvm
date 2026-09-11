#ifndef UI_BASE_MAILBOX_H
#define UI_BASE_MAILBOX_H

#include "lib/ui-base/frame_interface.h"
#include "lib/ui-base/mailbox_wake.h"

#include "lib/types/atomic.h"

#define UI_COMPONENT_CONTROL_CAPACITY 32u
#define UI_COMPONENT_CONTROL_STORAGE_CAPACITY (UI_COMPONENT_CONTROL_CAPACITY + 1u)
#define UI_COMPONENT_WINDOW_TITLE_CAPACITY 128u

typedef enum ui_component_control_kind {
    UI_COMPONENT_CONTROL_STOP,
    UI_COMPONENT_CONTROL_SET_WINDOW_TITLE,
    UI_COMPONENT_CONTROL_SET_WINDOW_FROZEN,
    UI_COMPONENT_CONTROL_RELEASE_WINDOW_MOUSE
} ui_component_control_kind;

typedef struct ui_component_control {
    ui_component_control_kind kind;
    union {
        char title[UI_COMPONENT_WINDOW_TITLE_CAPACITY];
        lib_bool window_frozen;
    } value;
} ui_component_control;

/* Each UI leaf owns exactly one of these. It contains two independent
 * mailboxes: a latest-wins copied frame and a FIFO control queue. The native
 * wake object is merely their shared wait primitive, never a third mailbox. */
typedef struct ui_component_mailboxes {
    lib_atomic_flag frame_lock;
    lib_atomic_flag control_lock;
    ui_frame frame;
    lib_u32 frame_generation;
    ui_component_control controls[UI_COMPONENT_CONTROL_STORAGE_CAPACITY];
    lib_u32 control_head;
    lib_u32 control_count;
    lib_bool stop_queued;
    ui_mailbox_wake *wake;
} ui_component_mailboxes;

lib_status ui_component_mailboxes_create(ui_component_mailboxes *mailboxes);
void ui_component_mailboxes_destroy(ui_component_mailboxes *mailboxes);
lib_status ui_component_mailboxes_publish_frame(ui_component_mailboxes *mailboxes,
    const ui_frame *frame);
lib_status ui_component_mailboxes_enqueue_control(
    ui_component_mailboxes *mailboxes, const ui_component_control *control);
/* Appends a non-empty control batch atomically. A capacity failure leaves the
 * existing FIFO and every requested control unchanged. STOP is terminal and
 * must be submitted as its own one-record batch. */
lib_status ui_component_mailboxes_enqueue_controls(
    ui_component_mailboxes *mailboxes, const ui_component_control *controls,
    lib_u32 control_count);
lib_bool ui_component_mailboxes_take_control(ui_component_mailboxes *mailboxes,
    ui_component_control *out_control);
lib_bool ui_component_mailboxes_capture_frame(ui_component_mailboxes *mailboxes,
    lib_u32 *in_out_generation, ui_frame *out_frame);
ui_mailbox_wake *ui_component_mailboxes_wake(
    const ui_component_mailboxes *mailboxes);

#endif
