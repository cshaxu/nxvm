/* This file is a part of NXVM project. */

#include "../vapi.h"

#include "tchar.h"

#include "win32con.h"
#include "w32cdisp.h"

static CHAR_INFO *charBuf;
static COORD coordDefaultBufSize, coordBufSize, coordBufStart;
static SMALL_RECT srctWriteRect;
static UCHAR sizeRow, sizeCol;
//static CONSOLE_CURSOR_INFO defaultCurInfo;
static CONSOLE_SCREEN_BUFFER_INFO defaultBufInfo;

void w32cdispInit()
{
//	GetConsoleCursorInfo(hOut, (PCONSOLE_CURSOR_INFO)(&defaultCurInfo));
	GetConsoleScreenBufferInfo(hOut, &defaultBufInfo);
	vapiSleep(1000);
	w32cdispSetScreen();
}
void w32cdispSetScreen()
{
//	CONSOLE_CURSOR_INFO curInfo = defaultCurInfo;
//	curInfo.bVisible = 0;
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
	charBuf = (CHAR_INFO *)malloc(sizeCol * sizeRow * sizeof(CHAR_INFO));
//	SetConsoleCursorInfo(hOut, &curInfo);
	SetConsoleScreenBufferSize(hOut, coordBufSize);
}

void w32cdispPaint()
{
	UCHAR ansiChar;
	WCHAR unicodeChar;
	WORD  charProp;
	UCHAR i, j;
	COORD curPos;
//	HANDLE hOutBuf;
	for(i = 0;i < sizeCol;++i) {
		for(j = 0;j < sizeRow;++j) {
			ansiChar = vapiCallBackDisplayGetCurrentChar(i, j);
			charProp = vapiCallBackDisplayGetCurrentCharProp(i, j);
			MultiByteToWideChar(437, 0, (LPCSTR)(&ansiChar), 1, (LPWSTR)(&unicodeChar), 1);
			charBuf[i * sizeRow + j].Char.UnicodeChar = unicodeChar;
			charBuf[i * sizeRow + j].Attributes = charProp;
		}
	}
/*	hOutBuf = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,
		0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	WriteConsoleOutput(hOutBuf, charBuf,
		coordBufSize, coordBufStart, &srctWriteRect);
	curPos.X = vapiCallBackDisplayGetCurrentCursorPosY();
	curPos.Y = vapiCallBackDisplayGetCurrentCursorPosX();
	SetConsoleCursorPosition(hOutBuf,curPos);
	SetConsoleActiveScreenBuffer(hOutBuf);*/

	WriteConsoleOutput(hOut, charBuf,
		coordBufSize, coordBufStart, &srctWriteRect);
	curPos.X = vapiCallBackDisplayGetCurrentCursorPosY();
	curPos.Y = vapiCallBackDisplayGetCurrentCursorPosX();
	SetConsoleCursorPosition(hOut,curPos);

//	SetConsoleActiveScreenBuffer(hOut);
//	CloseHandle(hOutBuf);
}
void w32cdispFinal()
{
	if (charBuf) free((void *)charBuf);
//	SetConsoleCursorInfo(hOut, &defaultCurInfo);
	SetConsoleScreenBufferSize(hOut, defaultBufInfo.dwSize);
	hOut = INVALID_HANDLE_VALUE;
}
