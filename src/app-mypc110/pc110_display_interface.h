/* Copyright 2012-2026 Neko. */
#ifndef MYPC110_DISPLAY_INTERFACE_H
#define MYPC110_DISPLAY_INTERFACE_H

#include "x86-core/machine/machine_interface.h"
#include "x86-nxvm/chips/ct-vga/ct_vga_interface.h"

#define MYPC110_DISPLAY_MEMORY_START 0x000a0000u
#define MYPC110_DISPLAY_MEMORY_BYTES 0x00020000u

/* Installs only the sourced CT_VGA extension ports and the normal VGA
 * A0000h-BFFFFh CPU aperture. It is not a panel or VGA scanout contract. */
lib_status mypc110_display_install_routes(x86_core_machine *machine,
    x86_ct_vga *controller);

#endif
