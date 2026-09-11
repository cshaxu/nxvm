#include "lib/ui-window/window.h"

#include <stdlib.h>
#include <string.h>

static void ui_window_component_stop(ui_component *base)
{ ui_window_native_stop((ui_window *)base); }

static void ui_window_component_dispose(ui_component *base)
{
    ui_window *window = (ui_window *)base;
    ui_component_mailboxes_destroy(&window->base.mailboxes);
    free(window);
}

static lib_status ui_window_enqueue(ui_window *window,
    ui_component_control control)
{
    return window == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        ui_component_enqueue_controls(&window->base, &control, 1u);
}

lib_status ui_window_create(ui_window **out_window,
    const ui_window_options *options)
{
    ui_window *window;
    lib_status status;

    if (out_window == LIB_NULL || options == LIB_NULL ||
        options->initial_title == LIB_NULL ||
        memchr(options->initial_title, '\0', UI_WINDOW_TITLE_CAPACITY) == LIB_NULL ||
        options->component.input_sink == LIB_NULL ||
        options->component.failure_sink == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_window = LIB_NULL;
    window = calloc(1u, sizeof(*window));
    if (window == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    memcpy(window->initial_title, options->initial_title,
        strlen(options->initial_title) + 1u);
    window->initial_frozen = options->initial_frozen != LIB_FALSE;
    status = ui_component_initialize(&window->base, &options->component,
        ui_window_component_stop, ui_window_component_dispose);
    if (status == LIB_STATUS_OK) status = ui_window_native_start(window);
    if (status != LIB_STATUS_OK) {
        ui_component_mailboxes_destroy(&window->base.mailboxes);
        free(window);
        return status;
    }
    *out_window = window;
    return LIB_STATUS_OK;
}

lib_status ui_window_publish_frame(ui_window *window, const ui_frame *frame)
{
    return window == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        ui_component_publish_frame(&window->base, frame);
}

void ui_window_destroy(ui_window *window)
{
    if (window != LIB_NULL) ui_component_destroy(&window->base);
}

lib_status ui_window_set_title(ui_window *window, const char *title)
{
    ui_component_control control = {
        .kind = UI_COMPONENT_CONTROL_SET_WINDOW_TITLE
    };
    if (title == LIB_NULL || memchr(title, '\0',
            UI_COMPONENT_WINDOW_TITLE_CAPACITY) == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    memcpy(control.value.title, title, strlen(title) + 1u);
    return ui_window_enqueue(window, control);
}

lib_status ui_window_unfreeze(ui_window *window)
{
    ui_component_control control = {
        .kind = UI_COMPONENT_CONTROL_SET_WINDOW_FROZEN
    };
    control.value.window_frozen = LIB_FALSE;
    return ui_window_enqueue(window, control);
}

lib_status ui_window_freeze(ui_window *window)
{
    ui_component_control controls[2] = {
        { .kind = UI_COMPONENT_CONTROL_SET_WINDOW_FROZEN },
        { .kind = UI_COMPONENT_CONTROL_RELEASE_WINDOW_MOUSE }
    };
    if (window == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    controls[0].value.window_frozen = LIB_TRUE;
    return ui_component_enqueue_controls(&window->base, controls, 2u);
}

lib_status ui_window_release_mouse(ui_window *window)
{
    ui_component_control control = {
        .kind = UI_COMPONENT_CONTROL_RELEASE_WINDOW_MOUSE
    };
    return ui_window_enqueue(window, control);
}
