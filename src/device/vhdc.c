/* Copyright 2012-2014 Neko. */

/* VHDC is Hard Disk Driver Controller, not yet implemented. */

#include "vbios.h"
#include "vmachine.h"
#include "vhdc.h"

static void init() {vbiosAddInt(VHDC_INT_SOFT_HDD_13, 0x13);}

static void reset() {}

static void refresh() {}

static void final() {}

void vhdcRegister() {vmachineAddMe;}
