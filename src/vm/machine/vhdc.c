/* Copyright 2012-2014 Neko. */

/* VHDC is Hard Disk Driver Controller, not yet implemented. */

#include "vm/profile/default_profile/firmware/vbios.h"
#include "vm/machine/vhdc.h"

void vhdcInit() {
    vbiosAddInt(VHDC_INT_SOFT_HDD_13, 0x13);
}
void vhdcReset() {}
void vhdcRefresh() {}
void vhdcFinal() {}
