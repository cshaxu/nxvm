/* This file is a part of NXVM project. */

// Programmable Interval Timer: 8254

#ifndef NXVM_VPIT_H
#define NXVM_VPIT_H

#include "vglobal.h"

#define VPIT_TICK 54.925493

typedef struct {
	t_nubit8 mode0,mode1,mode2,rcw;
	t_nubit16 count0,count1,count2;
} t_pit;

extern t_pit vpit;

void vpitIntTick();

void PITInit();
void PITTerm();

#endif