#ifndef MYDESKPRO386_MACHINE_OBSERVATION_INTERFACE_H
#define MYDESKPRO386_MACHINE_OBSERVATION_INTERFACE_H

#include "core/machine/construction_interface.h"
#include "app-mydeskpro386/d4_platform_interface.h"

typedef struct mydeskpro386_observation {
    mydeskpro386_d4_platform_observation d4;
    core_machine_fdc_terminal_observation fdc_terminal;
    lib_bool fdc_terminal_valid;
} mydeskpro386_observation;

/* Serialized with Core execution, or captured while its executor is paused.
 * The Profile owns observations; Core's attachment owns the borrowed D4.
 * A non-Model40 construction returns UNSUPPORTED without changing the output. */
lib_status mydeskpro386_observe(
    const vm_machine_construction *construction, mydeskpro386_observation *out_observation);

#endif
