/* This file is a part of NekoVM project. */

/*	Type of platforms: NONE, MSDOS, WIN32
	Platform related codes: ./vmachine/system/:	ioapi.ch, msdos.ch, win32con.ch, win32app.ch
							./:					main.c
*/

#define DEBUG_VER "Alpha Build 0x0026"

#define PRODUCT "Neko's Virtual Machine Under Construction\n\
Copyright (c) 2012 Neko. All rights reserved.\n"

/*
	Developer:	Xu Ha
	Email:		cshaxu@gatech.edu
	Start:		01/25/2011
	End:		(null)
	Modules:	asm86, nvm, main(console,debug,dosint)
*/

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "nvm/vmachine.h"

#include "global.h"
#include "console.h"

//#define NVM_WIN32

#ifdef NVM_WIN32
#include "windows.h"
#pragma comment(linker, "/subsystem:\"console\" /entry:\"WinMainCRTStartup\"")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPreInstance,
                   LPSTR lpCmdLine,
                   int nShowCmd)
#else
int main(int argc, char **argv)
#endif
{
#ifndef DEBUG
	fprintf(stdout,"%s\n",PRODUCT);
#else
	fprintf(stdout,"%s\n",DEBUG_VER);
#endif
	NVMInit();
#ifdef NVM_WIN32
	MessageBox(NULL,"Neko's Virtual Machine WIN32APP Edition","WinMain",MB_OK);
#endif
	NSConsole();
	NVMTerm();
    return 0;
}