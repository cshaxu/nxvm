#include "lib/base/base.h"
#include "lib/ux/mailbox.h"
#include "lib/ux/internal/mailbox_native.h"

struct ux_mailbox {
    atomic_flag lock;
    lib_u32 generation;
    ux_frame frame;
    ux_mailbox_native *native_mailbox;
};

static void ux_mailbox_lock(ux_mailbox *mailbox)
{
    while (atomic_flag_test_and_set_explicit(&mailbox->lock,
        memory_order_acquire)) {}
}

lib_status ux_mailbox_create(ux_mailbox **out_mailbox)
{
    ux_mailbox *mailbox;

    if (out_mailbox == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_mailbox = LIB_NULL;
    mailbox = malloc(sizeof(*mailbox));
    if (mailbox == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    mailbox->lock = (atomic_flag)ATOMIC_FLAG_INIT;
    atomic_flag_clear_explicit(&mailbox->lock, memory_order_release);
    mailbox->generation = 0u;
    mailbox->native_mailbox = ux_mailbox_native_create();
    if (mailbox->native_mailbox == LIB_NULL) {
        free(mailbox);
        return LIB_STATUS_NO_MEMORY;
    }
    memset(&mailbox->frame, 0, sizeof(mailbox->frame));
    *out_mailbox = mailbox;
    return LIB_STATUS_OK;
}

void ux_mailbox_destroy(ux_mailbox *mailbox)
{
    if (mailbox == LIB_NULL) return;
    ux_mailbox_lock(mailbox);
    memset(&mailbox->frame, 0, sizeof(mailbox->frame));
    atomic_flag_clear_explicit(&mailbox->lock, memory_order_release);
    ux_mailbox_native_destroy(mailbox->native_mailbox);
    free(mailbox);
}

lib_status ux_mailbox_publish(ux_mailbox *mailbox, const ux_frame *frame)
{
    if (mailbox == LIB_NULL || !ux_frame_is_valid(frame))
        return LIB_STATUS_INVALID_ARGUMENT;
    ux_mailbox_lock(mailbox);
    mailbox->frame = *frame;
    mailbox->frame.sequence = ++mailbox->generation;
    ux_mailbox_native_signal(mailbox->native_mailbox);
    atomic_flag_clear_explicit(&mailbox->lock, memory_order_release);
    return LIB_STATUS_OK;
}

void ux_mailbox_wake(ux_mailbox *mailbox)
{
    if (mailbox == LIB_NULL) return;
    ux_mailbox_lock(mailbox);
    ux_mailbox_native_signal(mailbox->native_mailbox);
    atomic_flag_clear_explicit(&mailbox->lock, memory_order_release);
}

ux_mailbox_native *ux_mailbox_native_for_mailbox(const ux_mailbox *mailbox)
{
    return mailbox == LIB_NULL ? LIB_NULL : mailbox->native_mailbox;
}

lib_u32 ux_mailbox_generation(const ux_mailbox *mailbox)
{
    ux_mailbox *mutable_mailbox = (ux_mailbox *)mailbox;
    lib_u32 generation = 0u;

    if (mailbox == LIB_NULL) return 0u;
    ux_mailbox_lock(mutable_mailbox);
    generation = mailbox->generation;
    atomic_flag_clear_explicit(&mutable_mailbox->lock, memory_order_release);
    return generation;
}

lib_status ux_mailbox_capture(const ux_mailbox *mailbox, ux_frame *out_frame)
{
    ux_mailbox *mutable_mailbox = (ux_mailbox *)mailbox;
    if (mailbox == LIB_NULL || out_frame == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    ux_mailbox_lock(mutable_mailbox);
    *out_frame = mailbox->frame;
    atomic_flag_clear_explicit(&mutable_mailbox->lock, memory_order_release);
    return LIB_STATUS_OK;
}
