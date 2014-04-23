/* This file is a part of NXVM project. */

// Floppy Disk Controller: Intel 8272A
// Floppy Disk Drive: File loaded to memory

#ifndef NXVM_VFDC_H
#define NXVM_VFDC_H

#include "vglobal.h"

typedef struct {
	t_nubit8 dor;		// Digital Output Register
	t_nubit8 msr;		// Main Status Register
	t_nubit8 dr;		// Data Register
	t_nubit8 dir;		// Digital Input Register
	t_nubit8 ccr;		// Configuration Control Register
// input
	/*t_nubit4 hut;	// Head Unload Time
	t_nubit4 hlt;	// Head Load Time
	t_nubit8 srt;	// Step Rate Time
	t_bool nd;		// 0 = Non-DMA Mode; 1 = DMA Mode*/
	t_nubit8 pcn;	// Present Cylinder Number
	t_bool intr;	// 0 = No Intr; 1 = Has Intr
// output
	t_nubit8 st0,st1,st2,st3;
} t_fdc;

extern t_fdc vfdc;

void FDCInit();
void FDCTerm();

#endif