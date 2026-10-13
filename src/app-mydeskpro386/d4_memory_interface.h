#ifndef MYDESKPRO386_D4_MEMORY_INTERFACE_H
#define MYDESKPRO386_D4_MEMORY_INTERFACE_H

#include "lib/types/types_interface.h"

/* Frozen DeskPro D4 RAM decoding; firmware mapping remains separate. */
typedef struct mydeskpro386_d4_memory_config {
    lib_bool present;
    lib_u8 diagnostic_low;
    lib_u8 diagnostic_high;
    lib_u16 ram_setup;
} mydeskpro386_d4_memory_config;

lib_bool mydeskpro386_d4_memory_config_is_valid(const mydeskpro386_d4_memory_config *config);

#endif
