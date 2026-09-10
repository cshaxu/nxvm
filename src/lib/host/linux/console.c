#include "lib/host/internal/console_native.h"

#ifndef _WIN32
struct host_console_native { int unavailable; };
lib_status host_console_native_create(host_console_native **out_native)
{
    if (out_native != LIB_NULL) *out_native = LIB_NULL;
    return LIB_STATUS_UNSUPPORTED;
}
void host_console_native_destroy(host_console_native *native_console)
{ (void)native_console; }
lib_status host_console_native_prepare(host_console_native *native_console,
    lib_console *console, host_console_mode mode)
{ (void)native_console; (void)console; (void)mode; return LIB_STATUS_UNSUPPORTED; }
void host_console_native_discard_prepare(host_console_native *native_console)
{ (void)native_console; }
lib_status host_console_native_activate(host_console_native *native_console,
    lib_console *console, host_console_mode mode, lib_u32 generation)
{ (void)native_console; (void)console; (void)mode; (void)generation; return LIB_STATUS_UNSUPPORTED; }
lib_status host_console_native_deactivate(host_console_native *native_console)
{ (void)native_console; return LIB_STATUS_UNSUPPORTED; }
void host_console_native_lock_output(host_console_native *native_console)
{ (void)native_console; }
void host_console_native_unlock_output(host_console_native *native_console)
{ (void)native_console; }
lib_status host_console_native_write_bound(host_console_native *native_console,
    lib_console *expected_console, lib_u32 expected_generation,
    const char *text, lib_size length)
{ (void)native_console; (void)expected_console; (void)expected_generation;
  (void)text; (void)length; return LIB_STATUS_NOT_CURRENT; }
lib_status host_console_native_write_text_frame_bound(host_console_native *native_console,
    lib_console *expected_console, lib_u32 expected_generation,
    const lib_console_text_frame *frame)
{ (void)native_console; (void)expected_console; (void)expected_generation;
  (void)frame; return LIB_STATUS_NOT_CURRENT; }
#endif
