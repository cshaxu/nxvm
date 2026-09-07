#include "lib/base/base.h"
#include "console.h"

#ifdef _WIN32
#include "actions.h"
#include "geometry.h"
#include "input.h"
#include "mailbox.h"

#include <windows.h>
#include <stdlib.h>
#include <string.h>

static atomic_flag ux_win32_console_lease = ATOMIC_FLAG_INIT;

typedef struct ux_win32_console ux_win32_console;

struct ux_win32_console {
    const ux_binding *binding;
    HANDLE input;
    HANDLE output;
    DWORD original_mode;
    int private_console;
    ux_frame *frame;
    unsigned char previous[UX_TEXT_COLUMNS * UX_TEXT_ROWS];
    unsigned short previous_attributes[UX_TEXT_COLUMNS * UX_TEXT_ROWS];
    uint32_t previous_palette[16u];
    unsigned int previous_columns;
    unsigned int previous_rows;
    uint32_t displayed_sequence;
    ux_win32_keyboard_normalizer keyboard_normalizer;
    COORD mouse_previous;
    int mouse_previous_valid;
};

static lib_bool ux_win32_console_acquire(void)
{
    return !atomic_flag_test_and_set_explicit(&ux_win32_console_lease,
        memory_order_acquire);
}

static void ux_win32_console_release(void)
{
    atomic_flag_clear_explicit(&ux_win32_console_lease,
        memory_order_release);
}

static int win32_console_open(const ux_binding *binding,
    ux_win32_console *console)
{
    HANDLE input;
    HANDLE output;
    DWORD original_mode;

    input = CreateFileA("CONIN$", GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    output = CreateFileA("CONOUT$", GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (input == INVALID_HANDLE_VALUE || output == INVALID_HANDLE_VALUE ||
        !GetConsoleMode(input, &original_mode)) {
        if (input != INVALID_HANDLE_VALUE) CloseHandle(input);
        if (output != INVALID_HANDLE_VALUE) CloseHandle(output);
        if (GetConsoleCP() != 0u || !AllocConsole()) return 0;
        console->private_console = 1;
        input = CreateFileA("CONIN$", GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        output = CreateFileA("CONOUT$", GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (input == INVALID_HANDLE_VALUE || output == INVALID_HANDLE_VALUE ||
            !GetConsoleMode(input, &original_mode)) {
            if (input != INVALID_HANDLE_VALUE) CloseHandle(input);
            if (output != INVALID_HANDLE_VALUE) CloseHandle(output);
            FreeConsole();
            return 0;
        }
    }
    console->input = input;
    console->output = output;
    console->original_mode = original_mode;
    return 1;
}

static void win32_console_close(ux_win32_console *console)
{
    if (console == NULL) return;
    if (console->input != NULL && console->input != INVALID_HANDLE_VALUE)
        CloseHandle(console->input);
    if (console->output != NULL && console->output != INVALID_HANDLE_VALUE)
        CloseHandle(console->output);
    if (console->private_console) FreeConsole();
    console->input = NULL;
    console->output = NULL;
}

static int win32_console_ensure_text_surface(HANDLE output)
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    COORD largest;
    COORD required;
    SMALL_RECT viewport = { 0, 0, UX_TEXT_COLUMNS - 1, UX_TEXT_ROWS - 1 };

    if (output == NULL || output == INVALID_HANDLE_VALUE ||
        !GetConsoleScreenBufferInfo(output, &info)) return 0;
    required.X = info.dwSize.X < (SHORT)UX_TEXT_COLUMNS ?
        (SHORT)UX_TEXT_COLUMNS : info.dwSize.X;
    required.Y = info.dwSize.Y < (SHORT)UX_TEXT_ROWS ?
        (SHORT)UX_TEXT_ROWS : info.dwSize.Y;
    if ((required.X != info.dwSize.X || required.Y != info.dwSize.Y) &&
        !SetConsoleScreenBufferSize(output, required)) return 0;
    largest = GetLargestConsoleWindowSize(output);
    if (largest.X < (SHORT)UX_TEXT_COLUMNS || largest.Y < (SHORT)UX_TEXT_ROWS)
        return 0;
    return SetConsoleWindowInfo(output, TRUE, &viewport) != 0;
}

static ux_run_result win32_console_key(ux_win32_console *console,
    const KEY_EVENT_RECORD *key)
{
    ux_action action;
    lib_u8 modifiers;

    if (!key->bKeyDown && console->keyboard_normalizer.suppressed_virtual_key ==
        key->wVirtualKeyCode) {
        console->keyboard_normalizer.suppressed_virtual_key = 0u;
        return UX_RUN_CONTINUE;
    }
    modifiers = ux_win32_modifiers_from_console_state(key->dwControlKeyState);
    action = key->bKeyDown ? ux_actions_match(console->binding->actions,
        key->wVirtualKeyCode, modifiers) : UX_ACTION_NONE;
    if (action != UX_ACTION_NONE) {
        ux_run_result result = ux_binding_invoke_action(console->binding, action);
        console->keyboard_normalizer.suppressed_virtual_key = key->wVirtualKeyCode;
        return result;
    }
    if (key->wVirtualScanCode == 0u && key->bKeyDown &&
        key->uChar.UnicodeChar != 0u) {
        (void)ux_win32_keyboard_submit_utf16(&console->keyboard_normalizer,
            console->binding->context, console->binding->input_sink,
            key->uChar.UnicodeChar);
    } else {
        (void)ux_win32_keyboard_submit_transition(console->binding->context,
            console->binding->input_sink, key->wVirtualScanCode,
            key->wVirtualKeyCode, key->dwControlKeyState, key->bKeyDown != 0);
    }
    return UX_RUN_CONTINUE;
}

static void win32_console_mouse(ux_win32_console *console,
    const MOUSE_EVENT_RECORD *mouse)
{
    ux_event event;

    if (console == NULL || mouse == NULL) return;
    memset(&event, 0, sizeof(event));
    event.type = UX_EVENT_MOUSE;
    event.data.mouse.relative = 1u;
    if (console->mouse_previous_valid) {
        event.data.mouse.delta_x = ((int32_t)mouse->dwMousePosition.X -
            console->mouse_previous.X) * 8;
        event.data.mouse.delta_y = ((int32_t)mouse->dwMousePosition.Y -
            console->mouse_previous.Y) * 16;
    }
    console->mouse_previous = mouse->dwMousePosition;
    console->mouse_previous_valid = 1;
    event.data.mouse.buttons =
        (mouse->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) != 0u ?
            UX_MOUSE_BUTTON_LEFT : 0u;
    if ((mouse->dwButtonState & RIGHTMOST_BUTTON_PRESSED) != 0u)
        event.data.mouse.buttons |= UX_MOUSE_BUTTON_RIGHT;
    (void)console->binding->input_sink(console->binding->context, &event);
}

static int win32_console_paint(ux_win32_console *console)
{
    CHAR_INFO cells[UX_TEXT_COLUMNS * UX_TEXT_ROWS];
    COORD size = { UX_TEXT_COLUMNS, UX_TEXT_ROWS };
    COORD position = { 0, 0 };
    SMALL_RECT region = { 0, 0, UX_TEXT_COLUMNS - 1, UX_TEXT_ROWS - 1 };
    unsigned int row;
    unsigned int columns;
    unsigned int rows;
    ux_frame *frame = console->frame;

    if (frame->valid == 0u || frame->graphics != 0u) return 1;
    if (frame->text_columns == 0u || frame->text_rows == 0u ||
        frame->text_columns > UX_TEXT_COLUMNS || frame->text_rows > UX_TEXT_ROWS)
        return 0;
    if (!win32_console_ensure_text_surface(console->output)) return 0;
    columns = frame->text_columns;
    rows = frame->text_rows;
    if (memcmp(frame->text_palette, console->previous_palette,
            sizeof(frame->text_palette)) != 0) {
        CONSOLE_SCREEN_BUFFER_INFOEX info;
        unsigned int index;

        memset(&info, 0, sizeof(info));
        info.cbSize = sizeof(info);
        if (GetConsoleScreenBufferInfoEx(console->output, &info)) {
            for (index = 0u; index < 16u; ++index)
                info.ColorTable[index] = ux_win32_colorref_from_rgb(
                    frame->text_palette[index]);
            (void)SetConsoleScreenBufferInfoEx(console->output, &info);
        }
        memcpy(console->previous_palette, frame->text_palette,
            sizeof(frame->text_palette));
    }
    if (console->previous_columns == columns && console->previous_rows == rows &&
        memcmp(frame->text, console->previous, sizeof(frame->text)) == 0 &&
        memcmp(frame->attributes, console->previous_attributes,
            sizeof(frame->attributes)) == 0) return 1;
    for (row = 0u; row < UX_TEXT_ROWS; ++row) {
        unsigned int column;
        for (column = 0u; column < UX_TEXT_COLUMNS; ++column) {
            size_t offset = row * UX_TEXT_COLUMNS + column;
            cells[offset].Char.AsciiChar = row < rows && column < columns &&
                frame->text[offset] >= 0x20u && frame->text[offset] < 0x7fu ?
                (CHAR)frame->text[offset] : ' ';
            cells[offset].Attributes = (WORD)(row < rows && column < columns ?
                frame->attributes[offset] : 0u);
        }
    }
    if (!WriteConsoleOutputA(console->output, cells, size, position, &region))
        return 0;
    memcpy(console->previous, frame->text, sizeof(frame->text));
    memcpy(console->previous_attributes, frame->attributes,
        sizeof(frame->attributes));
    console->previous_columns = columns;
    console->previous_rows = rows;
    {
        CONSOLE_CURSOR_INFO cursor;
        cursor.dwSize = frame->cursor_bottom >= frame->cursor_top &&
            frame->font_height != 0u ? (DWORD)((frame->cursor_bottom -
                frame->cursor_top + 1u) * 100u / frame->font_height) : 100u;
        if (cursor.dwSize == 0u || cursor.dwSize > 100u) cursor.dwSize = 100u;
        cursor.bVisible = frame->cursor_visible != 0u && frame->cursor_phase != 0u &&
            frame->cursor_column >= 0 && frame->cursor_row >= 0 &&
            frame->cursor_column < (int32_t)columns && frame->cursor_row < (int32_t)rows;
        if (cursor.bVisible) {
            position.X = (SHORT)frame->cursor_column;
            position.Y = (SHORT)frame->cursor_row;
            (void)SetConsoleCursorPosition(console->output, position);
        }
        (void)SetConsoleCursorInfo(console->output, &cursor);
    }
    return 1;
}

static lib_status ux_win32_console_create(const ux_binding *binding,
    ux_win32_console **out_console)
{
    ux_win32_console *console;

    if (out_console == NULL || ux_binding_validate(binding) != LIB_STATUS_OK)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_console = NULL;
    if (!ux_win32_console_acquire()) return LIB_STATUS_INVALID_STATE;
    console = calloc(1u, sizeof(*console));
    if (console == NULL) {
        ux_win32_console_release();
        return LIB_STATUS_NO_MEMORY;
    }
    console->binding = binding;
    if (!win32_console_open(binding, console) ||
        !SetConsoleMode(console->input, (console->original_mode &
            ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT |
                ENABLE_QUICK_EDIT_MODE)) | ENABLE_MOUSE_INPUT |
            ENABLE_EXTENDED_FLAGS)) {
        win32_console_close(console);
        free(console);
        ux_win32_console_release();
        return LIB_STATUS_INVALID_STATE;
    }
    if (!win32_console_ensure_text_surface(console->output)) {
        (void)SetConsoleMode(console->input, console->original_mode);
        win32_console_close(console);
        free(console);
        ux_win32_console_release();
        return LIB_STATUS_INVALID_STATE;
    }
    console->frame = calloc(1u, sizeof(*console->frame));
    if (console->frame == NULL) {
        (void)SetConsoleMode(console->input, console->original_mode);
        win32_console_close(console);
        free(console);
        ux_win32_console_release();
        return LIB_STATUS_NO_MEMORY;
    }
    memset(console->previous, 0xff, sizeof(console->previous));
    memset(console->previous_attributes, 0xff, sizeof(console->previous_attributes));
    memset(console->previous_palette, 0xff, sizeof(console->previous_palette));
    ux_router_set_active_target(binding->router, UX_TARGET_CONSOLE);
    *out_console = console;
    return LIB_STATUS_OK;
}

static ux_run_result ux_win32_console_run(ux_win32_console *console)
{
    HANDLE wait_handles[2];
    ux_run_result result = UX_RUN_STOPPED_RESULT;
    int running = 1;

    if (console == NULL) return UX_RUN_ERROR_RESULT;
    wait_handles[0] = console->input;
    wait_handles[1] = ux_win32_mailbox_wait_handle(console->binding->mailbox);
    while (running) {
        INPUT_RECORD record;
        DWORD available;
        DWORD read;

        while (PeekConsoleInputA(console->input, &record, 1u, &available) &&
            available != 0u) {
            if (!ReadConsoleInputA(console->input, &record, 1u, &read)) {
                running = 0;
                break;
            }
            if (record.EventType == KEY_EVENT) {
                result = win32_console_key(console, &record.Event.KeyEvent);
                if (result != UX_RUN_CONTINUE) {
                    running = 0;
                    break;
                }
            } else if (record.EventType == MOUSE_EVENT) {
                win32_console_mouse(console, &record.Event.MouseEvent);
            }
        }
        if (ux_router_target(console->binding->router) == UX_TARGET_NONE) {
            result = UX_RUN_STOPPED_RESULT;
            break;
        }
        if (ux_router_target(console->binding->router) == UX_TARGET_WINDOW) {
            result = UX_RUN_SWITCH_WINDOW;
            break;
        }
        if (ux_mailbox_generation(console->binding->mailbox) !=
                console->displayed_sequence &&
            ux_mailbox_capture(console->binding->mailbox, console->frame) ==
                LIB_STATUS_OK) {
            if (win32_console_paint(console))
                console->displayed_sequence = console->frame->sequence;
        }
        if (WaitForMultipleObjects(2u, wait_handles, FALSE, INFINITE) ==
            WAIT_FAILED) {
            result = UX_RUN_ERROR_RESULT;
            running = 0;
        }
    }
    return result;
}

static void ux_win32_console_destroy(ux_win32_console *console)
{
    if (console == NULL) return;
    (void)SetConsoleMode(console->input, console->original_mode);
    ux_router_set_active_target(console->binding->router, UX_TARGET_NONE);
    win32_console_close(console);
    free(console->frame);
    free(console);
    ux_win32_console_release();
}

ux_run_result ux_win32_run_console(const ux_binding *binding)
{
    ux_win32_console *console = NULL;
    ux_run_result result;

    if (ux_win32_console_create(binding, &console) != LIB_STATUS_OK)
        return UX_RUN_ERROR_RESULT;
    result = ux_win32_console_run(console);
    ux_win32_console_destroy(console);
    return result;
}
#endif
