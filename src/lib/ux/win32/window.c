#include "lib/base/base.h"
#include "window.h"

#ifdef _WIN32
#include "actions.h"
#include "geometry.h"
#include "input.h"
#include "mailbox.h"
#include "mouse.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIN32_WINDOW_TEXT_CELL_WIDTH 8
#define WIN32_WINDOW_TEXT_CELL_HEIGHT 16
#define WIN32_WINDOW_TEXT_SURFACE_WIDTH (UX_TEXT_COLUMNS * WIN32_WINDOW_TEXT_CELL_WIDTH)
#define WIN32_WINDOW_TEXT_SURFACE_HEIGHT (UX_TEXT_ROWS * WIN32_WINDOW_TEXT_CELL_HEIGHT)
#define WIN32_WINDOW_GRAPHICS_SURFACE_MAX_WIDTH 1280u
#define WIN32_WINDOW_GRAPHICS_SURFACE_MAX_HEIGHT 768u
#define WIN32_WINDOW_FRAME_READY (WM_APP + 1u)
#define WIN32_WINDOW_CURSOR_BLINK_INTERVAL_MS 250u

typedef struct ux_win32_window_context {
    const ux_binding *binding;
    ux_frame *frame;
    HDC text_dc;
    HBITMAP text_bitmap;
    HGDIOBJ text_previous_bitmap;
    uint32_t *text_pixels;
    HDC graphics_dc;
    HBITMAP graphics_bitmap;
    HGDIOBJ graphics_previous_bitmap;
    uint32_t *graphics_pixels;
    uint32_t graphics_width;
    uint32_t graphics_height;
    uint32_t graphics_palette[UX_GRAPHICS_PALETTE_ENTRIES];
    int graphics_valid;
    unsigned char presented_text[UX_TEXT_COLUMNS * UX_TEXT_ROWS];
    unsigned short presented_attributes[UX_TEXT_COLUMNS * UX_TEXT_ROWS];
    uint32_t presented_text_palette[16u];
    unsigned char presented_font[256u * 16u];
    unsigned char presented_secondary_font[256u * 16u];
    uint32_t presented_font_height;
    uint32_t presented_attribute_font_select;
    uint16_t presented_text_columns;
    uint16_t presented_text_rows;
    int presented_text_valid;
    uint32_t displayed_sequence;
    int result;
    ux_win32_keyboard_normalizer keyboard_normalizer;
    int left_button;
    int right_button;
    ux_win32_mouse mouse_state;
    WPARAM suppressed_hotkey;
    uint32_t surface_width;
    uint32_t surface_height;
    int client_width;
    int client_height;
    int cursor_blink_visible;
    DWORD cursor_blink_due;
} ux_win32_window_context;

/* This is dispatch-local, not presenter state. Every mutable field belongs to
 * the heap context below; one UI thread may drive one native window loop. */
static _Thread_local ux_win32_window_context *ux_win32_window_current;

#define win32_window_binding ux_win32_window_current->binding
#define win32_window_frame ux_win32_window_current->frame
#define win32_window_text_dc ux_win32_window_current->text_dc
#define win32_window_text_bitmap ux_win32_window_current->text_bitmap
#define win32_window_text_previous_bitmap ux_win32_window_current->text_previous_bitmap
#define win32_window_text_pixels ux_win32_window_current->text_pixels
#define win32_window_graphics_dc ux_win32_window_current->graphics_dc
#define win32_window_graphics_bitmap ux_win32_window_current->graphics_bitmap
#define win32_window_graphics_previous_bitmap ux_win32_window_current->graphics_previous_bitmap
#define win32_window_graphics_pixels ux_win32_window_current->graphics_pixels
#define win32_window_graphics_width ux_win32_window_current->graphics_width
#define win32_window_graphics_height ux_win32_window_current->graphics_height
#define win32_window_graphics_palette ux_win32_window_current->graphics_palette
#define win32_window_graphics_valid ux_win32_window_current->graphics_valid
#define win32_window_presented_text ux_win32_window_current->presented_text
#define win32_window_presented_attributes ux_win32_window_current->presented_attributes
#define win32_window_presented_text_palette ux_win32_window_current->presented_text_palette
#define win32_window_presented_font ux_win32_window_current->presented_font
#define win32_window_presented_secondary_font ux_win32_window_current->presented_secondary_font
#define win32_window_presented_font_height ux_win32_window_current->presented_font_height
#define win32_window_presented_attribute_font_select ux_win32_window_current->presented_attribute_font_select
#define win32_window_presented_text_columns ux_win32_window_current->presented_text_columns
#define win32_window_presented_text_rows ux_win32_window_current->presented_text_rows
#define win32_window_presented_text_valid ux_win32_window_current->presented_text_valid
#define win32_window_displayed_sequence ux_win32_window_current->displayed_sequence
#define win32_window_result ux_win32_window_current->result
#define win32_window_keyboard_normalizer ux_win32_window_current->keyboard_normalizer
#define win32_window_left_button ux_win32_window_current->left_button
#define win32_window_right_button ux_win32_window_current->right_button
#define win32_window_mouse_state ux_win32_window_current->mouse_state
#define win32_window_suppressed_hotkey ux_win32_window_current->suppressed_hotkey
#define win32_window_surface_width ux_win32_window_current->surface_width
#define win32_window_surface_height ux_win32_window_current->surface_height
#define win32_window_client_width ux_win32_window_current->client_width
#define win32_window_client_height ux_win32_window_current->client_height
#define win32_window_cursor_blink_visible ux_win32_window_current->cursor_blink_visible
#define win32_window_cursor_blink_due ux_win32_window_current->cursor_blink_due

static int win32_window_content_running(void)
{
    return win32_window_binding != NULL &&
        win32_window_binding->get_state(win32_window_binding->context) ==
        UX_RUN_RUNNING;
}

static void win32_window_update_title(HWND window)
{
    char title[128];

    if (window == NULL || win32_window_binding == NULL ||
        win32_window_binding->get_title == NULL) return;
    win32_window_binding->get_title(win32_window_binding->context, title,
        sizeof(title));
    SetWindowTextA(window, title);
}

static COLORREF win32_window_colour(unsigned int colour)
{
    static const COLORREF palette[16] = {
        RGB(0, 0, 0), RGB(0, 0, 170), RGB(0, 170, 0), RGB(0, 170, 170),
        RGB(170, 0, 0), RGB(170, 0, 170), RGB(170, 85, 0), RGB(170, 170, 170),
        RGB(85, 85, 85), RGB(85, 85, 255), RGB(85, 255, 85), RGB(85, 255, 255),
        RGB(255, 85, 85), RGB(255, 85, 255), RGB(255, 255, 85), RGB(255, 255, 255)
    };
    unsigned int index = colour & 0x0fu;

    /* Each copied frame supplies its active palette. The fallback is used only
       before a valid frame exists. */
    if (win32_window_frame != NULL && win32_window_frame->valid != 0u)
        return (COLORREF)win32_window_frame->text_palette[index];
    return palette[index];
}

/* A COLORREF is packed for Win32 colour APIs (0x00bbggrr), whereas the
 * standalone text DIB is a 32-bit BI_RGB surface (0x00rrggbb as a DWORD).
 * Convert only at this final GDI storage boundary. */
static uint32_t win32_window_dib_pixel(COLORREF colour)
{
    return ux_win32_dib_pixel(colour);
}

/* The frontend has no independent canvas size. By default its client area is
 * the copied content surface; after a user resize, the final GDI blit follows
 * that client area with no unused letterbox space. */
static int win32_window_display_rect(HWND window, uint32_t source_width,
    uint32_t source_height, RECT *display)
{
    if (window == NULL) return 0;
    return ux_win32_display_rect(win32_window_client_width,
        win32_window_client_height, source_width, source_height, display);
}

static void win32_window_capture_client_size(HWND window)
{
    RECT client;

    if (window == NULL) return;
    GetClientRect(window, &client);
    win32_window_client_width = client.right - client.left;
    win32_window_client_height = client.bottom - client.top;
}

static void win32_window_resize_surface(HWND window, uint32_t width,
    uint32_t height)
{
    if (window == NULL || width == 0u || height == 0u ||
        (win32_window_surface_width == width &&
         win32_window_surface_height == height)) return;
    /* Preserve the user's desktop position across content-size changes. */
    if (!ux_win32_resize_client(window, width, height)) return;
    win32_window_surface_width = width;
    win32_window_surface_height = height;
}

static void win32_window_resize_frame(HWND window)
{
    if (win32_window_frame == NULL || win32_window_frame->valid == 0u)
        return;
    if (win32_window_frame->graphics != 0u)
        win32_window_resize_surface(window, win32_window_frame->graphics_width,
            win32_window_frame->graphics_height);
    else if (win32_window_frame->text_columns != 0u &&
        win32_window_frame->text_rows != 0u &&
        win32_window_frame->text_columns <= UX_TEXT_COLUMNS &&
        win32_window_frame->text_rows <= UX_TEXT_ROWS)
        win32_window_resize_surface(window,
            win32_window_frame->text_columns * WIN32_WINDOW_TEXT_CELL_WIDTH,
            win32_window_frame->text_rows * WIN32_WINDOW_TEXT_CELL_HEIGHT);
}

static void win32_window_current_surface_size(uint32_t *width,
    uint32_t *height)
{
    if (width == NULL || height == NULL) return;
    if (win32_window_frame != NULL && win32_window_frame->valid != 0u &&
        win32_window_frame->graphics != 0u) {
        *width = win32_window_frame->graphics_width;
        *height = win32_window_frame->graphics_height;
    } else if (win32_window_frame != NULL &&
        win32_window_frame->text_columns != 0u &&
        win32_window_frame->text_rows != 0u) {
        *width = win32_window_frame->text_columns * WIN32_WINDOW_TEXT_CELL_WIDTH;
        *height = win32_window_frame->text_rows * WIN32_WINDOW_TEXT_CELL_HEIGHT;
    } else {
        *width = WIN32_WINDOW_TEXT_SURFACE_WIDTH;
        *height = WIN32_WINDOW_TEXT_SURFACE_HEIGHT;
    }
}

static void win32_window_constrain_sizing(HWND window, WPARAM edge,
    RECT *outer)
{
    uint32_t source_width;
    uint32_t source_height;

    if (window == NULL || outer == NULL) return;
    win32_window_current_surface_size(&source_width, &source_height);
    ux_win32_constrain_sizing(window, edge, outer, source_width,
        source_height);
}

static void win32_window_update_text_surface(void)
{
    int row;
    if (win32_window_frame == NULL || win32_window_text_dc == NULL ||
        win32_window_frame->text_columns == 0u ||
        win32_window_frame->text_rows == 0u ||
        win32_window_frame->text_columns > UX_TEXT_COLUMNS ||
        win32_window_frame->text_rows > UX_TEXT_ROWS ||
        (win32_window_presented_text_valid && memcmp(win32_window_presented_text,
            win32_window_frame->text, sizeof(win32_window_presented_text)) == 0 &&
         memcmp(win32_window_presented_attributes,
            win32_window_frame->attributes,
            sizeof(win32_window_presented_attributes)) == 0 &&
         memcmp(win32_window_presented_text_palette,
            win32_window_frame->text_palette,
            sizeof(win32_window_presented_text_palette)) == 0 &&
         memcmp(win32_window_presented_font, win32_window_frame->font,
            sizeof(win32_window_presented_font)) == 0 &&
         memcmp(win32_window_presented_secondary_font,
            win32_window_frame->secondary_font,
            sizeof(win32_window_presented_secondary_font)) == 0 &&
         win32_window_presented_font_height ==
            win32_window_frame->font_height &&
         win32_window_presented_attribute_font_select ==
            win32_window_frame->attribute_font_select &&
         win32_window_presented_text_columns ==
            win32_window_frame->text_columns &&
         win32_window_presented_text_rows ==
            win32_window_frame->text_rows)) return;
    memset(win32_window_text_pixels, 0,
        WIN32_WINDOW_TEXT_SURFACE_WIDTH * WIN32_WINDOW_TEXT_SURFACE_HEIGHT *
        sizeof(*win32_window_text_pixels));
    for (row = 0; row < (int)win32_window_frame->text_rows; ++row) {
        int column;
        for (column = 0; column < (int)win32_window_frame->text_columns; ++column) {
            unsigned int scan;
            size_t index = (size_t)row * UX_TEXT_COLUMNS + column;
            unsigned char character = win32_window_frame->text[index];
            unsigned short attribute = win32_window_frame->attributes[index];
            for (scan = 0u; scan < WIN32_WINDOW_TEXT_CELL_HEIGHT; ++scan) {
                const unsigned char *font =
                    win32_window_frame->attribute_font_select != 0u &&
                    (attribute & 0x08u) != 0u ?
                    win32_window_frame->secondary_font : win32_window_frame->font;
                unsigned char bits = font[
                    (size_t)character * 16u + scan];
                unsigned int bit;
                uint32_t *pixels = win32_window_text_pixels +
                    ((size_t)row * WIN32_WINDOW_TEXT_CELL_HEIGHT + scan) *
                    WIN32_WINDOW_TEXT_SURFACE_WIDTH + column * WIN32_WINDOW_TEXT_CELL_WIDTH;
                for (bit = 0u; bit < WIN32_WINDOW_TEXT_CELL_WIDTH; ++bit)
                    pixels[bit] = win32_window_dib_pixel(
                        (bits & (0x80u >> bit)) ?
                        win32_window_colour(attribute) :
                        win32_window_colour(attribute >> 4));
            }
        }
    }
    memcpy(win32_window_presented_text, win32_window_frame->text,
        sizeof(win32_window_presented_text));
    memcpy(win32_window_presented_attributes, win32_window_frame->attributes,
        sizeof(win32_window_presented_attributes));
    memcpy(win32_window_presented_text_palette,
        win32_window_frame->text_palette,
        sizeof(win32_window_presented_text_palette));
    memcpy(win32_window_presented_font, win32_window_frame->font,
        sizeof(win32_window_presented_font));
    memcpy(win32_window_presented_secondary_font,
        win32_window_frame->secondary_font,
        sizeof(win32_window_presented_secondary_font));
    win32_window_presented_font_height = win32_window_frame->font_height;
    win32_window_presented_attribute_font_select =
        win32_window_frame->attribute_font_select;
    win32_window_presented_text_columns = win32_window_frame->text_columns;
    win32_window_presented_text_rows = win32_window_frame->text_rows;
    win32_window_presented_text_valid = 1;
}

/* Some Win32/RDP paths cannot blit an indexed, top-down DIB directly even
 * when its copied bytes are valid. Convert only at this final outlet to RGB32,
 * the same isolated presentation boundary used by text. */
static int win32_window_update_graphics_surface(RECT *changed)
{
    uint32_t source_stride;
    uint32_t row;
    int full_refresh;
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;

    if (win32_window_frame == NULL || win32_window_graphics_pixels == NULL ||
        win32_window_frame->graphics == 0u ||
        win32_window_frame->graphics_width == 0u ||
        win32_window_frame->graphics_height == 0u ||
        win32_window_frame->graphics_width > WIN32_WINDOW_GRAPHICS_SURFACE_MAX_WIDTH ||
        win32_window_frame->graphics_height > WIN32_WINDOW_GRAPHICS_SURFACE_MAX_HEIGHT ||
        changed == NULL)
        return 0;
    full_refresh = !win32_window_graphics_valid ||
        win32_window_graphics_width != win32_window_frame->graphics_width ||
        win32_window_graphics_height != win32_window_frame->graphics_height ||
        memcmp(win32_window_graphics_palette, win32_window_frame->graphics_palette,
            sizeof(win32_window_graphics_palette)) != 0;
    left = full_refresh ? 0 : win32_window_frame->dirty_left;
    top = full_refresh ? 0 : win32_window_frame->dirty_top;
    right = full_refresh ? (int32_t)win32_window_frame->graphics_width - 1 :
        win32_window_frame->dirty_right;
    bottom = full_refresh ? (int32_t)win32_window_frame->graphics_height - 1 :
        win32_window_frame->dirty_bottom;
    if (left < 0) left = 0;
    if (top < 0) top = 0;
    if (right >= (int32_t)win32_window_frame->graphics_width)
        right = (int32_t)win32_window_frame->graphics_width - 1;
    if (bottom >= (int32_t)win32_window_frame->graphics_height)
        bottom = (int32_t)win32_window_frame->graphics_height - 1;
    if (right < left || bottom < top) return 0;
    source_stride = win32_window_frame->graphics_stride;
    if (source_stride < win32_window_frame->graphics_width ||
        source_stride > UX_GRAPHICS_MAX_WIDTH) return 0;
    for (row = (uint32_t)top; row <= (uint32_t)bottom; ++row) {
        const uint8_t *source = win32_window_frame->graphics_pixels +
            row * source_stride;
        uint32_t *destination = win32_window_graphics_pixels +
            row * WIN32_WINDOW_GRAPHICS_SURFACE_MAX_WIDTH;
        uint32_t column;
        for (column = (uint32_t)left; column <= (uint32_t)right; ++column) {
            destination[column] = win32_window_frame->graphics_palette[
                source[column]];
        }
    }
    win32_window_graphics_width = win32_window_frame->graphics_width;
    win32_window_graphics_height = win32_window_frame->graphics_height;
    memcpy(win32_window_graphics_palette, win32_window_frame->graphics_palette,
        sizeof(win32_window_graphics_palette));
    win32_window_graphics_valid = 1;
    changed->left = left;
    changed->top = top;
    changed->right = right + 1;
    changed->bottom = bottom + 1;
    return 1;
}

static void win32_window_invalidate_graphics(HWND window, const RECT *source)
{
    RECT display;
    RECT target;
    uint32_t width;
    uint32_t height;

    if (window == NULL || source == NULL || win32_window_frame == NULL ||
        !win32_window_display_rect(window, win32_window_frame->graphics_width,
            win32_window_frame->graphics_height, &display)) return;
    width = win32_window_frame->graphics_width;
    height = win32_window_frame->graphics_height;
    ux_win32_map_dirty_rect(source, &display, width, height,
        &target);
    InvalidateRect(window, &target, FALSE);
}

/* Cursor blink is a host overlay. It only invalidates the copied-frame view
 * between otherwise unchanged frames. */
static int win32_window_cursor_rect(HWND window, RECT *cursor)
{
    RECT display;
    int width;
    int height;
    int cell_height;
    int cursor_height;
    uint32_t cursor_percent;

    if (window == NULL || cursor == NULL || win32_window_frame == NULL ||
        win32_window_frame->valid == 0u ||
        win32_window_frame->graphics != 0u ||
        win32_window_frame->cursor_visible == 0u ||
        win32_window_frame->cursor_phase == 0u ||
        win32_window_frame->cursor_column < 0 ||
        win32_window_frame->cursor_row < 0 ||
        win32_window_frame->text_columns == 0u ||
        win32_window_frame->text_rows == 0u ||
        win32_window_frame->cursor_column >=
            (lib_i32)win32_window_frame->text_columns ||
        win32_window_frame->cursor_row >=
            (lib_i32)win32_window_frame->text_rows) return 0;
    if (!win32_window_display_rect(window,
            win32_window_frame->text_columns * WIN32_WINDOW_TEXT_CELL_WIDTH,
            win32_window_frame->text_rows * WIN32_WINDOW_TEXT_CELL_HEIGHT,
            &display)) return 0;
    width = display.right - display.left;
    height = display.bottom - display.top;
    cell_height = height / win32_window_frame->text_rows;
    if (width <= 0 || cell_height <= 0) return 0;
    cursor_percent = win32_window_frame->cursor_bottom >=
        win32_window_frame->cursor_top && win32_window_frame->font_height != 0u ?
        (win32_window_frame->cursor_bottom - win32_window_frame->cursor_top + 1u) *
            100u / win32_window_frame->font_height : 100u;
    if (cursor_percent == 0u || cursor_percent > 100u) cursor_percent = 100u;
    cursor_height = (int)((cell_height * cursor_percent + 99u) / 100u);
    if (cursor_height > cell_height) cursor_height = cell_height;
    cursor->left = display.left + win32_window_frame->cursor_column * width /
        win32_window_frame->text_columns;
    cursor->right = display.left + (win32_window_frame->cursor_column + 1) * width /
        win32_window_frame->text_columns;
    cursor->top = display.top + (win32_window_frame->cursor_row + 1) * cell_height -
        cursor_height;
    cursor->bottom = display.top + (win32_window_frame->cursor_row + 1) * height /
        win32_window_frame->text_rows;
    return cursor->right > cursor->left && cursor->bottom > cursor->top;
}

static void win32_window_advance_cursor_blink(HWND window)
{
    RECT cursor;
    DWORD now = GetTickCount();

    /* The text cursor is a host presentation overlay.  Once the executor is
       paused, preserve the last composited frame exactly: do not let this
       host-only blink timer alter an otherwise frozen display. */
    if (!win32_window_content_running()) return;
    if ((LONG)(now - win32_window_cursor_blink_due) < 0) return;
    win32_window_cursor_blink_visible = !win32_window_cursor_blink_visible;
    win32_window_cursor_blink_due = now + WIN32_WINDOW_CURSOR_BLINK_INTERVAL_MS;
    if (win32_window_cursor_rect(window, &cursor))
        InvalidateRect(window, &cursor, FALSE);
}

static void win32_window_paint(HWND window, HDC dc)
{
    RECT display;

    if (win32_window_frame == NULL || win32_window_frame->valid == 0u) return;
    if (win32_window_frame->graphics != 0u) {
        if (!win32_window_display_rect(window,
                win32_window_frame->graphics_width,
                win32_window_frame->graphics_height, &display)) return;
        if (win32_window_graphics_dc == NULL ||
            win32_window_graphics_width != win32_window_frame->graphics_width ||
            win32_window_graphics_height != win32_window_frame->graphics_height)
            return;
        StretchBlt(dc, display.left, display.top,
            display.right - display.left, display.bottom - display.top,
            win32_window_graphics_dc, 0, 0,
            (int)win32_window_graphics_width,
            (int)win32_window_graphics_height, SRCCOPY);
        return;
    }
    win32_window_update_text_surface();
    if (!win32_window_display_rect(window,
            win32_window_frame->text_columns * WIN32_WINDOW_TEXT_CELL_WIDTH,
            win32_window_frame->text_rows * WIN32_WINDOW_TEXT_CELL_HEIGHT,
            &display)) return;
    StretchBlt(dc, display.left, display.top, display.right - display.left,
        display.bottom - display.top, win32_window_text_dc, 0, 0,
        win32_window_frame->text_columns * WIN32_WINDOW_TEXT_CELL_WIDTH,
        win32_window_frame->text_rows * WIN32_WINDOW_TEXT_CELL_HEIGHT, SRCCOPY);
    if (win32_window_cursor_blink_visible) {
        RECT cursor;
        /* Draw the copied text cursor only after the text DIB reaches the
           window; this remains a pure frontend overlay. */
        if (win32_window_cursor_rect(window, &cursor))
            InvertRect(dc, &cursor);
    }
}

static void win32_window_transition(WPARAM key, LPARAM lparam, int released)
{
    WORD scan = (WORD)((lparam >> 16) & 0xffu);
    DWORD control_state = (lparam & 0x01000000L) != 0 ? ENHANCED_KEY : 0u;
    if (scan == 0u && !released)
        ux_win32_keyboard_note_recovered_key(
            &win32_window_keyboard_normalizer, (WORD)key);
    if (scan == 0u && released)
        ux_win32_keyboard_release_recovered_key(
            &win32_window_keyboard_normalizer, (WORD)key);
    (void)ux_win32_keyboard_submit_transition(
        win32_window_binding->context, win32_window_binding->input_sink,
        scan, (WORD)key, control_state,
        !released);
}

static void win32_window_mouse(LPARAM position)
{
    int dx = 0, dy = 0;
    uint32_t content_width;
    uint32_t content_height;

    if (!win32_window_content_running()) return;
    /* WM_MOUSE reports pixels in the current client area. Normalize them to
       the copied content surface before reporting relative movement. */
    win32_window_current_surface_size(&content_width, &content_height);
    if (!ux_win32_mouse_move(&win32_window_mouse_state, position,
            win32_window_client_width, win32_window_client_height, content_width,
            content_height, &dx, &dy)) return;
    {
        ux_event event = { 0 };
        event.type = UX_EVENT_MOUSE;
        event.data.mouse.delta_x = dx;
        event.data.mouse.delta_y = dy;
        event.data.mouse.relative = 1u;
        event.data.mouse.buttons =
            (win32_window_left_button ? UX_MOUSE_BUTTON_LEFT : 0u) |
            (win32_window_right_button ? UX_MOUSE_BUTTON_RIGHT : 0u);
        (void)win32_window_binding->input_sink(win32_window_binding->context,
            &event);
    }
}

/* Once the user explicitly clicks the content surface, keep the host pointer
 * there so it cannot accidentally operate the desktop while relative deltas
 * are delivered. An explicit product release action or focus loss releases
 * this frontend capture. */
static void win32_window_release_mouse_capture(void)
{
    ux_win32_mouse_release(&win32_window_mouse_state);
}

static void win32_window_capture_mouse(HWND window, LPARAM position)
{
    if (window == NULL || !win32_window_content_running()) return;
    (void)ux_win32_mouse_capture(&win32_window_mouse_state, window,
        position);
    /* Returning NULL from WM_SETCURSOR keeps the desktop arrow out of the
       content surface without changing product input state. */
    SetCursor(NULL);
}

static LRESULT CALLBACK win32_window_proc(HWND window, UINT message,
    WPARAM wparam, LPARAM lparam)
{
    switch (message) {
    case WIN32_WINDOW_FRAME_READY:
        {
            if (ux_mailbox_generation(win32_window_binding->mailbox) !=
                    win32_window_displayed_sequence &&
                ux_mailbox_capture(win32_window_binding->mailbox,
                    win32_window_frame) == LIB_STATUS_OK) {
                win32_window_displayed_sequence = win32_window_frame->sequence;
                if (ux_router_target(win32_window_binding->router) ==
                    UX_TARGET_CONSOLE) {
                    win32_window_result = UX_RUN_SWITCH_CONSOLE;
                    DestroyWindow(window);
                    return 0;
                }
                if (win32_window_frame->graphics != 0u) {
                    RECT changed;
                    int graphics_changed = win32_window_update_graphics_surface(
                        &changed);
                    win32_window_resize_frame(window);
                    if (graphics_changed)
                        win32_window_invalidate_graphics(window, &changed);
                }
                if (win32_window_frame->graphics == 0u) {
                    win32_window_resize_frame(window);
                    InvalidateRect(window, NULL, FALSE);
                }
            }
            win32_window_advance_cursor_blink(window);
            win32_window_update_title(window);
            if (win32_window_binding->get_state(win32_window_binding->context) ==
                UX_RUN_STOPPED ||
                win32_window_binding->get_state(win32_window_binding->context) ==
                UX_RUN_ERROR) DestroyWindow(window);
        }
        return 0;
    case WM_PAINT:
        { PAINTSTRUCT paint; HDC dc = BeginPaint(window, &paint);
          win32_window_paint(window, dc); EndPaint(window, &paint); }
        return 0;
    case WM_SIZE:
        win32_window_capture_client_size(window);
        return 0;
    case WM_ERASEBKGND:
        return 1;
    case WM_NCLBUTTONDBLCLK:
        if (wparam == HTCAPTION) {
            if (IsZoomed(window)) ShowWindow(window, SW_RESTORE);
            /* A user resize deliberately does not alter the content-surface
               cache. Double-click restores the current copied size. */
            win32_window_surface_width = 0u;
            win32_window_surface_height = 0u;
            win32_window_resize_frame(window);
            return 0;
        }
        break;
    case WM_SIZING:
        win32_window_constrain_sizing(window, wparam, (RECT *)lparam);
        return TRUE;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN: {
        lib_u8 modifiers = ux_win32_modifiers_from_key_state();
        ux_action action = ux_actions_match(
            win32_window_binding->actions, (WORD)wparam,
            modifiers);
        if (action != UX_ACTION_NONE) {
            ux_run_result action_result;
            win32_window_release_mouse_capture();
            /* Registered actions always release the host capture before the
               product callback, regardless of what that callback does. */
            action_result = ux_binding_invoke_action(win32_window_binding,
                action, modifiers);
            if (action_result != UX_RUN_CONTINUE)
                win32_window_result = action_result;
            win32_window_update_title(window);
            win32_window_suppressed_hotkey = wparam;
        } else if (win32_window_content_running())
            win32_window_transition(wparam, lparam, 0);
        return 0;
    }
    case WM_KEYUP:
    case WM_SYSKEYUP:
        if (wparam == win32_window_suppressed_hotkey) {
            win32_window_suppressed_hotkey = 0u;
            return 0;
        }
        if (win32_window_content_running())
            win32_window_transition(wparam, lparam, 1);
        return 0;
    case WM_CHAR:
        /* A physical WM_KEYDOWN has already been delivered.  A scan-less
           RDP text packet is normalized only when it is not that recovered
           physical key's duplicate character. */
        if (win32_window_content_running() &&
            ((uint32_t)lparam >> 16u & 0xffu) == 0u &&
            !ux_win32_keyboard_consume_duplicate_character(
                &win32_window_keyboard_normalizer, (WORD)wparam))
            (void)ux_win32_keyboard_submit_utf16(
                &win32_window_keyboard_normalizer, win32_window_binding->context,
                win32_window_binding->input_sink, (WORD)wparam);
        return 0;
    case WM_MOUSEMOVE:
        if (win32_window_content_running() &&
            ux_win32_mouse_captured(&win32_window_mouse_state))
            win32_window_mouse(lparam);
        return 0;
    case WM_SETCURSOR:
        if (ux_win32_mouse_hides_host_cursor(
                &win32_window_mouse_state) && LOWORD(lparam) == HTCLIENT) {
            SetCursor(NULL);
            return TRUE;
        }
        break;
    case WM_LBUTTONDOWN:
        if (!win32_window_content_running()) return 0;
        win32_window_left_button = 1;
        win32_window_capture_mouse(window, lparam);
        win32_window_mouse(lparam);
        return 0;
    case WM_LBUTTONUP:
        if (!win32_window_content_running()) return 0;
        win32_window_left_button = 0;
        if (ux_win32_mouse_captured(&win32_window_mouse_state))
            win32_window_mouse(lparam);
        return 0;
    case WM_RBUTTONDOWN:
        if (!win32_window_content_running()) return 0;
        win32_window_right_button = 1;
        win32_window_capture_mouse(window, lparam);
        win32_window_mouse(lparam);
        return 0;
    case WM_RBUTTONUP:
        if (!win32_window_content_running()) return 0;
        win32_window_right_button = 0;
        if (ux_win32_mouse_captured(&win32_window_mouse_state))
            win32_window_mouse(lparam);
        return 0;
    case WM_KILLFOCUS:
        win32_window_release_mouse_capture();
        return 0;
    case WM_CLOSE:
        /* The product owns the close decision.  Retain this loop and its last
           copied frame whenever that callback keeps the session resumable. */
        win32_window_release_mouse_capture();
        win32_window_result = win32_window_binding->handle_close(
            win32_window_binding->context,
            win32_window_binding->input_sink);
        return 0;
    case WM_DESTROY:
        win32_window_release_mouse_capture();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(window, message, wparam, lparam);
}

ux_run_result ux_win32_run_window(
    const ux_binding *binding)
{
    WNDCLASSA klass;
    MSG message;
    HWND window;
    HDC dc;
    ux_win32_window_context *context;

    if (ux_binding_validate(binding) != LIB_STATUS_OK)
        return UX_RUN_ERROR_RESULT;
    ZeroMemory(&klass, sizeof(klass));
    klass.lpfnWndProc = win32_window_proc;
    klass.hInstance = GetModuleHandleA(NULL);
    klass.hCursor = LoadCursorA(NULL, IDC_ARROW);
    klass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    klass.lpszClassName = "Win32PresentationWindow";
    if (RegisterClassA(&klass) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        return UX_RUN_ERROR_RESULT;
    context = (ux_win32_window_context *)calloc(1u, sizeof(*context));
    if (context == NULL) return UX_RUN_ERROR_RESULT;
    ux_win32_window_current = context;
    win32_window_frame = (ux_frame *)calloc(1u, sizeof(*win32_window_frame));
    if (win32_window_frame == NULL) {
        free(context);
        ux_win32_window_current = NULL;
        return UX_RUN_ERROR_RESULT;
    }
    win32_window_binding = binding;
    win32_window_result = UX_RUN_STOPPED_RESULT;
    win32_window_presented_text_valid = 0;
    win32_window_graphics_valid = 0;
    ZeroMemory(win32_window_graphics_palette,
        sizeof(win32_window_graphics_palette));
    win32_window_displayed_sequence = 0u;
    ZeroMemory(&win32_window_keyboard_normalizer,
        sizeof(win32_window_keyboard_normalizer));
    win32_window_left_button = win32_window_right_button = 0;
    ux_win32_mouse_reset(&win32_window_mouse_state);
    win32_window_surface_width = 0u;
    win32_window_surface_height = 0u;
    win32_window_client_width = 0;
    win32_window_client_height = 0;
    win32_window_cursor_blink_visible = 1;
    win32_window_cursor_blink_due = GetTickCount() +
        WIN32_WINDOW_CURSOR_BLINK_INTERVAL_MS;
    /* Begin at the standard 80x25 text client dimensions. */
    {
        char title[128] = "Presentation";
        if (binding->get_title != NULL)
            binding->get_title(binding->context, title, sizeof(title));
        window = CreateWindowExA(0, klass.lpszClassName, title,
        WS_THICKFRAME | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU |
        WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
        CW_USEDEFAULT, 0, 680, 560, NULL, NULL, klass.hInstance, NULL);
    }
    if (window == NULL) {
        free(win32_window_frame);
        free(context);
        ux_win32_window_current = NULL;
        return UX_RUN_ERROR_RESULT;
    }
    win32_window_resize_surface(window, WIN32_WINDOW_TEXT_SURFACE_WIDTH,
        WIN32_WINDOW_TEXT_SURFACE_HEIGHT);
    win32_window_capture_client_size(window);
    dc = GetDC(window);
    win32_window_text_dc = CreateCompatibleDC(dc);
    win32_window_graphics_dc = CreateCompatibleDC(dc);
    {
        BITMAPINFO info;
        ZeroMemory(&info, sizeof(info));
        info.bmiHeader.biSize = sizeof(info.bmiHeader);
        info.bmiHeader.biWidth = WIN32_WINDOW_TEXT_SURFACE_WIDTH;
        info.bmiHeader.biHeight = -WIN32_WINDOW_TEXT_SURFACE_HEIGHT;
        info.bmiHeader.biPlanes = 1;
        info.bmiHeader.biBitCount = 32;
        info.bmiHeader.biCompression = BI_RGB;
        win32_window_text_bitmap = CreateDIBSection(dc, &info, DIB_RGB_COLORS,
            (void **)&win32_window_text_pixels, NULL, 0u);
    }
    {
        BITMAPINFO info;
        ZeroMemory(&info, sizeof(info));
        info.bmiHeader.biSize = sizeof(info.bmiHeader);
        info.bmiHeader.biWidth = WIN32_WINDOW_GRAPHICS_SURFACE_MAX_WIDTH;
        info.bmiHeader.biHeight = -(LONG)WIN32_WINDOW_GRAPHICS_SURFACE_MAX_HEIGHT;
        info.bmiHeader.biPlanes = 1;
        info.bmiHeader.biBitCount = 32;
        info.bmiHeader.biCompression = BI_RGB;
        win32_window_graphics_bitmap = CreateDIBSection(dc, &info,
            DIB_RGB_COLORS, (void **)&win32_window_graphics_pixels, NULL, 0u);
    }
    ReleaseDC(window, dc);
    if (win32_window_text_dc == NULL || win32_window_text_bitmap == NULL ||
        win32_window_graphics_dc == NULL || win32_window_graphics_bitmap == NULL) {
        DestroyWindow(window);
        free(win32_window_frame);
        free(context);
        ux_win32_window_current = NULL;
        return UX_RUN_ERROR_RESULT;
    }
    win32_window_text_previous_bitmap = SelectObject(win32_window_text_dc,
        win32_window_text_bitmap);
    win32_window_graphics_previous_bitmap = SelectObject(win32_window_graphics_dc,
        win32_window_graphics_bitmap);
    ShowWindow(window, SW_SHOW);
    UpdateWindow(window);
    SetForegroundWindow(window);
    SetFocus(window);
    for (;;) {
        HANDLE wait_handle = ux_win32_mailbox_wait_handle(binding->mailbox);
        DWORD wait = MsgWaitForMultipleObjects(1u, &wait_handle, FALSE,
            INFINITE, QS_ALLINPUT);

        if (wait == WAIT_OBJECT_0)
            SendMessageA(window, WIN32_WINDOW_FRAME_READY, 0, 0);
        else if (wait == WAIT_FAILED) {
            win32_window_result = UX_RUN_ERROR_RESULT;
            DestroyWindow(window);
        }
        while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) goto window_loop_done;
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
    }
window_loop_done:
    SelectObject(win32_window_text_dc, win32_window_text_previous_bitmap);
    DeleteDC(win32_window_text_dc);
    win32_window_text_dc = NULL;
    DeleteObject(win32_window_text_bitmap);
    win32_window_text_bitmap = NULL;
    win32_window_text_pixels = NULL;
    SelectObject(win32_window_graphics_dc, win32_window_graphics_previous_bitmap);
    DeleteDC(win32_window_graphics_dc);
    win32_window_graphics_dc = NULL;
    DeleteObject(win32_window_graphics_bitmap);
    win32_window_graphics_bitmap = NULL;
    win32_window_graphics_pixels = NULL;
    {
        ux_run_result result = win32_window_result;
        free(win32_window_frame);
        free(context);
        ux_win32_window_current = NULL;
        return result;
    }
}
#endif
