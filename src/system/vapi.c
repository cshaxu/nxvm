/* This file is a part of NXVM project. */

#include "stdio.h"
#include "stdarg.h"
#include "windows.h"

#include "../vmachine/vmachine.h"

#include "vlog.h"
#include "vapi.h"

#define VAPI_QDFDD

#ifdef VAPI_QDFDD
#include "../vmachine/qdfdd.h"
#define vfdd qdfdd
#else
#include "../vmachine/vfdd.h"
#endif

/* Console Print */
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

/* Win32 Application Window Operations */
HINSTANCE hInstance;
LPCWSTR szWindowClass = L"nxvm";
LPCWSTR szTitle = L"Neko's x86 Virtual Machine";
DWORD DisplayThreadId;
DWORD KernelThreadId;
HWND hWnd;
static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	switch (message) {
	case WM_COMMAND:
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 
		break;
 	case WM_PAINT:
 		/*OnPaint();*/
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		//exit(0);
		break;
	case WM_SIZE:
		/*rect.right=lParam & 0xffff;
		rect.bottom=lParam / 0x10000;*/
		break;
	case WM_SIZING:
		break;
	case WM_KEYDOWN:			//Bochs和NTVDM都只处理了MakeCode
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		vapiPrint("!");
		/*RecvKeyBoardMsg(message,wParam,lParam);*/
		break;
	case WM_SETFOCUS:
		/*RefreshKeyboardState();*/
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}
static ATOM MyRegisterClass()
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
static BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
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
static DWORD WINAPI DisplayThread(LPVOID lpParam)
{
	MSG msg;
	hInstance = GetModuleHandle(NULL);
	MyRegisterClass();
	InitInstance(hInstance, 0);
	while (vmachine.flagrun && GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	vmachine.flagrun = 0x00;
	return 0;
}
static DWORD WINAPI KernelThread(LPVOID lpParam)
{
	vlogInit();
	vmachineRunLoop();
	vlogFinal();
	return 0;
}
void vapiDisplayPaint()
{}
void vapiCreateDisplay()
{
	CreateThread(NULL, 0, DisplayThread, NULL, 0, &DisplayThreadId);
}
void vapiCreateKernel()
{
	CreateThread(NULL, 0, KernelThread, NULL, 0, &KernelThreadId);
}

/* Floppy Disk Drive Operations */
void vapiInsertFloppyDisk(const char *fname)
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
void vapiRemoveFloppyDisk(const char *fname)
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