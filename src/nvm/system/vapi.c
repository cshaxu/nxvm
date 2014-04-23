/* This file is a part of NekoVMac project. */

#include "stdio.h"
#include "stdarg.h"

#include "../coption.h"
#include "vapi.h"

int forceNone = 1;

#ifdef MSDOS
// MSDOS PART
#else
#ifdef WIN32CON
// WIN32CON PART
#else
#ifdef WIN32APP
// WIN32APP PART
#endif
#endif
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