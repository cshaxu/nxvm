/* Copyright 2012-2014 Neko. */

#ifndef NXVM_DEVICE_H
#define NXVM_DEVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vglobal.h"

typedef struct {
    t_bool flagFlip;  /* flag flips when device thread is created  */
    t_bool flagRun;   /* device thread is running (1) or not (0) */
    t_bool flagReset; /* reset command is issued or not */
} t_device;

extern t_device device;

/* Connection Functions Invoked By Other Modules */

/* Bios Settings */
void deviceConnectBiosSetBoot(t_bool flagHdd);
t_bool deviceConnectBiosGetBoot();

/* RAM Operations */
void deviceConnectRamAllocate(t_nubitcc newsize);

/* Disk Drive Operations */
void deviceConnectFloppyCreate();
t_bool deviceConnectFloppyInsert(const t_strptr fileName);
t_bool deviceConnectFloppyRemove(const t_strptr fileName);
void deviceConnectHardDiskCreate(t_nubit16 ncyl);
t_bool deviceConnectHardDiskInsert(const t_strptr fileName);
t_bool deviceConnectHardDiskRemove(const t_strptr fileName);

/* Keyboard Operations */
void deviceConnectKeyboardClrFlag0();
void deviceConnectKeyboardClrFlag1();

void deviceConnectKeyboardSetFlag0Insert();
void deviceConnectKeyboardSetFlag0CapLck();
void deviceConnectKeyboardSetFlag0NumLck();
void deviceConnectKeyboardSetFlag0ScrLck();
void deviceConnectKeyboardSetFlag0Alt();
void deviceConnectKeyboardSetFlag0Ctrl();
void deviceConnectKeyboardSetFlag0LeftShift();
void deviceConnectKeyboardSetFlag0RightShift();

void deviceConnectKeyboardClrFlag0Insert();
void deviceConnectKeyboardClrFlag0CapLck();
void deviceConnectKeyboardClrFlag0NumLck();
void deviceConnectKeyboardClrFlag0ScrLck();
void deviceConnectKeyboardClrFlag0Alt();
void deviceConnectKeyboardClrFlag0Ctrl();
void deviceConnectKeyboardClrFlag0LeftShift();
void deviceConnectKeyboardClrFlag0RightShift();

void deviceConnectKeyboardSetFlag1Insert();
void deviceConnectKeyboardSetFlag1CapLck();
void deviceConnectKeyboardSetFlag1NumLck();
void deviceConnectKeyboardSetFlag1ScrLck();
void deviceConnectKeyboardSetFlag1Pause();
void deviceConnectKeyboardSetFlag1SysRq();
void deviceConnectKeyboardSetFlag1LeftAlt();
void deviceConnectKeyboardSetFlag1LeftCtrl();

void deviceConnectKeyboardClrFlag1Insert();
void deviceConnectKeyboardClrFlag1CapLck();
void deviceConnectKeyboardClrFlag1NumLck();
void deviceConnectKeyboardClrFlag1ScrLck();
void deviceConnectKeyboardClrFlag1Pause();
void deviceConnectKeyboardClrFlag1SysRq();
void deviceConnectKeyboardClrFlag1LeftAlt();
void deviceConnectKeyboardClrFlag1LeftCtrl();

t_bool deviceConnectKeyboardGetFlag0NumLock();
t_bool deviceConnectKeyboardGetFlag0CapsLock();
t_bool deviceConnectKeyboardGetFlag0Shift();
t_bool deviceConnectKeyboardGetFlag0Alt();
t_bool deviceConnectKeyboardGetFlag0Ctrl();
void deviceConnectKeyboardRecvKeyPress(t_nubit16 code);

/* Display Operations */
t_bool deviceConnectDisplayGetCursorVisible();
t_bool deviceConnectDisplayGetCursorChange();
t_bool deviceConnectDisplayGetBufferChange();
t_nubit16 deviceConnectDisplayGetRowSize();
t_nubit16 deviceConnectDisplayGetColSize();
t_nubit8 deviceConnectDisplayGetCursorTop();
t_nubit8 deviceConnectDisplayGetCursorBottom();
t_nubit8 deviceConnectDisplayGetCurrentCursorPosX();
t_nubit8 deviceConnectDisplayGetCurrentCursorPosY();
t_nubit8 deviceConnectDisplayGetCurrentChar(t_nubit8 x, t_nubit8 y);
t_nubit8 deviceConnectDisplayGetCurrentCharProp(t_nubit8 x, t_nubit8 y);

/* Device Status Printer */
void devicePrintCpuSreg();
void devicePrintCpuCreg();
void devicePrintCpuReg();
void devicePrintCpuMem();
void devicePrintPic();
void devicePrintPit();
void devicePrintDma();
void devicePrintFdc();
void devicePrintFdd();
void devicePrintBios();
void devicePrintDebug();
void devicePrintMachine();

/* Device Thread Controller */
void deviceStart();
void deviceReset();
void deviceStop();

void deviceInit();
void deviceFinal();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
