#include "lib/console/console.h"
#include "lib/host/console_backend.h"

#ifdef _WIN32
#include <windows.h>
#include <stdlib.h>
#include <string.h>

struct host_console_backend {
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

static void host_console_emit_key(host_console_backend *backend,
    const KEY_EVENT_RECORD *key)
{
    lib_console_event event = { 0 };
    event.kind = LIB_CONSOLE_EVENT_RAW_KEY;
    event.binding_generation = backend->generation;
    event.value.raw_key.key = key->wVirtualKeyCode;
    event.value.raw_key.unicode = key->uChar.UnicodeChar;
    event.value.raw_key.scan_code = key->wVirtualScanCode;
    event.value.raw_key.modifiers = host_console_modifiers(key->dwControlKeyState);
    event.value.raw_key.extended =
        (key->dwControlKeyState & ENHANCED_KEY) != 0u ? LIB_TRUE : LIB_FALSE;
    event.value.raw_key.pressed = key->bKeyDown ? LIB_TRUE : LIB_FALSE;
    (void)lib_console_deliver_event(backend->console, &event);
}

static void host_console_emit_mouse(host_console_backend *backend,
    const MOUSE_EVENT_RECORD *mouse)
{
    lib_console_event event = { 0 };
    event.kind = LIB_CONSOLE_EVENT_RAW_MOUSE;
    event.binding_generation = backend->generation;
    event.value.raw_mouse.delta_x = mouse->dwMousePosition.X;
    event.value.raw_mouse.delta_y = mouse->dwMousePosition.Y;
    event.value.raw_mouse.buttons = mouse->dwButtonState;
    (void)lib_console_deliver_event(backend->console, &event);
}

static DWORD WINAPI host_console_reader(void *context)
{
    host_console_backend *backend = (host_console_backend *)context;
    if (backend->mode == HOST_CONSOLE_COOKED_LINES) {
        char text[LIB_CONSOLE_LINE_MAX];
        DWORD read = 0u;
        lib_console_event event = { 0 };
        if (!ReadConsoleA(backend->input, text,
                LIB_CONSOLE_LINE_MAX - 1u, &read, NULL)) {
            InterlockedExchange(&backend->cooked_line_pending, 0);
            return 0u;
        }
        InterlockedExchange(&backend->cooked_line_pending, 0);
        if (WaitForSingleObject(backend->stop_event, 0u) == WAIT_OBJECT_0)
            return 0u;
        while (read != 0u && (text[read - 1u] == '\r' || text[read - 1u] == '\n'))
            --read;
        event.kind = LIB_CONSOLE_EVENT_COOKED_LINE;
        event.binding_generation = backend->generation;
        event.value.line.length = read;
        memcpy(event.value.line.text, text, read);
        event.value.line.text[read] = '\0';
        (void)lib_console_deliver_event(backend->console, &event);
    } else {
        HANDLE waits[2] = { backend->stop_event, backend->input };
        while (WaitForMultipleObjects(2u, waits, FALSE, INFINITE) == WAIT_OBJECT_0 + 1u) {
            INPUT_RECORD record;
            DWORD read = 0u;
            if (!ReadConsoleInputA(backend->input, &record, 1u, &read)) break;
            if (record.EventType == KEY_EVENT) host_console_emit_key(backend,
                &record.Event.KeyEvent);
            else if (record.EventType == MOUSE_EVENT) host_console_emit_mouse(backend,
                &record.Event.MouseEvent);
        }
    }
    return 0u;
}

lib_status host_console_backend_create(host_console_backend **out_backend)
{
    host_console_backend *backend;
    DWORD mode;
    if (out_backend == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_backend = LIB_NULL;
    backend = calloc(1u, sizeof(*backend));
    if (backend == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    backend->input = CreateFileA("CONIN$", GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    backend->output = CreateFileA("CONOUT$", GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (backend->input == INVALID_HANDLE_VALUE ||
        backend->output == INVALID_HANDLE_VALUE ||
        !GetConsoleMode(backend->input, &mode)) {
        if (backend->input != INVALID_HANDLE_VALUE) CloseHandle(backend->input);
        if (backend->output != INVALID_HANDLE_VALUE) CloseHandle(backend->output);
        free(backend);
        return LIB_STATUS_UNSUPPORTED;
    }
    backend->original_mode = mode;
    InitializeCriticalSection(&backend->output_lock);
    *out_backend = backend;
    return LIB_STATUS_OK;
}

void host_console_backend_destroy(host_console_backend *backend)
{
    lib_status status;
    if (backend == LIB_NULL) return;
    status = host_console_backend_deactivate(backend);
    if (status != LIB_STATUS_OK) {
        /* A live reader still owns backend and its logical Console.
           Broker failure is terminal and the process must exit; intentionally
           retain these process-lifetime resources rather than free storage
           underneath a live native worker. */
        return;
    }
    if (backend->input != INVALID_HANDLE_VALUE) CloseHandle(backend->input);
    if (backend->output != INVALID_HANDLE_VALUE) CloseHandle(backend->output);
    DeleteCriticalSection(&backend->output_lock);
    free(backend);
}

static lib_status host_console_start_reader(host_console_backend *backend)
{
    if (backend->mode == HOST_CONSOLE_COOKED_LINES)
        InterlockedExchange(&backend->cooked_line_pending, 1);
    backend->reader = CreateThread(NULL, 0u, host_console_reader,
        backend, 0u, NULL);
    if (backend->reader == NULL) {
        InterlockedExchange(&backend->cooked_line_pending, 0);
        return LIB_STATUS_NO_MEMORY;
    }
    return LIB_STATUS_OK;
}

/* The native raw Console is the input surface. This behavior
 * is coupled to successful raw activation rather than exposed as a product
 * API: the broker owns the one process Console handle and knows whether a
 * live raw reader exists.  Cooked activation intentionally does not make a
 * native foreground request. */
static void host_console_activate_raw_input_surface(
    host_console_backend *backend)
{
    HWND window;

    if (backend == LIB_NULL ||
        backend->mode != HOST_CONSOLE_RAW_EVENTS ||
        backend->console == LIB_NULL || backend->reader == NULL)
        return;
    window = GetConsoleWindow();
    if (window == NULL) return;
    if (IsIconic(window)) (void)ShowWindow(window, SW_RESTORE);
    (void)SetForegroundWindow(window);
    (void)SetActiveWindow(window);
    (void)SetFocus(window);
}

lib_status host_console_backend_prepare(host_console_backend *backend,
    lib_console *console, host_console_mode mode)
{
    DWORD ignored;
    if (backend == LIB_NULL || console == LIB_NULL ||
        (mode != HOST_CONSOLE_RAW_EVENTS && mode != HOST_CONSOLE_COOKED_LINES))
        return LIB_STATUS_INVALID_ARGUMENT;
    if (backend->input == INVALID_HANDLE_VALUE ||
        backend->output == INVALID_HANDLE_VALUE ||
        !GetConsoleMode(backend->input, &ignored)) return LIB_STATUS_IO_ERROR;
    return LIB_STATUS_OK;
}

void host_console_backend_discard_prepare(host_console_backend *backend)
{ (void)backend; }

lib_status host_console_backend_activate(host_console_backend *backend,
    lib_console *console, host_console_mode mode, lib_u32 generation)
{
    DWORD configured;
    if (backend == LIB_NULL || console == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    configured = backend->original_mode;
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
    if (!SetConsoleMode(backend->input, configured)) return LIB_STATUS_IO_ERROR;
    /* A Current-Console cutover has one ownership boundary regardless of
     * input mode.  Records already buffered before it belong to neither the
     * old nor the newly activated logical Console, so discard them before
     * starting the new reader. */
    if (!FlushConsoleInputBuffer(backend->input))
        return LIB_STATUS_IO_ERROR;
    backend->stop_event = CreateEventA(NULL, TRUE, FALSE, NULL);
    if (backend->stop_event == NULL) return LIB_STATUS_NO_MEMORY;
    backend->console = console;
    backend->mode = mode;
    backend->generation = generation;
    memset(backend->previous, 0xff, sizeof(backend->previous));
    memset(backend->previous_attributes, 0xff,
        sizeof(backend->previous_attributes));
    memset(backend->previous_palette, 0xff,
        sizeof(backend->previous_palette));
    backend->previous_columns = 0u;
    backend->previous_rows = 0u;
    /* Cooked mode is a monitor surface, not an input request.  Its reader is
       armed solely by host_console_backend_request_cooked_line() after the app
       has actually published a prompt.  Starting ReadConsoleA here leaves a
       hidden line reader alive while a graphic Window is running; it can eat
       the first Enter after the later raw takeover. */
    if (mode == HOST_CONSOLE_RAW_EVENTS) {
        if (host_console_start_reader(backend) != LIB_STATUS_OK) {
            CloseHandle(backend->stop_event);
            backend->stop_event = NULL;
            backend->console = LIB_NULL;
            return LIB_STATUS_NO_MEMORY;
        }
        host_console_activate_raw_input_surface(backend);
    }
    return LIB_STATUS_OK;
}

lib_status host_console_backend_request_cooked_line(
    host_console_backend *backend)
{
    DWORD completed;

    if (backend == LIB_NULL ||
        backend->mode != HOST_CONSOLE_COOKED_LINES ||
        backend->console == LIB_NULL || backend->stop_event == NULL)
        return LIB_STATUS_INVALID_STATE;
    if (backend->reader != NULL) {
        if (InterlockedCompareExchange(&backend->cooked_line_pending,
                0, 0) != 0)
            return LIB_STATUS_OK;
        completed = WaitForSingleObject(backend->reader, 0u);
        if (completed == WAIT_TIMEOUT) {
            completed = WaitForSingleObject(backend->reader, INFINITE);
        }
        if (completed != WAIT_OBJECT_0) return LIB_STATUS_IO_ERROR;
        CloseHandle(backend->reader);
        backend->reader = NULL;
    }
    return host_console_start_reader(backend);
}

static lib_status host_console_retire_reader(host_console_backend *backend)
{
    DWORD completed;

    if (backend == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (backend->reader == NULL) return LIB_STATUS_OK;
    if (backend->stop_event == NULL ||
        !SetEvent(backend->stop_event)) return LIB_STATUS_IO_ERROR;
    /* A reader owns the process Console until it has exited.  Cancellation is
       deliberately best effort because Console implementations differ; the
       completion observation below is the actual ownership proof. */
    (void)CancelIoEx(backend->input, NULL);
    (void)CancelSynchronousIo(backend->reader);
    completed = WaitForSingleObject(backend->reader, 0u);
    if (completed == WAIT_TIMEOUT && backend->mode == HOST_CONSOLE_COOKED_LINES) {
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
        if (!WriteConsoleInputA(backend->input, &wake, 1u, &written) ||
            written != 1u) return LIB_STATUS_IO_ERROR;
    }
    completed = WaitForSingleObject(backend->reader,
        HOST_CONSOLE_READER_RETIRE_TIMEOUT_MS);
    if (completed != WAIT_OBJECT_0) return LIB_STATUS_IO_ERROR;
    CloseHandle(backend->reader);
    backend->reader = NULL;
    InterlockedExchange(&backend->cooked_line_pending, 0);
    return LIB_STATUS_OK;
}

lib_status host_console_backend_deactivate(host_console_backend *backend)
{
    lib_status status;

    if (backend == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = host_console_retire_reader(backend);
    if (status != LIB_STATUS_OK) return status;
    if (backend->stop_event != NULL) CloseHandle(backend->stop_event);
    backend->stop_event = NULL;
    backend->console = LIB_NULL;
    backend->generation = 0u;
    return SetConsoleMode(backend->input, backend->original_mode) ?
        LIB_STATUS_OK : LIB_STATUS_IO_ERROR;
}

void host_console_backend_lock_output(host_console_backend *backend)
{
    if (backend != LIB_NULL) EnterCriticalSection(&backend->output_lock);
}

void host_console_backend_unlock_output(host_console_backend *backend)
{
    if (backend != LIB_NULL) LeaveCriticalSection(&backend->output_lock);
}

lib_status host_console_backend_write_bound(host_console_backend *backend,
    lib_console *expected_console, lib_u32 expected_generation, const char *text,
    lib_size length)
{
    DWORD written = 0u;
    if (backend == LIB_NULL || (text == LIB_NULL && length != 0u) ||
        length > (lib_size)UINT32_MAX) return LIB_STATUS_INVALID_ARGUMENT;
    host_console_backend_lock_output(backend);
    if (backend->console != expected_console ||
        backend->generation != expected_generation) {
        host_console_backend_unlock_output(backend);
        return LIB_STATUS_NOT_CURRENT;
    }
    {
        lib_status status = WriteConsoleA(backend->output, text, (DWORD)length,
            &written, NULL) && written == (DWORD)length ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR;
        host_console_backend_unlock_output(backend);
        return status;
    }
}

lib_status host_console_backend_write_text_frame_bound(host_console_backend *backend,
    lib_console *expected_console, lib_u32 expected_generation,
    const lib_console_text_frame *frame)
{
    CHAR_INFO cells[LIB_CONSOLE_TEXT_COLUMNS * LIB_CONSOLE_TEXT_ROWS];
    COORD size = { LIB_CONSOLE_TEXT_COLUMNS, LIB_CONSOLE_TEXT_ROWS };
    COORD position = { 0, 0 };
    SMALL_RECT region = { 0, 0, LIB_CONSOLE_TEXT_COLUMNS - 1,
        LIB_CONSOLE_TEXT_ROWS - 1 };
    lib_u32 row;

    if (backend == LIB_NULL || frame == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    host_console_backend_lock_output(backend);
    if (backend->console != expected_console ||
        backend->generation != expected_generation) {
        host_console_backend_unlock_output(backend);
        return LIB_STATUS_NOT_CURRENT;
    }
    if (!host_console_ensure_text_surface(backend->output)) {
        host_console_backend_unlock_output(backend);
        return LIB_STATUS_IO_ERROR;
    }
    if (memcmp(frame->palette, backend->previous_palette,
            sizeof(frame->palette)) != 0) {
        CONSOLE_SCREEN_BUFFER_INFOEX info;
        lib_u32 index;

        memset(&info, 0, sizeof(info));
        info.cbSize = sizeof(info);
        if (GetConsoleScreenBufferInfoEx(backend->output, &info)) {
            for (index = 0u; index < 16u; ++index)
                info.ColorTable[index] = host_console_colorref_from_rgb(
                    frame->palette[index]);
            (void)SetConsoleScreenBufferInfoEx(backend->output, &info);
        }
        memcpy(backend->previous_palette, frame->palette,
            sizeof(frame->palette));
    }
    if (backend->previous_columns != frame->columns ||
        backend->previous_rows != frame->rows ||
        memcmp(frame->text, backend->previous,
            sizeof(frame->text)) != 0 ||
        memcmp(frame->attributes, backend->previous_attributes,
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
        if (!WriteConsoleOutputA(backend->output, cells, size, position,
                &region)) {
            host_console_backend_unlock_output(backend);
            return LIB_STATUS_IO_ERROR;
        }
        memcpy(backend->previous, frame->text, sizeof(frame->text));
        memcpy(backend->previous_attributes, frame->attributes,
            sizeof(frame->attributes));
        backend->previous_columns = frame->columns;
        backend->previous_rows = frame->rows;
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
            (void)SetConsoleCursorPosition(backend->output, position);
        }
        (void)SetConsoleCursorInfo(backend->output, &cursor);
    }
    host_console_backend_unlock_output(backend);
    return LIB_STATUS_OK;
}
#endif
