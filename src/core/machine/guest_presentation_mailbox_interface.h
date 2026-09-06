#ifndef CORE_PLATFORM_PRESENTATION_MAILBOX_INTERFACE_H
#define CORE_PLATFORM_PRESENTATION_MAILBOX_INTERFACE_H

#include "type.h"



#include "core/machine/guest_display_frame.h"

typedef struct core_machine_guest_presentation_mailbox core_machine_guest_presentation_mailbox;

type_status core_machine_guest_presentation_mailbox_create(
    core_machine_guest_presentation_mailbox **out_mailbox);
C_VOID core_machine_guest_presentation_mailbox_destroy(
    core_machine_guest_presentation_mailbox *mailbox);
type_status core_machine_guest_presentation_mailbox_publish(
    core_machine_guest_presentation_mailbox *mailbox,
    const core_machine_guest_display_frame *frame);
type_status core_machine_guest_presentation_mailbox_capture(
    const core_machine_guest_presentation_mailbox *mailbox,
    core_machine_guest_display_frame *out_frame);

#endif
