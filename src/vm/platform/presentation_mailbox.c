#include "vm/platform/presentation_mailbox.h"

#include <string.h>

#include "type.h"

static C_VOID vm_platform_presentation_mailbox_lock(
    vm_platform_presentation_mailbox *mailbox)
{
    while (atomic_flag_test_and_set_explicit(&mailbox->lock,
                                             memory_order_acquire)) {}
}

C_VOID vm_platform_presentation_mailbox_initialize(
    vm_platform_presentation_mailbox *mailbox)
{
    if (mailbox == NULL) return;
    atomic_flag_clear_explicit(&mailbox->lock, memory_order_release);
    STD_MEMSET(&mailbox->frame, 0, sizeof(mailbox->frame));
    mailbox->frame.columns = CORE_PLATFORM_DISPLAY_MAX_COLUMNS;
    mailbox->frame.rows = CORE_PLATFORM_DISPLAY_MAX_ROWS;
}

C_VOID vm_platform_presentation_mailbox_publish(
    vm_platform_presentation_mailbox *mailbox,
    const core_platform_display_frame *frame)
{
    if (mailbox == NULL || frame == NULL) return;
    vm_platform_presentation_mailbox_lock(mailbox);
    mailbox->frame = *frame;
    atomic_flag_clear_explicit(&mailbox->lock, memory_order_release);
}

C_VOID vm_platform_presentation_mailbox_capture(
    const vm_platform_presentation_mailbox *mailbox,
    core_platform_display_frame *out_frame)
{
    vm_platform_presentation_mailbox *mutable_mailbox =
        (vm_platform_presentation_mailbox *)mailbox;
    if (mailbox == NULL || out_frame == NULL) return;
    vm_platform_presentation_mailbox_lock(mutable_mailbox);
    *out_frame = mailbox->frame;
    atomic_flag_clear_explicit(&mutable_mailbox->lock, memory_order_release);
}
