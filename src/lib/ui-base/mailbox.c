#include "lib/ui-base/mailbox_interface.h"

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
    lib_memory_set(mailboxes, 0, sizeof(*mailboxes));
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
    lib_i32 left, top, right, bottom;
    lib_bool full;
    if (mailboxes == LIB_NULL || !ui_frame_is_valid(frame))
        return LIB_STATUS_INVALID_ARGUMENT;
    ui_component_mailboxes_lock(&mailboxes->frame_lock);
    if (mailboxes->closed) {
        ui_component_mailboxes_unlock(&mailboxes->frame_lock);
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
    if (includes_stop) ui_component_mailboxes_lock(&mailboxes->frame_lock);
    ui_component_mailboxes_lock(&mailboxes->control_lock);
    if (includes_stop != LIB_FALSE) {
        if (mailboxes->closed != LIB_FALSE) {
            ui_component_mailboxes_unlock(&mailboxes->control_lock);
            ui_component_mailboxes_unlock(&mailboxes->frame_lock);
            return LIB_STATUS_OK;
        }
        mailboxes->closed = LIB_TRUE;
    } else if (mailboxes->closed != LIB_FALSE || ordinary_count >
        UI_COMPONENT_CONTROL_CAPACITY - mailboxes->control_count) {
        lib_status status = mailboxes->closed != LIB_FALSE ?
            LIB_STATUS_INVALID_STATE : LIB_STATUS_LIMIT_EXCEEDED;
        ui_component_mailboxes_unlock(&mailboxes->control_lock);
        return status;
    }
    for (control_index = 0u; control_index < control_count; ++control_index) {
        index = (mailboxes->control_head + mailboxes->control_count) %
            UI_COMPONENT_CONTROL_STORAGE_CAPACITY;
        mailboxes->controls[index] = controls[control_index];
        ++mailboxes->control_count;
    }
    ui_component_mailboxes_unlock(&mailboxes->control_lock);
    if (includes_stop) ui_component_mailboxes_unlock(&mailboxes->frame_lock);
    ui_mailbox_wake_signal(mailboxes->wake);
    return LIB_STATUS_OK;
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
    lib_u32 *out_generation, ui_frame *out_frame)
{
    if (mailboxes == LIB_NULL || out_generation == LIB_NULL ||
        out_frame == LIB_NULL) return LIB_FALSE;
    ui_component_mailboxes_lock(&mailboxes->frame_lock);
    if (!mailboxes->frame_pending) {
        ui_component_mailboxes_unlock(&mailboxes->frame_lock);
        return LIB_FALSE;
    }
    *out_frame = mailboxes->frame;
    *out_generation = mailboxes->frame_generation;
    ui_component_mailboxes_unlock(&mailboxes->frame_lock);
    return LIB_TRUE;
}

void ui_component_mailboxes_acknowledge_frame(ui_component_mailboxes *mailboxes,
    lib_u32 generation)
{
    if (mailboxes == LIB_NULL) return;
    ui_component_mailboxes_lock(&mailboxes->frame_lock);
    if (generation == mailboxes->frame_generation)
        mailboxes->frame_pending = LIB_FALSE;
    ui_component_mailboxes_unlock(&mailboxes->frame_lock);
}

ui_mailbox_wake *ui_component_mailboxes_wake(
    const ui_component_mailboxes *mailboxes)
{
    return mailboxes == LIB_NULL ? LIB_NULL : mailboxes->wake;
}

void ui_component_mailboxes_close(ui_component_mailboxes *mailboxes)
{
    ui_component_mailboxes_lock(&mailboxes->frame_lock);
    ui_component_mailboxes_lock(&mailboxes->control_lock);
    mailboxes->closed = LIB_TRUE;
    ui_component_mailboxes_unlock(&mailboxes->control_lock);
    ui_component_mailboxes_unlock(&mailboxes->frame_lock);
}
