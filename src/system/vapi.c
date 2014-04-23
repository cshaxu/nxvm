/* This file is a part of NXVM project. */

#include "stdio.h"
#include "stdarg.h"

#include "vapi.h"

/* General Functions */
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
		if(c > 0x39) c += 0x07;
		vapiPrint("%c",c);
	}
}
void vapiPrintWord(unsigned short n)
{
	char c;
	int i;
	for(i = 3;i >= 0;--i) {
		c = ((n>>(i*4))&0x0f)+0x30;
		if(c > 0x39) c += 0x07;
		vapiPrint("%c",c);
	}
}
void vapiPrintAddr(unsigned short segment,unsigned short offset)
{vapiPrintWord(segment);vapiPrint(":");vapiPrintWord(offset);}

/* Record */
#include "../vmachine/vcpu.h"
#include "../vmachine/vram.h"

t_apirecord vapirecord;

#define _expression "%d\tcs:ip=%x:%x opcode=%x %x %x %x %x %x %x %x \
ax=%x bx=%x cx=%x dx=%x sp=%x bp=%x si=%x di=%x ds=%x es=%x ss=%x \
of=%1x sf=%1x zf=%1x cf=%1x af=%1x pf=%1x df=%1x if=%1x tf=%1x stack=%x\n"

void vapiRecordStart()
{
	if (!vapirecord.flag) return;
	if (vapirecord.fptr) fclose(vapirecord.fptr);
	vapirecord.count = 0;
	vapirecord.fptr = fopen(vapirecord.fname,"w");
}
void vapiRecordWrite()
{
	if (!vapirecord.flag) return;
	if ((!vapirecord.fptr)) return;
	fprintf(vapirecord.fptr, _expression,
		vapirecord.count, _cs, _ip,
		vramVarByte(_cs,_ip+0),vramVarByte(_cs,_ip+1),
		vramVarByte(_cs,_ip+2),vramVarByte(_cs,_ip+3),
		vramVarByte(_cs,_ip+4),vramVarByte(_cs,_ip+5),
		vramVarByte(_cs,_ip+6),vramVarByte(_cs,_ip+7),
		_ax,_bx,_cx,_dx,_sp,_bp,_si,_di,_ds,_es,_ss,
		_of,_sf,_zf,_cf,_af,_pf,_df,_if,_tf,vramVarWord(_ss,_sp));
}
void vapiRecordEnd()
{
	if (!vapirecord.flag) return;
	if (vapirecord.fptr) fclose(vapirecord.fptr);
	vapirecord.fptr = NULL;
	vapirecord.count = 0;
}

/* Trace */
#include "../console/console.h"

void vapiTrace()
{
	vapiCallBackByTrace();
}

/* Floppy Disk */
#include "../vmachine/qdfdd.h"
#define vfdd qdfdd

void vapiFloppyInsert(const char *fname)
{
	size_t count;
	FILE *image = fopen(fname, "rb");
	if (image) {
		count = fread((void *)vfdd.base, sizeof(unsigned char),
		              0x00168000, image);
		vfdd.flagexist = 0x01;
		fclose(image);
		vapiPrint("Floppy disk inserted.\n");
	} else
		vapiPrint("Cannot read floppy image from '%s'.\n", fname);
}
void vapiFloppyRemove(const char *fname)
{
	size_t count;
	FILE *image;
	image = fopen(fname, "wb");
	if(image) {
		if (!vfdd.flagro)
			count = fwrite((void *)vfdd.base, sizeof(unsigned char),
			               0x00168000, image);
		vfdd.flagexist = 0x00;
		fclose(image);
		vapiPrint("Floppy disk removed.\n");
	} else
		vapiPrint("Cannot write floppy image to '%s'.\n", fname);
}

/* Platform Related */
#include "win32app.h"

void vapiSleep(unsigned int milisec) {win32appSleep(milisec);}
void vapiDisplaySetScreen() {win32appDisplaySetScreen();}
void vapiDisplayPaint() {win32appDisplayPaint();}

void vapiStartDisplay() {win32appStartDisplay();}
void vapiStartKernel() {win32appStartKernel();}
