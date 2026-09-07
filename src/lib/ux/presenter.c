#include "lib/base/base.h"
#include "lib/ux/internal/mailbox_native.h"
#include "lib/ux/internal/presenter_internal.h"

struct ux_presenter {
    atomic_flag frame_lock;
    atomic_flag target_lock;
    atomic_flag title_lock;
    atomic_flag mouse_capturable_lock;
    atomic_flag mouse_release_lock;
    atomic_flag mouse_state_lock;
    lib_u32 frame_generation;
    lib_u32 target_generation;
    lib_u32 title_generation;
    lib_u32 mouse_capturable_generation;
    lib_bool mouse_capturable;
    lib_bool mouse_release_pending;
    ux_mouse_capture_state mouse_capture_state;
    ux_frame frame;
    ux_target target;
    char title[UX_WINDOW_TITLE_CAPACITY];
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

lib_status ux_presenter_create(ux_presenter **out_presenter)
{
    ux_presenter *presenter;

    if (out_presenter == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_presenter = LIB_NULL;
    presenter = calloc(1u, sizeof(*presenter));
    if (presenter == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    presenter->frame_lock = (atomic_flag)ATOMIC_FLAG_INIT;
    presenter->target_lock = (atomic_flag)ATOMIC_FLAG_INIT;
    presenter->title_lock = (atomic_flag)ATOMIC_FLAG_INIT;
    presenter->mouse_capturable_lock = (atomic_flag)ATOMIC_FLAG_INIT;
    presenter->mouse_release_lock = (atomic_flag)ATOMIC_FLAG_INIT;
    presenter->mouse_state_lock = (atomic_flag)ATOMIC_FLAG_INIT;
    atomic_flag_clear_explicit(&presenter->frame_lock, memory_order_release);
    atomic_flag_clear_explicit(&presenter->target_lock, memory_order_release);
    atomic_flag_clear_explicit(&presenter->title_lock, memory_order_release);
    atomic_flag_clear_explicit(&presenter->mouse_capturable_lock,
        memory_order_release);
    atomic_flag_clear_explicit(&presenter->mouse_release_lock,
        memory_order_release);
    atomic_flag_clear_explicit(&presenter->mouse_state_lock,
        memory_order_release);
    presenter->target_generation = 1u;
    presenter->mouse_capturable = LIB_FALSE;
    presenter->mouse_capture_state = UX_MOUSE_CAPTURE_RELEASED;
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
    if (presenter == LIB_NULL || (target != UX_TARGET_NONE &&
        target != UX_TARGET_CONSOLE && target != UX_TARGET_WINDOW))
        return LIB_STATUS_INVALID_ARGUMENT;
    ux_presenter_lock(&presenter->target_lock);
    presenter->target = target;
    ++presenter->target_generation;
    ux_presenter_unlock(&presenter->target_lock);
    ux_mailbox_native_signal(presenter->native_mailbox);
    return LIB_STATUS_OK;
}

lib_status ux_presenter_set_window_title(ux_presenter *presenter,
    const char *title)
{
    const char *end;
    ux_target target;

    if (presenter == LIB_NULL || title == LIB_NULL || (end = memchr(title, '\0',
            UX_WINDOW_TITLE_CAPACITY)) == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    ux_presenter_lock(&presenter->title_lock);
    memcpy(presenter->title, title, (size_t)(end - title) + 1u);
    ++presenter->title_generation;
    ux_presenter_unlock(&presenter->title_lock);
    (void)ux_presenter_capture_target(presenter, &target);
    if (target == UX_TARGET_WINDOW)
        ux_mailbox_native_signal(presenter->native_mailbox);
    return LIB_STATUS_OK;
}

lib_status ux_presenter_set_mouse_capturable(ux_presenter *presenter,
    lib_bool capturable)
{
    if (presenter == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    ux_presenter_lock(&presenter->mouse_capturable_lock);
    presenter->mouse_capturable = capturable != LIB_FALSE;
    ++presenter->mouse_capturable_generation;
    ux_presenter_unlock(&presenter->mouse_capturable_lock);
    ux_mailbox_native_signal(presenter->native_mailbox);
    return LIB_STATUS_OK;
}

lib_status ux_presenter_release_mouse(ux_presenter *presenter)
{
    if (presenter == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    ux_presenter_lock(&presenter->mouse_release_lock);
    presenter->mouse_release_pending = LIB_TRUE;
    ux_presenter_unlock(&presenter->mouse_release_lock);
    ux_mailbox_native_signal(presenter->native_mailbox);
    return LIB_STATUS_OK;
}

ux_mouse_capture_state ux_presenter_mouse_capture_state(
    const ux_presenter *presenter)
{
    ux_presenter *mutable_presenter = (ux_presenter *)presenter;
    ux_mouse_capture_state state;

    if (presenter == LIB_NULL) return UX_MOUSE_CAPTURE_RELEASED;
    ux_presenter_lock(&mutable_presenter->mouse_state_lock);
    state = presenter->mouse_capture_state;
    ux_presenter_unlock(&mutable_presenter->mouse_state_lock);
    return state;
}

lib_u32 ux_presenter_capture_target(const ux_presenter *presenter,
    ux_target *out_target)
{
    ux_presenter *mutable_presenter = (ux_presenter *)presenter;
    lib_u32 generation;

    if (presenter == LIB_NULL || out_target == LIB_NULL) return 0u;
    ux_presenter_lock(&mutable_presenter->target_lock);
    *out_target = presenter->target;
    generation = presenter->target_generation;
    ux_presenter_unlock(&mutable_presenter->target_lock);
    return generation;
}

lib_u32 ux_presenter_capture_window_title(const ux_presenter *presenter,
    char out_title[UX_WINDOW_TITLE_CAPACITY])
{
    ux_presenter *mutable_presenter = (ux_presenter *)presenter;
    lib_u32 generation;

    if (presenter == LIB_NULL || out_title == LIB_NULL) return 0u;
    ux_presenter_lock(&mutable_presenter->title_lock);
    memcpy(out_title, presenter->title, UX_WINDOW_TITLE_CAPACITY);
    generation = presenter->title_generation;
    ux_presenter_unlock(&mutable_presenter->title_lock);
    return generation;
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

lib_u32 ux_presenter_capture_mouse_capturable(const ux_presenter *presenter,
    lib_bool *out_capturable)
{
    ux_presenter *mutable_presenter = (ux_presenter *)presenter;
    lib_u32 generation;

    if (presenter == LIB_NULL || out_capturable == LIB_NULL) return 0u;
    ux_presenter_lock(&mutable_presenter->mouse_capturable_lock);
    *out_capturable = presenter->mouse_capturable;
    generation = presenter->mouse_capturable_generation;
    ux_presenter_unlock(&mutable_presenter->mouse_capturable_lock);
    return generation;
}

lib_bool ux_presenter_take_mouse_release(ux_presenter *presenter)
{
    lib_bool pending;

    if (presenter == LIB_NULL) return LIB_FALSE;
    ux_presenter_lock(&presenter->mouse_release_lock);
    pending = presenter->mouse_release_pending;
    presenter->mouse_release_pending = LIB_FALSE;
    ux_presenter_unlock(&presenter->mouse_release_lock);
    return pending;
}

void ux_presenter_set_mouse_capture_state(ux_presenter *presenter,
    ux_mouse_capture_state state)
{
    if (presenter == LIB_NULL) return;
    ux_presenter_lock(&presenter->mouse_state_lock);
    presenter->mouse_capture_state = state;
    ux_presenter_unlock(&presenter->mouse_state_lock);
}

ux_mailbox_native *ux_mailbox_native_for_presenter(const ux_presenter *presenter)
{
    return presenter == LIB_NULL ? LIB_NULL : presenter->native_mailbox;
}

lib_status ux_binding_validate(const ux_binding *binding)
{
    return binding == LIB_NULL || binding->presenter == LIB_NULL ||
        binding->actions == LIB_NULL || binding->input_sink == LIB_NULL ||
        binding->get_state == LIB_NULL || binding->release_pressed_keys == LIB_NULL ||
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
    if (!binding->release_pressed_keys(binding->context, binding->input_sink))
        return UX_RUN_ERROR_RESULT;
    return binding->handle_action(binding->context, action, binding->input_sink);
}
