#ifndef MYOB300_DISPLAY_H
#define MYOB300_DISPLAY_H

#include "x86-core/machine/machine_interface.h"
#include "x86-nxvm/chips/ct-vga/ct_vga_interface.h"

lib_status myob300_display_install(x86_core_machine *core, x86_ct_vga *chip);

#endif
