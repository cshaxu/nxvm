#include "lib/types/win32/scalar.h"
#include "lib/types/win32/sync.h"
#include "lib/types/types_interface.h"
#include "lib/ui-window/window.h"
#include "lib/ui-window/render.h"

#include "lib/ui-window/win32/geometry.h"
#include "lib/ui-base/input_interface.h"
#include "lib/ui-window/win32/input.h"
#include "lib/ui-base/mailbox_interface.h"
#include "lib/ui-window/win32/mouse.h"

#include "lib/types/win32/window.h"

#define WIN32_WINDOW_MAILBOX_READY (LIB_WIN32_WM_APP + 1u)
#define WIN32_WINDOW_MOUSE_READY (LIB_WIN32_WM_APP + 2u)
#define WIN32_WINDOW_DEFAULT_WIDTH 680
#define WIN32_WINDOW_DEFAULT_HEIGHT 560
#define WIN32_WINDOW_CURSOR_BLINK_INTERVAL_MS 250u

typedef struct ui_win32_window_context {
    ui_window *component;
    ui_frame frame;
    lib_win32_hdc surface_dc;
    lib_win32_hbitmap surface_bitmap;
    lib_win32_hgdiobj surface_previous_bitmap;
    lib_u32 *surface_pixels;
    lib_u32 surface_width;
    lib_u32 surface_height;
    lib_u32 graphics_palette[UI_GRAPHICS_PALETTE_ENTRIES];
    int graphics_valid;
    lib_u32 displayed_sequence;
    ui_keyboard_normalizer keyboard_normalizer;
    int left_button;
    int right_button;
    lib_i64 pending_mouse_dx;
    lib_i64 pending_mouse_dy;
    lib_u32 pending_mouse_buttons;
    int mouse_delivery_posted;
    ui_win32_mouse mouse;
    lib_u32 client_surface_width;
    lib_u32 client_surface_height;
    int client_width;
    int client_height;
    lib_bool correcting_aspect;
    lib_bool frozen;
    lib_bool cursor_blink_visible;
    lib_win32_dword cursor_blink_due;
    lib_win32_hcursor transparent_cursor;
} ui_win32_window_context;

static ui_win32_window_context *win32_window_context(lib_win32_hwnd window)
{
    return window == LIB_NULL ? LIB_NULL : (ui_win32_window_context *)
        lib_win32_get_window_long_ptr_a(window, LIB_WIN32_GWLP_USERDATA);
}

static int win32_window_accepting_input(const ui_win32_window_context *context)
{
    return context != LIB_NULL && context->component != LIB_NULL &&
        lib_atomic_i32_load_explicit(&context->component->base.stopping,
            LIB_MEMORY_ORDER_ACQUIRE) == 0;
}

/* Frozen is an application-requested content-input boundary. It is deliberately
 * separate from component lifetime: Window close and capture-release cleanup
 * still use accepting_input(). Native key transitions still reach ui-base's
 * generic matcher so a registered product hotkey can be delivered. */
static int win32_window_accepting_content_input(
    const ui_win32_window_context *context)
{
    return win32_window_accepting_input(context) &&
        context->frozen == LIB_FALSE;
}

/* ui-base has already attributed and matched this event.  Frozen Window
 * consumes ordinary matcher output, including mismatch replay, but continues
 * to forward the copied registered-hotkey event to the application sink. */
static int win32_window_deliver_normalized(void *opaque,
    const ui_input_event *event)
{
    ui_win32_window_context *context = (ui_win32_window_context *)opaque;

    if (!win32_window_accepting_input(context) || event == LIB_NULL) return 0;
    if (context->frozen != LIB_FALSE &&
        (event->type == UI_EVENT_KEY || event->type == UI_EVENT_TEXT ||
         event->type == UI_EVENT_MOUSE)) return 1;
    return context->component->base.input_sink(
        context->component->base.input_context, event);
}

static int win32_window_emit_normalized(void *opaque, const ui_input_event *event)
{
    ui_win32_window_context *context = (ui_win32_window_context *)opaque;

    if (!win32_window_accepting_input(context)) return 0;
    return ui_component_emit_to(&context->component->base, event,
        win32_window_deliver_normalized, context, context->frozen == LIB_FALSE);
}

static lib_win32_hcursor win32_window_create_transparent_cursor(void)
{
    unsigned char and_mask[32u * 4u];
    unsigned char xor_mask[32u * 4u];

    /* AND=1, XOR=0 preserves every underlying pixel, which is a transparent
       monochrome cursor.  Unlike SetCursor(NULL), this is an actual cursor
       image for remote-desktop cursor transport. */
    lib_memory_set(and_mask, 0xff, sizeof(and_mask));
    lib_memory_set(xor_mask, 0, sizeof(xor_mask));
    return lib_win32_create_cursor(lib_win32_get_module_handle_a(LIB_NULL), 0, 0, 32, 32,
        and_mask, xor_mask);
}

static void win32_window_set_client_cursor(
    const ui_win32_window_context *context, int captured)
{
    if (captured && context != LIB_NULL && context->transparent_cursor != LIB_NULL)
        lib_win32_set_cursor(context->transparent_cursor);
    else
        lib_win32_set_cursor(lib_win32_load_cursor_a(LIB_NULL, LIB_WIN32_IDC_ARROW));
}

static void win32_window_initial_bounds(int *left, int *top, int *width,
    int *height)
{
    lib_win32_point point = { 0, 0 };
    lib_win32_monitorinfo monitor_info;
    lib_win32_hmonitor monitor;
    ui_window_rect work, fitted;

    if (left == LIB_NULL || top == LIB_NULL || width == LIB_NULL || height == LIB_NULL) return;
    *left = LIB_WIN32_CW_USEDEFAULT;
    *top = 0;
    *width = WIN32_WINDOW_DEFAULT_WIDTH;
    *height = WIN32_WINDOW_DEFAULT_HEIGHT;
    (void)lib_win32_get_cursor_pos(&point);
    monitor = lib_win32_monitor_from_point(point, LIB_WIN32_MONITOR_DEFAULTTOPRIMARY);
    lib_win32_zero_memory(&monitor_info, sizeof(monitor_info));
    monitor_info.cbSize = sizeof(monitor_info);
    if (monitor == LIB_NULL || !lib_win32_get_monitor_info_a(monitor, &monitor_info))
        return;
    work = ui_win32_rect_value(&monitor_info.rcWork);
    if (!ui_window_fit_outer_rect(&work,
            WIN32_WINDOW_DEFAULT_WIDTH, WIN32_WINDOW_DEFAULT_HEIGHT, &fitted))
        return;
    *left = fitted.left;
    *top = fitted.top;
    *width = fitted.right - fitted.left;
    *height = fitted.bottom - fitted.top;
}

static void win32_window_destroy_surface(ui_win32_window_context *context)
{
    if (context == LIB_NULL) return;
    if (context->surface_dc != LIB_NULL && context->surface_previous_bitmap != LIB_NULL)
        lib_win32_select_object(context->surface_dc, context->surface_previous_bitmap);
    if (context->surface_bitmap != LIB_NULL) lib_win32_delete_object(context->surface_bitmap);
    if (context->surface_dc != LIB_NULL) lib_win32_delete_dc(context->surface_dc);
    context->surface_dc = LIB_NULL;
    context->surface_bitmap = LIB_NULL;
    context->surface_previous_bitmap = LIB_NULL;
    context->surface_pixels = LIB_NULL;
    context->surface_width = 0u;
    context->surface_height = 0u;
    context->graphics_valid = 0;
}

static int win32_window_ensure_surface(lib_win32_hwnd window,
    ui_win32_window_context *context, lib_u32 width, lib_u32 height)
{
    lib_win32_bitmapinfo info;
    lib_win32_hdc dc;

    if (window == LIB_NULL || context == LIB_NULL || width == 0u || height == 0u)
        return 0;
    if (context->surface_dc != LIB_NULL && context->surface_width == width &&
        context->surface_height == height) return 1;
    win32_window_destroy_surface(context);
    dc = lib_win32_get_dc(window);
    if (dc == LIB_NULL) return 0;
    context->surface_dc = lib_win32_create_compatible_dc(dc);
    lib_win32_zero_memory(&info, sizeof(info));
    info.bmiHeader.biSize = sizeof(info.bmiHeader);
    info.bmiHeader.biWidth = (lib_win32_long)width;
    info.bmiHeader.biHeight = -(lib_win32_long)height;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = LIB_WIN32_BI_RGB;
    if (context->surface_dc != LIB_NULL)
        context->surface_bitmap = lib_win32_create_dibsection(dc, &info, LIB_WIN32_DIB_RGB_COLORS,
            (void **)&context->surface_pixels, LIB_NULL, 0u);
    lib_win32_release_dc(window, dc);
    if (context->surface_dc == LIB_NULL || context->surface_bitmap == LIB_NULL ||
        context->surface_pixels == LIB_NULL) {
        win32_window_destroy_surface(context);
        return 0;
    }
    context->surface_previous_bitmap = lib_win32_select_object(context->surface_dc,
        context->surface_bitmap);
    if (context->surface_previous_bitmap == LIB_NULL) {
        win32_window_destroy_surface(context);
        return 0;
    }
    context->surface_width = width;
    context->surface_height = height;
    context->graphics_valid = 0;
    lib_memory_set(context->surface_pixels, 0,
        (lib_size)width * height * sizeof(*context->surface_pixels));
    return 1;
}

static int win32_window_display_rect(const ui_win32_window_context *context,
    lib_u32 source_width, lib_u32 source_height, ui_window_rect *display)
{
    return context != LIB_NULL && ui_window_display_rect(context->client_width,
        context->client_height, source_width, source_height, display);
}

static void win32_window_capture_client_size(lib_win32_hwnd window,
    ui_win32_window_context *context)
{
    lib_win32_rect client;

    if (window == LIB_NULL || context == LIB_NULL) return;
    if (!lib_win32_get_client_rect(window, &client)) return;
    context->client_width = client.right - client.left;
    context->client_height = client.bottom - client.top;
}

static void win32_window_enforce_aspect(lib_win32_hwnd window,
    ui_win32_window_context *context)
{
    if (context == LIB_NULL || context->correcting_aspect != LIB_FALSE ||
        context->surface_width == 0u || context->surface_height == 0u) return;
    context->correcting_aspect = LIB_TRUE;
    (void)ui_win32_enforce_client_aspect(window, context->surface_width,
        context->surface_height);
    context->correcting_aspect = LIB_FALSE;
}

static void win32_window_resize_client(lib_win32_hwnd window,
    ui_win32_window_context *context, lib_u32 width, lib_u32 height)
{
    if (window == LIB_NULL || context == LIB_NULL || width == 0u || height == 0u ||
        (context->client_surface_width == width &&
         context->client_surface_height == height)) return;
    if (!ui_win32_resize_client(window, width, height)) return;
    context->client_surface_width = width;
    context->client_surface_height = height;
}

static int win32_window_cursor_rect(lib_win32_hwnd window,
    const ui_win32_window_context *context, lib_win32_rect *cursor)
{
    ui_window_rect display;
    ui_window_rect result;
    if (!window || !context || !cursor || !win32_window_display_rect(context,
            context->surface_width, context->surface_height, &display)) return 0;
    if (!ui_window_cursor_rect(&context->frame, &display, &result)) return 0;
    ui_win32_rect_store(cursor, &result);
    return 1;
}

static int win32_window_paint(lib_win32_hwnd window, ui_win32_window_context *context,
    lib_win32_hdc dc)
{
    ui_window_rect display;

    if (context == LIB_NULL || context->surface_dc == LIB_NULL ||
        !ui_frame_is_valid(&context->frame) ||
        !win32_window_display_rect(context, context->surface_width,
            context->surface_height, &display)) return 1;
    if (!lib_win32_stretch_blt(dc, display.left, display.top, display.right - display.left,
        display.bottom - display.top, context->surface_dc, 0, 0,
        (int)context->surface_width, (int)context->surface_height, LIB_WIN32_SRCCOPY)) return 0;
    if (context->cursor_blink_visible) {
        lib_win32_rect cursor;
        if (win32_window_cursor_rect(window, context, &cursor))
            return lib_win32_invert_rect(dc, &cursor) != 0;
    }
    return 1;
}

static int win32_window_invalidate(lib_win32_hwnd window,
    ui_win32_window_context *context, const lib_win32_rect *rect)
{
    if (lib_win32_invalidate_rect(window, rect, LIB_WIN32_FALSE)) return 1;
    ui_component_fail(&context->component->base, LIB_STATUS_IO_ERROR);
    return 0;
}

static lib_win32_dword win32_window_cursor_blink_timeout(
    const ui_win32_window_context *context)
{
    lib_win32_dword now;

    if (!win32_window_accepting_input(context) ||
        context->frozen != LIB_FALSE ||
        !ui_frame_is_valid(&context->frame) ||
        context->frame.graphics != 0u || context->frame.cursor_visible == 0u)
        return LIB_WIN32_INFINITE;
    now = lib_win32_get_tick_count();
    return (lib_win32_long)(now - context->cursor_blink_due) >= 0 ? 0u :
        context->cursor_blink_due - now;
}

static void win32_window_advance_cursor_blink(lib_win32_hwnd window,
    ui_win32_window_context *context)
{
    lib_win32_rect cursor;

    if (win32_window_cursor_blink_timeout(context) != 0u) return;
    context->cursor_blink_visible = context->cursor_blink_visible == LIB_FALSE;
    context->cursor_blink_due = lib_win32_get_tick_count() + WIN32_WINDOW_CURSOR_BLINK_INTERVAL_MS;
    if (win32_window_cursor_rect(window, context, &cursor))
        (void)win32_window_invalidate(window, context, &cursor);
}

static int win32_window_transition(ui_win32_window_context *context,
    lib_win32_wparam key, lib_win32_lparam lparam, int released)
{
    ui_keyboard_record record = {
        UI_KEYBOARD_TRANSITION, (lib_u16)((lparam >> 16) & 0xffu),
        (lib_u16)key, 0u,
        ui_window_keyboard_flags_from_lparam((lib_u64)lparam),
        ui_window_modifiers_from_key_state(), !released, (lib_u16)lparam };
    return ui_keyboard_submit_record(&context->keyboard_normalizer,
        &context->component->base.hotkey_matcher, context,
        win32_window_emit_normalized, &record);
}

static lib_i32 win32_window_mouse_clamp(lib_i64 value)
{
    return value > LIB_INT32_MAX ? LIB_INT32_MAX :
        value < LIB_INT32_MIN ? LIB_INT32_MIN : (lib_i32)value;
}

static void win32_window_emit_mouse(ui_win32_window_context *context,
    lib_i32 dx, lib_i32 dy, lib_u32 buttons)
{
    ui_input_event event = { 0 };

    event.type = UI_EVENT_MOUSE;
    event.data.mouse.delta_x = dx;
    event.data.mouse.delta_y = dy;
    event.data.mouse.relative = 1u;
    event.data.mouse.buttons = buttons;
    (void)win32_window_emit_normalized(context, &event);
}

static void win32_window_flush_mouse(ui_win32_window_context *context)
{
    if (context == LIB_NULL || !context->mouse_delivery_posted) return;
    context->mouse_delivery_posted = 0;
    win32_window_emit_mouse(context,
        win32_window_mouse_clamp(context->pending_mouse_dx),
        win32_window_mouse_clamp(context->pending_mouse_dy),
        context->pending_mouse_buttons);
    context->pending_mouse_dx = 0;
    context->pending_mouse_dy = 0;
}

static void win32_window_queue_mouse(lib_win32_hwnd window,
    ui_win32_window_context *context, int dx, int dy)
{
    if (window == LIB_NULL || context == LIB_NULL) return;
    context->pending_mouse_dx += dx;
    context->pending_mouse_dy += dy;
    context->pending_mouse_buttons =
        (context->left_button ? UI_MOUSE_BUTTON_LEFT : 0u) |
        (context->right_button ? UI_MOUSE_BUTTON_RIGHT : 0u);
    if (context->mouse_delivery_posted) return;
    context->mouse_delivery_posted = 1;
    if (!lib_win32_post_message_a(window, WIN32_WINDOW_MOUSE_READY, 0u, 0))
        win32_window_flush_mouse(context);
}

static void win32_window_mouse(lib_win32_hwnd window, ui_win32_window_context *context,
    lib_win32_lparam position, int immediate)
{
    int dx = 0;
    int dy = 0;

    if (!win32_window_accepting_content_input(context) ||
        !ui_win32_mouse_move(&context->mouse, position, context->client_width,
            context->client_height, context->surface_width, context->surface_height,
            &dx, &dy)) return;
    if (immediate) {
        win32_window_emit_mouse(context, dx, dy,
            (context->left_button ? UI_MOUSE_BUTTON_LEFT : 0u) |
            (context->right_button ? UI_MOUSE_BUTTON_RIGHT : 0u));
    } else if (dx != 0 || dy != 0)
        win32_window_queue_mouse(window, context, dx, dy);
}

static void win32_window_release_mouse(ui_win32_window_context *context)
{
    if (context == LIB_NULL) return;
    win32_window_flush_mouse(context);
    /* left/right_button is content state only. A real content press must never
     * survive a host capture release, whereas the host-only capture gesture
     * leaves both bits clear and therefore emits nothing here. */
    if ((context->left_button || context->right_button) &&
        win32_window_accepting_input(context)) {
        context->left_button = 0;
        context->right_button = 0;
        win32_window_emit_mouse(context, 0, 0, 0u);
    } else {
        context->left_button = 0;
        context->right_button = 0;
    }
    ui_win32_mouse_release(&context->mouse);
}

static void win32_window_capture_mouse(lib_win32_hwnd window,
    ui_win32_window_context *context, lib_win32_lparam position)
{
    if (!win32_window_accepting_content_input(context))
        return;
    if (!ui_win32_mouse_capture(&context->mouse, window, position)) return;
    win32_window_set_client_cursor(context, 1);
}

static void win32_window_consume_frame(lib_win32_hwnd window,
    ui_win32_window_context *context)
{
    lib_u32 width;
    lib_u32 height;

    if (context == LIB_NULL || context->component == LIB_NULL ||
        !ui_component_mailboxes_capture_frame(&context->component->base.mailboxes,
            &context->displayed_sequence, &context->frame))
        return;
    if (!ui_window_frame_size(&context->frame, &width, &height) ||
        !win32_window_ensure_surface(window, context, width, height)) {
        ui_component_fail(&context->component->base, LIB_STATUS_IO_ERROR);
        return;
    }
    win32_window_resize_client(window, context, width, height);
    if (context->frame.graphics != 0u) {
        ui_window_rect changed;
        ui_window_rect changed_target;
        ui_window_rect display;
        lib_win32_rect target;
        if (ui_window_render_graphics(&context->frame, context->surface_pixels,
                context->surface_width, context->surface_height, context->graphics_palette,
                &context->graphics_valid, &changed) &&
            win32_window_display_rect(context, width, height, &display)) {
            ui_window_map_dirty_rect(&changed, &display, width, height, &changed_target);
            ui_win32_rect_store(&target, &changed_target);
            if (!win32_window_invalidate(window, context, &target)) return;
        }
    } else {
        ui_window_render_text(&context->frame, context->surface_pixels,
            context->surface_width, context->surface_height);
        if (!win32_window_invalidate(window, context, LIB_NULL)) return;
    }
    ui_component_mailboxes_acknowledge_frame(&context->component->base.mailboxes,
        context->displayed_sequence);
}

static int win32_window_consume_mailboxes(lib_win32_hwnd window,
    ui_win32_window_context *context)
{
    ui_component_control control;

    if (context == LIB_NULL || context->component == LIB_NULL) return 0;
    while (ui_component_mailboxes_take_control(&context->component->base.mailboxes,
            &control)) {
        if (control.kind == UI_COMPONENT_CONTROL_STOP) {
            lib_atomic_i32_store_explicit(&context->component->base.stopping, 1,
                LIB_MEMORY_ORDER_RELEASE);
            return 0;
        }
        if (control.kind == UI_COMPONENT_CONTROL_SET_WINDOW_TITLE) {
            if (!lib_win32_set_window_text_a(window, control.value.title)) {
                ui_component_fail(&context->component->base, LIB_STATUS_IO_ERROR);
                return 0;
            }
        }
        else if (control.kind == UI_COMPONENT_CONTROL_SET_WINDOW_FROZEN) {
            if (context->frozen == control.value.window_frozen) continue;
            if (context->frozen && !control.value.window_frozen) {
                (void)lib_win32_set_foreground_window(window);
                (void)lib_win32_set_focus(window);
            }
            context->frozen = control.value.window_frozen;
            if (context->frozen == LIB_FALSE) {
                context->cursor_blink_visible = LIB_TRUE;
                context->cursor_blink_due = lib_win32_get_tick_count() +
                    WIN32_WINDOW_CURSOR_BLINK_INTERVAL_MS;
            }
            if (!win32_window_invalidate(window, context, LIB_NULL)) return 0;
        } else if (control.kind == UI_COMPONENT_CONTROL_RELEASE_WINDOW_MOUSE)
            win32_window_release_mouse(context);
    }
    return 1;
}

static lib_win32_lresult LIB_WIN32_CALLBACK win32_window_proc(lib_win32_hwnd window, lib_win32_uint message,
    lib_win32_wparam wparam, lib_win32_lparam lparam)
{
    ui_win32_window_context *context;

    if (message == LIB_WIN32_WM_NCCREATE) {
        lib_win32_createstructw *create = (lib_win32_createstructw *)lparam;
        lib_win32_set_window_long_ptr_a(window, LIB_WIN32_GWLP_USERDATA, (lib_win32_long_ptr)create->lpCreateParams);
    }
    context = win32_window_context(window);
    if (context == LIB_NULL) return lib_win32_def_window_proc_w(window, message, wparam, lparam);
    switch (message) {
    case WIN32_WINDOW_MAILBOX_READY:
        if (win32_window_consume_mailboxes(window, context)) {
            win32_window_consume_frame(window, context);
        }
        return 0;
    case WIN32_WINDOW_MOUSE_READY:
        win32_window_flush_mouse(context);
        return 0;
    case LIB_WIN32_WM_PAINT:
        {
            lib_win32_paintstruct paint;
            lib_win32_hdc dc = lib_win32_begin_paint(window, &paint);
            int painted = dc != LIB_NULL && win32_window_paint(window, context, dc);
            /* End the paint transaction even if drawing failed. */
            if (dc != LIB_NULL) lib_win32_end_paint(window, &paint);
            if (!painted) ui_component_fail(&context->component->base, LIB_STATUS_IO_ERROR);
        }
        return 0;
    case LIB_WIN32_WM_SIZE:
        win32_window_capture_client_size(window, context);
        win32_window_enforce_aspect(window, context);
        if (!ui_win32_mouse_refresh_bounds(&context->mouse))
            win32_window_release_mouse(context);
        (void)win32_window_invalidate(window, context, LIB_NULL);
        return 0;
    case LIB_WIN32_WM_MOVE:
        if (!ui_win32_mouse_refresh_bounds(&context->mouse))
            win32_window_release_mouse(context);
        return 0;
    case LIB_WIN32_WM_CAPTURECHANGED:
        win32_window_release_mouse(context);
        return 0;
    case LIB_WIN32_WM_ERASEBKGND:
        return 1;
    case LIB_WIN32_WM_NCLBUTTONDBLCLK:
        if (wparam == LIB_WIN32_HTCAPTION) {
            if (lib_win32_is_zoomed(window)) lib_win32_show_window(window, LIB_WIN32_SW_RESTORE);
            context->client_surface_width = 0u;
            context->client_surface_height = 0u;
            if (context->surface_width != 0u)
                win32_window_resize_client(window, context, context->surface_width,
                    context->surface_height);
            return 0;
        }
        break;
    case LIB_WIN32_WM_SYSCOMMAND:
        if ((wparam & 0xfff0u) == LIB_WIN32_SC_MAXIMIZE) {
            if (context->surface_width != 0u && context->surface_height != 0u)
                (void)ui_win32_maximize_client(window, context->surface_width,
                    context->surface_height);
            return 0;
        }
        break;
    case LIB_WIN32_WM_SIZING:
        ui_win32_constrain_sizing(window, wparam, (lib_win32_rect *)lparam,
            context->surface_width, context->surface_height);
        return LIB_WIN32_TRUE;
    case LIB_WIN32_WM_KEYDOWN:
    case LIB_WIN32_WM_SYSKEYDOWN:
        if (win32_window_accepting_input(context) &&
            win32_window_transition(context, wparam, lparam, 0) == UI_KEYBOARD_UNMAPPED) {
            lib_win32_msg native = { 0 };
            native.hwnd = window; native.message = message;
            native.wParam = wparam; native.lParam = lparam;
            /* Only unmapped keys may produce characters. Accepted physical
             * input has no second WM_CHAR stream to correlate or suppress. */
            lib_win32_translate_message(&native);
        }
        return 0;
    case LIB_WIN32_WM_KEYUP:
    case LIB_WIN32_WM_SYSKEYUP:
        if (win32_window_accepting_input(context))
            win32_window_transition(context, wparam, lparam, 1);
        return 0;
    case LIB_WIN32_WM_CHAR:
    case LIB_WIN32_WM_SYSCHAR:
        if (win32_window_accepting_input(context)) {
            ui_keyboard_record record = {
                UI_KEYBOARD_CHARACTER, (lib_u16)((lparam >> 16) & 0xffu),
                0u, (lib_u16)wparam, 0u, 0u, LIB_TRUE, (lib_u16)lparam };
            (void)ui_keyboard_submit_record(&context->keyboard_normalizer,
                &context->component->base.hotkey_matcher, context,
                win32_window_emit_normalized, &record);
        }
        return 0;
    case LIB_WIN32_WM_MOUSEMOVE:
        if (ui_win32_mouse_captured(&context->mouse))
            win32_window_mouse(window, context, lparam, 0);
        return 0;
    case LIB_WIN32_WM_SETCURSOR:
        if (lib_win32_loword(lparam) == LIB_WIN32_HTCLIENT) {
            win32_window_set_client_cursor(context,
                ui_win32_mouse_captured(&context->mouse));
            return LIB_WIN32_TRUE;
        }
        break;
    case LIB_WIN32_WM_LBUTTONDOWN:
        if (!win32_window_accepting_content_input(context)) return 0;
        /* The first client click is the host-only capture gesture. Content
         * button state starts only with a later click while already captured. */
        if (!ui_win32_mouse_captured(&context->mouse)) {
            win32_window_capture_mouse(window, context, lparam);
            return 0;
        }
        win32_window_flush_mouse(context);
        context->left_button = 1;
        win32_window_mouse(window, context, lparam, 1);
        return 0;
    case LIB_WIN32_WM_LBUTTONUP:
        if (!win32_window_accepting_content_input(context)) return 0;
        /* A button which was never made content-visible is the matching
         * release of the host-only capture gesture. */
        if (!ui_win32_mouse_captured(&context->mouse) || !context->left_button)
            return 0;
        win32_window_flush_mouse(context);
        context->left_button = 0;
        win32_window_mouse(window, context, lparam, 1);
        return 0;
    case LIB_WIN32_WM_RBUTTONDOWN:
        if (!win32_window_accepting_content_input(context)) return 0;
        if (!ui_win32_mouse_captured(&context->mouse)) {
            win32_window_capture_mouse(window, context, lparam);
            return 0;
        }
        win32_window_flush_mouse(context);
        context->right_button = 1;
        win32_window_mouse(window, context, lparam, 1);
        return 0;
    case LIB_WIN32_WM_RBUTTONUP:
        if (!win32_window_accepting_content_input(context)) return 0;
        if (!ui_win32_mouse_captured(&context->mouse) || !context->right_button)
            return 0;
        win32_window_flush_mouse(context);
        context->right_button = 0;
        win32_window_mouse(window, context, lparam, 1);
        return 0;
    case LIB_WIN32_WM_KILLFOCUS:
        win32_window_release_mouse(context);
        return 0;
    case LIB_WIN32_WM_CLOSE:
        { ui_input_event close_event = { 0 };
        win32_window_release_mouse(context);
        close_event.type = UI_EVENT_WINDOW_CLOSE;
        (void)win32_window_emit_normalized(context, &close_event); }
        return 0;
    case LIB_WIN32_WM_DESTROY:
        win32_window_release_mouse(context);
        lib_win32_set_window_long_ptr_a(window, LIB_WIN32_GWLP_USERDATA, 0);
        return 0;
    }
    return lib_win32_def_window_proc_w(window, message, wparam, lparam);
}

static void win32_window_destroy(ui_win32_window_context *context, lib_win32_hwnd window)
{
    if (window != LIB_NULL && lib_win32_is_window(window)) lib_win32_destroy_window(window);
    win32_window_destroy_surface(context);
    if (context != LIB_NULL && context->transparent_cursor != LIB_NULL)
        lib_win32_destroy_cursor(context->transparent_cursor);
}

typedef struct ui_window_win32_state {
    lib_win32_handle worker;
    lib_win32_handle ready;
    lib_status startup_status;
    ui_win32_window_context context;
} ui_window_win32_state;

static lib_win32_dword LIB_WIN32_WINAPI ui_window_worker(void *opaque)
{
    ui_window *component = (ui_window *)opaque;
    ui_window_win32_state *state = component == LIB_NULL ? LIB_NULL :
        (ui_window_win32_state *)component->worker_state;
    ui_win32_window_context *context;
    lib_win32_wndclassw klass;
    lib_win32_msg message;
    lib_win32_hwnd window;
    int initial_left;
    int initial_top;
    int initial_width;
    int initial_height;

    if (state == LIB_NULL) return 0u;
    context = &state->context;
    lib_win32_zero_memory(&klass, sizeof(klass));
    klass.lpfnWndProc = win32_window_proc;
    klass.hInstance = lib_win32_get_module_handle_a(LIB_NULL);
    /* Client cursor selection is explicit in WM_SETCURSOR.  A class arrow
       would be restored by Windows (and, in practice, an RDP client) as the
       pointer moves, defeating content capture. */
    klass.hCursor = LIB_NULL;
    klass.hbrBackground = (lib_win32_hbrush)lib_win32_get_stock_object(LIB_WIN32_BLACK_BRUSH);
    klass.lpszClassName = L"LibUxWindow";
    if (lib_win32_register_class_w(&klass) == 0 && lib_win32_get_last_error() != LIB_WIN32_ERROR_CLASS_ALREADY_EXISTS) {
        state->startup_status = LIB_STATUS_INVALID_STATE;
        lib_win32_set_event(state->ready);
        return 0u;
    }
    win32_window_initial_bounds(&initial_left, &initial_top, &initial_width,
        &initial_height);
    window = lib_win32_create_window_ex_w(0, klass.lpszClassName, L"",
        LIB_WIN32_WS_THICKFRAME | LIB_WIN32_WS_OVERLAPPED | LIB_WIN32_WS_CAPTION | LIB_WIN32_WS_SYSMENU |
        LIB_WIN32_WS_MINIMIZEBOX | LIB_WIN32_WS_MAXIMIZEBOX, initial_left, initial_top,
        initial_width, initial_height,
        LIB_NULL, LIB_NULL, klass.hInstance, context);
    if (window == LIB_NULL) {
        state->startup_status = LIB_STATUS_INVALID_STATE;
        lib_win32_set_event(state->ready);
        return 0u;
    }
    if (!lib_win32_set_window_text_a(window, component->initial_title)) {
        lib_win32_destroy_window(window);
        state->startup_status = LIB_STATUS_IO_ERROR;
        lib_win32_set_event(state->ready);
        return 0u;
    }
    context->transparent_cursor = win32_window_create_transparent_cursor();
    if (context->transparent_cursor == LIB_NULL) {
        win32_window_destroy(context, window);
        state->startup_status = LIB_STATUS_IO_ERROR;
        lib_win32_set_event(state->ready);
        return 0u;
    }
    state->startup_status = LIB_STATUS_OK;
    ui_win32_mouse_reset(&context->mouse);
    lib_win32_send_message_a(window, WIN32_WINDOW_MAILBOX_READY, 0, 0);
    lib_win32_show_window(window, LIB_WIN32_SW_SHOW);
    lib_win32_update_window(window);
    lib_win32_set_foreground_window(window);
    lib_win32_set_focus(window);
    lib_win32_set_event(state->ready);
    while (lib_win32_is_window(window) && win32_window_accepting_input(context)) {
        ui_mailbox_wake_wait_result wait = ui_mailbox_wake_wait_messages(
            ui_component_mailboxes_wake(&component->base.mailboxes),
            win32_window_cursor_blink_timeout(context));
        if (wait == UI_MAILBOX_WAKE_WAIT_WAKE) {
            if (win32_window_accepting_input(context))
                lib_win32_send_message_a(window, WIN32_WINDOW_MAILBOX_READY, 0, 0);
        }
        else if (wait == UI_MAILBOX_WAKE_WAIT_FAULT) {
            ui_component_fail(&component->base, LIB_STATUS_IO_ERROR);
        }
        win32_window_advance_cursor_blink(window, context);
        while (win32_window_accepting_input(context) &&
            lib_win32_peek_message_w(&message, LIB_NULL, 0, 0, LIB_WIN32_PM_REMOVE)) {
            if (message.message == LIB_WIN32_WM_QUIT) {
                ui_component_fail(&component->base, LIB_STATUS_IO_ERROR);
                break;
            }
            lib_win32_dispatch_message_w(&message);
            win32_window_advance_cursor_blink(window, context);
        }
    }
    if (!lib_win32_is_window(window)) ui_component_fail(&component->base, LIB_STATUS_IO_ERROR);
    win32_window_release_mouse(context);
    if (lib_win32_is_window(window)) lib_win32_destroy_window(window);
    ui_component_retire(&component->base, LIB_STATUS_OK);
    win32_window_destroy(context, LIB_NULL);
    return 0u;
}

lib_status ui_window_worker_start(ui_window *component)
{
    ui_window_win32_state *state;
    lib_status startup_status;

    if (component == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    state = lib_allocate_zero(1u, sizeof(*state));
    if (state == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    state->context.component = component;
    state->context.frozen = component->initial_frozen;
    state->context.cursor_blink_visible = LIB_TRUE;
    state->context.cursor_blink_due = lib_win32_get_tick_count() +
        WIN32_WINDOW_CURSOR_BLINK_INTERVAL_MS;
    state->ready = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    if (state->ready == LIB_NULL) {
        lib_release(state); return LIB_STATUS_NO_MEMORY;
    }
    component->worker_state = state;
    state->worker = lib_win32_create_thread(LIB_NULL, 0u, ui_window_worker, component, 0u, LIB_NULL);
    if (state->worker == LIB_NULL) {
        component->worker_state = LIB_NULL;
        lib_win32_close_handle(state->ready);
        lib_release(state); return LIB_STATUS_NO_MEMORY;
    }
    (void)lib_win32_wait_for_single_object(state->ready, LIB_WIN32_INFINITE);
    if (state->startup_status != LIB_STATUS_OK) {
        startup_status = state->startup_status;
        (void)lib_win32_wait_for_single_object(state->worker, LIB_WIN32_INFINITE);
        lib_win32_close_handle(state->worker); lib_win32_close_handle(state->ready);
        component->worker_state = LIB_NULL; lib_release(state);
        return startup_status;
    }
    return LIB_STATUS_OK;
}

void ui_window_worker_join(ui_window *component)
{
    ui_window_win32_state *state;
    if (component == LIB_NULL || (state = (ui_window_win32_state *)
            component->worker_state) == LIB_NULL) return;
    /* STOP is already in the control FIFO.  The worker consumes it, closes
     * its Window, and thereby establishes completion before this join. */
    (void)lib_win32_wait_for_single_object(state->worker, LIB_WIN32_INFINITE);
    lib_win32_close_handle(state->worker);
    lib_win32_close_handle(state->ready);
    component->worker_state = LIB_NULL;
    lib_release(state);
}
