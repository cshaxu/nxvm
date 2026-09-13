#include "lib/kvm-base/mailbox_interface.h"

static void kvm_component_mailboxes_lock(lib_atomic_flag *lock)
{
    while (lib_atomic_flag_test_and_set_explicit(lock, LIB_MEMORY_ORDER_ACQUIRE)) { }
}

static void kvm_component_mailboxes_unlock(lib_atomic_flag *lock)
{
    lib_atomic_flag_clear_explicit(lock, LIB_MEMORY_ORDER_RELEASE);
}

static lib_status kvm_component_notify_waiter(void *context)
{
    return kvm_mailbox_wake_signal(context);
}

lib_status kvm_component_mailboxes_select_notify(kvm_component_mailboxes *mailboxes,
    kvm_mailbox_notify_fn notify, void *context)
{
    if (mailboxes == LIB_NULL || notify == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (mailboxes->notifier_selected) return LIB_STATUS_INVALID_STATE;
    kvm_mailbox_wake_destroy(mailboxes->wake);
    mailboxes->wake = LIB_NULL;
    mailboxes->notify = notify;
    mailboxes->notify_context = context;
    mailboxes->notifier_selected = LIB_TRUE;
    return LIB_STATUS_OK;
}

lib_status kvm_component_mailboxes_notify(kvm_component_mailboxes *mailboxes)
{
    return mailboxes->notify(mailboxes->notify_context);
}

lib_status kvm_component_mailboxes_create(kvm_component_mailboxes *mailboxes)
{
    lib_status status;
    if (mailboxes == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    lib_memory_set(mailboxes, 0, sizeof(*mailboxes));
    lib_atomic_flag_clear(&mailboxes->frame_lock);
    lib_atomic_flag_clear(&mailboxes->control_lock);
    status = kvm_mailbox_wake_create(&mailboxes->wake);
    mailboxes->notify = kvm_component_notify_waiter;
    mailboxes->notify_context = mailboxes->wake;
    return status;
}

void kvm_component_mailboxes_destroy(kvm_component_mailboxes *mailboxes)
{
    if (mailboxes == LIB_NULL) return;
    kvm_mailbox_wake_destroy(mailboxes->wake);
    mailboxes->wake = LIB_NULL;
}

lib_status kvm_component_mailboxes_publish_frame(kvm_component_mailboxes *mailboxes,
    const kvm_frame *frame)
{
    lib_i32 left, top, right, bottom;
    lib_bool full;
    if (mailboxes == LIB_NULL || !kvm_frame_is_valid(frame))
        return LIB_STATUS_INVALID_ARGUMENT;
    kvm_component_mailboxes_lock(&mailboxes->frame_lock);
    if (mailboxes->closed) {
        kvm_component_mailboxes_unlock(&mailboxes->frame_lock);
        return LIB_STATUS_INVALID_STATE;
    }
    full = mailboxes->frame_generation == 0u ||
        mailboxes->frame.graphics != frame->graphics ||
        mailboxes->frame.graphics_width != frame->graphics_width ||
        mailboxes->frame.graphics_height != frame->graphics_height ||
        mailboxes->frame.graphics_stride != frame->graphics_stride ||
        lib_memory_compare(mailboxes->frame.graphics_palette,
            frame->graphics_palette, sizeof(frame->graphics_palette)) != 0;
    left = frame->dirty_left; top = frame->dirty_top;
    right = frame->dirty_right; bottom = frame->dirty_bottom;
    if (frame->graphics && full) {
        left = top = 0u;
        right = frame->graphics_width - 1u;
        bottom = frame->graphics_height - 1u;
    } else if (frame->graphics && mailboxes->frame_pending) {
        if (mailboxes->frame.dirty_left < left) left = mailboxes->frame.dirty_left;
        if (mailboxes->frame.dirty_top < top) top = mailboxes->frame.dirty_top;
        if (mailboxes->frame.dirty_right > right) right = mailboxes->frame.dirty_right;
        if (mailboxes->frame.dirty_bottom > bottom) bottom = mailboxes->frame.dirty_bottom;
    }
    mailboxes->frame = *frame;
    mailboxes->frame.dirty_left = left; mailboxes->frame.dirty_top = top;
    mailboxes->frame.dirty_right = right; mailboxes->frame.dirty_bottom = bottom;
    mailboxes->frame_pending = LIB_TRUE;
    mailboxes->frame.sequence = ++mailboxes->frame_generation;
    kvm_component_mailboxes_unlock(&mailboxes->frame_lock);
    return LIB_STATUS_OK;
}

lib_status kvm_component_mailboxes_enqueue_controls(
    kvm_component_mailboxes *mailboxes, const kvm_component_control *controls,
    lib_u32 control_count)
{
    lib_u32 index;
    lib_u32 control_index;
    lib_u32 ordinary_count = 0u;
    lib_bool includes_stop = LIB_FALSE;

    if (mailboxes == LIB_NULL || controls == LIB_NULL || control_count == 0u)
        return LIB_STATUS_INVALID_ARGUMENT;
    for (control_index = 0u; control_index < control_count; ++control_index) {
        if (controls[control_index].kind > KVM_COMPONENT_CONTROL_RELEASE_WINDOW_MOUSE)
            return LIB_STATUS_INVALID_ARGUMENT;
        if (controls[control_index].kind == KVM_COMPONENT_CONTROL_STOP) {
            if (includes_stop != LIB_FALSE) return LIB_STATUS_INVALID_ARGUMENT;
            includes_stop = LIB_TRUE;
        } else ++ordinary_count;
    }
    if (includes_stop != LIB_FALSE && control_count != 1u)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (includes_stop) kvm_component_mailboxes_lock(&mailboxes->frame_lock);
    kvm_component_mailboxes_lock(&mailboxes->control_lock);
    if (includes_stop != LIB_FALSE) {
        if (mailboxes->closed != LIB_FALSE) {
            kvm_component_mailboxes_unlock(&mailboxes->control_lock);
            kvm_component_mailboxes_unlock(&mailboxes->frame_lock);
            return LIB_STATUS_OK;
        }
        mailboxes->closed = LIB_TRUE;
    } else if (mailboxes->closed != LIB_FALSE || ordinary_count >
        KVM_COMPONENT_CONTROL_CAPACITY - mailboxes->control_count) {
        lib_status status = mailboxes->closed != LIB_FALSE ?
            LIB_STATUS_INVALID_STATE : LIB_STATUS_LIMIT_EXCEEDED;
        kvm_component_mailboxes_unlock(&mailboxes->control_lock);
        return status;
    }
    for (control_index = 0u; control_index < control_count; ++control_index) {
        index = (mailboxes->control_head + mailboxes->control_count) %
            KVM_COMPONENT_CONTROL_STORAGE_CAPACITY;
        mailboxes->controls[index] = controls[control_index];
        ++mailboxes->control_count;
    }
    kvm_component_mailboxes_unlock(&mailboxes->control_lock);
    if (includes_stop) kvm_component_mailboxes_unlock(&mailboxes->frame_lock);
    return LIB_STATUS_OK;
}

lib_bool kvm_component_mailboxes_take_control(kvm_component_mailboxes *mailboxes,
    kvm_component_control *out_control)
{
    if (mailboxes == LIB_NULL || out_control == LIB_NULL) return LIB_FALSE;
    kvm_component_mailboxes_lock(&mailboxes->control_lock);
    if (mailboxes->control_count == 0u) {
        kvm_component_mailboxes_unlock(&mailboxes->control_lock);
        return LIB_FALSE;
    }
    *out_control = mailboxes->controls[mailboxes->control_head];
    mailboxes->control_head = (mailboxes->control_head + 1u) %
        KVM_COMPONENT_CONTROL_STORAGE_CAPACITY;
    --mailboxes->control_count;
    kvm_component_mailboxes_unlock(&mailboxes->control_lock);
    return LIB_TRUE;
}

lib_bool kvm_component_mailboxes_capture_frame(kvm_component_mailboxes *mailboxes,
    lib_u32 *out_generation, kvm_frame *out_frame)
{
    if (mailboxes == LIB_NULL || out_generation == LIB_NULL ||
        out_frame == LIB_NULL) return LIB_FALSE;
    kvm_component_mailboxes_lock(&mailboxes->frame_lock);
    if (!mailboxes->frame_pending) {
        kvm_component_mailboxes_unlock(&mailboxes->frame_lock);
        return LIB_FALSE;
    }
    *out_frame = mailboxes->frame;
    *out_generation = mailboxes->frame_generation;
    kvm_component_mailboxes_unlock(&mailboxes->frame_lock);
    return LIB_TRUE;
}

void kvm_component_mailboxes_acknowledge_frame(kvm_component_mailboxes *mailboxes,
    lib_u32 generation)
{
    if (mailboxes == LIB_NULL) return;
    kvm_component_mailboxes_lock(&mailboxes->frame_lock);
    if (generation == mailboxes->frame_generation)
        mailboxes->frame_pending = LIB_FALSE;
    kvm_component_mailboxes_unlock(&mailboxes->frame_lock);
}

kvm_mailbox_wake *kvm_component_mailboxes_wake(
    const kvm_component_mailboxes *mailboxes)
{
    return mailboxes == LIB_NULL ? LIB_NULL : mailboxes->wake;
}

void kvm_component_mailboxes_close(kvm_component_mailboxes *mailboxes)
{
    kvm_component_mailboxes_lock(&mailboxes->frame_lock);
    kvm_component_mailboxes_lock(&mailboxes->control_lock);
    mailboxes->closed = LIB_TRUE;
    kvm_component_mailboxes_unlock(&mailboxes->control_lock);
    kvm_component_mailboxes_unlock(&mailboxes->frame_lock);
}
