#ifndef CORE_PLATFORM_PRESENTATION_MAILBOX_INTERFACE_H
#define CORE_PLATFORM_PRESENTATION_MAILBOX_INTERFACE_H

#include "type.h"



#include "core/platform/display_frame.h"

typedef struct core_platform_presentation_mailbox {
    STD_ATOMIC_FLAG lock;
    C_INT active;
    core_platform_display_frame frame;
} core_platform_presentation_mailbox;

C_VOID core_platform_presentation_mailbox_initialize(
    core_platform_presentation_mailbox *mailbox);
C_VOID core_platform_presentation_mailbox_finalize(
    core_platform_presentation_mailbox *mailbox);
type_status core_platform_presentation_mailbox_publish(
    core_platform_presentation_mailbox *mailbox,
    const core_platform_display_frame *frame);
type_status core_platform_presentation_mailbox_capture(
    const core_platform_presentation_mailbox *mailbox,
    core_platform_display_frame *out_frame);

#endif
