#ifndef X86_NXVM_BOARD_FDC_OBSERVATION_INTERFACE_H
#define X86_NXVM_BOARD_FDC_OBSERVATION_INTERFACE_H
#include "lib/types/types_interface.h"


typedef struct x86_nxvm_board_fdc_terminal_observation {
    lib_u64 sequence;
    lib_u8 command;
    lib_u8 drive;
    lib_u8 result[7];
    lib_u8 successful;
} x86_nxvm_board_fdc_terminal_observation;

typedef void (*x86_nxvm_board_fdc_terminal_observation_callback)(void *context,
    const x86_nxvm_board_fdc_terminal_observation *observation);

typedef struct x86_nxvm_board_fdc_terminal_observation_provider {
    x86_nxvm_board_fdc_terminal_observation_callback callback;
    void *context;
} x86_nxvm_board_fdc_terminal_observation_provider;

#endif