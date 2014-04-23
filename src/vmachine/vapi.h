/* This file is a part of NXVM project. */

#ifndef NXVM_VAPI_H
#define NXVM_VAPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "stdarg.h"

#include "vglobal.h"

char* STRCAT(char *_Dest, const char *_Source);
char* STRCPY(char *_Dest, const char *_Source);
char* STRTOK(char *_Str, const char *_Delim);
int   STRCMP(const char *_Str1, const char *_Str2);
int   SPRINTF(char *_Dest, const char *_Format, ...);
FILE* FOPEN(const char *_Filename, const char *_Mode);
char* FGETS(char *_Buf, int _MaxCount, FILE *_File);

t_nubit32 vapiPrint(const t_string format, ...);

void vapiFloppyInsert(const t_string fname);
void vapiFloppyRemove(const t_string fname);
void vapiHardDiskInsert(const t_string fname);
void vapiHardDiskRemove(const t_string fname);
void vapiStartMachine();

void vapiSleep(t_nubit32 milisec);
void vapiDisplaySetScreen();
void vapiDisplayPaint();

void vapiCallBackDebugPrintRegs(t_bool bit32);

void vapiCallBackMachineRun();
t_nubit8 vapiCallBackMachineGetFlagRun();
void vapiCallBackMachineStart();
void vapiCallBackMachineResume();
void vapiCallBackMachineReset();
void vapiCallBackMachineStop();

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
