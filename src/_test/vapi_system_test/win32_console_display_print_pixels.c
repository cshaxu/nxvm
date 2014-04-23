#include <stdio.h>
#include <tchar.h>
#include <windows.h>
 
//typedef HWND (WINAPI *PROCGETCONSOLEWINDOW)();
//PROCGETCONSOLEWINDOW GetConsoleWindow;
 
int main()
{
//	HMODULE hKernel32 = GetModuleHandle(_T("kernel32"));
//	GetConsoleWindowX= (PROCGETCONSOLEWINDOW)GetProcAddress(hKernel32,"GetConsoleWindow");
	HWND cmd=GetConsoleWindow();
	HDC dc = GetDC(cmd);
	HBITMAP hBitmap;
	hBitmap=(HBITMAP)LoadImage(NULL,_T("d:/lena.bmp"),IMAGE_BITMAP,0,0,LR_LOADFROMFILE|LR_CREATEDIBSECTION);
	HDC cmdmem = CreateCompatibleDC(dc);
	SelectObject(cmdmem , hBitmap);
	BitBlt(dc , 0 , 0 , 580 , 1240 , cmdmem , 0 , 0 , SRCCOPY);
	getchar();
	return 1;
}