/* This file is a part of NXVM project. */

#include "../vmachine/vmachine.h"
#include "../vmachine/qdvga.h"

#include "win32app.h"
#include "w32adisp.h"

#define COLOR_BLACK			0
#define COLOR_BLUE			RGB(0,0,255)
#define COLOR_GREEN			RGB(0,128,0)
#define COLOR_CYAN			RGB(0,255,255)
#define COLOR_GRAY			RGB(128,128,128)
#define COLOR_LIGHTBLUE		RGB(173,216,230)
#define COLOR_LIGHTGREEN	RGB(144,238,144)
#define COLOR_LIGHTCYAN		RGB(224,255,255)
#define COLOR_RED			RGB(255,0,0)
#define COLOR_MAGENTA		RGB(255,0,255)
#define COLOR_BROWN			RGB(184,134,11)
#define COLOR_LIGHTGRAY		RGB(211,211,211)
#define COLOR_LIGHTRED		RGB(255,99,71)
#define COLOR_LIGHTMAGENTA  RGB(255,102,183)
#define COLOR_YELLOW		RGB(255,255,0)
#define COLOR_WHITE			RGB(255,255,255)

static HDC  hdcWnd, hdcBuf;
static HBITMAP hBmpBuf;
static INT clientHeight, clientWidth;
static HFONT hFont;
static LOGFONT logFont;
static INT flashCount;
static INT flashInterval;
static INT charWidth;
static INT charHeight;

void w32adispIncFlash()
{
	flashCount = (flashCount + 1) % 10;
}
void w32adispInit()
{
	hdcWnd = GetDC(hWnd);
	hdcBuf = CreateCompatibleDC(NULL);
	hBmpBuf = NULL;
	clientHeight = 0;
	clientWidth  = 0;
	flashCount   = 0;
	flashInterval = 5;
	charWidth = 10;
	charHeight = 16;
	logFont.lfWidth = 8;
	logFont.lfHeight = 8;
	logFont.lfEscapement = 0;
	logFont.lfOrientation = 0;
	logFont.lfWeight = 0;
	logFont.lfItalic = 0;
	logFont.lfUnderline = 0;
	logFont.lfStrikeOut = 0;
	logFont.lfCharSet = DEFAULT_CHARSET;
	logFont.lfOutPrecision = 0;
	logFont.lfClipPrecision = 0;
	logFont.lfQuality = 0;
	logFont.lfPitchAndFamily = 0;
//	lstrcpy(logFont.lfFaceName,L"");
	w32adispSetScreen();
}
void w32adispSetScreen()
{
	RECT clientRect,windowRect;
	LONG widthOffset, heightOffset;
	GetClientRect(hWnd,&clientRect);
	GetWindowRect(hWnd,&windowRect);
	widthOffset = windowRect.right - windowRect.left - clientRect.right;
	heightOffset = windowRect.bottom - windowRect.top - clientRect.bottom;//获取窗口和客户区大小，以决定窗口大小，从而决定客户区大小
	MoveWindow(hWnd, 0, 0, qdvga.rowsize * charWidth + widthOffset,
		qdvga.colsize * charHeight + heightOffset, SWP_NOMOVE);
	GetClientRect(hWnd,&clientRect);
	clientHeight = clientRect.bottom - clientRect.top;
	clientWidth  = clientRect.right - clientRect.left;
	logFont.lfWidth  = charWidth;
	logFont.lfHeight = charHeight;
	hFont  = CreateFontIndirect(&logFont);
	hFont  = (HFONT)SelectObject(hdcBuf, hFont);
	hBmpBuf = CreateCompatibleBitmap(hdcWnd,//创建缓冲位图
		GetDeviceCaps(hdcWnd, HORZRES), GetDeviceCaps(hdcWnd, VERTRES));
	SelectObject(hdcBuf, hBmpBuf);	
	SendMessage(hWnd, WM_PAINT, 0, 0);
}
static COLORREF CharProp2Color(UCHAR prop, BOOL font)
{
	UCHAR byte;
	if (font) byte = (prop & 0x0f);                             /* font color*/
	else      byte = ((prop & 0x70) >> 4);               /* background color */
	switch(byte) {
	case 0x00: return COLOR_BLACK;       break;
	case 0x01: return COLOR_BLUE;        break;
	case 0x02: return COLOR_GREEN;       break;
	case 0x03: return COLOR_CYAN;        break;
	case 0x04: return COLOR_RED;         break;
	case 0x05: return COLOR_MAGENTA;     break;
	case 0x06: return COLOR_BROWN;       break;
	case 0x07: return COLOR_LIGHTGRAY;   break;
	case 0x08: return COLOR_GRAY;        break;
	case 0x09: return COLOR_LIGHTBLUE;   break;
	case 0x0a: return COLOR_LIGHTGREEN;  break;
	case 0x0b: return COLOR_LIGHTGRAY;   break;
	case 0x0c: return COLOR_LIGHTRED;    break;
	case 0x0d: return COLOR_LIGHTMAGENTA;break;
	case 0x0e: return COLOR_YELLOW;      break;
	case 0x0f: return COLOR_WHITE;       break;
	default:   return COLOR_BLACK;       break;
	}
}
static VOID DisplayFlashCursor()
{
	UINT page = qdvga.page;
	HPEN hPen;
	INT x1_cursor, y1_cursor, x2_cursor, y2_cursor;
	x1_cursor = qdvga.cursor[page].x * logFont.lfHeight +
		qdvga.cursortop    + 8;
	x2_cursor = qdvga.cursor[page].x * logFont.lfHeight +
		qdvga.cursorbottom + 8;
	y1_cursor = (qdvga.cursor[page].y + 0)* logFont.lfWidth;
	y2_cursor = (qdvga.cursor[page].y + 1) * logFont.lfWidth;
	if (flashCount % 10 < flashInterval) { //光标变白
		hPen = (HPEN)CreatePen(PS_SOLID,2,RGB(255,255,255));
		SelectObject(hdcWnd, hPen);
		Rectangle(hdcWnd, y1_cursor, x1_cursor, y2_cursor, x2_cursor);
	} else { //光标消失。。即贴上背景图片。。
		BitBlt(hdcWnd, y1_cursor, x1_cursor, y2_cursor, x2_cursor,
			hdcBuf, y1_cursor, x1_cursor, SRCCOPY);
	}
}
void w32adispPaint()
{
	UINT i, j;
	HBRUSH hBrush;
	WCHAR unicodeChar;
	CHAR ansiChar;
	UCHAR charProp;
	hBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
	SelectObject(hdcBuf, hBrush);
	Rectangle(hdcBuf, 0, 0, logFont.lfWidth * qdvga.rowsize, 
		logFont.lfHeight * qdvga.colsize);
	for(i = 0;i < qdvga.colsize;++i) {
		for(j = 0;j < qdvga.rowsize;++j) {
			ansiChar = qdvgaVarChar(qdvga.page, i, j);
			charProp = qdvgaVarCharProp(qdvga.page, i, j);
			if (!ansiChar) continue;
			MultiByteToWideChar(437, 0, &ansiChar, 1, &unicodeChar, 1);
			SetTextColor(hdcBuf, CharProp2Color(charProp, TRUE));
			SetBkMode(hdcBuf, OPAQUE);
			SetBkColor(hdcBuf, CharProp2Color(charProp, FALSE));
			TextOut(hdcBuf, j * logFont.lfWidth, i * logFont.lfHeight,(LPWSTR)&unicodeChar, 1);
		}
	}
	BitBlt(hdcWnd, 0, 0, clientWidth, clientHeight, hdcBuf, 0, 0, SRCCOPY);
	DisplayFlashCursor();//闪烁光标
}
void w32adispFinal() {}
