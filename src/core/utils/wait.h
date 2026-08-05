#ifndef CORE_UTILS_WAIT_H
#define CORE_UTILS_WAIT_H

#include "type.h"
#include "core/utils/wait_provider.h"

C_VOID core_utils_wait_milliseconds(const core_utils_wait_scope *scope,
    uint32_t milliseconds);

#endif
