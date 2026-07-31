/* Copyright 2012-2014 Neko. */

#ifndef NXVM_VDEBUG_H
#define NXVM_VDEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vglobal.h"

#define NXVM_DEVICE_DEBUG "Unknown Hardware Debugger"

typedef struct {
    FILE *recordFile; /* pointer to dump file */
} t_debug_connect;

typedef struct {
    t_bool flagBreak; /* breakpoint set (1) or not (0) */
    t_bool flagBreak32;
    t_bool flagTrace; /* tracer set(1) or not (0) */
    t_nubitcc breakCount, traceCount;
    t_nubit16 breakCS, breakIP;
    t_nubit32 breakLinear;
} t_debug_data;

typedef struct {
    t_debug_data data;
    t_debug_connect connect;
} t_debug;

extern t_debug vdebug;

void vdebugInit();
void vdebugReset();
void vdebugRefresh();
void vdebugFinal();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
