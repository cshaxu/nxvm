#ifndef LIB_HOST_CONSOLE_BACKEND_H
#define LIB_HOST_CONSOLE_BACKEND_H

#include "lib/host/console_interface.h"

typedef struct host_console_backend host_console_backend;

lib_status host_console_backend_create(host_console_backend **out_backend);
/* Broker must deactivate successfully before disposal, including failed
 * activation cleanup. Disposal never retires readers or restores modes. */
void host_console_backend_destroy(host_console_backend *backend);
/* Validate that the next binding can be activated without changing the
 * current Console's reader, mode, or output ownership.  Preparation never
 * creates a competing reader. */
lib_status host_console_backend_prepare(host_console_backend *backend,
    lib_console *console, host_console_mode mode);
lib_status host_console_backend_activate(host_console_backend *backend,
    lib_console *console, host_console_mode mode, lib_u32 generation,
    lib_bool restore_cooked_request);
/* Cooked activation alone does not request a line. Each explicit request
 * arms one line; rollback restores only an unfinished retired request. */
lib_status host_console_backend_request_cooked_line(
    host_console_backend *backend);
/* Retire and join the current reader before invalidating its binding or
 * activating another. Retirement failure is terminal: do not start next or
 * claim that old remains usable. The broker fails closed. */
lib_status host_console_backend_deactivate(host_console_backend *backend,
    lib_bool *out_cooked_request);
/* The broker holds this gate across an indivisible native takeover.  Bound
 * writers take the same gate and validate their logical Console/generation
 * after it opens, so an old write can never land on a new Current Console. */
void host_console_backend_lock_transaction(host_console_backend *backend);
void host_console_backend_unlock_transaction(host_console_backend *backend);
void host_console_backend_lock_output(host_console_backend *backend);
void host_console_backend_unlock_output(host_console_backend *backend);
lib_status host_console_backend_write_bound(host_console_backend *backend,
    lib_console *expected_console, lib_u32 expected_generation, const char *text,
    lib_size length);
lib_status host_console_backend_write_text_frame_bound(host_console_backend *backend,
    lib_console *expected_console, lib_u32 expected_generation,
    const lib_console_text_frame *frame);

#endif
