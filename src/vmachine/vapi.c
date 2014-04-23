/* This file is a part of NXVM project. */

#include "string.h"

#include "vmachine.h"
#include "vapi.h"

void vapiTracePrintCall(t_api_trace_call *rtrace, t_nubit8 cid)
{
	t_nubit8 i;
	for (i = 0;i < rtrace->callstack[cid].bid;++i) {
		vapiPrint("%s", rtrace->callstack[cid].blockstack[i]);
		if (i != rtrace->callstack[cid].bid - 1) vapiPrint("::");
	}
	vapiPrint("\n");
}
void vapiTracePrintCallStack(t_api_trace_call *rtrace)
{
	t_nubit8 i;
	if (rtrace->cid) {
		for (i = rtrace->cid - 1;i > 0;--i) {
			vapiTracePrintCall(rtrace, i);
		}
		vapiTracePrintCall(rtrace, 0);
	}
}
void vapiTraceInit(t_api_trace_call *rtrace)
{
	rtrace->cid = 0x00;
	rtrace->flagerror = 0;
}
void vapiTraceFinal(t_api_trace_call *rtrace)
{
	if (!rtrace->flagerror && rtrace->cid) {
		vapiPrint("trace_final: call stack is not balanced. (stack: %d, call: %d, block: %d)\n",
			rtrace->cid, rtrace->callstack[rtrace->cid].cid, rtrace->callstack[rtrace->cid].bid);
		rtrace->flagerror = 1;
	}
	if (rtrace->flagerror)
		vapiTracePrintCallStack(rtrace);
	rtrace->cid = 0x00;
	rtrace->flagerror = 0;
}
void vapiTraceCallBegin(t_api_trace_call *rtrace, t_strptr s)
{
	if (rtrace->flagerror) return;
	if (rtrace->cid < 0xff) {
	#if VAPI_TRACE_DEBUG == 1
		vapiPrint("enter call(%d): %s\n", rtrace->cid, s);
	#endif
		rtrace->callstack[rtrace->cid].blockstack[0] = s;
		rtrace->callstack[rtrace->cid].bid = 1;
		rtrace->callstack[rtrace->cid].cid = rtrace->cid;
		rtrace->cid++;
	} else {
		vapiPrint("trace_call_begin: call stack is full.\n");
		rtrace->flagerror = 1;
	}
}
void vapiTraceCallEnd(t_api_trace_call *rtrace)
{
	if (rtrace->flagerror) return;
	if (rtrace->cid) {
		rtrace->cid--;
	#if VAPI_TRACE_DEBUG == 1
		vapiPrint("leave call(%d): %s\n", rtrace->cid,
			rtrace->callstack[rtrace->cid].blockstack[0]);
	#endif
		if (rtrace->callstack[rtrace->cid].bid != 1 ||
			rtrace->callstack[rtrace->cid].cid != rtrace->cid) {
			vapiPrint("trace_call_end: call stack is not balanced. (stack: %d, call: %d, block: %d)\n",
				rtrace->cid, rtrace->callstack[rtrace->cid].cid, rtrace->callstack[rtrace->cid].bid);
			rtrace->cid++;
			rtrace->flagerror = 1;
		}
	} else {
		vapiPrint("trace_call_end: call stack is empty.\n");
		rtrace->flagerror = 1;
	}
}
void vapiTraceBlockBegin(t_api_trace_call *rtrace, t_strptr s)
{
	if (rtrace->flagerror) return;
	if (rtrace->callstack[rtrace->cid - 1].bid < 0xff) {
#if VAPI_TRACE_DEBUG == 1
		vapiPrint("enter block(%d): %s\n", rtrace->callstack[rtrace->cid - 1].bid, s);
#endif
		rtrace->callstack[rtrace->cid - 1].blockstack[rtrace->callstack[rtrace->cid - 1].bid++] = s;
	} else {
		vapiPrint("trace_block_begin: block stack is full.\n");
		rtrace->flagerror = 1;
	}
}
void vapiTraceBlockEnd(t_api_trace_call *rtrace)
{
	if (rtrace->flagerror) return;
	if (rtrace->callstack[rtrace->cid - 1].bid) {
		rtrace->callstack[rtrace->cid - 1].bid--;
#if VAPI_TRACE_DEBUG == 1
		vapiPrint("leave block(%d): %s\n",
			rtrace->callstack[rtrace->cid - 1].bid,
			rtrace->callstack[rtrace->cid - 1].blockstack[rtrace->callstack[rtrace->cid - 1].bid]);
#endif
	} else {
		vapiPrint("trace_block_end: block stack is empty.\n");
		rtrace->flagerror = 1;
	}
}


/* Standard C Library */
struct tm* LOCALTIME(const time_t *_Time)
{
	return localtime(_Time);
}
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
void lcase(char *s)
{
	int i = 0;
	if(s[0] == '\'') return;
	while(s[i] != '\0') {
		if(s[i] == '\n') s[i] = '\0';
		else if(s[i] > 0x40 && s[i] < 0x5b)
			s[i] += 0x20;
		i++;
	}
}
t_nubit32 vapiPrint(const t_strptr format, ...)
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

/* Disk */
#include "vfdd.h"
void vapiFloppyInsert(const t_strptr fname)
{
	t_nubitcc count;
	FILE *image = FOPEN(fname, "rb");
	if (image && vfdd.base) {
		count = fread((void *)vfdd.base, sizeof(t_nubit8), vfddGetImageSize, image);
		vfdd.flagexist = 1;
		fclose(image);
		vapiPrint("Floppy disk inserted.\n");
	} else
		vapiPrint("Cannot read floppy image from '%s'.\n", fname);
}
void vapiFloppyRemove(const t_strptr fname)
{
	t_nubitcc count;
	FILE *image;
	if (fname) {
		image = FOPEN(fname, "wb");
		if(image) {
			if (!vfdd.flagro)
				count = fwrite((void *)vfdd.base, sizeof(t_nubit8), vfddGetImageSize, image);
			vfdd.flagexist = 0;
			fclose(image);
		} else {
			vapiPrint("Cannot write floppy image to '%s'.\n", fname);
			return;
		}
	}
	vfdd.flagexist = 0;
	memset((void *)vfdd.base, 0x00, vfddGetImageSize);
	vapiPrint("Floppy disk removed.\n");
}
#include "vhdd.h"
void vapiHardDiskInsert(const t_strptr fname)
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
		vhdd.flagexist = 1;
		fclose(image);
		vapiPrint("Hard disk connected.\n");
	} else
		vapiPrint("Cannot read hard disk image from '%s'.\n", fname);
}
void vapiHardDiskRemove(const t_strptr fname)
{
	t_nubitcc count;
	FILE *image;
	if (fname) {
		image = FOPEN(fname, "wb");
		if(image) {
			if (!vhdd.flagro)
				count = fwrite((void *)vhdd.base, sizeof(t_nubit8), vhddGetImageSize, image);
			vhdd.flagexist = 0;
			fclose(image);
		} else {
			vapiPrint("Cannot write hard disk image to '%s'.\n", fname);
			return;
		}
	}
	vhdd.flagexist = 0;
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

void vapiInit() {}
void vapiFinal() {}
