#ifndef CORE_PLATFORM_WAIT_INTERFACE_H
#define CORE_PLATFORM_WAIT_INTERFACE_H

#include "type.h"

typedef C_INT (*core_platform_wait_cancel_predicate)(C_VOID *context);

typedef enum core_platform_wait_result {
    CORE_PLATFORM_WAIT_COMPLETED,
    CORE_PLATFORM_WAIT_CANCELLED,
    CORE_PLATFORM_WAIT_INVALID_ARGUMENT
} core_platform_wait_result;

/* Wait for at most the supplied host interval. The predicate runs only on the
 * calling host thread and may observe caller-owned state without mutating it. */
core_platform_wait_result core_platform_wait_milliseconds(type_unsigned_32 milliseconds,
    core_platform_wait_cancel_predicate cancelled, C_VOID *context);

#endif
