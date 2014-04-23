/* This file is a part of NXVM project. */

#include "stdio.h"
#include "stdarg.h"
#include "windows.h"

#include "../vmachine/vmachine.h"

#include "vlog.h"
#include "vapi.h"

#define VAPI_QDFDD
#define VAPI_QDKEYB
#define VAPI_QDRTC

#ifdef VAPI_QDFDD
#include "../vmachine/qdfdd.h"
#define vfdd qdfdd
#else
#include "../vmachine/vfdd.h"
#endif

#ifdef VAPI_QDKEYB
#include "../vmachine/qdkeyb.h"
#define vkeybRecvKeyPress qdkeybRecvKeyPress
#define vkeybVarFlag0 qdkeybVarFlag0
#define vkeybVarFlag1 qdkeybVarFlag1
#define vkeybGetShift qdkeybGetShift
#define vkeybGetAlt   qdkeybGetAlt
#define vkeybGetCtrl  qdkeybGetCtrl
#define VKEYB_FLAG0_A_INSERT  QDKEYB_FLAG0_A_INSERT
#define VKEYB_FLAG0_A_CAPLCK  QDKEYB_FLAG0_A_CAPLCK
#define VKEYB_FLAG0_A_NUMLCK  QDKEYB_FLAG0_A_NUMLCK
#define VKEYB_FLAG0_A_SCRLCK  QDKEYB_FLAG0_A_SCRLCK
#define VKEYB_FLAG0_D_ALT     QDKEYB_FLAG0_D_ALT
#define VKEYB_FLAG0_D_CTRL    QDKEYB_FLAG0_D_CTRL
#define VKEYB_FLAG0_D_LSHIFT  QDKEYB_FLAG0_D_LSHIFT
#define VKEYB_FLAG0_D_RSHIFT  QDKEYB_FLAG0_D_RSHIFT
#define VKEYB_FLAG1_D_INSERT  QDKEYB_FLAG1_D_INSERT
#define VKEYB_FLAG1_D_CAPLCK  QDKEYB_FLAG1_D_CAPLCK
#define VKEYB_FLAG1_D_NUMLCK  QDKEYB_FLAG1_D_NUMLCK
#define VKEYB_FLAG1_D_SCRLCK  QDKEYB_FLAG1_D_SCRLCK
#define VKEYB_FLAG1_D_PAUSE   QDKEYB_FLAG1_D_PAUSE
#define VKEYB_FLAG1_D_SYSRQ   QDKEYB_FLAG1_D_SYSRQ
#define VKEYB_FLAG1_D_LALT    QDKEYB_FLAG1_D_LALT
#define VKEYB_FLAG1_D_LCTRL   QDKEYB_FLAG1_D_LCTRL
#else
#include "../vmachine/vkeyb.h"
#endif

#ifdef VAPI_QDRTC
#include "../vmachine/qdrtc.h"
#else
#include "../vmachine/vcmos.h"
#endif

/* General Functions */
int vapiPrint(const char *format, ...)
{// prints string out of vdisplay, i.e. in a outside console
	int nWrittenBytes = 0;
	va_list arg_ptr;
	va_start(arg_ptr, format);
	nWrittenBytes = vfprintf(stdout, format,arg_ptr);
	//nWrittenBytes = vsprintf(stringBuffer,format,arg_ptr);
	va_end(arg_ptr);
	return nWrittenBytes;
}
void vapiPrintByte(unsigned char n)
{
	char c;
	int i;
	for(i = 1;i >= 0;--i) {
		c = ((n>>(i*4))&0x0f)+0x30;
		if(c > 0x39) c += 0x27;
		vapiPrint("%c",c);
	}
}
void vapiPrintWord(unsigned short n)
{
	char c;
	int i;
	for(i = 3;i >= 0;--i) {
		c = ((n>>(i*4))&0x0f)+0x30;
		if(c > 0x39) c += 0x27;
		vapiPrint("%c",c);
	}
}
void vapiPrintAddr(unsigned short segment,unsigned short offset)
{vapiPrintWord(segment);vapiPrint(":");vapiPrintWord(offset);}
void vapiPause()
{
	fflush(stdin);
	vapiPrint("Press ENTER to continue . . .\n");
	getchar();
}
void vapiSleep(unsigned int milisec)
{
	Sleep(milisec);
}

/* Win32 Application Window Operations */
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
#include "../vmachine/qdvga.h"
HWND hWnd;
HDC  hdcWnd, hdcBuf;
HBITMAP hBmpBuf;
INT clientHeight, clientWidth;
HFONT hFont;
LOGFONT logFont;
INT flashCount;
INT flashInterval;
INT charWidth;
INT charHeight;
void vapiDisplayInit()
{
	hdcWnd = GetDC(hWnd);
	hdcBuf = hdcWnd;
	//hdcBuf = CreateCompatibleDC(NULL);
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
	//lstrcpy(logFont.lfFaceName,L"FixedSys");
}
void vapiDisplaySetScreen()
{
	RECT clientRect,windowRect;
	int widthOffset, heightOffset;
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
static void DisplayFlashCursor()
{
	UINT page = qdvgaVarPageNum;
	HPEN hPen;
	INT x1_cursor, y1_cursor, x2_cursor, y2_cursor;
	x1_cursor = (qdvga.cursor[page].x + 0)* logFont.lfWidth;
	x2_cursor = (qdvga.cursor[page].x + 1) * logFont.lfWidth;
	y1_cursor = qdvga.cursor[page].y * logFont.lfHeight +
		qdvgaVarCursorTop    + 8;
	y2_cursor = qdvga.cursor[page].y * logFont.lfHeight +
		qdvgaVarCursorBottom + 8;
	if (flashCount % 10 < flashInterval) { //光标变白
		hPen = (HPEN)CreatePen(PS_SOLID,2,RGB(255,255,255));
		SelectObject(hdcWnd, hPen);
		Rectangle(hdcWnd, x1_cursor, y1_cursor, x2_cursor, y2_cursor);
	} else { //光标消失。。即贴上背景图片。。
		BitBlt(hdcWnd, x1_cursor, y1_cursor, x2_cursor, y2_cursor,
			hdcBuf, x1_cursor, y1_cursor, SRCCOPY);
	}
}
void vapiDisplayPaint()
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
			ansiChar = qdvgaVarChar(qdvgaVarPageNum, i, j);
			charProp = qdvgaVarCharProp(qdvgaVarPageNum, i, j);
			if (!ansiChar) continue;
			MultiByteToWideChar(437, 0, &ansiChar, 1, &unicodeChar, 1);
			SetTextColor(hdcBuf, CharProp2Color(charProp, TRUE));
			SetBkMode(hdcBuf, OPAQUE);
			SetBkColor(hdcBuf, CharProp2Color(charProp, FALSE));
			TextOut(hdcBuf, j * logFont.lfWidth, i * logFont.lfHeight,(LPWSTR)&unicodeChar, 1);
		}
	}
	//BitBlt(hdcWnd, 0, 0, clientWidth, clientHeight, hdcBuf, 0, 0, SRCCOPY);
	//DisplayFlashCursor();//闪烁光标
}
void vapiDisplayFinal()
{}

static void KeyboardMakeStatus(UINT message, WPARAM wParam, LPARAM lParam)
{
	vkeybVarFlag0 = 0x00;
	vkeybVarFlag1 = 0x00;
	if(message == WM_KEYDOWN || message == WM_SYSKEYDOWN) {
		switch(wParam) { //如果是普通字符，则flag2是0 
		case VK_INSERT: SetBit(vkeybVarFlag1, VKEYB_FLAG1_D_INSERT);break;
		case VK_CAPITAL:SetBit(vkeybVarFlag1, VKEYB_FLAG1_D_CAPLCK);break;
		case VK_NUMLOCK:SetBit(vkeybVarFlag1, VKEYB_FLAG1_D_NUMLCK);break;
		case VK_SCROLL: SetBit(vkeybVarFlag1, VKEYB_FLAG1_D_SCRLCK);break;
		case VK_PAUSE:  SetBit(vkeybVarFlag1, VKEYB_FLAG1_D_PAUSE); break;
			//这里扫了Sys req 键的处理
		case VK_MENU:   SetBit(vkeybVarFlag1, VKEYB_FLAG1_D_LALT); break;
			//区别不了。。。就当做按下左边alt，也就是说按右边也会相应，但是不要紧~~
		case VK_CONTROL:SetBit(vkeybVarFlag1, VKEYB_FLAG1_D_LCTRL); break;
			//同上
		default: break;
		}
	} // else: keyUp, sysKeyUp, 功能键全部都处于没有按下状态
	//刷新功能键当前状态
	if (GetKeyState(VK_INSERT)  < 0) SetBit(vkeybVarFlag0, VKEYB_FLAG0_A_INSERT);
	if (GetKeyState(VK_CAPITAL) < 0) SetBit(vkeybVarFlag0, VKEYB_FLAG0_A_CAPLCK);
	if (GetKeyState(VK_NUMLOCK) < 0) SetBit(vkeybVarFlag0, VKEYB_FLAG0_A_NUMLCK);
	if (GetKeyState(VK_SCROLL)  < 0) SetBit(vkeybVarFlag0, VKEYB_FLAG0_A_SCRLCK);
	if (GetKeyState(VK_MENU)    < 0) SetBit(vkeybVarFlag0, VKEYB_FLAG0_D_ALT);
	if (GetKeyState(VK_CONTROL) < 0) SetBit(vkeybVarFlag0, VKEYB_FLAG0_D_CTRL);
	if (GetKeyState(VK_LSHIFT)  < 0) SetBit(vkeybVarFlag0, VKEYB_FLAG0_D_LSHIFT);
	if (GetKeyState(VK_RSHIFT)  < 0) SetBit(vkeybVarFlag0, VKEYB_FLAG0_D_RSHIFT);
}
static void KeyboardMakeChar(WPARAM wParam, LPARAM lParam)
{
	unsigned short ascii = (unsigned short)(((lParam & 0xffff0000) >> 8) | wParam);
	while(vkeybRecvKeyPress(ascii)) vapiSleep(10);
}
static void KeyboardMakeKey(UINT message, WPARAM wParam, LPARAM lParam)
{
	unsigned short ascii = (unsigned short)((lParam & 0xffff0000)>>8);
	if (WM_SYSKEYDOWN == message || WM_KEYDOWN == message) {
		switch(wParam) {
		case VK_F1:
		case VK_F2:
		case VK_F3:
		case VK_F4:
		case VK_F5:
		case VK_F6:
		case VK_F7:
		case VK_F8:
		case VK_F9:
		case VK_F10:
		case VK_F11:
		case VK_F12://F1~~F12
			if(vkeybGetShift)      ascii += 0x1900;
			else if(vkeybGetAlt)   ascii += 0x2d00;
			else if (vkeybGetCtrl) ascii += 0x2300;
			vkeybRecvKeyPress(ascii);
			break;
		case VK_ESCAPE://ESC
		case VK_PRIOR://pageUP
		case VK_NEXT://pageDown
		case VK_END:
		case VK_HOME:
		case VK_LEFT:
		case VK_UP:
		case VK_RIGHT:
		case VK_DOWN:
			 vkeybRecvKeyPress(ascii);
		     break;
		default://剩下的字符可能是alt。。ctl与普通字符等，但是updateKBStatus会过滤掉普通字符
			KeyboardMakeStatus(message, wParam, lParam);
			if (wParam>= 0x41 && wParam<=0x41+'Z'-'A') {
				if (vkeybGetAlt)  vkeybRecvKeyPress(ascii);
				if (vkeybGetCtrl) vkeybRecvKeyPress(ascii + wParam - 0x41);
				//如果不是按下ctrl，则是按下alt
			}
			break;
		}
		
	}
	else //keyUp, sysKeyUp
		KeyboardMakeStatus(message, wParam, lParam);
}

HINSTANCE hInstance;
LPCWSTR szWindowClass = L"nxvm";
LPCWSTR szTitle = L"Neko's x86 Virtual Machine";
DWORD ThreadIdDisplay;
DWORD ThreadIdKernel;
#define TIMER_PAINT   0
#define TIMER_RTC     1
static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	INT wmId, wmEvent;
	switch (message) {
	case WM_CREATE:
		SetTimer(hWnd, TIMER_PAINT, 50, NULL);
		SetTimer(hWnd, TIMER_RTC,   55, NULL);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam);
		break;
	case WM_TIMER:
		switch (wParam) {
		case TIMER_PAINT:
			flashCount = (flashCount + 1) % 10;
			vapiDisplayPaint();
			break;
		case TIMER_RTC:
			qdrtcUpdateTime();
			break;
		default: break;
		}
		break;
 	case WM_PAINT:
		BeginPaint(hWnd, &ps);
		vapiDisplayPaint();
		EndPaint(hWnd, &ps);
		break;
	case WM_SIZE:
		break;
	case WM_SIZING:
		break;
	case WM_CHAR:
		KeyboardMakeChar(wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		KeyboardMakeKey(message, wParam, lParam);
		break;
	case WM_SETFOCUS:
		KeyboardMakeStatus(WM_KEYUP, wParam, lParam);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}
static ATOM ThreadDisplayRegisterClass()
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(8);//(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= NULL;
	return RegisterClassEx(&wcex);
}
static BOOL ThreadDisplayInitInstance(HINSTANCE hInstance, int nCmdShow)
{
	DWORD dwStyle = WS_THICKFRAME | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU |
		            WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
	hWnd = CreateWindow(szWindowClass, szTitle, dwStyle, CW_USEDEFAULT, 0,
	                    888, 484, NULL, NULL, hInstance, NULL);
	                           /* window size is 888 x 484 for "Courier New" */
	if (!hWnd) return FALSE;
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
	return TRUE;
}
static DWORD WINAPI ThreadDisplay(LPVOID lpParam)
{
	MSG msg;
	hInstance = GetModuleHandle(NULL);
	ThreadDisplayRegisterClass();
	ThreadDisplayInitInstance(hInstance, 0);
	vapiDisplayInit();
	while (vmachine.flagrun && GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	vapiDisplayFinal();
	vmachine.flagrun = 0x00;
	return 0;
}
static DWORD WINAPI ThreadKernel(LPVOID lpParam)
{
	vlogInit();
	vmachineRunLoop();
	vlogFinal();
	return 0;
}
void vapiCreateDisplay()
{
	CreateThread(NULL, 0, ThreadDisplay, NULL, 0, &ThreadIdDisplay);
}
void vapiCreateKernel()
{
	CreateThread(NULL, 0, ThreadKernel, NULL, 0, &ThreadIdKernel);
}

/* Floppy Disk Drive Operations */
void vapiFloppyInsert(const char *fname)
{
	size_t count;
	FILE *image = fopen(fname, "rb");
	if (image) {
		count = fread((void *)vfdd.base, sizeof(unsigned char),
		              0x00168000, image);
		vfdd.flagexist = 0x01;
		fclose(image);
		vapiPrint("Floppy disk inserted.\n");
	} else
		vapiPrint("Cannot read floppy image from '%s'.\n", fname);
}
void vapiFloppyRemove(const char *fname)
{
	size_t count;
	FILE *image;
	image = fopen(fname, "wb");
	if(image) {
		if (!vfdd.flagro)
			count = fwrite((void *)vfdd.base, sizeof(unsigned char),
			               0x00168000, image);
		vfdd.flagexist = 0x00;
		fclose(image);
		vapiPrint("Floppy disk removed.\n");
	} else
		vapiPrint("Cannot write floppy image to '%s'.\n", fname);
}