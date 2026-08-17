#ifndef CORE_MACHINE_FDC_OBSERVATION_INTERFACE_H
#define CORE_MACHINE_FDC_OBSERVATION_INTERFACE_H

#include "type.h"

typedef struct core_machine_fdc_terminal_observation {
    type_unsigned_64 sequence;
    type_unsigned_8 command;
    type_unsigned_8 drive;
    type_unsigned_8 result[7];
    type_bool successful;
} core_machine_fdc_terminal_observation;

typedef C_VOID (*core_machine_fdc_terminal_observation_callback)(C_VOID *context,
    const core_machine_fdc_terminal_observation *observation);

typedef struct core_machine_fdc_terminal_observation_provider {
    core_machine_fdc_terminal_observation_callback callback;
    C_VOID *context;
} core_machine_fdc_terminal_observation_provider;

#endif