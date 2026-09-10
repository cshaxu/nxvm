#ifndef LIB_BASE_CONSOLE_H
#define LIB_BASE_CONSOLE_H

#include "lib/base/console_interface.h"

/* This adapter-only surface is for generic host implementations.  Products
 * receive events and write through the public object API; they never bind a
 * logical Console to native I/O themselves. */
typedef lib_status (*lib_console_output_sink)(void *context,
    const char *text, lib_size length);
typedef lib_status (*lib_console_text_frame_sink)(void *context,
    const lib_console_text_frame *frame);

lib_status lib_console_set_output_sink(lib_console *console,
    lib_console_output_sink sink, void *context);
lib_status lib_console_set_text_frame_sink(lib_console *console,
    lib_console_text_frame_sink sink, void *context);
lib_status lib_console_deliver_event(lib_console *console,
    const lib_console_event *event);
lib_status lib_console_bind_generation(lib_console *console,
    lib_u32 generation);
void lib_console_invalidate_binding(lib_console *console);

#endif
