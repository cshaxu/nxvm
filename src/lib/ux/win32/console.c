#include "console.h"

#ifdef _WIN32
#include "actions.h"
#include "input.h"

#include <windows.h>
#include <stdlib.h>
#include <string.h>

static STD_ATOMIC_FLAG ux_win32_console_lease = ATOMIC_FLAG_INIT;

static type_bool ux_win32_console_acquire(C_VOID)
{
    return !STD_ATOMIC_FLAG_TEST_AND_SET_EXPLICIT(&ux_win32_console_lease,
        STD_MEMORY_ORDER_ACQUIRE);
}

static C_VOID ux_win32_console_release(C_VOID)
{
    STD_ATOMIC_FLAG_CLEAR_EXPLICIT(&ux_win32_console_lease,
        STD_MEMORY_ORDER_RELEASE);
}

/* A console presenter is entirely a host surface.  It uses only the copied
 * frame ABI and the binding callbacks declared in presenter.h. */
static int win32_console_open(const ux_binding *binding,
    HANDLE *input_out, HANDLE *output_out,
    DWORD *original_mode_out, int *private_console_out)
{
    HANDLE input;
    HANDLE output;
    DWORD original_mode;
    int private_console = 0;

    input = CreateFileA("CONIN$", GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    output = CreateFileA("CONOUT$", GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (input == INVALID_HANDLE_VALUE || output == INVALID_HANDLE_VALUE ||
        !GetConsoleMode(input, &original_mode)) {
        if (input != INVALID_HANDLE_VALUE) CloseHandle(input);
        if (output != INVALID_HANDLE_VALUE) CloseHandle(output);
        if (GetConsoleCP() != 0u || !AllocConsole()) return 0;
        private_console = 1;
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
        if (binding->get_title != NULL) {
            char title[128] = "Presentation";
            binding->get_title(binding->context, title, sizeof(title));
            SetConsoleTitleA(title);
        }
    }
    *input_out = input;
    *output_out = output;
    *original_mode_out = original_mode;
    *private_console_out = private_console;
    return 1;
}

static void win32_console_ensure_text_surface(HANDLE output)
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    COORD required;

    if (output == NULL || output == INVALID_HANDLE_VALUE ||
        !GetConsoleScreenBufferInfo(output, &info)) return;
    required.X = info.dwSize.X < (SHORT)UX_TEXT_COLUMNS ?
        (SHORT)UX_TEXT_COLUMNS : info.dwSize.X;
    required.Y = info.dwSize.Y < (SHORT)UX_TEXT_ROWS ?
        (SHORT)UX_TEXT_ROWS : info.dwSize.Y;
    if (required.X != info.dwSize.X || required.Y != info.dwSize.Y)
        (void)SetConsoleScreenBufferSize(output, required);
}

static void win32_console_close(HANDLE input, HANDLE output,
    int private_console)
{
    if (private_console) {
        CloseHandle(input);
        CloseHandle(output);
        FreeConsole();
    }
}

static ux_run_result win32_console_key(
    const ux_binding *binding,
    ux_win32_keyboard_normalizer *normalizer,
    const KEY_EVENT_RECORD *key)
{
    ux_action action;

    if (!key->bKeyDown && normalizer->suppressed_virtual_key ==
        key->wVirtualKeyCode) {
        normalizer->suppressed_virtual_key = 0u;
        return UX_RUN_CONTINUE;
    }
    action = key->bKeyDown ? ux_actions_match(
        binding->actions, key->wVirtualKeyCode,
        ux_win32_modifiers_from_console_state(
            key->dwControlKeyState)) : UX_ACTION_NONE;
    if (action != UX_ACTION_NONE) {
        ux_run_result result = binding->handle_action(
            binding->context, action, binding->input_sink);
        normalizer->suppressed_virtual_key = key->wVirtualKeyCode;
        return result;
    }
    if (key->wVirtualScanCode == 0u && key->bKeyDown &&
        key->uChar.UnicodeChar != 0u) {
        (void)ux_win32_keyboard_submit_utf16(normalizer,
            binding->context, binding->input_sink, key->uChar.UnicodeChar);
    } else {
        (void)ux_win32_keyboard_submit_transition(binding->context,
            binding->input_sink, key->wVirtualScanCode,
            key->wVirtualKeyCode, key->dwControlKeyState,
            key->bKeyDown != 0);
    }
    return UX_RUN_CONTINUE;
}

static void win32_console_mouse(const ux_binding *binding,
    COORD *previous, int *previous_valid, const MOUSE_EVENT_RECORD *mouse)
{
    ux_event event;

    if (previous == NULL || previous_valid == NULL || mouse == NULL) return;
    memset(&event, 0, sizeof(event));
    event.type = UX_EVENT_MOUSE;
    if (*previous_valid) {
        event.data.mouse.delta_x = ((int32_t)mouse->dwMousePosition.X -
            previous->X) * 8;
        event.data.mouse.delta_y = ((int32_t)mouse->dwMousePosition.Y -
            previous->Y) * 16;
    }
    *previous = mouse->dwMousePosition;
    *previous_valid = 1;
    event.data.mouse.left_down =
        (mouse->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) != 0u;
    event.data.mouse.right_down =
        (mouse->dwButtonState & RIGHTMOST_BUTTON_PRESSED) != 0u;
    (void)binding->input_sink(binding->context, &event);
}

static int win32_console_paint(HANDLE output,
    const ux_frame *frame, unsigned char *previous,
    unsigned short *previous_attributes, uint32_t *previous_palette)
{
    CHAR_INFO cells[UX_TEXT_COLUMNS *
        UX_TEXT_ROWS];
    COORD size = { UX_TEXT_COLUMNS,
        UX_TEXT_ROWS };
    COORD position = { 0, 0 };
    SMALL_RECT region = { 0, 0, UX_TEXT_COLUMNS - 1,
        UX_TEXT_ROWS - 1 };
    unsigned int row;

    if (frame->valid == 0u || frame->graphics != 0u) return 1;
    if (memcmp(frame->text_palette, previous_palette,
            sizeof(frame->text_palette)) != 0) {
        CONSOLE_SCREEN_BUFFER_INFOEX info;
        unsigned int index;

        memset(&info, 0, sizeof(info));
        info.cbSize = sizeof(info);
        if (GetConsoleScreenBufferInfoEx(output, &info)) {
            for (index = 0u; index < 16u; ++index)
                info.ColorTable[index] = (COLORREF)frame->text_palette[index];
            (void)SetConsoleScreenBufferInfoEx(output, &info);
        }
        memcpy(previous_palette, frame->text_palette,
            sizeof(frame->text_palette));
    }
    if (memcmp(frame->text, previous, sizeof(frame->text)) == 0 &&
        memcmp(frame->attributes, previous_attributes,
            sizeof(frame->attributes)) == 0) return 1;
    for (row = 0; row < UX_TEXT_ROWS; ++row) {
        unsigned int column;
        for (column = 0; column < UX_TEXT_COLUMNS; ++column) {
            size_t offset = row * UX_TEXT_COLUMNS + column;
            unsigned char character = frame->text[offset];
            cells[offset].Char.AsciiChar = character >= 0x20u &&
                character < 0x7fu ? (CHAR)character : ' ';
            cells[offset].Attributes = (WORD)frame->attributes[offset];
        }
    }
    if (!WriteConsoleOutputA(output, cells, size, position, &region)) return 0;
    memcpy(previous, frame->text, sizeof(frame->text));
    memcpy(previous_attributes, frame->attributes, sizeof(frame->attributes));
    {
        CONSOLE_CURSOR_INFO cursor;
        cursor.dwSize = frame->cursor_size;
        if (cursor.dwSize == 0u || cursor.dwSize > 100u) cursor.dwSize = 100u;
        cursor.bVisible = frame->cursor_column >= 0 && frame->cursor_row >= 0 &&
            frame->cursor_column < (int32_t)UX_TEXT_COLUMNS &&
            frame->cursor_row < (int32_t)UX_TEXT_ROWS;
        if (cursor.bVisible) {
            position.X = (SHORT)frame->cursor_column;
            position.Y = (SHORT)frame->cursor_row;
            (void)SetConsoleCursorPosition(output, position);
        }
        (void)SetConsoleCursorInfo(output, &cursor);
    }
    return 1;
}

ux_run_result ux_win32_run_console(
    const ux_binding *binding)
{
    HANDLE input;
    HANDLE output;
    DWORD original_mode;
    unsigned char previous[UX_TEXT_COLUMNS *
        UX_TEXT_ROWS];
    unsigned short previous_attributes[UX_TEXT_COLUMNS *
        UX_TEXT_ROWS];
    uint32_t previous_palette[16u];
    ux_frame *frame;
    uint32_t displayed_sequence = 0u;
    ux_win32_keyboard_normalizer keyboard_normalizer = { 0 };
    COORD mouse_previous = { 0, 0 };
    int mouse_previous_valid = 0;
    int running = 1;
    int private_console;
    ux_run_result result =
        UX_RUN_STOPPED_RESULT;
    HANDLE wait_handles[2];

    if (ux_binding_validate(binding) != TYPE_STATUS_OK ||
        !ux_win32_console_acquire())
        return UX_RUN_ERROR_RESULT;
    if (!win32_console_open(binding, &input, &output, &original_mode,
            &private_console)) {
        ux_win32_console_release();
        return UX_RUN_ERROR_RESULT;
    }
    if (!SetConsoleMode(input, (original_mode & ~(ENABLE_ECHO_INPUT |
            ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT |
            ENABLE_QUICK_EDIT_MODE)) | ENABLE_MOUSE_INPUT |
            ENABLE_EXTENDED_FLAGS)) {
        win32_console_close(input, output, private_console);
        ux_win32_console_release();
        return UX_RUN_ERROR_RESULT;
    }
    win32_console_ensure_text_surface(output);
    frame = (ux_frame *)calloc(1u, sizeof(*frame));
    if (frame == NULL) {
        win32_console_close(input, output, private_console);
        ux_win32_console_release();
        return UX_RUN_ERROR_RESULT;
    }
    memset(previous, 0xff, sizeof(previous));
    memset(previous_attributes, 0xff, sizeof(previous_attributes));
    memset(previous_palette, 0xff, sizeof(previous_palette));
    wait_handles[0] = input;
    while (running && binding->get_state(binding->context) ==
        UX_RUN_RUNNING) {
        INPUT_RECORD record;
        DWORD available;
        DWORD read;
        while (PeekConsoleInputA(input, &record, 1u, &available) && available != 0u) {
            if (!ReadConsoleInputA(input, &record, 1u, &read)) {
                running = 0;
                break;
            }
            if (record.EventType == KEY_EVENT) {
                result = win32_console_key(binding, &keyboard_normalizer,
                    &record.Event.KeyEvent);
                if (result != UX_RUN_CONTINUE) {
                    running = 0;
                    break;
                }
            } else if (record.EventType == MOUSE_EVENT) {
                win32_console_mouse(binding, &mouse_previous,
                    &mouse_previous_valid, &record.Event.MouseEvent);
            }
        }
        if (ux_mailbox_generation(binding->mailbox) != displayed_sequence &&
            ux_mailbox_capture(binding->mailbox, frame) == TYPE_STATUS_OK) {
            if (ux_router_observe(binding->router, frame) ==
                UX_TARGET_WINDOW) {
                result = UX_RUN_SWITCH_WINDOW;
                running = 0;
                break;
            }
            if (win32_console_paint(output, frame, previous,
                    previous_attributes, previous_palette))
                displayed_sequence = frame->sequence;
        }
        if (WaitForMultipleObjects(1u, wait_handles, FALSE, 250u) ==
            WAIT_FAILED) {
            result = UX_RUN_ERROR_RESULT;
            running = 0;
        }
    }
    if (result == UX_RUN_STOPPED_RESULT) {
        ux_run_state state = binding->get_state(binding->context);
        if (state == UX_RUN_PAUSED)
            result = UX_RUN_PAUSED_RESULT;
        else if (state == UX_RUN_ERROR)
            result = UX_RUN_ERROR_RESULT;
    }
    free(frame);
    (void)SetConsoleMode(input, original_mode);
    win32_console_close(input, output, private_console);
    ux_win32_console_release();
    return result;
}
#endif
