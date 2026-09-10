#include "lib/host/console_backend.h"

#ifndef _WIN32
struct host_console_backend { int unavailable; };
lib_status host_console_backend_create(host_console_backend **out_backend)
{
    if (out_backend != LIB_NULL) *out_backend = LIB_NULL;
    return LIB_STATUS_UNSUPPORTED;
}
void host_console_backend_destroy(host_console_backend *backend)
{ (void)backend; }
lib_status host_console_backend_prepare(host_console_backend *backend,
    lib_console *console, host_console_mode mode)
{ (void)backend; (void)console; (void)mode; return LIB_STATUS_UNSUPPORTED; }
void host_console_backend_discard_prepare(host_console_backend *backend)
{ (void)backend; }
lib_status host_console_backend_activate(host_console_backend *backend,
    lib_console *console, host_console_mode mode, lib_u32 generation)
{ (void)backend; (void)console; (void)mode; (void)generation; return LIB_STATUS_UNSUPPORTED; }
lib_status host_console_backend_deactivate(host_console_backend *backend)
{ (void)backend; return LIB_STATUS_UNSUPPORTED; }
void host_console_backend_lock_output(host_console_backend *backend)
{ (void)backend; }
void host_console_backend_unlock_output(host_console_backend *backend)
{ (void)backend; }
lib_status host_console_backend_write_bound(host_console_backend *backend,
    lib_console *expected_console, lib_u32 expected_generation,
    const char *text, lib_size length)
{ (void)backend; (void)expected_console; (void)expected_generation;
  (void)text; (void)length; return LIB_STATUS_NOT_CURRENT; }
lib_status host_console_backend_write_text_frame_bound(host_console_backend *backend,
    lib_console *expected_console, lib_u32 expected_generation,
    const lib_console_text_frame *frame)
{ (void)backend; (void)expected_console; (void)expected_generation;
  (void)frame; return LIB_STATUS_NOT_CURRENT; }
#endif
