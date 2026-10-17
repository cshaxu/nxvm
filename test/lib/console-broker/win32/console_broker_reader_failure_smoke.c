#include "lib/types/test.h"
#include "lib/types/win32/test.h"
#include "lib/types/win32/console.h"
#include "lib/types/win32/sync.h"
#include "lib/console/binding_interface.h"

static lib_win32_bool LIB_WIN32_WINAPI failed_read(lib_win32_handle input, lib_win32_lpvoid bytes, lib_win32_dword length,
    lib_win32_lpdword read, lib_win32_lpvoid reserved)
{ (void)input; (void)bytes; (void)length; (void)read; (void)reserved; return LIB_WIN32_FALSE; }
static lib_win32_input_record records[5];
static lib_u32 record_count, record_index, raw_count;
static lib_u32 raw_unicode[16], raw_repeat[16];
static lib_key_input raw_keys[16];
static lib_console_raw_mouse raw_mouse;
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
#undef lib_win32_read_console_w
#undef lib_win32_wait_for_multiple_objects
#define lib_win32_read_console_w failed_read
#define lib_win32_wait_for_multiple_objects failed_wait
#include "lib/console-broker/win32/console.c"

static lib_i32 failures;
static void receive(void *context, const lib_console_event *event)
{
    (void)context;
    if (event->kind == LIB_CONSOLE_EVENT_INPUT_RESET) return;
    if (event->kind == LIB_CONSOLE_EVENT_RAW_MOUSE) {
        raw_mouse = event->value.raw_mouse;
        return;
    }
    if (event->kind == LIB_CONSOLE_EVENT_RAW_KEY) {
        lib_test_assert(raw_count < 16);
        raw_keys[raw_count] = event->value.raw_key;
        raw_repeat[raw_count] = event->value.raw_key.repeat_count;
        raw_unicode[raw_count++] = event->value.raw_key.scalar;
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
    records[3].Event.KeyEvent.wVirtualKeyCode = 'A';
    records[3].Event.KeyEvent.wVirtualScanCode = 0x1e;
    records[4].Event.KeyEvent.wRepeatCount = 0; /* preserve synthetic single record */
    records[4].Event.KeyEvent.wVirtualKeyCode = LIB_WIN32_KEY_CONTROL;
    records[4].Event.KeyEvent.wVirtualScanCode = 0x1d;
    records[4].Event.KeyEvent.dwControlKeyState = LIB_WIN32_ENHANCED_KEY | LIB_WIN32_RIGHT_CTRL_PRESSED;
    console_broker_reader(&backend);
    lib_test_assert(record_index == 5 && raw_count == 4 && failures == 3);
    lib_test_assert(raw_unicode[0] == 0x4e00 && raw_repeat[0] == 5);
    lib_test_assert(raw_unicode[1] == 0x1f600 && raw_unicode[2] == 0);
    lib_test_assert(raw_repeat[1] == 1 && raw_repeat[2] == 1);
    lib_test_assert(raw_repeat[3] == 1 && !raw_keys[2].pressed);
    lib_test_assert(raw_keys[3].key == LIB_KEY_CONTROL && raw_keys[3].extended &&
        raw_keys[3].scan_code == 0x1d && raw_keys[3].modifiers == LIB_KEY_MODIFIER_CONTROL);
    /* A binding reset discards an incomplete UTF-16 prefix in the producer,
     * never exposing either half to a public Console sink. */
    lib_win32_key_event_record key = {0};
    key.bKeyDown = LIB_WIN32_TRUE; key.wRepeatCount = 2;
    key.uChar.UnicodeChar = 0xd83d;
    lib_test_assert(console_broker_emit_key(&backend, &key) == LIB_STATUS_OK);
    lib_test_assert(raw_count == 4);
    lib_test_assert(console_broker_emit_input_reset(&backend) == LIB_STATUS_OK);
    key.uChar.UnicodeChar = 0xde00;
    lib_test_assert(console_broker_emit_key(&backend, &key) == LIB_STATUS_OK && raw_count == 4);
    key.uChar.UnicodeChar = 0xd83d;
    lib_test_assert(console_broker_emit_key(&backend, &key) == LIB_STATUS_OK);
    key.uChar.UnicodeChar = 0xde00;
    lib_test_assert(console_broker_emit_key(&backend, &key) == LIB_STATUS_OK);
    lib_test_assert(raw_count == 5 && raw_unicode[4] == 0x1f600 && raw_repeat[4] == 2);
    lib_win32_mouse_event_record mouse = {0};
    mouse.dwMousePosition.X = 7; mouse.dwMousePosition.Y = 9;
    mouse.dwButtonState = LIB_WIN32_FROM_LEFT_1ST_BUTTON_PRESSED | LIB_WIN32_RIGHTMOST_BUTTON_PRESSED;
    lib_test_assert(console_broker_emit_mouse(&backend, &mouse) == LIB_STATUS_OK);
    lib_test_assert(raw_mouse.column == 7 && raw_mouse.row == 9 &&
        raw_mouse.buttons == (LIB_MOUSE_BUTTON_LEFT | LIB_MOUSE_BUTTON_RIGHT));
    const lib_win32_dword native_buttons[] = { 0u,
        LIB_WIN32_FROM_LEFT_1ST_BUTTON_PRESSED, LIB_WIN32_RIGHTMOST_BUTTON_PRESSED,
        LIB_WIN32_FROM_LEFT_2ND_BUTTON_PRESSED };
    const lib_u32 neutral_buttons[] = { 0u, LIB_MOUSE_BUTTON_LEFT, LIB_MOUSE_BUTTON_RIGHT, 0u };
    for (lib_size i = 0; i < sizeof(native_buttons) / sizeof(native_buttons[0]); ++i) {
        mouse.dwButtonState = native_buttons[i];
        lib_test_assert(console_broker_emit_mouse(&backend, &mouse) == LIB_STATUS_OK);
        lib_test_assert(raw_mouse.buttons == neutral_buttons[i]);
    }
    lib_win32_close_handle(backend.stop_event);
    lib_console_release(backend.console);
    return 0;
}
