#include "lib/base/base.h"
#include "window.h"

#ifdef _WIN32
#include "actions.h"
#include "geometry.h"
#include "input.h"
#include "mailbox.h"
#include "mouse.h"

#include <windows.h>
#include <stdlib.h>
#include <string.h>

#define WIN32_WINDOW_TEXT_CELL_WIDTH 8u
#define WIN32_WINDOW_TEXT_CELL_HEIGHT 16u
#define WIN32_WINDOW_FRAME_READY (WM_APP + 1u)
#define WIN32_WINDOW_CURSOR_BLINK_INTERVAL_MS 250u

typedef struct ux_win32_window_context {
    const ux_binding *binding;
    ux_frame *frame;
    HDC surface_dc;
    HBITMAP surface_bitmap;
    HGDIOBJ surface_previous_bitmap;
    uint32_t *surface_pixels;
    uint32_t surface_width;
    uint32_t surface_height;
    uint32_t graphics_palette[UX_GRAPHICS_PALETTE_ENTRIES];
    int graphics_valid;
    uint32_t displayed_sequence;
    ux_run_result result;
    ux_win32_keyboard_normalizer keyboard_normalizer;
    int left_button;
    int right_button;
    ux_win32_mouse mouse;
    WPARAM suppressed_hotkey;
    uint32_t client_surface_width;
    uint32_t client_surface_height;
    int client_width;
    int client_height;
    int cursor_blink_visible;
    DWORD cursor_blink_due;
} ux_win32_window_context;

static ux_win32_window_context *win32_window_context(HWND window)
{
    return window == NULL ? NULL : (ux_win32_window_context *)
        GetWindowLongPtrA(window, GWLP_USERDATA);
}

static int win32_window_content_running(const ux_win32_window_context *context)
{
    return context != NULL && context->binding != NULL &&
        context->binding->get_state(context->binding->context) == UX_RUN_RUNNING;
}

static void win32_window_update_title(HWND window,
    const ux_win32_window_context *context)
{
    char title[128];

    if (window == NULL || context == NULL || context->binding->get_title == NULL)
        return;
    context->binding->get_title(context->binding->context, title, sizeof(title));
    SetWindowTextA(window, title);
}

static void win32_window_destroy_surface(ux_win32_window_context *context)
{
    if (context == NULL) return;
    if (context->surface_dc != NULL && context->surface_previous_bitmap != NULL)
        SelectObject(context->surface_dc, context->surface_previous_bitmap);
    if (context->surface_bitmap != NULL) DeleteObject(context->surface_bitmap);
    if (context->surface_dc != NULL) DeleteDC(context->surface_dc);
    context->surface_dc = NULL;
    context->surface_bitmap = NULL;
    context->surface_previous_bitmap = NULL;
    context->surface_pixels = NULL;
    context->surface_width = 0u;
    context->surface_height = 0u;
    context->graphics_valid = 0;
}

static int win32_window_ensure_surface(HWND window,
    ux_win32_window_context *context, uint32_t width, uint32_t height)
{
    BITMAPINFO info;
    HDC dc;

    if (window == NULL || context == NULL || width == 0u || height == 0u)
        return 0;
    if (context->surface_dc != NULL && context->surface_width == width &&
        context->surface_height == height) return 1;
    win32_window_destroy_surface(context);
    dc = GetDC(window);
    if (dc == NULL) return 0;
    context->surface_dc = CreateCompatibleDC(dc);
    ZeroMemory(&info, sizeof(info));
    info.bmiHeader.biSize = sizeof(info.bmiHeader);
    info.bmiHeader.biWidth = (LONG)width;
    info.bmiHeader.biHeight = -(LONG)height;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    if (context->surface_dc != NULL)
        context->surface_bitmap = CreateDIBSection(dc, &info, DIB_RGB_COLORS,
            (void **)&context->surface_pixels, NULL, 0u);
    ReleaseDC(window, dc);
    if (context->surface_dc == NULL || context->surface_bitmap == NULL ||
        context->surface_pixels == NULL) {
        win32_window_destroy_surface(context);
        return 0;
    }
    context->surface_previous_bitmap = SelectObject(context->surface_dc,
        context->surface_bitmap);
    context->surface_width = width;
    context->surface_height = height;
    context->graphics_valid = 0;
    memset(context->surface_pixels, 0,
        (size_t)width * height * sizeof(*context->surface_pixels));
    return 1;
}

static int win32_window_display_rect(const ux_win32_window_context *context,
    uint32_t source_width, uint32_t source_height, RECT *display)
{
    return context != NULL && ux_win32_display_rect(context->client_width,
        context->client_height, source_width, source_height, display);
}

static void win32_window_capture_client_size(HWND window,
    ux_win32_window_context *context)
{
    RECT client;

    if (window == NULL || context == NULL) return;
    GetClientRect(window, &client);
    context->client_width = client.right - client.left;
    context->client_height = client.bottom - client.top;
}

static void win32_window_resize_client(HWND window,
    ux_win32_window_context *context, uint32_t width, uint32_t height)
{
    if (window == NULL || context == NULL || width == 0u || height == 0u ||
        (context->client_surface_width == width &&
         context->client_surface_height == height)) return;
    if (!ux_win32_resize_client(window, width, height)) return;
    context->client_surface_width = width;
    context->client_surface_height = height;
}

static int win32_window_frame_size(const ux_frame *frame, uint32_t *width,
    uint32_t *height)
{
    if (!ux_frame_is_valid(frame) || width == NULL || height == NULL) return 0;
    if (frame->graphics != 0u) {
        *width = frame->graphics_width;
        *height = frame->graphics_height;
    } else {
        *width = frame->text_columns * WIN32_WINDOW_TEXT_CELL_WIDTH;
        *height = frame->text_rows * WIN32_WINDOW_TEXT_CELL_HEIGHT;
    }
    return 1;
}

static uint32_t win32_window_dib_pixel(COLORREF colour)
{
    return ux_win32_dib_pixel(colour);
}

static void win32_window_update_text(ux_win32_window_context *context)
{
    ux_frame *frame;
    uint32_t row;

    if (context == NULL || context->surface_pixels == NULL ||
        (frame = context->frame) == NULL || frame->graphics != 0u) return;
    memset(context->surface_pixels, 0, (size_t)context->surface_width *
        context->surface_height * sizeof(*context->surface_pixels));
    for (row = 0u; row < frame->text_rows; ++row) {
        uint32_t column;
        for (column = 0u; column < frame->text_columns; ++column) {
            size_t index = (size_t)row * UX_TEXT_COLUMNS + column;
            lib_u8 character = frame->text[index];
            lib_u16 attribute = frame->attributes[index];
            uint32_t scan;
            for (scan = 0u; scan < WIN32_WINDOW_TEXT_CELL_HEIGHT; ++scan) {
                const lib_u8 *font = frame->attribute_font_select != 0u &&
                    (attribute & 0x08u) != 0u ? frame->secondary_font : frame->font;
                lib_u8 bits = font[(size_t)character * 16u + scan];
                uint32_t *pixels = context->surface_pixels +
                    ((size_t)row * WIN32_WINDOW_TEXT_CELL_HEIGHT + scan) *
                    context->surface_width + column * WIN32_WINDOW_TEXT_CELL_WIDTH;
                uint32_t bit;
                for (bit = 0u; bit < WIN32_WINDOW_TEXT_CELL_WIDTH; ++bit)
                    pixels[bit] = win32_window_dib_pixel((COLORREF)
                        frame->text_palette[(bits & (0x80u >> bit)) != 0u ?
                            attribute & 0x0fu : (attribute >> 4) & 0x0fu]);
            }
        }
    }
}

static int win32_window_update_graphics(ux_win32_window_context *context,
    RECT *changed)
{
    ux_frame *frame;
    int full_refresh;
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;
    uint32_t row;

    if (context == NULL || context->surface_pixels == NULL || changed == NULL ||
        (frame = context->frame) == NULL || frame->graphics == 0u ||
        context->surface_width != frame->graphics_width ||
        context->surface_height != frame->graphics_height) return 0;
    full_refresh = !context->graphics_valid || memcmp(context->graphics_palette,
        frame->graphics_palette, sizeof(context->graphics_palette)) != 0;
    left = full_refresh ? 0 : frame->dirty_left;
    top = full_refresh ? 0 : frame->dirty_top;
    right = full_refresh ? (int32_t)frame->graphics_width - 1 : frame->dirty_right;
    bottom = full_refresh ? (int32_t)frame->graphics_height - 1 : frame->dirty_bottom;
    if (left < 0) left = 0;
    if (top < 0) top = 0;
    if (right >= (int32_t)frame->graphics_width) right = (int32_t)frame->graphics_width - 1;
    if (bottom >= (int32_t)frame->graphics_height) bottom = (int32_t)frame->graphics_height - 1;
    if (right < left || bottom < top) return 0;
    for (row = (uint32_t)top; row <= (uint32_t)bottom; ++row) {
        const lib_u8 *source = frame->graphics_pixels + row * frame->graphics_stride;
        uint32_t *destination = context->surface_pixels + row * context->surface_width;
        uint32_t column;
        for (column = (uint32_t)left; column <= (uint32_t)right; ++column)
            destination[column] = frame->graphics_palette[source[column]];
    }
    memcpy(context->graphics_palette, frame->graphics_palette,
        sizeof(context->graphics_palette));
    context->graphics_valid = 1;
    changed->left = left;
    changed->top = top;
    changed->right = right + 1;
    changed->bottom = bottom + 1;
    return 1;
}

static int win32_window_cursor_rect(HWND window,
    const ux_win32_window_context *context, RECT *cursor)
{
    RECT display;
    const ux_frame *frame;
    int width;
    int height;
    int cell_height;
    int cursor_height;
    uint32_t cursor_percent;

    if (window == NULL || context == NULL || cursor == NULL ||
        (frame = context->frame) == NULL || !ux_frame_is_valid(frame) ||
        frame->graphics != 0u || frame->cursor_visible == 0u ||
        frame->cursor_phase == 0u || frame->cursor_column < 0 ||
        frame->cursor_row < 0 || frame->cursor_column >= (lib_i32)frame->text_columns ||
        frame->cursor_row >= (lib_i32)frame->text_rows ||
        !win32_window_display_rect(context, context->surface_width,
            context->surface_height, &display)) return 0;
    width = display.right - display.left;
    height = display.bottom - display.top;
    cell_height = height / frame->text_rows;
    if (width <= 0 || cell_height <= 0) return 0;
    cursor_percent = frame->cursor_bottom >= frame->cursor_top &&
        frame->font_height != 0u ? (frame->cursor_bottom - frame->cursor_top + 1u) *
            100u / frame->font_height : 100u;
    if (cursor_percent == 0u || cursor_percent > 100u) cursor_percent = 100u;
    cursor_height = (int)((cell_height * cursor_percent + 99u) / 100u);
    if (cursor_height > cell_height) cursor_height = cell_height;
    cursor->left = display.left + frame->cursor_column * width / frame->text_columns;
    cursor->right = display.left + (frame->cursor_column + 1) * width /
        frame->text_columns;
    cursor->top = display.top + (frame->cursor_row + 1) * cell_height - cursor_height;
    cursor->bottom = display.top + (frame->cursor_row + 1) * height / frame->text_rows;
    return cursor->right > cursor->left && cursor->bottom > cursor->top;
}

static void win32_window_paint(HWND window, ux_win32_window_context *context,
    HDC dc)
{
    RECT display;

    if (context == NULL || context->surface_dc == NULL ||
        !ux_frame_is_valid(context->frame) ||
        !win32_window_display_rect(context, context->surface_width,
            context->surface_height, &display)) return;
    StretchBlt(dc, display.left, display.top, display.right - display.left,
        display.bottom - display.top, context->surface_dc, 0, 0,
        (int)context->surface_width, (int)context->surface_height, SRCCOPY);
    if (context->cursor_blink_visible) {
        RECT cursor;
        if (win32_window_cursor_rect(window, context, &cursor)) InvertRect(dc, &cursor);
    }
}

static void win32_window_advance_cursor_blink(HWND window,
    ux_win32_window_context *context)
{
    RECT cursor;
    DWORD now = GetTickCount();

    if (!win32_window_content_running(context) ||
        (LONG)(now - context->cursor_blink_due) < 0) return;
    context->cursor_blink_visible = !context->cursor_blink_visible;
    context->cursor_blink_due = now + WIN32_WINDOW_CURSOR_BLINK_INTERVAL_MS;
    if (win32_window_cursor_rect(window, context, &cursor))
        InvalidateRect(window, &cursor, FALSE);
}

static void win32_window_transition(ux_win32_window_context *context,
    WPARAM key, LPARAM lparam, int released)
{
    WORD scan = (WORD)((lparam >> 16) & 0xffu);
    DWORD control_state = (lparam & 0x01000000L) != 0 ? ENHANCED_KEY : 0u;

    if (scan == 0u && !released)
        ux_win32_keyboard_note_recovered_key(&context->keyboard_normalizer, (WORD)key);
    if (scan == 0u && released)
        ux_win32_keyboard_release_recovered_key(&context->keyboard_normalizer,
            (WORD)key);
    (void)ux_win32_keyboard_submit_transition(context->binding->context,
        context->binding->input_sink, scan, (WORD)key, control_state, !released);
}

static void win32_window_mouse(ux_win32_window_context *context,
    LPARAM position)
{
    int dx = 0;
    int dy = 0;
    ux_event event = { 0 };

    if (!win32_window_content_running(context) ||
        !ux_win32_mouse_move(&context->mouse, position, context->client_width,
            context->client_height, context->surface_width, context->surface_height,
            &dx, &dy)) return;
    event.type = UX_EVENT_MOUSE;
    event.data.mouse.delta_x = dx;
    event.data.mouse.delta_y = dy;
    event.data.mouse.relative = 1u;
    event.data.mouse.buttons = (context->left_button ? UX_MOUSE_BUTTON_LEFT : 0u) |
        (context->right_button ? UX_MOUSE_BUTTON_RIGHT : 0u);
    (void)context->binding->input_sink(context->binding->context, &event);
}

static void win32_window_release_mouse(ux_win32_window_context *context)
{
    if (context != NULL) ux_win32_mouse_release(&context->mouse);
}

static void win32_window_capture_mouse(HWND window,
    ux_win32_window_context *context, LPARAM position)
{
    if (!win32_window_content_running(context)) return;
    (void)ux_win32_mouse_capture(&context->mouse, window, position);
    SetCursor(NULL);
}

static void win32_window_consume_frame(HWND window,
    ux_win32_window_context *context)
{
    ux_target target;
    uint32_t width;
    uint32_t height;

    if (context == NULL) return;
    target = ux_router_target(context->binding->router);
    if (target == UX_TARGET_NONE) {
        context->result = UX_RUN_STOPPED_RESULT;
        DestroyWindow(window);
        return;
    }
    if (target == UX_TARGET_CONSOLE) {
        context->result = UX_RUN_SWITCH_CONSOLE;
        DestroyWindow(window);
        return;
    }
    if (ux_mailbox_generation(context->binding->mailbox) ==
            context->displayed_sequence ||
        ux_mailbox_capture(context->binding->mailbox, context->frame) != LIB_STATUS_OK)
        return;
    context->displayed_sequence = context->frame->sequence;
    if (!win32_window_frame_size(context->frame, &width, &height) ||
        !win32_window_ensure_surface(window, context, width, height)) {
        context->result = UX_RUN_ERROR_RESULT;
        DestroyWindow(window);
        return;
    }
    win32_window_resize_client(window, context, width, height);
    if (context->frame->graphics != 0u) {
        RECT changed;
        RECT display;
        RECT target;
        if (win32_window_update_graphics(context, &changed) &&
            win32_window_display_rect(context, width, height, &display)) {
            ux_win32_map_dirty_rect(&changed, &display, width, height, &target);
            InvalidateRect(window, &target, FALSE);
        }
    } else {
        win32_window_update_text(context);
        InvalidateRect(window, NULL, FALSE);
    }
}

static LRESULT CALLBACK win32_window_proc(HWND window, UINT message,
    WPARAM wparam, LPARAM lparam)
{
    ux_win32_window_context *context;

    if (message == WM_NCCREATE) {
        CREATESTRUCTA *create = (CREATESTRUCTA *)lparam;
        SetWindowLongPtrA(window, GWLP_USERDATA, (LONG_PTR)create->lpCreateParams);
    }
    context = win32_window_context(window);
    if (context == NULL) return DefWindowProcA(window, message, wparam, lparam);
    switch (message) {
    case WIN32_WINDOW_FRAME_READY:
        win32_window_consume_frame(window, context);
        win32_window_advance_cursor_blink(window, context);
        win32_window_update_title(window, context);
        return 0;
    case WM_PAINT:
        { PAINTSTRUCT paint; HDC dc = BeginPaint(window, &paint);
          win32_window_paint(window, context, dc); EndPaint(window, &paint); }
        return 0;
    case WM_SIZE:
        win32_window_capture_client_size(window, context);
        return 0;
    case WM_ERASEBKGND:
        return 1;
    case WM_NCLBUTTONDBLCLK:
        if (wparam == HTCAPTION) {
            if (IsZoomed(window)) ShowWindow(window, SW_RESTORE);
            context->client_surface_width = 0u;
            context->client_surface_height = 0u;
            if (context->surface_width != 0u)
                win32_window_resize_client(window, context, context->surface_width,
                    context->surface_height);
            return 0;
        }
        break;
    case WM_SIZING:
        ux_win32_constrain_sizing(window, wparam, (RECT *)lparam,
            context->surface_width, context->surface_height);
        return TRUE;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN: {
        lib_u8 modifiers = ux_win32_modifiers_from_key_state();
        ux_action action = ux_actions_match(context->binding->actions,
            (WORD)wparam, modifiers);
        if (action != UX_ACTION_NONE) {
            ux_run_result result;
            win32_window_release_mouse(context);
            result = ux_binding_invoke_action(context->binding, action);
            if (result != UX_RUN_CONTINUE) context->result = result;
            win32_window_update_title(window, context);
            context->suppressed_hotkey = wparam;
        } else if (win32_window_content_running(context)) {
            win32_window_transition(context, wparam, lparam, 0);
        }
        return 0;
    }
    case WM_KEYUP:
    case WM_SYSKEYUP:
        if (wparam == context->suppressed_hotkey) {
            context->suppressed_hotkey = 0u;
            return 0;
        }
        if (win32_window_content_running(context))
            win32_window_transition(context, wparam, lparam, 1);
        return 0;
    case WM_CHAR:
        if (win32_window_content_running(context) &&
            ((uint32_t)lparam >> 16u & 0xffu) == 0u &&
            !ux_win32_keyboard_consume_duplicate_character(&context->keyboard_normalizer,
                (WORD)wparam))
            (void)ux_win32_keyboard_submit_utf16(&context->keyboard_normalizer,
                context->binding->context, context->binding->input_sink, (WORD)wparam);
        return 0;
    case WM_MOUSEMOVE:
        if (ux_win32_mouse_captured(&context->mouse))
            win32_window_mouse(context, lparam);
        return 0;
    case WM_SETCURSOR:
        if (ux_win32_mouse_captured(&context->mouse) && LOWORD(lparam) == HTCLIENT) {
            SetCursor(NULL);
            return TRUE;
        }
        break;
    case WM_LBUTTONDOWN:
        if (!win32_window_content_running(context)) return 0;
        context->left_button = 1;
        win32_window_capture_mouse(window, context, lparam);
        win32_window_mouse(context, lparam);
        return 0;
    case WM_LBUTTONUP:
        if (!win32_window_content_running(context)) return 0;
        context->left_button = 0;
        if (ux_win32_mouse_captured(&context->mouse)) win32_window_mouse(context, lparam);
        return 0;
    case WM_RBUTTONDOWN:
        if (!win32_window_content_running(context)) return 0;
        context->right_button = 1;
        win32_window_capture_mouse(window, context, lparam);
        win32_window_mouse(context, lparam);
        return 0;
    case WM_RBUTTONUP:
        if (!win32_window_content_running(context)) return 0;
        context->right_button = 0;
        if (ux_win32_mouse_captured(&context->mouse)) win32_window_mouse(context, lparam);
        return 0;
    case WM_KILLFOCUS:
        win32_window_release_mouse(context);
        return 0;
    case WM_CLOSE:
        win32_window_release_mouse(context);
        context->result = context->binding->handle_close(context->binding->context,
            context->binding->input_sink);
        if (context->result != UX_RUN_CONTINUE) DestroyWindow(window);
        return 0;
    case WM_DESTROY:
        win32_window_release_mouse(context);
        SetWindowLongPtrA(window, GWLP_USERDATA, 0);
        return 0;
    }
    return DefWindowProcA(window, message, wparam, lparam);
}

static void win32_window_destroy(ux_win32_window_context *context, HWND window)
{
    if (window != NULL && IsWindow(window)) DestroyWindow(window);
    win32_window_destroy_surface(context);
    if (context != NULL) free(context->frame);
    free(context);
}

ux_run_result ux_win32_run_window(const ux_binding *binding)
{
    WNDCLASSA klass;
    MSG message;
    HWND window;
    ux_win32_window_context *context;
    ux_run_result result;

    if (ux_binding_validate(binding) != LIB_STATUS_OK) return UX_RUN_ERROR_RESULT;
    ZeroMemory(&klass, sizeof(klass));
    klass.lpfnWndProc = win32_window_proc;
    klass.hInstance = GetModuleHandleA(NULL);
    klass.hCursor = LoadCursorA(NULL, IDC_ARROW);
    klass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    klass.lpszClassName = "Win32PresentationWindow";
    if (RegisterClassA(&klass) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        return UX_RUN_ERROR_RESULT;
    context = calloc(1u, sizeof(*context));
    if (context == NULL) return UX_RUN_ERROR_RESULT;
    context->frame = calloc(1u, sizeof(*context->frame));
    if (context->frame == NULL) {
        free(context);
        return UX_RUN_ERROR_RESULT;
    }
    context->binding = binding;
    context->result = UX_RUN_STOPPED_RESULT;
    context->cursor_blink_visible = 1;
    context->cursor_blink_due = GetTickCount() + WIN32_WINDOW_CURSOR_BLINK_INTERVAL_MS;
    ux_win32_mouse_reset(&context->mouse);
    {
        char title[128] = "Presentation";
        if (binding->get_title != NULL)
            binding->get_title(binding->context, title, sizeof(title));
        window = CreateWindowExA(0, klass.lpszClassName, title,
            WS_THICKFRAME | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU |
            WS_MINIMIZEBOX | WS_MAXIMIZEBOX, CW_USEDEFAULT, 0, 680, 560,
            NULL, NULL, klass.hInstance, context);
    }
    if (window == NULL) {
        win32_window_destroy(context, NULL);
        return UX_RUN_ERROR_RESULT;
    }
    ShowWindow(window, SW_SHOW);
    UpdateWindow(window);
    SetForegroundWindow(window);
    SetFocus(window);
    while (IsWindow(window)) {
        HANDLE wait_handle = ux_win32_mailbox_wait_handle(binding->mailbox);
        DWORD wait = MsgWaitForMultipleObjects(1u, &wait_handle, FALSE,
            INFINITE, QS_ALLINPUT);

        if (wait == WAIT_OBJECT_0) SendMessageA(window, WIN32_WINDOW_FRAME_READY, 0, 0);
        else if (wait == WAIT_FAILED) {
            context->result = UX_RUN_ERROR_RESULT;
            DestroyWindow(window);
        }
        while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) {
                context->result = UX_RUN_ERROR_RESULT;
                if (IsWindow(window)) DestroyWindow(window);
                break;
            }
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
    }
    result = context->result;
    win32_window_destroy(context, NULL);
    return result;
}
#endif
