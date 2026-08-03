/* Copyright 2012-2014 Neko. */

/* W32CDISP provides win32 console output interface. */

#include "core/product/utils.h"

#include <stdlib.h>

#include "core/platform/display_frame.h"
#include "vm/platform/presentation_mailbox.h"

#include "vm/platform/win32/win32con.h"
#include "vm/platform/win32/w32cdisp.h"

static PCHAR_INFO charBuf;
static COORD coordDefaultBufSize, coordBufSize, coordBufStart;
static SMALL_RECT srctWriteRect;
static USHORT sizeRow, sizeCol;
static CONSOLE_CURSOR_INFO defaultCurInfo;
static UINT defaultCodePage;
static CONSOLE_SCREEN_BUFFER_INFO defaultBufInfo;
static UCHAR bufComp[0x1000];
static uint64_t displayedGeneration;

VOID w32cdispInit(HANDLE output,
                  const vm_platform_presentation_mailbox *mailbox) {
    /* Cursor information is retained only for the owned output handle. */
    GetConsoleScreenBufferInfo(output, &defaultBufInfo);
    defaultCodePage = GetConsoleCP();
    charBuf = NULL;
    w32cdispSetScreen(output, mailbox);
}

VOID w32cdispSetScreen(HANDLE output,
                       const vm_platform_presentation_mailbox *mailbox) {
    core_platform_display_frame frame;

    vm_platform_presentation_mailbox_capture(mailbox, &frame);
    sizeCol = frame.rows;
    sizeRow = frame.columns;
    coordBufSize.X = sizeRow; /* number of cols */
    coordBufSize.Y = sizeCol; /* number of rows */
    coordBufStart.X = 0;
    coordBufStart.Y = 0;
    srctWriteRect.Top = 0;
    srctWriteRect.Bottom = sizeCol - 1;
    srctWriteRect.Left = 0;
    srctWriteRect.Right = sizeRow - 1;
    if (charBuf) {
        free(charBuf);
    }
    charBuf = malloc(sizeCol * sizeRow * sizeof(CHAR_INFO));
    /* Legacy cursor-shape control remains intentionally disabled. */
    SetConsoleOutputCP(437);
    SetConsoleScreenBufferSize(output, coordBufSize);
}

VOID w32cdispPaint(HANDLE output,
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
    if (!charBuf) {
        return;
    }
    changed = flagForce || frame.generation != displayedGeneration;
    if (changed) {
        for (i = 0; i < sizeCol; ++i) {
            for (j = 0; j < sizeRow; ++j) {
                USHORT index = i * CORE_PLATFORM_DISPLAY_MAX_COLUMNS + j;
                ansiChar = frame.characters[index];
                charProp = frame.attributes[index]; /* & 0x7f; */
                /* if (!ansiChar) continue; */
                MultiByteToWideChar(437, 0, (LPCSTR)(&ansiChar), 1, (LPWSTR)(&unicodeChar), 1);
                charBuf[i * sizeRow + j].Char.UnicodeChar = unicodeChar;
                charBuf[i * sizeRow + j].Attributes = charProp;
            }
        }
        WriteConsoleOutput(output, charBuf, coordBufSize, coordBufStart, &srctWriteRect);
        displayedGeneration = frame.generation;
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

VOID w32cdispFinal(HANDLE output) {
    if (charBuf) {
        free(charBuf);
    }
    charBuf = NULL;
    SetConsoleCursorInfo(output, &defaultCurInfo);
    SetConsoleOutputCP(defaultCodePage);
    SetConsoleScreenBufferSize(output, defaultBufInfo.dwSize);
}
