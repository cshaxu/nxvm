#include "lib/types/test.h"
#include "lib/types/win32/test.h"
#include "lib/types/file.h"
#include "lib/types/win32/console.h"
#include "lib/types/win32/sync.h"
#include "lib/console/binding_interface.h"

static lib_win32_handle entered, finish_read, delivered;
static lib_u32 lines, flushes;
static lib_win32_bool LIB_WIN32_WINAPI read_line(lib_win32_handle h, lib_win32_lpvoid bytes, lib_win32_dword capacity, lib_win32_lpdword count, lib_win32_lpvoid p)
{
    (void)h; (void)p;
    lib_test_assert(capacity >= 9);
    lib_win32_set_event(entered);
    lib_test_assert(lib_win32_wait_for_single_object(finish_read, 5000) == LIB_WIN32_WAIT_OBJECT_0);
    lib_memory_copy(bytes, "partial\r\n", 9); *count = 9;
    return LIB_WIN32_TRUE;
}
static lib_win32_bool LIB_WIN32_WINAPI cancel_read(lib_win32_handle h, lib_win32_lpoverlapped p)
{ (void)h; (void)p; return lib_win32_set_event(finish_read); }
static lib_win32_bool LIB_WIN32_WINAPI cancel_thread(lib_win32_handle h) { (void)h; return LIB_WIN32_TRUE; }
static lib_win32_bool LIB_WIN32_WINAPI wake_read(lib_win32_handle h, const lib_win32_input_record *r, lib_win32_dword n, lib_win32_lpdword written)
{ (void)h; (void)r; *written = n; return LIB_WIN32_TRUE; }
static lib_win32_bool LIB_WIN32_WINAPI flush(lib_win32_handle h) { (void)h; ++flushes; return LIB_WIN32_TRUE; }
static lib_win32_bool LIB_WIN32_WINAPI set_mode(lib_win32_handle h, lib_win32_dword m) { (void)h; (void)m; return LIB_WIN32_TRUE; }
#undef lib_win32_read_console_a
#define lib_win32_read_console_a read_line
#undef lib_win32_cancel_io_ex
#define lib_win32_cancel_io_ex cancel_read
#undef lib_win32_cancel_synchronous_io
#define lib_win32_cancel_synchronous_io cancel_thread
#undef lib_win32_write_console_input_a
#define lib_win32_write_console_input_a wake_read
#undef lib_win32_flush_console_input_buffer
#define lib_win32_flush_console_input_buffer flush
#undef lib_win32_set_console_mode
#define lib_win32_set_console_mode set_mode
#include "lib/console-broker/win32/console.c"

static void receive(void *p, const lib_console_event *e)
{
    (void)p;
    lib_test_assert(e->kind == LIB_CONSOLE_EVENT_COOKED_LINE);
    lib_test_assert(lib_text_compare(e->value.line.text, "partial") == 0);
    ++lines;
    lib_win32_set_event(delivered);
}
int main(void)
{
    console_broker_backend b = {0};
    lib_bool completed;
    entered = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    finish_read = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    delivered = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    b.stop_event = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    lib_test_assert(entered && finish_read && delivered && b.stop_event);
    b.mode = CONSOLE_BROKER_COOKED_LINES; b.generation = 1;
    lib_test_assert(lib_console_create(&b.console) == 0);
    lib_test_assert(lib_console_bind_generation(b.console, 1) == 0);
    lib_test_assert(lib_console_set_event_sink(b.console, receive, LIB_NULL) == 0);
    /* Real worker is inside read. Cancellation joins it; its fragment cannot
     * become a command and a second read starts only after the join. */
    lib_test_assert(console_broker_backend_request_cooked_line(&b) == 0);
    lib_test_assert(lib_win32_wait_for_single_object(entered, 5000) == LIB_WIN32_WAIT_OBJECT_0);
    lib_test_assert(console_broker_backend_cancel_cooked_line(&b, &completed) == 0);
    lib_test_assert(!completed && !b.reader && !b.cooked_line_pending && lines == 0 && flushes == 1);
    lib_test_assert(lib_win32_wait_for_single_object(b.stop_event, 0) == LIB_WIN32_WAIT_TIMEOUT);
    lib_win32_reset_event(entered); lib_win32_reset_event(finish_read);
    lib_test_assert(console_broker_backend_request_cooked_line(&b) == 0);
    lib_test_assert(lib_win32_wait_for_single_object(entered, 5000) == LIB_WIN32_WAIT_OBJECT_0);
    /* A complete event wins the race. It remains consumable; cancellation and
     * another notification cannot turn it into an abandoned fragment. */
    lib_win32_set_event(finish_read);
    lib_test_assert(lib_win32_wait_for_single_object(delivered, 5000) == LIB_WIN32_WAIT_OBJECT_0);
    lib_test_assert(console_broker_backend_cancel_cooked_line(&b, &completed) == 0 && completed);
    lib_test_assert(!b.reader && lines == 1 && flushes == 2);
    lib_test_assert(console_broker_backend_cancel_cooked_line(&b, &completed) == 0 && completed);
    lib_console_release(b.console);
    lib_test_assert(console_broker_backend_deactivate(&b, LIB_NULL) == 0);
    lib_win32_close_handle(entered); lib_win32_close_handle(finish_read); lib_win32_close_handle(delivered);
    return 0;
}
