/* This file is a part of NXVM project. */

#include "stdio.h"
#include "stdlib.h"
#include "memory.h"

#include "vfdd.h"
#include "system/vapi.h"

t_fdd vfdd;

t_bool vfddRead(t_nubit8 *cyl,t_nubit8 *head,t_nubit8 *sector,t_vaddrcc memloc,t_nubit8 count)
{
	if((*cyl >= VFDD_CYLINDERS) || (*head >= VFDD_HEADS) || ((*sector < 0x01) && (*sector > VFDD_SECTORS)))
		return 1;

	memcpy((void *)memloc,
		(void *)(vfdd.ptrbase+(((*cyl)*VFDD_HEADS+(*head))*VFDD_SECTORS+(*sector)-1)*VFDD_BYTES),
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

	memcpy((void *)(vfdd.ptrbase+(((*cyl)*VFDD_HEADS+(*head))*VFDD_SECTORS+(*sector)-1)*VFDD_BYTES),
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
{memset((void *)(vfdd.ptrbase),fillbyte,0x168000);}

void FDDInit()
{
	t_nubitcc count;
	FILE *imgfile = fopen(vfdd.img,"rb");
	if(imgfile) {
		vfdd.ptrbase = (t_vaddrcc)malloc(0x168000);
		count = fread((void *)vfdd.ptrbase,sizeof(t_nubit8),0x168000,imgfile);
		//vfddFormat(0xf6);
		fclose(imgfile);
		//vapiPrint("File readed to %lx\n",vfdd.ptrbase);
	} else {
		vfdd.ptrbase = 0;
		// Floppy Disk Not Inserted
	}
}
void FDDTerm()
{
	t_nubitcc count;
	FILE *imgfile;
	if(vfdd.ptrbase) {
		imgfile = fopen(vfdd.img,"wb");
		if(imgfile) {
			count = fwrite((void *)vfdd.ptrbase,sizeof(t_nubit8),0x168000,imgfile);
			fclose(imgfile);
		} else vapiPrint("FDD:\tfail to write floppy image file '%s'.\n",vfdd.img);
		free((void *)vfdd.ptrbase);
		vfdd.ptrbase = 0;
	}
}