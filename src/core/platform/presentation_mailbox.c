#include "core/platform/presentation_mailbox_interface.h"


#include "type.h"

static C_VOID core_platform_presentation_mailbox_lock(
    core_platform_presentation_mailbox *mailbox)
{
    while (STD_ATOMIC_FLAG_TEST_AND_SET_EXPLICIT(&mailbox->lock,
                                             STD_MEMORY_ORDER_ACQUIRE)) {}
}

C_VOID core_platform_presentation_mailbox_initialize(
    core_platform_presentation_mailbox *mailbox)
{
    if (mailbox == STD_NULL) return;
    mailbox->lock = (STD_ATOMIC_FLAG)ATOMIC_FLAG_INIT;
    STD_ATOMIC_FLAG_CLEAR_EXPLICIT(&mailbox->lock, STD_MEMORY_ORDER_RELEASE);
    mailbox->active = TYPE_TRUE;
    STD_MEMSET(&mailbox->frame, 0, sizeof(mailbox->frame));
    mailbox->frame.columns = CORE_PLATFORM_DISPLAY_MAX_COLUMNS;
    mailbox->frame.rows = CORE_PLATFORM_DISPLAY_MAX_ROWS;
}

type_status core_platform_presentation_mailbox_publish(
    core_platform_presentation_mailbox *mailbox,
    const core_platform_display_frame *frame)
{
    type_status status = TYPE_STATUS_INVALID_STATE;

    if (mailbox == STD_NULL || frame == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    core_platform_presentation_mailbox_lock(mailbox);
    if (mailbox->active) {
        mailbox->frame = *frame;
        status = TYPE_STATUS_OK;
    }
    STD_ATOMIC_FLAG_CLEAR_EXPLICIT(&mailbox->lock, STD_MEMORY_ORDER_RELEASE);
    return status;
}

C_VOID core_platform_presentation_mailbox_finalize(
    core_platform_presentation_mailbox *mailbox)
{
    if (mailbox == STD_NULL) return;
    core_platform_presentation_mailbox_lock(mailbox);
    mailbox->active = TYPE_FALSE;
    STD_MEMSET(&mailbox->frame, 0, sizeof(mailbox->frame));
    STD_ATOMIC_FLAG_CLEAR_EXPLICIT(&mailbox->lock, STD_MEMORY_ORDER_RELEASE);
}

type_status core_platform_presentation_mailbox_capture(
    const core_platform_presentation_mailbox *mailbox,
    core_platform_display_frame *out_frame)
{
    core_platform_presentation_mailbox *mutable_mailbox =
        (core_platform_presentation_mailbox *)mailbox;
    type_status status = TYPE_STATUS_INVALID_STATE;

    if (mailbox == STD_NULL || out_frame == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    core_platform_presentation_mailbox_lock(mutable_mailbox);
    if (mailbox->active) {
        *out_frame = mailbox->frame;
        status = TYPE_STATUS_OK;
    }
    STD_ATOMIC_FLAG_CLEAR_EXPLICIT(&mutable_mailbox->lock, STD_MEMORY_ORDER_RELEASE);
    return status;
}
