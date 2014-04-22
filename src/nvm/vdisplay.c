#include "stdio.h"
#include "stdarg.h"
#include "vmachine.h"
#include "vdisplay.h"

int nvmprint(const char *format, ...)
{
	int nWrittenBytes = 0;
	va_list arg_ptr;
	va_start(arg_ptr, format);
	//nWrittenBytes = vfprintf(stdout,format,arg_ptr);
	//nWrittenBytes = vsprintf(stringBuffer,format,arg_ptr);
	va_end(arg_ptr);
	return nWrittenBytes;
}