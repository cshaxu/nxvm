#include "lib/types/test.h"
#include "lib/types/win32/test.h"
#include "lib/types/win32/console.h"
#include "lib/types/win32/sync.h"
#include "lib/console/binding_interface.h"
#include "../../fixtures/lifetime.h"

static lib_win32_handle entered, finish_read, delivered;
static lib_u32 lines, flushes, fragments;
static lib_bool fail_flush, fail_wake, split_line;
static lib_win32_bool LIB_WIN32_WINAPI read_line(lib_win32_handle h, lib_win32_lpvoid bytes, lib_win32_dword capacity, lib_win32_lpdword count, lib_win32_lpvoid p)
{
    (void)h; (void)p;
    lib_test_assert(capacity >= 9);
    lib_win32_set_event(entered);
    lib_test_assert(lib_win32_wait_for_single_object(finish_read, 5000) == LIB_WIN32_WAIT_OBJECT_0);
    if (split_line && fragments++ == 0u) {
        lib_win32_wchar *text = bytes;
        for (lib_win32_dword i = 0u; i < capacity; ++i) text[i] = L'x';
        *count = capacity;
    } else {
        lib_memory_copy(bytes, L"partial\r\n", 9u * sizeof(lib_win32_wchar)); *count = 9;
    }
    return LIB_WIN32_TRUE;
}
static lib_win32_bool LIB_WIN32_WINAPI wake_read(lib_win32_handle h, const lib_win32_input_record *r, lib_win32_dword n, lib_win32_lpdword written)
{
    (void)h;
    lib_test_assert(n == 1u && r->EventType == LIB_WIN32_KEY_EVENT);
    lib_test_assert(r->Event.KeyEvent.bKeyDown && r->Event.KeyEvent.uChar.AsciiChar == '\r');
    *written = fail_wake ? 0u : n;
    return fail_wake ? LIB_WIN32_FALSE : lib_win32_set_event(finish_read);
}
static lib_win32_bool LIB_WIN32_WINAPI flush(lib_win32_handle h)
{
    (void)h;
    ++flushes;
    return fail_flush ? LIB_WIN32_FALSE : LIB_WIN32_TRUE;
}
static lib_win32_bool LIB_WIN32_WINAPI set_mode(lib_win32_handle h, lib_win32_dword m) { (void)h; (void)m; return LIB_WIN32_TRUE; }
static lib_win32_dword LIB_WIN32_WINAPI exited_reader(void *context)
{ (void)context; return 0u; }
#undef lib_win32_read_console_w
#define lib_win32_read_console_w read_line
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
int main(int argc, char **argv)
{
    console_broker_backend b = {0};
    lib_bool completed;
    (void)argv;
    lifetime_fault = argc > 1;
    if (lifetime_fault) lib_test_assert(lib_test_at_exit(unexpected_cleanup) == 0);
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
    split_line = LIB_TRUE;
    lib_test_assert(console_broker_backend_request_cooked_line(&b) == 0);
    lib_test_assert(lib_win32_wait_for_single_object(entered, 5000) == LIB_WIN32_WAIT_OBJECT_0);
    lib_test_assert(console_broker_backend_cancel_cooked_line(&b, &completed) == 0);
    lib_test_assert(!completed && !b.reader && !b.cooked_line_pending && lines == 0 && flushes == 1);
    lib_test_assert(fragments == 2u);
    split_line = LIB_FALSE;
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
    /* An unquiesced reader cannot return to an owner releasing its context. */
    lib_win32_reset_event(entered); lib_win32_reset_event(finish_read);
    fail_wake = lifetime_fault != 0;
    lib_test_assert(console_broker_backend_request_cooked_line(&b) == 0);
    lib_test_assert(lib_win32_wait_for_single_object(entered, 5000) == LIB_WIN32_WAIT_OBJECT_0);
    lib_test_assert(console_broker_backend_cancel_cooked_line(&b, &completed) == LIB_STATUS_OK);
    lib_test_assert(!b.reader && lines == 1 && flushes == 3);
    /* A completed reader's I/O failure must not strand its thread handle. */
    b.reader = lib_win32_create_thread(LIB_NULL, 0u, exited_reader, LIB_NULL, 0u, LIB_NULL);
    lib_test_assert(b.reader && lib_win32_wait_for_single_object(b.reader, 5000u) == LIB_WIN32_WAIT_OBJECT_0);
    b.reader_status = LIB_STATUS_UNSUPPORTED;
    lib_test_assert(console_broker_retire_reader(&b) == LIB_STATUS_UNSUPPORTED && !b.reader);
    lib_console_release(b.console);
    /* A post-join error reports failure but still completes safe cleanup. */
    fail_flush = LIB_TRUE;
    lib_test_assert(console_broker_backend_deactivate(&b, LIB_NULL) == LIB_STATUS_IO_ERROR);
    lib_test_assert(b.stop_event == LIB_NULL && b.generation == 0u && flushes == 4);
    lib_win32_close_handle(entered); lib_win32_close_handle(finish_read); lib_win32_close_handle(delivered);
    return 0;
}
