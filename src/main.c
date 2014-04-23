/* This file is a part of NXVM project. */

#define PRODUCT "Neko's x86 Virtual Machine [0.1.0x54]\n\
Copyright (c) 2012 Neko. All rights reserved.\n"

/*
	Developer:	Xu Ha
	Email:		cshaxu@gatech.edu
	Start:		01/25/2012
	End:		(null)
	Module 0 - CONSOLE
		With Project
	Module I - ASM86
		Stage 1: Asm		01/25/2012 - 01/31/2012
		Stage 2: Disasm		02/01/2012 - 02/05/2012
	Module II - VM_KERNEL
		Stage 1: CPU		02/06/2012 - 02/12/2012
		Stage 2: PIC(8259)	02/24/2012 - 03/08/2012
		Stage 3: RTC(DS1302	03/02/2012 - Present.	// TODO: 在CMOSInit()中对CMOS寄存器初始化
		Stage 4: PIT(8254)	03/03/2012 - Present.	// IO not implemented.
	Module III - VM_STORAGE
		Stage 1: DMAC(8237)	03/03/2012 - Present.	// DMA is only used to store memory address and word count
		Stage 2: FDDrive)	03/08/2012 - 03/18/2012	// Load and unload floppy image file
		Stage 3: FDC(8272)	03/07/2012 - 03/18/2012	// Transfers data between flp image and memory.
	Module IV - VM_INPUT
		Stage ?: KEYB		Not Started
	Module V - VM_OUTPUT
		Stage ?: VGA		Not Started
		Stage ?: Monitor	Not Started
	Module VI - VM_BIOS
		Stage ?: BIOS		Not Started BIOS Service via I/O
		Stage ?: BIOS		Not Started BIOS INT Routine via ASM
	Module VII - VAPI
		Stage 1: NONE		Not Started
		Stage 2: W32CON		Not Started
		Stage 3: W32APP		Not Started
	Module VIII - DOSINT
	Other:
		Stage ?: Serial		Not Started
		Stage ?: Printer	Not Started
		Stage ?: HDD		Not Started
		Stage ?: CDROM		Not Started
*/

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "vmachine/coption.h"
#include "global.h"
#include "console.h"

#if NXVM_SYSTEM == NXVM_NONE
int main(int argc, char **argv)
{
	fprintf(stdout,"%s\n",PRODUCT);
	NSConsole();
    return 0;
}
#elif NXVM_SYSTEM == NXVM_LINUX_TERMINAL
#elif NXVM_SYSTEM == NXVM_LINUX_APPLICATION
#elif NXVM_SYSTEM == NXVM_WIN32_CONSOLE
int main(int argc, char **argv)
{
	fprintf(stdout,"%s\n",PRODUCT);
	NSConsole();
    return 0;
}
#elif NXVM_SYSTEM == NXVM_WIN32_APPLICATION
#include "windows.h"
//#pragma comment(linker, "/subsystem:\"console\" /entry:\"WinMainCRTStartup\"")
//#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPreInstance,
                   LPSTR lpCmdLine,
                   int nShowCmd)
{
	return 0;
}
#else
#endif
/*	COMMENTS
	unsigned int x = (unsigned int)test;
	void (*y)(void) = (*(void (*)(void))(x));
	y();
	MessageBox(NULL,"Neko's x86 Virtual Machine WIN32APP Edition","WinMain",MB_OK);
The NTVDM CPU has encountered an illegal instruction.
CS:0db 1 IP:ffd3 OP:63 fa 65 13 64 Choose 'close' to terminate the
application.	
	
*/