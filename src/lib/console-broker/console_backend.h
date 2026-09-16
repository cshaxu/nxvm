#ifndef LIB_CONSOLE_BROKER_BACKEND_H
#define LIB_CONSOLE_BROKER_BACKEND_H

#include "lib/console-broker/console_interface.h"

typedef struct console_broker_backend console_broker_backend;

lib_status console_broker_backend_create(console_broker_backend **out_backend);
/* Broker must deactivate successfully before disposal, including failed
 * activation cleanup. Disposal never retires readers or restores modes. */
lib_status console_broker_backend_destroy(console_broker_backend *backend);
/* Validate that the next binding can be activated without changing the
 * current Console's reader, mode, or output ownership.  Preparation never
 * creates a competing reader. */
lib_status console_broker_backend_prepare(console_broker_backend *backend,
    lib_console *console, console_broker_mode mode);
lib_status console_broker_backend_activate(console_broker_backend *backend,
    lib_console *console, console_broker_mode mode, lib_u32 generation,
    lib_bool restore_cooked_request);
/* Cooked activation alone does not request a line. Each explicit request
 * arms one line; rollback restores only an unfinished retired request. */
lib_status console_broker_backend_request_cooked_line(
    console_broker_backend *backend);
lib_status console_broker_backend_cancel_cooked_line(console_broker_backend *backend,
    lib_bool *out_completed);
/* Retire and join the current reader before invalidating its binding or
 * activating another. Retirement failure is terminal: do not start next or
 * claim that old remains usable. The broker fails closed. */
lib_status console_broker_backend_deactivate(console_broker_backend *backend,
    lib_bool *out_cooked_request);
/* The broker holds this gate across an indivisible native takeover.  Bound
 * writers take the same gate and validate their logical Console/generation
 * after it opens, so an old write can never land on a new Current Console. */
void console_broker_backend_lock_transaction(console_broker_backend *backend);
void console_broker_backend_unlock_transaction(console_broker_backend *backend);
void console_broker_backend_lock_output(console_broker_backend *backend);
void console_broker_backend_unlock_output(console_broker_backend *backend);
lib_status console_broker_backend_write_bound(console_broker_backend *backend,
    lib_console *expected_console, lib_u32 expected_generation, const char *text,
    lib_size length);
lib_status console_broker_backend_write_text_frame_bound(console_broker_backend *backend,
    lib_console *expected_console, lib_u32 expected_generation,
    const lib_console_text_frame *frame);

#endif
