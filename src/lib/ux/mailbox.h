#ifndef UX_MAILBOX_H
#define UX_MAILBOX_H

#include "type.h"
#include "lib/ux/frame.h"

typedef struct ux_mailbox ux_mailbox;

type_status ux_mailbox_create(ux_mailbox **out_mailbox);
C_VOID ux_mailbox_destroy(ux_mailbox *mailbox);
type_status ux_mailbox_publish(ux_mailbox *mailbox,
    const ux_frame *frame);
type_status ux_mailbox_capture(const ux_mailbox *mailbox,
    ux_frame *out_frame);
type_unsigned_32 ux_mailbox_generation(const ux_mailbox *mailbox);

#endif
