/* This file is a part of NXVM project. */

#ifndef NXVM_VAPI_H
#define NXVM_VAPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "time.h"
#include "stdarg.h"

#include "vglobal.h"
	
typedef struct {
	char* blockstack[0x100];
	t_nubit8 bid;
	t_nubit8 cid;
} t_api_trace_block;
typedef struct {
	t_api_trace_block callstack[0x100];
	t_nubit8 cid;
	t_bool flagerror;
} t_api_trace_call;

#define VAPI_TRACE_DEBUG 0

void vapiTracePrintCall(t_api_trace_call *rtrace, t_nubit8 cid);
void vapiTracePrintCallStack(t_api_trace_call *rtrace);
void vapiTraceInit(t_api_trace_call *rtrace);
void vapiTraceFinal(t_api_trace_call *rtrace);
void vapiTraceCallBegin(t_api_trace_call *rtrace, t_strptr s);
void vapiTraceCallEnd(t_api_trace_call *rtrace);
void vapiTraceBlockBegin(t_api_trace_call *rtrace, t_strptr s);
void vapiTraceBlockEnd(t_api_trace_call *rtrace);

/* Standard C Library */
struct tm* LOCALTIME(const time_t *_Time);
char* STRCAT(char *_Dest, const char *_Source);
char* STRCPY(char *_Dest, const char *_Source);
char* STRTOK(char *_Str, const char *_Delim);
int   STRCMP(const char *_Str1, const char *_Str2);
int   SPRINTF(char *_Dest, const char *_Format, ...);
FILE* FOPEN(const char *_Filename, const char *_Mode);
char* FGETS(char *_Buf, int _MaxCount, FILE *_File);

/* Internal Library */
void lcase(char *s);
t_nubit32 vapiPrint(const t_strptr format, ...);

/* Device Operations */
void vapiFloppyInsert(const t_strptr fname);
void vapiFloppyRemove(const t_strptr fname);
void vapiHardDiskInsert(const t_strptr fname);
void vapiHardDiskRemove(const t_strptr fname);
void vapiStartMachine();

void vapiSleep(t_nubit32 milisec);
void vapiDisplaySetScreen();
void vapiDisplayPaint();

/* Call Back Functions */
void vapiCallBackMachineRun();
t_nubit8 vapiCallBackMachineGetFlagRun();
void vapiCallBackMachineStart();
void vapiCallBackMachineResume();
void vapiCallBackMachineReset();
void vapiCallBackMachineStop();

void vapiCallBackCpuPrintSreg();
void vapiCallBackCpuPrintCreg();
void vapiCallBackCpuPrintReg();
void vapiCallBackCpuPrintMem();

void vapiCallBackKeyboardClrFlag0();
void vapiCallBackKeyboardClrFlag1();

void vapiCallBackKeyboardSetFlag0Insert();
void vapiCallBackKeyboardSetFlag0CapLck();
void vapiCallBackKeyboardSetFlag0NumLck();
void vapiCallBackKeyboardSetFlag0ScrLck();
void vapiCallBackKeyboardSetFlag0Alt();
void vapiCallBackKeyboardSetFlag0Ctrl();
void vapiCallBackKeyboardSetFlag0LeftShift();
void vapiCallBackKeyboardSetFlag0RightShift();

void vapiCallBackKeyboardClrFlag0Insert();
void vapiCallBackKeyboardClrFlag0CapLck();
void vapiCallBackKeyboardClrFlag0NumLck();
void vapiCallBackKeyboardClrFlag0ScrLck();
void vapiCallBackKeyboardClrFlag0Alt();
void vapiCallBackKeyboardClrFlag0Ctrl();
void vapiCallBackKeyboardClrFlag0LeftShift();
void vapiCallBackKeyboardClrFlag0RightShift();

void vapiCallBackKeyboardSetFlag1Insert();
void vapiCallBackKeyboardSetFlag1CapLck();
void vapiCallBackKeyboardSetFlag1NumLck();
void vapiCallBackKeyboardSetFlag1ScrLck();
void vapiCallBackKeyboardSetFlag1Pause();
void vapiCallBackKeyboardSetFlag1SysRq();
void vapiCallBackKeyboardSetFlag1LeftAlt();
void vapiCallBackKeyboardSetFlag1LeftCtrl();

void vapiCallBackKeyboardClrFlag1Insert();
void vapiCallBackKeyboardClrFlag1CapLck();
void vapiCallBackKeyboardClrFlag1NumLck();
void vapiCallBackKeyboardClrFlag1ScrLck();
void vapiCallBackKeyboardClrFlag1Pause();
void vapiCallBackKeyboardClrFlag1SysRq();
void vapiCallBackKeyboardClrFlag1LeftAlt();
void vapiCallBackKeyboardClrFlag1LeftCtrl();

t_bool vapiCallBackKeyboardGetFlag0NumLock();
t_bool vapiCallBackKeyboardGetFlag0CapsLock();
t_bool vapiCallBackKeyboardGetFlag0Shift();
t_bool vapiCallBackKeyboardGetFlag0Alt();
t_bool vapiCallBackKeyboardGetFlag0Ctrl();
void vapiCallBackKeyboardRecvKeyPress(t_nubit16 code);

t_bool vapiCallBackDisplayGetCursorVisible();
t_bool vapiCallBackDisplayGetCursorChange();
t_bool vapiCallBackDisplayGetBufferChange();
t_nubit16 vapiCallBackDisplayGetRowSize();
t_nubit16 vapiCallBackDisplayGetColSize();
t_nubit8  vapiCallBackDisplayGetCursorTop();
t_nubit8  vapiCallBackDisplayGetCursorBottom();
t_nubit8  vapiCallBackDisplayGetCurrentCursorPosX();
t_nubit8  vapiCallBackDisplayGetCurrentCursorPosY();
t_nubit8  vapiCallBackDisplayGetCurrentChar(t_nubit8 x, t_nubit8 y);
t_nubit8  vapiCallBackDisplayGetCurrentCharProp(t_nubit8 x, t_nubit8 y);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
