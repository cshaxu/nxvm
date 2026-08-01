/* Copyright 2012-2014 Neko. */

/* W32CDISP provides win32 console output interface. */

#include "core/product/utils.h"

#include "vm/platform/display_frame.h"

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

VOID w32cdispInit() {
    /* GetConsoleCursorInfo(hOut, (PCONSOLE_CURSOR_INFO)(&defaultCurInfo)); */
    GetConsoleScreenBufferInfo(hOut, &defaultBufInfo);
    defaultCodePage = GetConsoleCP();
    charBuf = NULL;
    w32cdispSetScreen();
}

VOID w32cdispSetScreen() {
    vm_platform_display_frame frame;

    vm_platform_display_capture(&frame);
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
        FREE((void *) charBuf);
    }
    charBuf = (PCHAR_INFO) MALLOC(sizeCol * sizeRow * sizeof(CHAR_INFO));
    /* SetConsoleCursorInfo(hOut, &curInfo); */
    SetConsoleOutputCP(437);
    SetConsoleScreenBufferSize(hOut, coordBufSize);
}

VOID w32cdispPaint(BOOL flagForce) {
    vm_platform_display_frame frame;
    UCHAR ansiChar;
    WCHAR unicodeChar;
    WORD  charProp;
    UCHAR i, j;
    COORD curPos;
    CONSOLE_CURSOR_INFO curInfo;
    BOOL changed;
    vm_platform_display_capture(&frame);
    if (!charBuf) {
        return;
    }
    changed = flagForce || frame.generation != displayedGeneration;
    if (changed) {
        for (i = 0; i < sizeCol; ++i) {
            for (j = 0; j < sizeRow; ++j) {
                USHORT index = i * VM_PLATFORM_DISPLAY_MAX_COLUMNS + j;
                ansiChar = frame.characters[index];
                charProp = frame.attributes[index]; /* & 0x7f; */
                /* if (!ansiChar) continue; */
                MultiByteToWideChar(437, 0, (LPCSTR)(&ansiChar), 1, (LPWSTR)(&unicodeChar), 1);
                charBuf[i * sizeRow + j].Char.UnicodeChar = unicodeChar;
                charBuf[i * sizeRow + j].Attributes = charProp;
            }
        }
        WriteConsoleOutput(hOut, charBuf, coordBufSize, coordBufStart, &srctWriteRect);
        displayedGeneration = frame.generation;
    }
    if (changed) {
        GetConsoleCursorInfo(hOut, (PCONSOLE_CURSOR_INFO)(&curInfo));
        curInfo.bVisible = frame.cursor_visible;
        curInfo.dwSize = (DWORD)(((frame.cursor_bottom - frame.cursor_top) % 8 + 1) * 100. / 8.);
        SetConsoleCursorInfo(hOut, &curInfo);
        curPos.X = frame.cursor_y;
        curPos.Y = frame.cursor_x;
        SetConsoleCursorPosition(hOut, curPos);
    }
}

VOID w32cdispFinal() {
    if (charBuf) {
        FREE((void *) charBuf);
    }
    charBuf = NULL;
    SetConsoleCursorInfo(hOut, &defaultCurInfo);
    SetConsoleOutputCP(defaultCodePage);
    SetConsoleScreenBufferSize(hOut, defaultBufInfo.dwSize);
    hOut = INVALID_HANDLE_VALUE;
}
