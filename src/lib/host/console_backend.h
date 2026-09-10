#ifndef LIB_HOST_CONSOLE_BACKEND_H
#define LIB_HOST_CONSOLE_BACKEND_H

#include "lib/host/console_interface.h"

typedef struct host_console_backend host_console_backend;

lib_status host_console_backend_create(host_console_backend **out_backend);
void host_console_backend_destroy(host_console_backend *backend);
/* Validate that the next binding can be activated without changing the
 * current Console's reader, mode, or output ownership.  Preparation never
 * creates a competing reader. */
lib_status host_console_backend_prepare(host_console_backend *backend,
    lib_console *console, host_console_mode mode);
void host_console_backend_discard_prepare(host_console_backend *backend);
lib_status host_console_backend_activate(host_console_backend *backend,
    lib_console *console, host_console_mode mode, lib_u32 generation);
/* Cooked input is deliberately one line per reader.  The product asks for
 * the next native line only when its monitor is actually ready for it; this
 * keeps a completed `start` line from racing a later raw-Console takeover. */
lib_status host_console_backend_request_cooked_line(
    host_console_backend *backend);
/* Retire the current native reader before a broker can invalidate its logical
 * binding or activate another one.  Failure leaves the current native object
 * intact, so a replacement remains an all-or-nothing ownership transaction. */
lib_status host_console_backend_deactivate(host_console_backend *backend);
/* The broker holds this gate across an indivisible native takeover.  Bound
 * writers take the same gate and validate their logical Console/generation
 * after it opens, so an old write can never land on a new Current Console. */
void host_console_backend_lock_output(host_console_backend *backend);
void host_console_backend_unlock_output(host_console_backend *backend);
lib_status host_console_backend_write_bound(host_console_backend *backend,
    lib_console *expected_console, lib_u32 expected_generation, const char *text,
    lib_size length);
lib_status host_console_backend_write_text_frame_bound(host_console_backend *backend,
    lib_console *expected_console, lib_u32 expected_generation,
    const lib_console_text_frame *frame);

#endif
