#ifndef UI_WINDOW_INTERFACE_H
#define UI_WINDOW_INTERFACE_H

#include "lib/ui-base/component_interface.h"

#define UI_WINDOW_TITLE_CAPACITY 128u

typedef struct ui_window ui_window;

typedef struct ui_window_options {
    ui_component_options component;
    /* Copied during creation.  The application owns both initial product
     * identity and later title changes; ui-window never supplies one. */
    const char *initial_title;
    /* Frozen forbids capture and cursor blink.  It does not own a capture;
     * an unfrozen Window still waits for a client-area click to acquire one. */
    lib_bool initial_frozen;
} ui_window_options;

lib_status ui_window_create(ui_window **out_window,
    const ui_window_options *options);
lib_status ui_window_publish_frame(ui_window *window, const ui_frame *frame);
void ui_window_destroy(ui_window *window);
lib_status ui_window_set_title(ui_window *window, const char *title);
/* Freeze atomically prevents future capture, stops the Window-local cursor
 * blink, and releases any current capture in FIFO order. */
lib_status ui_window_freeze(ui_window *window);
/* Unfreeze permits a later client-area click to capture. It never captures
 * the mouse itself and resumes the Window-local cursor blink. */
lib_status ui_window_unfreeze(ui_window *window);
lib_status ui_window_release_mouse(ui_window *window);

#endif
