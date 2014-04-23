/* This file is a part of NXVM project. */

#include "vdma.h"
#include "vfdd.h"

t_fdd vfdd;

#define IsTrackEnd  (vfdd.sector >= (vfdd.nsector + 1))
#define IsCylHalf   (vfdd.head == 0x00 && IsTrackEnd)
#define IsCylEnd    (vfdd.head == 0x01 && IsTrackEnd)

void vfddTransRead()
{
	if (IsCylEnd) return;
	vlatch.byte = d_nubit8(vfdd.curr);
	vfdd.curr++;
	vfdd.count++;
	if (!(vfdd.count % vfdd.nbyte)) {
		vfdd.sector++;
		if (IsCylHalf) {
			vfdd.sector = 0x01;
			vfdd.head   = 0x01;
		}
		vfddSetPointer;
	}
}
void vfddTransWrite()
{
	if (IsCylEnd) return;
	d_nubit8(vfdd.curr) = vlatch.byte;
	vfdd.curr++;
	vfdd.count++;
	if (!(vfdd.count % vfdd.nbyte)) {
		vfdd.sector++;
		if (IsCylHalf) {
			vfdd.sector = 0x01;
			vfdd.head   = 0x01;
		}
		vfddSetPointer;
	}
}

void vfddFormatTrack(t_nubit8 fillbyte)
{
	if (vfdd.cyl >= vfdd.ncyl) return;
	vfdd.head   = 0x00;
	vfdd.sector = 0x01;
	vfddSetPointer;
	memset((void *)(vfdd.curr), fillbyte, vfdd.nsector * vfdd.nbyte);
	vfdd.head   = 0x01;
	vfdd.sector = 0x01;
	vfddSetPointer;
	memset((void *)(vfdd.curr), fillbyte, vfdd.nsector * vfdd.nbyte);
	vfdd.sector = vfdd.nsector;
}

void vfddRefresh()
{}
void vfddInit()
{
	memset(&vfdd, 0x00, sizeof(t_fdd));
	vfdd.ncyl    = 0x0050;
	vfdd.nhead   = 0x0002;
	vfdd.nsector = 0x0012;
	vfdd.nbyte   = 0x0200;
	vfdd.base    = (t_vaddrcc)malloc(vfddGetImageSize);
	memset((void *)vfdd.base, 0x00, vfddGetImageSize);
}
void vfddReset() {}
void vfddFinal()
{
	if (vfdd.base) free((void *)(vfdd.base));
	vfdd.base = (t_vaddrcc)NULL;
}
