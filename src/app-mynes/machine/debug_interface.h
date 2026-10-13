#ifndef MACHINE_DEBUG_INTERFACE_H
#define MACHINE_DEBUG_INTERFACE_H

#include "lib/types/types_interface.h"

#define MACHINE_DEBUG_VERSION 1u
#define MACHINE_DEBUG_HEADER_BYTES 8u
#define MACHINE_DEBUG_DOMAIN_BYTES 4u

typedef enum machine_debug_operation {
    MACHINE_DEBUG_OBSERVE = 1u,
    MACHINE_DEBUG_PEEK = 2u,
    MACHINE_DEBUG_POKE = 3u,
    MACHINE_DEBUG_STEP = 4u,
    MACHINE_DEBUG_BREAK_SET = 5u,
    MACHINE_DEBUG_BREAK_LIST = 6u,
    MACHINE_DEBUG_WARM_RESET = 7u,
    MACHINE_DEBUG_OUTPUT_SET = 8u,
    MACHINE_DEBUG_BIND_SET = 9u
} machine_debug_operation;

#endif
