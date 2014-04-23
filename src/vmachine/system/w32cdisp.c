/* This file is a part of NXVM project. */

#include "../vapi.h"

#include "tchar.h"

#include "win32con.h"
#include "w32cdisp.h"

static CHAR_INFO *scrBuf;
static COORD coordBufSize, coordBufStart;
static SMALL_RECT srctWriteRect;
static UCHAR sizeRow, sizeCol;

void w32cdispInit()
{
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
	scrBuf = (CHAR_INFO *)malloc(sizeCol * sizeRow * sizeof(CHAR_INFO));
	SetConsoleScreenBufferSize(hOut, coordBufSize);
}

void w32cdispPaint()
{
	UCHAR ansiChar;
	WCHAR unicodeChar;
	WORD  charProp;
	UCHAR i, j;
	COORD curPos;
	for(i = 0;i < sizeCol;++i) {
		for(j = 0;j < sizeRow;++j) {
			ansiChar = vapiCallBackDisplayGetCurrentChar(i, j);
			charProp = vapiCallBackDisplayGetCurrentCharProp(i, j);
			MultiByteToWideChar(437, 0, (LPCSTR)(&ansiChar), 1, (LPWSTR)(&unicodeChar), 1);
			scrBuf[i * sizeRow + j].Char.UnicodeChar = unicodeChar;
			scrBuf[i * sizeRow + j].Attributes = charProp;
		}
	}
	WriteConsoleOutput(hOut, scrBuf,
		coordBufSize, coordBufStart, &srctWriteRect);
	curPos.X = vapiCallBackDisplayGetCurrentCursorPosY();
	curPos.Y = vapiCallBackDisplayGetCurrentCursorPosX();
	SetConsoleCursorPosition(hOut,curPos);
}
void w32cdispFinal()
{
	if (scrBuf) free((void *)scrBuf);
}
