#ifndef NTVDM64_VM_PLATFORM_PRESENTATION_MAILBOX_H
#define NTVDM64_VM_PLATFORM_PRESENTATION_MAILBOX_H

#include <stdatomic.h>

#include "core/platform/display_frame.h"

typedef struct vm_platform_presentation_mailbox {
    atomic_flag lock;
    core_platform_display_frame frame;
} vm_platform_presentation_mailbox;

void vm_platform_presentation_mailbox_initialize(
    vm_platform_presentation_mailbox *mailbox);
void vm_platform_presentation_mailbox_publish(
    vm_platform_presentation_mailbox *mailbox,
    const core_platform_display_frame *frame);
void vm_platform_presentation_mailbox_capture(
    const vm_platform_presentation_mailbox *mailbox,
    core_platform_display_frame *out_frame);

#endif
