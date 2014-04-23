/* This file is a part of NXVM project. */

#include "stdio.h"
#include "stdarg.h"

#include "../coption.h"
#include "vapi.h"

#include "../vfdd.h"

int forceNone = 1;

#if NXVM_SYSTEM == NXVM_NONE
// GENERAL
#elif NXVM_SYSTEM == NXVM_LINUX_TERMINAL
// LINUX TERMINAL
#elif NXVM_SYSTEM == NXVM_LINUX_APPLICATION
// LINUX APPLICATION QT
#elif NXVM_SYSTEM == NXVM_WIN32_CONSOLE
// WIN32 CONSOLE
#elif NXVM_SYSTEM == NXVM_WIN32_APPLICATION
// WIN32 APPLICATION
#else
// FAIL TO COMPILE 
#endif

// General Part: Print StdOut
int vapiPrint(const char *format, ...)
{// prints string out of vdisplay, i.e. in a outside console
	int nWrittenBytes = 0;
	va_list arg_ptr;
	va_start(arg_ptr, format);
	nWrittenBytes = vfprintf(stdout, format,arg_ptr);
	//nWrittenBytes = vsprintf(stringBuffer,format,arg_ptr);
	va_end(arg_ptr);
	return nWrittenBytes;
}
void vapiPrintByte(unsigned char n)
{
	char c;
	int i;
	for(i = 1;i >= 0;--i) {
		c = ((n>>(i*4))&0x0f)+0x30;
		if(c > 0x39) c += 0x27;
		vapiPrint("%c",c);
	}
}
void vapiPrintWord(unsigned short n)
{
	char c;
	int i;
	for(i = 3;i >= 0;--i) {
		c = ((n>>(i*4))&0x0f)+0x30;
		if(c > 0x39) c += 0x27;
		vapiPrint("%c",c);
	}
}
void vapiPrintAddr(unsigned short segment,unsigned short offset)
{vapiPrintWord(segment);vapiPrint(":");vapiPrintWord(offset);}
void vapiPause()
{
	fflush(stdin);
	vapiPrint("Press ENTER to continue . . .\n");
	getchar();
}

void vapiInsertFloppyDisk(const char *fname)
{
	size_t count;
	FILE *image = fopen(fname, "rb");
	if (image) {
		count = fread((void *)vfdd.base, sizeof(unsigned char),
		              0x00168000, image);
		/* vfddFormat(0xf6); */
		vfdd.flagexist = 0x01;
		fclose(image);
		/* vapiPrint("Disk image loaded to %lx\n",vfdd.base); */
		/* TODO: do other changes to vfdd, vfdc */
	} else vapiPrint("FDD:\tcannot read floppy image from '%s'.\n", fname);
}
void vapiRemoveFloppyDisk(const char *fname)
{
	size_t count;
	FILE *image;
	/* TODO: assert(vfdd.base) */
	image = fopen(fname, "wb");
	if(image) {
		if (vfdd.flagro)
			count = fwrite((void *)vfdd.base, sizeof(unsigned char),
			               0x00168000, image);
		vfdd.flagexist = 0x00;
		fclose(image);
	} else vapiPrint("FDD:\tcannot write floppy image to '%s'.\n", fname);
}