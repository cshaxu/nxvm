/* This file is a part of NXVM project. */

// Programmable Interval Timer: 8254

#ifndef NXVM_VPIT_H
#define NXVM_VPIT_H

#include "vglobal.h"

#define VPIT_TICK 54.925493

void vpitIntTick();

void PITInit();
void PITTerm();

#endif