#ifndef LIB_HOST_INTERNAL_CONSOLE_NATIVE_H
#define LIB_HOST_INTERNAL_CONSOLE_NATIVE_H

#include "lib/host/console.h"

typedef struct host_console_native host_console_native;

lib_status host_console_native_create(host_console_native **out_native);
void host_console_native_destroy(host_console_native *native_console);
/* Validate that the next binding can be activated without changing the
 * current Console's reader, mode, or output ownership.  Preparation never
 * creates a competing reader. */
lib_status host_console_native_prepare(host_console_native *native_console,
    lib_console *console, host_console_mode mode);
void host_console_native_discard_prepare(host_console_native *native_console);
lib_status host_console_native_activate(host_console_native *native_console,
    lib_console *console, host_console_mode mode, lib_u32 generation);
/* Cooked input is deliberately one line per reader.  The product asks for
 * the next native line only when its monitor is actually ready for it; this
 * keeps a completed `start` line from racing a later raw-Console takeover. */
lib_status host_console_native_request_cooked_line(
    host_console_native *native_console);
/* Retire the current native reader before a broker can invalidate its logical
 * binding or activate another one.  Failure leaves the current native object
 * intact, so a replacement remains an all-or-nothing ownership transaction. */
lib_status host_console_native_deactivate(host_console_native *native_console);
/* The broker holds this gate across an indivisible native takeover.  Bound
 * writers take the same gate and validate their logical Console/generation
 * after it opens, so an old write can never land on a new Current Console. */
void host_console_native_lock_output(host_console_native *native_console);
void host_console_native_unlock_output(host_console_native *native_console);
lib_status host_console_native_write_bound(host_console_native *native_console,
    lib_console *expected_console, lib_u32 expected_generation, const char *text,
    lib_size length);
lib_status host_console_native_write_text_frame_bound(host_console_native *native_console,
    lib_console *expected_console, lib_u32 expected_generation,
    const lib_console_text_frame *frame);

#endif
