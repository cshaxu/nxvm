/* This file is a part of NXVM project. */

// Floppy Disk Drive: 3.5", 1.44MB Floppy

#ifndef NXVM_VFDD_H
#define NXVM_VFDD_H

#include "vglobal.h"

#define BYTES_PER_SECTOR	512
#define SECTORS_PER_TRACK	18
#define TRACKS_PER_HEAD		80
#define HEADS_PER_CYLENDER	2

extern t_string vfddimage;
extern t_vaddrcc vfddbase;
extern t_vaddrcc vfddptr;

void FDDInit();
void FDDTerm();

#endif