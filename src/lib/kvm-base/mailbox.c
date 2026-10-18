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
    if (mailboxes->frames != LIB_NULL) kvm_frame_exchange_detach(mailboxes->frames);
    mailboxes->frames = LIB_NULL;
    base_sync_event_destroy(mailboxes->wake);
    mailboxes->wake = LIB_NULL;
    base_sync_mutex_destroy(mailboxes->frame_lock);
    mailboxes->frame_lock = LIB_NULL;
    base_sync_mutex_destroy(mailboxes->control_lock);
    mailboxes->control_lock = LIB_NULL;
}

lib_status kvm_component_mailboxes_publish_frame(kvm_component_mailboxes *mailboxes,
    kvm_frame_exchange *frames)
{
    if (mailboxes == LIB_NULL || frames == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    base_sync_mutex_lock(mailboxes->frame_lock);
    if (mailboxes->closed || (mailboxes->frames != LIB_NULL && mailboxes->frames != frames)) {
        base_sync_mutex_unlock(mailboxes->frame_lock);
        return LIB_STATUS_INVALID_STATE;
    }
    mailboxes->frames = frames;
    base_sync_mutex_unlock(mailboxes->frame_lock);
    return LIB_STATUS_OK;
}

lib_status kvm_component_mailboxes_enqueue_control(
    kvm_component_mailboxes *mailboxes, const kvm_component_control *control)
{
    lib_u32 index;
    lib_bool stop;

    if (mailboxes == LIB_NULL || control == LIB_NULL)
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

void kvm_component_mailboxes_redraw_frame(kvm_component_mailboxes *mailboxes)
{
    base_sync_mutex_lock(mailboxes->frame_lock);
    if (!mailboxes->closed && mailboxes->frames != LIB_NULL)
        kvm_frame_exchange_redraw(mailboxes->frames);
    base_sync_mutex_unlock(mailboxes->frame_lock);
}

const void *kvm_component_mailboxes_take_frame(kvm_component_mailboxes *mailboxes,
    const void **previous)
{
    const void *frame;
    if (previous != LIB_NULL) *previous = LIB_NULL;
    if (mailboxes == LIB_NULL) return LIB_NULL;
    base_sync_mutex_lock(mailboxes->frame_lock);
    frame = mailboxes->frames == LIB_NULL ? LIB_NULL :
        kvm_frame_exchange_take(mailboxes->frames, previous);
    base_sync_mutex_unlock(mailboxes->frame_lock);
    return frame;
}

void kvm_component_mailboxes_finish_frame(kvm_component_mailboxes *mailboxes,
    lib_bool success)
{
    if (mailboxes == LIB_NULL) return;
    base_sync_mutex_lock(mailboxes->frame_lock);
    if (mailboxes->frames != LIB_NULL)
        kvm_frame_exchange_finish(mailboxes->frames, success);
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
