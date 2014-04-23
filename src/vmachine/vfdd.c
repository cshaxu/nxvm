/* This file is a part of NXVM project. */

#include "stdlib.h"
#include "memory.h"

#include "vfdd.h"

t_fdd vfdd;

t_bool vfddRead(t_nubit8 *cyl,t_nubit8 *head,t_nubit8 *sector,t_vaddrcc memloc,t_nubit8 count)
{
	if((*cyl >= VFDD_NCYL) || (*head >= VFDD_NHEAD) || ((*sector < 0x01) && (*sector > vfdd.nsector)))
		return 1;

	memcpy((void *)memloc,
		(void *)(vfdd.base+(((*cyl)*VFDD_NHEAD+(*head))*vfdd.nsector+(*sector)-1)*vfdd.nbyte),
		count*vfdd.nbyte);

	*cyl = (*cyl+count/(VFDD_NHEAD*vfdd.nsector))%VFDD_NCYL;
	count %= VFDD_NHEAD*vfdd.nsector;
	*head = (*head+count/(vfdd.nsector))%VFDD_NHEAD;
	count %= vfdd.nsector;
	*sector = (*sector+count)%vfdd.nsector;
	return 0;
}
t_bool vfddWrite(t_nubit8 *cyl,t_nubit8 *head,t_nubit8 *sector,t_vaddrcc memloc,t_nubit8 count)
{
	if((*cyl >= VFDD_NCYL) || (*head >= VFDD_NHEAD) || ((*sector < 0x01) && (*sector > vfdd.nsector)))
		return 1;

	memcpy((void *)(vfdd.base+(((*cyl)*VFDD_NHEAD+(*head))*vfdd.nsector+(*sector)-1)*vfdd.nbyte),
		(void *)memloc,
		count*vfdd.nbyte);

	*cyl = (*cyl+count/(VFDD_NHEAD*vfdd.nsector))%VFDD_NCYL;
	count %= VFDD_NHEAD*vfdd.nsector;
	*head = (*head+count/(vfdd.nsector))%VFDD_NHEAD;
	count %= vfdd.nsector;
	*sector = (*sector+count)%vfdd.nsector;
	return 0;
}

void vfddFormatTrack(t_nubit8 fillbyte)
{
	vfdd.head   = 0x00;
	vfdd.sector = 0x01;
	vfdd.curr   = vfddSetCURR;
	memset((void *)(vfdd.curr), fillbyte, vfdd.nsector * vfdd.nbyte);
	vfdd.head   = 0x01;
	vfdd.sector = 0x01;
	vfdd.curr   = vfddSetCURR;
	memset((void *)(vfdd.curr), fillbyte, vfdd.nsector * vfdd.nbyte);
}

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