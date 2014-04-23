/* This file is a part of NXVM project. */

#include "stdio.h"
#include "stdlib.h"
#include "memory.h"

#include "vfdd.h"

t_string vfddimage;
t_vaddrcc vfddbase;
t_vaddrcc vfddptr;

void FDDInit()
{
	t_nubitcc count;
	FILE *imgfile = fopen(vfddimage,"rb");
	if(!imgfile) {
		// Floppy Disk Not Inserted
		vfddbase = vfddptr = 0;
	} else {
		vfddbase = (t_vaddrcc)malloc(0x168000);
		count = fread((void *)vfddbase,sizeof(t_nubit8),0x168000,imgfile);
		vfddptr = vfddbase;
		fclose(imgfile);	
	}
}
void FDDTerm()
{
	if(vfddbase) free((void *)vfddbase);
	vfddbase = vfddptr = 0;
}