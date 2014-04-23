/* This file is a part of NXVM project. */

#include "stdlib.h"
#include "memory.h"

#include "vdma.h"
#include "vfdd.h"

t_fdd vfdd;

#define IsTrackEnd  (vfdd.sector >= (vfdd.nsector + 1))
#define IsCylHalf   (vfdd.head == 0x00 && IsTrackEnd)
#define IsCylEnd    (vfdd.head == 0x01 && IsTrackEnd)

void vfddTransRead()
{
	if (IsCylEnd) return;
	vlatch.byte = *((t_nubit8 *)vfdd.curr);
	vfdd.curr++;
	vfdd.count++;
	if (!(vfdd.count % vfdd.nbyte)) {
		vfdd.sector++;
		if (IsCylHalf) {
			vfdd.sector = 0x01;
			vfdd.head   = 0x01;
		}
		vfddResetCURR;
	}
}
void vfddTransWrite()
{
	if (IsCylEnd) return;
	*((t_nubit8 *)vfdd.curr) = vlatch.byte;
	vfdd.curr++;
	vfdd.count++;
	if (!(vfdd.count % vfdd.nbyte)) {
		vfdd.sector++;
		if (IsCylHalf) {
			vfdd.sector = 0x01;
			vfdd.head   = 0x01;
		}
		vfddResetCURR;
	}
}

void vfddFormatTrack(t_nubit8 fillbyte)
{
	if (vfdd.cyl >= VFDD_NCYL) return;
	vfdd.head   = 0x00;
	vfdd.sector = 0x01;
	vfddResetCURR;
	memset((void *)(vfdd.curr), fillbyte, vfdd.nsector * vfdd.nbyte);
	vfdd.head   = 0x01;
	vfdd.sector = 0x01;
	vfddResetCURR;
	memset((void *)(vfdd.curr), fillbyte, vfdd.nsector * vfdd.nbyte);
	vfdd.sector = vfdd.nsector;
}

void vfddRefresh()
{}
void vfddInit()
{
	memset(&vfdd, 0x00, sizeof(t_fdd));
	vfdd.base = (t_vaddrcc)malloc(0x00168000);
	/* TODO: assert(vfdd.base); */
}
void vfddFinal()
{
	if (vfdd.base) free((void*)vfdd.base);
}