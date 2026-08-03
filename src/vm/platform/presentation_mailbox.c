#include "vm/platform/presentation_mailbox.h"

#include <string.h>

#include "type.h"

static C_VOID vm_platform_presentation_mailbox_lock(
    vm_platform_presentation_mailbox *mailbox)
{
    while (STD_ATOMIC_FLAG_TEST_AND_SET_EXPLICIT(&mailbox->lock,
                                             STD_MEMORY_ORDER_ACQUIRE)) {}
}

C_VOID vm_platform_presentation_mailbox_initialize(
    vm_platform_presentation_mailbox *mailbox)
{
    if (mailbox == STD_NULL) return;
    STD_ATOMIC_FLAG_CLEAR_EXPLICIT(&mailbox->lock, STD_MEMORY_ORDER_RELEASE);
    STD_MEMSET(&mailbox->frame, 0, sizeof(mailbox->frame));
    mailbox->frame.columns = CORE_PLATFORM_DISPLAY_MAX_COLUMNS;
    mailbox->frame.rows = CORE_PLATFORM_DISPLAY_MAX_ROWS;
}

C_VOID vm_platform_presentation_mailbox_publish(
    vm_platform_presentation_mailbox *mailbox,
    const core_platform_display_frame *frame)
{
    if (mailbox == STD_NULL || frame == STD_NULL) return;
    vm_platform_presentation_mailbox_lock(mailbox);
    mailbox->frame = *frame;
    STD_ATOMIC_FLAG_CLEAR_EXPLICIT(&mailbox->lock, STD_MEMORY_ORDER_RELEASE);
}

C_VOID vm_platform_presentation_mailbox_capture(
    const vm_platform_presentation_mailbox *mailbox,
    core_platform_display_frame *out_frame)
{
    vm_platform_presentation_mailbox *mutable_mailbox =
        (vm_platform_presentation_mailbox *)mailbox;
    if (mailbox == STD_NULL || out_frame == STD_NULL) return;
    vm_platform_presentation_mailbox_lock(mutable_mailbox);
    *out_frame = mailbox->frame;
    STD_ATOMIC_FLAG_CLEAR_EXPLICIT(&mutable_mailbox->lock, STD_MEMORY_ORDER_RELEASE);
}
