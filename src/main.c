/* This file is a part of NXVM project. */

#define PRODUCT "Neko's x86 Virtual Machine [0x0045]\n\
Copyright (c) 2012 Neko. All rights reserved.\n"

/*
	Developer:	Xu Ha
	Email:		cshaxu@gatech.edu
	Start:		01/25/2012
	End:		(null)
	Module 0 - CONSOLE
	Project Date:	With Project
	Module I - ASM86
	Project Date:
		Stage 1: Asm		01/25/2012 - 01/31/2012
		Stage 2: Disasm		02/01/2012 - 02/05/2012
	Module II - VCPU: Intel 8086 Instruction Set
	Project Date:
		Stage 1: Code		02/06/2012 - 02/12/2012
		Stage 2: Verify		Not Started
		Stage 3: Test		Not Started
		Stage 4: Connect	02/12/2012 - 02/12/2012
	Module III - VDEVICE
	Module IV - VAPI
	Module V - DOSINT
*/

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "vmachine/coption.h"
#include "global.h"
#include "console.h"

#if NXVM_SYSTEM == NXVM_WIN32_APPLICATION
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
	NSConsole();
    return 0;
}

/*	COMMENTS
	unsigned int x = (unsigned int)test;
	void (*y)(void) = (*(void (*)(void))(x));
	y();
	MessageBox(NULL,"Neko's x86 Virtual Machine WIN32APP Edition","WinMain",MB_OK);
The NTVDM CPU has encountered an illegal instruction.
CS:0db 1 IP:ffd3 OP:63 fa 65 13 64 Choose 'close' to terminate the
application.	
	
*/