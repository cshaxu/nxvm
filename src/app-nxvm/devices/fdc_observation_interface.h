#ifndef CORE_MACHINE_FDC_OBSERVATION_INTERFACE_H
#define CORE_MACHINE_FDC_OBSERVATION_INTERFACE_H
#include "lib/types/types_interface.h"


typedef struct core_machine_fdc_terminal_observation {
    lib_u64 sequence;
    lib_u8 command;
    lib_u8 drive;
    lib_u8 result[7];
    lib_u8 successful;
} core_machine_fdc_terminal_observation;

typedef void (*core_machine_fdc_terminal_observation_callback)(void *context,
    const core_machine_fdc_terminal_observation *observation);

typedef struct core_machine_fdc_terminal_observation_provider {
    core_machine_fdc_terminal_observation_callback callback;
    void *context;
} core_machine_fdc_terminal_observation_provider;

#endif