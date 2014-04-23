/* This file is a part of NXVM project. */

#include "../vapi.h"

#include "tchar.h"

#include "win32con.h"
#include "w32cdisp.h"

static PCHAR_INFO charBuf;
static COORD coordDefaultBufSize, coordBufSize, coordBufStart;
static SMALL_RECT srctWriteRect;
static USHORT sizeRow, sizeCol;
static CONSOLE_CURSOR_INFO defaultCurInfo;
static UINT defaultCodePage;
static CONSOLE_SCREEN_BUFFER_INFO defaultBufInfo;
static UCHAR bufComp[0x1000];
void w32cdispInit()
{
//	GetConsoleCursorInfo(hOut, (PCONSOLE_CURSOR_INFO)(&defaultCurInfo));
	GetConsoleScreenBufferInfo(hOut, &defaultBufInfo);
	defaultCodePage = GetConsoleCP();
	charBuf = NULL;
	w32cdispSetScreen();
}
void w32cdispSetScreen()
{
	sizeCol = vapiCallBackDisplayGetColSize();
	sizeRow = vapiCallBackDisplayGetRowSize();
	coordBufSize.X = sizeRow; // number of cols
	coordBufSize.Y = sizeCol; // number of rows
	coordBufStart.X = 0; 
	coordBufStart.Y = 0; 
	srctWriteRect.Top = 0;
	srctWriteRect.Bottom = sizeCol - 1;
	srctWriteRect.Left = 0;
	srctWriteRect.Right = sizeRow - 1;
	if (charBuf) free(charBuf);
	charBuf = (PCHAR_INFO)malloc(sizeCol * sizeRow * sizeof(CHAR_INFO));
//	SetConsoleCursorInfo(hOut, &curInfo);
	SetConsoleOutputCP(437);
	SetConsoleScreenBufferSize(hOut, coordBufSize);
}

void w32cdispPaint(BOOL force)
{
	UCHAR ansiChar;
	WCHAR unicodeChar;
	WORD  charProp;
	UCHAR i, j;
	COORD curPos;
	CONSOLE_CURSOR_INFO curInfo;
	if (!charBuf) return;
	if (force || vapiCallBackDisplayGetBufferChange()) {
		for(i = 0;i < sizeCol;++i) {
			for(j = 0;j < sizeRow;++j) {
				ansiChar = vapiCallBackDisplayGetCurrentChar(i, j);
				charProp = vapiCallBackDisplayGetCurrentCharProp(i, j);// & 0x7f;
				//if (!ansiChar) continue;
				MultiByteToWideChar(437, 0, (LPCSTR)(&ansiChar), 1, (LPWSTR)(&unicodeChar), 1);
				charBuf[i * sizeRow + j].Char.UnicodeChar = unicodeChar;
				charBuf[i * sizeRow + j].Attributes = charProp;
			}
		}
		WriteConsoleOutput(hOut, charBuf,
			coordBufSize, coordBufStart, &srctWriteRect);
	}
	if (force || vapiCallBackDisplayGetCursorChange()) {
		GetConsoleCursorInfo(hOut, (PCONSOLE_CURSOR_INFO)(&curInfo));
		curInfo.bVisible = vapiCallBackDisplayGetCursorVisible();
		curInfo.dwSize = (DWORD)(((vapiCallBackDisplayGetCursorBottom() -
			vapiCallBackDisplayGetCursorTop()) % 8 + 1) * 100. / 8.);
		SetConsoleCursorInfo(hOut, &curInfo);
		curPos.X = vapiCallBackDisplayGetCurrentCursorPosY();
		curPos.Y = vapiCallBackDisplayGetCurrentCursorPosX();
		SetConsoleCursorPosition(hOut,curPos);
	}
}
void w32cdispFinal()
{
	if (charBuf) free((void *)charBuf);
	charBuf = NULL;
	SetConsoleCursorInfo(hOut, &defaultCurInfo);
	SetConsoleOutputCP(defaultCodePage);
	SetConsoleScreenBufferSize(hOut, defaultBufInfo.dwSize);
	hOut = INVALID_HANDLE_VALUE;
}
