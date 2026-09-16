#include "lib/types/win32/console.h"
#include "lib/types/win32/sync.h"
#include "lib/console/binding_interface.h"
#include <assert.h>
#include <string.h>

static HANDLE entered, finish_read, delivered;
static unsigned lines, flushes;
static BOOL WINAPI read_line(HANDLE h, LPVOID bytes, DWORD capacity, LPDWORD count, LPVOID p)
{
    (void)h; (void)p;
    assert(capacity >= 9);
    SetEvent(entered);
    assert(WaitForSingleObject(finish_read, 5000) == WAIT_OBJECT_0);
    memcpy(bytes, "partial\r\n", 9); *count = 9;
    return TRUE;
}
static BOOL WINAPI cancel_read(HANDLE h, LPOVERLAPPED p)
{ (void)h; (void)p; return SetEvent(finish_read); }
static BOOL WINAPI cancel_thread(HANDLE h) { (void)h; return TRUE; }
static BOOL WINAPI wake_read(HANDLE h, const INPUT_RECORD *r, DWORD n, LPDWORD written)
{ (void)h; (void)r; *written = n; return TRUE; }
static BOOL WINAPI flush(HANDLE h) { (void)h; ++flushes; return TRUE; }
static BOOL WINAPI set_mode(HANDLE h, DWORD m) { (void)h; (void)m; return TRUE; }
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
    assert(e->kind == LIB_CONSOLE_EVENT_COOKED_LINE);
    assert(strcmp(e->value.line.text, "partial") == 0);
    ++lines;
    SetEvent(delivered);
}
int main(void)
{
    console_broker_backend b = {0};
    lib_bool completed;
    entered = CreateEventA(NULL, TRUE, FALSE, NULL);
    finish_read = CreateEventA(NULL, TRUE, FALSE, NULL);
    delivered = CreateEventA(NULL, TRUE, FALSE, NULL);
    b.stop_event = CreateEventA(NULL, TRUE, FALSE, NULL);
    assert(entered && finish_read && delivered && b.stop_event);
    b.mode = CONSOLE_BROKER_COOKED_LINES; b.generation = 1;
    assert(lib_console_create(&b.console) == 0);
    assert(lib_console_bind_generation(b.console, 1) == 0);
    assert(lib_console_set_event_sink(b.console, receive, NULL) == 0);
    /* Real worker is inside read. Cancellation joins it; its fragment cannot
     * become a command and a second read starts only after the join. */
    assert(console_broker_backend_request_cooked_line(&b) == 0);
    assert(WaitForSingleObject(entered, 5000) == WAIT_OBJECT_0);
    assert(console_broker_backend_cancel_cooked_line(&b, &completed) == 0);
    assert(!completed && !b.reader && !b.cooked_line_pending && lines == 0 && flushes == 1);
    assert(WaitForSingleObject(b.stop_event, 0) == WAIT_TIMEOUT);
    ResetEvent(entered); ResetEvent(finish_read);
    assert(console_broker_backend_request_cooked_line(&b) == 0);
    assert(WaitForSingleObject(entered, 5000) == WAIT_OBJECT_0);
    /* A complete event wins the race. It remains consumable; cancellation and
     * another notification cannot turn it into an abandoned fragment. */
    SetEvent(finish_read);
    assert(WaitForSingleObject(delivered, 5000) == WAIT_OBJECT_0);
    assert(console_broker_backend_cancel_cooked_line(&b, &completed) == 0 && completed);
    assert(!b.reader && lines == 1 && flushes == 2);
    assert(console_broker_backend_cancel_cooked_line(&b, &completed) == 0 && completed);
    lib_console_release(b.console);
    assert(console_broker_backend_deactivate(&b, NULL) == 0);
    CloseHandle(entered); CloseHandle(finish_read); CloseHandle(delivered);
    return 0;
}
