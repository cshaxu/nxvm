#include "lib/kvm-window/window.h"

static lib_status kvm_window_component_stop(kvm_component *base, lib_u32 timeout_ms)
{ return kvm_window_worker_join((kvm_window *)base, timeout_ms); }

static void kvm_window_component_dispose(kvm_component *base)
{
    kvm_window *window = (kvm_window *)base;
    kvm_component_mailboxes_destroy(&window->base.mailboxes);
    lib_release(window);
}

static lib_status kvm_window_enqueue(kvm_window *window,
    kvm_component_control control)
{
    return window == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        kvm_component_enqueue_control(&window->base, &control);
}

lib_status kvm_window_create(kvm_window **out_window,
    const kvm_window_options *options)
{
    kvm_window *window;
    lib_status status;

    if (out_window == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_window = LIB_NULL;
    if (options == LIB_NULL ||
        options->initial_title == LIB_NULL ||
        lib_memory_find(options->initial_title, '\0', KVM_WINDOW_TITLE_CAPACITY) == LIB_NULL ||
        options->component.input_sink == LIB_NULL ||
        options->component.failure_sink == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    window = lib_allocate_zero(1u, sizeof(*window));
    if (window == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    lib_memory_copy(window->initial_title, options->initial_title,
        lib_text_length(options->initial_title) + 1u);
    window->initial_frozen = options->initial_frozen != LIB_FALSE;
    status = kvm_component_initialize(&window->base, &options->component,
        kvm_window_component_stop, kvm_window_component_dispose,
        &window->pending_frame, sizeof(window->pending_frame));
    if (status == LIB_STATUS_OK) status = kvm_window_worker_start(window);
    if (status != LIB_STATUS_OK) {
        /* The worker start path has either joined its failed worker or has
         * entered the process-terminal failure path at the application edge.
         * A normal create failure never exposes a half-created Window. */
        if (window->worker_state == LIB_NULL)
            kvm_window_component_dispose(&window->base);
        return status;
    }
    *out_window = window;
    return LIB_STATUS_OK;
}

lib_status kvm_window_publish_frame(kvm_window *window, const kvm_window_frame *frame)
{
    lib_status status;
    if (window == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = kvm_window_frame_validate(frame);
    if (status != LIB_STATUS_OK) return status;
    return kvm_component_publish_frame(&window->base, frame,
        kvm_window_frame_size_bytes(frame));
}

lib_status kvm_window_destroy(kvm_window *window)
{
    return window == LIB_NULL ? LIB_STATUS_OK : kvm_component_destroy(&window->base);
}

lib_status kvm_window_set_title(kvm_window *window, const char *title)
{
    kvm_component_control control = {
        .kind = KVM_WINDOW_CONTROL_SET_TITLE
    };
    if (title == LIB_NULL || lib_memory_find(title, '\0',
            KVM_WINDOW_TITLE_CAPACITY) == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    lib_memory_copy(control.payload, title, lib_text_length(title) + 1u);
    return kvm_window_enqueue(window, control);
}

lib_status kvm_window_unfreeze(kvm_window *window)
{
    kvm_component_control control = {
        .kind = KVM_WINDOW_CONTROL_SET_FROZEN
    };
    control.payload[0] = LIB_FALSE;
    return kvm_window_enqueue(window, control);
}

lib_status kvm_window_freeze(kvm_window *window)
{
    kvm_component_control control = {
        .kind = KVM_WINDOW_CONTROL_SET_FROZEN
    };
    control.payload[0] = LIB_TRUE;
    return kvm_window_enqueue(window, control);
}

lib_status kvm_window_release_mouse(kvm_window *window)
{
    kvm_component_control control = {
        .kind = KVM_WINDOW_CONTROL_RELEASE_MOUSE
    };
    return kvm_window_enqueue(window, control);
}
