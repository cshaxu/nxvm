#include "lib/kvm-base/mailbox_interface.h"

static lib_status kvm_component_notify_waiter(void *context)
{
    return base_sync_event_signal(context);
}

lib_status kvm_component_mailboxes_select_notify(kvm_component_mailboxes *mailboxes,
    kvm_mailbox_notify_fn notify, void *context)
{
    if (mailboxes == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (mailboxes->notify != LIB_NULL) return LIB_STATUS_INVALID_STATE;
    if (notify == LIB_NULL) {
        lib_status status = base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET,
            &mailboxes->wake);
        if (status != LIB_STATUS_OK) return status;
        notify = kvm_component_notify_waiter;
        context = mailboxes->wake;
    }
    mailboxes->notify = notify;
    mailboxes->notify_context = context;
    return LIB_STATUS_OK;
}

lib_status kvm_component_mailboxes_notify(kvm_component_mailboxes *mailboxes)
{
    return mailboxes->notify == LIB_NULL ? LIB_STATUS_INVALID_STATE :
        mailboxes->notify(mailboxes->notify_context);
}

lib_status kvm_component_mailboxes_create(kvm_component_mailboxes *mailboxes)
{
    lib_status status;
    if (mailboxes == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    lib_memory_set(mailboxes, 0, sizeof(*mailboxes));
    status = base_sync_mutex_create(&mailboxes->frame_lock);
    if (status == LIB_STATUS_OK) status = base_sync_mutex_create(&mailboxes->control_lock);
    if (status != LIB_STATUS_OK) kvm_component_mailboxes_destroy(mailboxes);
    return status;
}

void kvm_component_mailboxes_destroy(kvm_component_mailboxes *mailboxes)
{
    if (mailboxes == LIB_NULL) return;
    base_sync_event_destroy(mailboxes->wake);
    mailboxes->wake = LIB_NULL;
    base_sync_mutex_destroy(mailboxes->frame_lock);
    mailboxes->frame_lock = LIB_NULL;
    base_sync_mutex_destroy(mailboxes->control_lock);
    mailboxes->control_lock = LIB_NULL;
}

lib_status kvm_component_mailboxes_publish_frame(kvm_component_mailboxes *mailboxes,
    const kvm_frame *frame)
{
    lib_i32 left, top, right, bottom;
    lib_bool full;
    if (mailboxes == LIB_NULL || !kvm_frame_is_valid(frame))
        return LIB_STATUS_INVALID_ARGUMENT;
    base_sync_mutex_lock(mailboxes->frame_lock);
    if (mailboxes->closed) {
        base_sync_mutex_unlock(mailboxes->frame_lock);
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
    base_sync_mutex_unlock(mailboxes->frame_lock);
    return LIB_STATUS_OK;
}

lib_status kvm_component_mailboxes_enqueue_control(
    kvm_component_mailboxes *mailboxes, const kvm_component_control *control)
{
    lib_u32 index;
    lib_bool stop;

    if (mailboxes == LIB_NULL || control == LIB_NULL ||
        control->kind > KVM_COMPONENT_CONTROL_RELEASE_WINDOW_MOUSE)
        return LIB_STATUS_INVALID_ARGUMENT;
    stop = control->kind == KVM_COMPONENT_CONTROL_STOP;
    if (stop) base_sync_mutex_lock(mailboxes->frame_lock);
    base_sync_mutex_lock(mailboxes->control_lock);
    if (stop) {
        if (mailboxes->closed != LIB_FALSE) {
            base_sync_mutex_unlock(mailboxes->control_lock);
            base_sync_mutex_unlock(mailboxes->frame_lock);
            return LIB_STATUS_OK;
        }
        mailboxes->closed = LIB_TRUE;
    } else if (mailboxes->closed != LIB_FALSE ||
        mailboxes->control_count == KVM_COMPONENT_CONTROL_CAPACITY) {
        lib_status status = mailboxes->closed != LIB_FALSE ?
            LIB_STATUS_INVALID_STATE : LIB_STATUS_LIMIT_EXCEEDED;
        base_sync_mutex_unlock(mailboxes->control_lock);
        return status;
    }
    index = (mailboxes->control_head + mailboxes->control_count) %
        KVM_COMPONENT_CONTROL_STORAGE_CAPACITY;
    mailboxes->controls[index] = *control;
    ++mailboxes->control_count;
    base_sync_mutex_unlock(mailboxes->control_lock);
    if (stop) base_sync_mutex_unlock(mailboxes->frame_lock);
    return LIB_STATUS_OK;
}

lib_bool kvm_component_mailboxes_take_control(kvm_component_mailboxes *mailboxes,
    kvm_component_control *out_control)
{
    if (mailboxes == LIB_NULL || out_control == LIB_NULL) return LIB_FALSE;
    base_sync_mutex_lock(mailboxes->control_lock);
    if (mailboxes->control_count == 0u) {
        base_sync_mutex_unlock(mailboxes->control_lock);
        return LIB_FALSE;
    }
    *out_control = mailboxes->controls[mailboxes->control_head];
    mailboxes->control_head = (mailboxes->control_head + 1u) %
        KVM_COMPONENT_CONTROL_STORAGE_CAPACITY;
    --mailboxes->control_count;
    base_sync_mutex_unlock(mailboxes->control_lock);
    return LIB_TRUE;
}

lib_bool kvm_component_mailboxes_capture_frame(kvm_component_mailboxes *mailboxes,
    lib_u32 *out_generation, kvm_frame *out_frame)
{
    if (mailboxes == LIB_NULL || out_generation == LIB_NULL ||
        out_frame == LIB_NULL) return LIB_FALSE;
    base_sync_mutex_lock(mailboxes->frame_lock);
    if (!mailboxes->frame_pending) {
        base_sync_mutex_unlock(mailboxes->frame_lock);
        return LIB_FALSE;
    }
    *out_frame = mailboxes->frame;
    *out_generation = mailboxes->frame_generation;
    base_sync_mutex_unlock(mailboxes->frame_lock);
    return LIB_TRUE;
}

void kvm_component_mailboxes_acknowledge_frame(kvm_component_mailboxes *mailboxes,
    lib_u32 generation)
{
    if (mailboxes == LIB_NULL) return;
    base_sync_mutex_lock(mailboxes->frame_lock);
    if (generation == mailboxes->frame_generation)
        mailboxes->frame_pending = LIB_FALSE;
    base_sync_mutex_unlock(mailboxes->frame_lock);
}

void kvm_component_mailboxes_close(kvm_component_mailboxes *mailboxes)
{
    base_sync_mutex_lock(mailboxes->frame_lock);
    base_sync_mutex_lock(mailboxes->control_lock);
    mailboxes->closed = LIB_TRUE;
    base_sync_mutex_unlock(mailboxes->control_lock);
    base_sync_mutex_unlock(mailboxes->frame_lock);
}
