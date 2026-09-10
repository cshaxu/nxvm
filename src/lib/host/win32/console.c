#include "lib/base/internal/console.h"
#include "lib/host/internal/console_native.h"

#ifdef _WIN32
#include <windows.h>

struct host_console_native {
    HANDLE input;
    HANDLE output;
    HANDLE stop_event;
    HANDLE reader;
    /* A cooked reader owns exactly one native line.  It clears this before
     * delivering that line, so host can safely rearm after the app consumes
     * it without confusing an in-flight line with a thread that is merely
     * returning from its callback. */
    volatile LONG cooked_line_pending;
    CRITICAL_SECTION output_lock;
    DWORD original_mode;
    lib_console *console;
    host_console_mode mode;
    lib_u32 generation;
    lib_u8 previous[LIB_CONSOLE_TEXT_COLUMNS * LIB_CONSOLE_TEXT_ROWS];
    lib_u16 previous_attributes[LIB_CONSOLE_TEXT_COLUMNS * LIB_CONSOLE_TEXT_ROWS];
    lib_u32 previous_palette[16u];
    lib_u16 previous_columns;
    lib_u16 previous_rows;
};

/* This is a liveness boundary, not an input debounce or product timer.
 * Normal cancellation completes immediately.  If the native Console does not
 * retire its sole reader in this interval, host I/O is no longer trustworthy
 * and the broker fails closed instead of hanging the control thread. */
#define HOST_CONSOLE_READER_RETIRE_TIMEOUT_MS 2000u

static COLORREF host_console_colorref_from_rgb(lib_u32 rgb)
{
    return RGB((rgb >> 16u) & 0xffu, (rgb >> 8u) & 0xffu, rgb & 0xffu);
}

static int host_console_ensure_text_surface(HANDLE output)
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    COORD required;
    SMALL_RECT viewport = { 0, 0, LIB_CONSOLE_TEXT_COLUMNS - 1,
        LIB_CONSOLE_TEXT_ROWS - 1 };

    if (output == NULL || output == INVALID_HANDLE_VALUE ||
        !GetConsoleScreenBufferInfo(output, &info)) return 0;
    required.X = info.dwSize.X < (SHORT)LIB_CONSOLE_TEXT_COLUMNS ?
        (SHORT)LIB_CONSOLE_TEXT_COLUMNS : info.dwSize.X;
    required.Y = info.dwSize.Y < (SHORT)LIB_CONSOLE_TEXT_ROWS ?
        (SHORT)LIB_CONSOLE_TEXT_ROWS : info.dwSize.Y;
    if ((required.X != info.dwSize.X || required.Y != info.dwSize.Y) &&
        !SetConsoleScreenBufferSize(output, required)) return 0;
    (void)SetConsoleWindowInfo(output, TRUE, &viewport);
    return 1;
}

static lib_u8 host_console_modifiers(DWORD state)
{
    lib_u8 modifiers = 0u;
    if ((state & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) != 0u)
        modifiers |= LIB_CONSOLE_MODIFIER_CONTROL;
    if ((state & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) != 0u)
        modifiers |= LIB_CONSOLE_MODIFIER_ALT;
    if ((state & SHIFT_PRESSED) != 0u)
        modifiers |= LIB_CONSOLE_MODIFIER_SHIFT;
    return modifiers;
}

static void host_console_emit_key(host_console_native *native_console,
    const KEY_EVENT_RECORD *key)
{
    lib_console_event event = { 0 };
    event.kind = LIB_CONSOLE_EVENT_RAW_KEY;
    event.binding_generation = native_console->generation;
    event.value.raw_key.key = key->wVirtualKeyCode;
    event.value.raw_key.unicode = key->uChar.UnicodeChar;
    event.value.raw_key.scan_code = key->wVirtualScanCode;
    event.value.raw_key.modifiers = host_console_modifiers(key->dwControlKeyState);
    event.value.raw_key.extended =
        (key->dwControlKeyState & ENHANCED_KEY) != 0u ? LIB_TRUE : LIB_FALSE;
    event.value.raw_key.pressed = key->bKeyDown ? LIB_TRUE : LIB_FALSE;
    (void)lib_console_deliver_event(native_console->console, &event);
}

static void host_console_emit_mouse(host_console_native *native_console,
    const MOUSE_EVENT_RECORD *mouse)
{
    lib_console_event event = { 0 };
    event.kind = LIB_CONSOLE_EVENT_RAW_MOUSE;
    event.binding_generation = native_console->generation;
    event.value.raw_mouse.delta_x = mouse->dwMousePosition.X;
    event.value.raw_mouse.delta_y = mouse->dwMousePosition.Y;
    event.value.raw_mouse.buttons = mouse->dwButtonState;
    (void)lib_console_deliver_event(native_console->console, &event);
}

static DWORD WINAPI host_console_reader(void *context)
{
    host_console_native *native_console = (host_console_native *)context;
    if (native_console->mode == HOST_CONSOLE_COOKED_LINES) {
        char text[LIB_CONSOLE_LINE_MAX];
        DWORD read = 0u;
        lib_console_event event = { 0 };
        if (!ReadConsoleA(native_console->input, text,
                LIB_CONSOLE_LINE_MAX - 1u, &read, NULL)) {
            InterlockedExchange(&native_console->cooked_line_pending, 0);
            return 0u;
        }
        InterlockedExchange(&native_console->cooked_line_pending, 0);
        if (WaitForSingleObject(native_console->stop_event, 0u) == WAIT_OBJECT_0)
            return 0u;
        while (read != 0u && (text[read - 1u] == '\r' || text[read - 1u] == '\n'))
            --read;
        event.kind = LIB_CONSOLE_EVENT_COOKED_LINE;
        event.binding_generation = native_console->generation;
        event.value.line.length = read;
        memcpy(event.value.line.text, text, read);
        event.value.line.text[read] = '\0';
        (void)lib_console_deliver_event(native_console->console, &event);
    } else {
        HANDLE waits[2] = { native_console->stop_event, native_console->input };
        while (WaitForMultipleObjects(2u, waits, FALSE, INFINITE) == WAIT_OBJECT_0 + 1u) {
            INPUT_RECORD record;
            DWORD read = 0u;
            if (!ReadConsoleInputA(native_console->input, &record, 1u, &read)) break;
            if (record.EventType == KEY_EVENT) host_console_emit_key(native_console,
                &record.Event.KeyEvent);
            else if (record.EventType == MOUSE_EVENT) host_console_emit_mouse(native_console,
                &record.Event.MouseEvent);
        }
    }
    return 0u;
}

lib_status host_console_native_create(host_console_native **out_native)
{
    host_console_native *native_console;
    DWORD mode;
    if (out_native == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_native = LIB_NULL;
    native_console = calloc(1u, sizeof(*native_console));
    if (native_console == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    native_console->input = CreateFileA("CONIN$", GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    native_console->output = CreateFileA("CONOUT$", GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (native_console->input == INVALID_HANDLE_VALUE ||
        native_console->output == INVALID_HANDLE_VALUE ||
        !GetConsoleMode(native_console->input, &mode)) {
        if (native_console->input != INVALID_HANDLE_VALUE) CloseHandle(native_console->input);
        if (native_console->output != INVALID_HANDLE_VALUE) CloseHandle(native_console->output);
        free(native_console);
        return LIB_STATUS_UNSUPPORTED;
    }
    native_console->original_mode = mode;
    InitializeCriticalSection(&native_console->output_lock);
    *out_native = native_console;
    return LIB_STATUS_OK;
}

void host_console_native_destroy(host_console_native *native_console)
{
    lib_status status;
    if (native_console == LIB_NULL) return;
    status = host_console_native_deactivate(native_console);
    if (status != LIB_STATUS_OK) {
        /* A live reader still owns native_console and its logical Console.
           Broker failure is terminal and the process must exit; intentionally
           retain these process-lifetime resources rather than free storage
           underneath a live native worker. */
        return;
    }
    if (native_console->input != INVALID_HANDLE_VALUE) CloseHandle(native_console->input);
    if (native_console->output != INVALID_HANDLE_VALUE) CloseHandle(native_console->output);
    DeleteCriticalSection(&native_console->output_lock);
    free(native_console);
}

static lib_status host_console_start_reader(host_console_native *native_console)
{
    if (native_console->mode == HOST_CONSOLE_COOKED_LINES)
        InterlockedExchange(&native_console->cooked_line_pending, 1);
    native_console->reader = CreateThread(NULL, 0u, host_console_reader,
        native_console, 0u, NULL);
    if (native_console->reader == NULL) {
        InterlockedExchange(&native_console->cooked_line_pending, 0);
        return LIB_STATUS_NO_MEMORY;
    }
    return LIB_STATUS_OK;
}

/* The native raw VM Console is the input surface.  This historical behavior
 * is coupled to successful raw activation rather than exposed as a product
 * API: the broker owns the one process Console handle and knows whether a
 * live raw reader exists.  Cooked activation intentionally does not make a
 * native foreground request. */
static void host_console_activate_raw_input_surface(
    host_console_native *native_console)
{
    HWND window;

    if (native_console == LIB_NULL ||
        native_console->mode != HOST_CONSOLE_RAW_EVENTS ||
        native_console->console == LIB_NULL || native_console->reader == NULL)
        return;
    window = GetConsoleWindow();
    if (window == NULL) return;
    if (IsIconic(window)) (void)ShowWindow(window, SW_RESTORE);
    (void)SetForegroundWindow(window);
    (void)SetActiveWindow(window);
    (void)SetFocus(window);
}

lib_status host_console_native_prepare(host_console_native *native_console,
    lib_console *console, host_console_mode mode)
{
    DWORD ignored;
    if (native_console == LIB_NULL || console == LIB_NULL ||
        (mode != HOST_CONSOLE_RAW_EVENTS && mode != HOST_CONSOLE_COOKED_LINES))
        return LIB_STATUS_INVALID_ARGUMENT;
    if (native_console->input == INVALID_HANDLE_VALUE ||
        native_console->output == INVALID_HANDLE_VALUE ||
        !GetConsoleMode(native_console->input, &ignored)) return LIB_STATUS_IO_ERROR;
    return LIB_STATUS_OK;
}

void host_console_native_discard_prepare(host_console_native *native_console)
{ (void)native_console; }

lib_status host_console_native_activate(host_console_native *native_console,
    lib_console *console, host_console_mode mode, lib_u32 generation)
{
    DWORD configured;
    if (native_console == LIB_NULL || console == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    configured = native_console->original_mode;
    if (mode == HOST_CONSOLE_RAW_EVENTS)
        /* ReadConsoleInput consumes classic INPUT_RECORD values.  A parent
         * Windows Terminal may have enabled VT input; retaining that flag
         * converts keyboard input into byte sequences for ReadConsole/ReadFile
         * instead, leaving this raw reader with no KEY_EVENT records. */
        configured = (configured & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT |
            ENABLE_PROCESSED_INPUT | ENABLE_QUICK_EDIT_MODE |
            ENABLE_VIRTUAL_TERMINAL_INPUT)) |
            ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS;
    else configured |= ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT;
    if (!SetConsoleMode(native_console->input, configured)) return LIB_STATUS_IO_ERROR;
    /* A Current-Console cutover has one ownership boundary regardless of
     * input mode.  Records already buffered before it belong to neither the
     * old nor the newly activated logical Console, so discard them before
     * starting the new reader. */
    if (!FlushConsoleInputBuffer(native_console->input))
        return LIB_STATUS_IO_ERROR;
    native_console->stop_event = CreateEventA(NULL, TRUE, FALSE, NULL);
    if (native_console->stop_event == NULL) return LIB_STATUS_NO_MEMORY;
    native_console->console = console;
    native_console->mode = mode;
    native_console->generation = generation;
    memset(native_console->previous, 0xff, sizeof(native_console->previous));
    memset(native_console->previous_attributes, 0xff,
        sizeof(native_console->previous_attributes));
    memset(native_console->previous_palette, 0xff,
        sizeof(native_console->previous_palette));
    native_console->previous_columns = 0u;
    native_console->previous_rows = 0u;
    /* Cooked mode is a monitor surface, not an input request.  Its reader is
       armed solely by host_console_native_request_cooked_line() after the app
       has actually published a prompt.  Starting ReadConsoleA here leaves a
       hidden line reader alive while a graphic Window is running; it can eat
       the first Enter after the later raw takeover. */
    if (mode == HOST_CONSOLE_RAW_EVENTS) {
        if (host_console_start_reader(native_console) != LIB_STATUS_OK) {
            CloseHandle(native_console->stop_event);
            native_console->stop_event = NULL;
            native_console->console = LIB_NULL;
            return LIB_STATUS_NO_MEMORY;
        }
        host_console_activate_raw_input_surface(native_console);
    }
    return LIB_STATUS_OK;
}

lib_status host_console_native_request_cooked_line(
    host_console_native *native_console)
{
    DWORD completed;

    if (native_console == LIB_NULL ||
        native_console->mode != HOST_CONSOLE_COOKED_LINES ||
        native_console->console == LIB_NULL || native_console->stop_event == NULL)
        return LIB_STATUS_INVALID_STATE;
    if (native_console->reader != NULL) {
        if (InterlockedCompareExchange(&native_console->cooked_line_pending,
                0, 0) != 0)
            return LIB_STATUS_OK;
        completed = WaitForSingleObject(native_console->reader, 0u);
        if (completed == WAIT_TIMEOUT) {
            completed = WaitForSingleObject(native_console->reader, INFINITE);
        }
        if (completed != WAIT_OBJECT_0) return LIB_STATUS_IO_ERROR;
        CloseHandle(native_console->reader);
        native_console->reader = NULL;
    }
    return host_console_start_reader(native_console);
}

static lib_status host_console_retire_reader(host_console_native *native_console)
{
    DWORD completed;

    if (native_console == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (native_console->reader == NULL) return LIB_STATUS_OK;
    if (native_console->stop_event == NULL ||
        !SetEvent(native_console->stop_event)) return LIB_STATUS_IO_ERROR;
    /* A reader owns the process Console until it has exited.  Cancellation is
       deliberately best effort because Console implementations differ; the
       completion observation below is the actual ownership proof. */
    (void)CancelIoEx(native_console->input, NULL);
    (void)CancelSynchronousIo(native_console->reader);
    completed = WaitForSingleObject(native_console->reader, 0u);
    if (completed == WAIT_TIMEOUT && native_console->mode == HOST_CONSOLE_COOKED_LINES) {
        INPUT_RECORD wake = { 0 };
        DWORD written = 0u;
        /* Some terminal hosts do not interrupt a line-buffered ReadConsoleA
           even after both documented cancellation requests.  Supply the
           terminating record ourselves while the old reader is still the
           sole owner.  It observes stop_event and discards this synthetic
           line; the new binding cannot observe it. */
        wake.EventType = KEY_EVENT;
        wake.Event.KeyEvent.bKeyDown = TRUE;
        wake.Event.KeyEvent.wRepeatCount = 1u;
        wake.Event.KeyEvent.wVirtualKeyCode = VK_RETURN;
        wake.Event.KeyEvent.wVirtualScanCode = (WORD)MapVirtualKeyA(VK_RETURN,
            MAPVK_VK_TO_VSC);
        wake.Event.KeyEvent.uChar.AsciiChar = '\r';
        if (!WriteConsoleInputA(native_console->input, &wake, 1u, &written) ||
            written != 1u) return LIB_STATUS_IO_ERROR;
    }
    completed = WaitForSingleObject(native_console->reader,
        HOST_CONSOLE_READER_RETIRE_TIMEOUT_MS);
    if (completed != WAIT_OBJECT_0) return LIB_STATUS_IO_ERROR;
    CloseHandle(native_console->reader);
    native_console->reader = NULL;
    InterlockedExchange(&native_console->cooked_line_pending, 0);
    return LIB_STATUS_OK;
}

lib_status host_console_native_deactivate(host_console_native *native_console)
{
    lib_status status;

    if (native_console == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = host_console_retire_reader(native_console);
    if (status != LIB_STATUS_OK) return status;
    if (native_console->stop_event != NULL) CloseHandle(native_console->stop_event);
    native_console->stop_event = NULL;
    native_console->console = LIB_NULL;
    native_console->generation = 0u;
    return SetConsoleMode(native_console->input, native_console->original_mode) ?
        LIB_STATUS_OK : LIB_STATUS_IO_ERROR;
}

void host_console_native_lock_output(host_console_native *native_console)
{
    if (native_console != LIB_NULL) EnterCriticalSection(&native_console->output_lock);
}

void host_console_native_unlock_output(host_console_native *native_console)
{
    if (native_console != LIB_NULL) LeaveCriticalSection(&native_console->output_lock);
}

lib_status host_console_native_write_bound(host_console_native *native_console,
    lib_console *expected_console, lib_u32 expected_generation, const char *text,
    lib_size length)
{
    DWORD written = 0u;
    if (native_console == LIB_NULL || (text == LIB_NULL && length != 0u) ||
        length > (lib_size)UINT32_MAX) return LIB_STATUS_INVALID_ARGUMENT;
    host_console_native_lock_output(native_console);
    if (native_console->console != expected_console ||
        native_console->generation != expected_generation) {
        host_console_native_unlock_output(native_console);
        return LIB_STATUS_NOT_CURRENT;
    }
    {
        lib_status status = WriteConsoleA(native_console->output, text, (DWORD)length,
            &written, NULL) && written == (DWORD)length ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR;
        host_console_native_unlock_output(native_console);
        return status;
    }
}

lib_status host_console_native_write_text_frame_bound(host_console_native *native_console,
    lib_console *expected_console, lib_u32 expected_generation,
    const lib_console_text_frame *frame)
{
    CHAR_INFO cells[LIB_CONSOLE_TEXT_COLUMNS * LIB_CONSOLE_TEXT_ROWS];
    COORD size = { LIB_CONSOLE_TEXT_COLUMNS, LIB_CONSOLE_TEXT_ROWS };
    COORD position = { 0, 0 };
    SMALL_RECT region = { 0, 0, LIB_CONSOLE_TEXT_COLUMNS - 1,
        LIB_CONSOLE_TEXT_ROWS - 1 };
    lib_u32 row;

    if (native_console == LIB_NULL || frame == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    host_console_native_lock_output(native_console);
    if (native_console->console != expected_console ||
        native_console->generation != expected_generation) {
        host_console_native_unlock_output(native_console);
        return LIB_STATUS_NOT_CURRENT;
    }
    if (!host_console_ensure_text_surface(native_console->output)) {
        host_console_native_unlock_output(native_console);
        return LIB_STATUS_IO_ERROR;
    }
    if (memcmp(frame->palette, native_console->previous_palette,
            sizeof(frame->palette)) != 0) {
        CONSOLE_SCREEN_BUFFER_INFOEX info;
        lib_u32 index;

        memset(&info, 0, sizeof(info));
        info.cbSize = sizeof(info);
        if (GetConsoleScreenBufferInfoEx(native_console->output, &info)) {
            for (index = 0u; index < 16u; ++index)
                info.ColorTable[index] = host_console_colorref_from_rgb(
                    frame->palette[index]);
            (void)SetConsoleScreenBufferInfoEx(native_console->output, &info);
        }
        memcpy(native_console->previous_palette, frame->palette,
            sizeof(frame->palette));
    }
    if (native_console->previous_columns != frame->columns ||
        native_console->previous_rows != frame->rows ||
        memcmp(frame->text, native_console->previous,
            sizeof(frame->text)) != 0 ||
        memcmp(frame->attributes, native_console->previous_attributes,
            sizeof(frame->attributes)) != 0) {
        for (row = 0u; row < LIB_CONSOLE_TEXT_ROWS; ++row) {
            lib_u32 column;
            for (column = 0u; column < LIB_CONSOLE_TEXT_COLUMNS; ++column) {
                lib_size offset = (lib_size)row * LIB_CONSOLE_TEXT_COLUMNS + column;
                cells[offset].Char.AsciiChar = row < frame->rows &&
                    column < frame->columns && frame->text[offset] >= 0x20u &&
                    frame->text[offset] < 0x7fu ? (CHAR)frame->text[offset] : ' ';
                cells[offset].Attributes = (WORD)(row < frame->rows &&
                    column < frame->columns ? frame->attributes[offset] : 0u);
            }
        }
        if (!WriteConsoleOutputA(native_console->output, cells, size, position,
                &region)) {
            host_console_native_unlock_output(native_console);
            return LIB_STATUS_IO_ERROR;
        }
        memcpy(native_console->previous, frame->text, sizeof(frame->text));
        memcpy(native_console->previous_attributes, frame->attributes,
            sizeof(frame->attributes));
        native_console->previous_columns = frame->columns;
        native_console->previous_rows = frame->rows;
    }
    {
        CONSOLE_CURSOR_INFO cursor;
        cursor.dwSize = frame->cursor_bottom >= frame->cursor_top &&
            frame->font_height != 0u ? (DWORD)((frame->cursor_bottom -
                frame->cursor_top + 1u) * 100u / frame->font_height) : 100u;
        if (cursor.dwSize == 0u || cursor.dwSize > 100u) cursor.dwSize = 100u;
        cursor.bVisible = frame->cursor_visible != 0u && frame->cursor_phase != 0u &&
            frame->cursor_column >= 0 && frame->cursor_row >= 0 &&
            frame->cursor_column < (lib_i32)frame->columns &&
            frame->cursor_row < (lib_i32)frame->rows;
        if (cursor.bVisible) {
            position.X = (SHORT)frame->cursor_column;
            position.Y = (SHORT)frame->cursor_row;
            (void)SetConsoleCursorPosition(native_console->output, position);
        }
        (void)SetConsoleCursorInfo(native_console->output, &cursor);
    }
    host_console_native_unlock_output(native_console);
    return LIB_STATUS_OK;
}
#endif
