/* Copyright 2012-2014 Neko. */

#ifndef NXVM_CORE_VADP_H
#define NXVM_CORE_VADP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "core/machine/vglobal.h"

#define NXVM_DEVICE_VADP "Unknown Video Adapter"

typedef struct t_vadp_data {
    t_bool    flagColor;
    t_nubit8  colSize; /* char per column */
    t_vaddrcc bufcomp[0x00040000]; /* buffer for video memory comparison */
    t_nubit8  oldCurPosX, oldCurPosY;
    t_nubit8  oldCurTop, oldCurBottom;
} t_vadp_data;

typedef struct t_vadp {
    t_vadp_data data;
} t_vadp;

void core_machine_vadp_initialize(t_vadp *adapter);
void core_machine_vadp_reset(t_vadp *adapter);
void core_machine_vadp_refresh(t_vadp *adapter);
void core_machine_vadp_finalize(t_vadp *adapter);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
