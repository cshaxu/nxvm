/* This file is a part of NXVM project. */

#include "stdio.h"
#include "stdarg.h"
#include "string.h"
#include "memory.h"

#include "vmachine.h"
#include "vapi.h"
#include "debug/dasm.h"

/* Standard C Library */
char* STRCAT(char *_Dest, const char *_Source)
{return strcat(_Dest, _Source);}
char* STRCPY(char *_Dest, const char *_Source)
{return strcpy(_Dest, _Source);}
char* STRTOK(char *_Str, const char *_Delim)
{return strtok(_Str, _Delim);}
int STRCMP(const char *_Str1, const char *_Str2)
{return strcmp(_Str1, _Str2);}
int SPRINTF(char *_Dest, const char *_Format, ...)
{
	int nWrittenBytes = 0;
	va_list arg_ptr;
	va_start(arg_ptr, _Format);
	nWrittenBytes = vsprintf(_Dest, _Format, arg_ptr);
	va_end(arg_ptr);
	return nWrittenBytes;
}
FILE* FOPEN(const char *_Filename, const char *_Mode)
{return fopen(_Filename, _Mode);}
char* FGETS(char *_Buf, int _MaxCount, FILE *_File)
{return fgets(_Buf, _MaxCount, _File);}

/* General Functions */
t_nubit32 vapiPrint(const t_string format, ...)
{
	t_nubit32 nWrittenBytes = 0;
	va_list arg_ptr;
	va_start(arg_ptr, format);
	nWrittenBytes = vfprintf(stdout, format,arg_ptr);
	//nWrittenBytes = vsprintf(stringBuffer,format,arg_ptr);
	va_end(arg_ptr);
	fflush(stdout);
	return nWrittenBytes;
}
void vapiPrintIns(t_nubit16 segment, t_nubit16 offset, t_string ins)
{
	vapiPrint("%04X:%04X  %s\n",segment,offset,ins);
}

/* Record */

t_apirecord vapirecord;

#define _expression "cs:ip=%04x:%04x opcode=%02x %02x %02x %02x %02x %02x %02x %02x \
ax=%04x bx=%04x cx=%04x dx=%04x sp=%04x bp=%04x si=%04x di=%04x ds=%04x es=%04x ss=%04x \
of=%1x sf=%1x zf=%1x cf=%1x af=%1x pf=%1x df=%1x if=%1x tf=%1x %s\n"
#define _rec (vapirecord.rec[(i + vapirecord.start) % VAPI_RECORD_SIZE])
#define _recpu     (_rec.rcpu)
#define _restmt    (_rec.stmt)
#define _rec_of    (GetBit(_rec.rcpu.eflags, VCPU_EFLAGS_OF))
#define _rec_sf    (GetBit(_rec.rcpu.eflags, VCPU_EFLAGS_SF))
#define _rec_zf    (GetBit(_rec.rcpu.eflags, VCPU_EFLAGS_ZF))
#define _rec_cf    (GetBit(_rec.rcpu.eflags, VCPU_EFLAGS_CF))
#define _rec_af    (GetBit(_rec.rcpu.eflags, VCPU_EFLAGS_AF))
#define _rec_pf    (GetBit(_rec.rcpu.eflags, VCPU_EFLAGS_PF))
#define _rec_df    (GetBit(_rec.rcpu.eflags, VCPU_EFLAGS_DF))
#define _rec_tf    (GetBit(_rec.rcpu.eflags, VCPU_EFLAGS_TF))
#define _rec_if    (GetBit(_rec.rcpu.eflags, VCPU_EFLAGS_IF))
#define _rec_ptr_last ((vapirecord.start + vapirecord.size) % VAPI_RECORD_SIZE)

void vapiRecordDump(const t_string fname)
{
	t_nubitcc i = 0, j;
	FILE *dump = FOPEN(fname, "w");
	if (!dump) {
		vapiPrint("ERROR:\tcannot write dump file.\n");
		return;
	}
	if (!vapirecord.size) {
		vapiPrint("ERROR:\tno record to dump.\n");
		return;
	}
	while (i < vapirecord.size) {
		_restmt[strlen(_restmt) - 1] = 0;
		for (j = 0;j < strlen(_restmt);++j)
			if (_restmt[j] == '\n') _restmt[j] = ' ';
		fprintf(dump, _expression,
			_recpu.cs, _recpu.eip,
			vramVarByte(_recpu.cs,_recpu.eip+0),vramVarByte(_recpu.cs,_recpu.eip+1),
			vramVarByte(_recpu.cs,_recpu.eip+2),vramVarByte(_recpu.cs,_recpu.eip+3),
			vramVarByte(_recpu.cs,_recpu.eip+4),vramVarByte(_recpu.cs,_recpu.eip+5),
			vramVarByte(_recpu.cs,_recpu.eip+6),vramVarByte(_recpu.cs,_recpu.eip+7),
			_recpu.ax,_recpu.bx,_recpu.cx,_recpu.dx,
			_recpu.sp,_recpu.bp,_recpu.si,_recpu.di,
			_recpu.ds,_recpu.es,_recpu.ss,
			_rec_of,_rec_sf,_rec_zf,_rec_cf,
			_rec_af,_rec_pf,_rec_df,_rec_if,_rec_tf,_restmt);
		++i;
	}
	vapiPrint("Record dumped to '%s'.\n", fname);
	fclose(dump);
}
void vapiRecordStart()
{
	vapirecord.start = 0;
	vapirecord.size = 0;
}
void vapiRecordExec()
{
#if VAPI_RECORD_SELECT_FIRST == 1
	if (vapirecord.size == VAPI_RECORD_SIZE) {
		vmachine.flagrecord = 0x00;
		return;
	}
#endif
	vapirecord.rec[_rec_ptr_last].rcpu = vcpu;
	dasm(vapirecord.rec[_rec_ptr_last].stmt, vcpu.cs, vcpu.eip, 0x00);
	if (vapirecord.size == VAPI_RECORD_SIZE) vapirecord.start++;
	else vapirecord.size++;
}
void vapiRecordEnd() {}

/* Disk */
#include "vfdd.h"
void vapiFloppyInsert(const t_string fname)
{
	t_nubitcc count;
	FILE *image = FOPEN(fname, "rb");
	if (image && vfdd.base) {
		count = fread((void *)vfdd.base, sizeof(t_nubit8), vfddGetImageSize, image);
		vfdd.flagexist = 0x01;
		fclose(image);
		vapiPrint("Floppy disk inserted.\n");
	} else
		vapiPrint("Cannot read floppy image from '%s'.\n", fname);
}
void vapiFloppyRemove(const t_string fname)
{
	t_nubitcc count;
	FILE *image;
	if (fname) {
		image = FOPEN(fname, "wb");
		if(image) {
			if (!vfdd.flagro)
				count = fwrite((void *)vfdd.base, sizeof(t_nubit8), vfddGetImageSize, image);
			vfdd.flagexist = 0x00;
			fclose(image);
		} else {
			vapiPrint("Cannot write floppy image to '%s'.\n", fname);
			return;
		}
	}
	vfdd.flagexist = 0x00;
	memset((void *)vfdd.base, 0x00, vfddGetImageSize);
	vapiPrint("Floppy disk removed.\n");
}
#include "vhdd.h"
void vapiHardDiskInsert(const t_string fname)
{
	t_nubitcc count;
	FILE *image = FOPEN(fname, "rb");
	if (image) {
		fseek(image, 0, SEEK_END);
		count = ftell(image);
		vhdd.ncyl = (t_nubit16)(count / vhdd.nhead / vhdd.nsector / vhdd.nbyte);
		fseek(image, 0, SEEK_SET);
		vhddAlloc();
		count = fread((void *)vhdd.base, sizeof(t_nubit8), vhddGetImageSize, image);
		vhdd.flagexist = 0x01;
		fclose(image);
		vapiPrint("Hard disk connected.\n");
	} else
		vapiPrint("Cannot read hard disk image from '%s'.\n", fname);
}
void vapiHardDiskRemove(const t_string fname)
{
	t_nubitcc count;
	FILE *image;
	if (fname) {
		image = FOPEN(fname, "wb");
		if(image) {
			if (!vhdd.flagro)
				count = fwrite((void *)vhdd.base, sizeof(t_nubit8), vhddGetImageSize, image);
			vhdd.flagexist = 0x00;
			fclose(image);
		} else {
			vapiPrint("Cannot write hard disk image to '%s'.\n", fname);
			return;
		}
	}
	vhdd.flagexist = 0x00;
	memset((void *)vhdd.base, 0x00, vhddGetImageSize);
	vapiPrint("Hard disk removed.\n");
}

/* Platform Related */
#if VGLOBAL_PLATFORM == VGLOBAL_VAR_WIN32
	#include "system/win32.h"
	void vapiSleep(t_nubit32 milisec) {win32Sleep(milisec);}
	void vapiDisplaySetScreen() {win32DisplaySetScreen(vmachine.flagmode);}
	void vapiDisplayPaint() {win32DisplayPaint(vmachine.flagmode);}
	void vapiStartMachine() {win32StartMachine(vmachine.flagmode);}
#elif VGLOBAL_PLATFORM == VGLOBAL_VAR_LINUX
	#include "system/linux.h"
	void vapiSleep(t_nubit32 milisec) {linuxSleep(milisec);}
	void vapiDisplaySetScreen() {linuxDisplaySetScreen();}
	void vapiDisplayPaint() {linuxDisplayPaint(0x01);}
	void vapiStartMachine() {linuxStartMachine();}
#endif

void vapiInit() {memset(&vapirecord, 0x00, sizeof(t_apirecord));}
void vapiFinal() {}
