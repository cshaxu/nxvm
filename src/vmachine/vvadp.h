/* This file is a part of NXVM project. */

/* Video Adapter: not implemented yet */

#ifndef NXVM_VVADP_H
#define NXVM_VVADP_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#include "vglobal.h"

typedef struct {
	t_bool    color;
	t_nubit8  colsize; // char per col
	t_vaddrcc bufcomp;
	t_nubit8  oldcurposx,oldcurposy;
	t_nubit8  oldcurtop, oldcurbottom;
} t_vadp;

extern t_vadp vvadp;

void vvadpInit();
void vvadpReset();
void vvadpRefresh();
void vvadpFinal();

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
