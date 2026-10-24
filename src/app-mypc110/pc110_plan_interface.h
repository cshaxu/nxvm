#ifndef MYPC110_PLAN_INTERFACE_H
#define MYPC110_PLAN_INTERFACE_H

#include "x86-nxvm/machine/input_interface.h"

/* PC110 consumes the generic immutable asset views, but its 256 KiB BIOS and
 * 1 MiB Kanji font retain their App-local layout and mapping semantics. */
lib_status mypc110_machine_construction_create(const x86_nxvm_machine_config *config,
    const x86_nxvm_machine_assets *assets,
    x86_nxvm_machine_construction *out_construction);

#endif
