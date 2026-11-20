#ifndef MYOB300_MACHINE_INTERFACE_H
#define MYOB300_MACHINE_INTERFACE_H

#include "x86-nxvm/machine/input_interface.h"

#define MYOB300_SYSTEM_CARD_BYTES 0x1000000u

/* The caller supplies a verified raw System Card path. On success the
 * construction owns the loaded bytes until Machine releases the profile. */
lib_status myob300_machine_prepare(const x86_nxvm_machine_config *config,
    const char *system_card_path, x86_nxvm_machine_construction *out);

#endif
