/* This file is a part of NXVM project. */

#include "../vapi.h"

#include "w32adisp.h"
#include "w32akeyb.h"
#include "win32app.h"

HWND hWnd = NULL;

static HINSTANCE hInstance;
static LPCWSTR szWindowClass = L"nxvm";
static LPCWSTR szTitle = L"Neko's x86 Virtual Machine";
static DWORD ThreadIdDisplay;
static DWORD ThreadIdKernel;
#define TIMER_PAINT   0
#define TIMER_RTC     1
static LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
                                WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	INT wmId, wmEvent;
	switch (message) {
	case WM_CREATE:
		SetTimer(hWnd, TIMER_PAINT, 50, NULL);
		SetTimer(hWnd, TIMER_RTC,   55, NULL);
		break;
	case WM_DESTROY:
		vapiCallBackMachineSetRunFlag(0x00);
		PostQuitMessage(0);
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam);
		break;
	case WM_TIMER:
		switch (wParam) {
		case TIMER_PAINT:
			w32adispIncFlash();
			w32adispPaint();
			break;
		case TIMER_RTC:
			vapiCallBackRtcUpdateTime();
			break;
		default: break;
		}
		break;
 	case WM_PAINT:
		BeginPaint(hWnd, &ps);
		w32adispPaint();
		EndPaint(hWnd, &ps);
		break;
	case WM_SIZE:
		break;
	case WM_SIZING:
		break;
	case WM_CHAR:
		w32akeybMakeChar(wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		w32akeybMakeKey(message, wParam, lParam);
		break;
	case WM_SETFOCUS:
		w32akeybMakeStatus(WM_KEYUP, wParam, lParam);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}
static ATOM ThreadDisplayRegisterClass(HINSTANCE hInstance)
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
	wcex.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
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
	ThreadDisplayRegisterClass(hInstance);
	if (!ThreadDisplayInitInstance(hInstance, 0)) return FALSE;

	w32adispInit();
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	vapiCallBackMachineSetRunFlag(0x00);
	hWnd = NULL;
	w32adispFinal();

	return 0;
}
static DWORD WINAPI ThreadKernel(LPVOID lpParam)
{
	vapiCallBackMachineRun();
	return 0;
}

void win32appSleep(DWORD milisec)
{
	Sleep(milisec);
}
void win32appDisplaySetScreen() {w32adispSetScreen();}
void win32appDisplayPaint() {w32adispPaint();}
void win32appStartMachine()
{
	if (!hWnd)
		CreateThread(NULL, 0, ThreadDisplay, NULL, 0, &ThreadIdDisplay);
	if (!vapiCallBackMachineGetRunFlag())
		CreateThread(NULL, 0, ThreadKernel, NULL, 0, &ThreadIdKernel);
}
