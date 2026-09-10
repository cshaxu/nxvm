#ifndef UX_WINDOW_INTERFACE_H
#define UX_WINDOW_INTERFACE_H

#include "lib/ux-base/component_interface.h"

#define UX_WINDOW_TITLE_CAPACITY 128u

typedef struct ux_window ux_window;

typedef struct ux_window_options {
    ux_component_options component;
    /* Copied during creation.  The application owns both initial product
     * identity and later title changes; ux-window never supplies one. */
    const char *initial_title;
    /* Frozen forbids capture and cursor blink.  It does not own a capture;
     * an unfrozen Window still waits for a client-area click to acquire one. */
    lib_bool initial_frozen;
} ux_window_options;

lib_status ux_window_create(ux_window **out_window,
    const ux_window_options *options);
lib_status ux_window_publish_frame(ux_window *window, const ux_frame *frame);
void ux_window_destroy(ux_window *window);
lib_status ux_window_set_title(ux_window *window, const char *title);
/* Freeze atomically prevents future capture, stops the Window-local cursor
 * blink, and releases any current capture in FIFO order. */
lib_status ux_window_freeze(ux_window *window);
/* Unfreeze permits a later client-area click to capture. It never captures
 * the mouse itself and resumes the Window-local cursor blink. */
lib_status ux_window_unfreeze(ux_window *window);
lib_status ux_window_release_mouse(ux_window *window);

#endif
