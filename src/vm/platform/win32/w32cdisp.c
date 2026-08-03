/* Copyright 2012-2014 Neko. */

/* W32CDISP provides win32 console output interface. */

#include "type.h"

#include "core/product/utils.h"




#include "core/platform/display_frame.h"

#include "vm/platform/presentation_mailbox.h"


#include "vm/platform/win32/win32con.h"

#include "vm/platform/win32/w32cdisp.h"

struct w32cdisp_context {
    PCHAR_INFO char_buffer;
    COORD buffer_size;
    COORD buffer_start;
    SMALL_RECT write_rect;
    USHORT rows;
    USHORT columns;
    CONSOLE_CURSOR_INFO default_cursor;
    UINT default_code_page;
    CONSOLE_SCREEN_BUFFER_INFO default_buffer;
    uint64_t displayed_generation;
};

w32cdisp_context *w32cdisp_context_create(C_VOID) {
    return STD_CALLOC(1u, sizeof(w32cdisp_context));
}

VOID w32cdisp_context_destroy(w32cdisp_context *context) {
    if (context == STD_NULL) return;
    STD_FREE(context->char_buffer);
    STD_FREE(context);
}

uint64_t w32cdisp_context_generation(const w32cdisp_context *context) {
    return context == STD_NULL ? 0u : context->displayed_generation;
}

VOID w32cdispInit(w32cdisp_context *context, HANDLE output,
                  const vm_platform_presentation_mailbox *mailbox) {
    if (context == STD_NULL) return;
    /* Cursor information is retained only for the owned output handle. */
    GetConsoleScreenBufferInfo(output, &context->default_buffer);
    GetConsoleCursorInfo(output, &context->default_cursor);
    context->default_code_page = GetConsoleCP();
    context->char_buffer = STD_NULL;
    context->displayed_generation = 0u;
    w32cdispSetScreen(context, output, mailbox);
}

VOID w32cdispSetScreen(w32cdisp_context *context, HANDLE output,
                       const vm_platform_presentation_mailbox *mailbox) {
    core_platform_display_frame frame;

    if (context == STD_NULL) return;
    vm_platform_presentation_mailbox_capture(mailbox, &frame);
    context->columns = frame.rows;
    context->rows = frame.columns;
    context->buffer_size.X = context->rows;
    context->buffer_size.Y = context->columns;
    context->buffer_start.X = 0;
    context->buffer_start.Y = 0;
    context->write_rect.Top = 0;
    context->write_rect.Bottom = context->columns - 1;
    context->write_rect.Left = 0;
    context->write_rect.Right = context->rows - 1;
    if (context->char_buffer) {
        STD_FREE(context->char_buffer);
    }
    context->char_buffer = STD_MALLOC(context->columns * context->rows * sizeof(CHAR_INFO));
    /* Legacy cursor-shape control remains intentionally disabled. */
    SetConsoleOutputCP(437);
    SetConsoleScreenBufferSize(output, context->buffer_size);
}

VOID w32cdispPaint(w32cdisp_context *context, HANDLE output,
                   const vm_platform_presentation_mailbox *mailbox,
                   BOOL flagForce) {
    core_platform_display_frame frame;
    UCHAR ansiChar;
    WCHAR unicodeChar;
    WORD  charProp;
    UCHAR i, j;
    COORD curPos;
    CONSOLE_CURSOR_INFO curInfo;
    BOOL changed;
    vm_platform_presentation_mailbox_capture(mailbox, &frame);
    if (context == STD_NULL || !context->char_buffer) {
        return;
    }
    changed = flagForce || frame.generation != context->displayed_generation;
    if (changed) {
        for (i = 0; i < context->columns; ++i) {
            for (j = 0; j < context->rows; ++j) {
                USHORT index = i * CORE_PLATFORM_DISPLAY_MAX_COLUMNS + j;
                ansiChar = frame.characters[index];
                charProp = frame.attributes[index]; /* & 0x7f; */
                /* if (!ansiChar) continue; */
                MultiByteToWideChar(437, 0, (LPCSTR)(&ansiChar), 1, (LPWSTR)(&unicodeChar), 1);
                context->char_buffer[i * context->rows + j].Char.UnicodeChar = unicodeChar;
                context->char_buffer[i * context->rows + j].Attributes = charProp;
            }
        }
        WriteConsoleOutput(output, context->char_buffer, context->buffer_size,
                           context->buffer_start, &context->write_rect);
        context->displayed_generation = frame.generation;
    }
    if (changed) {
        GetConsoleCursorInfo(output, (PCONSOLE_CURSOR_INFO)(&curInfo));
        curInfo.bVisible = frame.cursor_visible;
        curInfo.dwSize = (DWORD)(((frame.cursor_bottom - frame.cursor_top) % 8 + 1) * 100. / 8.);
        SetConsoleCursorInfo(output, &curInfo);
        curPos.X = frame.cursor_y;
        curPos.Y = frame.cursor_x;
        SetConsoleCursorPosition(output, curPos);
    }
}

VOID w32cdispFinal(w32cdisp_context *context, HANDLE output) {
    if (context == STD_NULL) return;
    if (context->char_buffer) {
        STD_FREE(context->char_buffer);
    }
    context->char_buffer = STD_NULL;
    SetConsoleCursorInfo(output, &context->default_cursor);
    SetConsoleOutputCP(context->default_code_page);
    SetConsoleScreenBufferSize(output, context->default_buffer.dwSize);
}
