/* Copyright 2012-2014 Neko. */

/* W32ADISP provides win32 window output interface. */


#include "type.h"
#include "core/platform/display_frame.h"
#include "core/platform/presentation_mailbox_interface.h"

#include "vm/platform/win32/win32app.h"
#include "vm/platform/win32/w32adisp.h"

#define COLOR_BLACK         0
#define COLOR_BLUE          RGB(0,0,255)
#define COLOR_GREEN         RGB(0,128,0)
#define COLOR_CYAN          RGB(0,255,255)
#define COLOR_GRAY          RGB(128,128,128)
#define COLOR_LIGHTBLUE     RGB(173,216,230)
#define COLOR_LIGHTGREEN    RGB(144,238,144)
#define COLOR_LIGHTCYAN     RGB(224,255,255)
#define COLOR_RED           RGB(255,0,0)
#define COLOR_MAGENTA       RGB(255,0,255)
#define COLOR_BROWN         RGB(184,134,11)
#define COLOR_LIGHTGRAY     RGB(190,190,190)
#define COLOR_LIGHTRED      RGB(255,99,71)
#define COLOR_LIGHTMAGENTA  RGB(255,102,183)
#define COLOR_YELLOW        RGB(255,255,0)
#define COLOR_WHITE         RGB(255,255,255)

#define FONT_WIDTH  8
#define FONT_HEIGHT 16
#define FONT_NCHAR  256
#define FONT_NCOLOR 256 /* 128 */
#define W32ADISP_FONT_BYTES (FONT_NCHAR * FONT_HEIGHT)

struct w32adisp_context {
    WIN32_HWND window;
    HDC window_dc;
    HDC buffer_dc;
    HBITMAP buffer_bitmap;
    INT client_height;
    INT client_width;
    INT flash_count;
    INT flash_interval;
    USHORT rows;
    USHORT columns;
    UCHAR cursor_top;
    UCHAR cursor_bottom;
    type_unsigned_64 displayed_generation;
    HDC font_dc;
    HBITMAP font_bitmap;
    UCHAR cached_glyphs[FONT_NCHAR][FONT_HEIGHT];
    BOOL font_character_exists[FONT_NCHAR][FONT_NCOLOR];
};

static COLORREF CharProp2Color(UCHAR prop, BOOL flagForeColor) {
    UCHAR byte;
    if (flagForeColor) {
        byte = (prop & 0x0f); /* fore color */
    } else {
        byte = ((prop & 0xf0) >> 4);  /* back color */
    }
    switch (byte) {
    case 0x00:
        return COLOR_BLACK;
        break;
    case 0x01:
        return COLOR_BLUE;
        break;
    case 0x02:
        return COLOR_GREEN;
        break;
    case 0x03:
        return COLOR_CYAN;
        break;
    case 0x04:
        return COLOR_RED;
        break;
    case 0x05:
        return COLOR_MAGENTA;
        break;
    case 0x06:
        return COLOR_BROWN;
        break;
    case 0x07:
        return COLOR_LIGHTGRAY;
        break;
    case 0x08:
        return COLOR_GRAY;
        break;
    case 0x09:
        return COLOR_LIGHTBLUE;
        break;
    case 0x0a:
        return COLOR_LIGHTGREEN;
        break;
    case 0x0b:
        return COLOR_LIGHTCYAN;
        break;
    case 0x0c:
        return COLOR_LIGHTRED;
        break;
    case 0x0d:
        return COLOR_LIGHTMAGENTA;
        break;
    case 0x0e:
        return COLOR_YELLOW;
        break;
    case 0x0f:
        return COLOR_WHITE;
        break;
    default:
        return COLOR_BLACK;
        break;
    }
}

static VOID CreateBitmapFontChar(w32adisp_context *context, UCHAR ch,
                                 UCHAR prop);

static VOID CreateBitmapFontChar(w32adisp_context *context, UCHAR ch,
                                 UCHAR prop) {
    UCHAR i, j;
    COLORREF fc, bc;
    HDC hdcChar;
    HBITMAP hBmpChar;
    HGDIOBJ hOldGdiObj;
    hdcChar = CreateCompatibleDC(STD_NULL);
    hBmpChar = CreateCompatibleBitmap(context->window_dc, FONT_WIDTH, FONT_HEIGHT);
    hOldGdiObj = SelectObject(hdcChar, hBmpChar);
    /* prop &= 0x7f; */
    fc = CharProp2Color(prop, TRUE);
    bc = CharProp2Color(prop, FALSE);
    for (i = 0; i < FONT_HEIGHT; ++i) {
        for (j = 0; j < FONT_WIDTH; ++j) {
            if (!!(context->cached_glyphs[ch][i] & (1 << j))) {
                SetPixel(hdcChar, j, i, fc);
            } else {
                SetPixel(hdcChar, j, i, bc);
            }
        }
    }
    BitBlt(context->font_dc, ch * FONT_WIDTH, prop * FONT_HEIGHT,
           FONT_WIDTH, FONT_HEIGHT, hdcChar, 0, 0, SRCCOPY);
    SelectObject(hdcChar, hOldGdiObj);
    DeleteObject(hBmpChar);
    DeleteDC(hdcChar);
    context->font_character_exists[ch][prop] = TRUE;
}

w32adisp_context *w32adisp_context_create(C_VOID) {
    return STD_CALLOC(1u, sizeof(w32adisp_context));
}

C_VOID w32adisp_context_destroy(w32adisp_context *context) {
    if (context == STD_NULL) return;
    STD_FREE(context);
}

type_unsigned_64 w32adisp_context_generation(const w32adisp_context *context) {
    return context == STD_NULL ? 0u : context->displayed_generation;
}

C_VOID w32adispInit(w32adisp_context *context, WIN32_HWND window,
                  const core_platform_presentation_mailbox *mailbox) {
    UINT i, j;
    if (context == STD_NULL) return;
    context->window = window;
    context->window_dc = GetDC(window);
    context->buffer_dc = CreateCompatibleDC(STD_NULL);
    context->buffer_bitmap = STD_NULL;
    context->client_height = 0;
    context->client_width  = 0;
    context->flash_count   = 0;
    context->flash_interval = 5;
    context->displayed_generation = 0u;
    context->font_dc = CreateCompatibleDC(STD_NULL);
    context->font_bitmap = CreateCompatibleBitmap(context->window_dc,
        FONT_WIDTH * FONT_NCHAR, FONT_HEIGHT * FONT_NCOLOR);
    SelectObject(context->font_dc, context->font_bitmap);
    for (i = 0; i < FONT_NCHAR; ++i) {
        for (j = 0; j < FONT_NCOLOR; ++j) {
            context->font_character_exists[i][j] = FALSE;
        }
    }
    w32adispSetScreen(context, window, mailbox);
}

C_VOID w32adispSetScreen(w32adisp_context *context, WIN32_HWND window,
                        const core_platform_presentation_mailbox *mailbox) {
    RECT clientRect,windowRect;
    LONG widthOffset, heightOffset;
    HBITMAP bufferBitmap;
    HGDIOBJ previousBitmap;
    core_platform_display_frame frame;

    if (context == STD_NULL || core_platform_presentation_mailbox_capture(mailbox,
            &frame) != TYPE_STATUS_OK) return;
    context->rows = frame.kind == CORE_PLATFORM_DISPLAY_KIND_INDEXED_PIXELS ?
        frame.pixel_width : frame.columns;
    context->columns = frame.kind == CORE_PLATFORM_DISPLAY_KIND_INDEXED_PIXELS ?
        frame.pixel_height : frame.rows;
    GetClientRect(window, &clientRect);
    GetWindowRect(window, &windowRect);

    /* fetch window and customer area size to decide window side */
    widthOffset = windowRect.right - windowRect.left - clientRect.right;
    heightOffset = windowRect.bottom - windowRect.top - clientRect.bottom;
    MoveWindow(window, windowRect.left, windowRect.top,
        (frame.kind == CORE_PLATFORM_DISPLAY_KIND_INDEXED_PIXELS ?
            frame.pixel_width : context->rows * FONT_WIDTH) + widthOffset,
        (frame.kind == CORE_PLATFORM_DISPLAY_KIND_INDEXED_PIXELS ?
            frame.pixel_height : context->columns * FONT_HEIGHT) + heightOffset,
        SWP_NOMOVE);
    GetClientRect(window, &clientRect);
    context->client_height = clientRect.bottom - clientRect.top;
    context->client_width  = clientRect.right - clientRect.left;
    bufferBitmap = CreateCompatibleBitmap(context->window_dc,
        GetDeviceCaps(context->window_dc, HORZRES), GetDeviceCaps(context->window_dc, VERTRES));
    if (bufferBitmap == STD_NULL) return;
    previousBitmap = SelectObject(context->buffer_dc, bufferBitmap);
    if (context->buffer_bitmap != STD_NULL && previousBitmap == context->buffer_bitmap) {
        DeleteObject(context->buffer_bitmap);
    }
    context->buffer_bitmap = bufferBitmap;
    w32adispPaint(context, window, mailbox, TRUE);
}

static VOID DisplayCursor(w32adisp_context *context,
                          const core_platform_display_frame *frame) {
    HBRUSH hBrush;
    HGDIOBJ hOldGdiObj;
    RECT rect;
    INT x1_cursor, y1_cursor, x2_cursor, y2_cursor;
    x1_cursor = x2_cursor = frame->cursor_y * FONT_HEIGHT; /* + FONT_HEIGHT / 2 */;
    context->cursor_top = frame->cursor_top;
    context->cursor_bottom = frame->cursor_bottom;
    x1_cursor += (context->cursor_top % 8) * FONT_HEIGHT / 8;
    x2_cursor += (context->cursor_bottom % 8) * FONT_HEIGHT / 8;
    y1_cursor = (frame->cursor_x + 0) * FONT_WIDTH;
    y2_cursor = (frame->cursor_x + 1) * FONT_WIDTH;
    rect.left = y1_cursor;
    rect.top = x1_cursor;
    rect.right = y2_cursor;
    rect.bottom = x2_cursor;
    hBrush = CreateSolidBrush(COLOR_LIGHTGRAY);
    hOldGdiObj = SelectObject(context->window_dc, hBrush);
    FillRect(context->window_dc, &rect, hBrush);
    SelectObject(context->window_dc, hOldGdiObj);
    DeleteObject(hBrush);
}

static C_VOID w32adisp_paint_indexed_pixels(w32adisp_context *context,
    const core_platform_display_frame *frame)
{
    struct {
        BITMAPINFOHEADER header;
        RGBQUAD palette[256];
    } information = {0};
    type_unsigned_16 index;

    if (context == STD_NULL || frame == STD_NULL ||
        frame->pixel_width == 0u || frame->pixel_height == 0u) return;
    information.header.biSize = sizeof(information.header);
    information.header.biWidth = frame->pixel_width;
    information.header.biHeight = -(LONG)frame->pixel_height;
    information.header.biPlanes = 1u;
    information.header.biBitCount = 8u;
    information.header.biCompression = BI_RGB;
    for (index = 0u; index < 256u; ++index) {
        const type_unsigned_32 rgb = frame->palette_rgb[index];
        information.palette[index].rgbRed = (BYTE)(rgb >> 16u);
        information.palette[index].rgbGreen = (BYTE)(rgb >> 8u);
        information.palette[index].rgbBlue = (BYTE)rgb;
    }
    (C_VOID)StretchDIBits(context->window_dc, 0, 0, context->client_width,
        context->client_height, 0, 0, frame->pixel_width, frame->pixel_height,
        frame->pixels, (const BITMAPINFO *)&information, DIB_RGB_COLORS, SRCCOPY);
}

C_VOID w32adispPaint(w32adisp_context *context, WIN32_HWND window,
                   const core_platform_presentation_mailbox *mailbox,
                   WIN32_BOOL flagForce) {
    UCHAR i, j, ch, prop;
    USHORT index;
    BOOL changed;
    core_platform_display_frame frame;

    (C_VOID)window;
    if (context == STD_NULL || core_platform_presentation_mailbox_capture(mailbox,
            &frame) != TYPE_STATUS_OK) return;
    context->flash_count = (context->flash_count + 1) % 10;
    changed = flagForce || frame.generation != context->displayed_generation;
    if (frame.kind == CORE_PLATFORM_DISPLAY_KIND_INDEXED_PIXELS) {
        if (changed) {
            w32adisp_paint_indexed_pixels(context, &frame);
            context->displayed_generation = frame.generation;
        }
        return;
    }
    if (changed) {
        if (!frame.text_glyphs_present) return;
        if (STD_MEMCMP(context->cached_glyphs, frame.text_glyphs,
                W32ADISP_FONT_BYTES) != 0) {
            STD_MEMCPY(context->cached_glyphs, frame.text_glyphs,
                W32ADISP_FONT_BYTES);
            STD_MEMSET(context->font_character_exists, 0,
                sizeof(context->font_character_exists));
        }
        for (i = 0; i < context->columns; ++i) {
            for (j = 0; j < context->rows; ++j) {
                index = i * CORE_PLATFORM_DISPLAY_MAX_COLUMNS + j;
                ch = frame.characters[index];
                prop = frame.attributes[index]; /* & 0x7f; */
                if (!context->font_character_exists[ch][prop]) {
                    CreateBitmapFontChar(context, ch, prop);
                }
                BitBlt(context->buffer_dc, j * FONT_WIDTH, i * FONT_HEIGHT,
                       FONT_WIDTH, FONT_HEIGHT, context->font_dc,
                       ch * FONT_WIDTH, prop * FONT_HEIGHT, SRCCOPY);
            }
        }
        context->displayed_generation = frame.generation;
    }
    BitBlt(context->window_dc, 0, 0, context->client_width,
           context->client_height, context->buffer_dc, 0, 0, SRCCOPY);
    if (frame.kind == CORE_PLATFORM_DISPLAY_KIND_TEXT && frame.cursor_visible &&
        ((context->flash_count % 10) < context->flash_interval)) {
        DisplayCursor(context, &frame);
    }
}

C_VOID w32adispFinal(w32adisp_context *context) {
    if (context == STD_NULL) return;
    DeleteDC(context->font_dc);
    DeleteObject(context->font_bitmap);
    DeleteDC(context->buffer_dc);
    DeleteObject(context->buffer_bitmap);
    if (context->window_dc != STD_NULL) {
        ReleaseDC(context->window, context->window_dc);
        context->window_dc = STD_NULL;
    }
}
