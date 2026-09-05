#include "lib/ux/mailbox.h"

struct ux_mailbox {
    STD_ATOMIC_FLAG lock;
    type_bool active;
    type_unsigned_32 generation;
    ux_frame frame;
};

static C_VOID ux_mailbox_lock(ux_mailbox *mailbox)
{
    while (STD_ATOMIC_FLAG_TEST_AND_SET_EXPLICIT(&mailbox->lock,
        STD_MEMORY_ORDER_ACQUIRE)) {}
}

type_status ux_mailbox_create(ux_mailbox **out_mailbox)
{
    ux_mailbox *mailbox;

    if (out_mailbox == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_mailbox = STD_NULL;
    mailbox = STD_MALLOC(sizeof(*mailbox));
    if (mailbox == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    mailbox->lock = (STD_ATOMIC_FLAG)ATOMIC_FLAG_INIT;
    STD_ATOMIC_FLAG_CLEAR_EXPLICIT(&mailbox->lock, STD_MEMORY_ORDER_RELEASE);
    mailbox->active = TYPE_TRUE;
    mailbox->generation = 0u;
    STD_MEMSET(&mailbox->frame, 0, sizeof(mailbox->frame));
    *out_mailbox = mailbox;
    return TYPE_STATUS_OK;
}

C_VOID ux_mailbox_destroy(ux_mailbox *mailbox)
{
    if (mailbox == STD_NULL) return;
    ux_mailbox_lock(mailbox);
    mailbox->active = TYPE_FALSE;
    STD_MEMSET(&mailbox->frame, 0, sizeof(mailbox->frame));
    STD_ATOMIC_FLAG_CLEAR_EXPLICIT(&mailbox->lock, STD_MEMORY_ORDER_RELEASE);
    STD_FREE(mailbox);
}

type_status ux_mailbox_publish(ux_mailbox *mailbox, const ux_frame *frame)
{
    type_status status = TYPE_STATUS_INVALID_STATE;

    if (mailbox == STD_NULL || frame == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    ux_mailbox_lock(mailbox);
    if (mailbox->active) {
        mailbox->frame = *frame;
        mailbox->frame.sequence = ++mailbox->generation;
        status = TYPE_STATUS_OK;
    }
    STD_ATOMIC_FLAG_CLEAR_EXPLICIT(&mailbox->lock, STD_MEMORY_ORDER_RELEASE);
    return status;
}

type_unsigned_32 ux_mailbox_generation(const ux_mailbox *mailbox)
{
    ux_mailbox *mutable_mailbox = (ux_mailbox *)mailbox;
    type_unsigned_32 generation = 0u;

    if (mailbox == STD_NULL) return 0u;
    ux_mailbox_lock(mutable_mailbox);
    if (mailbox->active) generation = mailbox->generation;
    STD_ATOMIC_FLAG_CLEAR_EXPLICIT(&mutable_mailbox->lock, STD_MEMORY_ORDER_RELEASE);
    return generation;
}

type_status ux_mailbox_capture(const ux_mailbox *mailbox, ux_frame *out_frame)
{
    ux_mailbox *mutable_mailbox = (ux_mailbox *)mailbox;
    type_status status = TYPE_STATUS_INVALID_STATE;

    if (mailbox == STD_NULL || out_frame == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    ux_mailbox_lock(mutable_mailbox);
    if (mailbox->active) {
        *out_frame = mailbox->frame;
        status = TYPE_STATUS_OK;
    }
    STD_ATOMIC_FLAG_CLEAR_EXPLICIT(&mutable_mailbox->lock, STD_MEMORY_ORDER_RELEASE);
    return status;
}
