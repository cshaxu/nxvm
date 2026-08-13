#ifndef CORE_UTILS_WAIT_PROVIDER_H
#define CORE_UTILS_WAIT_PROVIDER_H

#include "type.h"

typedef C_VOID (*core_utils_wait_provider)(C_VOID *context, type_unsigned_32 milliseconds);

typedef struct core_utils_wait_scope {
    core_utils_wait_provider provider;
    C_VOID *context;
} core_utils_wait_scope;

C_VOID core_utils_wait_scope_initialize(core_utils_wait_scope *scope,
    core_utils_wait_provider provider, C_VOID *context);

#endif
