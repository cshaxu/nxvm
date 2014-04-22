/* This file is a part of NekoVMac project. */

#include "selector.h"
#include "stdio.h"
#include "stdarg.h"

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