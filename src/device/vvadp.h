/* Copyright 2012-2014 Neko. */

#ifndef NXVM_VVADP_H
#define NXVM_VVADP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vglobal.h"

#define NXVM_DEVICE_VADP "Unknown Video Adapter"

typedef struct {
	t_bool    flagColor;
	t_nubit8  colSize; /* char per column */
	t_vaddrcc bufcomp[0x00040000]; /* buffer for video memory comparison */
	t_nubit8  oldCurPosX, oldCurPosY;
	t_nubit8  oldCurTop, oldCurBottom;
} t_vadp;

extern t_vadp vvadp;

void vvadpRegister();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
