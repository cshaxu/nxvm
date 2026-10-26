/* Copyright 2012-2026 Neko. */
#ifndef MYPC110_PCIC_INTERFACE_H
#define MYPC110_PCIC_INTERFACE_H

#include "x86-core/machine/machine_interface.h"
#include "x86-nxvm/chips/rb5c396/rb5c396_interface.h"

lib_status mypc110_pcic_install_routes(x86_core_machine *machine,
    x86_rb5c396 *controller);

#endif
