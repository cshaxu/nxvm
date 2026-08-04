/* Copyright 2012-2014 Neko. */

#ifndef CORE_MACHINE_VADP_H
#define CORE_MACHINE_VADP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"

#define CORE_MACHINE_DEVICE_VADP "Unknown Video Adapter"

typedef struct t_vadp_data {
    type_bool    flagColor;
    type_unsigned_8  colSize; /* C_CHAR per column */
    type_virtual_address bufcomp[0x00040000]; /* buffer for video memory comparison */
    type_unsigned_8  oldCurPosX, oldCurPosY;
    type_unsigned_8  oldCurTop, oldCurBottom;
} t_vadp_data;

typedef struct t_vadp {
    t_vadp_data data;
} t_vadp;

C_VOID core_machine_vadp_initialize(t_vadp *adapter);
C_VOID core_machine_vadp_reset(t_vadp *adapter);
C_VOID core_machine_vadp_refresh(t_vadp *adapter);
C_VOID core_machine_vadp_finalize(t_vadp *adapter);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
