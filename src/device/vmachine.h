/* Copyright 2012-2014 Neko. */

#ifndef NXVM_VMACHINE_H
#define NXVM_VMACHINE_H

#ifdef __cplusplus
extern "C" {
#endif

#define NXVM_DEVICE_MACHINE "IBM PC/AT"

#include "vglobal.h"

typedef struct {
    t_faddrcc deviceTable[4][0x100]; /* device init/reset/refresh/final functions */
    t_nubitcc numDevices; /* numebr of registered devices */
} t_machine;

extern t_machine vmachine;

#define VMACHINE_DEVICE_INIT    0
#define VMACHINE_DEVICE_RESET   1
#define VMACHINE_DEVICE_REFRESH 2
#define VMACHINE_DEVICE_FINAL   3

#define vmachineAddMe vmachineAddDevice((t_faddrcc) init, \
    (t_faddrcc) reset, (t_faddrcc) refresh, (t_faddrcc) final)

void vmachineAddDevice(t_faddrcc fpInit, t_faddrcc fpReset, t_faddrcc fpRefresh, t_faddrcc fpFinal);

void vmachineInit();
void vmachineReset();
void vmachineRefresh();
void vmachineFinal();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
