#include "lib/base/base.h"
#include "lib/ux/internal/mailbox_native.h"
#include "lib/ux/internal/presenter_internal.h"

struct ux_presenter {
    atomic_flag frame_lock;
    atomic_flag control_lock;
    lib_u32 frame_generation;
    ux_frame frame;
    ux_presenter_control controls[UX_PRESENTER_CONTROL_CAPACITY];
    lib_u32 control_head;
    lib_u32 control_count;
    ux_mailbox_native *native_mailbox;
};

static void ux_presenter_lock(atomic_flag *lock)
{
    while (atomic_flag_test_and_set_explicit(lock, memory_order_acquire)) {}
}

static void ux_presenter_unlock(atomic_flag *lock)
{
    atomic_flag_clear_explicit(lock, memory_order_release);
}

static lib_status ux_presenter_enqueue(ux_presenter *presenter,
    const ux_presenter_control *control)
{
    lib_u32 tail;

    if (presenter == LIB_NULL || control == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    ux_presenter_lock(&presenter->control_lock);
    if (presenter->control_count == UX_PRESENTER_CONTROL_CAPACITY) {
        ux_presenter_unlock(&presenter->control_lock);
        return LIB_STATUS_INVALID_STATE;
    }
    tail = (presenter->control_head + presenter->control_count) %
        UX_PRESENTER_CONTROL_CAPACITY;
    presenter->controls[tail] = *control;
    ++presenter->control_count;
    ux_presenter_unlock(&presenter->control_lock);
    ux_mailbox_native_signal(presenter->native_mailbox);
    return LIB_STATUS_OK;
}

lib_status ux_presenter_create(ux_presenter **out_presenter)
{
    ux_presenter *presenter;

    if (out_presenter == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_presenter = LIB_NULL;
    presenter = calloc(1u, sizeof(*presenter));
    if (presenter == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    presenter->frame_lock = (atomic_flag)ATOMIC_FLAG_INIT;
    presenter->control_lock = (atomic_flag)ATOMIC_FLAG_INIT;
    atomic_flag_clear_explicit(&presenter->frame_lock, memory_order_release);
    atomic_flag_clear_explicit(&presenter->control_lock, memory_order_release);
    presenter->native_mailbox = ux_mailbox_native_create();
    if (presenter->native_mailbox == LIB_NULL) {
        free(presenter);
        return LIB_STATUS_NO_MEMORY;
    }
    *out_presenter = presenter;
    return LIB_STATUS_OK;
}

void ux_presenter_destroy(ux_presenter *presenter)
{
    if (presenter == LIB_NULL) return;
    ux_mailbox_native_destroy(presenter->native_mailbox);
    free(presenter);
}

lib_status ux_presenter_publish_frame(ux_presenter *presenter,
    const ux_frame *frame)
{
    if (presenter == LIB_NULL || !ux_frame_is_valid(frame))
        return LIB_STATUS_INVALID_ARGUMENT;
    ux_presenter_lock(&presenter->frame_lock);
    presenter->frame = *frame;
    presenter->frame.sequence = ++presenter->frame_generation;
    ux_presenter_unlock(&presenter->frame_lock);
    ux_mailbox_native_signal(presenter->native_mailbox);
    return LIB_STATUS_OK;
}

lib_status ux_presenter_set_target(ux_presenter *presenter, ux_target target)
{
    ux_presenter_control control = { 0 };

    if (target != UX_TARGET_CONSOLE && target != UX_TARGET_WINDOW)
        return LIB_STATUS_INVALID_ARGUMENT;
    control.kind = UX_PRESENTER_CONTROL_TARGET;
    control.target = target;
    return ux_presenter_enqueue(presenter, &control);
}

lib_status ux_presenter_set_window_title(ux_presenter *presenter,
    const char *title)
{
    ux_presenter_control control = { 0 };
    const char *end;

    if (title == LIB_NULL || (end = memchr(title, '\0',
            UX_WINDOW_TITLE_CAPACITY)) == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    control.kind = UX_PRESENTER_CONTROL_WINDOW_TITLE;
    memcpy(control.title, title, (size_t)(end - title) + 1u);
    return ux_presenter_enqueue(presenter, &control);
}

lib_status ux_presenter_stop(ux_presenter *presenter)
{
    const ux_presenter_control control = { UX_PRESENTER_CONTROL_STOP,
        UX_TARGET_CONSOLE, { 0 } };

    return ux_presenter_enqueue(presenter, &control);
}

lib_bool ux_presenter_take_control(ux_presenter *presenter,
    ux_presenter_control *out_control)
{
    lib_bool more;

    if (presenter == LIB_NULL || out_control == LIB_NULL) return LIB_FALSE;
    ux_presenter_lock(&presenter->control_lock);
    if (presenter->control_count == 0u) {
        ux_presenter_unlock(&presenter->control_lock);
        return LIB_FALSE;
    }
    *out_control = presenter->controls[presenter->control_head];
    presenter->control_head = (presenter->control_head + 1u) %
        UX_PRESENTER_CONTROL_CAPACITY;
    --presenter->control_count;
    more = presenter->control_count != 0u;
    ux_presenter_unlock(&presenter->control_lock);
    if (more) ux_mailbox_native_signal(presenter->native_mailbox);
    return LIB_TRUE;
}

lib_status ux_presenter_capture_frame(const ux_presenter *presenter,
    ux_frame *out_frame)
{
    ux_presenter *mutable_presenter = (ux_presenter *)presenter;

    if (presenter == LIB_NULL || out_frame == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    ux_presenter_lock(&mutable_presenter->frame_lock);
    *out_frame = presenter->frame;
    ux_presenter_unlock(&mutable_presenter->frame_lock);
    return LIB_STATUS_OK;
}

lib_u32 ux_presenter_frame_generation(const ux_presenter *presenter)
{
    ux_presenter *mutable_presenter = (ux_presenter *)presenter;
    lib_u32 generation;

    if (presenter == LIB_NULL) return 0u;
    ux_presenter_lock(&mutable_presenter->frame_lock);
    generation = presenter->frame_generation;
    ux_presenter_unlock(&mutable_presenter->frame_lock);
    return generation;
}

ux_mailbox_native *ux_mailbox_native_for_presenter(const ux_presenter *presenter)
{
    return presenter == LIB_NULL ? LIB_NULL : presenter->native_mailbox;
}

lib_status ux_binding_validate(const ux_binding *binding)
{
    return binding == LIB_NULL || binding->presenter == LIB_NULL ||
        binding->actions == LIB_NULL || binding->input_sink == LIB_NULL ||
        binding->get_state == LIB_NULL || binding->release_inputs == LIB_NULL ||
        binding->handle_action == LIB_NULL || binding->handle_close == LIB_NULL ||
        binding->window_initial_title[0] == '\0' || memchr(
            binding->window_initial_title, '\0',
            sizeof(binding->window_initial_title)) == LIB_NULL ?
        LIB_STATUS_INVALID_ARGUMENT : LIB_STATUS_OK;
}

ux_run_result ux_binding_invoke_action(const ux_binding *binding,
    ux_action action)
{
    if (ux_binding_validate(binding) != LIB_STATUS_OK || action == UX_ACTION_NONE)
        return UX_RUN_ERROR_RESULT;
    if (!binding->release_inputs(binding->context, binding->input_sink))
        return UX_RUN_ERROR_RESULT;
    return binding->handle_action(binding->context, action, binding->input_sink);
}
