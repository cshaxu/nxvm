/* This file is a part of NXVM project. */

#include "stdio.h"
#include "stdlib.h"
#include "memory.h"

#include "vfdd.h"
#include "system/vapi.h"

t_fdd vfdd;

void FDDInit()
{
	t_nubitcc count;
	FILE *imgfile = fopen(vfdd.img,"rb");
	memset(&vfdd,0,sizeof(t_fdd));
	if(imgfile) {
		vfdd.ptrbase = (t_vaddrcc)malloc(0x168000);
		count = fread((void *)vfdd.ptrbase,sizeof(t_nubit8),0x168000,imgfile);
		fclose(imgfile);
	} else {
		// Floppy Disk Not Inserted
		// vfdd.ptrbase == NULL
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