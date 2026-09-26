#include "lib/types/types_interface.h"
#include "lib/types/win32/file.h"
#include "lib/types/win32/input.h"
#include "lib/types/win32/scalar.h"
#include "lib/types/win32/sync.h"
#include "lib/types/win32/window.h"
#include "lib/console/binding_interface.h"
#include "lib/console-broker/console_backend.h"
#include "lib/base/sync_interface.h"

#include "lib/types/win32/console.h"

struct console_broker_backend {
    lib_win32_handle input;
    lib_win32_handle output;
    /* Stream output retains its native cells, scrollback and cursor while
     * frame output uses a separate screen buffer. Only output is active. */
    lib_win32_handle cooked_output;
    lib_win32_handle raw_output;
    lib_win32_console_screen_buffer_infoex cooked_display;
    lib_win32_console_screen_buffer_infoex raw_display;
    lib_bool output_ready;
    lib_win32_handle stop_event;
    lib_win32_handle reader;
    /* Written by reader, observed only after its completion. */
    lib_status reader_status;
    /* A cooked reader owns exactly one native line.  It clears this before
     * delivering that line, so host can safely rearm after the app consumes
     * it without confusing an in-flight line with a thread that is merely
     * returning from its callback. */
    volatile lib_win32_long cooked_line_pending;
    base_sync_mutex *output_lock;
    base_sync_mutex *transaction_lock;
    lib_win32_dword original_mode;
    lib_console *console;
    console_broker_mode mode;
    lib_u32 generation;
    lib_u16 previous[LIB_CONSOLE_TEXT_COLUMNS * LIB_CONSOLE_TEXT_ROWS];
    lib_u8 previous_foreground[LIB_CONSOLE_TEXT_COLUMNS * LIB_CONSOLE_TEXT_ROWS];
    lib_u8 previous_background[LIB_CONSOLE_TEXT_COLUMNS * LIB_CONSOLE_TEXT_ROWS];
    lib_u32 previous_palette[16u];
    lib_u16 previous_columns;
    lib_u16 previous_rows;
    lib_u16 coverage_rows;
};

/* This is a liveness boundary, not an input debounce or product timer.
 * Normal cancellation completes immediately.  If the native Console does not
 * retire its sole reader in this interval, host I/O is no longer trustworthy
 * and the broker fails closed instead of hanging the control thread. */
#define CONSOLE_BROKER_READER_RETIRE_TIMEOUT_MS 2000u

static lib_win32_colorref console_broker_colorref_from_rgb(lib_u32 rgb)
{
    return lib_win32_rgb((rgb >> 16u) & 0xffu, (rgb >> 8u) & 0xffu, rgb & 0xffu);
}

static lib_bool console_broker_ensure_text_surface(console_broker_backend *backend,
    lib_u16 rows)
{
    lib_win32_handle output = backend->output;
    lib_win32_console_screen_buffer_info info;
    lib_win32_coord required;

    if (output == LIB_NULL || output == LIB_WIN32_INVALID_HANDLE_VALUE ||
        !lib_win32_get_console_screen_buffer_info(output, &info)) return LIB_FALSE;
    required.X = info.dwSize.X < (lib_win32_short)LIB_CONSOLE_TEXT_COLUMNS ?
        (lib_win32_short)LIB_CONSOLE_TEXT_COLUMNS : info.dwSize.X;
    required.Y = info.dwSize.Y < (lib_win32_short)rows ?
        (lib_win32_short)rows : info.dwSize.Y;
    /* A host may shrink backing storage between frames without making the
     * next frame itself need growth. The cache is invalid, but only rows that
     * still exist can remain part of the next frame's clearing coverage. */
    if (backend->coverage_rows > (lib_u16)info.dwSize.Y) {
        backend->previous_columns = backend->previous_rows = 0u;
        backend->coverage_rows = (lib_u16)info.dwSize.Y;
    }
    /* Frame storage is independent of the host's visible window. Preserve its
     * font and scroll position, including when the whole frame cannot fit. */
    if (required.X != info.dwSize.X || required.Y != info.dwSize.Y) {
        /* Surface geometry changed, so no completed-frame cache is valid. */
        backend->previous_columns = backend->previous_rows = 0u;
        if (!lib_win32_set_console_screen_buffer_size(output, required)) return LIB_FALSE;
    }
    return lib_win32_get_console_screen_buffer_info(output, &info) &&
        info.dwSize.X >= required.X && info.dwSize.Y >= required.Y;
}

static lib_u8 console_broker_modifiers(lib_win32_dword state)
{
    lib_u8 modifiers = 0u;
    if ((state & (LIB_WIN32_LEFT_CTRL_PRESSED | LIB_WIN32_RIGHT_CTRL_PRESSED)) != 0u)
        modifiers |= LIB_CONSOLE_MODIFIER_CONTROL;
    if ((state & (LIB_WIN32_LEFT_ALT_PRESSED | LIB_WIN32_RIGHT_ALT_PRESSED)) != 0u)
        modifiers |= LIB_CONSOLE_MODIFIER_ALT;
    if ((state & LIB_WIN32_SHIFT_PRESSED) != 0u)
        modifiers |= LIB_CONSOLE_MODIFIER_SHIFT;
    return modifiers;
}

static lib_status console_broker_emit_key(console_broker_backend *backend,
    const lib_win32_key_event_record *key)
{
    lib_console_event event = { 0 };
    event.kind = LIB_CONSOLE_EVENT_RAW_KEY;
    event.binding_generation = backend->generation;
    event.value.raw_key.key = key->wVirtualKeyCode;
    event.value.raw_key.unicode = key->uChar.UnicodeChar;
    event.value.raw_key.scan_code = key->wVirtualScanCode;
    event.value.raw_key.modifiers = console_broker_modifiers(key->dwControlKeyState);
    event.value.raw_key.extended =
        (key->dwControlKeyState & LIB_WIN32_ENHANCED_KEY) != 0u ? LIB_TRUE : LIB_FALSE;
    event.value.raw_key.pressed = key->bKeyDown ? LIB_TRUE : LIB_FALSE;
    event.value.raw_key.repeat_count = key->wRepeatCount;
    return lib_console_deliver_event(backend->console, &event);
}

static lib_status console_broker_emit_mouse(console_broker_backend *backend,
    const lib_win32_mouse_event_record *mouse)
{
    lib_console_event event = { 0 };
    event.kind = LIB_CONSOLE_EVENT_RAW_MOUSE;
    event.binding_generation = backend->generation;
    event.value.raw_mouse.delta_x = mouse->dwMousePosition.X;
    event.value.raw_mouse.delta_y = mouse->dwMousePosition.Y;
    event.value.raw_mouse.buttons = mouse->dwButtonState;
    return lib_console_deliver_event(backend->console, &event);
}

static void console_broker_reader_failed(console_broker_backend *backend)
{
    lib_console_event event = { 0 };
    if (lib_win32_wait_for_single_object(backend->stop_event, 0u) == LIB_WIN32_WAIT_OBJECT_0)
        return;
    backend->reader_status = LIB_STATUS_IO_ERROR;
    event.kind = LIB_CONSOLE_EVENT_IO_FAILURE;
    event.binding_generation = backend->generation;
    /* Last notification attempt. A broken event gate cannot notify itself;
     * reader_status also makes subsequent rearm/retirement return the fault. */
    (void)lib_console_deliver_event(backend->console, &event);
}

static lib_win32_dword LIB_WIN32_WINAPI console_broker_reader(void *context)
{
    console_broker_backend *backend = (console_broker_backend *)context;
    if (backend->mode == CONSOLE_BROKER_COOKED_LINES) {
        char text[LIB_CONSOLE_LINE_MAX];
        lib_console_event event = { 0 };
        lib_bool overflow = LIB_FALSE, complete = LIB_FALSE;
        /* ReadConsole may return a buffer fragment rather than a whole line.
         * Keep bounded storage, but drain an oversized line through its LF. */
        while (!complete) {
            lib_win32_dword read = 0u, i;
            if (!lib_win32_read_console_a(backend->input, text,
                    LIB_CONSOLE_LINE_MAX - 1u, &read, LIB_NULL) || read == 0u) {
                if (lib_win32_wait_for_single_object(backend->stop_event, 0u) !=
                        LIB_WIN32_WAIT_OBJECT_0)
                    lib_win32_interlocked_exchange(&backend->cooked_line_pending, 0);
                console_broker_reader_failed(backend);
                return 0u;
            }
            if (lib_win32_wait_for_single_object(backend->stop_event, 0u) == LIB_WIN32_WAIT_OBJECT_0) {
                return 0u;
            }
            for (i = 0u; i < read; ++i) {
                if (text[i] == '\n') { complete = LIB_TRUE; break; }
                if (text[i] == '\r') continue;
                if (event.value.line.length == LIB_CONSOLE_LINE_MAX - 1u)
                    overflow = LIB_TRUE;
                else event.value.line.text[event.value.line.length++] = text[i];
            }
        }
        lib_win32_interlocked_exchange(&backend->cooked_line_pending, 0);
        event.kind = overflow ? LIB_CONSOLE_EVENT_REJECTED_LINE : LIB_CONSOLE_EVENT_COOKED_LINE;
        event.binding_generation = backend->generation;
        if (overflow) event.value.line.length = 0u;
        event.value.line.text[event.value.line.length] = '\0';
        if (lib_console_deliver_event(backend->console, &event) != LIB_STATUS_OK)
            console_broker_reader_failed(backend);
    } else {
        lib_win32_handle waits[2] = { backend->stop_event, backend->input };
        while (lib_win32_wait_for_multiple_objects(2u, waits, LIB_WIN32_FALSE, LIB_WIN32_INFINITE) == LIB_WIN32_WAIT_OBJECT_0 + 1u) {
            lib_win32_input_record record;
            lib_win32_dword read = 0u;
            if (!lib_win32_read_console_input_w(backend->input, &record, 1u, &read)) break;
            if (read == 0u) continue;
            if (record.EventType == LIB_WIN32_KEY_EVENT && console_broker_emit_key(backend,
                    &record.Event.KeyEvent) != LIB_STATUS_OK) break;
            if (record.EventType == LIB_WIN32_MOUSE_EVENT && console_broker_emit_mouse(backend,
                    &record.Event.MouseEvent) != LIB_STATUS_OK) break;
        }
        console_broker_reader_failed(backend);
    }
    return 0u;
}

lib_status console_broker_backend_create(console_broker_backend **out_backend)
{
    console_broker_backend *backend;
    lib_status status;
    lib_win32_dword mode;
    if (out_backend == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_backend = LIB_NULL;
    backend = lib_allocate_zero(1u, sizeof(*backend));
    if (backend == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    status = base_sync_mutex_create(&backend->output_lock);
    if (status == LIB_STATUS_OK) status = base_sync_mutex_create(&backend->transaction_lock);
    if (status != LIB_STATUS_OK) goto release;
    backend->input = lib_win32_create_file_a("CONIN$", LIB_WIN32_GENERIC_READ | LIB_WIN32_GENERIC_WRITE,
        LIB_WIN32_FILE_SHARE_READ | LIB_WIN32_FILE_SHARE_WRITE, LIB_NULL, LIB_WIN32_OPEN_EXISTING, 0, LIB_NULL);
    backend->output = lib_win32_create_file_a("CONOUT$", LIB_WIN32_GENERIC_READ | LIB_WIN32_GENERIC_WRITE,
        LIB_WIN32_FILE_SHARE_READ | LIB_WIN32_FILE_SHARE_WRITE, LIB_NULL, LIB_WIN32_OPEN_EXISTING, 0, LIB_NULL);
    if (backend->input == LIB_WIN32_INVALID_HANDLE_VALUE ||
        backend->output == LIB_WIN32_INVALID_HANDLE_VALUE ||
        !lib_win32_get_console_mode(backend->input, &mode)) {
        if (backend->input != LIB_WIN32_INVALID_HANDLE_VALUE) lib_win32_close_handle(backend->input);
        if (backend->output != LIB_WIN32_INVALID_HANDLE_VALUE) lib_win32_close_handle(backend->output);
        status = LIB_STATUS_UNSUPPORTED;
        goto release;
    }
    backend->original_mode = mode;
    backend->cooked_output = backend->output;
    backend->output_ready = LIB_TRUE;
    *out_backend = backend;
    return LIB_STATUS_OK;
release:
    base_sync_mutex_destroy(backend->transaction_lock);
    base_sync_mutex_destroy(backend->output_lock);
    lib_release(backend);
    return status;
}

static lib_status console_broker_select_output(console_broker_backend *backend,
    lib_win32_handle output)
{
    lib_win32_console_screen_buffer_infoex *previous, *next;
    if (backend->output == output) return LIB_STATUS_OK;
    previous = backend->output == backend->cooked_output ?
        &backend->cooked_display : &backend->raw_display;
    next = output == backend->cooked_output ? &backend->cooked_display : &backend->raw_display;
    /* A failed metadata restore must not replace that buffer's saved state
     * with its half-restored state when the broker rolls back. */
    if (backend->output_ready) {
        previous->cbSize = sizeof(*previous);
        if (!lib_win32_get_console_screen_buffer_info_ex(backend->output, previous))
            return LIB_STATUS_IO_ERROR;
    }
    if (!lib_win32_set_console_active_screen_buffer(output)) return LIB_STATUS_IO_ERROR;
    backend->output = output;
    backend->output_ready = LIB_FALSE;
    if (next->cbSize != 0u) {
        lib_win32_console_screen_buffer_infoex display = *next;
        /* The native setter uses exclusive right/bottom window bounds. It
         * also restores palette/geometry affected by the other screen buffer. */
        ++display.srWindow.Right;
        ++display.srWindow.Bottom;
        if (!lib_win32_set_console_screen_buffer_info_ex(output, &display))
            return LIB_STATUS_IO_ERROR;
    }
    backend->output_ready = LIB_TRUE;
    return LIB_STATUS_OK;
}

lib_status console_broker_backend_destroy(console_broker_backend *backend)
{
    if (backend == LIB_NULL) return LIB_STATUS_OK;
    /* Reader is already joined. Restore the original display before closing
     * the alternate buffer, including failure during initial raw activation. */
    if (backend->raw_output != LIB_NULL) {
        if (backend->output != backend->cooked_output) {
            lib_status status = console_broker_select_output(backend, backend->cooked_output);
            if (status != LIB_STATUS_OK) return status;
        }
        if (!lib_win32_close_handle(backend->raw_output)) return LIB_STATUS_IO_ERROR;
        backend->raw_output = LIB_NULL;
    }
    if (backend->input != LIB_WIN32_INVALID_HANDLE_VALUE) {
        if (!lib_win32_close_handle(backend->input)) return LIB_STATUS_IO_ERROR;
        backend->input = LIB_WIN32_INVALID_HANDLE_VALUE;
    }
    if (backend->output != LIB_WIN32_INVALID_HANDLE_VALUE) {
        if (!lib_win32_close_handle(backend->output)) return LIB_STATUS_IO_ERROR;
        backend->output = LIB_WIN32_INVALID_HANDLE_VALUE;
    }
    base_sync_mutex_destroy(backend->output_lock);
    base_sync_mutex_destroy(backend->transaction_lock);
    lib_release(backend);
    return LIB_STATUS_OK;
}

static lib_status console_broker_start_reader(console_broker_backend *backend)
{
    backend->reader_status = LIB_STATUS_OK;
    if (backend->mode == CONSOLE_BROKER_COOKED_LINES)
        lib_win32_interlocked_exchange(&backend->cooked_line_pending, 1);
    backend->reader = lib_win32_create_thread(LIB_NULL, 0u, console_broker_reader,
        backend, 0u, LIB_NULL);
    if (backend->reader == LIB_NULL) {
        lib_win32_interlocked_exchange(&backend->cooked_line_pending, 0);
        return LIB_STATUS_NO_MEMORY;
    }
    return LIB_STATUS_OK;
}

/* The native raw Console is the input surface. This behavior
 * is coupled to successful raw activation rather than exposed as a product
 * API: the broker owns the one process Console handle and knows whether a
 * live raw reader exists.  Cooked activation intentionally does not make a
 * native foreground request. */
static void console_broker_activate_raw_input_surface(
    console_broker_backend *backend)
{
    lib_win32_hwnd window;

    if (backend == LIB_NULL ||
        backend->mode != CONSOLE_BROKER_RAW_EVENTS ||
        backend->console == LIB_NULL || backend->reader == LIB_NULL)
        return;
    window = lib_win32_get_console_window();
    if (window == LIB_NULL) return;
    if (lib_win32_is_iconic(window)) (void)lib_win32_show_window(window, LIB_WIN32_SW_RESTORE);
    (void)lib_win32_set_foreground_window(window);
    (void)lib_win32_set_active_window(window);
    (void)lib_win32_set_focus(window);
}

static lib_status console_broker_prepare_output(console_broker_backend *backend,
    console_broker_mode mode)
{
    if (mode == CONSOLE_BROKER_RAW_EVENTS && backend->raw_output == LIB_NULL) {
        lib_win32_handle output = lib_win32_create_console_screen_buffer(
            LIB_WIN32_GENERIC_READ | LIB_WIN32_GENERIC_WRITE,
            LIB_WIN32_FILE_SHARE_READ | LIB_WIN32_FILE_SHARE_WRITE, LIB_NULL,
            LIB_WIN32_CONSOLE_TEXTMODE_BUFFER, LIB_NULL);
        if (output == LIB_WIN32_INVALID_HANDLE_VALUE) return LIB_STATUS_IO_ERROR;
        backend->raw_output = output;
    }
    return LIB_STATUS_OK;
}

lib_status console_broker_backend_prepare(console_broker_backend *backend,
    lib_console *console, console_broker_mode mode)
{
    lib_win32_dword ignored;
    if (backend == LIB_NULL || console == LIB_NULL ||
        (mode != CONSOLE_BROKER_RAW_EVENTS && mode != CONSOLE_BROKER_COOKED_LINES))
        return LIB_STATUS_INVALID_ARGUMENT;
    if (backend->input == LIB_WIN32_INVALID_HANDLE_VALUE ||
        backend->output == LIB_WIN32_INVALID_HANDLE_VALUE ||
        !lib_win32_get_console_mode(backend->input, &ignored)) return LIB_STATUS_IO_ERROR;
    return console_broker_prepare_output(backend, mode);
}


lib_status console_broker_backend_activate(console_broker_backend *backend,
    lib_console *console, console_broker_mode mode, lib_u32 generation,
    lib_bool restore_cooked_request)
{
    lib_win32_dword configured;
    lib_win32_handle output;
    lib_status status;
    if (backend == LIB_NULL || console == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = console_broker_prepare_output(backend, mode);
    if (status != LIB_STATUS_OK) return status;
    output = mode == CONSOLE_BROKER_RAW_EVENTS ? backend->raw_output : backend->cooked_output;
    configured = backend->original_mode;
    if (mode == CONSOLE_BROKER_RAW_EVENTS)
        /* ReadConsoleInput consumes classic INPUT_RECORD values.  A parent
         * Windows Terminal may have enabled VT input; retaining that flag
         * converts keyboard input into byte sequences for ReadConsole/ReadFile
         * instead, leaving this raw reader with no KEY_EVENT records. */
        configured = (configured & ~(LIB_WIN32_ENABLE_ECHO_INPUT | LIB_WIN32_ENABLE_LINE_INPUT |
            LIB_WIN32_ENABLE_PROCESSED_INPUT | LIB_WIN32_ENABLE_QUICK_EDIT_MODE |
            LIB_WIN32_ENABLE_VIRTUAL_TERMINAL_INPUT)) |
            LIB_WIN32_ENABLE_MOUSE_INPUT | LIB_WIN32_ENABLE_EXTENDED_FLAGS;
    else configured |= LIB_WIN32_ENABLE_ECHO_INPUT | LIB_WIN32_ENABLE_LINE_INPUT | LIB_WIN32_ENABLE_PROCESSED_INPUT;
    if (!lib_win32_set_console_mode(backend->input, configured)) return LIB_STATUS_IO_ERROR;
    /* A Current-Console cutover has one ownership boundary regardless of
     * input mode.  Records already buffered before it belong to neither the
     * old nor the newly activated logical Console, so discard them before
     * starting the new reader. */
    if (!lib_win32_flush_console_input_buffer(backend->input))
        return LIB_STATUS_IO_ERROR;
    /* The broker's output gate covers selection and reader startup. A failed
     * activation is rolled back through this same path, not a second restore
     * implementation. Same-mode replacements leave the display untouched. */
    status = console_broker_select_output(backend, output);
    if (status != LIB_STATUS_OK) return status;
    backend->stop_event = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    if (backend->stop_event == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    backend->console = console;
    backend->mode = mode;
    backend->generation = generation;
    lib_memory_set(backend->previous, 0xff, sizeof(backend->previous));
    lib_memory_set(backend->previous_foreground, 0xff, sizeof(backend->previous_foreground));
    lib_memory_set(backend->previous_background, 0xff, sizeof(backend->previous_background));
    lib_memory_set(backend->previous_palette, 0xff,
        sizeof(backend->previous_palette));
    backend->previous_columns = 0u;
    backend->previous_rows = 0u;
    backend->coverage_rows = 0u;
    /* Normal cooked activation is not a line request. Rollback alone may
     * restore the unfinished request captured after the old reader joined. */
    if (mode == CONSOLE_BROKER_RAW_EVENTS || restore_cooked_request) {
        if (console_broker_start_reader(backend) != LIB_STATUS_OK) {
            lib_win32_close_handle(backend->stop_event);
            backend->stop_event = LIB_NULL;
            backend->console = LIB_NULL;
            return LIB_STATUS_NO_MEMORY;
        }
        console_broker_activate_raw_input_surface(backend);
    }
    return LIB_STATUS_OK;
}

lib_status console_broker_backend_request_cooked_line(
    console_broker_backend *backend)
{
    lib_win32_dword completed;

    if (backend == LIB_NULL ||
        backend->mode != CONSOLE_BROKER_COOKED_LINES ||
        backend->console == LIB_NULL || backend->stop_event == LIB_NULL)
        return LIB_STATUS_INVALID_STATE;
    if (backend->reader != LIB_NULL) {
        if (lib_win32_interlocked_compare_exchange(&backend->cooked_line_pending,
                0, 0) != 0)
            return LIB_STATUS_OK;
        completed = lib_win32_wait_for_single_object(backend->reader, 0u);
        if (completed == LIB_WIN32_WAIT_TIMEOUT) {
            completed = lib_win32_wait_for_single_object(backend->reader, LIB_WIN32_INFINITE);
        }
        if (completed != LIB_WIN32_WAIT_OBJECT_0) return LIB_STATUS_IO_ERROR;
        if (backend->reader_status != LIB_STATUS_OK) return backend->reader_status;
        if (!lib_win32_close_handle(backend->reader)) return LIB_STATUS_IO_ERROR;
        backend->reader = LIB_NULL;
    }
    return console_broker_start_reader(backend);
}

static lib_status console_broker_retire_reader(console_broker_backend *backend)
{
    lib_win32_dword completed;

    if (backend == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (backend->reader == LIB_NULL) return LIB_STATUS_OK;
    if (backend->stop_event == LIB_NULL ||
        !lib_win32_set_event(backend->stop_event)) return LIB_STATUS_IO_ERROR;
    /* A reader owns the process Console until it has exited.  Cancellation is
       deliberately best effort because Console implementations differ; the
       completion observation below is the actual ownership proof. */
    (void)lib_win32_cancel_io_ex(backend->input, LIB_NULL);
    (void)lib_win32_cancel_synchronous_io(backend->reader);
    completed = lib_win32_wait_for_single_object(backend->reader, 0u);
    if (completed == LIB_WIN32_WAIT_TIMEOUT && backend->mode == CONSOLE_BROKER_COOKED_LINES) {
        lib_win32_input_record wake = { 0 };
        lib_win32_dword written = 0u;
        /* Some terminal hosts do not interrupt a line-buffered ReadConsoleA
           even after both documented cancellation requests.  Supply the
           terminating record ourselves while the old reader is still the
           sole owner.  It observes stop_event and discards this synthetic
           line; the new binding cannot observe it. */
        wake.EventType = LIB_WIN32_KEY_EVENT;
        wake.Event.KeyEvent.bKeyDown = LIB_WIN32_TRUE;
        wake.Event.KeyEvent.wRepeatCount = 1u;
        wake.Event.KeyEvent.wVirtualKeyCode = LIB_WIN32_KEY_RETURN;
        wake.Event.KeyEvent.wVirtualScanCode = (lib_win32_word)lib_win32_map_virtual_key(LIB_WIN32_KEY_RETURN,
            LIB_WIN32_MAPVK_VK_TO_VSC);
        wake.Event.KeyEvent.uChar.AsciiChar = '\r';
        if (!lib_win32_write_console_input_a(backend->input, &wake, 1u, &written) ||
            written != 1u) return LIB_STATUS_IO_ERROR;
    }
    completed = lib_win32_wait_for_single_object(backend->reader,
        CONSOLE_BROKER_READER_RETIRE_TIMEOUT_MS);
    if (completed != LIB_WIN32_WAIT_OBJECT_0) return LIB_STATUS_IO_ERROR;
    if (backend->reader_status != LIB_STATUS_OK) return backend->reader_status;
    if (!lib_win32_close_handle(backend->reader)) return LIB_STATUS_IO_ERROR;
    backend->reader = LIB_NULL;
    return LIB_STATUS_OK;
}

lib_status console_broker_backend_cancel_cooked_line(console_broker_backend *backend,
    lib_bool *out_completed)
{
    lib_status status;
    if (backend == LIB_NULL || out_completed == LIB_NULL ||
        backend->mode != CONSOLE_BROKER_COOKED_LINES || backend->stop_event == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (backend->reader == LIB_NULL) {
        *out_completed = LIB_TRUE;
        return LIB_STATUS_OK;
    }
    status = console_broker_retire_reader(backend);
    if (status != LIB_STATUS_OK) return status;
    /* Only inspect completion after join: cancellation may race a successful
     * event delivery. A committed line is not an abandoned editing fragment. */
    *out_completed = backend->cooked_line_pending == 0;
    lib_win32_interlocked_exchange(&backend->cooked_line_pending, 0);
    /* A raced completion is already delivered. Flush only native leftovers,
     * including a synthetic wake that lost that race, never the queued line. */
    if (!lib_win32_flush_console_input_buffer(backend->input))
        return LIB_STATUS_IO_ERROR;
    return lib_win32_reset_event(backend->stop_event) ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR;
}

lib_status console_broker_backend_deactivate(console_broker_backend *backend,
    lib_bool *out_cooked_request)
{
    lib_status status;

    if (backend == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = console_broker_retire_reader(backend);
    if (status != LIB_STATUS_OK) return status;
    /* A completed line clears pending before delivery; cancelled reads retain
     * it until this join. Never snapshot it while the reader can still finish. */
    if (out_cooked_request != LIB_NULL)
        *out_cooked_request = backend->cooked_line_pending != 0;
    lib_win32_interlocked_exchange(&backend->cooked_line_pending, 0);
    if (backend->stop_event != LIB_NULL && !lib_win32_close_handle(backend->stop_event))
        return LIB_STATUS_IO_ERROR;
    backend->stop_event = LIB_NULL;
    backend->console = LIB_NULL;
    backend->generation = 0u;
    return lib_win32_set_console_mode(backend->input, backend->original_mode) ?
        LIB_STATUS_OK : LIB_STATUS_IO_ERROR;
}

void console_broker_backend_lock_output(console_broker_backend *backend)
{
    if (backend != LIB_NULL) base_sync_mutex_lock(backend->output_lock);
}

void console_broker_backend_unlock_output(console_broker_backend *backend)
{
    if (backend != LIB_NULL) base_sync_mutex_unlock(backend->output_lock);
}

lib_status console_broker_backend_write_bound(console_broker_backend *backend,
    lib_console *expected_console, lib_u32 expected_generation, const char *text,
    lib_size length)
{
    lib_win32_dword written = 0u;
    if (backend == LIB_NULL || (text == LIB_NULL && length != 0u) ||
        length > (lib_size)LIB_UINT32_MAX) return LIB_STATUS_INVALID_ARGUMENT;
    console_broker_backend_lock_output(backend);
    if (backend->console != expected_console ||
        backend->generation != expected_generation) {
        console_broker_backend_unlock_output(backend);
        return LIB_STATUS_OK;
    }
    {
        /* A stream write invalidates copied cells, but does not discard the
         * former frame extent that a following frame must clear. */
        if (length != 0u) backend->previous_columns = backend->previous_rows = 0u;
        lib_status status = lib_win32_write_console_a(backend->output, text, (lib_win32_dword)length,
            &written, LIB_NULL) && written == (lib_win32_dword)length ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR;
        console_broker_backend_unlock_output(backend);
        return status;
    }
}

lib_status console_broker_backend_write_text_frame_bound(console_broker_backend *backend,
    lib_console *expected_console, lib_u32 expected_generation,
    const lib_console_text_frame *frame)
{
    lib_win32_char_info cells[LIB_CONSOLE_TEXT_COLUMNS * LIB_CONSOLE_TEXT_ROWS];
    lib_win32_coord size = { LIB_CONSOLE_TEXT_COLUMNS, LIB_CONSOLE_TEXT_ROWS };
    lib_win32_coord position = { 0, 0 };
    lib_win32_small_rect region = { 0, 0, LIB_CONSOLE_TEXT_COLUMNS - 1,
        LIB_CONSOLE_TEXT_ROWS - 1 };
    lib_u32 row;

    if (backend == LIB_NULL || frame == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    console_broker_backend_lock_output(backend);
    if (backend->console != expected_console ||
        backend->generation != expected_generation) {
        console_broker_backend_unlock_output(backend);
        return LIB_STATUS_OK;
    }
    if (lib_memory_compare(frame->palette, backend->previous_palette,
            sizeof(frame->palette)) != 0) {
        lib_win32_console_screen_buffer_infoex info;
        lib_u32 index;

        lib_memory_set(&info, 0, sizeof(info));
        info.cbSize = sizeof(info);
        if (lib_win32_get_console_screen_buffer_info_ex(backend->output, &info)) {
            for (index = 0u; index < 16u; ++index)
                info.ColorTable[index] = console_broker_colorref_from_rgb(
                    frame->palette[index]);
            ++info.srWindow.Right;
            ++info.srWindow.Bottom;
            if (lib_win32_set_console_screen_buffer_info_ex(backend->output, &info))
                lib_memory_copy(backend->previous_palette, frame->palette,
                    sizeof(frame->palette));
        }
        /* Some terminal hosts cannot apply a palette. Do not mark it applied:
         * another frame may retry while text output remains usable. */
    }
    /* Palette application can also change native buffer/viewport geometry.
     * Establish the write surface after that operation, never before it. */
    if (!console_broker_ensure_text_surface(backend, frame->rows)) {
        console_broker_backend_unlock_output(backend);
        return LIB_STATUS_IO_ERROR;
    }
    /* Clear only this frame plus a possible tail written by the prior frame.
     * Host viewport height is not frame content: a 30-row Terminal must not
     * turn every 25-row DOS frame into a 30-row terminal update. */
    if (backend->coverage_rows > frame->rows)
        size.Y = (lib_win32_short)backend->coverage_rows;
    else
        size.Y = (lib_win32_short)frame->rows;
    region.Bottom = size.Y - 1;
    if (backend->previous_columns != frame->columns ||
        backend->previous_rows != frame->rows ||
        lib_memory_compare(frame->text, backend->previous,
            sizeof(frame->text)) != 0 ||
        lib_memory_compare(frame->foreground, backend->previous_foreground, sizeof(frame->foreground)) != 0 ||
        lib_memory_compare(frame->background, backend->previous_background, sizeof(frame->background)) != 0) {
        for (row = 0u; row < (lib_u32)size.Y; ++row) {
            lib_u32 column;
            for (column = 0u; column < LIB_CONSOLE_TEXT_COLUMNS; ++column) {
                lib_size offset = (lib_size)row * LIB_CONSOLE_TEXT_COLUMNS + column;
                cells[offset].Char.UnicodeChar = row < frame->rows &&
                    column < frame->columns ? frame->text[offset] : ' ';
                cells[offset].Attributes = (lib_win32_word)(row < frame->rows &&
                    column < frame->columns ? frame->foreground[offset] |
                        (frame->background[offset] << 4u) : 0);
            }
        }
        /* A failed or clipped write may already have changed some cells. The
         * completed-frame cache is invalid, but its former coverage remains
         * part of the next retry's rectangle. */
        backend->previous_columns = backend->previous_rows = 0u;
        backend->coverage_rows = (lib_u16)size.Y;
        if (!lib_win32_write_console_output_w(backend->output, cells, size, position,
                &region) || region.Left != 0 || region.Top != 0 ||
                region.Right != LIB_CONSOLE_TEXT_COLUMNS - 1 ||
                region.Bottom != size.Y - 1) {
            console_broker_backend_unlock_output(backend);
            return LIB_STATUS_IO_ERROR;
        }
        lib_memory_copy(backend->previous, frame->text, sizeof(frame->text));
        lib_memory_copy(backend->previous_foreground, frame->foreground, sizeof(frame->foreground));
        lib_memory_copy(backend->previous_background, frame->background, sizeof(frame->background));
        backend->previous_columns = frame->columns;
        backend->previous_rows = frame->rows;
        backend->coverage_rows = frame->rows;
    }
    {
        lib_win32_console_cursor_info cursor;
        cursor.dwSize = frame->cursor_bottom >= frame->cursor_top &&
            frame->font_height != 0u ? (lib_win32_dword)((frame->cursor_bottom -
                frame->cursor_top + 1u) * 100u / frame->font_height) : 100u;
        if (cursor.dwSize == 0u || cursor.dwSize > 100u) cursor.dwSize = 100u;
        cursor.bVisible = frame->cursor_visible != 0u && frame->cursor_phase != 0u &&
            frame->cursor_column >= 0 && frame->cursor_row >= 0 &&
            frame->cursor_column < (lib_i32)frame->columns &&
            frame->cursor_row < (lib_i32)frame->rows;
        if (cursor.bVisible) {
            position.X = (lib_win32_short)frame->cursor_column;
            position.Y = (lib_win32_short)frame->cursor_row;
            if (!lib_win32_set_console_cursor_position(backend->output, position)) {
                console_broker_backend_unlock_output(backend);
                return LIB_STATUS_IO_ERROR;
            }
        }
        if (!lib_win32_set_console_cursor_info(backend->output, &cursor)) {
            console_broker_backend_unlock_output(backend);
            return LIB_STATUS_IO_ERROR;
        }
    }
    console_broker_backend_unlock_output(backend);
    return LIB_STATUS_OK;
}

void console_broker_backend_lock_transaction(console_broker_backend *backend)
{ base_sync_mutex_lock(backend->transaction_lock); }
void console_broker_backend_unlock_transaction(console_broker_backend *backend)
{ base_sync_mutex_unlock(backend->transaction_lock); }
