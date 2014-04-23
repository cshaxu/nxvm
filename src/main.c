/* This file is a part of NekoVMac project. */

/*
	Type of systems: NONE, MSDOS, WIN32CON, WIN32APP
*/

#define PRODUCT "Neko's Virtual Machine [0x002c]\n\
Copyright (c) 2012 Neko. All rights reserved.\n"

/*
	Developer:	Xu Ha
	Email:		cshaxu@gatech.edu
	Start:		01/25/2012
	End:		(null)
	Module I - ASM86
	Project Date:	01/25/2012 - 02/05/2012
	Module II - CONSOLE
	Project Date:	01/25/2012 - Present
*/

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "global.h"
#include "console.h"

void test()
{fprintf(stdout,"test\n");}

#ifdef NVM_WIN32APP
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
	fprintf(stdout,"%s\n",PRODUCT);
	NSConsole(argc,argv);
    return 0;
}

/*	COMMENTS
	unsigned int x = (unsigned int)test;
	void (*y)(void) = (*(void (*)(void))(x));
	y();
	MessageBox(NULL,"Neko's Virtual Machine WIN32APP Edition","WinMain",MB_OK);
The NTVDM CPU has encountered an illegal instruction.
CS:0db 1 IP:ffd3 OP:63 fa 65 13 64 Choose 'close' to terminate the
application.	
	
*/