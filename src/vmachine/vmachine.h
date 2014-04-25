/* Copyright 2012-2014 Neko. */

#ifndef NXVM_VMACHINE_H
#define NXVM_VMACHINE_H

#ifdef __cplusplus
extern "C" {
#endif

#define VMACHINE_DEBUG

#define NXVM_DEVICE_MACHINE "IBM PC/AT"

#include "vglobal.h"

#define VMACHINE_DEVICE_INIT    0
#define VMACHINE_DEVICE_RESET   1
#define VMACHINE_DEVICE_REFRESH 2
#define VMACHINE_DEVICE_FINAL   3

typedef struct {
	t_bool    flagrun;    /* vmachine is running (1) or not running (0) */
	t_bool    flagmode;   /* mode flag: console (0) or application (1) */
	t_bool    flagboot;   /* boot from floppy (0) or hard disk (1) */
	t_bool    flagreset;  /* reset command is issued or not */
	t_faddrcc deviceTable[4][0x100]; /* device init/reset/refresh/final functions */
	t_nubitcc numDevices; /* numebr of registered devices */
} t_machine;

extern t_machine vmachine;

void vmachineStart();
void vmachineReset();
void vmachineStop();
void vmachineResume();

void vmachineInit();
void vmachineFinal();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
