#include "lib/types/win32/console.h"
#include "lib/types/win32/sync.h"
#include "lib/console/binding_interface.h"
#include <assert.h>

static BOOL WINAPI failed_read(HANDLE input, LPVOID bytes, DWORD length,
    LPDWORD read, LPVOID reserved)
{ (void)input; (void)bytes; (void)length; (void)read; (void)reserved; return FALSE; }
static INPUT_RECORD records[5];
static unsigned record_count, record_index, raw_count;
static unsigned raw_unicode[16], raw_repeat[16];
static BOOL WINAPI read_wide(HANDLE input, PINPUT_RECORD record, DWORD length, LPDWORD read)
{
    (void)input; assert(length == 1 && record_index < record_count);
    *record = records[record_index++]; *read = 1; return TRUE;
}
static DWORD WINAPI failed_wait(DWORD count, const HANDLE *handles, BOOL all, DWORD timeout)
{ (void)count; (void)handles; (void)all; (void)timeout;
  return record_index < record_count ? WAIT_OBJECT_0 + 1 : WAIT_FAILED; }
#undef lib_win32_read_console_input_w
#define lib_win32_read_console_input_w read_wide
#undef lib_win32_read_console_a
#undef lib_win32_wait_for_multiple_objects
#define lib_win32_read_console_a failed_read
#define lib_win32_wait_for_multiple_objects failed_wait
#include "lib/host/win32/console.c"

static int failures;
static void receive(void *context, const lib_console_event *event)
{
    (void)context;
    if (event->kind == LIB_CONSOLE_EVENT_RAW_KEY) {
        assert(raw_count < 16);
        raw_repeat[raw_count] = event->value.raw_key.repeat_count;
        raw_unicode[raw_count++] = event->value.raw_key.unicode;
        return;
    }
    assert(event->kind == LIB_CONSOLE_EVENT_IO_FAILURE);
    ++failures;
}
int main(void)
{
    host_console_backend backend = { 0 };
    assert(lib_console_create(&backend.console) == LIB_STATUS_OK);
    assert(lib_console_set_event_sink(backend.console, receive, NULL) == LIB_STATUS_OK);
    assert(lib_console_bind_generation(backend.console, 1u) == LIB_STATUS_OK);
    backend.generation = 1u;
    backend.stop_event = CreateEventA(NULL, TRUE, FALSE, NULL);
    assert(backend.stop_event);
    for (int mode = 0; mode != 2; ++mode) {
        backend.mode = mode == 0 ? HOST_CONSOLE_RAW_EVENTS : HOST_CONSOLE_COOKED_LINES;
        ResetEvent(backend.stop_event);
        host_console_reader(&backend);
        assert(failures == mode + 1);
        SetEvent(backend.stop_event);
        host_console_reader(&backend);
        assert(failures == mode + 1);
    }
    ResetEvent(backend.stop_event);
    backend.generation = 2u;
    host_console_reader(&backend);
    assert(failures == 2);
    backend.generation = 1u;
    backend.mode = HOST_CONSOLE_RAW_EVENTS;
    record_count = 5;
    for (unsigned i = 0; i < record_count; ++i) {
        records[i].EventType = KEY_EVENT;
        records[i].Event.KeyEvent.bKeyDown = TRUE;
        records[i].Event.KeyEvent.wRepeatCount = i == 0 ? 5 : 1;
        records[i].Event.KeyEvent.uChar.UnicodeChar =
            i == 0 ? 0x4e00 : i == 1 ? 0xd83d : i == 2 ? 0xde00 : 0;
    }
    records[3].Event.KeyEvent.bKeyDown = FALSE;
    records[3].Event.KeyEvent.wRepeatCount = 5; /* one physical release */
    records[4].Event.KeyEvent.wRepeatCount = 0; /* preserve synthetic single record */
    host_console_reader(&backend);
    assert(record_index == 5 && raw_count == 5 && failures == 3);
    assert(raw_unicode[0] == 0x4e00 && raw_repeat[0] == 5);
    assert(raw_unicode[1] == 0xd83d && raw_unicode[2] == 0xde00);
    assert(raw_repeat[1] == 1 && raw_repeat[2] == 1);
    assert(raw_repeat[3] == 5 && raw_repeat[4] == 0);
    CloseHandle(backend.stop_event);
    lib_console_release(backend.console);
    return 0;
}
