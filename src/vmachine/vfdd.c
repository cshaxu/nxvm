/* This file is a part of NXVM project. */

#include "stdlib.h"
#include "memory.h"

#include "vfdd.h"

t_fdd vfdd;

t_bool vfddRead(t_nubit8 *cyl,t_nubit8 *head,t_nubit8 *sector,t_vaddrcc memloc,t_nubit8 count)
{
	if((*cyl >= VFDD_CYLINDERS) || (*head >= VFDD_HEADS) || ((*sector < 0x01) && (*sector > VFDD_SECTORS)))
		return 1;

	memcpy((void *)memloc,
		(void *)(vfdd.base+(((*cyl)*VFDD_HEADS+(*head))*VFDD_SECTORS+(*sector)-1)*VFDD_BYTES),
		count*VFDD_BYTES);

	*cyl = (*cyl+count/(VFDD_HEADS*VFDD_SECTORS))%VFDD_CYLINDERS;
	count %= VFDD_HEADS*VFDD_SECTORS;
	*head = (*head+count/(VFDD_SECTORS))%VFDD_HEADS;
	count %= VFDD_SECTORS;
	*sector = (*sector+count)%VFDD_SECTORS;
	return 0;
}
t_bool vfddWrite(t_nubit8 *cyl,t_nubit8 *head,t_nubit8 *sector,t_vaddrcc memloc,t_nubit8 count)
{
	if((*cyl >= VFDD_CYLINDERS) || (*head >= VFDD_HEADS) || ((*sector < 0x01) && (*sector > VFDD_SECTORS)))
		return 1;

	memcpy((void *)(vfdd.base+(((*cyl)*VFDD_HEADS+(*head))*VFDD_SECTORS+(*sector)-1)*VFDD_BYTES),
		(void *)memloc,
		count*VFDD_BYTES);

	*cyl = (*cyl+count/(VFDD_HEADS*VFDD_SECTORS))%VFDD_CYLINDERS;
	count %= VFDD_HEADS*VFDD_SECTORS;
	*head = (*head+count/(VFDD_SECTORS))%VFDD_HEADS;
	count %= VFDD_SECTORS;
	*sector = (*sector+count)%VFDD_SECTORS;
	return 0;
}
void vfddFormat(t_nubit8 fillbyte)
{
	memset((void *)(vfdd.base), fillbyte, 0x168000);
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