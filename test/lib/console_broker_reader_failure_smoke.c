#include "lib/types/test.h"
#include "lib/types/win32/test.h"
#include "lib/types/file.h"
#include "lib/types/win32/console.h"
#include "lib/types/win32/sync.h"
#include "lib/console/binding_interface.h"

static lib_win32_bool LIB_WIN32_WINAPI failed_read(lib_win32_handle input, lib_win32_lpvoid bytes, lib_win32_dword length,
    lib_win32_lpdword read, lib_win32_lpvoid reserved)
{ (void)input; (void)bytes; (void)length; (void)read; (void)reserved; return LIB_WIN32_FALSE; }
static lib_win32_input_record records[5];
static lib_u32 record_count, record_index, raw_count;
static lib_u32 raw_unicode[16], raw_repeat[16];
static lib_win32_bool LIB_WIN32_WINAPI read_wide(lib_win32_handle input, lib_win32_input_record *record, lib_win32_dword length, lib_win32_lpdword read)
{
    (void)input; lib_test_assert(length == 1 && record_index < record_count);
    *record = records[record_index++]; *read = 1; return LIB_WIN32_TRUE;
}
static lib_win32_dword LIB_WIN32_WINAPI failed_wait(lib_win32_dword count, const lib_win32_handle *handles, lib_win32_bool all, lib_win32_dword timeout)
{ (void)count; (void)handles; (void)all; (void)timeout;
  return record_index < record_count ? LIB_WIN32_WAIT_OBJECT_0 + 1 : LIB_WIN32_WAIT_FAILED; }
#undef lib_win32_read_console_input_w
#define lib_win32_read_console_input_w read_wide
#undef lib_win32_read_console_a
#undef lib_win32_wait_for_multiple_objects
#define lib_win32_read_console_a failed_read
#define lib_win32_wait_for_multiple_objects failed_wait
#include "lib/console-broker/win32/console.c"

static lib_i32 failures;
static void receive(void *context, const lib_console_event *event)
{
    (void)context;
    if (event->kind == LIB_CONSOLE_EVENT_RAW_KEY) {
        lib_test_assert(raw_count < 16);
        raw_repeat[raw_count] = event->value.raw_key.repeat_count;
        raw_unicode[raw_count++] = event->value.raw_key.unicode;
        return;
    }
    lib_test_assert(event->kind == LIB_CONSOLE_EVENT_IO_FAILURE);
    ++failures;
}
int main(void)
{
    console_broker_backend backend = { 0 };
    lib_test_assert(lib_console_create(&backend.console) == LIB_STATUS_OK);
    lib_test_assert(lib_console_set_event_sink(backend.console, receive, LIB_NULL) == LIB_STATUS_OK);
    lib_test_assert(lib_console_bind_generation(backend.console, 1u) == LIB_STATUS_OK);
    backend.generation = 1u;
    backend.stop_event = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    lib_test_assert(backend.stop_event);
    for (lib_i32 mode = 0; mode != 2; ++mode) {
        backend.mode = mode == 0 ? CONSOLE_BROKER_RAW_EVENTS : CONSOLE_BROKER_COOKED_LINES;
        lib_win32_reset_event(backend.stop_event);
        console_broker_reader(&backend);
        lib_test_assert(failures == mode + 1);
        lib_win32_set_event(backend.stop_event);
        console_broker_reader(&backend);
        lib_test_assert(failures == mode + 1);
    }
    lib_win32_reset_event(backend.stop_event);
    backend.generation = 2u;
    console_broker_reader(&backend);
    lib_test_assert(failures == 2);
    backend.generation = 1u;
    backend.mode = CONSOLE_BROKER_RAW_EVENTS;
    record_count = 5;
    for (lib_u32 i = 0; i < record_count; ++i) {
        records[i].EventType = LIB_WIN32_KEY_EVENT;
        records[i].Event.KeyEvent.bKeyDown = LIB_WIN32_TRUE;
        records[i].Event.KeyEvent.wRepeatCount = i == 0 ? 5 : 1;
        records[i].Event.KeyEvent.uChar.UnicodeChar =
            i == 0 ? 0x4e00 : i == 1 ? 0xd83d : i == 2 ? 0xde00 : 0;
    }
    records[3].Event.KeyEvent.bKeyDown = LIB_WIN32_FALSE;
    records[3].Event.KeyEvent.wRepeatCount = 5; /* one physical release */
    records[4].Event.KeyEvent.wRepeatCount = 0; /* preserve synthetic single record */
    console_broker_reader(&backend);
    lib_test_assert(record_index == 5 && raw_count == 5 && failures == 3);
    lib_test_assert(raw_unicode[0] == 0x4e00 && raw_repeat[0] == 5);
    lib_test_assert(raw_unicode[1] == 0xd83d && raw_unicode[2] == 0xde00);
    lib_test_assert(raw_repeat[1] == 1 && raw_repeat[2] == 1);
    lib_test_assert(raw_repeat[3] == 5 && raw_repeat[4] == 0);
    lib_win32_close_handle(backend.stop_event);
    lib_console_release(backend.console);
    return 0;
}
