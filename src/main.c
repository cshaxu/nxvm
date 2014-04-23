/* This file is a part of NXVM project. */

#define PRODUCT "Neko's x86 Virtual Machine [0.1.0x52]\n\
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
		Stage 1: PIC(8259)	02/24/2012 - 03/08/2012
		Stage 2: RTC(DS1302	03/02/2012 - Present // 未在CMOSInit()中对CMOS寄存器初始化
		Stage 3: PIT(8254)	03/03/2012 - Present.	// IO端口没有实现，对于BIOS和MSDOS也没有必要实现
		Stage 4: DMAC(8237)	03/03/2012 - Present.	// 仅实现了IO端口，但DMA传送功能没有实现
		Stage 5: FDC(8272)	03/07/2012 - Present.	// 需要实现IO端口，以及和DMA的交互
		Stage ?: KEYB		Not Started
		Stage ?: VGA		Not Started
		Stage ?: Monitor	Not Started

		Stage ?: BIOS		Not Started BIOS Service via I/O
		Stage ?: BIOS		Not Started BIOS INT Routine via ASM

		Stage ?: Serial		Not Started
		Stage ?: Printer	Not Started
		Stage ?: HDD		Not Started
		Stage ?: CDROM		Not Started

	Module IV - VAPI
		Stage 1: NONE		Not Started
		Stage 2: W32CON		Not Started
		Stage 3: W32APP		Not Started
	Module V - DOSINT
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