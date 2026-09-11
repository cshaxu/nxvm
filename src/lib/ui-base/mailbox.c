#include "lib/ui-base/mailbox.h"

#include <string.h>

static void ui_component_mailboxes_lock(lib_atomic_flag *lock)
{
    while (lib_atomic_flag_test_and_set_explicit(lock, LIB_MEMORY_ORDER_ACQUIRE)) { }
}

static void ui_component_mailboxes_unlock(lib_atomic_flag *lock)
{
    lib_atomic_flag_clear_explicit(lock, LIB_MEMORY_ORDER_RELEASE);
}

lib_status ui_component_mailboxes_create(ui_component_mailboxes *mailboxes)
{
    if (mailboxes == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    memset(mailboxes, 0, sizeof(*mailboxes));
    lib_atomic_flag_clear(&mailboxes->frame_lock);
    lib_atomic_flag_clear(&mailboxes->control_lock);
    mailboxes->wake = ui_mailbox_wake_create();
    return mailboxes->wake == LIB_NULL ? LIB_STATUS_NO_MEMORY : LIB_STATUS_OK;
}

void ui_component_mailboxes_destroy(ui_component_mailboxes *mailboxes)
{
    if (mailboxes == LIB_NULL) return;
    ui_mailbox_wake_destroy(mailboxes->wake);
    mailboxes->wake = LIB_NULL;
}

lib_status ui_component_mailboxes_publish_frame(ui_component_mailboxes *mailboxes,
    const ui_frame *frame)
{
    if (mailboxes == LIB_NULL || !ui_frame_is_valid(frame))
        return LIB_STATUS_INVALID_ARGUMENT;
    ui_component_mailboxes_lock(&mailboxes->frame_lock);
    mailboxes->frame = *frame;
    mailboxes->frame.sequence = ++mailboxes->frame_generation;
    ui_component_mailboxes_unlock(&mailboxes->frame_lock);
    ui_mailbox_wake_signal(mailboxes->wake);
    return LIB_STATUS_OK;
}

lib_status ui_component_mailboxes_enqueue_controls(
    ui_component_mailboxes *mailboxes, const ui_component_control *controls,
    lib_u32 control_count)
{
    lib_u32 index;
    lib_u32 control_index;
    lib_u32 ordinary_count = 0u;
    lib_bool includes_stop = LIB_FALSE;

    if (mailboxes == LIB_NULL || controls == LIB_NULL || control_count == 0u)
        return LIB_STATUS_INVALID_ARGUMENT;
    for (control_index = 0u; control_index < control_count; ++control_index) {
        if (controls[control_index].kind > UI_COMPONENT_CONTROL_RELEASE_WINDOW_MOUSE)
            return LIB_STATUS_INVALID_ARGUMENT;
        if (controls[control_index].kind == UI_COMPONENT_CONTROL_STOP) {
            if (includes_stop != LIB_FALSE) return LIB_STATUS_INVALID_ARGUMENT;
            includes_stop = LIB_TRUE;
        } else ++ordinary_count;
    }
    if (includes_stop != LIB_FALSE && control_count != 1u)
        return LIB_STATUS_INVALID_ARGUMENT;
    ui_component_mailboxes_lock(&mailboxes->control_lock);
    if (includes_stop != LIB_FALSE) {
        if (mailboxes->stop_queued != LIB_FALSE) {
            ui_component_mailboxes_unlock(&mailboxes->control_lock);
            return LIB_STATUS_OK;
        }
        mailboxes->stop_queued = LIB_TRUE;
    } else if (mailboxes->stop_queued != LIB_FALSE || ordinary_count >
        UI_COMPONENT_CONTROL_CAPACITY - mailboxes->control_count) {
        ui_component_mailboxes_unlock(&mailboxes->control_lock);
        return mailboxes->stop_queued != LIB_FALSE ? LIB_STATUS_INVALID_STATE :
            LIB_STATUS_LIMIT_EXCEEDED;
    }
    for (control_index = 0u; control_index < control_count; ++control_index) {
        index = (mailboxes->control_head + mailboxes->control_count) %
            UI_COMPONENT_CONTROL_STORAGE_CAPACITY;
        mailboxes->controls[index] = controls[control_index];
        ++mailboxes->control_count;
    }
    ui_component_mailboxes_unlock(&mailboxes->control_lock);
    ui_mailbox_wake_signal(mailboxes->wake);
    return LIB_STATUS_OK;
}

lib_status ui_component_mailboxes_enqueue_control(
    ui_component_mailboxes *mailboxes, const ui_component_control *control)
{
    return ui_component_mailboxes_enqueue_controls(mailboxes, control, 1u);
}

lib_bool ui_component_mailboxes_take_control(ui_component_mailboxes *mailboxes,
    ui_component_control *out_control)
{
    if (mailboxes == LIB_NULL || out_control == LIB_NULL) return LIB_FALSE;
    ui_component_mailboxes_lock(&mailboxes->control_lock);
    if (mailboxes->control_count == 0u) {
        ui_component_mailboxes_unlock(&mailboxes->control_lock);
        return LIB_FALSE;
    }
    *out_control = mailboxes->controls[mailboxes->control_head];
    mailboxes->control_head = (mailboxes->control_head + 1u) %
        UI_COMPONENT_CONTROL_STORAGE_CAPACITY;
    --mailboxes->control_count;
    ui_component_mailboxes_unlock(&mailboxes->control_lock);
    return LIB_TRUE;
}

lib_bool ui_component_mailboxes_capture_frame(ui_component_mailboxes *mailboxes,
    lib_u32 *in_out_generation, ui_frame *out_frame)
{
    if (mailboxes == LIB_NULL || in_out_generation == LIB_NULL ||
        out_frame == LIB_NULL) return LIB_FALSE;
    ui_component_mailboxes_lock(&mailboxes->frame_lock);
    if (*in_out_generation == mailboxes->frame_generation) {
        ui_component_mailboxes_unlock(&mailboxes->frame_lock);
        return LIB_FALSE;
    }
    *out_frame = mailboxes->frame;
    *in_out_generation = mailboxes->frame_generation;
    ui_component_mailboxes_unlock(&mailboxes->frame_lock);
    return LIB_TRUE;
}

ui_mailbox_wake *ui_component_mailboxes_wake(
    const ui_component_mailboxes *mailboxes)
{
    return mailboxes == LIB_NULL ? LIB_NULL : mailboxes->wake;
}
