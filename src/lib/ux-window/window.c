#include "lib/ux-window/internal.h"

static void ux_window_component_stop(ux_component *base)
{ ux_window_native_stop((ux_window *)base); }

static void ux_window_component_dispose(ux_component *base)
{
    ux_window *window = (ux_window *)base;
    ux_component_mailboxes_destroy(&window->base.mailboxes);
    free(window);
}

static lib_status ux_window_enqueue(ux_window *window,
    ux_component_control control)
{
    return window == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        ux_component_enqueue_controls(&window->base, &control, 1u);
}

lib_status ux_window_create(ux_window **out_window,
    const ux_window_options *options)
{
    ux_window *window;
    lib_status status;

    if (out_window == LIB_NULL || options == LIB_NULL ||
        options->initial_title == LIB_NULL ||
        memchr(options->initial_title, '\0', UX_WINDOW_TITLE_CAPACITY) == LIB_NULL ||
        options->component.input_sink == LIB_NULL ||
        options->component.failure_sink == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_window = LIB_NULL;
    window = calloc(1u, sizeof(*window));
    if (window == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    memcpy(window->initial_title, options->initial_title,
        strlen(options->initial_title) + 1u);
    window->initial_frozen = options->initial_frozen != LIB_FALSE;
    status = ux_component_initialize(&window->base, &options->component,
        ux_window_component_stop, ux_window_component_dispose);
    if (status == LIB_STATUS_OK) status = ux_window_native_start(window);
    if (status != LIB_STATUS_OK) {
        ux_component_mailboxes_destroy(&window->base.mailboxes);
        free(window);
        return status;
    }
    *out_window = window;
    return LIB_STATUS_OK;
}

lib_status ux_window_publish_frame(ux_window *window, const ux_frame *frame)
{
    return window == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        ux_component_publish_frame(&window->base, frame);
}

void ux_window_destroy(ux_window *window)
{
    if (window != LIB_NULL) ux_component_destroy(&window->base);
}

lib_status ux_window_set_title(ux_window *window, const char *title)
{
    ux_component_control control = { 0 };
    if (title == LIB_NULL || memchr(title, '\0',
            UX_COMPONENT_WINDOW_TITLE_CAPACITY) == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    control.kind = UX_COMPONENT_CONTROL_SET_WINDOW_TITLE;
    memcpy(control.value.title, title, strlen(title) + 1u);
    return ux_window_enqueue(window, control);
}

lib_status ux_window_unfreeze(ux_window *window)
{
    ux_component_control control = { UX_COMPONENT_CONTROL_SET_WINDOW_FROZEN,
        { 0 } };
    control.value.window_frozen = LIB_FALSE;
    return ux_window_enqueue(window, control);
}

lib_status ux_window_freeze(ux_window *window)
{
    ux_component_control controls[2] = {
        { UX_COMPONENT_CONTROL_SET_WINDOW_FROZEN, { 0 } },
        { UX_COMPONENT_CONTROL_RELEASE_WINDOW_MOUSE, { 0 } }
    };
    if (window == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    controls[0].value.window_frozen = LIB_TRUE;
    return ux_component_enqueue_controls(&window->base, controls, 2u);
}

lib_status ux_window_release_mouse(ux_window *window)
{
    ux_component_control control = { UX_COMPONENT_CONTROL_RELEASE_WINDOW_MOUSE,
        { 0 } };
    return ux_window_enqueue(window, control);
}
