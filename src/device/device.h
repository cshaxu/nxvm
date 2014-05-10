/* Copyright 2012-2014 Neko. */

#ifndef NXVM_DEVICE_H
#define NXVM_DEVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../global.h"

typedef struct {
    int flagFlip;  /* flag flips when device thread is created  */
    int flagRun;   /* device thread is running (1) or not (0) */
    int flagReset; /* reset command is issued or not */
} t_device;

extern t_device device;

/* Connection Functions Invoked By Other Modules */

/* Debug Controls */
void deviceConnectDebugSetBreak(uint16_t breakCS, uint16_t breakIP);
void deviceConnectDebugClearBreak();
void deviceConnectDebugSetBreak32(uint32_t breakLinear);
void deviceConnectDebugClearBreak32();
size_t deviceConnectDebugGetBreakCount();
void deviceConnectDebugSetTrace(size_t traceCount);
void deviceConnectDebugClearTrace();
void deviceConnectDebugRecordStart(const char *fileName);
void deviceConnectDebugRecordStop();

/* Bios Settings */
void deviceConnectBiosSetBoot(int flagHdd);
int deviceConnectBiosGetBoot();

/* CPU Operations */
int deviceConnectCpuReadLinear(uint32_t linear, void *rdest, uint8_t size);
int deviceConnectCpuWriteLinear(uint32_t linear, void *rsrc, uint8_t size);
int deviceConnectCpuLoadES(uint16_t selector);
int deviceConnectCpuLoadCS(uint16_t selector);
int deviceConnectCpuLoadSS(uint16_t selector);
int deviceConnectCpuLoadDS(uint16_t selector);
int deviceConnectCpuLoadFS(uint16_t selector);
int deviceConnectCpuLoadGS(uint16_t selector);
void deviceConnectCpuSetWR(uint32_t linear);
void deviceConnectCpuSetWW(uint32_t linear);
void deviceConnectCpuSetWE(uint32_t linear);
void deviceConnectCpuClearWR();
void deviceConnectCpuClearWW();
void deviceConnectCpuClearWE();

int deviceConnectCpuGetCsDefSize();
uint32_t deviceConnectCpuGetCsBase();

int deviceConnectCpuGetCF();
int deviceConnectCpuGetPF();
int deviceConnectCpuGetAF();
int deviceConnectCpuGetZF();
int deviceConnectCpuGetSF();
int deviceConnectCpuGetTF();
int deviceConnectCpuGetIF();
int deviceConnectCpuGetDF();
int deviceConnectCpuGetOF();
int deviceConnectCpuGetNT();
int deviceConnectCpuGetRF();
int deviceConnectCpuGetVM();

void deviceConnectCpuSetCF();
void deviceConnectCpuSetPF();
void deviceConnectCpuSetAF();
void deviceConnectCpuSetZF();
void deviceConnectCpuSetSF();
void deviceConnectCpuSetTF();
void deviceConnectCpuSetIF();
void deviceConnectCpuSetDF();
void deviceConnectCpuSetOF();
void deviceConnectCpuSetNT();
void deviceConnectCpuSetRF();
void deviceConnectCpuSetVM();

void deviceConnectCpuClearCF();
void deviceConnectCpuClearPF();
void deviceConnectCpuClearAF();
void deviceConnectCpuClearZF();
void deviceConnectCpuClearSF();
void deviceConnectCpuClearTF();
void deviceConnectCpuClearIF();
void deviceConnectCpuClearDF();
void deviceConnectCpuClearOF();
void deviceConnectCpuClearNT();
void deviceConnectCpuClearRF();
void deviceConnectCpuClearVM();

#define _eax    (*(uint32_t *)(deviceConnectCpuGetRefEAX()))
#define _ecx    (*(uint32_t *)(deviceConnectCpuGetRefECX()))
#define _edx    (*(uint32_t *)(deviceConnectCpuGetRefEDX()))
#define _ebx    (*(uint32_t *)(deviceConnectCpuGetRefEBX()))
#define _esp    (*(uint32_t *)(deviceConnectCpuGetRefESP()))
#define _ebp    (*(uint32_t *)(deviceConnectCpuGetRefEBP()))
#define _esi    (*(uint32_t *)(deviceConnectCpuGetRefESI()))
#define _edi    (*(uint32_t *)(deviceConnectCpuGetRefEDI()))
#define _eflags (*(uint32_t *)(deviceConnectCpuGetRefEFLAGS()))
#define _eip    (*(uint32_t *)(deviceConnectCpuGetRefEIP()))
#define _cr(i)  (*(uint32_t *)(deviceConnectCpuGetRefCR(i)))

#define _ax     (*(uint16_t *)(deviceConnectCpuGetRefAX()))
#define _cx     (*(uint16_t *)(deviceConnectCpuGetRefCX()))
#define _dx     (*(uint16_t *)(deviceConnectCpuGetRefDX()))
#define _bx     (*(uint16_t *)(deviceConnectCpuGetRefBX()))
#define _sp     (*(uint16_t *)(deviceConnectCpuGetRefSP()))
#define _bp     (*(uint16_t *)(deviceConnectCpuGetRefBP()))
#define _si     (*(uint16_t *)(deviceConnectCpuGetRefSI()))
#define _di     (*(uint16_t *)(deviceConnectCpuGetRefDI()))
#define _flags  (*(uint16_t *)(deviceConnectCpuGetRefFLAS()))
#define _ip     (*(uint16_t *)(deviceConnectCpuGetRefIP()))

#define _es     (*(uint16_t *)(deviceConnectCpuGetRefES()))
#define _cs     (*(uint16_t *)(deviceConnectCpuGetRefCS()))
#define _ss     (*(uint16_t *)(deviceConnectCpuGetRefSS()))
#define _ds     (*(uint16_t *)(deviceConnectCpuGetRefDS()))
#define _fs     (*(uint16_t *)(deviceConnectCpuGetRefFS()))
#define _gs     (*(uint16_t *)(deviceConnectCpuGetRefGS()))

#define _ah     (*(uint8_t *)(deviceConnectCpuGetRefAH()))
#define _ch     (*(uint8_t *)(deviceConnectCpuGetRefCH()))
#define _dh     (*(uint8_t *)(deviceConnectCpuGetRefDH()))
#define _bh     (*(uint8_t *)(deviceConnectCpuGetRefBH()))
#define _al     (*(uint8_t *)(deviceConnectCpuGetRefAL()))
#define _cl     (*(uint8_t *)(deviceConnectCpuGetRefCL()))
#define _dl     (*(uint8_t *)(deviceConnectCpuGetRefDL()))
#define _bl     (*(uint8_t *)(deviceConnectCpuGetRefBL()))

void *deviceConnectCpuGetRefEAX();
void *deviceConnectCpuGetRefECX();
void *deviceConnectCpuGetRefEDX();
void *deviceConnectCpuGetRefEBX();
void *deviceConnectCpuGetRefESP();
void *deviceConnectCpuGetRefEBP();
void *deviceConnectCpuGetRefESI();
void *deviceConnectCpuGetRefEDI();
void *deviceConnectCpuGetRefEFLAGS();
void *deviceConnectCpuGetRefEIP();
void *deviceConnectCpuGetRefCR(uint8_t crId);

void *deviceConnectCpuGetRefAX();
void *deviceConnectCpuGetRefCX();
void *deviceConnectCpuGetRefDX();
void *deviceConnectCpuGetRefBX();
void *deviceConnectCpuGetRefSP();
void *deviceConnectCpuGetRefBP();
void *deviceConnectCpuGetRefSI();
void *deviceConnectCpuGetRefDI();
void *deviceConnectCpuGetRefFLAGS();
void *deviceConnectCpuGetRefIP();

void *deviceConnectCpuGetRefES();
void *deviceConnectCpuGetRefCS();
void *deviceConnectCpuGetRefSS();
void *deviceConnectCpuGetRefDS();
void *deviceConnectCpuGetRefFS();
void *deviceConnectCpuGetRefGS();

void *deviceConnectCpuGetRefAH();
void *deviceConnectCpuGetRefCH();
void *deviceConnectCpuGetRefDH();
void *deviceConnectCpuGetRefBH();
void *deviceConnectCpuGetRefAL();
void *deviceConnectCpuGetRefCL();
void *deviceConnectCpuGetRefDL();
void *deviceConnectCpuGetRefBL();

/* RAM Operations */
void deviceConnectRamAllocate(size_t newsize);
void deviceConnectRamRealRead(uint16_t seg, uint16_t off, void *rdest, size_t size);
void deviceConnectRamRealWrite(uint16_t seg, uint16_t off, void *rsrc, size_t size);

/* Port Operations */
void deviceConnectPortRead(uint16_t portId);
void deviceConnectPortWrite(uint16_t portId);
uint32_t deviceConnectPortGetValue();
void deviceConnectPortSetValue(uint32_t value);

/* Disk Drive Operations */
void deviceConnectFloppyCreate();
int deviceConnectFloppyInsert(const char *fileName);
int deviceConnectFloppyRemove(const char *fileName);

void deviceConnectHardDiskCreate(uint16_t ncyl);
int deviceConnectHardDiskInsert(const char *fileName);
int deviceConnectHardDiskRemove(const char *fileName);

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

int deviceConnectKeyboardGetFlag0NumLock();
int deviceConnectKeyboardGetFlag0CapsLock();
int deviceConnectKeyboardGetFlag0Shift();
int deviceConnectKeyboardGetFlag0Alt();
int deviceConnectKeyboardGetFlag0Ctrl();
void deviceConnectKeyboardRecvKeyPress(uint16_t code);

/* Display Operations */
int deviceConnectDisplayGetCursorVisible();
int deviceConnectDisplayGetCursorChange();
int deviceConnectDisplayGetBufferChange();
uint16_t deviceConnectDisplayGetRowSize();
uint16_t deviceConnectDisplayGetColSize();
uint8_t deviceConnectDisplayGetCursorTop();
uint8_t deviceConnectDisplayGetCursorBottom();
uint8_t deviceConnectDisplayGetCurrentCursorPosX();
uint8_t deviceConnectDisplayGetCurrentCursorPosY();
uint8_t deviceConnectDisplayGetCurrentChar(uint8_t x, uint8_t y);
uint8_t deviceConnectDisplayGetCurrentCharProp(uint8_t x, uint8_t y);

/* Device Status Printer */
void devicePrintCpuSreg();
void devicePrintCpuCreg();
void devicePrintCpuReg();
void devicePrintCpuMem();
void devicePrintCpuWatch();
void devicePrintPic();
void devicePrintPit();
void devicePrintDma();
void devicePrintFdc();
void devicePrintFdd();
void devicePrintBios();
void devicePrintMachine();
void devicePrintStatus();

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
