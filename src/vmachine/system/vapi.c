/* This file is a part of NXVM project. */

#include "stdio.h"
#include "stdarg.h"

#include "../coption.h"
#include "vapi.h"

int forceNone = 1;

#if NXVM_SYSTEM == NXVM_NONE
// GENERAL
#elif NXVM_SYSTEM == NXVM_LINUX_TERMINAL
// LINUX TERMINAL
#elif NXVM_SYSTEM == NXVM_LINUX_APPLICATION_QT
// LINUX APPLICATION QT
#elif NXVM_SYSTEM == NXVM_WIN32_CONSOLE
// WIN32 CONSOLE
#elif NXVM_SYSTEM == NXVM_WIN32_APPLICATION
// WIN32 APPLICATION
#else
// FAIL TO COMPILE 
#endif

// General Part
int nvmprint(const char *format, ...)
{// prints string out of vdisplay, i.e. in a outside console
	int nWrittenBytes = 0;
	va_list arg_ptr;
	va_start(arg_ptr, format);
	nWrittenBytes = vfprintf(stdout,format,arg_ptr);
	//nWrittenBytes = vsprintf(stringBuffer,format,arg_ptr);
	va_end(arg_ptr);
	return nWrittenBytes;
}
void nvmprintbyte(t_nubit8 n)
{
	char c;
	int i;
	for(i = 1;i >= 0;--i) {
		c = ((n>>(i*4))&0x0f)+0x30;
		if(c > 0x39) c += 0x27;
		nvmprint("%c",c);
	}
}
void nvmprintword(t_nubit16 n)
{
	char c;
	int i;
	for(i = 3;i >= 0;--i) {
		c = ((n>>(i*4))&0x0f)+0x30;
		if(c > 0x39) c += 0x27;
		nvmprint("%c",c);
	}
}

void nvmprintaddr(t_nubit16 segment,t_nubit16 offset)
{nvmprintword(segment);nvmprint(":");nvmprintword(offset);}