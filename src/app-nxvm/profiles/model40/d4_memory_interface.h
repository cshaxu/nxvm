#ifndef CORE_MACHINE_D4_MEMORY_INTERFACE_H
#define CORE_MACHINE_D4_MEMORY_INTERFACE_H

#include "lib/types/types_interface.h"

/* Frozen DeskPro D4 RAM decoding; firmware mapping remains separate. */
typedef struct core_machine_d4_memory_config {
    lib_u8 present;
    lib_u8 diagnostic_low;
    lib_u8 diagnostic_high;
    lib_u16 ram_setup;
} core_machine_d4_memory_config;

lib_i32 core_machine_d4_memory_config_is_valid(const core_machine_d4_memory_config *config);

#endif
