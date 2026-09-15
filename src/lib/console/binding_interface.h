#ifndef LIB_CONSOLE_BINDING_INTERFACE_H
#define LIB_CONSOLE_BINDING_INTERFACE_H

#include "lib/console/console_interface.h"

/* This adapter-only surface is for generic host implementations.  Products
 * receive events and write through the public object API; they never bind a
 * logical Console to native I/O themselves. */
typedef lib_status (*lib_console_output_sink)(void *context,
    const char *text, lib_size length);
typedef lib_status (*lib_console_text_frame_sink)(void *context,
    const lib_console_text_frame *frame);

typedef struct lib_console_output_binding {
    lib_console_output_sink text;
    lib_console_text_frame_sink frame;
    void *context;
} lib_console_output_binding;

/* Copies the binding; NULL clears both outputs. Waits for in-flight output
 * before replacing it. The borrowed context must survive until replacement
 * returns; output callbacks must not reenter binding or destruction.
 * A valid Console always accepts replacement. An absent callback returns
 * NOT_CURRENT from its corresponding write operation. */
lib_status lib_console_set_output_binding(lib_console *console,
    const lib_console_output_binding *binding);
lib_status lib_console_deliver_event(lib_console *console,
    const lib_console_event *event);
lib_status lib_console_bind_generation(lib_console *console,
    lib_u32 generation);
void lib_console_invalidate_binding(lib_console *console);

#endif
