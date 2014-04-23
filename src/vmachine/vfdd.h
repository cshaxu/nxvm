/* This file is a part of NXVM project. */

// Floppy Disk Drive: 3.5", 1.44MB Floppy

#ifndef NXVM_VFDD_H
#define NXVM_VFDD_H

#include "vglobal.h"

#define VFDD_BYTES		0x0200
#define VFDD_SECTORS	0x0012
#define VFDD_CYLINDERS	0x0050
#define VFDD_HEADS		0x0002

typedef struct {
	t_string img;
	t_vaddrcc ptrbase;
} t_fdd;

extern t_fdd vfdd;

t_bool vfddRead(t_nubit8 *cyl,t_nubit8 *head,t_nubit8 *sector,t_vaddrcc memloc,t_nubit8 count);	// From FDD To RAM, count sectors
t_bool vfddWrite(t_nubit8 *cyl,t_nubit8 *head,t_nubit8 *sector,t_vaddrcc memloc,t_nubit8 count);	// From RAM To FDD, count sectors
void vfddFormat(t_nubit8 fillbyte);
void FDDInit();
void FDDTerm();

#endif