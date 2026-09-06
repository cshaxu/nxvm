#ifndef UX_MAILBOX_H
#define UX_MAILBOX_H

#include "lib/base/base.h"
#include "lib/ux/frame.h"

typedef struct ux_mailbox ux_mailbox;

lib_status ux_mailbox_create(ux_mailbox **out_mailbox);
void ux_mailbox_destroy(ux_mailbox *mailbox);
lib_status ux_mailbox_publish(ux_mailbox *mailbox,
    const ux_frame *frame);
/* Wakes a native loop for a lifecycle transition without publishing a frame. */
void ux_mailbox_wake(ux_mailbox *mailbox);
lib_status ux_mailbox_capture(const ux_mailbox *mailbox,
    ux_frame *out_frame);
lib_u32 ux_mailbox_generation(const ux_mailbox *mailbox);

#endif
