#ifndef CORE_PLATFORM_PRESENTATION_MAILBOX_INTERFACE_H
#define CORE_PLATFORM_PRESENTATION_MAILBOX_INTERFACE_H

#include "type.h"



#include "core/platform/display_frame.h"

typedef struct core_platform_presentation_mailbox core_platform_presentation_mailbox;

type_status core_platform_presentation_mailbox_create(
    core_platform_presentation_mailbox **out_mailbox);
C_VOID core_platform_presentation_mailbox_destroy(
    core_platform_presentation_mailbox *mailbox);
type_status core_platform_presentation_mailbox_publish(
    core_platform_presentation_mailbox *mailbox,
    const core_platform_display_frame *frame);
type_status core_platform_presentation_mailbox_capture(
    const core_platform_presentation_mailbox *mailbox,
    core_platform_display_frame *out_frame);

#endif
