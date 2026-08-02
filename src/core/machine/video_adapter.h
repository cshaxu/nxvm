/* Copyright 2012-2014 Neko. */

#ifndef NXVM_CORE_VIDEO_ADAPTER_H
#define NXVM_CORE_VIDEO_ADAPTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "core/machine/vglobal.h"

#define NXVM_DEVICE_VADP "Unknown Video Adapter"

typedef struct {
    t_bool    flagColor;
    t_nubit8  colSize; /* char per column */
    t_vaddrcc bufcomp[0x00040000]; /* buffer for video memory comparison */
    t_nubit8  oldCurPosX, oldCurPosY;
    t_nubit8  oldCurTop, oldCurBottom;
} t_vadp_data;

typedef struct {
    t_vadp_data data;
} t_vadp;

t_vadp *core_machine_video_adapter_current(void);
void core_machine_video_adapter_bind_live(t_vadp *adapter);
void core_machine_video_adapter_unbind_live(void);
#define vvadp (*core_machine_video_adapter_current())

void vvadpInit();
void vvadpReset();
void vvadpRefresh();
void vvadpFinal();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
