/* This file is a part of NXVM project. */

#include "stdlib.h"
#include "memory.h"

#include "vport.h"
#include "vvadp.h"

t_vadp vvadp;

void vvadpInit()
{
	memset(&vvadp, 0x00, sizeof(t_vadp));
	vvadp.bufcomp = (t_vaddrcc)malloc(0x00040000);
}
void vvadpFinal()
{
	if (vvadp.bufcomp) free((void *)vvadp.bufcomp);
	vvadp.bufcomp = (t_vaddrcc)NULL;
}
void vvadpReset() {}
void vvadpRefresh() {}