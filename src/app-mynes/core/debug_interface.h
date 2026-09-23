#ifndef CORE_DEBUG_INTERFACE_H
#define CORE_DEBUG_INTERFACE_H

#include "lib/types/types_interface.h"

#define CORE_DEBUG_VERSION 1u
#define CORE_DEBUG_HEADER_BYTES 8u
#define CORE_DEBUG_DOMAIN_BYTES 4u

typedef enum core_debug_operation {
    CORE_DEBUG_OBSERVE = 1u,
    CORE_DEBUG_PEEK = 2u,
    CORE_DEBUG_POKE = 3u,
    CORE_DEBUG_STEP = 4u,
    CORE_DEBUG_BREAK_SET = 5u,
    CORE_DEBUG_BREAK_LIST = 6u,
    CORE_DEBUG_WARM_RESET = 7u,
    CORE_DEBUG_OUTPUT_SET = 8u,
    CORE_DEBUG_BIND_SET = 9u
} core_debug_operation;

#endif
