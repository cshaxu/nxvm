/* This file is a part of NXVM project. */

// Floppy Disk Drive: 3.5", 1.44MB Floppy

#ifndef NXVM_VFDD_H
#define NXVM_VFDD_H

#include "vglobal.h"

#define VFDD_BYTES_PER_SECTOR	0x0200
#define VFDD_SECTORS_PER_TRACK	0x0012
#define VFDD_TRACKS_PER_HEAD	0x0050
#define VFDD_HEADS_PER_DISK		0x0002

typedef struct {
	t_string img;
	t_vaddrcc ptrbase;
	t_nubit8 pcn[2];	// Present Track Number
} t_fdd;

extern t_fdd vfdd;

void FDDInit();
void FDDTerm();

#endif